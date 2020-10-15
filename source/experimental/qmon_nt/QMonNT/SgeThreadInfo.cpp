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
// CodThreadInfo.cpp: Implementation of the class CCodThreadInfo.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"

#include "CodThreadInfo.h"
#include "qmonnt.h"
#include "Messages.h"
#include "Debug.h"
#include "Queue.h"
#include "Job.h"
#include "Host.h"
#include "Complexes.h"
#include "cull_list.h"

namespace Codine {	// the namespace is needed, because the USN symbol is 
					// used both in Codine(userset NAMEDEF) and in winnt.h (Update Sequence Number)
					// too
extern "C" {
#include "cod_api.h"
#include "cod_api_intern.h"
#include "cod_queueL.h" 
#include "cod_hostL.h"
#include "cod_prognames.h"
#include "cod_all_listsL.h"
#include "utility.h"
#include "def.h"
#include "shutdown.h"
#include "qm_name.h"
}

} // namespace Codine

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CCodThreadInfo CCodThreadInfo::CodThreadInfo;  // declared in CodThreadInfo.h

//////////////////////////////////////////////////////////////////////
// construction/destruction
//////////////////////////////////////////////////////////////////////

CCodThreadInfo::CCodThreadInfo()
{
	EventKillThread = CreateEvent(NULL, FALSE, FALSE, NULL);
	ASSERT(NULL != EventKillThread);
	EventThreadKilled = CreateEvent(NULL, FALSE, FALSE, NULL);
	ASSERT(NULL != EventThreadKilled);
	EventCommand = CreateEvent(NULL, FALSE, FALSE, NULL);
	ASSERT(NULL != EventCommand);

	m_Command  = TC_NOCOMMAND;
	MainWindow = NULL;
}

CCodThreadInfo::~CCodThreadInfo()
{
	ASSERT(NULL != EventKillThread);
	CloseHandle(EventKillThread);
	EventKillThread = NULL;

	ASSERT(NULL != EventThreadKilled);
	CloseHandle(EventThreadKilled);
	EventThreadKilled = NULL;

	ASSERT(NULL != EventCommand);
	CloseHandle(EventCommand);
	EventCommand = NULL;
}

CWinThread *CCodThreadInfo::StartThread()
{
	return AfxBeginThread(CodThreadProc, NULL);
}

