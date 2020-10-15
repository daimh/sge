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

#include "uti/sge_rmon_monitoring_level.h"


/* *************************************************************** 

   NAME
   mlcheck() checks consistency of a monitoring level 

   RETURN VALUE
   0    consistency error
   1    ok

   int rmon_mlcheck( m )
   monitoring_level *m;
   {
   int j;

   for (j=0; j<N_LAYER; j++)  
   if (NO_LEVEL
   }
 */


/* *************************************************************** 

   NAME
   rmon_mliszero() tests if the monitoring level is zero 

 */
int rmon_mliszero(
monitoring_level *m 
) {
   int j;


   for (j = 0; j < N_LAYER; j++)
      if (m->ml[j] != 0) {
         return 0;
      }


   return 1;
}


/* *************************************************************** 

   NAME
   rmon_mlcpy() performs

   d(estination) = s(ource)

 */

void rmon_mlcpy(
monitoring_level *d,
monitoring_level *s 
) {
   int j;

   for (j = 0; j < N_LAYER; j++)
      d->ml[j] = s->ml[j];

   return;
}

/* *************************************************************** 

   NAME
   rmon_mlclr() performs

   d(estination) = 0

 */

void rmon_mlclr(
monitoring_level *d 
) {
   int j;


   for (j = 0; j < N_LAYER; j++)
      d->ml[j] = 0;


   return;
}


/* *************************************************************** 

   NAME
   rmon_mlgetl() returns the bitmask for layer i 

 */
u_long rmon_mlgetl(
monitoring_level *s,
int i 
) {
   return s->ml[i];
}

/* *************************************************************** 

   NAME
   rmon_mlputl() sets the bitmask for layer i 

 */
void rmon_mlputl(
monitoring_level *s,
int i,
u_long mask 
) {
   s->ml[i] = mask;
}

