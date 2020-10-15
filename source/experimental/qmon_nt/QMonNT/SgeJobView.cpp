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
// CodJobView.cpp: Implementierungsdatei
//
// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "Debug.h"
#include "CodJobView.h"
#include "QmonNTDoc.h"

extern "C" {
#include "cod_jobL.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/****************************************************************************
** Eigenschaftenseite CCodJobViewScript 
****************************************************************************/

IMPLEMENT_DYNCREATE(CCodJobViewScript, CCodPropertyPage)

CCodJobViewScript::CCodJobViewScript() : CCodPropertyPage(CCodJobViewScript::IDD)
{
	//{{AFX_DATA_INIT(CCodJobViewScript)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}

CCodJobViewScript::~CCodJobViewScript()
{
}

void CCodJobViewScript::DoDataExchange(CDataExchange* pDX)
{
	TRACE("CCodJobViewScript::DoDataExchange\n");
	CCodPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCodJobViewScript)
		// HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCodJobViewScript, CCodPropertyPage)
	//{{AFX_MSG_MAP(CCodJobViewScript)
		// HINWEIS: Der Klassen-Assistent fügt hier Zuordnungsmakros für Nachrichten ein
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodJobViewScript 

/****************************************************************************
** Eigenschaftenseite CCodJobViewGeneral 
****************************************************************************/

IMPLEMENT_DYNCREATE(CCodJobViewGeneral, CCodPropertyPage)

CCodJobViewGeneral::CCodJobViewGeneral() : CCodPropertyPage(CCodJobViewGeneral::IDD)
{
	m_DDXRunning   = true;
	m_pLocalJobSet = NULL;
	//{{AFX_DATA_INIT(CCodJobViewGeneral)
	m_JobFile = _T("");
	m_ScriptFile = _T("");
	m_JobNumber = _T("");
	m_InternalID = _T("");
	//}}AFX_DATA_INIT
}

CCodJobViewGeneral::~CCodJobViewGeneral()
{
}

void CCodJobViewGeneral::SetLocalJobSet(CJobSet *pLocalJobSet)
{
	ASSERT(NULL != pLocalJobSet);
	m_pLocalJobSet = pLocalJobSet;
}

void CCodJobViewGeneral::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(NULL != pDX);

	CCodPropertyPage::DoDataExchange(pDX);

	CJob *j;
	static char Buffer[32];

	if (!pDX->m_bSaveAndValidate && NULL != m_pLocalJobSet) {
		// Daten aus dem Job-Set holen und in die Variablen schreiben:
		ASSERT(!m_pLocalJobSet->IsEmpty());
		j = m_pLocalJobSet->GetTemp();
		ASSERT(NULL != j);

		itoa(j->GetID(), Buffer, 10);
		m_InternalID = Buffer;
		SetAmbDlgItem(m_pLocalJobSet, JB_job_number,	m_JobNumber,	j->job_number,	IDC_STATIC_JOBNUMBER);
		SetAmbDlgItem(m_pLocalJobSet, JB_job_file,		m_JobFile,		j->job_file,		IDC_STATIC_JOBFILE);
		SetAmbDlgItem(m_pLocalJobSet, JB_script_file,	m_ScriptFile,	j->script_file,	IDC_STATIC_SCRIPTFILE);
	}

	//{{AFX_DATA_MAP(CCodJobViewGeneral)
	DDX_Text(pDX, IDC_JOBFILE, m_JobFile);
	DDX_Text(pDX, IDC_SCRIPTFILE, m_ScriptFile);
	DDX_Text(pDX, IDC_JOBNUMBER, m_JobNumber);
	DDX_Text(pDX, IDC_INTERNALID, m_InternalID);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCodJobViewGeneral, CCodPropertyPage)
	//{{AFX_MSG_MAP(CCodJobViewGeneral)
		// HINWEIS: Der Klassen-Assistent fügt hier Zuordnungsmakros für Nachrichten ein
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodJobViewGeneral 

/****************************************************************************
** CCodJobView
****************************************************************************/


IMPLEMENT_DYNCREATE(CCodJobView, CCodView)

CCodJobView::CCodJobView()
{
	m_bPropertySheetCreated = false;
}

CCodJobView::~CCodJobView()
{
}

void CCodJobView::DoDataExchange(CDataExchange* pDX)
{
	CCodView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCodJobView)
	//}}AFX_DATA_MAP
}

