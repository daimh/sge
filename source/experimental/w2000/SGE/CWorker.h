#if !defined(AFX_WORKER_H__BA7839C7_2625_4FCB_99B4_493BAD163C93__INCLUDED_)
#define AFX_WORKER_H__BA7839C7_2625_4FCB_99B4_493BAD163C93__INCLUDED_
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

#include <list>

#include "CThread.h"
#include "CTimer.h"
#include "CMutex.h"

using namespace std;

namespace GridEngine {

class CWorker {
 private:
   class CCpuThread : public CThread {
    public:
      CCpuThread(PVOID lpParameter);
      virtual DWORD ThreadMain(PVOID lpParameter);
   };

   class CIoThread : public CThread {
    public:
      CIoThread(PVOID lpParameter);
      virtual DWORD ThreadMain(PVOID lpParameter);
   };

   class CMemoryThread : public CThread, CTimer {
    public:
      CMemoryThread(PVOID lpParameter);
      virtual DWORD ThreadMain(PVOID lpParameter);
      virtual DWORD TimerMain(PVOID lpParameter);
   };

   class CSleepThread : public CThread {
    public:
      CSleepThread(PVOID lpParameter);
      virtual DWORD ThreadMain(PVOID lpParameter);
   };

   class CThreadWormThread : public CThread {
    public:
      CThreadWormThread(PVOID lpParameter);
      virtual DWORD ThreadMain(PVOID lpParameter);
   };

   friend class MemoryThread;
   friend class CpuThread;
   friend class IoThread;
   friend class SleepThread;
   friend class ThreadWormThread;

 private:
   DWORD m_dwMemory;
   DWORD m_dwMemoryTime;

 public:
   CMutex m_Mutex;
   DWORD m_dwTime;
   DWORD m_dwDelay;
   list<CThread*> m_ThreadList;
   BOOL m_bExit;

 public:
	CWorker(DWORD dwTime);
	virtual ~CWorker();

   virtual void AddCpuThread();
   virtual void AddIoThread();
   virtual void AddMemoryThread();
   virtual void AddSleeperThread();
   virtual void AddThreadWormThread();

   virtual DWORD GetMemory();
   virtual DWORD GetMemoryTime();

   virtual void SetMemory(DWORD dwMemory);
   virtual void SetMemoryTime(DWORD dwTime);

   virtual void WaitForObject();
};

} // namescape

#endif // !defined(AFX_WORKER_H__BA7839C7_2625_4FCB_99B4_493BAD163C93__INCLUDED_)
