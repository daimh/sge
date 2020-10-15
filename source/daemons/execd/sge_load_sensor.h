#ifndef __SGE_LOAD_SENSOR_H
#define __SGE_LOAD_SENSOR_H
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

#include <sys/time.h>

#define IDLE_LOADSENSOR_NAME "qidle"
#define QLOADSENSOR_NAME "qloadsensor"

enum { 
   LS_OK = 0,
   LS_ALREADY_STARTED,            /* load sensor is already started     */
   LS_CANT_PEOPEN,                /* failure in starting LS with peopen */
   LS_NOT_STARTED,                /* load sensor not started            */
   LS_BROKEN_PIPE                 /* ls has exited or select <= 0       */
};

int sge_ls_get(const char* qualified_hostname, const char *binary_path, lList **lpp);

void sge_ls_stop(int);

void trigger_ls_restart(void);

void sge_ls_qidle(int);

void sge_ls_qls(int);

void set_ls_fds(fd_set *fds);

int sge_ls_stop_if_pid(pid_t pid);

#endif /* __SGE_LOAD_SENSOR_H */

