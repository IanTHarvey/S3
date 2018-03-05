#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"

#include "S3AgentDlg.h"
#include "S3AgentAboutDlg.h"
#include "afxwin.h"
#include "S3LogCopyDlg.h"



//For Serial port enumeration
#define REG_NAME_ACTIVE		"HARDWARE\\DEVICEMAP\\SERIALCOMM"


#define M_SCREEN_WIDTH		800
#define M_SCREEN_HEIGHT		480

#define S3SCREENSMALL 0
#define S3SCREENLARGE 1

extern int S3TimerInit();
extern int S3TimerStart(unsigned char Tid);
extern int S3TimerStop(unsigned char Tid);

// TODO: Make static members
SOCKET ConnectSocket;
char	RxBuf[DEFAULT_BUFLEN];
bool	gScriptRunning = false;
//Connection to Sentinel3 method flag
int connectionmethod = 0;

// USB virtual COM port stuff
bool			m_USBEnabled;
CS3USBVCP		*m_COMPort;

//GPIB Connection data
Addr4882_t GPIBAddress;


UINT SendMessageThread(LPVOID pParam);

//Sentinel3 Connection Details
CString COM_name;
CString IPv4Addr;
CString IPV4Port;
//Preferenced storage location
TCHAR DataLocStr[MAX_PATH];

#define IDT_S3_GUI_UPDATE_TIMER	(WM_USER + 200)

void CS3AgentDlg::OnGUIUpdateTimer(void)
{    
	m_AnimateState = !m_AnimateState;
	// Redraw GDI region
	m_GDIStatic.S3GDIRedraw();

	return;
}
void CS3AgentDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == IDT_S3_GUI_UPDATE_TIMER)
	{		
		OnGUIUpdateTimer();
	}
	CDialog::OnTimer(nIDEvent);
}

// CS3AgentDlg dialog

CS3AgentDlg::CS3AgentDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CS3AgentDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON2);

	SOCKET ConnectSocket = INVALID_SOCKET;
	m_COMPort = NULL;
    m_AnimateState = 0;

	m_SWUpdateScheduled = false;
	m_AppUpdateScheduled = false;

	connectionmethod = ETHERNET; // Set default connection method
}

void CS3AgentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    
    //Buttons
	DDX_Control(pDX, IDC_RUN_SCRIPT_BUTTON, m_StartStopScriptButton);
    DDX_Control(pDX, IDC_CONNECT_BUTTON, m_ConnectS3Button);
    DDX_Control(pDX, IDC_MAN_CMD_SEND_BUTTON, m_SendManCMDButton);

    //Drop Down
    DDX_Control(pDX, IDC_SERIAL_DROP, m_SerialDropDown);

    //RadioButtons
    DDX_Control(pDX, IDC_ETH_RDO, m_EthSelRdoButton);
    DDX_Control(pDX, IDC_USB_RDO, m_USBSelRdoButton);
    DDX_Control(pDX, IDC_USBSERIAL, m_SerialSelRdoButton);
    DDX_Control(pDX, IDC_USBGPIB, m_GPIBSelRdoButton);

    //Tickboxes
    DDX_Control(pDX, IDC_AUTOUPDATE_GUI, m_AutoUpdateGUITick);
    DDX_Control(pDX, IDC_REMOTE_LOCAL_CHECK, m_RemoteLocalCheck);

    //EditTexts
	DDX_Control(pDX, IDC_MAN_CMD_EDIT, m_MessageEdit);
	DDX_Control(pDX, IDC_IPV4_ADDR_EDIT, m_IPv4AddrEdit);
	DDX_Control(pDX, IDC_IPV4_PORT_EDIT, m_PortEdit);

    //TextViews
	DDX_Control(pDX, IDC_SCRIPT_NAME_STATIC, m_ScriptNameStatic);
	DDX_Control(pDX, IDC_COMMAND_EDIT, m_CommandEdit);

    //GDI Region for S3 UI emulation
    DDX_Control(pDX, IDC_GDI_FRAME, m_GDIStatic);  
    DDX_Control(pDX, IDC_IMG_STATIC, m_IMGStatic);

    DDX_Control(pDX, IDC_MANUAL_CONTROL_GROUP, m_ManualGroupBox);
}

