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
// JobSet.cpp: Implementierung der Klasse CJobSet.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "JobSet.h"
#include "Debug.h"

extern "C" {
#include "cod_jobL.h"
}

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CJobSet::CJobSet()
{
}

CJobSet::~CJobSet()
{
}

/*
** begin
**
** Liefert einen Iterator auf den Anfang der Job-Liste zurück.
*/
CJobList::iterator CJobSet::begin()
{
	return m_Jobs.begin();
}

/*
** end
**
** Liefert einen Iterator auf das Ende der Job-Liste zurück.
*/
CJobList::iterator CJobSet::end()
{
	return m_Jobs.end();
}

/*
** IsLocal
**
** Liefert 0, falls keines der im Set enthaltenen Elemente das Flag 'CO_LOCAL'
** gesetzt hat, 1, falls alle das Flag gesetzt haben und -1 falls einige es
** gesetzt haben.
*/
int CJobSet::IsLocal()
{
	bool set = false, notset = false;

	for (CJobList::iterator Iterator = m_Jobs.begin(); Iterator != m_Jobs.end(); Iterator++)
		if (Iterator->IsFlagSet(CO_LOCAL))
			set = true;
		else
			notset = true;
	
	return set ? (notset ? -1 : 1) : 0;
}

/*
** IsEmpty
**
** Liefert true, falls das Set leer ist.
*/
bool CJobSet::IsEmpty()
{
	return m_Jobs.empty();
}

/*
** MergeChanges
**
** Übernimmt alle Felder, die als geändert markiert wurden,
** aus dem Temp-Element in alle im Set enthaltenen Jobs.
*/
void CJobSet::MergeChanges()
{
	for (CJobList::iterator Iterator = m_Jobs.begin(); Iterator != m_Jobs.end(); Iterator++) 
	{
		if (IsModified(JB_job_number))		
			Iterator->job_number	= m_Temp.job_number;
//		if (IsModified(JB_status))		
//			Iterator->status	= m_Temp.status;
		if (IsModified(JB_notify))		
			Iterator->notify	= m_Temp.notify;
    	if (IsModified(JB_priority))		
			Iterator->priority	= m_Temp.priority;
    	if (IsModified(JB_restart))		
			Iterator->restart	= m_Temp.restart;
//    	if (IsModified(JB_hold))		
//			Iterator->hold		= m_Temp.hold;
    	if (IsModified(JB_merge_stderr))		
			Iterator->merge_stderr		= m_Temp.merge_stderr;
		if (IsModified(JB_submission_time))		
			Iterator->submission_time	= m_Temp.submission_time;    
//		if (IsModified(JB_start_time))		
//			Iterator->start_time		= m_Temp.start_time;    
		if (IsModified(JB_execution_time))		
			Iterator->execution_time	= m_Temp.execution_time;    
		if (IsModified(JB_mail_options))		
			Iterator->mail_options		= m_Temp.mail_options;
	
		/*CString jobfile;
		CString scriptfile;
		CString jobname;
		CString owner;
		CString cell;
		CString cwd;
		CString pe;
		CString account;

		CStrList predecessors;
		CStrList mailto;
		CStrList stdout_path;
		CStrList stderr_path;
		CStrList pe_range;
		CStrList job_args;
		CStrList pending_queues;
		CStrList running_queues;*/

		// >>> Code für neue Felder hier einfügen
	}
}

void CJobSet::Clear()
{
	m_Jobs.clear();
}

void CJobSet::Delete(CJob &j)
{
}

void CJobSet::Add(CJob &j)
{
	m_Jobs.push_back(j);
	RecalcAmbiguous();
}

CJob *CJobSet::GetTemp()
{
	ASSERT(!m_Jobs.empty());
	m_Temp = m_Jobs.front();
	return &m_Temp;
}

void CJobSet::DebugOut()
{
	DENTER(GUI_LAYER, "CJobSet::DebugOut");
	DPRINTF(("--- JobSet: -----------"));

	for (CJobList::iterator jit = m_Jobs.begin(); jit != m_Jobs.end(); jit++)
		DPRINTF(("Job-Number: %lu, Job-Script: %s", jit->job_number, jit->script_file));
	DPRINTF((""));

	DEXIT;
}

void CJobSet::RecalcAmbiguous()
{
	DENTER(GUI_LAYER, "CJobSet::RecalcAmbiguous");

	ClearAmbiguous();
	if (m_Jobs.size() > 1) {
		CJobList::iterator jit = m_Jobs.begin();
		CJob fj = *jit;

		while ((++jit) != m_Jobs.end()) {
			if (fj.job_number  != jit->job_number)	
				SetAmbiguous(JB_job_number);
			if (fj.job_file    != jit->job_file)		
				SetAmbiguous(JB_job_file);
			if (fj.script_file != jit->script_file)	
				SetAmbiguous(JB_script_file);
		}
	}

	DEXIT;
}

/*
** operator lList*
**
** Wandelt das JobSet in eine Cull-Liste um.
** HINWEIS: Die zurückgegebene Cull-Liste muß von der aufrufenden Funktion
** gelöscht werden!
*/
CJobSet::operator lList* ()
{
	lList *lp = lCreateList("jobs", JB_Type);
	ASSERT(NULL != lp);

	lListElem *ep;
	for (CJobList::iterator Iterator = m_Jobs.begin(); Iterator != m_Jobs.end(); Iterator++) {
		ep = (lListElem *) *Iterator;
		ASSERT(NULL != ep);
		lAppendElem(lp, ep);
	}

	return lp;
}

/*
** operator lEnumeration*
**
** Erzeugt einen What-Deskriptor für das JobSet. Die Felder im What-Deskriptor
** werden entsprechend den Modify-Flags gesetzt.
** HINWEIS: Der zurückgegebene What-Deskriptor muß von der aufrufenden Funktion 
** gelöscht werden!
*/
CJobSet::operator lEnumeration* ()
{
	SetModified(JB_job_name);	// Schlüsselfeld, wird immer benötigt!

	int *ModIntVector = GetModIntVector();
	ASSERT(NULL != ModIntVector);

	lEnumeration *what = lIntVector2What(JB_Type, ModIntVector);
	ASSERT(NULL != what);

	return what;
}

/*
** SetTag
**
** Setzt das Tag-Flag in allen im Set enthaltenen Objekten.
*/
void CJobSet::SetTag()
{
	for (CJobList::iterator it = m_Jobs.begin(); it != m_Jobs.end(); it++)
		it->SetFlag(CO_TAG);
}

/*
** ClearTag
**
** Löscht das Tag-Flag bei dem Objekt, das die angegebene ID besitzt.
*/
void CJobSet::ClearTag(ULONG ID)
{
	for (CJobList::iterator it = m_Jobs.begin(); it != m_Jobs.end(); it++) 
		if (ID == it->GetID())
			it->ClearFlag(CO_TAG);
}

/*
** DeleteTagged
**
** Löscht alle Elemente aus dem Job-Set, dessen Tag-Flag gesetzt ist.
*/
void CJobSet::DeleteTagged()
{
	CJobList::iterator it = m_Jobs.begin();
	while (it != m_Jobs.end())
		if (it->IsFlagSet(CO_TAG))
			it = m_Jobs.erase(it);
		else
			it++;

	RecalcAmbiguous();
}
