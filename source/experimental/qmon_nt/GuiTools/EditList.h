#if !defined(AFX_EDITLIST_H__A0FA15B6_F231_11D2_9721_0020AFA6CCC8__INCLUDED_)
#define AFX_EDITLIST_H__A0FA15B6_F231_11D2_9721_0020AFA6CCC8__INCLUDED_
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
// EditList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditList window

class AFX_EXT_CLASS CEditList : public CListCtrl
{
	DECLARE_DYNAMIC(CEditList);
// Construction
public:
	CEditList();

// Attributes
public:
	LVITEM m_item;
// Operations
public:
	int     HitTestEx(CPoint &point, int *col) const;
	CRect	GetInPlaceRect(int nItem, int nCol);
	void    ShowListValues();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditList)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditList)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITLIST_H__A0FA15B6_F231_11D2_9721_0020AFA6CCC8__INCLUDED_)
