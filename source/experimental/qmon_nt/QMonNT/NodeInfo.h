// NodeInfo.h: interface of the CNodeInfo.class
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODEINFO_H__06431ED4_5770_11D2_A84D_0020AFA6CCC8__INCLUDED_)
#define AFX_NODEINFO_H__06431ED4_5770_11D2_A84D_0020AFA6CCC8__INCLUDED_
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

// NodeInfo types
enum {
	NI_UNKNOWN			= 99,
	NI_QUEUEROOT,
	NI_QUEUE,
	NI_JOBROOT,
	NI_JOB,
	NI_HOSTROOT,
	NI_HOST,
	NI_COMPLEXROOT,
	NI_COMPLEX,
	NI_COMPLEXATRIBUTE
	// >>> place code for further data types here
};

class CNodeInfo  
{
public:
	CString m_Title;
	CString m_Path;
	int		m_Type;
	bool	m_Tag;
	ULONG	m_ID;

	CNodeInfo();
	CNodeInfo(int Type, ULONG ID, CString Title);
	virtual ~CNodeInfo();

	bool operator == (const CNodeInfo &other);
	static char *PrintType(int Type);

private:
	void InitOnConstruct();
};


class CNodeInfoSet : public std::deque<CNodeInfo>  
{
public:
	CNodeInfoSet();
	virtual ~CNodeInfoSet();

	int GetType();
	void Add(const CNodeInfo &ni);
	void Delete(const CNodeInfo &ni);
	CNodeInfoSet::iterator Find(const CNodeInfo &ni);
	CNodeInfoSet::iterator Find(ULONG ID);
	void DebugOut();
	bool Contains(ULONG ID);
	bool ContainsOtherType(int nType);
	void ClearTag();
	void ClearTag(ULONG ID);
	void SetTag();
	void SetTag(ULONG ID);
	bool IsTagSet(ULONG ID);
};

#endif // !defined(AFX_NODEINFO_H__06431ED4_5770_11D2_A84D_0020AFA6CCC8__INCLUDED_)
