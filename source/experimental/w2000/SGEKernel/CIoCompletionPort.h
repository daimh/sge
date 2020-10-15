#if !defined(AFX_IOCOMPLETIONPORT_H__C1375CF7_4776_4BF5_AAFF_F327EB2515E6__INCLUDED_)
#define AFX_IOCOMPLETIONPORT_H__C1375CF7_4776_4BF5_AAFF_F327EB2515E6__INCLUDED_
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

class CIoCompletionPort : public CKernelObject {
 private:
   class CStatusThread : public CThread {
    public:
      CStatusThread(PVOID lpParameter);
      virtual DWORD ThreadMain(PVOID lpParameter);
   };

   class CStatus {
    public:
      DWORD dwNumberOfBytes;
      ULONG nCompletionKey;
      LPOVERLAPPED lpOverlapped;
   };
   friend class CStatusThread;

 private:
   BOOL m_bExit;
   DWORD m_dwTimeout;
   CStatusThread *m_pStatusThread;
   PVOID m_pvParameter;

 public:
	CIoCompletionPort(PVOID lpParameter, DWORD dwCompletionKey, DWORD dwCuncurrentThreads, DWORD dwTimeout);
	virtual ~CIoCompletionPort();

   virtual DWORD IoCompletionPortMain(PVOID lpParameter, CIoCompletionPort::CStatus Status) = 0;

   virtual void WaitForObject(DWORD dwMilliseconds = INFINITE) const;
};

} // namespace

#endif // !defined(AFX_IOCOMPLETIONPORT_H__C1375CF7_4776_4BF5_AAFF_F327EB2515E6__INCLUDED_)
