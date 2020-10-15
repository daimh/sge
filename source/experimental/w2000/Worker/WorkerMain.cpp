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

#include "CWorker.h"
#include "CSehTranslator.h"
#include "CException.h"
#include "CError.h"
#include "CTimer.h"
#include "CTrace.h"

using namespace std;
using namespace GridEngine;

// CTrace Tracefile(L"Worker");

void show_usage (void) {
   wcerr << L"woker.exe [-h] [-w <time>] [-m <memory><seconds>]" << endl;
   wcerr << endl;
   wcerr << L"   -h  show this help" << endl;
   wcerr << L"   -w  work <time> seconds" << endl;
   wcerr << endl;
}

int wmain(int argc, LPTSTR argv[], LPTSTR envp[]) {
   // enable the SEH to CEH translator
   _se_translator_function pOldTranslator;
   pOldTranslator = _set_se_translator(TranslateSeToCe);

   // enable tracing
//   CTrace::CEnter Enter(CTrace::Layer::KERNEL, L"wmain(int, LPTSTR**, LPTSTR**)");

   try {
      static const DWORD dwDefaultTime = 60000;
      DWORD dwTime = dwDefaultTime;
      DWORD dwMemory = 0;
      DWORD dwMemoryTime = 0;
      int nNextParam = 1;

      while (nNextParam < argc) {
         if (!wcscmp(argv[nNextParam], L"-w")) {
            nNextParam++;
            dwTime = _wtoi(argv[nNextParam]);
            nNextParam++;
            if (!dwTime) {
               dwTime = dwDefaultTime;
            }
            wcerr << L"-w " << dwTime << endl;
         } else if (!wcscmp(argv[nNextParam], L"-m")) {
            static const DWORD dwKByte = 1024;
            static const DWORD dwMByte = dwKByte * 1024;

            nNextParam++;
            if (nNextParam < argc) {
               dwMemory = _wtoi(argv[nNextParam]);
               if (argv[nNextParam][wcslen(argv[nNextParam])-1] == L'K') {
                  dwMemory *= dwKByte;
               } else if (argv[nNextParam][wcslen(argv[nNextParam])-1] == L'M') {
                  dwMemory *= dwMByte;
               } else {
                  dwMemory *= dwKByte;
               }
            } 
            if (nNextParam >= argc || dwMemory == 0) {
               wcerr << L"Syntax error in -m switch" << endl;
            }
            nNextParam++;
            if (nNextParam < argc) {
               dwMemoryTime = _wtoi(argv[nNextParam]);
               if (dwMemoryTime) {
                  nNextParam++;
               }
            }
            wcerr << L"-m " << dwMemory << L" ";
            if (dwMemoryTime) {
               wcerr << dwMemoryTime;
            }
            wcerr << endl;
         } else {
            wcerr << L"Syntax error in command line" << endl;
            break;
         }
      }
      
      CWorker NewWorker(dwTime);
      NewWorker.AddSleeperThread();

      if (dwMemory) {
         NewWorker .SetMemory(dwMemory);
         if (dwMemoryTime) {
            NewWorker .SetMemoryTime(dwMemoryTime);
         }
         NewWorker.AddMemoryThread();
      }

      NewWorker.AddCpuThread();


#if 0
      NewWorker.AddIoThread();
      NewWorker.AddThreadWormThread();
#endif

      NewWorker.WaitForObject();
   } catch (CCodineException& ex) {
      wcout << ex;
   } catch (CException& ex) {
      wcout << ex;
   } catch (...) {
      wcout << L"Caught unknown exception in main()" << endl;
   }
   _set_se_translator(pOldTranslator);
   return 87;
}

