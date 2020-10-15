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
// qmonntDoc.cpp : Implementation of the class CQmonntDoc

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "mainfrm.h"

#include "qmonntDoc.h"
#include "Debug.h"
#include "Messages.h"
#include "setdebugleveldialog.h"
#include "AnswerMessageBox.h"
#include "Queue.h"
#include "QueueSet.h"
#include "ComplexAtributeSet.h"
#include "CodTreeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQmonntDoc

IMPLEMENT_DYNCREATE(CQmonntDoc, CDocument)

BEGIN_MESSAGE_MAP(CQmonntDoc, CDocument)
	//{{AFX_MSG_MAP(CQmonntDoc)
	ON_COMMAND(ID_EDIT_REFRESH, OnEditRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQmonntDoc construction/destruction
//

CQmonntDoc::CQmonntDoc()
{
	m_bGetQueueListSucceeded	= false;
	m_bGetJobListSucceeded		= false;
	m_bGetHostListSucceeded		= false;
	m_bGetComplexListSucceeded	= false;

	m_bWorkerThreadIdle     = true;
	m_bHasMessage			= false;

	m_pQueueList			= NULL;
	m_pJobList				= NULL;
	m_pHostList				= NULL;
	m_pComplexList			= NULL;
	CodThread				= NULL;
}

CQmonntDoc::~CQmonntDoc()
{
}

/*
** OnNewDocument (public)
**
** This funtion is called, when a new CQmonnt document is to be created.
** Initializes the workerthread.
*/
BOOL CQmonntDoc::OnNewDocument()
{
	// Initializations
	m_bGetQueueListSucceeded	= false;
	m_bGetJobListSucceeded		= false;
	m_bGetHostListSucceeded		= false;
	m_bGetComplexListSucceeded	= false;

	m_bWorkerThreadIdle     = true;
	m_bHasMessage			= false;

	m_pQueueList			= NULL;
	m_pJobList				= NULL;
	m_pHostList				= NULL;
	m_pComplexList			= NULL;
	CodThread				= NULL;

	// >>> Insert code for additional data types here

	if (!CDocument::OnNewDocument())
		return FALSE;

	ResetEvent(CCodThreadInfo::CodThreadInfo.EventThreadKilled);
    ResetEvent(CCodThreadInfo::CodThreadInfo.EventKillThread);

	// start workerthread
	CodThread = CCodThreadInfo::StartThread();
	ASSERT_VALID(CodThread);

	// Stefan Mihaila: now, the document is ready to process commands
	// given by the central view of the application, which is 'CCodTreeView'

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CQmonntDoc Serialization

void CQmonntDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: insert code for saving here
	}
	else
	{
		// TODO: insert code for loading here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CQmonntDoc Diagnose

#ifdef _DEBUG

void CQmonntDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CQmonntDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CQmonntDoc Commands

/*
** OnCloseDocument (public)
**
** 
*/
void CQmonntDoc::OnCloseDocument() 
{
	DENTER(GUI_LAYER, "CQmonntDoc::OnCloseDocument");

	ASSERT_VALID(CodThread);

	SetMessage(IDS_MSG_CLOSINGCODINE);

	// FIXME: The ClosingCodine message doesn't work properly, because
	// the View is not redrawn, while we suck in a wait-loop down there.
	// Perhaps this could be solved using messages

	ResetEvent(CCodThreadInfo::CodThreadInfo.EventThreadKilled);

	PostCommand(TC_SHUTDOWN);
	
	if (WAIT_TIMEOUT == WaitForSingleObject(CCodThreadInfo::CodThreadInfo.EventThreadKilled, 2000)) {
		TerminateThread(CodThread->m_hThread, 1);
		AfxMessageBox(IDS_HARDTERMINATEDTHREAD);
	}

	m_bGetQueueListSucceeded	= false;
	m_bGetJobListSucceeded		= false;
	m_bGetHostListSucceeded		= false;
	m_bGetComplexListSucceeded	= false;

	m_bWorkerThreadIdle     = true;
	m_bHasMessage			= false;
	CodThread				= NULL;

	CDocument::OnCloseDocument();
	DEXIT;
}

/*
** OnCodNotify (public)
**
** Is called by MainFrame, when the WM_CODINECONTACTED message is received
** (sent by the workerthread).
** Calls the function corresponding to the Notify message.
*/
void CQmonntDoc::OnCodNotify(WPARAM NotifyCode, LPARAM lp)
{
	DENTER(GUI_LAYER, "CQmonntDoc::OnCodNotify");

	// Make workerthread idle, because command is processed
	m_bWorkerThreadIdle = true;

	switch (NotifyCode) {
		case TR_CONNECTION_FAILURE:
			OnConnectionFailed(lp);			
			break;

		case TR_CODAPI_FAILURE:
			ShowAnswerList();
			break;

		case TR_GETQUEUELIST_SUCCESS:			
			OnGetQueueListSucceeded(lp);
			break;

		case TR_GETJOBLIST_SUCCESS:
			OnGetJobListSucceeded(lp);
			break;

		case TR_GETHOSTLIST_SUCCESS:
			OnGetHostListSucceeded(lp);					
			break;

		case TR_GETCOMPLEXLIST_SUCCESS:
			OnGetComplexListSucceeded(lp);
			break;

		case TR_SUBMITJOB_SUCCESS:
			OnSubmitJobSucceeded(lp);
			break;

		case TR_READY:
			break;

		// >>> insert code for additional Notification-Codes here
	
		default:
			TRACE("Unknown Notify Code in CQmonntDoc::OnCodNotify\n");
			ASSERT(false);
	}
	
	CMainFrame *MainFrame = dynamic_cast<CMainFrame *>(AfxGetMainWnd());
	ASSERT_VALID(MainFrame);
	MainFrame->SetStatusText(0);

	PostCommand(TC_NOCOMMAND);
	DEXIT;
}

bool CQmonntDoc::IsDataAvailable()
{
	return	m_bGetQueueListSucceeded && m_bGetJobListSucceeded &&
			m_bGetHostListSucceeded	 && m_bGetComplexListSucceeded;
}

/*
** SetMessage (private)
**
** Loads the MessageText in the document's specified string-ressource
** and activates the message-flag
*/
void CQmonntDoc::SetMessage(UINT nID)
{
	m_MessageText.LoadString(nID);
	m_bHasMessage = true;
	UpdateAllViews(NULL, UAVH_MESSAGE);
}

/*
** ClearMessage (private)
**
** Clears the message text in the document
*/
void CQmonntDoc::ClearMessage()
{
	m_MessageText.Empty();
	m_bHasMessage = false;
	UpdateAllViews(NULL, UAVH_NOMESSAGE);
}

/*
** HasMessage (public)
**
** returns TRUE, if a there is a message text 
*/
bool CQmonntDoc::HasMessage()
{
	return m_bHasMessage;
}

/*
** GetMessage (public)
**
** Returns the message text.
** Call this function only if there actually exists a message text 
*/
CString CQmonntDoc::GetMessage()
{
	return m_MessageText;
}

/* 
** OnSetupQMasterDown (private)
**
** Is called, if the workerthread fails to connect to the qmaster
*/
void CQmonntDoc::OnConnectionFailed(LPARAM lp)
{
	DENTER(GUI_LAYER, "CQmonntDoc::OnConnectionFailed");

	SetMessage(IDS_MSG_QMASTERDOWN);

	DEXIT;
}

/*
** ShowAnswerList (private)
**
** Displays the contents of the Answerlist in the CodThreadInfo-object,
** if it isn't empty.
** The Answerlist is deleted, after being displayed.
*/
void CQmonntDoc::ShowAnswerList()
{
	ASSERT(!CCodThreadInfo::CodThreadInfo.AnswerList.IsEmpty());

	CAnswerMessageBox AnswerBox;
	AnswerBox << CCodThreadInfo::CodThreadInfo.AnswerList;
	AnswerBox.DoModal();
	CCodThreadInfo::CodThreadInfo.AnswerList.Clear();
}

/*
** OnGetQueueListSucceeded (private)
**
** Is called when a new Queue-list is received from the workerthread.
** lp represents a pointer to the Queue-list.
** The new list is concatenated with the saved one.
*/
void CQmonntDoc::OnGetQueueListSucceeded(LPARAM lp)
{
	CQueueList *QueueList = reinterpret_cast<CQueueList *>(lp);
	ASSERT(NULL != QueueList);

	// Mark the old queues for deleting purposes
	if (NULL == m_pQueueList)  
		m_pQueueList = QueueList;
	else {
		CQueueList::iterator qit;

		for (qit = m_pQueueList->begin(); qit != m_pQueueList->end(); qit++) 
				qit->m_Tag = true;

		// Adapt any queue, that is element of both, the new and old list with the old ID
		CQueueList::iterator nqit = QueueList->begin();

		while (nqit != QueueList->end()) {
			// search queue with same name in old list 
			for (qit = m_pQueueList->begin(); qit != m_pQueueList->end(); qit++) 
				if (0 == qit->qname.CompareNoCase(nqit->qname)) 
					break;
			
			if (qit != m_pQueueList->end()) {
				// queues are identical, remove old one and append the new one
				nqit->SetID(qit->GetID());		// adapt old queue id
				m_pQueueList->erase(qit);		// remove old queue
				m_pQueueList->push_back(*nqit);	// append new queue in old list

				nqit = QueueList->erase(nqit);	// erase new queue and increment iterator
			}
			else 
				nqit++;
		}

		// append any queue remaining in the new list with new ID
		nqit = QueueList->begin();
		while (nqit != QueueList->end()) {
			m_pQueueList->push_back(*nqit);
			nqit++;
		}
		QueueList->clear();
		delete QueueList;
		QueueList = NULL;

		// delete any remaining old queues
		qit = m_pQueueList->begin();
		while (qit != m_pQueueList->end()) {
			if (qit->m_Tag) {
				qit = m_pQueueList->erase(qit);
				continue;
			}
			qit++;
		}
	} 

	ClearMessage();
	m_bGetQueueListSucceeded = true;
	UpdateAllViews(NULL, UAVH_QUEUES);
}

/*
** OnGetJobListSucceeded (private)
**
** Is called, when a new job-list is received from the workerthread.
** lp represents a pointer to the joblist.
** The new list is concateneted with the saved one.
*/
void CQmonntDoc::OnGetJobListSucceeded(LPARAM lp)
{
	CJobList *pJobList = reinterpret_cast<CJobList *>(lp);
	ASSERT(NULL != pJobList);

	if (NULL == m_pJobList)
		// No job list present, adapt complete new list
		m_pJobList = pJobList;
	else {	// Merge
		CJobList::iterator it;

		// mark all old jobs for deleting purposes
		for (it = m_pJobList->begin(); it != m_pJobList->end(); it++)
			it->m_Tag = true;

		// Adapt any job, that is element of both, the new and old list with the old ID
		CJobList::iterator newit = pJobList->begin();
		while (newit != pJobList->end()) {
			// search job with same name in old list 
			for (it = m_pJobList->begin(); it != m_pJobList->end(); it++)
				if (it->job_number == newit->job_number)
					break;

			if (it != m_pJobList->end()) {
				// jobs are identical, remove old one and append the new one
				newit->SetID(it->GetID());			// adapt old job id
				m_pJobList->erase(it);				// remove old job
				m_pJobList->push_back(*newit);		// append new job to old list
				newit = pJobList->erase(newit);		// erase new job and increment iterator
			} 
			else
				newit++;
		}

		// append any job remaining in the new list with new IDs
		newit = pJobList->begin();
		while (newit != pJobList->end()) {
			m_pJobList->push_back(*newit);
			newit++;
		}
		pJobList->clear();
		delete pJobList;
		pJobList = NULL;

		// delete any remaining old job
		it = m_pJobList->begin();
		while (it != m_pJobList->end()) {
			if(it->m_Tag) {
				it = m_pJobList->erase(it);
				continue;
			}
			it++;
		}
	}

	
/*	// TODO: merge the joblists here
	if(m_pJobList != NULL) {
		delete m_pJobList;
	}
	m_pJobList = JobList;
*/

	ClearMessage();
	m_bGetJobListSucceeded = true;
	UpdateAllViews(NULL, UAVH_JOBS);
}

/*
** OnGetHostListSucceeded (private)
**
** 
*/
void CQmonntDoc::OnGetHostListSucceeded(LPARAM lp)
{
	CHostList *pHostList = reinterpret_cast<CHostList *>(lp);
	ASSERT(NULL != pHostList);

	if (NULL == m_pHostList)
		// No host list present, adapt complete new list
		m_pHostList = pHostList;
	else {	// Merge
		CHostList::iterator it;

		// mark all old hosts for deleting purposes
		for (it = m_pHostList->begin(); it != m_pHostList->end(); it++)
			it->m_Tag = true;

		// Adapt any host, that is element of both, the new and old list with the old ID
		CHostList::iterator newit = pHostList->begin();
		while (newit!=pHostList->end()) {
			// search host with same name in old list 
			for(it = m_pHostList->begin(); it != m_pHostList->end(); it++)
				if (0 == it->hostname.CompareNoCase(newit->hostname))
					break;
			
			if (it != m_pHostList->end()) {
				// hosts are identical, remove old one and append the new one
				newit->SetID(it->GetID());			// adapt old host id
				m_pHostList->erase(it);				// remove old host
				m_pHostList->push_back(*newit);		// append new host to old list
				newit = pHostList->erase(newit);	// erase new host and increment iterator
			} 
			else
				newit++;
		}

		// append any host remaining in the new list with new ID
		newit = pHostList->begin();
		while (newit != pHostList->end()) {
			m_pHostList->push_back(*newit);
			newit++;
		}
		pHostList->clear();
		delete pHostList;
		pHostList = NULL;

		// delete any remaining old host
		it = m_pHostList->begin();
		while (it != m_pHostList->end()) {
			if (it->m_Tag) {
				it = m_pHostList->erase(it);
				continue;
			}
			it++;
		}
	}

	ClearMessage();
	m_bGetHostListSucceeded = true;
	UpdateAllViews(NULL, UAVH_HOSTS);
}

void CQmonntDoc::OnGetComplexListSucceeded(LPARAM lp)
{
	CComplexList *pComplexList = reinterpret_cast<CComplexList *>(lp);
	ASSERT(NULL != pComplexList);

	if (NULL == m_pComplexList)
		m_pComplexList = pComplexList;
	else {
		CComplexList::iterator it;

		for (it = m_pComplexList->begin(); it != m_pComplexList->end(); it++) 
			it->m_Tag = true;

		CComplexList::iterator newit = pComplexList->begin();
		while (newit!=pComplexList->end()) {
			for (it = m_pComplexList->begin(); it != m_pComplexList->end(); it++)
				if (it->name == newit->name)
					break;
			
			if (it != m_pComplexList->end()) {
				// jobs are identical, remove old one and append the new one
				newit->SetID(it->GetID());				// adapt old job id
				m_pComplexList->erase(it);				// remove old job
				m_pComplexList->push_back(*newit);		// append new job to old list
				newit = pComplexList->erase(newit);		// erase new job and increment iterator
			} 
			else
				newit++;
		}

		// append any job remaining in the new list with new IDs
		newit = pComplexList->begin();
		while (newit != pComplexList->end()) {
			m_pComplexList->push_back(*newit);
			newit++;
		}
		pComplexList->clear();
		delete pComplexList;
		pComplexList = NULL;

		// delete any remaining old job
		it = m_pComplexList->begin();
		while (it != m_pComplexList->end()) {
			if (it->m_Tag == true) {
				it = m_pComplexList->erase(it);
				continue;
			}
			it++;
		}
	}

	
/*	// TODO: merge the joblists here
	if(m_pJobList != NULL) {
		delete m_pJobList;
	}
	m_pJobList = JobList;
*/

	ClearMessage();
	m_bGetComplexListSucceeded = true;
	UpdateAllViews(NULL, UAVH_COMPLEXES);
}

void CQmonntDoc::OnSubmitJobSucceeded(LPARAM lp)
{
	RetrieveAllJobs();
	AfxMessageBox("Job submited ok");
}

/*
** DeleteContents (public)
**
** Is called before destroying the document.
** Deletes all members.
*/
void CQmonntDoc::DeleteContents() 
{
	if (NULL != m_pQueueList) {
		delete m_pQueueList;
		m_pQueueList = NULL;
	}
	if (NULL != m_pJobList) {
		delete m_pJobList;
		m_pJobList = NULL;
	}
	if (NULL != m_pHostList) {
		delete m_pHostList;
		m_pHostList = NULL;
	}
	if (NULL != m_pComplexList) {
		delete m_pComplexList;
		m_pComplexList = NULL;
	}

	CDocument::DeleteContents();
}

/*
** GetQueueSet
**
** Refreshes the queueset passed by pQueueSet in regard to the selection,
** specified by pNodeInfoSet
*/
void CQmonntDoc::GetQueueSet(CQueueSet *pQueueSet, CNodeInfoSet *pNodeInfoSet)
{
	DENTER(GUI_LAYER, "CQmonntDoc::GetQueueSet");

	ASSERT(NULL != pQueueSet);
	ASSERT(NULL != pNodeInfoSet);

	pNodeInfoSet->DebugOut();

	// Keep any queue, that is contained in the NodeinfoSet, but not in the document,
	// and mark it as "deleted"
	
	// Activate all tags in the InfoSet
	pNodeInfoSet->SetTag();

	// Activate all tags in the queueset
	pQueueSet->SetTag();

	// Alle Queues im Dokument durchlaufen
	// iterate the document's queues
	CQueueList::iterator qit;
	ULONG ID;
	for (qit = m_pQueueList->begin(); qit != m_pQueueList->end(); qit++) {
		ID = qit->GetID();

		// if InfoSet contains queue
		if (pNodeInfoSet->Contains(ID)) {
			// delete old queue
			pQueueSet->Delete(ID);

			// clear tag
			pQueueSet->ClearTag(ID);

			// add new queue
			pQueueSet->Add(*qit);

			// clear tag in InfoSet
			pNodeInfoSet->ClearTag(ID);
		}
	}

	// Any object corresponding to a nodeinfo in the infoset,whose 
	// tag was still activated, is now removed.
	// Now the corresponding tag in the queueset must be reset.
	// Otherwise they would be removed again too

	DPRINTF(("****** pQueueSet dazwischen******"));
	pQueueSet->DebugOut();

	for (qit = pQueueSet->begin(); qit != pQueueSet->end(); qit++) {
		ID = qit->GetID();
		if (pNodeInfoSet->IsTagSet(ID))
			pQueueSet->ClearTag(ID);
	}

	// The elements of QueueSet,where the Tag is still set,
	// have been deselected and can be deleted now
	// TODO: DELETING!
	pQueueSet->DeleteTagged();

	// Commented by Stefan Mihaila
	// pQueueSet->CreateTemp();

	DPRINTF(("****** pQueueSet danach******"));
	pQueueSet->DebugOut();

	pQueueSet->ClearModified();

	DEXIT;
}

/*
** GetJobSet
**
** Refreshes the JobSet passed by pJobSet, regarding the selection specified
** by pNodeInfoSet
*/
void CQmonntDoc::GetJobSet(CJobSet *pJobSet, CNodeInfoSet *pNodeInfoSet)
{
	ASSERT(NULL != pJobSet);
	ASSERT(NULL != pNodeInfoSet);

	// TODO: Implementation similar to GetQueueSet!
	pJobSet->Clear();

	CJobList::iterator jit;
	for (jit = m_pJobList->begin(); jit != m_pJobList->end(); jit++)
		if (pNodeInfoSet->Contains(jit->GetID()))
			pJobSet->Add(*jit);
}

/*
** GetHostSet
**
** Refreshes the HostSet passed by pHostSet, regarding the selection specified
** by pNodeInfoSet
*/
void CQmonntDoc::GetHostSet(CHostSet * pHostSet, CNodeInfoSet *pNodeInfoSet)
{
	ASSERT(NULL != pHostSet);
	ASSERT(NULL != pNodeInfoSet);

	// TODO: Implementation similar to GetQueueSet!
	pHostSet->Clear();

	CHostList::iterator hit;
	for (hit = m_pHostList->begin(); hit != m_pHostList->end(); hit++)
		if (pNodeInfoSet->Contains(hit->GetID()))
			pHostSet->Add(*hit);
}

void CQmonntDoc::GetComplexAtributeSet(CComplexAtributeSet *pAtributeSet, CNodeInfoSet *pNodeInfoSet)
{
	ASSERT(NULL != pAtributeSet);
	ASSERT(NULL != pNodeInfoSet);

	pAtributeSet->Clear();

	CComplexAtributeList::iterator Iterator;
	for (CComplexList::iterator cit = m_pComplexList->begin(); cit != m_pComplexList->end(); cit++)
		for (Iterator = cit->AtributeList.begin(); Iterator != cit->AtributeList.end(); Iterator++)
			if (pNodeInfoSet->Contains(Iterator->GetID()))
				pAtributeSet->Add(*Iterator);
}

/*
** OnEditRefresh
**
** 
*/
void CQmonntDoc::OnEditRefresh() 
{
	// Stefan Mihaila: Please read the comment from 'CCodTreeView::OnInitialUpdate()'
	/*// Prepare a 'MessageView' in the right, to display connection messages.
	SetMessage(IDS_MSG_CONTACTINGCODINE);
	CMainFrame *MainFrame = dynamic_cast<CMainFrame *>(AfxGetMainWnd());
	ASSERT_VALID(MainFrame);
	MainFrame->SetStatusText(IDS_MSG_RECV);*/

	RetrieveAllComplexes();
	RetrieveAllQueues();
	RetrieveAllHosts();
	RetrieveAllJobs();
}

/*
** RetrieveAllQueues (public)
**
** demands a list of all queues from the workerthread
*/
void CQmonntDoc::RetrieveAllQueues()
{
	m_bGetQueueListSucceeded	= false;
	PostCommand(TC_GETALLQUEUES);
}

/*
** RetrieveAllJobs (public)
**
** Demands the job-list from the worker-thread
*/
void CQmonntDoc::RetrieveAllJobs()
{
	m_bGetJobListSucceeded = false;
	PostCommand(TC_GETALLJOBS);
}

/*
** RetrieveAllHosts (public)
**
** Demands the entire host-list from the worker-thread
*/
void CQmonntDoc::RetrieveAllHosts()
{
	m_bGetHostListSucceeded = false;
	PostCommand(TC_GETALLHOSTS);
}

/*
** RetrieveAllComplexes (public)
**
** Demands the entire complex-list from the worker-thread
*/

void CQmonntDoc::RetrieveAllComplexes()
{
	m_bGetComplexListSucceeded = false;
	PostCommand(TC_GETALLCOMPLEXES);
}

/*
** PostCommand
**
** Sends the specified command to the workerthread if there isn't
** a command already waiting in the command-queue.
** If Command equals TC_NOCOMMAND only the next command in the 
** command-queue will be sent (if there exists one).
** A command will not be sent if the workerthread isn't idle.
** The return-value is TRUE, if the command is being sent immediately.
** (e.g. the workerthread is idle) , otherwise it is FALSE.
** In case of TC_NOCOMMAND a return-value of TRUE means that 
** a command has been sent successfully, a return
** value of FALSE indicates that there is an empty queue.
*/
bool CQmonntDoc::PostCommand(CCommand Command)
{
	bool	 bResult;
	CCommand NextCommand;

	if (m_bWorkerThreadIdle) {
		// A command can be sent immediately
		if (TC_NOCOMMAND == Command.m_nCode) {
			// process buffer
			if (m_CommandQueue.empty()) 
				bResult = false;	 //nothing to do
			else {
				// send next command
				bResult = true;
				NextCommand = m_CommandQueue.front();
				m_CommandQueue.pop_front();
				SendCommand(NextCommand);
			}
		} 
		else {
			// add command and send next one
			if (m_CommandQueue.empty()) {
				bResult = true;		 // Current command is submitted immediately
				SendCommand(Command);
			} 
			else {
				// push current command into queue and send next one 
				m_CommandQueue.push_back(Command);
				// TODO: Erase multiple identical commands
				NextCommand = m_CommandQueue.front();
				m_CommandQueue.pop_front();
				SendCommand(NextCommand);
			}
		}
	} 
	else {
		if (TC_NOCOMMAND == Command.m_nCode)
			bResult = true;	// ignore
		else {
			// just push command into queue
			bResult = false;
			m_CommandQueue.push_back(Command);
		}
	}

	return bResult;
}

/*
** SendCommand
**
** Sends the specified command to the workerthread.
** The thread then has to be idle.
** The idle-flag then is reset to non-idle.
*/
void CQmonntDoc::SendCommand(CCommand Command)
{
	ASSERT(m_bWorkerThreadIdle);

	m_bWorkerThreadIdle = false;
	
	CCodThreadInfo::CodThreadInfo.m_Command = Command.m_nCode;
	CCodThreadInfo::CodThreadInfo.m_Arg     = Command.m_Arg;

	// TODO: Assign command parameter
	SetEvent(CCodThreadInfo::CodThreadInfo.EventCommand);

	CMainFrame *MainFrame = dynamic_cast<CMainFrame *>(AfxGetMainWnd()); 
	ASSERT_VALID(MainFrame);

	// set the text in the status-bar to "recv" or "send" in order to
	// display data transfer from and to the codine-system
	switch (Command.m_nCode) {
		case TC_GETALLQUEUES:		// nobreak;
		case TC_GETALLJOBS:			// nobreak;
		case TC_GETALLHOSTS:		// nobreak;
		case TC_GETALLCOMPLEXES:	// nobreak;
		// >> place further 'receive' codes here

			MainFrame->SetStatusText(IDS_MSG_RECV);
			break;

		case TC_MODQUEUES:		// nobreak;
		// >> place further 'send' codes here

			MainFrame->SetStatusText(IDS_MSG_SEND);
			break;
	}
}

/*
** SetChangedData
**
** Joins the current queue-list and the queuset passed by pQueueSet and sends 
** its elements to the workerthread. The workerthread passes a 'change'-request
** to the codine-system. Any element the QueueSet contains is 
** marked as 'local'.
*/
void CQmonntDoc::SetChangedData(CQueueSet *pQueueSet)
{
	DENTER(GUI_LAYER, "CQmonntDoc::SetChangedData");

	ASSERT(NULL != pQueueSet);

	pQueueSet->MergeChanges();
	CQueueSet *pTempQueueSet = new CQueueSet(*pQueueSet); // the thread is responsible for destruction
	ASSERT(NULL != pTempQueueSet);

	DPRINTF((">>>>>>>> 1"));
	pQueueSet->DebugOut();
	DPRINTF(("<<<<<<<< 1"));

	for (CQueueList::iterator Iterator = pQueueSet->begin(); Iterator != pQueueSet->end(); Iterator++) {
		// make a copy of all set elements and save it in a CQueueList
		// this list is then passed to the thread
//		QueueList->push_back(*Iterator);

		// Set the "local"-Flag in every element of the set
		Iterator->SetFlag(CO_LOCAL);

		// remove all elements, that are contained in the Set
		m_pQueueList->RemoveByID(Iterator->GetID());
		DPRINTF(("RemoveID: %d\n", Iterator->GetID()));

		// Add all elements of the set to the queuelist
		m_pQueueList->push_back(*Iterator);
	}

	DPRINTF((">>>>>>>> 2"));
	pTempQueueSet->DebugOut();
	DPRINTF(("<<<<<<<< 2"));

	PostCommand(CCommand(TC_MODQUEUES, LPARAM(pTempQueueSet)));
	RetrieveAllQueues();

	m_pQueueList->DebugOut();
	UpdateAllViews(NULL, UAVH_QUEUES);

	DEXIT;
}

void CQmonntDoc::SetChangedData(CComplexAtributeSet * pAtributeSet)
{
/*
CComplexAtributeSet *pTempAtributeSet;
CComplexAtributeList::iterator Iterator;

	DENTER(GUI_LAYER, "CQmonntDoc::SetChangedData");

	pAtributeSet->MergeChanges();
	pTempAtributeSet = new CComplexAtributeSet(*pAtributeSet); // the thread is responsible for destruction
	DPRINTF((">>>>>>>> 1"));
	pAtributeSet->DebugOut();
	DPRINTF(("<<<<<<<< 1"));

	Iterator = pAtributeSet->begin();
	while(Iterator!=pAtributeSet->end()) {
		// make a copy of all set elements and save it in a CComplexAtributeList
		// this list is then passed to the thread
//		QueueList->push_back(*Iterator);

		// Set the "local"-Flag in every element of the set
		(*Iterator).SetFlag(CO_LOCAL);

		// remove all elements, that are contained in the Set
		m_pQueueList->RemoveByID((*Iterator).GetID());
		DPRINTF(("RemoveID: %d\n", (*Iterator).GetID()));

		// Add all elements of the set to the queuelist
		m_pQueueList->push_back(*Iterator);

		Iterator++;
	}

	DPRINTF((">>>>>>>> 2"));
	pTempAtributeSet->DebugOut();
	DPRINTF(("<<<<<<<< 2"));

	PostCommand(CCommand(TC_MODQUEUES, LPARAM(pTempQueueSet)));
	PostCommand(CCommand(TC_GETALLQUEUES));

	m_pQueueList->DebugOut();
	UpdateAllViews(NULL, UAVH_QUEUES);
	DEXIT;
*/
}

/*
** DeleteQueues
**
** Submits a "delete"-request to the workerthread concerning 
** all the queues contained by the specified set
*/
void CQmonntDoc::DeleteQueues(CNodeInfoSet *pSet)
{
	ASSERT(NULL != pSet);

	CQueueSet *pTempQueueSet = new CQueueSet;		// thread is responsible for destruction
	ASSERT(NULL != pTempQueueSet);

	for (CNodeInfoSet::iterator NodeInfoIt = pSet->begin(); NodeInfoIt != pSet->end(); NodeInfoIt++)
		pTempQueueSet->Add(*(m_pQueueList->FindByID(NodeInfoIt->m_ID)));
	pTempQueueSet->DebugOut();

	PostCommand(CCommand(TC_DELALLQUEUES, LPARAM(pTempQueueSet)));
	RetrieveAllQueues();

	UpdateAllViews(NULL, UAVH_QUEUES);
}

/*
** DeleteJobs
**
** Submits a "delete"-request to the workerthread concerning 
** all the jobs contained by the specified set
*/
void CQmonntDoc::DeleteJobs(CNodeInfoSet *pSet)
{
	ASSERT(NULL != pSet);

	CJobSet *pTempJobSet = new CJobSet;		// thread is responsible for destruction
	ASSERT(NULL != pTempJobSet);

	for (CNodeInfoSet::iterator NodeInfoIt = pSet->begin(); NodeInfoIt != pSet->end(); NodeInfoIt++)
		pTempJobSet->Add(*(m_pJobList->FindByID(NodeInfoIt->m_ID)));
	pTempJobSet->DebugOut();

	PostCommand(CCommand(TC_DELALLJOBS, LPARAM(pTempJobSet)));
	RetrieveAllJobs();

	UpdateAllViews(NULL, UAVH_JOBS);
}

void CQmonntDoc::SubmitJob(const CJob &Job)
{
	CJob *JobToSubmit = new CJob(Job);
	ASSERT(NULL != JobToSubmit);
	// Stefan Mihaila: 'JobToSubmit' will be deleted by the worker thread

	PostCommand(CCommand(TC_SUBMITJOB, LPARAM(JobToSubmit)));
}
