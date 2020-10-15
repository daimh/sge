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

#include "cull/cull.h"

#include "sgeobj/sge_feature.h"

#include "gdi/pack_job_delivery.h"

/****** pack_job_delivery/pack_job_delivery() **********************************
*  NAME
*     pack_job_delivery() -- pack a job to be sent to execd
*
*  SYNOPSIS
*     int pack_job_delivery(sge_pack_buffer *pb, lListElem *jep, lList *qlp, 
*     lListElem *pep) 
*
*  FUNCTION
*     This function is used in qmaster and by qrsh -inherit to deliver
*     jobs to execd's.
*
*  INPUTS
*     sge_pack_buffer *pb - packing buffer
*     lListElem *jep      - JB_Type
*
*  RESULT
*     int - PACK_SUCCESS on success
*
*  NOTES
*     MT-NOTE: pack_job_delivery() is MT safe
*******************************************************************************/
int pack_job_delivery(sge_pack_buffer *pb, lListElem *jep)
{
   int ret;

   if ((ret=packint(pb, feature_get_active_featureset()))) {
      return ret;  
   }
   if ((ret=cull_pack_elem(pb, jep)) != PACK_SUCCESS) {
      return ret;
   }
   return PACK_SUCCESS;
}

