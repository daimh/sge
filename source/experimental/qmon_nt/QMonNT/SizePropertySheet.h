#ifndef AFX_SIZEPROPERTYSHEET_H__91D8D733_5380_11D2_A848_0020AFA6CCC8__INCLUDED_
#define AFX_SIZEPROPERTYSHEET_H__91D8D733_5380_11D2_A848_0020AFA6CCC8__INCLUDED_
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

// SizePropertySheet.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// CSizePropertySheet

#include "ViewDlgBar.h"

class AFX_EXT_CLASS CSizePropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CSizePropertySheet)

public:
	// Attribute

	// Konstruktion
	CSizePropertySheet();
	CSizePropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CSizePropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CSizePropertySheet();

	// Operationen
	void EnableButtonRevert(bool bEnable = true);
	void EnableButtonSend(bool bEnable = true);
	void Resize(const CSize &SizeNew = CSize(0, 0));

	// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSizePropertySheet)
	public:
	virtual BOOL OnInitDialog();
	protected:
	//}}AFX_VIRTUAL


	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CSizePropertySheet)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CPoint				m_PageOffset;
	CPoint				m_TabOffset;
	CSize				m_TabInset;
	CSize				m_PageInset;
	CSize				m_SavedSize;
//	CSize				m_ButtonSize;
//	CSize				m_ButtonBorder;
	CSize				m_ButtonBarSize;
//	bool				m_bHasButtons;
//	CButton				m_CancelButton;
//	CDialogBar			m_DlgBar;
	CViewDlgBar			m_ButtonBar;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_SIZEPROPERTYSHEET_H__91D8D733_5380_11D2_A848_0020AFA6CCC8__INCLUDED_
