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
 *  Portions of this code are Copyright 2011 Univa Corporation.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "uti/sge_rmon.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_prog.h"
#include "uti/sge_unistd.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_hostname.h"
#include "uti/sge_log.h"
#include "uti/sge_profiling.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_cull_xml.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"

#include "sched/sge_select_queue.h"

#include "comm/commlib.h"

#include "sge.h"
#include "basis_types.h"
#include "sig_handlers.h"
#include "qstat_printing.h"
#include "sge_qhost.h"
#include "msg_common.h"
#include "msg_clients_common.h"
#include "msg_qhost.h"

extern char **environ;

static bool sge_parse_cmdline_qhost(char **argv, char **envp, lList **ppcmdline, lList **alpp);
static int sge_parse_qhost(lList **ppcmdline, lList **pplres, lList **ppFres, lList **pphost, lList **ppuser, u_long32 *show, qhost_report_handler_t **report_handler, lList **alpp);
static bool qhost_usage(FILE *fp);

static qhost_report_handler_t* xml_report_handler_create(lList **alpp);
static int xml_report_handler_destroy(qhost_report_handler_t** handler, lList **alpp);
static int xml_report_finished(qhost_report_handler_t* handler, lList **alpp);
static int xml_report_started(qhost_report_handler_t* handler, lList **alpp);
static int xml_report_host_begin(qhost_report_handler_t* handler, const char* host_name, lList **alpp);
static int xml_report_host_string_value(qhost_report_handler_t* handler, const char* name, const char *value, lList **alpp);
static int xml_report_host_ulong_value(qhost_report_handler_t* handler, const char* name, u_long32 value, lList **alpp);
static int xml_report_host_finished(qhost_report_handler_t* handler, const char* host_name, lList **alpp);
static int xml_report_resource_value(qhost_report_handler_t* handler, const char* dominance, const char* name, const char* value, lList **alpp);
static int xml_report_queue_begin(qhost_report_handler_t* handler, const char* qname, lList **alpp);
static int xml_report_queue_string_value(qhost_report_handler_t* handler, const char* qname, const char* name, const char *value, lList **alpp);
static int xml_report_queue_ulong_value(qhost_report_handler_t* handler, const char* qname, const char* name, u_long32 value, lList **alpp);
static int xml_report_queue_finished(qhost_report_handler_t* handler, const char* qname, lList **alpp);
static int xml_report_job_begin(qhost_report_handler_t* handler, const char *qname, const char* jobname, lList **alpp);
static int xml_report_job_string_value(qhost_report_handler_t* handler, const char *qname, const char* jobname, const char* name, const char *value, lList **alpp);
static int xml_report_job_ulong_value(qhost_report_handler_t* handler, const char *qname, const char* jobname, const char* name, u_long32 value, lList **alpp);
static int xml_report_job_double_value(qhost_report_handler_t* handler, const char *qname, const char* jobname, const char* name, double value, lList **alpp);
static int xml_report_job_finished(qhost_report_handler_t* handler, const char *qname, const char* jobname, lList **alpp);




static int xml_report_started(qhost_report_handler_t* handler, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_started");

   printf("<?xml version='1.0'?>\n");
   printf("<qhost xmlns:xsd=\"http://arc.liv.ac.uk/repos/darcs/sge/source/dist/util/resources/schemas/qhost/qhost.xsd\">\n");
   
   DRETURN(QHOST_SUCCESS);
}

static int xml_report_finished(qhost_report_handler_t* handler, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_finished");
   
   printf("</qhost>\n");
   
   DRETURN(QHOST_SUCCESS);
}