/*
** CodThreadProc
**
** thread-function, that manages the communication between the GUI and Codine
*/
UINT CCodThreadInfo::CodThreadProc(LPVOID pParam)
{
	using namespace Codine;

	DENTER(TOP_LAYER, "CodThreadProc");

	// The main window is saved. It should stay the same all the time.
	if (NULL == CodThreadInfo.MainWindow) {
		CWinApp *App = AfxGetApp();
		ASSERT_VALID(App);
		CodThreadInfo.MainWindow = App->GetMainWnd();
	}
	ASSERT(NULL != CodThreadInfo.MainWindow);

	/*
	** Initialisieren und Codine-Master kontaktieren:
	*/

	cod_api_param(SET_ISALIVE, 0, 0);
    lInit(Codine::nmv);
	prepare_enroll("QMonNT", 0, NULL);
	lList *alp = lCreateList("answer", AN_Type);
	ASSERT(NULL != alp);
	cod_setup(QUSERDEFINED, &alp);

	// Answerlist erzeugen und mit Message ans Dokument �bergeben.
	// Dokument ist f�r das L�schen der Answerlist zust�ndig!
	CodThreadInfo.AnswerList.Append(alp);
	alp = lFreeList(alp);

	// Commented by Stefan Mihaila
	/*int setup_ok = 0 == check_isalive(cod_get_master(0)) 
		? AE_OK : AE_QMASTER_DOWN;

	switch (setup_ok) {
		case AE_OK:
			// QMaster konnte kontaktiert werden. Meldung
			// schicken und bei Auftragsschleife weitermachen:
			CodThreadInfo.MainWindow->SendMessage(WM_CODNOTIFY, TR_CONNECTIONSETUP_SUCCESS, NULL);
			break;

		case AE_QMASTER_DOWN:
			// QMaster ist down. Meldung schicken, api beenden und bei
			// Auftragsschleife weitermachen:
			CodThreadInfo.MainWindow->SendMessage(WM_CODNOTIFY, TR_CONNECTIONSETUP_FAILURE, NULL);
			cod_api_shutdown();
			break;

		default:
			// Anderes Problem. Meldung schicken und bei 
			// Auftragsschleife weitermachen:
			CodThreadInfo.MainWindow->SendMessage(WM_CODNOTIFY, TR_CONNECTIONSETUP_UNKNOWNERR, NULL);
			// TODO: Ja, ja, dieser Fall kann hier ja nie auftreten, ich wei�.
			// aber vielleicht braucht man es ja mal......
			break;
	}*/

	/*
	** Beginn der Auftragsschleife. Hier wird auf ein Event
	** gewartet, das vom Dokument gesetzt wird. Anschlie�end
	** wird das Kommando, das mit dem Event verbunden ist,
	** ausgef�hrt. In diese Schleife wird auch gesprungen,
	** wenn beim Setup ein Fehler auftrat, die diese Schleife
	** auf das Beendigungsflag pr�ft.
	*/
	UINT Command;
	while (true) {
		// Auf Kommandomeldung warten:
		WaitForSingleObject(CodThreadInfo.EventCommand, INFINITE);

		Command = CodThreadInfo.m_Command;
		ASSERT(Command > 0);

		switch (Command) {
			case TC_SHUTDOWN:
				cod_shutdown();
				SetEvent(CodThreadInfo.EventThreadKilled);
				DEXIT;
				return 0;

			case TC_GETALLQUEUES:
				CodThreadInfo.GetAllQueues();
				break;

			case TC_GETALLJOBS:
				CodThreadInfo.GetAllJobs();
				break;

			case TC_GETALLHOSTS:
				CodThreadInfo.GetAllHosts();
				break;

			case TC_GETALLCOMPLEXES:
				CodThreadInfo.GetAllComplexes();
				break;

			case TC_MODQUEUES:
				ASSERT(NULL != reinterpret_cast<CQueueSet *>(CodThreadInfo.m_Arg));
				CodThreadInfo.ModQueues(reinterpret_cast<CQueueSet *>(CodThreadInfo.m_Arg));
				break;

			case TC_DELALLQUEUES:
				ASSERT(NULL != reinterpret_cast<CQueueSet *>(CodThreadInfo.m_Arg));
				CodThreadInfo.DelAllQueues(reinterpret_cast<CQueueSet *>(CodThreadInfo.m_Arg));
				break;

			case TC_DELALLJOBS:
				ASSERT(NULL != reinterpret_cast<CJobSet *>(CodThreadInfo.m_Arg));
				CodThreadInfo.DelAllJobs(reinterpret_cast<CJobSet *>(CodThreadInfo.m_Arg));
				break;

			// Added by Stefan Mihaila
			case TC_DELALLHOSTS:
				ASSERT(NULL != reinterpret_cast<CHostSet *>(CodThreadInfo.m_Arg));
				CodThreadInfo.DelAllHosts(reinterpret_cast<CHostSet *>(CodThreadInfo.m_Arg));
				break;

			case TC_DELALLCOMPLEXES:
				ASSERT(NULL != reinterpret_cast<CComplexAtributeSet *>(CodThreadInfo.m_Arg));
				CodThreadInfo.DelAllComplexes(reinterpret_cast<CComplexAtributeSet *>(CodThreadInfo.m_Arg));
				break;

			case TC_SUBMITJOB:
				ASSERT(NULL != reinterpret_cast<CJob *>(CodThreadInfo.m_Arg));
				CodThreadInfo.SubmitJob(reinterpret_cast<CJob *>(CodThreadInfo.m_Arg));
				break;

			// >>> Code f�r neue Thread Commands hier einf�gen
			default:
				DPRINTF(("Unhandled ThreadCommand: %u\n", Command));
				ASSERT(false);
		}
	}

	DEXIT;
}

bool CCodThreadInfo::CheckQmasterIsAlive()
{
	using namespace Codine;

	ASSERT(NULL != CodThreadInfo.MainWindow);

	bool Alive = 0 == check_isalive(cod_get_master(0));
	if (!Alive)
		CodThreadInfo.MainWindow->SendMessage(WM_CODNOTIFY, TR_CONNECTION_FAILURE, NULL);
		
	return Alive;
}

