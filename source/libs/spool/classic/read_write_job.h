#ifndef _READ_WRITE_JOB_H_
#define _READ_WRITE_JOB_H_
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

#include "uti/sge_spool.h"
#include "uti/sge_htable.h"

int job_write_spool_file(lListElem *jep, u_long32 ja_taskid, 
                         const char *pe_task_id,
                         sge_spool_flags_t flags);

int job_remove_spool_file(u_long32 job_id, u_long32 ja_taskid, 
                          const char *pe_task_id,
                          sge_spool_flags_t flags);

int job_list_read_from_disk(lList **job_list, char *list_name, int check,
                            sge_spool_flags_t flags,
                            int (*init_function)(lListElem*)); 

int job_write_common_part(lListElem *job, u_long32 ja_task_id,
                          sge_spool_flags_t flags);

#endif /* _READ_WRITE_JOB_H_ */
