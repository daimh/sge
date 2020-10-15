#if !defined(AFX_ENVIRONMENT_H__717E8656_B08B_4971_AF60_8AE38C4053C7__INCLUDED_)
#define AFX_ENVIRONMENT_H__717E8656_B08B_4971_AF60_8AE38C4053C7__INCLUDED_
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
#include <Userenv.h>
#include <map>

#include "CUser.h"

using std::map;

namespace GridEngine { 

class CEnvironment {
 private:
   LPVOID m_pEnvironmentBlock;
   PROFILEINFO m_ProfileInfo;
   CUser m_User;
   map<CString*, CString*> m_AdditionalEnvironmentEntries;

 private:
   virtual void DestroyPrivateEnvironment();

 public:
   static CString GetCurrentValue(CString Variable);
   static void SetCurrentValue(CString Variable, CString Value);
   static CString& GetExpandedEnvironmentString(CString& String);
 public:
	CEnvironment();
   CEnvironment(const CUser& User);
	virtual ~CEnvironment();

   virtual LPVOID GetEnvironmentBlock();

#if 0
   virtual void CommitChanges();
   virtual void SetRawValue(CString Variable, CString Value);
   virtual void Print();
#endif

   virtual void LoadUserProfile();
   virtual void UnloadUserProfile();

};

} // namespace

#endif // !defined(AFX_ENVIRONMENT_H__717E8656_B08B_4971_AF60_8AE38C4053C7__INCLUDED_)