static qhost_report_handler_t* xml_report_handler_create(lList **alpp)
{
   qhost_report_handler_t* ret = (qhost_report_handler_t*)sge_malloc(sizeof(qhost_report_handler_t));

   DENTER(TOP_LAYER, "xml_report_handler_create");

   if (ret == NULL ) {
      answer_list_add_sprintf(alpp, STATUS_EMALLOC, ANSWER_QUALITY_ERROR,
                              MSG_MEM_MEMORYALLOCFAILED_S, SGE_FUNC);
      DRETURN(NULL);
   }
   /*
   ** for xml_report_handler ctx is a dstring
   */
   ret->ctx = sge_malloc(sizeof(dstring));
   if (ret->ctx == NULL ) {
      answer_list_add_sprintf(alpp, STATUS_EMALLOC, ANSWER_QUALITY_ERROR,
                              MSG_MEM_MEMORYALLOCFAILED_S, SGE_FUNC);
      DRETURN(NULL);
   }
   /*
   ** corresponds to initializing with DSTRING_INIT
   */
   memset(ret->ctx, 0, sizeof(dstring));
   
   ret->report_started = xml_report_started;
   ret->report_finished = xml_report_finished;
   ret->report_host_begin = xml_report_host_begin;
   ret->report_host_string_value = xml_report_host_string_value;
   ret->report_host_ulong_value = xml_report_host_ulong_value;
   ret->report_host_finished = xml_report_host_finished;
   
   ret->report_resource_value = xml_report_resource_value;

   ret->report_queue_begin = xml_report_queue_begin;
   ret->report_queue_string_value = xml_report_queue_string_value;
   ret->report_queue_ulong_value = xml_report_queue_ulong_value;
   ret->report_queue_finished = xml_report_queue_finished;
   
   ret->report_job_begin = xml_report_job_begin;
   ret->report_job_string_value = xml_report_job_string_value;
   ret->report_job_ulong_value = xml_report_job_ulong_value;
   ret->report_job_double_value = xml_report_job_double_value;
   ret->report_job_finished = xml_report_job_finished;

   ret->destroy = xml_report_handler_destroy;

   DRETURN(ret);
}

static int xml_report_handler_destroy(qhost_report_handler_t** handler, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_handler_destroy");

   if (handler != NULL && *handler != NULL ) {
      sge_dstring_free((dstring*)(*handler)->ctx);
      sge_free(&((*handler)->ctx));
      sge_free(handler);
      *handler = NULL;
   }

   DRETURN(QHOST_SUCCESS);
}

static int xml_report_host_begin(qhost_report_handler_t* handler, const char* host_name, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_host_begin");

   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(host_name, (dstring*)handler->ctx);
   printf(" <host name='%s'>\n", sge_dstring_get_string((dstring*)handler->ctx));

   DRETURN(QHOST_SUCCESS);
}

static int xml_report_host_string_value(qhost_report_handler_t* handler, const char*name, const char *value, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_host_string_value");

   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(name, (dstring*)handler->ctx);
   printf("   <hostvalue name='%s'>", sge_dstring_get_string((dstring*)handler->ctx) );
   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(value, (dstring*)handler->ctx);
   printf("%s</hostvalue>\n", sge_dstring_get_string((dstring*)handler->ctx));

   DRETURN(QHOST_SUCCESS);
}

static int xml_report_host_ulong_value(qhost_report_handler_t* handler, const char* name, u_long32 value, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_host_ulong_value");

   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(name, (dstring*)handler->ctx);
   printf("   <hostvalue name='%s'>"sge_U32CFormat"</hostvalue>\n", sge_dstring_get_string((dstring*)handler->ctx), sge_u32c(value));

   DRETURN(QHOST_SUCCESS);
}

static int xml_report_host_finished(qhost_report_handler_t* handler, const char* host_name, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_host_finished");

   printf(" </host>\n");   

   DRETURN(QHOST_SUCCESS);
}

static int xml_report_resource_value(qhost_report_handler_t* handler, const char* dominance, const char* name, const char* value, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_resource_value");

   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(name, (dstring*)handler->ctx);
   printf("   <resourcevalue name='%s' ", sge_dstring_get_string((dstring*)handler->ctx));
   
   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(dominance, (dstring*)handler->ctx);   
   printf("dominance='%s'>", sge_dstring_get_string((dstring*)handler->ctx));
   
   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(value, (dstring*)handler->ctx);   
   printf("%s</resourcevalue>\n", sge_dstring_get_string((dstring*)handler->ctx));
   
   DRETURN(QHOST_SUCCESS);
}

static int xml_report_queue_begin(qhost_report_handler_t* handler, const char* qname, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_queue_begin");

   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(qname, (dstring*)handler->ctx);
   printf(" <queue name='%s'>\n", sge_dstring_get_string((dstring*)handler->ctx));

   DRETURN(QHOST_SUCCESS);
}

static int xml_report_queue_string_value(qhost_report_handler_t* handler, const char* qname, const char* name, const char *value, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_queue_string_value");

   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(qname, (dstring*)handler->ctx);
   printf("   <queuevalue qname='%s'", sge_dstring_get_string((dstring*)handler->ctx));
   
   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(name, (dstring*)handler->ctx);      
   printf(" name='%s'>", sge_dstring_get_string((dstring*)handler->ctx));
   
   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(value, (dstring*)handler->ctx);      
   printf("%s</queuevalue>\n", sge_dstring_get_string((dstring*)handler->ctx));
   
   DRETURN(QHOST_SUCCESS);
}

