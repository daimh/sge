#ifndef PARSE_QSUB_H
#define PARSE_QSUB_H
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

#include "sgeobj/sge_boundaries.h"
#include "cull/cull.h"

/*
 * defines for SPA_argtype
 * argtypes are lFloatT - lListT plus others
 */
#define NUM_SPA_ARGTYPE_DATETIME    (lListT + 100 + 1)
#define NUM_SPA_ARGTYPE_SIMPLELIST  (lListT + 100 + 2)  /* a,b,c */
#define NUM_SPA_ARGTYPE_DEFLIST     (lListT + 100 + 3)  /* a=x,b=y ... */

/*
 * defines for SPA_occurrence
 */
#define BIT_SPA_OCC_NONE               0x00000000L
#define BIT_SPA_OCC_NOARG              0x00000001L
#define BIT_SPA_OCC_ARG                0x00000002L

/*
** defines for pseudo-arguments
*/
#define STR_PSEUDO_JOBID       "jobid"
#define STR_PSEUDO_SCRIPT      "script"
#define STR_PSEUDO_JOBARG      "jobarg"
#define STR_PSEUDO_SCRIPTLEN   "scriptlen"
#define STR_PSEUDO_SCRIPTPTR   "scriptptr"

/*
** flags
*/
#define FLG_USE_PSEUDOS 1
#define FLG_QALTER      2

/* I've added a -wd option to cull_parse_job_parameter() to deal with the
 * DRMAA_WD attribute.  It makes sense to me that since -wd exists and is
 * handled by cull_parse_job_parameter() that -cwd should just become an alias
 * for -wd.  Code to do that is ifdef'ed out below just in case we decide
 * it's a good idea. */
#if 0
/*
** marker to indicate that a -wd was originally a -cwd
*/
#define SGE_HOME_DIRECTORY "$$HOME$$"
#endif

lList *cull_parse_cmdline(u_long32 prog_number, char **arg_list, char **envp, lList **pcmdline, u_long32 flags);

char *reroot_path(lListElem* pjob, const char *path, lList **alpp);

#endif /* PARSE_QSUBL_H */













