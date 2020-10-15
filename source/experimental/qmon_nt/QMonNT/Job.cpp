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
// Job.cpp: Implementierung der Klasse CJob.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "Job.h"
#include "Debug.h"

extern "C" {
#include "cull_multitype.h"
#include "cod_jobL.h"
#include "cod_job_refL.h"
#include "cod_rangeL.h"
#include "cod_requestL.h"
#include "cod_peL.h"
#include "cod_stringL.h"
}

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////
// CPathName and CPathNameList

CPathName::CPathName(const CString &path, const CString &host)
{
	this->path = path;
	this->host = host;
}

CPathName::CPathName(lListElem *ep)
{
	ASSERT(NULL != ep);

	path = lGetString(ep, PN_path);
	host = lGetString(ep, PN_host);
}

CPathName::operator lListElem* ()
{
	lListElem *ep = lCreateElem(PN_Type);
	ASSERT(NULL != ep);

	lSetString(ep, PN_path, path.IsEmpty() ? NULL : path.GetBuffer(0));
	lSetString(ep, PN_host, host.IsEmpty() ? NULL : host.GetBuffer(0));

	return ep;
}

lList *CPathNameList::MakelList()
{
	lList		*lp = NULL;
	lListElem	*ep;

	if (!empty()) {
		lp = lCreateList("pathname list", PN_Type);
		ASSERT(NULL != lp); 
	}
	for (iterator it = begin(); it != end(); ++it) {
		ep = (lListElem *) *it;
		ASSERT(NULL != ep);
		lAppendElem(lp, ep);
	}

	return lp;
}

///////////////////////////////////////////
// CMailRecipient and CMailRecipientList

CMailRecipient::CMailRecipient(lListElem *ep)
{
	ASSERT(NULL != ep);

	user = lGetString(ep, MR_user);
	host = lGetString(ep, MR_host);
}

CMailRecipient::operator lListElem* ()
{
	lListElem *ep = lCreateElem(MR_Type);
	ASSERT(NULL != ep);

	lSetString(ep, MR_user, user.IsEmpty() ? NULL : user.GetBuffer(0));
	lSetString(ep, MR_host, host.IsEmpty() ? NULL : host.GetBuffer(0));

	return ep;
}

lList *CMailRecipientList::MakelList()
{
	lList		*lp = NULL;
	lListElem	*ep;

	if (!empty()) {
		lp = lCreateList("mail recipient list", MR_Type);
		ASSERT(NULL != lp); 
	}
	for (iterator it = begin(); it != end(); ++it) {
		ep = (lListElem *) *it;
		ASSERT(NULL != ep);
		lAppendElem(lp, ep);
	}

	return lp;
}

////////////////////////////////////////////////////7
// CResourceRange and CResourceRangeList

CResourceRange::CResourceRange(lListElem *ep)
{
	ASSERT(NULL != ep);

	min = lGetUlong(ep, RN_min);
	max = lGetUlong(ep, RN_max);
}

CResourceRange::operator lListElem* ()
{
	lListElem *ep = lCreateElem(RN_Type);
	ASSERT(NULL != ep);

	lSetUlong(ep, RN_min, min);
	lSetUlong(ep, RN_max, max);

	return ep;
};

lList *CResourceRangeList::MakelList()
{
	lList		*lp = NULL;
	lListElem	*ep;

	if (!empty()) {
		lp = lCreateList("resource range list", RN_Type);
		ASSERT(NULL != lp); 
	}
	for (iterator it = begin(); it != end(); ++it) {
		ep = (lListElem *) *it;
		ASSERT(NULL != ep);
		lAppendElem(lp, ep);
	}

	return lp;
}

//////////////////////////////////////////////////
// CJobResource  and CJobResourceList

