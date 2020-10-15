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
// NodeInfo.cpp: implementation of the CNodeInfo.class
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "NodeInfo.h"
#include "Debug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// construction/destruction
//////////////////////////////////////////////////////////////////////

/*
** CNodeInfo
**
*/
CNodeInfo::CNodeInfo()
{
	DENTER(GUI_LAYER, "CNodeInfo::CNodeInfo()");
	InitOnConstruct();
	DEXIT;
}

/*
** CNodeInfo
**
** constructor initializes object with specified values
*/
CNodeInfo::CNodeInfo(int Type, ULONG ID, CString Title)
{
	DENTER(GUI_LAYER, "CNodeInfo::CNodeInfo(int, ULONG, CString)");
	InitOnConstruct();
	m_Type = Type;
	m_ID = ID;
	m_Title = Title;
	DEXIT;
}

void CNodeInfo::InitOnConstruct()
{
	m_ID = 0;
	m_Title = "";
	m_Path = "";
	m_Type = NI_UNKNOWN;
	m_Tag = false;
}

/*
** ~CNodeInfo
**
** destructor
*/
CNodeInfo::~CNodeInfo()
{
}

/*
** operator ==
**
** returns true, if the compared NodeInfo objects match 
** by type and ID, false otherwise.
*/
bool CNodeInfo::operator == (const CNodeInfo &other)
{
	DENTER(GUI_LAYER, "CNodeInfo::operator==");

	bool Result = other.m_Type == m_Type && other.m_ID == m_ID;

	DEXIT;
	return Result;
}

/*
** PrintType
**
** returns the name of the specified type in form of a string
*/
char *CNodeInfo::PrintType(int Type)
{
	switch (Type) {
		case NI_UNKNOWN:	
			return "NI_UNKNOWN";

		case NI_QUEUEROOT:	
			return "NI_QUEUEROOT";

		case NI_QUEUE:		
			return "NI_QUEUE";

		case NI_JOBROOT:	
			return "NI_JOBROOT";

		case NI_JOB:		
			return "NI_JOB";

		case NI_HOST:		
			return "NI_HOST";

		case NI_COMPLEXATRIBUTE:
			return "NI_COMPLEXATRIBUTE";

		default:			
			return "COMPLETELY UNKNOWN";
	}
}

//////////////////////////////////////////////////////////////////////
// CNodeInfoSet class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// construction/destruction
//////////////////////////////////////////////////////////////////////

/*
** CNodeInfoSet
**
*/
CNodeInfoSet::CNodeInfoSet()
{

}

/*
** ~CNodeInfoSet
**
** Destruktor
*/
CNodeInfoSet::~CNodeInfoSet()
{

}

/*
** Find 
**
** Seeks for a NodeInfo object, that matches the specified type, ID and path
** and returns an iterator to it (in case of success)
*/
CNodeInfoSet::iterator CNodeInfoSet::Find(const CNodeInfo &ni)
{
	DENTER(GUI_LAYER, "CNodeInfoSet::Find");

	for (CNodeInfoSet::iterator it = begin(); it != end(); it++)
		if (const_cast<CNodeInfo &>(ni) == *it && ni.m_Path == it->m_Path)
			break;

	DEXIT;
	return it;
}

/*
** Find
**
** Seeks for a NodeInfo object, that matches the specified ID
** and returns an iterator to it
*/
CNodeInfoSet::iterator CNodeInfoSet::Find(ULONG ID)
{
	for (CNodeInfoSet::iterator it = begin(); it != end(); it++)
		if (ID == it->m_ID)
			break;
	
	return it;
}

/*
** Add
**
** Adds the specified NodeInfo object to the set, if there isn't 
** already an identical one included(identical content and path)
*/
void CNodeInfoSet::Add(const CNodeInfo &ni)
{
	DENTER(GUI_LAYER, "CNodeInfoSet::Add");	

	if (Find(ni) == end())
		push_back(ni);

	DEXIT;
}

/*
** Delete
**
** Removes an element off the Set, the content of which is identical
** to the the specified one (identical path)
*/
void CNodeInfoSet::Delete(const CNodeInfo &ni)
{
	DENTER(GUI_LAYER, "CNodeInfoSet::Delete");

	iterator it = Find(ni);
	if (it != end())
		erase(it);
	
	DEXIT;
}

/*
** Contains
**
** returns true, if the specified ID is included in the InfoSet
** false otherwise
*/
bool CNodeInfoSet::Contains(ULONG ID)
{
	DENTER(GUI_LAYER, "CNodeInfoSet::Contains");

	for (iterator it = begin(); it != end(); it++)
		if (ID == it->m_ID)
			break;

	DEXIT;

	return it != end();
}

/*
** ContainsOtherType
**
** returns true if any of the InfoSet's elements is of a different type 
** then the specified one
*/
bool CNodeInfoSet::ContainsOtherType(int nType)
{
	DENTER(GUI_LAYER, "CNodeInfoSet::ContainsOtherType");

	for (iterator it = begin(); it != end(); it++) 
		if (nType != it->m_Type)
			break;
	DEXIT;

	return it != end();
}

/*
** GetType
**
** returns type (NI_...) of first element in NodeInfo object
*/
int CNodeInfoSet::GetType()
{
	iterator it = begin();
	ASSERT(it != end());
	return it->m_Type;
}

/*
** DebugOut
**
** displays the contents of the NodeInfoSet in the debug view
*/
void CNodeInfoSet::DebugOut()
{
	DENTER(GUI_LAYER, "CNodeInfoSet::DebugOut");

	DPRINTF(("--- NodeInfoSet: --------"));
	for (iterator it = begin(); it != end(); it++)
		switch (it->m_Type) {
			case NI_QUEUEROOT:	
				DPRINTF(("QUEUEROOT   ID = %lu  ", it->m_ID));
				break;

			case NI_JOBROOT:	
				DPRINTF(("JOBROOT     ID = %lu  ", it->m_ID));
				break;

			case NI_QUEUE:		
				DPRINTF(("QUEUE       ID = %lu  ", it->m_ID));
				break;

			case NI_JOB:		
				DPRINTF(("JOB         ID = %lu  ", it->m_ID));
				break;

			default:			
				DPRINTF(("UNKNOWN     ID = %lu  ", it->m_ID));
		}
	
	DEXIT;
}

/*
** CearTag (public)
**
** clears the Tag-flag of any NodeInfo object included in the InfoSet
*/
void CNodeInfoSet::ClearTag()
{
	for (iterator it = begin(); it != end(); it++)
		it->m_Tag = false;
}

/*
** ClearTag
**
** clears the Tag-flag of any NodeInfo object with the specified ID
*/
void CNodeInfoSet::ClearTag(ULONG ID)
{
	iterator it = Find(ID);
	if (it != end())
		it->m_Tag = false;
}

/*
** SetTag
**
** Activates the Tag-flag of any NodeInfo object included in the InfoSet
*/
void CNodeInfoSet::SetTag()
{
	for (iterator it = begin(); it != end(); it++)
		it->m_Tag = true;
}

/*
** SetTag
**
** Activates the Tag-flag of the NodeInfo object with the specified ID
*/
void CNodeInfoSet::SetTag(ULONG ID)
{
	iterator it = Find(ID);
	if (it != end())
		it->m_Tag = true;
}

/*
** IsTagSet
**
** returns true if the Tag-flag is activated in the NodeInfo object with
** the specified ID
** returns false if it isn't activated or if there's no element in the
** InfoSet with the specified ID
*/
bool CNodeInfoSet::IsTagSet(ULONG ID)
{
	iterator it = Find(ID);
	return it != end() ? it->m_Tag : false;
}
