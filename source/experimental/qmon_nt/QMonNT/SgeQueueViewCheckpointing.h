#if !defined(AFX_SGEQUEUEVIEWCHECKPOINTING_H__C3F7C56D_D2E0_11D2_9703_0020AFA6CCC8__INCLUDED_)
#define AFX_SGEQUEUEVIEWCHECKPOINTING_H__C3F7C56D_D2E0_11D2_9703_0020AFA6CCC8__INCLUDED_
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
// SgeQueueViewCheckpointing.h : header file
//

#include "SgePropertyPage.h"
#include "QueueComplex.h"
#include "QmonntDoc.h"
#include "..\GUITools\ClickList.h"

/////////////////////////////////////////////////////////////////////////////
// CSgeQueueViewCheckpointing dialog

class CSgeQueueViewCheckpointing : public CSgePropertyPage
{
	DECLARE_DYNCREATE(CSgeQueueViewCheckpointing)

public:
	// Construction
	CSgeQueueViewCheckpointing();
	~CSgeQueueViewCheckpointing();

	// Dialog Data
	//{{AFX_DATA(CSgeQueueViewCheckpointing)
	enum { IDD = IDD_SGEQUEUEVIEWCHECKPOINTING };
	CButton	m_EmptyButton;
	CButton	m_DeleteButton;
	CButton	m_AddButton;
	CClickList m_Table;
	//}}AFX_DATA

	// Operations
	void Init(CQueueSet *pLocalQueueSet, CQmonntDoc *pDoc);
	bool HasAlreadyCheckPointing(const CString &Name);
	
	// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSgeQueueViewCheckpointing)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSgeQueueViewCheckpointing)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddbutton();
	afx_msg void OnDeletebutton();
	afx_msg void OnEmptybutton();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool					m_DDXRunning;
	CQueueSet			   *m_pLocalQueueSet;
	CQueue				   *m_pTempQueue;	
	CComplexAtributeList	m_AllAvailableCheckPointings;
	
	void InitTable();
	void FillTable();
	void AddCheckPointing(const CString &Name, CDataType Type, const CString &Value = "");
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SGEQUEUEVIEWCHECKPOINTING_H__C3F7C56D_D2E0_11D2_9703_0020AFA6CCC8__INCLUDED_)
