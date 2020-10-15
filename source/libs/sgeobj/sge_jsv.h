#ifndef __SGE_JSV_H 
#define __SGE_JSV_H 
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
 *   Copyright: 2008 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/       


#include "uti/sge_dstring.h"

#include "gdi/sge_gdi_ctx.h"

#include "sgeobj/sge_jsv_JSV_L.h"

#define JSV_CONTEXT_CLIENT "client"

bool        
jsv_url_parse(dstring *jsv_url_str, lList **answer_list, dstring *type, 
              dstring *user, dstring *path, bool in_client);

bool
jsv_send_command(lListElem *jsv, lList **answer_list, const char *message);

bool
jsv_do_verify(sge_gdi_ctx_class_t* ctx, const char *context, lListElem **job, 
              lList **answer_list, bool hold_global_lock);

bool
jsv_stop(lListElem *jsv, lList **answer_list, bool try_soft_quit);

bool
jsv_start(lListElem *jsv, lList **answer_list);

bool
jsv_list_add(const char *name, const char *context, 
             lList **answer_list, const char *jsv_url);

bool
jsv_list_remove(const char *name, const char *context);

bool 
jsv_list_remove_all(void);

bool
jsv_is_enabled(const char *context);

bool
jsv_list_update(const char *name, const char *context,
                lList **answer_list, const char *jsv_url);

#endif /* __SGE_JSV_H */    
