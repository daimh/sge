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

#ifndef __SIMPLELOG_H
#define __SIMPLELOG_H

#include "win32_type.h"

extern DWORD error_id;
extern TXCHAR error_message;

#define DENTER(function)                    \
   static const char SGE_FUNC[] = function; \
   error_printf("-> %s\n", function);                                       

#define DEXIT                               \
   error_printf("<- %s\n", SGE_FUNC);    

#define DPRINTF(msg)                        \
   error_printf("   ");                     \
   error_printf msg

void
error_set(DWORD id, LPSTR message);

void
error_print(void);

int 
error_printf(const char *fmt, ...);

void
error_set_tracingenabled(BOOL new_state);

int
error_open_tracefile(TXCHAR tracefile);

int
error_close_tracefile(void);

#endif
