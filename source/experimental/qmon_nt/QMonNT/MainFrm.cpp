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
// MainFrm.cpp : Implementierung der Klasse CMainFrame
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"

#include "MainFrm.h"
#include "DebugView.h"
#include "DebugDoc.h"
#include "Messages.h"
#include "Debug.h"
#include "SetDebugLevelDialog.h"
#include "AnswerMessageBox.h"
#include "AnswerList.h"
#include "Queue.h"
#include "SplitterFrame.h"

namespace Codine {
extern "C" {
#include "cod_api.h"
#include "cod_api_intern.h"
#include "cod_queueL.h" 
#include "cod_prognames.h"
#include "cod_all_listsL.h"
}
}


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_MESSAGE(WM_CODNOTIFY, OnCodNotify)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_OPTIONS_SETDEBUGLEVEL, OnOptionsSetDebugLevel)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	// Globale Hilfebefehle
	ON_COMMAND(ID_HELP_FINDER, CMDIFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CMDIFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CMDIFrameWnd::OnHelpFinder)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame Nachrichten-Handler

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Stefan Mihaila has started here
	static UINT Indicators[] = { 
		ID_SEPARATOR,

		IDS_MSG_AGEDDATA, 
		IDS_MSG_EDIT,
		IDS_MSG_LOCAL,
		IDS_MSG_SEND,

		ID_INDICATOR_CAPS,
		ID_INDICATOR_NUM,
		ID_INDICATOR_SCRL
	};
	// Stefan Mihaila has finished here

	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // Fehler beim Erzeugen
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(Indicators,
		  sizeof(Indicators) / sizeof(Indicators[0])))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // Fehler beim Erzeugen
	}

	// Stefan Mihaila has started here
	m_wndStatusBar.SetPaneInfo(0, Indicators[0], SBPS_STRETCH | SBPS_NOBORDERS, 0);
	m_wndStatusBar.SetPaneInfo(1, Indicators[1], SBPS_NORMAL, 55);
	m_wndStatusBar.SetPaneInfo(2, Indicators[2], SBPS_DISABLED, 20);
	m_wndStatusBar.SetPaneInfo(3, Indicators[3], SBPS_DISABLED, 35);

	CString str;
	str.LoadString(IDS_MSG_AGEDDATA);
	m_wndStatusBar.SetPaneText(1, str);
	str.LoadString(IDS_MSG_EDIT);
	m_wndStatusBar.SetPaneText(2, str);
	str.LoadString(IDS_MSG_LOCAL);
	m_wndStatusBar.SetPaneText(3, str);
	// Stefan Mihaila has finished here
	
	// ZU ERLEDIGEN: Entfernen, wenn Sie keine QuickInfos oder variable Symbolleiste wünschen
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// ZU ERLEDIGEN: Löschen Sie diese drei Zeilen, wenn Sie nicht wollen, dass die Symbolleiste
	//  andockbar ist.
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// ZU ERLEDIGEN: Ändern Sie hier die Fensterklasse oder das Erscheinungsbild, indem Sie
	//  CREATESTRUCT cs modifizieren.

	return CMDIFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame Konstruktion/Destruktion

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame Diagnose


/*
** OnOptionsSetDebugLevel
**
** 
*/
void CMainFrame::OnOptionsSetDebugLevel() 
{
	DENTER(GUI_LAYER, "CMainFrame::OnOptionsSetDebugLevel");

	// If the window isn't created yet, then create and initialize it
	if (NULL == SetDebugLevelDialog.m_hWnd) {
		SetDebugLevelDialog.m_MonitoringLevel = DebugObj.m_MonitoringLevel;
		SetDebugLevelDialog.Create(CSetDebugLevelDialog::IDD, this);
	}

	SetDebugLevelDialog.ShowWindow(SW_NORMAL);
	SetDebugLevelDialog.SetFocus();

	DEXIT;
}

/*
** GetQmonNTDoc
**
** Ermittelt das QmonNT-Dokument. Achtung: Es wird davon ausgegangen, daß
** zu jeder Zeit nur ein solches Dokument existiert (es wird das erste
** zurückgegeben). Außerdem sollte die Funktion nicht aufgerufen werden,
** wenn kein Dokument existiert (ASSERT!)
*/
CQmonntDoc *CMainFrame::GetQmonntDoc()
{
	DENTER(GUI_LAYER, "CMainFrame::GetQmonNTDoc");

	CQmonntApp *App = dynamic_cast<CQmonntApp *>(AfxGetApp());
	ASSERT_VALID(App);
	ASSERT_VALID(App->m_pQMonNTDocTemplate);

	POSITION CurPos = App->m_pQMonNTDocTemplate->GetFirstDocPosition();
	ASSERT(NULL != CurPos);	// TODO: Assert ist vermutlich etwas hart. Sollte in 
							// if(CurPos != NULL) {.... umgewandelt werden!

	CQmonntDoc *pDoc = dynamic_cast<CQmonntDoc *>(App->m_pQMonNTDocTemplate->GetNextDoc(CurPos));
	ASSERT_VALID(pDoc);

	DEXIT;

	return pDoc;
}

/*
** OnCodNotify
**
** Reagiert auf die Meldung WM_CODNOTIFY, die der Workerthread
** an den MainFrame sendet, wenn eine Antwort an das Dokument
** gesendet werden soll.
** Es wird die gleichnamige Funktion im QmonNT-Dokument aufgerufen.
*/
LRESULT CMainFrame::OnCodNotify(WPARAM wp, LPARAM lp)
{
	DENTER(GUI_LAYER, "CMainFrame::OnCodNotify");

	// Die Meldung an das Dokument per Funktionsaufruf weiterleiten:
	CQmonntDoc *pDoc = GetQmonntDoc();
	ASSERT_VALID(pDoc);
	pDoc->OnCodNotify(wp, lp);
	
	DEXIT;
	return 0;
}

void CMainFrame::SetStatusText(UINT Msg)
{
	if (0 == Msg)
		m_wndStatusBar.SetPaneText(4, "");
	else {
		CString str;
		str.LoadString(Msg);
		m_wndStatusBar.SetPaneText(4, str);
	}
}

void CMainFrame::OnClose() 
{
	CSplitterFrame *ChildFrame;
	bool ChildModified;

	for (CWnd *Child = MDIGetActive(); NULL != Child; Child = Child->GetNextWindow()) {
		ChildFrame = dynamic_cast<CSplitterFrame *>(Child);
		ASSERT_VALID(ChildFrame);

		ChildModified = ChildFrame->IsCurrentViewModified();
		if (ChildModified) {
			MDIActivate(ChildFrame);
			if (IDCANCEL == AfxMessageBox(IDS_LOOSECHANGESANDCLOSE, MB_OKCANCEL | MB_DEFBUTTON2))
				return;
			ChildFrame->LooseChanges();
		}
	}

	CMDIFrameWnd::OnClose();
}
