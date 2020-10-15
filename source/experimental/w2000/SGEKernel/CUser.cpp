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

#include "CBasicTypes.h"
#include "CUser.h"
#include "CError.h"
#include "CException.h"
#include "CHandle.h"
#include "CProcess.h"
#include "CTrace.h"

using namespace std;

namespace GridEngine {

BOOL CUser::m_bTestOk = FALSE;

CUser::CUser() :
   CKernelObject(),
   m_Password(),
   m_Username() 
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CUser::CUser()");
   LogonCurrentUser();
}

CUser::CUser(const CString& Username, const CString& Password) : 
   CKernelObject(),
   m_Password(Password),
   m_Username(Username)
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CUser::CUser(const CString&, const CString&)");
   LogonNewUser();
}

CUser::CUser(const CUser& TheUser) :
   CKernelObject(TheUser),
   m_Password(TheUser.m_Password),
   m_Username(TheUser.m_Username)
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CUser::CUser(const CUser&)");
}

CUser::~CUser() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"CUser::~CUser");
}

CString CUser::GetUsername() {
   return m_Username;
}

void CUser::Impersonate() {
   CTrace::CEnter CEnter(CTrace::Layer::KERNEL, L"User::Impersonate()");
   BOOL bRet;

   bRet = ImpersonateLoggedOnUser(GetHandle());
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

void CUser::LogonCurrentUser(void) {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"User::LogonCurrentUser()");
   static const DWORD dwMaxUsername = 4096;
   CProcess ThisProcess;
   DWORD dwLengthUsername = dwMaxUsername;
   WCHAR szUsername[dwMaxUsername];
   BOOL bRet;
   HANDLE hToken;

   bRet = OpenProcessToken(ThisProcess.GetHandle(), TOKEN_ALL_ACCESS, &hToken);
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } else {
      SetHandle(hToken);
   }
   bRet = ::GetUserName(szUsername, &dwLengthUsername);
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }   
   m_Username = szUsername;
}

void CUser::LogonNewUser(void) {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"User::LogonNewUser()");
   BOOL bRet;
   HANDLE hToken;

   bRet = LogonUser(const_cast<PWSTR>(m_Username.c_str()), NULL, 
      const_cast<PWSTR>(m_Password.c_str()), LOGON32_LOGON_INTERACTIVE,
      LOGON32_PROVIDER_DEFAULT, &hToken);
   if (bRet == 0) {
      wcerr << L"Username: " << m_Username.c_str() << L" Password: " << m_Password.c_str() << endl;
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   } else {
      SetHandle(hToken);
   }
}

void CUser::ReturnToSelf() {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"User::ReturnToSelf");
   BOOL bRet;

   bRet = RevertToSelf();
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

BOOL CUser::TestOk() {
   if (m_bTestOk) {
      return m_bTestOk;
   }

   m_bTestOk = TRUE;

   return m_bTestOk;
}

} // namespace