BEGIN_MESSAGE_MAP(CS3AgentDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_TIMER()

    //Connection Group
    ON_BN_CLICKED(IDC_CONNECT_BUTTON, &CS3AgentDlg::OnBnClickedS3ConnectButton)
    ON_BN_CLICKED(IDC_ETH_RDO, &CS3AgentDlg::OnBnClickedS3Eth)
    ON_BN_CLICKED(IDC_USB_RDO, &CS3AgentDlg::OnBnClickedS3USB)
    ON_BN_CLICKED(IDC_USBSERIAL, &CS3AgentDlg::OnBnClickedS3USBSerial)
    ON_BN_CLICKED(IDC_USBGPIB, &CS3AgentDlg::OnBnClickedS3USBGPIB)

    //Script Control
    ON_BN_CLICKED(IDC_RUN_SCRIPT_BUTTON, &CS3AgentDlg::OnBnClickedRunScriptButton)
    ON_BN_CLICKED(IDC_CHOOSE_SCRIPT_BUTTON, &CS3AgentDlg::OnBnClickedChooseScriptButton)
    
    //Manual Control
    ON_BN_CLICKED(IDC_MAN_CMD_SEND_BUTTON, &CS3AgentDlg::OnBnClickedSendS3MsgButton)

    //Menu Items
    ON_BN_CLICKED(ID_FILE_QUIT, &CS3AgentDlg::ExitS3RemController)
    ON_BN_CLICKED(ID_ABOUT_APP, &CS3AgentDlg::AboutDlg)
    ON_BN_CLICKED(ID_AUTOSYNC_TIME, &CS3AgentDlg::OnBnClickedAutoSyncTime)
    ON_BN_CLICKED(ID_MANUAL_ENTRY, &CS3AgentDlg::OnBnClickedManualEntry)
    ON_BN_CLICKED(ID_LOGFILECOPY, &CS3AgentDlg::OnBnClickedLogFileCopy)

    //Remote GUI
    ON_BN_CLICKED(IDC_AUTOUPDATE_GUI, &CS3AgentDlg::OnAutoUpdateGUIBtnClick)
    ON_BN_CLICKED(IDC_REMOTE_LOCAL_CHECK, &CS3AgentDlg::OnRemoteLocalPermissionsBtnClick)

    //Other UI
    ON_MESSAGE(WM_LOGCOPYCLOSED, &CS3AgentDlg::OnLogCopyFinished)
    ON_MESSAGE(S3_DISCONNECT, &CS3AgentDlg::OnDisconnect)

END_MESSAGE_MAP()


// ----------------------------------------------------------------------------
// CS3AgentDlg message handlers
BOOL CS3AgentDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);



	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	if(!m_ToolTip.Create(this))
    {
        TRACE0("Unable to create the tooltip");
    }
    else
    {
        m_ToolTip.AddTool(&m_USBSelRdoButton, _T("Remote GUI refresh rate will be severely limited when using Serial/GPIB connection (10s update frequency)."));
        m_ToolTip.AddTool(&m_SerialSelRdoButton, _T("Remote GUI refresh rate will be severely limited when using Serial/GPIB connection (10s update frequency)."));
        m_ToolTip.AddTool(&m_GPIBSelRdoButton, _T("Remote GUI refresh rate will be severely limited when using Serial/GPIB connection (10s update frequency)."));
        m_ToolTip.AddTool(&m_SerialDropDown, _T("Remote GUI refresh rate will be severely limited when using Serial/GPIB connection (10s update frequency)."));
        m_ToolTip.Activate(TRUE);
    }

    //Assume Remote & Local commands are enabled.
    m_RemoteLocalCheck.SetCheck(BST_CHECKED);
    Sentinel3.isRemoteAndManualAccess = true;
    LogCopyInProgress = false;


    //If this program hasn't run on this PC/profile before, set up the config location in APPDATA
    //Default to not showing the manual command entry window
	char Filename[MAX_PATH];

	FILE *fid;
	int err = 0;
    
    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, DataLocStr)))
    {
        PathAppend(DataLocStr, TEXT("\\Pulse Power & Measurement Ltd"));
        if(GetFileAttributes(DataLocStr) == INVALID_FILE_ATTRIBUTES)
        {
            CreateDirectory(DataLocStr, NULL);
        }
        PathAppend(DataLocStr, TEXT("\\Sentinel3Remote"));
        if(GetFileAttributes(DataLocStr) == INVALID_FILE_ATTRIBUTES)
        {
            CreateDirectory(DataLocStr, NULL);

            sprintf_s(Filename, MAX_PATH, "%ls\\S3VM", DataLocStr);
            err = fopen_s(&fid, Filename, "w");
            if (!err)
            {
                fputc('0',fid);
                fclose(fid);
            }
        }
    }
    else
    {
        _tcscpy_s(DataLocStr, MAX_PATH, _T("C:\\"));
        PathAppend(DataLocStr, TEXT("\\Pulse Power & Measurement Ltd"));
        if(GetFileAttributes(DataLocStr) == INVALID_FILE_ATTRIBUTES)
        {
            CreateDirectory(DataLocStr, NULL);
        }
        PathAppend(DataLocStr, TEXT("\\Sentinel3Remote"));
        if(GetFileAttributes(DataLocStr) == INVALID_FILE_ATTRIBUTES)
        {
            CreateDirectory(DataLocStr, NULL);

            sprintf_s(Filename, MAX_PATH, "%ls\\S3VM", DataLocStr);
            err = fopen_s(&fid, Filename, "w");
            if (!err)
            {
                fputc('0',fid);
                fclose(fid);
            }
        }
    }
    ManualCommandEntryVisible = false;
    sprintf_s(Filename, MAX_PATH, "%ls\\S3VM", DataLocStr);
    err = fopen_s(&fid, Filename, "r");
    if(!err)
    {
        char mode = fgetc(fid);
        if(mode == '0')
        {
            ManualCommandEntryVisible = false;
        }
        else
        {
            ManualCommandEntryVisible = true;
        }
        fclose(fid);
    }

	// ------------------------------------------------------------------------
	// Read IP address file

	char	IPAddr[MAX_IP_ADDR_LEN];

	sprintf_s(Filename, MAX_PATH, "%ls\\S3IP.txt", DataLocStr);

	CString	tmp;
	err = fopen_s(&fid, Filename, "r");
	if (err)
	{
		m_IPv4AddrEdit.SetWindowText(_T(DEFAULT_IP_ADDR));
		m_IPv4AddrEdit.GetWindowTextW(tmp);
		CStringA tmpA(tmp);
		strcpy_s(IPAddr, MAX_IP_ADDR_LEN, (LPCSTR)tmpA);
	}
	else
	{
		fgets(IPAddr, sizeof(IPAddr), fid);
		fclose(fid);
		CString tmp1(IPAddr);
		m_IPv4AddrEdit.SetWindowText(tmp1);
	}
	m_PortEdit.SetWindowText(_T(DEFAULT_PORT));
	if (IPv4ToolsRemLeadZ(IPAddr))
	{
		AfxMessageBox(_T("Invalid IPv4 address"));
	}

	// ------------------------------------------------------------------------

    EnumCOMPorts();
    m_SerialDropDown.SetCurSel(0);

	m_COMPort = new CS3USBVCP(this);


    sprintf_s(m_TestScriptName, MAX_SCRIPT_LEN, "C:\\S3Script.s3s");
	tmp = m_TestScriptName;
	m_ScriptNameStatic.SetWindowTextW(tmp);


	LOGFONT lf; 

	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = 14;                  // pixels
	wcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("Courier New"));
	m_font.CreateFontIndirect(&lf);
	m_CommandEdit.SetFont(&m_font);

	int line_no = 0;
	m_ScriptThreadRun = false;

    
    CMenu *pMenu = GetMenu();
    if(ManualCommandEntryVisible)
    {
        pMenu->CheckMenuItem(ID_MANUAL_ENTRY, MF_CHECKED|MF_BYCOMMAND);
        ResizeWindow(S3SCREENLARGE);
    }
    else
    {
        pMenu->CheckMenuItem(ID_MANUAL_ENTRY, MF_UNCHECKED|MF_BYCOMMAND);
        ResizeWindow(S3SCREENSMALL);
    }
    AutoSyncTimesOnConnect = true;
    if(AutoSyncTimesOnConnect)
    {
        pMenu->CheckMenuItem(ID_AUTOSYNC_TIME, MF_CHECKED|MF_BYCOMMAND);
    }
    else
    {
        pMenu->CheckMenuItem(ID_AUTOSYNC_TIME, MF_UNCHECKED|MF_BYCOMMAND);
    }
    // ------------------------------------------------------------------------
	// Start setting up the GDI region for drawing the Sentinel 3 UI

	S3Init(false);

    m_AutoUpdateGUITick.SetCheck(true);
    m_GDIStatic.ShowWindow(SW_HIDE);
	m_IMGStatic.ShowWindow(SW_SHOW);
    
    m_GDIStatic.SetWindowPos(&wndTop, 0, 0, M_SCREEN_WIDTH, M_SCREEN_HEIGHT, SWP_NOMOVE); // | SWP_NOSIZE);
    
	m_GDIStatic.m_Parent = this;
    m_GDIStatic.S3GDIInit();

    m_AnimateState = 0;
    
    SetTimer(IDT_S3_GUI_UPDATE_TIMER,	500,					NULL);

    m_LogCopyDlg = NULL;

	if (connectionmethod == ETHERNET)
		m_EthSelRdoButton.SetCheck(TRUE);
	else if (connectionmethod = USB)
		m_USBSelRdoButton.SetCheck(TRUE);
	else if (connectionmethod = GPIB)
		m_GPIBSelRdoButton.SetCheck(TRUE);

	S3TimerInit();

	return TRUE;  // return TRUE  unless you set the focus to a control
}
// ----------------------------------------------------------------------------
BOOL CS3AgentDlg::PreTranslateMessage(MSG* pMsg)
{
    m_ToolTip.RelayEvent(pMsg);

    return CDialog::PreTranslateMessage(pMsg);
}


