// SgeComplexAtributeView.h: interface for the CSgeComplexAtributeView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SGECOMPLEXATRIBUTEVIEW_H__837AEE34_E34A_11D2_9716_0020AFA6CCC8__INCLUDED_)
#define AFX_SGECOMPLEXATRIBUTEVIEW_H__837AEE34_E34A_11D2_9716_0020AFA6CCC8__INCLUDED_
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


#include "SgeView.h"
#include "ComplexAtributeView.h"
#include "SizePropertySheet.h"
#include "ComplexAtributeSet.h"

/////////////////////////////////////////////////////////////////////////////
// CSgeComplexAtributeView view

class CSgeComplexAtributeView : public CSgeView
{
protected:
	CSgeComplexAtributeView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSgeComplexAtributeView)

public:
	// Attributes

	// Operations
	virtual bool IsModified();
	virtual void LooseChanges();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSgeComplexAtributeView)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSgeComplexAtributeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CSgeComplexAtributeView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	afx_msg void OnSend();
	DECLARE_MESSAGE_MAP()

private:
	bool					m_bPropertySheetCreated;
	CComplexAtributeSet		m_LocalComplexAtributeSet;
	CSizePropertySheet		m_PropertySheet;
	CComplexAtributeView	m_ComplexAtributePage;

	void UpdateComplexAtributeList();
	void UpdateSelection();
};

#endif // !defined(AFX_SGECOMPLEXATRIBUTEVIEW_H__837AEE34_E34A_11D2_9716_0020AFA6CCC8__INCLUDED_)

/////////////////////////////////////////////////////////////////////////////
