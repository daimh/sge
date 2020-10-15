#ifndef __QSTAT_PRINTING_H
#define __QSTAT_PRINTING_H
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

#define QSTAT_INDENT    "       "
#define QSTAT_INDENT2   "                         "

/*
** used in qstat for specifying different display modes
** the display modes are stored in the job structures full_listing field
*/
#define QSTAT_DISPLAY_FULL          (1<<0)
#define QSTAT_DISPLAY_EXTENDED      (1<<1)
#define QSTAT_DISPLAY_RESOURCES     (1<<2)
#define QSTAT_DISPLAY_QRESOURCES    (1<<3)
#define QSTAT_DISPLAY_TASKS         (1<<4)
#define QSTAT_DISPLAY_NOEMPTYQ      (1<<5)
#define QSTAT_DISPLAY_PENDING       (1<<6)
#define QSTAT_DISPLAY_SUSPENDED     (1<<7)
#define QSTAT_DISPLAY_RUNNING       (1<<8)
#define QSTAT_DISPLAY_FINISHED      (1<<9)
#define QSTAT_DISPLAY_ZOMBIES       (1<<10)
#define QSTAT_DISPLAY_ALARMREASON   (1<<11)
#define QSTAT_DISPLAY_USERHOLD      (1<<12)
#define QSTAT_DISPLAY_SYSTEMHOLD    (1<<13)
#define QSTAT_DISPLAY_OPERATORHOLD  (1<<14)
#define QSTAT_DISPLAY_JOBARRAYHOLD  (1<<15)
#define QSTAT_DISPLAY_JOBHOLD       (1<<16)
#define QSTAT_DISPLAY_STARTTIMEHOLD (1<<17)
#define QSTAT_DISPLAY_URGENCY       (1<<18)
#define QSTAT_DISPLAY_PRIORITY      (1<<19)
#define QSTAT_DISPLAY_PEND_REMAIN   (1<<20)
#define QSTAT_DISPLAY_BINDING       (1<<21)

#define QSTAT_DISPLAY_HOLD (QSTAT_DISPLAY_USERHOLD|QSTAT_DISPLAY_SYSTEMHOLD|QSTAT_DISPLAY_OPERATORHOLD|QSTAT_DISPLAY_JOBARRAYHOLD|QSTAT_DISPLAY_JOBHOLD|QSTAT_DISPLAY_STARTTIMEHOLD)
#define QSTAT_DISPLAY_ALL (QSTAT_DISPLAY_PENDING|QSTAT_DISPLAY_SUSPENDED|QSTAT_DISPLAY_RUNNING|QSTAT_DISPLAY_FINISHED)

#define TAG_DEFAULT       0x00
#define TAG_SHOW_IT       0x01
#define TAG_FOUND_IT      0x02
#define TAG_SELECT_IT     0x04


void sge_printf_header(u_long32 full_listing, u_long32 sge_ext);

/*
** for qhost_report_handler_t
*/
#include "sge_qhost.h"

int 
sge_print_jobs_queue(lListElem *qep, lList *job_list, const lList *pe_list, 
                     lList *user_list, lList *ehl, lList *cl, 
                     int print_jobs_of_queue, u_long32 full_listing, 
                     char *indent, u_long32 group_opt, int queue_name_length,
                     qhost_report_handler_t *report_handler, lList **alpp);

#endif /* __QSTAT_PRINTING_H */