// ----------------------------------------------------------------------------
void CS3AgentDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		//CAboutDlg dlgAbout;
		//dlgAbout.DoModal();
        AboutDlg();
	}
    else if ((nID & 0xFFF0) == SC_CLOSE)
    {
        ExitS3RemController();
    }
    else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}
// ----------------------------------------------------------------------------
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CS3AgentDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}
// ----------------------------------------------------------------------------
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CS3AgentDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
// ----------------------------------------------------------------------------
// Exit Function
// Closes any open connections & shuts the application
void CS3AgentDlg::ExitS3RemController()
{
    if(m_LogCopyDlg != NULL)
    {
        AfxMessageBox(_T("Log File transfer in progress."));
    }
    else
    {
        //If there is a USB connection open, close it.
        if (m_COMPort)
            m_COMPort->Close();
        
        delete m_COMPort;
        
        if(connectionmethod == ETHERNET && Sentinel3.isConnected)
        {
            CloseSocketSC3();
        }

        KillTimer(IDT_S3_GUI_UPDATE_TIMER);

        CDialog::OnOK();
    }
}
// ----------------------------------------------------------------------------
// Shows the About Dialog
void CS3AgentDlg::AboutDlg()
{
    CAboutDlg dlgAbout;
    dlgAbout.DoModal();
}

