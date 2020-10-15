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

#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_string.h"
#include "uti/sge_hostname.h"
#include "uti/sge_parse_num_par.h"

#include "gdi/sge_gdi.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_qinstance_type.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_subordinate.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/cull_parse_util.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_eval_expression.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "msg_common.h"

#define OBJECT_LAYER BASIS_LAYER

static lList *Master_Job_List = NULL;
static lList *Master_Zombie_List = NULL;
static lList *Master_Job_Schedd_Info_List = NULL;
static lList *Master_CEntry_List = NULL;
static lList *Master_HGroup_List = NULL;
static lList *Master_Userset_List = NULL;
static lList *Master_Project_List = NULL;
static lList *Master_User_List = NULL;
static lList *Master_CQueue_List = NULL;
static lList *Master_Exechost_List = NULL;
static lList *Master_Adminhost_List = NULL;
static lList *Master_Submithost_List = NULL;
static lList *Master_Calendar_List = NULL;
static lList *Master_Ckpt_List = NULL;
static lList *Master_Manager_List = NULL;
static lList *Master_Operator_List = NULL;
static lList *Master_Sharetree_List = NULL;
static lList *Master_Pe_List = NULL;
static lList *Master_SUser_List = NULL;
static lList *Master_RQS_List = NULL;
static lList *Master_AR_List = NULL;
static lList *Master_SchedulerConfig_List = NULL;

static lList *Master_Config_List = NULL;

#ifndef __SGE_NO_USERMAPPING__
static lList *Master_Cuser_List = NULL;
#endif


/* One entry per event type */
static object_description object_base[SGE_TYPE_ALL] = {
   /* master list                  set function              name                 descr      key               */
   { &Master_Adminhost_List,       NULL,                   "ADMINHOST",         AH_Type,   AH_name           },
   { &Master_Calendar_List,        NULL,                   "CALENDAR",          CAL_Type,  CAL_name          },
   { &Master_Ckpt_List,            NULL,                   "CKPT",              CK_Type,   CK_name           },
   { &Master_Config_List,          NULL,                   "CONFIG",            CONF_Type, CONF_name         },
   { NULL,                         NULL,                   "GLOBAL_CONFIG",     NULL,      NoName            },
   { &Master_Exechost_List,        NULL,                   "EXECHOST",          EH_Type,   EH_name           },
   { NULL,                         NULL,                   "JATASK",            JAT_Type,  JAT_task_number   },
   { NULL,                         NULL,                   "PETASK",            PET_Type,  PET_id            },
   { &Master_Job_List,             NULL,                   "JOB",               JB_Type,   JB_job_number     },
   { &Master_Job_Schedd_Info_List, NULL,                   "JOB_SCHEDD_INFO",   SME_Type,  NoName            },
   { &Master_Manager_List,         NULL,                   "MANAGER",           UM_Type,   UM_name           },
   { &Master_Operator_List,        NULL,                   "OPERATOR",          UO_Type,   UO_name           },
   { &Master_Sharetree_List,       NULL,                   "SHARETREE",         STN_Type,  STN_name          },
   { &Master_Pe_List,              NULL,                   "PE",                PE_Type,   PE_name           },
   { &Master_Project_List,         NULL,                   "PROJECT",           PR_Type,   PR_name           },
   { &Master_CQueue_List,          NULL,                   "CQUEUE",            CQ_Type,   CQ_name           },
   { NULL,                         NULL,                   "QINSTANCE",         QU_Type,   QU_qname          },
   { &Master_SchedulerConfig_List, sconf_validate_config_, "SCHEDD_CONF",       SC_Type,   NoName            },
   { NULL,                         NULL,                   "SCHEDD_MONITOR",    NULL,      NoName            },
   { NULL,                         NULL,                   "SHUTDOWN",          NULL,      NoName            },
   { NULL,                         NULL,                   "QMASTER_GOES_DOWN", NULL,      NoName            },
   { &Master_Submithost_List,      NULL,                   "SUBMITHOST",        SH_Type,   SH_name           },
   { &Master_User_List,            NULL,                   "USER",              UU_Type,   UU_name           },
   { &Master_Userset_List,         NULL,                   "USERSET",           US_Type,   US_name           },
   { &Master_HGroup_List,          NULL,                   "HOSTGROUP",         HGRP_Type, HGRP_name         },
   { &Master_CEntry_List,          NULL,                   "COMPLEX_ENTRY",     CE_Type,   CE_name           },
   { &Master_Zombie_List,          NULL,                   "ZOMBIE_JOBS",       JB_Type,   JB_job_number     },
   { &Master_SUser_List,           NULL,                   "SUBMIT_USER",       SU_Type,   SU_name           },
   { &Master_RQS_List,             NULL,                   "RQS",               RQS_Type,  RQS_name          },
   { &Master_AR_List,              NULL,                   "AR",                AR_Type,   AR_id             },
   { NULL,                         NULL,                   "JOBSCRIPT",         STU_Type,  STU_name          },
#ifndef __SGE_NO_USERMAPPING__
   { &Master_Cuser_List,           NULL,                   "USERMAPPING",       CU_Type,  CU_name            }
#endif
};

/*-------------------------*/
/* multithreading support  */
/*-------------------------*/

/* contains the information for the thread local structure. */
typedef struct {
   bool global;                                  /* thrue means, that the global structure is referenced */
   lList *lists[SGE_TYPE_ALL];                   /* stores the list information */
   object_description object_base[SGE_TYPE_ALL]; /* subscription handlers */ 
} obj_state_t;

/* the key for the thread local memeory */
static pthread_key_t   obj_state_key;

static pthread_once_t obj_once = PTHREAD_ONCE_INIT;

static void 
obj_state_destroy(void* st);

static void
obj_thread_local_once_init(void);

static void
obj_thread_local_once_init(void) 
{
   pthread_key_create(&obj_state_key, obj_state_destroy);
}

static bool 
object_parse_raw_field_from_string(lListElem *object, lList **answer_list, 
                                   const int nm, const char *value);

static const char *
object_append_raw_field_to_dstring(const lListElem *object, lList **answer_list,
                                   dstring *buffer, const int nm,
                                   char string_quotes);

/****** sge_object/obj_state_init() ********************************************
*  NAME
*     obj_state_init() --  Inits the thread local memory none global
*
*  SYNOPSIS
*     static void obj_state_init(obj_state_t *state) 
*
*  FUNCTION
*     Inits the thread local memory, by coping the static information and 
*     setting the list pointers to NULL
*
*  INPUTS
*     obj_state_t *state - the thread local memory
*
*  NOTES
*     MT-NOTE: obj_state_init() is MT safe 
*
*  SEE ALSO
*     sge_object/obj_state_global_init
*     sge_object/obj_state_destroy
*     sge_object/obj_mt_init
*******************************************************************************/
static void obj_state_init(obj_state_t *state) 
{
   int i;

   state->global = false;

   memcpy(state->object_base, object_base, sizeof(object_description) * SGE_TYPE_ALL);
  
   /* initialize mirroring data structures - only changeable fields */
   for (i = 0; i < SGE_TYPE_ALL; i++) {
      state->lists[i] = NULL;
      state->object_base[i].list = &(state->lists[i]);                          /* master list                    */
 
   }
}

/****** sge_object/obj_state_global_init() ************************************
*  NAME
*     obj_state_global_init() --  Inits the thread local memory global
*
*  SYNOPSIS
*     static void obj_state_global_init(obj_state_t *state) 
*
*  FUNCTION
*     Inits the thread local memory, by coping the static information and 
*     setting the list pointers to NULL
*
*  INPUTS
*     obj_state_t *state - the thread local memory
*
*  NOTES
*     MT-NOTE: obj_state_global_init() is MT safe 
*
*  SEE ALSO
*     sge_object/obj_state_init
*     sge_object/obj_state_destroy
*     sge_object/obj_mt_init
*******************************************************************************/
static void obj_state_global_init(obj_state_t* state) 
{
   int i;

   DENTER(TOP_LAYER, "obj_state_global_init");

   if (state != NULL) {
      state->global=true;
      memcpy(state->object_base, object_base, sizeof(object_description) * SGE_TYPE_ALL);

      /* initialize mirroring data structures - only changeable fields */
      for (i = 0; i < SGE_TYPE_ALL; i++) {
         state->lists[i] = NULL;
         state->object_base[i].list = object_base[i].list;
      }
   }
   else {
      /* SG: we need a error message */
      abort();
   }
   DEXIT;
}


/****** sge_object/obj_state_destroy() *****************************************
*  NAME
*     obj_state_destroy() -- frees the thread local memory
*
*  SYNOPSIS
*     static void obj_state_destroy(void* st) 
*
*  INPUTS
*     void* st - thread local memory
*
*  NOTES
*     MT-NOTE: obj_state_destroy() is MT safe 
*
*  SEE ALSO
*     sge_object/obj_state_global_init
*     sge_object/obj_state_init
*     sge_object/obj_mt_init
*******************************************************************************/
static void obj_state_destroy(void* st) 
{
   int i;
   obj_state_t *state = (obj_state_t*) st;

   for (i = 0; i < SGE_TYPE_ALL; i++) {
      lFreeList(&(state->lists[i])); 
   }
      
   sge_free(&state);
}

/****** sge_object/obj_mt_init() ***********************************************
*  NAME
*     obj_mt_init() -- creats the pthread key
*
*  SYNOPSIS
*     void obj_mt_init(void) 
*
*  FUNCTION
*     Creates the pthread key. Needs to be called when the daemon, clients
*     starts up
*
*  NOTES
*     MT-NOTE: obj_mt_init() is not MT safe 
*
*
*  SEE ALSO
*     sge_object/obj_state_global_init
*     sge_object/obj_state_init
*     sge_object/obj_state_destroy
*******************************************************************************/
void obj_mt_init(void) 
{
   pthread_once(&obj_once, obj_thread_local_once_init);
}

void obj_init(bool is_global) 
{
   obj_state_t *state = NULL;
   int ret = 0;
   bool init = false;
   DENTER(TOP_LAYER, "obj_init");

   if((state = pthread_getspecific(obj_state_key)) == NULL) { 
      state = (obj_state_t*) malloc(sizeof(obj_state_t));
      memset(state, 0 , sizeof(obj_state_t));
      init = true;

      ret = pthread_setspecific(obj_state_key, (void*)state);
      if (ret != 0) { 
         abort();  /* find a better way for this, use a return code */
      }  
   }    

   if (init || (state->global != is_global)) {
      if (is_global) {
         if (!init) {
            int i;
            for (i = 0; i < SGE_TYPE_ALL; i++) {
               lFreeList(&(state->lists[i])); 
            }
         }
         obj_state_global_init(state);
      } else {
         obj_state_init(state);
      }
   }
   DEXIT;
} 

/****** sgeobj/object/object_has_type() ***************************************
*  NAME
*     object_has_type() -- has an object a certain type?
*
*  SYNOPSIS
*     bool 
*     object_has_type(const lListElem *object, const lDescr *descr) 
*
*  FUNCTION
*     Checks if an object has a certain type.
*     The check is done by looking up the primary key field in descr and
*     checking, if this key field is contained in the given object.
*
*  INPUTS
*     const lListElem *object - object to check
*     const lDescr *descr     - type to check against
*
*  RESULT
*     bool - true, if the object has the given type, else false
*
*  NOTES
*     As looking up the primary key of an object is only implemented
*     for selected types, this function also will work only for these
*     object types.
*
*  SEE ALSO
*     sgeobj/object/object_get_primary_key()
*******************************************************************************/
bool 
object_has_type(const lListElem *object, const lDescr *descr) 
{
   bool ret = false;
 
   /*
    * we assume that "object" is of the given type when the 
    * primary key is contained in the element
    *
    * --> make sure that your object is handled in object_get_primary_key() 
    */
   if (object != NULL && descr != NULL &&
       _lGetPosInDescr(object->descr, object_get_primary_key(descr)) != -1) {
      ret = true;
   }

   return ret;
} 

