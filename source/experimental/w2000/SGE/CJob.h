// Job.h: interface for the Job class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JOB_H__C1C03F29_D185_47A5_9A97_644E99F791EF__INCLUDED_)
#define AFX_JOB_H__C1C03F29_D185_47A5_9A97_644E99F791EF__INCLUDED_
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

#include "CJobObject.h"
#include "CLimit.h"
#include "CUsage.h"

namespace GridEngine {

class CJob : public CJobObject {
 public:
   static const unsigned long Success;
   static const unsigned long EndOfJobTime;
   static const unsigned long EndOfProcessTime;
   static const unsigned long ActiveProcessLimit;
   static const unsigned long AbnormalExitProcess;
   static const unsigned long JobMemoryLimit;
   static const unsigned long ProcessMemoryLimit;

 private:
   CLimit m_Hard;
   CLimit m_Soft;
   mutable CUsage m_Usage;
   long m_ExitStatus;

 public:
	CJob(const CLimit& Soft, const CLimit& Hard);
   CJob(DWORD dwJobId);
	virtual ~CJob();

   virtual DWORD ActionEndOfJobTime();
   virtual DWORD ActionEndOfProcessTime();
   virtual DWORD ActionActiveProcessLimit();
   virtual DWORD ActionAbnormalExitProcess();
   virtual DWORD ActionJobMemoryLimit();
   virtual DWORD ActionProcessMemoryLimit();
   virtual DWORD ActionNewProcess();
   virtual DWORD ActionExitProcess();
   virtual DWORD ActionActiveProcessZero();

   virtual DWORD GetExitCode() const;
   virtual const CUsage& GetExtendedUsage() const;
};

}

#endif // !defined(AFX_JOB_H__C1C03F29_D185_47A5_9A97_644E99F791EF__INCLUDED_)
