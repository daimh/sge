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
// CodObj.cpp: Implementierung der Klasse CCodObj.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "CodObj.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

ULONG CCodObj::NextID = 1;


CCodObj::CCodObj()
{
	ID		= NextID++;
	m_Tag	= false;
	m_Flag	= 0;
}

CCodObj::CCodObj(const CCodObj &c)
{
	TRACE("CCodObj::CCodObj (Copyconstructor)\n");
	ID		= c.ID;
	m_Tag	= c.m_Tag;
	m_Flag	= c.m_Flag;
}

CCodObj::~CCodObj()
{
}

/*
** GetID
**
** Liefert die ID des aktuellen Objekts zurück
*/
ULONG CCodObj::GetID() const
{
	return ID;
}

/*
** SetID
**
** Setzt die ID des Objekts auf die angegebene neue ID. Wird
** benötigt, um Objekte, die durch neue Versionen ersetzt
** werden, über die gleiche ID wieder identifizieren zu können.
*/
void CCodObj::SetID(ULONG NewID)
{
	ID = NewID;
}

/*
** SetFlag
**
** Setzt das angegebene Flag im Objekt.
*/
void CCodObj::SetFlag(ULONG Flag)
{
	m_Flag |= Flag;
}

/*
** ClearFlag
**
** Löscht das angegebene Flag im Objekt.
*/
void CCodObj::ClearFlag(ULONG Flag)
{
	m_Flag &= ~Flag;
}

bool CCodObj::IsFlagSet(ULONG Flag)
{
	return 0 != (m_Flag & Flag);
}
