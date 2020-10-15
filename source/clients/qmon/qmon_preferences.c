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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <Xmt/Xmt.h>
#include <Xmt/Util.h>

#include "qmon_preferences.h"

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_stdio.h"
#include "uti/sge_unistd.h"
#include "uti/sge_log.h"

#include "sgeobj/sge_pref_PREF_L.h"
#include "sgeobj/sge_qref.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_config.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_conf.h"

#include "gdi/version.h"

#include "sge.h"
#include "msg_common.h"
#include "msg_qmon.h"

typedef int (*ReadWorkFuncT)(lList **alpp, lList **clpp, int fields[], lListElem *ep, int spool, int flag, int *tag, int parsing_type);

struct read_object_args {
   lDescr *objtype;
   char *objname;
   ReadWorkFuncT work_func;
};

static int read_pref_work(lList **alpp, lList **clpp, int fields[], lListElem *ep, int spool, int flag, int *tag, int parsing_type);

static lListElem *cull_read_in_pref(char *dirname, char *filename, int spool, int flag, int *tag);

static lList* write_pref(char *filename, lListElem *ep);

static lListElem* read_object( const char *dirname, const char *filename, int spool, int flag,
                        int read_config_list_flag, struct read_object_args *args,
                        int *tag, int fields[]);

/*
** the qmon preferences are kept here
*/
static lListElem *qmon_preferences = NULL;

lListElem *qmonGetPreferences(void)
{

   DENTER(TOP_LAYER, "qmonGetPreferences");

   if (!qmon_preferences) {
      qmon_preferences = lCreateElem(PREF_Type);
      lSetBool(qmon_preferences, PREF_job_filter_compact, True);
   }   

   DEXIT;
   return qmon_preferences;
}   


void qmonReadPreferences(void)
{
   DENTER(TOP_LAYER, "qmonReadPreferences");

   qmon_preferences = cull_read_in_pref(XmtGetHomeDir(), ".qmon_preferences",
                                        0, 0, NULL);
   DEXIT;
}

lList* qmonWritePreferences(void)
{
   char filename[SGE_PATH_MAX];
   lList *alp;

   DENTER(TOP_LAYER, "qmonWritePreferences");

   sprintf(filename, "%s/.qmon_preferences", XmtGetHomeDir());
   alp = write_pref(filename, qmon_preferences);

   DEXIT;
   return alp;
}

/****
 **** cull_read_in_pref
 ****/
static lListElem *cull_read_in_pref(
char *dirname,
char *filename,
int spool,
int flag,
int *tag 
) {
   lListElem *ep;
   struct read_object_args args = { PREF_Type, "pref", read_pref_work };
   int intern_tag = 0;

   DENTER(TOP_LAYER, "cull_read_in_pref");

   ep = read_object(dirname, filename, spool, 0, 0,&args, tag?tag:&intern_tag, NULL);
  
   DEXIT;
   return ep;
}


/* ------------------------------------------------------------

   read_pref_work()

   spool:
      1 write for spooling
      0 write only user controlled fields

*/
static int read_pref_work(
lList **alpp,
lList **clpp,
int fields[],
lListElem *ep,
int spool,
int flag,
int *tag,
int parsing_type 
) {
   static int intprt_as_cplx_entry[] = { 
      CE_name, 
      CE_stringval,
      0
   }; 

   DENTER(TOP_LAYER, "read_pref_work");

   /* --------- PREF_job_filter_resources */
   if (!set_conf_deflist(alpp, clpp, fields, "job_filter_resources", ep, 
                      PREF_job_filter_resources, 
                        CE_Type, intprt_as_cplx_entry)) {
      DEXIT;
      return -1;
   }
   
   /* --------- PREF_job_filter_owners */
   if (!set_conf_list(alpp, clpp, fields, "job_filter_owners", ep, 
                      PREF_job_filter_owners, ST_Type, ST_name)) {
      DEXIT;
      return -1;
   }
   
   /* --------- PREF_job_filter_fields */
   if (!set_conf_list(alpp, clpp, fields, "job_filter_fields", ep, 
                      PREF_job_filter_fields, ST_Type, ST_name)) {
      DEXIT;
      return -1;
   }
   
   /* --------- PREF_job_filter_compact */
   set_conf_bool(NULL, clpp, fields, "job_filter_compact", ep, 
                 PREF_job_filter_compact);

   /* --------- PREF_queue_filter_resources */
   if (!set_conf_deflist(alpp, clpp, fields, "queue_filter_resources", ep, 
                      PREF_queue_filter_resources,
                      CE_Type, intprt_as_cplx_entry)) {
      DEXIT;
      return -1;
   }
   
   /* --------- PREF_queue_filter_pe */
   if (!set_conf_list(alpp, clpp, fields, "queue_filter_pe", ep, 
                      PREF_queue_filter_pe, ST_Type, ST_name)) {
      DEXIT;
      return -1;
   }

   /* --------- PREF_queue_filter_user */
   if (!set_conf_list(alpp, clpp, fields, "queue_filter_user", ep, 
                      PREF_queue_filter_user, ST_Type, ST_name)) {
      DEXIT;
      return -1;
   }
   
   /* --------- PREF_queue_filter_q */
   if (!set_conf_list(alpp, clpp, fields, "queue_filter_q", ep, 
                      PREF_queue_filter_q, QR_Type, QR_name)) {
      DEXIT;
      return -1;
   }
   
   /* --------- PREF_queue_filter_state */
   if (!set_conf_string(alpp, clpp, fields, "queue_filter_state", ep, 
                 PREF_queue_filter_state)) {
      DEXIT;
      return -1;
   }

   /* --------- PREF_ar_filter_fields */
   if (!set_conf_list(alpp, clpp, fields, "ar_filter_fields", ep, 
                      PREF_ar_filter_fields, ST_Type, ST_name)) {
      DEXIT;
      return -1;
   }
   
   DEXIT;
   return 0;
}


