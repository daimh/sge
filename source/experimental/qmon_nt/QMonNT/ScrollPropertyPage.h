#ifndef AFX_SCROLLPROPERTYPAGE_H__5FCC2083_52E4_11D2_81F1_0000B45BAE88__INCLUDED_
#define AFX_SCROLLPROPERTYPAGE_H__5FCC2083_52E4_11D2_81F1_0000B45BAE88__INCLUDED_
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

// ScrollPropertyPage.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CScrollPropertyPage 

class AFX_EXT_CLASS CScrollPropertyPage : public CPropertyPage
{
/*	DECLARE_DYNCREATE(CScrollPropertyPage)
*/
	DECLARE_DYNAMIC(CScrollPropertyPage)

public:
	// Konstruktion
	CScrollPropertyPage();
	CScrollPropertyPage( UINT nIDTemplate, UINT nIDCaption = 0 );
	~CScrollPropertyPage();

	// Operations
	void SetClientRect(const CRect &NewRect);

// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CScrollPropertyPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CScrollPropertyPage)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CRect	m_ClientRect;
	int		m_VPos;
	int		m_HPos;
	CSize	m_ScrollSize;
	CSize	m_ClientSize;

	void RecalcScrollbars();
	void ScrollTo(int HPos, int VPos);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_SCROLLPROPERTYPAGE_H__5FCC2083_52E4_11D2_81F1_0000B45BAE88__INCLUDED_
