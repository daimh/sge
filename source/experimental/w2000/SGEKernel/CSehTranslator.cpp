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
#include <shlwapi.h>
#include <eh.h>
#include <iostream>

#include "CSehTranslator.h"
#include "CException.h"

using namespace std;

namespace GridEngine {

void _cdecl TranslateSeToCe(UINT nExceptionCode, PEXCEPTION_POINTERS pExceptionInformation) {
	switch ((*pExceptionInformation->ExceptionRecord).ExceptionCode) {

	  // Floating point exceptions
     case EXCEPTION_FLT_DENORMAL_OPERAND:
		throw CFltDenormalOperandException(pExceptionInformation);
	  case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		throw CFltDivideByZeroException(pExceptionInformation);
	  case EXCEPTION_FLT_INEXACT_RESULT:
		throw CFltInexactResultException(pExceptionInformation);
	  case EXCEPTION_FLT_INVALID_OPERATION:
		throw CFltInvalidOpertionException(pExceptionInformation);
	  case EXCEPTION_FLT_OVERFLOW:
		throw CFltOverflowException(pExceptionInformation);
	  case EXCEPTION_FLT_STACK_CHECK:
		throw CFltStackCheckException(pExceptionInformation);
	  case EXCEPTION_FLT_UNDERFLOW:
		throw CFltUnderflowException(pExceptionInformation);

	  // Integer Exceptions
	  case EXCEPTION_INT_DIVIDE_BY_ZERO:
		throw CIntDivideByZeroException(pExceptionInformation);
	  case EXCEPTION_INT_OVERFLOW:
		throw CIntOverflowException(pExceptionInformation);

	  // Memory Exceptions
	  case EXCEPTION_PRIV_INSTRUCTION:
		throw CMemoryPrivilegedInstructionException(pExceptionInformation);
	  case EXCEPTION_STACK_OVERFLOW:
		throw CMemoryStackOverflowException(pExceptionInformation);
	  case EXCEPTION_ACCESS_VIOLATION:
		throw CMemoryAccessVialationException(pExceptionInformation);
	  case EXCEPTION_DATATYPE_MISALIGNMENT:
		throw CMemoryDatatypeMisalignmentException(pExceptionInformation);
	  case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		throw CArrayBoundExceededException(pExceptionInformation);
	  case EXCEPTION_IN_PAGE_ERROR:
		throw CInPageErrorException(pExceptionInformation);
	  case EXCEPTION_ILLEGAL_INSTRUCTION:
		throw CIllegalInstructionException(pExceptionInformation);
	  case EXCEPTION_GUARD_PAGE:
		throw CGuardPageException(pExceptionInformation);

	  // Debug Exceptions
	  case EXCEPTION_BREAKPOINT:
		throw CBreakpointException(pExceptionInformation);
	  case EXCEPTION_SINGLE_STEP:
		throw CSingleStepException(pExceptionInformation);

	  // Exception Exceptions
	  case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		throw CNonContinuableException(pExceptionInformation);
	  case EXCEPTION_INVALID_DISPOSITION:
		throw CInvalidDispositionException(pExceptionInformation);
	  case EXCEPTION_INVALID_HANDLE:
		throw CInvalidHandleException(pExceptionInformation);

	  // Are there other Exception ???
	  default:
		throw CException(pExceptionInformation);
	}
}

}; // namespace
