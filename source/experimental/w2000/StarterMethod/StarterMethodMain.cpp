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
#include <Userenv.h>

#include "StarterMethodMain.h"
#include "CConfiguration.h"
#include "CShepherdEnvironment.h"
#include "CError.h"
#include "CException.h"
#include "CJob.h"
#include "CProcess.h"
#include "CRegistryKey.h"
#include "CSehTranslator.h"
#include "CShepherd.h"
#include "CTimer.h"
#include "CTrace.h"

using namespace std;
using namespace GridEngine;

int wmain(int argc, LPTSTR argv[], LPTSTR envp[]) {
   // enable the SEH to CEH translator
   _se_translator_function pOldTranslator;
   pOldTranslator = _set_se_translator(TranslateSeToCe);
   
   int nRet = 0;

   try {
      if (argc == 2) {
         CString BasePath = argv[1];
         CConfiguration Configuration(BasePath);
         CString CommandLine;

         // Read Configuration from file
         try {
            Configuration.ReadConfiguration();
            Configuration.ReadOsJobId();
         } catch(...) {
            CShepherd::WriteExitStatusFile(BasePath, CShepherd::ExitCodes::ReadConfig);
            wcerr << L"Could not read configuration" << endl;
            return StarterFailed;
         }

         // Change the current environment
         try {
            CShepherdEnvironment Environmnet(BasePath);

            Environmnet.ReadFromFile();
         } catch(...) {
            CShepherd::WriteExitStatusFile(BasePath, CShepherd::ExitCodes::ReadConfig);
            wcerr << L"Could not read environmnet" << endl;
            return StarterFailed;
         }

         // Shell
         CommandLine = CEnvironment::GetExpandedEnvironmentString(Configuration.GetShellPath());
         CommandLine += L" ";

         // Script
         CommandLine += CEnvironment::GetExpandedEnvironmentString(Configuration.GetScriptFile());

         // Arguments
         for (DWORD dwIndex=0; dwIndex<Configuration.GetJobArgs(); dwIndex++) {
            CommandLine += L" ";
            CommandLine += Configuration.GetJobArg(dwIndex);
         }

         // StdOut redirection
         CommandLine += L" 1>" + CEnvironment::GetExpandedEnvironmentString(Configuration.GetStdOutPath());

         // StdErr redirection
         if (Configuration.GetMergeStdErr()) {
            CommandLine += L" 2>&1";
         } else {
            CommandLine += L" 2>" + CEnvironment::GetExpandedEnvironmentString(Configuration.GetStdErrPath());
         }

         // Start first process of job
         try {
            CProcess Process(CommandLine, CProcess::Type::INVISIBLE);
            Process.Start();
            Process.WaitForObject();
            return Process.GetExitCode();
         } catch (...) {
            CShepherd::WriteExitStatusFile(BasePath, CShepherd::ExitCodes::NoShell);
            wcerr << L"Could not start job" << endl;
            return StarterFailed;
         }
      }
   } catch (CCodineException& ex) {
      wcout << ex;
   } catch (CException& ex) {
      wcout << ex;
   } catch (...) {
      wcout << L"Caught unknown exception in main()" << endl;
   }

   // disable translator
   _set_se_translator(pOldTranslator);
   return 0;
}

