#ifndef __ERR_TRACE_H
#define __ERR_TRACE_H
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

#include "sge.h"                /* for __attribute__ */

typedef int (*tShepherd_trace)(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));

extern int foreground;      /* != 0 if we can write to stderr/out     */

void shepherd_trace_init(void);
void shepherd_trace_exit(void);
void shepherd_trace_chown(const char* job_owner);

void shepherd_error_init(void);
void shepherd_error_exit(void);
void shepherd_error_chown(const char* job_owner);

int  shepherd_trace(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));
void shepherd_error(int do_exit, const char *format, ...)
  __attribute__ ((format (printf, 2, 3)));
void shepherd_error_ptr(const char *text);
void shepherd_write_exit_status( const char *exit_status );

int  is_shepherd_trace_fd( int fd );
int  count_exit_status(void);

#endif /* __ERR_TRACE_H */

