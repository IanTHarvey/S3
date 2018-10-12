// S3USBVCP.cpp : implementation file
//
// Multithreaded MFC based  
// 1.	Enumerate FTDI and Prolific VCP devices
// 2.	Open, Close, Read and Write to the device
//	
// See: http://www.ftdichip.com/Support/SoftwareExamples/CodeExamples/VC++/VCPTest_vcpp.zip
//

#include "stdafx.h"

#include "resource.h"

#include "S3DataModel.h"

#include "S3GPIB.h"
#include "S3USBVCP.h"
#include "S3ControllerDlg.h"

#ifdef S3_USB_SLAVE
#include "S3DataModel.h"
#include "S3ControllerDlg.h"

#include "S3GPIB.h"
#else
#define S3_CMD_TERMINATOR		'\n' // Line feed/0xA/10
#endif

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

// TODO: These are FTDI-specific locations and different to the Prolific
// ones.

#ifdef TRIZEPS
// WEC
#define REG_NAME_KEY		"Key"
#define REG_NAME_NAME		"Name"
#define REG_NAME_PREFIX		"COM"
#define REG_NAME_PROL_KEY	"Drivers\\USBSER"
#define REG_NAME_FTDI_KEY	"Drivers\\USB\\ClientDrivers\\FTDI_DEVICE"
#define REG_NAME_ACTIVE		"Drivers\\Active"

#else

// Windows desktop
#define REG_NAME_KEY		"Key"
#define REG_NAME_NAME		"Name"
#define REG_NAME_PREFIX		"COM"
#define REG_NAME_FTDI_KEY	"\\Device\\VCP" // May be VCP[012...]
#define REG_NAME_PROL_KEY	"\\Device\\ProlificSerial0"
#define REG_NAME_ACTIVE		"HARDWARE\\DEVICEMAP\\SERIALCOMM"
#endif

#define READ_THREAD_WAIT_TIMEOUT	5000
#define WRITE_THREAD_WAIT_TIMEOUT	5000

UINT SendThread(		LPVOID pArg);
UINT ReadThreadMaster(	LPVOID pArg);
UINT ReadThreadSlave(	LPVOID pArg);

/////////////////////////////////////////////////////////////////////////////
// CS3USBVCP dialog


// TODO: Temp - this replaces the original List control

// ---------------------------------------------------------------------------

CS3USBVCP::CS3USBVCP(CWnd* pParent /*=NULL*/)
{
	InitData(pParent);
}

// ---------------------------------------------------------------------------

int CS3USBVCP::InitData(CWnd* pParent)
{
	m_hPort = INVALID_HANDLE_VALUE;
	fContinue = TRUE;
	sReadBuffer.size = S3_MAX_GPIB_CMD_LEN;
	sWriteBuffer.size = S3_MAX_GPIB_RET_LEN;
	m_DevCnt = 0;
	m_DevOpen = -1;

	pThreadRead = pThreadWrite = NULL;

	m_Parent = (CS3ControllerDlg *)pParent;

	m_Master = true;

	m_ReadFileFail = false;

	m_DriverType = _T("N/A");

	return 0;
}

// ---------------------------------------------------------------------------
// CS3USBVCP message handlers

// ---------------------------------------------------------------------------
// Applicable to slave application.

int CS3USBVCP::Init(CWnd* pParent)
{
	InitData(pParent);
	
	m_Master = false;

	int Ports = EnumCOMPorts();

	return Ports;
}

// ----------------------------------------------------------------------------
// Write button press event handler
//
int CS3USBVCP::Write(const char *msg) 
{
	if (m_hPort == INVALID_HANDLE_VALUE)
	{
		TEXT("Error: Port not open");
		return 1;
	}

	int iCount = strlen(msg);

	if (iCount >= S3_MAX_GPIB_RET_LEN) 
		iCount = S3_MAX_GPIB_RET_LEN - 1;

	// Terminator setting irrelevant here, as must be terminated with '\n'
	// anyway.
	strcpy_s(sWriteBuffer.cBuf, S3_MAX_GPIB_RET_LEN, msg);
	sWriteBuffer.size = iCount;

	// Allow the write thread to write to the USB stream
	SetEvent(m_hEventWrite);

	if (m_Master)
		m_WaitingAck = true;

	return 0;
}

