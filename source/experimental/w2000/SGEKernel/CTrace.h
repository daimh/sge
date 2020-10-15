#if !defined(AFX_TRACE_H__3964D5BC_7853_406A_BC59_C0B34C6255AB__INCLUDED_)
#define AFX_TRACE_H__3964D5BC_7853_406A_BC59_C0B34C6255AB__INCLUDED_
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

#include "CBasicTypes.h"

namespace GridEngine {

class CTrace {
 public:
   class CEnter {
    private:
      CString m_String;
      WORD m_wLayer;

    public:
      CEnter(WORD wLayer, const CString& String);
      virtual ~CEnter();
   };
   friend class Enter;

 private:
   static DWORD m_dwTraceId;

 public:
   CTrace();
	CTrace(const CString& Name);
   CTrace(const CString& Name, const CString& Filename);
	virtual ~CTrace();

   static void Print(WORD wLayer, PCWSTR Format, ...);

 public:
	enum Layer {
	   KERNEL = 0x0001,
	   TEST = 0x0002,
	   SGE = 0x0004
	};
};

} // namespace

#endif // !defined(AFX_TRACE_H__3964D5BC_7853_406A_BC59_C0B34C6255AB__INCLUDED_)
