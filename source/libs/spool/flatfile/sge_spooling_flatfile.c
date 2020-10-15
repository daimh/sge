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

/* system */
#include <errno.h>
#include <string.h>

#include "uti/setup_path.h"
#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_stdio.h"
#include "uti/sge_string.h"
#include "uti/sge_dstring.h"
#include "uti/sge_io.h"
#include "uti/sge_spool.h"

#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_cqueue.h"

/* includes for old job spooling */
#include "spool/sge_dirent.h"
#include "spool/msg_spoollib.h"
#include "spool/classic/read_write_job.h"
#include "spool/flatfile/msg_spoollib_flatfile.h"
#include "spool/flatfile/sge_flatfile.h"
#include "spool/flatfile/sge_spooling_flatfile.h"
#include "spool/flatfile/sge_flatfile_obj.h"

#include "msg_common.h"

static const char *spooling_method = "classic";

#ifdef SPOOLING_classic
const char *get_spooling_method(void)
#else
const char *get_classic_spooling_method(void)
#endif
{
   return spooling_method;
}

static bool write_manop(int spool, int target);
static bool read_manop(int target);


/* These files exist in the qmaster spool directory. These files may be
 * accessed directly, since they are used after chdir() of qmaster/execd
 * to their spool directory
 */
#define EXECHOST_DIR              "exec_hosts"
#define ADMINHOST_DIR             "admin_hosts"
#define SUBMITHOST_DIR            "submit_hosts"
#define ACCESS_DIR                "access_lists"
#define CQUEUE_DIR                "cqueues"
#define PE_DIR                    "pe"
#define UME_DIR                   "usermapping"
#define HGROUP_DIR                "hostgroups"
#define CENTRY_DIR                "centry"
#define CKPTOBJ_DIR               "ckpt"
#define CAL_DIR                   "calendars"
#define RESOURCEQUOTAS_DIR        "resource_quotas"
#define MAN_FILE                  "managers"
#define OP_FILE                   "operators"
#define USER_DIR                  "users"
#define USERSET_DIR               "usersets"
#define PROJECT_DIR               "projects"
#define AR_DIR                    "advance_reservations"

