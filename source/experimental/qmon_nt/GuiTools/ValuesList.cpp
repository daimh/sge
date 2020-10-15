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
// ValuesList.cpp : implementation file
//

#include "stdafx.h"
#include "ValuesList.h"
#include "InPlaceEdit.h"
#include "..\GUITools\resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CValuesList

IMPLEMENT_DYNAMIC (CValuesList,CListCtrl)

CValuesList::CValuesList()
{
//	m_nHighlight = HIGHLIGHT_ALLCOLUMNS;
	m_nHighlight = HIGHLIGHT_ROW;
}

CValuesList::~CValuesList()
{
}


BEGIN_MESSAGE_MAP(CValuesList, CListCtrl)
	//{{AFX_MSG_MAP(CValuesList)
	ON_WM_LBUTTONDBLCLK()
	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndlabeledit)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_CHAR()
	ON_WM_PAINT()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CValuesList message handlers

void CValuesList::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	int row = 1,col =0;
	UINT flags = LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON;
	row = HitTestEx(point,&col);
	if (row > -1)
	{
		m_item.mask =  LVIF_PARAM;
		m_item.iItem = row;
		m_item.iSubItem = col;
		GetItem(&m_item);
	}
	ShowEditControl();
	CListCtrl::OnLButtonDblClk(nFlags, point);
}


