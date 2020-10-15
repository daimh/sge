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
#include <Tlhelp32.h>
#include <map>
#include <vector>

#include "CProcess.h"
#include "CError.h"
#include "CEnvironment.h"
#include "CException.h"
#include "CHandle.h"
#include "CThread.h"
#include "CTrace.h"
#include "CUser.h"

using namespace std;

namespace GridEngine {

CProcess::CProcess() :
   m_dwProcessId(GetCurrentProcessId()),
   m_dwThreadId(0),
   m_Command(L""),
   m_pUser(NULL)
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::CProcess())");
   OpenExistingProcess();
}

CProcess::CProcess(CString Command, Type Type) :
   m_dwProcessId(0),
   m_dwThreadId(0),
   m_Command(Command),
   m_pUser(NULL)
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::CProcess(CString, Type))");
   CreateNewProcess(NULL, NULL, Type);
}

CProcess::CProcess(CString Command, const CUser& StartUser, Type Type) :
   m_dwProcessId(0),
   m_dwThreadId(0),
   m_Command(Command),
   m_pUser(new CUser(StartUser))
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::CProcess(CString, CUser, Type))");
   CreateNewProcess(m_pUser, NULL, Type);
}

CProcess::CProcess(CString Command, const CUser& StartUser, CEnvironment& Environment, Type Type) :
   m_dwProcessId(0),
   m_dwThreadId(0),
   m_Command(Command),
   m_pUser(new CUser(StartUser))
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::CProcess(CString, CUser, CEnvironment, Type))");
   CreateNewProcess(m_pUser, &Environment, Type);
}

CProcess::CProcess(CString Command, CEnvironment& Environment) :
   m_dwProcessId(0),
   m_dwThreadId(0),
   m_Command(Command),
   m_pUser(NULL)
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::CProcess(CString, CEnvironment))");
   CreateNewProcess(NULL , &Environment, INVISIBLE);
}

CProcess::CProcess(DWORD dwProcessId) :
   m_dwProcessId(dwProcessId),
   m_dwThreadId(0),
   m_Command(L""),
   m_pUser(NULL)
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::CProcess(DWORD)");
   OpenExistingProcess();
}

CProcess::~CProcess() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::~CProcess()");
}

void CProcess::CreateNewProcess(CUser *StartUser, CEnvironment *Environment, Type Type) {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::CreateNewProcess(CUser, CEnvironment, Type)");
   BOOL bRet;
   PROCESS_INFORMATION ProcessInfo;
   STARTUPINFO StartupInfo;

   // Create a ne Process (with one thread)
   memset(&StartupInfo, 0, sizeof(STARTUPINFO));
   StartupInfo.cb = sizeof(STARTUPINFO);
   if (Type == INVISIBLE) {
      StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
      StartupInfo.wShowWindow = SW_HIDE;
   } else {
      ;
   }
   if (StartUser) {
      bRet = CreateProcessAsUser(StartUser->GetHandle(), NULL, const_cast<PWSTR>(m_Command.c_str()), NULL, NULL, FALSE,
         CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP
         | CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT,
         Environment ? Environment->GetEnvironmentBlock() : NULL, 
         NULL, 
         &StartupInfo, &ProcessInfo);
   } else {
      bRet = CreateProcess(NULL, const_cast<PWSTR>(m_Command.c_str()), NULL, NULL, FALSE,
         CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP
         | CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT,
         Environment ? Environment->GetEnvironmentBlock() : NULL, 
         NULL, 
         &StartupInfo, &ProcessInfo);
   }
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } 
   m_dwProcessId = ProcessInfo.dwProcessId;
   m_dwThreadId = ProcessInfo.dwThreadId;
   SetHandle(ProcessInfo.hProcess);

   // We don't need this Handle to the master thread anymore
   CloseHandle(ProcessInfo.hThread);

}

