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
// CodTreeView.cpp: Implementierungsdatei
//
// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "QmonntDoc.h"
#include "CodTreeView.h"
#include "CodJobView.h"
#include "MainFrm.h"
#include "SplitterFrame.h"
#include "Messages.h"
#include "NodeInfo.h"
#include "Debug.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodTreeView

IMPLEMENT_DYNCREATE(CCodTreeView, CFormView)

CCodTreeView::CCodTreeView()
	: CFormView(CCodTreeView::IDD)
{
	TRACE("CCodTreeView::CCodTreeView\n");
	//{{AFX_DATA_INIT(CCodTreeView)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT

	m_SplitterFrame	   = NULL;
	m_bTreeCtrlCreated = false;
}

CCodTreeView::~CCodTreeView()
{
	TRACE("CCodTreeView::~CCodTreeView\n");
}

void CCodTreeView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCodTreeView)
	DDX_Control(pDX, IDC_TREE, m_TreeCtrl);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCodTreeView, CFormView)
	//{{AFX_MSG_MAP(CCodTreeView)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, OnTreeSelchangedTree)
	ON_NOTIFY(TVN_SELCHANGING, IDC_TREE, OnTreeSelchangingTree)
	ON_NOTIFY(TVN_DELETEITEM, IDC_TREE, OnTreeDeleteitemTree)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_TREE, OnRclickTree)
	ON_COMMAND(ID_TREEVIEWQUEUE_DELETEQUEUE, OnDeleteQueue)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Diagnose CCodTreeView

#ifdef _DEBUG
void CCodTreeView::AssertValid() const
{
	CFormView::AssertValid();
}

void CCodTreeView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCodTreeView 

BOOL CCodTreeView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	BOOL Result = CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	m_TreeCtrl.Create(WS_CHILD | WS_VISIBLE | TVS_HASLINES | 
		TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_HASBUTTONS,
		0, rect, this, IDC_TREE);
	
	m_SplitterFrame = dynamic_cast<CSplitterFrame *>(GetParentFrame());
	ASSERT_VALID(m_SplitterFrame);

	m_bTreeCtrlCreated = true;

	return Result;
}

void CCodTreeView::OnInitialUpdate() 
{
	TreeIcons.Create(IDB_TREEICONS, 16, 16, RGB(255, 0, 255));
	m_TreeCtrl.SetImageList(&TreeIcons, TVSIL_NORMAL);

	// Overlay-Images festlegen:
	TreeIcons.SetOverlayImage(6,  1);
	TreeIcons.SetOverlayImage(7,  2);
	TreeIcons.SetOverlayImage(8,  3);
	TreeIcons.SetOverlayImage(9,  4);
	TreeIcons.SetOverlayImage(10, 5);

	CString	ItemName;

	ItemName.LoadString(IDS_QUEUES);
	HTREEITEM CurrentItem = m_TreeCtrl.InsertItem(TVIF_TEXT |
		TVIF_IMAGE | TVIF_SELECTEDIMAGE, ItemName, 3, 3, 0, 0, NULL,
		NULL, TVI_LAST);
	ASSERT(NULL != CurrentItem);
	CNodeInfo *NodeInfo = new CNodeInfo(NI_QUEUEROOT, 0, "");
	ASSERT(NULL != NodeInfo);
	m_TreeCtrl.SetItemData(CurrentItem,	LPARAM(NodeInfo));

	ItemName.LoadString(IDS_JOBS);
	CurrentItem = m_TreeCtrl.InsertItem(TVIF_TEXT | TVIF_IMAGE | 
		TVIF_SELECTEDIMAGE, ItemName, 0, 0, 0, 0, NULL,
		NULL, TVI_LAST);
	ASSERT(NULL != CurrentItem);
	NodeInfo = new CNodeInfo(NI_JOBROOT, 0, "");
	ASSERT(NULL != NodeInfo);
	m_TreeCtrl.SetItemData(CurrentItem,	LPARAM(NodeInfo));

	ItemName.LoadString(IDS_HOSTS);
	CurrentItem = m_TreeCtrl.InsertItem(TVIF_TEXT | TVIF_IMAGE |
		TVIF_SELECTEDIMAGE, ItemName, 10, 10, 0, 0, NULL,
		NULL, TVI_LAST);
	ASSERT(NULL != CurrentItem);
	NodeInfo = new CNodeInfo(NI_HOSTROOT, 0, "");
	ASSERT(NULL != NodeInfo);
	m_TreeCtrl.SetItemData(CurrentItem,	LPARAM(NodeInfo));

	ItemName.LoadString(IDS_COMPLEXES);
	CurrentItem = m_TreeCtrl.InsertItem(TVIF_TEXT | TVIF_IMAGE |
		TVIF_SELECTEDIMAGE, ItemName, 10, 10, 0,0, NULL,
		NULL, TVI_LAST);
	ASSERT(NULL != CurrentItem);
	NodeInfo = new CNodeInfo(NI_COMPLEXROOT, 0, "");
	m_TreeCtrl.SetItemData(CurrentItem,	LPARAM(NodeInfo));

	// >>> Hier Code für neue Datentypen einfügen

	CRect ClientRect;
	GetClientRect(&ClientRect);
	CSize ClientSize = ClientRect.Size();
	m_TreeCtrl.MoveWindow(0, 0, ClientSize.cx, ClientSize.cy);
	SetScrollSizes(MM_TEXT, CSize(0, 0));

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	
	// Prepare a 'MessageView' in the right, to display connection messages.
	pDoc->SetMessage(IDS_MSG_CONTACTINGCODINE);
	CMainFrame *MainFrame = dynamic_cast<CMainFrame *>(AfxGetMainWnd());
	ASSERT_VALID(MainFrame);
	MainFrame->SetStatusText(IDS_MSG_RECV);

	// Stefan Mihaila: FIXME: The display of the 'MessageView' in the right 
	// should be made in the body of 'CQmonntDoc::OnEditRefresh()'
	// In this case, you must ensure that, after you'll have 
	// 'CDocument::IsDataAvailable()' true, the proper view will be displayed
	// in the right

	pDoc->OnEditRefresh();
	
	// >>> Hier Code für neue Datentypen einfügen
}

