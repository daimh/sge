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

#include "Worker.h"
#include "SehTranslator.h"
#include "Exception.h"
#include "GError.h"
#include "Timer.h"
#include "GTrace.h"

using namespace std;
using namespace GridEngine;

Trace Tracefile(L"Worker");

int wmain(int argc, LPTSTR *argv[], LPTSTR *envp[]) {
   // enable the SEH to CEH translator
   _se_translator_function pOldTranslator;
   pOldTranslator = _set_se_translator(TranslateSeToCe);

   // enable tracing
   Trace::Enter Enter(Trace::Layer::KERNEL, L"wmain(int, LPTSTR**, LPTSTR**)");

   try {
      Worker NewWorker(5000);
     
      NewWorker.AddSleeperThread();
      NewWorker.AddCpuThread();
      NewWorker.AddMemoryThread();
      NewWorker.AddIoThread();

      NewWorker.WaitForWorker();
   } catch (CodineException& ex) {
      wcout << ex;
   } catch (Exception& ex) {
      wcout << ex;
   } catch (...) {
      wcout << L"Caught unknown exception in main()" << endl;
   }
   _set_se_translator(pOldTranslator);
   return 0;
}

