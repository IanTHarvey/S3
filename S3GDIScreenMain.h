#pragma once

#include "S3GDINameValue.h"
#include "S3GDIEdit.h"
#include "S3GDINumEdit.h"
#include "S3GDINumberPad.h"
#include "S3GDIParameterMenu.h"

// m_Screen values
#define S3_OVERVIEW_SCREEN		0
#define S3_RX_SCREEN			1
#define S3_TX_SCREEN			2
#define S3_CH_SCREEN			3
#define S3_SETTINGS_SCREEN		4
#define S3_SHUTDOWN_SCREEN		5 // Obs: not a screen
#define S3_SLEEP_SCREEN			6
#define S3_OS_UPDATE_SCREEN		7
#define S3_LOG_COPY_SCREEN		8
#define S3_CALIBRATE_SCREEN		9
#define S3_FACTORY_SCREEN		10
#define S3_CLOSED_SCREEN		11
#define S3_APP_UPDATE_SCREEN	12
#define S3_FACTORY_SYS_SCREEN	13
#define S3_PREVIOUS_SCREEN		0x7f

#define S3_MAIN_RX_RLL_BARS		5
// #define S3_CHARGER_BATT_SEGS	5
#define S3_CHARGER_SEG_GAP		3	// Pixels
#define S3_N_BATT_SEGS			5

#define S3GDI_SCREEN_WIDTH		800
#define S3GDI_SCREEN_HEIGHT		480

#define S3GDI_MAX_SCREEN_MSG	256

// Text box heights
#define HEAD_ROW			45
#define SUBHEAD_ROW			35
#define PARA_ROW			25

#define S3_HEIGHT_MSG_BAR	50

// For DrawText()
#define S3_BTN_CENTRE			(DT_SINGLELINE | DT_VCENTER | DT_CENTER)

// CS3GDIScreenMain
#ifdef S3_AGENT
class CS3AgentDlg;
#else
class CS3ControllerDlg;
#endif
class CTextInputPopup;
class CS3GDIInfoPopup;
class CS3NameValue;
class CS3Button;

// Convenience macros
#define S3BLT_BLK(BMP, X, Y, W, H) 	\
	TransparentBlt(m_HDC, X, Y, W, H, BMP, 0, 0, W, H, RGB(0, 0, 0))
#define S3BLT(BMP, X, Y, W, H) 	\
	TransparentBlt(m_HDC, X, Y, W, H, BMP, 0, 0, W, H, m_crWhite)
#define S3BLTR(BMP, R) 			\
	TransparentBlt(m_HDC, R.left, R.top, R.Width(), R.Height(), \
		BMP, 0, 0, R.Width(), R.Height(), m_crWhite)
#define S3OPBLTR(BMP, R) 		\
	BitBlt(m_HDC, R.left, R.top, R.Width(), R.Height(), BMP, 0, 0, SRCCOPY)

#define S3_RECT(A, B)			\
	Rectangle(A, B.left, B.top, B.right, B.bottom)
// For NULL pen annoyance...
#define S3_RECT_N(A, B)			\
	Rectangle(A, B.left, B.top, B.right + 1, B.bottom + 1)
#define S3_RECTR(A, B, C)		\
	RoundRect(A, B.left, B.top, B.right, B.bottom, C, C)

// ----------------------------------------------------------------------------

class CS3GDIScreenMain : public CStatic
{
	DECLARE_DYNAMIC(CS3GDIScreenMain)

private:
	CParameterMenu	*m_ParaMenu;
	CS3NumberPad	*m_NumericPad;

	// ITH: Ever want to revisit this?
	// CTextInputPopup	*m_TextInput;

// ----------------------------------------------------------------------------
// GDI objects

	HBRUSH		m_hBrushBG1, m_hBrushBG2, m_hBrushBG3, m_hBrushBG4,
				m_hBrushSleep,
				m_hLiveIPBrush, m_hBrushIP, m_hBrushAlarm,
				m_hBrushWhite,
				m_hBrushLightGrey, m_hBrushMediumGrey, m_hBrushDarkGrey;

	HBRUSH		m_hBrushTx, m_hBrushTxSleep, m_hBrushTxInactive,
				m_hBrushTxTest;

