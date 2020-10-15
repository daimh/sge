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
/* Portions of this code are Copyright (c) 2011 Univa Corporation. */
/*___INFO__MARK_END__*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* do not compile in monitoring code */
#ifndef NO_SGE_COMPILE_DEBUG
#define NO_SGE_COMPILE_DEBUG
#endif

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_profiling.h"

#include "cull/cull_list.h"
#include "cull/cull_hash.h"
#include "cull/cull_lerrnoP.h"
#include "cull/cull_listP.h"
#include "cull/cull_multitypeP.h"
#include "cull/cull_whatP.h"
#include "cull/cull_whereP.h"
#include "cull/cull_pack.h"
#include "cull/cull_parse.h"
#include "cull/cull_what_print.h"

/* =========== implementation ================================= */

static int cull_unpack_switch(sge_pack_buffer *pb, lMultiType *dst, int type, int flags);

static int 
cull_pack_switch(sge_pack_buffer *pb, const lMultiType *src, lEnumeration *what,
                 int type, int flags);

static int cull_unpack_descr(sge_pack_buffer *pb, lDescr **dpp);
static int cull_pack_descr(sge_pack_buffer *pb, const lDescr *dp);

static int cull_unpack_cont(sge_pack_buffer *pb, lMultiType **mpp, const lDescr *dp, int flags);

static int 
cull_pack_cont(sge_pack_buffer *pb, const lMultiType *cp, const lDescr *dp,
               const lEnumeration *what, int flags);

static int cull_unpack_object(sge_pack_buffer *pb, lListElem **epp, int flags);
static int cull_pack_object(sge_pack_buffer *pb, const lListElem *ep, int flags);
static int
cull_pack_enum_as_descr(sge_pack_buffer * pb, const lEnumeration *what,
                        const lDescr *dp);

/* ------------------------------------------------------------

   cull_unpack_switch() - switch used to unpack a lMulitype

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
static int cull_unpack_switch(
sge_pack_buffer *pb,
lMultiType *dst,
int type,
int flags
) {
   int ret;
   u_long32 i=0;
   u_long64 i64=0;

   DENTER(CULL_LAYER, "cull_unpack_switch");

   switch (type) {

   case lBoolT:
      ret = unpackint(pb, &i);
      dst->b = i;
      break;

   case lUlongT:
      ret = unpackint(pb, &i);
      dst->ul = i;
      break;

   case lUlong64T:
      ret = unpackint64(pb, &i64);
      dst->ul64 = i64;
      break;

   case lStringT:
      ret = unpackstr(pb, &(dst->str));
      break;

   case lHostT:
      ret = unpackstr(pb, &(dst->host));
      break;

   case lListT:
      ret = cull_unpack_list_partial(pb, &(dst->glp), flags);
      break;

   case lObjectT:
      ret = cull_unpack_object(pb, &(dst->obj), flags);
      break;

   case lDoubleT:
      ret = unpackdouble(pb, &(dst->db));
      break;

   case lRefT:
      dst->ref = NULL;
      ret = PACK_SUCCESS;
      break;
      
   case lIntT:
   case lFloatT:
   case lLongT:
   case lCharT:
      DPRINTF(("type not implemented\n"));
      ret = PACK_FORMAT;
      break;
   default:
      DPRINTF(("unknown type %d\n", type));
      ret = PACK_FORMAT;
      break;
   }

   DEXIT;
   return ret;
}

/* ------------------------------------------------------------

   cull_pack_switch() - switch used to pack a lMulitype

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
static int 
cull_pack_switch(sge_pack_buffer *pb, const lMultiType *src, lEnumeration *what,
                 int type, int flags) {
   int ret;

   DENTER(CULL_LAYER, "cull_pack_switch");

   switch (type) {

   case lBoolT:
      ret = packint(pb, src->b);
      break;

   case lUlongT:
      ret = packint(pb, src->ul);
      break;

   case lUlong64T:
      ret = packint64(pb, src->ul64);
      break;

   case lStringT:
      ret = packstr(pb, src->str);
      break;

   case lHostT:
      ret = packstr(pb, src->host);
      break;

   case lListT:
      ret = cull_pack_list_partial(pb, src->glp, what, flags);
      break;

   case lObjectT:
      ret = cull_pack_object(pb, src->obj, flags);
      break;

   case lDoubleT:
      ret = packdouble(pb, src->db);
      break;

   case lRefT:
      ret = PACK_SUCCESS;
      break;

   case lIntT:
   case lFloatT:
   case lLongT:
   case lCharT:
      DPRINTF(("not implemented\n"));
      ret = PACK_FORMAT;
      break;
   default:
      DPRINTF(("unknown type %d\n", type));
      ret = PACK_FORMAT;
      break;
   }

   DEXIT;
   return ret;
}

/* ------------------------------------------------------------

   cull_unpack_descr() - unpacks a complete descriptor

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
static int cull_unpack_descr(
sge_pack_buffer *pb,
lDescr **dpp 
) {
   lDescr *dp;
   int ret;
   u_long32 n=0;
   u_long32 i=0;
   u_long32 temp=0;

   DENTER(CULL_LAYER, "cull_unpack_descr");

   *dpp = NULL;

   /* read in number of lDescr fields (without end mark) */
   if ((ret = unpackint(pb, &n))) {
      DEXIT;
      return ret;
   }

   /* n+1 does not work because this casts n to int */ 
   if ( n > MAX_DESCR_SIZE - 1 ) {
      LERROR(LEMALLOC);
      DEXIT;
      return PACK_ENOMEM;
   }

   if ((dp = (lDescr *) malloc(sizeof(lDescr) * (n+1))) == NULL) {
      LERROR(LEMALLOC);
      DEXIT;
      return PACK_ENOMEM;
   }

   memset(dp, 0, sizeof(lDescr) * (n+1));

   dp[n].nm = NoName;
   dp[n].mt = lEndT;
   dp[n].ht = NULL;

   /* read in n lDescr fields */
   for (i = 0; i < n; i++) {
      if ((ret = unpackint(pb, &temp))) {
         sge_free(&dp);
         DEXIT;
         return ret;
      }
      dp[i].nm = temp;

      if ((ret = unpackint(pb, &temp))) {
         sge_free(&dp);
         DEXIT;
         return ret;
      }
      dp[i].mt = temp;
      dp[i].ht = NULL;
   }

   /* 
    * above we made the assumption that the unpacked descriptor was reduced
    * now we have to check that the assumption is correct 
    */
   {
      const lNameSpace *ns = cull_state_get_name_space();
      int i = 0;

      /* at first we assume the element is reduced */
      bool is_reduced = true;


      /*
       * EB: TODO: I don't know why but when this function is executed within drmma
       * functionality then the namespace is sometimes NULL. 
       * If this happens, then I assume that the element is reduced. 
       */
      if (ns == NULL) {
         is_reduced = true;
      } else {
         /* 
          * check if the descriptor is really reduced.
          * this in done by iterating over the whole namespace
          * we try to find the element and all field for an entry in the 
          * namespace 
          */
         while (ns[i].lower != 0 && ns[i].size != 0 && ns[i].namev) {
            /* 
             * check if the first element in the descriptor is 
             * equivalent with the current entry in the namespace (first nm id) 
             */ 
            if (dp[0].nm == ns[i].lower) {
               int nm, k;

               /* we change the initial assumption */
               is_reduced = false;

               /* 
                * check if all other entries are also in the descriptor 
                * as soon as a element is missing we can break: the element is then reduced
                */
               for (nm = ns[i].lower, k = 0; nm < ns[i].lower + ns[i].size; nm++, k++) {
                  if (dp[k].nm != nm) {
                     is_reduced = true;
                     break;
                  }
               }
            
               /* 
                * if we did not skip the loop above but the field in the
                * descriptor are more that thet in the namespace then
                * we have also to set the is_reduced flag. (this should never happen)
                */
               if (k != n) {
                  is_reduced = true;
                  break;
               }

               break;
            } 

            /* move to next element */
            i++;
         }
      }

      /* 
       * we only have to correct the is_reduced flag if the check above 
       * found all names in the descriptor
       */
      {
         int l;

         for (l = 0; l <= n; l++) {
            dp[l].mt |= (is_reduced == true) ? CULL_IS_REDUCED : 0; 
         }
      }
   }
   
   *dpp = dp;

   DEXIT;
   return PACK_SUCCESS;
}

