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
// ComplexAtributeSet.cpp: implementation of the CComplexAtributeSet class.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "Debug.h"
#include "ComplexAtributeSet.h"

extern "C" {
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

CComplexAtributeSet::CComplexAtributeSet()
{
}

CComplexAtributeSet::~CComplexAtributeSet()
{
}

CComplexAtributeList::iterator CComplexAtributeSet::begin()
{
	return m_ComplexAtributes.begin();
}

CComplexAtributeList::iterator CComplexAtributeSet::end()
{
	return m_ComplexAtributes.end();
}

int CComplexAtributeSet::IsLocal()
{
	bool set = false, notset = false;
	CComplexAtributeList::iterator Iterator;

	for (Iterator = m_ComplexAtributes.begin(); Iterator != m_ComplexAtributes.end(); Iterator++)
		if (Iterator->IsFlagSet(CO_LOCAL))
			set = true;
		else
			notset = true;

	return set ? (notset ? -1 : 1) : 0;
}

bool CComplexAtributeSet::IsEmpty()
{
	return m_ComplexAtributes.empty();
}

void CComplexAtributeSet::MergeChanges()
{
	// Stefan Mihaila: TODO
	ASSERT(false);
}

void CComplexAtributeSet::Clear()
{
	m_ComplexAtributes.clear();
}

void CComplexAtributeSet::Delete(CComplexAtribute &ca)
{
}

void CComplexAtributeSet::Add(CComplexAtribute &ca)
{
	m_ComplexAtributes.push_back(ca);
	RecalcAmbiguous();
}

CComplexAtribute *CComplexAtributeSet::GetTemp()
{
	ASSERT(!m_ComplexAtributes.empty());
	m_Temp = m_ComplexAtributes.front();
	return &m_Temp;
}

void CComplexAtributeSet::DebugOut()
{
	DENTER(GUI_LAYER, "CComplexAtributeSet::DebugOut");
	DPRINTF(("--- QueueSet: -----------"));

	CComplexAtributeList::iterator cait;
	CComplexAtribute ca;
	for (cait = m_ComplexAtributes.begin(); cait != m_ComplexAtributes.end(); cait++) {
		ca = *cait;
		DPRINTF(("Name: %s, String Val: %s", ca.name, ca.stringval));
	}

	DPRINTF((""));
	DEXIT;
}

void CComplexAtributeSet::RecalcAmbiguous()
{
	DENTER(GUI_LAYER, "Set::RecalcAmbiguous");

	ClearAmbiguous();
	if (m_ComplexAtributes.size() > 1) {
		CComplexAtributeList::iterator cait = m_ComplexAtributes.begin();
		CComplexAtribute fca = *cait;

		while ((++cait) != m_ComplexAtributes.end()) {
			if (fca.name != cait->name)
				SetAmbiguous(CE_name);
			if (fca.shortcut != cait->shortcut)
				SetAmbiguous(CE_shortcut);
			if (fca.stringval != cait->stringval)		
				SetAmbiguous(CE_stringval);
			if (fca.valtype  != cait->valtype)
				SetAmbiguous(CE_valtype);
			if (fca.request  != cait->request)
				SetAmbiguous(CE_request);
			if (fca.consumable  != cait->consumable)	
				SetAmbiguous(CE_consumable);
			if (fca.relop  != cait->relop)				
				SetAmbiguous(CE_relop);
			if (fca.defaultval  != cait->defaultval)	
				SetAmbiguous(CE_default);
		}
	}

	DEXIT;
}

CComplexAtributeSet::operator lList* ()
{
	return m_ComplexAtributes.MakelList();
}

CComplexAtributeSet::operator lEnumeration* ()
{
	/*int *ModIntVector = GetModIntVector();
	ASSERT(NULL != ModIntVector);

	lEnumeration *what = lIntVector2What(EH_Type, ModIntVector);
	ASSERT(NULL != what);

	return what;*/

	// Stefan Mihaila: TODO
	ASSERT(false);
	return NULL;
}

/*
** SetTag
**
** Setzt das Tag-Flag in allen im Set enthaltenen Objekten.
*/
void CComplexAtributeSet::SetTag()
{
	for (CComplexAtributeList::iterator Iterator = m_ComplexAtributes.begin(); Iterator != m_ComplexAtributes.end(); Iterator++)
		Iterator->SetFlag(CO_TAG);
}

/*
** ClearTag
**
** Löscht das Tag-Flag bei dem Objekt, das die angegebene ID besitzt.
*/
void CComplexAtributeSet::ClearTag(ULONG ID)
{
	for (CComplexAtributeList::iterator Iterator = m_ComplexAtributes.begin(); Iterator != m_ComplexAtributes.end(); Iterator++)
		if (ID == Iterator->GetID())
			Iterator->ClearFlag(CO_TAG);
}

void CComplexAtributeSet::DeleteTagged()
{
	CComplexAtributeList::iterator Iterator = m_ComplexAtributes.begin();
	while (Iterator != m_ComplexAtributes.end())
		if (Iterator->IsFlagSet(CO_TAG))
			Iterator = m_ComplexAtributes.erase(Iterator);
		else 
			Iterator++;
		
	RecalcAmbiguous();
}
