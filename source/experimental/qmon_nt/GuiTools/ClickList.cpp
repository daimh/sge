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
// ClickList.cpp : implementation file
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "resource.h"
#include "ClickList.h"
#include "popuptimectrl.h"
#include "Popupedit.h"
#include "InPlaceEdit.h"
#include "InPlaceList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClickList

IMPLEMENT_DYNAMIC (CClickList,CListCtrl)

CClickList::CClickList()
{
	m_HighlightType	= HT_ROW;
	m_AllowAddRemoveRowsByKeyboard = false;
	m_LastEditedRow = m_LastEditedCol = -1;
}

CClickList::~CClickList()
{
}

CHighlightType CClickList::SetHighlightType(CHighlightType Type)
{
	ASSERT(HT_NORMAL <= Type && Type <= HT_ROW);

	CHighlightType OldType = m_HighlightType;
	m_HighlightType = Type;
	Invalidate();
	
	return OldType;
}

void CClickList::SetEditableColumns(int Row, const CEditableColList &EditableColumns)
{
	ASSERT(0 <= Row && Row < GetItemCount());

	CRowInfo *RowInfo = new CRowInfo(EditableColumns);
	ASSERT(NULL != RowInfo);

	SetItemData(Row, LPARAM(RowInfo));
}

bool CClickList::GetLastEditedRowCol(int &Row, int &Col)
{
	Row = m_LastEditedRow;
	Col = m_LastEditedCol;

	return -1 != Row && -1 != Col;
}

void CClickList::AllowAddRemoveRowsByKeyboard(bool Allow /* = true */)
{
	m_AllowAddRemoveRowsByKeyboard = Allow;
}

BEGIN_MESSAGE_MAP(CClickList, CListCtrl)
	//{{AFX_MSG_MAP(CClickList)
	ON_WM_LBUTTONDBLCLK()
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndlabeledit)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_CHAR()
	ON_WM_PAINT()
	ON_NOTIFY_REFLECT_EX(LVN_DELETEITEM, OnDeleteitem)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClickList message handlers

void CClickList::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CListCtrl::OnLButtonDblClk(nFlags, point);

	int Col = 0, Row = HitTestEx(point, Col);

	CHeaderCtrl *pHeader = const_cast<CClickList *>(this)->GetHeaderCtrl();
	if (NULL == pHeader || Row < 0 || Col >= pHeader->GetItemCount())
		return;

	CRowInfo *RowInfo = reinterpret_cast<CRowInfo *>(GetItemData(Row));
	if (NULL != RowInfo)
		for (CEditableColList::iterator it = RowInfo->EditableColList.begin();
			it != RowInfo->EditableColList.end(); ++it)
				if (Col == it->ColNumber) {
					m_LastEditedRow = m_LastEditedCol = -1;
					ShowEditControl(Row, Col, it->DataType);
					return;
				}
}

int CClickList::HitTestEx(const CPoint &Point, int &Col) const
{
	Col = 0;

    // Make sure that the ListView is in LVS_REPORT
    if (LVS_REPORT != (GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK))
		return HitTest(Point, NULL);

    // Get the top and bottom row visible
    int row = GetTopIndex();
    int bottom = row + GetCountPerPage();
    if (bottom > GetItemCount())
		bottom = GetItemCount();
    
    // Get the number of columns
    CHeaderCtrl *pHeader = const_cast<CClickList *>(this)->GetHeaderCtrl();
	if (NULL == pHeader)
		return -1;

    // Loop through the visible rows
	CRect rect;
	int nColumnCount = pHeader->GetItemCount(), colnum, colwidth;
    for (; row <= bottom; row++) {
		// Get bounding rect of item and check whether point falls in it.
		GetItemRect(row, &rect, LVIR_BOUNDS);

		if (rect.PtInRect(Point)) // Now find the column
			for (colnum = 0; colnum < nColumnCount; colnum++) {
				colwidth = GetColumnWidth(colnum);
				if (Point.x >= rect.left && Point.x <= rect.left + colwidth) {
					Col = colnum;
					return row;
				}
				rect.left += colwidth;
			}
    }

    return -1;
}

