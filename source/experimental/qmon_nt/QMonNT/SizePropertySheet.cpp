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
// SizePropertySheet.cpp: Implementierungsdatei
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "SizePropertySheet.h"
#include "ScrollPropertyPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSizePropertySheet

IMPLEMENT_DYNAMIC(CSizePropertySheet, CPropertySheet)

CSizePropertySheet::CSizePropertySheet() : CPropertySheet(),
	m_PageOffset(0, 0), m_TabOffset(0, 0), m_TabInset(0, 0),
	m_PageInset(0, 0),  m_SavedSize(0, 0), m_ButtonBarSize(0, 0)
{
}

CSizePropertySheet::CSizePropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
	  m_PageOffset(0, 0), m_TabOffset(0, 0), m_TabInset(0, 0),
	  m_PageInset(0, 0),  m_SavedSize(0, 0), m_ButtonBarSize(0, 0)
{
}

CSizePropertySheet::CSizePropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(pszCaption, pParentWnd, iSelectPage),
	  m_PageOffset(0, 0), m_TabOffset(0, 0), m_TabInset(0, 0),
	  m_PageInset(0, 0),  m_SavedSize(0, 0), m_ButtonBarSize(0, 0)

{
}

CSizePropertySheet::~CSizePropertySheet()
{
}

BEGIN_MESSAGE_MAP(CSizePropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CSizePropertySheet)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CSizePropertySheet 

void CSizePropertySheet::Resize(const CSize &SizeNew)
{
	CSize NewSize = SizeNew;
	if (CSize(0, 0) == NewSize) {
		ASSERT(CSize(0, 0) != m_SavedSize);
		NewSize = m_SavedSize;
	} 
	else
		m_SavedSize = NewSize;

	CTabCtrl *pTab = GetTabControl();
	ASSERT_VALID(pTab);
	CRect TabRect, SheetRect;
	pTab->GetWindowRect(&TabRect);
	GetWindowRect(&SheetRect);
	
	CRect PageRect;
	CPropertyPage *ActivePage = GetActivePage();
	ASSERT_VALID(ActivePage);
	ActivePage->GetWindowRect(&PageRect);

	ScreenToClient(&PageRect);
	ScreenToClient(&TabRect);
	ScreenToClient(&SheetRect);

	// Das eigentliche PropertySheet:
	MoveWindow(0, 0, NewSize.cx, NewSize.cy, FALSE);

	m_ButtonBar.MoveWindow(m_PageInset.cx, NewSize.cy - m_ButtonBarSize.cy - m_PageInset.cy, 
		NewSize.cx - 2 * m_PageInset.cx, m_ButtonBarSize.cy);

	// Das Tab-Control:
	pTab->MoveWindow(m_TabOffset.x, m_TabOffset.y,
		NewSize.cx - m_TabOffset.x - m_TabInset.cx,		// Breite
		NewSize.cy - m_TabOffset.y - m_TabInset.cy);	// Höhe

	// Die aktive Seite (PropertyPage):
	int PageCount = GetPageCount();
	CScrollPropertyPage *SPropPage;
	for (int i = 0; i < PageCount; i++) {
		SPropPage = dynamic_cast<CScrollPropertyPage *>(GetPage(i));
		ASSERT_VALID(SPropPage);

		SPropPage->SetClientRect(CRect(m_PageOffset.x, m_PageOffset.y,
			m_PageOffset.x + NewSize.cx - m_PageOffset.x - m_PageInset.cx,	// Breite
			m_PageOffset.y + NewSize.cy - m_PageOffset.y - m_PageInset.cy - m_ButtonBarSize.cy-2));	// Höhe
	}
	ActivePage->MoveWindow(m_PageOffset.x, m_PageOffset.y,
		NewSize.cx - m_PageOffset.x - m_PageInset.cx,	// Breite
		NewSize.cy - m_PageOffset.y - m_PageInset.cy - m_ButtonBarSize.cy-2);	// Höhe

	pTab->ModifyStyle(TCS_MULTILINE, TCS_SINGLELINE);
}

BOOL CSizePropertySheet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
	CTabCtrl *pTab = GetTabControl();
	ASSERT_VALID(pTab);
	CRect TabRect, SheetRect;
	pTab->GetWindowRect(&TabRect);
	GetWindowRect(&SheetRect);

	CRect PageRect;
	CPropertyPage *ActivePage = GetActivePage();
	ASSERT_VALID(ActivePage);
	ActivePage->GetWindowRect(&PageRect);

	ScreenToClient(&PageRect);
	ScreenToClient(&TabRect);
	ScreenToClient(&SheetRect);

	m_PageOffset = PageRect.TopLeft();
	m_TabOffset  = TabRect.TopLeft();
	m_PageInset  = CSize(SheetRect.right - PageRect.right, 
						SheetRect.bottom - PageRect.bottom);
	m_TabInset   = CSize(SheetRect.right - TabRect.right,
						SheetRect.bottom - TabRect.bottom);

	m_ButtonBar.Create(IDD_SHEETDLGBAR, this);
	CWnd *Parent = GetParent();
	ASSERT_VALID(Parent);
	m_ButtonBar.SetNotifyWnd(Parent);

	CRect BarRect;
	m_ButtonBar.GetWindowRect(&BarRect);
	m_ButtonBarSize = BarRect.Size();

	m_ButtonBar.SetWindowPos(NULL, m_PageInset.cx, 
		SheetRect.bottom - m_ButtonBarSize.cy - m_PageInset.cy, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	m_ButtonBar.ShowWindow(SW_SHOW);

	return bResult;
}

void CSizePropertySheet::EnableButtonSend(bool bEnable)
{
	CWnd *Button = m_ButtonBar.GetDlgItem(IDC_SEND);
	ASSERT_VALID(Button);
	Button->EnableWindow(bEnable);
}

void CSizePropertySheet::EnableButtonRevert(bool bEnable)
{
	CWnd *Button = m_ButtonBar.GetDlgItem(IDC_REVERT);
	ASSERT_VALID(Button);
	Button->EnableWindow(bEnable);
}
