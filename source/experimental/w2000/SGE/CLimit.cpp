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

#include "CLimit.h"

namespace GridEngine {

const ULONGLONG CLimit::nNanosecondsPerSecond = 1000000000;
const ULONGLONG CLimit::nMillisecondsPerSecond = 1000;
const ULONGLONG CLimit::nNanosecondsPerMillisecond = nNanosecondsPerSecond / nMillisecondsPerSecond; 

CLimit::CLimit() {
   UnSetAll();
}

CLimit::CLimit(const CLimit& Limit) {
   memcpy(&m_ExtLimitInfo, &Limit.m_ExtLimitInfo, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
}

CLimit::~CLimit() {
   UnSetAll();
}

const JOBOBJECT_EXTENDED_LIMIT_INFORMATION& CLimit::GetExtendedLimitInformation() const {
   return m_ExtLimitInfo;
}

void CLimit::SetExtendedLimitInformation(const JOBOBJECT_EXTENDED_LIMIT_INFORMATION& Limit) {
   memcpy(&m_ExtLimitInfo, &Limit, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
}


void CLimit::SetProcess(DWORD nProcesses) {
   m_ExtLimitInfo.BasicLimitInformation.ActiveProcessLimit = nProcesses;
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
}

void CLimit::SetJobUserTime(ULONGLONG lnUserTime) {
   m_ExtLimitInfo.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = 
      lnUserTime * (nNanosecondsPerSecond / 100);
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_TIME;
}

void CLimit::SetProcessUserTime(ULONGLONG lnUserTime) {
   m_ExtLimitInfo.BasicLimitInformation.PerProcessUserTimeLimit.QuadPart = 
      lnUserTime * (nNanosecondsPerSecond / 100);
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags |=  JOB_OBJECT_LIMIT_PROCESS_TIME;
}

void CLimit::SetProcessorSet(DWORD Processors) {
   m_ExtLimitInfo.BasicLimitInformation.Affinity = Processors;
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_AFFINITY;
} 

void CLimit::SetProcessMemory(SIZE_T nMemory) {
   m_ExtLimitInfo.ProcessMemoryLimit = nMemory;
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_MEMORY;
}

void CLimit::SetJobMemory(SIZE_T nMemory) {
   m_ExtLimitInfo.JobMemoryLimit = nMemory;
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
}

void CLimit::SetPriority(DWORD dwPriority) {
   m_ExtLimitInfo.BasicLimitInformation.PriorityClass;
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PRIORITY_CLASS;
}

void CLimit::SetSchedulingClass(DWORD dwPriority) {
   m_ExtLimitInfo.BasicLimitInformation.SchedulingClass;
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_SCHEDULING_CLASS;
}

void CLimit::SetWorkingSetSize(DWORD dwMin, DWORD dwMax) {
   m_ExtLimitInfo.BasicLimitInformation.MinimumWorkingSetSize = dwMin;
   m_ExtLimitInfo.BasicLimitInformation.MaximumWorkingSetSize = dwMax;
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_WORKINGSET;
}

void CLimit::UnSetAll() {
   memset (&m_ExtLimitInfo, 0, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
}

void CLimit::UnSetProcess() {
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags &= ~JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
}

void CLimit::UnSetJobUserTime() {
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags &= ~JOB_OBJECT_LIMIT_JOB_TIME;
}

void CLimit::UnSetProcessUserTime() {
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags &=  ~JOB_OBJECT_LIMIT_PROCESS_TIME;
}

void CLimit::UnSetProcessorSet() {
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags &= ~JOB_OBJECT_LIMIT_AFFINITY;
}

void CLimit::UnSetProcessMemory() {
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags &= ~JOB_OBJECT_LIMIT_PROCESS_MEMORY;
}

void CLimit::UnSetJobMemory() {
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags &= ~JOB_OBJECT_LIMIT_JOB_MEMORY;
}

void CLimit::UnSetPriority() {
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags &= ~JOB_OBJECT_LIMIT_PRIORITY_CLASS;
}

void CLimit::UnSetSchedulingClass() {
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags &= ~JOB_OBJECT_LIMIT_SCHEDULING_CLASS;
}

void CLimit::UnSetWorkingSetSize() {
   m_ExtLimitInfo.BasicLimitInformation.LimitFlags &= ~JOB_OBJECT_LIMIT_WORKINGSET;
}

BOOL CLimit::IsSetProcess() {
   if (m_ExtLimitInfo.BasicLimitInformation.LimitFlags & JOB_OBJECT_LIMIT_ACTIVE_PROCESS) {
      return true;
   } else {
      return false;
   }
}

BOOL CLimit::IsSetJobUserTime() {
   if (m_ExtLimitInfo.BasicLimitInformation.LimitFlags & JOB_OBJECT_LIMIT_JOB_TIME) {
      return true;
   } else {
      return false;
   }
}

BOOL CLimit::IsSetProcessUserTime() {
   if (m_ExtLimitInfo.BasicLimitInformation.LimitFlags & JOB_OBJECT_LIMIT_PROCESS_TIME) {
      return true;
   } else {
      return false;
   }
}

BOOL CLimit::IsSetProcessorSet() {
   if (m_ExtLimitInfo.BasicLimitInformation.LimitFlags & JOB_OBJECT_LIMIT_AFFINITY) {
      return true;
   } else {
      return false;
   }
}

BOOL CLimit::IsSetProcessMemory() {
   if (m_ExtLimitInfo.BasicLimitInformation.LimitFlags & JOB_OBJECT_LIMIT_PROCESS_MEMORY) {
      return true;
   } else {
      return false;
   }
}

BOOL CLimit::IsSetJobMemory() {
   if (m_ExtLimitInfo.BasicLimitInformation.LimitFlags & JOB_OBJECT_LIMIT_JOB_MEMORY) {
      return true;
   } else {
      return false;
   }
}

BOOL CLimit::IsSetWorkingSetSize() {
   if (m_ExtLimitInfo.BasicLimitInformation.LimitFlags & JOB_OBJECT_LIMIT_WORKINGSET) {
      return true;
   } else {
      return false;
   }
} 

BOOL CLimit::IsSetPriority() {
   if (m_ExtLimitInfo.BasicLimitInformation.LimitFlags & JOB_OBJECT_LIMIT_PRIORITY_CLASS) {
      return true;
   } else {
      return false;
   }
}

BOOL CLimit::IsSetSchedulingClass() {
   if (m_ExtLimitInfo.BasicLimitInformation.LimitFlags & JOB_OBJECT_LIMIT_SCHEDULING_CLASS) {
      return true;
   } else {
      return false;
   }
}


DWORD CLimit::GetProcess() {
   return m_ExtLimitInfo.BasicLimitInformation.ActiveProcessLimit;
}

ULONGLONG CLimit::GetJobUserTime() {
   return m_ExtLimitInfo.BasicLimitInformation.PerJobUserTimeLimit.QuadPart 
      / (nNanosecondsPerSecond / 100);
}

ULONGLONG CLimit::GetProcessUserTime() {
   return m_ExtLimitInfo.BasicLimitInformation.PerProcessUserTimeLimit.QuadPart
      / (nNanosecondsPerSecond / 100);
}

DWORD CLimit::GetProcessorSet() {
   return m_ExtLimitInfo.BasicLimitInformation.Affinity;
}

SIZE_T CLimit::GetProcessMemory() {
   return m_ExtLimitInfo.ProcessMemoryLimit;
}

SIZE_T CLimit::GetJobMemory() {
   return m_ExtLimitInfo.JobMemoryLimit;
}

DWORD CLimit::GetWorkingSetSizeMin() {
   return m_ExtLimitInfo.BasicLimitInformation.MinimumWorkingSetSize;
}

DWORD CLimit::GetWorkingSetSizeMax() {
   return m_ExtLimitInfo.BasicLimitInformation.MaximumWorkingSetSize;
}

DWORD CLimit::GetPriority() {
   return m_ExtLimitInfo.BasicLimitInformation.PriorityClass;
}

DWORD CLimit::GetSchedulingClass() {
   return m_ExtLimitInfo.BasicLimitInformation.SchedulingClass;
}

} // namespace
