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
// Queue.cpp: Implementierung der Klasse CQueue.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include <stdlib.h>
#include "qmonnt.h"
#include "QmonntDoc.h"
#include "Queue.h"
#include "Debug.h"

extern "C" {
#include "cod_queueL.h"
#include "cod_complexL.h"
#include "cod_complex_schedd.h"
}

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

/*
** CQueue
**
** 
*/
CQueue::CQueue()
{
}

/*
** CQueue
**
** Konstruktor, der ein Queue-Objekt aus einem Cull-Listelement
** erzeugt und initialisiert.
*/
CQueue::CQueue(lListElem *qep)
{
	ASSERT(NULL != qep);

	qname				=	lGetString(qep, QU_qname);
	qhostname			=	lGetString(qep, QU_qhostname);
	qjobslots			=	lGetUlong(qep,	QU_job_slots);
	qtype				=	lGetUlong(qep,	QU_qtype);
	qpriority			=	lGetUlong(qep,	QU_priority);
	qshell				=	lGetString(qep, QU_shell);
	qtmpdir				=	lGetString(qep, QU_tmpdir);
	qstate				=	lGetUlong(qep,	QU_state);
	CString TimeString  =	lGetString(qep,	QU_notify);
	qnotifytime			=	CTime(1999, 1, 1, atoi((const char *) TimeString.Left(2)), 
			atoi((const char *) TimeString.Mid(TimeString.Find(':') + 1, 2)), 
			atoi((const char *) TimeString.Right(2))); 

	TimeString			=	lGetString(qep, QU_h_rt);
	qhardrealtime		=	-1 == TimeString.Find(':') ? StringToTime(TimeString) 
			: CTime(1999, 1, 1, atoi((const char *) TimeString.Left(2)), 
				atoi((const char *) TimeString.Mid(TimeString.Find(':') + 1, 2)), 
				atoi((const char *) TimeString.Right(2)));
	TimeString			=	lGetString(qep, QU_h_cpu);
	qhardcputime		=	-1 == TimeString.Find(':') ? StringToTime(TimeString) 
			: CTime(1999, 1, 1, atoi((const char *) TimeString.Left(2)), 
				atoi((const char *) TimeString.Mid(TimeString.Find(':') + 1, 2)), 
				atoi((const char *) TimeString.Right(2))); 

	qhardfilesize		=	lGetString(qep,	QU_h_fsize);
	qharddatasize		=	lGetString(qep,	QU_h_data);
	qhardstacksize		=	lGetString(qep,	QU_h_stack);
	qhardcorefilesize	=	lGetString(qep,	QU_h_core);
	qhardresidentsetsize=	lGetString(qep,	QU_h_rss);
	qhardvirtualmemory  =	lGetString(qep,	QU_h_vmem);

	TimeString			=	lGetString(qep,	QU_s_rt);
	qsoftrealtime		=	-1 == TimeString.Find(':') ? StringToTime(TimeString) 
			: CTime(1999, 1, 1, atoi((const char *) TimeString.Left(2)), 
				atoi((const char *) TimeString.Mid(TimeString.Find(':') + 1, 2)), 
				atoi((const char *) TimeString.Right(2)));
	TimeString			=	lGetString(qep,	QU_s_cpu);
	qsoftcputime		=	-1 == TimeString.Find(':') ? StringToTime(TimeString) 
			: CTime(1999, 1, 1, atoi((const char *) TimeString.Left(2)), 
				atoi((const char *) TimeString.Mid(TimeString.Find(':') + 1, 2)), 
				atoi((const char *)TimeString.Right(2)));
	qsoftfilesize		=	lGetString(qep,	QU_s_fsize);
	qsoftdatasize		=	lGetString(qep,	QU_s_data);
	qsoftstacksize		=	lGetString(qep,	QU_s_stack);
	qsoftcorefilesize	=	lGetString(qep,	QU_s_core);
	qsoftresidentsetsize=	lGetString(qep,	QU_s_rss);
	qsoftvirtualmemory  =	lGetString(qep,	QU_s_vmem);

	lList *lp = lGetList(qep, QU_complex_list);
	lListElem *ep;
	for_each_cpp(ep, lp)
		qComplexList.push_back(CQueueComplex(ep));

	lp = lGetList(qep, QU_migr_load_thresholds);
	for_each_cpp(ep, lp)
		qCheckPointingList.push_back(CComplexAtribute(ep));
	// Stefan Mihaila: Warning ! The qCheckPointingList doesn't contain
	// the proper 'valtype' field values for CheckPointing elements yet !!!
	// This is because of job manager, that doesn't fill this field, as
	// it doesn't need it (it uses 'stringval' field instead)
	// Only the scheduler needs this type information and it will fill in
	// this field only when really needs it (eg: when it checks if 
	// a checkpointing condition is met for a queue.
}

