#ifndef AFX_SPLITTERFRAME_H__655360D4_3D86_11D2_A83A_0020AFA6CCC8__INCLUDED_
#define AFX_SPLITTERFRAME_H__655360D4_3D86_11D2_A83A_0020AFA6CCC8__INCLUDED_
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

// SplitterFrame.h : Header-Datei
//

#include "NodeInfo.h"

/////////////////////////////////////////////////////////////////////////////
// Rahmen CSplitterFrame 

enum CRightViewType {
	RVT_NOTHING	= 100,	/* We have nothing to show for this view */	
	RVT_MESSAGE,			/* A Message should be displayed */
	RVT_NOMESSAGE,			/* */
	
	RVT_QUEUEROOT,			/* Show a list of queues */
	RVT_QUEUE,				/* show queues */
	RVT_JOBROOT,				
	RVT_JOB,				   /* show jobs */
	RVT_HOST,				/* show hosts */
	RVT_COMPLEXATRIBUTE		
		
	// >>> Code für neue Datentypen hier einfügen
};

// Forward-Deklaration:
class CSgeTreeView;

class CSplitterFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CSplitterFrame)

public:
	void SetAged(bool set);
	void SetModified(bool set = true);
	void SetLocal(int set);

	CNodeInfoSet *GetTreeSelection();
	void OnSelectionChanged(CNodeInfoSet *NodeInfoSet);
	bool IsCurrentViewModified();
	void LooseChanges();

	void SetRightView(CRightViewType ViewType);
	CRightViewType GetCurrentRightViewType();

protected:
	CSplitterFrame();           // Dynamische Erstellung verwendet geschützten Konstruktor
	virtual ~CSplitterFrame();

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSplitterFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSplitterFrame)
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CSplitterWnd	m_wndSplitter;
	CDialogBar		m_DlgBar;
	CStatusBar		*m_pStatusBar;
	CSgeTreeView	*m_pTreeView;
	CRightViewType	m_CurrentRightViewType;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_SPLITTERFRAME_H__655360D4_3D86_11D2_A83A_0020AFA6CCC8__INCLUDED_
