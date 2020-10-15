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
// PopupTimeCtrl.cpp : implementation file
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "resource.h"
#include "PopupTimeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPopupTimeCtrl

CPopupTimeCtrl::CPopupTimeCtrl()
{
	m_bIsEmbeddedIntoListCtrl = false;
	m_bESC = false;
	m_ListCtrlRow = m_ListCtrlCol = -1;
	
	m_Hours     = 0;
	m_Delta     = 0;
	m_KeyCount  = 0;
	m_bInfinity = false;
	m_bClick    = false;
}

CPopupTimeCtrl::CPopupTimeCtrl(int ListCtrlRow, int ListCtrlCol, const CString &InitText)
{
	m_bIsEmbeddedIntoListCtrl	= true;
	m_bESC						= false;
	m_ListCtrlRow				= ListCtrlRow;
	m_ListCtrlCol				= ListCtrlCol;
	m_InitText					= InitText;
	
	m_Hours		= 0;
	m_Delta		= 0;
	m_KeyCount	= 0;
	m_bInfinity = false;
	m_bClick    = false;
}

CPopupTimeCtrl::~CPopupTimeCtrl()
{
}

BEGIN_MESSAGE_MAP(CPopupTimeCtrl, CDateTimeCtrl)
	ON_WM_CONTEXTMENU()
	//{{AFX_MSG_MAP(CPopupTimeCtrl)
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_COMMAND(ID_POPUP_TIMEINFINITY, OnPopupTimeInfinity)
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_NOTIFY_REFLECT(DTN_DATETIMECHANGE, OnDatetimechange)
	ON_NOTIFY_REFLECT(DTN_FORMATQUERY, OnFormatquery)
	ON_NOTIFY_REFLECT(DTN_FORMAT, OnFormat)
	ON_NOTIFY_REFLECT(DTN_WMKEYDOWN, OnWmkeydown)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPopupTimeCtrl message handlers
void CPopupTimeCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CPoint local = point; 
	ClientToScreen(&local);

	CMenu menu;
	VERIFY(menu.LoadMenu(IDM_TIMEPOPUP));
	CMenu *PopupMenu = menu.GetSubMenu(0);
	ASSERT_VALID(PopupMenu);

	SetFocus();
	PopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, local.x, local.y, this);
	
	CDateTimeCtrl::OnRButtonDown(nFlags, point);
}

void CPopupTimeCtrl::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	OnRButtonDown(nFlags, point);
}

void CPopupTimeCtrl::OnContextMenu(CWnd*, CPoint point)
{
	if (-1 == point.x && -1 == point.y) {
		CRect rect;
		GetClientRect(rect);
		ClientToScreen(rect);

		point = rect.TopLeft();
		point.Offset(5, 5);
	}

	CMenu menu;
	VERIFY(menu.LoadMenu(CG_IDR_POPUP_POPUP_TIME_CTRL));

	CMenu *pPopup = menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);
	CWnd *pWndPopupOwner = this;

	while (pWndPopupOwner->GetStyle() & WS_CHILD) {
		pWndPopupOwner = pWndPopupOwner->GetParent();
		ASSERT_VALID(pWndPopupOwner);
	}

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWndPopupOwner);
}

void CPopupTimeCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (this == GetFocus()) {
		CWnd *Parent = GetParent();
		ASSERT_VALID(Parent);

		CWnd *Item = Parent->GetNextDlgTabItem(this);
		ASSERT_VALID(Item);
		Item->SetFocus();
	}

	CDateTimeCtrl::OnLButtonDown(nFlags, point);
}

void CPopupTimeCtrl::OnPopupTimeInfinity() 
{
	SYSTEMTIME	SysTime;

	if (GDT_NONE != DateTime_GetSystemtime(m_hWnd, &SysTime)) {
		SetFormat("' INFINITY'");
		DateTime_SetSystemtime(m_hWnd, GDT_NONE, &SysTime);
		m_bInfinity = true;
		m_KeyCount = 0;

		NMHDR Message;
		Message.code     = DTN_HOURCHANGE;
		Message.hwndFrom = GetSafeHwnd();
		Message.idFrom   = GetDlgCtrlID();

		CWnd *NotifyWnd = GetParent();
		ASSERT_VALID(NotifyWnd);
		if (m_bIsEmbeddedIntoListCtrl) {
			NotifyWnd = NotifyWnd->GetParent();
			ASSERT_VALID(NotifyWnd);
		}

		NotifyWnd->SendMessage(WM_NOTIFY, NULL, LPARAM(&Message));
	}
}

