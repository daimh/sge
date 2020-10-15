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
// ComplexAtribute.cpp: implementation of the CComplexAtribute class.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include <stdlib.h>
#include "qmonnt.h"
#include "ComplexAtribute.h"
#include "Debug.h"

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

CComplexAtribute::CComplexAtribute()
{
}

CComplexAtribute::CComplexAtribute(lListElem *qep)
{
	ASSERT(NULL != qep);

	name			= lGetString(qep, CE_name);
	shortcut		= lGetString(qep, CE_shortcut);
	stringval		= lGetString(qep, CE_stringval);
	defaultval		= lGetString(qep, CE_default);        
	pj_stringval	= lGetString(qep, CE_pj_stringval);
	valtype			= lGetUlong( qep, CE_valtype);
	relop			= lGetUlong( qep, CE_relop);
	request			= lGetUlong( qep, CE_request);
	consumable		= lGetUlong( qep, CE_consumable);
	forced			= lGetUlong( qep, CE_forced);
	dominant		= lGetUlong( qep, CE_dominant);
	pj_dominant		= lGetUlong( qep, CE_pj_dominant);
	doubleval		= lGetDouble(qep, CE_doubleval);
	pj_doubleval	= lGetDouble(qep, CE_pj_doubleval);
}

CComplexAtribute::~CComplexAtribute()
{
}

CComplexAtribute:: operator lListElem* ()
{
	lListElem *ep = lCreateElem(CE_Type);	
	ASSERT(NULL != ep);

	lSetString(ep, CE_name,           name.GetBuffer(0));
	lSetString(ep, CE_shortcut,       shortcut.GetBuffer(0));
	lSetString(ep, CE_stringval,      stringval.GetBuffer(0));
	lSetString(ep, CE_default,        defaultval.GetBuffer(0));
	lSetString(ep, CE_pj_stringval,   pj_stringval.GetBuffer(0));

	lSetUlong(ep, CE_valtype,         valtype);
	lSetUlong(ep, CE_relop,           relop);
	lSetUlong(ep, CE_request,         request);
	lSetUlong(ep, CE_consumable,      consumable);
	lSetUlong(ep, CE_forced,          forced);
	lSetUlong(ep, CE_dominant,        dominant);
	lSetUlong(ep, CE_pj_dominant,     pj_dominant);

	lSetDouble(ep, CE_doubleval,      doubleval);
	lSetDouble(ep, CE_pj_doubleval,   pj_doubleval);

	return ep;
}

//////////////////////////////////////////////////////////////////////
// CComplexAtributeList Class
//////////////////////////////////////////////////////////////////////

CComplexAtributeList::iterator CComplexAtributeList::FindByID(ULONG ID)
{
	for (CComplexAtributeList::iterator it = begin(); it != end(); it++)
		if (ID == it->GetID())
			break;
	
	return it;
}

CComplexAtributeList::iterator CComplexAtributeList::FindByName(const CString &Name)
{
	for (CComplexAtributeList::iterator it = begin(); it != end(); it++)
		if (Name == it->name)
			break;
	
	return it;
}

void CComplexAtributeList::RemoveByID(ULONG ID)
{
	CComplexAtributeList::iterator it = FindByID(ID);
	if (it != end())
		erase(it);
}

lList *CComplexAtributeList::MakelList()
{
	lList *lp = NULL;

	if (!empty()) {
		lp = lCreateList("atributes", CX_Type);
		ASSERT(NULL != lp);
	}

	lListElem *ep;
	for (CComplexAtributeList::iterator Iterator = begin(); Iterator != end(); Iterator++) {
		ep = (lListElem *) *Iterator;
		ASSERT(NULL != ep);
		lAppendElem(lp, ep);
	}

	return lp;
}