int cnt = 0;


// ----------------------------------------------------------------------------
// Connect to the Sentinel 3 chassis over the defined connection method & 
// confirm it is a Sentinel 3 
void CS3AgentDlg::OnBnClickedS3ConnectButton()
{
    if (Sentinel3.isConnected && m_LogCopyDlg == NULL)
    {
        OnDisconnect(0,0);
    }
    else if(!Sentinel3.isConnected)
    {
        bool serialCanConnect = true;
        //Save the current connection details
        m_SerialDropDown.GetWindowText(COM_name);
        m_IPv4AddrEdit.GetWindowTextW(IPv4Addr);
        m_PortEdit.GetWindowTextW(IPV4Port);

        connectionmethod = 0;
        if(m_EthSelRdoButton.GetCheck())
        {
            connectionmethod = ETHERNET;
        }
        else
        {
            if(m_SerialSelRdoButton.GetCheck())
            {
                connectionmethod = USB;
                if(COM_name.GetLength() == 0)
                {
                    AfxMessageBox(_T("No Serial port specified!."));
                    serialCanConnect = false;
                }
            }
            else if(m_GPIBSelRdoButton.GetCheck())
            {
                connectionmethod = GPIB;
            }
        }

        if(serialCanConnect)
        {
            m_ConnectToSentinel3Thread = AfxBeginThread(AttemptConnectToSentinel3, this);
        }
    }
}

