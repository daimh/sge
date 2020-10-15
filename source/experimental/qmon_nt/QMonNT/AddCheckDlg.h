#if !defined(AFX_ADDCHECKDLG_H__FA1A5056_DC4E_11D2_970E_0020AFA6CCC8__INCLUDED_)
#define AFX_ADDCHECKDLG_H__FA1A5056_DC4E_11D2_970E_0020AFA6CCC8__INCLUDED_
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

// AddCheckDlg.h : header file
//

#include "SgeQueueViewCheckpointing.h"
#include "ComplexAtribute.h"

/////////////////////////////////////////////////////////////////////////////
// CAddCheckDlg dialog

class CAddCheckDlg : public CDialog
{
public:
	// Construction
	CAddCheckDlg(CComplexAtributeList &AllAvailableCheckPointings, 
		CSgeQueueViewCheckpointing *pParent);

	// Dialog Data
	//{{AFX_DATA(CAddCheckDlg)
	enum { IDD = IDD_ADDCHECKDIALOG };
	CEdit	m_Edit;
	CListCtrl	m_Table;
	//}}AFX_DATA

	// Operations
	bool HasSelection();
	CComplexAtribute GetSelectedCheckPointing();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddCheckDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAddCheckDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnItemchangedChecpointslist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkChecpointslist(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CSgeQueueViewCheckpointing	*m_Parent;
	// Pass by reference, to optimize the code
	CComplexAtributeList		&m_AllAvailableCheckPointings;
	CString						m_SelectedCheckPointName;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDCHECKDLG_H__FA1A5056_DC4E_11D2_970E_0020AFA6CCC8__INCLUDED_)
