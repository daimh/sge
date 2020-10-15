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
#include "uti/sge_dstring.h"

#include "sgeobj/sge_object.h"

#include "spool/msg_spoollib.h"
#include "spool/template/msg_spoollib_template.h"
#include "spool/template/sge_spooling_template.h"

#include "msg_common.h"

static const char *spooling_method = "template";

#ifdef SPOOLING_template
const char *get_spooling_method(void)
#else
const char *get_template_spooling_method(void)
#endif
{
   return spooling_method;
}


/****** spool/template/spool_template_create_context() ********************
*  NAME
*     spool_template_create_context() -- create a template spooling context
*
*  SYNOPSIS
*     lListElem* 
*     spool_template_create_context(lList **answer_list, const char *args)
*
*  FUNCTION
*     Create a spooling context for the template spooling.
* 
*  INPUTS
*     lList **answer_list - to return error messages
*     int argc     - number of arguments in argv
*     char *argv[] - argument vector
*
*  RESULT
*     lListElem* - on success, the new spooling context, else NULL
*
*  SEE ALSO
*     spool/--Spooling
*     spool/template/--Template-Spooling
*******************************************************************************/
lListElem *
spool_template_create_context(lList **answer_list, const char *args)
{
   lListElem *context = NULL;

   DENTER(TOP_LAYER, "spool_template_create_context");


   DEXIT;
   return context;
}

/****** spool/template/spool_template_default_startup_func() **************
*  NAME
*     spool_template_default_startup_func() -- setup 
*
*  SYNOPSIS
*     bool 
*     spool_template_default_startup_func(lList **answer_list, 
*                                         const char *args, bool check)
*
*  FUNCTION
*
*  INPUTS
*     lList **answer_list   - to return error messages
*     const lListElem *rule - the rule containing data necessary for
*                             the startup (e.g. path to the spool directory)
*     bool check            - check the spooling database
*
*  RESULT
*     bool - true, if the startup succeeded, else false
*
*  NOTES
*     This function should not be called directly, it is called by the
*     spooling framework.
*
*  SEE ALSO
*     spool/template/--Template-Spooling
*     spool/spool_startup_context()
*******************************************************************************/
bool
spool_template_default_startup_func(lList **answer_list, 
                                    const lListElem *rule, bool check)
{
   bool ret = true;
   const char *url;

   DENTER(TOP_LAYER, "spool_template_default_startup_func");

   url = lGetString(rule, SPR_url);

   DEXIT;
   return ret;
}

/****** spool/template/spool_template_default_shutdown_func() **************
*  NAME
*     spool_template_default_shutdown_func() -- shutdown spooling context
*
*  SYNOPSIS
*     bool 
*     spool_template_default_shutdown_func(lList **answer_list, 
*                                          lListElem *rule);
*
*  FUNCTION
*     Shuts down the context, e.g. the database connection.
*
*  INPUTS
*     lList **answer_list - to return error messages
*     const lListElem *rule - the rule containing data necessary for
*                             the shutdown (e.g. path to the spool directory)
*
*  RESULT
*     bool - true, if the shutdown succeeded, else false
*
*  NOTES
*     This function should not be called directly, it is called by the
*     spooling framework.
*
*  SEE ALSO
*     spool/template/--Spooling-Template
*     spool/spool_shutdown_context()
*******************************************************************************/
bool
spool_template_default_shutdown_func(lList **answer_list, 
                                    const lListElem *rule)
{
   bool ret = true;
   const char *url;

   DENTER(TOP_LAYER, "spool_template_default_shutdown_func");

   url = lGetString(rule, SPR_url);


   DEXIT;
   return ret;
}

/****** spool/template/spool_template_default_maintenance_func() ************
*  NAME
*     spool_template_default_maintenance_func() -- maintain database
*
*  SYNOPSIS
*     bool 
*     spool_template_default_maintenance_func(lList **answer_list, 
*                                    lListElem *rule
*                                    const spooling_maintenance_command cmd,
*                                    const char *args);
*
*  FUNCTION
*     Maintains the database:
*        - initialization
*        - ...
*
*  INPUTS
*     lList **answer_list   - to return error messages
*     const lListElem *rule - the rule containing data necessary for
*                             the maintenance (e.g. path to the spool 
*                             directory)
*     const spooling_maintenance_command cmd - the command to execute
*     const char *args      - arguments to the maintenance command
*
*  RESULT
*     bool - true, if the maintenance succeeded, else false
*
*  NOTES
*     This function should not be called directly, it is called by the
*     spooling framework.
*
*  SEE ALSO
*     spool/template/--Spooling-Template
*     spool/spool_maintain_context()
*******************************************************************************/
bool
spool_template_default_maintenance_func(lList **answer_list, 
                                    const lListElem *rule, 
                                    const spooling_maintenance_command cmd,
                                    const char *args)
{
   bool ret = true;

   DENTER(TOP_LAYER, "spool_template_default_maintenance_func");

   switch (cmd) {
      case SPM_init:
         break;
      default:
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_WARNING, 
                                 "unknown maintenance command %d\n", cmd);
         ret = false;
         break;
         
   }

   DEXIT;
   return ret;
}