BOOL CPopupTimeCtrl::SetTime(const CTime &TimeNew)
{
	m_Hours = TimeNew.GetHour() + (TimeNew.GetMonth() - 1) * 24 + 
		(TimeNew.GetDay() - 1) * 24;

	if (2037 == TimeNew.GetYear()) {
		CTime temp(2037, 1, 1, 0, 0, 0);
		CDateTimeCtrl::SetTime(&temp);
		m_Hours = 0;

		SYSTEMTIME SysTime;
		DateTime_GetSystemtime(m_hWnd, &SysTime);
		SetFormat("' INFINITY'");
		DateTime_SetSystemtime(m_hWnd, GDT_NONE, &SysTime);
		m_bInfinity = true;

		return TRUE;
	}
	
	if (m_bInfinity) {
		SetFormat("XXX':'mm':'ss");
		m_bInfinity = false;
	}

	return CDateTimeCtrl::SetTime(&TimeNew);
}

DWORD CPopupTimeCtrl::GetTime(CTime &timeDest)
{
	CTime temp;
	DWORD result = CDateTimeCtrl::GetTime(temp);

	UINT year, month, day, hour, minute, second;

	if (m_bInfinity)
		year = 2037;
	else
		year = 1999;

	if (m_Hours > 23) {
		if (m_Hours > 720) {
			month = (m_Hours - (m_Hours % 24) - 720) / 24 + 1;
			day   = 31;
		}
		else {
			month = 1;
			day   = (m_Hours - (m_Hours % 24)) / 24 + 1;
		}
	}
	else {
		month = 1;
		day = 1;
	}
	
	hour = m_Hours - (day - 1) * 24 - (month - 1) * 24;
	minute = temp.GetMinute();
	second = temp.GetSecond();

	timeDest = CTime(year, month, day, hour, minute, second);
	return result;
}

BOOL CPopupTimeCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	ASSERT(NULL != pResult);
	
	NMHDR *pNMHDR = reinterpret_cast<NMHDR *>(lParam);
	ASSERT(NULL != pNMHDR);

	UINT notificationCode	= pNMHDR->code;
	UINT ItemID				= pNMHDR->idFrom;

	if (DTM_SETFORMAT == notificationCode)
		AfxMessageBox("foo");

	NMUPDOWN *UpDown;
	SYSTEMTIME	SysTime;

	if (GDT_NONE != DateTime_GetSystemtime(m_hWnd, &SysTime))
		switch (notificationCode) {
			case UDN_DELTAPOS:
				ASSERT(!m_bInfinity);

				m_bClick    = true;
				UpDown     = reinterpret_cast<NMUPDOWN *>(lParam);
				ASSERT(NULL != UpDown);
				m_Delta    = UpDown->iDelta;
				m_Hours   -= m_Delta;
				m_KeyCount = 0;

				if (m_Hours > 999)
					m_Hours = 0;
				if (m_Hours < 0 )
					m_Hours = 999;

				CWnd *NotifyWnd = GetParent();
				ASSERT_VALID(NotifyWnd);
				if (m_bIsEmbeddedIntoListCtrl) {
					NotifyWnd = NotifyWnd->GetParent();
					ASSERT_VALID(NotifyWnd);
				}

				NMHDR Message;
				Message.code     = DTN_HOURCHANGE;
				Message.hwndFrom = GetSafeHwnd();
				Message.idFrom   = GetDlgCtrlID();
					
				NotifyWnd->SendMessage(WM_NOTIFY, NULL, LPARAM(&Message));

				RedrawWindow();
				break;
		}

	return CDateTimeCtrl::OnNotify(wParam, lParam, pResult);
}

void CPopupTimeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	m_bClick = false;

	CDateTimeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPopupTimeCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CDateTimeCtrl::OnKillFocus(pNewWnd);
	
    if (m_bIsEmbeddedIntoListCtrl) {
		CString str;
		CTime temp;
		GetTime(temp);
		str = TimeToString(temp);

		CWnd *ParentListCtrl = GetParent();
		ASSERT_VALID(ParentListCtrl);

		LV_DISPINFO dispinfo;
		dispinfo.hdr.hwndFrom   = ParentListCtrl->m_hWnd;
		dispinfo.hdr.idFrom     = GetDlgCtrlID();
		dispinfo.hdr.code       = LVN_ENDLABELEDIT;

		dispinfo.item.mask      = LVIF_TEXT;
		dispinfo.item.iItem     = m_ListCtrlRow;
		dispinfo.item.iSubItem  = m_ListCtrlCol;

		if (m_bESC) {
			dispinfo.item.pszText    = NULL;
			dispinfo.item.cchTextMax = 0;
		}
		else {
			dispinfo.item.pszText    = const_cast<LPTSTR>((LPCTSTR) str);
			dispinfo.item.cchTextMax = str.GetLength();
		}

		ParentListCtrl->SendMessage(WM_NOTIFY, NULL, LPARAM(&dispinfo));
		DestroyWindow();
	}
}

