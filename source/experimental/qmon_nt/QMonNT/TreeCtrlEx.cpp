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
///////////////////////////////////////////////////////////////////////////////
// 
// CTreeCtrlEx - Multiple selection tree control (MFC 4.2)
// 
// Bendik Engebretsen (c) 1997 
// bendik@techsoft.no
// http://www.techsoft.no/bendik/
//
// Oct 9,  1997 : Fixed problem with notification to parent (TVN_BEGINDRAG)
// Oct 17, 1997 : Fixed bug with deselection when collapsing node with no sibling
// Nov 5,  1997 : Fixed problem with label editing
// Feb 17, 1998 : Fixed another notfication to parent (TVN_KEYDOWN)
// Apr 27, 1998	: Added TVN_SELCHANGED notification to parent in SelectMultiple()
//

#include "stdafx.h"
#include "TreeCtrlEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeCtrlEx

BEGIN_MESSAGE_MAP(CTreeCtrlEx, CTreeCtrl)
	//{{AFX_MSG_MAP(CTreeCtrlEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
//	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT_EX(TVN_ITEMEXPANDING, OnItemexpanding)
	ON_NOTIFY_REFLECT_EX(NM_SETFOCUS, OnSetfocus)
	ON_NOTIFY_REFLECT_EX(NM_KILLFOCUS, OnKillfocus)
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CTreeCtrlEx, CTreeCtrl)

BOOL CTreeCtrlEx::Create(DWORD dwStyle, DWORD dwExStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CreateEx( dwExStyle, WC_TREEVIEW, NULL, dwStyle,
		rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, 
		pParentWnd->GetSafeHwnd(), (HMENU)nID );
}

/////////////////////////////////////////////////////////////////////////////
// CTreeCtrlEx message handlers


///////////////////////////////////////////////////////////////////////////////
// The tree control dosn't support multiple selection. However we can simulate 
// it by taking control of the left mouse click and arrow key press before the
// control gets them, and setting/clearing the TVIS_SELECTED style on the items

void CTreeCtrlEx::OnLButtonDown( UINT nFlags, CPoint point )
{

	UINT nHitFlags = 0;
	HTREEITEM hClickedItem = HitTest( point, &nHitFlags );

	// Must invoke label editing explicitly. The base class OnLButtonDown would normally
	// do this, but we can't call it here because of the multiple selection...
	if( nHitFlags & LVHT_ONITEMLABEL )
		if ( hClickedItem == GetSelectedItem() )
		{
			TRACE("EditLabel\n");
			EditLabel( hClickedItem );
//			return;
		}

	if( nHitFlags & LVHT_ONITEM )
	{
		SetFocus();

		m_hClickedItem = hClickedItem;

		// Is the clicked item already selected ?
		BOOL bIsClickedItemSelected = GetItemState( hClickedItem, TVIS_SELECTED ) & TVIS_SELECTED;

		if ( bIsClickedItemSelected )
		{
			// Maybe user wants to drag/drop multiple items!
			// So, wait until OnLButtonUp() to do the selection stuff. 
			m_bSelectPending=TRUE;
			m_ptClick=point;
		}
		else
		{
			SelectMultiple( hClickedItem, nFlags );
			m_bSelectPending=FALSE;
		}
	}
	else
		CTreeCtrl::OnLButtonDown( nFlags, point );
}

void CTreeCtrlEx::OnLButtonUp( UINT nFlags, CPoint point )
{
	if ( m_bSelectPending )
	{
		// A select has been waiting to be performed here
		SelectMultiple( m_hClickedItem, nFlags );
		m_bSelectPending=FALSE;
	}

	m_hClickedItem=NULL;

	CTreeCtrl::OnLButtonUp( nFlags, point );
}


void CTreeCtrlEx::OnMouseMove( UINT nFlags, CPoint point )
{
	// If there is a select pending, check if cursor has moved so much away from the 
	// down-click point that we should cancel the pending select and initiate
	// a drag/drop operation instead!

	if ( m_hClickedItem )
	{
		CSize sizeMoved = m_ptClick-point;

		if ( abs(sizeMoved.cx) > GetSystemMetrics( SM_CXDRAG ) || abs(sizeMoved.cy) > GetSystemMetrics( SM_CYDRAG ) )
		{
			m_bSelectPending=FALSE;

			// Notify parent that he may begin drag operation
			// Since we have taken over OnLButtonDown(), the default handler doesn't
			// do the normal work when clicking an item, so we must provide our own
			// TVN_BEGINDRAG notification for the parent!

			CWnd* pWnd = GetParent();
			if ( pWnd )
			{
				NM_TREEVIEW tv;

				tv.hdr.hwndFrom = GetSafeHwnd();
				tv.hdr.idFrom = GetWindowLong( GetSafeHwnd(), GWL_ID );
				tv.hdr.code = TVN_BEGINDRAG;

				tv.itemNew.hItem = m_hClickedItem;
				tv.itemNew.state = GetItemState( m_hClickedItem, 0xffffffff );
				tv.itemNew.lParam = GetItemData( m_hClickedItem );

				tv.ptDrag.x = point.x;
				tv.ptDrag.y = point.y;

				pWnd->SendMessage( WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv );
			}

			m_hClickedItem=NULL;
		}
	}

	CTreeCtrl::OnMouseMove( nFlags, point );
}