CJobResource::CJobResource(lListElem *ep)
{
	ASSERT(NULL != ep);

	lList *lRanges  = lGetList(ep, RE_ranges);
	lList *lEntries = lGetList(ep, RE_entries);

	lListElem *pElem;

	for_each_cpp(pElem, lRanges)
		ranges.push_back(CResourceRange(pElem));

	for_each_cpp(pElem, lEntries)
		entries.push_back(CComplexAtribute(pElem));
}

CJobResource::operator lListElem* ()
{
	lListElem *ep = lCreateElem(RE_Type);
	ASSERT(NULL != ep);

	lSetList(ep, RE_ranges,  ranges.MakelList());
	lSetList(ep, RE_entries, entries.MakelList());

	return ep;
}

lList *CJobResourceList::MakelList()
{
	lList		*lp = NULL;
	lListElem	*ep;

	if (!empty()) {
		lp = lCreateList("job resource list", RE_Type);
		ASSERT(NULL != lp); 
	}
	for (iterator it = begin(); it != end(); ++it) {
		ep = (lListElem *) *it;
		ASSERT(NULL != ep);
		lAppendElem(lp, ep);
	}

	return lp;
}

/////////////////////////////////////////////////
// CJob

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CJob::CJob()
{
	job_number			= 0;
	status				= 0;
    notify				= 0;
	priority			= 0;
    restart				= 0;
    hold				= 0;
	merge_stderr		= FALSE;

    submission_time		= CTime::GetCurrentTime();
    execution_time		= 0;
	start_time			= 0;

	mail_options		= 0;
	script_size			= 0;
}

CJob::CJob(lListElem *jep)
{
	ASSERT(NULL != jep);

	job_number		= lGetUlong(jep,	JB_job_number);
//	status			= lGetUlong(jep,	JB_status);
    notify			= lGetUlong(jep,	JB_notify);
    priority		= lGetUlong(jep,	JB_priority) - BASE_PRIORITY;
    restart			= lGetUlong(jep,	JB_restart);
//    hold			= lGetUlong(jep,	JB_hold);
    merge_stderr	= lGetUlong(jep,	JB_merge_stderr);
    submission_time = CTime(lGetUlong(jep, JB_submission_time));
//    start_time		= CTime(lGetUlong(jep, JB_start_time));
    execution_time	= CTime(lGetUlong(jep, JB_execution_time));
	mail_options	= lGetUlong(jep,	JB_mail_options);

	job_file		= lGetString(jep,	JB_job_file);
	script_file		= lGetString(jep,	JB_script_file);
	script_size		= lGetUlong(jep,	JB_script_size);
	script_ptr		= lGetString(jep,	JB_script_ptr);
	job_name		= lGetString(jep,	JB_job_name);
	owner			= lGetString(jep,	JB_owner);
	cell			= lGetString(jep,	JB_cell);
	cwd				= lGetString(jep,	JB_cwd);
	pe				= lGetString(jep,	JB_pe);
	account			= lGetString(jep,	JB_account);

	lList	  *lp = lGetList(jep, JB_jid_predecessor_list);
	lListElem *ep;

	for_each_cpp(ep, lp)
		jid_predecessor_list.push_back(lGetUlong(ep, JRE_job_number));

	lp = lGetList(jep, JB_mail_list);
	for_each_cpp(ep, lp)
		mail_list.push_back(CMailRecipient(ep));

	lp = lGetList(jep, JB_stdout_path_list);
	for_each_cpp(ep, lp)
		stdout_path_list.push_back(CPathName(ep));

	lp = lGetList(jep, JB_stderr_path_list);
	for_each_cpp(ep, lp)
		stderr_path_list.push_back(CPathName(ep));

	lp = lGetList(jep, JB_pe_range);
	for_each_cpp(ep, lp)
		pe_range.push_back(CResourceRange(ep));

	CString str;

	lp = lGetList(jep, JB_job_args);
	for_each_cpp(ep, lp) {
		str = lGetString(ep, STR);
		job_args.push_back(str);
	}

//	lp = lGetList(jep, JB_granted_destin_identifier_list);
	for_each_cpp(ep, lp) {
		str.Format("%s (%lu slots)", LPCTSTR(lGetString(ep, JG_qname)), 
			lGetUlong(ep, JG_slots));
		running_queues.push_back(str);
	}

	directive_prefix = lGetString(jep, JB_directive_prefix);

	lp = lGetList(jep, JB_shell_list);
	for_each_cpp(ep, lp)
		shell_list.push_back(CPathName(ep));
	
	lp = lGetList(jep, JB_hard_resource_list);
	for_each_cpp(ep, lp)
		hard_resource_list.push_back(CJobResource(ep));

	lp = lGetList(jep, JB_soft_resource_list);
	for_each_cpp(ep, lp)
		soft_resource_list.push_back(CJobResource(ep));
}