/****** sgeobj/object/object_get_type() ***************************************
*  NAME
*     object_get_type() -- return type (descriptor) for object
*
*  SYNOPSIS
*     const lDescr * object_get_type(const lListElem *object) 
*
*  FUNCTION
*     Returns the cull type (descriptor) for a certain object.
*     This descriptor can be different from the objects descriptor,
*     as the objects descriptor can come from an object created from
*     communication, can be a partial descriptor etc.
*
*  INPUTS
*     const lListElem *object - the object to analyze
*
*  RESULT
*     const lDescr * - the object type / descriptor
*
*******************************************************************************/
const lDescr * 
object_get_type(const lListElem *object)
{
   const lDescr *ret = NULL;

   if (object_has_type(object, AH_Type)) {
      ret = AH_Type;
   } else if (object_has_type(object, CAL_Type)) {
      ret = CAL_Type;
   } else if (object_has_type(object, CK_Type)) {
      ret = CK_Type;
   } else if (object_has_type(object, EH_Type)) {
      ret = EH_Type;
   } else if (object_has_type(object, JAT_Type)) {
      ret = JAT_Type;
   } else if (object_has_type(object, JB_Type)) {
      ret = JB_Type;
   } else if (object_has_type(object, PE_Type)) {
      ret = PE_Type;
   } else if (object_has_type(object, PET_Type)) {
      ret = PET_Type;
   } else if (object_has_type(object, QU_Type)) {
      ret = QU_Type;
   } else if (object_has_type(object, QR_Type)) {
      ret = QR_Type;
   } else if (object_has_type(object, RN_Type)) {
      ret = RN_Type;
   } else if (object_has_type(object, SH_Type)) {
      ret = SH_Type;
   } else if (object_has_type(object, VA_Type)) {
      ret = VA_Type;
   }

   return ret;
}

/****** sgeobj/object/object_get_primary_key() ********************************
*  NAME
*     object_get_primary_key() -- get primary key for object type
*
*  SYNOPSIS
*     int object_get_primary_key(const lDescr *descr) 
*
*  FUNCTION
*     Returns the primary key field for a given object type.
*
*  INPUTS
*     const lDescr *descr - the object type (descriptor)
*
*  RESULT
*     int - name (nm) of the primary key field or 
*           NoName (-1) on error.
*
*  NOTES
*     Only implemented for selected object types.
*     It would be better to have the necessary data in the object 
*     description, e.g. via a field property CULL_PRIMARY_KEY.
*
*     Function interface breaks style guide - either we would have to 
*     pass an object, or call it descr_get_primary_key.
*******************************************************************************/
int 
object_get_primary_key(const lDescr *descr)
{
   int ret = NoName;

   if (descr != NULL) {
      int i;

      for (i = 0; descr[i].nm != NoName; i++) {
         if (descr[i].mt & CULL_PRIMARY_KEY) {
            ret = descr[i].nm;
            break;
         }
      }
   }

#if 0
      ret = MR_user; /* JG: TODO: is this really the primary key? */
      ret = PA_origin; /* JG: TODO: this is most probably no primary key! */
#endif

   return ret;
}
 
/****** sgeobj/object/object_get_name_prefix() ********************************
*  NAME
*     object_get_name_prefix() -- get prefix of cull attribute name
*
*  SYNOPSIS
*     const char *
*     object_get_name_prefix(const lDescr *descr, dstring *buffer) 
*
*  FUNCTION
*     Returns the prefix that is used in attribute names characterizing 
*     the object type (e.g. "QU_" for the QU_Type).
*
*  INPUTS
*     const lDescr *descr - object type to use
*     dstring *buffer     - buffer that is used to return the result
*
*  RESULT
*     const char * - the prefix or
*                    NULL, if an error occured
*
*  EXAMPLE
*     object_get_name_prefix(QU_Type, buffer) = "QU_"
*     object_get_name_prefix(JB_Type, buffer) = "JB_"
*
*  NOTES
*     The function relies on object_get_primary_key. This function only
*     is implemented for some object types.
*     For types not handled in object_get_primary_key, NULL will be 
*     returned.
*
*  SEE ALSO
*     sgeobj/object/object_get_primary_key()
*******************************************************************************/
const char * 
object_get_name_prefix(const lDescr *descr, dstring *buffer)
{
   int nm;

   if (descr == NULL || buffer == NULL) {
      return NULL;
   }

   nm = descr[0].nm;

   if (nm != NoName) {
      const char *name = lNm2Str(nm);

      if (name != NULL) {
         const char *underscore = strchr(name, '_');

         if (underscore != NULL) {
            sge_dstring_sprintf(buffer, "%.*s",
                                (int) (underscore - name + 1), name);
            return sge_dstring_get_string(buffer);
         }
      }
   }

   return NULL;
}

/****** sge_object/object_get_name() *******************************************
*  NAME
*     object_get_name() -- get the object name by object descriptor
*
*  SYNOPSIS
*     const char *
*     object_get_name(const lDescr *descr) 
*
*  FUNCTION
*     Returns the object name for a given descriptor.
*     If the descriptor doesn't match a descriptor in object_base,
*     return "unknown".
*
*  INPUTS
*     const lDescr *descr - descriptor.
*
*  RESULT
*     const char * - the object name
*
*  EXAMPLE
*     object_get_name(JOB_Type) returns "JOB"
*
*  NOTES
*     MT-NOTE: object_get_name() is MT safe 
*******************************************************************************/
const char *
object_get_name(const lDescr *descr)
{
   const char *name = "unknown";

   if (descr != NULL) {
      int i;

      for (i = SGE_TYPE_ADMINHOST; i < SGE_TYPE_ALL; i++) {
         if (object_base[i].descr == descr) {
            name = object_base[i].type_name;
            break;
         }
      }
   }

   return name;
}

/****** sgeobj/object/object_append_field_to_dstring() ************************
*  NAME
*     object_append_field_to_dstring() -- object field to string
*
*  SYNOPSIS
*     const char *
*     object_append_field_to_dstring(const lListElem *object, 
*                                    lList **answer_list, 
*                                    dstring *buffer, const int nm,
*                                    char string_quotes) 
*
*  FUNCTION
*     Returns a string representation of a given object attribute.
*     If errors arrise they are returned in the given answer_list.
*     Data will be created in the given dynamic string buffer.
*     For some fields a special handling is implemented, e.g. mapping
*     bitfields to string lists.
*
*  INPUTS
*     const lListElem *object - object to use
*     lList **answer_list     - used to return error messages
*     dstring *buffer         - buffer used to format the result
*     const int nm            - attribute to output
*     char string_quotes      - character to be used for string quoting 
*                               '\0' means no quoting
*
*  RESULT
*     const char * - string representation of the attribute value 
*                    (pointer to the string in the dynamic string 
*                    buffer, or NULL if an error occured.
*
*  NOTES
*     For sublists, subobjects and references NULL is returned.
*
*  BUGS
*     For the handled special cases, the dstring is cleared,
*     the default handling appends to the dstring buffer.
*
*  SEE ALSO
*     sgeobj/object/--GDI-object-Handling
*     sgeobj/object/object_parse_field_from_string()
*******************************************************************************/
const char * 
object_append_field_to_dstring(const lListElem *object, lList **answer_list, 
                               dstring *buffer, const int nm, 
                               const char string_quotes)
{
   const char *ret = NULL;
   dstring string = DSTRING_INIT;
   bool quote_special_case = false;

   DENTER(OBJECT_LAYER, "object_append_field_to_dstring");

   SGE_CHECK_POINTER_NULL(object, answer_list);

   /* handle special cases 
    * these special cases are for instance bitfields that shall be 
    * output as readable strings.
    * Usually such special cases are evidence of bad data base design.
    * Example: QU_qtype is a bitfield of types (BATCH, PARALLEL, ...)
    *          Instead, multiple boolean fields should be created:
    *          QU_batch, QU_parallel, ...
    */

   switch (nm) {
      case CE_valtype:
         ret = map_type2str(lGetUlong(object, nm));
         quote_special_case = true;
         break;
      case CE_relop:
         ret = map_op2str(lGetUlong(object, nm));
         quote_special_case = true;
         break;
      case CE_requestable:
         ret = map_req2str(lGetUlong(object, nm));
         break;
      case CE_consumable:
         ret = map_consumable2str(lGetUlong(object, nm));
         quote_special_case = true;
         break;
      case QU_qtype:
         qinstance_print_qtype_to_dstring(object, &string, false);
         ret = sge_dstring_get_string(&string); 
         quote_special_case = true;
         break;
      case US_type:
         ret = userset_get_type_string(object, answer_list, &string);
         quote_special_case = true;
         break;

      case ASTRLIST_value:
         ret = str_list_append_to_dstring(lGetList(object, nm), &string, ' ');
         break;
      case AUSRLIST_value:
         ret = userset_list_append_to_dstring(lGetList(object, nm), &string);
         break;
      case APRJLIST_value:
         ret = prj_list_append_to_dstring(lGetList(object, nm), &string);
         break;
      case ACELIST_value:
         ret = centry_list_append_to_dstring(lGetList(object, nm), &string);
         break;
      case ASOLIST_value:
         ret = so_list_append_to_dstring(lGetList(object, nm), &string);
         break;
      case AQTLIST_value:
         ret = qtype_append_to_dstring(lGetUlong(object, nm), &string);
         break;
   }

   /* we had a special case - append to result dstring */
   if (ret != NULL) {
      if (quote_special_case && string_quotes != '\0') {
         sge_dstring_append_char(buffer, string_quotes);
         sge_dstring_append(buffer, ret);
         ret = sge_dstring_append_char(buffer, string_quotes);
      } else { 
         ret = sge_dstring_append(buffer, ret);
      }
   } else {
      ret = object_append_raw_field_to_dstring(object, answer_list, buffer,
                                               nm, string_quotes);
   }
   sge_dstring_free(&string);

   DRETURN(ret);
}