/****** spool/flatfile/spool_flatfile_create_context() ********************
*  NAME
*     spool_flatfile_create_context() -- create a flatfile spooling context
*
*  SYNOPSIS
*     lListElem* 
*     spool_flatfile_create_context(lList **answer_list, const char *args)
*
*  FUNCTION
*     Create a spooling context for the flatfile spooling.
* 
*     Two rules are created: One for spooling in the common directory, the
*     other for spooling in the spool directory.
*
*     The following object type descriptions are create:
*        - for SGE_TYPE_CONFIG, referencing the rule for the common directory
*        - for SGE_TYPE_SCHEDD_CONF, also referencing the rule for the common
*          directory
*        - for SGE_TYPE_ALL (default for all object types), referencing the rule
*          for the spool directory
*
*     The function expects to get as argument two absolute paths:
*        1. The path of the common directory
*        2. The path of the spool directory
*     The format of the input string is "<common_dir>;<spool_dir>"
*
*  INPUTS
*     lList **answer_list - to return error messages
*     const char *args - arguments to classic spooling
*
*  RESULT
*     lListElem* - on success, the new spooling context, else NULL
*
*  SEE ALSO
*     spool/--Spooling
*     spool/flatfile/--Flatfile-Spooling
*******************************************************************************/
lListElem *
spool_classic_create_context(lList **answer_list, const char *args)
{
   lListElem *context = NULL;

   DENTER(TOP_LAYER, "spool_classic_create_context");

   /* check parameters - both must be set and be absolute paths */
   if (args == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              "%s", MSG_SPOOL_INCORRECTPATHSFORCOMMONANDSPOOLDIR);
   } else {
      char *common_dir, *spool_dir;
      struct saved_vars_s *strtok_context = NULL;

      common_dir = sge_strtok_r(args, ";", &strtok_context);
      spool_dir  = sge_strtok_r(NULL, ";", &strtok_context);
      
      if (common_dir == NULL || spool_dir == NULL ||
         *common_dir != '/' || *spool_dir != '/') {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 "%s", MSG_SPOOL_INCORRECTPATHSFORCOMMONANDSPOOLDIR);
      } else {   
         sge_object_type i;
         flatfile_info *field_info;
         lListElem *rule, *type;

         /* create info which fields to spool once */
         field_info = malloc(sizeof(flatfile_info) * SGE_TYPE_ALL);
         for (i = SGE_TYPE_ADMINHOST; i < SGE_TYPE_ALL; i++) {
            switch (i) {
               /* pseudo types without spooling action */
               case SGE_TYPE_GLOBAL_CONFIG:
               case SGE_TYPE_JOB_SCHEDD_INFO:
               case SGE_TYPE_SCHEDD_MONITOR:
               case SGE_TYPE_SHUTDOWN:
               case SGE_TYPE_MARK_4_REGISTRATION:
                  field_info[i].fields = NULL;
                  field_info[i].instr  = NULL;
                  break;
               case SGE_TYPE_CQUEUE:
                  field_info[i].fields = CQ_fields;
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_ADMINHOST:
                  field_info[i].fields = AH_fields;
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_SUBMITHOST:
                  field_info[i].fields = SH_fields;
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_HGROUP:
                  field_info[i].fields = HGRP_fields;
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_CALENDAR:
                  field_info[i].fields = CAL_fields;
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_QINSTANCE:
                  field_info[i].fields = sge_build_QU_field_list(false, true);
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_USERSET:
                  field_info[i].fields = US_fields;
                  field_info[i].instr  = &qconf_param_sfi;
                  break;
               /* standard case of spooling */
#ifndef __SGE_NO_USERMAPPING__ 
               case SGE_TYPE_CUSER:
#endif
               case SGE_TYPE_SCHEDD_CONF:
                  field_info[i].fields = spool_get_fields_to_spool(answer_list, 
                                              object_type_get_descr(i), 
                                              &spool_config_instr);
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_MANAGER:
               case SGE_TYPE_OPERATOR:
                  field_info[i].fields = NULL;
                  field_info[i].instr  = NULL;
                  break;
               case SGE_TYPE_EXECHOST:
                  field_info[i].fields = sge_build_EH_field_list(true, false, false);
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_PE:
                  field_info[i].fields = PE_fields;
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_CKPT:
                  field_info[i].fields = CK_fields;
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_CONFIG:
               /* special case config spooling */
                  field_info[i].fields = sge_build_CONF_field_list(true);
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_PROJECT:
                  field_info[i].fields = sge_build_PR_field_list(true);
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_USER:
                  field_info[i].fields = sge_build_UU_field_list(true);
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_SHARETREE:
                  field_info[i].fields = sge_build_STN_field_list(true, true);
                  field_info[i].instr  = &qconf_name_value_list_sfi;
                  break;
               case SGE_TYPE_CENTRY:
                  field_info[i].fields = CE_fields;
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_RQS:
                  field_info[i].fields = RQS_fields;
                  field_info[i].instr  = &qconf_rqs_sfi;
                  break;
               case SGE_TYPE_AR:
                  field_info[i].fields = AR_fields;
                  field_info[i].instr  = &qconf_sfi;
                  break;
               case SGE_TYPE_JOB:
               case SGE_TYPE_JATASK:
               case SGE_TYPE_PETASK:
               default:
                  break;
            }
         }

         /* create spooling context */
         context = spool_create_context(answer_list, "flatfile spooling");
         
         /* create rule and type for all objects spooled in the spool dir */
         rule = spool_context_create_rule(answer_list, context, 
                                          "default rule (spool dir)", 
                                          spool_dir,
                                          NULL,
                                          spool_classic_default_startup_func,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          spool_classic_default_list_func,
                                          spool_classic_default_read_func,
                                          spool_classic_default_write_func,
                                          spool_classic_default_delete_func,
                                          spool_default_validate_func,
                                          spool_default_validate_list_func);
         lSetRef(rule, SPR_clientdata, field_info);
         type = spool_context_create_type(answer_list, context, SGE_TYPE_ALL);
         spool_type_add_rule(answer_list, type, rule, true);

         /* create rule and type for all objects spooled in the common dir */
         rule = spool_context_create_rule(answer_list, context, 
                                          "default rule (common dir)", 
                                          common_dir,
                                          NULL,
                                          spool_classic_common_startup_func,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          spool_classic_default_list_func,
                                          spool_classic_default_read_func,
                                          spool_classic_default_write_func,
                                          spool_classic_default_delete_func,
                                          spool_default_validate_func,
                                          spool_default_validate_list_func);
         lSetRef(rule, SPR_clientdata, field_info);
         type = spool_context_create_type(answer_list, context, 
                                          SGE_TYPE_CONFIG);
         spool_type_add_rule(answer_list, type, rule, true);
         type = spool_context_create_type(answer_list, context, 
                                          SGE_TYPE_SCHEDD_CONF);
         spool_type_add_rule(answer_list, type, rule, true);
      }
      sge_free_saved_vars(strtok_context);
   }

   DRETURN(context);
}

/****** spool/flatfile/spool_flatfile_default_startup_func() **************
*  NAME
*     spool_flatfile_default_startup_func() -- setup the spool directory
*
*  SYNOPSIS
*     bool
*     spool_flatfile_default_startup_func(lList **answer_list, 
*                                         const lListElem *rule, bool check) 
*
*  FUNCTION
*     Checks the existence of the spool directory.
*     If the current working directory is not yet the spool direcotry,
*     changes the current working directory.
*     If the subdirectories for the different object types do not yet
*     exist, they are created.
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
*     spool/flatfile/--Flatfile-Spooling
*     spool/spool_startup_context()
*******************************************************************************/
bool
spool_classic_default_startup_func(lList **answer_list, 
                                    const lListElem *rule, bool check)
{
   bool ret = true;
   const char *url;

   DENTER(TOP_LAYER, "spool_classic_default_startup_func");

   /* check spool directory */
   url = lGetString(rule, SPR_url);
   if (!sge_is_directory(url)) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_SPOOL_SPOOLDIRDOESNOTEXIST_S, url);
      ret = false;
   } else {
      if (sge_chdir(url) != 0) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_ERRORCHANGINGCWD_SS, url, 
                                 strerror(errno));
        ret = false;
      } else {
         /* create spool sub directories */
         sge_mkdir2(url, JOB_DIR,  0755, true);
         sge_mkdir2(url, ZOMBIE_DIR, 0755, true);
         sge_mkdir2(url, CQUEUE_DIR,  0755, true);
         sge_mkdir2(url, QINSTANCES_DIR,  0755, true);
         sge_mkdir2(url, EXECHOST_DIR, 0755, true);
         sge_mkdir2(url, SUBMITHOST_DIR, 0755, true);
         sge_mkdir2(url, ADMINHOST_DIR, 0755, true);
         sge_mkdir2(url, CENTRY_DIR, 0755, true);
         sge_mkdir2(url, EXEC_DIR, 0755, true);
         sge_mkdir2(url, PE_DIR, 0755, true);
         sge_mkdir2(url, CKPTOBJ_DIR, 0755, true);
         sge_mkdir2(url, USERSET_DIR, 0755, true);
         sge_mkdir2(url, CAL_DIR, 0755, true);
         sge_mkdir2(url, HGROUP_DIR, 0755, true);
   #ifndef  __SGE_NO_USERMAPPING__
         sge_mkdir2(url, UME_DIR, 0755, true);
   #endif
         sge_mkdir2(url, USER_DIR, 0755, true);
         sge_mkdir2(url, PROJECT_DIR, 0755, true);
         sge_mkdir2(url, RESOURCEQUOTAS_DIR, 0755, true);
         sge_mkdir2(url, AR_DIR, 0755, true);
      }
   }

   DRETURN(ret);
}

