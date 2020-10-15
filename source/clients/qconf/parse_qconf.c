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
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <limits.h>
#include <ctype.h>
#include <fnmatch.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_stdio.h"
#include "uti/sge_unistd.h"
#include "uti/sge_dstring.h"
#include "uti/sge_string.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_spool.h"
#include "uti/sge_io.h"
#include "uti/sge_edit.h"

#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_event.h"
#include "sgeobj/sge_id.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_config.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_sharetree.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_calendar.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_ckpt.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_resource_quota.h"
#include "sgeobj/sge_href.h"
#include "sgeobj/sge_qref.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_attr.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_utility.h"

#include "spool/flatfile/sge_flatfile.h"
#include "spool/flatfile/sge_flatfile_obj.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi2.h"

#include "comm/commlib.h"

#include "sge.h"
#include "sge_options.h"
#include "usage.h"
#include "sge_client_access.h"
#include "parse_qconf.h"
#include "sge_hgroup_qconf.h"
#include "sge_centry_qconf.h"
#include "sge_cqueue_qconf.h"
#include "sge_resource_quota_qconf.h"
#include "msg_common.h"
#include "msg_clients_common.h"
#include "msg_qconf.h"

#ifndef __SGE_NO_USERMAPPING__
#define SGE_OBJ_USER_MAPPING           "usermapping"
#endif

static int sge_next_is_an_opt(char **ptr);
static int sge_error_and_exit(sge_gdi_ctx_class_t *ctx, const char *ptr);

/* ------------------------------------------------------------- */
static bool show_object_list(sge_gdi_ctx_class_t *ctx, u_long32, lDescr *, int, char *);
static int show_eventclients(sge_gdi_ctx_class_t *ctx);

/* ------------------------------------------------------------- */
static void parse_name_list_to_cull(char *name, lList **lpp, lDescr *dp, int nm, char *s);
static bool add_host_of_type(sge_gdi_ctx_class_t *ctx, lList *arglp, u_long32 target);
static bool del_host_of_type(sge_gdi_ctx_class_t *ctx, lList *arglp, u_long32 target);
static int print_acl(sge_gdi_ctx_class_t *ctx, lList *arglp);
static int qconf_modify_attribute(sge_gdi_ctx_class_t *ctx, lList **alpp, int from_file, char ***spp, lListElem **epp, int sub_command, struct object_info_entry *info_entry); 
static lListElem *edit_exechost(sge_gdi_ctx_class_t *ctx, lListElem *ep, uid_t uid, gid_t gid);
static int edit_usersets(sge_gdi_ctx_class_t *ctx, lList *arglp);

/************************************************************************/
static int print_config(sge_gdi_ctx_class_t *ctx, const char *config_name);
static int delete_config(sge_gdi_ctx_class_t *ctx, const char *config_name);
static int add_modify_config(sge_gdi_ctx_class_t *ctx, const char *config_name, const char *filename, u_long32 flags);
static lList* edit_sched_conf(sge_gdi_ctx_class_t *ctx, lList *confl, uid_t uid, gid_t gid);
static lListElem* edit_project(sge_gdi_ctx_class_t *ctx, lListElem *ep, uid_t uid, gid_t gid);
static lListElem* edit_user(sge_gdi_ctx_class_t *ctx, lListElem *ep, uid_t uid, gid_t gid);
static lListElem *edit_sharetree(sge_gdi_ctx_class_t *ctx, lListElem *ep, uid_t uid, gid_t gid);

/************************************************************************/
static int qconf_is_manager(sge_gdi_ctx_class_t *ctx, const char *user);
static int qconf_is_adminhost(sge_gdi_ctx_class_t *ctx, const char *host);
/************************************************************************/

static const char *write_attr_tmp_file(const char *name, const char *value, 
                                       const char *delimiter, dstring *error_message);

/***************************************************************************/
static char **sge_parser_get_next(sge_gdi_ctx_class_t *ctx, char **arg) 
{
   DENTER(TOP_LAYER, "sge_parser_get_next");
   if (!*(arg+1)) {
      ERROR((SGE_EVENT, MSG_PARSE_NOOPTIONARGPROVIDEDTOX_S , *arg));
      sge_usage(QCONF, stderr);
      SGE_EXIT((void **)&ctx, 1);
   }

   DRETURN(++arg);
}

/*------------------------------------------------------------*/
int sge_parse_qconf(sge_gdi_ctx_class_t *ctx, char *argv[])
{
   int status;
   char *cp = NULL;
   char **spp = NULL;
   int opt;
   int sge_parse_return = 0;
   lEnumeration *what = NULL;
   lCondition *where = NULL;
   lList *lp=NULL, *arglp=NULL, *alp=NULL, *newlp=NULL;
   lListElem *hep=NULL, *ep=NULL, *argep=NULL, *aep=NULL, *newep = NULL;
   const char *host = NULL;
   char *filename;
   const char *filename_stdout;
   int fields_out[MAX_NUM_FIELDS];
   int missing_field = NoName;
   const char* qualified_hostname = ctx->get_qualified_hostname(ctx);
   const char* username = ctx->get_username(ctx);
   const char* default_cell = ctx->get_default_cell(ctx);
   u_long32 prog_number = ctx->get_who(ctx);
   uid_t uid = ctx->get_uid(ctx);
   gid_t gid = ctx->get_gid(ctx);

   DENTER(TOP_LAYER, "sge_parse_qconf");

   /* If no arguments were given, output the help message on stderr. */
   if (*argv == NULL) {
      sge_usage(QCONF, stderr);
      fprintf(stderr, "%s\n", MSG_PARSE_NOOPTIONARGUMENT);
      DRETURN(1);
   }
   /* This is duplicated below (catching -help as non-initial arg).
      Do it here for common help case before possibly bailing out if
      we're not on an admin host.  */
   if (strcmp("-help", *argv) == 0) {
      sge_usage(QCONF, stdout);
      DRETURN(0);
   }

   /* Any actions require being on an admin host.  Check here rather
      than for each case below.  */
   qconf_is_adminhost(ctx, qualified_hostname);

   /* now start from beginning */
   spp = argv;
   while (*spp) {

/*----------------------------------------------------------------------------*/
      /* "-acal cal-name" */

      if ((strcmp("-acal", *spp) == 0) ||
          (strcmp("-Acal", *spp) == 0)) {
         if (!strcmp("-acal", *spp)) {
            qconf_is_manager(ctx, username);

            spp = sge_parser_get_next(ctx, spp);
           
            /* get a generic calendar */
            ep = sge_generic_cal(*spp); 
            filename = (char *)spool_flatfile_write_object(&alp, ep, false,
                                                 CAL_fields, &qconf_sfi,
                                                 SP_DEST_TMP, SP_FORM_ASCII,
                                                 NULL, false);
            lFreeElem(&ep);
            if (answer_list_output(&alp)) {
               if (filename != NULL) {
                  unlink(filename);
                  sge_free(&filename);
               }
               sge_error_and_exit(ctx, NULL);
            }

            /* edit this file */
            status = sge_edit(filename, uid, gid);
            if (status < 0) {
               unlink(filename);
               sge_free(&filename);
               if (sge_error_and_exit(ctx, MSG_PARSE_EDITFAILED)) {
                  continue;
               }
            }

            if (status > 0) {
               unlink(filename);
               sge_free(&filename);
               if (sge_error_and_exit(ctx, MSG_FILE_FILEUNCHANGED)) {
                  continue;
               }
            }
         
            /* read it in again */
            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, CAL_Type, NULL,
                                            CAL_fields, fields_out, true, &qconf_sfi,
                                            SP_FORM_ASCII, NULL, filename);
            unlink(filename);
            sge_free(&filename);
            
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }

            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(CAL_fields, fields_out, &alp);
            }

            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }      
            
            if (ep == NULL) {
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE)) {
                  continue;
               }
            }
         } else { /* -Acal */
            spp = sge_parser_get_next(ctx, spp);
           
            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, CAL_Type, NULL,
                                            CAL_fields, fields_out, true, &qconf_sfi,
                                            SP_FORM_ASCII, NULL, *spp);
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }

            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(CAL_fields, fields_out, &alp);
            }

            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }
            
            if (ep == NULL) {
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE)) {
                  continue;
               }
            }
         }

         /* send it to qmaster */
         lp = lCreateList("cal to add", CAL_Type); 
         lAppendElem(lp, ep);
         alp = ctx->gdi(ctx, SGE_CAL_LIST, SGE_GDI_ADD, &lp, NULL, NULL, false);

         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*-----------------------------------------------------------------------------*/
      /* "-ackpt ckpt_name" or "-Ackpt fname" */

      if ((strcmp("-ackpt", *spp) == 0) ||
          (strcmp("-Ackpt", *spp) == 0)) {

         if (!strcmp("-ackpt", *spp)) {
            qconf_is_manager(ctx, username);

            spp = sge_parser_get_next(ctx, spp);

            /* get a generic ckpt configuration */
            ep = sge_generic_ckpt(*spp); 
            filename = (char *)spool_flatfile_write_object(&alp, ep, false,
                                                 CK_fields, &qconf_sfi,
                                                 SP_DEST_TMP, SP_FORM_ASCII,
                                                 NULL, false);
            if (answer_list_output(&alp)) {
               if (filename != NULL) {
                  unlink(filename);
                  sge_free(&filename);
               }
               sge_error_and_exit(ctx, NULL);
            }

            lFreeElem(&ep);
            
            /* edit this file */
            status = sge_edit(filename, uid, gid);
            if (status < 0) {
               unlink(filename);
               sge_free(&filename);
               if (sge_error_and_exit(ctx, MSG_PARSE_EDITFAILED))
                  continue;
            }

            if (status > 0) {
               unlink(filename);
               sge_free(&filename);
               if (sge_error_and_exit(ctx, MSG_FILE_FILEUNCHANGED))
                  continue;
            }
         
            /* read it in again */
            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, CK_Type, NULL,
                                            CK_fields, fields_out, true, &qconf_sfi,
                                            SP_FORM_ASCII, NULL, filename);
            unlink(filename);
            sge_free(&filename);
            
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }

            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(CK_fields, fields_out, &alp);
            }

            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }
            
            if ((ep != NULL) && (ckpt_validate(ep, &alp) != STATUS_OK)) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }
            
            if (ep == NULL) {
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE)) {
                  continue;
               }
            }
         } else { /* -Ackpt */
            spp = sge_parser_get_next(ctx, spp);

            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, CK_Type, NULL,
                                            CK_fields, fields_out,  true, &qconf_sfi,
                                            SP_FORM_ASCII, NULL, *spp);
            
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }

            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(CK_fields, fields_out, &alp);
            }

            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }

            if ((ep != NULL) && (ckpt_validate(ep, &alp) != STATUS_OK)) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }
               
            if (ep == NULL) {
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE)) {
                  continue;
               }
            }
         }

         /* send it to qmaster */
         lp = lCreateList("CKPT list to add", CK_Type); 
         lAppendElem(lp, ep);

         alp = ctx->gdi(ctx, SGE_CK_LIST, SGE_GDI_ADD, &lp, NULL, NULL, false);

         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-ae [server_name]" */
      if (strcmp("-ae", *spp) == 0) {
         char *host = NULL;

         cp = NULL;
         qconf_is_manager(ctx, username);
         if (!sge_next_is_an_opt(spp)) {
            lListElem *hep = NULL;

            spp = sge_parser_get_next(ctx, spp);

            /* try to resolve hostname */
            hep = lCreateElem(EH_Type);
            lSetHost(hep, EH_name, *spp);

            switch (sge_resolve_host(hep, EH_name)) {
               case CL_RETVAL_OK:
                  break;
               default:
                  fprintf(stderr, MSG_SGETEXT_CANTRESOLVEHOST_S, lGetHost(hep, EH_name));
                  fprintf(stderr, "\n");
                  lFreeElem(&hep);
                  DRETURN(1);
            }
            
            host = sge_strdup(host, lGetHost(hep, EH_name));
            lFreeElem(&hep);
         } else {
            /* no template name given - then use "template" as name */
            host = sge_strdup(host, SGE_TEMPLATE_NAME);
         }
         /* get a template host entry .. */
         where = lWhere("%T( %Ih=%s )", EH_Type, EH_name, host);
         what = lWhat("%T(ALL)", EH_Type);
         alp = ctx->gdi(ctx, SGE_EH_LIST, SGE_GDI_GET, &arglp, where, what, false);
         lFreeWhat(&what);
         lFreeWhere(&where);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            sge_free(&host);
            lFreeList(&alp);
            spp++;
            sge_parse_return = 1;
            continue;
         }
         
         if (arglp == NULL || lGetNumberOfElem(arglp) == 0) {
            fprintf(stderr, MSG_EXEC_XISNOEXECHOST_S, host);   
            fprintf(stderr, "\n");
            sge_free(&host);
            lFreeList(&alp);
            lFreeList(&arglp);
            spp++;
            sge_parse_return = 1;
            continue;
         }

         sge_free(&host);
         lFreeList(&alp);
         
         /* edit the template */
         argep = lFirst(arglp);
         ep = edit_exechost(ctx, argep, uid, gid);
         if (ep == NULL) {
            lFreeList(&arglp);
            spp++;
            sge_parse_return = 1;
            continue;
         }

         switch (sge_resolve_host(ep, EH_name)) {
         case CL_RETVAL_OK:
            break;
         default:
            fprintf(stderr, MSG_SGETEXT_CANTRESOLVEHOST_S, lGetHost(ep, EH_name));
            fprintf(stderr, "\n");
            lFreeList(&arglp);
            DRETURN(1);
         }

         host = sge_strdup(host, lGetHost(ep, EH_name));
         lFreeList(&arglp);

         lp = lCreateList("hosts to add", EH_Type);
         lAppendElem(lp, ep);
         alp = ctx->gdi(ctx, SGE_EH_LIST, SGE_GDI_ADD, &lp, NULL, NULL, false);
         lFreeList(&lp);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            sge_free(&host);
            spp++;
            sge_parse_return = 1;
            continue;
         }

         ep = lFirst(alp);
         answer_exit_if_not_recoverable(ep);
         if (answer_get_status(ep) == STATUS_OK) {
            fprintf(stderr, MSG_EXEC_ADDEDHOSTXTOEXECHOSTLIST_S, host);
         } else {
            fprintf(stderr, "%s", lGetString(ep, AN_text));
         }
         fprintf(stderr, "\n");
      
         sge_free(&host);
         lFreeList(&alp);
         spp++;
         continue;
      }

/*-----------------------------------------------------------------------------*/
      /* "-Ae fname" */
      if (strcmp("-Ae", *spp) == 0) {
         spooling_field *fields = sge_build_EH_field_list(false, false, false);
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp); 

         /* read file */
         lp = lCreateList("exechosts to add", EH_Type); 
         fields_out[0] = NoName;
         ep = spool_flatfile_read_object(&alp, EH_Type, NULL,
                                         fields, fields_out, true, &qconf_sfi,
                                         SP_FORM_ASCII, NULL, *spp);
         if (answer_list_output(&alp)) {
            lFreeElem(&ep);
         }

         if (ep != NULL) {
            missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
         }

         sge_free(&fields);
            
         if (missing_field != NoName) {
            lFreeElem(&ep);
            answer_list_output(&alp);
            sge_parse_return = 1;
         }

         if (!ep) {
            fprintf(stderr, "%s\n", MSG_ANSWER_INVALIDFORMAT); 
            DRETURN(1);
         }
         lAppendElem(lp, ep);

         /* test host name */
         switch (sge_resolve_host(ep, EH_name)) {
         case CL_RETVAL_OK:
            break;
         default:
            fprintf(stderr, MSG_SGETEXT_CANTRESOLVEHOST_S, lGetHost(ep, EH_name));
            fprintf(stderr, "\n");
            lFreeElem(&ep);
            DRETURN(1);
         }

         alp = ctx->gdi(ctx, SGE_EH_LIST, SGE_GDI_ADD, &lp, NULL, NULL, false);

         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/

      /* "-ah server_name[,server_name,...]" */
      if (strcmp("-ah", *spp) == 0) {
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);
         parse_name_list_to_cull("host to add", &lp, AH_Type, AH_name, *spp);
         if (!add_host_of_type(ctx, lp, SGE_AH_LIST)) {
            sge_parse_return |= 1;
         }
         
         lFreeList(&lp);

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-arqs rqs_name" */
      if (strcmp("-arqs", *spp) == 0) {
         const char *name = "template";

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            name = *spp;
         }
         qconf_is_manager(ctx, username);
         rqs_add(ctx, &alp, name);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));          
            lFreeList(&alp);
            lFreeList(&lp);
            DRETURN(1);
         } else {
            fprintf(stdout, "%s\n", lGetString(aep, AN_text));
         }
         lFreeList(&alp);

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-Arqs fname" */
      if (strcmp("-Arqs", *spp) == 0) {
         const char *file = NULL;

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN);
         }
         qconf_is_manager(ctx, username);

         rqs_add_from_file(ctx, &alp, file);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));          
            lFreeList(&alp);
            lFreeList(&lp);
            DRETURN(1);
         } else {
            fprintf(stdout, "%s\n", lGetString(aep, AN_text));
         }
         lFreeList(&alp);

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-am user_list" */

      if (strcmp("-am", *spp) == 0) {
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);
         lString2List(*spp, &lp, UM_Type, UM_name, ", ");
         alp = ctx->gdi(ctx, SGE_UM_LIST, SGE_GDI_ADD, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*-----------------------------------------------------------------------------*/
      /* "-ao user_list" */

      if (strcmp("-ao", *spp) == 0) {
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);
         lString2List(*spp, &lp, UO_Type, UO_name, ", ");
         alp = ctx->gdi(ctx, SGE_UO_LIST, SGE_GDI_ADD, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*-----------------------------------------------------------------------------*/
      /* "-ap pe_name" */

      if ((strcmp("-ap", *spp) == 0) ||
          (strcmp("-Ap", *spp) == 0)) {

         if (!strcmp("-ap", *spp)) {
            qconf_is_manager(ctx, username);
           
            spp = sge_parser_get_next(ctx, spp);

            /* get a generic parallel environment */
            ep = pe_create_template(*spp); 
            filename = (char *)spool_flatfile_write_object(&alp, ep, false,
                                                 PE_fields, &qconf_sfi,
                                                 SP_DEST_TMP, SP_FORM_ASCII,
                                                 NULL, false);
            lFreeElem(&ep);
            
            if (answer_list_output(&alp)) {
               if (filename != NULL) {
                  unlink(filename);
                  sge_free(&filename);
               }
               sge_error_and_exit(ctx, NULL);
            }

            /* edit this file */
            status = sge_edit(filename, uid, gid);
            if (status < 0) {
               unlink(filename);
               sge_free(&filename);
               if (sge_error_and_exit(ctx, MSG_PARSE_EDITFAILED)) {
                  continue;
               }
            }

            if (status > 0) {
               unlink(filename);
               sge_free(&filename);
               if (sge_error_and_exit(ctx, MSG_FILE_FILEUNCHANGED)) {
                  continue;
               }
            }
         
            /* read it in again */
            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, PE_Type, NULL,
                                            PE_fields, fields_out,  true, &qconf_sfi,
                                            SP_FORM_ASCII, NULL, filename);
            unlink(filename);
            sge_free(&filename);
            
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }

            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(PE_fields, fields_out, &alp);
            }

            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }

            if ((ep != NULL) && (pe_validate(ep, &alp, 0) != STATUS_OK)) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }
            
            if (ep == NULL) {
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE)) {
                  continue;
               }
            }
         } else { /* -Ap */
            spp = sge_parser_get_next(ctx, spp);

            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, PE_Type, NULL,
                                            PE_fields, fields_out, true, &qconf_sfi,
                                            SP_FORM_ASCII, NULL, *spp);
            
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }

            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(PE_fields, fields_out, &alp);
            }

            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }

            if ((ep != NULL) && (pe_validate(ep, &alp, 0) != STATUS_OK)) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }

            if (ep == NULL) {
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE)) {
                  continue;
               }
            }
         }

         /* send it to qmaster */
         lp = lCreateList("PE list to add", PE_Type); 
         lAppendElem(lp, ep);
         alp = ctx->gdi(ctx, SGE_PE_LIST, SGE_GDI_ADD, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);

         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-auser" */

      if (strcmp("-auser", *spp) == 0) {
         qconf_is_manager(ctx, username);

         /* get a template for editing */
         ep = getUserTemplate(); 

         newep = edit_user(ctx, ep, uid, gid);
         lFreeElem(&ep);

         /* send it to qmaster */
         lp = lCreateList("User list to add", UU_Type); 
         lAppendElem(lp, newep);
         alp = ctx->gdi(ctx, SGE_UU_LIST, SGE_GDI_ADD, &lp, NULL, NULL, false);
         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));          
            lFreeList(&alp);
            lFreeList(&lp);
            DRETURN(1);
         } else {
            fprintf(stdout, "%s\n", lGetString(aep, AN_text));
         }
                  
         spp++;
         lFreeList(&alp);
         lFreeList(&lp);

         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-aprj" */

      if (strcmp("-aprj", *spp) == 0) {
         qconf_is_manager(ctx, username);
        
         /* get a template for editing */
         ep = getPrjTemplate(); 
         
         newep = edit_project(ctx, ep, uid, gid);
         lFreeElem(&ep);

         /* send it to qmaster */
         lp = lCreateList("Project list to add", PR_Type); 
         lAppendElem(lp, newep);
         alp = ctx->gdi(ctx, SGE_PR_LIST, SGE_GDI_ADD, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
                  
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-Auser" */

      if (strcmp("-Auser", *spp) == 0) {
         char* file = NULL;
         spooling_field *fields = NULL;
         
         /* no adminhost/manager check needed here */

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN); 
         }        


         /* get project  */
         ep = NULL;
         fields_out[0] = NoName;
         fields = sge_build_UU_field_list(false);
         ep = spool_flatfile_read_object(&alp, UU_Type, NULL, fields, fields_out,
                                          true, &qconf_sfi, SP_FORM_ASCII, NULL,
                                          file);
         
         if (answer_list_output(&alp)) {
            lFreeElem(&ep);
         }

         if (ep != NULL) {
            missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
         }

         sge_free(&fields);
         
         if (missing_field != NoName) {
            lFreeElem(&ep);
            answer_list_output(&alp);
            sge_parse_return = 1;
         }

         if (ep == NULL) {
            sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE); 
         }
         
         /* send it to qmaster */
         lp = lCreateList("User to add", UU_Type); 
         lAppendElem(lp, ep);
         alp = ctx->gdi(ctx, SGE_UU_LIST, SGE_GDI_ADD, &lp, NULL, NULL, false);
         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));          
            lFreeList(&alp);
            lFreeList(&lp);
            DRETURN(1);
         } else {
            fprintf(stdout, "%s\n", lGetString(aep, AN_text));
         }         
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }


