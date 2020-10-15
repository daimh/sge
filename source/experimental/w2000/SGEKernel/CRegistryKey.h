#if !defined(AFX_RegistryKey_H__890537C1_C360_4477_BB92_A1A2D4A1122E__INCLUDED_)
#define AFX_RegistryKey_H__890537C1_C360_4477_BB92_A1A2D4A1122E__INCLUDED_
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

#include "stdafx.h"
#include <windows.h>
#include <vector>

#include "CBasicTypes.h"

using std::vector;

namespace GridEngine {

class CRegistryKey {
private:
   HKEY m_hSubKey;
   vector<CString*> m_dwMultiString;

private:
   virtual void GetValue(CString Name, PBYTE pbData, PDWORD pdwSize);
   virtual void SetValue(CString Name, DWORD dwType, CONST BYTE *pbData, DWORD dwSize);

public:
	CRegistryKey(HKEY hKey, CString SubKey, BOOL Create = FALSE);

	virtual ~CRegistryKey();

   virtual DWORD GetDWORD(CString Name);
   virtual CString GetString(CString Name);
   virtual vector<CString*>& GetMultiString(CString Name);
   virtual void GetBinary(CString Name, PBYTE pbData, PDWORD pdwSize);

   virtual void SetDWORD(CString Name, DWORD Value);
   virtual void SetString(CString Name, CString Value);
   virtual void SetExpandString(CString Name, CString Value);
   virtual void SetMultiString(CString Name, vector<CString*>& Value);
   virtual void SetBinary(CString Name, PBYTE Value, DWORD dwSize);
};

} // namespace

#endif // !defined(AFX_RegistryKey_H__890537C1_C360_4477_BB92_A1A2D4A1122E__INCLUDED_)