	HPEN		m_hPenBG, m_hPenIPLive, m_hPenIPOff, m_hPenIPSelected,
				m_hPenNone, m_hPenAlarm;

	HPEN		m_hPenDetected, m_hPenSleep;
	
	HPEN		m_hPenFOL1,		m_hPenFOL2,		m_hPenFOL6,
				m_hPenFOL1Dark,	m_hPenFOL2Dark,	m_hPenFOL6Dark;

	// Required for geometric line types, but not supported by KuK BSP
	// so moot anyway. 
	CPen		m_PenFOL1, m_PenFOL2, m_PenFOL6;
	CPen		m_PenFOL1Dark, m_PenFOL2Dark, m_PenFOL6Dark;

	// Demarcate screen regions
	COLORREF	m_crBG1, m_crBG2, m_crBG3, m_crBG4;
	COLORREF	m_crSleep;
	COLORREF	m_crTx, m_crTxSleep, m_crTxInactive, m_crTxTest;

	COLORREF	m_crIP;
	COLORREF	m_crLiveIPFill;
	COLORREF	m_crAlarmFill;

	// Generic colours
	COLORREF	m_crRed, m_crAmber, m_crGreen;
	HBRUSH		m_hBrushRed, m_hBrushAmber, m_hBrushGreen;

	COLORREF	m_crFOL;

	// General selection indicator
	COLORREF	m_crSel;
	HBRUSH		m_hBrushSel;
	HPEN		m_hPenSel;

	// Receiver
	COLORREF	m_crRx;
	HBRUSH		m_hBrushRx;

	COLORREF	m_crRLLBg, m_crRLLOn, m_crRLLOnUnsel, m_crRLLOff;

	HBRUSH		m_hBrushRxRLLBg, m_hBrushRxRLLOn, m_hBrushRxRLLOnUnsel,
				m_hBrushRxRLLOff, m_hBrushRxNum, m_hBrushFOL;

	COLORREF	m_crGainBg, m_crGainOn, m_crGainOnUnsel, m_crGainOff;
	HBRUSH		m_hBrushGainBg, m_hBrushGainOn, m_hBrushGainOnUnsel,
				m_hBrushGainOff;

// ----------------------------------------------------------------------------

	// Main screen regions
	CRect	m_RectPhysicalScreen,
			m_RectScreen, m_RectHeader,
			m_RectRx,
			m_RectFOL,
			m_RectTx,
			m_RectCharger,
			m_RectInfo; //, m_RectTxTable;

	// Individual characters
	CRect	m_RectCharL, m_RectCharS;

	// Buttons
	CRect	m_RectButtons;
	CRect	m_RectSettingsButton, m_RectShutdownButton, m_RectBackButton;

	// Receiver screen regions
	CRect	m_RectRxRx, m_RectRxAGC, m_RectRxParaList, m_RectRxTable, m_RectRxTx;
	CRect	m_RectRxMsg;
	CRect	m_RectRxNodeName;

	// Transmitter screen
	CRect	m_RectTxNodeName, m_RectTxType, m_RectTxPowerMode, m_RectTxDoComp;

	// Shutdown screen
	CRect	m_RectShutdownScreen;
				/*m_RectTxSleepAll,
				m_RectTxWakeAll,
				m_RectSysShutdown,
				m_RectSysRestart;*/

	CS3Button	*m_ButtonTxSleepAll,
				*m_ButtonTxWakeAll,
				*m_ButtonSysShutdown,
				*m_ButtonSysRestart;

	// Factory screen
	// CRect	m_RectFactoryScreen; // , m_RectTxSleepAll, m_RectSysShutdown,
		// m_RectSysRestart, m_RectAppClose;

	// OS update screen
	CRect	m_RectSWUpdateScreen, m_RectSWUpdateInstr, m_RectYes, m_RectNo;

	// App update screen
	CRect	m_RectAppUpdateScreen, m_RectAppUpdateInstr;

	// Log copy screen
	CRect	m_RectLogCopyScreen, m_RectLogCopyInstr,
			m_RectLogCopyYes, m_RectLogCopyNo;