LRESULT CS3AgentDlg::OnDisconnect(WPARAM wParam, LPARAM lParam)
{
    //disconnect from the currently connected Sentinel 3
    switch (connectionmethod)
    {
        case ETHERNET:
            CloseSocketSC3();
            //Reenable the Ethernet Controls
            m_IPv4AddrEdit.EnableWindow(true);
            m_PortEdit.EnableWindow(true);

            break;
        case USB:
            //Close the USB connection
            CloseConnectUSB();

            //Reenable the USB controls
            m_SerialDropDown.EnableWindow(true);
            m_GPIBSelRdoButton.EnableWindow(true);
            m_SerialSelRdoButton.EnableWindow(true);
            break;
        case GPIB:
            CloseGPIBConnection();
            m_GPIBSelRdoButton.EnableWindow(true);
            m_SerialSelRdoButton.EnableWindow(true);
            break;
    }
   
    m_EthSelRdoButton.EnableWindow(true);
    m_USBSelRdoButton.EnableWindow(true);
    m_RemoteLocalCheck.EnableWindow(false);
    m_GDIStatic.ShowWindow(SW_HIDE);
    m_IMGStatic.ShowWindow(SW_SHOW);
    if(ManualCommandEntryVisible)
    {
        m_ConnectS3Button.SetWindowTextW(L"Connect to Sentinel 3 Device");
    }
    else
    {
        m_ConnectS3Button.SetWindowTextW(L"Connect to\r\nSentinel 3\r\nDevice");
    }
    Sentinel3.isConnected = false;
    return 0;
}
// ----------------------------------------------------------------------------
// Handle UI controls for the selection of the connection method 
// - Grey out unused controls
// - Control connectionmethod variable 
void CS3AgentDlg::OnBnClickedS3Eth()
{
    m_IPv4AddrEdit.EnableWindow(true);
    m_PortEdit.EnableWindow(true);
    m_SerialDropDown.EnableWindow(false);
    m_GPIBSelRdoButton.EnableWindow(false);
    m_SerialSelRdoButton.EnableWindow(false);
}
void CS3AgentDlg::OnBnClickedS3USB()
{
    m_IPv4AddrEdit.EnableWindow(false);
    m_PortEdit.EnableWindow(false);
    m_SerialDropDown.EnableWindow(true);
    m_GPIBSelRdoButton.EnableWindow(true);
    m_SerialSelRdoButton.EnableWindow(true);

}
void CS3AgentDlg::OnBnClickedS3USBSerial()
{
    m_SerialDropDown.EnableWindow(true);
}
void CS3AgentDlg::OnBnClickedS3USBGPIB()
{
    m_SerialDropDown.EnableWindow(false);
}
// ----------------------------------------------------------------------------
// Send a manual command to the Sentinel 3 chassis over the predefined interface
// Update the response window as well
void CS3AgentDlg::OnBnClickedSendS3MsgButton()
{
    if(Sentinel3.isConnected)
    {
        CString RespStr, CMDMsg, MsgResponse;
        m_SendManCMDButton.EnableWindow(false);
        m_MessageEdit.GetWindowTextW(CMDMsg);
        CMDMsg.Trim();
        if (CMDMsg.IsEmpty())
        {
            m_MessageEdit.SetWindowTextW(L"");
        }
        else
        {
			AfxBeginThread(SendMessageThread, this, THREAD_PRIORITY_NORMAL);
        }
        m_SendManCMDButton.EnableWindow(true);
    }
}

UINT SendMessageThread(LPVOID pParam)
{
    HWND *phObjectHandle = static_cast<HWND *>(pParam);
    CS3AgentDlg *pObject = (CS3AgentDlg *)pParam;

    CString RespStr, CMDMsg, MsgResponse;

    pObject->m_MessageEdit.GetWindowTextW(CMDMsg);
    CMDMsg.Trim();
    if(!CMDMsg.IsEmpty())
    {
		// pObject->
		RxBuf[0] = '\0';
        pObject->m_MessageEdit.EnableWindow(false);
        MsgResponse = SendSentinel3Message(CMDMsg);

		if (MsgResponse.Left(2) != _T("E:") &&
			MsgResponse.Left(2) != _T("I:") &&
			MsgResponse.Left(3) != _T("OK:"))
		{
			MsgResponse = _T("Invalid Response");
		}

        MsgResponse.Replace(_T("\n"), _T("\r\n"));

        RespStr.Format(_T("%s\t%s\r\n"),CMDMsg,MsgResponse);

        int nLength = (int)pObject->m_CommandEdit.GetWindowTextLength();

		if (nLength < (int)pObject->m_CommandEdit.GetLimitText())
		{
			pObject->m_CommandEdit.SetSel(nLength, nLength);
			pObject->m_CommandEdit.ReplaceSel(RespStr);
		}
		else
		{
			pObject->m_CommandEdit.SetSel(2000, nLength);
			pObject->m_CommandEdit.ReplaceSel(RespStr);
		}

		pObject->m_MessageEdit.SetWindowTextW(L"");
		pObject->m_MessageEdit.EnableWindow(true);

    }
    return 0;
}

