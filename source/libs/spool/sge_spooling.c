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
#include "uti/sge_profiling.h"
#include "uti/sge_bootstrap.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_spooling_SPC_L.h"
#include "sgeobj/sge_spooling_SPR_L.h"
#include "sgeobj/sge_spooling_SPT_L.h"
#include "sgeobj/sge_spooling_SPTR_L.h"
#include "sgeobj/sge_spooling_SPM_L.h"

#include "spool/msg_spoollib.h"
#include "spool/sge_spooling.h"

static lListElem *Default_Spool_Context;

/****** spool/spool_create_context() ************************************
*  NAME
*     spool_create_context() -- create a new spooing context
*
*  SYNOPSIS
*     lListElem *
*     spool_create_context(lList **answer_list, const char *name);
*
*  FUNCTION
*     Create a new spooling context.
*
*  INPUTS
*     lList **answer_list  - to return error messages
*     const char *name     - name of the context
*
*  RESULT
*     lListElem* - the new spooling context
*
*  EXAMPLE
*     lListElem *context;
*     
*     context = spool_create_context(answer_list, "my spooling context");
*     ...
*
*
*  NOTES
*     Usually, a service function creating a spooling context
*     for a certain storage system will be called, e.g. 
*     spool_classic_create_context().
*
*  SEE ALSO
*     spool/--Spooling
*     spool/spool_free_context()
*     spool/classic/spool_classic_create_context()
*******************************************************************************/
lListElem *
spool_create_context(lList **answer_list, const char *name)
{
   lListElem *ep = NULL;

   DENTER(TOP_LAYER, "spool_create_context");
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);

   if (name == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR,
                              "%s", MSG_SPOOL_CONTEXTNEEDSNAME);
   } else {
      ep = lCreateElem(SPC_Type);
      lSetString(ep, SPC_name, name);
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(ep);
}

/****** spool/spool_free_context() **************************************
*  NAME
*     spool_free_context() -- free resources of a spooling context
*
*  SYNOPSIS
*     lListElem* 
*     spool_free_context(lList **answer_list, lListElem *context) 
*
*  FUNCTION
*     Performs a shutdown of the spooling context and releases
*     all allocated resources.
*
*  INPUTS
*     lList **answer_list  - to return error messages
*     lListElem *context   - the context to free
*
*  RESULT
*     lListElem* - NULL
*
*  EXAMPLE
*     lListElem *context;
*     ...
*     context = spool_free_context(answer_list, context);
*
*  SEE ALSO
*     spool/--Spooling
*     spool/spool_create_context()
*     spool/spool_shutdown_context()
*******************************************************************************/
lListElem *
spool_free_context(lList **answer_list, lListElem *context)
{
   DENTER(TOP_LAYER, "spool_free_context");
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);
  
   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S, 
                              SGE_FUNC);
   } else {
      spool_shutdown_context(answer_list, context);
      lFreeElem(&context);
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(context);
}

bool
spool_set_option(lList **answer_list, lListElem *context, const char *option)
{
   bool ret = true;

   DENTER(TOP_LAYER, "spool_set_option");
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);
  
   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S, 
                              SGE_FUNC);
   } else {
      lListElem *rule;

      for_each (rule, lGetList(context, SPC_rules)) {
         spooling_option_func func = (spooling_option_func)
                                       lGetRef(rule, SPR_option_func);
         if (func != NULL) {
            if (!func(answer_list, rule, option)) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_SPOOL_SETOPTIONOFRULEFAILED_SS,
                                       lGetString(rule, SPR_name), 
                                       lGetString(context, SPC_name));
               ret = false;
               break;
            }
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(ret);
}

