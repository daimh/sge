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

#include <iostream>

#include "CTimer.h"
#include "CError.h"
#include "CException.h"
#include "CTrace.h"

using namespace std;

namespace GridEngine {

CTimer::CTimerThread::CTimerThread() : 
   CThread() 
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Timer::TimerThread::TimerThread()");
}

CTimer::CTimerThread::CTimerThread(PVOID lpParameter) : 
   CThread(lpParameter) 
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CTimer::CTimerThread::TimerThread(PVOID)");
}

DWORD CTimer::CTimerThread::ThreadMain(PVOID lpParameter) {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CTimer::CTimerThread::ThreadMain(PVOID)");
   CTimer *TheTimer = (CTimer*)lpParameter;
   DWORD dwRet;
   BOOL bExit = false;

   while (!bExit) {
      dwRet = WaitForSingleObject(TheTimer->GetHandle(), INFINITE);
      if (dwRet == WAIT_OBJECT_0) {
         dwRet = TheTimer->TimerMain(TheTimer->m_pvTimerParameter);
         if (dwRet) {
            bExit = true;
         }
      } 
   }

   return 0;
}

CTimer::CTimer(DWORD dwTime) : 
   m_dwTime(dwTime),
   m_pTimerThread(NULL),
   m_pvTimerParameter(0) 
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CTimer::CTimer(DWORD)");
   m_pTimerThread = new CTimerThread(this);
   CreateTimer();
   m_pTimerThread->Start();
}

CTimer::CTimer(DWORD dwTime, PVOID lpTimerParameter) : 
   m_dwTime(dwTime),
   m_pTimerThread(NULL),
   m_pvTimerParameter(lpTimerParameter)
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CTimer::CTimer(DWORD, PVOID)");
   m_pTimerThread = new CTimerThread(this);
   CreateTimer();
   m_pTimerThread->Start();
}

CTimer::~CTimer() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CTimer::~CTimer()");
   if (m_pTimerThread) {
      delete m_pTimerThread;
      m_pTimerThread = NULL;
   }
}

void CTimer::CreateTimer() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CTimer::CreateTimer()");
   BOOL bRet;
   HANDLE hTimer;
   LARGE_INTEGER LargeInt;

   hTimer = CreateWaitableTimer(0, false, NULL);
   if (hTimer == NULL) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } else {
      SetHandle(hTimer);
   }
   LargeInt.QuadPart = -1;
   bRet = SetWaitableTimer(GetHandle(), &LargeInt, m_dwTime, NULL, this, false);
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

DWORD CTimer::TimerMain(PVOID lpParameter) {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CTimer::TimerMain(PVOID)");
   return 0;
}

void CTimer::WaitForObject(DWORD dwMilliseconds) const {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CTimer::WaitForObject");
   m_pTimerThread->WaitForObject();
}

} // namespace
