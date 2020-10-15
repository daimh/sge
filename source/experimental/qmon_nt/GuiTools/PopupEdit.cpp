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
// PopupEdit.cpp : implementation file
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "resource.h"
#include "PopupEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CPopupEdit

IMPLEMENT_DYNAMIC(CPopupEdit, CEdit)

CPopupEdit::CPopupEdit()
{
	m_bIsEmbeddedIntoListCtrl = false;
	m_bESC = false;

	m_iItem = m_iSubItem = -1;
}

CPopupEdit::CPopupEdit(int iItem, int iSubItem, const CString &sInitText)
{
	m_bIsEmbeddedIntoListCtrl = true;
	m_bESC = false;

	m_iItem     = iItem;
	m_iSubItem  = iSubItem;

	m_sInitText = sInitText;
}

CPopupEdit::~CPopupEdit()
{
}

BEGIN_MESSAGE_MAP(CPopupEdit, CEdit)
	//{{AFX_MSG_MAP(CPopupEdit)
	ON_WM_NCDESTROY()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_COMMAND(ID_POPUP_INFINITY, OnPopupInfinity)
	ON_COMMAND(ID_POPUP_KBYTE1000, OnPopupKbyte1000)
	ON_COMMAND(ID_POPUP_KBYTE1024, OnPopupKbyte1024)
	ON_COMMAND(ID_POPUP_GBYTE1000, OnPopupGbyte1000)
	ON_COMMAND(ID_POPUP_GBYTE1024, OnPopupGbyte1024)
	ON_COMMAND(ID_POPUP_MBYTE1000, OnPopupMbyte1000)
	ON_COMMAND(ID_POPUP_MBYTE1024, OnPopupMbyte1024)
	ON_COMMAND(ID_POPUP_BYTE, OnPopupByte)
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPopupEdit message handlers

void CPopupEdit::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CPoint local = point; 
	ClientToScreen(&local);

	CMenu menu;
	VERIFY(menu.LoadMenu(IDM_MEMORYUNITS));
	CMenu *PopupMenu = menu.GetSubMenu(0);
	ASSERT_VALID(PopupMenu);

	SetFocus();
	PopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, local.x, local.y, this);
	
	CEdit::OnRButtonDown(nFlags, point);
}

void CPopupEdit::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	OnRButtonDown(nFlags,point);
}

void CPopupEdit::OnPopupInfinity() 
{
	SetWindowText("INFINITY");
}

void CPopupEdit::OnPopupKbyte1000() 
{
	CString Temp;

	GetWindowText(Temp);
	Temp.Format("%d%c", atoi(Temp), 'k');
	SetWindowText(Temp);
	SetSel(0, Temp.GetLength() - 1);
}

void CPopupEdit::OnPopupKbyte1024() 
{
	CString Temp;

	GetWindowText(Temp);
	Temp.Format("%d%c", atoi(Temp), 'K');
	SetWindowText(Temp);
	SetSel(0, Temp.GetLength() - 1);
}

void CPopupEdit::OnPopupGbyte1000() 
{
	CString Temp;

	GetWindowText(Temp);
	Temp.Format("%d%c", atoi(Temp), 'g');
	SetWindowText(Temp);
	SetSel(0, Temp.GetLength() - 1);
}

void CPopupEdit::OnPopupGbyte1024() 
{
	CString Temp;

	GetWindowText(Temp);
	Temp.Format("%d%c", atoi(Temp), 'G');
	SetWindowText(Temp);
	SetSel(0, Temp.GetLength() - 1);
}

void CPopupEdit::OnPopupMbyte1000() 
{
	CString Temp;

	GetWindowText(Temp);
	Temp.Format("%d%c", atoi(Temp), 'm');
	SetWindowText(Temp);
	SetSel(0, Temp.GetLength() - 1);
}

void CPopupEdit::OnPopupMbyte1024() 
{
	CString Temp;

	GetWindowText(Temp);
	Temp.Format("%d%c", atoi(Temp), 'M');
	SetWindowText(Temp);
	SetSel(0, Temp.GetLength() - 1);
}

void CPopupEdit::OnPopupByte() 
{
	CString Temp;

	GetWindowText(Temp);
	Temp.Format("%d", atoi(Temp));
	SetWindowText(Temp);
	SetSel(0, Temp.GetLength());
}

void CPopupEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);

	CString Text;
	GetWindowText(Text);

	if (Text != "INFINITY") {
		if (!atoi(Text)) {
			SetWindowText(m_sInitText);
			GetWindowText(Text);
		}

		Text.Format("%d", atoi(Text));

		CString Temp;
		GetWindowText(Temp);
		Temp = Temp.Right(1);
				
		if (!Temp.SpanIncluding("kKmMgG").IsEmpty())
			Text.Format("%s%s", LPCTSTR(Text), LPCTSTR(Temp));

		CString OldText;
		GetWindowText(OldText) ;
		if (GetModify() || OldText != Text)
			SetWindowText(Text);
	}

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

void CPopupEdit::OnNcDestroy() 
{
	CEdit::OnNcDestroy();

	if (m_bIsEmbeddedIntoListCtrl) {
		ASSERT_VALID(this);
		delete this;
	}
}

BOOL CPopupEdit::PreTranslateMessage(MSG *pMsg)
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
	
	return CEdit::PreTranslateMessage(pMsg);
}

void CPopupEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CWnd *Parent = GetParent();
	ASSERT_VALID(Parent);

	if (m_bIsEmbeddedIntoListCtrl &&
		(VK_ESCAPE == nChar || VK_RETURN == nChar))	
	{
		if (VK_ESCAPE == nChar)
			m_bESC = true;
		Parent->SetFocus();

		return;
	}

	CEdit::OnChar(nChar, nRepCnt, nFlags);

	if (m_bIsEmbeddedIntoListCtrl) {
		CString str;
		GetWindowText( str );

		CWindowDC dc(this);
		CFont *pFont = Parent->GetFont();
		ASSERT_VALID(pFont);
		CFont *pFontDC = dc.SelectObject(pFont);
		ASSERT_VALID(pFontDC);
		CSize size = dc.GetTextExtent(str);
		dc.SelectObject(pFontDC);

		size.cx += 5;	
		CRect rect, parentrect;	
		GetClientRect(&rect);
		Parent->GetClientRect(&parentrect);
		Parent->ScreenToClient(&rect);

		if (size.cx > rect.Width())	{
			if (size.cx + rect.left < parentrect.right)
				rect.right = rect.left + size.cx;
			else	
				rect.right = parentrect.right;

			MoveWindow(&rect);
		}
	}
}

int CPopupEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{	
	ASSERT(NULL != lpCreateStruct);

	if (-1 == CEdit::OnCreate(lpCreateStruct))
		return -1;

	if (m_bIsEmbeddedIntoListCtrl) {
		CWnd *Parent = GetParent();
		ASSERT_VALID(Parent);

		CFont *Font = Parent->GetFont();
		ASSERT_VALID(Font);

		SetFont(Font);
		SetWindowText(m_sInitText);
		SetFocus();

		CString Temp = m_sInitText.Right(1);
		if ((Temp.SpanIncluding("kKmMgG")).IsEmpty())
			SetSel(0, -1);
		else
			SetSel(0, m_sInitText.GetLength() - 1);
	}

	return 0;	
}
