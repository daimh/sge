#ifndef __MSG_QSTAT_XML_H
#define __MSG_QSTAT_XML_H
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

void xml_qstat_show_job_info(lList **list, lList **answer_list, qstat_env_t *qstat_env);

void xml_qstat_show_job(lList **job_list, lList **msg_list, lList **answer_list, lList **id_list, qstat_env_t *qstat_env);

void xml_qstat_jobs(lList *job_list, lList *zombie_list, const lList *pe_list, 
                    const lList *user_list, const lList *exechost_list, 
                    const lList *centry_list, lSortOrder *so, 
                    u_long32 full_listing, u_long32 group_opt, lList **target_list);

void xml_print_jobs_queue(lListElem *qep, lList *job_list, const lList *pe_list, const lList *user_list,
                          const lList *ehl, const lList *centry_list, int print_jobs_of_queue, u_long32 full_listing,
                          u_long32 group_opt, lList **target_list); 

lListElem *xml_print_queue(lListElem *q, const lList *exechost_list, const lList *centry_list,
                    u_long32 full_listing, const lList *qresource_list, u_long32 explain_bits);
                    
int qstat_xml_handler_init(qstat_handler_t* handler, lList **alpp);
int cqueue_summary_xml_handler_init(cqueue_summary_handler_t *handler, lList **alpp);
int qselect_xml_init(qselect_handler_t* handler, lList **alpp);


#endif /* __MSG_QSTAT_H */