/* ------------------------------------------------------------

   cull_pack_descr() - packs a complete descriptor

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
static int cull_pack_descr(sge_pack_buffer *pb, const lDescr *dp) 
{
   int i, ret;

   DENTER(CULL_LAYER, "cull_pack_descr");

   /* pack the number of lDescr fields (without end mark) */
   if ((ret = packint(pb, lCountDescr(dp)))) {
      DEXIT;
      return ret;
   }
   /* pack the lDescr fields */
   for (i = 0; mt_get_type(dp[i].nm) != NoName && mt_get_type(dp[i].mt) != lEndT; i++) {
      if ((ret = packint(pb, dp[i].nm))) {
         DEXIT;
         return ret;
      }
      if ((ret = packint(pb, dp[i].mt))) {
         DEXIT;
         return ret;
      }
   }

   DEXIT;
   return PACK_SUCCESS;
}

/* TODO EB: doc is missing */
/* TODO EB: remove not needed exit points */
static int
cull_pack_enum_as_descr(sge_pack_buffer * pb, const lEnumeration *what,
                        const lDescr *descr)
{
   int i, ret;

   DENTER(CULL_LAYER, "cull_pack_enum_as_descr");

   PROF_START_MEASUREMENT(SGE_PROF_PACKING);
   /* pack the number of lDescr fields (without end mark) */
   if ((ret = packint(pb, lCountWhat(what, descr)))) {
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }
   switch (what[0].pos) {
   case WHAT_ALL:               
      for (i = 0; descr[i].nm != NoName; i++) {
         if ((ret = packint(pb, descr[i].nm))) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
         if ((ret = packint(pb, descr[i].mt))) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
      }
      break;
   case WHAT_NONE:
      break;
   default:                
      /* pack the lEnumeration field similar to lDescr fields */
      for (i = 0; what[i].nm != NoName; i++) {
         if ((ret = packint(pb, what[i].nm))) {    
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
         if ((ret = packint(pb, what[i].mt))) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
      }
      break;
   }
   PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
   DEXIT;
   return PACK_SUCCESS;
}

/* ================================================================ */

/* ------------------------------------------------------------

   cull_pack_cont() - packs a contents array

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
static int 
cull_pack_cont(sge_pack_buffer *pb, const lMultiType *cp, const lDescr *dp,
               const lEnumeration *what, int flags) 
{
   int i, ret;
   int n;

   DENTER(CULL_LAYER, "cull_pack_cont");

   if (what == NULL) {
      n = lCountDescr(dp);
      for (i = 0; i < n; i++) {
         /* if flags are given, pack only fields matching flags, e.g. CULL_SPOOL */
         if (flags == 0 || (dp[i].mt & flags) != 0) {
            if ((ret = cull_pack_switch(pb, &cp[i], NULL, mt_get_type(dp[i].mt), flags))) {
               DEXIT;
               return ret;
            }
         }
      }
   } else {
      for (i = 0; what[i].nm != NoName; i++) {
         if (flags == 0 || (what[i].mt & flags) != 0) {
            if ((ret = cull_pack_switch(pb, &cp[what[i].pos], what[i].ep,
                                        mt_get_type(what[i].mt), flags))) {
               DEXIT;
               return ret;
            }
         }
      }
   }

   DEXIT;
   return PACK_SUCCESS;
}