CJob::~CJob()
{
}

/*
** operator lListElem*
**
** Wandelt das Queue-Objekt in ein Cull-List-Element um.
** HINWEIS: Das zurückgegebene List-Element muß von der aufrufenden
** Funktion gelöscht werden!
*/
CJob::operator lListElem* ()
{
	lListElem *ep = lCreateElem(JB_Type);
	ASSERT(NULL != ep);

	lSetUlong(ep,	JB_job_number,		job_number);
//	lSetUlong(ep,	JB_status,			status);
    lSetUlong(ep,	JB_notify,			notify);
	lSetUlong(ep,	JB_priority,		priority + BASE_PRIORITY);
    lSetUlong(ep,	JB_restart,			restart);
//    lSetUlong(ep,	JB_hold,			hold);
    lSetUlong(ep,	JB_merge_stderr,	merge_stderr);
    lSetUlong(ep,	JB_submission_time,	submission_time.GetTime());
//    lSetUlong(ep,	JB_start_time,		start_time.GetTime());
    lSetUlong(ep,	JB_execution_time,	execution_time.GetTime());
	lSetUlong(ep,	JB_mail_options,	mail_options);

	lSetString(ep, 	JB_job_file,		job_file.IsEmpty() ? NULL : job_file.GetBuffer(0));
	lSetString(ep,	JB_script_file,		script_file.IsEmpty() ? NULL : script_file.GetBuffer(0));
	lSetUlong( ep,	JB_script_size,		script_size);
	lSetString(ep,  JB_script_ptr,		script_ptr.IsEmpty() ? NULL : script_ptr.GetBuffer(0));
	lSetString(ep,	JB_job_name,		job_name.IsEmpty() ? NULL : job_name.GetBuffer(0));
	lSetString(ep,	JB_owner,			owner.IsEmpty() ? NULL : owner.GetBuffer(0));
	lSetString(ep,	JB_cell,			cell.IsEmpty() ? NULL : cell.GetBuffer(0));

	// Stefan Mihaila: in the NT client the JB_cdw is used only
	// for information purposes, it doesn't make sense to set the
	// current working dir to a path from the NT client local file system
	// So this field shouldn't be available in the user interface
	lSetString(ep,	JB_cwd,				cwd.IsEmpty() ? NULL : cwd.GetBuffer(0));
	lSetString(ep,	JB_pe,				pe.IsEmpty() ? NULL : pe.GetBuffer(0));
	lSetString(ep,	JB_account,			account.IsEmpty() ? NULL : account.GetBuffer(0));

	lList	  *lListField;
	lListElem *pElem;

	lListField = NULL;
	if (!jid_predecessor_list.empty()) {
		lListField = lCreateList("predecesors", JRE_Type);
		ASSERT(NULL != lListField); 
	}
	for (std::deque<ULONG>::iterator it1 = jid_predecessor_list.begin(); 
		it1 != jid_predecessor_list.end(); ++it1) 
	{
		pElem = lCreateElem(JRE_Type);
		ASSERT(NULL != pElem);
		lSetUlong(pElem, JRE_job_number, *it1);
		lAppendElem(lListField, pElem);
	}
	lSetList(ep, JB_jid_predecessor_list, lListField);

	lSetList(ep, JB_mail_list, mail_list.MakelList());

	lSetList(ep, JB_stdout_path_list, stdout_path_list.MakelList());
	lSetList(ep, JB_stderr_path_list, stderr_path_list.MakelList());
	
	lSetList(ep, JB_pe_range, pe_range.MakelList());

	lListField = NULL;
	if (!job_args.empty()) {
		lListField = lCreateList("job_args", ST_Type);
		ASSERT(NULL != lListField); 
	}
	for (CStrList::iterator it6 = job_args.begin(); 
		it6 != job_args.end(); ++it6) 
	{
		pElem = lCreateElem(ST_Type);
		ASSERT(NULL != pElem);
		lSetString(pElem, STR, it6->IsEmpty() ? NULL : it6->GetBuffer(0));
		lAppendElem(lListField, pElem);
	}
	lSetList(ep, JB_job_args, lListField);

	// Stefan Mihaila: Normally, the server ignores this data
	// It's useful only you want to see these list when displaying
	// jobs status
	lListField = NULL;
	if (!running_queues.empty()) {
		lListField = lCreateList("granted_destin_identifier_list", JG_Type);
		ASSERT(NULL != lListField); 
	}
	for (CStrList::iterator it7 = running_queues.begin(); 
		it7 != running_queues.end(); ++it7) 
	{
		pElem = lCreateElem(JG_Type);
		ASSERT(NULL != pElem);
		lSetString(pElem, JG_qname, it7->IsEmpty() ? NULL : it7->GetBuffer(0));
		lAppendElem(lListField, pElem);
	}
//	lSetList(ep, JB_granted_destin_identifier_list, lListField);

	lSetString(ep, JB_directive_prefix, directive_prefix.IsEmpty() ? NULL : directive_prefix.GetBuffer(0));

	lSetList(ep, JB_shell_list, shell_list.MakelList());

	lSetList(ep, JB_hard_resource_list, hard_resource_list.MakelList());
	lSetList(ep, JB_soft_resource_list, soft_resource_list.MakelList());

	return ep;
}