void CTreeCtrlEx::SelectMultiple( HTREEITEM hClickedItem, UINT nFlags )
{
CWnd *pWnd;

	// Start preparing an NM_TREEVIEW struct to send a notification after selection is done
	NM_TREEVIEW tv;
	HTREEITEM hOldItem = GetSelectedItem();
	if(hOldItem) {
		tv.itemOld.hItem = hOldItem;
		tv.itemOld.state = GetItemState( hOldItem, 0xffffffff );
		tv.itemOld.lParam = GetItemData( hOldItem );
		tv.itemOld.mask = TVIF_HANDLE|TVIF_STATE|TVIF_PARAM;
	} else {
		tv.itemOld.hItem = NULL;
		tv.itemOld.state = NULL;
		tv.itemOld.lParam = NULL;
		tv.itemOld.mask = NULL;
	}

	// Send TVN_SELCHANGING notification: 
	pWnd = GetParent();
	if(pWnd) {
		tv.hdr.hwndFrom = GetSafeHwnd();
		tv.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
		tv.hdr.code = TVN_SELCHANGING;
		tv.itemNew.hItem = hClickedItem;
		tv.itemNew.state = GetItemState(hClickedItem, 0xffffffff);
		tv.itemNew.lParam = GetItemData(hClickedItem);
		tv.itemNew.mask = TVIF_HANDLE|TVIF_STATE|TVIF_PARAM;
		tv.action = TVC_UNKNOWN;
		LRESULT lResult = pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
		if(lResult != 0) {
			// don't allow changing of selection
			return;
		}
	}


	// Action depends on whether the user holds down the Shift or Ctrl key
	if ( nFlags & MK_SHIFT )
	{
		// Select from first selected item to the clicked item
		if ( !m_hFirstSelectedItem )
			m_hFirstSelectedItem=GetSelectedItem();

		SelectItems( m_hFirstSelectedItem, hClickedItem );
	}
	else if ( nFlags & MK_CONTROL )
	{
		// Find which item is currently selected
		HTREEITEM hSelectedItem = GetSelectedItem();

		// Is the clicked item already selected ?
		BOOL bIsClickedItemSelected = GetItemState( hClickedItem, TVIS_SELECTED ) & TVIS_SELECTED;
		BOOL bIsSelectedItemSelected = GetItemState( hSelectedItem, TVIS_SELECTED ) & TVIS_SELECTED;

		// Select the clicked item (this will also deselect the previous one!)
		SelectItem( hClickedItem );

		// If the previously selected item was selected, re-select it
		if ( bIsSelectedItemSelected )
			SetItemState( hSelectedItem, TVIS_SELECTED, TVIS_SELECTED );

		// We want the newly selected item to toggle its selected state,
		// so unselect now if it was already selected before
		if ( bIsClickedItemSelected )
			SetItemState( hClickedItem, 0, TVIS_SELECTED );
		else
			SetItemState( hClickedItem, TVIS_SELECTED, TVIS_SELECTED );

		// Store as first selected item (if not already stored)
		if ( m_hFirstSelectedItem==NULL )
			m_hFirstSelectedItem = hClickedItem;
	}
	else
	{
		// Clear selection of all "multiple selected" items first
		ClearSelection();

		// Then select the clicked item
		SelectItem( hClickedItem );
		SetItemState( hClickedItem, TVIS_SELECTED, TVIS_SELECTED );


/*		SelectItem(hClickedItem);
		SetItemState(hClickedItem, TVIS_SELECTED, TVIS_SELECTED);
		ClearSelectionEx(hClickedItem);
*/
		// Store as first selected item
		m_hFirstSelectedItem = hClickedItem;
	}

	// Notify that selection has changed, by sending a TVN_SELCHANGED notification
	if ( pWnd )
	{
		tv.hdr.hwndFrom = GetSafeHwnd();
		tv.hdr.idFrom = GetWindowLong( GetSafeHwnd(), GWL_ID );
		tv.hdr.code = TVN_SELCHANGED;

		tv.itemNew.hItem = m_hClickedItem;
		tv.itemNew.state = GetItemState( m_hClickedItem, 0xffffffff );
		tv.itemNew.lParam = GetItemData( m_hClickedItem );
		tv.itemNew.mask = TVIF_HANDLE|TVIF_STATE|TVIF_PARAM;

		tv.action = TVC_UNKNOWN;

		pWnd->SendMessage( WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv );
	}
}

