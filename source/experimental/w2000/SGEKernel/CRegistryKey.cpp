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
#include <vector>

#include "CBasicTypes.h"
#include "CRegistryKey.h"
#include "CError.h"
#include "CException.h"

using std::vector;

namespace GridEngine {

CRegistryKey::CRegistryKey(HKEY hKey, CString SubKey, BOOL Create) :
   m_hSubKey(NULL),
   m_dwMultiString()
{
   LONG lError;
   DWORD dwDisposition;
      
   if (Create) {
      lError = ::RegCreateKeyEx(hKey, SubKey.c_str(), 0, NULL, 
         REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
         NULL, &m_hSubKey, &dwDisposition);
   } else {
      lError = ::RegOpenKeyEx(hKey, SubKey.c_str(), 0, KEY_ALL_ACCESS, &m_hSubKey); 
   }
   if (lError != ERROR_SUCCESS) {
      throw CCodineException(CError::GetErrorMessage(lError), __FILE__, __LINE__);
   }
}

CRegistryKey::~CRegistryKey() {
   if (m_hSubKey != NULL) { 
      ::RegCloseKey(m_hSubKey); 
   }
}

DWORD CRegistryKey::GetDWORD(CString Name) {
   DWORD dwValue = 0;
   DWORD dwSize;

   dwSize = sizeof(DWORD);
   GetValue(Name, (PBYTE)&dwValue, &dwSize);
   return dwValue;
}

CString CRegistryKey::GetString(CString Name) {
   const DWORD dwMaxBufferSize = 4096 * 2;
   DWORD dwSize = dwMaxBufferSize;
   BYTE Buffer[dwMaxBufferSize];

   GetValue(Name, (PBYTE)Buffer, &dwSize);
   return ((PWSTR) Buffer);
}

vector<CString*>& CRegistryKey::GetMultiString(CString Name) {
   static const DWORD dwMaxBufferSize = 4096 * 2;
   vector<CString*>::iterator Element;
   DWORD dwSize = dwMaxBufferSize;
   BYTE Buffer[dwMaxBufferSize];
   PWSTR pszBuffer;

   // delete old elements
   for (Element = m_dwMultiString.begin(); Element != m_dwMultiString.end(); Element++) {
      delete (*Element);
   }
   m_dwMultiString.clear();
   
   // get registry information
   GetValue(Name, Buffer, &dwSize);

   // create new elements
   pszBuffer = (PWSTR) Buffer;
   while (lstrcmp(pszBuffer, L"")) {
      m_dwMultiString.push_back(new CString(pszBuffer));
      pszBuffer += lstrlen(pszBuffer) + 1;
   }
   return m_dwMultiString;
}

void CRegistryKey::GetBinary(CString Name, PBYTE pbData, PDWORD pdwSize) {
   GetValue(Name, pbData, pdwSize);
}

void CRegistryKey::GetValue(CString Name, PBYTE pbData, PDWORD pdwSize) {
   LONG lError;

   lError = ::RegQueryValueEx(m_hSubKey, Name.c_str(), NULL, NULL, pbData, pdwSize);
   if (lError != ERROR_SUCCESS) {
      throw CCodineException(CError::GetErrorMessage(lError), __FILE__, __LINE__);
   }
}

void CRegistryKey::SetDWORD(CString Name, DWORD Value) {
   SetValue(Name, REG_DWORD, (PBYTE) &Value, sizeof(DWORD));
}

void CRegistryKey::SetString(CString Name, CString Value) {
   SetValue(Name, REG_SZ, (PBYTE)Value.c_str(), sizeof(WCHAR) * (Value.size() + 1));
}

void CRegistryKey::SetExpandString(CString Name, CString Value) {
   SetValue(Name, REG_EXPAND_SZ, (PBYTE)Value.c_str(), sizeof(WCHAR) * (Value.size() + 1));
}

void CRegistryKey::SetMultiString(CString Name, vector<CString*>& Value) {
   vector<CString*>::iterator Element;
   static const DWORD dwMaxBufferSize = 4096 * 2;
   DWORD dwSize = dwMaxBufferSize;
   BYTE Buffer[dwMaxBufferSize];
   PBYTE pBuffer;

   dwSize = 0;
   memset(Buffer, 0, dwMaxBufferSize);
   pBuffer = Buffer;
   for (Element = Value.begin(); Element != Value.end(); Element++) {
      dwSize += sizeof(WCHAR) * (*Element)->size();
      if (dwSize <= dwMaxBufferSize) {
         lstrcpy((PWSTR)pBuffer, (*Element)->c_str());
         pBuffer = Buffer + dwSize;
      } else {
         throw CCodineException(L"Buffer overflow", __FILE__, __LINE__);
      }
   }
   if (++dwSize > dwMaxBufferSize) {
      throw CCodineException(L"Buffer overflow", __FILE__, __LINE__);
   }
   SetValue(Name, REG_MULTI_SZ, Buffer, dwSize);
}

void CRegistryKey::SetBinary(CString Name, PBYTE Value, DWORD dwSize) {
   SetValue(Name, REG_BINARY, Value, dwSize);
}

void CRegistryKey::SetValue(CString Name, DWORD dwType, CONST BYTE *pbData, DWORD dwSize) {
   LONG lError;

   lError = ::RegSetValueEx(m_hSubKey, Name.c_str(), 0, dwType, pbData, dwSize);
   if (lError != ERROR_SUCCESS) {
      throw CCodineException(CError::GetErrorMessage(lError), __FILE__, __LINE__);
   }
}

} // namespace
