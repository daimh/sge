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
#include "stdafx.h"
#include <windows.h>
#include <winbase.h>
#include <userenv.h>
#include <shlwapi.h>

#include <iostream>

#include "CBasicTypes.h"
#include "CJobObject.h"
#include "CError.h"
#include "CException.h"
#include "CIoCompletionPort.h"
#include "CMutex.h"
#include "CSehTranslator.h"
#include "CThread.h"

using namespace std;

namespace GridEngine {

CJobObject::CJobIoCompletionPort::CJobIoCompletionPort(PVOID lpParameter, DWORD dwCompletionKey, DWORD dwCuncurrentThreads, DWORD dwTimeout) :
   CIoCompletionPort(lpParameter, dwCompletionKey, dwCuncurrentThreads, dwTimeout)
{
}

DWORD CJobObject::CJobIoCompletionPort::IoCompletionPortMain(PVOID lpParameter, CStatus TheStatus) {
   CJobObject *TheJob = (CJobObject*)(lpParameter);

   switch (TheStatus.dwNumberOfBytes) {
      case JOB_OBJECT_MSG_END_OF_JOB_TIME:
         TheJob->ActionEndOfJobTime();
         break;
      case JOB_OBJECT_MSG_END_OF_PROCESS_TIME:
         TheJob->ActionEndOfProcessTime();
         break;
      case JOB_OBJECT_MSG_ACTIVE_PROCESS_LIMIT:
         TheJob->ActionActiveProcessLimit();
         break;
      case JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS:
         TheJob->ActionAbnormalExitProcess();
         break;
      case JOB_OBJECT_MSG_JOB_MEMORY_LIMIT:
         TheJob->ActionJobMemoryLimit();
         break;
      case JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT:
         TheJob->ActionProcessMemoryLimit();
         break;
      case JOB_OBJECT_MSG_NEW_PROCESS:
         TheJob->ActionNewProcess();
         break;
      case JOB_OBJECT_MSG_EXIT_PROCESS:
         TheJob->ActionExitProcess();
         break;
      case JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO:
         TheJob->m_JobTerminated = true;
         TheJob->ActionActiveProcessZero();
         break;
   }
   return 0;
}

CJobObject::CJobObject() :
   m_dwJobId(0),
   m_JobTerminated(FALSE),
   m_Processes(),
   m_dwBuffer(NULL),
   m_pCompletionPort(NULL)
{
   CMutex::CEnter MutexEnter(m_Mutex);

   CreateNewJob();
   AssignCompletionPort();
}

CJobObject::CJobObject(DWORD dwJobId) :
   m_dwJobId(dwJobId),
   m_JobTerminated(FALSE),
   m_Processes(),
   m_dwBuffer(NULL),
   m_pCompletionPort(NULL)
{
   CMutex::CEnter MutexEnter(m_Mutex);

   OpenExistingJob();

   // It is not possible to assign a second,third... completion port
   // I don't know why

//   AssignCompletionPort();
}

CJobObject::~CJobObject() {
   CMutex::CEnter MutexEnter(m_Mutex);
   if (m_pCompletionPort) {
      delete m_pCompletionPort;
   }
}

void CJobObject::AssignProcess(CProcess& Process) {
   BOOL bRet;
   CMutex::CEnter MutexEnter(m_Mutex);

   bRet = AssignProcessToJobObject(GetHandle(), Process.GetHandle());   
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

void CJobObject::AssignCompletionPort() {
   CMutex::CEnter MutexEnter(m_Mutex);
   BOOL bRet;
   JOBOBJECT_ASSOCIATE_COMPLETION_PORT ObjectInfo;

   m_pCompletionPort = new CJobIoCompletionPort(this, 0, 0, INFINITE);

   ObjectInfo.CompletionKey = 0;
   ObjectInfo.CompletionPort = m_pCompletionPort->GetHandle();
   bRet = SetInformationJobObject(GetHandle(), JobObjectAssociateCompletionPortInformation, 
      &ObjectInfo, sizeof(JOBOBJECT_ASSOCIATE_COMPLETION_PORT));
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }

}

DWORD CJobObject::ActionEndOfJobTime() {
   return 0;
}

DWORD CJobObject::ActionEndOfProcessTime() {
   return 0;
}

DWORD CJobObject::ActionActiveProcessLimit() {
   return 0;
}

DWORD CJobObject::ActionAbnormalExitProcess() {
   return 0;
}

DWORD CJobObject::ActionJobMemoryLimit() {
   return 0;
}

DWORD CJobObject::ActionProcessMemoryLimit() {
   return 0;
}

DWORD CJobObject::ActionNewProcess() {
   return 0;
}

DWORD CJobObject::ActionExitProcess() {
   return 0;
}

DWORD CJobObject::ActionActiveProcessZero() {
   return 0;
}

void CJobObject::CreateNewJob() {
   CMutex::CEnter MutexEnter(m_Mutex);
   static const DWORD dwMaxBufferSize = 1024;
   WCHAR pszBuffer[dwMaxBufferSize];
   CString JobName;
   BOOL bError;
   HANDLE hJob;

   m_dwJobId = 0;
   bError = true;
   while (bError) {
      m_dwJobId++;
      if (m_dwJobId == 0) {
         m_dwJobId++;
      }
      wsprintf(pszBuffer, L"%ld", m_dwJobId);
      JobName = pszBuffer;

      hJob = CreateJobObject(0, const_cast<PWSTR>(JobName.c_str()));
      if(hJob == NULL) {
         bError = true;
      } else if (GetLastError() == ERROR_ALREADY_EXISTS) {
         bError = true;
      } else {
         bError = false;
         SetHandle(hJob);
      }
   }
}

void CJobObject::SetExtendedLimit(const JOBOBJECT_EXTENDED_LIMIT_INFORMATION& pLimit) {
   static BOOL bAlreadySet = FALSE;
   BOOL bRet;

   if (bAlreadySet == FALSE) {
      JOBOBJECT_END_OF_JOB_TIME_INFORMATION EndOfJobTimeAction;

      EndOfJobTimeAction.EndOfJobTimeAction = JOB_OBJECT_POST_AT_END_OF_JOB;
      bRet = SetInformationJobObject(GetHandle(), JobObjectEndOfJobTimeInformation,
         (PVOID) &EndOfJobTimeAction, sizeof(JOBOBJECT_END_OF_JOB_TIME_INFORMATION));
   }
   bRet = SetInformationJobObject(GetHandle(), JobObjectExtendedLimitInformation,
      (PVOID) &pLimit, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

const JOBOBJECT_EXTENDED_LIMIT_INFORMATION& CJobObject::GetExtendedLimit() const {
   BOOL bRet;

   bRet = QueryInformationJobObject(GetHandle(), JobObjectExtendedLimitInformation,
      (PVOID) &m_Limit, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION), NULL);
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
   return m_Limit;
}

DWORD CJobObject::GetId() {
   return m_dwJobId;
}

const JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION& CJobObject::GetExtendedUsageInfo() const{
   BOOL bRet;

   bRet = QueryInformationJobObject(GetHandle(), JobObjectBasicAndIoAccountingInformation,
      (PVOID) &m_UsageInfo, sizeof(JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION), NULL);
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
   return m_UsageInfo;
}


void CJobObject::OpenExistingJob() {
   CMutex::CEnter MutexEnter(m_Mutex);
   static const DWORD dwMaxBufferSize = 1024;
   WCHAR pszBuffer[dwMaxBufferSize];
   CString JobName;
   HANDLE hJob;

   if (m_dwJobId != 0) {
      wsprintf(pszBuffer, L"%ld", m_dwJobId);
      JobName = pszBuffer;
      hJob = CreateJobObject(0, const_cast<PWSTR>(JobName.c_str()));

      if(hJob == NULL) {
         throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
      } else {
         SetHandle(hJob);
      }
   } else {
      throw CCodineException(L"Can not create a job object with id 0", __FILE__, __LINE__);
   }
}

void CJobObject::Resume() {
   CMutex::CEnter MutexEnter(m_Mutex);
   PJOBOBJECT_BASIC_PROCESS_ID_LIST pPidList;

   pPidList = CreatePidList();

   if (pPidList == NULL) {
      throw CCodineException(L"Pid List is NULL", __FILE__, __LINE__);
   } else {
      for (int i = 0; i < pPidList->NumberOfProcessIdsInList; i++) {
         try {
            CProcess NewProcess(pPidList->ProcessIdList[i]);

            NewProcess.Resume();
         } catch (CCodineException&) {
            ;
         }
      }
   }
   DestroyPidList();
}

void CJobObject::Suspend() {
   CMutex::CEnter MutexEnter(m_Mutex);

   map<DWORD, int> ErrorProcesses;             
   map<DWORD, int>::iterator ErrorProcess;
   map<DWORD, BOOL> Processes;             
   map<DWORD, BOOL>::iterator CurrentProcess;
   map<DWORD, CProcess*>::iterator OldProcess;
   BOOL bNewProcesses;

   // Clear all Threads contained in the m_Threads member
   for (OldProcess = m_Processes.begin();
        OldProcess != m_Processes.end();
        OldProcess++) {
      delete OldProcess->second;
   }
   m_Processes.clear();

   
   // A)

   // Seach all threads which belong to this process and suspend them. To do this we create
   // a snapshot and suspend all threads contained in the snapshot.
   // Between A1 and A2 it is possible that new threads will be created. This makes it necessary to 
   // create a new snapshot and to suspend all new threads which we not suspended in a previous 
   // iteration.

   bNewProcesses = TRUE;
   while (bNewProcesses) {
      PJOBOBJECT_BASIC_PROCESS_ID_LIST pPidList;

      bNewProcesses = FALSE;
      pPidList = CreatePidList();
      // A1

      if (pPidList == NULL) {
         throw CCodineException(L"Pid List is NULL", __FILE__, __LINE__);
      } else {
         for (int i = 0; i < pPidList->NumberOfProcessIdsInList; i++) {
            if (m_Processes.find(pPidList->ProcessIdList[i]) == m_Processes.end()) {
               CProcess *NewProcess = NULL;

               try {
                  NewProcess = new CProcess(pPidList->ProcessIdList[i]);
                  NewProcess->Suspend();
                  m_Processes[pPidList->ProcessIdList[i]] = NewProcess;
                  Processes[pPidList->ProcessIdList[i]] = true;
                  bNewProcesses = TRUE;
               } catch (CCodineException& ex) {
                  ErrorProcess = ErrorProcesses.find(pPidList->ProcessIdList[i]);
                  if (ErrorProcess == ErrorProcesses.end() 
                      || (ErrorProcess != ErrorProcesses.end() && ErrorProcess->second < 3)) {

                     // Possible reasons for the exception
                     //    * We didn't get a handle
                     //       - Thread exited
                     //    * Thread belongs to a different process 
                     //      (no access)
                     //       - Thread exited and another process created 
                     //         a thread with the same id between A1 and A2
                     //
                     // => we try it once more -> the next snapshot won't 
                     //    contain the current thread

                     CTrace::Print(CTrace::Layer::TEST, L"Catched Exception during suspended"
                        L" for thread %ld", pPidList->ProcessIdList[i]);

                     ErrorProcesses[pPidList->ProcessIdList[i]]++;
                     bNewProcesses = TRUE;
                  } else {

                     // If we get an error three times for the same thread
                     // then we will assume that this thread belongs really 
                     // to this process
                     //
                     // => we could not solve the problem => ERROR
                     throw ex;
                  }
               }
            }
         }
         // A2
      }
      DestroyPidList();
   }      

   // B

   // Between A and B we created  a map of suspended threads (Threads) 
   // but we can not be sure that all these threads belong to this process 
   // (=> recycling of thread id's). It is possible, that we suspended 
   // threads of foreign processes!. Now we will set pair.second to false 
   // for all threads which belong to the current process
   {
      PJOBOBJECT_BASIC_PROCESS_ID_LIST pPidList;
      int i;


      pPidList = CreatePidList();
      if (pPidList == NULL) {
         throw CCodineException(L"Pid List is NULL", __FILE__, __LINE__);
      } else {
         try {
            for (i = 0; i < pPidList->NumberOfProcessIdsInList; i++) {
               CurrentProcess = Processes.find(pPidList->ProcessIdList[i]);
               if (CurrentProcess == Processes.end()) {
                  // If we get a new thread id here, then someone else
                  // created a new thread for this process (CreateRemoteThread())
                  CTrace::Print(CTrace::Layer::TEST, L"Unexpected new thread %ld", pPidList->ProcessIdList[i]);
                  throw CCodineException(L"", __FILE__, __LINE__);
               } else {
                  CurrentProcess->second = FALSE;
               }
            }
         } catch (CCodineException& ex) {
            map<DWORD, CProcess*>::iterator TheProcess;

            // If we come here then we will rollback previous activities
            // Therefore we have to resume all threads contained in Threads            
            for (TheProcess = m_Processes.begin();
                 TheProcess != m_Processes.end();
                 TheProcess++) {
               try {
                  (*TheProcess).second->Resume();
               } catch (CCodineException&) {
                  ;
               }
            }
            throw ex;
         }
      }
      DestroyPidList();
   }

   // C

   // Between B and C we set pair.second to false for all threads 
   // which belong to the current process
   // Now we have to resume the remaining threads
   // We will also add the suspended threads of this process to
   // the member m_Threads. 
   try {
      // Resume and copy
      for (CurrentProcess = Processes.begin();
           CurrentProcess != Processes.end();
           CurrentProcess++) {
         if (CurrentProcess->second == TRUE) {
            CProcess *ForeignProcess = m_Processes[CurrentProcess->first];

            ForeignProcess->Resume();
            m_Processes.erase(CurrentProcess->first);
            CTrace::Print(CTrace::Layer::TEST, L"Resumed foreign thread %ld", CurrentProcess->first);
            break;
         } 
      }
   } catch (CCodineException& ex) {
   
      // What can we do here?

      throw ex;
   }
}

PJOBOBJECT_BASIC_PROCESS_ID_LIST CJobObject::CreatePidList() const{
   DWORD dwMaxEntries = 8;
   JOBOBJECT_BASIC_PROCESS_ID_LIST *Structure;


   DestroyPidList();
   do {
      BOOL bRet;

      if (m_dwBuffer) {
         dwMaxEntries *= 2;
         delete [] m_dwBuffer;
         m_dwBuffer = NULL;
      }
      m_dwBuffer = new DWORD[dwMaxEntries + 2];
      Structure = (JOBOBJECT_BASIC_PROCESS_ID_LIST*)m_dwBuffer;

      memset(m_dwBuffer, 0, sizeof(m_dwBuffer));
      bRet = QueryInformationJobObject(GetHandle(), JobObjectBasicProcessIdList, 
         m_dwBuffer, sizeof(DWORD) * (dwMaxEntries + 2), NULL);
      if (bRet == 0 && GetLastError() != ERROR_MORE_DATA) {
         delete [] m_dwBuffer;
         throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
      }
   } while (Structure->NumberOfAssignedProcesses > Structure->NumberOfProcessIdsInList);

   return Structure;
}

void CJobObject::DestroyPidList() const {
   if (m_dwBuffer) {
      delete [] m_dwBuffer;
      m_dwBuffer = NULL;
   }
}

void CJobObject::Terminate(DWORD dwExitCode) {
   CMutex::CEnter MutexEnter(m_Mutex);
   BOOL bRet;

   bRet = TerminateJobObject(GetHandle(), dwExitCode);
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}


void CJobObject::WaitForObject(DWORD dwMilliseconds) const {
   PJOBOBJECT_BASIC_PROCESS_ID_LIST pPidList;

   while ((pPidList = CreatePidList())) {
      DWORD dwProcesses = pPidList->NumberOfProcessIdsInList;

      DestroyPidList();
      if (dwProcesses == 0) {
         break;
      } else if (dwProcesses > 1) {
         Sleep(1000);
      } else if (dwProcesses > 10) {
         Sleep(2000);
      } else if (dwProcesses > 100) {
         Sleep(3000);
      } 
   }
}

} // namespace
