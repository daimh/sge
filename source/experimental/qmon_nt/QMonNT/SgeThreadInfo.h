// SgeThreadInfo.h: Schnittstelle für die Klasse CSgeThreadInfo.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SGETHREADINFO_H__8E7C7763_30ED_11D2_A82D_0020AFA6CCC8__INCLUDED_)
#define AFX_SGETHREADINFO_H__8E7C7763_30ED_11D2_A82D_0020AFA6CCC8__INCLUDED_
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

#include "AnswerList.h"
#include "QueueSet.h"
#include "JobSet.h"
#include "HostSet.h"
#include "ComplexAtributeSet.h"

/*
** ThreadCommands:
*/
enum CThreadCommand {
	TC_NOCOMMAND		= 0,
	TC_SHUTDOWN			= 100,

	TC_GETALLQUEUES,
	TC_GETALLJOBS,
	TC_GETALLHOSTS,
	TC_GETALLCOMPLEXES,

	TC_MODQUEUES,

	TC_DELALLQUEUES,
	TC_DELALLJOBS,
	TC_DELALLHOSTS,
	TC_DELALLCOMPLEXES,

	TC_SUBMITJOB
	// >>> Code für neue Thread Commands hier einfügen
};

class CSgeThreadInfo  
{
public:
	CAnswerList				AnswerList;
	HANDLE					EventKillThread;
	HANDLE					EventThreadKilled;
	HANDLE					EventCommand;
	CWnd					*MainWindow;	// Fenster, an das die WM_SGENOTIFY-Nachrichten gesendet werden sollen.
	CThreadCommand			m_Command;
	LPARAM					m_Arg;
	static CSgeThreadInfo	SgeThreadInfo;  // definiert in SgeThreadInfo.cpp

	CSgeThreadInfo();
	virtual ~CSgeThreadInfo();

	static CWinThread *StartThread();

private:
	static UINT SgeThreadProc(LPVOID pParam);

	bool CheckQmasterIsAlive();

	void GetAllQueues();
	void GetAllJobs();
	void GetAllHosts();
	void GetAllComplexes();
	
	void ModQueues(CQueueSet *pQueueSet);

	void DelAllQueues(CQueueSet *pQueueSet);
	void DelAllJobs(CJobSet *pJobSet);
	void DelAllHosts(CHostSet *pHostSet);
	void DelAllComplexes(CComplexAtributeSet *pComplexAtributeSet);

	void SubmitJob(CJob *Job);
};

#endif // !defined(AFX_SGETHREADINFO_H__8E7C7763_30ED_11D2_A82D_0020AFA6CCC8__INCLUDED_)
