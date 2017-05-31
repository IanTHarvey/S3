// VCPTestDlg.h : header file
//
#ifndef S3USBVCP_H_
#define S3USBVCP_H_

// #define COM_BUF_SIZE		256
#define S3USB_MAX_DEVICES	256

typedef struct ComBuffer
{
	char	cBuf[COM_BUF_SIZE];
	int		size;
} COMBUFFER, *PCOMBUFFER;

#define TEXTSIZE 256

/////////////////////////////////////////////////////////////////////////////
// CVCPTestDlg dialog

class CS3AgentDlg;

class CS3USBVCP
{
// Construction
public:
	CS3USBVCP(CWnd* pParent = NULL);	// standard constructor

	CWinThread	*pThreadRead;
	CWinThread	*pThreadWrite;

	CS3AgentDlg	*m_Parent;

	int			InitData(CWnd* pParent);

	BOOL		ClosePort(void);
	BOOL		OpenPort(void);
	BOOL		IsPortOpen(void);
	BOOL		OpenPort(CString PortName);
	BOOL		GetOpenPortName(CString &PortName);
	int			Init(CWnd* pParent);
	int			Write(const char *msg);
	CString		Read();
	void		Close();
	bool		m_ReadFileFail;
	
	HANDLE		m_hPort;
	HANDLE		m_hEventWrite;
	COMBUFFER	sWriteBuffer;
	COMBUFFER	sReadBuffer;

	CString		m_RecvBuf;

	BOOL		fContinue;
	bool		m_WaitingAck;

	int			EnumCOMPorts();
	CString		GetDriverType() { return m_DriverType; };
// Implementation
protected:


private:
	BOOL		CheckDeviceType(HKEY hSubKey);
	BOOL		GetDriverPrefix(TCHAR * szPrefix, DWORD dwPrefixSize, HKEY hSubKey);
	int			CheckNameIndex(TCHAR * szName, TCHAR * szPrefix);

	void		SetUpComList(void);

	// True we're sending commands and expecting a response. If false, we're
	// listening for commands, doing as told and sending a response
	bool		m_Master;

	char		m_DevCnt;
	char		m_DevOpen;
	CString		m_DeviceList[S3USB_MAX_DEVICES];

	CString		m_DriverType;
};

#endif