/****** spool/flatfile/spool_flatfile_common_startup_func() ***************
*  NAME
*     spool_flatfile_common_startup_func() -- setup the common directory
*
*  SYNOPSIS
*     bool
*     spool_flatfile_common_startup_func(lList **answer_list, 
*                                        const lListElem *rule, bool check) 
*
*  FUNCTION
*     Checks the existence of the common directory.
*     If the subdirectory for the local configurtations does not yet exist,
*     it is created.
*
*  INPUTS
*     lList **answer_list - to return error messages
*     const lListElem *rule - rule containing data like the path to the common 
*                             directory
*     bool check            - check the spooling database
*
*  RESULT
*     bool - true, on success, else false
*
*  NOTES
*     This function should not be called directly, it is called by the
*     spooling framework.
*
*  SEE ALSO
*     spool/flatfile/--Flatfile-Spooling
*     spool/spool_startup_context()
*******************************************************************************/
bool
spool_classic_common_startup_func(lList **answer_list, 
                                   const lListElem *rule, bool check)
{
   bool ret = true;
   const char *url;

   DENTER(TOP_LAYER, "spool_classic_common_startup_func");

   /* check common directory */
   url = lGetString(rule, SPR_url);
   if (!sge_is_directory(url)) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_SPOOL_COMMONDIRDOESNOTEXIST_S, url);
      ret = false;
   } else {
      /* create directory for local configurations */
      sge_mkdir2(url, LOCAL_CONF_DIR, 0755, true);
   }

   DRETURN(ret);
}

static bool read_validate_object(lList **answer_list, 
                   const lListElem *type, const lListElem *rule, 
                   const char *key, int key_nm, 
                   sge_object_type object_type, lList **master_list)
{
   bool ret = true;
   lListElem *ep;

   DENTER(TOP_LAYER, "read_validate_object");

   DPRINTF(("reading "SFN" "SFQ"\n", object_type_get_name(object_type), key));

   ep = spool_classic_default_read_func(answer_list, type, rule, key, 
                                         object_type);
   if (ep == NULL) {
      ret = false;
   } else {
      spooling_validate_func validate_func;

      /* set key from filename */
      if (key_nm != NoName) {
         object_parse_field_from_string(ep, NULL, key_nm, key);
      }

      /* validate object */
      validate_func = (spooling_validate_func)lGetRef(rule, SPR_validate_func);
      if (validate_func != NULL) {
         if (!validate_func(answer_list, type, rule, ep, object_type)) {
            lFreeElem(&ep);
            ret = false;
         }
      }

      /* object read correctly and validate succeeded */
      if (ep != NULL) {
         lAppendElem(*master_list, ep);
      }
   }

   DRETURN(ret);
}

