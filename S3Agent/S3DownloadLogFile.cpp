
#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "afxwin.h"
#include "defines.h"
#include "S3Comms.h"
#include "S3LogCopyDlg.h"

#define MAX_RETRIES 2
extern bool LogCopyInProgress;

UINT AttemptLogFileCopyThread(LPVOID pParam)
{
    HWND *phObjectHandle = static_cast<HWND *>(pParam);
    CS3LogCopyDlg *pObject = (CS3LogCopyDlg *)pParam;
    LogCopyInProgress = true;

    pObject->m_FileCopyProgressBar.SetRange(0,100);
    pObject->m_FileCopyProgressBar.SetPos(0);

    CString Logfileaddr;
    char Filename[MAX_PATH];
    pObject->m_FileDirEdit.GetWindowTextW(Logfileaddr);
    sprintf_s(Filename, MAX_PATH, "%ls", Logfileaddr);
    
    int progresspercent = 0;

    int err = fopen_s(&pObject->Logfile, Filename, "wb");
    if(err)
    {
        //File open error
        LogCopyInProgress = false;
        pObject->m_CopyFileButton.EnableWindow(true);
        pObject->m_BrowseButton.EnableWindow(true);
        pObject->m_FileDirEdit.EnableWindow(true);
        pObject->m_FileCopyProgressBar.EnableWindow(false);
        pObject->m_PercentageStatic.EnableWindow(false);
        AfxMessageBox(_T("Error opening log file."), MB_OK | MB_ICONEXCLAMATION);
        return 1;
    }


    CString Command, Args, Response;
    Command = L"S3COPYLOG";
    Args.Format(_T(" SIZE"));
    
    Command.Append(Args);
    Response = SendSentinel3Message(Command);

    //If we recieve a Comms Error message, or an error message from the Sentinel
    if ((Response.Mid(0, 6).CompareNoCase(L"ERROR:") == 0) || (Response.Mid(0, 2).CompareNoCase(L"E:") == 0))
    {
        CString Message = L"Error:\r\n";
        Message.Append(Response);
        AfxMessageBox(Message, MB_OK);
        LogCopyInProgress = false;
        pObject->m_CopyFileButton.EnableWindow(true);
        pObject->m_BrowseButton.EnableWindow(true);
        pObject->m_FileDirEdit.EnableWindow(true);
        pObject->m_FileCopyProgressBar.EnableWindow(false);
        pObject->m_PercentageStatic.EnableWindow(false);
        fclose(pObject->Logfile);
        return 2;
    }
    int MaxPacketNo = _ttoi(Response);

    pObject->m_CopyFileButton.SetWindowTextW(L"Copying...");
    pObject->m_CancelButton.SetWindowTextW(L"Cancel");
    CString Progresstxt;
    int oldprogress = 0;
    bool isEndOfFile = false;
    int i = 1, retrynum = 0;
    while(LogCopyInProgress && !isEndOfFile)
    {
        Command = L"S3COPYLOG";
        Args.Format(_T(" %d"), i);

        Command.Append(Args);
        Response = SendSentinel3Message(Command);
        if ((Response.Mid(0, 6).CompareNoCase(L"ERROR:") == 0) || (Response.Mid(0, 2).CompareNoCase(L"E:") == 0))
        {
            //Error - retry after 0.5ms
            Sleep(500);
            retrynum++;

            if(retrynum > MAX_RETRIES)
            {
                CString Message;
                Message.Format(_T("Comms Error at Packet %d. Aborting"), i);
                AfxMessageBox(Message, MB_OK);
                LogCopyInProgress = false;
                pObject->m_CopyFileButton.EnableWindow(true);
                pObject->m_BrowseButton.EnableWindow(true);
                pObject->m_FileDirEdit.EnableWindow(true);
                pObject->m_FileCopyProgressBar.EnableWindow(false);
                pObject->m_PercentageStatic.EnableWindow(false);
                fclose(pObject->Logfile);
                pObject->m_FileCopyProgressBar.SetPos(0);
                pObject->m_PercentageStatic.SetWindowTextW(L"0%");
                pObject->m_CopyFileButton.SetWindowTextW(L"Copy");
                pObject->m_CancelButton.SetWindowTextW(L"OK");
                return 3;
            }
        }
        else
        {
            Response.FreeExtra();
            fprintf(pObject->Logfile, "%ls", Response.Left(Response.GetLength() - 2));

            progresspercent = (int)((i/(float)MaxPacketNo)*100);
            if(progresspercent != oldprogress)
            {
                pObject->m_FileCopyProgressBar.SetPos(progresspercent);
                Progresstxt.Format(_T("%d%%"),progresspercent);
                pObject->m_PercentageStatic.SetWindowTextW(Progresstxt);
                oldprogress = progresspercent;
            }
            if(i == MaxPacketNo)
            {
                isEndOfFile = true;
            }
            i++;
            retrynum = 0;
        }
    }
    
    fclose(pObject->Logfile);
    
    if((int)progresspercent >= 100)
    {
        pObject->m_FileCopyProgressBar.SetPos((int)progresspercent);
        AfxMessageBox(_T("Log File copy completed."), MB_OK | MB_ICONINFORMATION);
        LogCopyInProgress = false;
        pObject->m_CopyFileButton.EnableWindow(true);
        pObject->m_BrowseButton.EnableWindow(true);
        pObject->m_FileDirEdit.EnableWindow(true);
        pObject->m_FileCopyProgressBar.EnableWindow(false);
        pObject->m_PercentageStatic.EnableWindow(false);
    }
    pObject->m_CopyFileButton.SetWindowTextW(L"Copy");
    pObject->m_CancelButton.SetWindowTextW(L"OK");
    return 0;
}