static int xml_report_queue_ulong_value(qhost_report_handler_t* handler, const char* qname, const char* name, u_long32 value, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_queue_ulong_value");

   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(qname, (dstring*)handler->ctx);
   printf("   <queuevalue qname='%s'", sge_dstring_get_string((dstring*)handler->ctx));
   
   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(name, (dstring*)handler->ctx);      
   printf(" name='%s'>"sge_U32CFormat"</queuevalue>\n", sge_dstring_get_string((dstring*)handler->ctx), sge_u32c(value));
   
   DRETURN(QHOST_SUCCESS);
}

static int xml_report_queue_finished(qhost_report_handler_t* handler, const char* qname, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_queue_finished");

   printf(" </queue>\n");   

   DRETURN(QHOST_SUCCESS);
}

static int xml_report_job_begin(qhost_report_handler_t* handler, const char *qname, const char* jobname, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_job_begin");

   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(jobname, (dstring*)handler->ctx);
   printf(" <job name='%s'>\n", sge_dstring_get_string((dstring*)handler->ctx));

   DRETURN(QHOST_SUCCESS);
}

static int xml_report_job_string_value(qhost_report_handler_t* handler, const char *qname, const char* jobname, const char* name, const char *value, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_job_string_value");

   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(jobname, (dstring*)handler->ctx);
   printf("   <jobvalue jobid='%s'", sge_dstring_get_string((dstring*)handler->ctx));
   
   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(name, (dstring*)handler->ctx);      
   printf(" name='%s'>", sge_dstring_get_string((dstring*)handler->ctx));
   
   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(value, (dstring*)handler->ctx);      
   printf("%s</jobvalue>\n", sge_dstring_get_string((dstring*)handler->ctx));
   
   DRETURN(QHOST_SUCCESS);
}

static int xml_report_job_ulong_value(qhost_report_handler_t* handler, const char *qname, const char* jobname, const char* name, u_long32 value, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_job_ulong_value");

   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(jobname, (dstring*)handler->ctx);
   printf("   <jobvalue jobid='%s'", sge_dstring_get_string((dstring*)handler->ctx));
   
   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(name, (dstring*)handler->ctx);      
   printf(" name='%s'>"sge_U32CFormat"</jobvalue>\n", sge_dstring_get_string((dstring*)handler->ctx), sge_u32c(value));
   
   DRETURN(QHOST_SUCCESS);
}

static int xml_report_job_double_value(qhost_report_handler_t* handler, const char *qname, const char* jobname, const char* name, double value, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_job_double_value");

   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(jobname, (dstring*)handler->ctx);
   printf("   <jobvalue jobid='%s'", sge_dstring_get_string((dstring*)handler->ctx));
   
   sge_dstring_clear((dstring*)handler->ctx);
   escape_string(name, (dstring*)handler->ctx);      
   printf(" name='%s'>'%f'</jobvalue>\n", sge_dstring_get_string((dstring*)handler->ctx), value);
   
   DRETURN(QHOST_SUCCESS);
}

