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
/* Portions of this code are Copyright 2011 Univa Inc. */
/*___INFO__MARK_END__*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_stdio.h"
#include "uti/sge_prog.h"
#include "uti/sge_string.h"

#include "sgeobj/sge_feature.h"

#include "gdi/msg_gdilib.h"

#include "usage.h"
#include "sge_options.h"
#include "msg_common.h"

static void print_marked(u_long32 prog_number, FILE *fp);
static char* get_argument_syntax(u_long32 prog_number, int nr);
static void usage_silent(FILE *fp);


static int marker[OA__END];

void mark_argument_syntax(
int argument_number 
) {
   marker[argument_number] = 1;
}


static char* get_argument_syntax(u_long32 prog_number, int nr)
{ 
   switch (nr)
   {
     case OA_ACCOUNT_STRING:
       return MSG_GDI_ARGUMENTSYNTAX_OA_ACCOUNT_STRING;
     case OA_COMPLEX_LIST:
       return MSG_GDI_ARGUMENTSYNTAX_OA_COMPLEX_LIST;
     case OA_CONTEXT_LIST:
       return MSG_GDI_ARGUMENTSYNTAX_OA_CONTEXT_LIST; 
     case OA_CKPT_SEL:
         return MSG_GDI_ARGUMENTSYNTAX_OA_CKPT_SEL; 
     case OA_DATE_TIME:
         return MSG_GDI_ARGUMENTSYNTAX_OA_DATE_TIME; 
     case OA_DESTIN_ID_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_DESTIN_ID_LIST; 
     case OA_HOLD_LIST:
         if ((prog_number == QHOLD) ||
             (prog_number == QRLS)
         ){
            return MSG_GDI_ARGUMENTSYNTAX_OA_HOLD_LIST_QHOLD;
         }
         else {
            return MSG_GDI_ARGUMENTSYNTAX_OA_HOLD_LIST; 
         }
     case OA_HOST_ID_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_HOST_ID_LIST;
     case OA_HOSTNAME_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_HOSTNAME_LIST;
     case OA_JOB_ID_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_JOB_ID_LIST; 
     case OA_AR_ID:
         return MSG_GDI_ARGUMENTSYNTAX_OA_AR_ID; 
     case OA_AR_ID_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_AR_ID_LIST; 
     case OA_WC_AR:
         return MSG_GDI_ARGUMENTSYNTAX_OA_WC_AR; 
     case OA_WC_AR_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_WC_AR_LIST; 
     case OA_JOB_IDENTIFIER_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_JOB_IDENTIFIER_LIST; 
     case OA_JOB_QUEUE_DEST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_JOB_QUEUE_DEST; 
     case OA_LISTNAME_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_LISTNAME_LIST; 
     case OA_RQS_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_RQS_LIST; 
     case OA_MAIL_ADDRESS:
         return MSG_GDI_ARGUMENTSYNTAX_OA_MAIL_ADDRESS; 
     case OA_MAIL_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_MAIL_LIST; 
     case OA_MAIL_OPTIONS:
         return MSG_GDI_ARGUMENTSYNTAX_OA_MAIL_OPTIONS; 
     case OA_MAIL_OPTIONS_AR:
         return MSG_GDI_ARGUMENTSYNTAX_OA_MAIL_OPTIONS_AR; 
     case OA_NODE_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_NODE_LIST; 
     case OA_NODE_PATH:
         return MSG_GDI_ARGUMENTSYNTAX_OA_NODE_PATH; 
     case OA_NODE_SHARES_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_NODE_SHARES_LIST; 
     case OA_PATH:
         return MSG_GDI_ARGUMENTSYNTAX_OA_PATH; 
     case OA_PATH_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_PATH_LIST; 
     case OA_FILE_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_FILE_LIST; 
     case OA_PRIORITY:
         return MSG_GDI_ARGUMENTSYNTAX_OA_PRIORITY; 
     case OA_RESOURCE_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_RESOURCE_LIST; 
     case OA_SERVER:
         return MSG_GDI_ARGUMENTSYNTAX_OA_SERVER; 
     case OA_SERVER_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_SERVER_LIST; 
     case OA_SIGNAL:
         return MSG_GDI_ARGUMENTSYNTAX_OA_SIGNAL; 
     case OA_SIMPLE_CONTEXT_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_SIMPLE_CONTEXT_LIST; 
     case OA_SLOT_RANGE:
         return MSG_GDI_ARGUMENTSYNTAX_OA_SLOT_RANGE; 
     case OA_STATES:
         return MSG_GDI_ARGUMENTSYNTAX_OA_STATES; 
     case OA_JOB_TASK_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_JOB_TASK_LIST; 
     case OA_JOB_TASKS:
         if (prog_number == QRESUB) {
            return MSG_GDI_ARGUMENTSYNTAX_OA_JOB_TASKS_RESUB;
         }
         else {
            return MSG_GDI_ARGUMENTSYNTAX_OA_JOB_TASKS; 
         }
     case OA_TASK_ID_RANGE:
         return MSG_GDI_ARGUMENTSYNTAX_OA_TASK_ID_RANGE; 
     case OA_USER_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_USER_LIST; 
     case OA_VARIABLE_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_VARIABLE_LIST;
     case OA_OBJECT_NAME:
         return MSG_GDI_ARGUMENTSYNTAX_OA_OBJECT_NAME;
     case OA_ATTRIBUTE_NAME:
         return MSG_GDI_ARGUMENTSYNTAX_OA_ATTRIBUTE_NAME;
     case OA_OBJECT_ID_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_OBJECT_ID_LIST;
     case OA_PROJECT_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_PROJECT_LIST;
     case OA_EVENTCLIENT_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_EVENTCLIENT_LIST;
     case OA_HOST_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_HOST_LIST;
     case OA_JSV_URL:
         return MSG_GDI_ARGUMENTSYNTAX_OA_JSV_URL;
     case OA_WC_CQUEUE:
         return MSG_GDI_ARGUMENTSYNTAX_OA_WC_CQUEUE;
     case OA_WC_HOST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_WC_HOST;
     case OA_WC_HOSTGROUP:
         return MSG_GDI_ARGUMENTSYNTAX_OA_WC_HOSTGROUP;
     case OA_WC_QINSTANCE:
         return MSG_GDI_ARGUMENTSYNTAX_OA_WC_QINSTANCE;
     case OA_WC_QDOMAIN:
         return MSG_GDI_ARGUMENTSYNTAX_OA_WC_QDOMAIN;
     case OA_WC_QUEUE:
         return MSG_GDI_ARGUMENTSYNTAX_OA_WC_QUEUE;
     case OA_WC_QUEUE_LIST:
         return MSG_GDI_ARGUMENTSYNTAX_OA_WC_QUEUE_LIST;
     case OA_THREAD_NAME:
         return MSG_GDI_ARGUMENTSYNTAX_OA_THREAD_NAME;
     case OA_OBJECT_NAME2:
         return MSG_GDI_ARGUMENTSYNTAX_OA_OBJECT_NAME2;
     case OA_OBJECT_NAME3:
         return MSG_GDI_ARGUMENTSYNTAX_OA_OBJECT_NAME3;
     case OA_TIME:
         return MSG_GDI_ARGUMENTSYNTAX_OA_TIME;
     case OA_TASK_CONCURRENCY:
         return MSG_GDI_ARGUMENTSYNTAX_OA_TASK_CONCURRENCY;
     case OA_BINDING_EXPLICIT:
         return MSG_GDI_ARGUMENTSYNTAX_QA_BINDING_STRATEGY_EXP;
     case OA_BINDING_LINEAR:
         return MSG_GDI_ARGUMENTSYNTAX_QA_BINDING_STRATEGY_LIN;
     case OA_BINDING_STRIDING:
         return MSG_GDI_ARGUMENTSYNTAX_QA_BINDING_STRATEGY_STR;
     default:
         break; 
   }
  return ""; 
  /*(argument_syntax[nr]);*/
}