/*----------------------------------------------------------------------------*/

      /* "-Aprj" */

      if (strcmp("-Aprj", *spp) == 0) {
         char* file = NULL;
         spooling_field *fields = NULL;
         
         /* no adminhost/manager check needed here */

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN); 
         }        


         /* get project  */
         ep = NULL;
         fields_out[0] = NoName;
         fields = sge_build_PR_field_list(false);
         ep = spool_flatfile_read_object(&alp, PR_Type, NULL, fields, fields_out,
                                          true, &qconf_sfi, SP_FORM_ASCII, NULL,
                                          file);
         
         if (answer_list_output(&alp)) {
            lFreeElem(&ep);
         }

         if (ep != NULL) {
            missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
         }

         sge_free(&fields);
         
         if (missing_field != NoName) {
            lFreeElem(&ep);
            answer_list_output(&alp);
         }
            
         if (ep == NULL) {
            sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE); 
         }
        
         /* send it to qmaster */
         lp = lCreateList("Project list to add", PR_Type); 
         lAppendElem(lp, ep);
         alp = ctx->gdi(ctx, SGE_PR_LIST, SGE_GDI_ADD, &lp, NULL, NULL, false);
         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         fprintf(stderr, "%s\n", lGetString(aep, AN_text)); 
         if (answer_get_status(aep) != STATUS_OK) {         
            lFreeList(&alp);
            lFreeList(&lp);
            DRETURN(1);
         }         
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*-----------------------------------------------------------------------------*/

      /* "-as server_name[,server_name,...]" */
      if (strcmp("-as", *spp) == 0) {
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);
         parse_name_list_to_cull("host to add", &lp, SH_Type, SH_name, *spp);
         if (!add_host_of_type(ctx, lp, SGE_SH_LIST)) {
            sge_parse_return = 1;
         }
         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-astree", "-Astree file":  add sharetree */

      if ((strcmp("-astree", *spp) == 0) || (strcmp("-Astree", *spp) == 0)) {
         lListElem *unspecified = NULL;
         
         if (strcmp("-astree", *spp) == 0) {
            qconf_is_manager(ctx, username);

            /* get the sharetree .. */
            what = lWhat("%T(ALL)", STN_Type);
            alp = ctx->gdi(ctx, SGE_STN_LIST, SGE_GDI_GET, &lp, NULL, what, false);
            lFreeWhat(&what);

            sge_parse_return |= show_answer_list(alp);
            if (sge_parse_return) {
               lFreeList(&alp);
               spp++;
               continue;
            }
            lFreeList(&alp);
    
            ep = lFirst(lp);
            if (!(ep = edit_sharetree(ctx, ep, uid, gid)))
               continue;

            lFreeList(&lp);
         } else { /* -Astree */
            spooling_field *fields = sge_build_STN_field_list(false, true);
            
            spp = sge_parser_get_next(ctx, spp);
           
            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, STN_Type, NULL,
                                            fields, fields_out, true,
                                            &qconf_name_value_list_sfi,
                                            SP_FORM_ASCII, NULL, *spp);
            
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }

            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
            }

            sge_free(&fields);
            
            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }
               
            if (ep == NULL) {
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE)) {
                  continue;
               }
            }      
         }
         
         /* Make sure that no nodes are left unspecified.  An unspecified node
          * happens when a node appears in another node's child list, but does
          * not appear itself. */
         unspecified = sge_search_unspecified_node(ep);
         
         if (unspecified != NULL) {
            fprintf(stderr, MSG_STREE_NOVALIDNODEREF_U,
                    sge_u32c(lGetUlong(unspecified, STN_id)));
            fprintf(stderr, "\n");

            lFreeElem(&ep);
            spp++;
            continue;
         }
         
         newlp = lCreateList("sharetree add", STN_Type);
         lAppendElem(newlp, ep);

         what = lWhat("%T(ALL)", STN_Type);
         alp = ctx->gdi(ctx, SGE_STN_LIST, SGE_GDI_ADD, &newlp, NULL, what, false);
         lFreeWhat(&what);

         ep = lFirst(alp);
         answer_exit_if_not_recoverable(ep);
         if (answer_get_status(ep) == STATUS_OK)
            fprintf(stderr, "%s\n", MSG_TREE_CHANGEDSHARETREE);
         else
            fprintf(stderr, "%s\n", lGetString(ep, AN_text));
         lFreeList(&newlp);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-astnode node_path=shares[,...]"  create/modify sharetree node */
      /* "-mstnode node_path=shares[,...]"  modify sharetree node */

      if (((strcmp("-astnode", *spp) == 0) && ((opt=astnode_OPT) != 0)) ||
          ((strcmp("-mstnode", *spp) == 0) && ((opt=mstnode_OPT) != 0))) {
         int modified = 0;
         int print_usage = 0;

         /* no adminhost/manager check needed here */
         spp = sge_parser_get_next(ctx, spp);

         /* get the sharetree .. */
         what = lWhat("%T(ALL)", STN_Type);
         alp = ctx->gdi(ctx, SGE_STN_LIST, SGE_GDI_GET, &lp, NULL, what, false);
         lFreeWhat(&what);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            spp++;
            continue;
         }
         lFreeList(&alp);
 
         ep = lFirst(lp);
         if (!ep && opt == astnode_OPT) {
            ep = lAddElemStr(&lp, STN_name, "Root", STN_Type);
            if (ep) lSetUlong(ep, STN_shares, 1);
         }
         if (!ep) {
            fprintf(stderr, "%s\n", MSG_TREE_NOSHARETREE);
            spp++;
            continue;
         }

         lString2List(*spp, &arglp, STN_Type, STN_name, ", ");

         for_each(argep, arglp) {
            lListElem *node = NULL;
            char *buf=NULL, *nodepath=NULL, *sharestr=NULL;
            int shares;
            ancestors_t ancestors;

            buf = sge_strdup(buf, lGetString(argep, STN_name));
            nodepath = sge_strtok(buf, "=");
            sharestr = sge_strtok(NULL, "");
            if (nodepath && sharestr && sscanf(sharestr, "%d", &shares) == 1) {
               if (shares < 0) {
                  fprintf(stderr, "%s\n", MSG_QCONF_POSITIVE_SHARE_VALUE);                  
                  DRETURN(1);
               }
               memset(&ancestors, 0, sizeof(ancestors));
               node = search_named_node_path(ep, nodepath, &ancestors);
               if (!node && opt==astnode_OPT) {
                  char *c, *lc = NULL;
                  lListElem *pnode, *nnode;

                  /* scan for basename of nodepath */
                  for (c=nodepath; *c; c++)
                     if (*c == '/' || *c == '.')
                        lc = c;

                  /* search for parent node of new node */
                  if (lc && *nodepath && *(lc+1)) {
                     char savelc = *lc;
                     *lc = '\000';
                     if (lc == nodepath && savelc == '/') /* root? */
                        pnode = ep;
                     else
                        pnode = search_named_node_path(ep, nodepath,
                                                       &ancestors);
                     if (pnode) {
                        lList *children = lGetList(pnode, STN_children);
                        nnode = lAddElemStr(&children, STN_name, lc+1, STN_Type);
                        if (nnode && !lGetList(pnode, STN_children))
                           lSetList(pnode, STN_children, children);
                        free_ancestors(&ancestors);
                        memset(&ancestors, 0, sizeof(ancestors));
                        *lc = savelc;
                        node = search_named_node_path(ep, nodepath, &ancestors);
                        if (node != nnode) {
                           fprintf(stderr, MSG_TREE_CANTADDNODEXISNONUNIQUE_S, nodepath);
                           fprintf(stderr, "\n");
                           DRETURN(1);
                        }
                     }
                  }
               }
               if (node) {
                  int i;
                  modified++;
                  lSetUlong(node, STN_shares, shares);
                  fprintf(stderr, "%s\n", MSG_TREE_SETTING);
                  for (i=0; i<ancestors.depth; i++)
                     fprintf(stderr, "/%s",
                             lGetString(ancestors.nodes[i], STN_name));
                  fprintf(stderr, "=%d\n", shares);
               } else {
                  fprintf(stderr, MSG_TREE_UNABLETOLACATEXINSHARETREE_S,
                          nodepath);
                  fprintf(stderr, "\n");
               }
               free_ancestors(&ancestors);

            } else {
               fprintf(stderr, MSG_ANSWER_XISNOTVALIDSEENODESHARESLIST_S, *spp);
               fprintf(stderr, "\n");
               print_usage = 1;
            }

            sge_free(&buf);
         }
         lFreeList(&arglp);

         if (modified) {
            what = lWhat("%T(ALL)", STN_Type);
            alp = ctx->gdi(ctx, SGE_STN_LIST, SGE_GDI_MOD, &lp, NULL, what, false);
            lFreeWhat(&what);
            ep = lFirst(alp);
            answer_exit_if_not_recoverable(ep);
            if (answer_get_status(ep) == STATUS_OK)
               fprintf(stderr, "%s\n", MSG_TREE_MODIFIEDSHARETREE);
            else
               fprintf(stderr, "%s\n", lGetString(ep, AN_text));
            lFreeList(&alp);
         } else {
            fprintf(stderr, "%s\n", MSG_TREE_NOMIDIFIEDSHARETREE);
            if (print_usage)
               sge_usage(QCONF, stderr);
            DRETURN(1);
         }

         lFreeList(&lp);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-au user_list list_name[,list_name,...]" */

      if (strcmp("-au", *spp) == 0) {

         /* no adminhost/manager check needed here */

         /* get user list */
         spp = sge_parser_get_next(ctx, spp);
         if (!*(spp+1)) {
            ERROR((SGE_EVENT, MSG_ANSWER_NOLISTNAMEPROVIDEDTOAUX_S, *spp));
            sge_usage(QCONF, stderr);
            DRETURN(1);
         }

         lString2List(*spp, &lp, UE_Type, UE_name, ",  ");
         
         /* get list_name list */
         spp = sge_parser_get_next(ctx, spp);
         lString2List(*spp, &arglp, US_Type, US_name, ", ");

         /* add all users/groups from lp to the acls in alp */
         sge_client_add_user(ctx, &alp, lp, arglp);
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&arglp);
         lFreeList(&lp);

         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/
      /* "-clearusage"  clear sharetree usage */

      if (strcmp("-clearusage", *spp) == 0) {
         lList *lp2=NULL;

         qconf_is_manager(ctx, username);

         if (!sge_next_is_an_opt(spp)) {
            /* selected user list */
            spp = sge_parser_get_next(ctx, spp);
            lString2List(*spp, &lp, UU_Type, UU_name, ", ");
            lFreeList(&lp2);    /* ensure project list is null */
        } else {
            /* get total user list */
            what = lWhat("%T(ALL)", STN_Type);
            alp = ctx->gdi(ctx, SGE_UU_LIST, SGE_GDI_GET, &lp, NULL, what, false);
            lFreeWhat(&what);

            aep = lFirst(alp);
            answer_exit_if_not_recoverable(aep);
            if (answer_get_status(aep) != STATUS_OK) {
               fprintf(stderr, "%s\n", lGetString(aep, AN_text));
               spp++;
               continue;
            }
            lFreeList(&alp);

            /* get project list */
            what = lWhat("%T(ALL)", STN_Type);
            alp = ctx->gdi(ctx, SGE_PR_LIST, SGE_GDI_GET, &lp2, NULL, what, false);
            lFreeWhat(&what);

            aep = lFirst(alp);
            answer_exit_if_not_recoverable(aep);
            if (answer_get_status(aep) != STATUS_OK) {
               fprintf(stderr, "%s\n", lGetString(aep, AN_text));
               spp++;
               continue;
            }
            lFreeList(&alp);
         }
 
         /* clear user usage */
         for_each(ep, lp) {
            lSetList(ep, UU_usage, NULL);
            lSetList(ep, UU_project, NULL);
         }

         /* clear project usage */
         for_each(ep, lp2) {
            lSetList(ep, PR_usage, NULL);
            lSetList(ep, PR_project, NULL);
         }

         /* update user usage */
         if (lp != NULL && lGetNumberOfElem(lp) > 0) {
            alp = ctx->gdi(ctx, SGE_UU_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
            answer_list_on_error_print_or_exit(&alp, stderr);
            lFreeList(&alp);
         }

         /* update project usage */
         if (lp2 && lGetNumberOfElem(lp2) > 0) {
            alp = ctx->gdi(ctx, SGE_PR_LIST, SGE_GDI_MOD, &lp2, NULL, NULL, false);            answer_list_on_error_print_or_exit(&alp, stderr);
            lFreeList(&alp);
         }
         
         lFreeList(&lp);
         lFreeList(&lp2);
         spp++;
         continue;
      }

/*-----------------------------------------------------------------------------*/
      /* "-dcal calendar_name" */

      if (strcmp("-dcal", *spp) == 0) {
         /* no adminhost/manager check needed here */
         spp = sge_parser_get_next(ctx, spp);
         ep = lCreateElem(CAL_Type);
         lSetString(ep, CAL_name, *spp);
         lp = lCreateList("cal's to del", CAL_Type);
         lAppendElem(lp, ep);
         alp = ctx->gdi(ctx, SGE_CAL_LIST, SGE_GDI_DEL, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-dckpt ckpt_name" */

      if (strcmp("-dckpt", *spp) == 0) {
         /* no adminhost/manager check needed here */
         spp = sge_parser_get_next(ctx, spp);

         ep = lCreateElem(CK_Type);
         lSetString(ep, CK_name, *spp);
         lp = lCreateList("ckpt interfaces to del", CK_Type);
         lAppendElem(lp, ep);
         alp = ctx->gdi(ctx, SGE_CK_LIST, SGE_GDI_DEL, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/

      /* "-de server_name[,server_name,...]" */
      if (strcmp("-de", *spp) == 0) {
         /* no adminhost/manager check needed here */
         spp = sge_parser_get_next(ctx, spp);
         parse_name_list_to_cull("host to del", &lp, EH_Type, EH_name, *spp);
         if (!del_host_of_type(ctx, lp, SGE_EH_LIST)) {
            sge_parse_return = 1;
         }
         lFreeList(&lp);
         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/

      /* "-dh server_name[,server_name,...]" */
      if (strcmp("-dh", *spp) == 0) {
         /* no adminhost/manager check needed here */
         spp = sge_parser_get_next(ctx, spp);
         parse_name_list_to_cull("host to del", &lp, AH_Type, AH_name, *spp);
         if (!del_host_of_type(ctx, lp, SGE_AH_LIST)) {
            sge_parse_return = 1;
         }
         lFreeList(&lp);

         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/
      /* "-drqs rqs_name[,rqs_name,...]" */
      if (strcmp("-drqs", *spp) == 0) {
         /* no adminhost/manager check needed here */
         spp = sge_parser_get_next(ctx, spp);

         lString2List(*spp, &lp, RQS_Type, RQS_name, ", ");
         alp = ctx->gdi(ctx, SGE_RQS_LIST, SGE_GDI_DEL, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/
      /* "-dm user_list" */

      if (strcmp("-dm", *spp) == 0) {
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);

         lString2List(*spp, &lp, UM_Type, UM_name, ", ");
         alp = ctx->gdi(ctx, SGE_UM_LIST, SGE_GDI_DEL, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/
      /* "-do user_list" */

      if (strcmp("-do", *spp) == 0) {
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);

         lString2List(*spp, &lp, UO_Type, UO_name, ", ");
         alp = ctx->gdi(ctx, SGE_UO_LIST, SGE_GDI_DEL, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-dp pe-name" */

      if (strcmp("-dp", *spp) == 0) {
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);

         ep = lCreateElem(PE_Type);
         lSetString(ep, PE_name, *spp);
         lp = lCreateList("pe's to del", PE_Type);
         lAppendElem(lp, ep);
         alp = ctx->gdi(ctx, SGE_PE_LIST, SGE_GDI_DEL, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-ds server_name[,server_name,...]" */
      if (strcmp("-ds", *spp) == 0) {
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);
         parse_name_list_to_cull("host to del", &lp, SH_Type, SH_name, *spp);
         if (!del_host_of_type(ctx, lp, SGE_SH_LIST)) {
            sge_parse_return = 1;
         }
         lFreeList(&lp);

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-du user_list list_name[,list_name,...]" */

      if (strcmp("-du", *spp) == 0) {
         
         /* no adminhost/manager check needed here */

         /* get user list */
         spp = sge_parser_get_next(ctx, spp);
         if (!*(spp+1)) {
            ERROR((SGE_EVENT, MSG_ANSWER_NOLISTNAMEPROVIDEDTODUX_S, *spp));
            sge_usage(QCONF, stderr);
            DRETURN(1);
         }
         lString2List(*spp, &lp, UE_Type, UE_name, ",  ");
         
         /* get list_name list */
         spp = sge_parser_get_next(ctx, spp);
         lString2List(*spp, &arglp, US_Type, US_name, ", ");

         /* remove users/groups from lp from the acls in alp */
         sge_client_del_user(ctx, &alp, lp, arglp);
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&lp);
         lFreeList(&alp);
         lFreeList(&arglp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-dul list_name_list" */

      if (strcmp("-dul", *spp) == 0) {
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);

         lString2List(*spp, &lp, US_Type, US_name, ", ");
         alp = ctx->gdi(ctx, SGE_US_LIST, SGE_GDI_DEL, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/
      /* "-duser user,..." */

      if (strcmp("-duser", *spp) == 0) {
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);

         lString2List(*spp, &lp, UU_Type, UU_name, ", ");
         alp = ctx->gdi(ctx, SGE_UU_LIST, SGE_GDI_DEL, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-dprj project,..." */

      if (strcmp("-dprj", *spp) == 0) {
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);

         lString2List(*spp, &lp, PR_Type, PR_name, ", ");
         alp = ctx->gdi(ctx, SGE_PR_LIST, SGE_GDI_DEL, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);

         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-dstnode node_path[,...]"  delete sharetree node(s) */

      if (strcmp("-dstnode", *spp) == 0) {
         int modified = 0;
         /* no adminhost/manager check needed here */
         spp = sge_parser_get_next(ctx, spp);

         /* get the sharetree .. */
         what = lWhat("%T(ALL)", STN_Type);
         alp = ctx->gdi(ctx, SGE_STN_LIST, SGE_GDI_GET, &lp, NULL, what, false);
         lFreeWhat(&what);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            sge_parse_return = 1;
            spp++;
            continue;
         }
         lFreeList(&alp);
 
         ep = lFirst(lp);
         if (!ep) {
            fprintf(stderr, "%s\n", MSG_TREE_NOSHARETREE);
            sge_parse_return = 1;
            spp++;
            continue;
         }

         lString2List(*spp, &arglp, STN_Type, STN_name, ", ");

         for_each(argep, arglp) {

            lListElem *node = NULL;
            const char *nodepath = NULL;
            ancestors_t ancestors;

            nodepath = lGetString(argep, STN_name);
            if (nodepath) {
               memset(&ancestors, 0, sizeof(ancestors_t));
               node = search_named_node_path(ep, nodepath, &ancestors);
               if (node) {
                  if (lGetList(node, STN_children) == NULL) {
                     if (ancestors.depth > 0) {
                        int i;
                        lList *siblings = NULL;
                        lListElem *pnode = NULL;
                        modified++;
                        if (ancestors.depth == 1)
                           pnode = ep;
                        else
                           pnode = ancestors.nodes[ancestors.depth-2];
                        fprintf(stderr, "%s\n", MSG_TREE_REMOVING);
                        for (i=0; i<ancestors.depth; i++)
                           fprintf(stderr, "/%s",
                                   lGetString(ancestors.nodes[i], STN_name));
                        fprintf(stderr, "\n");
                        siblings = lGetList(pnode, STN_children);
                        lRemoveElem(siblings, &node);
                        if (lGetNumberOfElem(siblings) == 0)
                           lSetList(pnode, STN_children, NULL);
                     } else {
                        fprintf(stderr, "%s\n", MSG_TREE_CANTDELROOTNODE);
                     }
                  } else {
                     fprintf(stderr, "%s\n", MSG_TREE_CANTDELNODESWITHCHILD);
                  }
               } else {
                  fprintf(stderr, MSG_TREE_UNABLETOLACATEXINSHARETREE_S,
                          nodepath);
                  fprintf(stderr, "\n");
               }
               free_ancestors(&ancestors);

            }
         }
         lFreeList(&arglp);

         if (modified) {
            what = lWhat("%T(ALL)", STN_Type);
            alp = ctx->gdi(ctx, SGE_STN_LIST, SGE_GDI_MOD, &lp, NULL, what, false);
            lFreeWhat(&what);
            ep = lFirst(alp);
            answer_exit_if_not_recoverable(ep);
            if (answer_get_status(ep) == STATUS_OK)
               fprintf(stderr, "%s\n", MSG_TREE_MODIFIEDSHARETREE);
            else {
               fprintf(stderr, "%s\n", lGetString(ep, AN_text));
               sge_parse_return = 1;
            }
            lFreeList(&alp);
         } else {
            fprintf(stderr, "%s\n", MSG_TREE_NOMIDIFIEDSHARETREE);
            DRETURN(1);
         }

         lFreeList(&lp);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-dstree" */

      if (strcmp("-dstree", *spp) == 0) {
         /* no adminhost/manager check needed here */
         
         alp = ctx->gdi(ctx, SGE_STN_LIST, SGE_GDI_DEL, NULL, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-help" */

      if (strcmp("-help", *spp) == 0) {
         sge_usage(QCONF, stdout);
         DRETURN(0);
      }

/*----------------------------------------------------------------------------*/
      /* "-ks" */

      if (strcmp("-ks", *spp) == 0) {
         /* no adminhost/manager check needed here */

         alp = ctx->kill(ctx, NULL, default_cell, 0, SCHEDD_KILL);
         for_each(aep, alp) {
            answer_exit_if_not_recoverable(aep);
            if (answer_get_status(aep) != STATUS_OK)
               sge_parse_return = 1;
            answer_print_text(aep, stderr, NULL, NULL);
         }

         lFreeList(&alp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-km" */

      if (strcmp("-km", *spp) == 0) {
         /* no adminhost/manager check needed here */
         alp = ctx->kill(ctx, NULL, default_cell, 0, MASTER_KILL);
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* -kec <id> ... */
      /* <id> may be "all" */
      /* parse before -ke[j] */

      if (strncmp("-kec", *spp, 4) == 0) {
         int opt = EVENTCLIENT_KILL;
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);
         /* found namelist -> process */
         if (strcmp(*spp, "all") == 0) { /* kill all dynamic event clients (EV_ID_ANY) */
            alp = ctx->kill(ctx, NULL, default_cell, 0, opt);
         } else {
            lString2List(*spp, &lp, ID_Type, ID_str, ", ");
            alp = ctx->kill(ctx, lp, default_cell, 0, opt);
         }      
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);
         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/

      /* -at <name> ... */
      /* <name> may be "scheduler", "jvm" */

      if (strncmp("-at", *spp, 4) == 0) {
         int opt = THREAD_START;
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);
         lString2List(*spp, &lp, ID_Type, ID_str, ", ");
         for_each(ep, lp) {
            lSetUlong(ep, ID_action, SGE_THREAD_TRIGGER_START);
         }
         alp = ctx->kill(ctx, lp, default_cell, 0, opt);
         lFreeList(&lp);
         answer_list_on_error_print_or_exit(&alp, stderr);
         lFreeList(&alp);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* -kt <name> ... */
      /* <name> may be "scheduler", "jvm" */

      if (strncmp("-kt", *spp, 4) == 0) {
         int opt = THREAD_START;
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp);
         lString2List(*spp, &lp, ID_Type, ID_str, ", ");
         for_each(ep, lp) {
            lSetUlong(ep, ID_action, SGE_THREAD_TRIGGER_STOP);
         }
         alp = ctx->kill(ctx, lp, default_cell, 0, opt);
         lFreeList(&lp);
         answer_list_on_error_print_or_exit(&alp, stderr);
         lFreeList(&alp);
         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/

      /* -ke[j] <host> ... */
      /* <host> may be "all" */
      if (strncmp("-k", *spp, 2) == 0) {
         int opt = EXECD_KILL;
         /* no adminhost/manager check needed here */

         cp = (*spp) + 2;
         switch (*cp++) {
            case 'e':
               break;
            default:
               ERROR((SGE_EVENT, MSG_ANSWER_XISNOTAVALIDOPTIONY_SU, *spp, sge_u32c(prog_number)));
               sge_usage(QCONF, stderr);
               DRETURN(1);
         }

         if (*cp == 'j') {
            cp++;
            opt |= JOB_KILL;
         }

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
         } else {
            if (sge_error_and_exit(ctx, MSG_HOST_NEEDAHOSTNAMEORALL))
               continue;
         }

         if (strcmp(*spp, "all") == 0) { /* kill all dynamic event clients (EV_ID_ANY) */
            alp = ctx->kill(ctx, NULL, default_cell, 0, opt);
         } else {   
            /* found namelist -> process */
            lString2List(*spp, &lp, EH_Type, EH_name, ", ");
            alp = ctx->kill(ctx, lp, default_cell, 0, opt);
         }
         sge_parse_return |= show_answer_list(alp);

         lFreeList(&alp);
         lFreeList(&lp);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-mc" */

      if (strcmp("-mc", *spp) == 0) {
         lList *answer_list = NULL;

         qconf_is_manager(ctx, username);

         if (centry_list_modify(ctx, &answer_list) == false) {
            sge_parse_return = 1;
         }
         sge_parse_return |= show_answer_list(answer_list);
         lFreeList(&answer_list);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-mcal cal-name" */

      if ((strcmp("-mcal", *spp) == 0) || 
          (strcmp("-Mcal", *spp) == 0)) {
         if (!strcmp("-mcal", *spp)) {
            qconf_is_manager(ctx, username);

            spp = sge_parser_get_next(ctx, spp);
           
            where = lWhere("%T( %I==%s )", CAL_Type, CAL_name, *spp);
            what = lWhat("%T(ALL)", CAL_Type);
            alp = ctx->gdi(ctx, SGE_CAL_LIST, SGE_GDI_GET, &lp, where, what, false);
            lFreeWhere(&where);
            lFreeWhat(&what);

            aep = lFirst(alp);
            answer_exit_if_not_recoverable(aep);
            if (answer_get_status(aep) != STATUS_OK) {
               fprintf(stderr, "%s\n", lGetString(aep, AN_text));
               sge_parse_return = 1;
               spp++;
               continue;
            }
            lFreeList(&alp);

            if (lp == NULL || lGetNumberOfElem(lp) == 0) {
               fprintf(stderr, MSG_CALENDAR_XISNOTACALENDAR_S, *spp);
               fprintf(stderr, "\n");
               lFreeList(&lp);
               DRETURN(1);
            }

            ep = lFirst(lp);
            filename = (char *)spool_flatfile_write_object(&alp, ep, false,
                                                 CAL_fields, &qconf_sfi,
                                                 SP_DEST_TMP, SP_FORM_ASCII,
                                                 NULL, false);
            
            if (answer_list_output(&alp)) {
               if (filename != NULL) {
                  unlink(filename);
                  sge_free(&filename);
               }
               sge_error_and_exit(ctx, NULL);
            }

            lFreeList(&lp);
            
            /* edit this file */
            status = sge_edit(filename, uid, gid);
            if (status < 0) {
               unlink(filename);
               sge_free(&filename);
               if (sge_error_and_exit(ctx, MSG_PARSE_EDITFAILED))
                  continue;
            }

            if (status > 0) {
               unlink(filename);
               sge_free(&filename);
               if (sge_error_and_exit(ctx, MSG_FILE_FILEUNCHANGED))
                  continue;
            }

            /* read it in again */
            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, CAL_Type, NULL,
                                            CAL_fields, fields_out, true, &qconf_sfi,
                                            SP_FORM_ASCII, NULL, filename);
            unlink(filename);
            sge_free(&filename);
            
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }

            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(CAL_fields, fields_out, &alp);
            }

            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }
               
            if (ep == NULL) {
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE)) {
                  continue;
               }
            }
         } else {
            spp = sge_parser_get_next(ctx, spp);
           
            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, CAL_Type, NULL,
                                            CAL_fields, fields_out, true, &qconf_sfi,
                                            SP_FORM_ASCII, NULL, *spp);
            
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }

            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(CAL_fields, fields_out, &alp);
            }

            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }
               
            if (ep == NULL) 
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE)) {
                  continue;
               }
         }

         /* send it to qmaster */
         lp = lCreateList("calendar to add", CAL_Type); 
         lAppendElem(lp, ep);
         alp = ctx->gdi(ctx, SGE_CAL_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/
      /* "-Mc complex file" */

      if (strcmp("-Mc", *spp) == 0) {
         lList *answer_list = NULL;
         char* file = NULL;

         /* no adminhost/manager check needed here */

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN);
         }
         if (!centry_list_modify_from_file(ctx, &answer_list, file)) {
            sge_parse_return = 1;
         }
         sge_parse_return |= show_answer_list(answer_list);
         lFreeList(&answer_list);
         spp++;
         continue;
      }

/*-----------------------------------------------------------------------------*/
      /* "-mckpt ckpt_name" or "-Mckpt fname" */

      if ((strcmp("-mckpt", *spp) == 0) || 
          (strcmp("-Mckpt", *spp) == 0)) {
         if (strcmp("-mckpt", *spp) == 0) {
            qconf_is_manager(ctx, username);

            spp = sge_parser_get_next(ctx, spp);

            /* get last version of this pe from qmaster */
            where = lWhere("%T( %I==%s )", CK_Type, CK_name, *spp);
            what = lWhat("%T(ALL)", CK_Type);
            alp = ctx->gdi(ctx, SGE_CK_LIST, SGE_GDI_GET, &lp, where, what, false);
            lFreeWhere(&where);
            lFreeWhat(&what);

            aep = lFirst(alp);
            answer_exit_if_not_recoverable(aep);
            if (answer_get_status(aep) != STATUS_OK) {
               fprintf(stderr, "%s\n", lGetString(aep, AN_text));
               sge_parse_return = 1;
               spp++;
               continue;
            }
            lFreeList(&alp);

            if (lp == NULL || lGetNumberOfElem(lp) == 0 ) {
               fprintf(stderr, MSG_CKPT_XISNOTCHKPINTERFACEDEF_S, *spp);
               fprintf(stderr, "\n");
               lFreeList(&lp);
               DRETURN(1);
            }

            ep = lFirst(lp);
            filename = (char *)spool_flatfile_write_object(&alp, ep, false,
                                                 CK_fields, &qconf_sfi,
                                                 SP_DEST_TMP, SP_FORM_ASCII,
                                                 NULL, false);
            
            if (answer_list_output(&alp)) {
               if (filename != NULL) {
                  unlink(filename);
                  sge_free(&filename);
               }
               sge_error_and_exit(ctx, NULL);
            }

            lFreeList(&lp);
            
            /* edit this file */
            status = sge_edit(filename, uid, gid);
            if (status < 0) {
               unlink(filename);
               sge_free(&filename);
               if (sge_error_and_exit(ctx, MSG_PARSE_EDITFAILED))
                  continue;
            }

            if (status > 0) {
               unlink(filename);
               sge_free(&filename);
               if (sge_error_and_exit(ctx, MSG_FILE_FILEUNCHANGED))
                  continue;
            }

            /* read it in again */
            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, CK_Type, NULL,
                                            CK_fields, fields_out, true, &qconf_sfi,
                                            SP_FORM_ASCII, NULL, filename);
            unlink(filename);
            sge_free(&filename);
            
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }

            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(CK_fields, fields_out, &alp);
            }

            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }

            if ((ep != NULL) && (ckpt_validate(ep, &alp) != STATUS_OK)) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }
               
            if (ep == NULL) {
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE)) {
                  continue;
               }
            }
         } else {
            spp = sge_parser_get_next(ctx, spp);

            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, CK_Type, NULL,
                                            CK_fields, fields_out, true, &qconf_sfi,
                                            SP_FORM_ASCII, NULL, *spp);
            
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }

            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(CK_fields, fields_out, &alp);
            }

            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }

            if ((ep != NULL) && (ckpt_validate(ep, &alp) != STATUS_OK)) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }

            if (ep == NULL) {
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE)) {
                  continue;
               }
            }
         }

         /* send it to qmaster */
         lp = lCreateList("CKPT list to add", CK_Type); 
         lAppendElem(lp, ep);
         alp = ctx->gdi(ctx, SGE_CK_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-Me fname" */
      if (strcmp("-Me", *spp) == 0) {
         spooling_field *fields = sge_build_EH_field_list(false, false, false);
         
         /* no adminhost/manager check needed here */

         spp = sge_parser_get_next(ctx, spp); 

         /* read file */
         lp = lCreateList("exechosts to change", EH_Type);
         fields_out[0] = NoName;
         ep = spool_flatfile_read_object(&alp, EH_Type, NULL,
                                         fields, fields_out, true, &qconf_sfi,
                                         SP_FORM_ASCII, NULL, *spp);
         
         if (answer_list_output(&alp)) {
            lFreeElem(&ep);
         }

         if (ep != NULL) {
            missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
         }

         sge_free(&fields);
         
         if (missing_field != NoName) {
            lFreeElem(&ep);
            answer_list_output(&alp);
            sge_parse_return = 1;
         }

         if (ep == NULL) {
            fprintf(stderr, "%s\n", MSG_ANSWER_INVALIDFORMAT); 
            DRETURN(1);
         }

         lAppendElem(lp, ep);

         /* test host name */
         if (sge_resolve_host(ep, EH_name) != CL_RETVAL_OK) {
            fprintf(stderr, MSG_SGETEXT_CANTRESOLVEHOST_S, lGetHost(ep, EH_name));
            fprintf(stderr, "\n");
            lFreeElem(&ep);
            sge_parse_return = 1;
            DRETURN(1);
         }

         alp = ctx->gdi(ctx, SGE_EH_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);

         sge_parse_return |= show_answer(alp);
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-me [server_name,...]" */

      if (strcmp("-me", *spp) == 0) {
         qconf_is_manager(ctx, username);
         
         spp = sge_parser_get_next(ctx, spp);
         parse_name_list_to_cull("hosts to change", &arglp, EH_Type, EH_name, 
            *spp);

         for_each(argep, arglp) {
            /* resolve hostname */
            if (sge_resolve_host(argep, EH_name) != CL_RETVAL_OK) {
               fprintf(stderr, MSG_SGETEXT_CANTRESOLVEHOST_S, lGetHost(argep, EH_name));
               fprintf(stderr, "\n");
               sge_parse_return = 1;
               continue;
            }
            host = lGetHost(argep, EH_name);

            /* get the existing host entry .. */
            where = lWhere("%T( %Ih=%s )", EH_Type, EH_name, host);
            what = lWhat("%T(ALL)", EH_Type);
            alp = ctx->gdi(ctx, SGE_EH_LIST, SGE_GDI_GET, &lp, where, what, false);
            lFreeWhere(&where);
            lFreeWhat(&what);

            if (show_answer(alp) == 1) {
               lFreeList(&alp);
               sge_parse_return = 1;
               continue;
            }

            if (lGetNumberOfElem(lp) == 0) {
               fprintf(stderr, MSG_EXEC_XISNOTANEXECUTIONHOST_S, host);
               fprintf(stderr, "\n");
               sge_parse_return = 1;
               continue;
            }
            lFreeList(&alp);

            ep = edit_exechost(ctx, lFirst(lp), uid, gid);
            if (ep == NULL) {
               continue;
            }
            lFreeList(&lp);
            lp = lCreateList("host to mod", EH_Type);
            lAppendElem(lp, ep);
            alp = ctx->gdi(ctx, SGE_EH_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
            lFreeList(&lp);

            if (show_answer(alp) == 1) {
               lFreeList(&alp);
               sge_parse_return = 1;
               continue;
            }
            lFreeList(&alp);
         }
         lFreeList(&arglp);
         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-mrqs rqs_name" */
      if (strcmp("-mrqs", *spp) == 0) { 
         const char *name = NULL; 

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            name = *spp;
         }
         qconf_is_manager(ctx, username);
         rqs_modify(ctx, &alp, name);
         sge_parse_return |= show_answer_list(alp);

         lFreeList(&alp);

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-Mrqs fname [rqs_name,...]" */
      if (strcmp("-Mrqs", *spp) == 0) {
         const char *file = NULL;
         const char *name = NULL;

         qconf_is_manager(ctx, username);
         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN);
         }

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp); 
            name = *spp;
         }

         rqs_modify_from_file(ctx, &alp, file, name);
         sge_parse_return |= show_answer_list(alp);

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-mp pe_name" */

      if ((strcmp("-mp", *spp) == 0) || 
          (strcmp("-Mp", *spp) == 0)) {

         if (!strcmp("-mp", *spp)) {
            qconf_is_manager(ctx, username);
         
            spp = sge_parser_get_next(ctx, spp);

            /* get last version of this pe from qmaster */
            where = lWhere("%T( %I==%s )", PE_Type, PE_name, *spp);
            what = lWhat("%T(ALL)", PE_Type);
            alp = ctx->gdi(ctx, SGE_PE_LIST, SGE_GDI_GET, &lp, where, what, false);
            lFreeWhere(&where);
            lFreeWhat(&what);

            aep = lFirst(alp);
            answer_exit_if_not_recoverable(aep);
            if (answer_get_status(aep) != STATUS_OK) {
               fprintf(stderr, "%s\n", lGetString(aep, AN_text));
               lFreeList(&alp);
               sge_parse_return = 1;
               spp++;
               continue;
            }
            lFreeList(&alp);

            if (lp == NULL || lGetNumberOfElem(lp) == 0) {
               fprintf(stderr, MSG_PARALLEL_XNOTAPARALLELEVIRONMENT_S, *spp);
               fprintf(stderr, "\n");
               lFreeList(&lp);
               DRETURN(1);
            }

            ep = lFirst(lp);

            /* write pe to temp file */
            filename = (char *)spool_flatfile_write_object(&alp, ep, false,
                                                 PE_fields, &qconf_sfi,
                                                 SP_DEST_TMP, SP_FORM_ASCII,
                                                 NULL, false);
            lFreeList(&lp);

            if (answer_list_output(&alp)) {
               if (filename != NULL) {
                  unlink(filename);
                  sge_free(&filename);
               }
               sge_error_and_exit(ctx, NULL);
            }

            /* edit this file */
            status = sge_edit(filename, uid, gid);
            if (status < 0) {
               unlink(filename);
               if (sge_error_and_exit(ctx, MSG_PARSE_EDITFAILED)) {
                  sge_free(&filename);
                  continue;
               }
            }

            if (status > 0) {
               unlink(filename);
               if (sge_error_and_exit(ctx, MSG_FILE_FILEUNCHANGED)) {
                  sge_free(&filename);
                  continue;
               }
            }

            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, PE_Type, NULL,
                                            PE_fields, fields_out, true, &qconf_sfi,
                                            SP_FORM_ASCII, NULL, filename);
            
            unlink(filename);
            sge_free(&filename);
            
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }

            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(PE_fields, fields_out, &alp);
            }

            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }

            if ((ep != NULL) && (pe_validate(ep, &alp, 0) != STATUS_OK)) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }
               
            if (ep == NULL) {
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE)) {
                  continue;
               }
            }
         } else {
            spp = sge_parser_get_next(ctx, spp);

            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, PE_Type, NULL,
                                            PE_fields, fields_out, true, &qconf_sfi,
                                            SP_FORM_ASCII, NULL, *spp);
            
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }

            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(PE_fields, fields_out, &alp);
            }

            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }

            if ((ep != NULL) && (pe_validate(ep, &alp, 0) != STATUS_OK)) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }
               
            if (ep == NULL) {
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE)) {
                  continue;
               }
            }
         }

         /* send it to qmaster */
         lp = lCreateList("PE list to add", PE_Type); 
         lAppendElem(lp, ep);
         alp = ctx->gdi(ctx, SGE_PE_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
         
         sge_parse_return |= show_answer_list(alp);
         
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
   if (strcmp("-sobjl", *spp) == 0) {
      dstring object_name = DSTRING_INIT;
      dstring attribute_pattern = DSTRING_INIT;
      dstring value_pattern = DSTRING_INIT;
      bool handle_cqueue;
      bool handle_domain;
      bool handle_qinstance;
      bool handle_exechost;

      spp = sge_parser_get_next(ctx, spp);
      sge_dstring_copy_string(&object_name, *spp);
      spp = sge_parser_get_next(ctx, spp);
      sge_dstring_copy_string(&attribute_pattern, *spp);
      spp = sge_parser_get_next(ctx, spp);
      sge_dstring_copy_string(&value_pattern, *spp);

      handle_cqueue = (strcmp(sge_dstring_get_string(&object_name), "queue") == 0) ? true : false;
      handle_domain = (strcmp(sge_dstring_get_string(&object_name), "queue_domain") == 0) ? true : false;
      handle_qinstance = (strcmp(sge_dstring_get_string(&object_name), "queue_instance") == 0) ? true : false;
      handle_exechost = (strcmp(sge_dstring_get_string(&object_name), "exechost") == 0) ? true : false;

      if (handle_exechost) {
         lEnumeration *what = NULL;
         lList *list = NULL;
         lListElem *elem = NULL;
         lList *answer_list = NULL;
         lListElem *answer_ep;

         what = lWhat("%T(ALL)", EH_Type);
         answer_list = ctx->gdi(ctx, SGE_EH_LIST, SGE_GDI_GET, &list, NULL, what, false);
         lFreeWhat(&what);

         answer_ep = lFirst(answer_list);
         answer_exit_if_not_recoverable(answer_ep);
         if (answer_get_status(answer_ep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(answer_ep, AN_text));
            lFreeList(&answer_list);
            sge_dstring_free(&object_name);
            sge_dstring_free(&attribute_pattern);
            sge_dstring_free(&value_pattern);
            DRETURN(0);
         }
         lFreeList(&answer_list);

         for_each(elem, list) {
            const char *hostname = NULL;
            bool already_handled = false;

            /*
             * hostname
             */ 
            hostname = lGetHost(elem, EH_name);
            if (!fnmatch(sge_dstring_get_string(&attribute_pattern), SGE_ATTR_HOSTNAME, 0) &&
                !fnmatch(sge_dstring_get_string(&value_pattern), hostname, 0)) {
               printf("%s\n", hostname);
               already_handled = true;
            }

            /*
             * load scaling list
             */
            if (!already_handled && 
                !fnmatch(sge_dstring_get_string(&attribute_pattern), SGE_ATTR_LOAD_SCALING, 0)) {
               dstring value = DSTRING_INIT;
               lList *value_list = NULL;
               lListElem *value_elem = NULL;
               const char *value_string = NULL;

               value_list = lGetList(elem, EH_scaling_list);
               value_elem = lFirst(value_list);

               while (value_elem != NULL) {
                  sge_dstring_sprintf_append(&value, "%s=%.10g", lGetString(value_elem, HS_name), lGetDouble(value_elem, HS_value));
                  value_elem = lNext(value_elem);
                  if (value_elem != NULL) {
                     sge_dstring_append(&value, ",");
                  }
               }
               value_string = sge_dstring_get_string(&value);
               if (value_string == NULL) {
                  sge_dstring_copy_string(&value, "NONE");
                  value_string = sge_dstring_get_string(&value);
               }
               if (!fnmatch(sge_dstring_get_string(&value_pattern), value_string, 0)) {
                  printf("%s\n", hostname);
                  already_handled = true;
               }
               sge_dstring_free(&value);
            }

            /*
             * complex_values list
             */
            if (!already_handled &&
                !fnmatch(sge_dstring_get_string(&attribute_pattern), SGE_ATTR_COMPLEX_VALUES, 0)) {
               dstring value = DSTRING_INIT;
               lList *value_list = NULL;
               lListElem *value_elem = NULL;
               const char *value_string = NULL;

               value_list = lGetList(elem, EH_consumable_config_list);
               value_elem = lFirst(value_list);

               while (value_elem != NULL) {
                  sge_dstring_sprintf_append(&value, "%s=", lGetString(value_elem, CE_name));
                  if (lGetString(value_elem, CE_stringval) != NULL) {
                     sge_dstring_append(&value, lGetString(value_elem, CE_stringval));
                  } else {
                     sge_dstring_sprintf_append(&value, "%f",
                                                lGetDouble(value_elem, CE_doubleval));
                  }
                  value_elem = lNext(value_elem);
                  if (value_elem != NULL) {
                     sge_dstring_append(&value, ",");
                  }
               }
               value_string = sge_dstring_get_string(&value);
               if (value_string == NULL) {
                  sge_dstring_copy_string(&value, "NONE");
                  value_string = sge_dstring_get_string(&value);
               }
               if (!fnmatch(sge_dstring_get_string(&value_pattern), value_string, 0)) {
                  printf("%s\n", hostname);
                  already_handled = true;
               }
               sge_dstring_free(&value);
            }

            /*
             * load_values list
             */
            if (!already_handled &&
                !fnmatch(sge_dstring_get_string(&attribute_pattern), SGE_ATTR_LOAD_VALUES, 0)) {
               dstring value = DSTRING_INIT;
               lList *value_list = NULL;
               lListElem *value_elem = NULL;
               const char *value_string = NULL;

               value_list = lGetList(elem, EH_load_list);
               value_elem = lFirst(value_list);

               while (value_elem != NULL) {
                  sge_dstring_sprintf_append(&value, "%s=", lGetString(value_elem, HL_name));
                  sge_dstring_sprintf_append(&value, "%s", lGetString(value_elem, HL_value));
                  value_elem = lNext(value_elem);
                  if (value_elem != NULL) {
                     sge_dstring_append(&value, ",");
                  }
               }
               value_string = sge_dstring_get_string(&value);
               if (value_string == NULL) {
                  sge_dstring_copy_string(&value, "NONE");
                  value_string = sge_dstring_get_string(&value);
               }
               if (!fnmatch(sge_dstring_get_string(&value_pattern), value_string, 0)) {
                  printf("%s\n", hostname);
                  already_handled = true;
               }
               sge_dstring_free(&value);
            }

            /*
             * processors 
             */
            if (!already_handled &&
                !fnmatch(sge_dstring_get_string(&attribute_pattern), SGE_ATTR_PROCESSORS, 0)) {
               dstring value = DSTRING_INIT;
               const char *value_string = NULL;

               sge_dstring_sprintf(&value, "%d", (int) lGetUlong(elem, EH_processors));
               value_string = sge_dstring_get_string(&value);
               if (!fnmatch(sge_dstring_get_string(&value_pattern), value_string, 0)) {
                  printf("%s\n", hostname);
                  already_handled = true;
               }
               sge_dstring_free(&value);
            }


            /*
             * user_lists list
             */
            if (!already_handled &&
                !fnmatch(sge_dstring_get_string(&attribute_pattern), SGE_ATTR_USER_LISTS, 0)) {
               dstring value = DSTRING_INIT;
               lList *value_list = NULL;
               lListElem *value_elem = NULL;
               const char *value_string = NULL;

               value_list = lGetList(elem, EH_acl);
               value_elem = lFirst(value_list);

               while (value_elem != NULL) {
                  sge_dstring_append(&value, lGetString(value_elem, US_name));
                  value_elem = lNext(value_elem);
                  if (value_elem != NULL) {
                     sge_dstring_append(&value, " ");
                  }
               }
               value_string = sge_dstring_get_string(&value);
               if (value_string == NULL) {
                  sge_dstring_copy_string(&value, "NONE");
                  value_string = sge_dstring_get_string(&value);
               }
               if (!fnmatch(sge_dstring_get_string(&value_pattern), value_string, 0)) {
                  printf("%s\n", hostname);
                  already_handled = true;
               }
               sge_dstring_free(&value);
            }

            /*
             * user_lists list
             */
            if (!already_handled &&
                !fnmatch(sge_dstring_get_string(&attribute_pattern), SGE_ATTR_XUSER_LISTS, 0)) {
               dstring value = DSTRING_INIT;
               lList *value_list = NULL;
               lListElem *value_elem = NULL;
               const char *value_string = NULL;

               value_list = lGetList(elem, EH_xacl);
               value_elem = lFirst(value_list);

               while (value_elem != NULL) {
                  sge_dstring_append(&value, lGetString(value_elem, US_name));
                  value_elem = lNext(value_elem);
                  if (value_elem != NULL) {
                     sge_dstring_append(&value, " ");
                  }
               }
               value_string = sge_dstring_get_string(&value);
               if (value_string == NULL) {
                  sge_dstring_copy_string(&value, "NONE");
                  value_string = sge_dstring_get_string(&value);
               }
               if (!fnmatch(sge_dstring_get_string(&value_pattern), value_string, 0)) {
                  printf("%s\n", hostname);
                  already_handled = true;
               }
               sge_dstring_free(&value);
            }
         }
      } else if (handle_cqueue || handle_domain || handle_qinstance) {
         lEnumeration *what = NULL;
         lList *list = NULL;
         lListElem *elem = NULL;
         lList *answer_list = NULL;
         lListElem *answer_ep;

         what = lWhat("%T(ALL)", CQ_Type);
         answer_list = ctx->gdi(ctx, SGE_CQ_LIST, SGE_GDI_GET, &list, NULL, what, false);
         lFreeWhat(&what);

         answer_ep = lFirst(answer_list);
         answer_exit_if_not_recoverable(answer_ep);
         if (answer_get_status(answer_ep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(answer_ep, AN_text));
            lFreeList(&answer_list);
            sge_dstring_free(&object_name);
            sge_dstring_free(&attribute_pattern);
            sge_dstring_free(&value_pattern);
            DRETURN(0);
         }
         lFreeList(&answer_list);

         for_each(elem, list) {
            int index = 0;
            bool already_handled = false;

            /*
             * Handle special case: qname
             */
            if (!fnmatch(sge_dstring_get_string(&attribute_pattern),
                         SGE_ATTR_QNAME, 0)) {
               if (handle_cqueue &&
                   !fnmatch(sge_dstring_get_string(&value_pattern), lGetString(elem, CQ_name), 0)) {
                  printf("%s\n", lGetString(elem, CQ_name));
                  already_handled = true;
               }
            }

            /*
             * Handle special case: hostlist
             */
            if (!already_handled &&
                !fnmatch(sge_dstring_get_string(&attribute_pattern),
                         SGE_ATTR_HOST_LIST, 0)) {
               dstring value = DSTRING_INIT;
               const lList *hostref_list = lGetList(elem, CQ_hostlist);

               if (hostref_list != NULL) {
                  href_list_append_to_dstring(hostref_list, &value);
               } else {
                  sge_dstring_copy_string(&value, "NONE");
               }
               if (handle_cqueue &&
                   !fnmatch(sge_dstring_get_string(&value_pattern), sge_dstring_get_string(&value), 0)) {
                  printf("%s\n", lGetString(elem, CQ_name));
                  already_handled = true;
               }
               sge_dstring_free(&value);
            }

            /*
             * Handle all other CQ attributes
             */
            while (!already_handled &&
                   cqueue_attribute_array[index].cqueue_attr != NoName) {
               if (!fnmatch(sge_dstring_get_string(&attribute_pattern),
                            cqueue_attribute_array[index].name, 0)) {
                  dstring value = DSTRING_INIT;
                  lList *attribute_list = lGetList(elem, cqueue_attribute_array[index].cqueue_attr);
                  lListElem *attribute;

                  already_handled = false;
                  for_each(attribute, attribute_list) {
                     const lDescr *descr = lGetListDescr(attribute_list);
                     lList *tmp_attribute_list = lCreateList("", descr);
                     lListElem *tmp_attribute = lCopyElem(attribute);
                     const char *host_hgroup = lGetHost(attribute, cqueue_attribute_array[index].href_attr);
                     bool is_cqueue;
                     bool is_domain;
                     bool is_qinstance;

                     is_cqueue = (strcmp(host_hgroup, HOSTREF_DEFAULT) == 0) ? true : false;
                     is_domain = false;
                     if (!is_cqueue) {
                        is_domain = is_hgroup_name(host_hgroup);
                     }
                     is_qinstance = (!is_domain && !is_cqueue) ? true : false;

                     lAppendElem(tmp_attribute_list, tmp_attribute);
                     lSetHost(tmp_attribute,
                              cqueue_attribute_array[index].href_attr,
                              HOSTREF_DEFAULT);

                     attr_list_append_to_dstring(tmp_attribute_list,
                                                 &value, cqueue_attribute_array[index].href_attr,
                                                 cqueue_attribute_array[index].value_attr);

                     if (!fnmatch(sge_dstring_get_string(&value_pattern), sge_dstring_get_string(&value), 0)) {
                        if (handle_cqueue && is_cqueue) {
                           printf("%s\n", lGetString(elem, CQ_name));
                        } else if ((handle_domain && is_domain) || (handle_qinstance && is_qinstance)) {
                           printf("%s@%s\n", lGetString(elem, CQ_name), host_hgroup);
                        }
                        already_handled = true;
                     }
                     lFreeList(&tmp_attribute_list);
                     if (already_handled) {
                        break;
                     }
                  }
                  sge_dstring_free(&value);
                  if (already_handled) {
                     break;
                  }
               }
               index++;
            }
         }
         lFreeList(&list);
      }
      sge_dstring_free(&object_name);
      sge_dstring_free(&attribute_pattern);
      sge_dstring_free(&value_pattern);

      spp++;
      continue;
   }

/*---------------------------------------------------------------------------*/

   if ((strcmp("-mattr", *spp) == 0) || (strcmp("-Mattr", *spp) == 0) ||
       (strcmp("-aattr", *spp) == 0) || (strcmp("-Aattr", *spp) == 0) ||   
       (strcmp("-rattr", *spp) == 0) || (strcmp("-Rattr", *spp) == 0) ||   
       (strcmp("-dattr", *spp) == 0) || (strcmp("-Dattr", *spp) == 0)) {   
     
/* *INDENT-OFF* */ 
      /* fixme: can conf, schedconf be added?  (no attribute_name to
         match, and *params are space-separated) */
      static object_info_entry info_entry[] = {
         {SGE_CQ_LIST,         SGE_OBJ_CQUEUE,    CQ_Type,   SGE_ATTR_QNAME,     CQ_name,   NULL,     &qconf_sfi,        cqueue_xattr_pre_gdi},
         {SGE_EH_LIST,         SGE_OBJ_EXECHOST,  EH_Type,   SGE_ATTR_HOSTNAME,  EH_name,   NULL,     &qconf_sfi,        NULL},
         {SGE_PE_LIST,         SGE_OBJ_PE,        PE_Type,   SGE_ATTR_PE_NAME,   PE_name,   NULL,     &qconf_sfi,        NULL},
         {SGE_CK_LIST,         SGE_OBJ_CKPT,      CK_Type,   SGE_ATTR_CKPT_NAME, CK_name,   NULL,     &qconf_sfi,        NULL},
         {SGE_HGRP_LIST,       SGE_OBJ_HGROUP,    HGRP_Type, SGE_ATTR_HGRP_NAME, HGRP_name, NULL,     &qconf_sfi,        NULL},
         {SGE_RQS_LIST,        SGE_OBJ_RQS,       RQS_Type,  SGE_ATTR_RQS_NAME,  RQS_name, NULL,      &qconf_rqs_sfi,    rqs_xattr_pre_gdi},
         {SGE_UU_LIST,         SGE_OBJ_USER,      UU_Type,   SGE_ATTR_USER_NAME, UU_name,  NULL,      &qconf_sfi,        NULL},
         {SGE_PR_LIST,         SGE_OBJ_PROJECT,   PR_Type,   SGE_ATTR_PROJECT_NAME, PR_name, NULL,    &qconf_sfi,        NULL},
         {SGE_CAL_LIST,        SGE_OBJ_CALENDAR,  CAL_Type,  SGE_ATTR_CALENDAR_NAME, CAL_name, NULL,  &qconf_sfi,        NULL},
         {SGE_US_LIST,         SGE_OBJ_USERSET,   US_Type,   SGE_ATTR_USERSET_NAME, US_name, NULL,    &qconf_sfi,        NULL},
         {0,                   NULL,              0,         NULL,               0,         NULL,     NULL,        NULL}
      }; 
/* *INDENT-ON* */
      
      int from_file;
      int index;
      int ret = 0;
      int sub_command = 0;
   
      /* This does not have to be freed later */
      info_entry[0].fields = CQ_fields;
      /* These have to be freed later */
      info_entry[1].fields = sge_build_EH_field_list(false, false, false);
      info_entry[6].fields = sge_build_UU_field_list(false);
      info_entry[7].fields = sge_build_PR_field_list(false);
      /* These do not */
      info_entry[2].fields = PE_fields;
      info_entry[3].fields = CK_fields;
      info_entry[4].fields = HGRP_fields;
      info_entry[5].fields = RQS_fields;
      info_entry[8].fields = CAL_fields;
      info_entry[9].fields = US_fields;
      
      /* no adminhost/manager check needed here */
        
      /* Capital letter => we will read from file */
      if (isupper((*spp)[1])) {
         from_file = 1;
      } else {
         from_file = 0;
      }

      /* Set sub command for co_gdi call */
      if ((*spp)[1] == 'm' || (*spp)[1] == 'M') {
         sub_command = SGE_GDI_CHANGE;
      } else if ((*spp)[1] == 'a' || (*spp)[1] == 'A') {
         sub_command = SGE_GDI_APPEND;
      } else if ((*spp)[1] == 'd' || (*spp)[1] == 'D') {
         sub_command = SGE_GDI_REMOVE;
      } else if ((*spp)[1] == 'r' || (*spp)[1] == 'R') {
         sub_command = SGE_GDI_SET;
      }
      spp = sge_parser_get_next(ctx, spp);

      /* is the objectname given in commandline
         supported by this function */
      index = 0;
      while(info_entry[index].object_name) {
         if (!strcmp(info_entry[index].object_name, *spp)) {
            spp = sge_parser_get_next(ctx, spp);
            break; 
         }
         index++;
      }

      if (!info_entry[index].object_name) {
         fprintf(stderr, "Modification of object "SFQ" not supported\n", *spp);
         sge_free(&(info_entry[1].fields));
         sge_free(&(info_entry[6].fields));
         sge_free(&(info_entry[7].fields));
         DRETURN(1);
      } 

      /* */
      DTRACE;
      ret = qconf_modify_attribute(ctx, &alp, from_file, &spp, &ep, 
                                   sub_command, &(info_entry[index])); 
      lFreeElem(&ep);

      /* Error handling */
      if (ret || lGetNumberOfElem(alp)) {
         int exit = 0;

         for_each(aep, alp) {
            FILE *std_x = NULL;
            
            if (lGetUlong(aep, AN_status) != STATUS_OK) {
               std_x = stderr;
               exit = 1;
            } else {
               std_x = stdout;
            } 
            fprintf(std_x, "%s\n", lGetString(aep, AN_text)); 
         }
         lFreeList(&alp);   
         if (exit) {
            sge_free(&(info_entry[1].fields));
            DRETURN(1);
         }
      }
      
      sge_free(&(info_entry[1].fields));
      sge_free(&(info_entry[6].fields));
      sge_free(&(info_entry[7].fields));
      
      continue;
   }


/*----------------------------------------------------------------------------*/
   /* "-purge" */
   if (strcmp("-purge", *spp) == 0) {

      static object_info_entry info_entry[] = {
         {SGE_CQ_LIST,     SGE_OBJ_CQUEUE,    QR_Type,   SGE_ATTR_QNAME,     QR_name,   NULL,        &qconf_sfi,    cqueue_xattr_pre_gdi},
#ifndef __SGE_NO_USERMAPPING__
         {SGE_USER_MAPPING_LIST, SGE_OBJ_USER_MAPPING, CU_Type, NULL,            CU_name,   NULL,        &qconf_sfi,    cqueue_xattr_pre_gdi},
#endif
         {0,                   NULL,              0,         NULL,               0,         NULL,        NULL, NULL}
      };

      int index = 0;
      char *object_instance = NULL;
      char *object = NULL;
      char *hgroup_or_hostname = NULL;
      char *attr = NULL;
      lListElem *cqueue = NULL;

      /* This does not have to be freed later */
      info_entry[0].fields = CQ_fields;

      spp = sge_parser_get_next(ctx, spp);

      /* is the object given in commandline
         supported by this function */
      index = 0;
      while(info_entry[index].object_name) {
         if (!strcmp(info_entry[index].object_name, *spp)) {
            spp = sge_parser_get_next(ctx, spp);
         break; 
      }
      index++;
   }

   if (!info_entry[index].object_name) {
      ERROR((SGE_EVENT, MSG_QCONF_MODIFICATIONOFOBJECTNOTSUPPORTED_S, *spp));
      DRETURN(1);
   } 

   /* parse command line arguments */
   attr = sge_strdup(NULL, *spp);
   if (attr == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_QCONF_NOATTRIBUTEGIVEN));
      DRETURN(1);
   }
   spp = sge_parser_get_next(ctx, spp);

   object_instance = sge_strdup(NULL, *spp);

   /* object_instance look like queue@host */
   if ((object = sge_strdup(NULL, sge_strtok(object_instance, "@"))) != NULL) {
       hgroup_or_hostname = sge_strdup(NULL, sge_strtok(NULL, NULL));
   }
   
   if (object == NULL || hgroup_or_hostname == NULL) {
      ERROR((SGE_EVENT, MSG_QCONF_GIVENOBJECTINSTANCEINCOMPLETE_S, object_instance));
      sge_free(&attr);
      sge_free(&object_instance);
      sge_free(&object);
      DRETURN(1);
   }
     
   /* queue_instance no longer neede */
   sge_free(&object_instance);

   if (strcmp("@/", hgroup_or_hostname) == 0) {
      ERROR((SGE_EVENT, MSG_QCONF_MODIFICATIONOFHOSTNOTSUPPORTED_S, hgroup_or_hostname));
      sge_free(&attr);
      sge_free(&object);
      sge_free(&hgroup_or_hostname);
      DRETURN(1);
   }

