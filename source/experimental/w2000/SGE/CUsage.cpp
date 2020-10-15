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
#include <time.h>

#include "CUsage.h"
#include "CException.h"
#include "CJob.h"

namespace GridEngine {

const ULONGLONG CUsage::nNanosecondsPerSecond = 1000000000;
const ULONGLONG CUsage::nMillisecondsPerSecond = 1000;
const ULONGLONG CUsage::nNanosecondsPerMillisecond = nNanosecondsPerSecond / nMillisecondsPerSecond; 

CUsage::CUsage() :
   m_StartTime(0),
   m_EndTime(0),
   m_nExitStatus(CJob::Success)
{
   memset(&m_BasicAndIOInfo, 0, sizeof(JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION));
}

CUsage::~CUsage() {
}

const JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION& CUsage::GetBasicAndIoAccountingInformation() const {
   return m_BasicAndIOInfo;
}

void CUsage::SetBasicAndIoAccountingInformation(const JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION& Usage) {
   memcpy(&m_BasicAndIOInfo, &Usage, sizeof(JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION));
}

ULONGLONG CUsage::GetKernelTime() const {
   return m_BasicAndIOInfo.BasicInfo.TotalKernelTime.QuadPart / (nNanosecondsPerSecond / 100);
}

ULONGLONG CUsage::GetUserTime() const {
   return m_BasicAndIOInfo.BasicInfo.TotalUserTime.QuadPart / (nNanosecondsPerSecond / 100);
}

DWORD CUsage::GetProcesses() const {
   return m_BasicAndIOInfo.BasicInfo.TotalProcesses;
}

ULONGLONG CUsage::GetReadOperations() const {
   return m_BasicAndIOInfo.IoInfo.ReadOperationCount;
}

ULONGLONG CUsage::GetWriteOperations() const {
   return m_BasicAndIOInfo.IoInfo.WriteOperationCount;
}

ULONGLONG CUsage::GetOtherOperations() const {
   return m_BasicAndIOInfo.IoInfo.OtherOperationCount;
}

ULONGLONG CUsage::GetReadBytes() const {
   return m_BasicAndIOInfo.IoInfo.ReadTransferCount;
}

ULONGLONG CUsage::GetWriteBytes() const {
   return m_BasicAndIOInfo.IoInfo.WriteTransferCount;
}

ULONGLONG CUsage::GetOtherBytes() const {
   return m_BasicAndIOInfo.IoInfo.OtherTransferCount;
}

DWORD CUsage::GetExitStatus() const {
   return m_nExitStatus;
}

time_t CUsage::GetStartTime() const {
   return m_StartTime;
}

time_t CUsage::GetEndTime() const {
   return m_EndTime;
}

void CUsage::SetExitStatus(DWORD Value) {
   m_nExitStatus = Value;
}

void CUsage::SetStartTime(time_t Time) {
   m_StartTime = Time;
}

void CUsage::SetEndTime(time_t Time) {
   m_EndTime = Time;
}

void CUsage::WriteUsageToFile(CString Filename) const {
   FILE *UsageFile;
   char PathPlusFilename[1024];

   sprintf(PathPlusFilename, "%S", Filename.c_str());
   if ((UsageFile = fopen(PathPlusFilename, "w"))) {
      static const int nMaxSize = 1024;
      char szBuffer[nMaxSize];

      // Job exit status
      fprintf(UsageFile, "exit_status=%lu\n", m_nExitStatus);

      // Start time
      fprintf(UsageFile, "start_time=%lu\n", m_StartTime ? m_StartTime : 0);

      // End time
      fprintf(UsageFile, "end_time=%lu\n", m_EndTime ? m_EndTime : 0);

      // Wallclock time
      fprintf(UsageFile, "ru_wallclock=%lu\n", 
         (m_StartTime && m_EndTime) ? m_EndTime - m_StartTime : 0);

      // User Time
      _ui64toa(GetUserTime(), szBuffer, 10);
      fprintf(UsageFile, "ru_utime=%s\n", szBuffer);

      // System Time
      _ui64toa(GetKernelTime(), szBuffer, 10);
      fprintf(UsageFile, "ru_stime=%s\n", szBuffer);

      fprintf(UsageFile, "ru_maxrss=0\n", szBuffer);
      fprintf(UsageFile, "ru_ixrss=0\n", szBuffer);
      fprintf(UsageFile, "ru_idrss=0\n", szBuffer);
      fprintf(UsageFile, "ru_isrss=0\n", szBuffer);
      fprintf(UsageFile, "ru_minflt=0\n", szBuffer);
      fprintf(UsageFile, "ru_majflt=0\n", szBuffer);
      fprintf(UsageFile, "ru_nswap=0\n", szBuffer);
      fprintf(UsageFile, "ru_inblock=0\n", szBuffer);
      fprintf(UsageFile, "ru_outblock=0\n", szBuffer);
      fprintf(UsageFile, "ru_msgsnd=0\n", szBuffer);
      fprintf(UsageFile, "ru_msgrcv=0\n", szBuffer);
      fprintf(UsageFile, "ru_nsignals=0\n", szBuffer);
      fprintf(UsageFile, "ru_nvcsw=0\n", szBuffer);
      fprintf(UsageFile, "ru_nivcsw=0\n", szBuffer);

      fclose(UsageFile);
   } else {
      throw CCodineException(L"Can not write usage file", __FILE__, __LINE__);
   }
}


} // namespace