/****** sgeobj/object/object_append_raw_field_to_dstring() *********************
*  NAME
*     object_append_raw_field_to_dstring() -- object field to string
*
*  SYNOPSIS
*     const char *
*     object_append_raw_field_to_dstring(const lListElem *object, 
*                                        lList **answer_list, 
*                                        dstring *buffer, const int nm,
*                                        char string_quotes) 
*
*  FUNCTION
*     Returns a string representation of a given object attribute.
*     If errors arrise they are returned in the given answer_list.
*     Data will be created in the given dynamic string buffer.
*     For some fields a special handling is implemented, e.g. mapping
*     bitfields to string lists.
*
*  INPUTS
*     const lListElem *object - object to use
*     lList **answer_list     - used to return error messages
*     dstring *buffer         - buffer used to format the result
*     const int nm            - attribute to output
*     char string_quotes      - character to be used for string quoting 
*                               '\0' means no quoting
*
*  RESULT
*     const char * - string representation of the attribute value 
*                    (pointer to the string in the dynamic string 
*                    buffer, or NULL if an error occured.
*
*  NOTES
*     For sublists, subobjects and references NULL is returned.
*
*  BUGS
*     For the handled special cases, the dstring is cleared,
*     the default handling appends to the dstring buffer.
*
*  SEE ALSO
*     sgeobj/object/--GDI-object-Handling
*     sgeobj/object/object_parse_field_from_string()
*******************************************************************************/
static const char *
object_append_raw_field_to_dstring(const lListElem *object, lList **answer_list,
                                   dstring *buffer, const int nm,
                                   char string_quotes)
{
   const char *str;
   const char *result = NULL;
   int pos;

   DENTER(OBJECT_LAYER, "object_append_raw_field_to_dstring");

   pos = lGetPosViaElem(object, nm, SGE_NO_ABORT);

   if (pos < 0) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_NMNOTINELEMENT_S, 
                              lNm2Str(nm));
   } else {
      const lDescr *descr;
      int type;

      descr = lGetElemDescr(object);
      type = lGetPosType(descr, pos);

      /* no special case: read and copy data from object */
      switch (type) {
         case lFloatT:
            result = sge_dstring_sprintf_append(buffer, "%f", lGetPosFloat(object, pos));
            break;
         case lDoubleT:
            result = sge_dstring_sprintf_append(buffer, "%lf", lGetPosDouble(object, pos));
            break;
         case lUlongT:
            result = sge_dstring_sprintf_append(buffer, sge_U32CFormat, lGetPosUlong(object, pos));
            break;
         case lLongT:
            result = sge_dstring_sprintf_append(buffer, "%ld", lGetPosLong(object, pos));
            break;
         case lCharT:
            result = sge_dstring_sprintf_append(buffer, "%c", lGetPosChar(object, pos));
            break;
         case lBoolT:
            result = sge_dstring_append(buffer, lGetPosBool(object, pos) ? TRUE_STR : FALSE_STR);
            break;
         case lIntT:
            result = sge_dstring_sprintf_append(buffer, "%d", lGetPosInt(object, pos));
            break;
         case lStringT:
            str = lGetPosString(object, pos);
            if (string_quotes != '\0') {
               sge_dstring_append_char(buffer, string_quotes);
               sge_dstring_append(buffer, str != NULL ? str : NONE_STR);
               result = sge_dstring_append_char(buffer, string_quotes);
            } else {
               result = sge_dstring_append(buffer, str != NULL ? str : NONE_STR);
            }
            break;
         case lHostT:
            str = lGetPosHost(object, pos);
            if (string_quotes != '\0') {
               sge_dstring_append_char(buffer, string_quotes);
               sge_dstring_append(buffer, str != NULL ? str : NONE_STR);
               result = sge_dstring_append_char(buffer, string_quotes);
            } else {
               result = sge_dstring_append(buffer, str != NULL ? str : NONE_STR);
            }
            break;
         case lListT:
         case lObjectT:
         case lRefT:
            /* what do to here? */
            break;
         default:
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_INVALIDCULLDATATYPE_D, type);
            break;
      }
   }

   DRETURN(result);
}

/****** sgeobj/object/object_parse_field_from_string() ************************
*  NAME
*     object_parse_field_from_string() -- set object attr. from str
*
*  SYNOPSIS
*     bool 
*     object_parse_field_from_string(lListElem *object, 
*                                    lList **answer_list, 
*                                    const int nm, const char *value) 
*
*  FUNCTION
*     Sets a new value for a certain object attribute.
*     The new value is passed as parameter in string format.
*     For some fields a special handling is implemented, e.g. mapping
*     string lists to bitfields.
*
*  INPUTS
*     lListElem *object   - the object to change
*     lList **answer_list - used to return error messages
*     const int nm        - the attribute to change
*     const char *value   - the new value
*
*  RESULT
*     bool - true on success,
*            false, on error, error description in answer_list 
*
*  NOTES
*     Sublists, subobjects and references cannot be set with this 
*     function.
*
*  SEE ALSO
*     sgeobj/object/--GDI-object-Handling
*     sgeobj/object/object_append_field_to_dstring()
******************************************************************************/
bool 
object_parse_field_from_string(lListElem *object, lList **answer_list, 
                               const int nm, const char *value)
{
   bool ret = true;

   DENTER(OBJECT_LAYER, "object_parse_field_from_string");

   SGE_CHECK_POINTER_FALSE(object, answer_list);

   /* handle special cases */
   switch (nm) {
      case CE_valtype:
         {
            /* JG: TODO: we should better have a function map_str2type 
             * and isn't there some sort of framework for mapping 
             * strings to ints and vice versa? See QU_qtype implementation.
             */
            u_long32 type = 0;
            int i;

            for (i=TYPE_FIRST; !type && i<=TYPE_CE_LAST; i++) {
               if (strcasecmp(value, map_type2str(i)) == 0) {
                  type = i;
               }
            }

            if (type == 0) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                       ANSWER_QUALITY_ERROR,
                                       MSG_SGETEXT_UNKNOWN_ATTR_TYPE_S,
                                       value);
               ret = false;
            } else {
               lSetUlong(object, nm, type);
            }
         }
         break;
      case CE_relop:
         {
            u_long32 op = 0;
            int i;

            for (i=TYPE_FIRST; !op && i<=TYPE_DOUBLE; i++) {
               if (!strcasecmp(value, map_op2str(i)))  
                  op = i;
            }

            if (op == 0) {
               /* error output necessary? Should be created by parsing function */
               ret = false;
            } else {
               lSetUlong(object, nm, op);
            }
         }
         break;
      case CE_requestable:
         {
            u_long32 requestable = REQU_NO;

            if (strcasecmp(value, "y") == 0 || strcasecmp(value, "yes") == 0) {
               requestable = REQU_YES;
            } else if (strcasecmp(value, "n") == 0 || strcasecmp(value, "no") == 0) {
               requestable = REQU_NO;
            } else if (strcasecmp(value, "f") == 0 || strcasecmp(value, "forced") == 0) {
               requestable = REQU_FORCED;
            } else {
               ret = false;
            }
            if (ret) {
               lSetUlong(object, nm, requestable);
            }
         }
         break;
      case CE_consumable:
         {
            u_long32 cond = CONSUMABLE_NO;
            if (strcasecmp(value, "y") == 0 || strcasecmp(value, "yes") == 0) {
               cond = CONSUMABLE_YES;
            } else if (strcasecmp(value, "n") == 0 || strcasecmp(value, "no") == 0) {
               cond = CONSUMABLE_NO;
            } else if (strcasecmp(value, "j") == 0 || strcasecmp(value, "job") == 0) {
               cond = CONSUMABLE_JOB;
            } else {
               ret = false;
            }
            lSetUlong(object, nm, cond);
         }
         break;
      case QU_qtype:
         ret = qinstance_parse_qtype_from_string(object, answer_list, value);
         break;
      case US_type:
         ret = userset_set_type_string(object, answer_list, value);
         break;
      case AMEM_value:
         ret = object_parse_mem_from_string(object, answer_list, nm, value);
         break;
      case ATIME_value:
         ret = object_parse_time_from_string(object, answer_list, nm, value);
         break;
      case AINTER_value:
         ret = object_parse_inter_from_string(object, answer_list, nm, value);
         break;
      case ASTRLIST_value:
         ret = object_parse_list_from_string(object, answer_list, nm, value,
                                             ST_Type, ST_name);
         break;
      case AUSRLIST_value:
         ret = object_parse_list_from_string(object, answer_list, nm, value,
                                             US_Type, US_name);
         break;
      case APRJLIST_value:
         ret = object_parse_list_from_string(object, answer_list, nm, value,
                                             PR_Type, PR_name);
         break;
      case ACELIST_value:
         ret = object_parse_celist_from_string(object, answer_list, nm, value);
         break;
      case ASOLIST_value:
         ret = object_parse_solist_from_string(object, answer_list, nm, value);
         break;
      case AQTLIST_value:
         ret = object_parse_qtlist_from_string(object, answer_list, nm, value);
         break;
      default:
         ret = object_parse_raw_field_from_string(object, answer_list, nm, 
                                                  value);
         break;
   }

   DRETURN(ret);
}

/****** sgeobj/object/object_parse_raw_field_from_string() ************************
*  NAME
*     object_parse_raw_field_from_string() -- set object attr. from str
*
*  SYNOPSIS
*     bool 
*     object_parse_raw_field_from_string(lListElem *object, 
*                                        lList **answer_list, 
*                                        const int nm, const char *value) 
*
*  FUNCTION
*     Sets a new value for a certain object attribute.
*     The new value is passed as parameter in string format.
*
*  INPUTS
*     lListElem *object   - the object to change
*     lList **answer_list - used to return error messages
*     const int nm        - the attribute to change
*     const char *value   - the new value
*
*  RESULT
*     bool - true on success,
*            false, on error, error description in answer_list 
*
*  NOTES
*     Sublists, subobjects and references cannot be set with this 
*     function.
*
*  SEE ALSO
*     sgeobj/object/--GDI-object-Handling
*     sgeobj/object/object_append_field_to_dstring()
******************************************************************************/
static bool 
object_parse_raw_field_from_string(lListElem *object, lList **answer_list, 
                                   const int nm, const char *value)
{
   bool ret = true;
   int pos;

   DENTER(OBJECT_LAYER, "object_parse_raw_field_from_string");

   pos = lGetPosViaElem(object, nm, SGE_NO_ABORT);
   if (pos < 0) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, MSG_NMNOTINELEMENT_S, 
                              lNm2Str(nm));
      ret = false;
   } else {
      const lDescr *descr;
      int type;

      descr = lGetElemDescr(object);
      type = lGetPosType(descr, pos);

      /* read data */
      switch (type) {
         case lFloatT:
            ret = object_parse_float_from_string(object, answer_list, nm, value);
            break;
         case lDoubleT:
            ret = object_parse_double_from_string(object, answer_list, nm, value);
            break;
         case lUlongT:
            ret = object_parse_ulong32_from_string(object, answer_list, nm, value);
            break;
         case lLongT:
            ret = object_parse_long_from_string(object, answer_list, nm, value);
            break;
         case lCharT:
            ret = object_parse_char_from_string(object, answer_list, nm, value);
            break;
         case lBoolT:
            ret = object_parse_bool_from_string(object, answer_list, nm, value);
            break;
         case lIntT:
            ret = object_parse_int_from_string(object, answer_list, nm, value);
            break;
         case lStringT:
            lSetPosString(object, pos, value);
            break;
         case lHostT:
            lSetPosHost(object, pos, value);
            break;
         case lListT:
         case lObjectT:
         case lRefT:
            /* what do to here? */
            break;
         default:
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_INVALIDCULLDATATYPE_D, type);
            break;
      }
   }

   DRETURN(ret);
}

/****** sgeobj/object/object_delete_range_id() ********************************
*  NAME
*     object_delete_range_id() -- del certain id from an range_list
*
*  SYNOPSIS
*     void 
*     object_delete_range_id(lListElem *object, lList **answer_list, 
*                            const int rnm, const u_long32 id) 
*
*  FUNCTION
*     Deletes a certain id from an objects sublist that is a range list.
*
*  INPUTS
*     lListElem *object   - the object to handle
*     lList **answer_list - error messages will be put here
*     const int rnm       - attribute containing the range list
*     const u_long32 id   - id to delete
*
*******************************************************************************/
void 
object_delete_range_id(lListElem *object, lList **answer_list, 
                       const int rnm, const u_long32 id)
{
   lList *range_list = NULL;

   lXchgList(object, rnm, &range_list);
   range_list_remove_id(&range_list, answer_list, id);
   range_list_compress(range_list);
   lXchgList(object, rnm, &range_list);
}

