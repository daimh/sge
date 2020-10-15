#if !defined(AFX_POPUPEDIT_H__BCC7B96D_D137_11D2_9701_0020AFA6CCC8__INCLUDED_)
#define AFX_POPUPEDIT_H__BCC7B96D_D137_11D2_9701_0020AFA6CCC8__INCLUDED_
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
// PopupEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPopupEdit window

class AFX_EXT_CLASS CPopupEdit : public CEdit
{
	DECLARE_DYNAMIC(CPopupEdit);

public:
	// Attributes

	// Construction
	CPopupEdit();
	CPopupEdit(int iItem, int iSubItem, const CString &sInitText);
	virtual ~CPopupEdit();

	// Operations

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPopupEdit)
	//}}AFX_VIRTUAL

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// Generated message map functions
protected:

	//{{AFX_MSG(CPopupEdit)
	afx_msg void OnNcDestroy();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPopupInfinity();
	afx_msg void OnPopupKbyte1000();
	afx_msg void OnPopupKbyte1024();
	afx_msg void OnPopupGbyte1000();
	afx_msg void OnPopupGbyte1024();
	afx_msg void OnPopupMbyte1000();
	afx_msg void OnPopupMbyte1024();
	afx_msg void OnPopupByte();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	bool	m_bIsEmbeddedIntoListCtrl;
	bool    m_bESC;	
	int		m_iItem;
	int		m_iSubItem;
	CString m_sInitText;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPUPEDIT_H__BCC7B96D_D137_11D2_9701_0020AFA6CCC8__INCLUDED_)