/*
** GetAllQueues
**
** Holt vom QMaster alle Queues ab, baut eine CQueueList auf und sendet das
** Ergebnis ans Dokument.
*/
void CCodThreadInfo::GetAllQueues()
{
	DENTER(TOP_LAYER, "CCodThreadInfo::GetAllQueues");

	ASSERT(NULL != MainWindow);

	// Stefan Mihaila: Check for connection with Qmaster first
	if (!CheckQmasterIsAlive()) {
		DEXIT;
		return;
	}

	lEnumeration *whatall = lWhat("%T(ALL)", Codine::QU_Type);
	ASSERT(NULL != whatall);
	lList *lp = NULL;
	lList *alp = cod_api(COD_QUEUE_LIST, COD_API_GET, &lp, NULL, whatall);
	ASSERT(NULL != alp);
	AnswerList.Append(alp);
lFreeWhat(&whatall);
    lFreeList(&alp);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           

	// Stefan Mihaila: if something was wrong,
	// send the proper notification message to 'MainWindow'
	if (!AnswerList.IsEmpty()) {
		// Stefan Mihaila: Warning: 'lp' may or may be not NULL !
		lp = lFreeList(lp);
		MainWindow->SendMessage(WM_CODNOTIFY, TR_CODAPI_FAILURE, 0);	
		DEXIT;
		return;
	}
	
	// 'cod_api()' request was succesfull
	// If 'lp' is NULL, that means that the expected list is empty

	// Stefan Mihaila: The deletion of QueueList pointer is the responsability
	// of the WM_CODNOTIFY handler function !!!
	CQueueList *QueueList = new CQueueList;
	ASSERT(NULL != QueueList);

	// Daten aus lp holen und CQueueList aufbauen:
	lListElem *ep;
	for_each_cpp(ep, lp) {
		CQueue q(ep);
		QueueList->push_back(q);
		DPRINTF(("ID: %lu, Queuename: %s, Hostname: %s", q.GetID(), q.qname, q.qhostname));
	}
	// Stefan Mihaila: Warning: 'lp' may or may be not NULL !
	lp = lFreeList(lp);

	MainWindow->SendMessage(WM_CODNOTIFY, TR_GETQUEUELIST_SUCCESS, LPARAM(QueueList));
	
	DEXIT;
}

/*
** GetAllJobs
**
** Holt vom QMaster alle Jobs ab, baut eine CJobList auf und sendet das Ergebnis
** ans Dokument.
*/
void CCodThreadInfo::GetAllJobs()
{
	DENTER(TOP_LAYER, "CCodThreadInfo::GetAllJobs");

	ASSERT(NULL != MainWindow);

	// Stefan Mihaila: Check for connection with Qmaster first
	if (!CheckQmasterIsAlive()) {
		DEXIT;
		return;
	}

	/*lCondition *whereAllExceptFinished = lWhere("%T(!(%I m= %u))", Codine::JB_Type, 
		Codine::JB_status, JFINISHED);
	ASSERT(NULL != whereAllExceptFinished);*/

	lEnumeration *whatall = lWhat("%T(ALL)", Codine::JB_Type);
	ASSERT(NULL != whatall);

	lList *lp = NULL;
	lList *alp = cod_api(COD_JOB_LIST, COD_API_GET, &lp, 
		NULL /*whereAllExceptFinished*/, whatall);
	ASSERT(NULL != alp);
	AnswerList.Append(alp);
lFreeWhere(&whereAllExceptFinished);
	whatall = lFreeWhat(whatall);
    lFreeList(&alp);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           

	// Stefan Mihaila: if something was wrong,
	// send the proper notification message to 'MainWindow'
	if (!AnswerList.IsEmpty()) {
		// Stefan Mihaila: Warning: 'lp' may or may be not NULL !
		lp = lFreeList(lp);
		MainWindow->SendMessage(WM_CODNOTIFY, TR_CODAPI_FAILURE, 0);	
		DEXIT;
		return;
	}

	// 'cod_api()' request was succesfull
	// If 'lp' is NULL, that means that the expected list is empty

	// The deletion of JobList pointer is the responsability
	// of the WM_CODNOTIFY handler function !!!
	CJobList *JobList = new CJobList;	// F�r das L�schen ist das Dokument zust�ndig!
	ASSERT(NULL != JobList);

	// Daten aus lp holen und CJobList aufbauen:
	lListElem *ep;
	for_each_cpp(ep, lp) {
		CJob j(ep);
		JobList->push_back(j);
		DPRINTF(("ID: %lu, Job-Number: %lu", j.GetID(), j.job_number));
	}
	// Stefan Mihaila: Warning: 'lp' may or may be not NULL !
	lp = lFreeList(lp);

	MainWindow->SendMessage(WM_CODNOTIFY, TR_GETJOBLIST_SUCCESS, LPARAM(JobList));

	DEXIT;
}