/****** sgeobj/object/object_set_range_id() **********************************
*  NAME
*     object_set_range_id() -- store the initial range ids in "job"
*
*  SYNOPSIS
*     int object_set_range_id(lListElem *job, u_long32 start, 
*                                 u_long32 end, u_long32 step) 
*
*  FUNCTION
*     The function stores the initial range id values ("start", "end" 
*     and "step") in the range list of an object. It should only be used 
*     in functions initializing objects range lists.
*
*  INPUTS
*     lListElem *object - object to handle
*     const int rnm     - attribute containing the range list
*     u_long32 start    - first id 
*     u_long32 end      - last id 
*     u_long32 step     - step size 
*
*  RESULT
*     int - 0 -> OK
*           1 -> no memory
*
*  NOTES
*     MT-NOTE: object_set_range_id() is MT safe
******************************************************************************/
int 
object_set_range_id(lListElem *object, int rnm, u_long32 start, u_long32 end,
                    u_long32 step)
{
   lListElem *range_elem;  /* RN_Type */
   int ret = 0;
 
   range_elem = lFirst(lGetList(object, rnm));
   if (range_elem == NULL) {
      lList *range_list;
 
      range_elem = lCreateElem(RN_Type);
      range_list = lCreateList("task_id_range", RN_Type);
      if (range_elem == NULL || range_list == NULL) {
         lFreeElem(&range_elem);
         lFreeList(&range_list);

         /* No memory */
         ret = 1;
      } else {
         lAppendElem(range_list, range_elem);
         lSetList(object, rnm, range_list);
      }
   }
   if (range_elem != NULL) {
      lSetUlong(range_elem, RN_min, start);
      lSetUlong(range_elem, RN_max, end);
      lSetUlong(range_elem, RN_step, step);
   }
   return ret;
}          


/****** sgeobj/object/object_type_get_master_list() **************************
*  NAME
*     object_type_get_master_list() -- get master list for object type
*
*  SYNOPSIS
*     lList** object_type_get_master_list(const sge_object_type type) 
*
*  FUNCTION
*     Returns a pointer to the master list holding objects of the 
*     given type.
*
*  INPUTS
*     const sge_object_type type - the object type 
*
*  RESULT
*     lList** - the corresponding master list, or NULL, if the object 
*               type has no associated master list
*
*  EXAMPLE
*     object_type_get_master_list(SGE_TYPE_JOB) will return a pointer 
*     to the Master_Job_List.
*
*     object_type_get_master_list(SGE_TYPE_SHUTDOWN) will return NULL,
*     as this object type has no associated master list.
*
*  NOTES
*
*  SEE ALSO
*     sgeobj/object/object_type_get_master_list()
*     sgeobj/object/object_type_get_name()
*     sgeobj/object/object_type_get_descr()
*     sgeobj/object/object_type_get_key_nm()
*******************************************************************************/
lList **object_type_get_master_list(const sge_object_type type)
{
   lList **ret = NULL;

   DENTER(OBJECT_LAYER, "object_type_get_master_list");

   if (type >= 0 && type < SGE_TYPE_ALL) {
      GET_SPECIFIC(obj_state_t, obj_state, obj_state_global_init, obj_state_key, "object_type_get_master_list");
      
      if (obj_state->object_base[type].list != NULL) {
         ret = obj_state->object_base[type].list;
      } else {
         ERROR((SGE_EVENT, MSG_OBJECT_NO_LIST_TO_MOD_TYPE_SI, SGE_FUNC, type));
      }
   } else {
      ERROR((SGE_EVENT, MSG_OBJECT_INVALID_OBJECT_TYPE_SI, SGE_FUNC, type));
   }
   
   DRETURN(ret);
}

/****** sgeobj/object/sge_master_list() **************************
*  NAME
*     sge_master_list() -- get master list for object type
*
*  SYNOPSIS
*     lList** sge_master_list(const object_description *object_base,
*                             const sge_object_type type) 
*
*  FUNCTION
*     Returns a pointer to the master list holding objects of the 
*     given type.
*
*  INPUTS
*     const sge_object_type type - the object type 
*
*  RESULT
*     lList** - the corresponding master list, or NULL, if the object 
*               type has no associated master list
*
*  EXAMPLE
*     object_type_get_master_list(SGE_TYPE_JOB) will return a pointer 
*     to the Master_Job_List.
*
*     object_type_get_master_list(SGE_TYPE_SHUTDOWN) will return NULL,
*     as this object type has no associated master list.
*
*  NOTES
*
*  SEE ALSO
*     sgeobj/object/object_type_get_master_list()
*     sgeobj/object/object_type_get_name()
*     sgeobj/object/object_type_get_descr()
*     sgeobj/object/object_type_get_key_nm()
*******************************************************************************/
lList **sge_master_list(const object_description *object_base, const sge_object_type type)
{
   lList **ret = NULL;

   DENTER(OBJECT_LAYER, "sge_master_list");

   if (type >= 0 && type < SGE_TYPE_ALL) {
      
      if (object_base[type].list != NULL) {
         ret = object_base[type].list;
      } else {
         ERROR((SGE_EVENT, MSG_OBJECT_NO_LIST_TO_MOD_TYPE_SI, SGE_FUNC, type));
      }
   } else {
      ERROR((SGE_EVENT, MSG_OBJECT_INVALID_OBJECT_TYPE_SI, SGE_FUNC, type));
   }
   
   DRETURN(ret);
}


/****** sge_object/object_type_get_object_description() ************************
*  NAME
*     object_type_get_object_description() -- gets the master list table
*
*  SYNOPSIS
*     object_description* object_type_get_object_description() 
*
*  FUNCTION
*     ??? 
*
*  RESULT
*     object_description* - the table with all master lists and its information
*
*  NOTES
*     MT-NOTE: object_type_get_object_description() is MT safe 
*
*  SEE ALSO
*     sgeobj/object/object_type_get_master_list()
*     sgeobj/object/object_type_get_name()
*     sgeobj/object/object_type_get_descr()
*     sgeobj/object/object_type_get_key_nm()
*******************************************************************************/
object_description *object_type_get_object_description(void)
{
   object_description *ret = NULL;

   DENTER(OBJECT_LAYER, "object_type_get_object_description");
   {
      GET_SPECIFIC(obj_state_t, obj_state, obj_state_global_init, obj_state_key, 
                   "object_type_get_object_description");

      ret = obj_state->object_base;
   } 
   DRETURN(ret);
}

/****** sge_object/object_type_get_global_object_description() *****************
*  NAME
*     object_type_get_global_object_description() -- grands access to the global 
*                                                    master lists
*
*  SYNOPSIS
*     object_description* object_type_get_global_object_description(void) 
*
*  RESULT
*     object_description* - the global object description / master lists
*
*  NOTES
*     MT-NOTE: object_type_get_global_object_description() is MT safe 
*
*  SEE ALSO
*     sge_object/object_type_get_global_object_description()
*
*******************************************************************************/
object_description *object_type_get_global_object_description(void)
{
   object_description *ret = NULL;

   DENTER(OBJECT_LAYER, "object_type_get_object_description");
  
   ret = object_base;
 
   DRETURN(ret);
}


bool object_type_commit_master_list(const sge_object_type type, lList **answer_list) 
{
   bool ret = true;
   
   DENTER(OBJECT_LAYER, "object_type_set_master_list");
   
   if (type >= 0 && type < SGE_TYPE_ALL) {
      
      if (object_base[type].commitMasterList) {
         ret = object_base[type].commitMasterList(answer_list);
      }
   } else {
         ERROR((SGE_EVENT, MSG_OBJECT_INVALID_OBJECT_TYPE_SI, SGE_FUNC, type));
         ret = false;
   }

   DRETURN(ret);
}


/****** sgeobj/object/object_type_free_master_list() ***************************
*  NAME
*     object_type_free_master_list() -- free the master list 
*
*  SYNOPSIS
*     bool 
*     object_type_free_master_list(const sge_object_type type) 
*
*  FUNCTION
*     Frees the masterlist for a certain type of objects.
*
*  INPUTS
*     const sge_object_type type - the object type
*
*  RESULT
*     bool - true, if the list existed and could be freed, else false
*
*  NOTES
*
*  SEE ALSO
*     sgeobj/object/--object-Typedefs
*******************************************************************************/
bool object_type_free_master_list(const sge_object_type type)
{
   bool ret = false;

   DENTER(OBJECT_LAYER, "object_type_free_master_list");

   if (type >= 0 && type < SGE_TYPE_ALL) {
      GET_SPECIFIC(obj_state_t, obj_state, obj_state_global_init, obj_state_key, SGE_FUNC);
      
      if (obj_state->object_base[type].list) {
          lFreeList(obj_state->object_base[type].list);
          ret = true;
      }
   } else {
      ERROR((SGE_EVENT, MSG_OBJECT_INVALID_OBJECT_TYPE_SI, SGE_FUNC, type));
   }

   DRETURN(ret);
}

/****** sgeobj/object/object_type_get_name() *********************************
*  NAME
*     object_type_get_name() -- get a printable name for event type
*
*  SYNOPSIS
*     const char* object_type_get_name(const sge_object_type type) 
*
*  FUNCTION
*     Returns a printable name for an event type.
*
*  INPUTS
*     const sge_object_type type - the event type
*
*  RESULT
*     const char* - string describing the type
*
*  EXAMPLE
*     object_type_get_name(SGE_TYPE_JOB) will return "JOB"
*
*  SEE ALSO
*     sgeobj/object/object_type_get_master_list()
*     sgeobj/object/object_type_get_descr()
*     sgeobj/object/object_type_get_key_nm()
*******************************************************************************/
const char *object_type_get_name(const sge_object_type type)
{
   const char *ret = "unknown";

   DENTER(OBJECT_LAYER, "object_type_get_name");

   if (type >= 0 && type < SGE_TYPE_ALL) {
      ret = object_base[type].type_name;
   } else if (type == SGE_TYPE_ALL) {
      ret = "default";
   } else {
      ERROR((SGE_EVENT, MSG_OBJECT_INVALID_OBJECT_TYPE_SI, SGE_FUNC, type));
   }
  
   DRETURN(ret);
}

/****** sgeobj/object/object_name_get_type() **********************************
*  NAME
*     object_name_get_type() -- Return type id of a certain object 
*
*  SYNOPSIS
*     sge_object_type object_name_get_type(const char *name) 
*
*  FUNCTION
*     returns the type id a an object given by "name" 
*     We allow to pass in names in the form <object_name>:<key>, e.g.
*     USERSET:deadlineusers.
*
*  INPUTS
*     const char *name - object name 
*
*  RESULT
*     sge_object_type - type of the object
*
*  NOTES
*     MT-NOTE: object_name_get_type() is MT safe 
*******************************************************************************/
sge_object_type object_name_get_type(const char *name)
{
   sge_object_type ret = SGE_TYPE_ALL;
   sge_object_type i;
   char *type_name;
   char *colon;

   DENTER(OBJECT_LAYER, "object_name_get_type");

   type_name = strdup(name);
   colon = strchr(type_name, ':');
   if (colon != NULL) {
      *colon = '\0';
   }

   for (i = SGE_TYPE_ADMINHOST; i < SGE_TYPE_ALL; i++) {
      if (strcasecmp(object_base[i].type_name, type_name) == 0) {
         ret = i;
         break;
      }
   }

   sge_free(&type_name);
   DRETURN(ret);
}

