#if !defined(AFX_THREAD_H__8720992B_F266_4202_80A6_5DE151905C18__INCLUDED_)
#define AFX_THREAD_H__8720992B_F266_4202_80A6_5DE151905C18__INCLUDED_
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
#include "CUser.h"

namespace GridEngine {

class CThread : public CKernelObject {
 private:
   static DWORD WINAPI RealMain(LPVOID lpParameter);

 private:
   DWORD m_dwThreadId;
   CUser *m_pUser;
   PVOID m_pvThreadParameter;

 private:
   virtual void CreateNewThread();
   virtual void OpenExistingThread();
   virtual DWORD ThreadMain(PVOID lpParameter);

 public:
   CThread();
   CThread(const CUser& User);
   CThread(PVOID lpThreadParameter);

   CThread(DWORD dwThreadId);
   CThread(DWORD dwThreadId, const CUser& User);

   virtual ~CThread();

   virtual DWORD GetThreadId() const;
   virtual DWORD GetExitCode() const;
   virtual void Resume();
   virtual void Start();
   virtual void Suspend();
   virtual void Terminate(DWORD dwExitCode);
   virtual void WaitForObject(DWORD dwMilliseconds = INFINITE) const;
};

} // namespace

#endif // !defined(AFX_THREAD_H__8720992B_F266_4202_80A6_5DE151905C18__INCLUDED_)
