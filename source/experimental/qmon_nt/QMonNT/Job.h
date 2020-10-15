// Job.h: Interface for CJob.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JOB_H__F96880C1_5EB6_11D2_81F1_0000B45BAE88__INCLUDED_)
#define AFX_JOB_H__F96880C1_5EB6_11D2_81F1_0000B45BAE88__INCLUDED_
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

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <deque>

#include "SgeObj.h"
#include "ComplexAtribute.h"

typedef std::deque<CString>		CStrList;

class CPathName
{
public:
	CString		path;
	CString		host;

	CPathName(const CString &path, const CString &host);
	CPathName(lListElem *ep);

	operator lListElem* ();
};

class CPathNameList : public std::deque<CPathName>
{
public:
	lList *MakelList();
};

class CMailRecipient
{
public:
	CString user;
	CString host;

	CMailRecipient(lListElem *ep);

	operator lListElem* ();
};

class CMailRecipientList : public std::deque<CMailRecipient>	
{
public:
	lList *MakelList();
};

class CResourceRange
{
public:
	ULONG	min;
	ULONG	max;

	CResourceRange(lListElem *ep);

	operator lListElem* ();
};

class CResourceRangeList : public std::deque<CResourceRange>
{
public:
	lList *MakelList();
};

class CJobResource
{
public:
	CResourceRangeList		ranges;
	CComplexAtributeList	entries;

	CJobResource(lListElem *ep);

	operator lListElem* ();
};

class CJobResourceList : public std::deque<CJobResource>
{
public:
	lList *MakelList();
};

class CJob : public CSgeObj  
{
public:
	ULONG job_number;
	ULONG status;
    ULONG notify;
	ULONG priority;
    ULONG restart;
    ULONG hold;

	ULONG merge_stderr;

    CTime submission_time;
    CTime execution_time;
	CTime start_time;

	ULONG	mail_options;
	CString	job_file;
	CString	script_file;
	ULONG	script_size;
	CString	script_ptr;
	CString	job_name;
	CString owner;
	CString cell;
	CString	cwd;
	CString pe;
	CString account;

	std::deque<ULONG>	jid_predecessor_list;
	CMailRecipientList	mail_list;
	CPathNameList		stdout_path_list;
	CPathNameList		stderr_path_list;
	CResourceRangeList	pe_range;
	CStrList			job_args;
	CStrList			running_queues;

	CString				directive_prefix;
	CPathNameList		shell_list;
	
	CJobResourceList	hard_resource_list;
	CJobResourceList	soft_resource_list;

	CJob();
	CJob(lListElem *jep);
	virtual ~CJob();

	operator lListElem* ();
};

class CJobList : public std::deque<CJob>
{
public:
	CJobList::iterator FindByID(ULONG ID);
	void RemoveByID(ULONG ID);
	void DebugOut();
	lList *MakelList();
};

#endif // !defined(AFX_JOB_H__F96880C1_5EB6_11D2_81F1_0000B45BAE88__INCLUDED_)