/******************************************************************************
****                                                                       ****
**** Klasse: CJobList                                                      ****
****                                                                       ****
******************************************************************************/

/*
** FindByID
**
** Sucht ein Element mit der angegebenen ID in der Liste
** und liefert den Iterator darauf zurück. Falls der Iterator
** gleich end() ist, wurde das Element nicht gefunden!
*/
CJobList::iterator CJobList::FindByID(ULONG ID)
{
	for (CJobList::iterator it = begin(); it != end(); it++)
		if (ID == it->GetID())
			break;
	
	return it;
}

/*
** RemoveByID
**
** Löscht das Element mit der angegebenen ID aus der Liste.
*/
void CJobList::RemoveByID(ULONG ID)
{
	CJobList::iterator it = FindByID(ID);
	if (it != end())
		erase(it);
}

/*
** DebugOut (public)
**
** Gibt den Inhalt der Queue-Liste auf der Debug-Konsole aus.
** (Kontext GUI_LAYER).
*/
void CJobList::DebugOut()
{
	DENTER(GUI_LAYER, "CJobList::DebugOut");

	DPRINTF(("--- CJobList: -----------"));
	for (CJobList::iterator it = begin(); it != end(); it++)
		DPRINTF(("ID: %ld", it->GetID()));
	DPRINTF((""));

	DEXIT;
}

lList *CJobList::MakelList()
{
	lList *lp = lCreateList("jobs", JB_Type);
	ASSERT(NULL != lp);
	
	lListElem *ep;
	for (CJobList::iterator Iterator = begin(); Iterator != end(); Iterator++) {
		ep = (lListElem *) *Iterator;
		ASSERT(NULL != ep);
		lAppendElem(lp, ep);
	}

	return lp;
}
