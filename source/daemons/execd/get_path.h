#ifndef __GET_PATH_H
#define __GET_PATH_H
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

#include "uti/sge_dstring.h"

#include "cull/cull.h"

#include "basis_types.h"

#define SGE_STDIN           0x00100000
#define SGE_STDOUT          0x00200000
#define SGE_STDERR          0x00400000
#define SGE_SHELL           0x04000000
#define SGE_PAR_STDOUT      0x20000000
#define SGE_PAR_STDERR      0x40000000

int sge_get_path(const char * qualified_hostname, lList *lp, const char *cwd, const char *owner, 
                 const char *job_name, u_long32 job_number, 
                 u_long32 task_number, int type, char *path, size_t path_len);
                 
bool sge_get_fs_path(lList* lp, char* fs_host, size_t fs_host_len,
                                char* fs_path, size_t fs_path_len);

const char *sge_make_ja_task_active_dir(const lListElem *job, const lListElem *ja_task, dstring *err_str);
const char *sge_make_pe_task_active_dir(const lListElem *job, const lListElem *ja_task, const lListElem *pe_task, dstring *err_str);

const char* expand_path(const char *path_in, u_long32 job_id, u_long32 ja_task_id, const char *job_name, const char *user, const char *fqhost);

#endif /* __GET_PATH_H */

