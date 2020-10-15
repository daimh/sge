#ifndef __SGE_HELPER_H
#define __SGE_HELPER_H

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

#include "cull/cull.h"
#include "sgeobj/sge_boundaries.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*
** list for sorted output of job sums for QACCT
*/
enum {
   QAJ_host = QAJ_LOWERBOUND,
   QAJ_queue,                /* qname */
   QAJ_group,
   QAJ_owner,
   QAJ_project,              /* project */
   QAJ_department,           /* department */
   QAJ_ru_wallclock,
   QAJ_ru_utime,
   QAJ_ru_stime,
   QAJ_ru_maxrss,
   QAJ_ru_inblock,
   QAJ_granted_pe,
   QAJ_slots,
   QAJ_cpu,
   QAJ_mem,
   QAJ_io,
   QAJ_iow,
   QAJ_maxvmem,
   QAJ_arid,
   QAJ_account
};

LISTDEF(QAJ_Type)
   SGE_HOST(QAJ_host, CULL_DEFAULT)
   SGE_STRING(QAJ_queue, CULL_DEFAULT)
   SGE_STRING(QAJ_group, CULL_DEFAULT)
   SGE_STRING(QAJ_owner, CULL_DEFAULT)
   SGE_STRING(QAJ_project, CULL_DEFAULT)
   SGE_STRING(QAJ_department, CULL_DEFAULT)
   SGE_DOUBLE(QAJ_ru_wallclock, CULL_DEFAULT)
   SGE_DOUBLE(QAJ_ru_utime, CULL_DEFAULT)
   SGE_DOUBLE(QAJ_ru_stime, CULL_DEFAULT)
   SGE_DOUBLE(QAJ_ru_maxrss, CULL_DEFAULT)
   SGE_DOUBLE(QAJ_ru_inblock, CULL_DEFAULT)
   SGE_STRING(QAJ_granted_pe, CULL_DEFAULT)
   SGE_ULONG(QAJ_slots, CULL_DEFAULT)
   SGE_DOUBLE(QAJ_cpu, CULL_DEFAULT)
   SGE_DOUBLE(QAJ_mem, CULL_DEFAULT)
   SGE_DOUBLE(QAJ_io, CULL_DEFAULT)
   SGE_DOUBLE(QAJ_iow, CULL_DEFAULT)
   SGE_DOUBLE(QAJ_maxvmem, CULL_DEFAULT)
   SGE_ULONG(QAJ_arid, CULL_DEFAULT)
   SGE_STRING(QAJ_account, CULL_DEFAULT)
LISTEND


NAMEDEF(QAJN)
   NAME("QAJ_host")
   NAME("QAJ_queue")
   NAME("QAJ_group")
   NAME("QAJ_owner")
   NAME("QAJ_project")
   NAME("QAJ_department")
   NAME("QAJ_ru_wallclock")
   NAME("QAJ_ru_utime")
   NAME("QAJ_ru_stime")
   NAME("QAJ_ru_maxrss")
   NAME("QAJ_ru_inblock")
   NAME("QAJ_granted_pe")
   NAME("QAJ_slots")
   NAME("QAJ_cpu")
   NAME("QAJ_mem")
   NAME("QAJ_io")
   NAME("QAJ_iow")
   NAME("QAJ_maxvmem")
   NAME("QAJ_arid")
   NAME("QAJ_account")
NAMEEND

/* *INDENT-ON* */

#define QAJS sizeof(QAJN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif

#endif /* __SGE_HELPER_H */