/*
** OnUpdate
**
** Wird aufgerufen, wenn sich die Daten im Dokument geändert haben.
** Da der TreeView der einzige View ist, der immer angezeigt wird, muß hier
** dafür gesorgt werden, daß ggf. auf der rechten Seite des Splitter-Windows
** ein anderer View eingeblendet wird.
*/
void CCodTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	DENTER(GUI_LAYER, "CCodTreeView::OnUpdate");

	ASSERT_VALID(m_SplitterFrame);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);

	if (UAVH_NOMESSAGE == lHint)
		m_SplitterFrame->SetRightView(RVT_NOMESSAGE);

	if (0 == lHint || UAVH_MESSAGE == lHint)
		if (pDoc->HasMessage())
			m_SplitterFrame->SetRightView(RVT_MESSAGE);
		else
			pDoc->SetMessage(IDS_MSG_NOTHINGTODISPLAY);
	
	if (0 == lHint || UAVH_QUEUES == lHint)
		UpdateQueueList();
	
	if (0 == lHint || UAVH_JOBS == lHint)
		UpdateJobList();
	
	if (0 == lHint || UAVH_HOSTS == lHint) 
		UpdateHostList();
	
	if (0 == lHint || UAVH_COMPLEXES == lHint)
		UpdateComplexList();
	
	// >>> Code für neue Datentypen hier einfügen.

	DEXIT;
}

/*
** UpdateQueueList (private)
**
** Aktualisiert die Liste der Queues anhand der Daten im Dokument,
*/
void CCodTreeView::UpdateQueueList()
{
	HTREEITEM				QueueRootItem, CurrentItem;
//	CQueueList::iterator	Iterator;
//	CNodeInfoSet			SavedNodeInfoSet;
	CNodeInfo				*NodeInfo;
//	bool					FirstItem = true;
	static CNodeInfo		EMPTY_NI_QUEUEROOT(NI_QUEUEROOT, 0, "");

/*	Doc = dynamic_cast<CQmonntDoc*>(GetDocument());
	ASSERT(Doc != NULL);
*/
/*	if(Doc->m_pQueueList == NULL) {
		DEXIT;
		return;
	}
*/
/*	SavedNodeInfoSet = m_NodeInfoSet;


	// Alle Queues in QueueListe als 'nicht angezeigt' markieren:
	for(Iterator = Doc->m_pQueueList->begin(); Iterator != Doc->m_pQueueList->end(); Iterator++) {
		(*Iterator).SetFlag(CO_NOTDISPLAYED);
	}

	// Alle TreeItems, die Queues darstellen, durchlaufen:
	CurrentItem = m_TreeCtrl.GetRootItem();
	while(CurrentItem) {
		NodeInfo = (CNodeInfo*)m_TreeCtrl.GetItemData(CurrentItem);
		if(NodeInfo == NULL) {
			// Tree-Eintrag wurde bereits von der Queue-Liste abgekoppelt. Nichts machen.
			CurrentItem = m_TreeCtrl.GetNextItem(CurrentItem);
		} else if(NodeInfo->m_Type == NI_QUEUE) {
			// Versuchen, Queue in Liste zu finden:
			Iterator = Doc->m_pQueueList->FindByID(NodeInfo->m_ID);
			if(Iterator == Doc->m_pQueueList->end()) {
				// Queue, die im Tree ist, aber nicht in der QueueListe aus Tree löschen,
				// oder Info-Type auf Unknown setzen, falls Queue ausgewählt ist.
				// Dabei Markierung 'nicht angezeigt' der Queue aufheben.
				if(m_TreeCtrl.GetItemState(CurrentItem, TVIS_SELECTED) == TVIS_SELECTED) {
					NodeInfo->m_Type = NI_UNKNOWN;	// Ausgewählt, Info ändern
					m_TreeCtrl.SetItemState(CurrentItem, INDEXTOOVERLAYMASK(1), TVIS_OVERLAYMASK );
					CurrentItem = m_TreeCtrl.GetNextItem(CurrentItem);
				} else {
					HTREEITEM NextItem;	// Nicht ausgewählt: löschen.
					NextItem = m_TreeCtrl.GetNextItem(CurrentItem);
					m_TreeCtrl.DeleteItem(CurrentItem);
					CurrentItem = NextItem;
				}
			} else {
				// Queue, die im Tree und in der QueueListe ist, Info-Zeiger aktualisieren:
				// Dabei Markierung 'nicht angezeigt' der Queue aufheben.

				// INFO: Infozeiger muß nicht aktualisiert werden, da nur ID gespeichert wird, die
				// ändert sich aber nicht!!
				(*Iterator).ClearFlag(CO_NOTDISPLAYED);
				CurrentItem = m_TreeCtrl.GetNextItem(CurrentItem);
			}
		} else {
			CurrentItem = m_TreeCtrl.GetNextItem(CurrentItem);
		}
	}
*/
	// Root-Item des Queue-Zweiges suchen.
	// TODO: Wenn eine Queue in mehreren Zweigen eingeblendet werden soll, muss hier etwas mehr
	// gemacht werden.....
	CurrentItem = m_TreeCtrl.GetRootItem();
	while (NULL != CurrentItem) {
		NodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(CurrentItem));
		ASSERT(NULL != NodeInfo);

		if (EMPTY_NI_QUEUEROOT == *NodeInfo)
			break;
		
		CurrentItem = m_TreeCtrl.GetNextSiblingItem(CurrentItem);
	}

	QueueRootItem = CurrentItem;
	AddQueues(QueueRootItem);

	// Alle Queues in der Queue-Liste, die die Markierung 'nicht angezeigt' noch besitzen,
	// zusätzlich in die passenden Queue-Root-Zweige des Trees einfügen.
	// Dabei Markierung 'nicht angezeigt' der Queue aufheben.