CRect CClickList::GetInPlaceRect(int Row, int Col)
{
	static const int MIN_COL_WIDTH = 10;

	// Make sure that the item is visible
	if (!EnsureVisible(Row, TRUE)) 
		return CRect(0, 0, 0, 0);

	// Make sure that Col is valid 
	CHeaderCtrl *pHeader = GetHeaderCtrl();
	if (NULL == pHeader)
		return CRect(0, 0, 0, 0);

	int nColumnCount = pHeader->GetItemCount();
	if (Col >= nColumnCount || GetColumnWidth(Col) < MIN_COL_WIDTH) 
		return CRect(0, 0, 0, 0);

	// Get the column offset
	int offset = 0;
	for (int i = 0; i < Col; i++)
		offset += GetColumnWidth(i);

	CRect rect;
	GetItemRect(Row, &rect, LVIR_BOUNDS);
	rect.bottom += 5;

	// Now scroll if we need to expose the column
	CRect rcClient;
	GetClientRect(&rcClient);
	if (offset + rect.left < 0 || offset + rect.left > rcClient.right) {
		CSize size;
		size.cx = offset + rect.left;
		size.cy = 0;
		Scroll(size);
		rect.left -= size.cx;
	}

	rect.left   += offset;
	rect.right   = rect.left + GetColumnWidth(Col);

	if (rect.right > rcClient.right) 
		rect.right = rcClient.right;

	return rect;
}

CDateTimeCtrl *CClickList::ShowInPlaceTime(int Row, int Col)
{    
	CWnd *Parent = GetParent();
	ASSERT_VALID(Parent);

	CDateTimeCtrl *pTimeCtrl = new CPopupTimeCtrl(Row, Col, GetItemText(Row, Col));
	ASSERT_VALID(pTimeCtrl);

	pTimeCtrl->Create(WS_CHILD | WS_VISIBLE | DTS_UPDOWN | DTS_SHOWNONE, 
		GetInPlaceRect(Row, Col), Parent, IDL_INPLACE_TIME);

	pTimeCtrl->SetParent(this);
	pTimeCtrl->SetFormat("XXX':'mm':'ss");
	pTimeCtrl->SetFocus();

	return pTimeCtrl;
}

CEdit *CClickList::ShowInPlaceString(int Row, int Col)
{    
	CInPlaceEdit *pStringEdit = new CInPlaceEdit(Row, Col, GetItemText(Row, Col));
	ASSERT_VALID(pStringEdit);

	pStringEdit->Create(WS_BORDER | WS_CHILD | WS_VISIBLE, GetInPlaceRect(Row, Col), 
		this, IDL_INPLACE_STRING);
	pStringEdit->SetFocus();

	return pStringEdit;
}

CEdit *CClickList::ShowInPlaceMemory(int Row, int Col)
{    
	CEdit *pMemoryEdit = new CPopupEdit(Row, Col, GetItemText(Row, Col));
	ASSERT_VALID(pMemoryEdit);

	pMemoryEdit->Create(WS_BORDER | WS_CHILD | WS_VISIBLE, GetInPlaceRect(Row, Col), 
		this, IDL_INPLACE_MEMORY);
	pMemoryEdit->SetFocus();

	return pMemoryEdit;
}

CListBox *CClickList::ShowInPlaceList(int Row, int Col)
{
	CListBox *pListBox = new CInPlaceList(Row, Col, GetItemText(Row, Col));
	ASSERT_VALID(pListBox);

	CRect Rect = GetInPlaceRect(Row, Col);
	Rect.bottom += 75;
	pListBox->Create(WS_BORDER | WS_CHILD | WS_VISIBLE, Rect, 
		this, IDL_INPLACE_LIST);
	pListBox->SetFocus();

	return pListBox;
}

void CClickList::ShowEditControl(int Row, int Col, CDataType DataType)
{
	switch (DataType) {
		case DT_INTEGER:        // fall through
		case DT_STRING:
			ShowInPlaceString(Row, Col);
			break;

		case DT_TIME:
			{
				CRect rc;
				GetWindowRect(&rc);

				WINDOWPOS wp = {
					m_hWnd,			// hwnd
					NULL,			// hwndInsertAfter
					0,				// x
					0,				// y
					rc.Width(),		// cx
					rc.Height(),	// cy
					SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER
									// flags
				};
				SendMessage(WM_WINDOWPOSCHANGED, 0, LPARAM(&wp));
			}
			ShowInPlaceTime(Row, Col);
			break;

		case DT_MEMORY:
			ShowInPlaceMemory(Row, Col);
			break;

		case DT_LIST:
			ShowInPlaceList(Row, Col);
			break;

		case DT_BOOLEAN:
		case DT_CSTRING:
		case DT_HOST:
			// Stefan Mihaila: TODO.
			// Temporary, use 'ShowInPlaceString()'
			ShowInPlaceString(Row, Col);
			break;

		default:
			ASSERT(false);
			//TypeStr.Format("Please define it (valtype = %d)", it->valtype);
	}
}

