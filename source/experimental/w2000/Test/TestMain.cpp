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
#include <stdlib.h>
#include <stdio.h>
#include <Userenv.h>
#include <pdh.h>
#include <pdhmsg.h>

#include "CSehTranslator.h"
#include "CException.h"
#include "CError.h"
#include "CTimer.h"
#include "CTrace.h"
#include "CProcess.h"
#include "CRegistryKey.h"

#include "CConfiguration.h"
#include "CShepherd.h"
#include "CWorker.h"


using namespace std;
using namespace GridEngine;

int wmain(int argc, LPTSTR *argv[], LPTSTR *envp[]) {
   // enable the SEH to CEH translator
   _se_translator_function pOldTranslator;
   pOldTranslator = _set_se_translator(TranslateSeToCe);

   // enable tracing 
   CTrace TheTrace(L"Test");
   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"wmain(int, LPTSTR**, LPTSTR**)");

   try {
      HQUERY HostQuery;
      PDH_STATUS Ret;
      DWORD dwUserValue;

      dwUserValue = 1;
      Ret = PdhOpenQuery(0, dwUserValue, &HostQuery);
      if (Ret != ERROR_SUCCESS) {
         WCHAR szMessage[4096];

         wsprintf(szMessage, L"%s (0x%lx)", L"PdhOpenQuery failed", Ret);
         throw CCodineException(szMessage, __FILE__, __LINE__);
      } else {
         DWORD dwSize = 1024;
         WCHAR *szBuffer;

         do {
            dwSize *= 2;

            szBuffer = new TCHAR[dwSize];
            Ret = PdhExpandWildCardPath(NULL, L"\\Processor(*)\\% Processor Time", szBuffer, &dwSize, 0);
         } while (Ret == PDH_MORE_DATA);

         if (Ret != PDH_CSTATUS_VALID_DATA) {
            WCHAR szMessage[4096];

            wsprintf(szMessage, L"%s (0x%lx)", L"PdhExpandWildCardPath failed", Ret);
            throw CCodineException(szMessage, __FILE__, __LINE__);
         } else {
            int nCounter;

            while(*szBuffer != L'\0') {
               wcout << szBuffer << endl;

               szBuffer += wcslen(szBuffer) + 1;
            }
         }
   
         Ret = PdhCloseQuery(HostQuery);
         if (Ret != ERROR_SUCCESS) {
            WCHAR szMessage[4096];

            wsprintf(szMessage, L"%s (0x%lx)", L"PdhCloseQuery failed", Ret);
            throw CCodineException(szMessage, __FILE__, __LINE__);
         }
      }

   } catch (CCodineException& ex) {
      wcout << ex;
   } catch (CException& ex) {
      wcout << ex;
   } catch (...) {
      wcout << L"Caught unknown exception in main()" << endl;
   }

   // disable trabslator
   _set_se_translator(pOldTranslator);
   return 0;
}

