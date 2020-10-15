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
#include <eh.h>

#include <iostream>

#include "CException.h"

using namespace std;

namespace GridEngine {

// System Exceptions

PCWSTR CException::m_szName = L"CException";
PCWSTR CFltException::m_szName = L"CFltException";
PCWSTR CFltDenormalOperandException::m_szName = L"CFltDenormalOperandException";
PCWSTR CFltDivideByZeroException::m_szName = L"CFltDivideByZeroException";
PCWSTR CFltInexactResultException::m_szName = L"CFltInexactResultException";
PCWSTR CFltInvalidOpertionException::m_szName = L"CFltInvalidOpertionException";
PCWSTR CFltOverflowException::m_szName = L"CFltOverflowException";
PCWSTR CFltStackCheckException::m_szName = L"CFltStackCheckException";
PCWSTR CFltUnderflowException::m_szName = L"CFltUnderflowException";
PCWSTR CIntException::m_szName = L"CIntException";
PCWSTR CIntDivideByZeroException::m_szName = L"CIntDivideByZeroException";
PCWSTR CIntOverflowException::m_szName = L"CIntOverflowException";
PCWSTR CMemoryException::m_szName = L"CMemoryException";
PCWSTR CMemoryPrivilegedInstructionException::m_szName = L"CMemoryPrivilegedInstructionException";
PCWSTR CMemoryStackOverflowException::m_szName = L"CMemoryStackOverflowException";
PCWSTR CMemoryAccessVialationException::m_szName = L"CMemoryAccessVialationException";
PCWSTR CMemoryDatatypeMisalignmentException::m_szName = L"CMemoryDatatypeMisalignmentException";
PCWSTR CArrayBoundExceededException::m_szName = L"CArrayBoundExceededException";
PCWSTR CInPageErrorException::m_szName = L"CInPageErrorException";
PCWSTR CIllegalInstructionException::m_szName = L"CIllegalInstructionException";
PCWSTR CGuardPageException::m_szName = L"CGuardPageException";
PCWSTR CExceptionException::m_szName = L"CExceptionException";
PCWSTR CNonContinuableException::m_szName = L"CNonContinuableException";
PCWSTR CInvalidDispositionException::m_szName = L"CInvalidDispositionException";
PCWSTR CInvalidHandleException::m_szName = L"CInvalidHandleException";
PCWSTR CDebugException::m_szName = L"CDebugException";
PCWSTR CSingleStepException::m_szName = L"CSingleStepException";
PCWSTR CBreakpointException::m_szName = L"CBreakpointException";

// Codine Exceptions
PCWSTR CCodineException::m_szName = L"CCodineException";

wostream& operator<<(wostream& wostr, CException& ex) {
	wcerr << L"Name:    " << ex.GetExceptionName() << endl;
	wcerr << L"Id:      " << (DWORD)ex << endl;
   wcerr << L"Thread:  " << GetCurrentThreadId() << endl;
   wcerr << L"File:    " << ex.GetFile() << endl;
   wcerr << L"Line:    " << ex.GetLine() << endl;
   wcerr << L"Message: " << ex.GetMessage() << endl;
   return wostr;
}

} // namespace
