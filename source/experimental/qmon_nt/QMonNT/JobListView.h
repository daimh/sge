#if !defined(AFX_JOBLISTVIEW_H__5330BA03_ED79_11D2_971D_0020AFA6CCC8__INCLUDED_)
#define AFX_JOBLISTVIEW_H__5330BA03_ED79_11D2_971D_0020AFA6CCC8__INCLUDED_
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
// JobListView.h : header file
//

#include "SizePropertySheet.h"
#include "SgePropertyPage.h"
#include "SizePropertySheet.h"
#include "SgeView.h"
#include "QmonntDoc.h"
#include "..\GUITools\ClickList.h"

/////////////////////////////////////////////////////////////////////////////
// CJobListView dialog

class CJobListView : public CSgePropertyPage
{
	DECLARE_DYNCREATE(CJobListView)

public:
	// Construction
	CJobListView();   
	~CJobListView();   

	// Dialog Data
	//{{AFX_DATA(CJobListView)
	enum { IDD = IDD_SGEJOBSLIST };
	CButton	m_UnSuspendBtn;
	CButton	m_SuspendBtn;
	CButton	m_PriorityBtn;
	CButton	m_HoldBtn;
	CButton	m_DeleteJobBtn;
	CClickList	m_RunningJobList;
	CClickList	m_PendingJobList;
	CButton		m_RunningBtn;
	CButton		m_PendingBtn;
	CButton     m_SubmitBtn;
	//}}AFX_DATA

	// Operations
	void SetDocument(CQmonntDoc *pDoc);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJobListView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(CJobListView)
	virtual BOOL OnInitDialog();
	afx_msg void OnRunningradio();
	afx_msg void OnPendingradio();
	afx_msg void OnSubmitbutton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool				m_DDXRunning;
	CQmonntDoc			*m_pDoc;
		
	void InitTable(CListCtrl *m_Table);
	void AddJob(CJob *pJob);
	void ShowHideLists();
};

class CSgeJobListView : public CSgeView
{
protected:
	CSgeJobListView();
	DECLARE_DYNCREATE(CSgeJobListView)

public:
	// Attribute

	// Operationen
	virtual bool IsModified();
	virtual void LooseChanges();

	// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSgeJobListView)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementierung
protected:
	virtual ~CSgeJobListView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSgeJobListView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool				m_bPropertySheetCreated;
	CJobSet				m_LocalJobSet;
	CSizePropertySheet	m_PropertySheet;
	CJobListView		m_JobListPage;

	// >>> Hier Code für weitere Eigenschaftsseiten einfügen
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JOBLISTVIEW_H__5330BA03_ED79_11D2_971D_0020AFA6CCC8__INCLUDED_)
