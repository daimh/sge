#ifndef AFX_DEBUGVIEW_H__FA476C62_302B_11D2_A82C_0020AFA6CCC8__INCLUDED_
#define AFX_DEBUGVIEW_H__FA476C62_302B_11D2_A82C_0020AFA6CCC8__INCLUDED_
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

// DebugView.h : Header-Datei
//

#define UPDATE_INSERTLINE	100	/* we will update because we have a new message*/

/////////////////////////////////////////////////////////////////////////////
// Ansicht CDebugView 


class CDebugView : public CScrollView
{
private:
	CString m_DebugMessage;
	char Buffer[1024];
protected:
	CDebugView();           // Dynamische Erstellung verwendet geschützten Konstruktor
	DECLARE_DYNCREATE(CDebugView)

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CDebugView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementierung
protected:
	virtual ~CDebugView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	LRESULT OnDebugMessage(WPARAM, LPARAM);
	//{{AFX_MSG(CDebugView)
	afx_msg void OnDebugStartdebug();
	afx_msg void OnUpdateDebugStartdebug(CCmdUI* pCmdUI);
	afx_msg void OnDebugStopdebug();
	afx_msg void OnUpdateDebugStopdebug(CCmdUI* pCmdUI);
	afx_msg void OnDebugSeparator();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDebugClearbuffer();
	afx_msg void OnDebugSavebuffertofile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	int m_LineHeight;
	void DrawMessageList();
	void InsertLine(/*CString &*/);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_DEBUGVIEW_H__FA476C62_302B_11D2_A82C_0020AFA6CCC8__INCLUDED_