/****** spool/flatfile/spool_flatfile_default_list_func() *****************
*  NAME
*     spool_flatfile_default_list_func() -- read lists through flatfile spooling
*
*  SYNOPSIS
*     bool
*     spool_flatfile_default_list_func(lList **answer_list, 
*                                      const lListElem *type, 
*                                      const lListElem *rule, 
*                                      lList **list, 
*                                      const sge_object_type object_type) 
*
*  FUNCTION
*     Depending on the object type given, calls the appropriate functions
*     reading the correspondent list of objects using the old spooling
*     functions.
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
*     spool/flatfile/--Flatfile-Spooling
*     spool/spool_read_list()
*******************************************************************************/
bool
spool_classic_default_list_func(lList **answer_list, 
                                 const lListElem *type, 
                                 const lListElem *rule,
                                 lList **list, 
                                 const sge_object_type object_type)
{
   const lDescr *descr;

   const char *filename  = NULL;
   const char *directory = NULL;
   const char *url = NULL;
   int key_nm = NoName;

   bool ret = true;

   DENTER(TOP_LAYER, "spool_classic_default_list_func");

   if (!list) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                     ANSWER_QUALITY_WARNING, 
                     "Cannot read in list because target list is missing\n"); 
      ret = false;
   } else {
      url = lGetString(rule, SPR_url);
      descr = object_type_get_descr(object_type);

      if (*list == NULL && descr != NULL) {
         *list = lCreateList("master list", descr);
      }

      switch(object_type) {
         case SGE_TYPE_ADMINHOST:
            directory = ADMINHOST_DIR;
            break;
         case SGE_TYPE_CALENDAR:
            directory = CAL_DIR;
            break;
         case SGE_TYPE_CKPT:
            directory = CKPTOBJ_DIR;
            break;
         case SGE_TYPE_CONFIG:
            key_nm    = CONF_name;
            filename  = "global";
            directory = LOCAL_CONF_DIR;
            break;
         case SGE_TYPE_EXECHOST:
            directory = EXECHOST_DIR;
            break;
         case SGE_TYPE_MANAGER:
            ret = read_manop(SGE_UM_LIST);
            break;
         case SGE_TYPE_OPERATOR:
            ret = read_manop(SGE_UO_LIST);
            break;
         case SGE_TYPE_PE:
            directory = PE_DIR;
            break;
         case SGE_TYPE_QINSTANCE:
            directory = QINSTANCES_DIR;
            /* JG: TODO: we'll have to quicksort the queue list, see
             * function queue_list_add_queue
             */
            break;
         case SGE_TYPE_CQUEUE:
            directory = CQUEUE_DIR;
            /* JG: TODO: we'll have to quicksort the queue list, see
             * function cqueue_list_add_cqueue
             */
            break;
         case SGE_TYPE_SUBMITHOST:
            directory = SUBMITHOST_DIR;
            break;
         case SGE_TYPE_USERSET:
            directory = USERSET_DIR;
            break;
         case SGE_TYPE_HGROUP:
            directory = HGROUP_DIR;
            break;
#ifndef __SGE_NO_USERMAPPING__
         case SGE_TYPE_CUSER:
            directory = UME_DIR;
            break;
#endif
         case SGE_TYPE_PROJECT:
            directory = PROJECT_DIR;
            break;
         case SGE_TYPE_USER:
            directory = USER_DIR;
            break;
         case SGE_TYPE_SHARETREE:
            filename = SHARETREE_FILE;
            break;
         case SGE_TYPE_SCHEDD_CONF:
            filename = SCHED_CONF_FILE;
            break;
         case SGE_TYPE_RQS:
             directory = RESOURCEQUOTAS_DIR;
             break;
         case SGE_TYPE_AR:
             directory = AR_DIR;
             break;
         case SGE_TYPE_CENTRY:
             directory = CENTRY_DIR;
             break;
         case SGE_TYPE_JOB:
            job_list_read_from_disk(list, "Master_Job_List", 0,
                                    SPOOL_DEFAULT, NULL);
            job_list_read_from_disk(list, "Master_Zombie_List", 0,
                                    SPOOL_HANDLE_AS_ZOMBIE, NULL);
            break;
         default:
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_WARNING, 
                                    MSG_SPOOL_SPOOLINGOFXNOTSUPPORTED_S, 
                                    object_type_get_name(object_type));
            ret = false;
            break;
      }

      /* if all necessary data has been initialized */
      if (url != NULL && list != NULL && descr != NULL) {
         /* single file to parse (SHARETREE, global config, schedd config */
         if (filename != NULL) {
            ret = read_validate_object(answer_list, type, rule, filename, key_nm, 
                                     object_type, list);
         }

         /* if we have a directory (= multiple files) to parse */ 
         if (ret && directory != NULL) { 
            lList *direntries;
            lListElem *direntry;
            char abs_dir_buf[SGE_PATH_MAX];
            dstring abs_dir_dstring;
            const char *abs_dir;

            sge_dstring_init(&abs_dir_dstring, abs_dir_buf, SGE_PATH_MAX);
            abs_dir = sge_dstring_sprintf(&abs_dir_dstring, "%s/%s", url, 
                                          directory);

            direntries = sge_get_dirents(abs_dir);

            for_each(direntry, direntries) {
               const char *key = lGetString(direntry, ST_name);

               if (key[0] != '.') {
                  ret &= read_validate_object(answer_list, type, rule, key, key_nm, 
                                           object_type, list);
               }
            }
            lFreeList(&direntries);
         } 
      }
      
      switch(object_type) {
         case SGE_TYPE_CQUEUE:
            {
               lListElem *queue;
               lListElem *context = spool_get_default_context();
               lListElem *type = spool_context_search_type(context, SGE_TYPE_QINSTANCE);
               lListElem *rule = spool_type_search_default_rule(type);
               const char *url = lGetString(rule, SPR_url);

               dstring key = DSTRING_INIT;
               dstring dir = DSTRING_INIT;

               for_each(queue, *list) {
                  lList *direntries;
                  lListElem *direntry;
                  lList *qinstance_list = lCreateList("", QU_Type);

                  sge_dstring_sprintf(&dir, "%s/%s/%s", url, QINSTANCES_DIR, lGetString(queue, CQ_name));
                  direntries = sge_get_dirents(sge_dstring_get_string(&dir));
                  for_each(direntry, direntries) {
                     const char *directory = lGetString(direntry, ST_name);
                     if (directory[0] != '.') {
                        sge_dstring_sprintf(&key, "%s/%s", lGetString(queue, CQ_name), directory);
                        read_validate_object(answer_list, type, rule, sge_dstring_get_string(&key), NoName, 
                                           SGE_TYPE_QINSTANCE, &qinstance_list);
                     }
                  }
                  lFreeList(&direntries);

                  lSetList(queue, CQ_qinstances, qinstance_list);
               }
               sge_dstring_free(&dir);
               sge_dstring_free(&key);
            }
            break;
         default:
            break;
      }


#ifdef DEBUG_FLATFILE
      if (list != NULL && *list != NULL) {
         lWriteListTo(*list, stderr);
      }
#endif

      /* validate complete list */
      if (ret) {
         spooling_validate_list_func validate_list = 
            (spooling_validate_list_func)lGetRef(rule, SPR_validate_list_func);
         
         ret = validate_list(answer_list, type, rule, object_type);
      }
   }
   DRETURN(ret);
}

