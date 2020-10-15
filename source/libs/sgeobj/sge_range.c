
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
#include <ctype.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_dstring.h"
#include "uti/sge_prog.h"
#include "uti/sge_string.h"

#include "sched/sge_sched.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "sge.h"

#define RANGE_SEPARATOR_CHARS ","
#define RANGE_LAYER BASIS_LAYER


static bool range_is_overlapping(const lListElem *range1,
                                 const lListElem *range2);

/* we keep a descending sorted list of non overlapping ranges */
/* MT-NOTE: expand_range_list() is MT safe */
static void expand_range_list(lListElem *r, lList **rl)
{
   u_long32 rmin, rmax, rstep;
   lListElem *ep, *rr;

   DENTER(TOP_LAYER, "expand_range_list");

   rmin = lGetUlong(r, RN_min);
   rmax = lGetUlong(r, RN_max);
   rstep = lGetUlong(r, RN_step);

   /* create list */
   if (!*rl) {
      *rl = lCreateList("ranges", RN_Type);
   }

   if (lGetNumberOfElem(*rl) != 0) {
      ep = lFirst(*rl);
      while (ep) {

         if (rstep != lGetUlong(ep, RN_step) || rstep > 1 ||
             lGetUlong(ep, RN_step) > 1) {
            lInsertElem(*rl, NULL, r);
            break;
         } else if (rmin > lGetUlong(ep, RN_max)) {

            /* 
             ** r and ep are non-overlapping and r is to be
             ** sorted ahead of ep.
             */
            lInsertElem(*rl, NULL, r);
            break;

         } else if (rmax < lGetUlong(ep, RN_min)) {
            /* 
             ** r and ep are non-overlapping and r is to be
             ** sorted after ep ==> go for the next iteration
             */
            ep = lNext(ep);
            if (!ep) {
               /* We are at the end of the list ==> append r */
               lAppendElem(*rl, r);
            }

            continue;

         } else if ((rmax <= lGetUlong(ep, RN_max)) &&
                    (rmin >= lGetUlong(ep, RN_min))) {

            /* 
             ** r is fully contained within ep ==> delete it 
             */
            lFreeElem(&r);
            break;

         } else if (rmin < lGetUlong(ep, RN_min)) {

            /* 
             ** ep is either fully contained within r
             ** or r overlaps ep at ep's bottom ==>
             ** overwrite ep and see if further list elements
             ** are covered also;
             */
            if (rmax > lGetUlong(ep, RN_max)) {
               /* r contains ep */
               lSetUlong(ep, RN_max, rmax);
            }
            lSetUlong(ep, RN_min, rmin);

            /* we're already sure we can free r */
            lFreeElem(&r);
            rr = ep;
            ep = lNext(ep);
            while (ep) {
               if (rmin < lGetUlong(ep, RN_min)) {

                  /* it also contains this one */
                  r = ep;
                  ep = lNext(ep);
                  lRemoveElem(*rl, &r);

               } else if (rmin <= lGetUlong(ep, RN_max)) {

                  /* these overlap ==> glue them */
                  lSetUlong(rr, RN_min, lGetUlong(ep, RN_min));

                  r = ep;
                  ep = lNext(ep);
                  lRemoveElem(*rl, &r);
                  break;

               } else
                  /* they are non overlapping */
                  break;

               ep = lNext(ep);
            }

            /* we're now sure we inserted it */
            break;
         } else if (rmax > lGetUlong(ep, RN_max)) {
            /*
             ** r and ep overlap and r starts above ep
             ** ==> glue them
             */
            lSetUlong(ep, RN_max, rmax);
            lFreeElem(&r);
            break;
         }
      }
   } else {
      lAppendElem(*rl, r);
   }

   DRETURN_VOID;
}

/****** sgeobj/range/range_correct_end() **************************************
*  NAME
*     range_correct_end() -- correct end of a range element 
*
*  SYNOPSIS
*     static void range_correct_end(lListElem *this_range) 
*
*  FUNCTION
*     This function modifies the the 'end' id of the 'this_range' 
*     element if it is not correct. After the modification the 
*     'end' id is the last valid id which is part of the range. 
*
*  INPUTS
*     lListElem *this_range - RN_Type 
*
*  RESULT
*     'this_range' will be modified 
*
*  EXAMPLE
*     1-6:2 (1,3,5) will be modified to 1-5:2 
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*******************************************************************************/
void range_correct_end(lListElem *this_range)
{
   DENTER(RANGE_LAYER, "range_correct_end");
   if (this_range != NULL) {
      u_long32 start, end, step;

      range_get_all_ids(this_range, &start, &end, &step);
      if (step > 0) {
         if ((end - start) % step) {
            u_long32 factor;

            factor = (end - start) / step;
            end = start + factor * step;
            range_set_all_ids(this_range, start, end, step);
         }
      } else {
         step = end - start;
      }
      range_set_all_ids(this_range, start, end, step);
   }
   DRETURN_VOID;
}

/****** sgeobj/range/range_is_overlapping() ***********************************
*  NAME
*     range_is_overlapping() -- Do two ranges interleave? 
*
*  SYNOPSIS
*     static bool range_is_overlapping(const lListElem *this_elem, 
*                                      const lListElem *range) 
*
*  FUNCTION
*     True will be returned when the given ranges interleave. This
*     does not necessaryly mean that certain ids exist in both ranges. 
*
*  INPUTS
*     const lListElem *this_elem - RN_Type 
*     const lListElem *range - RN_Type 
*
*  RESULT
*     static bool - false or true 
*
*  EXAMPLE
*     1-5:3    4-10:7      => true 
*     1-5:3    5-10:6      => true 
*     1-5:3    6-10:4      => false 
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*
*  NOTES
*     MT-NOTE: range_is_overlapping() is MT safe
*******************************************************************************/
static bool range_is_overlapping(const lListElem *this_elem,
                                 const lListElem *range)
{
   bool ret = false;

   DENTER(RANGE_LAYER, "range_is_overlapping");
   if (this_elem != NULL && range != NULL) {
      u_long32 start1, end1, step1;
      u_long32 start2, end2, step2;

      range_get_all_ids(this_elem, &start1, &end1, &step1);
      range_get_all_ids(range, &start2, &end2, &step2);
      if (end1 >= start2) {
         ret = true;
      }
   }
   DRETURN(ret);
}

