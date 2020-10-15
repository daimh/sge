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
// DebugView.cpp: Implementation File
//
// Checked by Stefan Mihaila


#include "stdafx.h"
#include "qmonnt.h"
#include "DebugView.h"
#include "Debug.h"
#include "CodThreadInfo.h"
#include "DebugDoc.h"
#include "LogSaveDlg.h"
#include <stack>

#include "Messages.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDebugView

IMPLEMENT_DYNCREATE(CDebugView, CScrollView)

CDebugView::CDebugView()
{

}

CDebugView::~CDebugView()
{
}


BEGIN_MESSAGE_MAP(CDebugView, CScrollView)
	ON_MESSAGE(WM_DEBUGMESSAGE, OnDebugMessage)
	//{{AFX_MSG_MAP(CDebugView)
	ON_COMMAND(ID_DEBUG_STARTDEBUG, OnDebugStartdebug)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_STARTDEBUG, OnUpdateDebugStartdebug)
	ON_COMMAND(ID_DEBUG_STOPDEBUG, OnDebugStopdebug)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_STOPDEBUG, OnUpdateDebugStopdebug)
	ON_COMMAND(ID_DEBUG_SEPARATOR, OnDebugSeparator)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_DEBUG_CLEARBUFFER, OnDebugClearbuffer)
	ON_COMMAND(ID_DEBUG_SAVEBUFFERTOFILE, OnDebugSavebuffertofile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDebugView Draw

void CDebugView::OnDraw(CDC* pDC)
{
//	CDebugDoc* pDoc = (CDebugDoc*)GetDocument();
	DrawMessageList();
}

/////////////////////////////////////////////////////////////////////////////
// CDebugView diagnostics

#ifdef _DEBUG
void CDebugView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CDebugView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDebugView message managing routines 

/*
** InsertLine
**
** Inserts a specified string into the edit-field of the debug view.
** The text is inserted at the end. 
** Scroll positions and marks are gone (until now)
*/
void CDebugView::InsertLine(/*CString &s*/)
{
//	DC = GetDC();

	
//	DC->TextOut(20,20, s);

//	GetEditCtrl().SetSel(-1, 32000, TRUE); // Selektion aufheben
//	GetEditCtrl().ReplaceSel(m_DebugMessage);

//	ReleaseDC(DC);
}

/*
** OnDebugMessage
**
** Is called, when there is a debug message.
** Delivers the debug-string into the document and signals
** the sender of the message that the message has been processed.
*/
LRESULT CDebugView::OnDebugMessage(WPARAM, LPARAM lp)
{
	CDebugDoc *pDoc = dynamic_cast<CDebugDoc *>(GetDocument());
	ASSERT_VALID(pDoc);
	int Code;

	if (NULL != lp) {
//		x = CString((char*)lp)+"\r\n";
//		InsertLine(CString((char*)lp)+"\r\n");
	} 
	else {
//		x = CString(DebugObj.Buffer)+"\r\n";
//		InsertLine(CString(DebugObj.Buffer)+"\r\n");

		ASSERT(NULL != DebugObj.Mutex);
		WaitForSingleObject(DebugObj.Mutex, INFINITE);

		if (!DebugObj.m_MessageQueue.empty()) {
			m_DebugMessage = DebugObj.m_MessageQueue.front().m_Message;
			Code = DebugObj.m_MessageQueue.front().m_Code;

			DebugObj.m_MessageQueue.pop_front();
//			ASSERT(DebugObj.m_MessageQueue.empty());
			ReleaseMutex(DebugObj.Mutex);
			pDoc->InsertLine(m_DebugMessage, Code);
			pDoc->UpdateAllViews(NULL, UPDATE_INSERTLINE);
		} 
		else
			ReleaseMutex(DebugObj.Mutex);
	}

	return 0;
}

/*
** OnDebugStartdebug
**
** Allows further debug-output, by setting the handle of the 
** debug window to the handle of the current debug view.
*/
void CDebugView::OnDebugStartdebug() 
{
	DebugObj.SetNotifyWnd(m_hWnd);
}

/*
** OnUpdateDebugStartdebug
**
** displays and hides the "start debug" menu
*/
void CDebugView::OnUpdateDebugStartdebug(CCmdUI* pCmdUI) 
{
	ASSERT(NULL != pCmdUI);
	pCmdUI->Enable(NULL == DebugObj.DebugNotifyWnd);
}

/*
** OnDebugStopdebug
**
** suppresses further debug output by setting the handle
** of the debug window to NULL
*/
void CDebugView::OnDebugStopdebug() 
{
	DebugObj.SetNotifyWnd(NULL);
}

/*
** OnUpdateDebugStopdebug
**
** displays and hides the "stop debug" menu
*/
void CDebugView::OnUpdateDebugStopdebug(CCmdUI* pCmdUI) 
{
	ASSERT(NULL != pCmdUI);
	pCmdUI->Enable(NULL != DebugObj.DebugNotifyWnd);
}

/*
** OnUpdate
**
** 
*/
void CDebugView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CPoint ScrollPos;
	CSize TotalSize;
	RECT ClientRect;
	bool ScrollToEnd;
	CDebugDoc *pDoc;
	int NewScrollY;

	switch (lHint) {
		case UPDATE_INSERTLINE:
			// get scroll position
			GetClientRect(&ClientRect);
			ScrollPos = GetScrollPosition();
			TotalSize = GetTotalSize();

			if (ScrollPos.y >= TotalSize.cy-ClientRect.bottom)
				ScrollToEnd = true;
			else
				ScrollToEnd = false;
			// NO break;

		default:
			DrawMessageList();
	}

	if (ScrollToEnd) {
		// get new size and set scroller
		TotalSize = GetTotalSize();
		ScrollToPosition(CPoint(ScrollPos.x, TotalSize.cy));
	} 
	else {
		// assert that the same row remains visible
		pDoc = dynamic_cast<CDebugDoc *>(GetDocument());
		ASSERT_VALID(pDoc);

		if (pDoc->m_MessageQueueOverflow) {
			NewScrollY = ScrollPos.y-m_LineHeight;
			if (NewScrollY < 0)
				NewScrollY = 0;
			ScrollToPosition(CPoint(ScrollPos.x, NewScrollY));
		}
	}
}