DWORD CProcess::GetExitCode() const {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::GetExitCode()");
   DWORD dwExitCode;

   GetExitCodeProcess(GetHandle(), &dwExitCode);
   return dwExitCode;
}

DWORD CProcess::GetProcessId() const {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::GetProcessId()");
   return m_dwProcessId;
}

HANDLE CProcess::GetSnapshot() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::GetSnapshot()");
   static const DWORD dwDelay = 100;
   static const DWORD dwMaxRetries = 10;
   HANDLE hSnapshot;
   DWORD dwRetries;

   // Threads which create new threads with a short delay will result in 
   // an error of the CreateToolhelp32Snapshot function. We will retry 
   // to get a snapshot several times to solve this problem

   hSnapshot = INVALID_HANDLE_VALUE;
   dwRetries = 0;
   while (hSnapshot == INVALID_HANDLE_VALUE && dwRetries < dwMaxRetries) {
      if (dwRetries != 0) {
         Sleep(dwDelay);
      }
      hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, m_dwProcessId);
      dwRetries++;
   }
   return hSnapshot;
}

void CProcess::OpenExistingProcess() {
   HANDLE hProcess;

   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::OpenExistingProcess()");
   hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, m_dwProcessId);
   if (hProcess == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } else {
      SetHandle(hProcess);
   }
}

void CProcess::Resume() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::Resume()");
   CHandle Snapshot;

   Snapshot.SetHandle(GetSnapshot());
   if (Snapshot.GetHandle() == INVALID_HANDLE_VALUE) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } else {
      THREADENTRY32 ThreadEntry;
      BOOL bContinue;

      ThreadEntry.dwSize = sizeof(THREADENTRY32);
      for (bContinue = Thread32First(Snapshot.GetHandle(), &ThreadEntry); 
           bContinue; 
           bContinue = Thread32Next(Snapshot.GetHandle(), &ThreadEntry)) {
         if (ThreadEntry.th32OwnerProcessID == m_dwProcessId) {
            try {
               if (m_pUser) {
                  CThread NewThread(ThreadEntry.th32ThreadID, *m_pUser);
                  NewThread.Resume();
               } else {
                  CThread NewThread(ThreadEntry.th32ThreadID);
                  NewThread.Resume();
               }
            } catch (CCodineException&) {
               ;
            }
         }
      }
   }
}

void CProcess::Start() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::Start()");

   Resume();
}