/*	for(Iterator = Doc->m_pQueueList->begin(); Iterator != Doc->m_pQueueList->end(); Iterator++) {
		if((*Iterator).IsFlagSet(CO_NOTDISPLAYED)) {
			// Einfügen:
			CurrentItem = m_TreeCtrl.InsertItem(TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE, (*Iterator).qname,
				4, 5, 0, 0, NULL, QueueRootItem, TVI_LAST);
			NodeInfo = new CNodeInfo(NI_QUEUE, (*Iterator).GetID(), (*Iterator).qname);
			m_TreeCtrl.SetItemData(CurrentItem, (ULONG)NodeInfo);
			(*Iterator).ClearFlag(CO_NOTDISPLAYED);
		}
	}

	DPRINTF(("*********** UpdateQueueList: m_NodeInfoSet *********"));
	m_NodeInfoSet.DebugOut();
	DPRINTF(("<<<<<<<<<<<<"));
*/	
	m_TreeCtrl.RedrawWindow();
}

/*
** UpdateJobList
**
** Aktualisiert die Liste der Jobs anhand der Daten im Dokument.
*/
void CCodTreeView::UpdateJobList()
{
	HTREEITEM				JobRootItem, CurrentItem;
	CJobList::iterator		Iterator;
	CNodeInfo				*NodeInfo;
	static CNodeInfo		EMPTY_NI_JOBROOT(NI_JOBROOT, 0, "");

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pJobList);

	// Zweig der Jobs im TreeCtrl ermitteln und löschen:
	CurrentItem = m_TreeCtrl.GetRootItem();
	while (NULL != CurrentItem) {
		NodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(CurrentItem));
		ASSERT(NULL != NodeInfo);

		if (EMPTY_NI_JOBROOT == *NodeInfo)
			break;
		
		CurrentItem = m_TreeCtrl.GetNextSiblingItem(CurrentItem);
	}

	JobRootItem = CurrentItem;
	AddJobs(JobRootItem);

/*	NextItem = m_TreeCtrl.GetChildItem(CurrentItem);
	while((CurrentItem = NextItem) != NULL) {
		NextItem = m_TreeCtrl.GetNextSiblingItem(CurrentItem);
		m_TreeCtrl.DeleteItem(CurrentItem);
	}

	Iterator = Doc->m_pJobList->begin();
	while(Iterator != Doc->m_pJobList->end()) {
		CJob j;
		j = *Iterator;

		// Item einfügen:
		itoa(j.jobnumber, sJobNumber, 10);
		CurrentItem = m_TreeCtrl.InsertItem(TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE, sJobNumber,
			1, 2, 0, 0, NULL, JobRootItem, TVI_LAST);

		NodeInfo = new CNodeInfo(NI_JOB, j.GetID(), sJobNumber);
		m_TreeCtrl.SetItemData(CurrentItem, (ULONG)NodeInfo);

		m_NodeInfoSet.DebugOut();
		if(m_NodeInfoSet.Contains(j.GetID())) {
			m_TreeCtrl.SelectItemEx(CurrentItem);
		}

		Iterator++;
	}
*/	
	m_TreeCtrl.RedrawWindow();
}

/*
** UpdateHostList (private)
**
** Aktualisiert die Liste der Hosts anhand der Daten im Dokument.
*/
void CCodTreeView::UpdateHostList()
{
	HTREEITEM				hHostRootItem, hCurrentItem;
	CHostList::iterator		Iterator;
	CNodeInfo				*NodeInfo;
	static CNodeInfo		EMPTY_NI_HOSTROOT(NI_HOSTROOT, 0, "");

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pHostList);

	// Zweig der Hosts im TreeCtrl ermitteln und löschen:

	hCurrentItem = m_TreeCtrl.GetRootItem();
	while (NULL != hCurrentItem) {
		NodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(hCurrentItem));
		ASSERT(NULL != NodeInfo);

		if (EMPTY_NI_HOSTROOT == *NodeInfo)
			break;
		
		hCurrentItem = m_TreeCtrl.GetNextSiblingItem(hCurrentItem);
	}

	hHostRootItem = hCurrentItem;
	AddHosts(hHostRootItem);

/*	NextItem = m_TreeCtrl.GetChildItem(CurrentItem);
	while((CurrentItem = NextItem) != NULL) {
		NextItem = m_TreeCtrl.GetNextSiblingItem(CurrentItem);
		m_TreeCtrl.DeleteItem(CurrentItem);
	}

	Iterator = Doc->m_pHostList->begin();
	while(Iterator != Doc->m_pHostList->end()) {
		CHost h;
		h = *Iterator;
		ASSERT(h.GetID() == (*Iterator).GetID());

		// Item einfügen:
		CurrentItem = m_TreeCtrl.InsertItem(TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE, h.hostname,
			11, 12, 0, 0, NULL, HostRootItem, TVI_LAST);

		NodeInfo = new CNodeInfo(NI_HOST, h.GetID(), h.hostname);
		m_TreeCtrl.SetItemData(CurrentItem, (ULONG)NodeInfo);

		AddHostQueues(CurrentItem, h.hostname);

		m_NodeInfoSet.DebugOut();
		if(m_NodeInfoSet.Contains(h.GetID())) {
			m_TreeCtrl.SelectItemEx(CurrentItem);
		}

		Iterator++;
	}
*/	
	m_TreeCtrl.RedrawWindow();
}