void CClickList::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	static const COLORREF 
		COLOR_1 = RGB(239, 246, 249),
		COLOR_2 = RGB(209, 226, 239);

	ASSERT(NULL != lpDrawItemStruct);
	ASSERT(NULL != lpDrawItemStruct->hDC);

	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	ASSERT_VALID(pDC);
	// The CWinApp idle-time handler will automatically call
	// CDC::DeleteTempMap to delete any temporary CDC objects
	// created by FromHandle

	CRect rcItem(lpDrawItemStruct->rcItem);	
	int nItem = lpDrawItemStruct->itemID;
	int nSavedDC = pDC->SaveDC();

	// Get item image and state info
	LV_ITEM lvi;
	lvi.mask      = LVIF_IMAGE | LVIF_STATE;	
	lvi.iItem     = nItem;	
	lvi.iSubItem  = 0;
	lvi.stateMask = -1;
	GetItem(&lvi);

	// Should the item be highlighted
	bool bHighlight = LVIS_DROPHILITED == (lvi.state & LVIS_DROPHILITED) || 
		LVIS_SELECTED == (lvi.state & LVIS_SELECTED) && (this == GetFocus() || 
		LVS_SHOWSELALWAYS == (GetStyle() & LVS_SHOWSELALWAYS));

	// Get rectangles for drawing	
	CRect rcBounds, rcLabel, rcIcon;
	GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
	GetItemRect(nItem, rcLabel,  LVIR_LABEL);
	GetItemRect(nItem, rcIcon,   LVIR_ICON);
	CRect rcCol(rcBounds);
	CString sLabel = GetItemText(nItem, 0);

	// Labels are offset by a certain amount  
	// This offset is related to the width of a space character
	int offset = pDC->GetTextExtent(_T(" "), 1 ).cx * 2;

	CRect rcHighlight;
	CRect rcWnd;
	int   nExt;

	switch (m_HighlightType) {
		case HT_NORMAL: 
			nExt = pDC->GetOutputTextExtent(sLabel).cx + offset;
			rcHighlight = rcLabel;
			if (rcLabel.left + nExt < rcLabel.right)
				rcHighlight.right = rcLabel.left + nExt;
			break;	
		
		case HT_ALLCOLUMNS:
			rcHighlight = rcBounds;
			rcHighlight.left = rcLabel.left;
			break;

		case HT_ROW:
			GetClientRect(&rcWnd);
			rcHighlight       = rcBounds;
			rcHighlight.left  = rcLabel.left;
			rcHighlight.right = rcWnd.right;
			break;

		default:
			ASSERT(false);
	}

	// Draw the background color
	if (bHighlight)	{
		pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		pDC->FillRect(rcHighlight, &CBrush(::GetSysColor(COLOR_HIGHLIGHT)));
	} 
	else {
		CRect rcClient, rcRow = rcItem;
		GetClientRect(&rcClient);
		rcRow.right = rcClient.right;
		pDC->FillRect(rcRow, &CBrush(nItem % 2 ?  COLOR_1 : COLOR_2));
		//pDC->FillRect(rcHighlight, &CBrush(::GetSysColor(COLOR_WINDOW)));	
	}

	// Set clip region
	rcCol.right = rcCol.left + GetColumnWidth(0);
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rcCol);
	pDC->SelectClipRgn(&rgn);
	rgn.DeleteObject();

	// Draw state icon
	CImageList *pImageList;	
	if (LVIS_STATEIMAGEMASK == (lvi.state & LVIS_STATEIMAGEMASK)) {
		pImageList = GetImageList(LVSIL_STATE);
		if (NULL != pImageList) {
			int nImage = ((lvi.state & LVIS_STATEIMAGEMASK) >> 12) - 1;
			pImageList->Draw(pDC, nImage, CPoint(rcCol.left, rcCol.top), 
				ILD_TRANSPARENT);
		}
	}	

	// Draw normal and overlay icon	
	pImageList = GetImageList(LVSIL_SMALL);
	if (NULL != pImageList)	{
		UINT nOvlImageMask = (lvi.state & LVIS_OVERLAYMASK);
		pImageList->Draw(pDC, lvi.iImage, CPoint(rcIcon.left, rcIcon.top), 
			(bHighlight ? ILD_BLEND50 : 0) | ILD_TRANSPARENT | nOvlImageMask);
	}		

	// Draw item label - Column 0
	rcLabel.left  += offset / 2;
	rcLabel.right -= offset;
	pDC->DrawText(sLabel, -1, rcLabel, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | 
		DT_NOCLIP | DT_VCENTER | DT_END_ELLIPSIS);

	// Draw labels for remaining columns
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH;

	// Highlight only first column
	if (HT_NORMAL == m_HighlightType) {
		pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
		pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));
	}	

	rcBounds.right = rcHighlight.right > rcBounds.right ? rcHighlight.right 
		: rcBounds.right;
	rgn.CreateRectRgnIndirect(&rcBounds);
	pDC->SelectClipRgn(&rgn);				   
	rgn.DeleteObject();

	for (int nColumn = 1; GetColumn(nColumn, &lvc); nColumn++)	{
		rcCol.left   = rcCol.right;
		rcCol.right += lvc.cx;

		// Draw the background if needed
		if (HT_NORMAL == m_HighlightType)
			pDC->FillRect(rcCol, &CBrush(::GetSysColor(COLOR_WINDOW)));

		sLabel = GetItemText(nItem, nColumn);
		if (0 == sLabel.GetLength())
			continue;

		// Get the text justification
		UINT nJustify;
		switch (lvc.fmt & LVCFMT_JUSTIFYMASK) {
			case LVCFMT_RIGHT:
				nJustify = DT_RIGHT;
				break;

			case LVCFMT_CENTER:
				nJustify = DT_CENTER;
				break;

			default:
				 nJustify = DT_LEFT;
				break;
		}
		
		rcLabel        = rcCol;
		rcLabel.left  += offset;
		rcLabel.right -= offset;
		pDC->DrawText(sLabel, -1, rcLabel, nJustify | DT_SINGLELINE | DT_NOPREFIX |
			DT_VCENTER | DT_END_ELLIPSIS);
	}

	// Draw focus rectangle if item has focus
	if ((lvi.state & LVIS_FOCUSED) && this == GetFocus())
		pDC->DrawFocusRect(rcHighlight);

	// Restore dc
	pDC->RestoreDC(nSavedDC);
}

