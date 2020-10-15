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
// SplitterFrame.cpp: Implementierungsdatei
//
// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "SplitterFrame.h"
#include "CodTreeView.h"
#include "CodQueueView.h"
#include "CodJobView.h"
#include "CodHostView.h"
#include "CodMessageView.h"
#include "CodNothingView.h"
#include "CodQueueRootView.h"
#include "CodComplexAtributeView.h"
#include "JobListView.h"
#include "Messages.h"
#include "Debug.h"
#include "QmonntDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSplitterFrame

IMPLEMENT_DYNCREATE(CSplitterFrame, CMDIChildWnd)

CSplitterFrame::CSplitterFrame()
{
	m_pTreeView				= NULL;
	m_pStatusBar			= NULL;
	m_CurrentRightViewType	= RVT_NOTHING;
}

CSplitterFrame::~CSplitterFrame()
{
}

BEGIN_MESSAGE_MAP(CSplitterFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CSplitterFrame)
	ON_WM_GETMINMAXINFO()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CSplitterFrame 

BOOL CSplitterFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.cx = 680;
	cs.cy = 420;
	
	return CMDIChildWnd::PreCreateWindow(cs);
}

/*
** OnCreateClient
**
** Die Funktion wird aufgerufen, wenn das Rahmenfenster erzeugt wird und
** erzeugt einen statischen Splitter (zwei Panes vest verdrahtet) und
** initialisiert sie mit einem Treeview und einem CodMessageView.
*/
BOOL CSplitterFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	ASSERT(NULL != lpcs);
	ASSERT(NULL != pContext);

	// Stefan Mihaila has started here
	ASSERT(NULL == m_pStatusBar);
	CMDIFrameWnd *Parent = GetMDIFrame();
	ASSERT_VALID(Parent);
	m_pStatusBar = dynamic_cast<CStatusBar *>(Parent->GetMessageBar());
	ASSERT_VALID(m_pStatusBar);
	// Stefan Mihaila has finished here

	if (!m_wndSplitter.CreateStatic(this, 1, 2)) {
		TRACE0("Failed to CreateStaticSplitter\n");
		return FALSE;
	}

	if (!m_wndSplitter.CreateView(0, 0,
		RUNTIME_CLASS(CCodTreeView), CSize(130, 50), pContext)) {
			TRACE0("Failed to create first pane (CCodTreeView)\n");
			return FALSE;
	}

	ASSERT(NULL == m_pTreeView);
	m_pTreeView = dynamic_cast<CCodTreeView *>(m_wndSplitter.GetPane(0, 0));
	ASSERT_VALID(m_pTreeView);
	
	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc*>(m_pTreeView->GetDocument());
	ASSERT_VALID(pDoc);

	// Commented by Stefan Mihaila (replaced with the next 'if')
	/*if (pDoc->HasMessage()) {
		// Message-View anzeigen:
		if(!m_wndSplitter.CreateView(0, 1,
			RUNTIME_CLASS(CCodMessageView), CSize(0, 0), pContext)) 
		{
			TRACE0("Failed to create second pane (CCodMessageView)\n");
			return FALSE;
		}
	} 
	else {
		// 'Nothing'-View anzeigen:
		if(!m_wndSplitter.CreateView(0, 1,
			RUNTIME_CLASS(CCodNothingView), CSize(0, 0), pContext)) 
		{
			TRACE0("Failed to create second pane (CCodNothingView)\n");
			return FALSE;
		}
	}*/

	// Stefan Mihaila: Always start with a 'CCodMessageView' in the right pane,
	// to be able to display to user any messages that might occur from
	// 'CQmonntDoc'
	if (!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CCodMessageView), 
		CSize(0, 0), pContext)) 
	{
		TRACE0("Failed to create second pane (CCodMessageView)\n");
		return FALSE;
	}
	m_CurrentRightViewType = RVT_MESSAGE;

	CView *RightView = dynamic_cast<CView *>(m_wndSplitter.GetPane(0, 1));
	ASSERT_VALID(RightView);
	SetActiveView(RightView);

	return TRUE;
}

