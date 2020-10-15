#ifndef __CULL_WHEREP_H
#define __CULL_WHEREP_H
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


#include "cull/cull_where.h"
#include "cull/cull_multitypeP.h"

struct _lCondition {
   int op;                      /* operator of the condition                 */
   union {
      struct {
         int pos;               /* position in the cont/descr array          */
         int mt;                /* type of the compare value                 */
         int nm;                /* name of field                             */
         lMultiType val;        /* compare value                             */
      } cmp;
      struct {
         lCondition *first;     /* ptr to 1st operand                        */
         lCondition *second;    /* ptr to 2nd operand (if necessary)         */
      } log;
   } operand;
};


/* new data structure for dynamically buildable args for lWhere         */
/* the var_args model requires the knowledge of the fields at run time  */
struct _WhereArg {
   lDescr      *descriptor;
   int         field;
   lMultiType  value;
}; 

#endif /* __CULL_WHEREP_H */

