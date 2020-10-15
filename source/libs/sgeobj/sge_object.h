#ifndef __SGE_OBJECT_H
#define __SGE_OBJECT_H
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

#include "uti/sge_dstring.h"

#include "cull/cull.h"
#include "cull/cull_list.h"

/****** sgeobj/object/--Object-Handling ***************************************
*
*  NAME
*     Object Handling -- utilities for sgeobj object access
*
*  FUNCTION
*     This module provides utility functions for accessing CULL 
*     objects, e.g. getting a string representation for fields, 
*     setting field contents from string representation etc.
*
*  NOTES
*
*  SEE ALSO
*     sgeobj/Object/object_has_type()
*     sgeobj/Object/object_get_type()
*     sgeobj/Object/object_get_subtype()
*     sgeobj/Object/object_get_primary_key()
*     sgeobj/Object/object_get_name_prefix()
*     sgeobj/Object/object_append_field_to_dstring()
*     sgeobj/Object/object_parse_field_from_string()
*     sgeobj/Object/object_type_get_master_list()
*     sgeobj/Object/object_type_free_master_list()
*     sgeobj/Object/object_type_get_name()
*     sgeobj/Object/object_type_get_descr()
*     sgeobj/Object/object_type_get_key_nm()
******************************************************************************/

#define NULL_OUT_NONE(ep, nm) \
   if (lGetString(ep, nm) != NULL && \
       strcasecmp(lGetString(ep, nm), "none") == 0) { \
      lSetString(ep, nm, NULL); \
   }

/****** sgeobj/object/--Object-Typedefs ***************************************
*
*  NAME
*     Object-Typedefs -- typedefs for generic object handling
*
*  SYNOPSIS
*     The enumeration sge_object_type defines different object and 
*     message types.
*
*     The following types are defined:
*        SGE_TYPE_ADMINHOST
*        SGE_TYPE_CALENDAR
*        SGE_TYPE_CKPT
*        SGE_TYPE_CONFIG
*        SGE_TYPE_GLOBAL_CONFIG
*        SGE_TYPE_EXECHOST
*        SGE_TYPE_JATASK
*        SGE_TYPE_PETASK
*        SGE_TYPE_JOB
*        SGE_TYPE_JOB_SCHEDD_INFO
*        SGE_TYPE_MANAGER
*        SGE_TYPE_OPERATOR
*        SGE_TYPE_SHARETREE
*        SGE_TYPE_PE
*        SGE_TYPE_PROJECT
*        SGE_TYPE_CQUEUE
*        SGE_TYPE_QINSTANCE
*        SGE_TYPE_SCHEDD_CONF
*        SGE_TYPE_SCHEDD_MONITOR
*        SGE_TYPE_SHUTDOWN
*        SGE_TYPE_MARK_4_REGISTRATION
*        SGE_TYPE_SUBMITHOST
*        SGE_TYPE_USER
*        SGE_TYPE_USERSET
*        SGE_TYPE_CUSER
*        SGE_TYPE_CENTRY   
*        SGE_TYPE_ZOMBIE
*        SGE_TYPE_SUSER
*        SGE_TYPE_RQS
*        SGE_TYPE_AR
*        SGE_TYPE_JOBSCRIPT
*
*     If usermapping is enabled, an additional object type is defined:
*        SGE_TYPE_HGROUP
*  
*     The last value defined as obect type is SGE_TYPE_ALL. 
*****************************************************************************/
typedef enum {
   SGE_TYPE_ADMINHOST = 0,
   SGE_TYPE_CALENDAR,         /*1*/
   SGE_TYPE_CKPT,             /*2*/
   SGE_TYPE_CONFIG,           /*3*/
   SGE_TYPE_GLOBAL_CONFIG,    /*4*/
   SGE_TYPE_EXECHOST,         /*5*/
   SGE_TYPE_JATASK,           /*6*/
   SGE_TYPE_PETASK,           /*7*/
   SGE_TYPE_JOB,              /*8*/
   SGE_TYPE_JOB_SCHEDD_INFO,  /*9*/
   SGE_TYPE_MANAGER,          /*10*/
   SGE_TYPE_OPERATOR,         /*11*/
   SGE_TYPE_SHARETREE,        /*12*/
   SGE_TYPE_PE,               /*13*/
   SGE_TYPE_PROJECT,          /*14*/
   SGE_TYPE_CQUEUE,           /*15*/
   SGE_TYPE_QINSTANCE,        /*16*/
   SGE_TYPE_SCHEDD_CONF,      /*17*/
   SGE_TYPE_SCHEDD_MONITOR,   /*18*/
   SGE_TYPE_SHUTDOWN,         /*19*/
   SGE_TYPE_MARK_4_REGISTRATION,/*20*/
   SGE_TYPE_SUBMITHOST,       /*21*/
   SGE_TYPE_USER,             /*22*/
   SGE_TYPE_USERSET,          /*23*/
   SGE_TYPE_HGROUP,           /*24*/
   SGE_TYPE_CENTRY,           /*25*/   
   SGE_TYPE_ZOMBIE,           /*26*/
   SGE_TYPE_SUSER,            /*27*/
   SGE_TYPE_RQS,              /*28*/
   SGE_TYPE_AR,               /*29*/
   SGE_TYPE_JOBSCRIPT,        /*30*/


   /*
    * Don't forget to edit
    *
    *    'mirror_base' in libs/mir/sge_mirror.c
    *    'object_base' in libs/sgeobj/sge_object.c
    *    'table_base' in libs/spool/sge_spooling_database.c
    *
    *    '_sge_mirror_unsubscribe' libs/mir/sge_mirror.c
    *    '_sge_mirror_subscribe' libs/mir/sge_mirror.c
    * if something is changed here!
    */
#ifndef __SGE_NO_USERMAPPING__
   SGE_TYPE_CUSER,
#endif

   SGE_TYPE_ALL,            /* must be the second to the last entry */
   SGE_TYPE_NONE            /* this must the last entry */
} sge_object_type;

