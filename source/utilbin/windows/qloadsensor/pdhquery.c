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

#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>

#include <stdio.h>
#include <math.h>

#include "pdhservice.h"
#include "pdhquery.h"
#include "simplelog.h"

DWORD
pdhquery_initialize(t_pdhquery *query)
{
   int ret = 0;
   LPCTSTR data_source = NULL;
   DWORD user_data = 1;
   PDH_STATUS pdh_ret = 0;

   DENTER("pdhquery_initialize");
   pdh_ret = PdhOpenQuery(data_source, user_data, &(query->query));
   if (pdh_ret != ERROR_SUCCESS) {
      DPRINTF(("PdhOpenQuery failed with %x\n", pdh_ret));
      ret = 1;
   }
   DEXIT;
   return ret;
}

DWORD 
pdhquery_free(t_pdhquery *query)
{
   DWORD ret = 0;

   DENTER("pdhquery_free");
   PdhCloseQuery(query->query);
   DEXIT;
   return ret;
}

DWORD 
pdhquery_add_counterset(t_pdhquery *query, t_pdhcounterset *counterset) 
{
   DWORD ret = 0;
   LPSTR exp_list = NULL;
   PDH_STATUS pdh_ret;

   DENTER("pdhquery_add_counterset");
   /* 
    * Problem: PdhExpandCounterPath() sometimes fills more then 'length' Bytes,
    * so it writes to unallocated memory. It seems that the functions expands
    * the last path even if there is not enough space left to expand this path.
    * A possible solution (I'm not quite sure) could be:
    * Make buffer PDH_MAX_COUNTER_PATH Bytes larger than 'length", so the 
    * function does not write to unallocated memory.
    */
   /*
    * Expand all wildcard characters
    */
   {
      DWORD length = 0;

      do {
         if (length == 0) {
            /* Allocate buffer for min. 3 strings */
            length = PDH_MAX_COUNTER_PATH*3;
            exp_list = (LPSTR) malloc(length);
         } else {
            length *= 2;
            exp_list = (LPSTR) realloc(exp_list, length);
         }
         /* Pretend buffer could hold only 2 strings to work around 
          * bug in PdhExpandCounterPath()
          */
         length -= PDH_MAX_COUNTER_PATH;
         pdh_ret = PdhExpandCounterPath(counterset->wild_counter_path, 
                                        exp_list, &length);
      } while(pdh_ret == PDH_MORE_DATA);
   }

   /*
    * Add the expanded counter names to the query
    */
   if (pdh_ret == PDH_CSTATUS_VALID_DATA) {
      LPSTR name = NULL;
      DWORD j;

      /* how many elements are in the expanded list */
      for (name = exp_list, j = 0; 
           *name != '\0'; 
           name += lstrlen(name) + 1, j++) {
         ;
      }
      counterset->number_of_counters = j;

      counterset->counter_handles = (HCOUNTER*) malloc(sizeof(HCOUNTER) * j);
      counterset->pdh_name = (TXCHAR*) malloc(sizeof(TXCHAR) * j);
      if (counterset->counter_handles != NULL) {
         for (name = exp_list, j = 0; 
              *name != '\0'; 
              name += lstrlen(name) + 1, j++) {
            lstrcpy((LPSTR)counterset->pdh_name[j], name);
            pdh_ret = PdhAddCounter(query->query, name, j, 
                                    &(counterset->counter_handles[j]));
            if (pdh_ret != ERROR_SUCCESS) {
               DPRINTF(("PdhAddCounter failed with %x\n", pdh_ret));
               ret = 3;
               break;
            }
         }            
      } else {
         DPRINTF(("malloc failed\n"));
         ret = 2;
      }
   } else {
      DPRINTF(("PdhExpandCounterPath failed with %x\n", pdh_ret));
      ret = 1;
   }
   free(exp_list);
   DEXIT;
   return ret;
}

DWORD 
pdhquery_remove_counterset(t_pdhquery *query, t_pdhcounterset *counterset) 
{
   DWORD ret = 0;
   DWORD j;

   DENTER("pdhquery_remove_counterset");
   for (j = 0; j < counterset->number_of_counters; j++) {
      PdhRemoveCounter(counterset->counter_handles[j]);
   }
   free(counterset->counter_handles);
   counterset->counter_handles = NULL;
   free(counterset->pdh_name);
   counterset->pdh_name = NULL;
   DEXIT;
   return ret;
}

DWORD
pdhquery_update(t_pdhquery *query) 
{
   DWORD ret = 0;
   int pdh_ret;

   DENTER("pdhquery_update");
   /*
    * "Prime" counters that need two values to display a formatted value.
    */
   pdh_ret = PdhCollectQueryData(query->query);
   if (pdh_ret != ERROR_SUCCESS) {
      DPRINTF(("PdhCollectQueryData failed with %x\n", pdh_ret));
      ret = 1;
   }

   /*
    * Wait some time. Code which was executed bofore this line might
    * influence certain load values (e.g. cpu usage) therefore we have
    * two wait some time before we collect the data.
    */
   Sleep(100);

   /*
    * Collect current data
    */
   pdh_ret = PdhCollectQueryData(query->query);
   if (pdh_ret != ERROR_SUCCESS) {
      DPRINTF(("PdhCollectQueryData failed with %x\n", pdh_ret));
      ret = 1;
   }
   DEXIT;
   return ret;
} 

/*
 * Translate the object counter and instance names into the language
 * of the installed OS. Store the result in an element of values[] 
 * because we need it later on.
 */
DWORD 
pdhcounterset_initialize(t_pdhcounterset *counterset, 
                         LPSTR object, LPSTR instance, LPSTR counter)
{
   DWORD ret = 0;
   LPSTR trans_object   = NULL;
   LPSTR trans_counter  = NULL;
   LPSTR trans_instance = NULL;

   DENTER("pdhcounterset_initialize");

   /* store untranslated values */
   strcpy(counterset->object, object);
   strcpy(counterset->instance, instance);
   strcpy(counterset->counter, counter);

   /* translate them */
   if(pdhservice_translate(object, counter, instance, TRUE,
      &trans_object, &trans_counter, &trans_instance)==FALSE) {
      ret = 1;
      DEXIT;
      return ret;
   }

   /* store translation */
   strcpy(counterset->trans_object, trans_object);
   strcpy(counterset->trans_counter, trans_counter);
   if(trans_instance != NULL) {   
      strcpy(counterset->trans_instance, trans_instance);
   } else {
      strcpy(counterset->trans_instance, "");
   }

   /* create full wildcard counter path */
   if (trans_instance != NULL) {
      sprintf(counterset->wild_counter_path, "\\%s(%s)\\%s", 
              trans_object, trans_instance, trans_counter);
   } else {
      sprintf(counterset->wild_counter_path, "\\%s\\%s", 
              trans_object, trans_counter);
   }
   DEXIT;
   return ret;
}
   
DWORD 
pdhcounterset_free(t_pdhcounterset *counterset)
{
   DWORD ret = 0;

   DENTER("pdhcounterset_free");
   DEXIT;
   return ret;
}


