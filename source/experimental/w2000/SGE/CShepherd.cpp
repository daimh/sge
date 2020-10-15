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

#include "CEnvironment.h"
#include "CUser.h"

#include "CShepherd.h"
#include "CConfiguration.h"
#include "CJob.h"
#include "CProcess.h"
#include "CUsage.h"

#include "StarterMethodMain.h"

namespace GridEngine {

CShepherd::CShepherd(const CString& BaseDirectory) :
   m_BaseDirectory(BaseDirectory),
   m_Configuration(BaseDirectory),
   m_Job(NULL),
   m_bNewJob(FALSE)
{
   try {
      m_Configuration.ReadConfiguration();
      m_Configuration.ReadOsJobId();
   } catch(...) {
      CShepherd::WriteExitStatusFile(m_BaseDirectory, CShepherd::ExitCodes::ReadConfig);
      wcerr << L"Shepherd: Could not read configuration" << endl;
      exit(1);
   }
   try {
      if (m_Configuration.GetOsJobId() != 0) {
         m_Job = new CJob(m_Configuration.GetOsJobId());
         m_bNewJob = FALSE;
      } else {
         m_Job = new CJob(m_Configuration.GetSoftLimit(), m_Configuration.GetHardLimit());
         WriteOsJobIdFile();
         m_Configuration.ReadOsJobId();
         m_bNewJob = TRUE;
      }
   } catch(...) {
      CShepherd::WriteExitStatusFile(m_BaseDirectory, CShepherd::ExitCodes::BeforeJob);
      wcerr << L"Shepherd: Can not create Job" << endl;
      exit(1);
   }
}

CShepherd::~CShepherd() {
   if (m_Job) {
      delete m_Job;
   }
}

void CShepherd::Resume() {
   // TODO: Get the password for the job owner
   CString Password = L"Test";
   CUser StartUser(m_Configuration.GetJobOwner(), Password);

   StartUser.Impersonate();
   m_Job->Resume();
   StartUser.ReturnToSelf();
}

void CShepherd::Start() {
   time_t StartTime;
   CString StarterCommand;
   CString Password = L"Test";

   try {
      // Get start time
      time(&StartTime);

      if (m_bNewJob == FALSE) {
         if (m_Job) {
            delete m_Job;
         }
         m_Job = new CJob(m_Configuration.GetSoftLimit(), m_Configuration.GetHardLimit());
         WriteOsJobIdFile();
         m_Configuration.ReadOsJobId();
         m_bNewJob = TRUE;
      }

      CUser StartUser(m_Configuration.GetJobOwner(), Password);
      CEnvironment UserEnvironment(StartUser);

      StarterCommand = L"%GRD_ROOT%\\bin\\w2000\\startermethod.exe ";
      StarterCommand = CEnvironment::GetExpandedEnvironmentString(StarterCommand);
      StarterCommand += m_BaseDirectory;

      CProcess Starter(StarterCommand, StartUser, UserEnvironment, CProcess::Type::INVISIBLE);
      m_Job->AssignProcess(Starter);
      Starter.Start();
      try {
         Starter.WaitForObject();
         if (Starter.GetExitCode() != StarterFailed) {
            time_t EndTime;
            CUsage Usage;
            DWORD dwExitCodeJob;
            DWORD dwExitCodeStarter;

            m_Job->WaitForObject();

            time(&EndTime);

            Usage = m_Job->GetExtendedUsage();

            Usage.SetStartTime(StartTime);
            Usage.SetEndTime(EndTime);

            // Job exit status
            dwExitCodeJob = Usage.GetExitStatus();
            dwExitCodeStarter = Starter.GetExitCode();

            if (dwExitCodeJob != CJob::Success) {
               CShepherd::WriteExitStatusFile(m_BaseDirectory, CShepherd::ExitCodes::DiedThruSignal);
            } else if (dwExitCodeStarter == 99 && !m_Configuration.GetForbidReschedule()) {
               CShepherd::WriteExitStatusFile(m_BaseDirectory, CShepherd::ExitCodes::Again);
            }

            Usage.SetExitStatus((dwExitCodeJob << 8) | dwExitCodeStarter);
            Usage.WriteUsageToFile(m_BaseDirectory + L"\\usage");
         } else {
            CShepherd::WriteExitStatusFile(m_BaseDirectory, CShepherd::ExitCodes::FailureAfterJob);
            wcerr << L"Shepherd: Can not create Job" << endl;
            exit(1);
         }
      } catch (...) {
         CShepherd::WriteExitStatusFile(m_BaseDirectory, CShepherd::ExitCodes::FailureAfterJob);
         wcerr << L"Shepherd: Can not create Job" << endl;
         exit(1);
      }
   } catch (...) {
      CShepherd::WriteExitStatusFile(m_BaseDirectory, CShepherd::ExitCodes::FailureAfterJob);
      wcerr << L"Shepherd: Can not create Job" << endl;
      exit(1);
   }
}

void CShepherd::Suspend() {
   // TODO: Get the password for the job owner
   CUser StartUser(m_Configuration.GetJobOwner(), L"Test");

   StartUser.Impersonate();
   m_Job->Suspend();
   StartUser.ReturnToSelf();
}

void CShepherd::Terminate(DWORD dwExitCode) {
   CShepherd::WriteExitStatusFile(m_BaseDirectory, CShepherd::ExitCodes::FailureAfterJob);
   m_Job->Terminate(dwExitCode);
}

void CShepherd::WriteExitStatusFile(CString BaseDirectory, ExitCodes eExitCode) {
   FILE *ExitStatusFile;
   char PathPlusFilename[1024];

   sprintf(PathPlusFilename, "%S\\%s", BaseDirectory.c_str(), "exit_status");
   if ((ExitStatusFile = fopen(PathPlusFilename, "w"))) {
      fprintf(ExitStatusFile, "%d", (int)eExitCode);
      fclose(ExitStatusFile);
   }
}


void CShepherd::WriteOsJobIdFile() {
   FILE *ConfigFile;
   char PathPlusFilename[1024];

   sprintf(PathPlusFilename, "%S\\%s", m_BaseDirectory.c_str(), "osjobid");
   if ((ConfigFile = fopen(PathPlusFilename, "w"))) {
      fprintf(ConfigFile, "%ld", m_Job->GetId());
      fclose(ConfigFile);
   }
}

void CShepherd::WaitForObject() {
   m_Job->WaitForObject();
}

} // namespace
