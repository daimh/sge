// ComplexAtributeSet.h: interface for the CComplexAtributeSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPLEXATRIBUTESET_H__D8CB12D3_E5A5_11D2_9717_0020AFA6CCC8__INCLUDED_)
#define AFX_COMPLEXATRIBUTESET_H__D8CB12D3_E5A5_11D2_9717_0020AFA6CCC8__INCLUDED_
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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SgeSet.h"
#include "ComplexAtribute.h"

class CComplexAtributeSet : public CSgeSet  
{
public:
	CComplexAtributeSet();
	virtual ~CComplexAtributeSet();

	CComplexAtributeList::iterator begin();  // Stefan
	CComplexAtributeList::iterator end();	// Stefan

	int	 IsLocal();
	bool IsEmpty();
	void MergeChanges(); // Stefan
	void Clear();
	void Delete(CComplexAtribute &ca);
	void Add(CComplexAtribute &ca);
	CComplexAtribute *GetTemp();
	void DebugOut();

	operator lList* ();         // Stefan
	operator lEnumeration* ();// Stefan

	void SetTag();// Stefan
	void ClearTag(ULONG ID); // Stefan
	void DeleteTagged(); // Stefan

private:
	CComplexAtributeList	m_ComplexAtributes;
	CComplexAtribute		m_Temp;

	void RecalcAmbiguous();
};

#endif // !defined(AFX_COMPLEXATRIBUTESET_H__D8CB12D3_E5A5_11D2_9717_0020AFA6CCC8__INCLUDED_)
