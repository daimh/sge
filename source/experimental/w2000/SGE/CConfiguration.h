#if !defined(AFX_CONFIGURATION_H__EA5C69A3_8273_4ED1_A8EA_5D8DC8C0124F__INCLUDED_)
#define AFX_CONFIGURATION_H__EA5C69A3_8273_4ED1_A8EA_5D8DC8C0124F__INCLUDED_
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

#include <windows.h>
#include <map>
#include "CBasicTypes.h"
#include "CLimit.h"

using namespace std;

namespace GridEngine {

class CConfiguration {
 private:
   CString m_BaseDirectory;

 private:
   CLimit m_HardLimit;
   DWORD m_dwJaTaskId;
   DWORD m_dwJobArgs;
   DWORD m_dwOsJobId;
   map<DWORD, CString> m_JobArg;
   DWORD m_dwJobId;
   CString m_JobName;
   CString m_JobOwner;
   BOOL m_bMergeStdErr;
   DWORD m_dwProcessors;
   CString m_QueueTmpDir;
   CString m_ShellPath;
   CString m_ScriptFile;
   CString m_StdErrPath;
   CString m_StdOutPath;
   CLimit m_SoftLimit;
   BOOL m_ForbidReschedule;

 public:
   void ReadConfiguration();
   void ReadOsJobId();

 public:
	CConfiguration(const CString& BaseDirectory);
	virtual ~CConfiguration();

   virtual BOOL GetForbidReschedule();
   virtual CLimit& GetHardLimit();
   virtual DWORD GetJaTaskId();
   virtual DWORD GetJobId();
   virtual CString GetShellPath();
   virtual CString GetScriptFile();
   virtual CLimit& GetSoftLimit();
   virtual CString GetStdOutPath();
   virtual CString GetStdErrPath();
   virtual DWORD GetJobArgs();
   virtual CString GetJobArg(DWORD dwIndex);
   virtual CString GetJobName();
   virtual CString GetJobOwner();
   virtual BOOL GetMergeStdErr();
   virtual DWORD GetOsJobId();
   virtual DWORD GetProcessors();
   virtual CString GetQueueTmpDir();

   virtual void Print();
};

} // namespace

#endif // !defined(AFX_CONFIGURATION_H__EA5C69A3_8273_4ED1_A8EA_5D8DC8C0124F__INCLUDED_)