static int xml_report_job_finished(qhost_report_handler_t* handler, const char *qname, const char* job_name, lList **alpp)
{
   DENTER(TOP_LAYER, "xml_report_job_finished");

   printf(" </job>\n");   

   DRETURN(QHOST_SUCCESS);
}

                                      
/************************************************************************/
int main(int argc, char **argv)
{
   lList *pcmdline = NULL;
   lList *ul = NULL;
   lList *host_list = NULL;
   u_long32 show = 0;
   lList *resource_list = NULL;
   lList *resource_match_list = NULL;
   lList *alp = NULL;
   qhost_report_handler_t *report_handler = NULL;
   int is_ok = 0;
   int qhost_result = 0;
   sge_gdi_ctx_class_t *ctx = NULL;

   DENTER_MAIN(TOP_LAYER, "qhost");

   sge_sig_handler_in_main_loop = 0;
   log_state_set_log_gui(true);
   sge_setup_sig_handlers(QHOST);

   if (sge_gdi2_setup(&ctx, QHOST, MAIN_THREAD, &alp) != AE_OK) {
      answer_list_output(&alp);
      sge_prof_cleanup();
      SGE_EXIT((void**)&ctx, 1);
   }

   /*
   ** stage 1 of commandline parsing
   */
   if (!sge_parse_cmdline_qhost(argv, environ, &pcmdline, &alp)) {
      /*
      ** high level parsing error! sow answer list
      */
      answer_list_output(&alp);
      lFreeList(&pcmdline);
      sge_prof_cleanup();
      SGE_EXIT((void **)&ctx, 1);
   }

   /*
   ** stage 2 of commandline parsing 
   */
   is_ok = sge_parse_qhost(&pcmdline, 
                           &resource_match_list,   /* -l resource_request           */
                           &resource_list,         /* -F qresource_request          */
                           &host_list,             /* -h host_list                  */
                           &ul,                    /* -u user_list                  */
                           &show,                  /* -q, -j                        */
                           &report_handler,
                           &alp);
   lFreeList(&pcmdline);
   if (is_ok == 0) {     
      /*
      ** low level parsing error! show answer list
      */
      answer_list_output(&alp);
      sge_prof_cleanup();
      SGE_EXIT(NULL, 1);
   } else if (is_ok == 2) {
      /* -help output generated, exit normally */ 
      answer_list_output(&alp);
      sge_prof_cleanup();
      SGE_EXIT(NULL, 0);
   }

   qhost_result = do_qhost(ctx, host_list, ul, resource_match_list, resource_list, 
                           show, &alp, report_handler);

   if (report_handler != NULL) {
      report_handler->destroy(&report_handler, &alp);
   }
   
   if (qhost_result != QHOST_SUCCESS) {
      answer_list_output(&alp);
      sge_prof_cleanup();
      SGE_EXIT((void**)&ctx, 1);
   }

   sge_prof_cleanup();
   SGE_EXIT((void**)&ctx, 0); /* 0 means ok - others are errors */
   DEXIT;
   return 0;
}


/*
** NAME
**   qhost_usage
** PARAMETER
**   none
** RETURN
**   none
** EXTERNAL
**   none
** DESCRIPTION
**   displays qhost_usage for qlist client
**   note that the other clients use a common function
**   for this. output was adapted to a similar look.
*/
static bool qhost_usage(
FILE *fp 
) {
   dstring ds;
   char buffer[256];

   DENTER(TOP_LAYER, "qhost_usage");

   if (fp == NULL) {
      DRETURN(false);
   }

   sge_dstring_init(&ds, buffer, sizeof(buffer));

   fprintf(fp, "%s\n", feature_get_product_name(FS_SHORT_VERSION, &ds));

   fprintf(fp,"%s qhost [options]\n", MSG_SRC_USAGE);
         
   fprintf(fp, "  [-F [resource_attribute]]  %s\n", MSG_QHOST_F_OPT_USAGE); 
   fprintf(fp, "  [-h hostlist]              %s\n", MSG_QHOST_h_OPT_USAGE);
   fprintf(fp, "  [-help]                    %s\n", MSG_COMMON_help_OPT_USAGE);
   fprintf(fp, "  [-j]                       %s\n", MSG_QHOST_j_OPT_USAGE);
   fprintf(fp, "  [-l attr=val,...]          %s\n", MSG_QHOST_l_OPT_USAGE);
   fprintf(fp, "  [-ncb]                     %s\n", MSG_QHOST_ncb_OPT_USAGE);
   fprintf(fp, "  [-q]                       %s\n", MSG_QHOST_q_OPT_USAGE);
   fprintf(fp, "  [-u user[,user,...]]       %s\n", MSG_QHOST_u_OPT_USAGE); 
   fprintf(fp, "  [-xml]                     %s\n", MSG_COMMON_xml_OPT_USAGE);

   DRETURN(true);
}

/****
 **** sge_parse_cmdline_qhost (static)
 ****
 **** 'stage 1' parsing of qhost-options. Parses options
 **** with their arguments and stores them in ppcmdline.
 ****/ 
static bool sge_parse_cmdline_qhost(
char **argv,
char **envp,
lList **ppcmdline,
lList **alpp
) {
   char **sp;
   char **rp;
   DENTER(TOP_LAYER, "sge_parse_cmdline_qhost");

   rp = ++argv;

   while(*(sp=rp)) {
      /* -help */
      if ((rp = parse_noopt(sp, "-help", NULL, ppcmdline, alpp)) != sp)
         continue;
 
      /* -ncb */
      if ((rp = parse_noopt(sp, "-ncb", NULL, ppcmdline, alpp)) != sp)
         continue;

      /* -q */
      if ((rp = parse_noopt(sp, "-q", NULL, ppcmdline, alpp)) != sp)
         continue;

      /* -F */
      if ((rp = parse_until_next_opt2(sp, "-F", NULL, ppcmdline, alpp)) != sp)
         continue;

      /* -h */
      if ((rp = parse_until_next_opt(sp, "-h", NULL, ppcmdline, alpp)) != sp)
         continue;

      /* -j */
      if ((rp = parse_noopt(sp, "-j", NULL, ppcmdline, alpp)) != sp)
         continue;

      /* -l */
      if ((rp = parse_until_next_opt(sp, "-l", NULL, ppcmdline, alpp)) != sp)
         continue;

      /* -u */
      if ((rp = parse_until_next_opt(sp, "-u", NULL, ppcmdline, alpp)) != sp)
         continue;

      /* -xml */
      if ((rp = parse_noopt(sp, "-xml", NULL, ppcmdline, alpp)) != sp)
         continue;
      
      /* oops */
      qhost_usage(stderr);
      answer_list_add_sprintf(alpp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR, MSG_PARSE_INVALIDOPTIONARGUMENTX_S, *sp);
      DRETURN(false);
   }
   DRETURN(true);
}