/****** sgeobj/range/range_list_initialize() **********************************
*  NAME
*     range_list_initialize() -- (Re)initialize a range list 
*
*  SYNOPSIS
*     void range_list_initialize(lList **this_list, 
*                                lList **answer_list) 
*
*  FUNCTION
*     'this_list' will be created if it does not exist. If it already
*     exists all elements contained in this list will be removed. 
*
*  INPUTS
*     lList **this_list  - Pointer to a RN_Type-list 
*     lList **answer_list - Pointer to a AN_Type-list or NULL
*
*  RESULT
*     *this_list will be an empty RN_Type list
*     *answer_list may contain error messages 
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*******************************************************************************/
void range_list_initialize(lList **this_list, lList **answer_list)
{
   DENTER(RANGE_LAYER, "range_list_initialize");
   if (this_list != NULL) {
      if (*this_list != NULL) {
         lListElem *range;
         lListElem *next_range;

         /* 
          * EB: CLEANUP: implement a new CULL function lEmptyList(lList *list)
          */

         next_range = lFirst(*this_list);
         while ((range = next_range)) {
            next_range = lNext(range);

            lRemoveElem(*this_list, &range);
         }
      } else {
         *this_list = lCreateList("", RN_Type);
         if (*this_list == NULL) {
            answer_list_add(answer_list, "unable to create range list",
                            STATUS_ERROR1, ANSWER_QUALITY_ERROR);
         }
      }
   }
   DRETURN_VOID;
}

/****** sgeobj/range/range_list_get_number_of_ids() ***************************
*  NAME
*     range_list_get_number_of_ids() -- Determines the number of ids 
*
*  SYNOPSIS
*     u_long32 range_list_get_number_of_ids(const lList *this_list) 
*
*  FUNCTION
*     This function determines the number of ids contained 
*     in 'this_list'. If 'this_list' is NULL then 0 will be returned.
*
*  INPUTS
*     const lList *this_list - RN_Type list 
*
*  RESULT
*     u_long32 - number of ids
*
*  EXAMPLE
*     1-5:2, 7-10:3, 20-23:1 (1, 3, 5, 7, 10, 20, 21, 22, 23) => 9
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*
*  NOTE
*     MT-NOTE: range_list_get_number_of_ids() is MT safe
*******************************************************************************/
u_long32 range_list_get_number_of_ids(const lList *this_list)
{
   u_long32 ret = 0;
   lListElem *range;

   DENTER(RANGE_LAYER, "range_list_get_number_of_ids");
   for_each(range, this_list) {
      ret += range_get_number_of_ids(range);
   }
   DRETURN(ret);
}

/****** sgeobj/range/range_get_number_of_ids() ********************************
*  NAME
*     range_get_number_of_ids() -- Number of ids within a range
*
*  SYNOPSIS
*     u_long32 range_get_number_of_ids(const lList *this_elem) 
*
*  FUNCTION
*     This function determines the number of ids contained 
*     in 'this_elem' 
*
*  INPUTS
*     const lList *this_elem - RN_Type element 
*
*  RESULT
*     u_long32 - number of ids
*
*  EXAMPLE
*     1-5:2 (1, 3, 5)   => 3
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*******************************************************************************/
u_long32 range_get_number_of_ids(const lListElem *this_elem)
{
   u_long32 start, end, step, ret;

   DENTER(RANGE_LAYER, "range_get_number_of_ids");
   range_get_all_ids(this_elem, &start, &end, &step);
   ret = 1 + (end - start) / step;
   DRETURN(ret);
}

/****** sgeobj/range/range_list_print_to_string() *****************************
*  NAME
*     range_list_print_to_string() -- Print range list into the string 
*
*  SYNOPSIS
*     void range_list_print_to_string(const lList *this_list, 
*                                     dstring *string, bool ignore_step,
*                                     bool comma_as_separator) 
*
*  FUNCTION
*     Print all ranges given in 'this_list' into the dynamic 'string'.
*     If 'this_list' is NULL then the word "UNDEFINED" will be added
*     to 'string'. If 'ignore_step' is 'true' then the stepsize of all
*     ranges will be suppressed.
*
*  INPUTS
*     const lList *this_list     - RN_Type 
*     dstring *string            - dynamic string 
*     bool ignore_step           - ignore step for printing
*     bool comma_as_separator    - use the format 1,2,3 instead of 1-2:
*     bool print_always_as_range - even if the range has only one id
*
*  RESULT
*     string will be modified
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*******************************************************************************/
void
range_list_print_to_string(const lList *this_list,
                           dstring *string, bool ignore_step,
                           bool comma_as_separator, bool print_always_as_range)
{
   DENTER(RANGE_LAYER, "range_list_print_to_string");
   if (string != NULL) {
      if (this_list != NULL) {
         lListElem *range;

         for_each(range, this_list) {
            u_long32 start, end, step;

            range_get_all_ids(range, &start, &end, &step);
            range_to_dstring(start, end, step, string, ignore_step, 
                             comma_as_separator, print_always_as_range);
         }
      } else {
         sge_dstring_append(string, "UNDEFINED");
      }
   }
   DRETURN_VOID;
}

/****** sgeobj/range/range_list_get_first_id() ********************************
*  NAME
*     range_list_get_first_id() -- First id contained in the list
*
*  SYNOPSIS
*     u_long32 range_list_get_first_id(const lList *range_list, 
*                                      lList **answer_list) 
*
*  FUNCTION
*     The first id of the first range element of the list will 
*     be returned. If 'range_list' is NULL or empty 0 will be
*     returned and 'answer_list' will be filled with an error 
*     message.
*
*  INPUTS
*     const lList *range_list - RN_Type list  
*     lList **answer_list     - Pointer to an AN_Type list 
*
*  RESULT
*     u_long32 - First id or 0
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*     sgeobj/range/range_list_get_last_id()
*
*  NOTES
*     MT-NOTE: range_list_get_first_id() is MT safe

******************************************************************************/
u_long32 range_list_get_first_id(const lList *range_list, lList **answer_list)
{
   u_long32 start = 0;
   lListElem *range = NULL;

   DENTER(RANGE_LAYER, "range_list_get_first_id");
   range = lFirst(range_list);
   if (range) {
      u_long32 end, step;

      range_get_all_ids(range, &start, &end, &step);
   } else {
      answer_list_add(answer_list, "range_list contains no elements",
                      STATUS_ERROR1, ANSWER_QUALITY_ERROR);
   }
   DRETURN(start);
}

