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
// DebugDoc.cpp: Implemetation File
//

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "DebugDoc.h"
#include "Debug.h"
#include "CodThreadInfo.h"
#include "DebugView.h"
#include <fstream.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDebugDoc

IMPLEMENT_DYNCREATE(CDebugDoc, CDocument)

/*
** CDebugDoc
**
** 
*/
CDebugDoc::CDebugDoc()
{
}

/*
** OnNewDocument
**
** 
*/
BOOL CDebugDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	ExportViewHandle();
	m_MessageQueueOverflow = false;

	return TRUE;
}

/*
** ~CDebugDoc
**
** 
*/
CDebugDoc::~CDebugDoc()
{
}


BEGIN_MESSAGE_MAP(CDebugDoc, CDocument)
	//{{AFX_MSG_MAP(CDebugDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDebugDoc diagnostics

#ifdef _DEBUG
void CDebugDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDebugDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
//  CDebugDoc Serialization 

void CDebugDoc::Serialize(CArchive& ar)
{
	// the standard saving funtion is not used, because several parameters cannot be 
	// specified here. Instead, SaveToFile is used, accessible through the context menu
	// in the debug view.
	if (ar.IsStoring())
	{
		// TODO: Place code for saving here
	}
	else
	{
		// TODO: Place code for loading here
	}
}

/////////////////////////////////////////////////////////////////////////////
// Befehle CDebugDoc 

/*
** OnCloseDocument
**
** Resets the handle of the debug view, the debug messages are
** submitted to. This avoids sending messages when no 
** debug document is open.
*/
void CDebugDoc::OnCloseDocument() 
{
	// Suspend debugging first
	DebugObj.SetNotifyWnd(NULL);

	CDocument::OnCloseDocument();
}

/*
** ExportViewHandle
** 
** Searches for that document's view and puts its window handle
** into the global object DebugObj, so that debug messages 
** may be submitted to this window
*/
void CDebugDoc::ExportViewHandle()
{
	// find view and make its handle global
	POSITION curPos = GetFirstViewPosition();
	if (NULL == curPos) {
		// no view present
		AfxMessageBox(IDS_NODEBUGVIEW);
		ASSERT(false);
	} 

	// attach debugging to the window
	CView *pView = GetNextView(curPos);
	ASSERT_VALID(pView);
	DebugObj.SetNotifyWnd(pView->m_hWnd);
}

/*
** InsertLine
**
** Inserts a debug-message into the document
*/
void CDebugDoc::InsertLine(const CString &s, int Code)
{
	if (m_MessageQueue.size() >= DebugObj.m_BufferLen) {
		while (m_MessageQueue.size() >= DebugObj.m_BufferLen)
			m_MessageQueue.pop_front();
		
		m_MessageQueueOverflow = true;
	}

	m_MessageQueue.push_back(CDebugMessageEntry(s, Code));
//	SetModifiedFlag();
}

void CDebugDoc::ClearBuffer()
{
	m_MessageQueue.clear();
}

bool CDebugDoc::SaveToFile(const CString &strFilename, int Flags)
{
	ofstream outfile;
	CDebugMessageQueue::iterator MsgIterator;
	char *strMonitoringLevels[] = {	"Top  ", "Cull ", "Basis", "GUI  ", "Sched", 
									"Commd", "Api  ", "Pack "};
	char *strNoMonitoringLevel = "     ";
	CDebugMessageEntry *pEntry;

	if (Flags & DDSTF_APPENDTOFILE) // Append
		outfile.open(strFilename, ios::out|ios::app, filebuf::sh_read);
	else // overwrite
		outfile.open(strFilename, ios::out, filebuf::sh_read);
	
	if (0 == outfile.is_open())
		return false;	// file couldn't be opened
	
	outfile << "\n\n";

	for (MsgIterator = m_MessageQueue.begin(); MsgIterator != m_MessageQueue.end(); MsgIterator++) {
		pEntry = &*MsgIterator;			
		if (Flags & DDSTF_INCLUDELAYER)
			outfile << strMonitoringLevels[pEntry->m_Code];
		else
			outfile << strNoMonitoringLevel;
		
		outfile << " : " << pEntry->m_Message << endl;
	}

	outfile.close();

	if (Flags & DDSTF_CLEARBUFFER) {
		ClearBuffer();
		UpdateAllViews(NULL, UPDATE_INSERTLINE);
	}

	return true;
}