// ----------------------------------------------------------------------------
//	Purpose: Signal from a read will send characters to the serial port

UINT SendThread(LPVOID pArg) 
{
    DWORD dwBytes, dwGoCode;
	CS3USBVCP *pMyHndl = (CS3USBVCP *)pArg;

	// Infinite loop to write to port until close
    while (pMyHndl->fContinue)
	{
		// Wait for a signal to write (or terminate the thread)
		dwGoCode = WaitForSingleObject(pMyHndl->m_hEventWrite, INFINITE);

		// Don't try to write if thread is being shut down (fContinue)
        if (dwGoCode == WAIT_OBJECT_0 && pMyHndl->fContinue)
		{
			*(pMyHndl->sWriteBuffer.cBuf + pMyHndl->sWriteBuffer.size - 1) = 0x03; // EOM 
			// pMyHndl->sWriteBuffer.size++;
			
			*(pMyHndl->sWriteBuffer.cBuf + pMyHndl->sWriteBuffer.size) = 0x00; // NL 
			
			// Write size worth of data to the port
            int err = WriteFile(
				pMyHndl->m_hPort,
				pMyHndl->sWriteBuffer.cBuf, 
				pMyHndl->sWriteBuffer.size,
                &dwBytes, 
				NULL);
        } 
		else
			return 1;
    }
    return 0;
}

// ----------------------------------------------------------------------------
//	Name: ReadThreadSlave
//
// Purpose: Continually receives characters from the serial port if available
// and responds as required like a good boy

UINT ReadThreadSlave(LPVOID pArg) 
{
#ifdef S3_USB_SLAVE
    DWORD		dwBytes;
    BYTE		szText[TEXTSIZE], *pPtr;
	CS3USBVCP	*pMyHndl = (CS3USBVCP *)pArg;
	CString		str;
	char		cStr[128];
	int			idx = 0;

	// Infinite loop to read from port until close
    while (pMyHndl->fContinue)
	{
        pPtr = szText;
		if (pMyHndl->m_hPort == INVALID_HANDLE_VALUE)
			return 1;

		// Read 1 byte - not very efficient this way
		BOOL OK = ReadFile(
			pMyHndl->m_hPort, 
			pMyHndl->sReadBuffer.cBuf, 
			1, 
			&dwBytes, 
			NULL);

		pMyHndl->m_ReadFileFail = (OK == FALSE);

		if (dwBytes)
		{
			// 10 \n linefeed
			// 13 \r carriage return
			cStr[idx++] = pMyHndl->sReadBuffer.cBuf[0];
			// cStr[idx++] = '\n';
			cStr[idx] = '\0';
			
			// if (pMyHndl->sReadBuffer.cBuf[0] == '\0')
			if (pMyHndl->sReadBuffer.cBuf[0] == S3_CMD_TERMINATOR)
			{
				cStr[idx - 1] = '\0';  // Replace S3_CMD_TERMINATOR

				int err = (pMyHndl->m_Parent)->ParseMsg(cStr, S3_USB);
				
				pMyHndl->Write(S3GPIBGetRetBuf());

				cStr[0] = '\0';
				idx = 0;

				str = _T("");
				pMyHndl->sReadBuffer.size = 0;
			}
		}
    }
#endif
    return 0;
}

// ----------------------------------------------------------------------------
//	Name: ReadThreadMaster
//
// Purpose: Continually receives characters from the serial port if available
// and processes response

UINT ReadThreadMaster(LPVOID pArg) 
{
    DWORD dwBytes;
    BYTE szText[TEXTSIZE], *pPtr;
	CS3USBVCP *pMyHndl = (CS3USBVCP *)pArg;
	CString	str;

	// Infinite loop to read from port until close
    while (pMyHndl->fContinue)
	{
        pPtr = szText;
		if (pMyHndl->m_hPort == INVALID_HANDLE_VALUE)
			return 1;

		// Read 1 byte - not very efficient this way
		BOOL OK = ReadFile(
			pMyHndl->m_hPort, 
			pMyHndl->sReadBuffer.cBuf, 
			1, 
			&dwBytes, 
			NULL);

		if (dwBytes)
		{
			str += pMyHndl->sReadBuffer.cBuf[0];

			// if (pMyHndl->sReadBuffer.cBuf[0] == '\0')
			if (pMyHndl->sReadBuffer.cBuf[0] == S3_CMD_TERMINATOR)
			{
				// TODO: Check we get some meaningful response
				// TODO: Implement a timeout here

				pMyHndl->m_RecvBuf = str;

				str = _T("");
				pMyHndl->sReadBuffer.size = 0;

				pMyHndl->m_WaitingAck = false;
			}
		}
    }

    return 0;
}