/****** sgeobj/range/range_list_get_last_id() *********************************
*  NAME
*     range_list_get_last_id() -- Returns last id contained in the list
*
*  SYNOPSIS
*     u_long32 range_list_get_last_id(const lList *range_list, 
*                                    lList **answer_list) 
*
*  FUNCTION
*     The last id of the last range element of the list will be 
*     returned. If 'range_list' is NULL or empty 0 will be
*     returned and 'answer_list' will be filled with an error
*     message. 
*
*  INPUTS
*     const lList *range_list - RN_Type list  
*     lList **answer_list     - Pointer to an AN_Type list 
*
*  RESULT
*     u_long32 - Last id or 0
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*     sgeobj/range/range_list_get_first_id()
******************************************************************************/
u_long32 range_list_get_last_id(const lList *range_list, lList **answer_list)
{
   u_long32 end = 0;
   lListElem *range = NULL;

   DENTER(RANGE_LAYER, "range_list_get_last_id");
   range = lLast(range_list);
   if (range) {
      u_long32 start, step;
      range_get_all_ids(range, &start, &end, &step);
   } else {
      answer_list_add(answer_list, "range_list contains no elements",
                      STATUS_ERROR1, ANSWER_QUALITY_ERROR);
   }
   DRETURN(end);
}

/****** sgeobj/range/range_list_get_average() *********************************
*  NAME
*     range_list_get_average() -- Return average of all numbers in range.
*
*  SYNOPSIS
*     double range_list_get_average(const lList *this_list) 
*
*  FUNCTION
*     The average of all numbers in the range is returned. For an empty 
*     range 0 is returned.
*
*  INPUTS
*     const lList *this_list - RN_Type list  
*     u_long32 upperbound    - This is used as range upperbound if non-0
*
*  RESULT
*     double - the average
*
*  NOTES
*     MT-NOTES: range_list_get_average() is MT safe
*******************************************************************************/
double range_list_get_average(const lList *this_list, u_long32 upperbound)
{
   lListElem *range;
   double sum = 0.0;
   u_long32 id, min, max, step;
   int n = 0;

   for_each (range, this_list) {
      range_get_all_ids(range, &min, &max, &step);
      if (upperbound != 0) {
         max = MIN(max, upperbound);
      }   
      for (id=min; id<=max; id+= step) {
         sum += id;
         n++;
      }
   }
   return (n > 0) ? (sum/n) : 0;
}