/****** spool/flatfile/spool_flatfile_default_read_func() *****************
*  NAME
*     spool_flatfile_default_read_func() -- read objects using flatfile spooling
*
*  SYNOPSIS
*     lListElem* 
*     spool_flatfile_default_read_func(lList **answer_list, 
*                                      const lListElem *type, 
*                                      const lListElem *rule, 
*                                      const char *key, 
*                                      const sge_object_type object_type) 
*
*  FUNCTION
*     Reads an individual object by calling the appropriate flatfile spooling 
*     function.
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
*     spool/flatfile/--Flatfile-Spooling
*     spool/spool_read_object()
*******************************************************************************/
lListElem *
spool_classic_default_read_func(lList **answer_list, 
                                 const lListElem *type, 
                                 const lListElem *rule,
                                 const char *key, 
                                 const sge_object_type object_type)
{
   const char *url = NULL;
   const char *directory = NULL;
   const char *filename = NULL;
   const lDescr *descr = NULL;
   flatfile_info *rule_clientdata;
   flatfile_info *field_info;
   lListElem *ep = NULL;
   bool parse_values = true;

   DENTER(TOP_LAYER, "spool_classic_default_read_func");

   rule_clientdata = lGetRef(rule, SPR_clientdata);
   field_info = &(rule_clientdata[object_type]);
   url = lGetString(rule, SPR_url);
   descr = object_type_get_descr(object_type);

   /* prepare filenames */
   switch(object_type) {
      case SGE_TYPE_ADMINHOST:
         directory = ADMINHOST_DIR;
         filename  = key;
         break;
      case SGE_TYPE_CALENDAR:
         directory = CAL_DIR;
         filename = key;
         break;
      case SGE_TYPE_CKPT:
         directory = CKPTOBJ_DIR;
         filename = key;
         break;
      case SGE_TYPE_CONFIG:
         parse_values = false;
         if (sge_hostcmp(key, "global") == 0) {
            directory = ".";
            filename  = CONF_FILE;
         } else {
            directory = LOCAL_CONF_DIR;
            filename  = key;
         }
         break;
      case SGE_TYPE_EXECHOST:
         directory = EXECHOST_DIR;
         filename  = key;
         break;
      case SGE_TYPE_MANAGER:
      case SGE_TYPE_OPERATOR:
         directory = NULL;
         filename  = NULL;
         break;
      case SGE_TYPE_PE:
         directory = PE_DIR;
         filename = key;
         break;
      case SGE_TYPE_CQUEUE:
         directory = CQUEUE_DIR;
         filename  = key;
         break;
      case SGE_TYPE_QINSTANCE:
         directory = QINSTANCES_DIR;
         filename  = key;
         break;
      case SGE_TYPE_SUBMITHOST:
         directory = SUBMITHOST_DIR;
         filename  = key;
         break;
      case SGE_TYPE_USERSET:
         directory = USERSET_DIR;
         filename  = key;
         break;
      case SGE_TYPE_HGROUP:
         directory = HGROUP_DIR;
         filename  = key;
         break;
#ifndef __SGE_NO_USERMAPPING__
      case SGE_TYPE_CUSER:
         directory = UME_DIR;
         filename  = key;
         break;
#endif
      case SGE_TYPE_PROJECT:
         directory = PROJECT_DIR;
         filename  = key;
         break;
      case SGE_TYPE_USER:
         directory = USER_DIR;
         filename  = key;
         break;
      case SGE_TYPE_SHARETREE:
         directory = ".";
         filename  = SHARETREE_FILE;
         break;
      case SGE_TYPE_SCHEDD_CONF:
         directory = ".";
         filename  = SCHED_CONF_FILE;
         break;
      case SGE_TYPE_RQS:
         directory = RESOURCEQUOTAS_DIR;
         filename  = key;
         break;
      case SGE_TYPE_AR:
         directory = AR_DIR;
         filename  = key;
         break;
      case SGE_TYPE_CENTRY:
         directory = CENTRY_DIR;
         filename  = key;
         break;
      case SGE_TYPE_JOBSCRIPT:
         {
            const char *exec_file = NULL;         
            char *dup=strdup(key);
            jobscript_parse_key(dup, &exec_file);
            if (exec_file != NULL ) {
               int len;
               char *str = sge_file2string(exec_file, &len);
               if (str != NULL) {
                  ep = lCreateElem(STU_Type);
                  lXchgString(ep, STU_name, &str);
               }
            }
            sge_free(&dup);
         }
         break;      
      case SGE_TYPE_JOB:
      default:
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_WARNING, 
                                 MSG_SPOOL_SPOOLINGOFXNOTSUPPORTED_S, 
                                 object_type_get_name(object_type));
         break;
   }

   /* spool, if possible */
   if (url != NULL && directory != NULL && filename != NULL && descr != NULL) {
      dstring filepath_dstring = DSTRING_INIT;
      const char *filepath;

      filepath = sge_dstring_sprintf(&filepath_dstring, "%s/%s/%s", 
                                     url, directory, filename);

      /* spool */
      ep = spool_flatfile_read_object(answer_list, descr, NULL,
                                      field_info->fields, NULL, 
                                      parse_values, field_info->instr, SP_FORM_ASCII, 
                                      NULL, filepath);

      sge_dstring_free(&filepath_dstring);
   } else {
      DPRINTF(("error: one of the required parameters is NULL\n"));
   }

   DRETURN(ep);
}

