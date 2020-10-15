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
#include <eh.h>
#include <iostream>

#include "CThread.h"
#include "CError.h"
#include "CException.h"
#include "CSehTranslator.h"
#include "CTrace.h"
#include "CUser.h"

using namespace std;

namespace GridEngine {

DWORD WINAPI CThread::RealMain(LPVOID pvParameter) {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::RealMain(LPVOID)");
   _se_translator_function pOldTranslator;

   pOldTranslator = _set_se_translator(TranslateSeToCe);
   try {
      CThread *ThisThread = (CThread*)pvParameter;

      return ThisThread->ThreadMain(ThisThread->m_pvThreadParameter);
   } catch (CCodineException& ex) {
      wcerr << ex;
   } catch (CException& ex) {
      wcerr << ex;
   } catch (...) {
      wcerr << L"Caught unknown exception in Thread::RealMain(LPVOID)" << endl;
   }
   _set_se_translator(pOldTranslator);
   return 1;
}

CThread::CThread() : 
   m_dwThreadId(0),
   m_pUser(NULL),
   m_pvThreadParameter(NULL)
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::Thread()");
   CreateNewThread();
}

CThread::CThread(const CUser& StartUser) :
   m_dwThreadId(0),
   m_pUser(new CUser(StartUser)),
   m_pvThreadParameter(NULL)
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::Thread()");
   CreateNewThread();
}


CThread::CThread(PVOID lpThreadParameter) : 
   m_dwThreadId(0),
   m_pUser(NULL),
   m_pvThreadParameter(lpThreadParameter)
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::Thread(PVOID)");
   CreateNewThread();
}

CThread::CThread(DWORD dwThreadId) : 
   m_dwThreadId(dwThreadId),
   m_pUser(NULL),
   m_pvThreadParameter(0)
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::Thread(DWORD)");
   OpenExistingThread();
}

CThread::CThread(DWORD dwThreadId, const CUser& User) : 
   m_dwThreadId(dwThreadId),
   m_pUser(new CUser(User)),
   m_pvThreadParameter(0)
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::Thread(DWORD)");
   OpenExistingThread();
}

CThread::~CThread() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::~Thread()");
   if (m_pUser) {
      delete m_pUser;
      m_pUser = NULL;
   }
}

void CThread::CreateNewThread() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::CreateNewThread()");
   HANDLE hThread;

   if (m_pUser) {
      m_pUser->Impersonate();
   }
   hThread = CreateThread(NULL, 0, CThread::RealMain, this, CREATE_SUSPENDED, 
      &m_dwThreadId);
   if (m_pUser) {
      m_pUser->ReturnToSelf();
   }
   if (hThread == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } else {
      SetHandle(hThread);
   }
}

DWORD CThread::GetExitCode() const {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::GetExitCode()");
   DWORD dwExitCode;

   GetExitCodeThread(GetHandle(), &dwExitCode);
   return dwExitCode;
}

DWORD CThread::GetThreadId() const {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::GetThreadId()");
   return m_dwThreadId;
}

void CThread::OpenExistingThread() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::OpenExistingThread()");
   HANDLE hThread;

   if (m_pUser) {
      m_pUser->Impersonate();
   }
   hThread = OpenThread(THREAD_ALL_ACCESS, false, m_dwThreadId);
   if (m_pUser) {
      m_pUser->ReturnToSelf();
   }
   if (hThread == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } else {
      SetHandle(hThread);
   }
}

void CThread::Start() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::Start()");
   Resume();
}

void CThread::Suspend() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::Suspend()");
   DWORD dwRet;

   if (m_pUser) {
      m_pUser->Impersonate();
   }
   dwRet = SuspendThread(GetHandle());
   if (m_pUser) {
      m_pUser->ReturnToSelf();
   }
   if (dwRet == 0xFFFFFFFF) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } 
}

void CThread::Resume() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::Resume()");
   DWORD dwRet;

   if (m_pUser) {
      m_pUser->Impersonate();
   }
   dwRet = ResumeThread(GetHandle());
   if (m_pUser) {
      m_pUser->ReturnToSelf();
   }
   if (dwRet == 0xFFFFFFFF) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } 
}

void CThread::Terminate(DWORD dwExitCode) {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::Terminate(DWORD)");
   BOOL bRet;

   if (m_pUser) {
      m_pUser->Impersonate();
   }
   bRet = TerminateThread(GetHandle(), dwExitCode);
   if (m_pUser) {
      m_pUser->ReturnToSelf();
   }
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } 
}

DWORD CThread::ThreadMain(PVOID lpParameter) {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::ThreadMain(PVOID)");
   return 0;
}

void CThread::WaitForObject(DWORD dwMilliseconds) const {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Thread::WaitForThread()");
   DWORD dwRet;

   dwRet = WaitForSingleObject(GetHandle(), INFINITE);
   if (dwRet != WAIT_OBJECT_0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } 
}

} // namespace
