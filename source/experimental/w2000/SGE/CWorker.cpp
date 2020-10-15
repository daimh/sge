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
#include <list>

#include "CWorker.h"
#include "CMutex.h"
#include "CThread.h"
#include "CException.h"
#include "CTrace.h"

using namespace std;

namespace GridEngine {

// CPU Thread

CWorker::CCpuThread::CCpuThread(PVOID lpParameter) : CThread(lpParameter) {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::CpuThread::CpuThread(PVOID)");
   Start();
}

DWORD CWorker::CCpuThread::ThreadMain (PVOID lpParameter) {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::CpuThread::ThreadMain(PVOID)");
   CWorker *TheWorker = (CWorker*)lpParameter;
//   HANDLE hMutex;

   while (!TheWorker->m_bExit) {
      double d1 = 1.0;
      double d2 = 2.0;
      double d3 = 3.0;

      d3 *= d1 + d2;
      d3 *= d1 + d2;
      

//      hMutex = CreateMutex(0, false, L"TestMutex");
//      CloseHandle(hMutex);
   }
   return 0;
}

// IO Thread

CWorker::CIoThread::CIoThread(PVOID lpParameter) : CThread(lpParameter) {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::IoThread::IoThread(PVOID)");
   Start();
}

DWORD CWorker::CIoThread::ThreadMain (PVOID lpParameter) {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::IoThread::ThreadMain(PVOID)");
   CWorker *TheWorker = (CWorker*)lpParameter;

   while (!TheWorker->m_bExit) {
      wcout << L"IO ";
   }
   return 0;
}

// Memory Thread

CWorker::CMemoryThread::CMemoryThread(PVOID lpParameter) : 
   CThread(lpParameter), CTimer(((CWorker*)lpParameter)->GetMemoryTime(), (PVOID) lpParameter)
{
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::MemoryThread::MemoryThread(PVOID)");
   Start();
}

DWORD CWorker::CMemoryThread::ThreadMain (PVOID lpParameter) {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::MemoryThread::ThreadMain(PVOID)");
   CWorker *TheWorker = (CWorker*)lpParameter;

   while (!TheWorker->m_bExit) {
      Sleep(500);
   }
   return 0;
}

DWORD CWorker::CMemoryThread::TimerMain(PVOID lpParameter) {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::MemoryThread::TimerMain(PVOID)");
   CWorker *TheWorker = (CWorker*)lpParameter;

   if (!TheWorker->m_bExit) {
      char *leak = new char[TheWorker->GetMemory()];
      
      if (!leak)
      wcout << L"Error in CWorker::CMemoryThread::TimerMain: new failed" << endl;
      return 0;
   } 
   return 1;
}

// Sleeper Thread

CWorker::CSleepThread::CSleepThread(PVOID lpParameter) : CThread(lpParameter) {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::SleepThread::SleepThread(PVOID)");
   Start();
}

DWORD CWorker::CSleepThread::ThreadMain (PVOID lpParameter) {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::SleepThread::ThreadMain(PVOID)");
   CWorker *TheWorker = (CWorker*)lpParameter;

   Sleep(TheWorker->m_dwTime);
   TheWorker->m_bExit = true; 
   return 0;
}

// ThreadWorm Thread

CWorker::CThreadWormThread::CThreadWormThread(PVOID lpParameter) : CThread(lpParameter) {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::ThreadWormThread::ThreadWormThread(PVOID)");
   Start();
}

DWORD CWorker::CThreadWormThread::ThreadMain (PVOID lpParameter) {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::ThreadWormThread::ThreadMain(PVOID)");
   static const DWORD dwLifetime = 2000;
   CWorker *TheWorker = (CWorker*)lpParameter;

   if (!TheWorker->m_bExit) {
      Sleep(TheWorker->m_dwDelay);
      CThreadWormThread NewThread(lpParameter);
      NewThread.Resume();
      Sleep(dwLifetime);
   }
   return 0;
}

// Worker

CWorker::CWorker(DWORD dwTime) {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::Worker(DWORD)");
   m_bExit = false;
   m_dwTime = dwTime;
   m_dwDelay = 100;
   m_dwMemory = 0;
   m_dwMemoryTime = 0;
}

CWorker::~CWorker() {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::~Worker()");
   m_bExit = true;
}

void CWorker::AddCpuThread() {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::AddCpuThread()");
   CWorker::CCpuThread *TheCpuThread = new CWorker::CCpuThread(this);
   m_ThreadList.push_back(TheCpuThread);
}

void CWorker::AddSleeperThread() {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::AddSleeperThread()");
   CWorker::CSleepThread *TheSleepThread = new CWorker::CSleepThread(this);
   m_ThreadList.push_back(TheSleepThread);
}

void CWorker::AddMemoryThread() {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::AddMemoryThread()");
   CWorker::CMemoryThread *TheMemoryThread = new CWorker::CMemoryThread(this);
   m_ThreadList.push_back(TheMemoryThread);
}

void CWorker::AddIoThread() {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::AddIoThread()");
   CWorker::CIoThread *TheIoThread = new CWorker::CIoThread(this);
   m_ThreadList.push_back(TheIoThread);
}

void CWorker::AddThreadWormThread() {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::AddThreadWormThread()");
   CWorker::CThreadWormThread *TheThreadWormThread = new CWorker::CThreadWormThread(this);
   m_ThreadList.push_back(TheThreadWormThread);
}

DWORD CWorker::GetMemory() {
   return m_dwMemory;
}

DWORD CWorker::GetMemoryTime() {
   return m_dwMemoryTime;
}

void CWorker::SetMemory(DWORD dwMemory) {
   m_dwMemory = dwMemory;
}

void CWorker::SetMemoryTime(DWORD dwTime) {
   m_dwMemoryTime = dwTime;
}

void CWorker::WaitForObject() {
   CTrace::CEnter Enter(CTrace::Layer::CODINE, L"Worker::WaitForObject()");

   for (list<CThread*>::iterator it = m_ThreadList.begin(); it != m_ThreadList.end(); it++) {
      CThread *TheThread;
   
      TheThread = *it;
      TheThread->WaitForObject();
   }
}

} // namespace