void CPopupTimeCtrl::OnNcDestroy() 
{
	CDateTimeCtrl::OnNcDestroy();

	if (m_bIsEmbeddedIntoListCtrl) {
		ASSERT_VALID(this);
		delete this;
	}
}

CTime CPopupTimeCtrl::StringToTime(const CString &stringTime)
{
	UINT year, month, day, hour, min, sec;
	UINT i, j, k;
	
	if (stringTime == "INFINITY") {
		year  = 2037;
		month = day = 1;
		hour  = min = sec = 0;
	} 
	else {
		year = 1999;
		i = stringTime.Find(':');
		j = stringTime.ReverseFind(':');
		k = atoi(stringTime.Left(i));

		if (k > 23 ) {
			if (k > 720 ) {
				month = (k - (k % 24) - 720) / 24 + 1;
				day   = 31;
			} 
			else {
				month = 1;
				day   = (k - (k % 24)) / 24 + 1;
			}
		} 
		else {
			month = 1;
			day   = 1;
		}
	
		hour = k - (day - 1) * 24 - (month - 1) * 24;
		min  = atoi(stringTime.Mid(i + 1, j - i));
		sec  = atoi(stringTime.Mid(j + 1, stringTime.GetLength() - j - 1));
	}

	return CTime(year, month, day, hour, min, sec);
}

CString CPopupTimeCtrl::TimeToString(const CTime &stringTime)
{
	UINT    hour, min, sec;
	CString Temp;

	if (2037 == stringTime.GetYear()) 
		Temp = "INFINITY";
	else {
		hour = (stringTime.GetMonth() - 1) * 24 + (stringTime.GetDay() - 1) * 24 + 
			stringTime.GetHour();
		min  = stringTime.GetMinute();
		sec  = stringTime.GetSecond();
		Temp.Format("%d:%d:%d", hour, min, sec);
	}

	return Temp;
}

BOOL CPopupTimeCtrl::PreTranslateMessage(MSG* pMsg) 
{
	ASSERT(NULL != pMsg);

	if (WM_KEYDOWN == pMsg->message &&
	   (VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam))
	{
		::TranslateMessage(pMsg);
		::DispatchMessage(pMsg);
		return TRUE;                           
	}

	return CDateTimeCtrl::PreTranslateMessage(pMsg);
}

void CPopupTimeCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (!m_bIsEmbeddedIntoListCtrl) {
		CDateTimeCtrl::OnChar(nChar, nRepCnt, nFlags);
		return;
	}

	CWnd *Parent = GetParent();
	ASSERT_VALID(Parent);
	
	if (VK_ESCAPE == nChar || VK_RETURN == nChar) {
		if (VK_ESCAPE == nChar)
			m_bESC = true;
		Parent->SetFocus();

		return;
	}

	CDateTimeCtrl::OnChar(nChar, nRepCnt, nFlags);

	// Get text extent
	CString str;
	GetWindowText(str);

	CWindowDC dc(this);
	CFont *pFont = Parent->GetFont();
	ASSERT_VALID(pFont);
	CFont *pFontDC = dc.SelectObject(pFont);
	ASSERT_VALID(pFontDC);
	CSize size = dc.GetTextExtent(str);
	dc.SelectObject(pFontDC);

	size.cx += 5;			   	// add some extra buffer	// Get client rect
	CRect rect, parentrect;	
	GetClientRect(&rect);
	Parent->GetClientRect(&parentrect);
	// Transform rect to parent coordinates	ClientToScreen( &rect );
	Parent->ScreenToClient(&rect);

	// Check whether control needs to be resized
	// and whether there is space to grow
	if (size.cx > rect.Width())	{
		if (size.cx + rect.left < parentrect.right)
			rect.right = rect.left + size.cx;
		else	
			rect.right = parentrect.right;

		MoveWindow(&rect);
	}	
}

void CPopupTimeCtrl::OnDatetimechange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ASSERT(NULL != pNMHDR);
	ASSERT(NULL != pResult);

	NMDATETIMECHANGE *pChange = reinterpret_cast<NMDATETIMECHANGE *>(pNMHDR);
	ASSERT(NULL != pChange);

    if (GDT_NONE == pChange->dwFlags) {
		m_bInfinity = true;
        SetFormat("' INFINITY'");	
	}
    else {
		if (m_bInfinity) 
			SetFormat("XXX':'mm':'ss");
		else if (m_bClick) {
			m_bClick  = false;
			m_Hours += m_Delta;
			RedrawWindow();
		}

		m_bInfinity = false;
	}

	*pResult = 0;
}

