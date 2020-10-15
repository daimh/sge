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
// CodView.cpp: Implementierungsdatei
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "CodView.h"
#include "QmonntDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodView

//IMPLEMENT_DYNCREATE(CCodView, CView)
IMPLEMENT_DYNAMIC(CCodView, CView)

CCodView::CCodView()
{
}

CCodView::~CCodView()
{
}


BEGIN_MESSAGE_MAP(CCodView, CView)
	//{{AFX_MSG_MAP(CCodView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Zeichnung CCodView 

void CCodView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// ZU ERLEDIGEN: Code zum Zeichnen hier einfügen
}

/////////////////////////////////////////////////////////////////////////////
// Diagnose CCodView

#ifdef _DEBUG
void CCodView::AssertValid() const
{
	CView::AssertValid();
}

void CCodView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodView 

void CCodView::Update()
{
	OnUpdate(NULL, UAVH_SELECTION, NULL);
}

void CCodView::SetModified(bool set)
{
	ASSERT_VALID(m_SplitterFrame);

	m_Modified = set;
	m_SplitterFrame->SetModified(set);
}

/*
** SetLocal
**
** Setzt die Markierung für die lokale Bearbeitung. Falls set = 0 ist,
** wird die Markierung ausgeschaltet, falls set = 1 ist, wird sie eingeschaltet,
** bei set = -1 wird '(Local)', also in Klammern angezeigt, was bedeutet, daß
** nicht alle der angezeigten Objekte Lokal sind.
*/
void CCodView::SetLocal(int set)
{
	ASSERT_VALID(m_SplitterFrame);

	m_Local = set;
	m_SplitterFrame->SetLocal(set);
}

/*
** SetAged
**
** Setzt die Markierung 'Aged Data' in der Statuszeile. Bei set = true wird
** die Markierung eingeschaltet, bei false wird sie ausgeschaltet.
*/
void CCodView::SetAged(bool set)
{
	ASSERT_VALID(m_SplitterFrame);

	m_Aged = set;
	m_SplitterFrame->SetAged(set);
}

BOOL CCodView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	BOOL Result = CView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	m_SplitterFrame = dynamic_cast<CSplitterFrame *>(GetParentFrame());
	ASSERT_VALID(m_SplitterFrame);

	return Result;
}