/****** spool/spool_startup_context() ***********************************
*  NAME
*     spool_startup_context() -- startup a spooling context
*
*  SYNOPSIS
*     bool 
*     spool_startup_context(lList **answer_list, lListElem *context, bool check)
*
*  FUNCTION
*     Starts up the spooling context.
*     Checks consistency of the spooling context.
*
*     Then the startup callback for all rules will be called, which will 
*     startup the different rules.
*
*     For file based spooling, this can been to chdir into the spool directory,
*     for database spooling it means opening the database connection.
*
*     If the parameter check is set to true, the startup callbacks
*     check the data base, e.g. if the spooling database
*     was created for the current Grid Engine version.
*     This check shall be done for all operations, except when creating the
*     database.
*
*
*  INPUTS
*     lList **answer_list  - to return error messages
*     lListElem *context   - the context to startup
*     bool check           - check database?
*
*  RESULT
*     bool - true, if the context is OK and all startup callbacks reported
*                 success,
*            else false
*
*  SEE ALSO
*     spool/--Spooling
*     spool/spool_shutdown_context()
*******************************************************************************/
bool 
spool_startup_context(lList **answer_list, lListElem *context, bool check)
{
   bool ret = true;

   DENTER(TOP_LAYER, "spool_startup_context");
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);

   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S,
                              SGE_FUNC);
      ret = false;
   } else if (lGetNumberOfElem(lGetList(context, SPC_types)) == 0) {
      /* check consistency: the context has to contain types */
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_SPOOL_CONTEXTCONTAINSNOTYPES_S, 
                              lGetString(context, SPC_name));
      ret = false;
   } else {
      lListElem *rule, *type;

      /* each type needs at least one rule and exactly one default rule */
      for_each(type, lGetList(context, SPC_types)) {
         lListElem *type_rule;
         int default_rules = 0;
         
         if (lGetNumberOfElem(lGetList(type, SPT_rules)) == 0) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_SPOOL_TYPECONTAINSNORULES_SS, 
                                    lGetString(type, SPT_name),
                                    lGetString(context, SPC_name));
            ret = false;
            goto error;
         }

         /* count default rules */
         for_each (type_rule, lGetList(type, SPT_rules)) {
            if(lGetBool(type_rule, SPTR_default)) {
               default_rules++;
            }
         }
         
         if (default_rules == 0) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_SPOOL_TYPEHASNODEFAULTRULE_SS,
                                    lGetString(type, SPT_name),
                                    lGetString(context, SPC_name));
            ret = false;
            goto error;
         }

         if (default_rules > 1) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_SPOOL_TYPEHASMORETHANONEDEFAULTRULE_SS,
                                    lGetString(type, SPT_name),
                                    lGetString(context, SPC_name));
            ret = false;
            goto error;
         }
      }
    
      /* the context has to contain rules */
      if (lGetNumberOfElem(lGetList(context, SPC_rules)) == 0) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_SPOOL_CONTEXTCONTAINSNORULES_S, 
                                 lGetString(context, SPC_name));
         ret = false;
         goto error;
      }
      
      for_each (rule, lGetList(context, SPC_rules)) {
         spooling_startup_func func = (spooling_startup_func)
                                      lGetRef(rule, SPR_startup_func);
         if (func != NULL) {
            if (!func(answer_list, rule, check)) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_SPOOL_STARTUPOFRULEFAILED_SS,
                                       lGetString(rule, SPR_name), 
                                       lGetString(context, SPC_name));
               ret = false;
               goto error;
            }
         }
      }
   }

error:
   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(ret);
}

/****** spool/spool_maintain_context() **********************************
*  NAME
*     spool_maintain_context() -- maintain a context
*
*  SYNOPSIS
*     bool 
*     spool_maintain_context(lList **answer_list, lListElem *context,
*                            const spooling_maintenance_command cmd,
*                            const char *args) 
*
*  FUNCTION
*     Do maintenance on spooling context's database.
*     Calls the maintenance callback for all defined spooling rules.
*     These callbacks will
*        - initialize the database
*        - backup
*        - switch between spooling with/without history
*        - etc.
*
*  INPUTS
*     lList **answer_list - to return error messages
*     lListElem *context  - the context to maintain
*     const char *args    - arguments to maintenance callback
*
*  RESULT
*     bool - true, if all maintenance callbacks reported success,
*            else false
*
*  SEE ALSO
*     spool/--Spooling
*     spool/spool_startup_context()
*******************************************************************************/
bool 
spool_maintain_context(lList **answer_list, lListElem *context, 
                       const spooling_maintenance_command cmd,
                       const char *args)
{
   bool ret = true;

   DENTER(TOP_LAYER, "spool_maintain_context");
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);

   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S,
                              SGE_FUNC);
      ret = false;
   } else {
      lListElem *rule;

      for_each (rule, lGetList(context, SPC_rules)) {
         spooling_maintenance_func func = (spooling_maintenance_func)
                                       lGetRef(rule, SPR_maintenance_func);
         if (func != NULL) {
            if (!func(answer_list, rule, cmd, args)) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_SPOOL_MAINTENANCEOFRULEFAILED_SS,
                                       lGetString(rule, SPR_name), 
                                       lGetString(context, SPC_name));
               ret = false;
               break;
            }
         }
      }
   }
   
   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(ret);
}