/****
 **** sge_parse_qhost (static)
 ****
 **** 'stage 2' parsing of qhost-options. Gets the options from pcmdline
 ****/
static int sge_parse_qhost(lList **ppcmdline,
                            lList **pplres,
                            lList **ppFres,
                            lList **pphost,
                            lList **ppuser,
                            u_long32 *show,
                            qhost_report_handler_t **report_handler,
                            lList **alpp) 
{
   u_long32 helpflag;
   bool usageshowed = false;
   u_long32 full = 0;
   u_long32 binding = 0;
   char * argstr = NULL;
   lListElem *ep;
   int ret = 1;

   DENTER(TOP_LAYER, "sge_parse_host");
 
   /* Loop over all options. Only valid options can be in the
      ppcmdline list. 
   */

   /* display topology related information per default */
   (*show) |= QHOST_DISPLAY_BINDING;

   while (lGetNumberOfElem(*ppcmdline))
   {
      if (parse_flag(ppcmdline, "-help",  alpp, &helpflag)) {
         usageshowed = true;
         qhost_usage(stdout);
         ret = 2;
         goto exit;   
      }

      if (parse_multi_stringlist(ppcmdline, "-h", alpp, pphost, ST_Type, ST_name)) {
         /* 
         ** resolve hostnames and replace them in list
         */
         for_each(ep, *pphost) {
            if (sge_resolve_host(ep, ST_name) != CL_RETVAL_OK) {
               char buf[BUFSIZ];
               sprintf(buf, MSG_SGETEXT_CANTRESOLVEHOST_S, lGetString(ep,ST_name) );
               answer_list_add(alpp, buf, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
               goto error;
            }
         }
         continue;
      }

      if (parse_multi_stringlist(ppcmdline, "-F", alpp, ppFres, ST_Type, ST_name)) {
         (*show) |= QHOST_DISPLAY_RESOURCES;
         continue;
      }
      if (parse_flag(ppcmdline, "-ncb", alpp, &binding)) {
         /* disable topology related information */
         (*show) ^= QHOST_DISPLAY_BINDING;
         continue;
      }
      if (parse_flag(ppcmdline, "-q", alpp, &full)) {
         if(full) {
            (*show) |= QHOST_DISPLAY_QUEUES;
            full = 0;
         }
         continue;
      }

      if (parse_flag(ppcmdline, "-j", alpp, &full)) {
         if(full) {
            (*show) |= QHOST_DISPLAY_JOBS;
            full = 0;
         }
         continue;
      }
      while (parse_string(ppcmdline, "-l", alpp, &argstr)) {
         *pplres = centry_list_parse_from_string(*pplres, argstr, false);
         sge_free(&argstr);
         continue;
      }
      
      if (parse_multi_stringlist(ppcmdline, "-u", alpp, ppuser, ST_Type, ST_name)) {
         (*show) |= QHOST_DISPLAY_JOBS;
         continue;
      }

      if (parse_flag(ppcmdline, "-xml", alpp, &full)) {
         *report_handler = xml_report_handler_create(alpp);
         continue;
      }

   }
   if (lGetNumberOfElem(*ppcmdline)) {
     answer_list_add_sprintf(alpp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR,
                             "%s", MSG_PARSE_TOOMANYOPTIONS);
     goto error;
   }

   DRETURN(1);

   error:
      ret = 0;
   exit:
      if (!usageshowed) {
         qhost_usage(stderr);
      }
      if (report_handler && *report_handler) {
         (*report_handler)->destroy(report_handler, alpp);
      }
      lFreeList(pplres);
      lFreeList(ppFres);
      lFreeList(pphost);
      lFreeList(ppuser);
   
      DRETURN(ret);
}

