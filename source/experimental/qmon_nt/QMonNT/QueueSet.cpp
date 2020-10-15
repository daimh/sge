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
// QueueSet.cpp: Implementierung der Klasse CQueueSet.
//
//////////////////////////////////////////////////////////////////////

// Checked by Stefan Mihaila

#include "stdafx.h"
#include "qmonnt.h"
#include "QueueSet.h"
#include "Debug.h"

extern "C" {
#include "cod_queueL.h"
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
** CQueueSet (Konstruktor)
**
** 
*/
CQueueSet::CQueueSet()
{
}

/*
** ~CQueueSet (Destruktor)
**
** 
*/
CQueueSet::~CQueueSet()
{
}

/*
** begin
**
** Liefert einen Iterator auf den Anfang der Queue-Liste zurück.
*/
CQueueList::iterator CQueueSet::begin()
{
	return m_Queues.begin();
}

/*
** end
**
** Liefert einen Iterator auf das Ende der Queue-Liste zurück.
*/
CQueueList::iterator CQueueSet::end()
{
	return m_Queues.end();
}

/*
** IsLocal
**
** Liefert 0, falls keines der im Set enthaltenen Elemente das Flag 'CO_LOCAL'
** gesetzt hat, 1, falls alle das Flag gesetzt haben und -1 falls einige es
** gesetzt haben.
*/
int CQueueSet::IsLocal()
{
	bool set = false, notset = false;

	for (CQueueList::iterator Iterator = m_Queues.begin(); Iterator != m_Queues.end(); Iterator++)
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
bool CQueueSet::IsEmpty()
{
	return m_Queues.empty();
}

/*
** MergeChanges
**
** Übernimmt alle Felder, die als geändert markiert wurden,
** aus dem Temp-Element in alle im Set enthaltenen Queues.
*/
void CQueueSet::MergeChanges()
{
	for (CQueueList::iterator Iterator = m_Queues.begin(); Iterator != m_Queues.end(); Iterator++) 
	{
		// QU_qname nicht mergen, da dies das Schlüsselfeld ist!!
		if (IsModified(QU_priority))
			Iterator->qpriority		= m_Temp.qpriority;
		if (IsModified(QU_job_slots))		
			Iterator->qjobslots		= m_Temp.qjobslots;
		if (IsModified(QU_qhostname))
			Iterator->qhostname		= m_Temp.qhostname;
		if (IsModified(QU_shell))
			Iterator->qshell		= m_Temp.qshell;
		if (IsModified(QU_tmpdir))
			Iterator->qtmpdir		= m_Temp.qtmpdir;
		if (IsModified(QU_notify))	
			Iterator->qnotifytime	= m_Temp.qnotifytime;
		if (IsModified(QU_h_rt))	
			Iterator->qhardrealtime	= m_Temp.qhardrealtime;
		if (IsModified(QU_s_rt))
			Iterator->qsoftrealtime	= m_Temp.qsoftrealtime;
		if (IsModified(QU_h_cpu))
			Iterator->qhardcputime	= m_Temp.qhardcputime;
		if (IsModified(QU_s_cpu))
			Iterator->qsoftcputime	= m_Temp.qsoftcputime;

		if (IsModified(QU_h_fsize))	
			Iterator->qhardfilesize	= m_Temp.qhardfilesize;
		if (IsModified(QU_s_fsize))
			Iterator->qsoftfilesize	= m_Temp.qsoftfilesize;
		if (IsModified(QU_h_data))	
			Iterator->qharddatasize	= m_Temp.qharddatasize;
		if (IsModified(QU_s_data))			
			Iterator->qsoftdatasize		= m_Temp.qsoftdatasize;
		if (IsModified(QU_h_stack))	
			Iterator->qhardstacksize	= m_Temp.qhardstacksize;
		if (IsModified(QU_s_stack))			
			Iterator->qsoftstacksize	= m_Temp.qsoftstacksize;
		if (IsModified(QU_h_core))		
			Iterator->qhardcorefilesize	= m_Temp.qhardcorefilesize;
		if (IsModified(QU_s_core))			
			Iterator->qsoftcorefilesize	= m_Temp.qsoftcorefilesize;
		if (IsModified(QU_h_rss))
			Iterator->qhardresidentsetsize  = m_Temp.qhardresidentsetsize;
		if (IsModified(QU_s_rss))			
			Iterator->qsoftresidentsetsize  = m_Temp.qsoftresidentsetsize;
		if (IsModified(QU_h_vmem))
			Iterator->qhardvirtualmemory	= m_Temp.qhardvirtualmemory;
		if (IsModified(QU_s_vmem))			
			Iterator->qsoftvirtualmemory	= m_Temp.qsoftvirtualmemory;

		// >>> Code für neue Felder hier einfügen
		if (IsModified(QU_migr_load_thresholds))
			Iterator->qCheckPointingList = m_Temp.qCheckPointingList;
	}
}

/*
** Clear
**
** Leert das komplette Set. Alle darin enthaltenen Elemente werden gelöscht.
*/
void CQueueSet::Clear()
{
	m_Queues.clear();
}

/*
** Delete
**
** Löscht aus dem Queue-Set diejenige Queue mit der angegebenen Objekt-ID.
** Falls diese Queue nicht vorhanden ist, wird nichts gemacht.
*/
void CQueueSet::Delete(ULONG ID)
{
	for (CQueueList::iterator it = m_Queues.begin(); it != m_Queues.end(); it++)
		if (ID == it->GetID()) {
			m_Queues.erase(it);
			break;
		}
}

/*
** Add
**
** Hängt die angegebene Queue ins Set ein und berechnet
** die Mehrdeutigkeit neu.
*/
void CQueueSet::Add(const CQueue &q)
{
	m_Queues.push_back(q);
	RecalcAmbiguous();
}

/*
** GetTemp (public)
**
** Liefert einen Zeiger auf das temoräre Queue-Objekt zurück.
*/
CQueue *CQueueSet::GetTemp()
{
	ASSERT(!m_Queues.empty());
	m_Temp = m_Queues.front();
	return &m_Temp;
}

/*
** DebugOut
**
** Gibt das komplette QueueSet in der Debug-Ausgabe aus.
*/
void CQueueSet::DebugOut()
{
	DENTER(GUI_LAYER, "CQueueSet::DebugOut");
	m_Queues.DebugOut();
	DEXIT;
}

/*
** RecalcAmbiguous
**
** Berechnet die Mehrdeutigkeit der Felder neu.
*/
void CQueueSet::RecalcAmbiguous()
{
	DENTER(GUI_LAYER, "CQueueSet::RecalcAmbiguous");

	ClearAmbiguous();

	if (m_Queues.size() <= 1) {
		DEXIT;
		return;
	}

	CQueueList::iterator sit = m_Queues.begin();
	CQueue fq = *sit;

	while ((++sit) != m_Queues.end()) {
		if (fq.qhostname != sit->qhostname)
			SetAmbiguous(QU_qhostname);

		if (fq.qjobslots != sit->qjobslots)
			SetAmbiguous(QU_job_slots);

		if (fq.qname     != sit->qname)
			SetAmbiguous(QU_qname);

		if (fq.qpriority != sit->qpriority)
			SetAmbiguous(QU_priority);

		if (fq.qshell	!= sit->qshell)
			SetAmbiguous(QU_shell);

		if (fq.qtmpdir	!= sit->qtmpdir)
			SetAmbiguous(QU_tmpdir);

		if (fq.qnotifytime.GetHour()   != sit->qnotifytime.GetHour() || 
			fq.qnotifytime.GetMinute() != sit->qnotifytime.GetMinute() || 
			fq.qnotifytime.GetSecond() != sit->qnotifytime.GetSecond())
				SetAmbiguous(QU_notify);

		if (fq.qhardrealtime.GetHour()	 != sit->qhardrealtime.GetHour() ||
			fq.qhardrealtime.GetMinute() != sit->qhardrealtime.GetMinute() || 
			fq.qhardrealtime.GetSecond() != sit->qhardrealtime.GetSecond())  
				SetAmbiguous(QU_h_rt);

		if (fq.qhardcputime.GetHour()   != sit->qhardcputime.GetHour() || 
			fq.qhardcputime.GetMinute() != sit->qhardcputime.GetMinute() ||
			fq.qhardcputime.GetSecond() != sit->qhardcputime.GetSecond())  
				SetAmbiguous(QU_h_cpu);

		if (fq.qhardfilesize		!=	sit->qhardfilesize)	
			SetAmbiguous(QU_h_fsize);

		if (fq.qharddatasize		!=	sit->qharddatasize)	
			SetAmbiguous(QU_h_data);

		if (fq.qhardstacksize	!=	sit->qhardstacksize)	
			SetAmbiguous(QU_h_stack);

		if (fq.qhardcorefilesize	!=	sit->qhardcorefilesize)	
			SetAmbiguous(QU_h_core);

		if (fq.qhardresidentsetsize	!=	sit->qhardresidentsetsize)
			SetAmbiguous(QU_h_rss);

		if (fq.qhardvirtualmemory	!=	sit->qhardvirtualmemory)	
			SetAmbiguous(QU_h_vmem);

		if (fq.qsoftrealtime.GetHour()   != sit->qsoftrealtime.GetHour() || 
			fq.qsoftrealtime.GetMinute() != sit->qsoftrealtime.GetMinute() || 
			fq.qsoftrealtime.GetSecond() != sit->qsoftrealtime.GetSecond())  
				SetAmbiguous(QU_s_rt);

		if (fq.qsoftcputime.GetHour()   != sit->qsoftcputime.GetHour() || 
			fq.qsoftcputime.GetMinute() != sit->qsoftcputime.GetMinute() || 
			fq.qsoftcputime.GetSecond() != sit->qsoftcputime.GetSecond())  
				SetAmbiguous(QU_s_cpu);

		if (fq.qsoftfilesize		!=	sit->qsoftfilesize)	
			SetAmbiguous(QU_s_fsize);

		if (fq.qsoftdatasize		!=	sit->qsoftdatasize)	
			SetAmbiguous(QU_s_data);

		if (fq.qsoftstacksize	!=	sit->qsoftstacksize)	
			SetAmbiguous(QU_s_stack);

		if (fq.qsoftcorefilesize	!=	sit->qsoftcorefilesize)	
			SetAmbiguous(QU_s_core);

		if (fq.qsoftresidentsetsize	!=	sit->qsoftresidentsetsize)
			SetAmbiguous(QU_s_rss);

		if (fq.qsoftvirtualmemory	!=	sit->qsoftvirtualmemory)	
			SetAmbiguous(QU_s_vmem);

		// >>> Code für neue Felder hier einfügen
	}

	DEXIT;
}

/*
** operator lList*
**
** Wandelt das QueueSet in eine Cull-Liste um.
** HINWEIS: Die zurückgegebene Cull-Liste muß von der aufrufenden Funktion
** gelöscht werden!
*/
CQueueSet::operator lList* ()
{
	 return m_Queues.MakelList();
}

/*
** operator lEnumeration*
**
** Erzeugt einen What-Deskriptor für das QueueSet. Die Felder im What-Deskriptor
** werden entsprechend den Modify-Flags gesetzt.
** HINWEIS: Der zurückgegebene What-Deskriptor muß von der aufrufenden Funktion 
** gelöscht werden!
*/
CQueueSet::operator lEnumeration* ()
{
	SetModified(QU_qname);	// Schlüsselfeld, wird immer benötigt!

	int *ModIntVector = GetModIntVector();
	ASSERT(NULL != ModIntVector);

	lEnumeration *what = lIntVector2What(QU_Type, ModIntVector);
	ASSERT(NULL != what);

	return what;
}

/*
** SetTag
**
** Setzt das Tag-Flag in allen im Set enthaltenen Objekten.
*/
void CQueueSet::SetTag()
{
	for (CQueueList::iterator it = m_Queues.begin(); it != m_Queues.end(); it++)
		it->SetFlag(CO_TAG);
}

/*
** ClearTag
**
** Löscht das Tag-Flag bei dem Objekt, das die angegebene ID besitzt.
*/
void CQueueSet::ClearTag(ULONG ID)
{
	for (CQueueList::iterator it = m_Queues.begin(); it != m_Queues.end(); it++) 
		if (ID == it->GetID())
			it->ClearFlag(CO_TAG);
}

/*
** DeleteTagged
**
** Löscht alle Elemente aus dem Queue-Set, dessen Tag-Flag gesetzt ist.
*/
void CQueueSet::DeleteTagged()
{
	CQueueList::iterator it = m_Queues.begin();
	while (it != m_Queues.end())
		if (it->IsFlagSet(CO_TAG))
			it = m_Queues.erase(it);
		else
			it++;

	RecalcAmbiguous();
}