/****** spool/spool_shutdown_context() **********************************
*  NAME
*     spool_shutdown_context() -- shutdown a context
*
*  SYNOPSIS
*     bool 
*     spool_shutdown_context(lList **answer_list, lListElem *context) 
*
*  FUNCTION
*     Shut down a spooling context.
*     Calls the shutdown callback for all defined spooling rules.
*     Usually these callbacks will flush unwritten data, close
*     file handles, close database connections etc.
*
*     A context that has been shutdown can be reused by calling
*     spool_startup_context()
*
*  INPUTS
*     lList **answer_list  - to return error messages
*     lListElem *context - the context to shutdown
*
*  RESULT
*     bool - true, if all shutdown callbacks reported success,
*            else false
*
*  SEE ALSO
*     spool/--Spooling
*     spool/spool_startup_context()
*******************************************************************************/
bool 
spool_shutdown_context(lList **answer_list, lListElem *context)
{
   bool ret = true;

   DENTER(TOP_LAYER, "spool_shutdown_context");
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);

   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S,
                              SGE_FUNC);
      ret = false;
   } else {
      lListElem *rule;

      for_each (rule, lGetList(context, SPC_rules)) {
         spooling_shutdown_func func = (spooling_shutdown_func)
                                       lGetRef(rule, SPR_shutdown_func);
         if (func != NULL) {
            if (!func(answer_list, rule)) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_SPOOL_SHUTDOWNOFRULEFAILED_SS,
                                       lGetString(rule, SPR_name), 
                                       lGetString(context, SPC_name));
               ret = false;
               break;
            }
         }
      }
   }
   
   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(ret);
}

bool
spool_trigger_context(lList **answer_list, lListElem *context,
                      time_t trigger, time_t *next_trigger)
{
   bool ret = true;

   DENTER(TOP_LAYER, "spool_trigger_context");
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);

   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S,
                              SGE_FUNC);
      ret = false;
   } else {
      lListElem *rule;

      for_each (rule, lGetList(context, SPC_rules)) {
         spooling_trigger_func func = (spooling_trigger_func)
                                       lGetRef(rule, SPR_trigger_func);
         if (func != NULL) {
            if (!func(answer_list, rule, trigger, next_trigger)) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_SPOOL_TRIGGEROFRULEFAILED_SS,
                                       lGetString(rule, SPR_name), 
                                       lGetString(context, SPC_name));
               ret = false;
               break;
            }
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(ret);
}

bool spool_transaction(lList **answer_list, lListElem *context, 
                       spooling_transaction_command cmd)
{
   bool ret = true;

   DENTER(TOP_LAYER, "spool_transaction");
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);

   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S,
                              SGE_FUNC);
      ret = false;
   } else {
      lListElem *rule;

      for_each (rule, lGetList(context, SPC_rules)) {
         spooling_transaction_func func = (spooling_transaction_func)
                                       lGetRef(rule, SPR_transaction_func);
         if (func != NULL) {
            if (!func(answer_list, rule, cmd)) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_SPOOL_TRANSACTIONOFRULEFAILED_SS,
                                       lGetString(rule, SPR_name), 
                                       lGetString(context, SPC_name));
               ret = false;
               break;
            }
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(ret);
}


/****** spool/spool_set_default_context() *******************************
*  NAME
*     spool_set_default_context() -- set a default context
*
*  SYNOPSIS
*     void spool_set_default_context(lListElem *context) 
*
*  FUNCTION
*     The spooling framework can have a default context.
*     A context that has been created before can be set as 
*     default context using this function.
*     The default context can be retrieved later with the function
*     spool_get_default_context().
*
*  INPUTS
*     lListElem *context - the context to be the default context
*
*  SEE ALSO
*     spool/--Spooling
*     spool/spool_get_default_context()
*******************************************************************************/
void spool_set_default_context(lListElem *context)
{
   Default_Spool_Context = context;
}

/****** spool/spool_get_default_context() *******************************
*  NAME
*     spool_get_default_context() -- retrieve the default spooling context 
*
*  SYNOPSIS
*     lListElem* 
*     spool_get_default_context(void) 
*
*  FUNCTION
*     Retrieves a spooling context that has been set earlier using the function
*     spool_set_default_context()
*
*  RESULT
*     lListElem* - the spooling context, or NULL, if no default context
*                  has been set.
*
*  SEE ALSO
*     spool/--Spooling
*     spool/spool_set_default_context()
*******************************************************************************/
lListElem *
spool_get_default_context(void)
{
   return Default_Spool_Context;
}

/****** spool/spool_context_search_rule() *******************************
*  NAME
*     spool_context_search_rule() -- search a certain rule 
*
*  SYNOPSIS
*     lListElem* 
*     spool_context_search_rule(const lListElem *context, const char *name) 
*
*  FUNCTION
*     Searches a certain rule (given by its name) in a given spooling context.
*
*  INPUTS
*     const lListElem *context - the context to search
*     const char *name         - name of the rule
*
*  RESULT
*     lListElem* - the rule, if it exists, else NULL
*
*  SEE ALSO
*     spool/--Spooling
*******************************************************************************/
lListElem *
spool_context_search_rule(const lListElem *context, const char *name)
{
   return lGetElemStr(lGetList(context, SPC_rules), SPR_name, name);
}

