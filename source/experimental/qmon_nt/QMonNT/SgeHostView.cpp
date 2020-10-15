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
// CodHostView.cpp: Implementierungsdatei
//
// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "Debug.h"
#include "QmonNTDoc.h"
#include "CodHostView.h"

extern "C" {
#include "cod_hostL.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodHostView

IMPLEMENT_DYNCREATE(CCodHostView, CCodView)

CCodHostView::CCodHostView()
{
	m_bPropertySheetCreated = false;
}

CCodHostView::~CCodHostView()
{
}

BEGIN_MESSAGE_MAP(CCodHostView, CCodView)
	//{{AFX_MSG_MAP(CCodHostView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Zeichnung CCodHostView 

void CCodHostView::OnDraw(CDC* pDC)
{
	CDocument *pDoc = GetDocument();
	// ZU ERLEDIGEN: Code zum Zeichnen hier einfügen
}

/////////////////////////////////////////////////////////////////////////////
// Diagnose CCodHostView

#ifdef _DEBUG
void CCodHostView::AssertValid() const
{
	CCodView::AssertValid();
}

void CCodHostView::Dump(CDumpContext& dc) const
{
	CCodView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodHostView 

bool CCodHostView::IsModified()
{
	// Stefan Mihaila: TODO
	return false;
}

void CCodHostView::LooseChanges()
{
	m_LocalHostSet.ClearModified();
}

/*
** Create (virtual, public)
**
** Erstellt beim Erzeugen des CodHostView-Objekts die eingebetteten
** Eigenschaftsseiten.
*/
BOOL CCodHostView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	BOOL Result = CCodView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	ASSERT_VALID(m_SplitterFrame);

	m_PropertySheet.AddPage(&m_GeneralPage);
	m_PropertySheet.AddPage(&m_QueuePage);
	// >>> Hier Code für weitere Eigenschaftsseiten einfügen
	
	m_PropertySheet.Create(this, WS_CHILD | WS_VISIBLE);
	m_bPropertySheetCreated = true;

	m_PropertySheet.EnableButtonSend(false);
	m_PropertySheet.EnableButtonRevert(false);

	m_GeneralPage.SetParentView(this);
	m_QueuePage.SetParentView(this);
	// >>> Hier Code für weitere Eigenschaftsseiten einfügen

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pHostList);
	m_QueuePage.SetDocument(pDoc);

	m_DlgBar.Create(this, IDD_SHEETDLGBAR, CBRS_BOTTOM, 0xe800);

	return Result;
}

/*
** OnSize (protected)
**
** Reagiert auf eine Größenänderung des Views und ändert die 
** Größe des eingebetteten PropertySheets.
*/
void CCodHostView::OnSize(UINT nType, int cx, int cy) 
{
	CCodView::OnSize(nType, cx, cy);
	
	if (m_bPropertySheetCreated) {
		// Wir empfangen diese Meldung schon, bevor das PropertySheet existiert!
		static CRect ClientRect;

		GetClientRect(ClientRect);
		m_PropertySheet.Resize(ClientRect.Size());
	}
}

/*
** OnUpdate
**
** Wird aufgerufen, wenn das Dokument neue Daten hat.
*/

void CCodHostView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CQmonntDoc *Doc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(Doc);

	if (0 == lHint || UAVH_HOSTS == lHint)
		UpdateHostList();
	else if (UAVH_SELECTION == lHint) // Auswahl im Treeview hat sich geändert
		UpdateSelection(); 
	
	UpdateData(FALSE);	// Dialogelemente aus Member-Variablen updaten
}

/*
** UpdateHostList (private)
**
** 
*/
void CCodHostView::UpdateHostList()
{
	ASSERT_VALID(m_SplitterFrame);

	CNodeInfoSet *pSelection = m_SplitterFrame->GetTreeSelection();
	ASSERT(NULL != pSelection);

	DPRINTF(("******* CCodHostView::UpdateHostList Selection"));
	pSelection->DebugOut();
	m_LocalHostSet.DebugOut();

	if (m_LocalHostSet.IsModified()) {
		// Datensatz wurde geändert. Aged-Markierung setzten und nichts aktualisieren!
		SetAged();
	} 
	else {
		// Daten können einfach übernommen werden!
		UpdateSelection();
	}
}

