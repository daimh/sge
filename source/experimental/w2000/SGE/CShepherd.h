// Shepherd.h: interface for the Shepherd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHEPHERD_H__E462562C_F6BD_48F4_9A0B_0350C8CBCA55__INCLUDED_)
#define AFX_SHEPHERD_H__E462562C_F6BD_48F4_9A0B_0350C8CBCA55__INCLUDED_
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

#include "stdafx.h"
#include "CConfiguration.h"
#include "CJob.h"

namespace GridEngine {

class CShepherd {
 public:
   enum ExitCodes {
      NoConfig = 3,
      ReadConfig = 5,
      ProcSetNotSet = 6,
      BeforeJob = 11,
      DiedThruSignal = 17,
      NoExitStatus = 19,
      UnknownJob = 21,
      Again = 25, 
      NoShell = 27,
      NoCwd = 28,
      FailureAfterJob = 100
   };

   static void WriteExitStatusFile(CString BaseDirectory, ExitCodes eExitCode);
 private:
   CConfiguration m_Configuration;
   CString m_BaseDirectory;
   CJob *m_Job;
   BOOL m_bNewJob;

 public:
	CShepherd(const CString& BaseDirectory);
	virtual ~CShepherd();

   virtual void Resume();
   virtual void Start();
   virtual void Suspend();
   virtual void Terminate(DWORD dwExitCode);
   virtual void WriteOsJobIdFile();

   virtual void WaitForObject();
};

} // namespace

#endif // !defined(AFX_SHEPHERD_H__E462562C_F6BD_48F4_9A0B_0350C8CBCA55__INCLUDED_)
