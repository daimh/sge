#if !defined(AFX_JOB_H__42C1AC52_4643_49FB_9DC4_F55E61F59522__INCLUDED_)
#define AFX_JOB_H__42C1AC52_4643_49FB_9DC4_F55E61F59522__INCLUDED_
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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
#include <windows.h>
#include <map>

#include "CBasicTypes.h"
#include "CKernelObject.h"
#include "CIoCompletionPort.h"
#include "CMutex.h"
#include "CProcess.h"
#include "CThread.h"

namespace GridEngine {

class CJobObject : public CKernelObject {
 private:
   class CJobIoCompletionPort : public CIoCompletionPort {
    public:
      CJobIoCompletionPort(PVOID lpParameter, DWORD dwCompletionKey, DWORD dwCuncurrentThreads, DWORD dwTimeout);
      DWORD IoCompletionPortMain(PVOID lpParameter, CStatus);
   };

   friend CJobIoCompletionPort;

 private:
   // Job id
   DWORD m_dwJobId;                  

   // Completion Port
   CJobIoCompletionPort *m_pCompletionPort;           

   // Mutex which protects all private members
   CMutex m_Mutex;

   // true if all processes of the job are terminated
   BOOL m_JobTerminated;

   // list of processes
   map<DWORD, CProcess*> m_Processes;

   mutable DWORD *m_dwBuffer;

   JOBOBJECT_EXTENDED_LIMIT_INFORMATION m_Limit;

   JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION m_UsageInfo;

 private:
   DWORD HandleMessagesFromCompletionPort();

   virtual void AssignCompletionPort();
   virtual void CreateNewJob();
   virtual PJOBOBJECT_BASIC_PROCESS_ID_LIST CreatePidList() const;
   virtual void DestroyPidList() const;
   virtual void OpenExistingJob();

 protected:
   virtual void SetExtendedLimit(const JOBOBJECT_EXTENDED_LIMIT_INFORMATION& pLimit);
   virtual const JOBOBJECT_EXTENDED_LIMIT_INFORMATION& GetExtendedLimit() const;
   virtual const JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION& GetExtendedUsageInfo() const;

 public:
	CJobObject();
   CJobObject(DWORD dwOsJobId);
	virtual ~CJobObject();

   virtual DWORD ActionEndOfJobTime();
   virtual DWORD ActionEndOfProcessTime();
   virtual DWORD ActionActiveProcessLimit();
   virtual DWORD ActionAbnormalExitProcess();
   virtual DWORD ActionJobMemoryLimit();
   virtual DWORD ActionProcessMemoryLimit();
   virtual DWORD ActionNewProcess();
   virtual DWORD ActionExitProcess();
   virtual DWORD ActionActiveProcessZero();

   virtual void AssignProcess(CProcess& Process);

   virtual DWORD GetId();

   virtual void Resume();
   virtual void Suspend();
   virtual void Terminate(DWORD dwExitCode);

   virtual void WaitForObject(DWORD dwMilliseconds = INFINITE) const;

};

} // namespace

#endif // !defined(AFX_JOB_H__42C1AC52_4643_49FB_9DC4_F55E61F59522__INCLUDED_)