/*
** Create
**
** Erstellt beim Erzeugen des CodJobView-Objekts die eingebetteten
** Eigenschaftsseiten.
*/
BOOL CCodJobView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	BOOL Result = CCodView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	ASSERT_VALID(m_SplitterFrame);

	m_PropertySheet.AddPage(&m_GeneralPage);
//	m_PropertySheet.AddPage(&m_ScriptPage);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pJobList);

	m_PropertySheet.Create(this, WS_CHILD | WS_VISIBLE);
	m_bPropertySheetCreated = true;

	return Result;
}

BEGIN_MESSAGE_MAP(CCodJobView, CCodView)
	//{{AFX_MSG_MAP(CCodJobView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Diagnose CCodJobView

#ifdef _DEBUG
void CCodJobView::AssertValid() const
{
	CCodView::AssertValid();
}

void CCodJobView::Dump(CDumpContext& dc) const
{
	CCodView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodJobView 

/*
** OnSize
**
** Reagiert auf eine Größenänderung des Views und ändert die 
** Größe des eingebetteten PropertySheets.
*/
void CCodJobView::OnSize(UINT nType, int cx, int cy) 
{
	CCodView::OnSize(nType, cx, cy);

	CRect ClientRect;
	if (m_bPropertySheetCreated) {
		// Wir empfangen diese Meldung schon, bevor das PropertySheet existiert!
		GetClientRect(ClientRect);
		m_PropertySheet.Resize(ClientRect.Size());
	}
}

/*
** OnDraw
**
** Wird benötigt, da sie in der Basisklasse rein virtuell Deklariert wird!
*/
void CCodJobView::OnDraw(CDC* pDC) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
}

bool CCodJobView::IsModified()
{
	// Stefan Mihaila: TODO
	return false;
}

void CCodJobView::LooseChanges()
{
	m_LocalJobSet.ClearModified();
}

void CCodJobView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CQmonntDoc *Doc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(Doc);

	if (0 == lHint || UAVH_JOBS == lHint)
		UpdateJobList();
	else if (UAVH_SELECTION == lHint) // Auswahl im Treeview hat sich geändert.
		UpdateSelection();
	
	UpdateData(FALSE);	// Dialogelemente aus Member-Variablen updaten.
}

/*
** UpdateJobList (private)
**
** 
*/
void CCodJobView::UpdateJobList()
{
	DENTER(GUI_LAYER, "CCodJobView::UpdateJobList");

	ASSERT_VALID(m_SplitterFrame);

	CNodeInfoSet *pSelection = m_SplitterFrame->GetTreeSelection();
	ASSERT(NULL != pSelection);

	DPRINTF(("******* CCodJobView::UpdateJobList Selection"));
	pSelection->DebugOut();
	m_LocalJobSet.DebugOut();

	if (m_LocalJobSet.IsModified()) {
		// Datensatz wurde geändert. Aged-Markierung setzten und nichts aktualisieren!
		SetAged();
	} 
	else {
		// Daten können einfach übernommen werden!
		UpdateSelection();
	}

	DEXIT;
}

void CCodJobView::UpdateSelection()
{
	ASSERT_VALID(m_SplitterFrame);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);

	// Selektion anfordern:
	CNodeInfoSet *pSelection = m_SplitterFrame->GetTreeSelection();
	ASSERT(NULL != pSelection);

	// Job Set vom Dokument anfordern und mit altem austauschen:
	pDoc->GetJobSet(&m_LocalJobSet, pSelection);

	DPRINTF(("***************** JobSet *********************"));
	m_LocalJobSet.DebugOut();
	m_GeneralPage.SetLocalJobSet(&m_LocalJobSet);

	CPropertyPage *ActivePage = m_PropertySheet.GetActivePage();
	ASSERT_VALID(ActivePage);
	ActivePage->OnInitDialog();

	SetModified(m_LocalJobSet.IsModified());
	SetLocal(m_LocalJobSet.IsLocal());
	SetAged(false);
}