#ifndef __SGE_NO_USERMAPPING__
   if (strcmp(info_entry[index].object_name, SGE_CQ_LIST)) {
#endif

      /* now get the queue, delete the objects and send the queue back to the master */
      cqueue = cqueue_get_via_gdi(ctx, &alp, object);

      if (cqueue == NULL) {
         ERROR((SGE_EVENT, MSG_CQUEUE_DOESNOTEXIST_S, object));  
         sge_free(&attr); 
         sge_free(&object);
         sge_free(&hgroup_or_hostname);
         DRETURN(1);
      }

      parse_name_list_to_cull("attribute list", &lp, US_Type, US_name, attr);
      if (cqueue_purge_host(cqueue, &alp, lp, hgroup_or_hostname) == true) {
         cqueue_add_del_mod_via_gdi(ctx, cqueue, &alp, SGE_GDI_MOD | SGE_GDI_SET_ALL);
      } else {
         WARNING((SGE_EVENT, MSG_PARSE_ATTR_ARGS_NOT_FOUND, attr, hgroup_or_hostname));
      }
      lFreeList(&lp);

#ifndef __SGE_NO_USERMAPPING__
   } else {
      /* Usermapping could be done analoguous to code above */
   }
#endif

      /* Error handling */
      for_each(aep, alp) {
            FILE *std_x = NULL;
            
            if (lGetUlong(aep, AN_status) != STATUS_OK) {
               std_x = stderr;
            } else {
               std_x = stdout;
            } 
            fprintf(std_x, "%s\n", lGetString(aep, AN_text)); 
         }

      if (cqueue != NULL) {
         lFreeElem(&cqueue);
      }

      sge_free(&attr); 
      sge_free(&object);
      sge_free(&hgroup_or_hostname);
      spp++;
      continue;
   }

