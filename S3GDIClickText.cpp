#include "stdafx.h"

#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

#include "S3GDIClickText.h"

// ----------------------------------------------------------------------------

void CClickText::Draw(void)
{
	HGDIOBJ fobj;

	// S3_RECT(m_HDC, m_r);
	
	// This ties this class to the TxScreen, so have to make sure
	// m_TxPowerMode == S3_TX_ON in all other contexts if re-used
	if (m_Enabled && m_Parent->m_TxPowerState == S3_TX_ON)
	{
		if (m_Font == 0)
			fobj = SelectObject(m_HDC, m_Parent->m_hFontSB);
		else
			fobj = SelectObject(m_HDC, m_Parent->m_hFontLB);
	}
	else
	{
		if (m_Font == 0)
			fobj = SelectObject(m_HDC, m_Parent->m_hFontS);
		else
			fobj = SelectObject(m_HDC, m_Parent->m_hFontL);
	}

	DrawText(m_HDC, m_s, -1, &m_r, m_Justification);

	SelectObject(m_HDC, fobj);
}

// ----------------------------------------------------------------------------

BOOL CClickText::Find(POINT p)
{
	if (m_Enabled)
		return m_r.PtInRect(p);

	return FALSE;
}

// ----------------------------------------------------------------------------
