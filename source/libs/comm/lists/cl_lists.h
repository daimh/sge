#ifndef __CL_LISTS_H
#define __CL_LISTS_H

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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#if !defined(_WIN32)

/* this is the main header file for lib ngclists */
#include <pthread.h>

#include "comm/lists/cl_errors.h"
#include "comm/lists/cl_list_types.h"
#include "comm/lists/cl_raw_list.h"
#include "comm/lists/cl_thread.h"
#include "comm/lists/cl_thread_list.h"
#include "comm/lists/cl_log_list.h"
#include "comm/lists/cl_string_list.h"

#else
/* windows */
#define CL_LOG(log_type, log_text)              /* cl_log_list_log(log_type, __LINE__ , __CL_FUNCTION__ ,__FILE__ , log_text, NULL) */
#define CL_LOG_STR(log_type, log_text, log_str) /* cl_log_list_log(log_type, __LINE__ , __CL_FUNCTION__ ,__FILE__ , log_text, log_str )*/
#define CL_LOG_INT(log_type, log_text, log_str) /* cl_log_list_log_int(log_type, __LINE__ , __CL_FUNCTION__ ,__FILE__ , log_text, log_str )*/
#define CL_LOG_STR_STR_INT(log_type, log_text, log_str1, log_str2, log_str3)
#endif

#endif /* __CL_LISTS_H */