/****** spool/spool_context_create_rule() *******************************
*  NAME
*     spool_context_create_rule() -- create a rule in a spooling context
*
*  SYNOPSIS
*     lListElem* 
*     spool_context_create_rule(lList **answer_list, lListElem *context, 
*                               const char *name, const char *url, 
*                               spooling_option_func option_func, 
*                               spooling_startup_func startup_func, 
*                               spooling_shutdown_func shutdown_func,
*                               spooling_maintenance_func maintenance_func,
*                               spooling_list_func list_func, 
*                               spooling_read_func read_func, 
*                               spooling_write_func write_func, 
*                               spooling_delete_func delete_func) 
*                               spooling_validate_func validate_func) 
*
*  FUNCTION
*     Creates a rule in the given context and assigns it the given attributes.
*
*  INPUTS
*     lList **answer_list  - to return error messages
*     lListElem *context                   - the context to contain the new rule
*     const char *name                     - the name of the rule
*     const char *url                      - the name of the url
*     spooling_option_func option_func     - function to set options for the rule
*     spooling_startup_func startup_func   - startup function for the rule
*     spooling_shutdown_func shutdown_func - shutdown function
*     spooling_maintenance_func shutdown_func - maintenance function 
*                                               (initializaion, backup, etc.)
*     spooling_list_func list_func         - function reading a list of objects
*     spooling_read_func read_func         - function reading an individual 
*                                            object 
*     spooling_write_func write_func       - function writing an individual 
*                                            object
*     spooling_delete_func delete_func     - function deleting an individual 
*                                            object
*     spooling_validate_func validate_func     - function validateing an individual 
*                                                object
*     spooling validate_list_func validate_list_func - function for validating 
*                                                      the new list
*
*  RESULT
*     lListElem* - the new rule, if it could be created, else NULL
*
*  SEE ALSO
*     spool/--Spooling
*******************************************************************************/
lListElem *
spool_context_create_rule(lList **answer_list, lListElem *context, 
                          const char *name, const char *url,
                          spooling_option_func option_func, 
                          spooling_startup_func startup_func, 
                          spooling_shutdown_func shutdown_func, 
                          spooling_maintenance_func maintenance_func, 
                          spooling_trigger_func trigger_func, 
                          spooling_transaction_func transaction_func, 
                          spooling_list_func list_func, 
                          spooling_read_func read_func, 
                          spooling_write_func write_func, 
                          spooling_delete_func delete_func,
                          spooling_validate_func validate_func,
                          spooling_validate_list_func validate_list_func)
{
   lListElem *ep = NULL;

   DENTER(TOP_LAYER, "spool_context_create_rule");
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);

   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S,
                              SGE_FUNC);
   } else if (lGetElemStr(lGetList(context, SPC_rules), SPR_name, name) 
             != NULL) {
      /* check for duplicates */
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_SPOOL_RULEALREADYEXISTS_SS, 
                              name, lGetString(context, SPC_name));
   } else {
      lList *lp;

      /* create rule */
      ep = lCreateElem(SPR_Type);
      lSetString(ep, SPR_name, name);
      lSetString(ep, SPR_url, url);
      lSetRef(ep, SPR_option_func, (void *)option_func);
      lSetRef(ep, SPR_startup_func, (void *)startup_func);
      lSetRef(ep, SPR_shutdown_func, (void *)shutdown_func);
      lSetRef(ep, SPR_maintenance_func, (void *)maintenance_func);
      lSetRef(ep, SPR_trigger_func, (void *)trigger_func);
      lSetRef(ep, SPR_transaction_func, (void *)transaction_func);
      lSetRef(ep, SPR_list_func, (void *)list_func);
      lSetRef(ep, SPR_read_func, (void *)read_func);
      lSetRef(ep, SPR_write_func, (void *)write_func);
      lSetRef(ep, SPR_delete_func, (void *)delete_func);
      lSetRef(ep, SPR_validate_func, (void *)validate_func);
      lSetRef(ep, SPR_validate_list_func, (void *)validate_list_func);

      /* append rule to rule list */
      lp = lGetList(context, SPC_rules);
      if (lp == NULL) {
         lp = lCreateList("spooling rules", SPR_Type);
         lSetList(context, SPC_rules, lp);
      }

      lAppendElem(lp, ep);
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(ep);
}