/****** sgeobj/object/object_type_get_descr() ********************************
*  NAME
*     object_type_get_descr() -- get the descriptor for an event type
*
*  SYNOPSIS
*     const lDescr* object_type_get_descr(const sge_object_type type) 
*
*  FUNCTION
*     Returns the CULL element descriptor for the object type 
*     associated with the given event.
*
*  INPUTS
*     const sge_object_type type - the event type
*
*  RESULT
*     const lDescr* - the descriptor, or NULL, if no descriptor is 
*                     associated with the type
*
*  EXAMPLE
*     object_type_get_descr(SGE_TYPE_JOB) will return the descriptor 
*        JB_Type,
*     object_type_get_descr(SGE_TYPE_SHUTDOWN) will return NULL
*
*  SEE ALSO
*     sgeobj/object/object_type_get_master_list()
*     sgeobj/object/object_type_get_name()
*     sgeobj/object/object_type_get_key_nm()
*******************************************************************************/
const lDescr *object_type_get_descr(const sge_object_type type)
{
   const lDescr *ret = NULL;

   DENTER(OBJECT_LAYER, "object_type_get_descr");

   if (type >= 0 && type < SGE_TYPE_ALL) {
      ret = object_base[type].descr;
   } else {
      ERROR((SGE_EVENT, MSG_OBJECT_INVALID_OBJECT_TYPE_SI, SGE_FUNC, type));
   }

   DRETURN(ret);
}

/****** sgeobj/object/object_type_get_key_nm() *******************************
*  NAME
*     object_type_get_key_nm() -- get the primary key attribute for type
*
*  SYNOPSIS
*     int object_type_get_key_nm(const sge_object_type type) 
*
*  FUNCTION
*     Returns the primary key attribute for the object type associated 
*     with the given event type.
*
*  INPUTS
*     const sge_object_type type - event type
*
*  RESULT
*     int - the key number (struct element nm of the descriptor), or
*           -1, if no object type is associated with the event type
*
*  EXAMPLE
*     object_type_get_key_nm(SGE_TYPE_JOB) will return JB_job_number
*     object_type_get_key_nm(SGE_TYPE_SHUTDOWN) will return -1
*
*  SEE ALSO
*     sgeobj/object/object_type_get_master_list()
*     sgeobj/object/object_type_get_name()
*     sgeobj/object/object_type_get_descr()
*******************************************************************************/
int object_type_get_key_nm(const sge_object_type type)
{
   int ret = NoName;

   DENTER(OBJECT_LAYER, "object_type_get_key_nm");

   if (type >= 0 && type < SGE_TYPE_ALL) {
      ret = object_base[type].key_nm;
   } else {
      ERROR((SGE_EVENT, MSG_OBJECT_INVALID_OBJECT_TYPE_SI, SGE_FUNC, type));
   }
   
   DRETURN(ret);
}

bool
object_parse_bool_from_string(lListElem *this_elem, lList **answer_list,
                              int name, const char *string)
{
   bool ret = true;

   DENTER(OBJECT_LAYER, "object_parse_bool_from_string");
   if (this_elem != NULL && string != NULL) {
      int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);

      if (!strcasecmp(string, "true") || !strcasecmp(string, "t") || 
          !strcmp(string, "1") ||
          !strcasecmp(string, "yes") || !strcasecmp(string, "y")) {
         lSetPosBool(this_elem, pos, true);
      } else if (!strcasecmp(string, "false") || !strcasecmp(string, "f") ||
                 !strcmp(string, "0") ||
                 !strcasecmp(string, "no") || !strcasecmp(string, "n")) {
         lSetPosBool(this_elem, pos, false);
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR,
                                 MSG_ERRORPARSINGVALUEFORNM_S, string);
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}

bool
object_parse_time_from_string(lListElem *this_elem, lList **answer_list,
                              int name, const char *string)
{
   bool ret = true;

   DENTER(OBJECT_LAYER, "object_parse_time_from_string");
   if (this_elem != NULL && string != NULL) {
      int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);

      if (parse_ulong_val(NULL, NULL, TYPE_TIM, string, NULL, 0)) {
         lSetPosString(this_elem, pos, string);
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR,
                                 MSG_ERRORPARSINGVALUEFORNM_S, string);
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                              ANSWER_QUALITY_ERROR,
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}

bool
object_parse_inter_from_string(lListElem *this_elem, lList **answer_list,
                               int name, const char *string)
{
   bool ret = true;

   DENTER(OBJECT_LAYER, "object_parse_inter_from_string");
   if (this_elem != NULL && string != NULL) {
      int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);

      if (parse_ulong_val(NULL, NULL, TYPE_TIM, string, NULL, 0)) {
         lSetPosString(this_elem, pos, string);
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR,
                                 MSG_ERRORPARSINGVALUEFORNM_S, string);
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                              ANSWER_QUALITY_ERROR,
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}

bool
object_parse_list_from_string(lListElem *this_elem, lList **answer_list,
                              int name, const char *string, 
                              const lDescr *descr, int nm)
{
   bool ret = true;

   DENTER(OBJECT_LAYER, "object_parse_strlist_from_string");
   if (this_elem != NULL && string != NULL) {
      lList *tmp_list = NULL;
      int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);

      lString2List(string, &tmp_list, descr, nm, "\t \v\r,");
      if (tmp_list != NULL) {
         lListElem *first_elem = lFirst(tmp_list);
         const char *first_string = lGetString(first_elem, nm);

         if (strcasecmp(NONE_STR, first_string)) {
            lSetPosList(this_elem, pos, tmp_list);
         } else {
            lFreeList(&tmp_list);
         }
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR,
                                 MSG_ERRORPARSINGVALUEFORNM_S, string);
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                              ANSWER_QUALITY_ERROR,
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}

bool
object_parse_celist_from_string(lListElem *this_elem, lList **answer_list,
                                int name, const char *string)
{
   static int rule[] = {CE_name, CE_stringval, 0};
   bool ret = true;

   DENTER(TOP_LAYER, "object_parse_celist_from_string");

   if (this_elem != NULL && string != NULL) {
      lList *tmp_list = NULL;
      int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);

      if (!cull_parse_definition_list((char *)string, &tmp_list, "", CE_Type, rule)) {
         lSetPosList(this_elem, pos, tmp_list);
      } else {
         lFreeList(&tmp_list);
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR,
                                 MSG_ERRORPARSINGVALUEFORNM_S, string);
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                              ANSWER_QUALITY_ERROR,
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}

bool
object_parse_solist_from_string(lListElem *this_elem, lList **answer_list,
                                int name, const char *string)
{
   bool ret = true;

   DENTER(OBJECT_LAYER, "object_parse_solist_from_string");
   if (this_elem != NULL && string != NULL) {
      lList               *tmp_list = NULL;
      lListElem           *tmp_elem = NULL;
      struct saved_vars_s *last = NULL;
      char                *slots_text = NULL;
      int                 pos = -1;
      
      pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);

      /*
       * The queue wise subordinate is defined like this:
       * subordinate_list  A.q=3,B.q=4
       * The slotwise subordinate is defined like this:
       * subordinate_list slots=4(A.q:1:lr, B.q:2:sr)
       * If the value of the "subordinate_list" key begins with
       * "slots=", it's slotwise subordinate, otherwise it's
       * queue wise subordinate.
       *
       * TODO: HP: This parser could be improved, now a queue wise subordinate
       * that begins with a queue called "slots" will be interpreted as
       * slotwise subordinate and therefore lead to an error.
       */
      slots_text = sge_strtok_r(string, "=", &last);
      if (slots_text != NULL && strncasecmp("slots", slots_text, 5) == 0) {
         /*
          * slot-wise suspend on subordinate
          */
         char *sub_queues_text = NULL;
         char *slots_sum_text = NULL;
         char *endptr = NULL;
         lUlong slots_sum_value = 0;

         /*
          * format of string: slots=8(queue1.q:3:sr, queue2.q:2:lr)
          */
         /* the "slots=" was already skipped in the first sge_strtok_r() */
         /* parse the "8" in our example above */
         slots_sum_text = sge_strtok_r(NULL, "(", &last);
         if (slots_sum_text != NULL) {
            slots_sum_value = strtol(slots_sum_text, &endptr, 10);
         } else {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
               ANSWER_QUALITY_ERROR, MSG_ERRORPARSINGVALUEFORNM_S, string);
            ret = false;
         }
         if (endptr != NULL && *endptr != '\0') {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
               ANSWER_QUALITY_ERROR, MSG_ERRORPARSINGVALUEFORNM_S, string);
            ret = false;
         }
         if (ret) {
            /* point the "sub_queues_text" to the start of the sub queue list */
            sub_queues_text = sge_strtok_r(NULL, ")", &last);
            /* split up the original string from the first subordinated queue on */
            lString2List(sub_queues_text, &tmp_list, SO_Type, SO_name, ",) \t");
            for_each(tmp_elem, tmp_list) {
               const char *queue_value = lGetString(tmp_elem, SO_name);
               char *queuename   = sge_strtok(queue_value, ":");
               char *value_str   = sge_strtok(NULL, ":");
               char *action_str  = sge_strtok(NULL, ":");

               sge_strip_blanks(queuename);
               sge_strip_blanks(value_str);
               sge_strip_blanks(action_str);

               if (queuename != NULL) {
                  lSetString(tmp_elem, SO_name, queuename);
               } else {
                  answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                     ANSWER_QUALITY_ERROR, MSG_ERRORPARSINGVALUEFORNM_S, string);
                  lFreeList(&tmp_list);
                  ret = false;
                  break;
               }

               if (slots_sum_value > 0) {
                  lSetUlong(tmp_elem, SO_slots_sum, slots_sum_value);
               } else {
                  answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                     ANSWER_QUALITY_ERROR, MSG_ERRORPARSINGVALUEFORNM_S, string);
                  lFreeList(&tmp_list);
                  ret = false;
                  break;
               }

               if (value_str != NULL) {
                  char     *endptr = NULL;
                  u_long32 value   = strtol(value_str, &endptr, 10);

                  if (endptr != NULL && *endptr == '\0') {
                     lSetUlong(tmp_elem, SO_seq_no, value); 
                  } else {
                     answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                        ANSWER_QUALITY_ERROR, MSG_ERRORPARSINGVALUEFORNM_S, string);
                     lFreeList(&tmp_list);
                     ret = false;
                     break;
                  }
               } else {
                  lSetUlong(tmp_elem, SO_seq_no, 0);
               }

               if (action_str != NULL) {
                  if (strcmp(action_str, "lr") == 0) {
                     lSetUlong(tmp_elem, SO_action, SO_ACTION_LR);
                  } else if (strcmp(action_str, "sr") == 0) {
                     lSetUlong(tmp_elem, SO_action, SO_ACTION_SR);
                  } else {
                     answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                        ANSWER_QUALITY_ERROR, MSG_ERRORPARSINGVALUEFORNM_S, string);
                     lFreeList(&tmp_list);
                     ret = false;
                     break;
                  }
               } else {
                  /* default is "sr" */
                  lSetUlong(tmp_elem, SO_action, SO_ACTION_SR);
               }
            } 
            lSetPosList(this_elem, pos, tmp_list);
         }
      } else {
         lString2List(string, &tmp_list, SO_Type, SO_name, ", \t");
         if (tmp_list != NULL) {
            if (!strcasecmp("NONE", lGetString(lFirst(tmp_list), SO_name))) {
               lFreeList(&tmp_list);
            } else {
               /* 
                * queue instance-wise suspend on subordinate
                */
               for_each(tmp_elem, tmp_list) {
                  const char *queue_value = lGetString(tmp_elem, SO_name);
                  const char *queuename = sge_strtok(queue_value, ":=");
                  const char *value_str = sge_strtok(NULL, ":=");

                  lSetString(tmp_elem, SO_name, queuename);
                  if (value_str != NULL) {
                     char *endptr = NULL;
                     u_long32 value = strtol(value_str, &endptr, 10);

                     if (endptr != NULL && *endptr == '\0') {
                        lSetUlong(tmp_elem, SO_threshold, value);
                     } else {
                        answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                          ANSWER_QUALITY_ERROR,
                                          MSG_ERRORPARSINGVALUEFORNM_S, string);
                        lFreeList(&tmp_list);
                        ret = false;
                        break;
                     }
                  } else {
                     /*
                      * No value is explicitely allowed
                      */
                  } 
               } 
               if (ret) {
                  lSetPosList(this_elem, pos, tmp_list);
               }
            }
         }
      }
      sge_free_saved_vars(last);
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                              ANSWER_QUALITY_ERROR,
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}