void CTreeCtrlEx::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags ) 
{
	if ( nChar==VK_UP || nChar==VK_DOWN )
	{
		if ( !( GetKeyState( VK_SHIFT )&0x8000 ) )
		{
			// User pressed arrow key without holding 'Shift':
			// Clear multiple selection and let base class do normal 
			// selection work!
			ClearSelection( TRUE );
			CTreeCtrl::OnKeyDown( nChar, nRepCnt, nFlags );
			return;
		}

		// Find which item is currently selected
		HTREEITEM hSelectedItem = GetSelectedItem();

		HTREEITEM hNextItem;
		if ( nChar==VK_UP )
			hNextItem = GetPrevVisibleItem( hSelectedItem );
		else
			hNextItem = GetNextVisibleItem( hSelectedItem );

		if ( hNextItem )
		{
			// If the next item is already selected, we assume user is
			// "moving back" in the selection, and thus we should clear 
			// selection on the previous one
			BOOL bSelect = !( GetItemState( hNextItem, TVIS_SELECTED ) & TVIS_SELECTED );

			// Select the next item (this will also deselect the previous one!)
			SelectItem( hNextItem );

			// Now, re-select the previously selected item
			if ( bSelect )
				SetItemState( hSelectedItem, TVIS_SELECTED, TVIS_SELECTED );
		}

		// Since the base class' OnKeyDown() isn't called in this case,
		// we must provide our own TVN_KEYDOWN notification to the parent

		CWnd* pWnd = GetParent();
		if ( pWnd )
		{
			NMTVKEYDOWN tvk;

			tvk.hdr.hwndFrom = GetSafeHwnd();
			tvk.hdr.idFrom = GetWindowLong( GetSafeHwnd(), GWL_ID );
			tvk.hdr.code = TVN_KEYDOWN;

			tvk.wVKey = nChar;
			tvk.flags = 0;

			pWnd->SendMessage( WM_NOTIFY, tvk.hdr.idFrom, (LPARAM)&tvk );
		}
	}
	else
		// Behave normally
		CTreeCtrl::OnKeyDown( nChar, nRepCnt, nFlags );
}


///////////////////////////////////////////////////////////////////////////////
// I want clicking on an item with the right mouse button to select the item,
// but not if there is currently a multiple selection

void CTreeCtrlEx::OnRButtonDown( UINT nFlags, CPoint point )
{
	UINT nHitFlags = 0;
	HTREEITEM hClickedItem = HitTest( point, &nHitFlags );

	m_ptRClick = point;

	if( nHitFlags&LVHT_ONITEM )
		if ( GetSelectedCount()<2 )
			SelectItem( hClickedItem );

	CTreeCtrl::OnRButtonDown( nFlags, point );
}


///////////////////////////////////////////////////////////////////////////////
// Get number of selected items

UINT CTreeCtrlEx::GetSelectedCount() const
{
	// Only visible items should be selected!
	UINT uCount=0;
	for ( HTREEITEM hItem = GetRootItem(); hItem!=NULL; hItem = GetNextVisibleItem( hItem ) )
		if ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			uCount++;

	return uCount;
}


///////////////////////////////////////////////////////////////////////////////
// Helpers to list out selected items. (Use similar to GetFirstVisibleItem(), 
// GetNextVisibleItem() and GetPrevVisibleItem()!)

HTREEITEM CTreeCtrlEx::GetFirstSelectedItem()
{
	for ( HTREEITEM hItem = GetRootItem(); hItem!=NULL; hItem = GetNextVisibleItem( hItem ) )
		if ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}

HTREEITEM CTreeCtrlEx::GetNextSelectedItem( HTREEITEM hItem )
{
	for ( hItem = GetNextVisibleItem( hItem ); hItem!=NULL; hItem = GetNextVisibleItem( hItem ) )
		if ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}