/****** spool/spool_context_search_type() *******************************
*  NAME
*     spool_context_search_type() -- search an object type description
*
*  SYNOPSIS
*     lListElem* 
*     spool_context_search_type(const lListElem *context, 
*                               const  sge_object_type object_type) 
*
*  FUNCTION
*     Searches the object type description with the given type in the 
*     given context.
*     If no specific description for the given type is found, but a 
*     default type description (for all object types) exists, this
*     default type description is returned.
*
*  INPUTS
*     const lListElem *context        - the context to search
*     const sge_object_type object_type - the object type to search
*
*  RESULT
*     lListElem* - an object type description or NULL, if none was found.
*
*  SEE ALSO
*     spool/--Spooling
*******************************************************************************/
lListElem *
spool_context_search_type(const lListElem *context, 
                          const sge_object_type object_type)
{
   lListElem *ep;

   /* search fitting rule */
   ep = lGetElemUlong(lGetList(context, SPC_types), SPT_type, object_type);

   /* if no specific rule is found, return default rule */
   if (ep == NULL) {
      ep = lGetElemUlong(lGetList(context, SPC_types), SPT_type, SGE_TYPE_ALL);
   }
   
   return ep;
}

/****** spool/spool_context_create_type() *******************************
*  NAME
*     spool_context_create_type() -- create an object type description 
*
*  SYNOPSIS
*     lListElem* 
*     spool_context_create_type(lList **answer_list, lListElem *context, 
*                               const sge_object_type object_type) 
*
*  FUNCTION
*     Creates a new description how a certain object type shall be 
*     spooled.
*
*     If the given object_type is SGE_TYPE_ALL, the description will
*     be the default for object types that are not individually
*     handled.
*
*  INPUTS
*     lList **answer_list  - to return error messages
*     lListElem *context              - the context to contain the new 
*                                       description
*     const sge_object_type object_type - the object type
*
*  RESULT
*     lListElem* - the new object type description
*
*  SEE ALSO
*     spool/--Spooling
*******************************************************************************/
lListElem *
spool_context_create_type(lList **answer_list, lListElem *context, 
                          const sge_object_type object_type)
{
   lListElem *ep = NULL;

   DENTER(TOP_LAYER, "spool_context_create_type");
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);

   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S,
                              SGE_FUNC);
   } else {
      lList *lp;

      /* create new type */
      ep = lCreateElem(SPT_Type);
      lSetUlong(ep, SPT_type, object_type);
      lSetString(ep, SPT_name, object_type_get_name(object_type));
    
      /* append it to the types list of the context */
      lp = lGetList(context, SPC_types);
      if (lp == NULL) {
         lp = lCreateList("spooling object types", SPT_Type);
         lSetList(context, SPC_types, lp);
      }

      lAppendElem(lp, ep);
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(ep);
}

/****** spool/spool_type_search_default_rule() **************************
*  NAME
*     spool_type_search_default_rule() -- search the default rule
*
*  SYNOPSIS
*     lListElem* 
*     spool_type_search_default_rule(const lListElem *spool_type) 
*
*  FUNCTION
*     Searches and returns the default spooling rule for a certain object type.
*
*  INPUTS
*     const lListElem *spool_type - the object type
*
*  RESULT
*     lListElem* - the default rule, or NULL, if no rule could be found.
*
*  SEE ALSO
*     spool/--Spooling
*******************************************************************************/
lListElem *
spool_type_search_default_rule(const lListElem *spool_type)
{  
   lList *lp;
   lListElem *ep;
   lListElem *rule = NULL;

   lp = lGetList(spool_type, SPT_rules);
   for_each (ep, lp) {
      if (lGetBool(ep, SPTR_default)) {
         rule = (lListElem *)lGetRef(ep, SPTR_rule);
         break;
      }
   }

   return rule;
}

