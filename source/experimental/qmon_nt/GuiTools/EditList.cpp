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
// EditList.cpp : implementation file
//

#include "stdafx.h"
#include "..\GUITools\resource.h"
#include "EditList.h"
#include "InPlaceList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditList
IMPLEMENT_DYNAMIC (CEditList,CListCtrl)

CEditList::CEditList()
{
}

CEditList::~CEditList()
{
}


BEGIN_MESSAGE_MAP(CEditList, CListCtrl)
	//{{AFX_MSG_MAP(CEditList)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditList message handlers


int CEditList::HitTestEx(CPoint &point, int* col) const
{
        int colnum = 0;
        int row = HitTest( point, NULL );
        
        if( col ) *col = 0;

        if( (GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT )
                return row;

        row = GetTopIndex();
        int bottom = row + GetCountPerPage();
        if( bottom > GetItemCount() )
                bottom = GetItemCount();
        
        CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
        int nColumnCount = pHeader->GetItemCount();

        for( ;row <=bottom;row++)
        {
                CRect rect;
                GetItemRect( row, &rect, LVIR_BOUNDS );
                if( rect.PtInRect(point) ) {
                        for( colnum = 0; colnum < nColumnCount; colnum++ ) {
                                int colwidth = GetColumnWidth(colnum);
                                if( point.x >= rect.left && point.x <= (rect.left + colwidth ) ) {
                                        if( col ) *col = colnum;
                                        return row;
                                }
                                rect.left += colwidth;
                        }
                }
        }
        return -1;
}

CRect CEditList::GetInPlaceRect(int nItem, int nCol)
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
		rect.bottom+= 80;
        if( rect.right > rcClient.right) rect.right = rcClient.right;

		return rect;
}

void CEditList::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	int row = 1,col =0;
	UINT flags = LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON;
	row = HitTestEx(point,&col);

	if (row > -1)
		if (col==2 || col==14 || col==18 || col==13 || col==21 ||  col==23 || col==24 ) 
		{
			m_item.mask =  LVIF_PARAM;
			m_item.iItem = row;
			m_item.iSubItem = col;
			GetItem(&m_item);
			ShowListValues();
		}

	CListCtrl::OnLButtonDblClk(nFlags, point);
}


void CEditList::ShowListValues()
{
	UINT type = (UINT)m_item.lParam;
	CString ListStr;
	ListStr = GetItemText(m_item.iItem,m_item.iSubItem);
/*
	char buf1[32];
	char buf2[32];
	itoa(m_item.iItem,buf1,10);
	itoa(m_item.iSubItem,buf2,10);
	ListStr = ListStr + "  Item=" + CString(buf1) + "   SubItem=" + CString(buf2);
*/
	CRect rect = GetInPlaceRect(m_item.iItem,m_item.iSubItem);
	DWORD dwStyle = WS_BORDER | WS_CHILD|WS_VISIBLE;
	CInPlaceList* pList = new CInPlaceList(m_item.iItem, m_item.iSubItem, ListStr);
	pList->Create( dwStyle, rect, this, IDL_INPLACE_LIST);
	pList->SetFocus();
}

void CEditList::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if( GetFocus() != this ) SetFocus();
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CEditList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if( GetFocus() != this ) SetFocus();	
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CEditList::OnSetFocus(CWnd* pOldWnd) 
{
	CListCtrl::OnSetFocus(pOldWnd);
	
	// TODO: Add your message handler code here
	
}

void CEditList::OnKillFocus(CWnd* pNewWnd) 
{
	CListCtrl::OnKillFocus(pNewWnd);
	
	// TODO: Add your message handler code here
	
}