	// Factory set-up screen
	CRect	m_RectFactoryScreen,
				m_RectFactoryLock,
				m_RectFactorySleepAll,
				m_RectFactoryShutdown,
				m_RectFactoryRestart,
				m_RectFactoryClose;
	
	CRect	m_RectCalibrate, m_RectSystem, m_RectTest, m_RectDemo;
	CRect	m_RectFactoryMsg;

	CString	m_StrFactoryMsg;

// ----------------------------------------------------------------------------

	CString		m_InfoStr;

	char		*m_NodeName;

	// Sizing
	int			m_ndh, m_ndv; // Screen horizontal and vertical

	double		m_th_l[S3_MAX_IPS], m_th_r[S3_MAX_IPS];	// Angle of 8 Tx inputs (l & r)

	int			m_HeadH;
	int			m_RxH;
	int			m_FOLH;
	int			m_TxH;

	int			m_RxSep;

	// Rx main
	int		m_wRx;
	int		m_hRx;

	int		m_RxYref;
	int		m_RxBottom;

	int		m_RxNumXOffset;
	int		m_RxNumYOffset;
	int		m_RxNumRad;

	// Rx system gain - right & bottom
	int		m_radGainVal;
	int		m_xosGainVal;
	int		m_yosGainVal;

	int		m_radGainUnits;
	int		m_xosGainUnits;
	int		m_yosGainUnits;

	// RxRLL
	static const int		m_RxRLLBar_h[S3_MAIN_RX_RLL_BARS];
	static const int		m_RxRLLBar_w[S3_MAIN_RX_RLL_BARS];

	int		m_RxRLLBar_ys;	// Vertical inter-bar spacing
	int		m_RxRLLBar_xs;	// Horizontal distance to centre-line
	int		m_RxRLLBar_yos;	// Vertical distance from bottom bar to centre-line

	int		m_TxTop;

	// Header region
	static const int		m_settings_xref = 650;
	static const int		m_settings_yref = 0;

	static const int		m_shutdown_xref = m_settings_xref + 70;
	static const int		m_shutdown_yref = 0;

	// Charger batteries
	static const int		m_lChBatt = 60;
	static const int		m_wChBatt = 25;
	static const int		m_lChBattBtn = 5;

	// Tx selected
	static const int		m_radTxSel = 30;
	static const int		m_radTxSelIP = 5;
	static const int		m_radTxSelIPAct = 7;

	// Tx unselected
	static const int		m_radTxUnsel = 18;
	static const int		m_radTxUnselIP = 3;
	static const int		m_radTxUnselIPAct = 4;

	double					m_radRatioPosIP;
	double					m_radRatioPosActIP;

	double					m_posIPUnsel;
	double					m_posActiveIPUnsel;

	static const int		m_xUnselOffset = 35;
	static const int		m_yUnselSep = 48;

	// Input layout start angle (degrees) from due south
	static const int		m_ThStart = -40;
	
	// TODO: Should reference settings button
	int						RxBackBtn_xref;
	int						RxBackBtn_yref;

	char					m_Screen;		// Current display
	char					m_PrevScreen;	// Previous display

	bool					m_DoubleClickRequired;

	// Receiver screen
	int						m_hTx;
	int						m_wRxRx;
	int						m_wRLL, m_hRLL;
	int						m_wRxParaList;

	// Transmitter screen
	int						m_hIPGain, m_wGainBar;
	int						m_wTxTx;
	int						m_wTxIPPara;

	double					m_dBperPix;

	// Gain widget
	bool		m_IPGainIsUp;

	POINT		mIPGainTriangleUp1[3], mIPGainTriangleUp2[3], 
				mIPGainTriangleDown1[3], mIPGainTriangleDown2[3];

	double		m_posIP; // = radRatioPosIP * m_radTxSel;
	double		m_posActiveIP; // = radRatioPosActiveIP * m_radTxSel;


	void		S3DrawCircle(int xo, int yo, int r); // ellipse() wrapper

	void		S3DrawGDIBatt(		char Rx, char Tx, int xref, int yref);
	void		S3DrawGDIBattUnsel(	char Rx, char Tx,
						int xref, int yref, char IsLeft);