/*
** GetAllHosts
**
** Holt vom QMaster alle Hosts ab, baut eine CHostList auf und sendet
** das Ergebnis ans Dokument.
*/
void CCodThreadInfo::GetAllHosts()
{
	DENTER(TOP_LAYER, "CCodThreadInfo::GetAllHosts");

	ASSERT(NULL != MainWindow);

	// Stefan Mihaila: Check for connection with Qmaster first
	if (!CheckQmasterIsAlive()) {
		DEXIT;
		return;
	}

	lEnumeration *whatall = lWhat("%T(ALL)", Codine::EH_Type);
	ASSERT(NULL != whatall);
	lList *lp = NULL;
	lList *alp = cod_api(COD_EXECHOST_LIST, COD_API_GET, &lp, NULL, whatall);
	ASSERT(NULL != alp);
	AnswerList.Append(alp);
	whatall = lFreeWhat(whatall);
	alp		= lFreeList(alp);

	// Stefan Mihaila: if something was wrong,
	// send the proper notification message to 'MainWindow'
	if (!AnswerList.IsEmpty()) {
		// Stefan Mihaila: Warning: 'lp' may or may be not NULL !
		lp = lFreeList(lp);
		MainWindow->SendMessage(WM_CODNOTIFY, TR_CODAPI_FAILURE, 0);	
		DEXIT;
		return;
	}

	// 'cod_api()' request was succesfull
	// If 'lp' is NULL, that means that the expected list is empty

	// The deletion of HostList pointer is the responsability
	// of the WM_CODNOTIFY handler function !!!
	CHostList *HostList = new CHostList;	// F�r das L�schen ist das Dokument zust�ndig!
	ASSERT(NULL != HostList);

	// Daten aus lp holen und CHostList aufbauen:
	lListElem *ep;
	for_each_cpp(ep, lp) {
		CHost h(ep);
		HostList->push_back(h);
		DPRINTF(("ID: %lu, Host-Name: %s", h.GetID, h.hostname));
	}
	// Stefan Mihaila: Warning: 'lp' may or may be not NULL !
	lp = lFreeList(lp);

	MainWindow->SendMessage(WM_CODNOTIFY, TR_GETHOSTLIST_SUCCESS, LPARAM(HostList));

	DEXIT;
}

/*
** GetAllComplexes
**
** Holt vom QMaster alle Complexes ab, baut eine CComplexList auf und sendet das Ergebnis
** ans Dokument.
*/
void CCodThreadInfo::GetAllComplexes()
{
	DENTER(TOP_LAYER, "CCodThreadInfo::GetAllComplexes");

	ASSERT(NULL != MainWindow);

	// Stefan Mihaila: Check for connection with Qmaster first
	if (!CheckQmasterIsAlive()) {
		DEXIT;
		return;
	}

	lEnumeration *whatall = lWhat("%T(ALL)", Codine::CX_Type);
	ASSERT(NULL != whatall);
	lList *lp = NULL;
	lList *alp = cod_api(COD_COMPLEX_LIST, COD_API_GET, &lp, NULL, whatall);
	ASSERT(NULL != alp);
	AnswerList.Append(alp);
	whatall = lFreeWhat(whatall);
	alp		= lFreeList(alp);

	// Stefan Mihaila: if something was wrong,
	// send the proper notification message to 'MainWindow'
	if (!AnswerList.IsEmpty()) {
		// Stefan Mihaila: Warning: 'lp' may or may be not NULL !
		lp = lFreeList(lp);
		MainWindow->SendMessage(WM_CODNOTIFY, TR_CODAPI_FAILURE, 0);	
		DEXIT;
		return;
	}

	// 'cod_api()' request was succesfull
	// If 'lp' is NULL, that means that the expected list is empty

	// The deletion of ComplexList pointer is the responsability
	// of the WM_CODNOTIFY handler function !!!
	CComplexList *ComplexList = new CComplexList; 
	ASSERT(NULL != ComplexList);

	lListElem *ep;
	for_each_cpp(ep, lp) {
		CComplex cmplx(ep);
		ComplexList->push_back(cmplx);
		DPRINTF(("Name: %s", cmplx.name));
	}
	// Stefan Mihaila: Warning: 'lp' may or may be not NULL !
	lp = lFreeList(lp);

	MainWindow->SendMessage(WM_CODNOTIFY, TR_GETCOMPLEXLIST_SUCCESS, LPARAM(ComplexList));
	
	DEXIT;
}