/* ------------------------------------------------------------

   cull_unpack_cont() - unpacks a contents array

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
static int cull_unpack_cont(
sge_pack_buffer *pb,
lMultiType **mpp,
const lDescr *dp,
int flags
) {
   int i, n, ret;
   bool only_at_bufferend = true;   /* Everything had PACK_SUCCESS or
                                     * PACK_FORMAT error happend but only at the
                                     * end of the descriptor */
   bool format_error = false;       /* set to true when at least one PACK_FORMAT 
                                     * error happens */
   int last_error = PACK_SUCCESS;   /* error happend in last iteration 
                                     * or PACK_SUCCESS */
   lMultiType *cp = NULL;

   DENTER(CULL_LAYER, "cull_unpack_cont");

   *mpp = NULL;
   n = lCountDescr(dp);
   if ((cp = (lMultiType *) calloc(1, sizeof(lMultiType) * (n + 1))) == NULL) {
      LERROR(LEMALLOC);
      DEXIT;
      return PACK_ENOMEM;
   }

   for (i = 0; i < n; i++) {
      /* if flags are given, unpack only fields matching flags, e.g. CULL_SPOOL */
      if (flags == 0 || (dp[i].mt & flags) != 0) {
         int lret = cull_unpack_switch(pb, &(cp[i]), mt_get_type(dp[i].mt), flags);

         last_error = lret;
         if (lret == PACK_FORMAT) {
            format_error = true;
         } else if (lret != PACK_SUCCESS) {
            only_at_bufferend = false;
            break;
         }
      }
   }

   if (format_error && only_at_bufferend) {
      /*
       * We catch all format errors that occur during unparsing attributes 
       * from the end of a descriptos. Format errors mean that
       * these attributes where not contained at the end of the packbuffer
       * This is the case after an update release has been installed.
       * The new master finds old objects. Due to the fact that during the 
       * process reading in all objects a full (new) descriptor is used
       * these errors can be skipped. 
       *
       * Missing attributes will contain 0 values. If there is a need to
       * initialize them then this has to happen some layers above.
       * (setup_qmaster)
       */
      ret = PACK_SUCCESS;
   } else if (last_error != PACK_SUCCESS || only_at_bufferend == false) {
      /*
       * If we are here then an error != PACK_FORMAT happened or
       * or a PACK_FORMAT occured but after it occured either
       * a different error happended or an operation succeeded. 
       * Format errors should only be catched if they ocure at the en of 
       * the descriptor.
       */
      sge_free(&cp);
      ret = (last_error != PACK_SUCCESS) ? last_error : PACK_FORMAT;
   } else {
      /*
       * This is hopefully the default case
       */
      ret = PACK_SUCCESS;
   }
   *mpp = cp;
   DRETURN(ret);
}

/* ================================================================ */

