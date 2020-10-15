#if !defined(AFX_COMPLEXATRIBUTEVIEW_H__837AEE33_E34A_11D2_9716_0020AFA6CCC8__INCLUDED_)
#define AFX_COMPLEXATRIBUTEVIEW_H__837AEE33_E34A_11D2_9716_0020AFA6CCC8__INCLUDED_
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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ComplexAtributeView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CComplexAtributeView dialog

#include "SizePropertySheet.h"
#include "SgePropertyPage.h"
#include "ComplexAtributeSet.h"

class CComplexAtributeView : public CSgePropertyPage
{
	DECLARE_DYNCREATE(CComplexAtributeView)

public:
	// Construction
	CComplexAtributeView();   
	~CComplexAtributeView(); 
	
	// Dialog Data
	//{{AFX_DATA(CComplexAtributeView)
	enum { IDD = IDD_SGECOMPLEXVIEW };
	CEdit	m_Value;
	CComboBox	m_Type;
	CEdit	m_Shortcut;
	CComboBox	m_Requestable;
	CComboBox	m_Relation;
	CEdit	m_Name;
	CEdit	m_Default;
	CComboBox	m_Consumable;
	//}}AFX_DATA

	// Operations
	void SetLocalComplexAtributeSet(CComplexAtributeSet *pLocalComplexAtributeSet);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CComplexAtributeView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CComplexAtributeView)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool					 m_DDXRunning;
	CComplexAtributeSet		*m_pLocalComplexAtributeSet;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPLEXATRIBUTEVIEW_H__837AEE33_E34A_11D2_9716_0020AFA6CCC8__INCLUDED_)
