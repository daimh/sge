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
// ViewDlgBar.cpp: Implementierungsdatei
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "ViewDlgBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CViewDlgBar 

CViewDlgBar::CViewDlgBar() : CDialog()
{
	m_NotifyWnd = NULL;
}

CViewDlgBar::CViewDlgBar(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
{
	m_NotifyWnd = NULL;

	//{{AFX_DATA_INIT(CViewDlgBar)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}

void CViewDlgBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewDlgBar)
		// HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CViewDlgBar, CDialog)
	//{{AFX_MSG_MAP(CViewDlgBar)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CViewDlgBar 

void CViewDlgBar::SetNotifyWnd(CWnd *pWnd)
{
	m_NotifyWnd = pWnd;
}

BOOL CViewDlgBar::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	return NULL != m_NotifyWnd 
		? m_NotifyWnd->SendMessage(WM_COMMAND, wParam, lParam)
		: CDialog::OnCommand(wParam, lParam);
}

void CViewDlgBar::OnSize(UINT nType, int cx, int cy) 
{
	CWnd *FirstDlgItem = GetDlgItem(IDC_STATIC_LINE);

	if (NULL != FirstDlgItem) {
		CRect WndRect;
		FirstDlgItem->GetWindowRect(&WndRect);
		FirstDlgItem->SetWindowPos(NULL, 0, 0, cx, WndRect.Height(),
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	CDialog::OnSize(nType, cx, cy);
}