/****** spool/spool_type_add_rule() *************************************
*  NAME
*     spool_type_add_rule() -- adds a rule for a spooling object type 
*
*  SYNOPSIS
*     lListElem* 
*     spool_type_add_rule(lList **answer_list, lListElem *spool_type, 
*                         const lListElem *rule, lBool is_default) 
*
*  FUNCTION
*     Adds a spooling rule to an object type description.
*     The rule can be installed as default rule for this object type.
*
*  INPUTS
*     lList **answer_list  - to return error messages
*     lListElem *spool_type - the object type description
*     const lListElem *rule - the rule to add 
*     lBool is_default      - is the rule the default rule?
*
*  RESULT
*     lListElem* - the newly created mapping object between type and rule 
*                  (SPTR_Type), or NULL, if an error occured.
*
*  SEE ALSO
*     spool/--Spooling
*     spool/spool_context_create_type()
*     spool/spool_context_create_rule()
*******************************************************************************/
lListElem *
spool_type_add_rule(lList **answer_list, lListElem *spool_type, 
                    const lListElem *rule, lBool is_default)
{
   lListElem *ep = NULL;

   DENTER(TOP_LAYER, "spool_type_add_rule");
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);

   if(spool_type == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_SPOOL_NOVALIDSPOOLTYPE_S, SGE_FUNC);
   } else if(rule == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDRULE_S,
                              SGE_FUNC);
   } else if(is_default && spool_type_search_default_rule(spool_type) != NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_SPOOL_TYPEALREADYHASDEFAULTRULE_S, 
                              lGetString(spool_type, SPT_name));
   } else {
      lList *lp;

      /* create mapping object */
      ep = lCreateElem(SPTR_Type);
      lSetBool(ep, SPTR_default, is_default);
      lSetString(ep, SPTR_rule_name, lGetString(rule, SPR_name));
      lSetRef(ep, SPTR_rule, (void *)rule);

      /* append it to the list of mapping for this type */
      lp = lGetList(spool_type, SPT_rules);
      if (lp == NULL) {
         lp = lCreateList("spooling object type rules", SPTR_Type);
         lSetList(spool_type, SPT_rules, lp);
      }
      
      lAppendElem(lp, ep);
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(ep);
}

/****** spool/spool_read_list() *****************************************
*  NAME
*     spool_read_list() -- read a list of objects from spooled data
*
*  SYNOPSIS
*     bool
*     spool_read_list(lList **answer_list, const lListElem *context, 
*                     lList **list, const sge_object_type object_type) 
*
*  FUNCTION
*     Read the list of objects associated with a certain object type
*     from the spooled data and store it into the given list.
*
*     The function will call the read_list callback from the default rule
*     for the given object type.
*
*  INPUTS
*     lList **answer_list  - to return error messages
*     const lListElem *context        - the context to use for reading
*     lList **list                    - the target list
*     const sge_object_type object_type - the object type
*
*  RESULT
*     int - true, on success, false, if an error occured
*
*  EXAMPLE
*     spool_read_list(context, object_type_get_master_list(SGE_TYPE_JOB), SGE_TYPE_JOB);
*     will read the job list.
*
*  SEE ALSO
*     spool/--Spooling
*******************************************************************************/
bool 
spool_read_list(lList **answer_list, const lListElem *context, 
                lList **list, const sge_object_type object_type)
{
   bool ret = false;

   DENTER(TOP_LAYER, "spool_read_list");
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);

   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S,
                              SGE_FUNC);
   } else {
      lListElem *type;

      /* find the object type description */
      type = spool_context_search_type(context, object_type);
      if (type == NULL) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_SPOOL_UNKNOWNOBJECTTYPEINCONTEXT_SS, 
                                 object_type_get_name(object_type), 
                                 lGetString(context, SPC_name));
      } else {
         lListElem *rule;

         /* use the default rule to read list */
         rule = spool_type_search_default_rule(type);
         if (rule == NULL) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_SPOOL_NODEFAULTRULEFORTYPEINCONTEXT_SS,
                                    object_type_get_name(object_type), 
                                    lGetString(context, SPC_name));
         } else {
            spooling_list_func func;

            /* read and call the list callback function */
            func = (spooling_list_func)lGetRef(rule, SPR_list_func);
            if (func == NULL) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_SPOOL_CORRUPTRULEINCONTEXT_SSS,
                                       lGetString(rule, SPR_name), 
                                       lGetString(context, SPC_name),
                                       SGE_FUNC);
            } else {
               ret = func(answer_list, type, rule, list, object_type);
            }
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(ret);
}