static void print_marked(
u_long32 prog_number,
FILE *fp 
) {
   int i;
   for (i=0; i<OA__END; i++)
      if (marker[i]==1)
         fprintf(fp, "%s\n", get_argument_syntax(prog_number, i));
}



void sge_usage(u_long32 prog_number, FILE *fp) {

  char namebuf[128];
  dstring ds;
  char buffer[256];
  const char *prog_name = prognames[prog_number];
  
#define PRINTITD(o,d) print_option_syntax(fp,o,d)
#define PRINTIT(o) print_option_syntax(fp,o,NULL)
#define MARK(n) mark_argument_syntax(n)

   DENTER(TOP_LAYER, "sge_usage");

   sge_dstring_init(&ds, buffer, sizeof(buffer));

   fprintf(fp, "%s\n", feature_get_product_name(FS_SHORT_VERSION, &ds));
   
   if (!strcmp(prog_name, "execd"))
      strcpy(namebuf, "sge_execd");
   else if (!strcmp(prog_name, "qmaster"))
      strcpy(namebuf, "sge_qmaster");
   else
      sge_strlcpy(namebuf, prog_name, sizeof(namebuf));
         
   if (VALID_OPT(JOB_ID_OPR, prog_number)) {
      fprintf(fp, "%s %s [options] %s\n", MSG_GDI_USAGE_USAGESTRING , namebuf, MSG_GDI_USAGE_JOB_ID_OPR);
   } else if (VALID_OPT(JOB_TASK_OPR, prog_number)) {
      fprintf(fp, "%s %s [options] %s\n", MSG_GDI_USAGE_USAGESTRING , namebuf, MSG_GDI_USAGE_TASK_OPR);
      MARK(OA_JOB_TASK_LIST);
      MARK(OA_JOB_TASKS);
      MARK(OA_TASK_ID_RANGE);
   } else {
      fprintf(fp, "%s %s [options]\n", MSG_GDI_USAGE_USAGESTRING , namebuf);
   }

   /* reset all option markers */
   memset(marker, 0, sizeof(marker));

   if (VALID_OPT(a_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_a_OPT_DATE_TIME , MSG_GDI_UTEXT_a_OPT_DATE_TIME);
      MARK(OA_DATE_TIME);
   }

   if (VALID_OPT(aattr_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_aattr_OPT, MSG_GDI_UTEXT_aattr_OPT);
      MARK(OA_OBJECT_NAME); 
      MARK(OA_ATTRIBUTE_NAME); 
      MARK(OA_OBJECT_ID_LIST); 
   } 

   if (VALID_OPT(Aattr_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Aattr_OPT, MSG_GDI_UTEXT_Aattr_OPT);
      MARK(OA_OBJECT_NAME); 
      MARK(OA_ATTRIBUTE_NAME); 
      MARK(OA_OBJECT_ID_LIST); 
   }  

   if (VALID_OPT(ac_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ac_OPT_CONTEXT_LIST, 
         MSG_GDI_UTEXT_ac_OPT_CONTEXT_LIST );
      MARK(OA_COMPLEX_LIST);
   }

   if (VALID_OPT(acal_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_acal_OPT_FNAME , MSG_GDI_UTEXT_acal_OPT_FNAME);
   }

   if (VALID_OPT(Acal_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Acal_OPT_FNAME, MSG_GDI_UTEXT_Acal_OPT_FNAME);
   }

   if (VALID_OPT(ackpt_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ackpt_OPT_CKPT_NAME, 
         MSG_GDI_UTEXT_ackpt_OPT_CKPT_NAME);
   }

   if (VALID_OPT(Ackpt_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Ackpt_OPT_FNAME, 
         MSG_GDI_UTEXT_Ackpt_OPT_FNAME);
   }

   if (VALID_OPT(aconf_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_aconf_OPT_HOST_LIST, 
         MSG_GDI_UTEXT_aconf_OPT_HOST_LIST);
   }

   if (VALID_OPT(Aconf_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Aconf_OPT_FILE_LIST, MSG_GDI_UTEXT_Aconf_OPT_FILE_LIST );
   }

   if (VALID_OPT(ae_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ae_OPT_EXEC_SERVER_TEMPLATE, MSG_GDI_UTEXT_ae_OPT_EXEC_SERVER_TEMPLATE );
   }

   if (VALID_OPT(Ae_OPT, prog_number)) {
      PRINTITD( MSG_GDI_USAGE_Ae_OPT_FNAME, MSG_GDI_UTEXT_Ae_OPT_FNAME );
   }

   if (VALID_OPT(ah_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ah_OPT_HOSTNAME, MSG_GDI_UTEXT_ah_OPT_HOSTNAME );
      MARK(OA_HOSTNAME_LIST);
   }

   if (VALID_OPT(ahgrp_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ahgrp_OPT, MSG_GDI_UTEXT_ahgrp_OPT);
   }

   if (VALID_OPT(Ahgrp_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Ahgrp_OPT, MSG_GDI_UTEXT_Ahgrp_OPT);
   }

   if (VALID_OPT(arqs_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_arqs_OPT, MSG_GDI_UTEXT_arqs_OPT);
      MARK(OA_RQS_LIST);
   }

   if (VALID_OPT(Arqs_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Arqs_OPT, MSG_GDI_UTEXT_Arqs_OPT);
      MARK(OA_RQS_LIST);
   }

   if (VALID_OPT(am_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_am_OPT_USER_LIST, MSG_GDI_UTEXT_am_OPT_USER_LIST);
      MARK(OA_USER_LIST);
   }


   if (VALID_OPT(ao_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ao_OPT_USER_LIST , MSG_GDI_UTEXT_ao_OPT_USER_LIST );
      MARK(OA_USER_LIST);
   }

   if (VALID_OPT(ap_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ap_OPT_PE_NAME , MSG_GDI_UTEXT_ap_OPT_PE_NAME);
   }

   if (VALID_OPT(Ap_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Ap_OPT_FNAME , MSG_GDI_UTEXT_Ap_OPT_FNAME );
   }

   if (VALID_OPT(aprj_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_APRJ , MSG_GDI_UTEXT_APRJ );
   }

   if (VALID_OPT(Aprj_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Aprj , MSG_GDI_UTEXT_Aprj );
   }

   if (VALID_OPT(aq_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_aq_OPT_Q_TEMPLATE , MSG_GDI_UTEXT_aq_OPT_Q_TEMPLATE );
   }

   if (VALID_OPT(Aq_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Aq_OPT_FNAME , MSG_GDI_UTEXT_Aq_OPT_FNAME );
   }

   if (VALID_OPT(ar_OPT, prog_number)) {
      if (prog_number == QRSTAT) {
         PRINTITD(MSG_GDI_USAGE_ar_list_OPT, MSG_GDI_UTEXT_ar_QRSTAT_OPT);
         MARK(OA_AR_ID_LIST);
         MARK(OA_AR_ID);
      } else if (prog_number == QRDEL) {
         PRINTITD(MSG_GDI_USAGE_wc_ar_list_OPT, MSG_GDI_UTEXT_wc_ar_list_OPT);
         MARK(OA_WC_AR_LIST);
         MARK(OA_WC_AR);
      } else {
         PRINTITD(MSG_GDI_USAGE_ar_OPT, MSG_GDI_UTEXT_ar_OPT);
         MARK(OA_AR_ID);
      }
   }

   if (VALID_OPT(as_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_as_OPT_HOSTNAME , MSG_GDI_UTEXT_as_OPT_HOSTNAME);
      MARK(OA_HOSTNAME_LIST);
   }

   if (VALID_OPT(astnode_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ASTNODE_NODE_SHARES_LIST , MSG_GDI_UTEXT_ASTNODE_NODE_SHARES_LIST );
      MARK(OA_NODE_SHARES_LIST);
      MARK(OA_NODE_PATH);
   }

   if (VALID_OPT(astree_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ASTREE , MSG_GDI_UTEXT_ASTREE);
   }

   if (VALID_OPT(Astree_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ASTREE_FNAME, MSG_GDI_UTEXT_ASTREE_FNAME);
   }

   if (VALID_OPT(at_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_at_OPT, MSG_GDI_UTEXT_at_OPT);
      MARK(OA_THREAD_NAME);
   }

   if (VALID_OPT(au_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_au_OPT_USER_LIST_LISTNAME_LIST , MSG_GDI_UTEXT_au_OPT_USER_LIST_LISTNAME_LIST );
      MARK(OA_USER_LIST);
      MARK(OA_LISTNAME_LIST);
   }

   if (VALID_OPT(Au_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Au_OPT_LISTNAME_LIST , MSG_GDI_UTEXT_Au_OPT_LISTNAME_LIST );
   }

#ifndef __SGE_NO_USERMAPPING__
   if (VALID_OPT(aumap_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_aumap_OPT, MSG_GDI_UTEXT_aumap_OPT );
   }

   if (VALID_OPT(Aumap_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Aumap_OPT, MSG_GDI_UTEXT_Aumap_OPT );
   }
#endif 
 
   if (VALID_OPT(aus_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_AUSER , MSG_GDI_UTEXT_AUSER );
   }

   if (VALID_OPT(Aus_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Auser , MSG_GDI_UTEXT_Auser );
   }

   if (VALID_OPT(A_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_A_OPT_ACCOUNT_STRING , MSG_GDI_UTEXT_A_OPT_ACCOUNT_STRING );
      MARK(OA_ACCOUNT_STRING);
   }

   if (VALID_OPT(b_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_b_OPT_YN, MSG_GDI_UTEXT_b_OPT_YN);
   }
   
   if (VALID_OPT(binding_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_binding_OPT_YN, MSG_GDI_UTEXT_binding_OPT_YN);
      MARK(OA_BINDING_EXPLICIT);
      MARK(OA_BINDING_LINEAR);
      MARK(OA_BINDING_STRIDING);
   }

   if (VALID_OPT(c_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_c_OPT_CKPT_SELECTOR ,  MSG_GDI_UTEXT_c_OPT_CKPT_SELECTOR );
      MARK(OA_CKPT_SEL);
   }

   if (VALID_OPT(cl_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_c_OPT , MSG_GDI_UTEXT_c_OPT );
   }

   if (VALID_OPT(ckptobj_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ckpt_OPT_CKPT_NAME ,  MSG_GDI_UTEXT_ckpt_OPT_CKPT_NAME );
   }

   if (VALID_OPT(clear_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_clear_OPT, MSG_GDI_UTEXT_clear_OPT);
   }

   if (VALID_OPT(cu_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_clearusage_OPT, MSG_GDI_UTEXT_clearusage_OPT);
   }

   if (VALID_OPT(cwd_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_cwd_OPT, MSG_GDI_UTEXT_cwd_OPT);
   }

   if (VALID_OPT(cq_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_cq_OPT_DESTIN_ID_LIST, 
         MSG_GDI_UTEXT_cq_OPT_DESTIN_ID_LIST);
      MARK(OA_DESTIN_ID_LIST);
   }

   if (VALID_OPT(C_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_C_OPT_DIRECTIVE_PREFIX, 
         MSG_GDI_UTEXT_C_OPT_DIRECTIVE_PREFIX);
   }

   if (VALID_OPT(d_OPT, prog_number)) {
      if (prog_number == QRSUB) {
         PRINTITD(MSG_GDI_USAGE_d_OPT_TIME, MSG_GDI_UTEXT_d_OPT_TIME);
         MARK(OA_TIME);
      } else {
         PRINTITD(MSG_GDI_USAGE_d_OPT, MSG_GDI_UTEXT_d_OPT);
      }
   }

   if (VALID_OPT(dattr_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dattr_OPT, MSG_GDI_UTEXT_dattr_OPT);
      MARK(OA_OBJECT_NAME);
      MARK(OA_ATTRIBUTE_NAME);
      MARK(OA_OBJECT_ID_LIST);  
   }

   if (VALID_OPT(Dattr_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Dattr_OPT, MSG_GDI_UTEXT_Dattr_OPT);
      MARK(OA_OBJECT_NAME);
      MARK(OA_ATTRIBUTE_NAME);
      MARK(OA_OBJECT_ID_LIST);  
   }    

   if (VALID_OPT(dc_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dc_OPT_SIMPLE_COMPLEX_LIST , MSG_GDI_UTEXT_dc_OPT_SIMPLE_COMPLEX_LIST );
      MARK(OA_SIMPLE_CONTEXT_LIST);
   }

   if (VALID_OPT(dcal_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dcal_OPT_CALENDAR_NAME , MSG_GDI_UTEXT_dcal_OPT_CALENDAR_NAME );
   }

   if (VALID_OPT(dckpt_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dckpt_OPT_CKPT_NAME , MSG_GDI_UTEXT_dckpt_OPT_CKPT_NAME );
   }

   if (VALID_OPT(dconf_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dconf_OPT_HOST_LIST , MSG_GDI_UTEXT_dconf_OPT_HOST_LIST );
   }

   if (VALID_OPT(de_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_de_OPT_HOST_LIST , MSG_GDI_UTEXT_de_OPT_HOST_LIST );
   }

   if (VALID_OPT(display_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_display_OPT_DISPLAY, MSG_GDI_UTEXT_display_OPT_DISPLAY );
   }

   if (VALID_OPT(dh_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dh_OPT_HOST_LIST , MSG_GDI_UTEXT_dh_OPT_HOST_LIST );
   }

   if (VALID_OPT(dhgrp_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dhgrp_OPT, MSG_GDI_UTEXT_dhgrp_OPT);
   }

   if (VALID_OPT(dl_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dl_OPT_DATE_TIME , MSG_GDI_UTEXT_dl_OPT_DATE_TIME );
      MARK(OA_DATE_TIME);
   }

   if (VALID_OPT(drqs_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_drqs_OPT, MSG_GDI_UTEXT_drqs_OPT);
      MARK(OA_RQS_LIST);
   }

   if (VALID_OPT(dm_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dm_OPT_USER_LIST , MSG_GDI_UTEXT_dm_OPT_USER_LIST );
      MARK(OA_USER_LIST);
   }

   if (VALID_OPT(do_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_do_OPT_USER_LIST , MSG_GDI_UTEXT_do_OPT_USER_LIST );
      MARK(OA_USER_LIST);
   }

   if (VALID_OPT(dp_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dp_OPT_PE_NAME, MSG_GDI_UTEXT_dp_OPT_PE_NAME );
   }

   if (VALID_OPT(dprj_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dprj_OPT_PROJECT, MSG_GDI_UTEXT_dprj_OPT_PROJECT );
      MARK(OA_PROJECT_LIST);
   }

   if (VALID_OPT(dq_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dq_OPT_DESTIN_ID_LIST, 
         MSG_GDI_UTEXT_dq_OPT_DESTIN_ID_LIST);
      MARK(OA_DESTIN_ID_LIST);
   }

   if (VALID_OPT(ds_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ds_OPT_HOST_LIST, MSG_GDI_UTEXT_ds_OPT_HOST_LIST);
   }

   if (VALID_OPT(dstnode_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_DSTNODE_NODELIST, MSG_GDI_UTEXT_DSTNODE_NODELIST);
      MARK(OA_NODE_LIST);
      MARK(OA_NODE_PATH);
   }

   if (VALID_OPT(dstree_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_DSTREE  , MSG_GDI_UTEXT_DSTREE );
   }

   if (VALID_OPT(du_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_du_OPT_USER_LIST_LISTNAME_LIST , MSG_GDI_UTEXT_du_OPT_USER_LIST_LISTNAME_LIST );
      MARK(OA_USER_LIST);
      MARK(OA_LISTNAME_LIST);
   }

   if (VALID_OPT(dul_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dul_OPT_LISTNAME_LIST , MSG_GDI_UTEXT_dul_OPT_LISTNAME_LIST );
      MARK(OA_LISTNAME_LIST);
   }
   
#ifndef __SGE_NO_USERMAPPING__
   if (VALID_OPT(dumap_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_dumap_OPT, MSG_GDI_UTEXT_dumap_OPT );
   }
#endif
   if (VALID_OPT(dus_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_DUSER_USER, MSG_GDI_UTEXT_DUSER_USER );
      MARK(OA_USER_LIST);
   }


   if (VALID_OPT(e_OPT, prog_number)) {
      if (prog_number == QMOD) {
         PRINTITD(MSG_GDI_USAGE_e_OPT , MSG_GDI_UTEXT_e_OPT);
      } else if (prog_number == QRSUB) {      
         PRINTITD(MSG_GDI_USAGE_e_OPT_END_TIME , MSG_GDI_UTEXT_e_OPT_END_TIME);
         MARK(OA_DATE_TIME);
      } else {
         PRINTITD(MSG_GDI_USAGE_e_OPT_PATH_LIST, MSG_GDI_UTEXT_e_OPT_PATH_LIST );
         MARK(OA_PATH_LIST);
      }
   }

   if (VALID_OPT(ext_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ext_OPT, MSG_GDI_UTEXT_ext_OPT );
   }

   if (VALID_OPT(f_OPT, prog_number)) {
      if (prog_number == QSTAT) 
         PRINTITD(MSG_GDI_USAGE_f_OPT,MSG_GDI_UTEXT_f_OPT_FULL_OUTPUT );
      else
         PRINTITD(MSG_GDI_USAGE_f_OPT,MSG_GDI_UTEXT_f_OPT_FORCE_ACTION );
   }

   if (VALID_OPT(h_OPT, prog_number)) {
      if ((prog_number == QALTER) || (prog_number == QRESUB)  || 
          (prog_number == QHOLD)  || (prog_number == QRLS)) {
         PRINTITD(MSG_GDI_USAGE_h_OPT_HOLD_LIST , MSG_GDI_UTEXT_h_OPT_HOLD_LIST );
         MARK(OA_HOLD_LIST);
      } else { /* QSUB */
         PRINTITD(MSG_GDI_USAGE_h_OPT, MSG_GDI_UTEXT_h_OPT);
      }
   }

   if (VALID_OPT(hard_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_hard_OPT , MSG_GDI_UTEXT_hard_OPT );
   }

   if (VALID_OPT(he_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_he_OPT , MSG_GDI_UTEXT_he_OPT );
   }

   if (VALID_OPT(help_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_help_OPT , MSG_GDI_UTEXT_help_OPT );
   }

   if (VALID_OPT(hold_jid_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_hold_jid_OPT , MSG_GDI_UTEXT_hold_jid_OPT );
      MARK(OA_JOB_IDENTIFIER_LIST);
   }

   if (VALID_OPT(hold_jid_ad_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_hold_jid_ad_OPT , MSG_GDI_UTEXT_hold_jid_ad_OPT );
      MARK(OA_JOB_IDENTIFIER_LIST);
   }
   
   if (VALID_OPT(i_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_i_OPT_PATH_LIST, MSG_GDI_UTEXT_i_OPT_PATH_LIST );
      MARK(OA_PATH_LIST);
   }

   if (VALID_OPT(inherit_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_inherit_OPT, MSG_GDI_UTEXT_inherit_OPT );
   }

   if (VALID_OPT(j_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_j_OPT_YN , MSG_GDI_UTEXT_j_OPT_YN );
   }

   if (VALID_OPT(jid_OPT, prog_number)) {
      if (prog_number == QSTAT) {
         PRINTITD(MSG_GDI_USAGE_jid_OPT_JOB_ID_LIST , MSG_GDI_UTEXT_jid_OPT_JOB_ID_LIST_PRINTED );
         MARK(OA_JOB_ID_LIST);
      } else if (prog_number == QALTER) {
         PRINTITD(MSG_GDI_USAGE_jid_OPT_JOB_ID_LIST , MSG_GDI_UTEXT_jid_OPT_JOB_ID_LIST_ALTERED );
         MARK(OA_JOB_ID_LIST);
      } else {
         PRINTIT(MSG_GDI_USAGE_jid_OPT_JID );
      }
   }

   if (VALID_OPT(js_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_js_OPT_YN , MSG_GDI_UTEXT_js_OPT_YN );
   }

   if (VALID_OPT(jsv_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_jsv_OPT_YN , MSG_GDI_UTEXT_jsv_OPT_YN );
      MARK(OA_JSV_URL);
   }

   if (VALID_OPT(ke_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ke_OPT_HOSTS, MSG_GDI_UTEXT_ke_OPT_HOSTS );
      PRINTITD(MSG_GDI_USAGE_k_OPT_MASTERORSCHEDULINGDAEMON, MSG_GDI_UTEXT_k_OPT_MASTERORSCHEDULINGDAEMON );
      MARK(OA_HOST_LIST);
   }

   if (VALID_OPT(kec_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_kec_OPT, MSG_GDI_UTEXT_kec_OPT );
      MARK(OA_EVENTCLIENT_LIST);
   }

   if (VALID_OPT(kt_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_kt_OPT, MSG_GDI_UTEXT_kt_OPT );
      MARK(OA_THREAD_NAME);
   }

   if (VALID_OPT(l_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_l_OPT_RESOURCE_LIST , MSG_GDI_UTEXT_l_OPT_RESOURCE_LIST );
      MARK(OA_RESOURCE_LIST);
   }

   if (VALID_OPT(M_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_M_OPT_MAIL_LIST, MSG_GDI_UTEXT_M_OPT_MAIL_LIST);
      MARK(OA_MAIL_LIST);
      MARK(OA_MAIL_ADDRESS);
   }

   if (VALID_OPT(m_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_m_OPT_MAIL_OPTIONS, 
         MSG_GDI_UTEXT_m_OPT_MAIL_OPTIONS);
      if ( prog_number == QRSUB ) {
        MARK(OA_MAIL_OPTIONS_AR);
      } else {
        MARK(OA_MAIL_OPTIONS);
      }  
   }

   if (VALID_OPT(masterq_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_masterq_OPT_DESTIN_ID_LIST, 
         MSG_GDI_UTEXT_masterq_OPT_DESTIN_ID_LIST_BIND );
      MARK(OA_WC_CQUEUE);
      MARK(OA_WC_HOST);
      MARK(OA_WC_HOSTGROUP);
      MARK(OA_WC_QINSTANCE);
      MARK(OA_WC_QDOMAIN);
      MARK(OA_WC_QUEUE);
      MARK(OA_WC_QUEUE_LIST);
   }

   if (VALID_OPT(mattr_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_mattr_OPT, MSG_GDI_UTEXT_mattr_OPT);
      MARK(OA_OBJECT_NAME); 
      MARK(OA_ATTRIBUTE_NAME); 
      MARK(OA_OBJECT_ID_LIST); 
   }

   if (VALID_OPT(Mattr_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Mattr_OPT, MSG_GDI_UTEXT_Mattr_OPT);
      MARK(OA_OBJECT_NAME); 
      MARK(OA_ATTRIBUTE_NAME); 
      MARK(OA_OBJECT_ID_LIST); 
   }    

   if (VALID_OPT(mc_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_mc_OPT_COMPLEX, MSG_GDI_UTEXT_mc_OPT_COMPLEX);
   }

   if (VALID_OPT(Mc_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Mc_OPT_COMPLEX_NAME_FNAME, 
         MSG_GDI_UTEXT_Mc_OPT_COMPLEX_NAME_FNAME);
   }

   if (VALID_OPT(mcal_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_mcal_OPT_CALENDAR_NAME, 
         MSG_GDI_UTEXT_mcal_OPT_CALENDAR_NAME);
   }

   if (VALID_OPT(Mcal_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Mcal_OPT_FNAME, MSG_GDI_UTEXT_Mcal_OPT_FNAME);
   }

   if (VALID_OPT(mckpt_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_mckpt_OPT_CKPT_NAME,
         MSG_GDI_UTEXT_mckpt_OPT_CKPT_NAME);
   }

   if (VALID_OPT(Mckpt_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Mckpt_OPT_FNAME , MSG_GDI_UTEXT_Mckpt_OPT_FNAME );
   }

   if (VALID_OPT(mconf_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_mconf_OPT_HOSTLISTORGLOBAL, 
         MSG_GDI_UTEXT_mconf_OPT_HOSTLISTORGLOBAL);
   }

   if (VALID_OPT(Mconf_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Mconf_OPT_FILE_LIST, MSG_GDI_UTEXT_Mconf_OPT_FILE_LIST );
   }

   if (VALID_OPT(me_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_me_OPT_SERVER, MSG_GDI_UTEXT_me_OPT_SERVER);
   }

   if (VALID_OPT(Me_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Me_OPT_FNAME, MSG_GDI_UTEXT_Me_OPT_FNAME);
   }

   if (VALID_OPT(mhgrp_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_mhgrp_OPT, MSG_GDI_UTEXT_mhgrp_OPT);
   }

   if (VALID_OPT(Mhgrp_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Mhgrp_OPT, MSG_GDI_UTEXT_Mhgrp_OPT);
   }

   if (VALID_OPT(mrqs_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_mrqs_OPT, MSG_GDI_UTEXT_mrqs_OPT);
      MARK(OA_RQS_LIST);
   }

   if (VALID_OPT(Mrqs_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Mrqs_OPT, MSG_GDI_UTEXT_Mrqs_OPT);
      MARK(OA_RQS_LIST);
   }

   if (VALID_OPT(mp_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_mp_OPT_PE_NAME , MSG_GDI_UTEXT_mp_OPT_PE_NAME );
   }

   if (VALID_OPT(Mp_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Mp_OPT_FNAME , MSG_GDI_UTEXT_Mp_OPT_FNAME );
   }

   if (VALID_OPT(mprj_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_mprj_OPT_PROJECT, MSG_GDI_UTEXT_mprj_OPT_PROJECT);
   }

   if (VALID_OPT(Mprj_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Mprj_OPT_PROJECT, MSG_GDI_UTEXT_Mprj_OPT_PROJECT);
   }

   if (VALID_OPT(mq_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_mq_OPT_QUEUE , MSG_GDI_UTEXT_mq_OPT_QUEUE );
   }

   if (VALID_OPT(Mq_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Mq_OPT_FNAME, MSG_GDI_UTEXT_Mq_OPT_FNAME);
   }

   if (VALID_OPT(msconf_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_msconf_OPT, MSG_GDI_UTEXT_msconf_OPT);
   }
   
   if (VALID_OPT(Msconf_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Msconf_OPT, MSG_GDI_UTEXT_Msconf_OPT);
   }
   
   if (VALID_OPT(mstnode_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_MSTNODE_NODE_SHARES_LIST, 
         MSG_GDI_UTEXT_MSTNODE_NODE_SHARES_LIST );
      MARK(OA_NODE_SHARES_LIST);
      MARK(OA_NODE_PATH);
   }

   if (VALID_OPT(mstree_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_MSTREE , MSG_GDI_UTEXT_MSTREE );
   }

   if (VALID_OPT(Mstree_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_MSTREE_FNAME, MSG_GDI_UTEXT_MSTREE_FNAME);
   }

   if (VALID_OPT(mu_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_mu_OPT_LISTNAME_LIST, 
         MSG_GDI_UTEXT_mu_OPT_LISTNAME_LIST);
      MARK(OA_LISTNAME_LIST);
   }
   
   if (VALID_OPT(Mu_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Mu_OPT_LISTNAME_LIST, 
         MSG_GDI_UTEXT_Mu_OPT_LISTNAME_LIST);
   }

#ifndef __SGE_NO_USERMAPPING__
   if (VALID_OPT(Mumap_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Mumap_OPT, MSG_GDI_UTEXT_Mumap_OPT);
   }

   if (VALID_OPT(mumap_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_mumap_OPT, MSG_GDI_UTEXT_mumap_OPT);
   }
#endif
   if (VALID_OPT(mus_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_muser_OPT_USER, MSG_GDI_UTEXT_muser_OPT_USER);
   }

   if (VALID_OPT(Mus_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Muser_OPT_USER, MSG_GDI_UTEXT_Muser_OPT_USER);
   }

   if (VALID_OPT(N_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_N_OPT_NAME, MSG_GDI_UTEXT_N_OPT_NAME);
   }

   if (VALID_OPT(nostdin_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_nostdin_OPT, MSG_GDI_UTEXT_nostdin_OPT);
   }

   if (VALID_OPT(noshell_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_noshell_OPT, MSG_GDI_UTEXT_noshell_OPT);
   }

   if (VALID_OPT(notify_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_notify_OPT, MSG_GDI_UTEXT_notify_OPT);
   }

   if (VALID_OPT(now_OPT, prog_number)) {
      if (prog_number == QRSUB) {
         PRINTITD(MSG_GDI_USAGE_now_OPT_YN, MSG_GDI_UTEXT_now_qrsub_OPT_YN);
      } else {
         PRINTITD(MSG_GDI_USAGE_now_OPT_YN, MSG_GDI_UTEXT_now_OPT_YN);
      }
   }

   if (VALID_OPT(o_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_o_OPT_PATH_LIST, MSG_GDI_UTEXT_o_OPT_PATH_LIST);
      MARK(OA_PATH_LIST);
   }

   if (VALID_OPT(ot_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ot_OPT_TICKETS, MSG_GDI_UTEXT_ot_OPT_TICKETS);
   }

   if (VALID_OPT(P_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_P_OPT_PROJECT_NAME, 
         MSG_GDI_UTEXT_P_OPT_PROJECT_NAME);
   }
   if (VALID_OPT(p_OPT, prog_number)) {
      if (prog_number == QSELECT || prog_number == QSH || prog_number == QLOGIN 
          || prog_number == QRSH ||prog_number == QSUB || prog_number == QALTER) {
         PRINTITD(MSG_GDI_USAGE_p_OPT_PRIORITY, MSG_GDI_UTEXT_p_OPT_PRIORITY);
      } else {
         PRINTIT(MSG_GDI_USAGE_p_OPT_PRIORITY);
      }
      MARK(OA_PRIORITY);
   }

   if (VALID_OPT(pe_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_pe_OPT_PE_NAME_SLOT_RANGE, 
         MSG_GDI_UTEXT_pe_OPT_PE_NAME_SLOT_RANGE );
      MARK(OA_SLOT_RANGE);
   }

   if (VALID_OPT(pty_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_pty_OPT, MSG_GDI_UTEXT_pty_OPT );
   }

   if (VALID_OPT(purge_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_purge_OPT,
         MSG_GDI_UTEXT_purge_OPT);
      MARK(OA_OBJECT_NAME3);
   }

   if (VALID_OPT(q_OPT, prog_number)) {
      if (prog_number == QSUB || prog_number == QALTER || prog_number == QSH 
          || prog_number == QLOGIN || prog_number == QRSH ) {
         PRINTITD(MSG_GDI_USAGE_q_OPT_DESTIN_ID_LIST, 
            MSG_GDI_UTEXT_q_OPT_DESTIN_ID_LIST_BIND);
      } else {
         PRINTITD(MSG_GDI_USAGE_q_OPT_DESTIN_ID_LIST, 
            MSG_GDI_UTEXT_q_OPT_DESTIN_ID_LIST_INFO);
      }
      MARK(OA_WC_CQUEUE);
      MARK(OA_WC_HOST);
      MARK(OA_WC_HOSTGROUP);
      MARK(OA_WC_QINSTANCE);
      MARK(OA_WC_QDOMAIN);
      MARK(OA_WC_QUEUE);
      MARK(OA_WC_QUEUE_LIST);
   }

   if (VALID_OPT(R_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_R_OPT_YN , MSG_GDI_UTEXT_R_OPT_YN );
   }

   if (VALID_OPT(r_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_r_OPT_YN, MSG_GDI_UTEXT_r_OPT_YN);
   }

   if (VALID_OPT(res_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_res_OPT, MSG_GDI_UTEXT_res_OPT);
   }

   if (VALID_OPT(s_OPT, prog_number)) {
      if (prog_number == EXECD || prog_number == SCHEDD) { 
         usage_silent(fp);
      } else if (prog_number == QSELECT) {
         PRINTIT(MSG_GDI_USAGE_s_OPT_STATES );
         MARK(OA_STATES);
      }
      else if (prog_number == QMOD) {
         PRINTITD(MSG_GDI_USAGE_s_OPT ,MSG_GDI_UTEXT_s_OPT );
      }
      else if (prog_number != QMASTER) {
         PRINTIT(MSG_GDI_USAGE_s_OPT_SIGNAL);
         MARK(OA_SIGNAL);
      }
   }

   if (VALID_OPT(rattr_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_rattr_OPT, MSG_GDI_UTEXT_rattr_OPT);
      MARK(OA_OBJECT_NAME);
      MARK(OA_ATTRIBUTE_NAME);
      MARK(OA_OBJECT_ID_LIST);  
   }

   if (VALID_OPT(Rattr_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_Rattr_OPT, MSG_GDI_UTEXT_Rattr_OPT);
      MARK(OA_OBJECT_NAME);
      MARK(OA_ATTRIBUTE_NAME);
      MARK(OA_OBJECT_ID_LIST);  
   }    

   if (VALID_OPT(rsstnode_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_rsstnode_OPT_NODE_LIST,
         MSG_GDI_UTEXT_rsstnode_OPT_NODE_LIST);
      MARK(OA_NODE_LIST);
      MARK(OA_NODE_PATH);
   }

   if (VALID_OPT(sc_OPT, prog_number)) {
      if(prog_number == QCONF) {
         PRINTITD(MSG_GDI_USAGE_sc_OPT_COMPLEX_LIST, 
            MSG_GDI_UTEXT_sc_OPT_COMPLEX_LIST_SHOW);
         MARK(OA_COMPLEX_LIST);
      }
      else {
         PRINTITD(MSG_GDI_USAGE_sc_OPT_CONTEXT_LIST, 
            MSG_GDI_UTEXT_sc_OPT_CONTEXT_LIST_SET);
         MARK(OA_CONTEXT_LIST);
      }
   }

   if (VALID_OPT(scal_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_scal_OPT_CALENDAR_NAME, 
         MSG_GDI_UTEXT_scal_OPT_CALENDAR_NAME);
   }

   if (VALID_OPT(scall_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_scall_OPT, MSG_GDI_UTEXT_scall_OPT);
   }

   if (VALID_OPT(sckpt_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sckpt_OPT_CKPT_NAME, 
         MSG_GDI_UTEXT_sckpt_OPT_CKPT_NAME);
   }

   if (VALID_OPT(sckptl_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sckptl_OPT, MSG_GDI_UTEXT_sckptl_OPT);
   }

   if (VALID_OPT(sconf_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sconf_OPT_HOSTLISTORGLOBAL, 
         MSG_GDI_UTEXT_sconf_OPT_HOSTLISTORGLOBAL);
   }

   if (VALID_OPT(sconfl_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sconfl_OPT, MSG_GDI_UTEXT_sconfl_OPT);
   }

   if (VALID_OPT(sds_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sds_OPT , MSG_GDI_UTEXT_sds_OPT );
   }

   if (VALID_OPT(se_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_se_OPT_SERVER, MSG_GDI_UTEXT_se_OPT_SERVER);
   }

   if (VALID_OPT(secl_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_secl_OPT, MSG_GDI_UTEXT_secl_OPT );
   }

   if (VALID_OPT(sel_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sel_OPT, MSG_GDI_UTEXT_sel_OPT);
   }

   if (VALID_OPT(sh_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sh_OPT, MSG_GDI_UTEXT_sh_OPT);
   }

   if (VALID_OPT(shell_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_shell_OPT_YN, MSG_GDI_UTEXT_shell_OPT_YN);
   }

   if (VALID_OPT(shgrp_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_shgrp_OPT, MSG_GDI_UTEXT_shgrp_OPT);
   }
   if (VALID_OPT(shgrp_tree_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_shgrp_tree_OPT, MSG_GDI_UTEXT_shgrp_tree_OPT);
   }
   if (VALID_OPT(shgrp_resolved_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_shgrp_resolved_OPT, MSG_GDI_UTEXT_shgrp_resolved_OPT);
   }

   if (VALID_OPT(shgrpl_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_shgrpl_OPT, MSG_GDI_UTEXT_shgrpl_OPT);
   }

   if (VALID_OPT(sm_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sm_OPT , MSG_GDI_UTEXT_sm_OPT );
   }

   if (VALID_OPT(so_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_so_OPT , MSG_GDI_UTEXT_so_OPT );
   }

   if (VALID_OPT(sobjl_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sobjl_OPT, MSG_GDI_UTEXT_sobjl_OPT);
      MARK(OA_OBJECT_NAME2); 
      MARK(OA_ATTRIBUTE_NAME); 
   } 

   if (VALID_OPT(soft_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_soft_OPT ,  MSG_GDI_UTEXT_soft_OPT );
   }

   if (VALID_OPT(sp_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sp_OPT_PE_NAME , MSG_GDI_UTEXT_sp_OPT_PE_NAME );
   }

   if (VALID_OPT(spl_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_spl_OPT , MSG_GDI_UTEXT_spl_OPT );
   }

   if (VALID_OPT(sprj_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sprj_OPT_PROJECT, MSG_GDI_UTEXT_sprj_OPT_PROJECT);
   }

   if (VALID_OPT(sprjl_OPT, prog_number)) {
      PRINTITD( MSG_GDI_USAGE_sprjl_OPT, MSG_GDI_UTEXT_sprjl_OPT);
   }

   if (VALID_OPT(sq_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sq_OPT_DESTIN_ID_LIST, 
         MSG_GDI_UTEXT_sq_OPT_DESTIN_ID_LIST);
      MARK(OA_DESTIN_ID_LIST);
   }

   if (VALID_OPT(sql_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sql_OPT , MSG_GDI_UTEXT_sql_OPT );
   }

   if (VALID_OPT(srqs_OPT,  prog_number)) {
      PRINTITD(MSG_GDI_USAGE_srqs_OPT , MSG_GDI_UTEXT_srqs_OPT );
      MARK(OA_RQS_LIST);
   }

   if (VALID_OPT(srqsl_OPT,  prog_number)) {
      PRINTITD(MSG_GDI_USAGE_srqsl_OPT , MSG_GDI_UTEXT_srqsl_OPT );
      MARK(OA_RQS_LIST);
   }

   if (VALID_OPT(ss_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ss_OPT, MSG_GDI_UTEXT_ss_OPT );
   } 

   if (VALID_OPT(sss_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sss_OPT , MSG_GDI_UTEXT_sss_OPT );
   } 

   if (VALID_OPT(ssconf_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_ssconf_OPT , MSG_GDI_UTEXT_ssconf_OPT );
   } 

   if (VALID_OPT(sstnode_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sstnode_OPT_NODE_LIST, 
         MSG_GDI_UTEXT_sstnode_OPT_NODE_LIST);
      MARK(OA_NODE_LIST);
      MARK(OA_NODE_PATH);
   }

   if (VALID_OPT(sst_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sst_OPT, MSG_GDI_UTEXT_sst_OPT);
   }

   if (VALID_OPT(sstree_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sstree_OPT, MSG_GDI_UTEXT_sstree_OPT);
   }


   if (VALID_OPT(su_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_su_OPT_LISTNAME_LIST, 
         MSG_GDI_UTEXT_su_OPT_LISTNAME_LIST);
      MARK(OA_LISTNAME_LIST);
   }
#ifndef __SGE_NO_USERMAPPING__
   if (VALID_OPT(sumap_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sumap_OPT, MSG_GDI_UTEXT_sumap_OPT);
   }

   if (VALID_OPT(sumapl_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sumapl_OPT, MSG_GDI_UTEXT_sumapl_OPT);
   }
#endif
   if (VALID_OPT(sul_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sul_OPT, MSG_GDI_UTEXT_sul_OPT);
   }

   if (VALID_OPT(sus_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_suser_OPT_USER, MSG_GDI_UTEXT_suser_OPT_USER);
      MARK(OA_USER_LIST);
   }

   if (VALID_OPT(susl_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_suserl_OPT, MSG_GDI_UTEXT_suserl_OPT);
   }

   if (VALID_OPT(sync_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_sync_OPT_YN, MSG_GDI_UTEXT_sync_OPT_YN);
   }

   if (VALID_OPT(S_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_S_OPT_PATH_LIST, MSG_GDI_UTEXT_S_OPT_PATH_LIST);
      MARK(OA_PATH_LIST);
   }


   if (VALID_OPT(t_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_t_OPT_TASK_ID_RANGE, 
         MSG_GDI_UTEXT_t_OPT_TASK_ID_RANGE );
      MARK(OA_TASK_ID_RANGE);
   }

   if (VALID_OPT(tc_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_tc_OPT, MSG_GDI_UTEXT_tc_OPT);
      MARK(OA_TASK_CONCURRENCY);
   }

   if (VALID_OPT(terse_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_terse_OPT , MSG_GDI_UTEXT_terse_OPT );
   }

   if (VALID_OPT(tsm_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_tsm_OPT , MSG_GDI_UTEXT_tsm_OPT );
   }

   if (VALID_OPT(u_OPT, prog_number)) {
     
      if (prog_number == QDEL) {
         PRINTITD(MSG_GDI_USAGE_u_OPT_USERLISTORUALL, 
         MSG_GDI_UTEXT_u_OPT_USERLISTORUALL_QDEL);
      } else if  ((prog_number == QRSTAT) || (prog_number == QRDEL)) {
         PRINTITD(MSG_GDI_USAGE_u_OPT_USERLISTORUALL, 
         MSG_GDI_UTEXT_u_OPT_USERLISTORUALL_QRSTAT);
      } else {
         PRINTITD(MSG_GDI_USAGE_u_OPT_USERLISTORUALL, 
         MSG_GDI_UTEXT_u_OPT_USERLISTORUALL);
         PRINTITD("", MSG_GDI_UTEXT_ATTACH__u_OPT_USERLISTORUALL );
         
      }
      MARK(OA_USER_LIST);
   }
 
   if (VALID_OPT(us_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_us_OPT, MSG_GDI_UTEXT_us_OPT );
   }

   if (VALID_OPT(v_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_v_OPT_VARIABLE_LIST, 
         MSG_GDI_UTEXT_v_OPT_VARIABLE_LIST);
      MARK(OA_VARIABLE_LIST);
   }

   if (VALID_OPT(verbose_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_verbose_OPT, MSG_GDI_UTEXT_verbose_OPT );
   }

   if (VALID_OPT(verify_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_verify_OPT, MSG_GDI_UTEXT_verify_OPT );
   }

   if (VALID_OPT(V_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_V_OPT, MSG_GDI_UTEXT_V_OPT );
   }

   if (VALID_OPT(w_OPT, prog_number)) {
      if ( prog_number == QRSUB ) {
        PRINTITD(MSG_GDI_USAGE_w_OPT_EV, MSG_GDI_UTEXT_w_OPT_EV);
      } else {
        PRINTITD(MSG_GDI_USAGE_w_OPT_EWNVP, MSG_GDI_UTEXT_w_OPT_EWNVP);
      }
   }

   if (VALID_OPT(wd_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_wd_OPT, MSG_GDI_UTEXT_wd_OPT);
      MARK(OA_PATH);
   }

   if (VALID_OPT(xml_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_xml_OPT , MSG_GDI_UTEXT_xml_OPT );
   }

   if (VALID_OPT(explain_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_explain_OPT , MSG_GDI_UTEXT_explain_OPT );
   }

   if (VALID_OPT(AT_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_AT_OPT_FILE, MSG_GDI_UTEXT_AT_OPT_FILE );
   }

   if (VALID_OPT(JQ_DEST_OPR, prog_number)) {
      PRINTIT(MSG_GDI_USAGE_JQ_DEST_OPR );
      MARK(OA_JOB_QUEUE_DEST);
   }

   if (VALID_OPT(JOB_ID_OPR, prog_number)) {
      PRINTIT(MSG_GDI_USAGE_JOB_ID_OPR);
      MARK(OA_JOB_ID_LIST);
   }

   if (VALID_OPT(JOB_TASK_OPR, prog_number)) {
      if (prog_number == QDEL) {
         PRINTITD(MSG_GDI_USAGE_TASK_OPR , MSG_GDI_UTEXT_TASK_OPR);
      } else {
         PRINTITD(MSG_GDI_USAGE_TASK_OPR , MSG_GDI_UTEXT_JOB_ID_OPR);
      }
      MARK(OA_JOB_TASK_LIST);
      MARK(OA_JOB_TASKS);
      MARK(OA_TASK_ID_RANGE);
   }

   if (VALID_OPT(SCRIPT_OPR, prog_number)) {
      PRINTIT(MSG_GDI_USAGE_SCRIPT_OPR);
   }
#if 0				/* unimplemented Univa extension */
   if (VALID_OPT(suspend_remote_OPT, prog_number)) {
      PRINTITD(MSG_GDI_USAGE_suspend_remote_OPT, MSG_GDI_UTEXT_suspend_remote_OPT );
   }
#endif
   fprintf(fp, "\n");
   print_marked(prog_number, fp);

   if (prog_number == QSTAT) {
      fprintf(fp, "\n\n");
      fprintf(fp, "%s %s [options]\n",MSG_GDI_USAGE_USAGESTRING , prog_name);
                    
   }

   fflush(fp);

   DRETURN_VOID;
}

static void usage_silent(
FILE *fp 
) {
   DENTER(TOP_LAYER, "usage_silent");
   print_option_syntax(fp, "[-s]", MSG_GDI_USAGE_SILENT);
   DRETURN_VOID;
}
