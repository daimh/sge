#if !defined(AFX_TIMER_H__8D7C2129_E170_41E3_9A2F_94CEB6A08E01__INCLUDED_)
#define AFX_TIMER_H__8D7C2129_E170_41E3_9A2F_94CEB6A08E01__INCLUDED_
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

#include <windows.h>

#include "CKernelObject.h"
#include "CThread.h"

namespace GridEngine {

class CTimer : public CKernelObject {
 private:
   class CTimerThread : public CThread {
    public:
      CTimerThread();
      CTimerThread(PVOID lpParameter);
      virtual DWORD ThreadMain(PVOID lpParameter);
   };
   friend class CTimerThread;

 private:
   DWORD m_dwTime;
   CTimerThread *m_pTimerThread;
   PVOID m_pvTimerParameter;

   virtual void CreateTimer();

 public:
	CTimer(DWORD dwTime);
	CTimer(DWORD dwTime, PVOID m_lpTimerParameter);
	virtual ~CTimer();

   virtual DWORD TimerMain(PVOID lpParameter);
   virtual void WaitForObject(DWORD dwMilliseconds = INFINITE) const;
};

} // namespace

#endif // !defined(AFX_TIMER_H__8D7C2129_E170_41E3_9A2F_94CEB6A08E01__INCLUDED_)
