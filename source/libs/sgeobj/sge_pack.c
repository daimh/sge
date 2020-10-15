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

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "cull/cull_list.h"
#include "cull/cull_pack.h"

#include "sgeobj/sge_pack.h"

#include "msg_common.h"

lListElem *lWhereToElem(const lCondition *where){
   lListElem *whereElem = NULL;
   sge_pack_buffer pb;
   DENTER(CULL_LAYER, "lWhereToElem");

   if (init_packbuffer(&pb, 1024, 0) == PACK_SUCCESS) {
      if (cull_pack_cond(&pb, where) == PACK_SUCCESS) {
         whereElem = lCreateElem(PACK_Type);
         lSetUlong(whereElem, PACK_id, SGE_WHERE);
         setByteArray( (char*)pb.head_ptr, pb.bytes_used, whereElem, PACK_string);
      }
   }
   clear_packbuffer(&pb);

   DRETURN(whereElem);
}

lCondition *lWhereFromElem(const lListElem *where){
   lCondition *cond = NULL;
   sge_pack_buffer pb;
   int size=0;
   char *buffer;
   int ret=0;
   DENTER(CULL_LAYER, "lWhereFromCull");

   if (lGetUlong(where, PACK_id) == SGE_WHERE) {
      size = getByteArray(&buffer, where, PACK_string);
      if (size <= 0){
         ERROR((SGE_EVENT, SFNMAX, MSG_PACK_INVALIDPACKDATA));
      } else if ((ret = init_packbuffer_from_buffer(&pb, buffer, size)) == PACK_SUCCESS) {
         cull_unpack_cond(&pb, &cond);
         clear_packbuffer(&pb);
      }
      else {
         sge_free(&buffer);
         ERROR((SGE_EVENT, MSG_PACK_ERRORUNPACKING_S, cull_pack_strerror(ret)));
      }
   }
   else {
      ERROR((SGE_EVENT, MSG_PACK_WRONGPACKTYPE_UI, sge_u32c(lGetUlong(where, PACK_id)), SGE_WHERE));
   }
   DRETURN(cond);
}

lListElem *lWhatToElem(const lEnumeration *what)
{
   lListElem *whatElem = NULL;
   sge_pack_buffer pb;

   DENTER(CULL_LAYER, "lWhatToElem");

   if (init_packbuffer(&pb, 1024, 0) == PACK_SUCCESS) {
      if (cull_pack_enum(&pb, what) == PACK_SUCCESS) {
         whatElem = lCreateElem(PACK_Type);
         lSetUlong(whatElem, PACK_id, SGE_WHAT);

         setByteArray((char*)pb.head_ptr, pb.bytes_used, whatElem, PACK_string);
      }
   }
   clear_packbuffer(&pb);

   DEXIT;
   return whatElem;
}

lEnumeration *lWhatFromElem(const lListElem *what){
   lEnumeration *cond = NULL;
   sge_pack_buffer pb;
   int size=0;
   char *buffer;
   int ret=0;
   DENTER(CULL_LAYER, "lWhatFromCull");

   if (lGetUlong(what, PACK_id) == SGE_WHAT) {
      size = getByteArray(&buffer, what, PACK_string);
      if (size <= 0){
         ERROR((SGE_EVENT, SFNMAX, MSG_PACK_INVALIDPACKDATA));
      } else if ((ret = init_packbuffer_from_buffer(&pb, buffer, size)) == PACK_SUCCESS) {
         cull_unpack_enum(&pb, &cond);
         clear_packbuffer(&pb);
      } else {
         sge_free(&buffer);
         ERROR((SGE_EVENT, MSG_PACK_ERRORUNPACKING_S, cull_pack_strerror(ret)));
      }
   }
   else {
      ERROR((SGE_EVENT, MSG_PACK_WRONGPACKTYPE_UI,
             sge_u32c(lGetUlong(what, PACK_id)), SGE_WHAT));
   }
   DEXIT;
   return cond;
}


