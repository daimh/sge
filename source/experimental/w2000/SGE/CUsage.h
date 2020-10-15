#if !defined(AFX_USAGE_H__6B64017A_932A_4DB6_95EA_7A6038A155A5__INCLUDED_)
#define AFX_USAGE_H__6B64017A_932A_4DB6_95EA_7A6038A155A5__INCLUDED_
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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <time.h>

#include "CBasicTypes.h"

namespace GridEngine {

class CUsage {
 public:
   static const ULONGLONG nNanosecondsPerSecond;
   static const ULONGLONG nMillisecondsPerSecond;
   static const ULONGLONG nNanosecondsPerMillisecond; 

 private:
   JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION m_BasicAndIOInfo;
   CString m_BaseDirectory;
   unsigned long m_nExitStatus;
   time_t m_StartTime;
   time_t m_EndTime;

 public:
   const JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION& GetBasicAndIoAccountingInformation() const;
   void SetBasicAndIoAccountingInformation(const JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION& Usage);

 public:
	CUsage();
	virtual ~CUsage();

   ULONGLONG GetKernelTime() const ;
   ULONGLONG GetUserTime() const ;
   DWORD GetProcesses() const;

   ULONGLONG GetReadOperations() const;
   ULONGLONG GetWriteOperations() const;
   ULONGLONG GetOtherOperations() const;

   ULONGLONG GetReadBytes() const;
   ULONGLONG GetWriteBytes() const;
   ULONGLONG GetOtherBytes() const;

   DWORD GetExitStatus() const;
   time_t GetStartTime() const;
   time_t GetEndTime() const;

   void SetExitStatus(DWORD Value);
   void SetStartTime(time_t Time);
   void SetEndTime(time_t Time);

   void WriteUsageToFile(CString Filename) const;
};

} // namespace

#endif // !defined(AFX_USAGE_H__6B64017A_932A_4DB6_95EA_7A6038A155A5__INCLUDED_)