/*
** UpdateComplexList (private)
**
** Aktualisiert die Liste der Hosts anhand der Daten im Dokument.
*/
void CCodTreeView::UpdateComplexList()
{
	HTREEITEM				ComplexRootItem, CurrentItem;
	CComplexList::iterator	Iterator;
	CNodeInfo				*NodeInfo;
	static CNodeInfo		EMPTY_NI_COMPLEXROOT(NI_COMPLEXROOT, 0, "");

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pComplexList);

	CurrentItem = m_TreeCtrl.GetRootItem();
	while (NULL != CurrentItem) {
		NodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(CurrentItem));
		ASSERT(NULL != NodeInfo);

		if (EMPTY_NI_COMPLEXROOT == *NodeInfo)
			break;
		
		CurrentItem = m_TreeCtrl.GetNextSiblingItem(CurrentItem);
	}

	ComplexRootItem = CurrentItem;
	AddComplexes(ComplexRootItem);

	m_TreeCtrl.RedrawWindow();
}

/*
** OnTreeSelchangedTree
**
** Reagiert auf eine Änderung der Auswahl im TreeView. Baut
** das NodeInfoSet entsprechend auf und ab.
*/
void CCodTreeView::OnTreeSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	DENTER(GUI_LAYER, "CCodTreeView::OnTreeSelchangedTree");

	NM_TREEVIEW *pNMTreeView = reinterpret_cast<NM_TREEVIEW *>(pNMHDR);
	ASSERT(NULL != pNMTreeView);
	ASSERT(NULL != pResult);

	*pResult = 0;

	// Stefan Mihaila: there is a problem: when the selection in the
	// tree changes, you will receive TWO 'OnTreeSelchangedTree' - one for
	// the 'olditem' selection change (sel -> not sel) and the other for
	// the 'newitem' selection change (not sel -> sel). So the code would 
	// be executed twice for each selection change. 
	// This behaviour is unwanted and time consuming
	// So, I suggest uncomment my 'if' code and make all
	// the modifications needed to display the view from the right properly
	// (Normally, it should be such kind of 'if' in OnTreeSelchanging, too.
	/*
	if (TVIS_SELECTED  != (pNMTreeView->itemNew.state & TVIS_SELECTED) ||
		TVIS_SELECTED  == (pNMTreeView->itemOld.state & TVIS_SELECTED))
	{
		DEXIT;
		return;
	}
	*/
	
	ASSERT_VALID(m_SplitterFrame);
	
	m_NodeInfoSet.clear();

	HTREEITEM CurrentItem, NextItem;
	CNodeInfo *NodeInfo;

	// NodeInfoSet neu aufbauen:
	CurrentItem = m_TreeCtrl.GetFirstSelectedItem();
	while (NULL != CurrentItem) {
		NodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(CurrentItem));
		ASSERT(NULL != NodeInfo);
		m_NodeInfoSet.Add(*NodeInfo);
		CurrentItem = m_TreeCtrl.GetNextSelectedItem(CurrentItem);
	}

	// Alle sichtbaren Items durchsuchen und ggf. als gelöscht markierte entfernen:
	CurrentItem = m_TreeCtrl.GetRootItem();
	while (NULL != CurrentItem) {
		NextItem = m_TreeCtrl.GetNextItem(CurrentItem, TVGN_NEXTVISIBLE);
		NodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(CurrentItem));
		ASSERT(NULL != NodeInfo);
		if (NI_UNKNOWN == NodeInfo->m_Type && 
		    TVIS_SELECTED != m_TreeCtrl.GetItemState(CurrentItem, TVIS_SELECTED))
				m_TreeCtrl.DeleteItem(CurrentItem);
		
		CurrentItem = NextItem;
	}

	// Das Rahmenfenster benachrichtigen, daß sich die Auswahl geändert hat. Dabei auch
	// den Typ der ausgewählten Elemente übermitteln:
	m_SplitterFrame->OnSelectionChanged(&m_NodeInfoSet);

	m_TreeCtrl.SetFocus();

	DEXIT;
}

/*
** OnTreeSelchangingTree
**
** Hier wird getestet, ob eine Änderung der Treeselektion
** erlaubt ist. Zuerst wird der View gefragt (über das Rahmenfenster),
** dann wird geprüft, ob verschiedene Typen ausgewählt wurden.
*/
void CCodTreeView::OnTreeSelchangingTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	DENTER(GUI_LAYER, "CCodTreeView::OnTreeSelchangingTree");

	NM_TREEVIEW *pNMTreeView = reinterpret_cast<NM_TREEVIEW *>(pNMHDR);
	ASSERT(NULL != pNMTreeView);
	ASSERT(NULL != pResult);

	ASSERT_VALID(m_SplitterFrame);

	bool Modified = m_SplitterFrame->IsCurrentViewModified();
	if (Modified && IDCANCEL == AfxMessageBox(IDS_LOSECHANGES, 
		MB_OKCANCEL | MB_DEFBUTTON2))
	{
		// Änderung wird nicht erlaubt!
		*pResult = 1;
		DEXIT;
		return;
	}
	if (Modified)
		m_SplitterFrame->LooseChanges();

	// Änderung wird erlaubt, ggf. wird vorherige Auswahl aufgehoben	
	*pResult = 0;

	CNodeInfo *NodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(pNMTreeView->itemNew.hItem));
	ASSERT(NULL != NodeInfo);

	TRACE("NodeSize: %d\n", m_NodeInfoSet.size());
	if (TVIS_SELECTED == (pNMTreeView->itemNew.state & TVIS_SELECTED))
		m_NodeInfoSet.Add(*NodeInfo);
	
	DPRINTF(("Check if ambiguous selection"));
	if (m_NodeInfoSet.ContainsOtherType(NodeInfo->m_Type)) {
		// Vorherige Auswahl löschen:
		m_TreeCtrl.ClearSelection();
		m_NodeInfoSet.clear();
	}

	m_NodeInfoSet.DebugOut();
	DEXIT;
}