bool
object_parse_qtlist_from_string(lListElem *this_elem, lList **answer_list,
                                int name, const char *string)
{
   bool ret = true;

   DENTER(TOP_LAYER, "object_parse_qtlist_from_string");
   if (this_elem != NULL && string != NULL) {
      u_long32 value;
      int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);

      if (sge_parse_bitfield_str(string, queue_types, &value, "", 
                                 answer_list, true)) {
         lSetPosUlong(this_elem, pos, value);
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR,
                                 MSG_QTYPE_INCORRECTSTRING, string);
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                              ANSWER_QUALITY_ERROR,
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}


bool
object_parse_mem_from_string(lListElem *this_elem, lList **answer_list,
                             int name, const char *string)
{
   bool ret = true;

   DENTER(OBJECT_LAYER, "object_parse_mem_from_string");
   if (this_elem != NULL && string != NULL) {
      int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);

      if (parse_ulong_val(NULL, NULL, TYPE_MEM, string, NULL, 0)) {
         lSetPosString(this_elem, pos, string);
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR,
                                 MSG_ERRORPARSINGVALUEFORNM_S, string);
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                              ANSWER_QUALITY_ERROR,
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}

bool
object_parse_ulong32_from_string(lListElem *this_elem, lList **answer_list,
                                 int name, const char *string)
{
   bool ret = true;
   
   DENTER(OBJECT_LAYER, "object_parse_ulong32_from_string");
   if (this_elem != NULL && string != NULL) {
      int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);

      if (strlen(string) == 0) {
         /*
          * Empty string will be parsed as '0'
          */
         lSetPosUlong(this_elem, pos, (u_long32)0);
      } else {
         const double epsilon = 1.0E-12;
         char *end_ptr = NULL;
         double dbl_value;
         u_long32 ulng_value;

         dbl_value = strtod(string, &end_ptr);
         ulng_value = dbl_value;
         if (dbl_value < 0 || dbl_value - ulng_value > epsilon) {
            /*
             * value to big for u_long32 variable
             */
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                    ANSWER_QUALITY_ERROR,
                                    MSG_OBJECT_VALUENOTULONG_S, string);
            ret = false;
         } else if (end_ptr != NULL && *end_ptr == '\0') {
            lSetPosUlong(this_elem, pos, ulng_value);
         } else {
            /*
             * Not a number or
             * garbage after number
             */
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                    ANSWER_QUALITY_ERROR,
                                    MSG_ULONG_INCORRECTSTRING, string);
            ret = false;
         }
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}

bool
object_parse_int_from_string(lListElem *this_elem, lList **answer_list,
                             int name, const char *string)
{
   bool ret = true;

   DENTER(OBJECT_LAYER, "object_parse_int_from_string");
   if (this_elem != NULL && string != NULL) {
      int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);
      int value;

      if (sscanf(string, "%d", &value) == 1) {
         lSetPosInt(this_elem, pos, value);
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR,
                                 MSG_INT_INCORRECTSTRING, string);
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}

bool
object_parse_char_from_string(lListElem *this_elem, lList **answer_list,
                             int name, const char *string)
{
   bool ret = true;

   DENTER(OBJECT_LAYER, "object_parse_char_from_string");
   if (this_elem != NULL && string != NULL) {
      int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);
      char value;

      if (sscanf(string, "%c", &value) == 1) {
         lSetPosChar(this_elem, pos, value);
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_CHAR_INCORRECTSTRING, string);
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}

bool
object_parse_long_from_string(lListElem *this_elem, lList **answer_list,
                             int name, const char *string)
{
   bool ret = true;

   DENTER(OBJECT_LAYER, "object_parse_long_from_string");
   if (this_elem != NULL && string != NULL) {
      int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);
      long value;

      if (sscanf(string, "%ld", &value) == 1) {
         lSetPosLong(this_elem, pos, value);
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_LONG_INCORRECTSTRING, string);
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}

bool
object_parse_double_from_string(lListElem *this_elem, lList **answer_list,
                                int name, const char *string)
{
   bool ret = true;

   DENTER(OBJECT_LAYER, "object_parse_double_from_string");
   if (this_elem != NULL && string != NULL) {
      int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);
      double value;

      if (sscanf(string, "%lf", &value) == 1) {
         lSetPosDouble(this_elem, pos, value);
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR,
                                 MSG_DOUBLE_INCORRECTSTRING, string);
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}

bool
object_parse_float_from_string(lListElem *this_elem, lList **answer_list,
                               int name, const char *string)
{
   bool ret = true;

   DENTER(OBJECT_LAYER, "object_parse_float_from_string");
   if (this_elem != NULL && string != NULL) {
      int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);
      float value;

      if (sscanf(string, "%f", &value) == 1) {
         lSetPosFloat(this_elem, pos, value);
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR,
                                 MSG_FLOAT_INCORRECTSTRING, string);
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_ERRORPARSINGVALUEFORNM_S, "<null>");
      ret = false;
   }
   DRETURN(ret);
}

bool
object_set_any_type(lListElem *this_elem, int name, void *value)
{
   int cull_ret = 0;
   int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);
   int type = lGetPosType(lGetElemDescr(this_elem), pos);

   DENTER(OBJECT_LAYER, "object_set_any_type");

   if (type == lStringT) {
      cull_ret = lSetPosString(this_elem, pos, *((const char **)value));
   } else if (type == lHostT) {
      cull_ret = lSetPosHost(this_elem, pos, *((const char **)value));
   } else if (type == lUlongT) {
      cull_ret = lSetPosUlong(this_elem, pos, *((lUlong*)value));
   } else if (type == lDoubleT) {
      cull_ret = lSetPosDouble(this_elem, pos, *((lDouble*)value));
   } else if (type == lFloatT) {
      cull_ret = lSetPosFloat(this_elem, pos, *((lFloat*)value));
   } else if (type == lLongT) {
      cull_ret = lSetPosLong(this_elem, pos, *((lLong*)value));
   } else if (type == lCharT) {
      cull_ret = lSetPosChar(this_elem, pos, *((lChar*)value));
   } else if (type == lBoolT) {
      cull_ret = lSetPosBool(this_elem, pos, *((bool*)value));
   } else if (type == lIntT) {
      cull_ret = lSetPosInt(this_elem, pos, *((int*)value));
   } else if (type == lObjectT) {
      cull_ret = lSetPosObject(this_elem, pos, *((lListElem **)value));
   } else if (type == lRefT) {
      cull_ret = lSetPosRef(this_elem, pos, *((lRef*)value));
   } else if (type == lListT) {
      cull_ret = lSetPosList(this_elem, pos, lCopyList("", *((lList **)value)));
   } else {
      /* not possible */
      cull_ret = -1;
   }
   DRETURN(cull_ret == 0 ? true : false);
}

bool
object_replace_any_type(lListElem *this_elem, int name, lListElem *org_elem)
{
   int cull_ret = 0;
   int out_pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);
   int in_pos = lGetPosViaElem(org_elem, name, SGE_NO_ABORT);
   int type = lGetPosType(lGetElemDescr(this_elem), out_pos);

   DENTER(OBJECT_LAYER, "object_replace_any_type");
   if (type == lStringT) {
      const char *value = lGetPosString(org_elem, in_pos);

      cull_ret = lSetPosString(this_elem, out_pos, value);
   } else if (type == lHostT) {
      const char *value = lGetPosHost(org_elem, in_pos);
      
      cull_ret = lSetPosHost(this_elem, out_pos, value);
   } else if (type == lUlongT) {
      u_long32 value = lGetPosUlong(org_elem, in_pos);
   
      cull_ret = lSetPosUlong(this_elem, out_pos, value);
   } else if (type == lDoubleT) {
      double value = lGetPosDouble(org_elem, in_pos);
   
      cull_ret = lSetPosDouble(this_elem, out_pos, value);
   } else if (type == lFloatT) {
      float value = lGetPosFloat(org_elem, in_pos);

      cull_ret = lSetPosFloat(this_elem, out_pos, value);
   } else if (type == lLongT) {
      int value = lGetPosLong(org_elem, in_pos);

      cull_ret = lSetPosLong(this_elem, out_pos, value);
   } else if (type == lCharT) {
      char value = lGetPosChar(org_elem, in_pos);

      cull_ret = lSetPosChar(this_elem, out_pos, value);
   } else if (type == lBoolT) {
      bool value = lGetPosBool(org_elem, in_pos) ? true : false;
   
      cull_ret = lSetPosBool(this_elem, out_pos, value);
   } else if (type == lIntT) {
      int value = lGetPosInt(org_elem, in_pos);
      
      cull_ret = lSetPosInt(this_elem, out_pos, value);
   } else if (type == lObjectT) {
      lListElem *value = lGetPosObject(org_elem, in_pos);
      
      cull_ret = lSetPosObject(this_elem, out_pos, value);
   } else if (type == lRefT) {
      void *value = lGetPosRef(org_elem, in_pos);

      cull_ret = lSetPosRef(this_elem, out_pos, value);
   } else {
      /* not possible */
      cull_ret = -1;
   }
   DRETURN(cull_ret == 0 ? true : false);
}

void 
object_get_any_type(lListElem *this_elem, int name, void *value)
{
   int pos = lGetPosViaElem(this_elem, name, SGE_NO_ABORT);
   int type = lGetPosType(lGetElemDescr(this_elem), pos);

   DENTER(OBJECT_LAYER, "object_get_any_type");
   if (value != NULL) {
      if (type == lStringT) {
         *((const char **)value) = lGetPosString(this_elem, pos);
      } else if (type == lHostT) {
         *((const char **)value) = lGetPosHost(this_elem, pos);
      } else if (type == lUlongT) {
         *((lUlong*)value) = lGetPosUlong(this_elem, pos);
      } else if (type == lDoubleT) {
         *((lDouble*)value) = lGetPosDouble(this_elem, pos);
      } else if (type == lFloatT) {
         *((lFloat*)value) = lGetPosFloat(this_elem, pos);
      } else if (type == lLongT) {
         *((lLong*)value) = lGetPosLong(this_elem, pos);
      } else if (type == lCharT) {
         *((lChar*)value) = lGetPosChar(this_elem, pos);
      } else if (type == lBoolT) {
         *((bool*)value) = lGetPosBool(this_elem, pos) ? true : false;
      } else if (type == lIntT) {
         *((int*)value) = lGetPosInt(this_elem, pos);
      } else if (type == lObjectT) {
         *((lListElem **)value) = lGetPosObject(this_elem, pos);
      } else if (type == lRefT) {
         *((lRef *)value) = lGetPosRef(this_elem, pos);
      } else if (type == lListT) {
         *((lList **)value) = lGetPosList(this_elem, pos);
      } else {
         DTRACE;
         /* not possible */
      }
   }
   DRETURN_VOID;
}

