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
// CodSet.cpp: Implementierung der Klasse CCodSet.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "CodSet.h"

namespace Codine {	
extern "C" {
#include "cull_list.h"
}
}

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CCodSet::CCodSet()
{
	m_pModIntVector = NULL;
}

CCodSet::~CCodSet()
{
	if (NULL != m_pModIntVector) {
		delete [] m_pModIntVector;
		m_pModIntVector = NULL;
	}
}

/*
** SetModified
**
** Markiert das angegebenen Feld als geändert.
*/
void CCodSet::SetModified(int Fieldname)
{
	m_ModifiedSet.insert(Fieldname);
}

/*
** ClearModified
**
** Löscht die Markierung "Geändert" für alle Felder.
*/
void CCodSet::ClearModified()
{
	m_ModifiedSet.clear();
}

/*
** ClearModified
**
** Hebt die Markierung "Geändert" für das angegebene
** Feld auf.
*/
void CCodSet::ClearModified(int Fieldname)
{
	m_ModifiedSet.erase(Fieldname);
}

/*
** IsModified
**
** Liefert TRUE, wenn das angegebene Feld als geändert
** markiert ist.
*/
bool CCodSet::IsModified(int Fieldname) const
{
	return m_ModifiedSet.find(Fieldname) != m_ModifiedSet.end();
}

/*
** IsModified
**
** Liefert TRUE, wenn irgendein Feld als geändert
** markiert ist.
*/
bool CCodSet::IsModified() const
{
	return !m_ModifiedSet.empty();
}

/*
** SetAmbiguous
**
** Markiert das angegebene Feld als mehrdeutig.
*/
void CCodSet::SetAmbiguous(int Fieldname)
{
	m_AmbiguousSet.insert(Fieldname);
}

/*
** ClearAmbiguous
**
** Löscht die Markierung 'Mehrdeutig' für das angegebene Feld.
*/
void CCodSet::ClearAmbiguous(int Fieldname)
{
	m_AmbiguousSet.erase(Fieldname);
}

/*
** ClearAmbiguous
**
** Löscht die Markierung 'Mehrdeutig' für alle Felder im Set.
*/
void CCodSet::ClearAmbiguous()
{
	m_AmbiguousSet.clear();
}

/*
** IsAmbiguous
**
** Liefert TRUE, falls das angegebene Feld als 
** mehrdeutig markiert ist.
*/
bool CCodSet::IsAmbiguous(int Fieldname) const
{
	return m_AmbiguousSet.find(Fieldname) != m_AmbiguousSet.end();
}


bool CCodSet::IsAmbiguous() const
{
	return !m_AmbiguousSet.empty();
}


/*
** GetModIntVector
**
** Wandelt das ModifiedSet in ein int-Array um, in dem sämtliche als geändert markierten
** Felder eingetragen werden. Der Zugriff auf den zurückgelieferten Zeiger ist nur während
** der Lebenszeit des entsprechenden CodSets erlaubt!
*/
int *CCodSet::GetModIntVector()
{
	if (NULL != m_pModIntVector) {	
		delete [] m_pModIntVector;
		m_pModIntVector = NULL;
	}
	
	m_pModIntVector = new int[m_ModifiedSet.size()+1];
	ASSERT(NULL != m_pModIntVector);

	intset::iterator Iterator;
	int i;
	for (Iterator = m_ModifiedSet.begin(), i = 0; Iterator != m_ModifiedSet.end(); Iterator++, i++)
		m_pModIntVector[i] = *Iterator;
	
	m_pModIntVector[i] = NoName;

	return m_pModIntVector;
}
