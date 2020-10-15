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

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_log.h"
#include "uti/setup_path.h"
#include "uti/sge_arch.h"

#include "sgeobj/sge_feature.h"

#include "shutdown.h"
#include "msg_daemons_common.h"

void starting_up()
{
   u_long32 old_ll = log_state_get_log_level();
   dstring ds;
   dstring ds2 = DSTRING_INIT;
   dstring ds3 = DSTRING_INIT;
   dstring features = DSTRING_INIT;
   char buffer[256];

   DENTER(TOP_LAYER, "starting_up");

   sge_dstring_init(&ds, buffer, sizeof(buffer));
   log_state_set_log_level(LOG_INFO);
   
   if (feature_get_active_featureset() == FEATURE_NO_SECURITY) {
      sge_dstring_copy_string(&ds2, feature_get_product_name(FS_VERSION, &ds)); 
   } else {   
      sge_dstring_sprintf(&ds2, "%s (%s)", 
                          feature_get_product_name(FS_VERSION, &ds),
                          feature_get_featureset_names(&features,
                                       feature_get_active_featureset()));
   }
   INFO((SGE_EVENT, MSG_STARTUP_STARTINGUP_SSS, 
         feature_get_product_name(FS_SHORT, &ds3), 
         sge_dstring_get_string(&ds2), sge_get_arch()));

   sge_dstring_free(&ds2);
   sge_dstring_free(&ds3);
   sge_dstring_free(&features);
   log_state_set_log_level(old_ll);

   DEXIT;
   return;
}

/******************************************************************************/
void sge_shutdown(void **context, int i)
{
   u_long32 old_ll = log_state_get_log_level();
   dstring ds;
   dstring ds2 = DSTRING_INIT;
   dstring features = DSTRING_INIT;
   char buffer[256];
   
   DENTER(TOP_LAYER, "sge_shutdown");

   sge_dstring_init(&ds, buffer, sizeof(buffer));
   log_state_set_log_level(LOG_INFO);
   if (feature_get_active_featureset() == FEATURE_NO_SECURITY) {
      sge_dstring_copy_string(&ds2, feature_get_product_name(FS_VERSION, &ds)); 
   } else {   
      sge_dstring_sprintf(&ds2, "%s (%s)", 
                          feature_get_product_name(FS_VERSION, &ds),
                          feature_get_featureset_names(&features,
                                       feature_get_active_featureset()));
   }
   if (i != 0) {
      INFO((SGE_EVENT, MSG_SHADOWD_CONTROLLEDSHUTDOWN_SU, sge_dstring_get_string(&ds2), sge_u32c(i)));
   } else {
      INFO((SGE_EVENT, MSG_SHADOWD_CONTROLLEDSHUTDOWN_S, sge_dstring_get_string(&ds2)));
   }

   sge_dstring_free(&ds2);
   sge_dstring_free(&features);
   log_state_set_log_level(old_ll);

   DEXIT;
   SGE_EXIT(context, i); /* call sge_exit() */
}