/*
** SetRightView
**
** Ersetzt den View auf der rechten Seite des SplitterFrames
** durch einen über ViewType spezifizierten View. 
*/
void CSplitterFrame::SetRightView(CRightViewType ViewType)
{
	DENTER(GUI_LAYER, "CSplitterFrame::SetView");

	CView *LeftView = dynamic_cast<CView *>(m_wndSplitter.GetPane(0, 0));
	ASSERT_VALID(LeftView);
	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(LeftView->GetDocument());
	ASSERT_VALID(pDoc);

	bool IsDataAvailable = pDoc->IsDataAvailable();
	ASSERT(IsDataAvailable || RVT_MESSAGE == ViewType || 
		RVT_NOMESSAGE == ViewType || RVT_NOTHING == ViewType);

	m_CurrentRightViewType = ViewType;

	CView *RightView = dynamic_cast<CView *>(m_wndSplitter.GetPane(0, 1));
	ASSERT_VALID(RightView);
	
	CRuntimeClass *OldViewClass = RightView->GetRuntimeClass();
	ASSERT(NULL != OldViewClass);
	CRuntimeClass *NewViewClass = NULL;
	
	TRACE("Set View: %d\n", ViewType);

	// View-Klasse, die angezeigt werden soll, bestimmen:
	switch (ViewType) {
		case RVT_NOTHING:	
			NewViewClass = RUNTIME_CLASS(CCodNothingView);
			break;

		case RVT_MESSAGE:	
			NewViewClass = RUNTIME_CLASS(CCodMessageView);
			break;

		case RVT_NOMESSAGE:	
			if (RUNTIME_CLASS(CCodMessageView) == OldViewClass)
				pDoc->SetMessage(IDS_MSG_NOTHINGTODISPLAY);
			
			DEXIT;
			return;

		case RVT_QUEUEROOT:
			NewViewClass = RUNTIME_CLASS(CCodQueueRootView);
			break;

		case RVT_QUEUE:		
			NewViewClass = RUNTIME_CLASS(CCodQueueView);
			break;

		case RVT_JOBROOT:		
			NewViewClass = RUNTIME_CLASS(CCodJobListView);
			break;

		case RVT_JOB:		
			NewViewClass = RUNTIME_CLASS(CCodJobView);
			break;

		case RVT_HOST:		
			NewViewClass = RUNTIME_CLASS(CCodHostView);
			break;

		// >>> Code für neue Datentypen hier einfügen
		case RVT_COMPLEXATRIBUTE:
			NewViewClass = RUNTIME_CLASS(CCodComplexAtributeView);
			break;

		default:
			TRACE0("Invalid ViewType in CSplitterFrame::SetView\n");
			DEXIT;
			ASSERT(false);

			return;
	}
	ASSERT(NULL != NewViewClass);

	if (OldViewClass == NewViewClass) { // Klasse hat sich nicht geändert!
		DPRINTF(("View not changed"));
		DEXIT;

		return;
	}

	// Es soll das gleiche Dokument verwendet werden, als im Pane(0,0) (=TreeView)
	// dargestellt wird. Das Dokument im Treeview wird sich nie ändern!
	CCreateContext Context;
	Context.m_pCurrentDoc = LeftView->GetDocument();
	ASSERT_VALID(Context.m_pCurrentDoc);
	Context.m_pNewViewClass		= NewViewClass;
	Context.m_pNewDocTemplate	= NULL;
	Context.m_pLastView			= NULL;
	Context.m_pCurrentFrame		= NULL;

	m_wndSplitter.DeleteView(0, 1);
	if (!m_wndSplitter.CreateView(0, 1, NewViewClass, CSize(0, 0), &Context)) {
		TRACE0("Failed to create new pane\n");
		DEXIT;

		return;
	}
	
	m_wndSplitter.RecalcLayout();

	DEXIT;
}

CRightViewType CSplitterFrame::GetCurrentRightViewType()
{
	return m_CurrentRightViewType;
}

/*
** OnGetMinMaxInfo
**
** Verhindert, daß das Fenster unter 400 * 200 Pixel
** verkleinert werden kann.
*/
void CSplitterFrame::OnGetMinMaxInfo(MINMAXINFO FAR *lpMMI) 
{
	ASSERT(NULL != lpMMI);
	lpMMI->ptMinTrackSize = CPoint(400, 200);

	CMDIChildWnd::OnGetMinMaxInfo(lpMMI);
}

bool CSplitterFrame::IsCurrentViewModified()
{
	CCodView *RightView = dynamic_cast<CCodView *>(m_wndSplitter.GetPane(0, 1));
	return NULL != RightView && RightView->IsModified();
}

void CSplitterFrame::LooseChanges()
{
	CCodView *RightView = dynamic_cast<CCodView *>(m_wndSplitter.GetPane(0, 1));
	ASSERT_VALID(RightView);
	RightView->LooseChanges();
}