/*
** UpdateSelection (private)
**
** Fordert vom TreeView die aktuelle Auswahl an und baut anhand dieser Auswahl
** die lokalen Datenstrukturen neu auf.
** HINWEIS: Es muß hier nicht geprüft werden, ob Daten modifiziert worden sind,
** dies wurde bereits gemacht, bevor die Selektionsänderung durchgeführt wurde!
*/
void CCodHostView::UpdateSelection()
{
	ASSERT_VALID(m_SplitterFrame);

	CQmonntDoc *Doc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(Doc);

	// Selektion anfordern:
	CNodeInfoSet *pSelection = m_SplitterFrame->GetTreeSelection();
	ASSERT(NULL != pSelection);
	// m_SplitterFrame wird von CCodView geerbt!

	// Host Set vom Dokument anfordern und mit altem austauschen:
	Doc->GetHostSet(&m_LocalHostSet, pSelection);

	DPRINTF(("***************** HostSet **********************"));
	m_LocalHostSet.DebugOut();

	m_GeneralPage.m_pLocalHostSet = &m_LocalHostSet;
	m_QueuePage.m_pLocalHostSet = &m_LocalHostSet;
	// >>> Hier Code für weitere Eigenschaftsseiten einfügen

	CPropertyPage *ActivePage = m_PropertySheet.GetActivePage();
	ASSERT_VALID(ActivePage);
	ActivePage->OnInitDialog();

	SetModified(m_LocalHostSet.IsModified());
	SetLocal(m_LocalHostSet.IsLocal());
	SetAged(false);
}

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CCodHostViewGeneral 

IMPLEMENT_DYNCREATE(CCodHostViewGeneral, CCodPropertyPage)

CCodHostViewGeneral::CCodHostViewGeneral() : CCodPropertyPage(CCodHostViewGeneral::IDD)
{
	m_DDXRunning	= true;
	m_pLocalHostSet = NULL;

	//{{AFX_DATA_INIT(CCodHostViewGeneral)
	m_HostName = _T("");
	m_InternalID = _T("");
	m_Processors = _T("");
	m_Realname = _T("");
	//}}AFX_DATA_INIT
}

CCodHostViewGeneral::~CCodHostViewGeneral()
{
}

void CCodHostViewGeneral::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(NULL != pDX);

	CHost *h;
	char Buffer[32];

	m_DDXRunning = true;
	CCodPropertyPage::DoDataExchange(pDX);
	
	if (!pDX->m_bSaveAndValidate && NULL != m_pLocalHostSet) {
		// Daten aus dem Host-Set holen und in die Variablen schreiben:
		ASSERT(!m_pLocalHostSet->IsEmpty());
		h = m_pLocalHostSet->GetTemp();
		ASSERT(NULL != h);

		itoa(h->GetID(), Buffer, 10);
		m_InternalID = Buffer;

		SetAmbDlgItem(m_pLocalHostSet, EH_name,			m_HostName,		h->hostname,		IDC_STATIC_HOSTNAME);
		SetAmbDlgItem(m_pLocalHostSet, EH_real_name,	m_Realname,		h->realname,		IDC_STATIC_REALNAME);
		SetAmbDlgItem(m_pLocalHostSet, EH_processors,	m_Processors,	h->processors,		IDC_STATIC_PROCESSORS);
		// >>> Code für neue Felder hier einfügen
	}

	//{{AFX_DATA_MAP(CCodHostViewGeneral)
	DDX_Text(pDX, IDC_HOSTNAME, m_HostName);
	DDX_Text(pDX, IDC_INTERNALID, m_InternalID);
	DDX_Text(pDX, IDC_PROCESSORS, m_Processors);
	DDX_Text(pDX, IDC_REALNAME, m_Realname);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate && NULL != m_pLocalHostSet) {
		ASSERT(!m_pLocalHostSet->IsEmpty());
		h = m_pLocalHostSet->GetTemp();
		ASSERT(NULL != h);

		// Hostname nicht übernehmen, da read-only!
		//	h->realname = m_Realname;
		h->processors = atol(m_Processors);
		// >>> Code für neue Felder hier einfügen
	}
	m_DDXRunning = false;
}


BEGIN_MESSAGE_MAP(CCodHostViewGeneral, CCodPropertyPage)
	//{{AFX_MSG_MAP(CCodHostViewGeneral)
		// HINWEIS: Der Klassen-Assistent fügt hier Zuordnungsmakros für Nachrichten ein
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodHostViewGeneral 

BOOL CCodHostViewGeneral::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD notificationCode = HIWORD(wParam);
	WORD ItemID			  = LOWORD(wParam);

	if (!m_DDXRunning && EN_CHANGE == notificationCode) {
		ASSERT(NULL != m_pLocalHostSet);

		switch(ItemID) {
			case IDC_REALNAME:		
				m_pLocalHostSet->SetModified(EH_name);			
				break;

			case IDC_PROCESSORS:	
				m_pLocalHostSet->SetModified(EH_processors);	
				break;

			// >>> Code für neue Felder hier einfügen
		}

		SetModified(m_pLocalHostSet);
	}

	return CCodPropertyPage::OnCommand(wParam, lParam);
}
