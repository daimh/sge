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
#include <string.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#include "uti/sge_rmon.h"
#include "uti/sge_stdio.h"
#include "uti/sge_time.h"
#include "uti/sge_string.h"

#include "sgeobj/cull_parse_util.h"
#include "sgeobj/sge_answer.h"

#include "schedd_monitor.h"
#include "msg_common.h"

static char schedd_log_file[SGE_PATH_MAX + 1] = "";

void schedd_set_schedd_log_file(sge_gdi_ctx_class_t *ctx)
{
   const char *cell_root = ctx->get_cell_root(ctx);
   
   DENTER(TOP_LAYER, "schedd_set_schedd_log_file");

   if (!*schedd_log_file) {
      snprintf(schedd_log_file, sizeof(schedd_log_file), "%s/%s/%s", cell_root, "common", SCHED_LOG_NAME);
      DPRINTF(("schedd log file >>%s<<\n", schedd_log_file));
   }

   DEXIT;
}


int schedd_log(const char *logstr, lList **monitor_alpp, bool monitor_next_run)
{
   DENTER(TOP_LAYER, "schedd_log");

   if (monitor_alpp != NULL) {
      /* add to answer list for verification (-w v) */
      answer_list_add(monitor_alpp, logstr, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
   }

   if (monitor_next_run) {
      /* do logging (-tsm) */
      time_t now;
      FILE *fp = NULL;
      char *time_str = NULL;
      char str[128];

      now = (time_t)sge_get_gmt();
      time_str =  ctime_r(&now, str);
      if (time_str[strlen(time_str) - 1] == '\n') {
         time_str[strlen(time_str) - 1] = '|';
      }

      fp = fopen(schedd_log_file, "a");
      if (!fp) {
         DPRINTF(("could not open schedd_log_file "SFQ"\n", schedd_log_file));
         DRETURN(-1);
      }

      fprintf(fp, "%s", time_str);
      fprintf(fp, "%s\n", logstr);
      FCLOSE(fp);
   }

   DRETURN(0);
FCLOSE_ERROR:
   DPRINTF((MSG_FILE_NOCLOSE_SS, schedd_log_file, strerror(errno)));
   DRETURN(-1);
}


#define NUM_ITEMS_ON_LINE 10

int schedd_log_list(lList **monitor_alpp, bool monitor_next_run, const char *logstr, lList *lp, int nm) {
   int fields[] = { 0, 0 };
   const char *delis[] = {NULL, " ", NULL};
   lList *lp_part = NULL;
   lListElem *ep = NULL;

   DENTER(TOP_LAYER, "schedd_log_list");

#ifndef WIN32NATIVE

   if (monitor_alpp == NULL && !monitor_next_run) {
      DRETURN(0);
   }

   fields[0] = nm;

   for_each(ep, lp) {
      if (!lp_part) {
         lp_part = lCreateList("partial list", lGetListDescr(lp));
      }
      lAppendElem(lp_part, lCopyElem(ep));
      if ((lGetNumberOfElem(lp_part) == NUM_ITEMS_ON_LINE) || !lNext(ep)) {
         char log_string[2048];

         sge_strlcpy(log_string, logstr, sizeof(log_string));
#ifndef WIN32NATIVE
         uni_print_list(NULL,
                        log_string + strlen(log_string),
                        sizeof(log_string) - strlen(log_string) - 1,
                        lp_part,
                        fields, delis, 0);
#endif
         schedd_log(log_string, monitor_alpp, monitor_next_run);
         lFreeList(&lp_part);
         lp_part = NULL;
      }
   }
#else
   DPRINTF(("schedd_log_list does nothing for QMonNT !!!\n"));
#endif

   DEXIT;
   return 0;
}

const char *job_descr(
u_long32 jobid 
) {
   static char descr[20];

   if (jobid) {
      snprintf(descr, sizeof(descr), "Job "sge_u32, jobid);
      return descr;
   } else
      return "Job";
   
}

