#ifndef __SGE_QHOST_H
#define __SGE_QHOST_H
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

#ifdef  __cplusplus
extern "C" {
#endif

#define QHOST_DISPLAY_QUEUES     (1<<0)
#define QHOST_DISPLAY_JOBS       (1<<1)
#define QHOST_DISPLAY_RESOURCES  (1<<2)
#define QHOST_DISPLAY_BINDING    (1<<3)

typedef struct qhost_report_handler_str qhost_report_handler_t;

#define QHOST_SUCCESS 0
#define QHOST_ERROR   -1

struct qhost_report_handler_str {

   void* ctx;
   
   int (*report_started)(qhost_report_handler_t* handler, lList **alpp);
   int (*report_finished)(qhost_report_handler_t* handler, lList **alpp);

   int (*report_host_begin)(qhost_report_handler_t* handler, const char* host_name, lList **alpp);
   int (*report_host_string_value)(qhost_report_handler_t* handler, const char *name, const char *value, lList **alpp);
   int (*report_host_ulong_value)(qhost_report_handler_t* handler, const char* name, u_long32 value, lList **alpp);
   int (*report_host_finished)(qhost_report_handler_t* handler, const char* host_name, lList **alpp);
   
   int (*report_resource_value)(qhost_report_handler_t* handler, const char* dominance, const char* name, const char* value, lList **alpp);
   
   int (*report_queue_begin)(qhost_report_handler_t* handler, const char* qname, lList **alpp);
   int (*report_queue_string_value)(qhost_report_handler_t* handler, const char* qname, const char* name, const char *value, lList **alpp);
   int (*report_queue_ulong_value)(qhost_report_handler_t* handler, const char* qname, const char* name, u_long32 value, lList **alpp);
   int (*report_queue_finished)(qhost_report_handler_t* handler, const char* qname, lList **alpp);
   
   int (*report_job_begin)(qhost_report_handler_t* handler, const char *qname, const char* jname, lList **alpp);
   int (*report_job_string_value)(qhost_report_handler_t* handler, const char *qname, const char* jname, const char* name, const char *value, lList **alpp);
   int (*report_job_ulong_value)(qhost_report_handler_t* handler, const char *qname, const char* jname, const char* name, u_long32 value, lList **alpp);
   int (*report_job_double_value)(qhost_report_handler_t* handler, const char *qname, const char* jname, const char* name, double value, lList **alpp);
   int (*report_job_finished)(qhost_report_handler_t* handler, const char *qname, const char* jname, lList **alpp);

   int (*destroy)(qhost_report_handler_t** handler, lList **alpp);
};

int do_qhost(void *ctx, lList *host_list, lList *user_list, lList *resource_match_list, 
              lList *resource_list, u_long32 show, lList **alp, qhost_report_handler_t* report_handler);

#ifdef  __cplusplus
}
#endif

#endif