/*
** OnTreeDeleteitemTree
**
** Wird aufgerufen, wenn ein Item aus dem Tree gelöscht wird.
** Gibt das TreeInfo-Objekt frei.
*/
void CCodTreeView::OnTreeDeleteitemTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW *pNMTreeView = reinterpret_cast<NM_TREEVIEW *>(pNMHDR);
	ASSERT(NULL != pNMTreeView);
	ASSERT(NULL != pResult);

	// TreeInfo-Objekt freigeben:
	CNodeInfo *NodeInfo = reinterpret_cast<CNodeInfo *>(pNMTreeView->itemOld.lParam);
	ASSERT(NULL != NodeInfo);
	delete NodeInfo;
	NodeInfo = NULL;
	
	*pResult = 0;
}

/*
** GetTreeSelection
**
** Liefert einen Zeiger auf das NodeInfoSet zurück, das die aktuelle Auswahl
** im Tree darstellt.
*/
CNodeInfoSet *CCodTreeView::GetTreeSelection()
{
	return &m_NodeInfoSet;
}

/*
** OnSize
**
** Passt den Scrollbereich und die Größe des TreeCtrls an die 
** Client-Größe an.
*/
void CCodTreeView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);

	if (m_bTreeCtrlCreated) {
		m_TreeCtrl.MoveWindow(0, 0, cx, cy);
		SetScrollSizes(MM_TEXT, CSize(0, 0));
	}
}


// FIXME: Bei allen Funktionen, die einen Tree-Zweig aktualisieren tritt
// folgendes Problem auf: Fall ein Objekt ausgewählt ist, das bereits 
// gelöscht wurde und auch sein übergeordnetes Objekt gelöscht wird, wird
// der Knoten für das übergeordnete Objekt entfernt (da es nicht direkt
// ausgewählt ist). Dies führt zu einem Fehler!
// Beispiel: Eine Queue, die einem Host untergeordnet ist, ist ausgewählt.
// Die Queue wird gelöscht, bleibt aber ausgewählt. Wenn nun auch der
// dazugehörige Host gelöscht wird, wird dessen Knoten entfernt, da er
// nicht ausgewählt ist (die Queue ist noch ausgewählt). Der untergeordnete
// Knoten müsste aber laut Konvention noch erhalten bleiben.
// Lösung: Bevor ein Knoten gelöscht wird, muß geprüft werden, ob nicht evtl.
// einer seiner Child-Knoten selektiert ist.

/*
** AddQueues
**
** Fügt alle Queues unterhalb des angegebenen Parent-TreeItems ein.
** Die gesamte Liste von Queues unterhalb dieses Items (nur eine Ebene tief!)
** wird dabei aktualisiert und ggf. nicht mehr vorhandene Items gelöscht.
*/
void CCodTreeView::AddQueues(HTREEITEM hParentItem)
{
	// Für weitere Kommentare siehe Funktion AddHostQueues!
	ASSERT(NULL != hParentItem);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pQueueList);
	
	TagChildItems(hParentItem, NI_QUEUE);

	for (CQueueList::iterator it = pDoc->m_pQueueList->begin(); it != pDoc->m_pQueueList->end(); it++)
		AddQueueNode(hParentItem, &*it);

	DeleteTaggedChildItems(hParentItem, NI_QUEUE);
}

/*
** AddJobs
**
** Fügt alle Jobs unterhalb des angegebenen Parent-TreeItems ein.
** Die gesamte Liste von Jobs unterhalb dieses Items (nur eine Ebene tief!)
** wird dabei aktualisiert und ggf. nicht mehr vorhandene Items gelöscht.
*/
void CCodTreeView::AddJobs(HTREEITEM hParentItem)
{
// Für weitere Kommentare siehe Funktion AddHostQueues!
	ASSERT(NULL != hParentItem);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pJobList);

	TagChildItems(hParentItem, NI_JOB);

	for (CJobList::iterator it = pDoc->m_pJobList->begin(); it != pDoc->m_pJobList->end(); it++)
		AddJobNode(hParentItem, &*it);
	
	DeleteTaggedChildItems(hParentItem, NI_JOB);
}

/*
** AddHosts
**
** Fügt alle Hosts unterhalb des angegebenen Parent-Tree-Items
** ein. Die gesamte Liste von Hosts unterhalb dieses Items (nur eine
** Ebene tief!) wird dabei aktualisiert und ggf. nicht mehr vorhandene
** Items gelöscht.
*/
void CCodTreeView::AddHosts(HTREEITEM hParentItem)
{
// Für weitere Kommentare siehe Funktion AddHostQueues!
	ASSERT(NULL != hParentItem);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pHostList);

	TagChildItems(hParentItem, NI_HOST);

	for (CHostList::iterator it = pDoc->m_pHostList->begin(); it != pDoc->m_pHostList->end(); it++)
		AddHostNode(hParentItem, &*it);
	
	DeleteTaggedChildItems(hParentItem, NI_HOST);
}

/*
** AddComplexes
**
** Fügt alle Hosts unterhalb des angegebenen Parent-Tree-Items
** ein. Die gesamte Liste von Hosts unterhalb dieses Items (nur eine
** Ebene tief!) wird dabei aktualisiert und ggf. nicht mehr vorhandene
** Items gelöscht.
*/
void CCodTreeView::AddComplexes(HTREEITEM hParentItem)
{
	// Für weitere Kommentare siehe Funktion AddHostQueues!
	ASSERT(NULL != hParentItem);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pComplexList);
	
	TagChildItems(hParentItem, NI_COMPLEX);

	for (CComplexList::iterator it = pDoc->m_pComplexList->begin(); it != pDoc->m_pComplexList->end(); it++)
		AddComplexNode(hParentItem, &*it);

	DeleteTaggedChildItems(hParentItem, NI_COMPLEX);
}

