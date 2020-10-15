#ifndef __MSG_SGE_H
#define __MSG_SGE_H
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

#include "basis_types.h"

#define MSG_SGE_USAGE         _MESSAGE(31025, _("usage: pdc [-snpgj] [-kK signo] [-iJPS secs] job_id [ ... ]\n\njob_id is the supplementary gid on most systems"))
#define MSG_SGE_s_OPT_USAGE   _MESSAGE(31026, _("show system data"  ))
#define MSG_SGE_n_OPT_USAGE   _MESSAGE(31027, _("no output"  ))
#define MSG_SGE_p_OPT_USAGE   _MESSAGE(31028, _("show process information"  ))
#define MSG_SGE_i_OPT_USAGE   _MESSAGE(31029, _("interval in seconds (default is 2)"  ))
#define MSG_SGE_g_OPT_USAGE   _MESSAGE(31030, _("produce output for gr_osview(1)"  ))
#define MSG_SGE_j_OPT_USAGE   _MESSAGE(31031, _("provide job name for gr_osview display (used only with -g)"  ))
#define MSG_SGE_J_OPT_USAGE   _MESSAGE(31032, _("job data collection interval in seconds (default is 0)"    ))
#define MSG_SGE_k_OPT_USAGE   _MESSAGE(31033, _("kill job using signal signo"    ))
#define MSG_SGE_K_OPT_USAGE   _MESSAGE(31034, _("kill job using signal signo (loop until all processes are dead)"    ))
#define MSG_SGE_P_OPT_USAGE   _MESSAGE(31035, _("process data collection interval in seconds (default is 0)"    ))
#define MSG_SGE_S_OPT_USAGE   _MESSAGE(31036, _("system data collection interval in seconds (default is 15)"    ))
#define MSG_SGE_JOBDATA       _MESSAGE(31037, _("******** Job Data **********"))
#define MSG_SGE_SYSTEMDATA    _MESSAGE(31038, _("******** System Data ***********"))
#define MSG_SGE_STATUS        _MESSAGE(31039, _("********** Status **************"))
#define MSG_SGE_CPUUSAGE      _MESSAGE(31040, _("CPU Usage:"))
#define MSG_SGE_SGEJOBUSAGECOMPARSION     _MESSAGE(31041, _("Job Usage Comparison"))
#define MSG_SGE_XISNOTAVALIDSIGNALNUMBER_S   _MESSAGE(31042, _(SFN" is not a valid signal number"))
#define MSG_SGE_XISNOTAVALIDINTERVAL_S       _MESSAGE(31043, _(SFN" is not a valid interval"))
#define MSG_SGE_XISNOTAVALIDJOBID_S          _MESSAGE(31044, _(SFN" is not a valid job_id"))
#define MSG_SGE_GROSVIEWEXPORTFILE           _MESSAGE(31045, _("gr_osview export file"))
#define MSG_SGE_PERMISSIONDENIED             _MESSAGE(31046, _("permission denied"))
#define MSG_SGE_NOJOBS                       _MESSAGE(31047, _("No jobs"))


/* 
** sge/procfs.c
*/ 
#define MSG_SGE_NGROUPS_MAXOSRECONFIGURATIONNECESSARY    _MESSAGE(31048, _("NGROUPS_MAX <= 0: os reconfiguration necessary"))
#define MSG_SGE_KILLINGPIDXY_UI                          _MESSAGE(31050, _("killing pid "sge_U32CFormat"/%d" ))
#define MSG_SGE_DONOTKILLROOTPROCESSXY_UI                _MESSAGE(31051, _("do not kill root process "sge_U32CFormat"/%d"   ))
/* #define MSG_SGE_PIOCPSINFOFAILED                         _message(31053, _("PIOCPSINFO failed")) __TS Removed automatically from testsuite!! TS__*/




#endif /* __MSG_SGE_H */
