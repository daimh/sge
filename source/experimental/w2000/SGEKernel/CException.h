#if !defined(AFX_ERXCEPTION_H__DC8428EE_BBAC_4C8E_ABE4_A225F358CBEB__INCLUDED_)
#define AFX_EXCEPTION_H__DC8428EE_BBAC_4C8E_ABE4_A225F358CBEB__INCLUDED_
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
#include <iostream>
#include <shlwapi.h>

#include "CTrace.h"

using namespace std;

namespace GridEngine {

// System Exceptions

class CException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 private:
   DWORD m_dwType;
 private:
	EXCEPTION_RECORD m_ExceptionInformation;
	CONTEXT m_CpuContext;
 private:
   PWSTR m_szMessage;
   PWSTR m_szFile;
   int m_nLine;
 public:
	operator DWORD() {
      DWORD dwCode;

      if (m_dwType == 0) {
         dwCode = m_ExceptionInformation.ExceptionCode;
      } else {
         dwCode = 0;
      }
		return dwCode;
	}
   CException(PCWSTR Message) : m_dwType(0) {
      m_szMessage = StrDup(Message);
      m_szFile = L"";
      m_nLine = 0;
   }
   CException(PCWSTR szMessage, char *szFile, int nLine) : m_dwType(1) {
      WCHAR szBuffer[1024];

      m_szMessage = StrDup(szMessage);
      wsprintf(szBuffer, L"%S", szFile);
      m_szFile = StrDup(szBuffer);
      m_nLine = nLine;
   }
   CException(PEXCEPTION_POINTERS pException) :
		m_ExceptionInformation(*pException->ExceptionRecord),
		m_CpuContext(*pException->ContextRecord),
      m_dwType(1)
   {
      m_szMessage = L"none";
      m_szFile = L"none";
      m_nLine = 0;
	}
   virtual PCWSTR GetMessage() {
      return m_szMessage;
   }
   virtual PCWSTR GetFile() {
      return m_szFile;
   }
   virtual int GetLine() {
      return m_nLine;
   }

	friend wostream& operator<<(wostream& ostr, CException& ex);
};

class  CFltException : public CException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CFltException(PEXCEPTION_POINTERS pException):CException(pException) {
	}
};

class CFltDenormalOperandException : public CFltException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CFltDenormalOperandException(PEXCEPTION_POINTERS pException)
		:CFltException(pException) {
	}
};

class CFltDivideByZeroException : public CFltException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CFltDivideByZeroException(PEXCEPTION_POINTERS pException)
		:CFltException(pException) {
	}
};

class CFltInexactResultException : public CFltException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CFltInexactResultException(PEXCEPTION_POINTERS pException)
		:CFltException(pException) {
	}
};

class CFltInvalidOpertionException : public CFltException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CFltInvalidOpertionException(PEXCEPTION_POINTERS pException)
		:CFltException(pException) {
	}
};

class CFltOverflowException : public CFltException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CFltOverflowException(PEXCEPTION_POINTERS pException)
		:CFltException(pException) {
	}
};

class CFltStackCheckException : public CFltException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CFltStackCheckException(PEXCEPTION_POINTERS pException)
		:CFltException(pException) {
	}
};

class CFltUnderflowException : public CFltException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CFltUnderflowException(PEXCEPTION_POINTERS pException)
		:CFltException(pException) {
	}
};

class CIntException : public CException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CIntException(PEXCEPTION_POINTERS pException)
		:CException(pException) {
	}
};

class CIntDivideByZeroException : public CIntException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CIntDivideByZeroException(PEXCEPTION_POINTERS pException)
		:CIntException(pException) {
	}
};

class CIntOverflowException : public CIntException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CIntOverflowException(PEXCEPTION_POINTERS pException)
		:CIntException(pException) {
	}
};

class CMemoryException : public CException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CMemoryException(PEXCEPTION_POINTERS pException)
		:CException(pException) {
	}
};

class CMemoryPrivilegedInstructionException : public CMemoryException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CMemoryPrivilegedInstructionException(PEXCEPTION_POINTERS pException)
		:CMemoryException(pException) {
	}
};

class CMemoryStackOverflowException : public CMemoryException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CMemoryStackOverflowException(PEXCEPTION_POINTERS pException)
		:CMemoryException(pException) {
	}
};

class CMemoryAccessVialationException : public CMemoryException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CMemoryAccessVialationException(PEXCEPTION_POINTERS pException)
		:CMemoryException(pException) {
	}
};

class CMemoryDatatypeMisalignmentException : public CMemoryException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CMemoryDatatypeMisalignmentException(PEXCEPTION_POINTERS pException)
		:CMemoryException(pException) {
	}
};

class CArrayBoundExceededException : public CMemoryException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CArrayBoundExceededException(PEXCEPTION_POINTERS pException)
		:CMemoryException(pException) {
	}
};

class CInPageErrorException : public CMemoryException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CInPageErrorException(PEXCEPTION_POINTERS pException)
		:CMemoryException(pException) {
	}
};

class CIllegalInstructionException : public CMemoryException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CIllegalInstructionException(PEXCEPTION_POINTERS pException)
		:CMemoryException(pException) {
	}
};

class CGuardPageException : public CMemoryException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CGuardPageException(PEXCEPTION_POINTERS pException)
		:CMemoryException(pException) {
	}
};

class CExceptionException : public CException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CExceptionException(PEXCEPTION_POINTERS pException)
		:CException(pException) {
	}
};

class CNonContinuableException : public CExceptionException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CNonContinuableException(PEXCEPTION_POINTERS pException)
		:CExceptionException(pException) {
	}
};

class CInvalidDispositionException : public CExceptionException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CInvalidDispositionException(PEXCEPTION_POINTERS pException)
		:CExceptionException(pException) {
	}
};

class CInvalidHandleException : public CExceptionException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CInvalidHandleException(PEXCEPTION_POINTERS pException)
		:CExceptionException(pException) {
	}
};

class CDebugException : public CException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CDebugException(PEXCEPTION_POINTERS pException)
		:CException(pException) {
	}
};

class CSingleStepException : public CDebugException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CSingleStepException(PEXCEPTION_POINTERS pException)
		:CDebugException(pException) {
	}
};

class CBreakpointException : public CDebugException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
	CBreakpointException(PEXCEPTION_POINTERS pException)
		:CDebugException(pException) {
	}
};

wostream& operator<<(wostream& ostr, CException& ex);

// User Exceptions

class CSgeException : public CException {
 private:
   static PCWSTR m_szName;
   virtual PCWSTR GetExceptionName() {
      return m_szName;
   }
 public:
   CSgeException(PCWSTR szMessage)
		:CException(szMessage) {
      CTrace::Print(~0, L"%s: %s", GetExceptionName(), szMessage);
	}
   CSgeException(PCWSTR szMessage, char *szFile, int nLine)
      :CException(szMessage, szFile, nLine) {
      CTrace::Print(~0, L"Exception: %s; File: %S; Line: %d; Message: %s", 
         GetExceptionName(), szFile, nLine, szMessage);
   }
};

} // namespace

#endif // !defined(AFX_EXCEPTION_H__DC8428EE_BBAC_4C8E_ABE4_A225F358CBEB__INCLUDED_)