/*
** AddHostQueues
**
** Fügt alle zum angegebenen Host gehörigen Queues unterhalb des angegebenen
** Parent-TreeItems ein. Die gesamte Liste von Queues unterhalb dieses Items
** (nur eine Ebene tief!) wird dabei aktualisiert und ggf. nicht mehr vorhandene
** Items gelöscht.
*/
void CCodTreeView::AddHostQueues(HTREEITEM hParentItem, CString Hostname)
{
	ASSERT(NULL != hParentItem);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pQueueList);
	
	// Alle TreeItems, die unterhalb des angegebenen Parent-Items liegen (nur eine 
	// Ebene tief), als 'nicht bearbeitet' markieren:
	TagChildItems(hParentItem, NI_QUEUE);

	// Objekte einzeln einhängen, falls es noch nicht im Tree existiert. Die Markierung
	// 'nicht angezeigt' wird dabei vom Objekt entfernt und das Tag vom TreeItem gelöscht:
	for (CQueueList::iterator it = pDoc->m_pQueueList->begin(); it != pDoc->m_pQueueList->end(); it++)
		if (0 == it->qhostname.CompareNoCase(Hostname))
			AddQueueNode(hParentItem, &*it);
	
	// Alle TreeItems, deren Markierung 'nicht bearbeitet' noch vorhanden ist, 
	// entfernen, falls sie nicht ausgewählt sind. Bei Ausgewählten Objekten 
	// nur die NodeInfo-Struktur entfernen:
	DeleteTaggedChildItems(hParentItem, NI_QUEUE);
}

void CCodTreeView::AddComplexAtributes(HTREEITEM hParentItem, CComplexAtributeList *AtribList)
{
	ASSERT(NULL != hParentItem);
	ASSERT(NULL != AtribList);

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);

	TagChildItems(hParentItem, NI_COMPLEXATRIBUTE);

	for (CComplexAtributeList::iterator it = AtribList->begin(); it != AtribList->end(); it++)
		AddComplexAtributeNode(hParentItem, &*it);

	DeleteTaggedChildItems(hParentItem, NI_COMPLEXATRIBUTE);
}

/*
** AddQueueNode
**
** Hängt ein TreeItem für die angegebene Queue unterhalb des angegebenen
** Parent-Items ein, falls diese Queue dort noch nicht existiert. Falls es
** bereits existiert, wird nur das Tag in der dazugehörigen NodeInfo-Struktur
** zurückgesetzt.
** Rückgabewert ist das Tree-Item-Handle für die angegebene Queue.
*/
HTREEITEM CCodTreeView::AddQueueNode(HTREEITEM hParentItem, CQueue *pQueue)
{
	ASSERT(NULL != hParentItem);
	ASSERT(NULL != pQueue);

	CNodeInfo	*pNodeInfo;
	// Prüfen, ob das TreeItem schon existiert:
	HTREEITEM hTreeItem = FindTreeChildItemByID(hParentItem, pQueue->GetID());
	
	if (NULL != hTreeItem) {
		// Existiert schon, nur Markierung löschen:
		pNodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(hTreeItem));
		ASSERT(NULL != pNodeInfo);
		pNodeInfo->m_Tag = false;
	} 
	else {
		// Item existiert noch nicht, einhängen:
		hTreeItem = m_TreeCtrl.InsertItem(TVIF_TEXT | TVIF_IMAGE | 
			TVIF_SELECTEDIMAGE, pQueue->qname,
			TVBM_QUEUE, TVBM_QUEUESEL, 0, 0, NULL, hParentItem, TVI_SORT);
		ASSERT(NULL != hTreeItem);

		pNodeInfo = new CNodeInfo(NI_QUEUE, pQueue->GetID(), pQueue->qname);
		ASSERT(NULL != pNodeInfo);
		BOOL Result = m_TreeCtrl.SetItemData(hTreeItem, LPARAM(pNodeInfo));
		ASSERT(FALSE != Result);
	}

	return hTreeItem;
}

/*
** AddJobNode
**
** Hängt ein TreeItem für den angegebenen Job unterhalb des angegebenen
** Parent-Items ein, falls dieser Job dort noch nicht existiert. Falls er
** bereits existiert, wird nur das Tag in der dazugehörigen NodeInfo-Struktur
** zurückgesetzt.
** Rückgabewert ist das Tree-Item-Handle für den angegebenen Job.
*/
HTREEITEM CCodTreeView::AddJobNode(HTREEITEM hParentItem, CJob *pJob)
{
	static char Buffer[16];

	ASSERT(NULL != hParentItem);
	ASSERT(NULL != pJob);

	CNodeInfo *pNodeInfo;
	// Prüfen, ob das TreeItem schon existeirt:
	HTREEITEM hTreeItem = FindTreeChildItemByID(hParentItem, pJob->GetID());
	if (NULL != hTreeItem) {
		// Existiert schon, nur Markierung löschen:
		pNodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(hTreeItem));
		ASSERT(NULL != pNodeInfo);
		pNodeInfo->m_Tag = false;
	} 
	else {
		// Item existiert noch nicht, einhängen:
		ltoa(pJob->job_number, Buffer, 10);
		hTreeItem = m_TreeCtrl.InsertItem(TVIF_TEXT | TVIF_IMAGE | 
			TVIF_SELECTEDIMAGE, Buffer,
			TVBM_JOB, TVBM_JOBSEL, 0, 0, NULL, hParentItem, TVI_SORT);
		ASSERT(NULL != hTreeItem);

		pNodeInfo = new CNodeInfo(NI_JOB, pJob->GetID(), Buffer);
		ASSERT(NULL != pNodeInfo);
		BOOL Result = m_TreeCtrl.SetItemData(hTreeItem, LPARAM(pNodeInfo));
		ASSERT(FALSE != Result);
	}

	return hTreeItem;
}