/*
** OnSelectionChanged
**
** Wird vom TreeView aufgerufen, wenn sich die Auswahl geändert hat.
** Setzt anhand der neuen Auswahl den entsprechenden Daten-View und
** aktualisiert ihn.
*/
void CSplitterFrame::OnSelectionChanged(CNodeInfoSet *NodeInfoSet)
{
	DENTER(GUI_LAYER, "CSplitterFrame::OnSelectionChanged");

	ASSERT(NULL != NodeInfoSet);
	NodeInfoSet->DebugOut();
	ASSERT_VALID(m_pStatusBar);
	ASSERT_VALID(m_pTreeView);

	int SelectedType = NodeInfoSet->empty() ? NI_UNKNOWN : NodeInfoSet->front().m_Type;
	DPRINTF(("Selected Type: %s", CNodeInfo::PrintType(SelectedType)));

	// In der Statuszeile anzeigen, wie viele Objekte ausgewählt wurden:
	int sel = NodeInfoSet->size(); 
	CString MsgText;
	MsgText.Format(1 != sel ? IDS_MSG_SELECTEDOBJECTS : IDS_MSG_SELECTEDOBJECT, sel);
	m_pStatusBar->SetPaneText(0, MsgText);

	// Stefan Mihaila: We allow the change of the right view
	// only if we have the connection to QMaster ok
	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(m_pTreeView->GetDocument());
	ASSERT_VALID(pDoc);

	if (!pDoc->IsDataAvailable()) {
		DEXIT;
		return;
	}
	
	// View für ausgewählten Typ anzeigen:
	switch (SelectedType) {
		case NI_QUEUEROOT:	
			SetRightView(RVT_QUEUEROOT);
			break;

		case NI_QUEUE:		
			SetRightView(RVT_QUEUE);
			break;

		case NI_JOBROOT:
			SetRightView(RVT_JOBROOT);
			break;
		
		case NI_JOB:		
			SetRightView(RVT_JOB);
			break;

		case NI_HOST:		
			SetRightView(RVT_HOST);
			break;
		
		case NI_COMPLEXATRIBUTE:	
			SetRightView(RVT_COMPLEXATRIBUTE);
			break;

		// >>> Code für neue Datentypen hier einfügen
		default:			
			SetRightView(RVT_NOTHING);
			break;
	}

	// If the current new selected view is a CCodView, then update it
	CCodView *RightView  = dynamic_cast<CCodView *>(m_wndSplitter.GetPane(0, 1));
	if (NULL != RightView)
		RightView->Update();

	DEXIT;
}

/*
** GetTreeSelection
**
** Liefert einen Zeiger auf das NodeInfoSet zurück, das die aktuelle Auswahl im
** TreeView darstellt.
*/
CNodeInfoSet *CSplitterFrame::GetTreeSelection()
{
	CCodTreeView *pView = dynamic_cast<CCodTreeView *>(m_wndSplitter.GetPane(0, 0));
	ASSERT_VALID(pView);

	return pView->GetTreeSelection();
}

/*
** SetAged
**
** Aktiviert die Markierung 'Aged Data' in der Statuszeile, falls set true ist,
** ansonsten wird sie deaktiviert.
*/
void CSplitterFrame::SetAged(bool set)
{
	ASSERT_VALID(m_pStatusBar);
	m_pStatusBar->SetPaneStyle(1, set ? SBPS_NORMAL : SBPS_DISABLED);
}

/*
** SetModified (public)
**
** Aktiviert die 'Mod' Markierung in der Statuszeile, falls set true ist,
** ansonsten wird sie deaktiviert.
*/
void CSplitterFrame::SetModified(bool set)
{
	ASSERT_VALID(m_pStatusBar);
	m_pStatusBar->SetPaneStyle(2, set ? SBPS_NORMAL : SBPS_DISABLED);
}

/*
** SetLocal (public)
**
** Aktiviert die 'Local' Markierung in der Statuszeile, falls set true ist,
** ansonsten wird sie deaktiviert.
*/
void CSplitterFrame::SetLocal(int set)
{
	ASSERT_VALID(m_pStatusBar);

	CString str;
	switch (set) {
		case 0: 
			m_pStatusBar->SetPaneStyle(3, SBPS_DISABLED);
			break;

		case -1:
			str.LoadString(IDS_MSG_LOCAL2);
			m_pStatusBar->SetPaneText(3, str);
			m_pStatusBar->SetPaneStyle(3, SBPS_NORMAL);
			break;

		case 1:
			str.LoadString(IDS_MSG_LOCAL);
			m_pStatusBar->SetPaneText(3, str);
			m_pStatusBar->SetPaneStyle(3, SBPS_NORMAL);
			break;

		default:
			ASSERT(false);
	}
}

void CSplitterFrame::OnClose() 
{
	CCodView *RightView = dynamic_cast<CCodView *>(m_wndSplitter.GetPane(0, 1));
	bool Modified = NULL != RightView && RightView->IsModified();

	if (!Modified || IDOK == AfxMessageBox(
		IDS_LOOSECHANGESANDCLOSE, MB_OKCANCEL | MB_DEFBUTTON2))
	{
		if (Modified) {
			ASSERT_VALID(RightView);
			RightView->LooseChanges();
		}
		
		CMDIChildWnd::OnClose();
	}
}