/*
** ~CQueue
**
** 
*/
CQueue::~CQueue()
{
}

//////////////////////////////////////////////////////////////////////
// global conversion function

CTime CQueue::StringToTime(const CString &timeString)
{
	UINT year, month, day, hour, min, sec, seconds;
	CString Temp;

	seconds = atoi((const char *) timeString);

	Temp.Format("%d", seconds);
	Temp = timeString.Mid(Temp.GetLength(), 1);
	if (!Temp.IsEmpty())
		switch (Temp[0]) {
			case 'm':
				seconds *= 1000000;
				break;
		}

	if (seconds > 3599999 || timeString == "INFINITY") {
		year = 2037;
		month = 1;
		day = 1;
		hour = 0;
		min = 0;
		sec = 0;
	}
	else {
		if (seconds > 86399) {
			if (seconds > 2678399) {
				month = ((seconds - 2678399) / 3600) / 24 + 1;
				day = 31;
			}
			else {
				month = 1;
				day = ((seconds - 86399) / 3600) / 24 +1;
			}
		}
		else {
			month =1;
			day =1;
		}

		hour = (seconds - (day - 1) * 24 * 3600 - (month - 1) * 24 * 3600) / 3600;
		min  = (seconds - (day - 1) * 24 * 3600 - (month - 1) * 24 * 3600 - hour * 3600) / 60;
		sec  = (seconds - (day - 1) * 24 * 3600 - (month - 1) * 24 * 3600 - hour * 3600 - min * 60);
		year = 1999;
	}

	return CTime(year, month, day, hour, min, sec);
}

CString CQueue::TimeToString(const CTime &stringTime)
{
	UINT hour, min, sec;
	CString Temp;

	if (2037 == stringTime.GetYear())
		Temp = "INFINITY";
	else {
		hour = ((stringTime.GetMonth() - 1) * 24) + ((stringTime.GetDay() - 1) * 24) + stringTime.GetHour();
		min  = stringTime.GetMinute();
		sec  = stringTime.GetSecond();
		Temp.Format("%d:%d:%d", hour, min, sec);
	}

	return Temp;
}

/*
** operator lListElem*
**
** Wandelt das Queue-Objekt in ein Cull-List-Element um.
** HINWEIS: Das zur�ckgegebene List-Element mu� von der aufrufenden
** Funktion gel�scht werden!
*/
CQueue::operator lListElem* ()
{
    lListElem *ep = lCreateElem(QU_Type);
	ASSERT(NULL != ep);
	
	lSetString	(ep, QU_qname,			qname.GetBuffer(0));	// Schl�sselfeld, wird immer ben�tigt!

	lSetUlong	(ep, QU_job_slots,		qjobslots);
	lSetString	(ep, QU_qhostname,		qhostname.GetBuffer(0));
	lSetUlong	(ep, QU_priority,		qpriority);
	lSetString	(ep, QU_shell,			qshell.GetBuffer(0));
	lSetString	(ep, QU_tmpdir,			qtmpdir.GetBuffer(0));
	lSetUlong	(ep, QU_state,			qstate);
	lSetString  (ep, QU_notify,			qnotifytime.Format("%H:%M:%S").GetBuffer(0));
	lSetString	(ep, QU_h_rt,			TimeToString(qhardrealtime).GetBuffer(0));
	lSetString	(ep, QU_h_cpu,			TimeToString(qhardcputime).GetBuffer(0));
	lSetString	(ep, QU_h_fsize,		qhardfilesize.GetBuffer(0));
	lSetString	(ep, QU_h_data,			qharddatasize.GetBuffer(0));
	lSetString	(ep, QU_h_stack,		qhardstacksize.GetBuffer(0));
	lSetString	(ep, QU_h_core,			qhardcorefilesize.GetBuffer(0));
	lSetString	(ep, QU_h_rss,			qhardresidentsetsize.GetBuffer(0));
	lSetString	(ep, QU_h_vmem,			qhardvirtualmemory.GetBuffer(0));
	lSetString	(ep, QU_s_rt,			TimeToString(qsoftrealtime).GetBuffer(0));
	lSetString	(ep, QU_s_cpu,			TimeToString(qsoftcputime).GetBuffer(0));
	lSetString	(ep, QU_s_fsize,		qsoftfilesize.GetBuffer(0));
	lSetString	(ep, QU_s_data,			qsoftdatasize.GetBuffer(0));
	lSetString	(ep, QU_s_stack,		qsoftstacksize.GetBuffer(0));
	lSetString	(ep, QU_s_core,			qsoftcorefilesize.GetBuffer(0));
	lSetString	(ep, QU_s_rss,			qsoftresidentsetsize.GetBuffer(0));
	lSetString	(ep, QU_s_vmem,			qsoftvirtualmemory.GetBuffer(0));
	// >>> Code f�r neue Felder hier einf�gen

	lList *lp = NULL;
	if (!qComplexList.empty()) {
		lp = lCreateList("complexes", CX_Type);
		ASSERT(NULL != lp); 
	}

	lListElem *ap;
	for (CQueueComplexList::iterator qcit = qComplexList.begin(); qcit != qComplexList.end(); qcit++) {
		ap = (lListElem *) *qcit;
		ASSERT(NULL != ap);
		lAppendElem(lp, ap);
	}
	lSetList(ep, QU_complex_list, lp);

	lp = NULL;
	if (!qCheckPointingList.empty()) {
		lp = lCreateList("checkpointings", CE_Type);
		ASSERT(NULL != lp);
	}

	for (CComplexAtributeList::iterator cait = qCheckPointingList.begin(); cait != qCheckPointingList.end(); cait++) {
		ap = (lListElem *) *cait;
		ASSERT(NULL != ap);
		lAppendElem(lp, ap);
	}
	lSetList(ep, QU_migr_load_thresholds, lp);

	return ep;
}

