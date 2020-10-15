#ifndef __CULL_LISTP_H
#define __CULL_LISTP_H
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

#include "cull/cull_list.h"
#include "uti/sge_bitfield.h"

#ifdef  __cplusplus
extern "C" {
#endif

/****** cull/list/-Cull-List-defines ***************************************
*
*  NAME
*     Cull-List-defines -- macros and constant definitions
*
*  SYNOPSIS
*     #define FREE_ELEM             (1<<0)
*     #define BOUND_ELEM            (1<<1)
*     #define TRANS_BOUND_ELEM      (1<<2)
*     #define OBJECT_ELEM           (1<<3)
*
*  FUNCTION
*     The following definitions describe possible values for the status 
*     a list element (lListElem):
*     FREE_ELEM        - a list element not being part of a list or
*                        being a sub object
*     BOUND_ELEM       - a list element contained in a list.
*     TRANS_BOUND_ELEM - temporary status while unpacking elements.
*                        After unpacking, bound elements or sub objects
*                        have this status to prevent errors from functions
*                        like lAppendElem, that reject bound objects.
*     OBJECT_ELEM      - a list element being subobject of another element
*
****************************************************************************
*/

#define FREE_ELEM             (1<<0)
#define BOUND_ELEM            (1<<1)
#define TRANS_BOUND_ELEM      (1<<2)
#define OBJECT_ELEM           (1<<3)

struct _lListElem {
   lListElem *next;             /* next lList element                        */
   lListElem *prev;             /* previous lList element                    */
   lUlong status;               /* status: element in list/ element free     */
   lDescr *descr;               /* pointer to the descriptor array           */
   lMultiType *cont;            /* pointer to the lMultiType array           */
   bitfield changed;            /* bitfield describing which fields have     */
                                /* changed since last spooling               */
};

struct _lList {
   int nelem;                   /* number of elements in the list            */
   char *listname;              /* name of the list                          */
   bool changed;                /* the list has been changed                 */
   lDescr *descr;               /* pointer to the descriptor array           */
   lListElem *first;            /* pointer to the first element of the list  */
   lListElem *last;             /* pointer to the last element of the list   */
};

#ifdef  __cplusplus
}
#endif

#endif /* __CULL_LISTP_H */
