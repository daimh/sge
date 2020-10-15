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
// CodMessageView.cpp: Implementierungsdatei
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "CodMessageView.h"
#include "QMonntDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodMessageView

IMPLEMENT_DYNCREATE(CCodMessageView, CFormView)

CCodMessageView::CCodMessageView()
	: CFormView(CCodMessageView::IDD)
{
	//{{AFX_DATA_INIT(CCodMessageView)
	m_Message = _T("");
	//}}AFX_DATA_INIT
}

CCodMessageView::~CCodMessageView()
{
}

void CCodMessageView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCodMessageView)
	DDX_Text(pDX, IDC_MESSAGE, m_Message);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCodMessageView, CFormView)
	//{{AFX_MSG_MAP(CCodMessageView)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Diagnose CCodMessageView

#ifdef _DEBUG
void CCodMessageView::AssertValid() const
{
	CFormView::AssertValid();
}

void CCodMessageView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodMessageView 

/*
** OnUpdate
**
** Wird aufgerufen, wenn der MessageView aktualisiert wird. Holt aus dem
** Dokument die aktuelle Meldung und zeigt sie an.
*/
void CCodMessageView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);

	m_Message = pDoc->GetMessage();
	ASSERT(m_Message);
	
	UpdateData(FALSE);
}

bool CCodMessageView::CanChangeSelection()
{
	return true;
}