/* ------------------------------------------------------------
   write preferences to file
*/
static lList* write_pref(
char *filename,
lListElem *ep 
) {
   lList *answer = NULL;
   FILE *fp;
   lListElem *sep;
   dstring ds;
   char buffer[256];

   DENTER(TOP_LAYER, "write_pref");

   sge_dstring_init(&ds, buffer, sizeof(buffer));

   if (!filename)
      fp = stdout;
   else
      fp = fopen(filename, "w");

   if (!fp) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_FILE_ERRORWRITETOFILEX_S, filename));
      answer_list_add(&answer, SGE_EVENT, STATUS_EDISK, ANSWER_QUALITY_ERROR);
      DEXIT;
      return answer;
   }

   /* --------- HEADER COMMENT */
   fprintf(fp, "# %s AUTOGENERATED FILE DO NOT EDIT !\n",
               feature_get_product_name(FS_LONG_VERSION, &ds));

   /* --------- PREF_job_filter_resources */
   fprintf(fp, "job_filter_resources   ");
   sep = lFirst(lGetList(ep, PREF_job_filter_resources));
   if (sep) {
      do {
         fprintf(fp, "%s=%s", lGetString(sep, CE_name), 
                     lGetString(sep, CE_stringval));
         sep = lNext(sep);
         if (sep) 
            fprintf(fp, ",");
      } while (sep);
      fprintf(fp, "\n");
   }
   else
      fprintf(fp, "NONE\n");

   /* --------- PREF_job_filter_owners */
   fprintf(fp, "job_filter_owners      ");
   sep = lFirst(lGetList(ep, PREF_job_filter_owners));
   if (sep) {
      do {
         fprintf(fp, "%s", lGetString(sep, ST_name));
         sep = lNext(sep);
         if (sep) 
            fprintf(fp, " ");
      } while (sep);
      fprintf(fp, "\n");
   }
   else
      fprintf(fp, "NONE\n");

   /* --------- PREF_job_filter_fields */
   fprintf(fp, "job_filter_fields      ");
   sep = lFirst(lGetList(ep, PREF_job_filter_fields));
   if (sep) {
      do {
         fprintf(fp, "%s", lGetString(sep, ST_name));
         sep = lNext(sep);
         if (sep) 
            fprintf(fp, " ");
      } while (sep);
      fprintf(fp, "\n");
   }
   else
      fprintf(fp, "NONE\n");

   /* --------- PREF_job_filter_compact */
   fprintf(fp, "job_filter_compact     %s\n", 
           lGetBool(ep, PREF_job_filter_compact) ?  "TRUE" : "FALSE");

   /* --------- PREF_queue_filter_resources */
   fprintf(fp, "queue_filter_resources ");
   sep = lFirst(lGetList(ep, PREF_queue_filter_resources));
   if (sep) {
      do {
         fprintf(fp, "%s=%s", lGetString(sep, CE_name), 
                     lGetString(sep, CE_stringval));
         sep = lNext(sep);
         if (sep) 
            fprintf(fp, ",");
      } while (sep);
      fprintf(fp, "\n");
   }
   else
      fprintf(fp, "NONE\n");

   /* --------- PREF_queue_filter_pe */
   fprintf(fp, "queue_filter_pe        ");
   sep = lFirst(lGetList(ep, PREF_queue_filter_pe));
   if (sep) {
      do {
         fprintf(fp, "%s", lGetString(sep, ST_name));
         sep = lNext(sep);
         if (sep) 
            fprintf(fp, " ");
      } while (sep);
      fprintf(fp, "\n");
   }
   else
      fprintf(fp, "NONE\n");

   /* --------- PREF_queue_filter_user */
   fprintf(fp, "queue_filter_user      ");
   sep = lFirst(lGetList(ep, PREF_queue_filter_user));
   if (sep) {
      do {
         fprintf(fp, "%s", lGetString(sep, ST_name));
         sep = lNext(sep);
         if (sep) 
            fprintf(fp, " ");
      } while (sep);
      fprintf(fp, "\n");
   }
   else
      fprintf(fp, "NONE\n");

   /* --------- PREF_queue_filter_q */
   fprintf(fp, "queue_filter_q         ");
   sep = lFirst(lGetList(ep, PREF_queue_filter_q));
   if (sep) {
      do {
         fprintf(fp, "%s", lGetString(sep, QR_name));
         sep = lNext(sep);
         if (sep) 
            fprintf(fp, " ");
      } while (sep);
      fprintf(fp, "\n");
   }
   else
      fprintf(fp, "NONE\n");

   /* --------- PREF_queue_filter_state */
   fprintf(fp, "queue_filter_state     %s\n", lGetString(ep, PREF_queue_filter_state)? lGetString(ep, PREF_queue_filter_state) : "NONE");

   /* --------- PREF_ar_filter_fields */
   fprintf(fp, "ar_filter_fields      ");
   sep = lFirst(lGetList(ep, PREF_ar_filter_fields));
   if (sep) {
      do {
         fprintf(fp, "%s", lGetString(sep, ST_name));
         sep = lNext(sep);
         if (sep) 
            fprintf(fp, " ");
      } while (sep);
      fprintf(fp, "\n");
   }
   else
      fprintf(fp, "NONE\n");

   if (fp != stdout) {
      FCLOSE(fp);
   }

   DEXIT;
   return NULL;