/*
** AddHostNode
**
** Hängt ein TreeItem für den angegebenen Host unterhalb des angegebenen
** Parent-Items ein, falls dieser Host dort noch nicht existiert. Falls er
** bereits existiert, wird nur das Tag in der dazugehörigen NodeInfo-Struktur
** zurückgesetzt. Die dem Host untergeordneten Queues werden anschließend
** ebenfalls aktualisiert.
** Rückgabewert ist das Tree-Item-Handle für den angegebenen Host.
*/
HTREEITEM CCodTreeView::AddHostNode(HTREEITEM hParentItem, CHost *pHost)
{
	ASSERT(NULL != hParentItem);
	ASSERT(NULL != pHost);

	CNodeInfo *pNodeInfo;
	// Prüfen, ob das TreeItem schon existiert:
	HTREEITEM hTreeItem = FindTreeChildItemByID(hParentItem, pHost->GetID());
	if (NULL != hTreeItem) {
		// Existiert schon, nur Markierung löschen:
		pNodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(hTreeItem));
		ASSERT(NULL != pNodeInfo);
		pNodeInfo->m_Tag = false;
	} 
	else {
		// Item existiert noch nicht, einhängen:
		hTreeItem = m_TreeCtrl.InsertItem(TVIF_TEXT | TVIF_IMAGE | 
			TVIF_SELECTEDIMAGE, pHost->hostname,
			TVBM_HOST, TVBM_HOSTSEL, 0, 0, NULL, hParentItem, TVI_SORT);
		ASSERT(NULL != hTreeItem);

		pNodeInfo = new CNodeInfo(NI_HOST, pHost->GetID(), pHost->hostname);
		ASSERT(NULL != pNodeInfo);
		BOOL Result = m_TreeCtrl.SetItemData(hTreeItem, LPARAM(pNodeInfo));
		ASSERT(FALSE != Result);
	}

	// Queues unterhalb der Hosts anordnen.
	AddHostQueues(hTreeItem, pHost->hostname);

	return hTreeItem;
}

HTREEITEM CCodTreeView::AddComplexNode(HTREEITEM hParentItem, CComplex *pComplex)
{
	ASSERT(NULL != hParentItem);
	ASSERT(NULL != pComplex);

	CNodeInfo *pNodeInfo;
	// Prüfen, ob das TreeItem schon existeirt:
	HTREEITEM hTreeItem = FindTreeChildItemByID(hParentItem, pComplex->GetID());
	if (NULL != hTreeItem) {
		// Existiert schon, nur Markierung löschen:
		pNodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(hTreeItem));
		ASSERT(NULL != pNodeInfo);
		pNodeInfo->m_Tag = false;
	} 
	else {
		// Item existiert noch nicht, einhängen:
		hTreeItem = m_TreeCtrl.InsertItem(TVIF_TEXT | TVIF_IMAGE |
			TVIF_SELECTEDIMAGE, pComplex->name,
			TVBM_COMPLEX, TVBM_COMPLEXSEL, 0, 0, NULL, hParentItem, TVI_SORT);
		ASSERT(NULL != hTreeItem);

		pNodeInfo = new CNodeInfo(NI_COMPLEX, pComplex->GetID(), pComplex->name);
		ASSERT(NULL != pNodeInfo);
		BOOL Result = m_TreeCtrl.SetItemData(hTreeItem, LPARAM(pNodeInfo));
		ASSERT(FALSE != Result);
	}

	// ASSERT(NULL != pComplex->AtributeList);
	AddComplexAtributes(hTreeItem, &pComplex->AtributeList);

	return hTreeItem;
}

HTREEITEM CCodTreeView::AddComplexAtributeNode(HTREEITEM hParentItem, CComplexAtribute *pComplexAtribute)
{
	ASSERT(NULL != hParentItem);
	ASSERT(NULL != pComplexAtribute);

	CNodeInfo *pNodeInfo;
	// Prüfen, ob das TreeItem schon existeirt:
	HTREEITEM hTreeItem = FindTreeChildItemByID(hParentItem, pComplexAtribute->GetID());
	if (NULL != hTreeItem) {
		// Existiert schon, nur Markierung löschen:
		pNodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(hTreeItem));
		ASSERT(NULL != pNodeInfo);
		pNodeInfo->m_Tag = false;
	} 
	else {
		// Item existiert noch nicht, einhängen:
		hTreeItem = m_TreeCtrl.InsertItem(TVIF_TEXT | TVIF_IMAGE |
			TVIF_SELECTEDIMAGE, pComplexAtribute->name,
			TVBM_COMPLEXATRIBUTE, TVBM_COMPLEXATRIBUTESEL, 0, 0, NULL, hParentItem, TVI_SORT);
		ASSERT(NULL != hTreeItem);

		pNodeInfo = new CNodeInfo(NI_COMPLEXATRIBUTE, pComplexAtribute->GetID(), pComplexAtribute->name);
		ASSERT(NULL != pNodeInfo);
		BOOL bResult = m_TreeCtrl.SetItemData(hTreeItem, LPARAM(pNodeInfo));
		ASSERT(FALSE != bResult);
	}

	return hTreeItem;
}