	void		ResetCoords(); // TODO: Not used
	void		S3CloseGDIMainScreen();

public:
	CS3GDIScreenMain();
	virtual ~CS3GDIScreenMain();

	virtual void OnInitialUpdate();

	afx_msg		void OnPaint();
	afx_msg		void OnStnClicked();
	afx_msg		void OnStnDblclick();

	// virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	// afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	HDC		GetDrawable()	{ return m_HDC; };
	HFONT	GetDefFont()	{ return m_hFontSB; };

	void		S3GDIReInitialise();

	S3TxPwrMode		m_TxPowerState;

	unsigned char		m_MsgID;  // SW update screen mode...

#ifdef S3_AGENT
	CS3AgentDlg			*m_Parent;
#else
	CS3ControllerDlg	*m_Parent;
#endif
	
	// Public to maintain universal theme for children only...
	COLORREF	m_crMenuBGDark, m_crMenuBGMed, m_crMenuBGLight,
				m_crMenuTxtGreyed, m_crMenuTxtActive,
				m_crTextGrey, m_crTextNorm;

	COLORREF	m_crBlack, m_crLightGrey, m_crMediumGrey, m_crDarkGrey,	m_crWhite;

	HBRUSH		m_hBrushMenuBGDark,	m_hBrushMenuBGMed, m_hBrushMenuBGLight;

	// Generic fonts
	LOGFONT		m_lf;
	HFONT		m_hFontL, m_hFontLB, m_hFontM, m_hFontS, m_hFontSB, m_hFontVS;
	CFont		m_cFontL, m_cFontS;

	void	S3GDIInit(void);
	void	S3GDIRedraw(void);
	void	S3GDIForceRedraw(void);
	void	S3GDIRemoteCmd(void);

	CRect	&S3GDIRectScreen(void) { return m_RectScreen; };
	void	GetBitmapDims(HBITMAP hBmp, int *w, int *h);
	void	GetDCDims(HDC hdc, int *w, int *h);

	void	S3ParaMenuClear() { m_ParaMenu->Clear(); }

	int		S3GFill(TRIVERTEX *vertex, CRect rect); // Gradient filled rectangle

	// TODO: Use this rather than changing m_Screen directly
	int		S3GDIChangeScreen(char screen);
	char	S3GDIGetScreen();

	void	S3DrawGDIRack(void);
	void	S3DrawGDIDbg(void);
	void	S3DrawShuttingDown(void);


	// ------------------------------------------------------------------------
	// Main screen
	void	S3InitGDIRx(void);
	void	S3DrawGDIRx(		char Rx);
	void		S3DrawGDIRxNumber(	char Rx,				int xref, int yref);
	void		S3DrawGDIRxRLL(		char Rx,				int xref, int yref);
	void		S3DrawGDIRxRLL2(	char Rx,				int xref, int yref);
	void		S3DrawGDIRxGain(	char Rx, char Tx,		int xref, int yref);

	void	S3DrawGDIFOLSym(	char Rx,				int xref, int yref, bool left);
	
	void	S3DrawGDITxSel(		char Rx, char Tx,
														int xref, int yref);
	void	S3DrawGDITxUnsel(	char Rx, char Tx,
														int xref, int yref);
	void	S3DrawGDITxId(		char Rx, char Tx,		int xref, int yref);
	void	S3DrawGDITxIdUnsel(	char Rx, char Tx,		int xref, int yref, char IsLeft);
	void	S3DrawGDITxTestIP(	int xref, int yref, int rad, double th, double size);

	// ------------------------------------------------------------------------
	// Header region for all screens
	void	S3DrawGDIHeader(void);
	void	S3DrawGDIBattChargers(void);
	void	S3DrawGDIBattCharge(	char Ch, int x = -1, int y = -1); // Continuous
	void	S3DrawGDIBattChargeSeg(	char Ch, int x = -1, int y = -1); // Segmented
	void	S3DrawGDISettings(	void);
	void	S3DrawGDIShutdown(	void);
	void	S3DrawGDIInfo(		void);
	void	S3DrawGDIInfoRemote(void);
	void	S3DrawGDIBackButton(void);

