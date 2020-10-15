#ifndef AFX_SGEQUEUEVIEW_H__655360D6_3D86_11D2_A83A_0020AFA6CCC8__INCLUDED_
#define AFX_SGEQUEUEVIEW_H__655360D6_3D86_11D2_A83A_0020AFA6CCC8__INCLUDED_
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

// SgeQueueView.h : Header-Datei
//

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "SgePropertyPage.h"
#include "SizePropertySheet.h"
#include "SgeView.h"
#include "SgeQueueViewCheckpointing.h"
#include "QueueSet.h"
#include "..\GuiTools\PopupEdit.h"
#include "..\GUITools\PopupTimeCtrl.h"
#include "..\GUITools\CBoxEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CSgeQueueViewLimits dialog

class CSgeQueueViewLimits : public CSgePropertyPage
{
	DECLARE_DYNCREATE(CSgeQueueViewLimits)

public:
	// Construction
	CSgeQueueViewLimits();   // standard constructor
	~CSgeQueueViewLimits();

	// Dialog Data
	//{{AFX_DATA(CSgeQueueViewLimits)
	enum { IDD = IDD_SGEQUEUEVIEWLIMITS };
	CPopupTimeCtrl	m_QueueSoftRealTime;
	CPopupTimeCtrl	m_QueueHardRealTime;
	CPopupTimeCtrl	m_QueueSoftCPUTime;
	CPopupTimeCtrl	m_QueueHardCPUTime;
	CPopupEdit	m_QueueHardVirtualMemory;
	CPopupEdit	m_QueueHardStackSize;
	CPopupEdit	m_QueueHardResidentSetSize;
	CPopupEdit	m_QueueHardFileSize;
	CPopupEdit	m_QueueHardDataSize;
	CPopupEdit	m_QueueHardCoreFileSize;
	CPopupEdit	m_QueueSoftStackSize;
	CPopupEdit	m_QueueSoftFileSize;
	CPopupEdit	m_QueueSoftDataSize;
	CPopupEdit	m_QueueSoftCoreFileSize;
	CPopupEdit	m_QueueSoftResidentSetSize;
	CPopupEdit	m_QueueSoftVirtualMemory;
	//}}AFX_DATA

	// Operations
	void SetLocalQueueSet(CQueueSet *pLocalQueueSet);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSgeQueueViewLimits)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSgeQueueViewLimits)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool		 m_DDXRunning;
	CQueueSet	*m_pLocalQueueSet;
};

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSgeQueueViewGeneral 

class CSgeQueueViewGeneral : public CSgePropertyPage
{
	DECLARE_DYNCREATE(CSgeQueueViewGeneral)

public:
	// Konstruktion
	CSgeQueueViewGeneral();
	~CSgeQueueViewGeneral();

	// Dialogfelddaten
	//{{AFX_DATA(CSgeQueueViewGeneral)
	enum { IDD = IDD_SGEQUEUEVIEWGENERAL };
	CSpinButtonCtrl	m_SpinQueuePriority;
	CPopupTimeCtrl	m_QueueNotifyTime;
	CString	m_QueueHostname;
	CString	m_QueueName;
	CString	m_QueueJobSlots;
	CString	m_InternalID;
	CString	m_QueuePriority;
	//}}AFX_DATA

	// Operations
	void SetLocalQueueSet(CQueueSet *pLocalQueueSet);

	// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSgeQueueViewGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSgeQueueViewGeneral)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool		 m_DDXRunning;
	CQueueSet	*m_pLocalQueueSet;
};

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSgeQueueViewEnv 

class CSgeQueueViewEnv : public CSgePropertyPage
{
	DECLARE_DYNCREATE(CSgeQueueViewEnv)

public:
	// Konstruktion
	CSgeQueueViewEnv();
	~CSgeQueueViewEnv();

	// Dialogfelddaten
	//{{AFX_DATA(CSgeQueueViewEnv)
	enum { IDD = IDD_SGEQUEUEVIEWENV };
	CString	m_QueueShell;
	CString	m_QueueTmpDir;
	//}}AFX_DATA

	// Operations
	void SetLocalQueueSet(CQueueSet *pLocalQueueSet);

	// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSgeQueueViewEnv)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSgeQueueViewEnv)
		// HINWEIS: Der Klassen-Assistent fügt hier Member-Funktionen ein
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool		 m_DDXRunning;
	CQueueSet	*m_pLocalQueueSet;
};

/////////////////////////////////////////////////////////////////////////////
// Formularansicht CSgeQueueView 

class CSgeQueueView : public CSgeView
{
public:
	// Attribute

	// Operationen
	virtual bool IsModified();
	virtual void LooseChanges();

	// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSgeQueueView)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementierung
protected:
	CSgeQueueView();           // Dynamische Erstellung verwendet geschützten Konstruktor
	virtual ~CSgeQueueView();

	DECLARE_DYNCREATE(CSgeQueueView)

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSgeQueueView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg void OnSend();
	DECLARE_MESSAGE_MAP()

private:
	bool						m_bPropertySheetCreated;
	CQueueSet					m_LocalQueueSet;
	CSgeQueueViewGeneral		m_GeneralPage;
	CSgeQueueViewEnv			m_EnvPage;
	CSgeQueueViewLimits			m_LimitsPage;
	CSgeQueueViewCheckpointing	m_CheckpointingPage;
	CSizePropertySheet			m_PropertySheet;
	CDialogBar					m_DlgBar;
	
	void UpdateQueueList();
	void UpdateSelection();
	// >>> Hier Code für weitere Eigenschaftsseiten einfügen
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_SGEQUEUEVIEW_H__655360D6_3D86_11D2_A83A_0020AFA6CCC8__INCLUDED_