/****** spool/flatfile/spool_flatfile_default_write_func() ****************
*  NAME
*     spool_flatfile_default_write_func() -- write object with flatfile spooling
*
*  SYNOPSIS
*     bool
*     spool_flatfile_default_write_func(lList **answer_list, 
*                                       const lListElem *type, 
*                                       const lListElem *rule, 
*                                       const lListElem *object, 
*                                       const char *key, 
*                                       const sge_object_type object_type) 
*
*  FUNCTION
*     Writes an object through the appropriate flatfile spooling functions.
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
*     spool/flatfile/--Flatfile-Spooling
*     spool/spool_delete_object()
*******************************************************************************/
bool
spool_classic_default_write_func(lList **answer_list, 
                                  const lListElem *type, 
                                  const lListElem *rule, 
                                  const lListElem *object, 
                                  const char *key, 
                                  const sge_object_type object_type)
{
   const char *url = NULL;
   const char *directory = NULL;
   const char *filename = NULL;
   const char *filepath = NULL;
   flatfile_info *rule_clientdata;
   flatfile_info *field_info;
   dstring tmp = DSTRING_INIT;
   bool ret = true;

   DENTER(TOP_LAYER, "spool_flatfile_default_write_func");

   rule_clientdata = lGetRef(rule, SPR_clientdata);
   field_info = &(rule_clientdata[object_type]);
   url = lGetString(rule, SPR_url);

   /* prepare filenames */
   switch(object_type) {
      case SGE_TYPE_ADMINHOST:
         directory = ADMINHOST_DIR;
         filename  = key;
         break;
      case SGE_TYPE_CALENDAR:
         directory = CAL_DIR;
         filename = key;
         break;
      case SGE_TYPE_CKPT:
         directory = CKPTOBJ_DIR;
         filename = key;
         break;
      case SGE_TYPE_CONFIG:
         if (sge_hostcmp(key, "global") == 0) {
            directory = ".";
            filename  = CONF_FILE;
         } else {
            directory = LOCAL_CONF_DIR;
            filename  = key;
         }
         break;
      case SGE_TYPE_EXECHOST:
         directory = EXECHOST_DIR;
         filename  = key;
         break;
      case SGE_TYPE_MANAGER:
         ret = write_manop(1, SGE_UM_LIST);
         break;
      case SGE_TYPE_OPERATOR:
         ret = write_manop(1, SGE_UO_LIST);
         break;
      case SGE_TYPE_PE:
         directory = PE_DIR;
         filename = key;
         break;
      case SGE_TYPE_CQUEUE:
         {
            dstring qi_dir = DSTRING_INIT;
            sge_dstring_sprintf(&qi_dir, "%s/%s", QINSTANCES_DIR, key);
            sge_mkdir(sge_dstring_get_string(&qi_dir), 0755, false, false);
            sge_dstring_free(&qi_dir);

         }
         directory = CQUEUE_DIR;
         filename  = key;
         break;
      case SGE_TYPE_QINSTANCE:
         sge_free(&directory);
         directory = sge_dstring_sprintf(&tmp, "%s/%s", QINSTANCES_DIR, lGetString(object, QU_qname));
         filename = lGetHost(object, QU_qhostname);
         break;
      case SGE_TYPE_SUBMITHOST:
         directory = SUBMITHOST_DIR;
         filename  = key;
         break;
      case SGE_TYPE_USERSET:
         directory = USERSET_DIR;
         filename  = key;
         break;
      case SGE_TYPE_HGROUP:
         directory = HGROUP_DIR;
         filename  = key;
         break;
#ifndef __SGE_NO_USERMAPPING__
      case SGE_TYPE_CUSER:
         directory = UME_DIR;
         filename  = key;
         break;
#endif
      case SGE_TYPE_PROJECT:
         directory = PROJECT_DIR;
         filename  = key;
         break;
      case SGE_TYPE_USER:
         directory = USER_DIR;
         filename  = key;
         break;
      case SGE_TYPE_SHARETREE:
         directory = ".";
         filename  = SHARETREE_FILE;
         break;
      case SGE_TYPE_SCHEDD_CONF:
         directory = ".";
         filename  = SCHED_CONF_FILE;
         break;
      case SGE_TYPE_JOB:
      case SGE_TYPE_JATASK:
      case SGE_TYPE_PETASK:
         {
            u_long32 job_id, ja_task_id;
            char *pe_task_id;
            char *dup = strdup(key);
            bool only_job;
            sge_spool_flags_t flags = SPOOL_DEFAULT;
            lListElem *job;
            
            job_parse_key(dup, &job_id, &ja_task_id, &pe_task_id, &only_job);

            DPRINTF(("spooling job "sge_u32"."sge_u32" %s\n", job_id, ja_task_id, 
                     pe_task_id != NULL ? pe_task_id : "<null>"));

            if (object_type == SGE_TYPE_JOB) {
               job = (lListElem *)object;

               /* we only want to spool the job object */
               if (only_job) {
                  flags |= SPOOL_IGNORE_TASK_INSTANCES;
               }
            } else {
               /* job_write_spool_file takes a job, even if we only want
                * to spool a ja_task or pe_task
                */
               job = job_list_locate(*(object_type_get_master_list(SGE_TYPE_JOB)), job_id);

               /* additional flags for job_write_spool_file
                * to avoid spooling too many files
                */
               if (object_type == SGE_TYPE_PETASK) {
                  flags |= SPOOL_ONLY_PETASK;
               } else if (object_type == SGE_TYPE_JATASK) {
                  flags |= SPOOL_ONLY_JATASK;
               }
            }

            if (job_write_spool_file((lListElem *)job, ja_task_id, pe_task_id, flags) != 0) {
               ret = false;
            }

            sge_free(&dup);
         }
         break;
      case SGE_TYPE_JOBSCRIPT:
         ret = sge_string2file(lGetString(object, JB_script_ptr), 
                               lGetUlong(object, JB_script_size),
                               lGetString(object, JB_exec_file)) ? false : true;
         break;           
      case SGE_TYPE_RQS:
         directory = RESOURCEQUOTAS_DIR;
         filename  = key;
         break;
      case SGE_TYPE_AR:
         directory = AR_DIR;
         filename  = key;
         break;
      case SGE_TYPE_CENTRY:
         directory = CENTRY_DIR;
         filename  = key;
         break;
      default:
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_WARNING, 
                                 MSG_SPOOL_SPOOLINGOFXNOTSUPPORTED_S, 
                                 object_type_get_name(object_type));
         ret = false;
         break;
   }

   /* spool, if possible using default spooling behaviour.
    * job are spooled in corresponding case branch 
    */
   if (url != NULL && directory != NULL && filename != NULL) {
      const char *tmpfilepath = NULL;
      dstring filepath_buffer = DSTRING_INIT;

      /* first write to a temporary file; for jobs it is already initialized */
      tmpfilepath = sge_dstring_sprintf(&filepath_buffer, "%s/%s/.%s", 
                                        url, directory, filename);

      /* spool */
      tmpfilepath = spool_flatfile_write_object(answer_list, object, false,
                                                field_info->fields, 
                                                field_info->instr, 
                                                SP_DEST_SPOOL, SP_FORM_ASCII,
                                                tmpfilepath, true);

      if (tmpfilepath == NULL) {
         ret = false;
      } else {
         /* spooling was ok: rename temporary to target file */
         filepath = sge_dstring_sprintf(&filepath_buffer, "%s/%s/%s", 
                                        url, directory, filename);

         if (rename(tmpfilepath, filepath) == -1) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_ERRORRENAMING_SSS, 
                                    tmpfilepath, filepath, strerror(errno));
            ret = false;
         }
      }

      sge_free(&tmpfilepath);
      sge_dstring_free(&filepath_buffer);
   }
   sge_dstring_free(&tmp);
   
   DRETURN(ret);
}

