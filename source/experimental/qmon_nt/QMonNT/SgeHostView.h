#ifndef AFX_SGEHOSTVIEW_H__06EB99E1_8AD0_11D2_81F1_0000B45BAE88__INCLUDED_
#define AFX_SGEHOSTVIEW_H__06EB99E1_8AD0_11D2_81F1_0000B45BAE88__INCLUDED_
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

// SgeHostView.h : Header-Datei
//

#include "SgeView.h"
#include "HostSet.h"	// Hinzugefügt von ClassView
#include "SizePropertySheet.h"
#include "HostSet.h"
#include "SgePropertyPage.h"
#include "SgeQueueViewList.h"

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSgeHostViewGeneral 

class CSgeHostViewGeneral : public CSgePropertyPage
{
	DECLARE_DYNCREATE(CSgeHostViewGeneral)

public:
	// Attributes
	CHostSet *m_pLocalHostSet;

	// Konstruktion
	CSgeHostViewGeneral();
	~CSgeHostViewGeneral();

	// Dialogfelddaten
	//{{AFX_DATA(CSgeHostViewGeneral)
	enum { IDD = IDD_SGEHOSTVIEWGENERAL };
	CString	m_HostName;
	CString	m_InternalID;
	CString	m_Processors;
	CString	m_Realname;
	//}}AFX_DATA

	// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSgeHostViewGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSgeHostViewGeneral)
		// HINWEIS: Der Klassen-Assistent fügt hier Member-Funktionen ein
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool m_DDXRunning;
};

/////////////////////////////////////////////////////////////////////////////
// Ansicht CSgeHostView 

class CSgeHostView : public CSgeView
{
protected:
	CSgeHostView();           // Dynamische Erstellung verwendet geschützten Konstruktor
	DECLARE_DYNCREATE(CSgeHostView)

public:
	// Attribute

	// Operationen
	virtual bool IsModified();
	virtual void LooseChanges();

	// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSgeHostView)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void OnDraw(CDC* pDC);      // Überschrieben zum Zeichnen dieser Ansicht
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementierung
protected:
	virtual ~CSgeHostView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CSgeHostView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool				m_bPropertySheetCreated;
	CHostSet			m_LocalHostSet;
	CDialogBar			m_DlgBar;
	CSizePropertySheet	m_PropertySheet;
	CSgeHostViewGeneral m_GeneralPage;
	CSgeQueueViewList   m_QueuePage;

	void UpdateHostList();
	void UpdateSelection();
	// >>> Hier Code für weitere Eigenschaftsseiten einfügen
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_SGEHOSTVIEW_H__06EB99E1_8AD0_11D2_81F1_0000B45BAE88__INCLUDED_
