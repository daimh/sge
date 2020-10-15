#ifndef __PROCFS_H
#define __PROCFS_H
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
int pt_dispatch_proc_to_job(char *pidname, lnk_link_t *job_list, int time_stamp, time_t last_time);
void init_procfs(void);
#if defined(LINUX) || defined(ALPHA) || defined(SOLARIS)

#include "err_trace.h"

int pt_open(void);
void pt_close(void);
void procfs_kill_addgrpid(gid_t add_grp_id, int sig,
   tShepherd_trace shepherd_trace);
void pt_dispatch_procs_to_jobs(lnk_link_t *job_list, int time_stamp, time_t last_time);
#endif  /* LINUX || ALPHA || SOLARIS */
#endif /* __PROCFS_H */