/****** spool/spool_read_object() ***************************************
*  NAME
*     spool_read_object() -- read a single object from spooled data
*
*  SYNOPSIS
*     lListElem* 
*     spool_read_object(lList **answer_list, const lListElem *context, 
*                       const  sge_object_type object_type, const char *key) 
*
*  FUNCTION
*     Read an objects characterized by its type and a unique key
*     from the spooled data.
*
*     The function will call the read callback from the default rule
*     for the given object type.
*
*  INPUTS
*     lList **answer_list  - to return error messages
*     const lListElem *context        - the context to use
*     const sge_object_type object_type - object type
*     const char *key                 - unique key
*
*  RESULT
*     lListElem* - the object, if it could be read, else NULL
*
*  SEE ALSO
*     spool/--Spooling
*******************************************************************************/
lListElem *
spool_read_object(lList **answer_list, const lListElem *context, 
                  const sge_object_type object_type, const char *key)
{
   lListElem *result = NULL;

   DENTER(TOP_LAYER, "spool_read_object");
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);

   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S,
                              SGE_FUNC);
   } else {
      lListElem *type;

      /* find the object type description */
      type = spool_context_search_type(context, object_type);
      if (type == NULL) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_SPOOL_UNKNOWNOBJECTTYPEINCONTEXT_SS,
                                 object_type_get_name(object_type), 
                                 lGetString(context, SPC_name));
      } else {
         lListElem *rule;

         /* use the default rule to read object */
         rule = spool_type_search_default_rule(type);
         if (rule == NULL) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_SPOOL_NODEFAULTRULEFORTYPEINCONTEXT_SS,
                                    object_type_get_name(object_type), 
                                    lGetString(context, SPC_name));
         } else {
            spooling_read_func func;

            /* retrieve and execute the read callback */
            func = (spooling_read_func)lGetRef(rule, SPR_read_func);
            if (func == NULL) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_SPOOL_CORRUPTRULEINCONTEXT_SSS,
                                       lGetString(rule, SPR_name), 
                                       lGetString(context, SPC_name),
                                       SGE_FUNC);
            } else {
               result = func(answer_list, type, rule, key, object_type);
            }
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(result);
}

/****** spool/spool_write_object() **************************************
*  NAME
*     spool_write_object() -- write (spool) a single object 
*
*  SYNOPSIS
*     bool 
*     spool_write_object(lList **answer_list, const lListElem *context, 
*                             const lListElem *object, const char *key, 
*                             const sge_object_type object_type,
*                             bool do_job_spooling) 
*
*  FUNCTION
*     Writes a single object using the given spooling context.
*     The function calls all rules associated with the object type
*     description for the given object type.
*
*  INPUTS
*     lList **answer_list  - to return error messages
*     const lListElem *context        - context to use
*     const lListElem *object         - object to spool
*     const char *key                 - unique key
*     const sge_object_type object_type - type of the object
*     bool  do_job_spooling - flag whether job_spooling shall be done
*
*  RESULT
*     bool - true, if writing was successfull, else false
*
*  SEE ALSO
*     spool/--Spooling
*******************************************************************************/
bool 
spool_write_object(lList **answer_list, const lListElem *context, 
                   const lListElem *object, const char *key, 
                   const sge_object_type object_type,
                   bool do_job_spooling)
{
   bool ret = false;
 
   DENTER(TOP_LAYER, "spool_write_object");

   switch (object_type) {

      case SGE_TYPE_JOB:
      case SGE_TYPE_JATASK:
      case SGE_TYPE_PETASK:
         if (!do_job_spooling) {
            DRETURN(true);
         }
         break;
      default : 
         break;
   }

   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);

   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S,
                              SGE_FUNC);
   } else {
      lListElem *type;

      /* find the object type description */
      type = spool_context_search_type(context, object_type);
      if (type == NULL) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_SPOOL_UNKNOWNOBJECTTYPEINCONTEXT_SS,
                                 object_type_get_name(object_type), 
                                 lGetString(context, SPC_name));
      } else {
         lList *type_rules;

         /* loop over all rules and call the writing callbacks */
         type_rules = lGetList(type, SPT_rules);
         if (type_rules == NULL || lGetNumberOfElem(type_rules) == 0) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_SPOOL_NORULESFORTYPEINCONTEXT_SS,
                                    object_type_get_name(object_type), 
                                    lGetString(context, SPC_name));
         } else {
            lListElem *type_rule;

            ret = true;

            /* spool using multiple rules */
            for_each (type_rule, type_rules) {
               lListElem *rule;
               spooling_write_func func;

               rule = (lListElem *)lGetRef(type_rule, SPTR_rule);
               func = (spooling_write_func)lGetRef(rule, SPR_write_func);
               if (func == NULL) {
                  answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                          ANSWER_QUALITY_ERROR, 
                                          MSG_SPOOL_CORRUPTRULEINCONTEXT_SSS,
                                          lGetString(rule, SPR_name), 
                                          lGetString(context, SPC_name),
                                          SGE_FUNC);
                   ret = false;
               } else {
                  if (!func(answer_list, type, rule, object, key, object_type)) {
                     answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                             ANSWER_QUALITY_WARNING, 
                                             MSG_SPOOL_RULEINCONTEXTFAILEDWRITING_SS,
                                             lGetString(rule, SPR_name), 
                                             lGetString(context, SPC_name));
                     ret = false;
                  }
               }
            }
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(ret);
}

