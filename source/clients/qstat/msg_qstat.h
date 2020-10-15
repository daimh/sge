#ifndef __MSG_QSTAT_H
#define __MSG_QSTAT_H
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
 * Portions of this code are Copyright 2011 Univa Inc.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "basis_types.h"

#define MSG_QSTAT_USAGE_VIEWALSOSCHEDULINGATTRIBUTES     _MESSAGE(19010, _("view additional attributes"))
#define MSG_QSTAT_USAGE_EXPLAINOPT                       _MESSAGE(19011, _("show reason for queue c(onfiguration ambiguous), a(larm), suspend A(larm), E(rror) state"))
#define MSG_QSTAT_USAGE_FULLOUTPUT                       _MESSAGE(19012, _("full output"))
#define MSG_QSTAT_USAGE_FULLOUTPUTANDSHOWRESOURCESOFQUEUES    _MESSAGE(19013, _("full output and show (selected) resources of queue(s)"))
#define MSG_QSTAT_USAGE_DISPLAYALLJOBARRAYTASKS    _MESSAGE(19014, _("display all job-array tasks (do not group)"))
#define MSG_QSTAT_USAGE_VIEWALSOBINDINGATTRIBUTES  _MESSAGE(19015, _("suppress additional binding specific parameters"))
#define MSG_QSTAT_USAGE_SHOWSCHEDULERJOBINFO    _MESSAGE(19016, _("show scheduler job information"))
#define MSG_QSTAT_USAGE_REQUESTTHEGIVENRESOURCES    _MESSAGE(19017, _("request the given resources"))
#define MSG_QSTAT_USAGE_HIDEEMPTYQUEUES    _MESSAGE(19018, _("hide empty queues"))
#define MSG_QSTAT_USAGE_SELECTONLYQUEESWITHONOFTHESEPE    _MESSAGE(19019, _("select only queues with one of these parallel environments"))
#define MSG_QSTAT_USAGE_PRINTINFOONGIVENQUEUE    _MESSAGE(19020, _("print information on given queue"))
#define MSG_QSTAT_USAGE_PRINTINFOCQUEUESTATESEL  _MESSAGE(19021, _("selects queues which are in the given state(s)"))
#define MSG_QSTAT_USAGE_SHOWREQUESTEDRESOURCESOFJOB    _MESSAGE(19022, _("show requested resources of job(s)"))
#define MSG_QSTAT_USAGE_SHOWPENDINGRUNNINGSUSPENDESZOMBIEJOBS    _MESSAGE(19023, _("show pending, running, suspended, zombie jobs,"))
#define MSG_QSTAT_USAGE_JOBSWITHAUSEROPERATORSYSTEMHOLD    _MESSAGE(19024, _("jobs with a user/operator/system/array-dependency hold, "))
#define MSG_QSTAT_USAGE_JOBSWITHSTARTTIMEINFUTORE    _MESSAGE(19025, _("jobs with a start time in future or any combination only."))
#define MSG_QSTAT_USAGE_HISABBREVIATIONFORHUHOHSHJHA    _MESSAGE(19026, _("h is an abbreviation for huhohshdhjha"))
#define MSG_QSTAT_USAGE_AISABBREVIATIONFOR              _MESSAGE(19027, _("a is an abbreviation for prsh"))
#define MSG_QSTAT_USAGE_SHOWTASKINFO    _MESSAGE(19028, _("show task information (implicitly -g t)"))
#define MSG_QSTAT_USAGE_VIEWONLYJOBSOFTHISUSER    _MESSAGE(19029, _("view jobs of this user list"))
#define MSG_QSTAT_USAGE_SELECTQUEUESWHEREUSERXHAVEACCESS    _MESSAGE(19030, _("select only queues where these users have access"))
#define MSG_QSTAT_USAGE_ADDITIONALDEBUGGINGOPTIONS    _MESSAGE(19031, _("Additional debugging options:"))
#define MSG_QSTAT_USAGE_DUMPCOMPLETEJOBLISTTOSTDOUT    _MESSAGE(19032, _("dump complete job list to stdout"))
#define MSG_QSTAT_USAGE_DUMPCOMPLETEQUEUELISTTOSTDOUT    _MESSAGE(19033, _("dump complete queue list to stdout"))
#define MSG_SCHEDD_SCHEDULINGINFO    _MESSAGE(19034, _("scheduling info"))
/* #define MSG_QSI_NOVALIDQSIHOSTSPECIFIED    _message(19035, _("no valid QSI host specified")) __TS Removed automatically from testsuite!! TS__*/
/* #define MSG_QSTAT_CANTREACHXCAUSEY_SS    _message(19036, _("can't reach "SFN" cause "SFN)) __TS Removed automatically from testsuite!! TS__*/
#define MSG_QSTAT_FOLLOWINGDONOTEXIST _MESSAGE(19040, _("Following jobs do not exist: "))
#define MSG_QSTAT_USAGE_DISPLAYALLPARALLELJOBTASKS    _MESSAGE(19041, _("display all parallel job tasks (do not group)"))
#define MSG_QSTAT_USAGE_DISPLAYCQUEUESUMMARY          _MESSAGE(19043, _("display cluster queue summary"))
#define MSG_QSTAT_URGENCYINFO                         _MESSAGE(19044, _("display job urgency information"))
#define MSG_QSTAT_PRIORITYINFO                        _MESSAGE(19045, _("display job priority information"))

#endif /* __MSG_QSTAT_H */