/*
** DrawMessageList
**
** 
*/
void CDebugView::DrawMessageList()
{
	CDC *pDC;
	CDebugDoc *pDoc;
	CDebugMessageQueue::iterator Iterator;
	int i, m, LinesPerView, FirstLine, LineOffset;
	RECT ClientRect;
	CPoint ScrollPos;

	ScrollPos = GetScrollPosition();
	pDoc = dynamic_cast<CDebugDoc *>(GetDocument());
	ASSERT_VALID(pDoc);

	pDC = GetDC();
	ASSERT(NULL != pDC);
	pDC->SetBkMode(OPAQUE);
	pDC->SetBkColor(GetSysColor(COLOR_WINDOW));

	// determine number of rows and the row height
	GetClientRect(&ClientRect);
	LinesPerView = (ClientRect.bottom - ClientRect.top) / m_LineHeight;
	FirstLine  = ScrollPos.y / m_LineHeight;
	LineOffset = ScrollPos.y % m_LineHeight;

	m = pDoc->m_MessageQueue.size() - 1;

	SetScrollSizes(MM_TEXT, CSize(1, m_LineHeight * (m + 1)),
		CSize(10 * m_LineHeight, 10 * m_LineHeight), 
		CSize(m_LineHeight, m_LineHeight));

	pDC->FillSolidRect(&ClientRect, GetSysColor(COLOR_WINDOW));

	i = 0;
	while (FirstLine + i <= m) {
		pDC->SetTextColor(DebugObj.m_DebugOutputColors[pDoc->m_MessageQueue[FirstLine+i].m_Code]);
/*		switch(pDoc->m_MessageQueue[FirstLine+i].m_Code) {
			case GUI_LAYER:
				pDC->SetTextColor(0x00ffffff);
				break;
			default:
				pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		}
*/		pDC->TextOut(20, i * m_LineHeight - LineOffset, pDoc->m_MessageQueue[FirstLine+i].m_Message);
		i++;
	}

	ReleaseDC(pDC);
}

/*
** OnInitialUpdate
**
** Initializes scroll area.
** Is needed because OnUpdate asserts otherwise
*/
void CDebugView::OnInitialUpdate() 
{
	CDC *pDC;
	TEXTMETRIC TextMetric;

	DENTER(GUI_LAYER, "CDebugView::OnInitialUpdate");
	SetScrollSizes(MM_TEXT, CSize(1, 1));

//	CScrollView::OnInitialUpdate();

	// determine height of a row 
	pDC = GetDC();
	ASSERT(NULL != pDC);
	pDC->GetTextMetrics(&TextMetric);
	m_LineHeight = TextMetric.tmHeight;
	ReleaseDC(pDC);
	
	CScrollView::OnInitialUpdate();

	DEXIT;
}


/*
** OnDebugSeparator
**
** Inserts a separator line into the debug output
*/
void CDebugView::OnDebugSeparator() 
{
	CDebugDoc *pDoc = dynamic_cast<CDebugDoc *>(GetDocument());
	ASSERT_VALID(pDoc);

	pDoc->InsertLine(CString(""), 0);
	pDoc->InsertLine(CString("========================================="), 0);
	pDoc->InsertLine(CString(""), 0);
	pDoc->UpdateAllViews(NULL, UPDATE_INSERTLINE);
}

void CDebugView::OnDebugClearbuffer() 
{
	CDebugDoc *pDoc = dynamic_cast<CDebugDoc *>(GetDocument());
	ASSERT_VALID(pDoc);

	pDoc->ClearBuffer();
	pDoc->UpdateAllViews(NULL, UPDATE_INSERTLINE);
}

void CDebugView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu menu;

	if (menu.LoadMenu(ID_POPUP_MENU)) {
		CMenu *pPopup = menu.GetSubMenu(0);
		ASSERT_VALID(pPopup);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y,
			AfxGetMainWnd());
	}
}


/*
** OnDebugSavebuffertofile
**
** Saves the current content of the debug buffer in a file
*/
void CDebugView::OnDebugSavebuffertofile() 
{
	CDebugDoc *pDoc = dynamic_cast<CDebugDoc *>(GetDocument());
	ASSERT_VALID(pDoc);

	CLogSaveDlg FileDialog(false, ".txt", "QMonntLog.txt");

	if (IDOK == FileDialog.DoModal()) {
		CString strFilename = FileDialog.GetPathName();
		BOOL bAppend = FileDialog.m_bAppendToFile;
		BOOL bIncludeLayers = FileDialog.m_bIncludeLayer;
		BOOL bClearBuffer = FileDialog.m_bClearBuffer;

		int Flags = bAppend		? DDSTF_APPENDTOFILE : 0;
		Flags |= bIncludeLayers ? DDSTF_INCLUDELAYER : 0;
		Flags |= bClearBuffer	? DDSTF_CLEARBUFFER  : 0;

		pDoc->SaveToFile(strFilename, Flags);

	}
}
