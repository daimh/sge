#ifndef __USAGE_H
#define __USAGE_H
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



/* 
   for use in mark_argument_syntax() calls:

   these values correspond to an array of 
   argument syntax description texts in usage.c

   note: both are sorted alphabetically (array and enum)

*/
enum {
   OA_ACCOUNT_STRING,
   OA_COMPLEX_LIST,
   OA_CONTEXT_LIST,
   OA_CKPT_SEL,
   OA_DATE_TIME,
   OA_DESTIN_ID_LIST,
   OA_HOLD_LIST,
   OA_HOST_ID_LIST,
   OA_HOSTNAME_LIST,
   OA_JOB_ID_LIST,
   OA_JOB_IDENTIFIER_LIST,
   OA_JOB_QUEUE_DEST,
   OA_JSV_URL,
   OA_LISTNAME_LIST,
   OA_RQS_LIST,
   OA_MAIL_ADDRESS,
   OA_MAIL_LIST,
   OA_MAIL_OPTIONS,
   OA_MAIL_OPTIONS_AR,
   OA_NODE_LIST,
   OA_NODE_PATH,
   OA_NODE_SHARES_LIST,
   OA_PATH,  
   OA_PATH_LIST,  
   OA_FILE_LIST,      /* unused -- apparently the same as path_list */
   OA_PRIORITY,
   OA_RESOURCE_LIST,
   OA_SERVER,
   OA_SERVER_LIST,
   OA_SIGNAL,
   OA_SIMPLE_CONTEXT_LIST,
   OA_SLOT_RANGE,
   OA_STATES,
   OA_JOB_TASK_LIST,
   OA_JOB_TASKS,
   OA_TASK_ID_RANGE,
   OA_USER_LIST,
   OA_VARIABLE_LIST,
   OA_OBJECT_NAME,
   OA_ATTRIBUTE_NAME,
   OA_OBJECT_ID_LIST,
   OA_PROJECT_LIST,
   OA_EVENTCLIENT_LIST,
   OA_HOST_LIST,
   OA_WC_CQUEUE,
   OA_WC_HOST,
   OA_WC_HOSTGROUP,
   OA_WC_QINSTANCE,
   OA_WC_QDOMAIN,
   OA_WC_QUEUE,
   OA_WC_QUEUE_LIST,
   OA_OBJECT_NAME2,
   OA_OBJECT_NAME3,
   OA_TIME,
   OA_AR_ID,
   OA_AR_ID_LIST,
   OA_WC_AR_LIST,
   OA_WC_AR,
   OA_THREAD_NAME,
   OA_TASK_CONCURRENCY,
   OA_BINDING_EXPLICIT,
   OA_BINDING_LINEAR,
   OA_BINDING_STRIDING,

   OA__END
};

void mark_argument_syntax(int argument_number);
void sge_usage(u_long32 prog_number, FILE *fp);

#endif /* __USAGE_H */
