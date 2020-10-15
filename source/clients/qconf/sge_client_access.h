#ifndef _SGE_CLIENT_ACCESS_H
#define _SGE_CLIENT_ACCESS_H
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

#include "gdi/sge_gdi_ctx.h"

/*

   These functions that are designed to implement
   high level access functions to the sge gdi

   In all cases an argument alpp is provided
   this is used to report errors found at 
   qmaster and client side. These errors 
   may also be mixed.
   
*/
int sge_client_get_acls(sge_gdi_ctx_class_t *ctx, lList **alpp, lList *acl_args, lList **dst);
int sge_client_add_user(sge_gdi_ctx_class_t *ctx, lList **alpp, lList *user_args, lList *acl_args);
int sge_client_del_user(sge_gdi_ctx_class_t *ctx, lList **alpp, lList *user_args, lList *acl_args);


#endif /* _SGE_CLIENT_ACCESS_H */