bool 
object_has_differences(const lListElem *this_elem, lList **answer_list,
                       const lListElem *old_elem, bool modify_changed_flag)
{
   bool ret = false;

   DENTER(TOP_LAYER, "object_has_differences");

   if (this_elem != NULL && old_elem != NULL) {
      lDescr *this_elem_descr = this_elem->descr;
      lDescr *old_elem_descr = old_elem->descr;
      lDescr *tmp_decr1 = NULL;
      lDescr *tmp_decr2 = NULL;

      /*
       * Compare each attribute of the given elements
       */
      for (tmp_decr1 = this_elem_descr, tmp_decr2 = old_elem_descr; 
           tmp_decr1->nm != NoName && tmp_decr2->nm != NoName; 
           tmp_decr1++, tmp_decr2++) {
         int pos = tmp_decr1 - this_elem_descr;
         int type1 = this_elem_descr[pos].mt;
         int type2 = old_elem_descr[pos].mt;
         bool equiv = false;

         /* 
          * Compare name and type
          */
         if (tmp_decr1->nm != tmp_decr2->nm ||
             mt_get_type(type1) != mt_get_type(type2)) {
            DPRINTF(("Attribute "SFQ" of type "SFQ" cannot be compared with"
                     "attribute "SFQ" of type "SFQ".\n", 
                     lNm2Str(tmp_decr1->nm), multitypes[mt_get_type(type1)],
                     lNm2Str(tmp_decr2->nm), multitypes[mt_get_type(type2)]));
            ret = true;
            break;
         }

         /*
          * Compare value of attributes
          */
         switch (mt_get_type(type1)) {
            case lFloatT:
               equiv = (lGetPosFloat(this_elem, pos) == lGetPosFloat(old_elem, pos)) ? true : false;
               break;
            case lDoubleT:
               equiv = (lGetPosDouble(this_elem, pos) == lGetPosDouble(old_elem, pos)) ? true : false;
               break;
            case lUlongT:
               equiv = (lGetPosUlong(this_elem, pos) == lGetPosUlong(old_elem, pos)) ? true : false;
               break;
            case lLongT:
               equiv = (lGetPosLong(this_elem, pos) == lGetPosLong(old_elem, pos)) ? true : false;
               break;
            case lCharT:
               equiv = (lGetPosChar(this_elem, pos) == lGetPosChar(old_elem, pos)) ? true : false;
               break;
            case lBoolT:
               equiv = (lGetPosBool(this_elem, pos) == lGetPosBool(old_elem, pos)) ? true : false;
               break;
            case lIntT:
               equiv = (lGetPosInt(this_elem, pos) == lGetPosInt(old_elem, pos)) ? true : false;
               break;
            case lStringT:
               {
                  const char *new_str = lGetPosString(this_elem, pos);
                  const char *old_str = lGetPosString(old_elem, pos);

                  if ((new_str == NULL && old_str != NULL) || 
                      (new_str != NULL && old_str == NULL)) {
                     equiv = false;
                  } else if (new_str == old_str) {
                     equiv = true;
                  } else {
                     equiv = (new_str /* maybe unnecessary, but avoids
                                         possible dataflow warning */
                              && strcmp(new_str, old_str) == 0) ?
                       true : false;
                  }
               }
               break;
            case lHostT:
               {                                                                                  
                  const char *new_str = lGetPosHost(this_elem, pos);
                  const char *old_str = lGetPosHost(old_elem, pos);

                  if ((new_str == NULL && old_str != NULL) || 
                      (new_str != NULL && old_str == NULL)) {
                     equiv = false;
                  } else if (new_str == old_str) {
                     equiv = true;
                  } else {
                     equiv = (sge_hostcmp(new_str, old_str) == 0) ? true : false;
                  }
               }
               break;
            case lRefT:
               equiv = (lGetPosRef(this_elem, pos) == lGetPosRef(old_elem, pos)) ? true : false;
               break;
            case lObjectT:
               {
                  lListElem *new_obj = lGetPosObject(this_elem, pos);
                  lListElem *old_obj = lGetPosObject(old_elem, pos);

                  equiv = object_has_differences(new_obj, answer_list, 
                                                  old_obj, modify_changed_flag)? false : true;
               }
               break;
            case lListT:
               {
                  lList *new_list = lGetPosList(this_elem, pos);
                  lList *old_list = lGetPosList(old_elem, pos);

                  if (object_list_has_differences(new_list, answer_list,
                                                  old_list, 
                                                  modify_changed_flag)) {
                     equiv = false;
                  } else {
                     equiv = true;
                  }
               }
               break;
            default:
               DTRACE;
               equiv = false;
               break;
         }

         if (!equiv) {
            DPRINTF(("Attributes "SFQ" of type "SFQ" are not equivalent.\n", 
                     lNm2Str(tmp_decr1->nm), multitypes[mt_get_type(type1)]));
            DTRACE;
            ret = true;
         }
      }

      /*
       * Compare number of attributes within each element
       */
      if (tmp_decr1->nm != tmp_decr2->nm) {
         DPRINTF(("Descriptor size is not equivalent\n"));
         ret = true;
      } 
   } else if (this_elem != NULL || old_elem != NULL) {
      /* One of both elems is not NULL */
      ret = true;
   }

   DRETURN(ret);
}

bool
object_list_has_differences(const lList *this_list, lList **answer_list,
                            const lList *old_list, bool modify_changed_flag)
{
   bool ret = false;

   DENTER(BASIS_LAYER, "object_list_has_differences");

   if (this_list == NULL && old_list == NULL) {
      ret = false;
   } else if (lGetNumberOfElem(this_list) == lGetNumberOfElem(old_list)) {
      lListElem *new_elem;
      lListElem *old_elem;

      for(new_elem = lFirst(this_list), old_elem = lFirst(old_list);
          new_elem != NULL && old_elem != NULL;
          new_elem = lNext(new_elem), old_elem = lNext(old_elem)) {

         ret = object_has_differences(new_elem, answer_list,
                                      old_elem, modify_changed_flag);
         if (ret) {
            break;
         }
      }
   } else {
      DTRACE;
      ret = true;
   }

   DRETURN(ret);
}

/****** sge_object/object_list_verify_cull() ***********************************
*  NAME
*     object_list_verify_cull() -- verify cull list structure
*
*  SYNOPSIS
*     bool 
*     object_list_verify_cull(const lList *lp, const lDescr *descr) 
*
*  FUNCTION
*     Verifies that a cull list, including all its objects and sublists,
*     is a valid object of a defined type.
*  
*     There are cases, where the type of a list cannot be verified, when the
*     type of a sublist is set to CULL_ANY_SUBTYPE in the cull list definition.
*     In this case, the list descriptor is not checked, but still all list
*     objects and there sublists are verified.
*
*  INPUTS
*     const lList *lp     - list to verify
*     const lDescr *descr - expected descriptor (or NULL, see above)
*
*  RESULT
*     bool - true, if the object is OK, else false
*
*  NOTES
*     MT-NOTE: object_list_verify_cull() is MT safe 
*
*  SEE ALSO
*     sge_object/object_verify_cull()
*     cull/list/lCompListDescr()
*******************************************************************************/
bool 
object_list_verify_cull(const lList *lp, const lDescr *descr)
{
   bool ret = true;

   /* 
    * valid input parameters?
    * descr may be NULL, if sublist is defined as CULL_ANY_SUBTYPE
    */
   if (lp == NULL) {
      ret = false;
   }

   /* does list descriptor match expected descriptor? */
   if (ret) {
      if (descr != NULL) {
         if (lCompListDescr(lp->descr, descr) != 0) {
            ret = false;
         }
      }
   }

   /* recursively check sublists and subobjects */
   if (ret) {
      const lListElem *ep;
      for_each (ep, lp) {
         if (!object_verify_cull(ep, NULL)) {
            ret = false;
            break;
         }
      }
   }

   return ret;
}

/****** sge_object/object_verify_cull() ****************************************
*  NAME
*     object_verify_cull() -- verify cull object structure
*
*  SYNOPSIS
*     bool 
*     object_verify_cull(const lListElem *ep, const lDescr *descr) 
*
*  FUNCTION
*     Verifies that a cull object, including all its sublists and subobjects,
*     is a valid object of a defined type.
*  
*     The type (descr) argument may be zero in two cases:
*        - the descriptor has already been checked. This is the case when called
*          from object_list_verify_cull().
*        - we don't know the type of a subobject (if it is defined as 
*          CULL_ANY_SUBTYPE in the cull object definition
*
*  INPUTS
*     const lListElem *ep - the object to verify
*     const lDescr *descr - expected object type, or NULL (see above)
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     MT-NOTE: object_verify_cull() is MT safe 
*
*  SEE ALSO
*     sge_object/object_verify_cull()
*     cull/list/lCompListDescr()
*******************************************************************************/
bool 
object_verify_cull(const lListElem *ep, const lDescr *descr)
{
   bool ret = true;

   /* 
    * valid input parameters?
    * descr may be NULL, if subobject is defined as CULL_ANY_SUBTYPE
    */
   if (ep == NULL) {
      ret = false;
   }

   /* does object descriptor match expected descriptor? */
   if (ret) {
      if (descr != NULL) {
         if (lCompListDescr(ep->descr, descr) != 0) {
            ret = false;
         }
      }
   }

   /* check contents of subobject */
   if (ret) {
      int i = 0;
      while (ep->descr[i].nm != NoName) {
         int type = mt_get_type(ep->descr[i].mt);
         if (type == lListT) {
            lList *lp = lGetList(ep, ep->descr[i].nm);
            if (lp != NULL) {
               const lDescr *subdescr = object_get_subtype(ep->descr[i].nm);
               if (!object_list_verify_cull(lp, subdescr)) {
                  ret = false;
                  break;
               }
            }
         } else if (type == lObjectT) {
            lListElem *sub_ep = lGetObject(ep, ep->descr[i].nm);
            if (sub_ep != NULL) {
               const lDescr *subdescr = object_get_subtype(ep->descr[i].nm);
               if (!object_verify_cull(sub_ep, subdescr)) {
                  ret = false;
                  break;
               }
            }
         } 
         i++;
      }
   }

   return ret;
}

/****** sge_object/object_verify_ulong_not_null() ******************************
*  NAME
*     object_verify_ulong_not_null() -- verify ulong attribute not null
*
*  SYNOPSIS
*     bool 
*     object_verify_ulong_not_null(const lListElem *ep, lList **answer_list, 
*                                  int nm) 
*
*  FUNCTION
*     Verify that a certain ulong attribute in an object is not 0
*
*  INPUTS
*     const lListElem *ep - object to verify
*     lList **answer_list - answer list to pass back error messages
*     int nm              - the attribute to verify
*
*  RESULT
*     bool - true on success,
*            false on error with error message in answer_list
*
*  NOTES
*     MT-NOTE: object_verify_ulong_not_null() is MT safe 
*
*  SEE ALSO
*     sge_object/object_verify_string_not_null()
*******************************************************************************/
bool 
object_verify_ulong_not_null(const lListElem *ep, lList **answer_list, int nm)
{
   bool ret = true;

   if (lGetUlong(ep, nm) == 0) {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR, 
                              MSG_OBJECT_ULONG_NOT_NULL, lNm2Str(nm));
      ret = false;
   }

   return ret;
}

/****** sge_object/object_verify_ulong_null() ******************************
*  NAME
*     object_verify_ulong_null() -- verify ulong attribute null
*
*  SYNOPSIS
*     bool 
*     object_verify_ulong_null(const lListElem *ep, lList **answer_list, 
*                                  int nm) 
*
*  FUNCTION
*     Verify that a certain ulong attribute in an object is 0
*
*  INPUTS
*     const lListElem *ep - object to verify
*     lList **answer_list - answer list to pass back error messages
*     int nm              - the attribute to verify
*
*  RESULT
*     bool - true on success,
*            false on error with error message in answer_list
*
*  NOTES
*     MT-NOTE: object_verify_ulong_null() is MT safe 
*
*  SEE ALSO
*     sge_object/object_verify_string_not_null()
*******************************************************************************/
bool 
object_verify_ulong_null(const lListElem *ep, lList **answer_list, int nm)
{
   bool ret = true;

   if (lGetUlong(ep, nm) != 0) {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR, 
                              MSG_OBJECT_ULONG_NULL, lNm2Str(nm));
      ret = false;
   }

   return ret;
}

