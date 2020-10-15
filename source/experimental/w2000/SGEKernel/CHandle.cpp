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

#include "CHandle.h"
#include "CException.h"
#include "CError.h"

namespace GridEngine {

CHandle::CHandle() :
   m_hHandle(INVALID_HANDLE_VALUE)
{
}

CHandle::CHandle(const HANDLE Handle) :
   m_hHandle(Handle)
{
}

CHandle::~CHandle() {
   CloseTheHandle();
}

void CHandle::CloseTheHandle() {
   if (m_hHandle != INVALID_HANDLE_VALUE) {
      BOOL bRet;

      bRet = ::CloseHandle(m_hHandle);
      if (bRet == 0) {
         throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
      }
      m_hHandle = INVALID_HANDLE_VALUE;
   }
}

BOOL CHandle::GetCloseFlag() const {
   return GetFlags() & HANDLE_FLAG_PROTECT_FROM_CLOSE;
}

DWORD CHandle::GetFlags() const {
   DWORD dwFlags;
   BOOL bRet;

   bRet = GetHandleInformation(m_hHandle, &dwFlags);
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
   return dwFlags;
}

HANDLE CHandle::GetHandle() const {
   return m_hHandle;
}

BOOL CHandle::GetInheritFlag() const {
   return GetFlags() & HANDLE_FLAG_INHERIT;
}

void CHandle::SetCloseFlag(BOOL DontCloseHandle) {
   SetFlags(HANDLE_FLAG_PROTECT_FROM_CLOSE, DontCloseHandle ? ~0 : 0);
}

void CHandle::SetFlags(DWORD dwMask, DWORD dwFlags) {
   BOOL bRet;

   bRet = SetHandleInformation(m_hHandle, dwMask, dwFlags);
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

void CHandle::SetHandle(const HANDLE Handle) {
   CloseTheHandle();
   m_hHandle = Handle;
}

void CHandle::SetInheritFlag(BOOL bInheritTheHandle) {
   SetFlags(HANDLE_FLAG_INHERIT, bInheritTheHandle ? ~0 : 0 );
}

void CHandle::WaitForObject(DWORD dwMilliseconds) const {
   DWORD dwRet;

   dwRet = WaitForSingleObject(m_hHandle, dwMilliseconds);
   if (dwRet != WAIT_OBJECT_0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } 
}

} // namespace
