#ifndef __SGE_CATEGORY_H
#define __SGE_CATEGORY_H
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

/*
** ------------ to be called from within the data model layer 
*/

int sge_add_job_category(lListElem *job, lList *acl_list, const lList *prj_list, const lList *lirs_list);
int sge_delete_job_category(lListElem *job);
void set_rebuild_categories(bool new_value); 
int sge_rebuild_job_category(lList *job_list, lList *acl_list, const lList *prj_list, const lList *lirs_list);
lList *sge_category_job_copy(lList *queue_list, lList **orders, bool monitor_next_run);

/*
** ------------ to be called from within the decision-making layer 
*/

void sge_reject_category(lRef cat, bool with_reservation);
bool sge_is_job_category_rejected_(lRef cat);
bool sge_is_job_category_reservation_rejected_(lRef cat);
int sge_is_job_category_rejected(lListElem *job);
int sge_is_job_category_reservation_rejected(lListElem *job);
int sge_reset_job_category(void);
bool sge_is_job_category_message_added(lRef cat);
void sge_set_job_category_message_added( lRef cat );

void sge_print_categories(void);
int sge_category_count(void);
int sge_cs_category_count(void); 

#endif /* __SGE_CATEGORY_H */

