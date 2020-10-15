#ifndef __SETRLIMITS_H
#define __SETRLIMITS_H
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
#define RES_PROC     1
#define RES_JOB      2
#define RES_BOTH     (RES_PROC|RES_JOB)

#define LIMIT_VMEM_MIN (10l*1024l*1024l)
#define LIMIT_STACK_MIN (1l*1024l*1024l)
#define LIMIT_CPU_MIN (2l)
#define LIMIT_FSIZE_MIN (15l*1024l)       
#define LIMIT_DESCR_MIN (100)
#define LIMIT_DESCR_MAX (65535)
#define LIMIT_PROC_MIN  (20)
#define LIMIT_MEMLOCK_MIN (4*1024)
#define LIMIT_LOCKS_MIN (2)

struct resource_table_entry {
   int resource;
   char *resource_name;
   int resource_type[2];
};

void setrlimits(int trace_limits);
#endif /* __SETRLIMITS_H */