/* ------------------------------------------------------------

   cull_pack_elem() - packs a list element

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
int cull_pack_elem(sge_pack_buffer *pb, const lListElem *ep) 
{
   int ret;

   DENTER(TOP_LAYER, "cull_pack_elem");
   ret = cull_pack_elem_partial(pb, ep, NULL, 0);
   DEXIT;
   return ret;
}


int 
cull_pack_elem_partial(sge_pack_buffer *pb, const lListElem *ep, 
                       const lEnumeration *what, int flags)
{
   int ret;

   DENTER(TOP_LAYER, "cull_pack_elem_partial");

   PROF_START_MEASUREMENT(SGE_PROF_PACKING);
   if(ep->descr == NULL) {
      DPRINTF(("element descriptor NULL not allowed !!!\n"));
      DEXIT;
      abort();
   }

   if((ret = packint(pb, ep->status)) != PACK_SUCCESS) {
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }

   if(ep->status == FREE_ELEM) {
      if (what == NULL) {
         if((ret = cull_pack_descr(pb, ep->descr)) != PACK_SUCCESS) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
      } else {
         if ((ret = cull_pack_enum_as_descr(pb, what, ep->descr)) != PACK_SUCCESS) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
      }
   }

   /*
    * pack dummy bitfield: later on this space might be used to store
    * the elements changed-bitfield
    */
   {
      bitfield field;

      if (what == NULL) {
         if (!sge_bitfield_init(&field, lCountDescr(ep->descr))) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return PACK_ENOMEM;
         }
      } else {
         if (!sge_bitfield_init(&field, lCountWhat(what, ep->descr))) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return PACK_ENOMEM;
         }
      }
      if((ret = packbitfield(pb, &(field))) != PACK_SUCCESS) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
      sge_bitfield_free_data(&field);
   }

   ret = cull_pack_cont(pb, ep->cont, ep->descr, what, flags);

   PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
   DEXIT;
   return ret;
}

/* ------------------------------------------------------------

   cull_unpack_elem() - unpacks a list element

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
int cull_unpack_elem(
sge_pack_buffer *pb,
lListElem **epp,
const lDescr *dp                  /* has to be NULL in case of free elements 
                                   must be the desriptor in case of bound elements */
) {
   int ret;
   
   DENTER(CULL_LAYER, "cull_unpack_elem");
   ret = cull_unpack_elem_partial(pb, epp, dp, 0);
   DEXIT;
   return ret;
}

int cull_unpack_elem_partial(sge_pack_buffer *pb, lListElem **epp, const lDescr *dp, int flags) 
{
   int ret;
   lListElem *ep = NULL;

   DENTER(CULL_LAYER, "cull_unpack_elem_partial");

   PROF_START_MEASUREMENT(SGE_PROF_PACKING);
   *epp = NULL;

   if((ep = (lListElem *) calloc(1, sizeof(lListElem))) == NULL) {
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return PACK_ENOMEM;
   }

   if((ret = unpackint(pb, &(ep->status))) != PACK_SUCCESS) {
      sge_free(&ep);
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }

   if(ep->status == FREE_ELEM) {
      if((ret = cull_unpack_descr(pb, &(ep->descr))) != PACK_SUCCESS) {
         sge_free(&ep);
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }

      /*
       * Special handling for feature update releses where the job has been changed.
       * In all cases where we got an descriptor from outside and where the descriptor
       * is one that contains JB_job_number as first entry we will use this 
       * descriptor and not that one that is stored in a packbuffer.
       *
       * As a result we are able to read "old" JB_Type objects from a previous
       * GridEngine version although the JB_Type object has been incresed in the new
       * version.
       *
       * Note: Only JB_type objects will be handled correctly and only in the case 
       * if JB_Type contains ALL attributes of the old descriptor and new elements
       * AT THE END of the new descriptor.
       *
       * 50 is JB_job_number. I did not use the enum value here because it is defined
       * in sgeobj header file and not cull.
       */   
      if (dp != NULL && dp[0].nm == 50) {
         sge_free(&(ep->descr));
         if((ep->descr = lCopyDescr((lDescr *) dp)) == NULL) {
            sge_free(&ep);
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return PACK_BADARG;
         }
      }
   } else {
      /* 
         if it is not a free element we need 
         a descriptor from outside 
       */
      if((ep->descr = (lDescr *) dp) == NULL) {
         sge_free(&ep);
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return PACK_BADARG;
      }
   }

   /* This is a hack, to avoid aborts in lAppendElem */
   if(ep->status == BOUND_ELEM || ep->status == OBJECT_ELEM)
      ep->status = TRANS_BOUND_ELEM;

   /* 
      here after setting status correctly we got 
      an element in a defined state - so we may
      call lFreeElem() in case of errors 
    */

   if((ret = unpackbitfield(pb, &(ep->changed), lCountDescr(ep->descr))) != PACK_SUCCESS) {
      if(ep->status == FREE_ELEM || ep->status == OBJECT_ELEM) {
         sge_free(&(ep->descr));
      }
      sge_free(&ep);
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }
    
   if((ret = cull_unpack_cont(pb, &(ep->cont), ep->descr, flags))) {
      if(ep->status == FREE_ELEM || ep->status == OBJECT_ELEM) {
         sge_free(&(ep->descr));
      }   
      sge_free(&ep);
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }

   *epp = ep;

   PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
   DEXIT;
   return PACK_SUCCESS;
}

/* ================================================================ */

/* ------------------------------------------------------------

   cull_pack_object() - packs a sub object

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
static int cull_pack_object(
sge_pack_buffer *pb,
const lListElem *ep,
int flags
) {
   int ret;

   DENTER(CULL_LAYER, "cull_pack_object");

   if((ret = packint(pb, ep != NULL)) != PACK_SUCCESS) {
      DEXIT;
      return ret;
   }

   if(ep != NULL) {
      /* pack descriptor */
      if((ret = cull_pack_descr(pb, ep->descr)) != PACK_SUCCESS) {
         DEXIT;
         return ret;
      }

      /* pack list element */
      if((ret = cull_pack_elem_partial(pb, ep, NULL, flags)) != PACK_SUCCESS) {
         DEXIT;
         return ret;
      }
   }

   DEXIT;
   return PACK_SUCCESS;
}

