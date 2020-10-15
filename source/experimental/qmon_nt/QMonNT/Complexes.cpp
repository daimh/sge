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
// Complex.cpp: implementation of the CComplex class.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include <stdlib.h>
#include "qmonnt.h"
#include "Complexes.h"
#include "Debug.h"
#include "ComplexAtribute.h"

extern "C" {
#include "cull_multitype.h"
#include "cod_complexL.h"
}



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CComplex::CComplex()
{
}

CComplex::CComplex(lListElem *qep)
{
	ASSERT(NULL != qep);

	name = lGetString(qep, CX_name);

	lListElem *ep;
	lList *calp = lGetList(qep, CX_entries);
	for_each_cpp(ep, calp)
		AtributeList.push_back(CComplexAtribute(ep));
}

CComplex::~CComplex()
{
}

CComplex:: operator lListElem* ()
{
	lListElem *ep = lCreateElem(CX_Type);
	ASSERT(NULL != ep);
	lSetString(ep, CX_name, name.GetBuffer(0));

	lList *lp = NULL;
	if (!AtributeList.empty()) {
		lp = lCreateList("attribute", CE_Type);
		ASSERT(NULL != lp);
	}

	lListElem *ap;
	for (CComplexAtributeList::iterator Iterator = AtributeList.begin(); Iterator != AtributeList.end(); Iterator++) {
		ap = (lListElem *) *Iterator;
		ASSERT(NULL != ap);
		lAppendElem(lp, ap);
	}
	lSetList(ep, CX_entries, lp);

	return ep;
}



//////////////////////////////////////////////////////////////////////
// CComplexList Class
//////////////////////////////////////////////////////////////////////

CComplexList::iterator CComplexList::FindByID(ULONG ID)
{
	for (CComplexList::iterator it = begin(); it != end(); it++)
		if (ID == it->GetID())
			break;
	
	return it;
}

void CComplexList::RemoveByID(ULONG ID)
{
	CComplexList::iterator it = FindByID(ID);
	if (it != end())
		erase(it);
}

void CComplexList::DebugOut()
{
	DENTER(GUI_LAYER, "CComplexList::DebugOut");

	DPRINTF(("--- CComplexList: -----------"));

	for (CComplexList::iterator it = begin(); it != end(); it++)
		DPRINTF(("Name: %s", it->name));
	DPRINTF((""));

	DEXIT;
}

lList *CComplexList::MakelList()
{
	lList *lp = NULL;

	if (!empty()) {
		lp = lCreateList("complexes", CX_Type);
		ASSERT(NULL != lp);
	}

	lListElem *ep;
	for (CComplexList::iterator Iterator = begin(); Iterator != end(); Iterator++) {
		ep = (lListElem *) *Iterator;
		ASSERT(NULL != ep);
		lAppendElem(lp, ep);
	}

	return lp;
}