CComplexAtributeList CQueue::GetAllAvailableCheckPointings(CQmonntDoc *pDoc)
{
	ASSERT_VALID(pDoc);

	ASSERT(NULL != pDoc->m_pComplexList);
	lList *lComplex = pDoc->m_pComplexList->MakelList();
	//ASSERT(NULL != lComplex);

	ASSERT(NULL != pDoc->m_pHostList);
	lList *lHost = pDoc->m_pHostList->MakelList();
	//ASSERT(NULL != lHost);

	lList *lQueueComplex = qComplexList.MakelList();
	//ASSERT(NULL != lQueueComplex);
	
	lListElem *qep = lCreateElem(QU_Type);
	ASSERT(NULL != qep);
	lSetString(qep, QU_qname,		 qname.GetBuffer(0));
	lSetString(qep, QU_qhostname,	 qhostname.GetBuffer(0));
	lSetList(  qep, QU_complex_list, lQueueComplex);

	lList *entries = NULL;
	queue_complexes2scheduler(&entries, qep, lHost, lComplex, 0);   
	ASSERT(NULL != entries);

	lCondition *lWhereCondition = lWhere("%T(%I != %s)", CE_Type, CE_name, "slots");
	ASSERT(NULL != lWhereCondition);
	entries = lSelectDestroy(entries, lWhereCondition);
lFreeWhere(&lWhereCondition);

	// Fill in the all checkpointings list
	CComplexAtributeList AllCheckPointings;
	lListElem *ep;
	for_each_cpp(ep, entries)
		AllCheckPointings.push_back(CComplexAtribute(ep));
		
	// Free the lists
lFreeElem(&qep);
    lFreeList(&entries);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
    lFreeList(&lHost);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
    lFreeList(&lComplex);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      

	return AllCheckPointings;
}

/******************************************************************************
****                                                                       ****
**** Klasse: CQueueList                                                    ****
****                                                                       ****
******************************************************************************/

/*
** FindByID
**
** Sucht ein Element mit der angegebenen ID in der Liste
** und liefert den Iterator darauf zur�ck. Falls der Iterator
** gleich end() ist, wurde das Element nicht gefunden!
*/
CQueueList::iterator CQueueList::FindByID(ULONG ID)
{
	for (CQueueList::iterator it = begin(); it != end(); it++)
		if (ID == it->GetID())
			break;
	
	return it;
}

/*
** RemoveByID
**
** L�scht das Element mit der angegebenen ID aus der Liste.
*/
void CQueueList::RemoveByID(ULONG ID)
{
	CQueueList::iterator it = FindByID(ID);
	if (it != end())
		erase(it);
}

/*
** DebugOut (public)
**
** Gibt den Inhalt der Queue-Liste auf der Debug-Konsole aus.
** (Kontext GUI_LAYER).
*/
void CQueueList::DebugOut()
{
	DENTER(GUI_LAYER, "CQueueList::DebugOut");

	DPRINTF(("--- CQueueList: -----------"));
	for (CQueueList::iterator it = begin(); it != end(); it++)
		DPRINTF(("ID: %ld, Queue: %s, Hostname: %s, Jobslots: %ld", it->GetID(), 
			it->qname, it->qhostname, it->qjobslots));
	DPRINTF((""));

	DEXIT;
}

lList *CQueueList::MakelList()
{
	lList *lp = NULL;

	if (!empty()) {
		lp = lCreateList("queues", QU_Type);
		ASSERT(NULL != lp);
	}
	
	lListElem *ep;
	for (CQueueList::iterator Iterator = begin(); Iterator != end(); Iterator++) {
		ep = (lListElem *) *Iterator;
		ASSERT(NULL != ep);
		lAppendElem(lp, ep);
	}

	return lp;
}
