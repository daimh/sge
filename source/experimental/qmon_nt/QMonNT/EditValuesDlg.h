#if !defined(AFX_EDITVALUESDLG_H__45025046_F8BB_11D2_9728_0020AFA6CCC8__INCLUDED_)
#define AFX_EDITVALUESDLG_H__45025046_F8BB_11D2_9728_0020AFA6CCC8__INCLUDED_
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
// EditValuesDlg.h : header file
//

#include "..\GUITools\ClickList.h"

/////////////////////////////////////////////////////////////////////////////
// CEditValuesDlg dialog

class CEditValuesDlg : public CDialog
{
public:
	// Attributes
	CString ListString;

	// Construction
	CEditValuesDlg(CWnd *pParent = NULL, const CString &str = "");   // standard constructor
	
	// Dialog Data
	//{{AFX_DATA(CEditValuesDlg)
	enum { IDD = IDD_LISTVALUESDLG };
	CClickList	m_ValuesList;
	CString	m_Host;
	CString	m_Path;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditValuesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CEditValuesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddbutton();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private: 
	bool m_bWasAlreadyAnEmptyHostAdded;

	void AddItem(const CString &Host, const CString &Path);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITVALUESDLG_H__45025046_F8BB_11D2_9728_0020AFA6CCC8__INCLUDED_)
