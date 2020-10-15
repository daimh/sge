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

#include "CMutex.h"
#include "CError.h"
#include "CException.h"
#include "CTrace.h"

using namespace std;

namespace GridEngine {

CMutex::CEnter::CEnter(CMutex& Mutex) : 
   m_Mutex(Mutex) 
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CMutex::CEnter::CEnter(Mutex&)");
   m_Mutex.WaitForObject();
}

CMutex::CEnter::~CEnter() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CMutex::CEnter::~CEnter()");
   m_Mutex.ReleaseMutex();
}

CMutex::CMutex() {
   CTrace::CEnter CEnter(CTrace::Layer::KERNEL, L"CMutex::CMutex()");
   HANDLE hMutex;

   hMutex = CreateMutex(0, false, NULL);
   if (hMutex == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } else {
      SetHandle(hMutex);
   }
}

CMutex::~CMutex() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CMutex::~Mutex()");
}

void CMutex::ReleaseMutex() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CMutex::ReleaseMutex()");
   int nRet;

   nRet = ::ReleaseMutex(GetHandle());
   if (nRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } 
}

} 
