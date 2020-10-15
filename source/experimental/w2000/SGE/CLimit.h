// Limit.h: interface for the Limit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LIMIT_H__7A2E74CD_7643_45EC_858E_3B62F731DA4D__INCLUDED_)
#define AFX_LIMIT_H__7A2E74CD_7643_45EC_858E_3B62F731DA4D__INCLUDED_
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

namespace GridEngine {

class CLimit {
public:
   static const ULONGLONG nNanosecondsPerSecond;
   static const ULONGLONG nMillisecondsPerSecond;
   static const ULONGLONG nNanosecondsPerMillisecond;

private:
   JOBOBJECT_EXTENDED_LIMIT_INFORMATION m_ExtLimitInfo;

public:
	CLimit();
   CLimit(const CLimit& Limit);
	virtual ~CLimit();

   virtual const JOBOBJECT_EXTENDED_LIMIT_INFORMATION& GetExtendedLimitInformation() const;
   virtual void SetExtendedLimitInformation(const JOBOBJECT_EXTENDED_LIMIT_INFORMATION& Limit);

   virtual void SetProcess(DWORD nProcesses);
   virtual void SetJobUserTime(ULONGLONG lnUserTime);
   virtual void SetProcessUserTime(ULONGLONG lnUserTime);
   virtual void SetProcessorSet(DWORD Processors);
   virtual void SetProcessMemory(SIZE_T nMemory);
   virtual void SetJobMemory(SIZE_T nMemory);
   virtual void SetWorkingSetSize(DWORD dwMin, DWORD dwMax);
   virtual void SetPriority(DWORD dwPriority);
   virtual void SetSchedulingClass(DWORD dwPriority);

   virtual DWORD GetProcess();
   virtual ULONGLONG GetJobUserTime();
   virtual ULONGLONG GetProcessUserTime();
   virtual DWORD GetProcessorSet();
   virtual SIZE_T GetProcessMemory();
   virtual SIZE_T GetJobMemory();
   virtual DWORD GetWorkingSetSizeMin();
   virtual DWORD GetWorkingSetSizeMax();
   virtual DWORD GetPriority();
   virtual DWORD GetSchedulingClass();

   virtual BOOL IsSetProcess();
   virtual BOOL IsSetJobUserTime();
   virtual BOOL IsSetProcessUserTime();
   virtual BOOL IsSetProcessorSet();
   virtual BOOL IsSetProcessMemory();
   virtual BOOL IsSetJobMemory();
   virtual BOOL IsSetWorkingSetSize();
   virtual BOOL IsSetPriority();
   virtual BOOL IsSetSchedulingClass();

   virtual void UnSetAll();
   virtual void UnSetProcess();
   virtual void UnSetJobUserTime();
   virtual void UnSetProcessUserTime();
   virtual void UnSetProcessorSet();
   virtual void UnSetProcessMemory();
   virtual void UnSetJobMemory();
   virtual void UnSetWorkingSetSize();
   virtual void UnSetPriority();
   virtual void UnSetSchedulingClass();
   
};

} // namespace

#endif // !defined(AFX_LIMIT_H__7A2E74CD_7643_45EC_858E_3B62F731DA4D__INCLUDED_)