void CClickList::RepaintSelectedItems()
{
	CRect rcBounds, rcLabel;

	// Invalidate focused item so it can repaint 
	int nItem = GetNextItem(-1, LVNI_FOCUSED);
	if (-1 != nItem)	{
		GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
		GetItemRect(nItem, rcLabel,  LVIR_LABEL);
		rcBounds.left = rcLabel.left;
		InvalidateRect(rcBounds, FALSE);
	}

	// Invalidate selected items depending on LVS_SHOWSELALWAYS
	if (LVS_SHOWSELALWAYS != (GetStyle() & LVS_SHOWSELALWAYS))
		for (nItem = GetNextItem(-1, LVNI_SELECTED); -1 != nItem; nItem = GetNextItem(nItem, LVNI_SELECTED))	
		{
			GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
			GetItemRect(nItem, rcLabel,  LVIR_LABEL);
			rcBounds.left = rcLabel.left;
			InvalidateRect(rcBounds, FALSE);
		}
	
	UpdateWindow();
}

void CClickList::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ASSERT(NULL != pNMHDR);
	ASSERT(NULL != pResult);
	
	LV_DISPINFO *pDispInfo = reinterpret_cast<LV_DISPINFO *>(pNMHDR);
	ASSERT(NULL != pDispInfo);

	SetItemState(pDispInfo->item.iItem, 0, LVNI_FOCUSED | LVNI_SELECTED);

    CString EditedVal = pDispInfo->item.pszText;
    if (!EditedVal.IsEmpty()) {
		ASSERT(-1 == m_LastEditedRow);
		ASSERT(-1 == m_LastEditedCol);
		m_LastEditedRow = pDispInfo->item.iItem;
		m_LastEditedCol = pDispInfo->item.iSubItem;

		SetItemText(m_LastEditedRow, m_LastEditedCol, EditedVal);

		CWnd *Parent = GetParent();
		ASSERT_VALID(Parent);

		if (IDL_INPLACE_TIME != pNMHDR->idFrom)
			Parent->SendMessage(WM_COMMAND, MAKEWPARAM(pNMHDR->idFrom, EN_CHANGE),
				LPARAM(pNMHDR->hwndFrom));
		else {
			NMHDR Message;
			Message.code     = DTN_HOURCHANGE;
			Message.hwndFrom = pNMHDR->hwndFrom;
			Message.idFrom   = pNMHDR->idFrom;

			Parent->SendMessage(WM_NOTIFY, NULL, LPARAM(&Message));
		}
	}
    
    *pResult = FALSE;
}