	// ------------------------------------------------------------------------
	// Receiever screen
	void	S3InitGDIRxScreen(void);
	void	S3CloseGDIRxScreen(void);
	void	S3DrawGDIRxScreen(void);
	void		S3DrawGDIRxRx(		char Rx);
	void		S3DrawGDIRxTx(		char Rx, char Tx);
	void		S3DrawGDIRxRLL(		char Rx, char Tx,		int xref, int yref);
	void		S3DrawGDIRxTxTable(	char Rx);
	void		S3DrawGDIRxTxRowName(	char Row, wchar_t *cstr);
	void		S3DrawGDIRxMessage(	char Rx);
	void		S3DrawGDIColHeader(	char Rx, char Tx, CRect &RectItem);
// TODO: Delete
	void		S3DrawGDIColHeaderOld(	char Rx, char Tx, CRect &RectItem);

	// ------------------------------------------------------------------------
	// Transmitter screen
	void	S3InitGDITxScreen(void);
	void	S3CloseGDITxScreen(void);
	void	S3DrawGDITxScreen(void);
	void		S3DrawGDITxBattSeg(	char Rx, char Tx, int xref, int yref);

	void		S3DrawGDITxTx(		char Rx, char Tx);
	void		S3DrawGDITxIP(		char Rx, char Tx, char IP,	int xref, int yref);
	void		S3DrawGDITxGain(	char Rx, char Tx, char IP,	int xref, int yref);
	void		S3DrawGDITxIPTable(	char Rx, char Tx); //,			int xref, int yref);
	void		S3DrawGDITxIPRowName(		int xref, int yref,
											char Row, CString cstr,
											int Justification = DT_RIGHT);

	CRect		S3RectGDITxIPRowName(		int xref, int yref, char Row,
											int Justification = DT_RIGHT);

	void		S3DrawGDITxMessage(	char Rx, char Tx);


	void		S3DrawGDITxIPParaSelect(	int xref, int yref,
													char SelectedPara);
	void		S3DrawGDIIP(HDC h, HDC h2, char Rx, char Tx, char IP,
										int xref, int yref, int r1);

	void		S3DrawGDIIPGain(char Rx, char Tx, char IP);
										
	void		S3InitGDIIPGain(char Rx, char Tx, char IP,
										int xref, int yref);

	void	S3DrawGDIParaPopUp(			int xref, int yref);

	int		S3FindTxScreen(			POINT p);
	int		S3FindTxScreenPara(		POINT p);
	void	S3GDITxNewTx(void);
	char	CalcGainTickMarks();

	// ------------------------------------------------------------------------

	int		S3GDIIPGainProcess(POINT p);
	void	S3GDIIPGainClose();

	// ------------------------------------------------------------------------
	// Settings screen
	void	S3InitSettingsScreen(void);
	void	S3CloseSettingsScreen(void);
	void	S3DrawGDISettingsScreen(void);
	void		S3DrawGDISettingsRemote(void);
	void		S3DrawGDISettingsDefaults(void);
	void		S3DrawGDISettingsSystem(void);

	int		S3FindSettingScreen(	POINT p);

	// ------------------------------------------------------------------------
	// Charger screen
	void	S3InitGDIChScreen(void);
	void	S3CloseGDIChScreen(void);
	void	S3DrawGDIChScreen(void);
	
	// ------------------------------------------------------------------------
	// Shutdown screen
	void	S3InitGDIShutdownScreen(void);
	void	S3DrawGDIShutdownScreen(void);
	void	S3CloseGDIShutdownScreen(void);

	int		S3FindShutdownScreen(	POINT p);
	int		S3LeaveShutdownScreen(void);

	// ------------------------------------------------------------------------
	// Factory screen
	void	S3InitGDIFactoryScreen(void);
	void	S3DrawGDIFactoryScreen(void);

	int		S3FindFactoryScreen(	POINT p);
	int		S3LeaveFactoryScreen(void);

	// ------------------------------------------------------------------------
	// SWUpdate screen
	void	S3InitGDISWUpdateScreen(void);
	void	S3DrawGDISWUpdateScreen(void);

	int		S3FindSWUpdateScreen(	POINT p);