/*
** ModQueues
**
** 
*/
void CCodThreadInfo::ModQueues(CQueueSet *pQueueSet)
{
	DENTER(TOP_LAYER, "CCodThreadInfo::ModQueues");

	ASSERT(NULL != MainWindow);
	ASSERT(NULL != pQueueSet);
	pQueueSet->DebugOut();

	// Stefan Mihaila: Check for connection with Qmaster first
	if (!CheckQmasterIsAlive()) {
		DEXIT;
		return;
	}

	lEnumeration *what = (lEnumeration *) *pQueueSet;
	ASSERT(NULL != what);
	lList *lp = (lList *) *pQueueSet;
	ASSERT(NULL != lp); // It must be at least one entry in the set

	DPRINTF(("************************* Mod Queue List ********"));
	lWriteList(lp);
	lWriteWhatTo(what, NULL);

	lList *alp = cod_api(COD_QUEUE_LIST, COD_API_MOD, &lp, NULL, what);
	ASSERT(NULL != alp);
	AnswerList.Append(alp);
	what = lFreeWhat(what);
    lFreeList(&alp);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
    lFreeList(&lp);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            

	// Stefan Mihaila: if something was wrong,
	// send the proper notification message to 'MainWindow'
	if (!AnswerList.IsEmpty()) {
		MainWindow->SendMessage(WM_CODNOTIFY, TR_CODAPI_FAILURE, 0);	
		DEXIT;
		return;
	}

	delete pQueueSet;
	pQueueSet = NULL;

	MainWindow->SendMessage(WM_CODNOTIFY, TR_READY, 0);
	DEXIT;
}

/*
** DelAllQueues
**
** 
*/
void CCodThreadInfo::DelAllQueues(CQueueSet *pQueueSet)
{
	DENTER(TOP_LAYER, "CCodThreadInfo::DelAllQueues");

	ASSERT(NULL != MainWindow);
	ASSERT(NULL != pQueueSet);

	// Stefan Mihaila: Check for connection with Qmaster first
	if (!CheckQmasterIsAlive()) {
		DEXIT;
		return;
	}

	lEnumeration *what = (lEnumeration *) *pQueueSet;
	ASSERT(NULL != what);
	lList *lp = (lList *) *pQueueSet;
	ASSERT(NULL != lp); // It must be at least one entry in the set

	lList *alp = cod_api(COD_QUEUE_LIST, COD_API_DEL, &lp, NULL, what);
	ASSERT(NULL != alp);
	AnswerList.Append(alp);
	what = lFreeWhat(what);
	alp  = lFreeList(alp);
    lFreeList(&lp);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            

	// Stefan Mihaila: if something was wrong,
	// send the proper notification message to 'MainWindow'
	if (!AnswerList.IsEmpty()) {
		MainWindow->SendMessage(WM_CODNOTIFY, TR_CODAPI_FAILURE, 0);	
		DEXIT;
		return;
	}

	delete pQueueSet;
	pQueueSet = NULL;

	MainWindow->SendMessage(WM_CODNOTIFY, TR_READY, 0);

	DEXIT;
}

/*
** DelAllJobs
**
** 
*/
void CCodThreadInfo::DelAllJobs(CJobSet *pJobSet)
{
	DENTER(TOP_LAYER, "CCodThreadInfo::DelAllJobs");

	ASSERT(NULL != MainWindow);
	ASSERT(NULL != pJobSet);

	// Stefan Mihaila: Check for connection with Qmaster first
	if (!CheckQmasterIsAlive()) {
		DEXIT;
		return;
	}

	lEnumeration *what = (lEnumeration *) *pJobSet;
	ASSERT(NULL != what);
	lList *lp = (lList *) *pJobSet;
	ASSERT(NULL != lp); // It must be at least one entry in the set

	lList *alp = cod_api(COD_JOB_LIST, COD_API_DEL, &lp, NULL, what);
	ASSERT(NULL != alp);
	AnswerList.Append(alp);
	what = lFreeWhat(what);
	alp  = lFreeList(alp);
	lp	 = lFreeList(lp);

	// Stefan Mihaila: if something was wrong,
	// send the proper notification message to 'MainWindow'
	if (!AnswerList.IsEmpty()) {
		MainWindow->SendMessage(WM_CODNOTIFY, TR_CODAPI_FAILURE, 0);	
		DEXIT;
		return;
	}

	delete pJobSet;
	pJobSet = NULL;

	MainWindow->SendMessage(WM_CODNOTIFY, TR_READY, 0);

	DEXIT;
}

