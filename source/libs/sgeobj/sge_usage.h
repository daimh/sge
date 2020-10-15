#ifndef __SGE_USAGE_H 
#define __SGE_USAGE_H 
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

#include "sgeobj/sge_usage_UA_L.h"

/*
 * sge standard usage value names
 * 
 * use these defined names for refering special usage values
 */

#define USAGE_ATTR_CPU "cpu"

/* integral memory usage */
#define USAGE_ATTR_MEM "mem"
#define USAGE_ATTR_IO "io"
#define USAGE_ATTR_IOW "iow"

#define USAGE_ATTR_CPU_ACCT "acct_cpu"

/* these are used for accounting */
#define USAGE_ATTR_MEM_ACCT "acct_mem"
#define USAGE_ATTR_IO_ACCT "acct_io"
#define USAGE_ATTR_IOW_ACCT "acct_iow"
#define USAGE_ATTR_MAXVMEM_ACCT "acct_maxvmem"

/* current amount of used memory */
#define USAGE_ATTR_VMEM "vmem"

/* max. vmem */
#define USAGE_ATTR_MAXVMEM "maxvmem"

#define USAGE_ATTR_FINISHED_JOBS "finished_jobs"

u_long64
usage_list_get_ulong_usage(const lList *usage_list, const char *name, u_long64 def);
double
usage_list_get_double_usage(const lList *usage_list, const char *name, double def);

void
usage_list_set_ulong_usage(lList *usage_list, const char *name, u_long64 value);
void
usage_list_set_double_usage(lList *usage_list, const char *name, double value);

void
usage_list_sum(lList *usage_list, const lList *add_usage_list);
void
usage_list_sub(lList *usage_list, const lList *sub_usage_list);

lList *scale_usage(lList *scaling, lList *prev_usage, lList *scaled_usage);
 
#endif /* __SGE_USAGE_H */
