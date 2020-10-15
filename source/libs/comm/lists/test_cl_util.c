
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

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>

#include <limits.h>

#include "comm/lists/cl_util.h"

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
int main(void) {
  unsigned long i = 0; 
  int back = 0;
  int i2 = 0;

  printf("\ntest cl_util_get_ulong_number_length()\n");

  printf("MAX_ULONG: %lu\n", ULONG_MAX);
/* INT_MAX , ULONG_MAX */
  back = cl_util_get_ulong_number_length(i);
  printf("min: %lu last back=%d\n",i,back);
  i--;
  back = cl_util_get_ulong_number_length(i);
  printf("max: %lu last back=%d\n",i,back);
  i=1000;
  back = cl_util_get_ulong_number_length(i);
  printf("max: %lu last back=%d\n",i,back);

  printf("\ntest cl_util_get_int_number_length()\n");

  back = cl_util_get_int_number_length(i2);
  printf("min: %d last back=%d\n",i2,back);
  i2--;
  back = cl_util_get_int_number_length(i2);
  printf("max: %d last back=%d\n",i2,back);
  i2=1000;
  back = cl_util_get_int_number_length(i2);
  printf("max: %d last back=%d\n",i2,back);

  printf("\ntest cl_util_get_ulong_value()\n");
  i = cl_util_get_ulong_value("4294967295");
  printf("value: %lu for string \"%s\"\n",i,"4294967295");

  i = cl_util_get_ulong_value("1000");
  printf("value: %lu for string \"%s\"\n",i,"1000");

  i = cl_util_get_ulong_value("-1");
  printf("value: %lu for string \"%s\"\n",i,"-1");


  printf("main done\n");
  return 0;
}


