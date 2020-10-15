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
 * Portions of this code are Copyright 2011 Univa Inc.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>
#include <ctype.h>
#include <pwd.h>

#include "uti/sge_rmon.h"
#include "uti/sge_time.h"
#include "uti/sge_log.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_string.h"
#include "uti/sge_unistd.h"
#include "uti/sge_io.h"
#include "uti/setup_path.h"
#include "uti/sge_prog.h"

#include "cull/cull_list.h"

#include "sgeobj/parse.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_feature.h"

#include "gdi/sge_gdi.h"

#include "qstat_cmdline.h"
#include "read_defaults.h"
#include "symbols.h"
#include "sge.h"
#include "msg_common.h"
#include "msg_qstat.h"
#include "msg_clients_common.h"

bool
switch_list_qstat_parse_from_file(lList **switch_list, lList **answer_list,
                                  int mode, const char *file)
{
   bool ret = true;

   DENTER(TOP_LAYER, "switch_list_qstat_parse_from_file");
   if (switch_list == NULL) {
      ret = false;
   } else {
      if (!sge_is_file(file)) {
         /*
          * This is no error
          */
         DPRINTF(("file "SFQ" does not exist\n", file));
         ret = true;
      } else {
         char *file_as_string = NULL;
         int file_as_string_length;

         file_as_string = sge_file2string(file, &file_as_string_length);
         if (file_as_string == NULL) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR,
                                    MSG_ANSWER_ERRORREADINGFROMFILEX_S, file);
            ret = false;
         } else {
            char **token = NULL;

            token = stra_from_str(file_as_string, " \n\t");
            ret = switch_list_qstat_parse_from_cmdline(switch_list, answer_list,
                                                       mode, token);
            sge_strafree(&token);
         }
         sge_free(&file_as_string);
      }
   }  
   DRETURN(ret); 
}