/*----------------------------------------------------------------------------*/
      /* "-Msconf" */
      if (strcmp("-Msconf", *spp) == 0) {
         qconf_is_manager(ctx, username);
         
         spp = sge_parser_get_next(ctx, spp);

         fields_out[0] = NoName;
         ep = spool_flatfile_read_object(&alp, SC_Type, NULL,
                                         SC_fields, fields_out, true, &qconf_comma_sfi,
                                         SP_FORM_ASCII, NULL, *spp);

         if (answer_list_output(&alp)) {
            lFreeElem(&ep);
         }
         
         if (ep != NULL) {
            missing_field = spool_get_unprocessed_field(SC_fields, fields_out, &alp);
         }

         if (missing_field != NoName) {
            lFreeElem(&ep);
            answer_list_output(&alp);
            sge_parse_return = 1;
         }
         
         if (ep != NULL) {
            lp = lCreateList("scheduler config", SC_Type);
            lAppendElem (lp, ep);
            
            if (!sconf_validate_config (&alp, lp)) {
               lFreeList(&lp);
               answer_list_output(&alp);
            }
         }
         
         /* else we let the check for lp != NULL catch the error below */

         if ((lp == NULL) && (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE))) {
            continue;
         }

         alp = ctx->gdi(ctx, SGE_SC_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) == STATUS_OK)
            fprintf(stderr, "%s\n", MSG_SCHEDD_CHANGEDSCHEDULERCONFIGURATION);
         else { /* error */
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            sge_parse_return = 1;
         }

         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
         
      }

/*----------------------------------------------------------------------------*/
      /* "-msconf"  modify scheduler configuration */

      if (strcmp("-msconf", *spp) == 0) {
         qconf_is_manager(ctx, username);

         /* get the scheduler configuration .. */
         what = lWhat("%T(ALL)", SC_Type);
         alp = ctx->gdi(ctx, SGE_SC_LIST, SGE_GDI_GET, &lp, NULL, what, false);
         lFreeWhat(&what);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            sge_parse_return = 1;
            spp++;
            continue;
         }
         lFreeList(&alp);

         if (!(newlp=edit_sched_conf(ctx, lp, uid, gid)))
            continue;

         lFreeList(&lp);
         what = lWhat("%T(ALL)", SC_Type);
         alp = ctx->gdi(ctx, SGE_SC_LIST, SGE_GDI_MOD, &newlp, NULL, what, false);
         lFreeWhat(&what);
         ep = lFirst(alp);
         answer_exit_if_not_recoverable(ep);
         if (answer_get_status(ep) == STATUS_OK)
            fprintf(stderr, "%s\n", MSG_SCHEDD_CHANGEDSCHEDULERCONFIGURATION);
         else {
            fprintf(stderr, "%s\n", lGetString(ep, AN_text));
            sge_parse_return = 1;
         }
         lFreeList(&newlp);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-mstree", "-Mstree file": modify sharetree */

      if ((strcmp("-mstree", *spp) == 0) || (strcmp("-Mstree", *spp) == 0)) {
         lListElem *unspecified = NULL;
         
         if (strcmp("-mstree", *spp) == 0) {
            qconf_is_manager(ctx, username);

            /* get the sharetree .. */
            what = lWhat("%T(ALL)", STN_Type);
            alp = ctx->gdi(ctx, SGE_STN_LIST, SGE_GDI_GET, &lp, NULL, what, false);
            lFreeWhat(&what);

            aep = lFirst(alp);
            answer_exit_if_not_recoverable(aep);
            if (answer_get_status(aep) != STATUS_OK) {
               fprintf(stderr, "%s\n", lGetString(aep, AN_text));
               spp++;
               continue;
            }
            lFreeList(&alp);
    
            ep = lFirst(lp);
            if (!(ep = edit_sharetree(ctx, ep, uid, gid)))
               continue;

            lFreeList(&lp);
         } else {
            spooling_field *fields = sge_build_STN_field_list(false, true);

            spp = sge_parser_get_next(ctx, spp);
           
            fields_out[0] = NoName;
            ep = spool_flatfile_read_object(&alp, STN_Type, NULL,
                                            fields, fields_out, true,
                                            &qconf_name_value_list_sfi,
                                            SP_FORM_ASCII, NULL, *spp);
            
            if (answer_list_output(&alp)) {
               lFreeElem(&ep);
            }
         
            if (ep != NULL) {
               missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
            }

            sge_free(&fields);
            
            if (missing_field != NoName) {
               lFreeElem(&ep);
               answer_list_output(&alp);
               sge_parse_return = 1;
            }
               
            if (ep == NULL) {
               if (sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE))
                  continue;
            }
         }

         /* Make sure that no nodes are left unspecified.  An unspecified node
          * happens when a node appears in another node's child list, but does
          * not appear itself. */
         unspecified = sge_search_unspecified_node(ep);
         
         if (unspecified != NULL) {
            fprintf(stderr, MSG_STREE_NOVALIDNODEREF_U,
                    sge_u32c(lGetUlong(unspecified, STN_id)));
            fprintf(stderr, "\n");
            sge_parse_return = 1;

            lFreeElem(&ep);
            spp++;
            continue;
         }
         
         newlp = lCreateList("sharetree modify", STN_Type);
         lAppendElem(newlp, ep);

         what = lWhat("%T(ALL)", STN_Type);
         alp = ctx->gdi(ctx, SGE_STN_LIST, SGE_GDI_MOD, &newlp, NULL, what, false);
         lFreeWhat(&what);
         ep = lFirst(alp);
         answer_exit_if_not_recoverable(ep);
         if (answer_get_status(ep) == STATUS_OK)
            fprintf(stderr, "%s\n", MSG_TREE_CHANGEDSHARETREE);
         else {
            fprintf(stderr, "%s\n", lGetString(ep, AN_text));
            sge_parse_return = 1;
         }
         lFreeList(&alp);
         lFreeList(&newlp);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-mu userset,..." */

      if (strcmp("-mu", *spp) == 0) {
         /* check for adminhost and manager privileges */
         qconf_is_manager(ctx, username);

         spp = sge_parser_get_next(ctx, spp);

         /* get userset */
         parse_name_list_to_cull("usersets", &lp, US_Type, US_name, *spp);

         if (edit_usersets(ctx, lp) != 0) {
            sge_parse_return = 1;
         }
         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-Mu fname" */

      if (strcmp("-Mu", *spp) == 0) {
         char* file = NULL;
         const char* usersetname = NULL;
         lList *acl = NULL;

         /* no adminhost/manager check needed here */

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN); 
         }     

          
         /* get userset from file */
         ep = NULL;
         fields_out[0] = NoName;
         ep = spool_flatfile_read_object(&alp, US_Type, NULL,
                                         US_fields, fields_out, true, &qconf_param_sfi,
                                         SP_FORM_ASCII, NULL, file);
         
         if (answer_list_output(&alp)) {
            lFreeElem(&ep);
         }
         
         if (ep != NULL) {
            missing_field = spool_get_unprocessed_field(US_fields, fields_out, &alp);
         }

         if (missing_field != NoName) {
            lFreeElem(&ep);
            answer_list_output(&alp);
            sge_parse_return = 1;
         }

         if ((ep != NULL) &&
            (userset_validate_entries(ep, &alp, 0) != STATUS_OK)) {
            lFreeElem(&ep);
            answer_list_output(&alp);
            sge_parse_return = 1;
         }
            
         if (ep == NULL) {
            sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE); 
         }
         usersetname = lGetString(ep, US_name);
 
         /* get userset from qmaster */
         where = lWhere("%T( %I==%s )", US_Type, US_name, usersetname);
         what = lWhat("%T(ALL)", US_Type);
         alp = ctx->gdi(ctx, SGE_US_LIST, SGE_GDI_GET, &lp, where, what, false);
         lFreeWhere(&where);
         lFreeWhat(&what);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            lFreeList(&alp);
            lFreeElem(&ep);
            lFreeList(&lp);
            DRETURN(1); 
         }

         if (lp == NULL || lGetNumberOfElem(lp) == 0) {
            fprintf(stderr, MSG_PROJECT_XISNOKNWOWNPROJECT_S, usersetname);
            fprintf(stderr, "\n");
            fflush(stdout);
            fflush(stderr);
            lFreeList(&alp);
            lFreeElem(&ep);
            lFreeList(&lp);
            DRETURN(1); 
         }
         lFreeList(&alp);
         lFreeList(&lp);

         acl = lCreateList("modified usersetlist", US_Type); 
         lAppendElem(acl, ep);

         alp = ctx->gdi(ctx, SGE_US_LIST, SGE_GDI_MOD, &acl, NULL, NULL, false);
         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            lFreeList(&alp);
            lFreeList(&acl);
            DRETURN(1);
         } 
         fprintf(stderr, "%s\n", lGetString(aep, AN_text));
         lFreeList(&alp);
         lFreeList(&acl);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-Au fname" */

      if (strcmp("-Au", *spp) == 0) {
         lList *acl = NULL;
         char* file = NULL;

         /* no adminhost/manager check needed here */

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN); 
         }     

          
         /* get userset  */
         ep = NULL;
         fields_out[0] = NoName;
         ep = spool_flatfile_read_object(&alp, US_Type, NULL,
                                         US_fields, fields_out,  true,
                                         &qconf_param_sfi,
                                         SP_FORM_ASCII, NULL, file);
         
         if (answer_list_output(&alp)) {
            lFreeElem(&ep);
         }

         if (ep != NULL) {
            missing_field = spool_get_unprocessed_field(US_fields, fields_out, &alp);
         }
         
         if (missing_field != NoName) {
            lFreeElem(&ep);
            answer_list_output(&alp);
            sge_parse_return = 1;
         }

         if ((ep != NULL) && (userset_validate_entries(ep, &alp, 0) != STATUS_OK)) {
            lFreeElem(&ep);
            answer_list_output(&alp);
            sge_parse_return = 1;
         }
            
         if (ep == NULL) {
            sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE); 
         }

         acl = lCreateList("usersetlist list to add", US_Type); 
         lAppendElem(acl,ep);
         alp = ctx->gdi(ctx, SGE_US_LIST, SGE_GDI_ADD, &acl, NULL, NULL, false);
         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            lFreeList(&alp);
            lFreeList(&acl);
            DRETURN(1);
         } 
         fprintf(stderr, "%s\n", lGetString(aep, AN_text));
         lFreeList(&alp);
         lFreeList(&acl);
         spp++;
         continue;
      }