	// ------------------------------------------------------------------------
	// AppUpdate screen
	void	S3InitGDIAppUpdateScreen(void);
	void	S3DrawGDIAppUpdateScreen(void);

	int		S3FindAppUpdateScreen(	POINT p);

	// ------------------------------------------------------------------------
	// LogCopy screen
	void	S3InitGDILogCopyScreen(void);
	void	S3DrawGDILogCopyScreen(void);

	int		S3FindLogCopyScreen(	POINT p);

	// ------------------------------------------------------------------------
	// Sleep screen
	void	S3DrawGDISleepScreen(void);

	// ------------------------------------------------------------------------
	// Closed screen
	void	S3DrawGDIClosedScreen(void);

	// Resource suppliers...
	// TODO: Or allow public access to selected 'theme' members?
	void GetPopUpColours(	HBRUSH &BrushBG,
							HBRUSH &BrushButton,
							HBRUSH &BrushDispBG);

	void GetPopUpFont(		HFONT &hFontL, COLORREF &crText, COLORREF &crTextBG);


	// Find pixel inclusion
	int		S3Find(					POINT p);
	int		S3FindHeader(			POINT p);
	int		S3FindOverviewScreen(	POINT p);
	int		S3FindRxScreen(			POINT p);
	int		S3FindRxScreenPara(		POINT p);

	int		S3FindChScreen(			POINT p);
	int		S3FindSettingsScreen(	POINT p);

	CRect	DrawNameVal(int xref, int yref,
								   CString Name, CString Val);

	CRect	InitNameVal(int xref, int yref,
								   CString Name, CString Val);

	int		S3GDITextSupplied(const wchar_t *txt);

	void	SelectionChanged();
	void	SetSelection(bool goToOverview);

protected:
	DECLARE_MESSAGE_MAP()

	HBITMAP			S3LoadBitmap(const int &nBitmapID);
	HDC				CreateBitmap(HDC ParentDC, const int &nBitmapID);

	// Charger
	HDC				m_hbmpBattCharging;
	HDC				m_hbmpBattExclam;
	HDC				m_hbmpBattFail;
	HDC				m_hbmpTxSelBattInvalid;
	HDC				m_hbmpTxUnselBattInvalid;

	// Buttons
	HDC				m_hbmpSettings;
	HDC				m_hbmpShutdown;
	HDC				m_hbmpRxBackBtn;
	HDC				m_hbmpRxMainRx1;
	HDC				m_hbmpRxMainRx2;
	HDC				m_hbmpRxMainEmpty;
	HDC				m_hbmpFOL;
	HDC				m_hbmpFOLDark;

	HDC				m_hbmpSysWarn;
	HDC				m_hbmpSysError;
	HDC				m_hbmpSysInfo;

	// Transmitter varieties and add-ons
	// For selected Txs
	HDC				m_hbmpTxSelAct;
	HDC				m_hbmpTxSelInact;
	HDC				m_hbmpTxSelIPInact;
	HDC				m_hbmpTxSelIPAct;
	HDC				m_hbmpTxSelIPActAlrm;
	HDC				m_hbmpTxSelIPInactAlrm;
	HDC				m_hbmpTxTxBattHot;
	HDC				m_hbmpTxTxBattCold;
	HDC				m_hbmpRedLED, m_hbmpBlkLED, m_hbmpGrnLED;
	HDC				m_hbmpTxSelSleep;
	HDC				m_hbmpTxLrgAlarm;
	HDC				m_hbmpTxSmlAlarm;
	HDC				m_hbmpTxLrgEmpty;
	HDC				m_hbmpTxSmlEmpty;

	HDC				m_hbmpRxEmptyBar;

	HDC				m_hbmpTxBatt[S3_N_BATT_SEGS + 2];

	// For unselected Txs
	HDC				m_hbmpTxUnselAct;
	HDC				m_hbmpTxUnselInact;
	HDC				m_hbmpTxUnselIPAct;
	HDC				m_hbmpTxUnselIPInact;
	HDC				m_hbmpTxUnselIPActAlrm;
	HDC				m_hbmpTxUnselIPInactAlrm;
	HDC				m_hbmpTxUnselSleep;