/****** spool/flatfile/spool_flatfile_default_delete_func() ***************
*  NAME
*     spool_flatfile_default_delete_func() -- delete object in flatfile spooling
*
*  SYNOPSIS
*     bool
*     spool_flatfile_default_delete_func(lList **answer_list, 
*                                        const lListElem *type, 
*                                        const lListElem *rule, 
*                                        const char *key, 
*                                        const sge_object_type object_type) 
*
*  FUNCTION
*     Deletes an object in the flatfile spooling.
*     In most cases, the correspondent spool file is deleted, in some cases
*     (e.g. jobs), a special remove function is called.
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
*     spool/flatfile/--Flatfile-Spooling
*     spool/spool_delete_object()
*******************************************************************************/
bool
spool_classic_default_delete_func(lList **answer_list, 
                                   const lListElem *type, 
                                   const lListElem *rule,
                                   const char *key, 
                                   const sge_object_type object_type)
{
   bool ret = true;

   DENTER(TOP_LAYER, "spool_classic_default_delete_func");

   switch(object_type) {
      case SGE_TYPE_ADMINHOST:
         ret = sge_unlink(ADMINHOST_DIR, key);
         break;
      case SGE_TYPE_CALENDAR:
         ret = sge_unlink(CAL_DIR, key);
         break;
      case SGE_TYPE_CKPT:
         ret = sge_unlink(CKPTOBJ_DIR, key);
         break;
      case SGE_TYPE_CENTRY:
         ret = sge_unlink(CENTRY_DIR, key);
         break;
      case SGE_TYPE_CONFIG:
         if(sge_hostcmp(key, "global") == 0) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    "%s", MSG_SPOOL_GLOBALCONFIGNOTDELETED);
            ret = false;
         } else {
            dstring dir_name_dstring = DSTRING_INIT;
            const char *dir_name;

            dir_name = sge_dstring_sprintf(&dir_name_dstring, "%s/%s",
                                           lGetString(rule, SPR_url), 
                                           LOCAL_CONF_DIR);
            ret = sge_unlink(dir_name, key);
            sge_dstring_free(&dir_name_dstring);
         }
         break;
      case SGE_TYPE_EXECHOST:
         ret = sge_unlink(EXECHOST_DIR, key);
         break;
      case SGE_TYPE_JOB:
      case SGE_TYPE_JATASK:   
      case SGE_TYPE_PETASK:   
         {
            u_long32 job_id, ja_task_id;
            char *pe_task_id;
            char *dup = strdup(key);
            bool only_job;
            
            job_parse_key(dup, &job_id, &ja_task_id, &pe_task_id, &only_job);
   
            DPRINTF(("spooling job "sge_u32"."sge_u32" %s\n", job_id, ja_task_id, 
                     pe_task_id != NULL ? pe_task_id : "<null>"));
            if (job_remove_spool_file(job_id, ja_task_id, pe_task_id, 
                                      SPOOL_DEFAULT) != 0) {
               ret = false;
            }
            sge_free(&dup);
         }
         break;
      case SGE_TYPE_JOBSCRIPT:
        {
            const char *exec_file;  
            char *dup = strdup(key);
            jobscript_parse_key(dup, &exec_file);
            ret = (unlink(exec_file) != 0) ? false: true;
            sge_free(&dup);
         }
         break;
      case SGE_TYPE_MANAGER:
         write_manop(1, SGE_UM_LIST);
         break;
      case SGE_TYPE_OPERATOR:
         write_manop(1, SGE_UO_LIST);
         break;
      case SGE_TYPE_SHARETREE:
         ret = sge_unlink(NULL, SHARETREE_FILE);
         break;
      case SGE_TYPE_PE:
         ret = sge_unlink(PE_DIR, key);
         break;
      case SGE_TYPE_PROJECT:
         ret = sge_unlink(PROJECT_DIR, key);
         break;
      case SGE_TYPE_CQUEUE:
         ret = sge_unlink(CQUEUE_DIR, key);
         break;
      case SGE_TYPE_QINSTANCE:
         ret = sge_unlink(QINSTANCES_DIR, key);
         break;
      case SGE_TYPE_SCHEDD_CONF:
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 "%s", MSG_SPOOL_SCHEDDCONFIGNOTDELETED);
         ret = false;
         break;
      case SGE_TYPE_SUBMITHOST:
         ret = sge_unlink(SUBMITHOST_DIR, key);
         break;
      case SGE_TYPE_USER:
         ret = sge_unlink(USER_DIR, key);
         break;
      case SGE_TYPE_USERSET:
         ret = sge_unlink(USERSET_DIR, key);
         break;