typedef bool (*commitMasterList)(lList **answer_list);

/* Datastructure for internal storage of object/message related information */
typedef struct {
   lList **list;                          /* master list                    */
   commitMasterList commitMasterList;     /* commit master list set changes */
   const char *type_name;                 /* type name, e.g. "JOB"      */
   lDescr *descr;                         /* descriptor, e.g. JB_Type       */
   const int key_nm;                      /* nm of key attribute        */
} object_description;


void obj_mt_init(void);
void obj_init(bool is_global);

lList **
object_type_get_master_list(const sge_object_type type);

lList **
sge_master_list(const object_description *object_base, const sge_object_type type);

bool 
object_type_commit_master_list(const sge_object_type type, lList **answer_list); 


object_description *object_type_get_object_description(void);

bool
object_type_free_master_list(const sge_object_type type);

const char *
object_type_get_name(const sge_object_type type);

sge_object_type 
object_name_get_type(const char *name);

const lDescr *
object_type_get_descr(const sge_object_type type);

int
object_type_get_key_nm(const sge_object_type type);

object_description *
object_type_get_global_object_description(void);

/* JG: TODO: rename to object_has_descr, make function object_has_type 
             and call this function where possible */
bool 
object_has_type(const lListElem *object, const lDescr *descr);

/* JG: TODO: rename to object_get_type_descr, check all calls, if possible pass sge_object_type */
const lDescr *
object_get_type(const lListElem *object);

const lDescr *
object_get_subtype(int nm);

int 
object_get_primary_key(const lDescr *descr);

const char *
object_get_name(const lDescr *descr);

const char *
object_get_name_prefix(const lDescr *descr, dstring *buffer);

const char *
object_append_field_to_dstring(const lListElem *object, lList **answer_list, 
                               dstring *buffer, const int nm,
                               const char string_quotes);
bool 
object_parse_field_from_string(lListElem *object, lList **answer_list, 
                               const int nm, const char *value);

void
object_delete_range_id(lListElem *object, lList **answer_list, 
                       const int rnm, const u_long32 id);

int 
object_set_range_id(lListElem *object, int rnm, u_long32 start, u_long32 end,
                            u_long32 step);

bool
object_parse_bool_from_string(lListElem *this_elem, lList **answer_list,
                              int name, const char *string);

bool
object_parse_ulong32_from_string(lListElem *this_elem, lList **answer_list,
                                 int name, const char *string);

bool
object_parse_int_from_string(lListElem *this_elem, lList **answer_list,
                             int name, const char *string);

bool
object_parse_char_from_string(lListElem *this_elem, lList **answer_list,
                             int name, const char *string);

bool
object_parse_long_from_string(lListElem *this_elem, lList **answer_list,
                             int name, const char *string);

bool
object_parse_double_from_string(lListElem *this_elem, lList **answer_list,
                                int name, const char *string);

bool
object_parse_float_from_string(lListElem *this_elem, lList **answer_list,
                               int name, const char *string);

bool
object_parse_time_from_string(lListElem *this_elem, lList **answer_list,
                                 int name, const char *string);

bool
object_parse_mem_from_string(lListElem *this_elem, lList **answer_list,
                             int name, const char *string);

bool
object_parse_inter_from_string(lListElem *this_elem, lList **answer_list,
                               int name, const char *string);

bool
object_parse_list_from_string(lListElem *this_elem, lList **answer_list,
                              int name, const char *string,
                              const lDescr *descr, int nm);

bool
object_parse_celist_from_string(lListElem *this_elem, lList **answer_list,
                                int name, const char *string);

bool
object_parse_solist_from_string(lListElem *this_elem, lList **answer_list,
                                int name, const char *string);

bool
object_parse_qtlist_from_string(lListElem *this_elem, lList **answer_list,
                                int name, const char *string);

bool
object_set_any_type(lListElem *this_elem, int name, void *value);

bool
object_replace_any_type(lListElem *this_elem, int name, lListElem *org_elem);

void
object_get_any_type(lListElem *this_elem, int name, void *value);

bool  
object_has_differences(const lListElem *this_elem, lList **answer_list,
                       const lListElem *old_elem, bool modify_changed_flag);

bool
object_list_has_differences(const lList *this_elem, lList **answer_list,
                            const lList *old_elem, bool modify_changed_flag);

bool object_unpack_elem_verify(lList **answer_list, sge_pack_buffer *pb, lListElem **epp, const lDescr *descr);
bool object_list_verify_cull(const lList *lp, const lDescr *descr);
bool object_verify_cull(const lListElem *ep, const lDescr *descr);

bool object_verify_ulong_not_null(const lListElem *ep, lList **answer_list, int nm);
bool object_verify_ulong_null(const lListElem *ep, lList **answer_list, int nm);
bool object_verify_double_null(const lListElem *ep, lList **answer_list, int nm);
bool object_verify_string_not_null(const lListElem *ep, lList **answer_list, int nm);
bool object_verify_expression_syntax(const lListElem *ep, lList **answer_list);

int object_verify_name(const lListElem *object, lList **answer_list, int name, const char *object_descr);
int object_verify_pe_range(lList **alpp, const char *pe_name, lList *pe_range, const char *object_descr);
int compress_ressources(lList **alpp, lList *rl, const char *object_descr );

#endif /* __SGE_OBJECT_H */
