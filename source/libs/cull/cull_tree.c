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
#include <stdlib.h>
#include <string.h>

/* do not compile in monitoring code */
#ifndef NO_SGE_COMPILE_DEBUG
#define NO_SGE_COMPILE_DEBUG
#endif

#include "uti/sge_rmon.h"

#include "cull/cull.h"

/****** cull/tree/lGetNumberOfNodes() *****************************************
*  NAME
*     lGetNumberOfNodes() -- Number of elements and subelements 
*
*  SYNOPSIS
*     int lGetNumberOfNodes(const lListElem *ep, const lList *lp, int nm) 
*
*  FUNCTION
*     Returns the number of elements and subelements in the sublist 'nm' 
*     of the element 'ep' (lp = NULL) or returns the sum of all elements
*     and subelements within the list 'lp' (ep = NULL)
*
*  INPUTS
*     const lListElem *ep - element 
*     const lList *lp     - list 
*     int nm              - field name id within element 
*
*  RESULT
*     int - number of elements
*******************************************************************************/
int lGetNumberOfNodes(const lListElem *ep, const lList *lp, int nm) 
{
   int n = 0;

   DENTER(CULL_LAYER, "lGetNumberOfNodes");

   if (ep) {
      int pos;

      n = 1;

      if ((pos = lGetPosViaElem(ep, nm, SGE_NO_ABORT)) >= 0 && mt_get_type(ep->descr[pos].mt) == lListT) {
         if ((lp = lGetPosList(ep, pos)))
            n += lGetNumberOfNodes(NULL, lp, nm);
      }
      DEXIT;
      return n;
   } else {
      for_each(ep, lp) {
         n += lGetNumberOfNodes(ep, NULL, nm);
      }
      DEXIT;
      return n;
   }
}

/****** cull/tree/lGetNumberOfLeafs() *****************************************
*  NAME
*     lGetNumberOfLeafs() -- Returns the number of leaves 
*
*  SYNOPSIS
*     int lGetNumberOfLeafs(const lListElem *ep, const lList *lp, int nm) 
*
*  FUNCTION
*     Returns the number of leaves 
*
*  INPUTS
*     const lListElem *ep - element 
*     const lList *lp     - list 
*     int nm              - field name if within ep 
*
*  RESULT
*     int - number of leaves 
******************************************************************************/
int lGetNumberOfLeafs(const lListElem *ep, const lList *lp, int nm) 
{
   int n = 0;

   DENTER(CULL_LAYER, "lGetNumberOfLeafs");

   if (ep) {
      int pos;

      if ((pos = lGetPosViaElem(ep, nm, SGE_NO_ABORT)) >= 0 && mt_get_type(ep->descr[pos].mt) == lListT) {
         if (!(lp = lGetPosList(ep, pos)))
            n = 1;
         else
            n = lGetNumberOfLeafs(NULL, lp, nm);
      }
      DEXIT;
      return n;
   }
   else {
      for_each(ep, lp) {
         n += lGetNumberOfLeafs(ep, NULL, nm);
      }
      DEXIT;
      return n;
   }
}