FCLOSE_ERROR:
   /* TODO: error handling */
   DEXIT;
   return NULL;
}

static lListElem* read_object( const char *dirname, const char *filename, int spool, int flag,
                        int read_config_list_flag, struct read_object_args *args,
                        int *tag, int fields[]) {
   int ret;
   char fullname[SGE_PATH_MAX];
   FILE *fp;
   lListElem *ep, *unused;
   lList *alp = NULL, *clp = NULL;
   SGE_STRUCT_STAT sb;
   size_t size;
   char *buf;

   DENTER(TOP_LAYER, "read_object");

   /* build full filename */
   if(dirname && filename)
      sprintf(fullname, "%s/%s", dirname, filename);
   else if(dirname)
      sprintf(fullname, "%s", dirname);
   else
      sprintf(fullname, "%s", filename);
      
   /* open file */
   if(!(fp = fopen(fullname, "r"))) {
      ERROR((SGE_EVENT, MSG_FILE_ERROROPENINGXY_SS, fullname, strerror(errno)));
      DEXIT;
      return NULL;
   }

   if (!SGE_STAT(fullname, &sb)) {
      size = MAX(sb.st_size, 10000);
      if ((off_t)size != MAX(sb.st_size, 10000) ||
         (buf = (char *) malloc(size)) == NULL) {
         FCLOSE(fp);
         ERROR((SGE_EVENT, MSG_MEMORY_CANTMALLOCBUFFERFORXOFFILEY_SS, 
               args->objname, fullname));
         DEXIT;
         return NULL;
      }
   } else {
      ERROR((SGE_EVENT, MSG_FILE_CANTDETERMINESIZEFORXOFFILEY_SS, 
             args->objname, fullname));
      FCLOSE(fp);
      DEXIT;
      return NULL;
   }


   /* create List Element */
   if (!(ep = lCreateElem(args->objtype))) {
      FCLOSE(fp);
      sge_free(&buf);
      ERROR((SGE_EVENT, SFNMAX, MSG_SGETEXT_NOMEM));
      DEXIT;
      return NULL;
   }

   /* read in config file */
   if (read_config_list(fp, &clp, &alp, CF_Type, CF_name, CF_value,
                        CF_sublist, NULL, read_config_list_flag, buf, size)) {
      ERROR((SGE_EVENT, SFNMAX, lGetString(lFirst(alp), AN_text)));
      lFreeList(&alp);
      FCLOSE(fp);
      sge_free(&buf);
      DEXIT;
      return NULL;
   }

   sge_free(&buf);
   FCLOSE(fp);

   /* well, let's do the work... */
   ret = args->work_func(&alp, &clp, fields, ep, spool, flag, tag, 0);
   if (ret) {
      if (alp) 
         ERROR((SGE_EVENT, SFNMAX, lGetString(lFirst(alp), AN_text)));
      lFreeList(&alp);
      lFreeList(&clp);
      lFreeElem(&ep);
      DEXIT;
      return NULL;
   }

   /* complain about unused configuration elements */
   if ((unused = lFirst(clp))) {
      ERROR((SGE_EVENT, MSG_SGETEXT_UNKNOWN_CONFIG_VALUE_SSS,
         lGetString(unused, CF_name), args->objname, fullname));
      lFreeList(&clp);
      lFreeElem(&ep);
      DEXIT;
      return NULL;
   }

   /* remove warnings in alp */
   lFreeList(&alp);

   DEXIT;
   return ep;
FCLOSE_ERROR:
   DEXIT;
   return NULL;
}