void CProcess::Suspend() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::Suspend()");
   map<DWORD, BOOL> Threads;             
   map<DWORD, BOOL>::iterator CurrentThread;
   map<DWORD, int> ErrorThreads;             
   map<DWORD, int>::iterator ErrorThread;
   BOOL bNewThreads;

   // A)

   // Seach all threads which belong to this process and suspend them. To do this we create
   // a snapshot and suspend all threads contained in the snapshot.
   // Between A1 and A2 it is possible that new threads will be created. This makes it necessary to 
   // create a new snapshot and to suspend all new threads which we not suspended in a previous 
   // iteration.

   bNewThreads = TRUE;
   while (bNewThreads) {
      CHandle Snapshot;

      bNewThreads = FALSE;
      Snapshot.SetHandle(GetSnapshot());
      // A1

      if (Snapshot.GetHandle() == INVALID_HANDLE_VALUE) {
         throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
      } else {
         THREADENTRY32 ThreadEntry;
         BOOL bContinue;

         ThreadEntry.dwSize = sizeof(THREADENTRY32);
         for (bContinue = Thread32First(Snapshot.GetHandle(), &ThreadEntry); 
              bContinue; 
              bContinue = Thread32Next(Snapshot.GetHandle(), &ThreadEntry)) {
            if (ThreadEntry.th32OwnerProcessID == m_dwProcessId) {
               if (Threads.find(ThreadEntry.th32ThreadID) == Threads.end()) {
                  try {
                     if (m_pUser) {
                        CThread NewThread(ThreadEntry.th32ThreadID, *m_pUser);
             
                        NewThread.Suspend();
                     } else {
                        CThread NewThread(ThreadEntry.th32ThreadID);
             
                        NewThread.Suspend();
                     }

                     Threads[ThreadEntry.th32ThreadID] = TRUE;
                     bNewThreads = TRUE;
                  } catch (CCodineException& ex) {
                     ErrorThread = ErrorThreads.find(ThreadEntry.th32ThreadID);
                     if (ErrorThread == Threads.end() 
                         || (ErrorThread != Threads.end() && ErrorThread->second < 3)) {

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
                           L" for thread %ld", ThreadEntry.th32ThreadID);

                        ErrorThreads[ThreadEntry.th32ThreadID]++;
                        bNewThreads = TRUE;
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
         }
         // A2
      }
   }

   // B

   // Between A and B we created  a map of suspended threads (Threads) 
   // but we can not be sure that all these threads belong to this process 
   // (=> recycling of thread id's). It is possible, that we suspended 
   // threads of foreign processes!. Now we will set pair.second to false 
   // for all threads which belong to the current process
   {
      CHandle Snapshot;
      THREADENTRY32 ThreadEntry;
      BOOL bContinue;

      Snapshot.SetHandle(GetSnapshot());
      if (Snapshot.GetHandle() == INVALID_HANDLE_VALUE) {
         throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
      } else {
         try {
            ThreadEntry.dwSize = sizeof(THREADENTRY32);
            for (bContinue = Thread32First(Snapshot.GetHandle(), &ThreadEntry); 
                 bContinue; 
                 bContinue = Thread32Next(Snapshot.GetHandle(), &ThreadEntry)) {
               if (ThreadEntry.th32OwnerProcessID == m_dwProcessId) {
                  CurrentThread = Threads.find(ThreadEntry.th32ThreadID);
                  if (CurrentThread == Threads.end()) {
                     // If we get a new thread id here, then someone else
                     // created a new thread for this process (CreateRemoteThread())
                     CTrace::Print(CTrace::Layer::TEST, L"Unexpected new thread %ld", ThreadEntry.th32ThreadID);
                     throw CCodineException(L"", __FILE__, __LINE__);
                  } else {
                     CurrentThread->second = FALSE;
                  }
               }
            }
         } catch (CCodineException& ex) {
            // If we come here then we will rollback previous activities
            // Therefore we have to resume all threads contained in Threads            
            for (CurrentThread = Threads.begin();
                 CurrentThread != Threads.end();
                 CurrentThread++) {
               try {
                  if (m_pUser) {
                     CThread NewThread(CurrentThread->first, *m_pUser);
          
                     NewThread.Resume();
                  } else {
                     CThread NewThread(CurrentThread->first);
          
                     NewThread.Resume();
                  }
               } catch (CCodineException&) {
                  ;
               }
            }
            throw ex;
         }
      }
   }

   // C

   // Between B and C we set pair.second to false for all threads 
   // which belong to the current process
   // Now we have to resume the remaining threads
   // We will also add the suspended threads of this process to
   // the member m_Threads. 
   try {
      // Resume and copy
      for (CurrentThread = Threads.begin();
           CurrentThread != Threads.end();
           CurrentThread++) {
         if (CurrentThread->second == TRUE) {
            if (m_pUser) {
               CThread ForeignThread(CurrentThread->first, *m_pUser);
          
               ForeignThread.Resume();
            } else {
               CThread ForeignThread(CurrentThread->first);
          
               ForeignThread.Resume();
            }
         } 
      }
   } catch (CCodineException& ex) {
      
      // What can we do here?

      throw ex;
   }
}

void CProcess::Terminate(DWORD dwExitCode) {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CProcess::Terminate()");
   BOOL bRet;

   bRet = TerminateProcess(GetHandle(), dwExitCode);
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

} // namespace
