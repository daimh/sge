#ifndef AFX_SGEPROPERTYPAGE_H__7E503FE1_611D_11D2_81F1_0000B45BAE88__INCLUDED_
#define AFX_SGEPROPERTYPAGE_H__7E503FE1_611D_11D2_81F1_0000B45BAE88__INCLUDED_
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

// SgePropertyPage.h : Header-Datei
//

#include "ScrollPropertyPage.h"
#include "SgeSet.h"
#include "SgeView.h"
#include "..\GUITools\CBoxEdit.h"
#include "..\GUITools\PopupTimeCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSgePropertyPage 

class CSgePropertyPage : public CScrollPropertyPage
{
/*	DECLARE_DYNCREATE(CSgePropertyPage)
*/	
	DECLARE_DYNAMIC(CSgePropertyPage)

public:
	// Konstruktion
	CSgePropertyPage();
	CSgePropertyPage(UINT nIDTemplate, UINT nIDCaption = 0);
	~CSgePropertyPage();

	// Operations
	void SetParentView(CSgeView *ParentView);

	// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSgePropertyPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	void SetModified(CSgeSet *set);
	void SetAmbDlgItem(const CSgeSet *Set, int nFieldname, CString &DlgVar, ULONG Content, UINT nDlgStaticItem);
	void SetAmbDlgItem(const CSgeSet *Set, int nFieldname, CEdit &DlgVar, const CString &Content, UINT nDlgStaticItem);
	void SetAmbDlgItem(const CSgeSet *Set, int nFieldname, CCBoxMemory &DlgVar, const CString &Content, UINT nDlgStaticItem);
	void SetAmbDlgItem(const CSgeSet *Set, int nFieldname, CPopupTimeCtrl &DlgVar, const CTime &Content, UINT nDlgStaticItem);
	void SetAmbDlgItem(const CSgeSet *Set, int nFieldname, CDateTimeCtrl &DlgVar, const CTime &Content, UINT nDlgStaticItem);
	void SetAmbDlgItem(const CSgeSet *Set, int nFieldname, CString &DlgVar, const CString &Content, UINT nDlgStaticItem);
	void SetAmbDlgItem(const CSgeSet *Set, int nFieldname, CComboBox &DlgVar, const CString &Content, UINT nDlgStaticItem);

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSgePropertyPage)
		// HINWEIS: Der Klassen-Assistent fügt hier Member-Funktionen ein
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CSgeView *m_ParentView;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_SGEPROPERTYPAGE_H__7E503FE1_611D_11D2_81F1_0000B45BAE88__INCLUDED_
