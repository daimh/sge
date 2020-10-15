#ifndef __CL_UTIL_H
#define __CL_UTIL_H
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

/*  #define U_LONG32_MAX 4294967295UL */

#include "basis_types.h"

/* This functions return the string length of the paramter */
int cl_util_get_ulong_number_length(unsigned long id);
int cl_util_get_int_number_length(int id);
int cl_util_get_double_number_length(double id);


/* This functions convert ascii text into number variables */
unsigned long cl_util_get_ulong_value(const char* text);

int cl_util_get_ascii_hex_buffer(unsigned char* buffer, unsigned long buf_len, char** ascii_buffer, char* separator);
int cl_util_get_binary_buffer(char* hex_buffer, unsigned char** buffer, unsigned long* buffer_lenght);
char cl_util_get_ascii_hex_char(unsigned char value);
int  cl_util_get_hex_value(char hex_char);

#endif /* __CL_UTIL_H */
