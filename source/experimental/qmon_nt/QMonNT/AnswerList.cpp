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
// AnswerList.cpp: Implementierung der Klasse CAnswerList.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "AnswerList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////7
// CAnswer

CAnswer::CAnswer(lListElem *aep)
{
	ASSERT(NULL != aep);

	status	= lGetUlong(aep,	AN_status);
	text    = lGetString(aep,	AN_text);
	quality = lGetUlong(aep,	AN_quality);
}

CAnswer::operator lListElem* ()
{
	lListElem *ep = lCreateElem(AN_Type);
	ASSERT(NULL != ep);

	lSetUlong (ep, AN_status,	status);
	lSetString(ep, AN_text,		text.GetBuffer(0));
	lSetUlong(ep,  AN_quality,  quality);
	
	return ep;
}

////////////////////////////////////////////////////
// CAnswerList

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CAnswerList::CAnswerList()
{
	InitializeCriticalSection(&CriticalSection);
}

CAnswerList::CAnswerList(lList *alp)
{
	InitializeCriticalSection(&CriticalSection);

	ASSERT(NULL != alp);

	// Answer-List alp durchsuchen und alle Einträge in m_Entries kopieren:
	lListElem *ep;
	for_each_cpp(ep, alp)
		m_Answers.push_back(CAnswer(ep));
}

CAnswerList::~CAnswerList()
{
	DeleteCriticalSection(&CriticalSection);
}

void CAnswerList::Append(lList *alp)
{
	EnterCriticalSection(&CriticalSection);

	ASSERT(NULL != alp);

	// Answer-List alp durchsuchen und alle Einträge an m_Entries anhängen:
	lListElem *ep;
	for_each_cpp(ep, alp)
		if (NUM_AN_ERROR == lGetUlong(ep, AN_quality))
			m_Answers.push_back(CAnswer(ep));
	
	LeaveCriticalSection(&CriticalSection);
}

CAnswer *CAnswerList::PopFront()
{
	EnterCriticalSection(&CriticalSection);

	// neues Element erstellen, dessen Zeiger zurückgegeben wird.
	// Für das Löschen des Elements ist die aufrufende Funktion
	// zuständig!
	CAnswer *Answer = new CAnswer(m_Answers.front());
	ASSERT(NULL != Answer);

	//m_Entries.front();

	LeaveCriticalSection(&CriticalSection);

	return Answer;
}

/*
** IsEmpty
**
** Liefert true zurück, falls die Answerlist leer ist,
** sonst false.
*/
bool CAnswerList::IsEmpty()
{
	return m_Answers.empty();
}

/*
** Clear
**
** Löscht die Answerlist.
*/
void CAnswerList::Clear()
{
	m_Answers.clear();
}