/****** sge_object/object_verify_ulong_null() ******************************
*  NAME
*     object_verify_double_null() -- verify double attribute null
*
*  SYNOPSIS
*     bool 
*     object_verify_ulong_null(const lListElem *ep, lList **answer_list, 
*                                  int nm) 
*
*  FUNCTION
*     Verify that a certain double attribute in an object is 0
*
*  INPUTS
*     const lListElem *ep - object to verify
*     lList **answer_list - answer list to pass back error messages
*     int nm              - the attribute to verify
*
*  RESULT
*     bool - true on success,
*            false on error with error message in answer_list
*
*  NOTES
*     MT-NOTE: object_verify_double_null() is MT safe 
*
*  SEE ALSO
*     sge_object/object_verify_string_not_null()
*******************************************************************************/
bool 
object_verify_double_null(const lListElem *ep, lList **answer_list, int nm)
{
   bool ret = true;

   if (lGetDouble(ep, nm) != 0.0) {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR, 
                              MSG_OBJECT_DOUBLE_NULL, lNm2Str(nm));
      ret = false;
   }

   return ret;
}

/****** sge_object/object_verify_string_not_null() *****************************
*  NAME
*     object_verify_string_not_null() -- verify string attribute not null
*
*  SYNOPSIS
*     bool 
*     object_verify_string_not_null(const lListElem *ep, lList **answer_list, 
*                                   int nm) 
*
*  FUNCTION
*     Verifies that a string attribute of a certain object is not NULL.
*
*  INPUTS
*     const lListElem *ep - the object to verify
*     lList **answer_list - answer list to pass back error messages
*     int nm              - the attribute to verify
*
*  RESULT
*     bool - true on success,
*            false on error with error message in answer_list
*
*  NOTES
*     MT-NOTE: object_verify_string_not_null() is MT safe 
*
*  SEE ALSO
*     sge_object/object_verify_ulong_not_null()
*******************************************************************************/
bool 
object_verify_string_not_null(const lListElem *ep, lList **answer_list, int nm)
{
   bool ret = true;

   if (lGetString(ep, nm) == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR, 
                              MSG_OBJECT_STRING_NOT_NULL, lNm2Str(nm));
      ret = false;
   }

   return ret;
}

/****** sge_object/object_verify_expression_syntax() *****************************
*  NAME
*     object_verify_expression_syntax() -- verify string attribute expression syntax
*
*  SYNOPSIS
*     bool 
*     object_verify_expression_syntax(const lListElem *ep, lList **answer_list) 
*
*  FUNCTION
*     Verifies that a string is expression in correct format.
*
*  INPUTS
*     const lListElem *ep - the object to verify
*     lList **answer_list - answer list to pass back error messages
*
*  RESULT
*     bool - true on success,
*            false on error with error message in answer_list
*
*  NOTES
*     MT-NOTE: object_verify_expression_syntax() is MT safe 
*
*  SEE ALSO
*******************************************************************************/
bool 
object_verify_expression_syntax(const lListElem *elem, lList **answer_list)
{
   bool ret = true;
   const char *expr;
   lUlong type;
   type = lGetUlong(elem, CE_valtype); 
   switch(type){
      case TYPE_STR:  
      case TYPE_CSTR: 
      case TYPE_RESTR: 
      case TYPE_HOST:  
         expr = lGetString(elem, CE_stringval);         
         if (sge_eval_expression(type,expr,"*",answer_list)==-1) {
            ret = false;
         }
         break;
   }    
   return ret;
}


/****** sge_object/object_verify_name() *****************************************
*  NAME
*     object_verify_name() - verifies object name
*
*  SYNOPSIS
*     int object_verify_name(const lListElem *object, lList **alpp, int name, object_descr) 
*
*  FUNCTION
*     These checks are done for the attribute JB_object_name of 'object':
*     #1 reject object name if it starts with a digit
*     A detailed problem description is added to the answer list.
*
*  INPUTS
*     const lListElem *object  - JB_Type elemente
*     lList **alpp             - the answer list
*     int   name               - object name  
*     char *object_descr       - used for the text in the answer list 
*
*  RESULT
*     int - returns != 0 if there is a problem with the object name
*
*  MT-NOTE: sge_resolve_host() is MT safe
*
******************************************************************************/
int object_verify_name(const lListElem *object, lList **answer_list, int name, 
                           const char *object_descr)
{
   const char *object_name = lGetString(object, name);
   int ret = 0;
   
   DENTER(TOP_LAYER, "object_verify_name");
   if (object_name != NULL) {
      if (isdigit(object_name[0])) {
         ERROR((SGE_EVENT, MSG_OBJECT_INVALID_NAME_S, object_name));
         answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = STATUS_EUNKNOWN;
      } else {
         if (verify_str_key(
               answer_list, object_name, MAX_VERIFY_STRING,
               lNm2Str(name), QSUB_TABLE) != STATUS_OK) {
            ret = STATUS_EUNKNOWN;
         }
      }
   }

   DRETURN(ret);
}


/****** sge_object/object_verify_pe_range() **********************************
*  NAME
*     object_verify_pe_range() -- Verify validness of a jobs PE range request
*
*  SYNOPSIS
*     int object_verify_pe_range(lList **alpp, const char *pe_name,
*     lList *pe_range)
*
*  FUNCTION
*     Verifies a jobs PE range is valid. Currently the following is done
*     - make PE range list normalized and ascending
*     - ensure PE range min/max not 0
*     - in case multiple PEs match the PE request in GEEE ensure
*       the urgency slots setting is non-ambiguous
*
*  INPUTS
*     lList **alpp        - Returns answer list with error context.
*     const char *pe_name - PE request
*     lList *pe_range     - PE range to be verified
*     const char *object_descr - object description for user messages
*
*  RESULT
*     static int - STATUS_OK on success
*
*  NOTES
*
*******************************************************************************/
int object_verify_pe_range(lList **alpp, const char *pe_name, lList *pe_range,
                                  const char *object_descr)
{
   lListElem *relem = NULL;
   unsigned long pe_range_max;
   unsigned long pe_range_min;
   
   DENTER(TOP_LAYER, "object_verify_pe_range");
   
   /* ensure jobs PE range list request is normalized and ascending */
   range_list_sort_uniq_compress(pe_range, NULL, true);
   
   for_each(relem, pe_range) {
      pe_range_min = lGetUlong(relem, RN_min);
      pe_range_max = lGetUlong(relem, RN_max);
      DPRINTF(("pe max = %ld, pe min = %ld\n", pe_range_max, pe_range_min));
      if ( pe_range_max == 0 || pe_range_min == 0  ) {
         ERROR((SGE_EVENT, MSG_OBJECT_PERANGEMUSTBEGRZERO_S, object_descr ));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(STATUS_EUNKNOWN);
      }
   }
   
   /* PE slot ranges used in conjunction with wildcards can cause number of slots
    finally being used for urgency value computation be ambiguous. We reject such
    jobs */
   if (range_list_get_number_of_ids(pe_range)>1) {
      lList *master_pe_list = *object_type_get_master_list(SGE_TYPE_PE);
      const lListElem *reference_pe = pe_list_find_matching(master_pe_list, pe_name);
      lListElem *pe;
      int nslots = pe_urgency_slots(reference_pe, lGetString(reference_pe, PE_urgency_slots), pe_range);
      for_each(pe, master_pe_list) {
         if (pe_is_matching(pe, pe_name) &&
             nslots != pe_urgency_slots(pe, lGetString(pe, PE_urgency_slots), pe_range))
         {
            ERROR((SGE_EVENT, MSG_OBJECT_WILD_RANGE_AMBIGUOUS_S, object_descr));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DRETURN(STATUS_EUNKNOWN);
         }
      }
   }
   
   DRETURN(STATUS_OK);
}

/****** sge_objectcompress_ressources() **********************************
*  NAME
*     compress_ressources() --  remove multiple requests for one resource
*
*  SYNOPSIS
*     int compress_ressources(lList **alpp, lList *rl, const char *object_descr )
*
*  FUNCTION
*     remove multiple requests for one resource
*     this can't be done fully in clients without having complex definitions
*     -l arch=linux -l a=sun4
*
*  INPUTS
*     lList **alpp        - Returns answer list with error context.
*     lList *rl           - object description for user messages
*     const char *object_descr - object description 
*
*  RESULT
*     static int - 0 on success
*
*  NOTES
*
*******************************************************************************/
int compress_ressources(lList **alpp, lList *rl, const char *object_descr) {
   lListElem *ep, *prev, *rm_ep;
   const char *attr_name;

   DENTER(TOP_LAYER, "compress_ressources");

   for_each_rev (ep, rl) { 
      attr_name = lGetString(ep, CE_name);

      /* ensure 'slots' is not requested explicitly */
      if (!strcmp(attr_name, "slots")) {
         ERROR((SGE_EVENT, MSG_OBJECT_NODIRECTSLOTS_S, object_descr)); 
         answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
         DRETURN(-1);
      }

      /* remove all previous requests with the same name */
      prev =  lPrev(ep);
      while ((rm_ep=prev)) {
         prev = lPrev(rm_ep);
         if (!strcmp(lGetString(rm_ep, CE_name), attr_name)) {
            DPRINTF(("resource request -l "SFN"="SFN" overrides previous -l "SFN"="SFN"\n",
               attr_name, lGetString(ep, CE_stringval), 
               attr_name, lGetString(rm_ep, CE_stringval)));
            lRemoveElem(rl, &rm_ep);
         }
      }
   }

   DRETURN(0);
}

/****** sge_object/object_unpack_elem_verify() *********************************
*  NAME
*     object_unpack_elem_verify() -- unpack and verify an object
*
*  SYNOPSIS
*     bool 
*     object_unpack_elem_verify(lList **answer_list, sge_pack_buffer *pb, 
*                               lListElem **epp, const lDescr *descr) 
*
*  FUNCTION
*     Unpacks the given packbuffer.
*     If unpacking was successfull, verifies the object
*     against the given descriptor.
*
*  INPUTS
*     lList **answer_list - answer list to report errors
*     sge_pack_buffer *pb - the packbuffer containing the object
*     lListElem **epp     - element pointer to pass back the unpacked object
*     const lDescr *descr - the expected object type
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     MT-NOTE: object_unpack_elem_verify() is MT safe 
*
*  SEE ALSO
*     sge_object/object_verify_cull()
*******************************************************************************/
bool object_unpack_elem_verify(lList **answer_list, sge_pack_buffer *pb, lListElem **epp, const lDescr *descr)
{
   bool ret = true;

   DENTER(TOP_LAYER, "object_unpack_elem_verify");

   if (pb == NULL || epp == NULL || descr == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                              MSG_NULLELEMENTPASSEDTO_S, "object_unpack_elem_verify");
      ret = false;
   }

   if (ret) {
      if (cull_unpack_elem(pb, epp, NULL) != 0) {
         answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 MSG_COM_UNPACKOBJ_S, object_get_name(descr));
         ret = false;
      }
   }

   if (ret) {
      if (!object_verify_cull(*epp, descr)) {
         lFreeElem(epp);
         answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR, 
                                 "%s", MSG_OBJECT_STRUCTURE_ERROR);
         ret = false;
      }
   }

   DRETURN(ret);
}