int CValuesList::HitTestEx(CPoint &point, int* col) const
{
        int colnum = 0;
        int row = HitTest( point, NULL );
        
        if( col ) *col = 0;

        // Make sure that the ListView is in LVS_REPORT
        if( (GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT )
                return row;

        // Get the top and bottom row visible
        row = GetTopIndex();
        int bottom = row + GetCountPerPage();
        if( bottom > GetItemCount() )
                bottom = GetItemCount();
        
        // Get the number of columns
        CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
        int nColumnCount = pHeader->GetItemCount();

        // Loop through the visible rows
        for( ;row <=bottom;row++)
        {
                // Get bounding rect of item and check whether point falls in it.
                CRect rect;
                GetItemRect( row, &rect, LVIR_BOUNDS );
                if( rect.PtInRect(point) )
                {
                        // Now find the column
                        for( colnum = 0; colnum < nColumnCount; colnum++ )
                        {
                                int colwidth = GetColumnWidth(colnum);
                                if( point.x >= rect.left 
                                        && point.x <= (rect.left + colwidth ) )
                                {
                                        if( col ) *col = colnum;
                                        return row;
                                }
                                rect.left += colwidth;
                        }
                }
        }
        return -1;
}

CRect CValuesList::GetInPlaceRect(int nItem, int nCol)
{
		// Make sure that the item is visible
		if( !EnsureVisible( nItem, TRUE ) ) return NULL;

        // Make sure that nCol is valid 
        CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
        int nColumnCount = pHeader->GetItemCount();
        if( nCol >= nColumnCount || GetColumnWidth(nCol) < 10 ) 
                return NULL;

        // Get the column offset
        int offset = 0;
        for( int i = 0; i < nCol; i++ )
                offset += GetColumnWidth( i );

        CRect rect;
        GetItemRect( nItem, &rect, LVIR_BOUNDS );

        // Now scroll if we need to expose the column
        CRect rcClient;
        GetClientRect( &rcClient );
        if( offset + rect.left < 0 || offset + rect.left > rcClient.right )
        {
                CSize size;
                size.cx = offset + rect.left;
                size.cy = 0;
                Scroll( size );
                rect.left -= size.cx;
        }

        rect.left += offset;
        rect.right = rect.left + GetColumnWidth( nCol );
		rect.bottom+= 5;
        if( rect.right > rcClient.right) rect.right = rcClient.right;

		return rect;
}

CEdit* CValuesList::ShowInPlaceString(int nItem, int nCol)
{    
	CRect rect = GetInPlaceRect(nItem,nCol);
	DWORD dwStyle = WS_BORDER | WS_CHILD|WS_VISIBLE;
	CInPlaceEdit* pEdit = new CInPlaceEdit(nItem, nCol, GetItemText(nItem, nCol) );
	pEdit->Create( dwStyle, rect, this, IDL_INPLACE_STRING);
	pEdit->SetFocus();
	return pEdit;
}


void CValuesList::ShowEditControl()
{
	ShowInPlaceString(m_item.iItem,m_item.iSubItem);
}


void CValuesList::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rcItem(lpDrawItemStruct->rcItem);	
	int nItem = lpDrawItemStruct->itemID;
	CImageList* pImageList;	
	int nSavedDC = pDC->SaveDC();
	// Get item image and state info
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_STATE;	
	lvi.iItem = nItem;	
	lvi.iSubItem = 0;
	lvi.stateMask = 0xFFFF;		
	GetItem(&lvi);
	// Should the item be highlighted
	BOOL bHighlight =((lvi.state & LVIS_DROPHILITED) || ( (lvi.state & LVIS_SELECTED) && ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS) ) ) );
	// Get rectangles for drawing	
	CRect rcBounds, rcLabel, rcIcon;
	GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
	GetItemRect(nItem, rcLabel, LVIR_LABEL);
	GetItemRect(nItem, rcIcon, LVIR_ICON);
	CRect rcCol( rcBounds );
	CString sLabel = GetItemText( nItem, 0 );

	// Labels are offset by a certain amount  
	// This offset is related to the width of a space character
	int offset = pDC->GetTextExtent(_T(" "), 1 ).cx*2;

	CRect rcHighlight;
	CRect rcWnd;
	int nExt;

	switch( m_nHighlight )	{
	case 0: 
		nExt = pDC->GetOutputTextExtent(sLabel).cx + offset;
		rcHighlight = rcLabel;
		if( rcLabel.left + nExt < rcLabel.right )
			rcHighlight.right = rcLabel.left + nExt;
		break;	case 1:
		rcHighlight = rcBounds;
		rcHighlight.left = rcLabel.left;
		break;
	case 2:
		GetClientRect(&rcWnd);
		rcHighlight = rcBounds;
		rcHighlight.left = rcLabel.left;
		rcHighlight.right = rcWnd.right;
		break;
	default:
		rcHighlight = rcLabel;
	}

	// Draw the background color
	if( bHighlight )	{
		pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		pDC->FillRect(rcHighlight, &CBrush(::GetSysColor(COLOR_HIGHLIGHT)));
	} else	{
		CRect rcClient, rcRow = rcItem;
		GetClientRect(&rcClient);
		rcRow.right = rcClient.right;
		pDC->FillRect(rcRow, &CBrush(nItem%2 ?  RGB(239,246,249) : RGB(209,226,239)));
//		pDC->FillRect(rcHighlight, &CBrush(::GetSysColor(COLOR_WINDOW)));	
	}


	// Set clip region
	rcCol.right = rcCol.left + GetColumnWidth(0);
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rcCol);
	pDC->SelectClipRgn(&rgn);
	rgn.DeleteObject();

	// Draw state icon
	if (lvi.state & LVIS_STATEIMAGEMASK)	{
		int nImage = ((lvi.state & LVIS_STATEIMAGEMASK)>>12) - 1;
		pImageList = GetImageList(LVSIL_STATE);
		if (pImageList)		{
			pImageList->Draw(pDC, nImage,
			CPoint(rcCol.left, rcCol.top), ILD_TRANSPARENT);
		}
	}	

	// Draw normal and overlay icon	
	pImageList = GetImageList(LVSIL_SMALL);
	if (pImageList)	{
		UINT nOvlImageMask=lvi.state & LVIS_OVERLAYMASK;
		pImageList->Draw(pDC, lvi.iImage,CPoint(rcIcon.left, rcIcon.top),(bHighlight?ILD_BLEND50:0) | ILD_TRANSPARENT | nOvlImageMask );
	}		

	// Draw item label - Column 0
	rcLabel.left += offset/2;
	rcLabel.right -= offset;
	pDC->DrawText(sLabel,-1,rcLabel,DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER | DT_END_ELLIPSIS);

	// Draw labels for remaining columns
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH;

	// Highlight only first column
	if( m_nHighlight == 0 )	{
		pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
		pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));
	}	

	rcBounds.right = rcHighlight.right > rcBounds.right ? rcHighlight.right : rcBounds.right;
	rgn.CreateRectRgnIndirect(&rcBounds);
	pDC->SelectClipRgn(&rgn);				   
	for(int nColumn = 1; GetColumn(nColumn, &lvc); nColumn++)	{
		rcCol.left = rcCol.right;
		rcCol.right += lvc.cx;
		// Draw the background if needed
		if( m_nHighlight == HIGHLIGHT_NORMAL )
			pDC->FillRect(rcCol, &CBrush(::GetSysColor(COLOR_WINDOW)));
		sLabel = GetItemText(nItem, nColumn);
		if (sLabel.GetLength() == 0)
			continue;
		// Get the text justification
		UINT nJustify = DT_LEFT;

		switch(lvc.fmt & LVCFMT_JUSTIFYMASK) {
		case LVCFMT_RIGHT:
			nJustify = DT_RIGHT;
			break;
		case LVCFMT_CENTER:
		    nJustify = DT_CENTER;
			break;
		default:
			break;
		}
		
		rcLabel = rcCol;
		rcLabel.left += offset;
		rcLabel.right -= offset;
		pDC->DrawText(sLabel, -1, rcLabel, nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS);
	}

	// Draw focus rectangle if item has focus
	if (lvi.state & LVIS_FOCUSED && (GetFocus() == this))
		pDC->DrawFocusRect(rcHighlight);
	// Restore dc
	pDC->RestoreDC( nSavedDC );

}