/*
** FindTreeChildItemByID
**
** Sucht im TreeView nach einem Treeitem, dessen Objekt die angegebene ID besitzt.
** Dabei wird nur der Zweig direkt unterhalb des angegebenen Parent-Items abgesucht
** (nur eine Ebene tief!).
*/
HTREEITEM CCodTreeView::FindTreeChildItemByID(HTREEITEM hParentItem, ULONG ID)
{
	ASSERT(NULL != hParentItem);

	CNodeInfo *pNodeInfo;
	HTREEITEM hCurrentItem = m_TreeCtrl.GetNextItem(hParentItem, TVGN_CHILD);
	while (NULL != hCurrentItem) {
		pNodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(hCurrentItem));
		ASSERT(NULL != pNodeInfo);

		if (ID == pNodeInfo->m_ID)
			break;
	
		hCurrentItem = m_TreeCtrl.GetNextItem(hCurrentItem, TVGN_NEXT);
	}

	return hCurrentItem;
}

/*
** TagChildItems
**
** Setzt das Tag-Flag der NodeInfo-Strukturen aller direkt dem angegebenen
** Parent-Item untergeordneten (nur eine Ebene!) Items, die den gewünschten
** Node-Info-Typ besitzen (NI_...). Falls ein Item keine Node-Info-Struktur
** mehr besitzt, wird es ignoriert.
*/
void CCodTreeView::TagChildItems(HTREEITEM hParentItem, int RequestedType)
{
	ASSERT(NULL != hParentItem);

	CNodeInfo *pNodeInfo;
	HTREEITEM hCurrentItem = m_TreeCtrl.GetNextItem(hParentItem, TVGN_CHILD);
	while (NULL != hCurrentItem) {
		pNodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(hCurrentItem));
		ASSERT(NULL != pNodeInfo);

		if (RequestedType == pNodeInfo->m_Type)
			pNodeInfo->m_Tag = true;
		
		hCurrentItem = m_TreeCtrl.GetNextItem(hCurrentItem, TVGN_NEXT);
	}
}

/*
** DeleteTaggedChildItems
**
** Entfernt alle TreeItems direkt unterhalb des angegebenen Parent-Items (nur eine
** Ebene tief), derern NodeInfo-Struktur ein gesetztes Tag-Flag hat und das dem
** angegebenen gewünschten Typ (NI_...) entspricht. Objekte, die keine 
** Node-Info-Struktur mehr besitzen (bzw. Typ Unknown), werden ignorieriert. 
** Objekte, die noch selektiert sind, werden nicht gelöscht, sondern es wird 
** nur die Node-Info-Struktur auf den Typ 'Unbekannt' gesetzt und als 
** gelöscht markiert (Overlay-Image).
*/
void CCodTreeView::DeleteTaggedChildItems(HTREEITEM hParentItem, int RequestedType)
{
	ASSERT(NULL != hParentItem);

	HTREEITEM hCurrentItem = m_TreeCtrl.GetNextItem(hParentItem, TVGN_CHILD);
	HTREEITEM hNextItem;
	CNodeInfo *pNodeInfo;
	while (NULL != hCurrentItem) {
		hNextItem = m_TreeCtrl.GetNextItem(hCurrentItem);

		pNodeInfo = reinterpret_cast<CNodeInfo *>(m_TreeCtrl.GetItemData(hCurrentItem));
		ASSERT(NULL != pNodeInfo);
		if (RequestedType == pNodeInfo->m_Type && pNodeInfo->m_Tag) {
			// Gewünschtes Item gefunden.
			if (TVIS_SELECTED == m_TreeCtrl.GetItemState(hCurrentItem, TVIS_SELECTED)) {
				// Selektiert, nur abklemmen und durch-X-en
				pNodeInfo->m_Type = NI_UNKNOWN;
				m_TreeCtrl.SetItemState(hCurrentItem, INDEXTOOVERLAYMASK(1), TVIS_OVERLAYMASK);
			} 
			else {
				// Nicht selektiert, löschen:
				m_TreeCtrl.DeleteItem(hCurrentItem);
			}
		}
		hCurrentItem = hNextItem;
	}
}

void CCodTreeView::OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ASSERT(NULL != pNMHDR);
	ASSERT(NULL != pResult);

	CMenu menu;
	CPoint ClickPoint;

	if (NI_QUEUE == m_NodeInfoSet.GetType()) {
		ClickPoint = m_TreeCtrl.GetRClickPoint();
		ClientToScreen(&ClickPoint);
		if (menu.LoadMenu(ID_POPUP_MENU)) {
			CMenu *pPopup = menu.GetSubMenu(1);
			ASSERT_VALID(pPopup);
			pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
				ClickPoint.x, ClickPoint.y, this);
		}
	}
	
	*pResult = 0;
}

void CCodTreeView::OnDeleteQueue() 
{
	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pQueueList);

	CQueue	*pQueue;
	CString strPrompt = "Do you really want to delete the following queue(s): ";
	
	for (CNodeInfoSet::iterator	it = m_NodeInfoSet.begin(); it != m_NodeInfoSet.end(); it++) {
		ASSERT(NI_QUEUE == it->m_Type);
		pQueue = &*(pDoc->m_pQueueList->FindByID(it->m_ID));
		ASSERT(NULL != pQueue);
		strPrompt += pQueue->qname + ", ";
	}

	pDoc->DeleteQueues(&m_NodeInfoSet);
}

/* Facut de Stefan Mihaila 
void CCodTreeView::OnDeleteJob() 
{
	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	ASSERT(NULL != pDoc->m_pJobList);

	CQueue	*pJob;
	CString strPrompt = "Do you really want to delete the following queue(s): ";
	
	for (CNodeInfoSet::iterator	it = m_NodeInfoSet.begin(); it != m_NodeInfoSet.end(); it++) {
		ASSERT(NI_QUEUE == it->m_Type);
		pQueue = &*(pDoc->m_pQueueList->FindByID(it->m_ID));
		ASSERT(NULL != pQueue);
		strPrompt += pQueue->qname + ", ";
	}

	pDoc->DeleteQueues(&m_NodeInfoSet);
}
*/
