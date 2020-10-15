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

#include "CIoCompletionPort.h"
#include "CError.h"
#include "CException.h"

using namespace std;

namespace GridEngine {

CIoCompletionPort::CStatusThread::CStatusThread(PVOID lpParameter) :
   CThread(lpParameter)
{
}

DWORD CIoCompletionPort::CStatusThread::ThreadMain(PVOID pvParameter) {
   CIoCompletionPort *pCompletionPort = (CIoCompletionPort*)pvParameter;
   CStatus TheStatus;

   while (!pCompletionPort->m_bExit) {
      DWORD dwRet;

      dwRet = GetQueuedCompletionStatus(
         pCompletionPort->GetHandle(),     
         &TheStatus.dwNumberOfBytes,     
         &TheStatus.nCompletionKey, 
         &TheStatus.lpOverlapped,
         pCompletionPort->m_dwTimeout
      );
      if (dwRet != 0) {
         dwRet = pCompletionPort->IoCompletionPortMain(pCompletionPort->m_pvParameter, TheStatus);
         if (dwRet > 0)
            return 0;
      } 
   }
   return 0;
}

CIoCompletionPort::CIoCompletionPort(PVOID pvParameter, DWORD dwCompletionKey, DWORD dwCuncurrentThreads, DWORD dwTimeout) :
   m_bExit(false),
   m_dwTimeout(dwTimeout),
   m_pvParameter(pvParameter)
{
   HANDLE hPort;

   m_pStatusThread = new CStatusThread(this);

   hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, dwCompletionKey, 
      dwCuncurrentThreads);
   if (hPort == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
   SetHandle(hPort);

   m_pStatusThread->Start();
}

CIoCompletionPort::~CIoCompletionPort() {
}

DWORD CIoCompletionPort::IoCompletionPortMain(PVOID lpParameter, CIoCompletionPort::CStatus Status) {
   return 0;
}

void CIoCompletionPort::WaitForObject(DWORD dwMilliseconds) const{
   m_pStatusThread->WaitForObject();
};


} // namespace