/* ------------------------------------------------------------

   cull_pack_list() - packs a complete list

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
int cull_pack_list(sge_pack_buffer *pb, const lList *lp) 
{
   int ret;

   DENTER(CULL_LAYER, "cull_pack_list");
   ret = cull_pack_list_partial(pb, lp, NULL, 0);
   DEXIT;
   return ret;
}

int
cull_pack_list_summary(sge_pack_buffer *pb, const lList *lp,
                       const lEnumeration *what, const char *name,
                       size_t *offset, size_t *used)
{     
   int ret;
         
   DENTER(CULL_LAYER, "cull_pack_list_summary");

   PROF_START_MEASUREMENT(SGE_PROF_PACKING);
   if((ret = packint(pb, lp != NULL)) != PACK_SUCCESS) {
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }

   if (lp != NULL) {
      /*
       * TODO: The next release where it is possible to change the pb-format
       *       makes it possible to delete this hack 
       *       Then it is possible to remove the argument 'bytes_used'
       *
       * Future implementation:
       *    The number of elements can't be written before the list
       *    elements. Instead each element has to be introduced by an int.
       *    0 means that no element will follow. 1 means that an element
       *    will follow    
       */
      *offset = pb->cur_ptr - pb->head_ptr;
      *used = pb->bytes_used;

      /*
       * pack number of elements contained in the list 
       */
      if((ret = packint(pb, lp->nelem)) != PACK_SUCCESS) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
     
      /*
       * name of the list
       */ 
      if (name == NULL) {
         name = lp->listname;
      }
      if((ret = packstr(pb, name)) != PACK_SUCCESS) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }

      /* changed variable */
      if((ret = packint(pb, lp->changed)) != PACK_SUCCESS) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }

      /* pack descriptor */
      if (what == NULL) {
         ret = cull_pack_descr(pb, lp->descr);
         if(ret != PACK_SUCCESS) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
      } else {
         ret = cull_pack_enum_as_descr(pb, what, lp->descr);
         if (ret != PACK_SUCCESS) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
      }
   }

   DEXIT;
   return PACK_SUCCESS;
}        

