// JobSet.h: Schnittstelle für die Klasse CJobSet.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JOBSET_H__C939DCE1_5F84_11D2_81F1_0000B45BAE88__INCLUDED_)
#define AFX_JOBSET_H__C939DCE1_5F84_11D2_81F1_0000B45BAE88__INCLUDED_
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

#include "SgeSet.h"
#include "Job.h"

class CJobSet : public CSgeSet  
{
public:
	CJobSet();
	virtual ~CJobSet();

	CJobList::iterator begin();
	CJobList::iterator end();

	int  IsLocal(); // Stefan
	bool IsEmpty();
	void MergeChanges(); // Stefan
	void Clear();
	void Delete(CJob &j);
	void Add(CJob &j);
	CJob *GetTemp();
	void DebugOut();

	operator lList* (); // Stefan
	operator lEnumeration* (); // Stefan

	// All 3 by Stefan
	void SetTag();
	void ClearTag(ULONG ID);
	void DeleteTagged();

private:
	CJobList	m_Jobs;
	CJob		m_Temp;

	void RecalcAmbiguous();
};

#endif // !defined(AFX_JOBSET_H__C939DCE1_5F84_11D2_81F1_0000B45BAE88__INCLUDED_)
