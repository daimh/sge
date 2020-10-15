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
#include <stdio.h>

#include "uti/sge_rmon.h"

#include "sgeobj/sge_range.h"

#include "sge_range_schedd.h"

/* ---------------------------------------

NAME 

   num_in_range

DESCR

   checks if a number is inside a range
   a non existent range (NULL) is seen as 1-1

RETURNS

   0 if the number is not inside the range
  
*/
u_long32 num_in_range(
u_long32 num,
lList *r 
) {
   lListElem *rep;
   u_long32 ret;

   DENTER(TOP_LAYER, "num_in_range");

   if (!r) {
      /* no ranges ==> we need to fit exactly 1 and we're sure we can */
      DEXIT;
      return MIN(num, 1);
   }

   for_each (rep, r) {
      if (num >= (ret=lGetUlong(rep,RN_max))) {
         /* We've more than we need */
         DEXIT;
         return ret;
      }
      else if (num < lGetUlong(rep,RN_min))
         /* We've to few for the range ==> next */
         continue;
      else {
         /* We're somewhere within the range */
         DEXIT;
         return num;
      }
   }

   /* it's a pitty - we simply don't have enough */
   DEXIT;
   return 0;
}