// ----------------------------------------------------------------------------
// Allow the user to visually select a Sentinel 3 script
void CS3AgentDlg::OnBnClickedChooseScriptButton()
{
    // szFilters is a text string that includes two file name filters: 
    // "*.my" for "MyType Files" and "*.*' for "All Files."
    TCHAR szFilters[] = _T("S3 remote test files (*.s3s)|*.s3s|All Files (*.*)|*.*||");

    // Create an Open dialog;
    CFileDialog fileDlg(TRUE, _T("s3s"), _T("*.s3s"),
        OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);

    // Display the file dialog. When user clicks OK, fileDlg.DoModal()  
    // returns IDOK. 

    int res = (int)fileDlg.DoModal();

    if (res == IDOK)
    {
        CString pathName = fileDlg.GetPathName();
        CString fileName = fileDlg.GetFileName();

        // Implement opening and reading file in here

        CStringA tmpA, tmpB;
        tmpA = pathName;
        strcpy_s(m_TestScriptName, MAX_SCRIPT_LEN, (LPCSTR)tmpA);

        m_ScriptNameStatic.SetWindowTextW(pathName);
    }
}
// ----------------------------------------------------------------------------
// Launch/Stop the script running thread
void CS3AgentDlg::OnBnClickedRunScriptButton()
{
    if (m_ScriptThreadRun)
    {
        m_ScriptStop = true;
        m_StartStopScriptButton.SetWindowText(_T("Run script"));
    }
    else
    {
        m_ScriptStop = false;
        m_ScriptThread = AfxBeginThread(ScriptProcessThread, this);
    }
}
// ----------------------------------------------------------------------------
// Manage whether the Sentinel 3 will accept local commands, or just remote commands.
void CS3AgentDlg::OnRemoteLocalPermissionsBtnClick(void)
{
    CString Command, Response;
    switch (m_RemoteLocalCheck.GetCheck())
    {
        case BST_UNCHECKED:
            Sentinel3.isRemoteAndManualAccess = false;
            Command = L"REMOTE";
            break;
        case BST_CHECKED:
            Sentinel3.isRemoteAndManualAccess = true;
            Command = L"LOCAL";
            break;
    }

    Response = SendSentinel3Message(Command);
    // If there was a comms error, put the UI back to how it was before
    if ((Response.Mid(0, 6).CompareNoCase(L"ERROR:") == 0) || (Response.Mid(0, 2).CompareNoCase(L"E:") == 0))
    {
        switch (m_RemoteLocalCheck.GetCheck())
        {
            case BST_UNCHECKED:
                m_RemoteLocalCheck.SetCheck(BST_CHECKED);
                Sentinel3.isRemoteAndManualAccess = true;
                break;
            case BST_CHECKED:
                m_RemoteLocalCheck.SetCheck(BST_UNCHECKED);
                Sentinel3.isRemoteAndManualAccess = false;
                break;
        }
    }
}


// ----------------------------------------------------------------------------
// Enable the Emulated UI section, by starting a thread which updates the GUI data from the Sentinel 3 device
void CS3AgentDlg::OnAutoUpdateGUIBtnClick(void)
{
    switch (m_AutoUpdateGUITick.GetCheck())
    {
        case BST_UNCHECKED:
            AutoCheckS3Data = false;
            break;
        case BST_CHECKED:
            AutoCheckS3Data = true;
            m_DataUpdateThread = AfxBeginThread(AutoUpdateSentinelDataBundleThread, this);
            break;
    }
}

// ----------------------------------------------------------------------------
// Handle the user selecting to syncronise the system times on connect to the Sentinel 3 (or not)
void CS3AgentDlg::OnBnClickedAutoSyncTime(void)
{
    CMenu *pMenu = GetMenu();
    if(pMenu != NULL)
    {
        switch(AutoSyncTimesOnConnect)
        {
        case true:
                pMenu->CheckMenuItem(ID_AUTOSYNC_TIME, MF_UNCHECKED|MF_BYCOMMAND);
                AutoSyncTimesOnConnect = false;
            break;
        case false:
                pMenu->CheckMenuItem(ID_AUTOSYNC_TIME, MF_CHECKED|MF_BYCOMMAND);
                AutoSyncTimesOnConnect = true;
            break;
        }
        
    }
}
// ----------------------------------------------------------------------------
// Handle resizing the UI if the user toggles the manual command entry setting
void CS3AgentDlg::OnBnClickedManualEntry(void)
{
	char Filename[MAX_PATH];
	FILE *fid;
	int err = 0;
    CMenu *pMenu = GetMenu();
    if(pMenu != NULL)
    {
        if(ManualCommandEntryVisible)
        {
            pMenu->CheckMenuItem(ID_MANUAL_ENTRY, MF_UNCHECKED|MF_BYCOMMAND);
            ManualCommandEntryVisible = false;
            ResizeWindow(S3SCREENSMALL);

            sprintf_s(Filename, MAX_PATH, "%ls\\S3VM", DataLocStr);
            err = fopen_s(&fid, Filename, "w");
            if (!err)
            {
                fputc('0',fid);
                fclose(fid);
            }
        }
        else
        {
            pMenu->CheckMenuItem(ID_MANUAL_ENTRY, MF_CHECKED|MF_BYCOMMAND);
            ManualCommandEntryVisible = true;
            ResizeWindow(S3SCREENLARGE);

            sprintf_s(Filename, MAX_PATH, "%ls\\S3VM", DataLocStr);
            err = fopen_s(&fid, Filename, "w");
            if (!err)
            {
                fputc('1',fid);
                fclose(fid);
            }
        }
    } 
}



