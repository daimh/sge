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
#include <Shellapi.h>
#include <Rtutils.h>
#include <stdarg.h>

#include "CTrace.h"
#include "CError.h"
#include "CException.h"

namespace GridEngine {

CTrace::CEnter::CEnter(WORD wLayer, const CString& Message) :
   m_String(Message),
   m_wLayer(wLayer)
{
   CString LocalString = m_String;

   CTrace::Print(m_wLayer, L"--> %s", LocalString);
}

CTrace::CEnter::~CEnter() {
   CString LocalString = m_String;

   CTrace::Print(m_wLayer, L"<-- %s", LocalString);
}

DWORD CTrace::m_dwTraceId = INVALID_TRACEID;

CTrace::CTrace() {
   PWSTR *szArgv;
   int nArgc;

   szArgv = CommandLineToArgvW(GetCommandLine(), &nArgc);
   if (nArgc > 0) {
      m_dwTraceId = TraceRegister(szArgv[0]);
      if (m_dwTraceId == INVALID_TRACEID) {
         throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
      }
   } else {

   }
   if (GlobalFree(szArgv)) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

CTrace::CTrace(const CString& Name) {
   CString LocalName(Name);
   
   m_dwTraceId = TraceRegister(LocalName.c_str());
   if (m_dwTraceId == INVALID_TRACEID) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

CTrace::CTrace(const CString& Name, const CString& Filename) {
   CString LocalName(Name);

   m_dwTraceId = TraceRegisterEx(LocalName.c_str(), TRACE_USE_FILE);
   if (m_dwTraceId == INVALID_TRACEID) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

CTrace::~CTrace(){
   DWORD dwRet;

   dwRet = TraceDeregister(m_dwTraceId);
   if (dwRet != 0) {
      throw CCodineException(CError::GetErrorMessage(GetLastError()), __FILE__, __LINE__);
   }
}

void CTrace::Print(WORD wLayer, PCWSTR Format, ...) {
   if (m_dwTraceId != INVALID_TRACEID) {
      DWORD dwRet;
      DWORD dwLayer;
      va_list Args;

      va_start(Args, Format);
      dwLayer = wLayer << 16;
      dwLayer |= TRACE_USE_MASK;
      dwRet = TraceVprintfEx(m_dwTraceId, dwLayer, Format, Args);
      va_end(Args);
      if (dwRet == 0) {
         // we will ignore this error
      }
   }
}

} // namespace
