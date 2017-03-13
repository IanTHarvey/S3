#pragma once



// CS3FactoryScreen form view

class CS3FactoryScreen : public CFormView
{
	DECLARE_DYNCREATE(CS3FactoryScreen)

protected:
	CS3FactoryScreen();           // protected constructor used by dynamic creation
	virtual ~CS3FactoryScreen();

public:
	enum { IDD = IDD_POCKETPC_LANDSCAPE };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedFinishButton();
};


