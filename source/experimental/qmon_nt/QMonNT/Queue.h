// Queue.h: Schnittstelle für die Klasse CQueue.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUEUE_H__D852EE44_48C0_11D2_81F1_0000B45BAE88__INCLUDED_)
#define AFX_QUEUE_H__D852EE44_48C0_11D2_81F1_0000B45BAE88__INCLUDED_
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
#include <afx.h>

#include "SgeObj.h"
#include "QueueComplex.h"
#include "ComplexAtribute.h"

// Added by Stefan Mihaila
class CQmonntDoc;

class CQueue : public CSgeObj  
{
public:
	ULONG				qpriority;
	ULONG				qjobslots;
	CString				qhostname;
	CString				qname;
	ULONG				qtype;
	CString				qshell;
	CString				qtmpdir;
	ULONG				qstate;

	CTime				qnotifytime;
	CTime				qhardrealtime;
	CTime				qsoftrealtime;
	CTime				qhardcputime;
	CTime				qsoftcputime;

	CString				qhardfilesize;
	CString				qsoftfilesize;
	CString				qharddatasize;
	CString				qsoftdatasize;
	CString				qhardstacksize;
	CString				qsoftstacksize;
	CString				qhardcorefilesize;
	CString				qsoftcorefilesize;
	CString				qhardresidentsetsize;
	CString				qsoftresidentsetsize;
	CString				qhardvirtualmemory;
	CString				qsoftvirtualmemory;

	CQueueComplexList		qComplexList;
	CComplexAtributeList	qCheckPointingList;
	// >>> Code für neue Felder hier einfügen

	CQueue();
    CQueue(lListElem *qep);
	virtual ~CQueue();

	operator lListElem* ();

	CComplexAtributeList GetAllAvailableCheckPointings(CQmonntDoc *pDoc);

private:
	CTime	StringToTime(const CString &timeString);
	CString TimeToString(const CTime   &stringTime);
};

class CQueueList : public std::deque<CQueue>
{
public:
	CQueueList::iterator FindByID(ULONG ID);
	void RemoveByID(ULONG ID);
	void DebugOut();
	lList *MakelList();
};

#endif // !defined(AFX_QUEUE_H__D852EE44_48C0_11D2_81F1_0000B45BAE88__INCLUDED_)
