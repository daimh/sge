#ifndef AFX_ANSWERMESSAGEBOX_H__C6C8CA21_474C_11D2_81F1_0000B45BAE88__INCLUDED_
#define AFX_ANSWERMESSAGEBOX_H__C6C8CA21_474C_11D2_81F1_0000B45BAE88__INCLUDED_
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

// AnswerMessageBox.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CAnswerMessageBox 

#include "AnswerList.h"

class CAnswerMessageBox : public CDialog
{
public:
	// Konstruktion
	CAnswerMessageBox(CWnd* pParent = NULL);   // Standardkonstruktor

	// Dialogfelddaten
	//{{AFX_DATA(CAnswerMessageBox)
	enum { IDD = IDD_ANSWERMESSAGE };
	CString	m_MessageStatic;
	//}}AFX_DATA

	// Operations
	CAnswerMessageBox &operator << (char *s);
	CAnswerMessageBox &operator << (CString &s);
	CAnswerMessageBox &operator << (CAnswerList &al);
	CAnswerMessageBox &operator << (CAnswerList *al);

	void Clear();

	// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CAnswerMessageBox)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CAnswerMessageBox)
		// HINWEIS: Der Klassen-Assistent fügt hier Member-Funktionen ein
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_ANSWERMESSAGEBOX_H__C6C8CA21_474C_11D2_81F1_0000B45BAE88__INCLUDED_
