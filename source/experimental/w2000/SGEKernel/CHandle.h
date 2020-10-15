#if !defined(AFX_HANDLE_H__DE4B47E7_21DD_4850_8589_36D4868DAA49__INCLUDED_)
#define AFX_HANDLE_H__DE4B47E7_21DD_4850_8589_36D4868DAA49__INCLUDED_
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

namespace GridEngine {

class CHandle {
 private:
   // Windows object handle
   HANDLE m_hHandle;

   // Close current handle
   virtual void CloseTheHandle();
 public:
   // Default constructor
	CHandle();

   // Cast operator (HANDLE -> CHandle)
   CHandle(const HANDLE hHandle);

   // Destructor
	virtual ~CHandle();

   // Get the handle stored in the object
   virtual BOOL GetCloseFlag() const;
   virtual DWORD GetFlags() const;
   virtual HANDLE GetHandle() const;
   virtual BOOL GetInheritFlag() const;

   // Set the handle stored in the object
   virtual void SetCloseFlag(BOOL bDontCloseHandle);
   virtual void SetFlags(DWORD dwMask, DWORD dwFlags);
   virtual void SetHandle(const HANDLE hHandle);
   virtual void SetInheritFlag(BOOL bInheritTheHandle);

   // Wait until handle is signalled
   virtual void WaitForObject(DWORD dwMilliseconds = INFINITE) const;
};

} // namespace

#endif // !defined(AFX_HANDLE_H__DE4B47E7_21DD_4850_8589_36D4868DAA49__INCLUDED_)