/****** spool/template/spool_template_default_list_func() *****************
*  NAME
*     spool_template_default_list_func() -- read lists through template spooling
*
*  SYNOPSIS
*     bool 
*     spool_template_default_list_func(
*                                      lList **answer_list, 
*                                      const lListElem *type, 
*                                      const lListElem *rule, lList **list, 
*                                      const sge_object_type object_type) 
*
*  FUNCTION
*
*  INPUTS
*     lList **answer_list - to return error messages
*     const lListElem *type           - object type description
*     const lListElem *rule           - rule to be used 
*     lList **list                    - target list
*     const sge_object_type object_type - object type
*
*  RESULT
*     bool - true, on success, else false
*
*  NOTES
*     This function should not be called directly, it is called by the
*     spooling framework.
*
*  SEE ALSO
*     spool/template/--Template-Spooling
*     spool/spool_read_list()
*******************************************************************************/
bool
spool_template_default_list_func(lList **answer_list, 
                                 const lListElem *type, 
                                 const lListElem *rule, lList **list, 
                                 const sge_object_type object_type)
{
   bool ret = true;

   DENTER(TOP_LAYER, "spool_template_default_list_func");

   switch (object_type) {
      default:
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_WARNING, 
                                 MSG_SPOOL_SPOOLINGOFXNOTSUPPORTED_S, 
                                 object_type_get_name(object_type));
         ret = false;
         break;
   }

   ret = spool_default_validate_list_func(answer_list, type, rule, object_type);

   DEXIT;
   return ret;
}

/****** spool/template/spool_template_default_read_func() *****************
*  NAME
*     spool_template_default_read_func() -- read objects through template spooling
*
*  SYNOPSIS
*     lListElem* 
*     spool_template_default_read_func(lList **answer_list, 
*                                      const lListElem *type, 
*                                      const lListElem *rule, const char *key, 
*                                      const sge_object_type object_type) 
*
*  FUNCTION
*
*  INPUTS
*     lList **answer_list - to return error messages
*     const lListElem *type           - object type description
*     const lListElem *rule           - rule to use
*     const char *key                 - unique key specifying the object
*     const sge_object_type object_type - object type
*
*  RESULT
*     lListElem* - the object, if it could be read, else NULL
*
*  NOTES
*     This function should not be called directly, it is called by the
*     spooling framework.
*
*  SEE ALSO
*     spool/template/--Template-Spooling
*     spool/spool_read_object()
*******************************************************************************/
lListElem *
spool_template_default_read_func(lList **answer_list, 
                                 const lListElem *type, 
                                 const lListElem *rule, const char *key, 
                                 const sge_object_type object_type)
{
   lListElem *ep = NULL;

   DENTER(TOP_LAYER, "spool_template_default_read_func");

   switch (object_type) {
      default:
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_WARNING, 
                                 MSG_SPOOL_SPOOLINGOFXNOTSUPPORTED_S, 
                                 object_type_get_name(object_type));
         break;
   }

   DEXIT;
   return ep;
}

/****** spool/template/spool_template_default_write_func() ****************
*  NAME
*     spool_template_default_write_func() -- write objects through template spooling
*
*  SYNOPSIS
*     bool
*     spool_template_default_write_func(lList **answer_list, 
*                                       const lListElem *type, 
*                                       const lListElem *rule, 
*                                       const lListElem *object, 
*                                       const char *key, 
*                                       const sge_object_type object_type) 
*
*  FUNCTION
*     Writes an object through the appropriate template spooling functions.
*
*  INPUTS
*     lList **answer_list - to return error messages
*     const lListElem *type           - object type description
*     const lListElem *rule           - rule to use
*     const lListElem *object         - object to spool
*     const char *key                 - unique key
*     const sge_object_type object_type - object type
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     This function should not be called directly, it is called by the
*     spooling framework.
*
*  SEE ALSO
*     spool/template/--Template-Spooling
*     spool/spool_delete_object()
*******************************************************************************/
bool
spool_template_default_write_func(lList **answer_list, 
                                  const lListElem *type, 
                                  const lListElem *rule, 
                                  const lListElem *object, 
                                  const char *key, 
                                  const sge_object_type object_type)
{
   bool ret = true;

   DENTER(TOP_LAYER, "spool_template_default_write_func");

   switch (object_type) {
      default:
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_WARNING, 
                                 MSG_SPOOL_SPOOLINGOFXNOTSUPPORTED_S, 
                                 object_type_get_name(object_type));
         ret = false;
         break;
   }

   DEXIT;
   return ret;
}

/****** spool/template/spool_template_default_delete_func() ***************
*  NAME
*     spool_template_default_delete_func() -- delete object in template spooling
*
*  SYNOPSIS
*     bool
*     spool_template_default_delete_func(lList **answer_list, 
*                                        const lListElem *type, 
*                                        const lListElem *rule, 
*                                        const char *key, 
*                                        const sge_object_type object_type) 
*
*  FUNCTION
*     Deletes an object in the template spooling.
*
*  INPUTS
*     lList **answer_list - to return error messages
*     const lListElem *type           - object type description
*     const lListElem *rule           - rule to use
*     const char *key                 - unique key 
*     const sge_object_type object_type - object type
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     This function should not be called directly, it is called by the
*     spooling framework.
*
*  SEE ALSO
*     spool/template/--Template-Spooling
*     spool/spool_delete_object()
*******************************************************************************/
bool
spool_template_default_delete_func(lList **answer_list, 
                                   const lListElem *type, 
                                   const lListElem *rule,
                                   const char *key, 
                                   const sge_object_type object_type)
{
   bool ret = true;

   DENTER(TOP_LAYER, "spool_template_default_delete_func");

   switch (object_type) {
      default:
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_WARNING, 
                                 MSG_SPOOL_SPOOLINGOFXNOTSUPPORTED_S, 
                                 object_type_get_name(object_type));
         ret = false;
         break;
   }

   DEXIT;
   return ret;
}