void CClickList::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (this != GetFocus())
		SetFocus();

	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CClickList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (this != GetFocus()) 
		SetFocus();

	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CClickList::PreTranslateMessage(MSG *pMsg)
{
	ASSERT(NULL != pMsg);

	if (WM_KEYDOWN != pMsg->message || !m_AllowAddRemoveRowsByKeyboard)
		return CListCtrl::PreTranslateMessage(pMsg);

	if (VK_RETURN == pMsg->wParam || VK_DELETE == pMsg->wParam || 
		VK_INSERT == pMsg->wParam || VK_ESCAPE == pMsg->wParam || GetKeyState(VK_CONTROL)) 
	{
		if (VK_DELETE == pMsg->wParam) {
			POSITION pos = GetFirstSelectedItemPosition();
			if (NULL != pos) {
				// First of all, notify the parent, to be prepared to
				// interpret the deletion properly
				CWnd *Parent = GetParent();
				ASSERT_VALID(Parent);
				Parent->SendMessage(WM_CHAR, pMsg->wParam, pMsg->lParam);

				int nItem = GetNextSelectedItem(pos); 
				DeleteItem(nItem);
				int ItemCount = GetItemCount();
				if (ItemCount > 0)
					SetItemState(nItem < ItemCount ? nItem : ItemCount - 1, 
						LVNI_FOCUSED | LVNI_SELECTED, LVNI_FOCUSED | LVNI_SELECTED);
				RedrawWindow();
			}
		}

		::TranslateMessage(pMsg);
		::DispatchMessage(pMsg);
		return TRUE;
	}
	
	return CListCtrl::PreTranslateMessage(pMsg);
}

void CClickList::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	POSITION pos;

	if (VK_RETURN != nChar || NULL == (pos = GetFirstSelectedItemPosition())) {
		CListCtrl::OnChar(nChar, nRepCnt, nFlags);
		return;
	}

	ASSERT(NULL != pos);
	int Row = GetNextSelectedItem(pos);

	CRowInfo *RowInfo = reinterpret_cast<CRowInfo *>(GetItemData(Row));
	if (NULL != RowInfo)
		for (CEditableColList::iterator it = RowInfo->EditableColList.begin();
			it != RowInfo->EditableColList.end(); ++it)
				if (1 == it->ColNumber) {
					ShowEditControl(Row, 1, it->DataType);
					return;
				}

	CListCtrl::OnChar(nChar, nRepCnt, nFlags);
}

void CClickList::OnPaint() 
{
	// in full row select mode, we need to extend the clipping region
	// so we can paint a selection all the way to the right
	if (HT_ROW == m_HighlightType && LVS_REPORT == (GetStyle() & LVS_TYPEMASK)) {
		CRect rcBounds, rcClient;
		GetItemRect(0, rcBounds, LVIR_BOUNDS);
		GetClientRect(&rcClient);

		if (rcBounds.right < rcClient.right) {
			CPaintDC dc(this);
			CRect rcClip;
			dc.GetClipBox(rcClip);
			rcClip.left = min(rcBounds.right - 1, rcClip.left);
			rcClip.right = rcClient.right;
			InvalidateRect(rcClip, FALSE);
		}	
	}

	CListCtrl::OnPaint();
}

BOOL CClickList::OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW *pNMListView = reinterpret_cast<NM_LISTVIEW *>(pNMHDR);
	ASSERT(NULL != pNMListView);
	ASSERT(NULL != pResult);

	CRowInfo *RowInfo = reinterpret_cast<CRowInfo *>(pNMListView->lParam);
	// Stefan Mihaila:
	// If 'RowInfo' isn't NULL, then the user of 'CClickList' class
	// used the 'editable columns' feature, so this item (that must be deleted)
	// contains proper 'RowInfo' data
	// WARNING: DO NOT put in a LV_ITEM's 'lParam' field anything
	// not NULL, unless it is a CRowInfo pointer (see CListCtrl::InsertItem()
	// and CListCtrl::SetItemData())
	// If you want to use the 'editable columns' feature, for a new inserted
	// row, please make use
	// of CClickList::SetEditableColumns() function instead of a 
	// CListCtrl::SetItemData() call.

	if (NULL != RowInfo) {
		delete RowInfo;
		RowInfo = NULL;
	}
	
	*pResult = 0;

	// Allow the parent to receive the message as well
	return TRUE;
}

BOOL CClickList::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD notificationCode = HIWORD(wParam);
	WORD ItemID           = LOWORD(wParam);

	if (EN_CHANGE == notificationCode)
		switch (ItemID) {
			case IDL_INPLACE_STRING:
			case IDL_INPLACE_LIST:
			case IDL_INPLACE_MEMORY:
				CWnd *Parent = GetParent();
				ASSERT_VALID(Parent);
				Parent->SendMessage(WM_COMMAND, wParam, lParam);

			// >>> Code für neue Felder hier einfügen
		}
	
	return CListCtrl::OnCommand(wParam, lParam);
}
