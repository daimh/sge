#ifndef AFX_SGEQUEUEVIEWLIST_H__1C098121_8F7B_11D2_81F1_0000B45BAE88__INCLUDED_
#define AFX_SGEQUEUEVIEWLIST_H__1C098121_8F7B_11D2_81F1_0000B45BAE88__INCLUDED_
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

// SgeQueueViewList.h : Header-Datei
//

#include "SgePropertyPage.h"
#include "QmonntDoc.h"
#include "..\GuiTools\ClickList.h"

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSgeQueueViewList 

class CSgeQueueViewList : public CSgePropertyPage
{
	DECLARE_DYNCREATE(CSgeQueueViewList)

public:
	// Attributes
	CHostSet *m_pLocalHostSet;

	// Konstruktion
	CSgeQueueViewList();
	~CSgeQueueViewList();

	// Dialogfelddaten
	//{{AFX_DATA(CSgeQueueViewList)
	enum { IDD = IDD_SGEQUEUEVIEWLIST };
	//}}AFX_DATA

	// Operations
	void SetDocument(CQmonntDoc *pDoc);

	// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSgeQueueViewList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSgeQueueViewList)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CClickList	m_QueueList;
	CImageList  m_StateIcons;
	bool		m_bQueueListCreated;
	bool		m_DDXRunning;
	CQmonntDoc	*m_pDoc;
	
	void AddQueue(CQueue *pQueue);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_SGEQUEUEVIEWLIST_H__1C098121_8F7B_11D2_81F1_0000B45BAE88__INCLUDED_