// ----------------------------------------------------------------------------

void CS3USBVCP::Close() 
{
	ClosePort();
}

// ----------------------------------------------------------------------------
//	Name: OpenPort
//
//	Purpose:	Open the selected port from the list box
//				Setup port if opened
//				Start the Send and Receive threads
//

BOOL CS3USBVCP::OpenPort()
{
	ClosePort();
	
	int Ports = EnumCOMPorts();

	if (Ports)
	{
		for(char i = 0; i < m_DevCnt; i++)
		{
			if (OpenPort(m_DeviceList[i]))
			{
				m_DevOpen = i;
				return true;
			}
			else
			{
				// No point in doing this if handle is already invalid
				// from attempt to open
				ClosePort();
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------------

BOOL CS3USBVCP::IsPortOpen()
{
	if (m_hPort == INVALID_HANDLE_VALUE)
		return FALSE;

	if (m_ReadFileFail)
		return FALSE;
	
	return TRUE;
}

// ----------------------------------------------------------------------------

BOOL CS3USBVCP::GetOpenPortName(CString &PortName)
{
	if (m_DevOpen != -1)
	{
		PortName = m_DeviceList[m_DevOpen];
		return true;
	}

	PortName = _T("N/A");

	return false;
}

// ----------------------------------------------------------------------------

BOOL CS3USBVCP::OpenPort(CString PortName)
{
	COMMTIMEOUTS	cto;
	DCB				dcbCommPort;


	if (m_hPort != INVALID_HANDLE_VALUE && m_ReadFileFail)
		ClosePort();

	if (m_hPort == INVALID_HANDLE_VALUE)
	{
		// Get Selected COM (or other if prefix has changed) port name from list box

		// Open the port
		m_hPort = CreateFile (
					PortName,							// Pointer to the name of the port
					GENERIC_READ | GENERIC_WRITE,	// Access (read-write) mode
					0,								// Share mode
					NULL,							// Pointer to the security attribute
					OPEN_EXISTING,					// How to open the serial port
					0,								// Port attributes
					NULL);							// Handle to port with attribute to copy

		if(m_hPort != INVALID_HANDLE_VALUE)
		{
			// Setup the port
			memset(&dcbCommPort, 0, sizeof(DCB));
			dcbCommPort.BaudRate	= CBR_9600;
			dcbCommPort.ByteSize	= 8;
			dcbCommPort.Parity		= NOPARITY;
			dcbCommPort.StopBits	= ONESTOPBIT;
		//	dcbCommPort.fOutxCtsFlow = TRUE;
		//	dcbCommPort.fRtsControl = RTS_CONTROL_HANDSHAKE;
			SetCommState(m_hPort, &dcbCommPort);

			// Set read and write timeouts to infinite
			cto.ReadIntervalTimeout			= 1;
			cto.ReadTotalTimeoutMultiplier	= 1;
			cto.ReadTotalTimeoutConstant	= 0;
			cto.WriteTotalTimeoutMultiplier = 0;
			cto.WriteTotalTimeoutConstant	= 0;
			SetCommTimeouts(m_hPort, &cto);

			// Initialise the Send and Receive threads
			fContinue		= TRUE;
			m_hEventWrite	= CreateEvent(NULL, FALSE, FALSE, NULL);
			
			if (m_Master)
				pThreadRead	= AfxBeginThread(ReadThreadMaster, this, THREAD_PRIORITY_NORMAL);	
			else
				pThreadRead	= AfxBeginThread(ReadThreadSlave, this, THREAD_PRIORITY_NORMAL);

			pThreadWrite = AfxBeginThread(SendThread, this, THREAD_PRIORITY_NORMAL);
		}
		else
		{
			// Failed to open - return false
			GetErrString();
			return FALSE;
		}
	}

#ifdef S3_USB_SLAVE
	S3SetUSBOpen(true);
#endif

	return TRUE;
}

// ----------------------------------------------------------------------------
//	Name: ClosePort
//
//	Purpose:	Close the currently opened port
//				Terminate the Send and Receive threads

BOOL CS3USBVCP::ClosePort()
{
	if (m_hPort == INVALID_HANDLE_VALUE)
	{
		return TRUE;
	}

	// Force need to re-enumerate
	m_DevCnt = 0;
	m_DevOpen = -1;
	
	m_DriverType = _T("N/A");

	// Stop both threads from looping
	fContinue = FALSE;

	// Stop the writing thread by signal (by allowing to see fContinue)
	SetEvent(m_hEventWrite);

	if (pThreadRead &&
		WaitForSingleObject(pThreadRead->m_hThread, READ_THREAD_WAIT_TIMEOUT) == WAIT_TIMEOUT)
	{
		// Didn't exit nicely so kill it
		TEXT("Error RThread");
		TerminateThread(pThreadRead->m_hThread, 0xFFFFFFFF);
	}

	// Release this resource
	if (m_hEventWrite)
	{
		CloseHandle(m_hEventWrite);
	}

	if (pThreadWrite &&
		WaitForSingleObject(pThreadWrite->m_hThread, WRITE_THREAD_WAIT_TIMEOUT) == WAIT_TIMEOUT)
	{
		// Didn't exit nicely so kill it
		TEXT("Error WThread");
		TerminateThread(pThreadWrite->m_hThread, 0xFFFFFFFF);
	}

	CloseHandle(m_hPort);
	m_hPort = INVALID_HANDLE_VALUE;

#ifdef S3_USB_SLAVE
	S3SetUSBOpen(false);
#endif

	return TRUE;
}

// ----------------------------------------------------------------------------
//	Name: EnumCOMPorts
//
//	Purpose: Find all COM ports related to FTDI or Prolific device on Windows
//  desktop and WEC platforms. Drivers and registry are different for each
//	combination.
//

#ifndef TRIZEPS

// Desktop
int CS3USBVCP::EnumCOMPorts(bool test)
{
	INT		i = 0, iRet;
	HKEY	hKey;
	wchar_t	szName[128];
	DWORD	dwType, dwSize;
	CString	NewDeviceList[S3USB_MAX_DEVICES];
	char	NewDevCnt;

	NewDevCnt = 0;

	// Open the "HARDWARE\\DEVICEMAP\\SERIALCOMM" key 
	iRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T(REG_NAME_ACTIVE), 0, 
											KEY_READ | KEY_WOW64_32KEY, &hKey);
	if (iRet != ERROR_SUCCESS)
	{
		return 0;
	}

	// Search through all devices
	while (NewDevCnt < S3USB_MAX_DEVICES)
	{
		// Read each key, finish loop when RegEnumValue fails
		dwSize = sizeof(szName);
		
		iRet = RegEnumValue(hKey, i++, szName, &dwSize, NULL, NULL, NULL, NULL);

		if (iRet == ERROR_SUCCESS)
		{
			// Check the "Key" value for an FTDI device "VCP"
			if (_tcslen(szName) >= _tcslen(_T(REG_NAME_FTDI_KEY)))
			{
				if (_tcsncmp(szName, _T(REG_NAME_FTDI_KEY), _tcslen(_T(REG_NAME_FTDI_KEY))) == 0)
				{
					// Key name good, get the value
					iRet = RegQueryValueEx(hKey, szName, 0, &dwType, (PBYTE)szName, &dwSize);

					if (iRet == ERROR_SUCCESS)
					{
						if(CheckNameIndex(szName, _T(REG_NAME_PREFIX)) != -1)
						{
							NewDeviceList[NewDevCnt++] = szName;
							m_DriverType = _T("FTDI");
						}
					}
				}
			}

			// ...and for a Prolific device
			if (_tcslen(szName) >= _tcslen(_T(REG_NAME_PROL_KEY)))
			{
				if (_tcsncmp(szName, _T(REG_NAME_PROL_KEY), _tcslen(_T(REG_NAME_PROL_KEY))) == 0)
				{
					// Get the "COMX:" name
					iRet = RegQueryValueEx(hKey, szName, 0, &dwType, (PBYTE)szName, &dwSize);

					// And extract the number
					if (iRet == ERROR_SUCCESS)
					{
						if(CheckNameIndex(szName, _T(REG_NAME_PREFIX)) != -1)
						{
							NewDeviceList[NewDevCnt++] = szName;
							m_DriverType = _T("Prolific");
						}
					}
				}
			}
		}
		else
			break; // No more enumerated keys
	}

	RegCloseKey(hKey);

	if (!test)
	{
		for(char i = 0; i < NewDevCnt; i++)
			m_DeviceList[i] = NewDeviceList[i];

		m_DevCnt = NewDevCnt;
	}
	else
	{
		for(char i = 0; i < S3USB_MAX_DEVICES; i++)
		{
			if (NewDeviceList[i] != m_DeviceList[i])
			{
				Close();
			}
		}
	}

	if (m_DevCnt == 0)
		m_DriverType = _T("N/A");

	return m_DevCnt;
}

#else
// WEC
int CS3USBVCP::EnumCOMPorts(bool test)
{
	INT		i = 0, iRet;
	HKEY	hKey, hSubKey;
	wchar_t	szName[128];
	DWORD	dwType, dwSize;
	CString	NewDeviceList[S3USB_MAX_DEVICES];
	char	NewDevCnt;

	NewDevCnt = 0;

	// Open the Drivers/Active key to search through all devices
	// "Drivers\\USB\\ClientDrivers\\FTDI_DEVICE"

	// Open "Drivers\\Active" key
	long ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T(REG_NAME_ACTIVE), 0, 
												KEY_READ, &hKey);
	if(ret != ERROR_SUCCESS)
	{
		return 0;
	}

	while (m_DevCnt < S3USB_MAX_DEVICES)
	{
		// Read each 2-digit key, finish loop when RegEnumValue fails
		dwSize = sizeof(szName);

		iRet = RegEnumKeyEx(hKey, i++, szName, &dwSize, NULL, NULL, NULL, NULL);
		if(iRet != ERROR_SUCCESS)
		{
			break;
		}

		// Open Drivers/Active/XX szName=number key. This is an extra
		// key level compared to the desktop version above.
		iRet = RegOpenKeyEx(hKey, szName, 0, KEY_READ, &hSubKey);

		if (iRet !=  ERROR_SUCCESS)
		{
			continue;
		}

		// Get the "Name" subkey (holds the COM etc...) of the Drivers/Active/XX key
		dwSize = sizeof(szName);
		iRet = RegQueryValueEx(hSubKey, _T(REG_NAME_KEY), 0, &dwType, (PBYTE)szName, &dwSize);

		if(iRet ==  ERROR_SUCCESS)
		{
			// Check the "Key" value for an FTDI device
			if (_tcslen(szName) >= _tcslen(_T(REG_NAME_FTDI_KEY)))
			{
				if (_tcsncmp(szName, _T(REG_NAME_FTDI_KEY), _tcslen(_T(REG_NAME_FTDI_KEY))) == 0)
				{
					// Get the "COMX:" name
					iRet = RegQueryValueEx(hSubKey, _T(REG_NAME_NAME), 0, &dwType, (PBYTE)szName, &dwSize);

					// And extract the number
					if (iRet == ERROR_SUCCESS)
					{
						if(CheckNameIndex(szName, _T(REG_NAME_PREFIX)) != -1)
						{
							NewDeviceList[NewDevCnt++] = szName;
							m_DriverType = _T("FTDI");
						}
					}
				}
			}

			// ...and same for a Prolific device
			if (_tcslen(szName) >= _tcslen(_T(REG_NAME_PROL_KEY)))
			{
				if (_tcsncmp(szName, _T(REG_NAME_PROL_KEY), _tcslen(_T(REG_NAME_PROL_KEY))) == 0)
				{
					iRet = RegQueryValueEx(hSubKey, _T(REG_NAME_NAME), 0, &dwType, (PBYTE)szName, &dwSize);

					if (iRet == ERROR_SUCCESS)
					{
						if(CheckNameIndex(szName, _T(REG_NAME_PREFIX)) != -1)
						{
							NewDeviceList[NewDevCnt++] = szName;
							m_DriverType = _T("Prolific");
						}
					}
				}
			}
		}
		else break;
	
		RegCloseKey(hSubKey);
	}

	RegCloseKey(hKey);

	if (!test)
	{
		for(char i = 0; i < NewDevCnt; i++)
			m_DeviceList[i] = NewDeviceList[i];

		m_DevCnt = NewDevCnt;
	}
	else
	{
		for(char i = 0; i < S3USB_MAX_DEVICES; i++)
		{
			if (NewDeviceList[i] != m_DeviceList[i])
			{
				Close();
			}
		}
	}

	return m_DevCnt;
}

#endif

// ----------------------------------------------------------------------------
//	Name: CheckNameIndex
//
//	Purpose: Check out name against the Prefix
//				
int CS3USBVCP::CheckNameIndex(TCHAR * szName, TCHAR * szPrefix)
{
	TCHAR cBuf[6];	// our COM or MOC or whatever name
	int iNum;
	int iRet;
		
	_tcsncpy_s(cBuf, 6, szName, 6);
	if(_tcsncmp(cBuf, szPrefix, 3) != 0)
		return -1;

	// Get the index from the device name
	iRet = swscanf_s(&cBuf[3], L"%d:", &iNum);
	if((iRet == 0) || (iRet == EOF))
		return -1;
	
	return iNum;
}

// ----------------------------------------------------------------------------
//	Name: CheckDeviceType
//
//	Purpose: Check it is an FTDI device
//				
BOOL CS3USBVCP::CheckDeviceType(HKEY hSubKey)
{
	INT rc;
	TCHAR szName[128];
	DWORD dwType, dwSize;

	dwSize = sizeof(szName);

	// Get the "Key" value in Drivers\Active\XX into szName
	rc = RegQueryValueEx(hSubKey, _T(REG_NAME_KEY), 0, &dwType,
										(PBYTE)szName, &dwSize);
	if (rc !=  ERROR_SUCCESS)
	{
		return FALSE;
	}

	// Check the "Key" value against what we would expect for and FTDI device
	if (_tcslen(szName) >= _tcslen(_T(REG_NAME_FTDI_KEY)))
	{
		if(_tcsncmp(szName, _T(REG_NAME_FTDI_KEY), _tcslen(_T(REG_NAME_FTDI_KEY))) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//	Name: GetDriverPrefix
//
//	Purpose: Get the "Prefix" from the registry
//				

BOOL CS3USBVCP::GetDriverPrefix(TCHAR * szPrefix, DWORD dwPrefixSize, HKEY hSubKey)
{
	INT		rc;
	TCHAR	szName[128];
	DWORD	dwType, dwSize;
	HKEY	hKey = NULL;

	dwSize = sizeof(szName);

	// Get the "Key" value in Drivers\Active\XX into szName
	rc = RegQueryValueEx(hSubKey, _T(REG_NAME_KEY), 0, &dwType,
								(PBYTE)szName, &dwSize);

	// Open our key name
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szName, 0, 0, &hKey) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	// Get the "Prefix" key into szPrefix
	rc = RegQueryValueEx(hKey, _T(REG_NAME_PREFIX), 0, &dwType,
								(PBYTE)szPrefix, &dwPrefixSize);
	if (rc !=  ERROR_SUCCESS)
	{
		return FALSE;
	}

	if(hKey)
	{
		RegCloseKey(hKey);
	}

	return TRUE;
}

// ----------------------------------------------------------------------------

const char *GetErrString()
{
	wchar_t *s = NULL;
	int err = GetLastError();
	FormatMessageW(	FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);

	sprintf_s(WSAErrString, WSA_ERR_STR_LEN, "%d: %S", err, s);

	LocalFree(s); // FormatMessageW uses LocalAlloc()

	return WSAErrString;
}

// ------------------------------ The End -------------------------------------
