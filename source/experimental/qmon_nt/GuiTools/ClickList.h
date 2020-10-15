#if !defined(AFX_CLICKLIST_H__A023E784_D526_11D2_9704_0020AFA6CCC8__INCLUDED_)
#define AFX_CLICKLIST_H__A023E784_D526_11D2_9704_0020AFA6CCC8__INCLUDED_
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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ClickList.h : header file
//

#include <afxdtctl.h>
#include <queue.h>

extern "C" {
#include "basis_types.h"
}

enum CHighlightType { 
	HT_NORMAL, HT_ALLCOLUMNS, HT_ROW 
};

enum CDataType
{
	DT_INTEGER	= TYPE_INT,
	DT_STRING   = TYPE_STR,
	DT_TIME		= TYPE_TIM,
	DT_MEMORY	= TYPE_MEM,
	DT_BOOLEAN	= TYPE_BOO,
	DT_CSTRING	= TYPE_CSTR,
	DT_HOST		= TYPE_HOST,

	DT_LIST		// For a listbox with selection values
};

class CEditableColList;

struct CEditableCol
{
	int			ColNumber;
	CDataType	DataType;

	CEditableCol(int ColNumber, CDataType DataType)
	{
		this->ColNumber = ColNumber;
		this->DataType  = DataType;
	}

	inline operator CEditableColList();
	inline CEditableColList operator + (const CEditableCol &EditableCol);
};

/////////////////////////////////////////////////////////////////////////////
// CEditableColumnList

class CEditableColList : public std::deque<CEditableCol>		
{
public:
	CEditableColList &operator + (const CEditableCol &EditableCol)
	{
		int n = size();

		push_back(EditableCol);
		return *this;
	}

protected:

private:
};

/////////////////////////////////////////////////////////////////////////////
// CEditableCol inline implementations

inline CEditableCol::operator CEditableColList()
{
	return CEditableColList() + *this;
}

inline CEditableColList CEditableCol::operator + (const CEditableCol &EditableCol)
{
	return CEditableColList() + *this + EditableCol;
}

/////////////////////////////////////////////////////////////////////////////
// CClickList window

class AFX_EXT_CLASS CClickList : public CListCtrl
{
	DECLARE_DYNAMIC(CClickList);

public:
	// Construction
	CClickList();
	virtual ~CClickList();

	// Operations
	CHighlightType SetHighlightType(CHighlightType Type);
	void SetEditableColumns(int Row, const CEditableColList &EditableColumns);
	bool GetLastEditedRowCol(int &Row, int &Col);
	void AllowAddRemoveRowsByKeyboard(bool Allow = true);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClickList)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL
	
	// Generated message map functions
protected:
	struct CRowInfo
	{
		CEditableColList EditableColList;

		CRowInfo(const CEditableColList &AnEditableColList) :
			EditableColList(AnEditableColList) {}
	};

	CHighlightType m_HighlightType;		

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	//{{AFX_MSG(CClickList)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPaint();
	afx_msg BOOL OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	bool m_AllowAddRemoveRowsByKeyboard;
	int	 m_LastEditedRow;
	int  m_LastEditedCol;

	int	 HitTestEx(const CPoint &Point, int &Col) const;
	void RepaintSelectedItems();

	CDateTimeCtrl  *ShowInPlaceTime  (int Row, int Col);
	CEdit          *ShowInPlaceString(int Row, int Col);
	CEdit          *ShowInPlaceLong  (int Row, int Col);
	CEdit          *ShowInPlaceMemory(int Row, int Col);
	CListBox       *ShowInPlaceList  (int Row, int Col);

	CRect			GetInPlaceRect   (int Row, int Col);
	void			ShowEditControl  (int Row, int Col, CDataType DataType);
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLICKLIST_H__A023E784_D526_11D2_9704_0020AFA6CCC8__INCLUDED_)
