#if !defined(AFX_GENERALPAGE_H__9EA2DDC1_F882_11D2_9727_0020AFA6CCC8__INCLUDED_)
#define AFX_GENERALPAGE_H__9EA2DDC1_F882_11D2_9727_0020AFA6CCC8__INCLUDED_
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
// GeneralPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGeneralPage dialog

class CGeneralPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CGeneralPage)

public:
	// Construction
	CGeneralPage();
	~CGeneralPage();

	// Dialog Data
	//{{AFX_DATA(CGeneralPage)
	enum { IDD = IDD_JOB_GENERAL_PAGE };
	CSpinButtonCtrl	m_JobPrioritySpin;
	CString	m_JobArgs;
	CString	m_JobName;
	CString	m_JobPrefix;
	int		m_JobPriority;
	CString	m_JobScriptFileName;
	CString	m_JobShell;
	CString	m_JobStderr;
	CString	m_JobStdout;
	BOOL	m_JobMergeOutput;
	CTime	m_JobStartTime;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGeneralPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGeneralPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnJobscriptBtn();
	afx_msg void OnJobstdoutbtn();
	afx_msg void OnJobstderrbtn();
	afx_msg void OnJobshellBtn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERALPAGE_H__9EA2DDC1_F882_11D2_9727_0020AFA6CCC8__INCLUDED_)
