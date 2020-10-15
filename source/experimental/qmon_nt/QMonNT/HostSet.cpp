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
// HostSet.cpp: Implementierung der Klasse CHostSet.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "HostSet.h"
#include "Debug.h"

extern "C" {
#include "cod_hostL.h"
}


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

/*
** CHostSet (Konstruktor)
**
** 
*/
CHostSet::CHostSet()
{
}

/*
** ~CHostSet (Destruktor)
**
** 
*/
CHostSet::~CHostSet()
{
}

/*
** begin
**
** Liefert einen Iterator auf den Anfang der Host-Liste zurück.
*/
CHostList::iterator CHostSet::begin()
{
	return m_Hosts.begin();
}

/*
** end
**
** Liefert einen Iterator auf das Ende der Host-Liste zurück.
*/
CHostList::iterator CHostSet::end()
{
	return m_Hosts.end();
}

/*
** IsLocal
**
** Liefert 0, falls keines der im Set enthaltenen Elemente das Flag 'CO_LOCAL'
** gesetzt hat, 1, falls alle das Flag gesetzt haben und -1, falls einige es
** gesetzt haben.
*/
int CHostSet::IsLocal()
{
	bool set = false, notset = false;

	for (CHostList::iterator Iterator = m_Hosts.begin(); Iterator != m_Hosts.end(); Iterator++)
		if (Iterator->IsFlagSet(CO_LOCAL))
			set = true;
		else
			notset = true;
	
	return set ? (notset ? -1 : 1) : 0;
}

/*
** IsEmpty
**
** Liefert true, falls das Set leer ist.
*/
bool CHostSet::IsEmpty()
{
	return m_Hosts.empty();
}

/*
** MergeChanges
**
** Übernimmt alle Filder, die als geändert markiert wurden,
** aus dem Temp-Element in alle im Set enthaltenen Hosts.
*/
void CHostSet::MergeChanges()
{
	for (CHostList::iterator Iterator = m_Hosts.begin(); Iterator != m_Hosts.end(); Iterator++) {
		// EH_name nicht mergen, da dies das Schlüsselfeld ist!!
//		if(IsModified(EH_processors))	(*Iterator).processors		= Temp.processors;
		// >>> Code für neue Felder hier einfügen
	}
}

/*
** Clear
**
** Leert das komplette Set. Alle darin enthaltenen Elemente werden gelöscht.
*/
void CHostSet::Clear()
{
	m_Hosts.clear();
}

/*
** Delete
**
** Löscht aus dem Host-Set den Host mit der angegebenen ID. Falls dieser Host
** nicht existiert, wird nichts gemacht.
*/
void CHostSet::Delete(ULONG ID)
{
	for (CHostList::iterator Iterator = m_Hosts.begin(); Iterator != m_Hosts.end(); Iterator++)
		if (ID == Iterator->GetID()) {
			m_Hosts.erase(Iterator);
			break;
		}
}

/*
** Add
**
** Hängt den angegebenen Host ins Set ein und berechnet
** die Mehrdeutigkeit neu.
*/
void CHostSet::Add(CHost &h)
{
	m_Hosts.push_back(h);
	RecalcAmbiguous();
}

/*
** GetTemp
**
** Liefert einen Zeiger auf das temporäre Host-Objekt zurück.
*/
CHost *CHostSet::GetTemp()
{
	ASSERT(!m_Hosts.empty());
	m_Temp = m_Hosts.front();
	return &m_Temp;
}

/*
** DebugOut
**
** Gibt das komplette HostSet in der Debug-Konsole aus.
*/
void CHostSet::DebugOut()
{
	DENTER(GUI_LAYER, "CHostSet::DebugOut");
	m_Hosts.DebugOut();
	DEXIT;
}

/*
** RecalcAmbiguous
**
** Berechnet die Mehrdeutigkeit der Felder neu.
*/
void CHostSet::RecalcAmbiguous()
{
	ClearAmbiguous();

	if (m_Hosts.size() <= 1)
		return;
	
	CHostList::iterator Iterator = m_Hosts.begin();
	CHost h, fh = *Iterator;

	while ((++Iterator) != m_Hosts.end()) {
		if (fh.hostname		!= Iterator->hostname)		
			SetAmbiguous(EH_name);
		/*if (fh.realname		!= Iterator->realname)
			SetAmbiguous(EH_real_name);
		if (fh.processors	!= Iterator->processors)
			SetAmbiguous(EH_processors);
		*/
		// >>> Code für neue Felder hier einfügen
	}
}

/*
** operator lList*
**
** Wandelt das HostSet in eine Cull-Liste um.
** HINWEIS: Die zurückgegebene Cull-Liste muß von der aufrufenden Funktion
** gelöscht werden!
*/
CHostSet::operator lList* ()
{
	lList *lp = lCreateList("hosts", EH_Type);
	ASSERT(NULL != lp);

	lListElem *ep;
	for (CHostList::iterator Iterator = m_Hosts.begin(); Iterator != m_Hosts.end(); Iterator++) {
		ep = (lListElem *) *Iterator;
		ASSERT(NULL != ep);
		lAppendElem(lp, ep);
	}
	return lp;
}

/*
** operator lEnumeration*
**
** Erzeugt einen What-Deskriptor für das HostSet. Die Felder im What-Deskriptor
** werden entsprechend den Modify-Flags gesetzt.
** HINWEIS: Der zurückgegebene What-Deskriptor muß von der aufrufenden Funktion
** gelöscht werden.
*/
CHostSet::operator lEnumeration* ()
{
	SetModified(EH_name);	// Schlüsselfeld, wird immer benötigt!

	int *ModIntVector = GetModIntVector();
	ASSERT(NULL != ModIntVector);

	lEnumeration *what = lIntVector2What(EH_Type, ModIntVector);
	ASSERT(NULL != what);

	return what;
}

/*
** SetTag
**
** Setzt das Tag-Flag in allen im Set enthaltenen Objekten.
*/
void CHostSet::SetTag()
{
	for (CHostList::iterator Iterator = m_Hosts.begin(); Iterator != m_Hosts.end(); Iterator++)
		Iterator->SetFlag(CO_TAG);
}

/*
** ClearTag
**
** Löscht das Tag-Flag bei dem Objekt, das die angegebene ID besitzt.
*/
void CHostSet::ClearTag(ULONG ID)
{
	for (CHostList::iterator Iterator = m_Hosts.begin(); Iterator != m_Hosts.end(); Iterator++)
		if (ID == Iterator->GetID())
			Iterator->ClearFlag(CO_TAG);
}

/*
** DeleteTagged
**
** Löscht alle Elemente aus dem Host-Set, dessen Tag-Flag gesetzt ist.
*/
void CHostSet::DeleteTagged()
{
	CHostList::iterator Iterator = m_Hosts.begin();
	while (Iterator != m_Hosts.end())
		if (Iterator->IsFlagSet(CO_TAG))
			Iterator = m_Hosts.erase(Iterator);
		else 
			Iterator++;
		
	RecalcAmbiguous();
}
