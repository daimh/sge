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

#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "sgeobj/sge_answer.h"

#ifdef SPOOLING_berkeleydb
#include "spool/berkeleydb/sge_spooling_berkeleydb.h"
#endif
#ifdef SPOOLING_classic
#include "spool/flatfile/sge_spooling_flatfile.h"
#endif
#ifdef SPOOLING_postgres
#include "spool/postgres/sge_spooling_postgres.h"
#endif
#ifdef SPOOLING_dynamic
#include "spool/dynamic/sge_spooling_dynamic.h"
#endif

#include "spool/loader/msg_spoollib_loader.h"

#include "spool/loader/sge_spooling_loader.h"

/****** spool/spool_create_dynamic_context() *********************
*  NAME
*     spool_create_dynamic_context() -- create a spooling context
*
*  SYNOPSIS
*     lListElem * 
*     spool_create_dynamic_context(lList **answer_list, 
*                                  const char *shlib_name, const char *args) 
*
*  FUNCTION
*     Create a spooling context.
*     Which spooling context to use can either be defined at compile time, e.g. 
*     by calling "aimk -spool-classic" or "aimk -spool-flatfile".
*
*     Or dynamic loading of a spooling framework is activated (by building
*     with "aimk -spool-dynamic".
*     In this case a shared library is dynamically loaded and the spooling
*     framework implementation of that shared library is used.
*
*  INPUTS
*     lList **answer_list    - to return error messages
*     const char *shlib_name - name of a shared lib
*     const char *args       - arguments to be passed to the initialization
*                              function in the specified shared lib.
*
*  RESULT
*     lListElem * - on success a spooling context, else NULL.
*
*  SEE ALSO
*     spool/--Spooling
*******************************************************************************/
lListElem *
spool_create_dynamic_context(lList **answer_list, const char *method,
                             const char *shlib_name, const char *args)
{
   const char *compiled_method;
   lListElem *context = NULL;

   DENTER(TOP_LAYER, "spool_create_dynamic_context");

#ifdef SPOOLING_berkeleydb
   compiled_method = "berkeleydb";
   if (strcmp(method, compiled_method) != 0) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_SPOOL_COMPILEDMETHODNECONFIGURED_SS,
                              compiled_method, method);
   } else {
      context = spool_berkeleydb_create_context(answer_list, args);
   }
#endif
#ifdef SPOOLING_classic
   compiled_method = "classic";
   if (strcmp(method, compiled_method) != 0) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_SPOOL_COMPILEDMETHODNECONFIGURED_SS,
                              compiled_method, method);
   } else {
      context = spool_classic_create_context(answer_list, args);
   }
#endif
#ifdef SPOOLING_postgres
   compiled_method = "postgres";
   if (strcmp(method, compiled_method) != 0) { 
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_SPOOL_COMPILEDMETHODNECONFIGURED_SS,
                              compiled_method, method);
   } else {
      context = spool_postgres_create_context(answer_list, args);
   }
#endif
#ifdef SPOOLING_dynamic
   /* 
    * don't need compiled_method here, but otherwise we get an
    * unused variable compiler warning. And we need the DPRINTF
    * on irix6, otherwise we get a set but not used warning.
    */
   compiled_method = "dynamic";
   DPRINTF(("creating "SFQ" spooling context\n", compiled_method));
   context = spool_dynamic_create_context(answer_list, method, shlib_name, args);
#endif

   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_SPOOL_ERRORCREATINGCONTEXT_S, 
                              method);
   }

   DEXIT;
   return context;
}