/*
** DelAllHosts
**
** 
*/
void CCodThreadInfo::DelAllHosts(CHostSet *pHostSet)
{
	DENTER(TOP_LAYER, "CCodThreadInfo::DelAllHosts");

	ASSERT(NULL != MainWindow);
	ASSERT(NULL != pHostSet);

	// Stefan Mihaila: Check for connection with Qmaster first
	if (!CheckQmasterIsAlive()) {
		DEXIT;
		return;
	}

	lEnumeration *what = (lEnumeration *) *pHostSet;
	ASSERT(NULL != what);
	lList *lp = (lList *) *pHostSet;
	ASSERT(NULL != lp); // It must be at least one entry in the set

	lList *alp = cod_api(COD_EXECHOST_LIST, COD_API_DEL, &lp, NULL, what);
	ASSERT(NULL != alp);
	AnswerList.Append(alp);
	what = lFreeWhat(what);
	alp  = lFreeList(alp);
	lp	 = lFreeList(lp);

	// Stefan Mihaila: if something was wrong,
	// send the proper notification message to 'MainWindow'
	if (!AnswerList.IsEmpty()) {
		MainWindow->SendMessage(WM_CODNOTIFY, TR_CODAPI_FAILURE, 0);	
		DEXIT;
		return;
	}

	delete pHostSet;
	pHostSet = NULL;

	MainWindow->SendMessage(WM_CODNOTIFY, TR_READY, NULL);

	DEXIT;
}

/*
** DelAllComplexes
**
** 
*/
void CCodThreadInfo::DelAllComplexes(CComplexAtributeSet *pComplexAtributeSet)
{
	/*DENTER(TOP_LAYER, "CCodThreadInfo::DelAllJobs");

	ASSERT(NULL != MainWindow);
	ASSERT(NULL != pComplexAtributeSet);

	lEnumeration *what = (lEnumeration *) *pComplexAtributeSet;
	ASSERT(NULL != what);
	lList *lp = (lList *) *pComplexAtributeSet; // It must be at least one entry in the set
	ASSERT(NULL != lp);
	lList *alp = cod_api(COD_COMPLEX_LIST, COD_API_DEL, &lp, NULL, what);
	ASSERT(NULL != alp)
	AnswerList.Append(alp);

	alp  = lFreeList(alp);
	lp	 = lFreeList(lp);
	what = lFreeWhat(what);

	delete pComplexAtributeSet;
	pComplexAtributeSet = NULL;

	MainWindow->SendMessage(WM_CODNOTIFY, CN_READY, NULL);

	DEXIT;*/
}

void CCodThreadInfo::SubmitJob(CJob *Job)
{
	DENTER(TOP_LAYER, "CCodThreadInfo::SubmitJob");

	ASSERT(NULL != Job);
	ASSERT(NULL != MainWindow);

	// Stefan Mihaila: Check for connection with Qmaster first
	if (!CheckQmasterIsAlive()) {
		DEXIT;
		return;
	}

	CJobList JobList;
	JobList.push_back(*Job);
	lList *lp = JobList.MakelList();
	ASSERT(NULL != lp);

	lList *alp = cod_api(COD_JOB_LIST, COD_API_ADD, &lp, NULL, NULL);
	ASSERT(NULL != alp);
	AnswerList.Append(alp);
	alp		= lFreeList(alp);
	// Stefan Mihaila: Warning: 'lp' may or may be not NULL !
    lFreeList(&lp);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            

	delete Job;
	Job = NULL;

	// Stefan Mihaila: if something was wrong,
	// send the proper notification message to 'MainWindow'
	if (AnswerList.IsEmpty()) // 'cod_api()' request was succesfull ?
		MainWindow->SendMessage(WM_CODNOTIFY, TR_SUBMITJOB_SUCCESS, 0);	
	else
		MainWindow->SendMessage(WM_CODNOTIFY, TR_CODAPI_FAILURE, 0);	

	DEXIT;
}