bool
switch_list_qstat_parse_from_cmdline(lList **ppcmdline, lList **answer_list,
                                     int qselect_mode, char **argv)
{
   bool ret = true;
   char **sp;
   char **rp;
   stringT str;

   DENTER(TOP_LAYER, "sge_parse_cmdline_qstat");

   rp = argv;
   while(*(sp=rp)) {
      /* -help */
      if ((rp = parse_noopt(sp, "-help", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -ncb */
      if ((rp = parse_noopt(sp, "-ncb", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -f option */
      if (!qselect_mode && (rp = parse_noopt(sp, "-f", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -F */
      if (!qselect_mode && (rp = parse_until_next_opt2(sp, "-F", NULL, ppcmdline, answer_list)) != sp)
         continue;

      if (!qselect_mode) {
         /* -ext option */
         if ((rp = parse_noopt(sp, "-ext", NULL, ppcmdline, answer_list)) != sp)
            continue;

         /* -urg option */
         if ((rp = parse_noopt(sp, "-urg", NULL, ppcmdline, answer_list)) != sp)
            continue;

         /* -urg option */
         if ((rp = parse_noopt(sp, "-pri", NULL, ppcmdline, answer_list)) != sp)
            continue;

         /* -xml option */
         if ((rp = parse_noopt(sp, "-xml", NULL, ppcmdline, answer_list)) != sp)
            continue;

      }

      /* -g */
      if (!qselect_mode && (rp = parse_until_next_opt(sp, "-g", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -j [jid {,jid}]*/
      if (!qselect_mode && (rp = parse_until_next_opt2(sp, "-j", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -l */
      if ((rp = parse_until_next_opt(sp, "-l", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -ne option */
      if (!qselect_mode && (rp = parse_noopt(sp, "-ne", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -s [p|r|s|h|d|...] option */
      if (!qselect_mode && (rp = parse_until_next_opt(sp, "-s", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -qs [.{a|c|d|o|..] option */
      if ((rp = parse_until_next_opt(sp, "-qs", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -explain [c|a|A...] option */
      if (!qselect_mode && (rp = parse_until_next_opt(sp, "-explain", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -q */
      if ((rp = parse_until_next_opt(sp, "-q", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -r */
      if (!qselect_mode && (rp = parse_noopt(sp, "-r", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -t */
      if (!qselect_mode && (rp = parse_noopt(sp, "-t", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -u */
      if (!qselect_mode && (rp = parse_until_next_opt(sp, "-u", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -U */
      if ((rp = parse_until_next_opt(sp, "-U", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /* -pe */
      if ((rp = parse_until_next_opt(sp, "-pe", NULL, ppcmdline, answer_list)) != sp)
         continue;

      /*
      ** Two additional flags only if MORE_INFO is set:
      ** -dj   dump jobs:  displays full global_job_list 
      ** -dq   dump queue: displays full global_queue_list
      */
      if (getenv("MORE_INFO")) {
         /* -dj */
         if ((rp = parse_noopt(sp, "-dj", NULL, ppcmdline, answer_list)) != sp)
            continue;

         /* -dq */
         if ((rp = parse_noopt(sp, "-dq", NULL, ppcmdline, answer_list)) != sp)
            continue;
      }

      /* oops */
      sprintf(str, MSG_ANSWER_INVALIDOPTIONARGX_S, *sp);
      qstat_usage(qselect_mode, stderr, NULL);
      answer_list_add(answer_list, str, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      DEXIT;
      return ret;
   }
   DEXIT;
   return ret;
}

/****
 **** qstat_usage (static)
 ****
 **** displays usage of qstat on file fp.
 **** Is what NULL, full usage will be displayed.
 ****
 **** Returns always 1.
 ****
 **** If what is a pointer to an option-string,
 **** only usage for that option will be displayed.
 ****   ** not implemented yet! **
 ****/
int 
qstat_usage(int qselect_mode, FILE *fp, char *what) 
{
   dstring ds;
   char buffer[256];
   
   sge_dstring_init(&ds, buffer, sizeof(buffer));

   fprintf(fp, "%s\n", feature_get_product_name(FS_SHORT_VERSION, &ds));
 
   if(!what) {
      /* display full usage */
      fprintf(fp, "%s %s [options]\n", MSG_SRC_USAGE ,qselect_mode?"qselect":"qstat");
      if (!qselect_mode) {
         fprintf(fp, "        [-ext]                            %s\n",MSG_QSTAT_USAGE_VIEWALSOSCHEDULINGATTRIBUTES);
      }
      if (!qselect_mode) {
         fprintf(fp, "        [-explain a|c|A|E]                %s\n",MSG_QSTAT_USAGE_EXPLAINOPT);
      }
      if (!qselect_mode) 
         fprintf(fp, "        [-f]                              %s\n",MSG_QSTAT_USAGE_FULLOUTPUT);
      if (!qselect_mode) 
         fprintf(fp, "        [-F [resource_attributes]]        %s\n",MSG_QSTAT_USAGE_FULLOUTPUTANDSHOWRESOURCESOFQUEUES);
      if (!qselect_mode) {
         fprintf(fp, "        [-g c]                            %s\n",MSG_QSTAT_USAGE_DISPLAYCQUEUESUMMARY);
         fprintf(fp, "        [-g d]                            %s\n",MSG_QSTAT_USAGE_DISPLAYALLJOBARRAYTASKS);
         fprintf(fp, "        [-g t]                            %s\n",MSG_QSTAT_USAGE_DISPLAYALLPARALLELJOBTASKS);
      }
      fprintf(fp, "        [-help]                           %s\n",MSG_COMMON_help_OPT_USAGE);
      if (!qselect_mode)
         fprintf(fp, "        [-j [job_identifier_list]]        %s\n",MSG_QSTAT_USAGE_SHOWSCHEDULERJOBINFO);
      fprintf(fp, "        [-l resource_list]                %s\n",MSG_QSTAT_USAGE_REQUESTTHEGIVENRESOURCES);
      if (!qselect_mode) 
         fprintf(fp, "        [-ne]                             %s\n",MSG_QSTAT_USAGE_HIDEEMPTYQUEUES);
      if (!qselect_mode) {
         fprintf(fp, "        [-ncb]                            %s\n",MSG_QSTAT_USAGE_VIEWALSOBINDINGATTRIBUTES);
      }
      fprintf(fp, "        [-pe pe_list]                     %s\n",MSG_QSTAT_USAGE_SELECTONLYQUEESWITHONOFTHESEPE);
      fprintf(fp, "        [-q wc_queue_list]                %s\n",MSG_QSTAT_USAGE_PRINTINFOONGIVENQUEUE);
      fprintf(fp, "        [-qs {a|c|d|o|s|u|A|C|D|E|S}]     %s\n",MSG_QSTAT_USAGE_PRINTINFOCQUEUESTATESEL);
      if (!qselect_mode) 
         fprintf(fp, "        [-r]                              %s\n",MSG_QSTAT_USAGE_SHOWREQUESTEDRESOURCESOFJOB);
      if (!qselect_mode) {
         fprintf(fp, "        [-s {p|r|s|z|hu|ho|hs|hd|hj|ha|h|a}] %s\n",MSG_QSTAT_USAGE_SHOWPENDINGRUNNINGSUSPENDESZOMBIEJOBS);
         fprintf(fp, "                                          %s\n",MSG_QSTAT_USAGE_JOBSWITHAUSEROPERATORSYSTEMHOLD);
         fprintf(fp, "                                          %s\n",MSG_QSTAT_USAGE_JOBSWITHSTARTTIMEINFUTORE);
         fprintf(fp, "                                          %s\n",MSG_QSTAT_USAGE_HISABBREVIATIONFORHUHOHSHJHA);
         fprintf(fp, "                                          %s\n",MSG_QSTAT_USAGE_AISABBREVIATIONFOR);
      }
      if (!qselect_mode) 
         fprintf(fp, "        [-t]                              %s\n",MSG_QSTAT_USAGE_SHOWTASKINFO);
      if (!qselect_mode){  
         fprintf(fp, "        [-u user_list]                    %s\n",MSG_QSTAT_USAGE_VIEWONLYJOBSOFTHISUSER);
      }   
      fprintf(fp, "        [-U user_list]                    %s\n",MSG_QSTAT_USAGE_SELECTQUEUESWHEREUSERXHAVEACCESS);

      if (!qselect_mode) {
         fprintf(fp, "        [-urg]                            %s\n",MSG_QSTAT_URGENCYINFO );
         fprintf(fp, "        [-pri]                            %s\n",MSG_QSTAT_PRIORITYINFO );
         fprintf(fp, "        [-xml]                            %s\n", MSG_COMMON_xml_OPT_USAGE);
      }   
      
      if (getenv("MORE_INFO")) {
         fprintf(fp, "%s\n", MSG_QSTAT_USAGE_ADDITIONALDEBUGGINGOPTIONS);
         fprintf(fp, "        [-dj]                             %s\n",MSG_QSTAT_USAGE_DUMPCOMPLETEJOBLISTTOSTDOUT);
         fprintf(fp, "        [-dq]                             %s\n",MSG_QSTAT_USAGE_DUMPCOMPLETEQUEUELISTTOSTDOUT);
      }
      fprintf(fp, "\n");
      fprintf(fp, "pe_list                  pe[,pe,...]\n");
      fprintf(fp, "job_identifier_list      [job_id|job_name|pattern]{, [job_id|job_name|pattern]}\n");
      fprintf(fp, "resource_list            resource[=value][,resource[=value],...]\n");
      fprintf(fp, "user_list                user|*|@group[,user|*|@group],...]\n");
      fprintf(fp, "resource_attributes      resource,resource,...\n");
      fprintf(fp, "wc_cqueue                %s\n", MSG_QSTAT_HELP_WCCQ);
      fprintf(fp, "wc_host                  %s\n", MSG_QSTAT_HELP_WCHOST);
      fprintf(fp, "wc_hostgroup             %s\n", MSG_QSTAT_HELP_WCHG);
      fprintf(fp, "wc_qinstance             wc_cqueue@wc_host\n");
      fprintf(fp, "wc_qdomain               wc_cqueue@wc_hostgroup\n");
      fprintf(fp, "wc_queue_list            wc_cqueue|wc_qdomain|wc_qinstance\n");
      fprintf(fp, "wc_queue_list            wc_queue[,wc_queue,...]\n");
   } else {
      /* display option usage */
      fprintf(fp, MSG_QDEL_not_available_OPT_USAGE_S,what);
      fprintf(fp, "\n");
   }
   return 1;
}
