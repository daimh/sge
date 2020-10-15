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

#include "sgeobj/sge_proc.h"

static lList *procList;

/****** sge_proc/get_pr() ******************************************************
*  NAME
*     get_pr() -- Look for a certain process entry in the proc table 
*
*  SYNOPSIS
*     lListElem* get_pr(int pid) 
*
*  FUNCTION
*     Looks for the element with the specified pid and return it.
*     Otherwise return NULL
*
*  INPUTS
*     int pid - The process ID of the process we're looking for. 
*
*  RESULT
*     lListElem* - PRO_Type object or NULL
*******************************************************************************/
lListElem *get_pr (int pid)
{
   if (!procList) {
      gen_procList ();
      return NULL;
   }

   return lGetElemUlong(procList, PRO_pid, pid);
}

void append_pr (lListElem *pr)
{
   if (!procList) {
      gen_procList ();
   }
   lAppendElem(procList, pr);
}

/****** sge_proc/gen_procList() ************************************************
*  NAME
*     gen_procList() -- creates the proc table 
*
*  SYNOPSIS
*     void gen_procList() 
*
*  FUNCTION
*     Creates the hashed list procList
*******************************************************************************/
void gen_procList()
{
#ifdef LINUX
   procList = lCreateListHash("procList", PRO_Type, true);
#endif
}

/****** sge_proc/free_procList() ***********************************************
*  NAME
*     free_procList() -- frees the formerly created procList 
*
*  SYNOPSIS
*     void free_procList() 
*
*  FUNCTION
*     Frees the formerly created procList
*******************************************************************************/
void free_procList()
{
#ifdef LINUX
   lFreeList(&procList);
#endif
}

/****** sge_proc/clean_procList() **********************************************
*  NAME
*     clean_procList() -- cleans the procList from already finished jobs 
*
*  SYNOPSIS
*     void clean_procList() 
*
*  FUNCTION
*     Remove all elements from procList which has not been marked as running.
*     Mark all remaining elements as not running.
*******************************************************************************/
void clean_procList()
{
#ifdef LINUX
   lListElem *next = NULL;
   lListElem *ep = NULL;
   lCondition *cp = lWhere("%T(%I == %b)", PRO_Type, PRO_run, false); 
   int pos = lGetPosInDescr(PRO_Type, PRO_run);

   next = lFindFirst(procList, cp);

   /* free all finished jobs */

   while (next != NULL) {
      ep = lFindNext(next, cp);
      lRemoveElem(procList, &next);
      next = ep;
   }

   lFreeWhere(&cp);

   /* mark all jobs to finished */

   for_each(next, procList) {
      lSetPosBool(next, pos, false);
   }
#endif
}