void CS3AgentDlg::ResizeWindow(char size)
{
    switch(size)
    {
    case S3SCREENSMALL:
        {
            //Disable & Hide the manual command viewboxes
            m_SendManCMDButton.EnableWindow(false);
            m_CommandEdit.EnableWindow(false);
            m_MessageEdit.EnableWindow(false);
            m_SendManCMDButton.ShowWindow(SW_HIDE);
            m_CommandEdit.ShowWindow(SW_HIDE);
            m_MessageEdit.ShowWindow(SW_HIDE);
            m_ManualGroupBox.ShowWindow(SW_HIDE);
       
            //Resize/Move the Connection settings, button & script areas
            CWnd *pDlgItem;

            //Connection Settings Control Group
            pDlgItem = GetDlgItem(IDC_CONNECTION_CONTROL_GROUP);
            pDlgItem->MoveWindow(11,10,357,83);
            //Connection button
            pDlgItem = GetDlgItem(IDC_CONNECT_BUTTON);
            pDlgItem->MoveWindow(372,14,100,80);
            if(Sentinel3.isConnected)
            {
                m_ConnectS3Button.SetWindowTextW(L"Disconnect from Sentinel 3 Device");
            }
            else
            {
                m_ConnectS3Button.SetWindowTextW(L"Connect to\r\nSentinel 3\r\nDevice");
            }
            //Ethernet Radio button
            pDlgItem = GetDlgItem(IDC_ETH_RDO);
            pDlgItem->MoveWindow(16,27,39,23);
            //IPV4 Label
            pDlgItem = GetDlgItem(IDC_IPV4_ADDR_STATIC);
            pDlgItem->MoveWindow(64,31,70,18);
            //IPV4 Editbox
            pDlgItem = GetDlgItem(IDC_IPV4_ADDR_EDIT);
            pDlgItem->MoveWindow(135,27,150,23);
            //Port label
            pDlgItem = GetDlgItem(IDC_IPV4_PORT_STATIC);
            pDlgItem->MoveWindow(292,31,25,18);
            //Port Editbox
            pDlgItem = GetDlgItem(IDC_IPV4_PORT_EDIT);
            pDlgItem->MoveWindow(317,27,45,23);

            //USB Radio button
            pDlgItem = GetDlgItem(IDC_USB_RDO);
            pDlgItem->MoveWindow(16,57,39,23);
            //USB Serial Radio Button
            pDlgItem = GetDlgItem(IDC_USBSERIAL);
            pDlgItem->MoveWindow(64,60,92,18);
            //USB Editbox
            pDlgItem = GetDlgItem(IDC_SERIAL_DROP);
            pDlgItem->MoveWindow(160,57,65,23);
            //USB GPIB Radio Button
            pDlgItem = GetDlgItem(IDC_USBGPIB);
            pDlgItem->MoveWindow(270,60,89,18);

            //Script Controlgroup
            pDlgItem = GetDlgItem(IDC_SCRIPT_CONTROL_GROUP);
            pDlgItem->MoveWindow(476,10,357,83);
            //ScriptName
            pDlgItem = GetDlgItem(IDC_SCRIPT_NAME_STATIC);
            pDlgItem->MoveWindow(485,32,340,26);
            //Script Select Button
            pDlgItem = GetDlgItem(IDC_CHOOSE_SCRIPT_BUTTON);
            pDlgItem->MoveWindow(484,62,83,28);
            //Script Run Button
            pDlgItem = GetDlgItem(IDC_RUN_SCRIPT_BUTTON);
            pDlgItem->MoveWindow(575,62,83,28);


            //Resize the whole window
            CRect rect;
            GetWindowRect(&rect);

            MoveWindow(rect.left,rect.top, 851 ,rect.Height());
        }
        break;
    case S3SCREENLARGE:
        {
            //Resize the whole window
            CRect rect;
            GetWindowRect(&rect);
            
            MoveWindow(rect.left,rect.top, 1250 ,rect.Height());

            //Resize/Move the Connection settings, button & script areas
            CWnd *pDlgItem;

            //Connection Settings Control Group
            pDlgItem = GetDlgItem(IDC_CONNECTION_CONTROL_GROUP);
            pDlgItem->MoveWindow(11,10,464,83);
            //Connection button
            pDlgItem = GetDlgItem(IDC_CONNECT_BUTTON);
            pDlgItem->MoveWindow(489,14,272,80);
            if(Sentinel3.isConnected)
            {
                m_ConnectS3Button.SetWindowTextW(L"Disconnect from Sentinel 3 Device");
            }
            else
            {
                m_ConnectS3Button.SetWindowTextW(L"Connect to Sentinel 3 Device");
            }
            //Ethernet Radio button
            pDlgItem = GetDlgItem(IDC_ETH_RDO);
            pDlgItem->MoveWindow(26,27,39,23);
            //IPV4 Label
            pDlgItem = GetDlgItem(IDC_IPV4_ADDR_STATIC);
            pDlgItem->MoveWindow(84,31,70,18);
            //IPV4 Editbox
            pDlgItem = GetDlgItem(IDC_IPV4_ADDR_EDIT);
            pDlgItem->MoveWindow(165,27,150,23);
            //Port label
            pDlgItem = GetDlgItem(IDC_IPV4_PORT_STATIC);
            pDlgItem->MoveWindow(332,31,25,18);
            //Port Editbox
            pDlgItem = GetDlgItem(IDC_IPV4_PORT_EDIT);
            pDlgItem->MoveWindow(367,27,45,23);

            //USB Radio button
            pDlgItem = GetDlgItem(IDC_USB_RDO);
            pDlgItem->MoveWindow(26,57,39,23);
            //USB Serial Radio Button
            pDlgItem = GetDlgItem(IDC_USBSERIAL);
            pDlgItem->MoveWindow(84,60,92,18);
            //USB Editbox
            pDlgItem = GetDlgItem(IDC_SERIAL_DROP);
            pDlgItem->MoveWindow(184,57,65,23);
            //USB GPIB Radio Button
            pDlgItem = GetDlgItem(IDC_USBGPIB);
            pDlgItem->MoveWindow(332,60,89,18);

            //Script Controlgroup
            pDlgItem = GetDlgItem(IDC_SCRIPT_CONTROL_GROUP);
            pDlgItem->MoveWindow(775,10,460,83);
            //ScriptName
            pDlgItem = GetDlgItem(IDC_SCRIPT_NAME_STATIC);
            pDlgItem->MoveWindow(784,32,440,26);
            //Script Select Button
            pDlgItem = GetDlgItem(IDC_CHOOSE_SCRIPT_BUTTON);
            pDlgItem->MoveWindow(784,62,83,28);
            //Script Run Button
            pDlgItem = GetDlgItem(IDC_RUN_SCRIPT_BUTTON);
            pDlgItem->MoveWindow(874,62,83,28);


            //Enable & Reveal the manual command viewboxes
            m_SendManCMDButton.EnableWindow(true);
            m_CommandEdit.EnableWindow(true);
            m_MessageEdit.EnableWindow(true);
            m_SendManCMDButton.ShowWindow(SW_SHOW);
            m_CommandEdit.ShowWindow(SW_SHOW);
            m_MessageEdit.ShowWindow(SW_SHOW);
            m_ManualGroupBox.ShowWindow(SW_SHOW);
        }
        break;
    }
}