/*----------------------------------------------------------------------------*/

      /* "-muser username" */

      if (strcmp("-muser", *spp) == 0) {
         qconf_is_manager(ctx, username);

         spp = sge_parser_get_next(ctx, spp);
        
         /* get user */
         where = lWhere("%T( %I==%s )", UU_Type, UU_name, *spp);
         what = lWhat("%T(ALL)", UU_Type);
         alp = ctx->gdi(ctx, SGE_UU_LIST, SGE_GDI_GET, &lp, where, what, false);
         lFreeWhere(&where);
         lFreeWhat(&what);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            lFreeList(&alp);
            lFreeList(&lp);
            sge_parse_return = 1;
            spp++;
            continue;
         }
         lFreeList(&alp);

         if (lp == NULL || lGetNumberOfElem(lp) == 0) {
            fprintf(stderr, MSG_USER_XISNOKNOWNUSER_S, *spp);
            fprintf(stderr, "\n");
            spp++;
            lFreeList(&lp);
            continue;
         }
         ep = lFirst(lp);
         
         /* edit user */
         newep = edit_user(ctx, ep, uid, gid);

         /* if the user name has changed, we need to print an error message */   
         if (newep == NULL || strcmp(lGetString(ep, UU_name), lGetString(newep, UU_name))) {
            fprintf(stderr, MSG_QCONF_CANTCHANGEOBJECTNAME_SS, lGetString(ep, UU_name), lGetString(newep, UU_name));
            fprintf(stderr, "\n");
            lFreeElem(&newep);
            lFreeList(&lp);
            DRETURN(1);
         } else {
            lFreeList(&lp);
            /* send it to qmaster */
            lp = lCreateList("User list to modify", UU_Type); 
            lAppendElem(lp, newep);
            alp = ctx->gdi(ctx, SGE_UU_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
            aep = lFirst(alp);
            answer_exit_if_not_recoverable(aep);
            if (answer_get_status(aep) != STATUS_OK) {
               fprintf(stderr, "%s\n", lGetString(aep, AN_text));
               lFreeList(&alp);
               lFreeList(&lp);
               DRETURN(1);
            } else {
               fprintf(stdout, "%s\n", lGetString(aep, AN_text));
            }
         }
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-mprj projectname" */

      if (strcmp("-mprj", *spp) == 0) {
         qconf_is_manager(ctx, username);

         spp = sge_parser_get_next(ctx, spp);
        
         /* get project */
         where = lWhere("%T( %I==%s )", PR_Type, PR_name, *spp);
         what = lWhat("%T(ALL)", PR_Type);
         alp = ctx->gdi(ctx, SGE_PR_LIST, SGE_GDI_GET, &lp, where, what, false);
         lFreeWhere(&where);
         lFreeWhat(&what);
                  
         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            lFreeList(&alp);
            lFreeList(&lp);
            spp++;
            continue;
         }
         lFreeList(&alp);

         if (lp == NULL || lGetNumberOfElem(lp) == 0) {
            fprintf(stderr, MSG_PROJECT_XISNOKNWOWNPROJECT_S, *spp);
            fprintf(stderr, "\n");
            lFreeList(&lp);
            continue;
         }
         lFreeList(&alp);
         ep = lFirst(lp);
         
         /* edit project */
         newep = edit_project(ctx, ep, uid, gid);

         /* send it to qmaster */
         lFreeList(&lp);
         lp = lCreateList("Project list to modify", PR_Type); 
         lAppendElem(lp, newep);
         alp = ctx->gdi(ctx, SGE_PR_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }




/*----------------------------------------------------------------------------*/

      /* "-Muser file" */

      if (strcmp("-Muser", *spp) == 0) {
         char* file = NULL;
         const char* username = NULL;
         spooling_field *fields = NULL;

         /* no adminhost/manager check needed here */

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN); 
         }

         /* get user from file */
         newep = NULL;
         fields_out[0] = NoName;
         fields = sge_build_UU_field_list(false);
         newep = spool_flatfile_read_object(&alp, UU_Type, NULL,
                                         fields, fields_out, true, &qconf_sfi,
                                         SP_FORM_ASCII, NULL, file);
         
         if (answer_list_output(&alp)) {
            lFreeElem(&newep);
         }
         
         if (newep != NULL) {
            missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
         }

         sge_free(&fields);
         
         if (missing_field != NoName) {
            lFreeElem(&newep);
            answer_list_output(&alp);
         }

         if (newep == NULL) {
            sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE); 
         } 
         
         username = lGetString(newep, UU_name); 
                 
         /* get user */
         where = lWhere("%T( %I==%s )", UU_Type, UU_name, username);
         what = lWhat("%T(ALL)", UU_Type);
         alp = ctx->gdi(ctx, SGE_UU_LIST, SGE_GDI_GET, &lp, where, what, false);
         lFreeWhere(&where);
         lFreeWhat(&what);
                  
         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            lFreeList(&alp);
            lFreeElem(&newep);
            lFreeList(&lp);
            DRETURN(1); 
         }

         if (lp == NULL || lGetNumberOfElem(lp) == 0) {
            fprintf(stderr, MSG_USER_XISNOKNOWNUSER_S, username);
            fprintf(stderr, "\n");
            fflush(stdout);
            fflush(stderr);
            lFreeList(&alp);
            lFreeElem(&newep);
            lFreeList(&lp);
            DRETURN(1); 
         }
         lFreeList(&alp);

         /* send it to qmaster */
         lFreeList(&lp);
         lp = lCreateList("User list to modify", UU_Type); 
         lAppendElem(lp, newep);
         alp = ctx->gdi(ctx, SGE_UU_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
           fprintf(stderr, "%s\n", lGetString(aep, AN_text));
           lFreeList(&alp);
           lFreeList(&lp);
           DRETURN(1);
         } else {
           fprintf(stdout, "%s\n", lGetString(aep, AN_text));
         }
          
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-Mprj file" */

      if (strcmp("-Mprj", *spp) == 0) {
         char* file = NULL;
         const char* projectname = NULL;
         spooling_field *fields = NULL;
   
         /* no adminhost/manager check needed here */

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
            if (!sge_is_file(*spp)) {
               sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN);
            }
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN); 
         }

         /* get project from file */
         newep = NULL;
         fields_out[0] = NoName;
         fields = sge_build_PR_field_list(false);
         newep = spool_flatfile_read_object(&alp, PR_Type, NULL,
                                         fields, fields_out, true, &qconf_sfi,
                                         SP_FORM_ASCII, NULL, file);
         
         if (answer_list_output(&alp)) {
            lFreeElem(&newep);
         }
         
         if (newep != NULL) {
            missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
         }

         sge_free(&fields);
         
         if (missing_field != NoName) {
            lFreeElem(&newep);
            answer_list_output(&alp);
            sge_parse_return = 1;
         }

         if (newep == NULL) {
            sge_error_and_exit(ctx, MSG_FILE_ERRORREADINGINFILE); 
         } 
         
         projectname = lGetString(newep, PR_name); 
                 
         /* get project */
         where = lWhere("%T( %I==%s )", PR_Type, PR_name, projectname);
         what = lWhat("%T(ALL)", PR_Type);
         alp = ctx->gdi(ctx, SGE_PR_LIST, SGE_GDI_GET, &lp, where, what, false);
         lFreeWhere(&where);
         lFreeWhat(&what);
                  
         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            lFreeList(&alp);
            lFreeElem(&newep);
            lFreeList(&lp);
            DRETURN(1); 
         }

         if (lp == NULL || lGetNumberOfElem(lp) == 0) {
            fprintf(stderr, MSG_PROJECT_XISNOKNWOWNPROJECT_S, projectname);
            fprintf(stderr, "\n");
            fflush(stdout);
            fflush(stderr);
            lFreeList(&lp);
            lFreeList(&alp);
            lFreeElem(&newep);
            DRETURN(1); 
         }
         lFreeList(&alp);
         lFreeList(&lp);
         
         /* send it to qmaster */
         lp = lCreateList("Project list to modify", PR_Type); 
         lAppendElem(lp, newep);
         alp = ctx->gdi(ctx, SGE_PR_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
         sge_parse_return |= show_answer_list(alp);
         
         lFreeList(&alp);
         lFreeList(&lp);

         spp++;
         continue;
      }

/*-----------------------------------------------------------------------------*/
      /* "-sc complex_name_list" */

      if (strcmp("-sc", *spp) == 0) {
         lList *answer_list = NULL;

         if (!centry_list_show(ctx, &answer_list)) {
            show_answer(answer_list);
            sge_parse_return = 1;
         }   
         lFreeList(&answer_list);
         spp++;
         continue;
      }

