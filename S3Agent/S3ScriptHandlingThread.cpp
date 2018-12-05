//#include "afxwin.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdafx.h"
#include "defines.h"
#include "S3AgentDlg.h"
#include "S3Comms.h"

extern char LastTerm;

// ----------------------------------------------------------------------------
// Script handling thread
// Disables the Connect/Disconnect button, and the manual command button during script execution.
// Executes a saved script line by line
// Stops on connection error, or "Stop Script" button, or end of sequence
UINT ScriptProcessThread(LPVOID pParam)
{
    HWND *phObjectHandle = static_cast<HWND *>(pParam);
    CS3AgentDlg *pObject = (CS3AgentDlg *)pParam;
    pObject->m_ScriptThreadRun = false;

    if (!Sentinel3.isConnected)
    {
        AfxMessageBox(_T("Cannot run script, no Sentinel 3 Connected."));
        return 1;
    }

    CString ScriptStartPoint = L"\r\nScript: ";

    int err = 0;

    //Try to open the Script file
    FILE *fid;
    int line_no = 0;
    err = fopen_s(&fid, pObject->m_TestScriptName, "r");
    if (err)
    {   // If we cannot, report to the user, and exit the thread.
        AfxMessageBox(_T("Failed to open script file."));
        return 1;
    }

    ScriptStartPoint.Append((CString)pObject->m_TestScriptName);
    ScriptStartPoint.Append(L"\r\n\r\n");

    int nLength = pObject->m_CommandEdit.GetWindowTextLength();
    pObject->m_CommandEdit.SetSel(nLength, nLength);
    pObject->m_CommandEdit.ReplaceSel(ScriptStartPoint);

    pObject->m_ScriptThreadRun = true;
    pObject->m_StartStopScriptButton.SetWindowText(_T("Stop script"));

    char line[256];
    size_t len;

	bool ShowTerm = false;

    CString MsgResponse;

    // Run each line of the script individually
    // Stop on error, end of script, or the user pressing the "Stop Script" button 
    while (!pObject->m_ScriptStop && (Sentinel3.isConnected == true) && !err && fgets(line, sizeof(line), fid))
    {

        // Cope with CR + LF
        len = strlen(line);
        if (line[len - 1] == '\n')
            line[len - 1] = '\0';

        len = strlen(line);
        if (len && line[len - 1] == '\r')
            line[len - 1] = '\0';

        line_no++;
        //Process the sequence line if it has characters
        if (len)
        {
            // If the line begins with a $, it is a directive to the script engine
            if (line[0] == '$')
            {
                CString Line(line);

                if ((Line.Mid(0, 7).CompareNoCase(L"$repeat") == 0))
                {
                    line_no = 0;
                    rewind(fid);
                }
				else if ((Line.Mid(0, 15).CompareNoCase(L"$showterminator") == 0))
                {
					ShowTerm = true;
				}
				else if ((Line.Mid(0, 15).CompareNoCase(L"$hideterminator") == 0))
                {
					ShowTerm = false;
				}
                else if ((Line.Mid(0, 6).CompareNoCase(L"$pause") == 0))
                {
                    CString msg;

                    if (len == 6)
                    {
                        msg = "Paused";
                        AfxMessageBox(msg);
                    }
                    else
                    {
                        msg = line + 6;

                        char *pEnd;
                        int t = (int)strtol(line + 6, &pEnd, 10);

                        if (*pEnd != '\0')
                            AfxMessageBox(msg);
                        else
                            Sleep(t);
                    }
                }
            }
            // If the line begins with a #, it is a comment. Do not execute. move on to the next line
            else if (line[0] == '#')
            {
                err = 0;
            }
            // If there's no special characters at the start of the line, send the command to the Seninel 3
            else
            {
                CString RespStr, CMDMsg(line);

                MsgResponse = SendSentinel3Message(CMDMsg);

                //Add the command, and its output to the output list 
                RespStr.Append(CMDMsg);
                RespStr.Append(L"\t");
                RespStr.Append(MsgResponse);

				if (ShowTerm)
				{
					CString tmp;

					if (LastTerm != '-')
						tmp.Format(_T(" (\\%02x)"), (int)LastTerm);
					else
						tmp = _T(" (-)");
					RespStr.Append(tmp);
				}

				RespStr.Append(L"\r\n");

				pObject->AddResultString(RespStr);

				//If there is a connection error, stop script running
                if ((MsgResponse.Mid(0, 6).CompareNoCase(L"ERROR:") == 0))
                {
                    err = 1;
                    AfxGetMainWnd()->PostMessage(S3_DISCONNECT, 0, 0);
                    break;
                }
            }
        }
        else
        {
            //If the sequence line has no characters, clear any errors and proceed to the next line
            err = 0;
        }
    }

    fclose(fid);

    if (err)
    {
        CString tmp, tmp2;

        tmp2 = line;

        tmp.Format(_T("Error: line %d: %s\r\n %s"), line_no, tmp2, MsgResponse);
        AfxMessageBox(tmp);
        
    }

    pObject->m_StartStopScriptButton.SetWindowText(_T("Run script"));
    pObject->m_ScriptThreadRun = false;

    return 0; // Thread completed successfully
}

