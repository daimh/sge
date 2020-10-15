#if !defined(AFX_PROCESS_H__5155C2FE_C5D3_46A9_ACDA_B16FB7554F0D__INCLUDED_)
#define AFX_PROCESS_H__5155C2FE_C5D3_46A9_ACDA_B16FB7554F0D__INCLUDED_
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
#include <vector>

#include "CBasicTypes.h"
#include "CEnvironment.h"
#include "CKernelObject.h"
#include "CThread.h"
#include "CUser.h"

using std::vector;

namespace GridEngine {

class CProcess : public CKernelObject {
 public:
   enum Type {
      VISIBLE,
      INVISIBLE
   };

 private:
   CString m_Command;
   DWORD m_dwProcessId;
   DWORD m_dwThreadId;
   CUser* m_pUser;

 private:
   virtual void CreateNewProcess(CUser *StartUser, CEnvironment *Environment, Type Type);   
   virtual void OpenExistingProcess();
   virtual HANDLE GetSnapshot();

 public:
   CProcess();
	CProcess(CString Command, Type Type = INVISIBLE);
	CProcess(CString Command, const CUser& StartUser, Type Type = INVISIBLE);
	CProcess(CString Command, const CUser& StartUser, CEnvironment& Environment, Type Type = INVISIBLE);
   CProcess(CString Command, CEnvironment& Environment);

	CProcess(DWORD dwProcessId);
   virtual ~CProcess();

   virtual DWORD GetProcessId() const;
   virtual DWORD GetExitCode() const;
   virtual void Resume();
   virtual void Start();
   virtual void Suspend();
   virtual void Terminate(DWORD dwExitCode);
};

} // namespace

#endif // !defined(AFX_PROCESS_H__5155C2FE_C5D3_46A9_ACDA_B16FB7554F0D__INCLUDED_)