int cull_pack_list_partial(sge_pack_buffer *pb, const lList *lp, 
                           lEnumeration *what, int flags) 
{
   int ret;
   lListElem *ep;

   DENTER(CULL_LAYER, "cull_pack_list_partial");

   PROF_START_MEASUREMENT(SGE_PROF_PACKING);

   if (lp != NULL && pb != NULL) {
      if((ret = packint(pb, 1)) != PACK_SUCCESS) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }

      if((ret = packint(pb, lp->nelem)) != PACK_SUCCESS) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }

      if((ret = packstr(pb, lp->listname)) != PACK_SUCCESS) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
      
      if((ret = packint(pb, lp->changed)) != PACK_SUCCESS) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }

      /* pack descriptor */
      if (what == NULL) {
         if((ret = cull_pack_descr(pb, lp->descr)) != PACK_SUCCESS) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
      } else {
         if ((ret = cull_pack_enum_as_descr(pb, what, lp->descr)) != PACK_SUCCESS) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
      }
   } else {
      if((ret = packint(pb, 0)) != PACK_SUCCESS) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
   }

   if (lp != NULL) {
      /* pack each list element */

      for_each(ep, lp) {
         if((ret = cull_pack_elem_partial(pb, ep, what, flags)) != PACK_SUCCESS) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
   DEXIT;
   return PACK_SUCCESS;
}

/* ------------------------------------------------------------

   cull_unpack_list() - unpacks a list 

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */

int cull_unpack_list(sge_pack_buffer *pb, lList **lpp)
{
   int ret;

   DENTER(CULL_LAYER, "cull_unpack_list");
   ret = cull_unpack_list_partial(pb, lpp, 0);
   DEXIT;
   return ret;
}

int cull_unpack_list_partial(sge_pack_buffer *pb, lList **lpp, int flags)
{
   int ret;
   lList *lp;
   lListElem *ep;

   u_long32 i=0;
   u_long32 n=0;
   u_long32 c=0;

   DENTER(CULL_LAYER, "cull_unpack_list_partial");

   PROF_START_MEASUREMENT(SGE_PROF_PACKING);
   *lpp = NULL;

   if((ret = unpackint(pb, &i))) {
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }

   /* do we have an empty list (NULL) ? */
   if(!i) {
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return PACK_SUCCESS;
   }

   if((lp = (lList *) calloc(1, sizeof(lList))) == NULL) {
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return PACK_ENOMEM;
   }

   if((ret = unpackint(pb, &n)) != PACK_SUCCESS) {
      lFreeList(&lp);
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }

   if((ret = unpackstr(pb, &(lp->listname))) != PACK_SUCCESS) {
      lFreeList(&lp);
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }

   if((ret = unpackint(pb, &c)) != PACK_SUCCESS) {
      lFreeList(&lp);
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }
   lp->changed = (bool)c;

   /* unpack descriptor */
   if((ret = cull_unpack_descr(pb, &(lp->descr))) != PACK_SUCCESS) {
      lFreeList(&lp);
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }

   /* unpack each list element */
   for(i = 0; i < n; i++) {
      if((ret = cull_unpack_elem_partial(pb, &ep, lp->descr, flags)) != PACK_SUCCESS) {
         lFreeList(&lp);
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
      lAppendElem(lp, ep);
   }

   cull_hash_create_hashtables(lp);

   *lpp = lp;

   PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
   DEXIT;
   return PACK_SUCCESS;
}

/* ------------------------------------------------------------

   cull_unpack_object() - unpacks a sub object 

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */

static int cull_unpack_object(
sge_pack_buffer *pb,
lListElem **epp,
int flags
) {
   int ret;
   lDescr *descr;
   lListElem *ep;
   u_long32 i=0;

   DENTER(CULL_LAYER, "cull_unpack_object");

   *epp = NULL;

   if((ret = unpackint(pb, &i))) {
      DEXIT;
      return ret;
   }

   /* do we have an empty object (NULL) ? */
   if(!i) {
      DEXIT;
      return PACK_SUCCESS;
   }

   /* unpack descriptor */
   if((ret = cull_unpack_descr(pb, &descr)) != PACK_SUCCESS) {
      DEXIT;
      return ret;
   }

   /* unpack each element */
   if((ret = cull_unpack_elem_partial(pb, &ep, descr, flags)) != PACK_SUCCESS) {
      sge_free(&descr);
      DEXIT;
      return ret;
   }

   ep->status = OBJECT_ELEM;
   *epp = ep;

   DEXIT;
   return PACK_SUCCESS;
}

enum {
   PackWhatAll = 0, PackWhatNone, PackWhatArray
};

/* ------------------------------------------------------------

   cull_pack_enum() - packs an enumeration

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
int cull_pack_enum(
sge_pack_buffer *pb,
const lEnumeration *enp 
) {
   int ret;
   u_long32 flag=0;
   int i=0, n=0;

   DENTER(CULL_LAYER, "cull_pack_enum");

   PROF_START_MEASUREMENT(SGE_PROF_PACKING);
   if ((ret = packint(pb, enp != NULL)))
      goto error;

   if (!enp) {
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return PACK_SUCCESS;
   }

   /* pack flag indicating ALL, NONE or normal enumeration (array) */
   switch (enp[0].pos) {
   case WHAT_ALL:
      flag = PackWhatAll;
      break;
   case WHAT_NONE:
      flag = PackWhatNone;
      break;
   default:
      flag = PackWhatArray;
   }

   if ((ret = packint(pb, flag)))
      goto error;

   if (flag == PackWhatArray) {
      /* compute and pack number of lEnumeration fields (without end mark) */
      for (n = 0; enp[n].nm != NoName; n++);
      if ((ret = packint(pb, n)))
         goto error;

      /* pack the lEnumeration fields */
      for (i = 0; mt_get_type(enp[i].nm) != NoName && mt_get_type(enp[i].mt) != lEndT; i++) {
         if ((ret = packint(pb, enp[i].pos)))
            goto error;
         if ((ret = packint(pb, enp[i].mt)))
            goto error;
         if ((ret = packint(pb, enp[i].nm)))
            goto error;
         if (enp[i].ep == NULL) {
            if ((ret = packint(pb, 0))) {
               goto error;
            }
         } else {
            if ((ret = packint(pb, 1))) {
               goto error;
            }
            if ((ret = cull_pack_enum(pb, enp[i].ep))) {
               goto error;
            }
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
   DEXIT;
   return PACK_SUCCESS;

 error:
   DPRINTF(("error packing enumeration\n"));
   PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
   DEXIT;
   return ret;
}

/* ------------------------------------------------------------

   cull_unpack_enum() - unpacks an enumeration

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
int cull_unpack_enum(
sge_pack_buffer *pb,
lEnumeration **enpp 
) {
   int ret;
   lEnumeration *enp = NULL;
   u_long32 flag=0, i=0, temp=0;
   u_long32 n=0;

   DENTER(CULL_LAYER, "cull_unpack_enum");

   PROF_START_MEASUREMENT(SGE_PROF_PACKING);
   *enpp = NULL;

   if ((ret = unpackint(pb, &i)))
      goto error;

   if (!i) {
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return PACK_SUCCESS;
   }

   if ((ret = unpackint(pb, &flag)))
      goto error;

   if (flag != PackWhatArray) {
      if (!(enp = (lEnumeration *) malloc(2 * sizeof(lEnumeration)))) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return PACK_ENOMEM;
      }

      switch (flag) {
      case PackWhatAll:
         enp[0].pos = WHAT_ALL;
         break;

      case PackWhatNone:
         enp[0].pos = WHAT_NONE;
         break;

      default:
         break;
      }
      enp[0].nm = -99;
      enp[0].mt = -99;
      enp[0].ep = NULL;
      enp[1].nm = NoName;
      enp[1].mt = lEndT;
      enp[1].ep = NULL;
   } else {
      /* read in number of lEnumeration fields (without end mark) */
      if ((ret = unpackint(pb, &n)))
         goto error;

      if ( n+1 > MAX_DESCR_SIZE ) {
         LERROR(LEMALLOC);
         DEXIT;
         return PACK_ENOMEM;
      }

      if (!(enp = (lEnumeration *) malloc(sizeof(lEnumeration) * (n + 1)))) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return PACK_ENOMEM;
      }

      /* read in n lEnumeration fields */
      for (i = 0; i < n; i++) {
         lEnumeration *tmp2 = NULL;

         if ((ret = unpackint(pb, &temp)))
            goto error;
         enp[i].pos = temp;
         if ((ret = unpackint(pb, &temp)))
            goto error;
         enp[i].mt = temp;
         if ((ret = unpackint(pb, &temp)))
            goto error;
         enp[i].nm = temp;
         if ((ret = unpackint(pb, &temp)))
            goto error;
         if (temp == 1) {
            if ((ret = cull_unpack_enum(pb, &tmp2))) {
               goto error;
            }
         }
         enp[i].ep = tmp2;
         tmp2 = NULL;
      }

      enp[n].nm = NoName;
      enp[n].mt = lEndT;
      enp[n].ep = NULL;
   }

   *enpp = enp;

   PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
   DEXIT;
   return PACK_SUCCESS;

 error:

   lFreeWhat(&enp);
   PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
   DEXIT;
   return ret;
}

/* ------------------------------------------------------------

   cull_pack_cond() - packs an condition

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
int cull_pack_cond(
sge_pack_buffer *pb,
const lCondition *cp 
) {
   int ret;

   DENTER(CULL_LAYER, "cull_pack_cond");

   PROF_START_MEASUREMENT(SGE_PROF_PACKING);
   if ((ret = packint(pb, cp != NULL))) {
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }

   if (!cp) {
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return PACK_SUCCESS;
   }

   /* pack operator indicating the following contents */
   if ((ret = packint(pb, cp->op))) {
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }

   DPRINTF(("cp->op: %d\n", cp->op));

   switch (cp->op) {
   case EQUAL:
   case NOT_EQUAL:
   case LOWER_EQUAL:
   case LOWER:
   case GREATER_EQUAL:
   case GREATER:
   case BITMASK:
   case STRCASECMP:
   case PATTERNCMP:
   case SUBSCOPE:
   case HOSTNAMECMP:
      DPRINTF(("cp->operand.cmp.pos: %d\n", cp->operand.cmp.pos));
      if ((ret = packint(pb, cp->operand.cmp.pos))) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
      DPRINTF(("cp->operand.cmp.mt: %d\n", cp->operand.cmp.mt));
      if ((ret = packint(pb, cp->operand.cmp.mt))) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
      DPRINTF(("cp->operand.cmp.nm: %d\n", cp->operand.cmp.nm));
      if ((ret = packint(pb, cp->operand.cmp.nm))) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }

      if (mt_get_type(cp->operand.cmp.mt) != lListT) {
         if ((ret = cull_pack_switch(pb, &(cp->operand.cmp.val), NULL,
                                     mt_get_type(cp->operand.cmp.mt), 0))) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
      } else {
         if ((ret = cull_pack_cond(pb, cp->operand.cmp.val.cp))) {
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
      }
      break;

   case NEG:
      if ((ret = cull_pack_cond(pb, cp->operand.log.first))) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
      break;

   case AND:
   case OR:
      if ((ret = cull_pack_cond(pb, cp->operand.log.first))) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
      if ((ret = cull_pack_cond(pb, cp->operand.log.second))) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
      break;

   default:
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return PACK_FORMAT;
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
   DEXIT;
   return PACK_SUCCESS;
}

/* ------------------------------------------------------------

   cull_unpack_cond() - unpacks an condition

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
int cull_unpack_cond(
sge_pack_buffer *pb,
lCondition **cpp 
) {
   int ret;
   u_long32 i=0;
   lCondition *cp = NULL;

   DENTER(CULL_LAYER, "cull_unpack_cond");

   PROF_START_MEASUREMENT(SGE_PROF_PACKING);
   *cpp = NULL;
   if ((ret = unpackint(pb, &i))) {
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }
   if (!i) {
      *cpp = NULL;
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return 0;
   }

   if (!(cp = (lCondition *) calloc(1, sizeof(lCondition)))) {
      LERROR(LEMALLOC);
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return PACK_ENOMEM;
   }

   if ((ret = unpackint(pb, &i))) {
      lFreeWhere(&cp);
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return ret;
   }
   cp->op = i;

   switch (cp->op) {
   case EQUAL:
   case NOT_EQUAL:
   case LOWER_EQUAL:
   case LOWER:
   case GREATER_EQUAL:
   case GREATER:
   case BITMASK:
   case STRCASECMP:
   case PATTERNCMP:
   case SUBSCOPE:
   case HOSTNAMECMP:
      if ((ret = unpackint(pb, &i))) {
         lFreeWhere(&cp);
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
      cp->operand.cmp.pos = i;
      if ((ret = unpackint(pb, &i))) {
         lFreeWhere(&cp);
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
      cp->operand.cmp.mt = i;
      if ((ret = unpackint(pb, &i))) {
         lFreeWhere(&cp);
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
      cp->operand.cmp.nm = i;

      if (mt_get_type(cp->operand.cmp.mt) != lListT) {
         if ((ret = cull_unpack_switch(pb, &(cp->operand.cmp.val), mt_get_type(cp->operand.cmp.mt), 0))) {
            lFreeWhere(&cp);
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
      } else {
         if ((ret = cull_unpack_cond(pb, &(cp->operand.cmp.val.cp)))) {
            lFreeWhere(&cp);
            PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
            DEXIT;
            return ret;
         }
      }
      break;

   case NEG:
      if ((ret = cull_unpack_cond(pb, &(cp->operand.log.first)))) {
         lFreeWhere(&cp);
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
      break;

   case AND:
   case OR:
      if ((ret = cull_unpack_cond(pb, &(cp->operand.log.first)))) {
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         lFreeWhere(&cp);
         DEXIT;
         return ret;
      }
      if ((ret = cull_unpack_cond(pb, &(cp->operand.log.second)))) {
         lFreeWhere(&cp);
         PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
         DEXIT;
         return ret;
      }
      break;

   default:
      lFreeWhere(&cp);
      PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
      DEXIT;
      return PACK_FORMAT;
   }

   *cpp = cp;

   PROF_STOP_MEASUREMENT(SGE_PROF_PACKING);
   DEXIT;
   return PACK_SUCCESS;
}

/****** cull/pack/setByteArray() ***********************************************
*  NAME
*     setByteArray() -- takes a byte array, transformes it into ASCII and sets
*                       it as a string into an element
*
*  SYNOPSIS
*     void setByteArray(const char *byteArray, int size, lListElem *elem, int 
*     name) 
*
*  FUNCTION
*     makes a string out of a byte array and sets that string into an element 
*
*  INPUTS
*     const char *byteArray -  byte array
*     int size              - size of the byte array 
*     lListElem *elem       - target element 
*     int name              - target attribute 
*
*  RESULT
*     void - nothing 
*
*******************************************************************************/
void setByteArray(const char *byteArray, int size, lListElem *elem, int name){
   const char *numbers = {"0123456789ABCDEF"};
   int lower_part;
   int upper_part;
   int target_size = size * 2 +1;
   char * z_stream_str=NULL;
   int i=0;
   int y=0;

   if (!byteArray || !elem)
      return;
      
   z_stream_str = sge_malloc(target_size);
   memset(z_stream_str, 0, target_size);

   for (i=0; i < size; i++){
      lower_part = (byteArray[i] & 0x0F);
      upper_part = (byteArray[i] & 0xF0) >> 4;
      z_stream_str[y++] = numbers[lower_part];
      z_stream_str[y++] = numbers[upper_part];
   }
   z_stream_str[y++] = '\0';
   lSetString(elem, name, z_stream_str);
   sge_free(&z_stream_str);    
} 

/****** cull/pack/getByteArray() ***********************************************
*  NAME
*     getByteArray() -- transforms a string into a byte array. 
*
*  SYNOPSIS
*     int getByteArray(char **byte, const lListElem *elem, int name) 
*
*  FUNCTION
*     extracts a string from an element and changes it into a byte array. The
*     target has to be a pointer to NULL. The array will be created in the function
*     and no memory is freed. The calling functions have to take care of that.
*
*  INPUTS
*     char **byte           - target byte array, has to be a pointer to NULL  
*     const lListElem *elem - the list element, which contains the string
*     int name              - name of the attribute containing the string 
*
*  RESULT
*     int - >= 0 the size of the byte array
*           < 0 the position of the first none hex character
*
*******************************************************************************/
int getByteArray(char **byte, const lListElem *elem, int name){
   const char *numbers = {"0123456789ABCDEF"};
   int lower_part = 0;
   int upper_part = 0;
   int i=0;
   int size=0;
   const char* string;
   if (!byte || !elem){
      return size;
   }

   string = lGetString(elem, name);
   size = strlen(string) /2;
   *byte = sge_malloc(size);
   memset(*byte, 0, size);

   for (i=0; i < size; i++){
      int a=0;
      
      for(a=0; a < 16; a++){
         if (numbers[a] == string[i*2]) {
            lower_part = a;
            break;
         }
      }
      if (a == 16) {
         return (i * -2);
      }
      
      for(a=0; a < 16; a++){
         if (numbers[a] == string[i*2+1]) {
            upper_part = a;
            break;
         }
      }
      if (a == 16) {
         return ((i * -2) -1);
      }
      (*byte)[i] = (upper_part << 4) + lower_part;
   }
   
   return size;
}

void cull_dump_pack_buffer(sge_pack_buffer *pb, FILE *fp)
{ 
   int i;
   int j = 0;
   char hex[2048], tex[2048];

   for (i=0; i<pb->bytes_used; i++) {
      sprintf(&hex[j*3], "%2x ", pb->head_ptr[i]);
      sprintf(&tex[j], "%c", isalnum(pb->head_ptr[i])?pb->head_ptr[i]:'.');

      if ((i % 16) == 0) {
         fprintf(fp, "%s  %s\n", hex, tex);
         j=0;
      } else {
         j++;
      }
   }
}
