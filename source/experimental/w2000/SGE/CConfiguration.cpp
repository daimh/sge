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
#include <shlwapi.h>
#include <iostream>
#include <stdio.h>

#include "CBasicTypes.h"
#include "CException.h"
#include "CConfiguration.h"

namespace GridEngine {

CConfiguration::CConfiguration(const CString& BaseDirectory) :
   m_BaseDirectory(BaseDirectory),
   m_bMergeStdErr(FALSE),
   m_dwJobArgs(0),
   m_dwJaTaskId(0),
   m_dwJobId(0),
   m_dwProcessors(0),
   m_HardLimit(),
   m_JobArg(),
   m_JobName(L""),
   m_JobOwner(L""),
   m_dwOsJobId(0),
   m_QueueTmpDir(L""),
   m_ScriptFile(L""),
   m_ShellPath(L""),
   m_SoftLimit(),
   m_StdErrPath(L""),
   m_StdOutPath(L""),
   m_ForbidReschedule(true)
{
   ;
}

CConfiguration::~CConfiguration() {
}

BOOL CConfiguration::GetForbidReschedule() {
   return m_ForbidReschedule;
}

CLimit& CConfiguration::GetHardLimit() {
   return m_HardLimit;
}


DWORD CConfiguration::GetJaTaskId() {
   return m_dwJaTaskId;
}

DWORD CConfiguration::GetJobArgs() {
   return m_dwJobArgs;
}

CString CConfiguration::GetJobArg(DWORD dwIndex) {
   return m_JobArg[dwIndex];
}

DWORD CConfiguration::GetJobId() {
   return m_dwJobId;
}

CString CConfiguration::GetJobName() {
   return m_JobName;
}

CString CConfiguration::GetJobOwner() {
   return m_JobOwner;
}

BOOL CConfiguration::GetMergeStdErr() {
   return m_bMergeStdErr;
}

DWORD CConfiguration::GetOsJobId() {
   return m_dwOsJobId;
}

DWORD CConfiguration::GetProcessors() {
   return m_dwProcessors;
}

CString CConfiguration::GetQueueTmpDir() {
   return m_QueueTmpDir;
}

CString CConfiguration::GetShellPath() {
   return m_ShellPath;
}

CString CConfiguration::GetScriptFile() {
   return m_ScriptFile;
}

CLimit& CConfiguration::GetSoftLimit() {
   return m_SoftLimit;
}

CString CConfiguration::GetStdOutPath() {
   return m_StdOutPath;
}

CString CConfiguration::GetStdErrPath() {
   return m_StdErrPath;
}

void CConfiguration::ReadConfiguration() {
   const DWORD dwMaxSize = 256;
   FILE *ConfigFile;
   char pszBuffer[dwMaxSize];
   char pszOrgBuffer[dwMaxSize];
   WCHAR pszNameBuffer[dwMaxSize];
   WCHAR pszValueBuffer[dwMaxSize];
   char PathPlusFilename[1024];

   sprintf(PathPlusFilename, "%S\\%s", m_BaseDirectory.c_str(), "config");
   if ((ConfigFile = fopen(PathPlusFilename, "r"))) {
      while (fgets(pszBuffer, dwMaxSize, ConfigFile) != NULL) {
         char *pName, *pValue;

         strcpy(pszOrgBuffer, pszBuffer);

         pName = strtok(pszBuffer, "=");
         if (pName) {
            pValue = strtok(NULL, "\n");
            wsprintf(pszNameBuffer, L"%S", pName);
            wsprintf(pszValueBuffer, L"%S", pValue);

            if (0) {
            } else if (!lstrcmp(pszNameBuffer, L"h_cpu")) {
               ULONGLONG CpuLimit = StrToLong(pszValueBuffer);

               if (CpuLimit) {
                  m_HardLimit.SetJobUserTime(CpuLimit);
               }
            } else if (!lstrcmp(pszNameBuffer, L"h_data")) {
               SIZE_T MemoryLimit = StrToLong(pszValueBuffer);

               if (MemoryLimit) {
                  m_HardLimit.SetJobMemory(MemoryLimit);
               }
            } else if (!lstrcmp(pszNameBuffer, L"ja_task_id")) {
               m_dwJaTaskId = StrToLong(pszValueBuffer);
            } else if (!lstrcmp(pszNameBuffer, L"job_owner")) {
               m_JobOwner = pszValueBuffer;
            } else if (!wcsncmp(pszNameBuffer, L"job_arg", 7)) {
               PWSTR pszNumber;
               DWORD dwNumber;

               pszNumber = pszNameBuffer + 7;
               dwNumber = StrToLong(pszNumber);
               if (dwNumber != 0) {
                  dwNumber--;
                  m_JobArg[dwNumber] = pszValueBuffer;
               }
            } else if (!lstrcmp(pszNameBuffer, L"job_id")) {
               m_dwJobId = StrToLong(pszValueBuffer);
            } else if (!lstrcmp(pszNameBuffer, L"job_name")) {
               m_JobName = pszValueBuffer;
            } else if (!lstrcmp(pszNameBuffer, L"merge_stderr")) {
               m_bMergeStdErr = (StrToLong(pszValueBuffer)==1) ? TRUE : FALSE;
            } else if (!lstrcmp(pszNameBuffer, L"njob_args")) {
               m_dwJobArgs = StrToLong(pszValueBuffer);
            } else if (!lstrcmp(pszNameBuffer, L"processors")) {
               m_dwProcessors = StrToLong(pszValueBuffer);
            } else if (!lstrcmp(pszNameBuffer, L"queue_tmpdir")) {
               m_QueueTmpDir = pszValueBuffer;
            } else if (!lstrcmp(pszNameBuffer, L"script_file")) {
               m_ScriptFile = pszValueBuffer;
            } else if (!lstrcmp(pszNameBuffer, L"shell_path")) {
               m_ShellPath = pszValueBuffer;
            } else if (!lstrcmp(pszNameBuffer, L"stdout_path")) {
               m_StdOutPath = pszValueBuffer;
            } else if (!lstrcmp(pszNameBuffer, L"stderr_path")) {
               m_StdErrPath = pszValueBuffer;
            } else if (!lstrcmp(pszNameBuffer, L"s_cpu")) {
               ULONGLONG CpuLimit = StrToLong(pszValueBuffer);

               if (CpuLimit) {
                  m_SoftLimit.SetJobUserTime(CpuLimit);
               }
            } else if (!lstrcmp(pszNameBuffer, L"s_data")) {
               SIZE_T MemoryLimit = StrToLong(pszValueBuffer);

               if (MemoryLimit) {
                  m_SoftLimit.SetJobMemory(MemoryLimit);
               }
            } else if (!lstrcmp(pszNameBuffer, L"forbid_reschedule")) {
               DWORD dwBool;

               dwBool = StrToLong(pszValueBuffer);
               if (dwBool == 1) {
                  m_ForbidReschedule = true;
               } else {
                  m_ForbidReschedule = false;
               }
            } 
         }
      }
      fclose(ConfigFile);
   } else {
      throw CCodineException(L"Can not open config file", __FILE__, __LINE__);
   }
}

void CConfiguration::ReadOsJobId() {
   const DWORD dwMaxSize = 256;
   FILE *ConfigFile;
   char pszBuffer[dwMaxSize];
   WCHAR pszValueBuffer[dwMaxSize];
   char PathPlusFilename[1024];

   sprintf(PathPlusFilename, "%S\\%s", m_BaseDirectory.c_str(), "osjobid");
   if ((ConfigFile = fopen(PathPlusFilename, "r"))) {
      while (fgets(pszBuffer, dwMaxSize, ConfigFile) != NULL) {
         wsprintf(pszValueBuffer, L"%S", pszBuffer);
         m_dwOsJobId = StrToLong(pszValueBuffer);
      }
      fclose(ConfigFile);
   } 
}

void CConfiguration::Print() {
   wcout << L"JaTaskId:" << GetJaTaskId() << endl;
   wcout << L"JobArgs:" << GetJobArgs() << endl;
   for (DWORD i=0; i<GetJobArgs(); i++) {
      wcout << L"JobArg(" << i << L"):" << GetJobArg(i) << endl;
   }
   wcout << L"JobId:" << GetJobId() << endl;
   wcout << L"JobName:" << GetJobName() << endl;
   wcout << L"JobOwner:" << GetJobOwner() << endl;
   wcout << L"MergeStdErr:" << GetMergeStdErr() << endl;
   wcout << L"OsJobId:" << GetOsJobId() << endl;
   wcout << L"Processors:" << GetProcessors() << endl;
   wcout << L"QueueTmpDir:" << GetQueueTmpDir() << endl;
   wcout << L"ScriptFile:" << GetScriptFile() << endl;
   wcout << L"ShellPath:" << GetShellPath() << endl;
   wcout << L"StdErrPath:" << GetStdErrPath() << endl;
   wcout << L"StdOutPath:" << GetStdOutPath() << endl;

   if (GetSoftLimit().IsSetJobMemory()) {
     wcout << L"JobSoftMemoryLimit:" << GetSoftLimit().GetJobMemory() << endl;
   }
   if (GetHardLimit().IsSetJobMemory()) {
     wcout << L"JobHardMemoryLimit:" << GetHardLimit().GetJobMemory() << endl;
   }
   if (GetSoftLimit().IsSetJobUserTime()) {
     wcout << L"JobSoftUserTimeLimit:" << (LONG) GetSoftLimit().GetJobUserTime() << endl;
   }
   if (GetHardLimit().IsSetJobUserTime()) {
     wcout << L"JobHardUserTimeLimit:" << (LONG) GetHardLimit().GetJobUserTime() << endl;
   }
}

} // namespace