/******asgeobj/range/range_list_sort_uniq_compress() **************************
*  NAME
*     range_list_sort_uniq_compress() -- makes lists fit as a fiddle 
*
*  SYNOPSIS
*     void range_list_sort_uniq_compress(lList *range_list, 
*                                        lList **answer_list) 
*
*  FUNCTION
*     After a call to this function 'range_list' fulfills 
*     following conditions:
*        (1) all ids are in ascending order
*        (2) each id is contained in the list only once
*        (3) ids are grouped so that a min of range elements exist 
*
*  INPUTS
*     lList *range_list   - RN_Type list 
*     lList **answer_list - Pointer to an AN_Type list 
*     bool correct_end    - if true, range_list is treated as id enumeration
*
*  RESULT
*     range_list will be modified 
*
*  EXAMPLE
*     12-12:7,1-7:1,3-5:2,14-16:2   => 1-7:1,12-16:2
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*
*  NOTES
*     MT-NOTE: range_list_sort_uniq_compress() is MT safe
******************************************************************************/
void range_list_sort_uniq_compress(lList *range_list, lList **answer_list, bool correct_end)
{
   DENTER(RANGE_LAYER, "range_list_sort_uniq_compress");
   if (range_list) {
      lListElem *range1, *next_range1;
      lListElem *range2, *next_range2;
      lList *tmp_list;

      /*
       * Sort the incomming stuff
       */
      lPSortList(range_list, "%I+", RN_min);

      /* 
       * Remove overlapping ranges
       */
      tmp_list = lCreateList("", RN_Type);
      if (tmp_list) {
         for (next_range1 = lFirst(range_list); (range1 = next_range1); next_range1 = lNext(range1)) {
            next_range2 = lNext(next_range1);
            if (correct_end)
               range_correct_end(range1);
            while ((range2 = next_range2)) {
               next_range2 = lNext(range2);
               if (correct_end)
                  range_correct_end(range2);
               if (range_is_overlapping(range1, range2)) {
                  range2 = lDechainElem(range_list, range2);
                  lAppendElem(tmp_list, range2);
               } else {
                  break;
               }
            }
         }

         /*
          * Insert all removed entries at the correct position
          */
         for_each(range1, tmp_list) {
            u_long32 start1, end1, step1;

            range_get_all_ids(range1, &start1, &end1, &step1);
            for (; start1 <= end1; start1 += step1) {
               range_list_insert_id(&range_list, answer_list, start1);
            }
         }

         lFreeList(&tmp_list);

         /*
          * Join sequenced ranges
          */
         range_list_compress(range_list);
      } else {
         answer_list_add(answer_list, "unable to create range list",
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      }
   }
   DRETURN_VOID;
}

/****** sgeobj/range/range_list_compress() ************************************
*  NAME
*     range_list_compress() -- Joins sequenced ranges within a list 
*
*  SYNOPSIS
*     void range_list_compress(lList *range_list) 
*
*  FUNCTION
*     Consecutive ranges within the list will be joined by this 
*     function. Following pre-conditions have to be fulfilled, so 
*     that this function works correctly:
*        (1) ids have to be in ascending order
*        (2) Only the first/last id of a range may be contained
*            in the predecessor/successor range
*
*  INPUTS
*     lList *range_list - RN_Type list 
*
*  RESULT
*     'range_list' will be modified 
*
*  EXAMPLE
*     1-3:1,4-5:1,6-8:2,8-10:2   => 1-5:1,6-10:2 
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*
*  NOTES
*     MT-NOTE: range_list_compress() is MT safe
******************************************************************************/
void range_list_compress(lList *range_list)
{
   DENTER(RANGE_LAYER, "range_list_compress");
   if (range_list != NULL) {
      lListElem *range1 = NULL;
      lListElem *range2 = NULL;
      lListElem *next_range1 = lFirst(range_list);
      lListElem *next_range2 = lNext(next_range1);

      while ((range1 = next_range1) && (range2 = next_range2)) {
         u_long32 start1, end1, step1;
         u_long32 start2, end2, step2;

         range_get_all_ids(range1, &start1, &end1, &step1);
         range_get_all_ids(range2, &start2, &end2, &step2);
         if (end1 + step1 == start2 && step1 == step2) {
            end1 = end2;
            step1 = step2;
            range_set_all_ids(range1, start1, end1, step1);
            lRemoveElem(range_list, &range2);
            range2 = NULL;
            next_range1 = range1;
         } else if (start1 == end1 && step1 == 1 && end1 == start2 - step2) {
            end1 = end2;
            step1 = step2;
            range_set_all_ids(range1, start1, end1, step1);
            lRemoveElem(range_list, &range2);
            range2 = NULL;
            next_range1 = range1;
         } else if (start2 == end2 && step2 == 1 && end1 + step1 == end2) {
            end1 = end2;
            range_set_all_ids(range1, start1, end1, step1);
            lRemoveElem(range_list, &range2);
            range2 = NULL;
            next_range1 = range1;
         } else if (start1 == end1 && start2 == end2 && step1 == step2 &&
                    step1 == 1) {
            end1 = start2;
            step1 = end1 - start1;
            range_set_all_ids(range1, start1, end1, step1);
            lRemoveElem(range_list, &range2);
            range2 = NULL;
            next_range1 = range1;
         } else {
            next_range1 = lNext(range1);
         }
         next_range2 = lNext(next_range1);
      }
   }
   DRETURN_VOID;
}

/****** sgeobj/range/range_list_is_id_within() ********************************
*  NAME
*     range_list_is_id_within() -- Is id contained in range list? 
*
*  SYNOPSIS
*     bool 
*     range_list_is_id_within(const lList *range_list, u_long32 id) 
*
*  FUNCTION
*     True is returned by this function if 'id' is part of at least
*     one range element of 'range_list' 
*
*  INPUTS
*     const lList *range_list - RN_Type list
*     u_long32 id             - id 
*
*  RESULT
*     bool - true or false 
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*
*  NOTES
*     MT-NOTE: range_list_is_id_within() is MT safe
*******************************************************************************/
bool range_list_is_id_within(const lList *range_list, u_long32 id)
{
   lListElem *range = NULL;
   bool ret = false;

   DENTER(RANGE_LAYER, "range_list_is_id_within");
   for_each(range, range_list) {
      if (range_is_id_within(range, id)) {
         ret = true;
         break;
      }
   }
   DRETURN(ret);
}

/****** sgeobj/range/range_list_containes_id_less_than() **********************
*  NAME
*     range_list_containes_id_less_than() -- has id less than x 
*
*  SYNOPSIS
*     bool range_list_containes_id_less_than(const lList *range_list, 
*                                            u_long32 id) 
*
*  FUNCTION
*     Is at least one id in the "range_list" less than "id" 
*
*  INPUTS
*     const lList *range_list - RN_Type list 
*     u_long32 id             - number 
*
*  RESULT
*     bool - true or false
*******************************************************************************/
bool range_list_containes_id_less_than(const lList *range_list, u_long32 id)
{
   lListElem *range = NULL;
   bool ret = false;

   DENTER(RANGE_LAYER, "range_list_containes_id_less_than");
   for_each(range, range_list) {
      if (range_containes_id_less_than(range, id)) {
         ret = true;
         break;
      }
   }
   DRETURN(ret);
}

/****** sgeobj/range/range_list_is_empty() ************************************
*  NAME
*     range_list_is_empty() -- check if id lists containes ids 
*
*  SYNOPSIS
*     bool range_list_is_empty(const lList *range_list) 
*
*  FUNCTION
*     Returns true if "range_list" containes no ids. 
*
*  INPUTS
*     const lList *range_list - RN_Type list 
*
*  RESULT
*     bool - true or false
*
*  SEE ALSO
*     sgeobj/range/RN_Type
******************************************************************************/
bool range_list_is_empty(const lList *range_list)
{
   return (range_list_get_number_of_ids(range_list) == 0 ? true : false);
}

/****** sgeobj/range/range_containes_id_less_than() ***************************
*  NAME
*     range_containes_id_less_than() -- is one id less than given id 
*
*  SYNOPSIS
*     bool 
*     range_containes_id_less_than(const lListElem *range, u_long32 id) 
*
*  FUNCTION
*     This function tests if at least one id in "range" is less 
*     than "id" 
*
*  INPUTS
*     const lListElem *range - RN_Type element 
*     u_long32 id            - number 
*
*  RESULT
*     bool - true or false
******************************************************************************/
bool range_containes_id_less_than(const lListElem *range, u_long32 id)
{
   bool ret = false;

   DENTER(RANGE_LAYER, "range_containes_id_less_than");
   if (range) {
      u_long32 start, end, step;

      range_get_all_ids(range, &start, &end, &step);
      if (start < id) {
         ret = true;
      }
   }
   DRETURN(ret);
}

/****** sgeobj/range/range_is_id_within() *************************************
*  NAME
*     range_is_id_within() -- Is id contained in range? 
*
*  SYNOPSIS
*     bool range_is_id_within(const lListElem *range, u_long32 id) 
*
*  FUNCTION
*     True is returned by this function if 'id' is part of 'range' 
*
*  INPUTS
*     const lListElem *range - RN_Type element 
*     u_long32 id            - id 
*
*  RESULT
*     bool - true or false
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*
*  NOTES
*     MT-NOTE: range_is_id_within() is MT safe
******************************************************************************/
bool range_is_id_within(const lListElem *range, u_long32 id)
{
   bool ret = false;

   DENTER(RANGE_LAYER, "range_is_id_within");
   if (range) {
      u_long32 start, end, step;

      range_get_all_ids(range, &start, &end, &step);
      if (id >= start && id <= end && ((id - start) % step) == 0) {
         ret = true;
      }
   }
   DRETURN(ret);
}

/****** sgeobj/range/range_list_remove_id() ***********************************
*  NAME
*     range_list_remove_id() -- remove an id from a range list 
*
*  SYNOPSIS
*     void 
*     range_list_remove_id(lList **range_list, lList **answer_list, 
*                          u_long32 id) 
*
*  FUNCTION
*     'id' will be removed from 'range_list'. 
*
*  INPUTS
*     lList **range_list  - pointer to a RN_Type list 
*     lList **answer_list - pointer to a AN_Type list 
*     u_long32 id         - new id 
*
*  RESULT
*     range_list and answer_list may be modified 
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
******************************************************************************/
void range_list_remove_id(lList **range_list, lList **answer_list, u_long32 id)
{
   lListElem *range = NULL;

   DENTER(RANGE_LAYER, "range_list_remove_id");
   if (range_list != NULL && *range_list != NULL) {
      lListElem *next_range = lFirst(*range_list);

      while ((range = next_range) != NULL) {
         u_long32 start, end, step;

         next_range = lNext(range);
         range_get_all_ids(range, &start, &end, &step);
         if (id >= start && id <= end && ((id - start) % step) == 0) {
            if ((id == start) && ((id == end) || (id + step > end))) {
               lRemoveElem(*range_list, &range);
               break;
            } else if (id == start) {
               start += step;
               range_set_all_ids(range, start, end, step);
               break;
            } else if (id == end) {
               end -= step;
               range_set_all_ids(range, start, end, step);
               break;
            } else {
               lListElem *new_range = lCreateElem(RN_Type);

               if (new_range != NULL) {
                  range_set_all_ids(range, start, id - step, step);
                  range_set_all_ids(new_range, id + step, end, step);
                  lInsertElem(*range_list, range, new_range);
               } else {
                  answer_list_add(answer_list, "unable to split range element",
                                  STATUS_ERROR1, ANSWER_QUALITY_ERROR);
               }
               break;
            }
         }
      }
      if (lGetNumberOfElem(*range_list) == 0) {
         lFreeList(range_list);
      }
   }
   DRETURN_VOID;
}

/****** sgeobj/range/range_list_move_first_n_ids() ****************************
*  NAME
*     range_list_move_first_n_ids() -- split a range list 
*
*  SYNOPSIS
*     void range_list_move_first_n_ids(lList **range_list, 
*                                      lList **answer_list, 
*                                      lList **range_list2, 
*                                      u_long32 n) 
*
*  FUNCTION
*     The first 'n' ids within 'range_list' will be moved into 
*     'range_list2'. Error messages may be found in 'answer_list' 
*
*  INPUTS
*     lList **range_list  - pointer to a RN_Type list (source) 
*     lList **answer_list - pointer to an AN_Type list 
*     lList **range_list2 - pointer to a RN_Type list (destination) 
*     u_long32 n          - number of ids 
*
*  RESULT
*     range_list, range_list2, answer_list may be modified 
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
******************************************************************************/
void range_list_move_first_n_ids(lList **range_list, lList **answer_list,
                                 lList **range_list2, u_long32 n)
{
   DENTER(RANGE_LAYER, "range_list_move_first_n_ids");
   if (range_list && *range_list && range_list2) {
      lListElem *range = NULL;
      u_long32 id;

      for_each(range, *range_list) {
         for (id = lGetUlong(range, RN_min);
              id <= lGetUlong(range, RN_max);
              id += lGetUlong(range, RN_step)) {
            range_list_insert_id(range_list2, answer_list, id);
            range_list_compress(*range_list2);
            n--;
            if (n == 0) {
               break;
            }
         }
      }
      for_each(range, *range_list2) {
         for (id = lGetUlong(range, RN_min);
              id <= lGetUlong(range, RN_max);
              id += lGetUlong(range, RN_step)) {
            range_list_remove_id(range_list, answer_list, id);
         }
      }
   }
   DRETURN_VOID;
}

/****** sgeobj/range/range_list_insert_id() ***********************************
*  NAME
*     range_list_insert_id() -- insert an id into a range list 
*
*  SYNOPSIS
*     void range_list_insert_id(lList **range_list, lList **answer_list, 
*                               u_long32 id) 
*
*  FUNCTION
*     'id' will be inserted into 'range_list'. 
*
*  INPUTS
*     lList **range_list  - pointer to a RN_Type list 
*     lList **answer_list - pointer to a AN_Type list 
*     u_long32 id         - new id 
*
*  NOTES
*     It may be possible that 'id' is multiply contained in 'range_list' 
*     after using this function. Use range_list_compress() or 
*     range_list_sort_uniq_compress() to eliminate them.
*
*  RESULT
*     range_list and answer_list may be modified 
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*     sgeobj/range/range_list_compress()
*     sgeobj/range/range_list_sort_uniq_compress()
*
*  NOTES
*     MT-NOTE: range_list_insert_id() is MT safe
******************************************************************************/
void range_list_insert_id(lList **range_list, lList **answer_list, u_long32 id)
{
   lListElem *range, *prev_range, *next_range;
   int inserted = 0;

   DENTER(RANGE_LAYER, "range_insert_id");

   lPSortList(*range_list, "%I+", RN_min);

   range = NULL;
   if (*range_list == NULL) {
      *range_list = lCreateList("task_id_range", RN_Type);
      if (*range_list == NULL) {
         answer_list_add(answer_list, "unable to insert id into range",
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      }
   }
   prev_range = lLast(*range_list);
   while ((next_range = range, range = prev_range)) {
      u_long32 start, end, step;
      u_long32 prev_start, prev_end, prev_step;
      u_long32 next_start, next_end, next_step;

      prev_range = lPrev(range);
      range_get_all_ids(range, &start, &end, &step);

      /* 1 */
      if (id < end) {
         if (prev_range) {
            continue;
         } else {
            next_range = range;
            range = prev_range;
            prev_range = NULL;
         }
      }

      if (next_range) {
         range_get_all_ids(next_range, &next_start, &next_end, &next_step);
      }
      if (prev_range) {
         range_get_all_ids(prev_range, &prev_start, &prev_end, &prev_step);
      }

      /* 2 */
      if (next_range && id > next_start) {
         if (((id - next_start) % next_step == 0)) {
            /* id is already part of the range */
            inserted = 1;
         } else {
            lListElem *new_range1, *new_range2;
            u_long32 factor, prev_id, next_id;

            factor = ((id - next_start) / next_step);
            prev_id = next_start + factor * next_step;
            next_id = next_start + (factor + 1) * next_step;
            range_set_all_ids(next_range, next_start, prev_id, next_step);
            new_range1 = lCreateElem(RN_Type);
            range_set_all_ids(new_range1, id, id, 1);
            lInsertElem(*range_list, next_range, new_range1);
            new_range2 = lCreateElem(RN_Type);
            range_set_all_ids(new_range2, next_id, next_end, next_step);
            lInsertElem(*range_list, new_range1, new_range2);
            inserted = 1;
         }
      } else {
         if ((range && (end == id)) || (next_range && (next_start == id))) {
            /* id is already part of the range */
            inserted = 1;
         } else if (range && (end + step == id)) {
            /* 3 */
            end = id;
            range_set_all_ids(range, start, end, step);
            inserted = 1;
         } else if (next_range && (next_start - next_step == id)) {
            /* 4 */
            next_start = id;
            range_set_all_ids(next_range, next_start, next_end, next_step);
            inserted = 1;
         } else {
            lListElem *new_range;

            /* 5 */
            new_range = lCreateElem(RN_Type);
            range_set_all_ids(new_range, id, id, 1);
            lInsertElem(*range_list, range, new_range);
            inserted = 1;
         }
      }
      if (inserted) {
         break;
      }
   }
   if (!inserted) {
      lListElem *new_range;

      new_range = lCreateElem(RN_Type);
      range_set_all_ids(new_range, id, id, 1);
      lAppendElem(*range_list, new_range);
      inserted = 1;
   }
   DRETURN_VOID;
}

/****** sgeobj/range/range_get_all_ids() **************************************
*  NAME
*     range_get_all_ids() -- reads 'start', 'end' and 'step' 
*
*  SYNOPSIS
*     void range_get_all_ids(const lListElem *range, u_long32 *min, 
*                            u_long32 *max, u_long32 *step) 
*
*  FUNCTION
*     Reads 'min' (start), 'max' (end) and 'step' from a range element.
*     If 'range' is NULL then 'min', 'max' and 'step' will be set to 0.
*
*  INPUTS
*     const lListElem *range - range element of type RN_Type 
*     u_long32 *min          - start value 
*     u_long32 *max          - end value 
*     u_long32 *step         - step size 
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*
*  NOTES
*     MT-NOTE: range_get_all_ids() is MT safe
******************************************************************************/
void range_get_all_ids(const lListElem *range, u_long32 *min, u_long32 *max,
                       u_long32 *step)
{
   DENTER(RANGE_LAYER, "range_get_all_ids");
   if (min != NULL && max != NULL && step != NULL) {
      if (range != NULL) {
         *min = lGetUlong(range, RN_min);
         *max = lGetUlong(range, RN_max);
         *step = lGetUlong(range, RN_step);
      } else {
         *min = *max = *step = 0;
      }
   }
   DRETURN_VOID;
}

/****** sgeobj/range/range_set_all_ids() **************************************
*  NAME
*     range_set_all_ids() -- writes 'start', 'end' and 'step' 
*
*  SYNOPSIS
*     void range_set_all_ids(lListElem *range, u_long32 min, 
*                            u_long32 max, u_long32 step) 
*
*  FUNCTION
*     Writes 'min' (start), 'max' (end) and 'step' into a range element 
*
*  INPUTS
*     lListElem *range - range element of type RN_Type 
*     u_long32 min     - start value 
*     u_long32 max     - end value 
*     u_long32 step    - step size 
*
*  NOTES
*     Step values will be nomalized. (e.g. 1-1:3 => 1-1:1)
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
*
*  NOTES
*     MT-NOTE: range_set_all_ids() is MT safe
******************************************************************************/
void range_set_all_ids(lListElem *range, u_long32 min, u_long32 max,
                       u_long32 step)
{
   DENTER(RANGE_LAYER, "range_set_all_ids");
   if (range != NULL) {
      lSetUlong(range, RN_min, min);
      lSetUlong(range, RN_max, max);
      if (min != max) {
         lSetUlong(range, RN_step, step);
      } else {
         lSetUlong(range, RN_step, 1);
      }
   }
   DRETURN_VOID;
}

/****** sgeobj/range/range_list_calculate_union_set() *************************
*  NAME
*     range_list_calculate_union_set() -- Union set of two range lists 
*
*  SYNOPSIS
*     void range_list_calculate_union_set(lList **range_list, 
*                                         lList **answer_list, 
*                                         const lList *range_list1, 
*                                         const lList *range_list2) 
*
*  FUNCTION
*     All ids contained in 'range_list1' and 'range_list2' will be 
*     contained in 'range_list' after a call of this function.
*      
*
*  INPUTS
*     lList **range_list       - pointer to union set RN_Type list 
*     lList **answer_list      - pointer to AN_Type list 
*     const lList *range_list1 - first source RN_Type list 
*     const lList *range_list2 - second source RN_Type list 
*
*  RESULT
*     range_list and answer_list may be modified 
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
******************************************************************************/
void range_list_calculate_union_set(lList **range_list,
                                    lList **answer_list,
                                    const lList *range_list1,
                                    const lList *range_list2)
{
   DENTER(RANGE_LAYER, "range_list_calculate_union_set");
   if (range_list != NULL && (range_list1 != NULL || range_list2 != NULL)) {
      lFreeList(range_list);

      if (range_list1 != NULL) {
         *range_list = lCopyList("", range_list1);
      } else {
         *range_list = lCopyList("", range_list2);
      }
      if (*range_list == NULL) {
         DTRACE;
         goto error;
      }

      range_list_sort_uniq_compress(*range_list, answer_list, true);
      if (answer_list_has_error(answer_list)) {
         DTRACE;
         goto error;
      }

      if (range_list1 != NULL && range_list2 != NULL) {
         lListElem *range2 = NULL;

         for_each(range2, range_list2) {
            u_long32 start2, end2, step2;

            range_get_all_ids(range2, &start2, &end2, &step2);
            for (; start2 <= end2; start2 += step2) {
               range_list_insert_id(range_list, answer_list, start2);
            }
         }
         range_list_compress(*range_list);
      }
   }
   DRETURN_VOID;

 error:
   lFreeList(range_list);
   answer_list_add(answer_list, "unable to calculate union set",
                   STATUS_ERROR1, ANSWER_QUALITY_ERROR);
   DRETURN_VOID;
}

/****** sgeobj/range/range_list_calculate_difference_set() ********************
*  NAME
*     range_list_calculate_difference_set() -- Difference set list 
*
*  SYNOPSIS
*     void range_list_calculate_difference_set(lList **range_list, 
*                                              lList **answer_list, 
*                                              const lList *range_list1, 
*                                              const lList *range_list2) 
*
*  FUNCTION
*     'range_list' will contain all ids part of 'range_list1' but not
*     contained in 'range_list2' 
*
*  INPUTS
*     lList **range_list       - pointer to result RN_Type list 
*     lList **answer_list      - pointer to AN_Type list 
*     const lList *range_list1 - first source RN_Type list 
*     const lList *range_list2 - second source RN_Type list 
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
******************************************************************************/
void range_list_calculate_difference_set(lList **range_list,
                                         lList **answer_list,
                                         const lList *range_list1,
                                         const lList *range_list2)
{
   DENTER(RANGE_LAYER, "range_list_calculate_difference_set");
   if (range_list != NULL && range_list1 != NULL) {
      lFreeList(range_list);
      *range_list = lCopyList("difference_set range list", range_list1);
      if (*range_list == NULL) {
         goto error;
      }

      range_list_sort_uniq_compress(*range_list, answer_list, true);
      if (answer_list_has_error(answer_list)) {
         goto error;
      }

      if (range_list2 != NULL) {
         lListElem *range2 = NULL;

         for_each(range2, range_list2) {
            u_long32 start2, end2, step2;

            range_get_all_ids(range2, &start2, &end2, &step2);
            for (; start2 <= end2; start2 += step2) {
               range_list_remove_id(range_list, answer_list, start2);
               if (answer_list_has_error(answer_list)) {
                  goto error;
               }
            }
         }
         range_list_compress(*range_list);
      }
   }
   DRETURN_VOID;

 error:
   lFreeList(range_list);
   answer_list_add(answer_list, "unable to calculate union set",
                   STATUS_ERROR1, ANSWER_QUALITY_ERROR);
   DRETURN_VOID;
}

/****** sgeobj/range/range_list_calculate_intersection_set() ******************
*  NAME
*     range_list_calculate_intersection_set() -- Intersection set 
*
*  SYNOPSIS
*     void range_list_calculate_intersection_set(lList **range_list, 
*                                          lList **answer_list, 
*                                          const lList *range_list1, 
*                                          const lList *range_list2) 
*
*  FUNCTION
*     'range_list' will contain all ids which are contained in 
*     'range_list1' and also in 'range_list2'.
*
*  INPUTS
*     lList **range_list       - pointer to result RN_Type list 
*     lList **answer_list      - pointer to AN_Type list 
*     const lList *range_list1 - first source RN_Type list 
*     const lList *range_list2 - second source RN_Type list 
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
******************************************************************************/
void range_list_calculate_intersection_set(lList **range_list,
                                           lList **answer_list,
                                           const lList *range_list1,
                                           const lList *range_list2)
{
   DENTER(RANGE_LAYER, "range_list_calculate_intersection_set");
   lFreeList(range_list);
   if (range_list1 && range_list2) {
      lListElem *range;

      for_each(range, range_list1) {
         u_long32 start, end, step;

         range_get_all_ids(range, &start, &end, &step);
         for (; start <= end; start += step) {
            if (range_list_is_id_within(range_list2, start)) {
               lListElem *new_range;

               if (*range_list == NULL) {
                  *range_list = lCreateList("", RN_Type);
                  if (*range_list == NULL) {
                     goto error;
                  }
               }
               new_range = lCreateElem(RN_Type);
               if (new_range == NULL) {
                  goto error;
               }
               range_set_all_ids(new_range, start, start, 1);
               lAppendElem(*range_list, new_range);
            }
         }
      }
      range_list_compress(*range_list);
   }
   DRETURN_VOID;

 error:
   lFreeList(range_list);
   answer_list_add(answer_list, "unable to calculate intersection set",
                   STATUS_ERROR1, ANSWER_QUALITY_ERROR);
   DRETURN_VOID;
}

/****** sgeobj/range/range_to_dstring() **************************************
*  NAME
*     range_to_dstring() -- Appends a range to a dynamic string 
*
*  SYNOPSIS
*     void range_to_dstring(u_long32 start, 
*                                   u_long32 end, 
*                                   int step, 
*                                   dstring *dyn_taskrange_str) 
*
*  FUNCTION
*     Appends a range to a dynamic string.
*
*  INPUTS
*     u_long32 start              - min id 
*     u_long32 end                - max id 
*     int step                    - step size 
*     dstring *dyn_taskrange_str  - dynamic string 
*     int ignore_step             - ignore step for output
*     bool use_comma_as_separator - use a comma instead of '-' and ':' for separation
*     bool print_always_as_range - even if the range has only one id
*
*  SEE ALSO
*     sgeobj/range/RN_Type 
******************************************************************************/
void range_to_dstring(u_long32 start, u_long32 end, int step,
                      dstring * dyn_taskrange_str, int ignore_step,
                      bool use_comma_as_separator, bool print_always_as_range)
{
   char tail[256] = "";
   char to_char = '-'; 
   char step_char = ':';

   if (use_comma_as_separator) {
      to_char = ',';
      step_char = ',';
   }
   if (dyn_taskrange_str->length > 0) {
      sge_dstring_append(dyn_taskrange_str, ",");
   }

   if (start == end && !print_always_as_range) {
      snprintf(tail, sizeof(tail), sge_u32, start);
   } else if (start == end && print_always_as_range) {
      snprintf(tail, sizeof(tail), sge_u32 "%c" sge_u32, start, to_char, end);
   } else if (start + step == end) {
      snprintf(tail, sizeof(tail), sge_u32 "," sge_u32, start, end);
   } else {
      if (ignore_step) {
         snprintf(tail, sizeof(tail), sge_u32 "%c" sge_u32, start, to_char, end);
      } else {
         snprintf(tail, sizeof(tail), sge_u32 "%c" sge_u32 "%c%d",
                  start, to_char, end, step_char, step);
      }
   }
   sge_dstring_append(dyn_taskrange_str, tail);
}

/* MT-NOTE: range_parse_from_string() is MT safe */
void range_parse_from_string(lListElem **range,
                             lList **answer_list,
                             const char *rstr,
                             int step_allowed, int inf_allowed)
{
   const char *old_str;
   char *dptr;
   u_long32 rmin, rmax, ldummy, step = 1;
   lListElem *r;
   char msg[MAX_STRING_SIZE];

   DENTER(TOP_LAYER, "range_parse_from_string");

   old_str = rstr;

   if (!strcasecmp(rstr, "UNDEFINED")) {
      *range = NULL;
      DRETURN_VOID;
   }
   r = lCreateElem(RN_Type);

   rmin = rmax = 0;
   if (rstr[0] == '-') {
      /* rstr e.g. is "-<n>" ==> min=1 */
      rmin = 1;
      rstr++;
      if (*rstr == '\0') {
         if (inf_allowed) {
            /* rstr is just "-" <==> "1-inf" */
            lSetUlong(r, RN_min, rmin);
            lSetUlong(r, RN_max, RANGE_INFINITY);
            *range = r;
            DRETURN_VOID;
         } else {
            *range = NULL;
            DRETURN_VOID;
         }
      }
   }

   /* rstr should point to a decimal now */
   ldummy = strtol(rstr, &dptr, 10);
   if ((ldummy == 0) && (rstr == dptr)) {
      snprintf(msg, sizeof(msg), MSG_GDI_INITIALPORTIONSTRINGNODECIMAL_S, rstr);
      answer_list_add(answer_list, msg, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      lFreeElem(&r);
      *range = NULL;
      DRETURN_VOID;
   }

   if (rmin != 0) {
      /* rstr is "-<n>" and ldummy contains <n>
       * dptr poits right after <n>.
       */
      if (*dptr != '\0' || (step_allowed && *dptr != ':')) {
         snprintf(msg, sizeof(msg), MSG_GDI_RANGESPECIFIERWITHUNKNOWNTRAILER_SS,
                  old_str, rstr);
         answer_list_add(answer_list, msg, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
         lFreeElem(&r);
         *range = NULL;
         DRETURN_VOID;
      }
      /* <n> is the max-value */
      rmax = ldummy;

   } else {     /* rmin==0) */
      /*
       ** rstr is "<n>" or "<n>-..." and ldummy contains <n>
       ** dptr poits right after <n>.
       */
      if (*dptr == '\0') {
         /* rstr is just "<n>" */
         rmin = ldummy;
         rmax = ldummy;
      } else {
         /* rstr should be "<n>-..." */
         if (!
             (*dptr == '-' || isdigit((int) *(dptr + 1)) || *(dptr + 1) == '\0'
              || (step_allowed && *dptr == ':'))) {
            /* ... but isn't */
            snprintf(msg, sizeof(msg), MSG_GDI_RANGESPECIFIERWITHUNKNOWNTRAILER_SS,
                     old_str, dptr);
            answer_list_add(answer_list, msg, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
            lFreeElem(&r);
            *range = NULL;
            DRETURN_VOID;
         } else {
            /* it is. set min to ldummy. now, what's after the - */
            rmin = ldummy;
            rstr = dptr + 1;
            if (*rstr == '\0') {
               /* the range string was "<n>-" <==> "ldummy-inf" */
               if (inf_allowed) {
                  rmax = RANGE_INFINITY;
               } else {
                  *range = NULL;
                  DRETURN_VOID;
               }
            } else {
               /* the trailer should contain a decimal - go for it */
               ldummy = strtol(rstr, &dptr, 10);
               if ((ldummy == 0) && (rstr == dptr)) {
                  snprintf(msg, sizeof(msg),
                           MSG_GDI_INITIALPORTIONSTRINGNODECIMAL_S, rstr);
                  answer_list_add(answer_list, msg, STATUS_ESYNTAX,
                                  ANSWER_QUALITY_ERROR);
                  lFreeElem(&r);
                  *range = NULL;
                  DRETURN_VOID;
               }

               if (!(*dptr == '\0' || (step_allowed && *dptr == ':'))) {
                  snprintf(msg, sizeof(msg),
                           MSG_GDI_RANGESPECIFIERWITHUNKNOWNTRAILER_SS,
                           rstr, dptr);
                  answer_list_add(answer_list, msg, STATUS_ESYNTAX,
                                  ANSWER_QUALITY_ERROR);
                  lFreeElem(&r);
                  *range = NULL;
                  DRETURN_VOID;
               }
               /* finally, we got the max-value in ldummy */
               rmax = ldummy;

               if (step_allowed && *dptr && *dptr == ':') {
                  const double epsilon = 1.0E-12;
                  double       dbldummy;
 
                  rstr = dptr + 1;
                  dbldummy = strtod(rstr, &dptr);
                  ldummy = dbldummy;
                  
                  if (dbldummy > 0) {
                     if (( dbldummy - ldummy > epsilon) ||
                        ((ldummy == 0) && (rstr == dptr))) {
                        snprintf(msg, sizeof(msg),
                                 MSG_GDI_INITIALPORTIONSTRINGNODECIMAL_S,
                                 rstr);
                        answer_list_add(answer_list, msg, STATUS_ESYNTAX,
                                        ANSWER_QUALITY_ERROR);
                        lFreeElem(&r);
                        *range = NULL;
                        DRETURN_VOID;
                     }
                  }
                  else if (dptr == rstr) {
                     snprintf(msg, sizeof(msg),
                              MSG_GDI_INITIALPORTIONSTRINGNODECIMAL_S, rstr);
                     answer_list_add(answer_list, msg, STATUS_ESYNTAX,
                                     ANSWER_QUALITY_ERROR);
                     lFreeElem(&r);
                     *range = NULL;
                     DRETURN_VOID;
                  }
                  else {
                     snprintf(msg, sizeof(msg), SFNMAX, MSG_GDI_NEGATIVSTEP);
                     answer_list_add(answer_list, msg, STATUS_ESYNTAX,
                                     ANSWER_QUALITY_ERROR);
                     lFreeElem(&r);
                     *range = NULL;
                     DRETURN_VOID;
                  }
                   
                  if (*dptr != '\0') {
                     snprintf(msg, sizeof(msg),
                              MSG_GDI_RANGESPECIFIERWITHUNKNOWNTRAILER_SS,
                              rstr, dptr);
                     answer_list_add(answer_list, msg, STATUS_ESYNTAX,
                                     ANSWER_QUALITY_ERROR);
                     lFreeElem(&r);
                     *range = NULL;
                     DRETURN_VOID;
                  }
                  /* finally, we got the max-value in ldummy */
                  step = ldummy;

               }
            }   /* if (*rstr == '\0') -- else clause */
         }      /* if (*dptr != '-') -- else clause */
      } /* if (*dptr == '\0') -- else clause */
   }    /* if (r->min != 0) -- else clause */

   /* We're ready? Not quite! We still have to check whether min<=max */
   if (rmin > rmax) {
      ldummy = rmax;
      rmax = rmin;
      rmin = ldummy;
   }

   lSetUlong(r, RN_min, rmin);
   lSetUlong(r, RN_max, rmax);
   lSetUlong(r, RN_step, step);

   /* Ughhhh! Done ... */

   *range = r;
   DRETURN_VOID;
}

/* 

   converts a range string into a range cull list

   an undefined range return NULL

   if answer_list is delivered no exit occurs instead the function fills the 
   answer list and returns NULL, *answer_list must be NULL !

   MT-NOTE: range_list_parse_from_string() is MT safe
*/
bool 
range_list_parse_from_string(lList **this_list, lList **answer_list, 
                             const char *string, bool just_parse, 
                             bool step_allowed, bool inf_allowed)
{
   const char *s;
   lListElem *range = NULL;
   lList *range_list = NULL;
   bool undefined = false, first = true;
   struct saved_vars_s *context = NULL;

   DENTER(TOP_LAYER, "range_list_parse_from_string");

   if (!this_list) {
      this_list = &range_list;
   }

   for (s = sge_strtok_r(string, RANGE_SEPARATOR_CHARS, &context);
        s; s = sge_strtok_r(NULL, RANGE_SEPARATOR_CHARS, &context)) {
      if (!first && undefined) {
         /* first was undefined - no more ranges allowed */
         ERROR((SGE_EVENT, SFNMAX, MSG_GDI_UNEXPECTEDRANGEFOLLOWINGUNDEFINED));
         sge_free_saved_vars(context);
         answer_list_add(answer_list, SGE_EVENT, STATUS_ESYNTAX,
                         ANSWER_QUALITY_ERROR);
         *this_list = NULL;
         DRETURN(false);
      }

      range_parse_from_string(&range, answer_list, s, 
                              step_allowed, inf_allowed);

      if (range == NULL) {
         if (first) {
            undefined = true;
         } else {
            /* second range may not be undefined ! */
            ERROR((SGE_EVENT, SFNMAX, MSG_GDI_UNEXPECTEDUNDEFINEDFOLLOWINGRANGE));
            sge_free_saved_vars(context);
            answer_list_add(answer_list, SGE_EVENT, STATUS_ESYNTAX,
                            ANSWER_QUALITY_ERROR);
            *this_list = NULL;
            DRETURN(false);
         }
      } else {
         if (just_parse) {
            lFreeElem(&range);
         } else {
            expand_range_list(range, this_list);
         }
      }

      first = false;
   }
   
   sge_free_saved_vars(context);

   DRETURN(true);
}