HTREEITEM CTreeCtrlEx::GetPrevSelectedItem( HTREEITEM hItem )
{
	for ( hItem = GetPrevVisibleItem( hItem ); hItem!=NULL; hItem = GetPrevVisibleItem( hItem ) )
		if ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// Select/unselect item without unselecting other items

BOOL CTreeCtrlEx::SelectItemEx(HTREEITEM hItem, BOOL bSelect/*=TRUE*/)
{
	HTREEITEM hSelItem = GetSelectedItem();

	if ( hItem==hSelItem )
	{
		if ( !bSelect )
		{
			SelectItem(NULL);
			return TRUE;
		}

		return FALSE;
	}

	SelectItem( hItem );
	m_hFirstSelectedItem=hItem;

	// Reselect previous "real" selected item which was unselected byt SelectItem()
	if ( hSelItem )
		SetItemState( hSelItem, TVIS_SELECTED, TVIS_SELECTED );

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Select visible items between specified 'from' and 'to' item (including these!)
// If the 'to' item is above the 'from' item, it traverses the tree in reverse 
// direction. Selection on other items is cleared!

BOOL CTreeCtrlEx::SelectItems( HTREEITEM hFromItem, HTREEITEM hToItem )
{
NM_TREEVIEW tv;
CWnd *pWnd;

	pWnd = GetParent();
	// Determine direction of selection 
	// (see what item comes first in the tree)
	HTREEITEM hItem = GetRootItem();

	while ( hItem && hItem!=hFromItem && hItem!=hToItem )
		hItem = GetNextVisibleItem( hItem );

	if ( !hItem )
		return FALSE;	// Items not visible in tree

	BOOL bReverse = hItem==hToItem;

	// "Really" select the 'to' item (which will deselect 
	// the previously selected item)
	SelectItem( hToItem );

	// Go through all visible items again and select/unselect
	hItem = hFromItem;

	BOOL bSelect = FALSE;

	tv.hdr.hwndFrom = GetSafeHwnd();
	tv.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
	tv.hdr.code = TVN_SELCHANGING;
	tv.itemOld.hItem = NULL;
	tv.itemOld.state = NULL;
	tv.itemOld.lParam = NULL;
	tv.itemOld.mask = NULL;

	while ( hItem )
	{
		tv.itemNew.hItem = hItem;
		tv.itemNew.lParam = GetItemData(hItem);
		tv.itemNew.mask = TVIF_HANDLE|TVIF_STATE|TVIF_PARAM;
		tv.action = TVC_UNKNOWN;
		if ( hItem == hFromItem )
			bSelect = TRUE;

		if ( bSelect )
		{
			if ( !( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED ) )
				SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );
		}
		else
		{
			if ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
				SetItemState( hItem, 0, TVIS_SELECTED );
		}

		tv.itemNew.state = GetItemState(hItem, 0xffffffff);
		if(bSelect) {
			pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
			TRACE("HItem: %x\n", hItem);
		}

		if ( hItem == hToItem)
			return TRUE;

		hItem = bReverse ? GetPrevVisibleItem(hItem) : 
							GetNextVisibleItem( hItem );
	}

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// Clear selected state on all visible items

void CTreeCtrlEx::ClearSelection(BOOL bMultiOnly/*=FALSE*/)
{
/*	if ( !bMultiOnly )
		SelectItem( NULL );
*/
	for ( HTREEITEM hItem=GetRootItem(); hItem!=NULL; hItem=GetNextVisibleItem( hItem ) )
		if ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			SetItemState( hItem, 0, TVIS_SELECTED );
}


void CTreeCtrlEx::ClearSelectionEx(HTREEITEM hItemExclude, BOOL bMultiOnly/*=FALSE*/)
{
/*	if (!bMultiOnly)
		SelectItem(NULL);
*/
	for ( HTREEITEM hItem=GetRootItem(); hItem!=NULL; hItem=GetNextVisibleItem( hItem ) )
		if (hItem!= hItemExclude && (GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED))
			SetItemState( hItem, 0, TVIS_SELECTED );
}



///////////////////////////////////////////////////////////////////////////////
// If a node is collapsed, we should clear selections of its child items 

BOOL CTreeCtrlEx::OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	if ( pNMTreeView->action == TVE_COLLAPSE )
	{
		HTREEITEM hItem = GetChildItem( pNMTreeView->itemNew.hItem );

		while ( hItem )
		{
			if ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
				SetItemState( hItem, 0, TVIS_SELECTED );

			// Get the next node: First see if current node has a child
			HTREEITEM hNextItem = GetChildItem( hItem );
			if ( !hNextItem )
			{
				// No child: Get next sibling item
				if ( !( hNextItem = GetNextSiblingItem( hItem ) ) )
				{
					HTREEITEM hParentItem = hItem;
					while ( !hNextItem )
					{
						// No more children: Get parent
						if ( !( hParentItem = GetParentItem( hParentItem ) ) )
							break;

						// Quit when parent is the collapsed node
						// (Don't do anything to siblings of this)
						if ( hParentItem == pNMTreeView->itemNew.hItem )
							break;

						// Get next sibling to parent
						hNextItem = GetNextSiblingItem( hParentItem );
					}

					// Quit when parent is the collapsed node
					if ( hParentItem == pNMTreeView->itemNew.hItem )
						break;
				}
			}

			hItem = hNextItem;
		}
	}
	
	*pResult = 0;
	return FALSE;	// Allow parent to handle this notification as well
}


///////////////////////////////////////////////////////////////////////////////
// Ensure the multiple selected items are drawn correctly when loosing/getting
// the focus

BOOL CTreeCtrlEx::OnSetfocus(NMHDR* pNMHDR, LRESULT* pResult) 
{
	Invalidate();
	*pResult = 0;
	return FALSE;
}

BOOL CTreeCtrlEx::OnKillfocus(NMHDR* pNMHDR, LRESULT* pResult) 
{
	Invalidate();
	*pResult = 0;
	return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// Retreives a tree ctrl item given the item's data

HTREEITEM CTreeCtrlEx::ItemFromData(DWORD dwData, HTREEITEM hStartAtItem/*=NULL*/) const
{
	// Traverse all items in tree control
	HTREEITEM hItem;
	if ( hStartAtItem )
		hItem = hStartAtItem;
	else
		hItem = GetRootItem();

	while ( hItem )
	{
		if ( dwData == (DWORD)GetItemData( hItem ) )
			return hItem;

		// Get first child node
		HTREEITEM hNextItem = GetChildItem( hItem );

		if ( !hNextItem )
		{
			// Get next sibling child
			hNextItem = GetNextSiblingItem( hItem );

			if ( !hNextItem )
			{
				HTREEITEM hParentItem=hItem;
				while ( !hNextItem && hParentItem )
				{
					// No more children: Get next sibling to parent
					hParentItem = GetParentItem( hParentItem );
					hNextItem = GetNextSiblingItem( hParentItem );
				}
			}
		}

		hItem = hNextItem;
	}

	return NULL;
}

HTREEITEM CTreeCtrlEx::GetNextItem( HTREEITEM hItem, UINT nCode )
{
	return CTreeCtrl::GetNextItem( hItem, nCode );
}

// GetNextItem  - Get next item as if outline was completely expanded
// Returns      - The item immediately below the reference item
// hItem        - The reference item

HTREEITEM CTreeCtrlEx::GetNextItem( HTREEITEM hItem )
{
HTREEITEM hti;

	if( ItemHasChildren( hItem ) ) {
		return GetChildItem( hItem );           // return first child
	} else {
		// return next sibling item
		// Go up the tree to find a parent's sibling if needed.
		while( (hti = GetNextSiblingItem( hItem )) == NULL ){
			if( (hItem = GetParentItem( hItem ) ) == NULL )
			{
				return NULL;
			}
		}
	}
	return hti;
}


/////////////////////////////////////////////////////////////////////////////
// Global function to retreive a HTREEITEM from a tree control, given the 
// item's itemdata.

HTREEITEM GetTreeItemFromData(CTreeCtrl& treeCtrl, DWORD dwData, HTREEITEM hStartAtItem /*=NULL*/)
{
	// Traverse from given item (or all items if hFromItem is NULL)
	HTREEITEM hItem;
	if ( hStartAtItem )
		hItem=hStartAtItem;
	else
		hItem = treeCtrl.GetRootItem();

	while ( hItem )
	{
		if ( dwData == (DWORD)treeCtrl.GetItemData( hItem ) )
			return hItem;

		// Get first child node
		HTREEITEM hNextItem = treeCtrl.GetChildItem( hItem );

		if ( !hNextItem )
		{
			// Get next sibling child
			hNextItem = treeCtrl.GetNextSiblingItem( hItem );

			if ( !hNextItem )
			{
				HTREEITEM hParentItem=hItem;
				while ( !hNextItem && hParentItem )
				{
					// No more children: Get next sibling to parent
					hParentItem = treeCtrl.GetParentItem( hParentItem );
					hNextItem = treeCtrl.GetNextSiblingItem( hParentItem );
				}
			}
		}
		hItem = hNextItem;
	}
	return NULL;
}


/*void CTreeCtrlEx::OnRButtonDown( UINT nFlags, CPoint point )
{
	m_ptRClick = point;
}
*/
CPoint CTreeCtrlEx::GetRClickPoint()
{
	return m_ptRClick;
}