void CValuesList::RepaintSelectedItems()
{
	CRect rcBounds, rcLabel;
	// Invalidate focused item so it can repaint 
	int nItem = GetNextItem(-1, LVNI_FOCUSED);
	if(nItem != -1)	{
		GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
		GetItemRect(nItem, rcLabel, LVIR_LABEL);
		rcBounds.left = rcLabel.left;
		InvalidateRect(rcBounds, FALSE);
	}
	// Invalidate selected items depending on LVS_SHOWSELALWAYS
	if(!(GetStyle() & LVS_SHOWSELALWAYS))	{
		for(nItem = GetNextItem(-1, LVNI_SELECTED);	nItem != -1; nItem = GetNextItem(nItem, LVNI_SELECTED))	{
			GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
			GetItemRect(nItem, rcLabel, LVIR_LABEL);
			rcBounds.left = rcLabel.left;
			InvalidateRect(rcBounds, FALSE);
		}
	}
	UpdateWindow();
}


void CValuesList::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	
	*pResult = 0;
}

void CValuesList::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	
	LV_DISPINFO* pDispInfo=(LV_DISPINFO*)pNMHDR;
    CString sEdit=pDispInfo->item.pszText;
    if(!sEdit.IsEmpty())    {
      SetItemText(pDispInfo->item.iItem,pDispInfo->item.iSubItem,sEdit);
    }
//    VERIFY(m_LVEdit.UnsubclassWindow()!=NULL);
    SetItemState(pDispInfo->item.iItem,0,LVNI_FOCUSED|LVNI_SELECTED);

    *pResult=false;
}

void CValuesList::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if( GetFocus() != this ) SetFocus();
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CValuesList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if( GetFocus() != this ) SetFocus();
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CValuesList::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_KEYDOWN )	{
		if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DELETE || pMsg->wParam == VK_INSERT || pMsg->wParam == VK_ESCAPE || GetKeyState( VK_CONTROL) ) {
			if(pMsg->wParam == VK_DELETE) {
				POSITION pos = GetFirstSelectedItemPosition();
				if (pos != NULL) {
					int i = GetNextSelectedItem(pos);
					DeleteItem(i);
				}		
			}
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;		
		}
	}
	return CListCtrl::PreTranslateMessage(pMsg);
}

void CValuesList::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == VK_RETURN)	{
		POSITION pos = GetFirstSelectedItemPosition();
		if (pos != NULL) {
			m_item.iItem = GetNextSelectedItem(pos);
			m_item.mask =  LVIF_PARAM;
			m_item.iSubItem = 1;
			GetItem(&m_item);
			ShowEditControl();
		}		
	}

	CListCtrl::OnChar(nChar, nRepCnt, nFlags);
}

void CValuesList::OnPaint() 
{
	// in full row select mode, we need to extend the clipping region
	// so we can paint a selection all the way to the right
	if (m_nHighlight == HIGHLIGHT_ROW && (GetStyle() & LVS_TYPEMASK) == LVS_REPORT ) {
		CRect rcBounds;
		GetItemRect(0, rcBounds, LVIR_BOUNDS);
		CRect rcClient;
		GetClientRect(&rcClient);
		if(rcBounds.right < rcClient.right)		{
			CPaintDC dc(this);
			CRect rcClip;
			dc.GetClipBox(rcClip);
			rcClip.left = min(rcBounds.right-1, rcClip.left);
			rcClip.right = rcClient.right;
			InvalidateRect(rcClip, FALSE);
		}	
	}

	CListCtrl::OnPaint();
}

int CValuesList::SetHighlightType(EHighlight hilite)
{
	int oldhilite = m_nHighlight;
	if( hilite <= HIGHLIGHT_ROW )	{
		m_nHighlight = hilite;
		Invalidate();
	}
	return oldhilite;
}

void CValuesList::OnKillFocus(CWnd* pNewWnd) 
{
	CListCtrl::OnKillFocus(pNewWnd);
/*	
	// check if we are losing focus to label edit box
	if(pNewWnd != NULL && pNewWnd->GetParent() == this)	
		return;
	// repaint items that should change appearance
	if((GetStyle() & LVS_TYPEMASK) == LVS_REPORT)
		RepaintSelectedItems();
*/
}

void CValuesList::OnSetFocus(CWnd* pOldWnd) 
{
	CListCtrl::OnSetFocus(pOldWnd);
/*	
	// check if we are getting focus from label edit box
	if(pOldWnd!=NULL && pOldWnd->GetParent()==this)	
		return;
	// repaint items that should change appearance
	if((GetStyle() & LVS_TYPEMASK)==LVS_REPORT)	
		RepaintSelectedItems();	
*/
}