	HDC				m_hbmpTxUBatt[S3_N_BATT_SEGS + 2];

	HDC				m_hbmpBlueButton;
	HDC				m_hbmpGreyButton;
	HDC				m_hbmpRedButton;
//	HDC				m_hbmpBootPPM;

	HDC				m_hbmpRx1RLLOn[5];
	HDC				m_hbmpRx1RLL5Red, m_hbmpRx1RLL1Red;

	HDC				m_hbmpRx2RLLOn[5];
	HDC				m_hbmpRx2RLL5Red, m_hbmpRx2RLL1Red;

	// Main bitmap for blitting
	HBITMAP			m_hBitmap;
	HDC				m_HDC;

	// Settings screen
	CS3NameValue	*m_SettingsAccess;
	CS3NameValue	*m_SettingsPort;
	CS3NameValue	*m_SettingsIPAddr;
	CS3NameValue	*m_SettingsIPSubnet;
	CS3NameValue	*m_SettingsMAC;

	CS3NameValue	*m_SettingsUSBPort;
	CS3NameValue	*m_SettingsUSBDriver;

	CS3NameValue	*m_SettingsContTComp;
	CS3NameValue	*m_SettingsRxAGC;

	CS3NameValue	*m_SettingsUnits;
	CS3NameValue	*m_SettingsScale;
	CS3NameValue	*m_SettingsSize;	// Signal type
	CS3NameValue	*m_Settings3PCLinearity;

	CS3NameValue	*m_SettingsTxStart;
	CS3NameValue	*m_SettingsTxSelfTest;

	CS3NameValue	*m_SettingsGain;
	CS3NameValue	*m_SettingsImp;
#ifdef S3LOWNOISE
	CS3NameValue	*m_SettingsLowNoise;
#endif

	CS3NameValue	*m_SettingsDate;
	CS3NameValue	*m_SettingsTime;

	CS3NameValue	*m_SettingsSN;
	CS3NameValue	*m_SettingsPN;
	CS3NameValue	*m_SettingsSW;
	CS3NameValue	*m_SettingsModel;
	CS3NameValue	*m_SettingsImageDate;
	CS3NameValue	*m_SettingsBuildNum;

	CS3NameValue	*m_SettingsCfg;
	CS3NameValue	*m_SettingsLog;

	// Pop-up number editors to attach to CS3NameValue items
	CS3NumEdit		*m_GDIIPAddrEdit;
	CS3NumEdit		*m_GDIIPSubnetEdit;
	CS3NumEdit		*m_GDIIPPortEdit;
	CS3NumEdit		*m_GDIDefaultGainEdit;

	// Tx screen
	CS3NameValue	*m_TxType, *m_TxNodeName, *m_TxPowerMode,
					*m_TxLaserPow, *m_TxTemp, *m_TxTempComp, *m_TxSN;
	CS3NameValue	*m_TxPeakThresh; //, *m_TxPeakHold;
	CS3NameValue	*m_TxBattT, *m_TxBattI;
	
	CS3NumEdit		*m_GDIGainEdit;
	CS3NumEdit		*m_GDIMaxPowerEdit;
	CS3GDIInfoPopup	*m_TxInfoPopup;
	CS3GDIInfoPopup	*m_TxBattInfoPopup;

	// Rx screen
	CS3NameValue	*m_RxType, *m_RxNodeName, *m_RxTemp, *m_RxVcc, *m_RxSN,
					*m_RxPN, *m_RxFW, *m_RxFWDate; // *m_RxHW, *m_RxAGC, 

	// Text editors to attach to CS3NameValue items
	CS3Edit			*m_GDINodeNameEdit;
	CS3Edit			*m_GDIDateEdit;
	CS3Edit			*m_GDITimeEdit;

#ifndef S3_AGENT
	CS3Edit			*m_GDIMaintKeyEdit;
#endif
//	CS3CheckBox		*m_GDIUSBEnableCB;

public:
	HDC				m_hbmpInfoButton, m_hbmpInfoButtonGrey;
//	afx_msg void OnStnDblclick();
};

// ----------------------------------------------------------------------------
