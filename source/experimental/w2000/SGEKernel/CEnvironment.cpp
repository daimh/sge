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
#include <Userenv.h>

#include "CEnvironment.h"
#include "CError.h"
#include "CException.h"
#include "CRegistryKey.h"
#include "CUser.h"

namespace GridEngine {

CEnvironment::CEnvironment() :
   m_pEnvironmentBlock(NULL),
   m_User(),
   m_AdditionalEnvironmentEntries()
{
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Environment::Environment()");
   BOOL bRet;

   memset(&m_ProfileInfo, 0, sizeof(PROFILEINFO));
   LoadUserProfile();

   bRet = CreateEnvironmentBlock(&m_pEnvironmentBlock, m_User.GetHandle(), false);
   if (bRet == NULL) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}


CEnvironment::CEnvironment(const CUser& User) :
   m_pEnvironmentBlock(NULL),
   m_User(User),
   m_AdditionalEnvironmentEntries()
{
   BOOL bRet;

   memset(&m_ProfileInfo, 0, sizeof(PROFILEINFO));
   LoadUserProfile();
      
   bRet = CreateEnvironmentBlock(&m_pEnvironmentBlock, m_User.GetHandle(), false);
   if (bRet == NULL) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

CEnvironment::~CEnvironment() {
   BOOL bRet;

   DestroyPrivateEnvironment();

   if (m_pEnvironmentBlock != NULL) {
      bRet = DestroyEnvironmentBlock(m_pEnvironmentBlock);
      if (bRet == NULL) {
         throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
      }
   }

   UnloadUserProfile();
}

void CEnvironment::DestroyPrivateEnvironment() {
   map<CString*, CString*>::iterator PrivateEnvEntry;
 
   for (PrivateEnvEntry = m_AdditionalEnvironmentEntries.begin();
        PrivateEnvEntry != m_AdditionalEnvironmentEntries.end();
        PrivateEnvEntry++) {
      delete PrivateEnvEntry->first;
      delete PrivateEnvEntry->second;
   }
   m_AdditionalEnvironmentEntries.clear();
}

LPVOID CEnvironment::GetEnvironmentBlock() {
   return m_pEnvironmentBlock;
}

void CEnvironment::LoadUserProfile() {
   BOOL bRet;

   UnloadUserProfile();

   memset(&m_ProfileInfo, 0, sizeof(PROFILEINFO));
   m_ProfileInfo.dwSize = sizeof(PROFILEINFO);
   m_ProfileInfo.dwFlags = PI_NOUI;
   m_ProfileInfo.lpUserName = const_cast<PWSTR>(m_User.GetUsername().c_str());

   bRet = ::LoadUserProfile(m_User.GetHandle(), &m_ProfileInfo);
   if (bRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

void CEnvironment::UnloadUserProfile() {
   BOOL bRet;

   if (m_ProfileInfo.hProfile) {
      bRet = ::UnloadUserProfile(m_User.GetHandle(), m_ProfileInfo.hProfile);
      if (bRet == 0) {
         throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
      }
   }
}

#if 0

void CEnvironment::SetRawValue(CString Variable, CString Value) {
   CString *NewVariable = new CString(Variable);
   CString *NewValue = new CString(Value);

   m_AdditionalEnvironmentEntries[NewVariable] = NewValue;
}

void CEnvironment::Print() {
   PWSTR pEnvironmentEntry = (PWSTR)m_pEnvironmentBlock;
   map<CString*, CString*>::iterator PrivateEnvEntry;

   wcout << L"User environment:" << endl;
   while (lstrcmp(pEnvironmentEntry, L"")) {
      wcout << (PWSTR) pEnvironmentEntry << endl;
      pEnvironmentEntry += lstrlen(pEnvironmentEntry) + 1;
   }

   wcout << L"Private environment:" << endl;
   for (PrivateEnvEntry = m_AdditionalEnvironmentEntries.begin();
        PrivateEnvEntry != m_AdditionalEnvironmentEntries.end();
        PrivateEnvEntry++) {
      wcout << *(PrivateEnvEntry->first) << L"=" << *(PrivateEnvEntry->second) << endl;
   }
}

void CEnvironment::CommitChanges() {
   map<CString*, CString*>::iterator PrivateEnvEntry;
   CRegistryKey Key((HKEY)m_ProfileInfo.hProfile, L"Environment");
   BOOL bRet;

   // add/overwrite registry entries
   for (PrivateEnvEntry = m_AdditionalEnvironmentEntries.begin();
        PrivateEnvEntry != m_AdditionalEnvironmentEntries.end();
        PrivateEnvEntry++) {
      if (StrStr(const_cast<PWSTR>((PrivateEnvEntry->second)->c_str()), L"%")) {
         Key.SetExpandString(*(PrivateEnvEntry->first), *(PrivateEnvEntry->second));
      } else {
         Key.SetString(*(PrivateEnvEntry->first), *(PrivateEnvEntry->second));
      }
   }

   // we don't need the private environment anymore
   DestroyPrivateEnvironment();

   // read the expanded environment variables again
   if (m_pEnvironmentBlock != NULL) {
      bRet = DestroyEnvironmentBlock(m_pEnvironmentBlock);
      if (bRet == NULL) {
         throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
      }
   }
   bRet = CreateEnvironmentBlock(&m_pEnvironmentBlock, m_User.GetHandle(), false);
   if (bRet == NULL) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

#endif

CString CEnvironment::GetCurrentValue(CString Variable) {
   const DWORD dwMaxSize = 4096;
   DWORD dwSize = dwMaxSize;
   WCHAR szBuffer[dwMaxSize];

   dwSize = GetEnvironmentVariable(const_cast<PWSTR>(Variable.c_str()), szBuffer, dwSize);
   if (dwSize == 0 || dwSize > dwMaxSize) {
      throw CCodineException(L"Buffer to small", __FILE__, __LINE__);
   }
   return szBuffer;
}

void CEnvironment::SetCurrentValue(CString Variable, CString Value) {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Environment::SetCurrentValue(String, String)");
   const DWORD dwMaxSize = 4096 * 2;
   DWORD dwSize = dwMaxSize;
   WCHAR szBuffer[dwMaxSize];
   DWORD dwRet;

   dwSize = ExpandEnvironmentStrings(const_cast<PWSTR>(Value.c_str()), szBuffer, dwMaxSize);
   if (dwSize == 0 || dwSize > dwMaxSize) {
      throw CCodineException(L"Buffer to small", __FILE__, __LINE__);
   }
   dwRet = SetEnvironmentVariable(const_cast<PWSTR>(Variable.c_str()), szBuffer);
   if (dwRet == 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

CString& CEnvironment::GetExpandedEnvironmentString(CString& String) {
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"Environment::ExpandEnvironmentString(String)");
   const DWORD dwMaxSize = 4096 * 2;
   DWORD dwSize = dwMaxSize;
   WCHAR pszBuffer[dwMaxSize];

   dwSize = ExpandEnvironmentStrings(String.c_str(), pszBuffer, dwMaxSize);
   if (dwSize == 0 || dwSize > dwMaxSize) {
      throw CCodineException(L"Buffer to small", __FILE__, __LINE__);
   }
   String = pszBuffer;

   return String;
}


} // namespace
