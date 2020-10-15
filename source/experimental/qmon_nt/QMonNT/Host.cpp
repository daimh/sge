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
// Host.cpp: Implementierung der Klasse CHost.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "Host.h"
#include "Debug.h"

extern "C" {
#include "cull_multitype.h"
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
** CHost
**
** 
*/
CHost::CHost()
{
}

/*
** CHost
**
** Konstruktor, der ein Host-Objekt aus einem Cull-Listelement
** erzeugt und initialisiert
*/
CHost::CHost(lListElem *hep)
{
	ASSERT(NULL != hep);

	hostname   = lGetString(hep, EH_name);
	realname   = lGetString(hep, EH_real_name);
	processors = lGetUlong(hep,  EH_processors);
	//realname = "wrongType";
	//processors = 0;
	// >>> Code für neue Felder hier einfügen
}

/*
** ~CHost
**
** 
*/
CHost::~CHost()
{
}

/*
** operator lListElem*
**
** Wandelt das Host-Objekt in ein Cull-Listelement um.
** HINWEIS: Das zurückgegebene List-Element muß von der aufrufenden
** Funktion gelöscht werden!
*/
CHost::operator lListElem* ()
{
	lListElem *ep = lCreateElem(EH_Type);
	ASSERT(NULL != ep);

	lSetString(ep, EH_name,		  hostname.GetBuffer(0));
	lSetString(ep, EH_real_name,  realname.GetBuffer(0));
	lSetUlong (ep, EH_processors, processors);
	// >>> Code für neue Felder hier einfügen

	return ep;
}

/******************************************************************************
****                                                                       ****
**** Klasse: CHostList                                                     ****
****                                                                       ****
******************************************************************************/

/*
** FindByID
**
** Sucht ein Element mit der angegebenen ID in der Liste
** und liefert den Iterator darauf zurück. Falls der Iterator
** gleich end() ist, wurde das Element nicht gefunden!
*/
CHostList::iterator CHostList::FindByID(ULONG ID)
{
	for (CHostList::iterator it = begin(); it != end(); it++)
		if (ID == it->GetID())
			break;
	
	return it;
}

/*
** RemoveByID
**
** Löscht das Element mit der angegebenen ID aus der Liste.
*/
void CHostList::RemoveByID(ULONG ID)
{
	CHostList::iterator it = FindByID(ID);
	if (it != end())
		erase(it);
}

/*
** DebugOut (public)
**
** Gibt den Inhalt der Host-Liste auf der Debug-Konsole aus
** (Kontext GUI_LAYER).
*/
void CHostList::DebugOut()
{
	DENTER(GUI_LAYER, "CHostList::DebugOut");

	DPRINTF(("--- CHostList: -------------"));
	for (CHostList::iterator it = begin(); it != end(); it++) {
		DPRINTF(("ID: %ld, ", it->GetID()));
		// >> Code für neue Felder hier einfügen
	}
	DPRINTF((""));

	DEXIT;
}

lList *CHostList::MakelList()
{
	lList *lp = NULL;

	if (!empty()) {
	    lp = lCreateList("hosts", EH_Type);
		ASSERT(NULL != lp);
	}

	lListElem *ep;
	for (CHostList::iterator Iterator = begin(); Iterator != end(); Iterator++) {
		ep = (lListElem *) *Iterator;
		ASSERT(NULL != ep);
		lAppendElem(lp, ep);
	}

	return lp;
}
