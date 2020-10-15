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

#include "CJobObject.h"
#include "CJob.h"
#include "CLimit.h"
#include "CUsage.h"

namespace GridEngine {

const unsigned long CJob::Success               = 0;
const unsigned long CJob::EndOfJobTime          = 1;
const unsigned long CJob::EndOfProcessTime      = 2;
const unsigned long CJob::ActiveProcessLimit    = 4;
const unsigned long CJob::AbnormalExitProcess   = 8;
const unsigned long CJob::JobMemoryLimit        = 16;
const unsigned long CJob::ProcessMemoryLimit    = 32;

CJob::CJob(const CLimit& Soft, const CLimit& Hard) :
   CJobObject(),
   m_Hard(Hard),
   m_Soft(Soft),
   m_Usage(),
   m_ExitStatus(Success)
{
   SetExtendedLimit(m_Hard.GetExtendedLimitInformation());
}

CJob::CJob(DWORD dwJobId) :
   CJobObject(dwJobId)
{
}


CJob::~CJob() {
}

DWORD CJob::ActionEndOfJobTime() {
   long nExitStatus;

   wcerr << L"EndOfJobTime: Job Terminated" << endl;
   Terminate(1);

   nExitStatus = m_Usage.GetExitStatus();
   m_Usage.SetExitStatus(nExitStatus |= EndOfJobTime);
   return 0;
}

DWORD CJob::ActionEndOfProcessTime() {
   return 0;
}

DWORD CJob::ActionActiveProcessLimit() {
   m_ExitStatus |= ActiveProcessLimit;
   return 0;
}

DWORD CJob::ActionAbnormalExitProcess() {
   long nExitStatus;

   nExitStatus = m_Usage.GetExitStatus();
   m_Usage.SetExitStatus(nExitStatus |= AbnormalExitProcess);
   return 0;
}

DWORD CJob::ActionJobMemoryLimit() {
   static BOOL bHardLimitSet = false;
   static BOOL bTerminated = false;
   CLimit TmpLimit;

   if (!bTerminated) {
      long nExitStatus;

      wcerr << L"JobMemoryLimit: Job Terminated" << endl;
      Terminate(1);

      nExitStatus = m_Usage.GetExitStatus();
      m_Usage.SetExitStatus(nExitStatus |= JobMemoryLimit);
      bTerminated = true;
   }
   return 0;
}

DWORD CJob::ActionProcessMemoryLimit() {
   long nExitStatus;
   nExitStatus = m_Usage.GetExitStatus();
   m_Usage.SetExitStatus(nExitStatus |= ProcessMemoryLimit);
   return 0;
}

DWORD CJob::ActionNewProcess() {
   return 0;
}

DWORD CJob::ActionExitProcess() {
   return 0;
}

DWORD CJob::ActionActiveProcessZero() {
   return 0;
}

DWORD CJob::GetExitCode() const {
   return m_ExitStatus;
}

const CUsage& CJob::GetExtendedUsage() const {
   m_Usage.SetBasicAndIoAccountingInformation(GetExtendedUsageInfo());
   return m_Usage;
}

} // namespace
