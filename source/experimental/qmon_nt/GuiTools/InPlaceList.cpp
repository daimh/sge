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
// InPlaceList.cpp : implementation file
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "InPlaceList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInPlaceList

CInPlaceList::CInPlaceList()
{
	m_bIsEmbeddedIntoListCtrl = false;
	m_bESC = false;

	m_iItem = m_iSubItem = -1;
}

CInPlaceList::CInPlaceList(int iItem, int iSubItem, const CString &sInitText)
{
	m_bIsEmbeddedIntoListCtrl = true;
	m_bESC           = false;

	m_iItem          = iItem;
	m_iSubItem       = iSubItem;

	m_sInitText      = sInitText;
}

CInPlaceList::~CInPlaceList()
{
}

BEGIN_MESSAGE_MAP(CInPlaceList, CListBox)
	//{{AFX_MSG_MAP(CInPlaceList)
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	ON_WM_CHAR()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceList message handlers
BOOL CInPlaceList::PreTranslateMessage(MSG* pMsg)
{
	ASSERT(NULL != pMsg);

	if (WM_KEYDOWN == pMsg->message &&
		(VK_RETURN == pMsg->wParam || VK_DELETE == pMsg->wParam || 
		 VK_ESCAPE == pMsg->wParam || GetKeyState(VK_CONTROL))) 
	{
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;		
	}

	return CListBox::PreTranslateMessage(pMsg);
}

void CInPlaceList::OnKillFocus(CWnd* pNewWnd) 
{
	CListBox::OnKillFocus(pNewWnd);

	if (m_bIsEmbeddedIntoListCtrl) {
		CWnd *Parent = GetParent();
		ASSERT_VALID(Parent);

		CString str;
		GetWindowText(str);

		LV_DISPINFO dispinfo;
		dispinfo.hdr.hwndFrom  = Parent->m_hWnd;
		dispinfo.hdr.idFrom    = GetDlgCtrlID();
		dispinfo.hdr.code      = LVN_ENDLABELEDIT;
		dispinfo.item.mask     = LVIF_TEXT;
		dispinfo.item.iItem    = m_iItem;
		dispinfo.item.iSubItem = m_iSubItem;
		if (m_bESC) {
			dispinfo.item.pszText    = NULL;
			dispinfo.item.cchTextMax = 0;
		}
		else {
			dispinfo.item.pszText    = const_cast<LPTSTR>((LPCTSTR) str);
			dispinfo.item.cchTextMax = str.GetLength();
		}

		CWnd *ParentParent = Parent->GetParent();
		ASSERT_VALID(ParentParent);
		ParentParent->SendMessage(WM_NOTIFY, Parent->GetDlgCtrlID(), LPARAM(&dispinfo));	
		DestroyWindow();	
	}
}

void CInPlaceList::OnNcDestroy() 
{
	CListBox::OnNcDestroy();

	if (m_bIsEmbeddedIntoListCtrl) {
		ASSERT_VALID(this);
		delete this;	
	}
}

void CInPlaceList::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CWnd *Parent = GetParent();
	ASSERT_VALID(Parent);

	if (m_bIsEmbeddedIntoListCtrl && (
		VK_ESCAPE == nChar || VK_RETURN == nChar)) 
	{
		if (VK_ESCAPE == nChar)
			m_bESC = true;
		Parent->SetFocus();

		return;
	}

	CListBox::OnChar(nChar, nRepCnt, nFlags);	

/*
	// Resize edit control if needed
	// Get text extent
	CString str;
	GetWindowText( str );
	CWindowDC dc(this);
	CFont *pFont = GetParent()->GetFont();
	CFont *pFontDC = dc.SelectObject( pFont );
	CSize size = dc.GetTextExtent( str );
	dc.SelectObject( pFontDC );
	size.cx += 5;			   	// add some extra buffer	// Get client rect
	CRect rect, parentrect;	GetClientRect( &rect );
	GetParent()->GetClientRect( &parentrect );
	// Transform rect to parent coordinates	ClientToScreen( &rect );
	GetParent()->ScreenToClient( &rect );
	// Check whether control needs to be resized
	// and whether there is space to grow
	if( size.cx > rect.Width() )	{
		if( size.cx + rect.left < parentrect.right )
			rect.right = rect.left + size.cx;
		else	
			rect.right = parentrect.right;
		MoveWindow( &rect );
	}
*/
}

int CInPlaceList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	ASSERT(NULL != lpCreateStruct);

	if (-1 == CListBox::OnCreate(lpCreateStruct))
		return -1;

	if (m_bIsEmbeddedIntoListCtrl) {
		CWnd *Parent = GetParent();
		ASSERT_VALID(Parent);

		CFont *Font = Parent->GetFont();
		ASSERT_VALID(Font);
		SetFont(Font);

		if (!m_sInitText.IsEmpty()) {
			for (int i = m_sInitText.Find(","); -1 != i; i = m_sInitText.Find(",")) 
			{
				AddString(m_sInitText.Left(i));
				m_sInitText.Delete(0, i + 2);
			}

			AddString(m_sInitText);
		}

		SetFocus();
	}

	return 0;
}