/****** spool/spool_delete_object() *************************************
*  NAME
*     spool_delete_object() -- delete a single object 
*
*  SYNOPSIS
*     bool 
*     spool_delete_object(lList **answer_list, const lListElem *context, 
*                         const sge_object_type object_type, const char *key,
*                         bool do_job_spooling) 
*
*  FUNCTION
*     Deletes a certain object characterized by type and a unique key
*     in the spooled data.
*     Calls the delete callback in all rules defined for the given
*     object type.
*
*  INPUTS
*     lList **answer_list  - to return error messages
*     const lListElem *context        - the context to use
*     const sge_object_type object_type - object type
*     const char *key                 - unique key
*     bool  do_job_spooling - flag if job_spooling shall be done
*
*  RESULT
*     bool - true, if all rules reported success, else false
*
*  SEE ALSO
*     spool/--Spooling
*******************************************************************************/
bool 
spool_delete_object(lList **answer_list, const lListElem *context, 
                    const sge_object_type object_type, const char *key,
                    bool do_job_spooling)
{
   bool ret = false;
   
   DENTER(TOP_LAYER, "spool_delete_object");

   switch (object_type) {

      case SGE_TYPE_JOB:
      case SGE_TYPE_JATASK:
      case SGE_TYPE_PETASK:
            if (!do_job_spooling) {
               DRETURN(true);
            }
         break;
      default : 
         break;
   }
   
   PROF_START_MEASUREMENT(SGE_PROF_SPOOLING);

   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S,
                              SGE_FUNC);
   } else {
      lListElem *type;

      /* find the object type description */
      type = spool_context_search_type(context, object_type);
      if (type == NULL) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_SPOOL_UNKNOWNOBJECTTYPEINCONTEXT_SS,
                                 object_type_get_name(object_type), 
                                 lGetString(context, SPC_name));
      } else {
         lList *type_rules;

         /* loop over all rules and call the deleting callbacks */
         type_rules = lGetList(type, SPT_rules);
         if (type_rules == NULL || lGetNumberOfElem(type_rules) == 0) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_SPOOL_NORULESFORTYPEINCONTEXT_SS,
                                    object_type_get_name(object_type), 
                                    lGetString(context, SPC_name));
         } else {
            lListElem *type_rule;

            ret = true;

            /* delete object using all spooling rules */
            for_each (type_rule, type_rules) {
               lListElem *rule;
               spooling_delete_func func;

               rule = (lListElem *)lGetRef(type_rule, SPTR_rule);
               func = (spooling_delete_func)lGetRef(rule, SPR_delete_func);
               if (func == NULL) {
                  answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                          ANSWER_QUALITY_ERROR, 
                                          MSG_SPOOL_CORRUPTRULEINCONTEXT_SSS,
                                          lGetString(rule, SPR_name), 
                                          lGetString(context, SPC_name),
                                          SGE_FUNC);
                  ret = false;
               } else {
                  if (!func(answer_list, type, rule, key, object_type)) {
                     answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                             ANSWER_QUALITY_WARNING, 
                                             MSG_SPOOL_RULEINCONTEXTFAILEDWRITING_SS,
                                             lGetString(rule, SPR_name), 
                                             lGetString(context, SPC_name));
                     ret = false;
                  }
               }
            }
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLING);
   DRETURN(ret);
}

/****** spool/spool_compare_objects() ***********************************
*  NAME
*     spool_compare_objects() -- compare objects by spooled data
*
*  SYNOPSIS
*     bool
*     spool_compare_objects(lList **answer_list, const lListElem *context, 
*                           const sge_object_type object_type, 
*                           const lListElem *ep1, const lListElem *ep2) 
*
*  FUNCTION
*     Compares two objects by comparing only the attributes that shall be 
*     spooled.
*
*  INPUTS
*     lList **answer_list  - to return error messages
*     const lListElem *context        - context to use
*     const sge_object_type object_type - type of the object
*     const lListElem *ep1            - object 1
*     const lListElem *ep2            - object 2
*
*  RESULT
*     bool - false, if the objects have no differences, else true
*
*  NOTES
*     Not yet implemented. 
*     First the attributes to be spooled have to be defined in the 
*     object definitions (libs/gdi/sge_*L.h).
*
*  SEE ALSO
*     spool/--Spooling
*******************************************************************************/
bool
spool_compare_objects(lList **answer_list, const lListElem *context, 
                      const sge_object_type object_type, 
                      const lListElem *ep1, const lListElem *ep2)
{
   bool ret = false;

   DENTER(TOP_LAYER, "spool_compare_objects");

   if (context == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_SPOOL_NOVALIDCONTEXT_S,
                              SGE_FUNC);
   }

   ret = true;

   DRETURN(ret);
}