// ----------------------------------------------------------------------------
// 
void CS3AgentDlg::OnBnClickedLogFileCopy(void)
{
    if(Sentinel3.isConnected)
    {
        if(m_LogCopyDlg == NULL) //Ensure only one is shown at a time
        {
            m_LogCopyDlg = new CS3LogCopyDlg(this);
            m_LogCopyDlg->Create(CS3LogCopyDlg::IDD);
            m_LogCopyDlg->ShowWindow(SW_SHOW);
            //Launch the dialog in a non-modal manner, so the user can still interact with the rest of the UI.
        }
    }
    else
    {
        AfxMessageBox(_T("No Sentinel 3 Connected.\r\nPlease connect to a Sentinel 3 Device and try again."));
    }
}
// Recieve the message from the log file copy dialog on close
// this sets the pointer to null, to allow us to open another one (therefore only allowing up to one log file copy dialog)
LRESULT CS3AgentDlg::OnLogCopyFinished(WPARAM wParam, LPARAM lParam)
{
    m_LogCopyDlg = NULL;
    return 0;
}

int CS3AgentDlg::EnumCOMPorts()
{
	INT		i = 0, iRet;
	HKEY	hKey;
	WCHAR	szName[128];
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
            // Get the "COMX:" name
		    iRet = RegQueryValueEx(hKey, szName, 0, &dwType, (PBYTE)szName, &dwSize);
            NewDeviceList[NewDevCnt++] = szName;
		}
		else
			break; // No more enumerated keys
	}

	RegCloseKey(hKey);


	for(char i = 0; i < NewDevCnt; i++)
    {
        m_SerialDropDown.AddString(NewDeviceList[i]);
    }
	return NewDevCnt;
}