void CPopupTimeCtrl::OnFormatquery(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ASSERT(NULL != pNMHDR);
	ASSERT(NULL != pResult);

	NMDATETIMEFORMATQUERY *pDTFQuery = reinterpret_cast<NMDATETIMEFORMATQUERY *>(
		pNMHDR);
	ASSERT(NULL != pDTFQuery);
	ASSERT(NULL != pDTFQuery->pszFormat);

	CDC *pDC = GetDC();
	ASSERT_VALID(pDC);
	CFont *pFont= GetFont();
	ASSERT_VALID(pFont);

	// Commented by Stefan Mihaila
	/*if(!pFont)
	{
		 pFont->CreateStockObject(DEFAULT_GUI_FONT);
	}*/

	CFont *pOrigFont = pDC->SelectObject(pFont);
	ASSERT_VALID(pOrigFont);
	
	if (0 == lstrcmp("XXX", pDTFQuery->pszFormat))
		::GetTextExtentPoint32(pDC->m_hDC, "999", 3, &pDTFQuery->szMax);
	
	pDC->SelectObject(pOrigFont);
	ReleaseDC(pDC);

	*pResult = 0;
}

void CPopupTimeCtrl::OnFormat(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ASSERT(NULL != pNMHDR);
	ASSERT(NULL != pResult);

	NMDATETIMEFORMAT *pDTFormat = reinterpret_cast<NMDATETIMEFORMAT *>(pNMHDR);
	ASSERT(NULL != pDTFormat);
	ASSERT(NULL != pDTFormat->pszFormat);

	if (0 == lstrcmp("XXX", pDTFormat->pszFormat)) {
		char Zero[4];

		if (m_Hours < 10 )
			strcpy(Zero, "00");
		else if (m_Hours < 100)
			strcpy(Zero, "0");
		else if (m_Hours < 1000)
			strcpy(Zero, "");	
		else
			strcpy(Zero, "");

		wsprintf(pDTFormat->szDisplay, "%s%d", Zero, m_Hours);    
	}
	
	*pResult = 0;
}

void CPopupTimeCtrl::OnWmkeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ASSERT(NULL != pNMHDR);
	ASSERT(NULL != pResult);

	NMDATETIMEWMKEYDOWN *pDTKeystroke = reinterpret_cast<NMDATETIMEWMKEYDOWN *>(
		pNMHDR);
	ASSERT(NULL != pDTKeystroke);
	ASSERT(NULL != pDTKeystroke->pszFormat);

	if (0 != lstrcmp(pDTKeystroke->pszFormat, "XXX")) {
		*pResult = 0;
		return;
	}

	CWnd *NotifyWnd = GetParent();
	ASSERT_VALID(NotifyWnd);
	if (m_bIsEmbeddedIntoListCtrl) {
		NotifyWnd = NotifyWnd->GetParent();
		ASSERT_VALID(NotifyWnd);
	}

	NMHDR Message;
	Message.code     = DTN_HOURCHANGE;
	Message.hwndFrom = GetSafeHwnd();
	Message.idFrom   = GetDlgCtrlID();

	switch (pDTKeystroke->nVirtKey) {            
		case VK_DOWN:
        case VK_SUBTRACT: 
			if (0 == m_Hours)
				m_Hours = 999;
			else
				--m_Hours;

			NotifyWnd->SendMessage(WM_NOTIFY, NULL, LPARAM(&Message));
			m_KeyCount = 0;
			break;

        case VK_UP: 
		case VK_ADD:
            if (999 == m_Hours)
				m_Hours = 0;
			else
				++m_Hours;
			
			NotifyWnd->SendMessage(WM_NOTIFY, NULL, LPARAM(&Message));
			m_KeyCount = 0;
			break;   

		case VK_HOME:
			m_Hours = 0;
			m_KeyCount = 0;

			NotifyWnd->SendMessage(WM_NOTIFY, NULL, LPARAM(&Message));
			break;

		case VK_END:
			m_Hours = 999;
			m_KeyCount = 0;
			
			NotifyWnd->SendMessage(WM_NOTIFY, NULL, LPARAM(&Message));
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			switch (m_KeyCount) {
				case 0:
					m_Hours = int(pDTKeystroke->nVirtKey) - int('0');
					++m_KeyCount;
					break;

				case 1:
					m_Hours = m_Hours * 10 + int(pDTKeystroke->nVirtKey) - int('0');
					++m_KeyCount;
					break;

				case 2:
					m_Hours = m_Hours * 10 + int(pDTKeystroke->nVirtKey) - int('0');
					m_KeyCount = 0;
					break;

				default:
					ASSERT(false);
			}

			NotifyWnd->SendMessage(WM_NOTIFY, NULL, LPARAM(&Message));
			break;
	}

	RedrawWindow();	

	*pResult = 0;
}

int CPopupTimeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	ASSERT(NULL != lpCreateStruct);

	if (-1 == CDateTimeCtrl::OnCreate(lpCreateStruct))
		return -1;
	
	if (m_bIsEmbeddedIntoListCtrl)
		SetTime(StringToTime(m_InitText));
	
	return 0;
}