#ifndef __SGE_NO_USERMAPPING__
      case SGE_TYPE_CUSER:
         ret = sge_unlink(UME_DIR, key);
         break;
#endif
      case SGE_TYPE_HGROUP:
         ret = sge_unlink(HGROUP_DIR, key);
         break;
      case SGE_TYPE_RQS:
         ret = sge_unlink(RESOURCEQUOTAS_DIR, key);
         break;
      case SGE_TYPE_AR:
         ret = sge_unlink(AR_DIR, key);
         break;
      default:
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_WARNING, 
                                 MSG_SPOOL_SPOOLINGOFXNOTSUPPORTED_S, 
                                 object_type_get_name(object_type));
         ret = false;
         break;
   }

   DRETURN(ret);
}

static bool write_manop(int spool, int target) {
   FILE *fp;
   lListElem *ep;
   lList *lp;
   char filename[255], real_filename[255];
   dstring ds = DSTRING_INIT;
   int key = NoName;

   DENTER(TOP_LAYER, "write_manop");

   switch (target) {
   case SGE_UM_LIST:
      lp = *object_type_get_master_list(SGE_TYPE_MANAGER);      
      strcpy(filename, ".");
      strcat(filename, MAN_FILE);
      strcpy(real_filename, MAN_FILE);
      key = UM_name;
      break;
      
   case SGE_UO_LIST:
      lp = *object_type_get_master_list(SGE_TYPE_OPERATOR);      
      strcpy(filename, ".");
      strcat(filename, OP_FILE);
      strcpy(real_filename, OP_FILE);
      key = UO_name;
      break;

   default:
      DRETURN(false);
   }

   fp = fopen(filename, "w");
   if (!fp) {
      ERROR((SGE_EVENT, MSG_ERRORWRITINGFILE_SS, filename, strerror(errno)));
      DRETURN(false);
   }

   if (spool && sge_spoolmsg_write(fp, COMMENT_CHAR,
             feature_get_product_name(FS_VERSION, &ds)) < 0) {
      sge_dstring_free(&ds);
      goto FPRINTF_ERROR;
   }
   sge_dstring_free(&ds);

   for_each(ep, lp) {
      FPRINTF((fp, "%s\n", lGetString(ep, key)));
   }

   FCLOSE(fp);

   if (rename(filename, real_filename) == -1) {
      DRETURN(false);
   } else {
      strcpy(filename, real_filename);
   }     

   DRETURN(true);

FPRINTF_ERROR:
FCLOSE_ERROR:
   DRETURN(false);  
}

static bool read_manop(int target) {
   lList **lpp;
   char filename[SGE_PATH_MAX];
   char str[256];
   FILE *fp;
   SGE_STRUCT_STAT st;
   int key = NoName;
   lDescr *descr = NULL;

   DENTER(TOP_LAYER, "read_manop");

   switch (target) {
   case SGE_UM_LIST:
      lpp = object_type_get_master_list(SGE_TYPE_MANAGER);      
      strcpy(filename, MAN_FILE);
      key = UM_name;
      descr = UM_Type;
      break;
      
   case SGE_UO_LIST:
      lpp = object_type_get_master_list(SGE_TYPE_OPERATOR);      
      strcpy(filename, OP_FILE);
      key = UO_name;
      descr = UO_Type;
      break;

   default:
      DRETURN(false);
   }

   /* if no such file exists. ok return without error */
   if (SGE_STAT(filename, &st) && errno==ENOENT) {
      DRETURN(true);
   }

   fp = fopen(filename, "r");
   if (!fp) {
      ERROR((SGE_EVENT, MSG_FILE_ERROROPENINGX_S, filename));
      DRETURN(false);
   }
   
   lFreeList(lpp);
   *lpp = lCreateList("man/op list", descr);

   while (fscanf(fp, "%[^\n]\n", str) == 1) {
      if (str[0] != COMMENT_CHAR) {
         lAddElemStr(lpp, key, str, descr);
      }
   }

   FCLOSE(fp);

   DRETURN(true);
FCLOSE_ERROR:
   ERROR((SGE_EVENT, MSG_FILE_ERRORCLOSEINGX_S, filename));
   DRETURN(false);
}
