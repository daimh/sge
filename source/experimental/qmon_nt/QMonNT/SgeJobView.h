#ifndef AFX_SGEJOBVIEW_H__655360D7_3D86_11D2_A83A_0020AFA6CCC8__INCLUDED_
#define AFX_SGEJOBVIEW_H__655360D7_3D86_11D2_A83A_0020AFA6CCC8__INCLUDED_
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

// SgeJobView.h : Header-Datei
//

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "SgePropertyPage.h"
#include "SizePropertySheet.h"
#include "SgeView.h"
#include "JobSet.h"

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSgeJobViewScript 

class CSgeJobViewScript : public CSgePropertyPage
{
	DECLARE_DYNCREATE(CSgeJobViewScript)

public:
	// Konstruktion
	CSgeJobViewScript();
	~CSgeJobViewScript();

	// Dialogfelddaten
	//{{AFX_DATA(CSgeJobViewScript)
	enum { IDD = IDD_SGEJOBVIEWSCRIPT };
		// HINWEIS - Der Klassen-Assistent fügt hier Datenelemente ein.
		//    Innerhalb dieser generierten Quellcodeabschnitte NICHTS BEARBEITEN!
	//}}AFX_DATA

	// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSgeJobViewScript)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSgeJobViewScript)
		// HINWEIS: Der Klassen-Assistent fügt hier Member-Funktionen ein
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSgeJobViewGeneral 

class CSgeJobViewGeneral : public CSgePropertyPage
{
	DECLARE_DYNCREATE(CSgeJobViewGeneral)

public:
	// Konstruktion
	CSgeJobViewGeneral();
	~CSgeJobViewGeneral();

	// Dialogfelddaten
	//{{AFX_DATA(CSgeJobViewGeneral)
	enum { IDD = IDD_SGEJOBVIEWGENERAL };
	CString	m_JobFile;
	CString	m_ScriptFile;
	CString	m_JobNumber;
	CString	m_InternalID;
	//}}AFX_DATA

	// Operations
	void SetLocalJobSet(CJobSet *pLocalJobSet);

	// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSgeJobViewGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSgeJobViewGeneral)
		// HINWEIS: Der Klassen-Assistent fügt hier Member-Funktionen ein
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool	m_DDXRunning;
	CJobSet *m_pLocalJobSet;
};

/////////////////////////////////////////////////////////////////////////////
// Formularansicht CSgeJobView 

class CSgeJobView : public CSgeView
{
protected:
	CSgeJobView();           // Dynamische Erstellung verwendet geschützten Konstruktor
	DECLARE_DYNCREATE(CSgeJobView)

public:
	// Attribute

	// Operationen
	virtual bool IsModified();
	virtual void LooseChanges();

	// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSgeJobView)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementierung
protected:
	virtual ~CSgeJobView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSgeJobView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool				m_bPropertySheetCreated;
	CJobSet				m_LocalJobSet;
	CSizePropertySheet	m_PropertySheet;
	CSgeJobViewGeneral	m_GeneralPage;
	CSgeJobViewScript	m_ScriptPage;
	// >>> Hier Code für weitere Eigenschaftsseiten einfügen

	void UpdateJobList();
	void UpdateSelection();
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_SGEJOBVIEW_H__655360D7_3D86_11D2_A83A_0020AFA6CCC8__INCLUDED_
