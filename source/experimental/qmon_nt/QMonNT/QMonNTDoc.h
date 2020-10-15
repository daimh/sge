// qmonntDoc.h : Schnittstelle der Klasse CQmonntDoc
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_QMONNTDOC_H__FA476C3B_302B_11D2_A82C_0020AFA6CCC8__INCLUDED_)
#define AFX_QMONNTDOC_H__FA476C3B_302B_11D2_A82C_0020AFA6CCC8__INCLUDED_
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

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/*
** UpdateAllViews-Hint-Constants:
**   - used in UpdateAllViews 
**   - show which part of the document changed
*/
enum {
	UAVH_MESSAGE	= 100,	/* massage text has been changed */

	UAVH_QUEUES,				/* queue changed */
	UAVH_JOBS,					/* job list changed */
	UAVH_HOSTS,					/* hostlist changed */
	UAVH_COMPLEXES,			/* complex list changed */

	UAVH_SELECTION,			/* selection in tree has changed */
	UAVH_NOMESSAGE				/* messagetext has been deleted */
	// >>> Code für neue Update Hints hier einfügen
};

#include "Queue.h"
#include "Job.h"
#include "Host.h"
#include "QueueSet.h"
#include "ComplexAtributeSet.h"
#include "ComplexAtribute.h"
#include "JobSet.h"
#include "HostSet.h"
#include "NodeInfo.h"
#include "SgeThreadInfo.h"
#include "Complexes.h"
#include <deque>

struct CCommand
{
	CThreadCommand m_nCode;
	LPARAM		   m_Arg;

	CCommand(CThreadCommand nCode) : m_nCode(nCode), m_Arg(NULL) {};
	CCommand(CThreadCommand nCode, LPARAM Arg) : m_nCode(nCode), m_Arg(Arg) {};
	CCommand() : m_nCode(TC_NOCOMMAND), m_Arg(NULL) {};
};

class CQmonntDoc : public CDocument
{
public:
	// Attributes
	CQueueList		*m_pQueueList;
	CJobList		*m_pJobList;
	CHostList		*m_pHostList;
	CComplexList	*m_pComplexList;

	CString			m_DummyJob;
	CString			m_DummyQueue;

	// Implementierung
	virtual ~CQmonntDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Operations
	void DeleteQueues(CNodeInfoSet *pSet);
	void DeleteJobs(CNodeInfoSet *pSet);
	void DeleteHosts(CNodeInfoSet *pSet);
	void DeleteComplexes(CNodeInfoSet *pSet);
	void SubmitJob(const CJob &Job);

	void GetQueueSet(CQueueSet *pQueueSet, CNodeInfoSet *pNodeInfoSet);
	void GetJobSet  (CJobSet   *pJobSet,   CNodeInfoSet *pNodeInfoSet);
	void GetHostSet (CHostSet  *pHostSet,  CNodeInfoSet *pNodeInfoSet);
	void GetComplexAtributeSet(CComplexAtributeSet *pAtributeSet, CNodeInfoSet *pNodeInfoSet);

	void SetChangedData(CQueueSet *QueueSet);
	void SetChangedData(CComplexAtributeSet *pAtributeSet);

	bool HasMessage();
	CString GetMessage();
	void SetMessage(UINT nID);
	
	void OnSgeNotify(WPARAM wp, LPARAM lp);

	bool IsDataAvailable();
	
	// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CQmonntDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void OnCloseDocument();
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

protected:
	CQmonntDoc();
	DECLARE_DYNCREATE(CQmonntDoc)

	// Generierte Message-Map-Funktionen
	//{{AFX_MSG(CQmonntDoc)
	afx_msg void OnEditRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool					m_bGetQueueListSucceeded;
	bool					m_bGetJobListSucceeded;
	bool					m_bGetHostListSucceeded;
	bool					m_bGetComplexListSucceeded;

	bool					m_bWorkerThreadIdle;
	bool					m_bHasMessage;
	CString					m_MessageText;
	CWinThread				*SgeThread;
	std::deque<CCommand>	m_CommandQueue;

	void SendCommand(CCommand Command);
	bool PostCommand(CCommand Command);

	void RetrieveAllQueues();
	void RetrieveAllJobs();
	void RetrieveAllHosts();
	void RetrieveAllComplexes();
	
	void OnConnectionFailed(LPARAM lp);
	void OnCodApiCallFailed(LPARAM lp);
	
	void OnGetJobListSucceeded(LPARAM lp);
	void OnGetQueueListSucceeded(LPARAM lp);
	void OnGetHostListSucceeded(LPARAM lp);
	void OnGetComplexListSucceeded(LPARAM lp);
	void OnSubmitJobSucceeded(LPARAM lp);

	void ShowAnswerList();
	void ClearMessage();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // !defined(AFX_QMONNTDOC_H__FA476C3B_302B_11D2_A82C_0020AFA6CCC8__INCLUDED_)