/*-----------------------------------------------------------------------------*/
      /* "-scal calendar_name" */
      if (strcmp("-scal", *spp) == 0) {
         spp = sge_parser_get_next(ctx, spp);

         /* get the existing pe entry .. */
         where = lWhere("%T( %I==%s )", CAL_Type, CAL_name, *spp);
         what = lWhat("%T(ALL)", CAL_Type);
         alp = ctx->gdi(ctx, SGE_CAL_LIST, SGE_GDI_GET, &lp, where, what, false);
         lFreeWhere(&where);
         lFreeWhat(&what);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            sge_parse_return = 1;
            spp++;
            continue;
         }
         lFreeList(&alp);

         if (!lp || lGetNumberOfElem(lp) == 0) {
            fprintf(stderr, MSG_CALENDAR_XISNOTACALENDAR_S, *spp);
            fprintf(stderr, "\n");
            lFreeList(&lp);
            DRETURN(1);
         }

         ep = lFirst(lp);
         filename_stdout = spool_flatfile_write_object(&alp, ep, false,
                                              CAL_fields, &qconf_sfi,
                                              SP_DEST_STDOUT, SP_FORM_ASCII,
                                              NULL, false);
         sge_free(&filename_stdout);
         lFreeList(&lp);
         if (answer_list_output(&alp)) {
            sge_error_and_exit(ctx, NULL);
         }

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-scall" */

      if (strcmp("-scall", *spp) == 0) {
         if (!show_object_list(ctx, SGE_CAL_LIST, CAL_Type, CAL_name, "calendar")) { 
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }

/*-----------------------------------------------------------------------------*/
      /* "-sconf host_list" || "-mconf host_list" || "-aconf host_list" || "-Mconf file_list" || "-Aconf file_list" */
      /* file list is also host list */
      if ((strcmp("-sconf", *spp) == 0) || 
         (strcmp("-aconf", *spp) == 0) || 
         (strcmp("-mconf", *spp) == 0) ||
         (strcmp("-Mconf", *spp) == 0) ||
         (strcmp("-Aconf", *spp) == 0)) {
         typedef enum {
            ACTION_sconf = 0,
            ACTION_aconf,
            ACTION_mconf,
            ACTION_Aconf,
            ACTION_Mconf
         } action_enum;   
         action_enum action = ACTION_sconf;
         char *host_list = NULL;
         int ret, first = 1;
         lListElem *hep;
         const char *host;

         if (!strcmp("-aconf", *spp)) {
            qconf_is_manager(ctx, username);
            action = ACTION_aconf;
         } else if (!strcmp("-mconf", *spp)) {
            qconf_is_manager(ctx, username);
            action = ACTION_mconf;
         } else if (!strcmp("-Aconf", *spp)) {
            action = ACTION_Aconf;
         } else if (!strcmp("-Mconf", *spp)) {
            action = ACTION_Mconf;
         }

         if (!sge_next_is_an_opt(spp))  {
            spp = sge_parser_get_next(ctx, spp);
            host_list = sge_strdup(NULL, *spp);
         } else {
            host_list = sge_strdup(NULL, SGE_GLOBAL_NAME);
         }
            
         /* host_list might look like host1,host2,... */
         hep = lCreateElem(EH_Type);

         for ((cp = sge_strtok(host_list, ",")); cp && *cp;
             (cp = sge_strtok(NULL, ","))) {

            if (!first) {
               fprintf(stdout, "\n");
            }

            /*
            ** it would be uncomfortable if you could only give files in .
            */
            if ((action == ACTION_Aconf || action == ACTION_Mconf) && cp && strrchr(cp, '/')) {
               lSetHost(hep, EH_name, strrchr(cp, '/') + 1);
            } else {
               lSetHost(hep, EH_name, cp);
            }
            
            switch ((ret=sge_resolve_host(hep, EH_name))) {
            case CL_RETVAL_OK:
               break;
            default:
               fprintf(stderr, MSG_SGETEXT_CANTRESOLVEHOST_SS, cp, cl_get_error_text(ret));
               fprintf(stderr, "\n");
               break;
            }
            host = lGetHost(hep, EH_name);

            first = 0;

            if (ret != CL_RETVAL_OK && (action == ACTION_sconf || action == ACTION_aconf) ) {
               sge_parse_return = 1;
               continue;
            }   
               
            if (action == ACTION_sconf) {
               if (print_config(ctx, host) != 0) {
                  sge_parse_return = 1;
               }
            } else if (action == ACTION_aconf) {
               if (add_modify_config(ctx, host, NULL, 1) != 0) {
                  sge_parse_return = 1;
               }
            } else if (action == ACTION_mconf) {
               if (add_modify_config(ctx, host, NULL, 0) != 0) {
                  sge_parse_return = 1;
               }
            } else if (action == ACTION_Aconf) {
               if (add_modify_config(ctx, host, cp, 1) != 0) {
                  sge_parse_return = 1;
               }
            } else if (action == ACTION_Mconf) {
               if (add_modify_config(ctx, host, cp, 2) != 0) {
                  sge_parse_return = 1;
               }
            }

         } /* end for */
         
         sge_free(&host_list);
         lFreeElem(&hep);

         spp++;
         continue;
      }
      
/*-----------------------------------------------------------------------------*/
      /* "-sckpt ckpt_name" */
      if (strcmp("-sckpt", *spp) == 0) {
         spp = sge_parser_get_next(ctx, spp);

         /* get the existing ckpt entry .. */
         where = lWhere("%T( %I==%s )", CK_Type, CK_name, *spp);
         what = lWhat("%T(ALL)", CK_Type);
         alp = ctx->gdi(ctx, SGE_CK_LIST, SGE_GDI_GET, &lp, where, what, false);
         lFreeWhere(&where);
         lFreeWhat(&what);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            sge_parse_return = 1;
            spp++;
            lFreeList(&alp);
            lFreeList(&lp);
            continue;
         }
         lFreeList(&alp);

         if (lp == NULL || lGetNumberOfElem(lp) == 0) {
            fprintf(stderr, MSG_CKPT_XISNOTCHKPINTERFACEDEF_S, *spp);
            fprintf(stderr, "\n");
            lFreeList(&lp);
            DRETURN(1);
         }

         ep = lFirst(lp);
         filename_stdout = spool_flatfile_write_object(&alp, ep, false,
                                             CK_fields, &qconf_sfi,
                                             SP_DEST_STDOUT, SP_FORM_ASCII,
                                             NULL, false);
         sge_free(&filename_stdout);
         lFreeList(&lp);
         if (answer_list_output(&alp)) {
            sge_error_and_exit(ctx, NULL);
         }

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-sckptl" */
      if (strcmp("-sckptl", *spp) == 0) {
         if (!show_object_list(ctx, SGE_CK_LIST, CK_Type, CK_name,
               "ckpt interface definition")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/
      /* "-sconfl" */

      if (strcmp("-sconfl", *spp) == 0) {
         if (!show_object_list(ctx, SGE_CONF_LIST, CONF_Type, CONF_name, "config")) {
            sge_parse_return = 1;
         }
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-dconf config_list" */
      if (strcmp("-dconf", *spp) == 0) {
         char *host_list = NULL;
         lListElem *hep = NULL;
         const char *host = NULL;
         int ret;

         /* no adminhost/manager check needed here */

         if (!sge_next_is_an_opt(spp))  {
            spp = sge_parser_get_next(ctx, spp);

            host_list = sge_strdup(NULL, *spp);
            hep = lCreateElem(EH_Type);

            for ((cp = sge_strtok(host_list, ",")); cp && *cp;
                (cp = sge_strtok(NULL, ","))) {
               
               lSetHost(hep, EH_name, cp);
               
               switch (sge_resolve_host(hep, EH_name)) {
               case CL_RETVAL_OK:
                  break;
               default:
                  fprintf(stderr, MSG_SGETEXT_CANTRESOLVEHOST_S, cp);
                  fprintf(stderr, "\n");
                  sge_parse_return = 1;
                  break;
               }
               host = lGetHost(hep, EH_name);
               ret = delete_config(ctx, host);
               /*
               ** try the unresolved name if this was different
               */
               if (ret && strcmp(cp, host)) {
                  delete_config(ctx, cp);
               }
            } /* end for */

            sge_free(&host_list);
            lFreeElem(&hep);
         }
         else {
            fprintf(stderr, "%s\n", MSG_ANSWER_NEEDHOSTNAMETODELLOCALCONFIG);
            sge_parse_return = 1;
         }

         spp++;
         continue;
      }

/*-----------------------------------------------------------------------------*/
      /* "-se exec_server" */
      if (strcmp("-se", *spp) == 0) {
         spp = sge_parser_get_next(ctx, spp);

         /* resolve host */
         hep = lCreateElem(EH_Type);
         lSetHost(hep, EH_name, *spp);
         
         switch (sge_resolve_host(hep, EH_name)) {
         case CL_RETVAL_OK:
            break;
         default:
            fprintf(stderr, MSG_SGETEXT_CANTRESOLVEHOST_S, lGetHost(hep, EH_name));
            fprintf(stderr, "\n");
            lFreeElem(&hep);
            DRETURN(1);
         }

         host = lGetHost(hep, EH_name);
        
         /* get the existing host entry .. */
         where = lWhere("%T( %Ih=%s )", EH_Type, EH_name, host);
         what = lWhat("%T(ALL)", EH_Type);
         alp = ctx->gdi(ctx, SGE_EH_LIST, SGE_GDI_GET, &lp, where, what, false);
         lFreeWhere(&where);
         lFreeWhat(&what);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            lFreeList(&lp);
            lFreeList(&alp);
            lFreeElem(&hep);
            sge_parse_return = 1;
            spp++;
            continue;
         }
         lFreeList(&alp);

         if (lp == NULL || lGetNumberOfElem(lp) == 0) {
            fprintf(stderr, MSG_EXEC_XISNOTANEXECUTIONHOST_S, host);
            fprintf(stderr, "\n");
            lFreeList(&lp);
            lFreeElem(&hep);
            sge_parse_return = 1; 
            spp++;
            continue;
         }

         lFreeElem(&hep);
         ep = lFirst(lp);
         
         {
            spooling_field *fields = sge_build_EH_field_list(false, true, false);
            filename_stdout = spool_flatfile_write_object(&alp, ep, false, fields, &qconf_sfi,
                                        SP_DEST_STDOUT, SP_FORM_ASCII, NULL,
                                        false);
            lFreeList(&lp);
            sge_free(&fields);
            sge_free(&filename_stdout);
            
            if (answer_list_output(&alp)) {
               sge_parse_return = 1; 
            }
         }

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-secl" */
      if (strcmp("-secl", *spp) == 0) {
         if (show_eventclients(ctx))
            sge_parse_return = 1;
         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-sel" */
      if (strcmp("-sel", *spp) == 0) {
         if (!show_object_list(ctx, SGE_EH_LIST, EH_Type, EH_name, 
               "execution host")) { 
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }
      /* "-sh" */
      if (strcmp("-sh", *spp) == 0) {
         if (!show_object_list(ctx, SGE_AH_LIST, AH_Type, AH_name, 
               "administrative host")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/
      /* "-srqs [rqs_name,...]" */
      if (strcmp("-srqs", *spp) == 0) {
         const char *name = NULL;
         bool ret = true;

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            name = *spp;
         }
         ret = rqs_show(ctx, &alp, name);
         if (!ret) {
            show_answer(alp);
            sge_parse_return = 1;
         }
         lFreeList(&alp);

         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/
      /* "-srqsl " */
      if (strcmp("-srqsl", *spp) == 0) {
         if (!show_object_list(ctx, SGE_RQS_LIST, RQS_Type, RQS_name, "resource quota set list")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/
      /* "-sm" */

      if (strcmp("-sm", *spp) == 0) {
         if (!show_object_list(ctx, SGE_UM_LIST, UM_Type, UM_name, "manager")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-sp pe_name" */
      if (strcmp("-sp", *spp) == 0) {
         spp = sge_parser_get_next(ctx, spp);

         /* get the existing pe entry .. */
         where = lWhere("%T( %I==%s )", PE_Type, PE_name, *spp);
         what = lWhat("%T(ALL)", PE_Type);
         alp = ctx->gdi(ctx, SGE_PE_LIST, SGE_GDI_GET, &lp, where, what, false);
         lFreeWhere(&where);
         lFreeWhat(&what);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            lFreeList(&lp);
            lFreeList(&alp);
            sge_parse_return = 1;
            spp++;
            continue;
         }
         lFreeList(&alp);

         if (lp == NULL || lGetNumberOfElem(lp) == 0) {
            fprintf(stderr,  MSG_PARALLEL_XNOTAPARALLELEVIRONMENT_S , *spp);
            fprintf(stderr, "\n");
            lFreeList(&lp);
            DRETURN(1);
         }

         ep = lFirst(lp);
         
         {
            filename_stdout = spool_flatfile_write_object(&alp, ep, false,
                                                 PE_fields, &qconf_sfi,
                                                 SP_DEST_STDOUT, SP_FORM_ASCII,
                                                 NULL, false);
            lFreeList(&lp);
            sge_free(&filename_stdout);
            
            if (answer_list_output(&alp)) {
               sge_error_and_exit(ctx, NULL);
            }
         }

         spp++;
         continue;
      }
/*-----------------------------------------------------------------------------*/
      /* "-spl" */
      if (strcmp("-spl", *spp) == 0) {
         if (!show_object_list(ctx, SGE_PE_LIST, PE_Type, PE_name,
               "parallel environment")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/
      /* "-so" */

      if (strcmp("-so", *spp) == 0) {
         if (!show_object_list(ctx, SGE_UO_LIST, UO_Type, UO_name, "operator")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-ssconf" */

      if (strcmp("-ssconf", *spp) == 0) {
         /* get the scheduler configuration .. */
         what = lWhat("%T(ALL)", SC_Type);
         alp = ctx->gdi(ctx, SGE_SC_LIST, SGE_GDI_GET, &lp, NULL, what, false);
         lFreeWhat(&what);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            sge_parse_return = 1; 
            spp++;
            continue;
         }
         lFreeList(&alp);
 
         filename_stdout = spool_flatfile_write_object(&alp, lFirst(lp), false, SC_fields,
                                     &qconf_comma_sfi, SP_DEST_STDOUT,
                                     SP_FORM_ASCII, NULL, false);
        
         sge_free(&filename_stdout);
         if (answer_list_output(&alp)) {
            fprintf(stderr, "%s\n", MSG_SCHEDCONF_CANTCREATESCHEDULERCONFIGURATION);
            sge_parse_return = 1; 
            spp++;
            continue;
         }

         lFreeList(&lp);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-sstnode node_path[,...]"  show sharetree node */

      if (strcmp("-sstnode", *spp) == 0) {
         int found = 0;

         spp = sge_parser_get_next(ctx, spp);

         /* get the sharetree .. */
         what = lWhat("%T(ALL)", STN_Type);
         alp = ctx->gdi(ctx, SGE_STN_LIST, SGE_GDI_GET, &lp, NULL, what, false);
         lFreeWhat(&what);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            spp++;
            continue;
         }
         lFreeList(&alp);
 
         ep = lFirst(lp);
         if (!ep) {
            fprintf(stderr, "%s\n", MSG_TREE_NOSHARETREE);
            spp++;
            continue;
         }

         lString2List(*spp, &arglp, STN_Type, STN_name, ", ");

         for_each(argep, arglp) {
            const char *nodepath = lGetString(argep, STN_name);

            if (nodepath) {
               found = show_sharetree_path(ep, nodepath);
            }
         }

         if ( found != 0 ) {
            DRETURN(1);
         }

         lFreeList(&arglp);
         lFreeList(&lp);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-rsstnode node_path[,...]"  show sharetree node */

      if (strcmp("-rsstnode", *spp) == 0) {
         int found = 0;

         spp = sge_parser_get_next(ctx, spp);

         /* get the sharetree .. */
         what = lWhat("%T(ALL)", STN_Type);
         alp = ctx->gdi(ctx, SGE_STN_LIST, SGE_GDI_GET, &lp, NULL, what, false);
         lFreeWhat(&what);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            spp++;
            continue;
         }
         lFreeList(&alp);
 
         ep = lFirst(lp);
         if (!ep) {
            fprintf(stderr, "%s\n", MSG_TREE_NOSHARETREE);
            spp++;
            continue;
         }

         lString2List(*spp, &arglp, STN_Type, STN_name, ", ");

         for_each(argep, arglp) {
            const char *nodepath = NULL;
            nodepath = lGetString(argep, STN_name);
            if (nodepath) {
               show_sharetree_path(ep, nodepath);
            }
         }                                                                      

         if (!found && *(spp+1) == NULL) {
            DRETURN(1);
         }

         lFreeList(&arglp);
         lFreeList(&lp);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-sstree" */

      if (strcmp("-sstree", *spp) == 0) {
         spooling_field *fields = NULL;

         /* get the sharetree .. */
         what = lWhat("%T(ALL)", STN_Type);
         alp = ctx->gdi(ctx, SGE_STN_LIST, SGE_GDI_GET, &lp, NULL, what, false);
         lFreeWhat(&what);

         sge_parse_return |= show_answer_list(alp);
         if (sge_parse_return) {
            lFreeList(&alp);
            lFreeList(&lp);
            spp++;
            continue;
         }
 
         ep = lFirst(lp);

         if (ep == NULL) {
            fprintf(stderr, "%s\n", MSG_OBJ_NOSTREEELEM);
            sge_parse_return = 1;
            lFreeList(&alp);
            lFreeList(&lp);
            spp++;
            continue;
         }

         fields = sge_build_STN_field_list(false, true);
         filename_stdout = spool_flatfile_write_object(&alp, ep, true, fields,
                                     &qconf_name_value_list_sfi,
                                     SP_DEST_STDOUT, SP_FORM_ASCII, 
                                     NULL, false);
         sge_free(&fields);
         sge_free(&filename_stdout);
         sge_parse_return |= show_answer_list(alp);
         if (sge_parse_return) {
            sge_error_and_exit(ctx, NULL);
         }

         lFreeList(&alp);
         lFreeList(&lp);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-sst" This is the documented switch for showing the share
         tree.  "-bonsai" is an undocumented alternative, kept for
         compatibility.  */

      if (strcmp("-sst", *spp) == 0 ||
          strcmp("-bonsai", *spp) == 0) {
         /* get the sharetree .. */
         what = lWhat("%T(ALL)", STN_Type);
         alp = ctx->gdi(ctx, SGE_STN_LIST, SGE_GDI_GET, &lp, NULL, what, false);
         lFreeWhat(&what);

         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            spp++;
            continue;
         }
         lFreeList(&alp);
 
         ep = lFirst(lp);

         if (!ep) {
            fprintf(stderr, "%s\n", MSG_TREE_NOSHARETREE);
            spp++;
            continue;
         } else {
          show_sharetree(ep, NULL);
         }

         lFreeList(&lp);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-ss" */
      if (strcmp("-ss", *spp) == 0) {

         if (!show_object_list(ctx, SGE_SH_LIST, SH_Type, SH_name, 
               "submit host")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }

/*-----------------------------------------------------------------------------*/
      /* "-sss" - show scheduler state */

      if (strcmp("-sss", *spp) == 0) {
         /* ... */
         if (!show_object_list(ctx, SGE_EV_LIST, EV_Type, EV_host, "scheduling host")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/
      /* "-su [list_name[,list_name,...]]" */

      if (strcmp("-su", *spp) == 0) {
         spp = sge_parser_get_next(ctx, spp);
         parse_name_list_to_cull("acl`s to show", &lp, 
               US_Type, US_name, *spp);
         if (print_acl(ctx, lp) != 0) {
            sge_parse_return = 1;
         }
         lFreeList(&lp);

         spp++;
         continue;
      }




/*----------------------------------------------------------------------------*/

#ifndef __SGE_NO_USERMAPPING__
      /* "-sumapl" */
      if (strcmp("-sumapl", *spp) == 0) {
         if (!show_object_list(ctx, SGE_USER_MAPPING_LIST, CU_Type, CU_name, "user mapping entries")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-Mumap user filename" */
      if (strcmp("-Mumap", *spp) == 0) {
         lList *answer_list = NULL;
         char* file = NULL;
        
         /* no adminhost/manager check needed here */

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN); 
         }
         cuser_modify_from_file(ctx, &answer_list, file);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-sumap user"  */
      if (strcmp("-sumap", *spp) == 0) {
         lList *answer_list = NULL;

         spp = sge_parser_get_next(ctx, spp);
         cuser_show(ctx, &answer_list, *spp);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         spp++;
         continue;
      }
#endif

#ifdef __SGE_CENTRY_DEBUG__
      /* "-sce attribute"  */
      if (strcmp("-sce", *spp) == 0) {
         lList *answer_list = NULL;

         spp = sge_parser_get_next(ctx, spp);
         if (!centry_show(&answer_list, *spp)) {
            show_answer(answer_list);
            sge_parse_return = 1;
         }
         lFreeList(&answer_list);
         spp++;
         continue;
      }
#endif

/*----------------------------------------------------------------------------*/

#ifndef __SGE_NO_USERMAPPING__
      /* "-mumap user"  */
      if (strcmp("-mumap", *spp) == 0) {
         lList *answer_list = NULL;

         qconf_is_manager(ctx, username);

         spp = sge_parser_get_next(ctx, spp);
         qconf_is_manager(ctx, username);
         cuser_modify(ctx, &answer_list, *spp);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         spp++;
         continue;
      }
#endif

#ifdef __SGE_CENTRY_DEBUG__

/*----------------------------------------------------------------------------*/

      /* "-mce centry"  */
      if (strcmp("-mce", *spp) == 0) {
         lList *answer_list = NULL;

         qconf_is_manager(ctx, username);

         spp = sge_parser_get_next(ctx, spp);
         qconf_is_manager(ctx, username);
         centry_modify(ctx, &answer_list, *spp);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         spp++;
         continue;
      }

#endif

         
/*----------------------------------------------------------------------------*/

#ifndef __SGE_NO_USERMAPPING__
      /* "-dumap user "  */
      if (strcmp("-dumap", *spp) == 0) {
         lList *answer_list = NULL;

         spp = sge_parser_get_next(ctx, spp);
         qconf_is_manager(ctx, username);
         cuser_delete(ctx, &answer_list, *spp);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         spp++;
         continue;
      }
#endif

/*----------------------------------------------------------------------------*/

#ifdef __SGE_CENTRY_DEBUG__
      /* "-dce attribute "  */
      if (strcmp("-dce", *spp) == 0) {
         lList *answer_list = NULL;
   
         spp = sge_parser_get_next(ctx, spp);
         qconf_is_manager(username);
         centry_delete(&answer_list, *spp);
         sge_parse_return |= show_answer(answer_list); 
         lFreeList(&answer_list);
         spp++;
         continue;
      }

#endif

/*----------------------------------------------------------------------------*/

#ifndef __SGE_NO_USERMAPPING__
      /* "-Aumap user mapfile"  */
      if (strcmp("-Aumap", *spp) == 0) {
         lList *answer_list = NULL;
         char* file = NULL;

         /* no adminhost/manager check needed here */

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN); 
         }
   
         cuser_add_from_file(ctx, &answer_list, file);
         sge_parse_return |= show_answer(answer_list); 
         lFreeList(&answer_list);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-aumap user"  */
      if (strcmp("-aumap", *spp) == 0) {
         lList *answer_list = NULL;

         qconf_is_manager(ctx, username);

         spp = sge_parser_get_next(ctx, spp);
         qconf_is_manager(ctx, username);
         cuser_add(ctx, &answer_list, *spp);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         spp++;
         continue;
      }
#endif

/*----------------------------------------------------------------------------*/

#ifdef __SGE_CENTRY_DEBUG__

/*----------------------------------------------------------------------------*/

      /* "-ace attribute"  */
      if (strcmp("-ace", *spp) == 0) {
         lList *answer_list = NULL;

         qconf_is_manager(ctx, username);

         spp = sge_parser_get_next(ctx, spp);
         qconf_is_manager(ctx, username);
         centry_add(ctx, &answer_list, *spp);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-sce attribute"  */
      if (strcmp("-sce", *spp) == 0) {
         lList *answer_list = NULL;

         spp = sge_parser_get_next(ctx, spp);
         centry_show(ctx, &answer_list, *spp);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-mce centry"  */
      if (strcmp("-mce", *spp) == 0) {
         lList *answer_list = NULL;

         qconf_is_manager(ctx, username);

         spp = sge_parser_get_next(ctx, spp);
         qconf_is_manager(ctx, username);
         centry_modify(ctx, &answer_list, *spp);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         spp++;
         continue;
      }

#endif

      /*
       * Hostgroup parameters
       */

      /* "-shgrpl" */
      if (strcmp("-shgrpl", *spp) == 0) {
         if (!show_object_list(ctx, SGE_HGRP_LIST, HGRP_Type, HGRP_name, 
                          "host group list")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }

      /* "-mhgrp user"  */
      if (strcmp("-mhgrp", *spp) == 0) {
         lList *answer_list = NULL;

         spp = sge_parser_get_next(ctx, spp);
         qconf_is_manager(ctx, username);
         hgroup_modify(ctx, &answer_list, *spp);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }

      /* "-Mhgrp user filename" */
      if (strcmp("-Mhgrp", *spp) == 0) {
         lList *answer_list = NULL;
         char* file = NULL;

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN); 
         }
         qconf_is_manager(ctx, username);
         hgroup_modify_from_file(ctx, &answer_list, file);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }

      /* "-ahgrp group"  */
      if (strcmp("-ahgrp", *spp) == 0) {
         lList *answer_list = NULL;
         char* group = "@template";
         bool is_validate_name = false; /* This boolean is needed to create a
                                           hgrp templete. One could have done
                                           it with a string compare deep in
                                           the code. I prefered this way. */

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            group = *spp;
            is_validate_name = true;
         }
         
         qconf_is_manager(ctx, username);
         hgroup_add(ctx, &answer_list, group, is_validate_name);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }

      /* "-Ahgrp file"  */
      if (strcmp("-Ahgrp", *spp) == 0) {
         lList *answer_list = NULL;
         char* file = NULL;

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN); 
         }
         qconf_is_manager(ctx, username);
         hgroup_add_from_file(ctx, &answer_list, file);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }

      /* "-dhgrp group "  */
      if (strcmp("-dhgrp", *spp) == 0) {
         lList *answer_list = NULL;
   
         spp = sge_parser_get_next(ctx, spp);
         qconf_is_manager(ctx, username);
         hgroup_delete(ctx, &answer_list, *spp);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }

      /* "-shgrp group"  */
      if (strcmp("-shgrp", *spp) == 0) {
         lList *answer_list = NULL;

         spp = sge_parser_get_next(ctx, spp);
         hgroup_show(ctx, &answer_list, *spp);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }

      /* Are these two options still supported?  qconf doesn't recognise them */
      /* "-shgrp_tree" */
      if (strcmp("-shgrp_tree", *spp) == 0) {
         lList *answer_list = NULL;

         spp = sge_parser_get_next(ctx, spp);
         hgroup_show_structure(ctx, &answer_list, *spp, true);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }

      /* "-shgrp_resolved" */
      if (strcmp("-shgrp_resolved", *spp) == 0) {
         lList *answer_list = NULL;

         spp = sge_parser_get_next(ctx, spp);
         hgroup_show_structure(ctx, &answer_list, *spp, false);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }

      /*
       * Cluster Queue parameter
       */

      /* "-sql" */
      if (strcmp("-sql", *spp) == 0) {
         if (!show_object_list(ctx, SGE_CQ_LIST, CQ_Type, CQ_name, "cqueue list")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }

      /* "-mq cqueue"  */
      if (strcmp("-mq", *spp) == 0) {
         lList *answer_list = NULL;

         spp = sge_parser_get_next(ctx, spp);

         qconf_is_manager(ctx, username);
         cqueue_modify(ctx, &answer_list, *spp);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }
  
      /* "-Mq filename"  */ 
      if (strcmp("-Mq", *spp) == 0) {
         lList *answer_list = NULL;
         char* file = NULL;

         qconf_is_manager(ctx, username);
         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN);
         }
         cqueue_modify_from_file(ctx, &answer_list, file);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }

      /* "-aq cqueue"  */
      if (strcmp("-aq", *spp) == 0) {
         lList *answer_list = NULL;
         const char *name = "template";

         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            name = *spp;
         }
         qconf_is_manager(ctx, username);
         cqueue_add(ctx, &answer_list, name);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }

      /* "-Aq filename"  */ 
      if (strcmp("-Aq", *spp) == 0) {
         lList *answer_list = NULL;
         char* file = NULL;

         qconf_is_manager(ctx, username);
         if (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            file = *spp;
         } else {
            sge_error_and_exit(ctx, MSG_FILE_NOFILEARGUMENTGIVEN);
         }
         
         if (!cqueue_add_from_file(ctx, &answer_list, file)) {
            sge_parse_return |= 1;
         }
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }

      /* "-dq cqueue"  */
      if (strcmp("-dq", *spp) == 0) {
         lList *answer_list = NULL;

         spp = sge_parser_get_next(ctx, spp);

         qconf_is_manager(ctx, username);
         cqueue_delete(ctx, &answer_list, *spp);
         sge_parse_return |= show_answer(answer_list);
         lFreeList(&answer_list);
         
         spp++;
         continue;
      }

      /* "-sq [pattern[,pattern,...]]" */
      if (strcmp("-sq", *spp) == 0) {

         while (!sge_next_is_an_opt(spp)) {
            spp = sge_parser_get_next(ctx, spp);
            lString2List(*spp, &arglp, QR_Type, QR_name, ", ");
         }
         
         cqueue_show(ctx, &alp, arglp);
         lFreeList(&arglp);
         sge_parse_return |= show_answer(alp);
         lFreeList(&alp);
         spp++;
         continue;
      }

      /* "-cq wc_queue_list" */
      if (strcmp("-cq", *spp) == 0) {
         spp = sge_parser_get_next(ctx, spp);
         lString2List(*spp, &lp, ID_Type, ID_str, ", ");
         for_each(ep, lp) {
            lSetUlong(ep, ID_action, QI_DO_CLEAN);
         }
         alp = ctx->gdi(ctx, SGE_CQ_LIST, SGE_GDI_TRIGGER, &lp, NULL, NULL, false);
         if (answer_list_has_error(&alp)) {
            sge_parse_return = 1;
         }
         answer_list_on_error_print_or_exit(&alp, stderr);
         lFreeList(&alp);
         lFreeList(&lp);
         spp++;
         continue;
      }

      if (strcmp("-sds", *spp) == 0) {
         lList *answer_list = NULL;

         /* Use cqueue_list_sick()'s return value to set the exit code */
         sge_parse_return |= (cqueue_list_sick(ctx, &answer_list)?0:1);
         show_answer(answer_list);
         lFreeList(&answer_list);
         spp++;
         continue;
      }


/*----------------------------------------------------------------------------*/

#ifdef __SGE_CENTRY_DEBUG__
      /* "-dce attribute "  */
      if (strcmp("-dce", *spp) == 0) {
         lList *answer_list = NULL;
   
         spp = sge_parser_get_next(ctx, spp);
         qconf_is_manager(username);
         centry_delete(&answer_list, *spp);
         sge_parse_return |= show_answer(answer_list); 
         lFreeList(&answer_list);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-ace attribute"  */
      if (strcmp("-ace", *spp) == 0) {
         lList *answer_list = NULL;

         qconf_is_manager(ctx, username);

         spp = sge_parser_get_next(ctx, spp);
         qconf_is_manager(username);
         if (!centry_add(ctx, &answer_list, *spp)) {
            show_answer(answer_list);
            sge_parse_return = 1;
         }   
         lFreeList(&answer_list);
         spp++;
         continue;
      }

#endif

/*----------------------------------------------------------------------------*/

      /* "-suser username" */

      if (strcmp("-suser", *spp) == 0) {
         const char*  user = NULL;
         lList* uList = NULL;
         lListElem* uep = NULL;
         bool first = true;
         spooling_field *fields = NULL;

         spp = sge_parser_get_next(ctx, spp);

         lString2List(*spp, &uList, ST_Type, ST_name , ", ");
         for_each(uep, uList) {
            user = lGetString(uep, ST_name);
            /* get user */
            where = lWhere("%T( %I==%s )", UU_Type, UU_name, user);
            what = lWhat("%T(ALL)", UU_Type);
            alp = ctx->gdi(ctx, SGE_UU_LIST, SGE_GDI_GET, &lp, where, what, false);
            lFreeWhere(&where);
            lFreeWhat(&what);
                        
            if (first) {
               first = false;
            } else {
               printf("\n");
            }
            
            aep = lFirst(alp);
            answer_exit_if_not_recoverable(aep);
            if (answer_get_status(aep) != STATUS_OK) {
               fprintf(stderr, "%s\n", lGetString(aep, AN_text));
               lFreeList(&alp);
               lFreeList(&lp);
               continue;
            }
            lFreeList(&alp);

            if (lp == NULL || lGetNumberOfElem(lp) == 0) {
               fprintf(stderr, MSG_USER_XISNOKNOWNUSER_S, user);
               fprintf(stderr, "\n");
               lFreeList(&lp);
               continue;
            }
            ep = lFirst(lp);
            
            /* print to stdout */
            fields = sge_build_UU_field_list(false);
            filename_stdout = spool_flatfile_write_object(&alp, ep, false, fields, &qconf_param_sfi,
                                                 SP_DEST_STDOUT, SP_FORM_ASCII, 
                                                 NULL, false);
            lFreeList(&lp);
            lFreeList(&alp);
            sge_free(&filename_stdout);
            sge_free(&fields);
         }

         lFreeList(&uList);
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-sprj projectname" */

      if (strcmp("-sprj", *spp) == 0) {
         spooling_field *fields = NULL;

         spp = sge_parser_get_next(ctx, spp);

         /* get project */
         where = lWhere("%T( %I==%s )", PR_Type, PR_name, *spp);
         what = lWhat("%T(ALL)", PR_Type);
         alp = ctx->gdi(ctx, SGE_PR_LIST, SGE_GDI_GET, &lp, where, what, false);
         lFreeWhere(&where);
         lFreeWhat(&what);
                  
         aep = lFirst(alp);
         answer_exit_if_not_recoverable(aep);
         if (answer_get_status(aep) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            lFreeList(&alp);
            lFreeList(&lp);
            spp++;
            sge_parse_return = 1; 
            continue;
         }

         lFreeList(&alp);
         if (lp == NULL || lGetNumberOfElem(lp) == 0) {
            fprintf(stderr, MSG_PROJECT_XISNOKNWOWNPROJECT_S, *spp);
            fprintf(stderr, "\n");
            lFreeList(&lp);
            spp++;
            sge_parse_return = 1; 
            continue;
         }
         ep = lFirst(lp);
         
         /* print to stdout */
         fields = sge_build_PR_field_list(false);
         filename_stdout = spool_flatfile_write_object(&alp, ep, false, fields, &qconf_sfi,
                                              SP_DEST_STDOUT, SP_FORM_ASCII, 
                                              NULL, false);
         lFreeList(&alp);
         lFreeList(&lp);
         sge_free(&filename_stdout);
         sge_free(&fields);

         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-ss" */
      if (strcmp("-ss", *spp) == 0) {
         if (!show_object_list(ctx, SGE_SH_LIST, SH_Type, SH_name, 
               "submit")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }
/*----------------------------------------------------------------------------*/
      /* "-sul" */

      if (strcmp("-sul", *spp) == 0) {
         if (!show_object_list(ctx, SGE_US_LIST, US_Type, US_name, 
               "userset list")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

       /* "-suserl" */

      if (strcmp("-suserl", *spp) == 0) {
         if (!show_object_list(ctx, SGE_UU_LIST, UU_Type, UU_name, 
               "user list")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

       /* "-sprjl" */

      if (strcmp("-sprjl", *spp) == 0) {
         if (!show_object_list(ctx, SGE_PR_LIST, PR_Type, PR_name, 
               "project list")) {
            sge_parse_return = 1; 
         }
         spp++;
         continue;
      }

/*----------------------------------------------------------------------------*/

      /* "-tsm" trigger scheduler monitoring */

      if (strcmp("-tsm", *spp) == 0) {
         /* no adminhost/manager check needed here */
         alp = ctx->tsm(ctx, NULL, NULL);
         answer_list_on_error_print_or_exit(&alp, stderr);
         lFreeList(&alp);
         
         spp++;
         continue;
      } 

/*----------------------------------------------------------------------------*/
      /* "-huh?" */

      ERROR((SGE_EVENT, MSG_ANSWER_INVALIDOPTIONARGX_S, *spp));
      fprintf(stderr, MSG_SRC_X_HELP_USAGE_S , "qconf");
      fprintf(stderr, "\n");
      DRETURN(1);
   }

   lFreeList(&alp);
   DRETURN(sge_parse_return);
}

/***********************************************************************/

static void parse_name_list_to_cull(char *name, lList **lpp, lDescr *dp, int nm, char *s)
{
   char *cp2 = NULL;
   lListElem *ep = NULL;
   int pos;
   int dataType;

   DENTER(TOP_LAYER, "parse_name_list_to_cull");

   
   *lpp = lCreateList(name, dp);
   cp2 = sge_strtok(s, ",");
   ep = lCreateElem(dp);
 
   pos = lGetPosInDescr(dp, nm);
   dataType = lGetPosType(dp,pos);
   switch (dataType) {
      case lStringT:
         DPRINTF(("parse_name_list_to_cull: Adding lStringT type element\n"));
         lSetString(ep, nm, cp2);
         break;
      case lHostT:
         DPRINTF(("parse_name_list_to_cull: Adding lHostT type element\n"));
         lSetHost(ep, nm, cp2);
         break;
      default:
         DPRINTF(("parse_name_list_to_cull: unexpected data type\n"));
         break;
   }
   lAppendElem(*lpp, ep);

   while ((cp2 = sge_strtok(0, ",")) != NULL) {
      ep = lCreateElem(dp);
      switch (dataType) {
         case lStringT:
            DPRINTF(("parse_name_list_to_cull: Adding lStringT type element\n"));
            lSetString(ep, nm, cp2);
            break;
         case lHostT:
            DPRINTF(("parse_name_list_to_cull: Adding lHostT type element\n"));
            lSetHost(ep, nm, cp2);
            sge_resolve_host(ep, EH_name);
            break;
         default:
            DPRINTF(("parse_name_list_to_cull: unexpected data type\n"));
            break;
      }
      lAppendElem(*lpp, ep);
   }

   DRETURN_VOID;
}

/****************************************************************************/
static int sge_next_is_an_opt(char **pptr) 
{
   DENTER(TOP_LAYER, "sge_next_is_an_opt");

   if (!*(pptr+1)) {
      DRETURN(1);
   }

   if (**(pptr+1) == '-') {
      DRETURN(1);
   }

   DRETURN(0);
}

/****************************************************************************/
static int sge_error_and_exit(sge_gdi_ctx_class_t *ctx, const char *ptr) {
   DENTER(TOP_LAYER, "sge_error_and_exit");

   fflush(stderr);
   fflush(stdout);

   if (ptr) {
      fprintf(stderr, "%s\n", ptr);
      fflush(stderr);
   }

   fflush(stderr);
   SGE_EXIT((void **)&ctx, 1);
   DRETURN(1); /* to prevent warning */
}

static bool add_host_of_type(sge_gdi_ctx_class_t *ctx, lList *arglp, u_long32 target)
{
   lListElem *argep=NULL, *ep=NULL;
   lList *lp=NULL, *alp=NULL;
   const char *host = NULL;
   int nm = NoName;
   lDescr *type = NULL;
   char *name = NULL;
   bool ret = true;

   DENTER(TOP_LAYER, "add_host_of_type");

   switch (target) {
      case SGE_SH_LIST:
         nm = SH_name;
         type = SH_Type;
         name = "submit host";
         break;
      case SGE_AH_LIST:
         nm = AH_name;
         type = AH_Type;
         name = "administrative host";
         break;
      default:
         DPRINTF(("add_host_of_type: unexpected type\n"));
         ret = false;
         DRETURN(ret);
   }
   
   for_each(argep, arglp) {
      /* resolve hostname */
      if (sge_resolve_host(argep, nm) != CL_RETVAL_OK) {
         const char* host = lGetHost(argep, nm);
         ret = false;
         if ( host == NULL) {
            host = "";
         }
         fprintf(stderr, MSG_SGETEXT_CANTRESOLVEHOST_S, host);
         fprintf(stderr, "\n");
         continue;
      }
      host = lGetHost(argep, nm);

      /* make a new host element */
      lp = lCreateList("host to add", type);
      ep = lCopyElem(argep);
      lAppendElem(lp, ep);


      /* add the new host to the host list */
      alp = ctx->gdi(ctx, target, SGE_GDI_ADD, &lp, NULL, NULL, false);

      /* report results */
      ep = lFirst(alp);
      answer_exit_if_not_recoverable(ep);
      if (answer_get_status(ep) == STATUS_OK) {
         fprintf(stderr, MSG_QCONF_XADDEDTOYLIST_SS, host, name);
         fprintf(stderr, "\n");
      } else {
         fprintf(stderr, "%s\n", lGetString(ep, AN_text));
      }

      lFreeList(&lp);
      lFreeList(&alp);
   }

   DRETURN(ret);
}

/* ------------------------------------------------------------ */

static bool del_host_of_type(sge_gdi_ctx_class_t *ctx, lList *arglp, u_long32 target )
{
   lListElem *argep=NULL, *ep=NULL;
   lList *lp=NULL, *alp=NULL;
   lDescr *type = NULL;
   bool ret = true;

   DENTER(TOP_LAYER, "del_host_of_type");

   switch (target) {
   case SGE_SH_LIST:
      type = SH_Type;
      break;
   case SGE_AH_LIST:
      type = AH_Type;
      break;
   case SGE_EH_LIST:
      type = EH_Type;
      break;
   }

   for_each(argep, arglp) {

      /* make a new host element */
      lp = lCreateList("host_to_del", type);
      ep = lCopyElem(argep);
      lAppendElem(lp, ep);

      /* delete element */
      alp = ctx->gdi(ctx, target, SGE_GDI_DEL, &lp, NULL, NULL, false);

      /* print results */
      if (answer_list_has_error(&alp)) {
         ret = false;
      }
      answer_list_on_error_print_or_exit(&alp, stderr);

      lFreeList(&alp);
      lFreeList(&lp);
   }

   DRETURN(ret);
}

/* ------------------------------------------------------------ */

static lListElem *edit_exechost(sge_gdi_ctx_class_t *ctx, lListElem *ep, uid_t uid, gid_t gid)
{
   int status;
   lListElem *hep = NULL;
   spooling_field *fields = sge_build_EH_field_list(false, false, false);
   lList *alp = NULL;
   int fields_out[MAX_NUM_FIELDS];
   int missing_field = NoName;

   /* used for generating filenames */
   char *filename = NULL;

   DENTER(TOP_LAYER, "edit_exechost");

   filename = (char *)spool_flatfile_write_object(&alp, ep, false, fields,
                                                  &qconf_sfi, SP_DEST_TMP,
                                                  SP_FORM_ASCII, filename,
                                                  false);
   if (answer_list_output(&alp)) {
      if (filename != NULL) {
         unlink(filename);
         sge_free(&filename);
      }
      sge_free(&fields);
      sge_error_and_exit(ctx, NULL);
   }

   lFreeList(&alp);
   status = sge_edit(filename, uid, gid);

   if (status < 0) {
      unlink(filename);
      sge_free(&filename);
      sge_free(&fields);
      if (sge_error_and_exit(ctx, MSG_PARSE_EDITFAILED))
         DRETURN(NULL);
   }

   if (status > 0) {
      unlink(filename);
      sge_free(&filename);
      sge_free(&fields);
      if (sge_error_and_exit(ctx, MSG_FILE_FILEUNCHANGED))
         DRETURN(NULL);
   }
   
   fields_out[0] = NoName;
   hep = spool_flatfile_read_object(&alp, EH_Type, NULL,
                                   fields, fields_out, true, &qconf_sfi,
                                   SP_FORM_ASCII, NULL, filename);
            
   if (answer_list_output(&alp)) {
      lFreeElem(&hep);
   }

   if (hep != NULL) {
      missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
   }

   if (missing_field != NoName) {
      lFreeElem(&hep);
      answer_list_output(&alp);
   }
   
   unlink(filename);
   sge_free(&filename);
   sge_free(&fields);
   DRETURN(hep);
}

/* ------------------------------------------------------------ */

static lList* edit_sched_conf(sge_gdi_ctx_class_t *ctx, lList *confl, uid_t uid, gid_t gid)
{
   int status;
   char *fname = NULL;
   lList *alp=NULL, *newconfl=NULL;
   lListElem *ep = NULL;
   int fields_out[MAX_NUM_FIELDS];
   int missing_field = NoName;

   DENTER(TOP_LAYER, "edit_sched_conf");

   fname = (char *)spool_flatfile_write_object(&alp, lFirst(confl), false,
                                       SC_fields, &qconf_comma_sfi,
                                       SP_DEST_TMP, SP_FORM_ASCII, 
                                       fname, false);
   if (answer_list_output(&alp)) {
      fprintf(stderr, "%s\n", MSG_SCHEDCONF_CANTCREATESCHEDULERCONFIGURATION);
      sge_free(&fname);
      SGE_EXIT((void **)&ctx, 1);
   }

   status = sge_edit(fname, uid, gid);

   if (status < 0) {
      unlink(fname);
      sge_free(&fname);
      
      if (sge_error_and_exit(ctx, MSG_PARSE_EDITFAILED))
         DRETURN(NULL);
   }

   if (status > 0) {
      unlink(fname);
      sge_free(&fname);
      
      if (sge_error_and_exit(ctx, MSG_FILE_FILEUNCHANGED)) {
         DRETURN(NULL);
      }
   }
   
   fields_out[0] = NoName;
   ep = spool_flatfile_read_object(&alp, SC_Type, NULL,
                                   SC_fields, fields_out, true, &qconf_comma_sfi,
                                   SP_FORM_ASCII, NULL, fname);
            
   if (answer_list_output(&alp)) {
      lFreeElem(&ep);
   }

   if (ep != NULL) {
      missing_field = spool_get_unprocessed_field(SC_fields, fields_out, &alp);
   }

   if (missing_field != NoName) {
      lFreeElem(&ep);
      answer_list_output(&alp);
   }

   if (ep != NULL) {
      newconfl = lCreateList("scheduler config", SC_Type);
      lAppendElem(newconfl, ep);
   }
   
   if ((newconfl != NULL) && !sconf_validate_config(&alp, newconfl)) {
      lFreeList(&newconfl);
      answer_list_output(&alp);
   }
   
   if (newconfl == NULL) {
      fprintf(stderr, MSG_QCONF_CANTREADCONFIG_S, "can't parse config");
      fprintf(stderr, "\n");
      unlink(fname);
      sge_free(&fname);
      SGE_EXIT((void **)&ctx, 1);
   }
   lFreeList(&alp);
   
   unlink(fname);
   sge_free(&fname);

   DRETURN(newconfl);
}

/* ------------------------------------------------------------ */

static lListElem *edit_user(sge_gdi_ctx_class_t *ctx, lListElem *ep, uid_t uid, gid_t gid)
{
   int status;
   lListElem *newep = NULL;
   lList *alp = NULL;
   char *filename = NULL;
   spooling_field *fields = sge_build_UU_field_list(false);
   int fields_out[MAX_NUM_FIELDS];
   int missing_field = NoName;

   DENTER(TOP_LAYER, "edit_user");

   filename = (char *)spool_flatfile_write_object(&alp, ep, false, fields,
                                                  &qconf_sfi, SP_DEST_TMP,
                                                  SP_FORM_ASCII, NULL, false);
   if (answer_list_output(&alp)) {
      if (filename != NULL) {
         unlink(filename);
         sge_free(&filename);
      }
      sge_free(&fields);
      sge_error_and_exit(ctx, NULL);
   }

   lFreeList(&alp);

   status = sge_edit(filename, uid, gid);

   if (status < 0) {
      sge_free(&fields);
      unlink(filename);
      if (sge_error_and_exit(ctx, MSG_PARSE_EDITFAILED))
         DRETURN(NULL);
   }

   if (status > 0) {
      sge_free(&fields);
      unlink(filename);
      if (sge_error_and_exit(ctx, MSG_FILE_FILEUNCHANGED))
         DRETURN(NULL);
   }

   fields_out[0] = NoName;
   newep = spool_flatfile_read_object(&alp, UU_Type, NULL, fields, fields_out,
                                    true, &qconf_sfi, SP_FORM_ASCII, NULL,
                                    filename);
   
   if (answer_list_output(&alp)) {
      lFreeElem(&newep);
   }

   if (newep != NULL) {
      missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
   }

   sge_free(&fields);
   
   if (missing_field != NoName) {
      lFreeElem(&newep);
      answer_list_output(&alp);
   }
   
   unlink(filename);
   sge_free(&filename);
   
   if (!newep) {
      fprintf(stderr, MSG_QCONF_CANTREADX_S, MSG_OBJ_USER);
      fprintf(stderr, "\n");
      SGE_EXIT((void **)&ctx, 1);
   }
   
   DRETURN(newep);
}

/* ------------------------------------------------------------ */
static lListElem *edit_project(sge_gdi_ctx_class_t *ctx, lListElem *ep, uid_t uid, gid_t gid)
{
   int status;
   lListElem *newep = NULL;
   lList *alp = NULL;
   char *filename = NULL;
   spooling_field *fields = sge_build_PR_field_list(false);
   int fields_out[MAX_NUM_FIELDS];
   int missing_field = NoName;

   DENTER(TOP_LAYER, "edit_project");

   filename = (char *)spool_flatfile_write_object(&alp, ep, false, fields,
                                                  &qconf_sfi, SP_DEST_TMP,
                                                  SP_FORM_ASCII, NULL, false);
   if (answer_list_output(&alp)) {
      if (filename != NULL) {
         unlink(filename);
         sge_free(&filename);
      }
      sge_free(&fields);
      sge_error_and_exit(ctx, NULL);
   }

   lFreeList(&alp);

   status = sge_edit(filename, uid, gid);

   if (status < 0) {
      sge_free(&fields);
      unlink(filename);
      if (sge_error_and_exit(ctx, MSG_PARSE_EDITFAILED))
         DRETURN(NULL);
   }

   if (status > 0) {
      sge_free(&fields);
      unlink(filename);
      if (sge_error_and_exit(ctx, MSG_FILE_FILEUNCHANGED))
         DRETURN(NULL);
   }

   fields_out[0] = NoName;
   newep = spool_flatfile_read_object(&alp, PR_Type, NULL, fields, fields_out,
                                    true, &qconf_sfi, SP_FORM_ASCII, NULL,
                                    filename);
   
   if (answer_list_output(&alp)) {
      lFreeElem(&newep);
   }

   if (newep != NULL) {
      missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
   }

   sge_free(&fields);
   
   if (missing_field != NoName) {
      lFreeElem(&newep);
      answer_list_output(&alp);
   }
   
   unlink(filename);
   sge_free(&filename);
   
   if (!newep) {
      fprintf(stderr, MSG_QCONF_CANTREADX_S, MSG_JOB_PROJECT);
      fprintf(stderr, "\n");
      SGE_EXIT((void **)&ctx, 1);
   }
   
   DRETURN(newep);
}

/****************************************************************/
static lListElem *edit_sharetree(sge_gdi_ctx_class_t *ctx, lListElem *ep, uid_t uid, gid_t gid)
{
   int status;
   lListElem *newep = NULL;
   const char *filename = NULL;
   lList *alp = NULL;
   spooling_field *fields = NULL;
   int fields_out[MAX_NUM_FIELDS];
   int missing_field = NoName;
   bool is_missing = false;

   DENTER(TOP_LAYER, "edit_sharetree");

   if (ep == NULL) {
      is_missing = true;
   }
   if (is_missing) {
      ep = getSNTemplate();
   }

   fields = sge_build_STN_field_list(false, true);
   
   filename = spool_flatfile_write_object(&alp, ep, false,
                                          fields, &qconf_name_value_list_sfi,
                                          SP_DEST_TMP, SP_FORM_ASCII,
                                          NULL, false);
   if (is_missing) {
      lFreeElem(&ep);
   }

   if (answer_list_output(&alp)) {
      if (filename != NULL) {
         unlink(filename);
         sge_free(&filename);
      }
      sge_free(&fields);
      sge_error_and_exit(ctx, NULL);
   }

   lFreeList(&alp);

   status = sge_edit(filename, uid, gid);

   if (status < 0) {
      sge_free(&fields);
      unlink(filename);
      sge_free(&filename);
      if (sge_error_and_exit(ctx, MSG_PARSE_EDITFAILED))
         DRETURN(NULL);
   }

   if (status > 0) {
      sge_free(&fields);
      unlink(filename);
      sge_free(&filename);
      if (sge_error_and_exit(ctx, MSG_FILE_FILEUNCHANGED))
         DRETURN(NULL);
   }
   
   fields_out[0] = NoName;
   newep = spool_flatfile_read_object(&alp, STN_Type, NULL,
                                      fields, fields_out,  true,
                                      &qconf_name_value_list_sfi,
                                      SP_FORM_ASCII, NULL, filename);
   
   if (answer_list_output(&alp)) {
      lFreeElem(&newep);
   }

   if (newep != NULL) {
      missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
   }

   sge_free(&fields);
   
   if (missing_field != NoName) {
      lFreeElem(&newep);
      answer_list_output(&alp);
   }

   unlink(filename);
   sge_free(&filename);

   if (newep == NULL) {
      /* JG: TODO: do we need the following output?
       * Isn't an error message already written by spool_flatfile_read_object?
       */
      fprintf(stderr, MSG_QCONF_CANTREADSHARETREEX_S, "");
      fprintf(stderr, "\n");
      SGE_EXIT((void **)&ctx, 1);
   }
   
   DRETURN(newep);
}

/* ------------------------------------------------------------ */

static bool show_object_list(sge_gdi_ctx_class_t *ctx, u_long32 target, lDescr *type, int keynm, char *name) 
{
   lEnumeration *what = NULL;
   lCondition *where = NULL;
   lList *alp = NULL, *lp = NULL;
   lListElem *ep = NULL;
   int pos;
   int dataType;
   bool ret = true;
   
   DENTER(TOP_LAYER, "show_object_list");

   what = lWhat("%T(%I)", type, keynm);

   switch (keynm) {
   case EH_name:
      where = lWhere("%T(!(%Ic=%s || %Ic=%s))",
         type, keynm, SGE_TEMPLATE_NAME, 
               keynm, SGE_GLOBAL_NAME );
      break;
   case CONF_name:
      where = lWhere("%T(!(%I c= %s))",
         type, keynm, SGE_GLOBAL_NAME );
      break;   
   case EV_host:
      where = lWhere("%T(%I==%u))", 
         type, EV_id, EV_ID_SCHEDD);
      break;
   default:
      where = NULL; /* all elements */
      break;
   } 

   alp = ctx->gdi(ctx, target, SGE_GDI_GET, &lp, where, what, false);
   lFreeWhat(&what);
   lFreeWhere(&where);

   lPSortList(lp, "%I+", keynm);

   ep = lFirst(alp);
   answer_exit_if_not_recoverable(ep);
   if (answer_list_output(&alp)) {
      lFreeList(&lp);
      DRETURN(false);
   }

   if (lGetNumberOfElem(lp) > 0) {
      for_each(ep, lp) {
         const char *line = NULL;
         pos = lGetPosInDescr(type, keynm);
         dataType = lGetPosType(type , pos);
         switch(dataType) {
            case lStringT: 
               line = lGetString(ep, keynm);
               if (line && line[0] != COMMENT_CHAR) { 
                  printf("%s\n", lGetString(ep, keynm));
               }
               break;
            case lHostT:
                line = lGetHost(ep, keynm);
               if (line && line[0] != COMMENT_CHAR) { 
                  printf("%s\n", lGetHost(ep, keynm));
               }
               break;
            default:
               DPRINTF(("show_object_list: unexpected data type\n")); 
         }
      }
   } else {
      fprintf(stderr, MSG_QCONF_NOXDEFINED_S, name);
      fprintf(stderr, "\n");
      ret = false;
   }
   
   lFreeList(&alp);
   lFreeList(&lp);
   
   DRETURN(ret);
}

static int show_eventclients(sge_gdi_ctx_class_t *ctx)
{
   lEnumeration *what = NULL;
   lList *alp = NULL, *lp = NULL;
   lListElem *ep = NULL;

   DENTER(TOP_LAYER, "show_eventclients");

   what = lWhat("%T(%I %I %I)", EV_Type, EV_id, EV_name, EV_host);

   alp = ctx->gdi(ctx, SGE_EV_LIST, SGE_GDI_GET, &lp, NULL, what, false);
   lFreeWhat(&what);

   ep = lFirst(alp);
   answer_exit_if_not_recoverable(ep);
   if (answer_get_status(ep) != STATUS_OK) {
      fprintf(stderr, "%s\n", lGetString(ep, AN_text));
      DRETURN(-1);
   }

   if (lp != NULL && lGetNumberOfElem(lp) > 0) {
      lPSortList(lp, "%I+", EV_id);
   
      printf("%8s %-15s %-25s\n",MSG_TABLE_EV_ID, MSG_TABLE_EV_NAME, MSG_TABLE_HOST);
      printf("--------------------------------------------------\n");
      for_each(ep, lp) {
         printf("%8d ", (int)lGetUlong(ep, EV_id));
         printf("%-15s ", lGetString(ep, EV_name));
         printf("%-25s\n", (lGetHost(ep, EV_host) != NULL) ? lGetHost(ep, EV_host) : "-");
      }
   }
   else {
      fprintf(stderr, "%s\n", MSG_QCONF_NOEVENTCLIENTSREGISTERED);
   }
   lFreeList(&alp);
   lFreeList(&lp);
   
   DRETURN(0);
}



/* - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -

   get all acls listed in acl_arg 
   from qmaster and print them

   returns
      0 on success
      1 if arglp contains acl names not present at qmaster
      -1 failed to get any acl
*/
static int print_acl(
sge_gdi_ctx_class_t *ctx,
lList *arglp 
) {
   lList *acls = NULL;
   lListElem *argep = NULL, *ep = NULL;
   int fail = 0;
   const char *acl_name = NULL;
   int first_time = 1;
   const char *filename_stdout;

   DENTER(TOP_LAYER, "print_acl");

   /* get all acls named in arglp, put them into acls */
   if (sge_client_get_acls(ctx, NULL, arglp, &acls)) {
      DRETURN(-1);
   }

   for_each(argep, arglp) {
      acl_name = lGetString(argep, US_name);

      ep = lGetElemStr(acls, US_name, acl_name);
      if (ep == NULL) {
         fprintf(stderr, MSG_SGETEXT_DOESNOTEXIST_SS, "access list", acl_name);
         fprintf(stderr, "\n");
         fail = 1;
      } else {
         lList *alp = NULL;
         
         if (first_time)
            first_time = 0;
         else {
            printf("\n");
         }
         
         filename_stdout = spool_flatfile_write_object(&alp, ep, false, US_fields, &qconf_param_sfi,
                                     SP_DEST_STDOUT, SP_FORM_ASCII, NULL,
                                     false);
         lFreeList(&alp);
         sge_free(&filename_stdout);
      }
   }
   lFreeList(&acls);
   DRETURN(fail);
}

/**************************************************************
   get all usersets listed in arglp 
   from qmaster and edit them

   This is pure client code, so output to stderr/out is allowed

   returns
      0 on success
      1 if arglp contains userset names not present at qmaster
      -1 failed to get any userset
      -2 if failed due to disk error
 **************************************************************/
static int edit_usersets(sge_gdi_ctx_class_t *ctx, lList *arglp)
{
   lList *usersets = NULL;
   lListElem *argep=NULL, *ep=NULL, *aep=NULL, *changed_ep=NULL;
   int status;
   const char *userset_name = NULL;
   lList *alp = NULL, *lp = NULL;
   char *fname = NULL;
   int cmd;
   int fields_out[MAX_NUM_FIELDS];
   int missing_field = NoName;
   uid_t uid = ctx->get_uid(ctx);
   gid_t gid = ctx->get_gid(ctx);

   DENTER(TOP_LAYER, "edit_usersets");

   /* get all usersets named in arglp, put them into usersets */
   if (sge_client_get_acls(ctx, NULL, arglp, &usersets)) {
      DRETURN(-1);
   }

   for_each(argep, arglp) {
      alp = NULL;
      userset_name = lGetString(argep, US_name);

      ep = lGetElemStr(usersets, US_name, userset_name);
      if (ep == NULL) {
         ep = lAddElemStr(&usersets, US_name, userset_name, US_Type);
         /* initialize type field in case of sge */
         lSetUlong(ep, US_type, US_ACL|US_DEPT);
         cmd = SGE_GDI_ADD;
      } else {
         cmd = SGE_GDI_MOD;
      }

      fname = (char *)spool_flatfile_write_object(&alp, ep, false, US_fields,
                                           &qconf_param_sfi, SP_DEST_TMP,
                                           SP_FORM_ASCII, fname, false);
      if (answer_list_output(&alp)) {
         fprintf(stderr, "%s\n", MSG_FILE_ERRORWRITINGUSERSETTOFILE);
         DRETURN(-2);
      }
      
      status = sge_edit(fname, uid, gid);
      
      if (status < 0) {
         unlink(fname);
         fprintf(stderr, "%s\n", MSG_PARSE_EDITFAILED);
         lFreeList(&usersets);
         DRETURN(-2);  /* why should the next edit have more luck */
      }

      if (status > 0) {
         unlink(fname);
         fprintf(stdout, "%s\n", MSG_FILE_FILEUNCHANGED);
         continue;
      }

      fields_out[0] = NoName;
      changed_ep = spool_flatfile_read_object(&alp, US_Type, NULL,
                                      US_fields, fields_out,  true, &qconf_param_sfi,
                                      SP_FORM_ASCII, NULL, fname);
      
      if (answer_list_output(&alp)) {
         lFreeElem(&changed_ep);
      }

      if (changed_ep != NULL) {
         missing_field = spool_get_unprocessed_field(US_fields, fields_out, &alp);
      }

      if (missing_field != NoName) {
         lFreeElem(&changed_ep);
         answer_list_output(&alp);
      }

      if (changed_ep == NULL) {
         fprintf(stderr, MSG_FILE_ERRORREADINGUSERSETFROMFILE_S, fname);
         fprintf(stderr, "\n");
         continue;   /* May be the user made a mistake. Just proceed with 
                        the next */
      }

      /* Create List; append Element; and do a modification gdi call */
      lp = lCreateList("userset list", US_Type);
      lAppendElem(lp, changed_ep);
      alp = ctx->gdi(ctx, SGE_US_LIST, cmd, &lp, NULL, NULL, false);
      lFreeList(&lp);

      for_each(aep, alp) {
         fprintf(stderr, "%s\n", lGetString(aep, AN_text));
      }
      lFreeList(&alp);
   }

   lFreeList(&usersets);
   sge_free(&fname);
   DRETURN(0);
}

/***************************************************************************
  -sconf option 
 ***************************************************************************/
static int print_config(
sge_gdi_ctx_class_t *ctx,
const char *config_name 
) {
   lCondition *where = NULL;
   lEnumeration *what = NULL;
   lList *alp = NULL, *lp = NULL;
   lListElem *ep = NULL;
   int fail=0;
   const char *cfn = NULL;
   spooling_field *fields = NULL;
   
   DENTER(TOP_LAYER, "print_config");

   /* get config */
   if (!strcasecmp(config_name, "global")) {
      cfn = SGE_GLOBAL_NAME;
   } else {
      cfn = config_name;   
   }

   where = lWhere("%T(%Ih=%s)", CONF_Type, CONF_name, cfn);
   what = lWhat("%T(ALL)", CONF_Type);
   alp = ctx->gdi(ctx, SGE_CONF_LIST, SGE_GDI_GET, &lp, where, what, false);
   lFreeWhat(&what);
   lFreeWhere(&where);

   ep = lFirst(alp);
   answer_exit_if_not_recoverable(ep);
   if (answer_get_status(ep) != STATUS_OK) {
      fprintf(stderr, "%s\n", lGetString(ep, AN_text));
      fail = 1;
   } else {
      const char *filename_stdout;

      if (!(ep = lFirst(lp))) {
         fprintf(stderr, MSG_ANSWER_CONFIGXNOTDEFINED_S, cfn);
         fprintf(stderr, "\n");
         lFreeList(&alp);
         lFreeList(&lp);
         DRETURN(1);
      }
      printf("#%s:\n", cfn);
      
      fields = sge_build_CONF_field_list(false);
      filename_stdout = spool_flatfile_write_object(&alp, ep, false, fields, &qconf_sfi,
                                  SP_DEST_STDOUT, SP_FORM_ASCII, NULL, false);
      sge_free(&fields);
      sge_free(&filename_stdout);
      
      if (answer_list_output(&alp)) {
         sge_error_and_exit(ctx, NULL);
      }
   }

   lFreeList(&alp);
   lFreeList(&lp);

   DRETURN(fail);
}

/*------------------------------------------------------------------------*
 * delete_config
 *------------------------------------------------------------------------*/
static int delete_config(
sge_gdi_ctx_class_t *ctx,
const char *config_name 
) {
   lList *alp = NULL, *lp = NULL;
   lListElem *ep = NULL;
   int fail = 0;
   
   DENTER(TOP_LAYER, "delete_config");

   lAddElemHost(&lp, CONF_name, config_name, CONF_Type);
   alp = ctx->gdi(ctx, SGE_CONF_LIST, SGE_GDI_DEL, &lp, NULL, NULL, false);

   ep = lFirst(alp);
   fprintf(stderr, "%s\n", lGetString(ep, AN_text));

   answer_exit_if_not_recoverable(ep);
   fail = !(answer_get_status(ep) == STATUS_OK);

   lFreeList(&alp);
   lFreeList(&lp);

   DRETURN(fail);
}

/*------------------------------------------------------------------------*
 * add_modify_config
 ** flags = 1 = add, 2 = modify, 3 = modify if exists, add if not
 *------------------------------------------------------------------------*/
static int add_modify_config(sge_gdi_ctx_class_t *ctx, const char *cfn, const char *filename, u_long32 flags)
{
   lCondition *where = NULL;
   lEnumeration *what = NULL;
   lList *alp = NULL, *lp = NULL;
   lListElem *ep = NULL;
   int failed=0;
   char *tmpname = NULL;
   int status;
   spooling_field *fields = NULL;
   int fields_out[MAX_NUM_FIELDS];
   int missing_field = NoName;
   uid_t uid = ctx->get_uid(ctx);
   gid_t gid = ctx->get_gid(ctx);
   
   DENTER(TOP_LAYER, "add_modify_config");

   where = lWhere("%T(%Ih=%s)", CONF_Type, CONF_name, cfn);
   what = lWhat("%T(ALL)", CONF_Type);
   alp = ctx->gdi(ctx, SGE_CONF_LIST, SGE_GDI_GET, &lp, where, what, false);
   lFreeWhat(&what);
   lFreeWhere(&where);

   failed = false;
   ep = lFirst(alp);
   answer_exit_if_not_recoverable(ep);
   if (answer_get_status(ep) != STATUS_OK) {
      fprintf(stderr, "%s\n", lGetString(ep, AN_text));
      lFreeList(&alp);
      lFreeList(&lp);
      DRETURN(1);
   }

   lFreeList(&alp);

   ep = lCopyElem(lFirst(lp));
   lFreeList(&lp);

   if (ep && (flags == 1)) {
      fprintf(stderr, MSG_ANSWER_CONFIGXALREADYEXISTS_S, cfn);
      fprintf(stderr, "\n");
      lFreeElem(&ep);
      DRETURN(2);
   }
   if (!ep && (flags == 2)) {
      fprintf(stderr, MSG_ANSWER_CONFIGXDOESNOTEXIST_S, cfn);
      fprintf(stderr, "\n");
      lFreeElem(&ep);
      DRETURN(3);
   }

   if (filename == NULL) {
      bool failed = false;

      /* get config or make an empty config entry if none exists */
      if (ep == NULL) {
         ep = lCreateElem(CONF_Type);
         lSetHost(ep, CONF_name, cfn);
      }

      fields = sge_build_CONF_field_list(false);
      tmpname = (char *)spool_flatfile_write_object(&alp, ep, false, fields,
                                            &qconf_sfi, SP_DEST_TMP, SP_FORM_ASCII, 
                                            tmpname, false);

      lFreeElem(&ep);
      status = sge_edit(tmpname, uid, gid);

      if (status != 0) {
         unlink(tmpname);
         failed = true;
         sge_free(&fields);
         sge_free(&tmpname);
      }
      if (status < 0) {
         fprintf(stderr, "%s\n", MSG_PARSE_EDITFAILED);
         sge_free(&fields);
         DRETURN(failed);
      }
      else if (status > 0) {
         fprintf(stderr, "%s\n", MSG_ANSWER_CONFIGUNCHANGED);
         sge_free(&fields);
         DRETURN(failed);
      }

      fields_out[0] = NoName;
      ep = spool_flatfile_read_object(&alp, CONF_Type, NULL,
                                      fields, fields_out, false, &qconf_sfi,
                                      SP_FORM_ASCII, NULL, tmpname);

      if (answer_list_output(&alp)) {
         lFreeElem(&ep);
         failed = true;
      }

      if (ep != NULL) {
         missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
      }

      sge_free(&fields);

      if (missing_field != NoName) {
         lFreeElem(&ep);
         answer_list_output(&alp);
         failed = true;
      }

      /* If the configuration is legitematly NULL, create an empty object. */
      if (!failed && (ep == NULL)) {
         ep = lCreateElem(CONF_Type);
      }

      if (ep != NULL) {
         lSetHost(ep, CONF_name, cfn);
      } else {
         fprintf(stderr, "%s\n", MSG_ANSWER_ERRORREADINGTEMPFILE);
         unlink(tmpname);
         sge_free(&tmpname);
         failed = true;
         DRETURN(failed);
      }
      unlink(tmpname);
      sge_free(&tmpname);
   } else {
      lFreeElem(&ep);

      fields_out[0] = NoName;
      fields = sge_build_CONF_field_list(false);
      ep = spool_flatfile_read_object(&alp, CONF_Type, NULL,
                                      fields, fields_out, false, &qconf_sfi,
                                      SP_FORM_ASCII, NULL, filename);

      if (answer_list_output(&alp)) {
         lFreeElem(&ep);
      }

      if (ep != NULL) {
         missing_field = spool_get_unprocessed_field(fields, fields_out, &alp);
      }

      sge_free(&fields);

      if (missing_field != NoName) {
         lFreeElem(&ep);
         answer_list_output(&alp);
      }

      if (ep != NULL) {
         lSetHost(ep, CONF_name, cfn);
      }

      if (!ep) {
         fprintf(stderr, MSG_ANSWER_ERRORREADINGCONFIGFROMFILEX_S, filename);
         fprintf(stderr, "\n");
         failed = true;
         DRETURN(failed);
      }
   }

   lp = lCreateList("modified configuration", CONF_Type); 
   lAppendElem(lp, ep);

   alp = ctx->gdi(ctx, SGE_CONF_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
   lFreeList(&lp);

   /* report results */
   failed = show_answer_list(alp);

   lFreeList(&alp);

   DRETURN(failed);
}




/*
** NAME
**   qconf_is_manager
** PARAMETER
**   user     - user name to check
** RETURN
**   
** EXTERNAL
**
** DESCRIPTION
**   retrieves manager list from qmaster and checks if the
**   given user is manager
*/
static int qconf_is_manager(sge_gdi_ctx_class_t *ctx, const char *user)
{
   int perm_return;
   lList *alp = NULL;

   DENTER(TOP_LAYER, "qconf_is_manager");

   
   if (!user || !*user) {
      /* no input name */
      DRETURN(-1);
   }
   perm_return = sge_gdi2_check_permission(ctx, &alp, MANAGER_CHECK);
   if (perm_return == true) {
     /* user is manager */
     if (alp != NULL) {
        lFreeList(&alp);
        alp = NULL;
     }
     DRETURN(1);
   }

   /*
   ** user is no manager
   */
   if (perm_return == -10 ) {
      /* fills SGE_EVENT with diagnosis information */
      if (alp != NULL) {
         lListElem *aep;
         if (lGetUlong(aep = lFirst(alp), AN_status) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
         }
      }
   } else {
      /* fixme: print real uname in case of fakeroot &c */
      fprintf(stderr, MSG_SGETEXT_MUSTBEMANAGER_S, user);
      fprintf(stderr, "\n");
   }

   if (alp != NULL) {
      lFreeList(&alp);
      alp = NULL;
   }

   SGE_EXIT((void **)&ctx, 1);
   DRETURN(0);
}



/*
** NAME
**   qconf_is_adminhost
** PARAMETER
**   host     -  resolved hostname to check
** RETURN
**   
** EXTERNAL
**
** DESCRIPTION
**   retrieves adminhost list from qmaster and checks if the
**   given host is contained 
*/
static int qconf_is_adminhost(sge_gdi_ctx_class_t *ctx, const char *host)
{
   lCondition *where = NULL;
   lEnumeration *what = NULL;
   lList *alp = NULL;
   lListElem *aep = NULL;
   lList *lp = NULL;
   lListElem  *ep = NULL;

   DENTER(TOP_LAYER, "qconf_is_adminhost");

   if (!host || !*host) {
      DRETURN(-1);
   }

   DPRINTF(("host: '%s'\n", host));

   /*
   ** GET SGE_AH_LIST 
   */
   where = lWhere("%T(%Ih=%s)", AH_Type, AH_name, host);
   what = lWhat("%T(ALL)", AH_Type);
   alp = ctx->gdi(ctx, SGE_AH_LIST, SGE_GDI_GET, &lp, where, what, false);
   lFreeWhat(&what);
   lFreeWhere(&where);

   if (!alp) {
      SGE_EXIT((void **)&ctx, 1);
   }
   if (lGetUlong(aep = lFirst(alp), AN_status) != STATUS_OK) {
      fprintf(stderr, "%s\n", lGetString(aep, AN_text));
      lFreeList(&alp);
      SGE_EXIT((void **)&ctx, 1);
   }
   lFreeList(&alp);

   ep = host_list_locate(lp, host);

   if (!ep) {
      /*
      ** host is no adminhost
      */
      lFreeList(&lp);
      fprintf(stderr, MSG_ANSWER_DENIEDHOSTXISNOADMINHOST_S, host);
      fprintf(stderr, "\n");
      SGE_EXIT((void **)&ctx, 1);
   }

   lFreeList(&lp);
   DRETURN(0);
}

/****** src/qconf_modify_attribute() ******************************************
*  NAME
*     qconf_modify_attribute() -- sends a modify request to the master 
*
*  SYNOPSIS
*
*     static int qconf_modify_attribute(lList **alpp, int from_file,
*                                        char ***spp, int sub_command,
*                                        struct object_info_entry *info_entry); 
*
*
*  FUNCTION
*     The function performs a SGE_GDI_MOD request to the qmaster.
*     It will get all necessary information from the command line or a
*     file. 
*     Depending on the parameters specified, the function may
*     modify only parts of an object. It is possible to address
*     only parts of a sublist of an object.
*     
*
*  INPUTS
*     alpp        - reference to an answer list where the master will
*                 store necessary messages for the user 
*
*     from_file   - if set to 1 then the next command line parameter
*                   (stored in spp) will contain a filename. 
*                   This file contains the 
*                   attributes which should be modified 
*
*     spp         - pending list of command line parameter
*
*     epp         - this reference will contain the reduced
*                   element which will be parsed from command line or file
*                   after this function was called
*
*     sub_command - bitmask which will be added to the "command"
*                   parameter of the sge_gdi-request: 
*        SGE_GDI_CHANGE - modify sublist elements
*        SGE_GDI_APPEND - add elements to a sublist
*        SGE_GDI_REMOVE - remove sublist elements
*        SGE_GDI_SET - replace the complete sublist
*
*     info_entry -  pointer to a structure with function 
*                   pointers, string pointers, and CULL names
*
*  RESULT
*     [alpp] Masters answer for the gdi request
*     1 for error
*     0 for success
******************************************************************************/
static int qconf_modify_attribute(sge_gdi_ctx_class_t *ctx,
                                  lList **alpp, int from_file, char ***spp,
                                  lListElem **epp, int sub_command, 
                                  struct object_info_entry *info_entry) 
{
   int fields[150];
   lListElem *add_qp = NULL;
   lList *qlp = NULL;
   lEnumeration *what = NULL;
   
   DENTER(TOP_LAYER, "qconf_modify_attribute");    

   DTRACE;

   fields[0] = NoName;

   if (from_file) {
      if (sge_next_is_an_opt(*spp))  {
         SGE_ADD_MSG_ID( sprintf(SGE_EVENT, 
            MSG_ANSWER_MISSINGFILENAMEASOPTIONARG_S, "qconf"));
         answer_list_add(alpp, SGE_EVENT, 
                         STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
         DRETURN(1);
      }                
      DTRACE;
      *epp = spool_flatfile_read_object(alpp, info_entry->cull_descriptor,
                                        NULL, info_entry->fields, fields,
                                        true, info_entry->instr, SP_FORM_ASCII,
                                        NULL, **spp);
            
      if (answer_list_output(alpp)) {
         DRETURN(1);
      }
      if (*epp == NULL){
         SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_FILE_ERRORREADINGINFILE));
         answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
         DRETURN(1);
      }
      DTRACE;
   } else {
      const char *name = NULL;
      const char *value = NULL;
      const char *filename = NULL;
      dstring delim = DSTRING_INIT;

      /* attribute name to be [admr] for info_entry obj */
      name = (const char *)strdup(**spp);
      *spp = sge_parser_get_next(ctx, *spp);
      /* attribute value to be [admr] for info_entry obj */
      value = (const char *)strdup(**spp);

      if (!strcmp(info_entry->object_name, SGE_OBJ_RQS) && !strcmp(name, "limit")) {
         sge_dstring_append(&delim, " to ");
      } else {
         sge_dstring_append_char(&delim, info_entry->instr->name_value_delimiter);
      }
      {
         dstring write_attr_tmp_file_error = DSTRING_INIT;
         filename = write_attr_tmp_file(name, value, sge_dstring_get_string(&delim), &write_attr_tmp_file_error);
         if (filename == NULL && sge_dstring_get_string(&write_attr_tmp_file_error) != NULL) {
            answer_list_add_sprintf(alpp, STATUS_EDISK, ANSWER_QUALITY_ERROR,
                                    "%s", sge_dstring_get_string(&write_attr_tmp_file_error));
         } else {
            *epp = spool_flatfile_read_object(alpp, info_entry->cull_descriptor, NULL,
                                      info_entry->fields, fields, true, info_entry->instr,
                                      SP_FORM_ASCII, NULL, filename);
            unlink(filename);
            sge_free(&filename);
         }
         sge_dstring_free(&write_attr_tmp_file_error);
      }

      /* Bugfix: Issuezilla #1005
       * Since we're writing the information from the command line to a file so
       * we can read it, the error messages that come back from the flatfile
       * spooling will sound a little odd.  To avoid this, we hijack the answer
       * list and replace the error messages with ones that will make better
       * sense. */
      if (answer_list_has_error(alpp)) {
         lListElem *aep = NULL;
         
         for_each(aep, *alpp) {
            if (answer_has_quality(aep, ANSWER_QUALITY_ERROR) &&
                (answer_get_status(aep) == STATUS_ESYNTAX)) {
               sprintf(SGE_EVENT, MSG_PARSE_BAD_ATTR_ARGS_SS, name, value);
               lSetString(aep, AN_text, SGE_EVENT);
            }
         }
         
         sge_free(&name);
         sge_free(&value);
         sge_dstring_free(&delim);
         
         DRETURN(1);
      }
      
      sge_free(&name);
      sge_free(&value);
      sge_dstring_free(&delim);
   }

   /* add object name to int vector and transform
      it into an lEnumeration */
   if (add_nm_to_set(fields, info_entry->nm_name) < 0) {
      SGE_ADD_MSG_ID( sprintf(SGE_EVENT, MSG_QCONF_CANTCHANGEOBJECTNAME_SS, "qconf", 
         info_entry->attribute_name));
      answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      lFreeElem(epp);
      DRETURN(1);
   }

   if (!(what = lIntVector2What(info_entry->cull_descriptor, fields))) {
      SGE_ADD_MSG_ID( sprintf(SGE_EVENT, MSG_QCONF_INTERNALFAILURE_S, "qconf"));
      lFreeElem(epp);
      DRETURN(1);
   }     

   while (!sge_next_is_an_opt(*spp)) { 
      *spp = sge_parser_get_next(ctx, *spp);
      if (!qlp)
         qlp = lCreateList("list", info_entry->cull_descriptor);
      add_qp = lCopyElem(*epp);
      switch(lGetType(add_qp->descr, info_entry->nm_name)) {
         case lUlongT:
            lSetUlong(add_qp, info_entry->nm_name, atol(**spp));
            break;
         case lLongT:
            lSetLong(add_qp, info_entry->nm_name, atol(**spp));
            break;
         case lIntT:
            lSetInt(add_qp, info_entry->nm_name, atoi(**spp));
            break;
         case lFloatT:
            lSetDouble(add_qp, info_entry->nm_name, atof(**spp));
            break;
         case lDoubleT:
            lSetDouble(add_qp, info_entry->nm_name, atof(**spp));
            break;
         case lCharT:
            lSetChar(add_qp, info_entry->nm_name, **spp[0]);
            break;
         case lStringT:
            lSetString(add_qp, info_entry->nm_name, **spp);
            break;
         case lHostT:   
            lSetHost(add_qp, info_entry->nm_name, **spp);
            break;
         default:
            SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_QCONF_INTERNALFAILURE_S, "qconf"));
            answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
            DRETURN(1);
      }
      lAppendElem(qlp, add_qp);
   }

   if (!qlp) {
      SGE_ADD_MSG_ID( sprintf(SGE_EVENT, MSG_QCONF_MQATTR_MISSINGOBJECTLIST_S, 
         "qconf"));
      answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      lFreeElem(epp);
      lFreeWhat(&what);
      DRETURN(1);
   }

   /* Bugfix: Issuezilla #1025
    * If we get a list value of NONE with -mattr, complain.  This comes after
    * the complaint about not including the object list on purpose.  That order
    * seems to make the most sense for error reporting, even though it means we
    * do some unnecessary work. */
   if (SGE_GDI_IS_SUBCOMMAND_SET(sub_command, SGE_GDI_CHANGE) && (lGetType((*epp)->descr, fields[0]) == lListT)) {
      lList *lp = lGetList(*epp, fields[0]);
      
      if (lp == NULL || lGetNumberOfElem(lp) == 0) {
         SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_QCONF_CANT_MODIFY_NONE));
         answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
         lFreeElem(epp);
         lFreeWhat(&what);
         lFreeList(&qlp);
         lFreeList(&lp);
         DRETURN(1);
      } else if (lGetNumberOfElem(lp) == 1) {
         lListElem *ep = lFirst(lp);
         int count = 1;
         int nm = 0;

         /* Look for the list and see if it's NULL.  These should all be very
          * simple CULL structures which should only contain an lHostT and an
          * lListT. */
         for (nm = ep->descr[0].nm; nm != NoName; nm = ep->descr[count++].nm) {            
            if (lGetType(ep->descr, nm) == lListT) {
               if (lGetList(ep, nm) == NULL) {
                  SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_QCONF_CANT_MODIFY_NONE));
                  answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
                  lFreeElem(epp);
                  lFreeWhat(&what);
                  lFreeList(&qlp);
                  DRETURN(1);
               }
            }
         }
      }
   }
   
   if (info_entry->pre_gdi_function == NULL || 
      info_entry->pre_gdi_function(qlp, alpp)) {
      *alpp = ctx->gdi(ctx, info_entry->target, SGE_GDI_MOD | sub_command, &qlp, 
                      NULL, what, false);
   }
   lFreeElem(epp);
   lFreeWhat(&what);
   lFreeList(&qlp);
   (*spp)++;

   DRETURN(0);
}

static const char *write_attr_tmp_file(const char *name, const char *value, 
                                       const char *delimiter, dstring *error_message)
{
   char *filename = sge_malloc(sizeof(char) * SGE_PATH_MAX);
   FILE *fp;
   int fd;
   DENTER(TOP_LAYER, "write_attr_tmp_file");

   if ((fd = sge_mkstemp(filename, sizeof(char) * SGE_PATH_MAX, error_message)) < 0) {
      sge_dstring_sprintf(error_message, MSG_ERROROPENINGFILEFORWRITING_SS, filename, sge_dstring_get_string(error_message));
      DRETURN(NULL);
   }
   
   if ((fp = fdopen (fd, "w"))) {
      fprintf(fp, "%s", name);
      fprintf(fp, "%s", delimiter);
      fprintf(fp, "%s\n", value);
   } else {
      close(fd);
      DRETURN(NULL);
   }
   
   FCLOSE(fp);
   
   DRETURN((const char *)filename);
FCLOSE_ERROR:
   DRETURN(NULL);
}
