/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
// ScrollPropertyPage.cpp: Implementierungsdatei
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "ScrollPropertyPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CScrollPropertyPage 

/*IMPLEMENT_DYNCREATE(CScrollPropertyPage, CPropertyPage)
*/
IMPLEMENT_DYNAMIC(CScrollPropertyPage, CPropertyPage)

CScrollPropertyPage::CScrollPropertyPage() : CPropertyPage(), m_ClientRect(0, 0, 0, 0),
	m_ScrollSize(0, 0), m_ClientSize(0, 0), m_HPos(0), m_VPos(0)
{
	//{{AFX_DATA_INIT(CScrollPropertyPage)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}

CScrollPropertyPage::CScrollPropertyPage(UINT nIDTemplate, UINT nIDCaption)
	: CPropertyPage(nIDTemplate, nIDCaption), m_ClientRect(0, 0, 0, 0),
	m_ScrollSize(0, 0), m_ClientSize(0, 0), m_HPos(0), m_VPos(0)
{
}

CScrollPropertyPage::~CScrollPropertyPage()
{
}

void CScrollPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScrollPropertyPage)
		// HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScrollPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CScrollPropertyPage)
	ON_WM_CREATE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CScrollPropertyPage 

int CScrollPropertyPage::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (-1 == CPropertyPage::OnCreate(lpCreateStruct))
		return -1;

	RECT WinRect;
	GetWindowRect(&WinRect);
//	if(m_ClientRect != CRect(0,0,0,0)) {
//		MoveWindow(&m_ClientRect,false);
//	}
	m_ClientSize = CSize(WinRect.right - WinRect.left, WinRect.bottom - WinRect.top);

	ModifyStyle(0, WS_HSCROLL | WS_VSCROLL);
	RecalcScrollbars();
	
	return 0;
}

void CScrollPropertyPage::RecalcScrollbars()
{
	CRect WinRect;
	GetWindowRect(&WinRect);

	SCROLLINFO	ScrollInfo;
	ScrollInfo.nMin  = 0;
	ScrollInfo.nMax  = m_ClientSize.cy;
	ScrollInfo.nPage = WinRect.bottom - WinRect.top;
	ScrollInfo.fMask = SIF_RANGE | SIF_PAGE;
	SetScrollInfo(SB_VERT, &ScrollInfo);

	ScrollInfo.nMin  = 0;
	ScrollInfo.nMax  = m_ClientSize.cx;
	ScrollInfo.nPage = WinRect.right - WinRect.left;
	ScrollInfo.fMask = SIF_RANGE | SIF_PAGE;
	SetScrollInfo(SB_HORZ, &ScrollInfo);
}

void CScrollPropertyPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SCROLLINFO ScrollInfo;
	GetScrollInfo(SB_HORZ, &ScrollInfo);

	int OldPos, NewPos;
	OldPos = ScrollInfo.nPos;

	switch (nSBCode) {
		case SB_LINEDOWN:
			NewPos = OldPos + 2;
			break;

		case SB_LINEUP:
			NewPos = OldPos - 2;
			break;

		case SB_THUMBTRACK:
			NewPos = nPos;
			break;

		case SB_PAGEDOWN:
			NewPos = OldPos + ScrollInfo.nPage;
			break;

		case SB_PAGEUP:
			NewPos = OldPos - ScrollInfo.nPage;
			break;

		default:
			NewPos = OldPos;
	}
	if (NewPos < 0)
		NewPos = 0;	// an den Anfang scrollen
	else if (NewPos > m_ScrollSize.cx)
		NewPos = m_ScrollSize.cx;	// ans Ende Scrollen
	
	ScrollTo(NewPos, -1);
	ScrollInfo.fMask = SIF_POS;
	ScrollInfo.nPos  = NewPos;
	SetScrollInfo(SB_HORZ, &ScrollInfo);
}

void CScrollPropertyPage::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SCROLLINFO ScrollInfo;
	GetScrollInfo(SB_VERT, &ScrollInfo);

	int OldPos, NewPos;
	OldPos = ScrollInfo.nPos;

	switch (nSBCode) {
		case SB_LINEDOWN:
			NewPos = OldPos + 2;
			break;

		case SB_LINEUP:
			NewPos = OldPos - 2;
			break;

		case SB_THUMBTRACK:
			NewPos = nPos;
			break;

		case SB_PAGEDOWN:
			NewPos = OldPos + ScrollInfo.nPage;
			break;

		case SB_PAGEUP:
			NewPos = OldPos - ScrollInfo.nPage;
			break;

		default:
			NewPos = OldPos;
	}
	if (NewPos < 0)
		NewPos = 0;	// an den Anfang scrollen
	else if (NewPos > m_ScrollSize.cy)
		NewPos = m_ScrollSize.cy;	// ans Ende Scrollen

	ScrollTo(-1, NewPos);
	ScrollInfo.fMask = SIF_POS;
	ScrollInfo.nPos  = NewPos;
	SetScrollInfo(SB_VERT, &ScrollInfo);
}

void CScrollPropertyPage::ScrollTo(int HPos, int VPos)
{
	if(-1 == HPos)
		HPos = m_HPos;
	
	if (-1 == VPos)
		VPos = m_VPos;
	
	ScrollWindow(m_HPos - HPos, m_VPos - VPos);
	m_HPos = HPos;
	m_VPos = VPos;
}

void CScrollPropertyPage::OnSize(UINT nType, int cx, int cy) 
{
	CPropertyPage::OnSize(nType, cx, cy);
	
	RECT WinRect;
	GetWindowRect(&WinRect);
	m_ScrollSize = CSize(m_ClientSize.cx - (WinRect.right - WinRect.left), 
						 m_ClientSize.cy - (WinRect.bottom - WinRect.top));

	if (m_ScrollSize.cx < 0)	
		m_ScrollSize.cx = 0;
	if (m_ScrollSize.cy < 0) 
		m_ScrollSize.cy = 0;	

	ScrollTo(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
	RecalcScrollbars();
}

void CScrollPropertyPage::SetClientRect(const CRect &NewRect)
{
	m_ClientRect = NewRect;
}

BOOL CScrollPropertyPage::OnSetActive() 
{
	if (CRect(0, 0, 0, 0) != m_ClientRect)
		MoveWindow(&m_ClientRect);
	
	return CPropertyPage::OnSetActive();
}
