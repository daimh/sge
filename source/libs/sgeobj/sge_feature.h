#ifndef __SGE_FEATURE_H
#define __SGE_FEATURE_H
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

#include "sgeobj/sge_feature_FES_L.h"

#ifdef  __cplusplus
extern "C" {
#endif  

typedef enum {
   FS_SHORT,
   FS_LONG,
   FS_VERSION,
   FS_SHORT_VERSION,
   FS_LONG_VERSION
} featureset_product_name_id_t;

typedef enum {
   FEATURE_UNINITIALIZED = 0, 
   FEATURE_NO_SECURITY,             /* No security mode active */
   FEATURE_AFS_SECURITY,            /* AFS security */
   FEATURE_DCE_SECURITY,            /* GSSAPI security */
   FEATURE_KERBEROS_SECURITY,       /* KERBEROS security */
   FEATURE_CSP_SECURITY,            /* CSP security */
   FEATURE_MUNGE_SECURITY,
 
   /* DON'T CHANGE THE ORDER OF THE ENTRIES ABOVE */
 
   FEATURE_LAST_ENTRY
} feature_id_t;

typedef struct {
   feature_id_t id;
   char *name;
} featureset_names_t;            

void feature_mt_init(void);

void    feature_set_already_read_from_file(int i);
int     feature_get_already_read_from_file(void);
lList** feature_get_master_featureset_list(void);


void            feature_initialize(void);
int             feature_initialize_from_string(const char *mode);
void            feature_activate(feature_id_t id);
const char*     feature_get_featureset_name(feature_id_t id);
u_long32        feature_get_active_featureset(void);
bool            feature_is_enabled(feature_id_t id);
const char*     feature_get_product_name(featureset_product_name_id_t style, dstring *buffer);
const char*     feature_get_featureset_names(dstring *buffer, u_long32 featureset);
 
#ifdef  __cplusplus
}
#endif                   

#endif
