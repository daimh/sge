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
// QueueComplex.cpp: implementation of the CQueueComplex class.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include <stdlib.h>
#include "QueueComplex.h"
#include "qmonnt.h"
#include "Queue.h"
#include "Debug.h"

extern "C" {
#include "cull_multitype.h"
#include "cod_queueL.h"
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

CQueueComplex::CQueueComplex()
{
}

CQueueComplex::CQueueComplex(lListElem *qep)
{
	ASSERT(NULL != qep);
	name = lGetString(qep, CX_name);
}

CQueueComplex::~CQueueComplex()
{
}

/*
** operator lListElem*
**
** Wandelt das Queue-Objekt in ein Cull-List-Element um.
** HINWEIS: Das zurückgegebene List-Element muß von der aufrufenden
** Funktion gelöscht werden!
*/
CQueueComplex::operator lListElem* ()
{
	lListElem *ep = lCreateElem(CX_Type);
	ASSERT(NULL != ep);

	lSetString(ep, CX_name, name.GetBuffer(0));

	return ep;
}


//////////////////////////////////////////////////////////////////////
// CQueueComplexList Class
//////////////////////////////////////////////////////////////////////

CQueueComplexList::iterator CQueueComplexList::FindByID(ULONG ID)
{
	for (CQueueComplexList::iterator it = begin(); it != end(); it++)
		if (ID != it->GetID())
			break;
	
	return it;
}

void CQueueComplexList::RemoveByID(ULONG ID)
{
	CQueueComplexList::iterator it = FindByID(ID);
	if (it != end())
		erase(it);
}

/*
** DebugOut (public)
**
** Gibt den Inhalt der Queue-Liste auf der Debug-Konsole aus.
** (Kontext GUI_LAYER).
*/
void CQueueComplexList::DebugOut()
{
	DENTER(GUI_LAYER, "CQueueComplexList::DebugOut");

	DPRINTF(("--- CQueueComplexList: -----------"));
	for (CQueueComplexList::iterator it = begin(); it != end(); it++)
		DPRINTF(("ID: %ld", it->GetID()));
	DPRINTF((""));

	DEXIT;
}

lList *CQueueComplexList::MakelList()
{
	lList *lp = NULL;

	if (!empty()) {
		lp = lCreateList("queuecomplexes", CX_Type);
		ASSERT(NULL != lp);
	}
	
	lListElem *ep;
	for (CQueueComplexList::iterator Iterator = begin(); Iterator != end(); Iterator++) {
		ep = (lListElem *) *Iterator;
		ASSERT(NULL != ep);
		lAppendElem(lp, ep);
	}

	return lp;
}
