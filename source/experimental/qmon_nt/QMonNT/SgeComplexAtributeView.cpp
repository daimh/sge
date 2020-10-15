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
// CodComplexAtributeView.cpp : implementation file
//
// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "Debug.h"
#include "QmonNTDoc.h"
#include "CodComplexAtributeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodComplexAtributeView

IMPLEMENT_DYNCREATE(CCodComplexAtributeView, CCodView)

CCodComplexAtributeView::CCodComplexAtributeView()
{
	m_bPropertySheetCreated = false;
}

CCodComplexAtributeView::~CCodComplexAtributeView()
{
}

BEGIN_MESSAGE_MAP(CCodComplexAtributeView, CCodView)
	//{{AFX_MSG_MAP(CCodComplexAtributeView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCodComplexAtributeView drawing
bool CCodComplexAtributeView::IsModified()
{
	// Stefan Mihaila: TODO
	return false;
}

void CCodComplexAtributeView::LooseChanges()
{
	// Stefan Mihaila: TODO
}

void CCodComplexAtributeView::OnSend()
{
	ASSERT_VALID(m_SplitterFrame);

	CQmonntDoc *Doc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(Doc);

	AfxMessageBox("Hello");

	CPropertyPage *ActivePage = m_PropertySheet.GetActivePage();
	ASSERT_VALID(ActivePage);
	ActivePage->UpdateData(TRUE);

	// Änderungen an das Dokument übergeben, dort werden sie mit der Gesamtliste 
	// vereinigt.
	Doc->SetChangedData(&m_LocalComplexAtributeSet);

	m_SplitterFrame->SetModified(false);
	m_LocalComplexAtributeSet.ClearModified();
}

void CCodComplexAtributeView::OnDraw(CDC* pDC)
{
	CDocument *pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CCodComplexAtributeView diagnostics

#ifdef _DEBUG
void CCodComplexAtributeView::AssertValid() const
{
	CCodView::AssertValid();
}

void CCodComplexAtributeView::Dump(CDumpContext& dc) const
{
	CCodView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCodComplexAtributeView message handlers

void CCodComplexAtributeView::OnSize(UINT nType, int cx, int cy) 
{
	CCodView::OnSize(nType, cx, cy);

	CRect ClientRect;
	if (m_bPropertySheetCreated) {
		// Wir empfangen diese Meldung schon, bevor das PropertySheet existiert!
		GetClientRect(ClientRect);
		m_PropertySheet.Resize(ClientRect.Size());
	}	
}

BOOL CCodComplexAtributeView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	BOOL Result = CCodView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	ASSERT_VALID(m_SplitterFrame);

	m_PropertySheet.AddPage(&m_ComplexAtributePage);
	// >>> Hier Code für weitere Eigenschaftsseiten einfügen
	
	m_PropertySheet.Create(this, WS_CHILD | WS_VISIBLE);
	m_bPropertySheetCreated = true;

	m_PropertySheet.EnableButtonSend(true);
	m_PropertySheet.EnableButtonRevert(true);

	m_ComplexAtributePage.SetParentView(this);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pComplexList);
	
//	m_ComplexAtributePage.SetDocument(pDoc);

//	m_DlgBar.Create(this, IDD_SHEETDLGBAR, CBRS_BOTTOM, 0xe800);

	return Result;
}

void CCodComplexAtributeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	/*CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);*/

	if (0 == lHint || UAVH_COMPLEXES == lHint)
		UpdateComplexAtributeList();
	else if (UAVH_SELECTION == lHint)
		UpdateSelection();
	
	UpdateData(FALSE);			
}

void CCodComplexAtributeView::UpdateComplexAtributeList()
{
	ASSERT_VALID(m_SplitterFrame);

	CNodeInfoSet *pSelection = m_SplitterFrame->GetTreeSelection();
	ASSERT(NULL != pSelection);

	DPRINTF(("******* CCodComplexAtributeView::UpdateComplexAtributeList Selection"));
	pSelection->DebugOut();
	m_LocalComplexAtributeSet.DebugOut();

	if (m_LocalComplexAtributeSet.IsModified())
		SetAged();
	else
		UpdateSelection();
}

void CCodComplexAtributeView::UpdateSelection()
{
	ASSERT_VALID(m_SplitterFrame);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);

	CNodeInfoSet *pSelection = m_SplitterFrame->GetTreeSelection();
	ASSERT(NULL != pSelection);
	pDoc->GetComplexAtributeSet(&m_LocalComplexAtributeSet, pSelection);

	DPRINTF(("***************** ComplexAtributeSet **********************"));
	m_LocalComplexAtributeSet.DebugOut();

	m_ComplexAtributePage.SetLocalComplexAtributeSet(&m_LocalComplexAtributeSet);

	CPropertyPage *ActivePage = m_PropertySheet.GetActivePage();
	ASSERT_VALID(ActivePage);
	ActivePage->OnInitDialog();

	SetModified(m_LocalComplexAtributeSet.IsModified());
	SetLocal(m_LocalComplexAtributeSet.IsLocal());
	SetAged(false);
}
