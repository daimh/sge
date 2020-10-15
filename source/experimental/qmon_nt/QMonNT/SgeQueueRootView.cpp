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
// CodQueueRootView.cpp: Implementierungsdatei
//
// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "CodQueueRootView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodQueueRootView

IMPLEMENT_DYNCREATE(CCodQueueRootView, CCodView)

CCodQueueRootView::CCodQueueRootView()
{
	m_bPropertySheetCreated = false;
}

CCodQueueRootView::~CCodQueueRootView()
{
}

BEGIN_MESSAGE_MAP(CCodQueueRootView, CCodView)
	//{{AFX_MSG_MAP(CCodQueueRootView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Zeichnung CCodQueueRootView 

void CCodQueueRootView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// ZU ERLEDIGEN: Code zum Zeichnen hier einfügen
}

/////////////////////////////////////////////////////////////////////////////
// Diagnose CCodQueueRootView

#ifdef _DEBUG
void CCodQueueRootView::AssertValid() const
{
	CCodView::AssertValid();
}

void CCodQueueRootView::Dump(CDumpContext& dc) const
{
	CCodView::Dump(dc);
}
#endif //_DEBUG

bool CCodQueueRootView::IsModified()
{
	return false;
}

void CCodQueueRootView::LooseChanges()
{
	// Stefan Mihaila: Just don't do anything
}

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodQueueRootView 

void CCodQueueRootView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (0 == lHint || UAVH_QUEUES == lHint) {
		CPropertyPage *ActivePage = m_PropertySheet.GetActivePage();
		ASSERT_VALID(ActivePage);
		ActivePage->OnInitDialog();
	}
}

BOOL CCodQueueRootView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	BOOL Result = CCodView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	ASSERT_VALID(m_SplitterFrame);

	m_PropertySheet.AddPage(&m_ListPage);
	// >>> Hier Code für weitere Eigenschaftsseiten einfügen

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pQueueList);
	m_ListPage.SetDocument(pDoc);

	m_PropertySheet.Create(this, WS_CHILD | WS_VISIBLE);
	m_bPropertySheetCreated = true;

	m_PropertySheet.EnableButtonSend(false);
	m_PropertySheet.EnableButtonRevert(false);

	m_ListPage.SetParentView(this);
	// >>> Hier Code für weitere Eigenschaftsseiten einfügen

	return Result;
}

void CCodQueueRootView::OnSize(UINT nType, int cx, int cy) 
{
	CCodView::OnSize(nType, cx, cy);

	CRect ClientRect;
	if (m_bPropertySheetCreated) {
		// Wir empfangen diese Meldung schon, bevor das PropertySheet existiert!
		GetClientRect(ClientRect);
		m_PropertySheet.Resize(ClientRect.Size());
	}
}
