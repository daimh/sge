#ifndef __MSG_QSH_H
#define __MSG_QSH_H
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

#define MSG_QSH_WAITINGFORINTERACTIVEJOBTOBESCHEDULED    _MESSAGE(17000, _("waiting for interactive job to be scheduled ..."))
#define MSG_QSH_REQUESTFORINTERACTIVEJOBHASBEENCANCELED    _MESSAGE(17001, _("\nRequest for interactive job has been canceled."))
#define MSG_QSH_REQUESTCANTBESCHEDULEDTRYLATER_S    _MESSAGE(17002, _("Your "SFQ" request could not be scheduled, try again later."))
#define MSG_QSH_INTERACTIVEJOBHASBEENSCHEDULED_S    _MESSAGE(17003, _("Your interactive job "SFN" has been successfully scheduled."))
#define MSG_QSH_ESTABLISHINGREMOTESESSIONTO_SS    _MESSAGE(17004, _("Establishing "SFN" session to host "SFN" ..."))
#define MSG_CONFIG_CANTGETCONFIGURATIONFROMQMASTER    _MESSAGE(17005, _("\nCannot get configuration from qmaster."))
/* #define MSG_JOB_CANTGETHOSTOFJOB    _message(17006, _("Cannot get host of job.\n")) __TS Removed automatically from testsuite!! TS__*/
#define MSG_EXEC_CANTEXECXYZ_SS    _MESSAGE(17007, _("Could not exec "SFN": "SFN))
#define MSG_QSH_CANTSTARTINTERACTIVEJOB    _MESSAGE(17008, _("\nCould not start interactive job."))
#define MSG_QSH_ERROROPENINGSTREAMSOCKET_S   _MESSAGE(17009, _("error opening stream socket: "SFN))
#define MSG_QSH_ERRORBINDINGSTREAMSOCKET_S   _MESSAGE(17010, _("error binding stream socket: "SFN))
#define MSG_QSH_ERRORGETTINGSOCKETNAME_S     _MESSAGE(17011, _("error getting socket name: "SFN))
#define MSG_QSH_ERRORLISTENINGONSOCKETCONNECTION_S  _MESSAGE(17012, _("error listening on socket connection: "SFN))
#define MSG_QSH_ERRORWAITINGONSOCKETFORCLIENTTOCONNECT_S _MESSAGE(17013, _("error waiting on socket for client to connect: "SFN))
#define MSG_QSH_ERRORINACCEPTONSOCKET_S  _MESSAGE(17014, _("error in accept on socket: "SFN))
#define MSG_QSH_ERRORREADINGSTREAMMESSAGE_S _MESSAGE(17015, _("error reading stream message: "SFN))
#define MSG_QSH_ERRORENDINGCONNECTION _MESSAGE(17016, _("error: ending connection before all data received"))
#define MSG_QSH_ERRORREADINGRETURNCODEOFREMOTECOMMAND _MESSAGE(17017, _("error reading returncode of remote command"))
#define MSG_QSH_MALLOCFAILED _MESSAGE(17018, _("malloc failed!"))
#define MSG_QSH_CANNOTFORKPROCESS_S _MESSAGE(17019, _("cannot fork process: "SFN))
#define MSG_QSH_EXITEDWITHCODE_SI _MESSAGE(17020, _(SFN" exited with exit code %d"))
#define MSG_QSH_EXITEDONSIGNAL_SIS _MESSAGE(17021, _(SFN" exited on signal %d ("SFN")"))
#define MSG_QSH_INHERITBUTJOB_IDNOTSET_SSS _MESSAGE(17022, _(SFQ" called with option "SFQ", but "SFQ" not set in environment"))
#define MSG_QSH_INVALIDJOB_ID_SS _MESSAGE(17023, _("invalid "SFQ" "SFN))
#define MSG_QSH_INHERITUSAGE_SS _MESSAGE(17024, _("usage with "SFQ" option: "SFQ))
#define MSG_QSH_EXECUTINGTASKOFJOBFAILED_IS _MESSAGE(17025, _("executing task of job %d failed: "SFN))
#define MSG_QSH_CANNOTGETCONNECTIONTOQLOGIN_STARTER_SS _MESSAGE(17026, _("\ncannot get connection to "SFQ" at host "SFQ))
#define MSG_QSH_ERRORREADINGCONTEXTFROMQLOGIN_STARTER_S _MESSAGE(17027, _("\nerror reading job context from "SFQ))
/* #define MSG_QSH_LINEFEED _message(17028, _("\n")) __TS Removed automatically from testsuite!! TS__*/
#define MSG_QSH_UNKNOWNJOBSTATUS_X _MESSAGE(17029, _("unknown job status " sge_X32CFormat))
#define MSG_QSH_SENDINGTASKTO_S _MESSAGE(17030, _("Starting server daemon at host "SFQ))
#define MSG_QSH_SERVERDAEMONSUCCESSFULLYSTARTEDWITHTASKID_S _MESSAGE(17031, _("Server daemon successfully started with task id "SFQ))
#define MSG_QSH_CLEANINGUPAFTERABNORMALEXITOF_S _MESSAGE(17032, _("cleaning up after abnormal exit of "SFN))
#define MSG_QSH_READINGEXITCODEFROMSHEPHERD  _MESSAGE(17033, _("reading exit code from shepherd ... "))
#define MSG_QSH_CREATINGCOMMLIBSERVER_S _MESSAGE(17034, _("creating a commlib server: "SFQ))
#define MSG_QSH_GOTNOCONNECTIONWITHINSECONDS_IS _MESSAGE(17035, _("got no connection within %d seconds. "SFQ))
#define MSG_QSH_SETTINGCONNECTIONPARAMS_S _MESSAGE(17036, _("setting connection parameters: "SFQ))
#define MSG_QSH_UNMATCHED_C _MESSAGE(17037, _("unmatched quote %c."))
#define MSG_QSH_ERRORRUNNINGIJSSERVER_S _MESSAGE(17038, _("error running IJS server: "SFQ))
#define MSG_QSH_ERRORWHILEWAITINGFORBUILTINIJSCONNECTION_S _MESSAGE(17039, _("error while waiting for builtin IJS connection: "SFQ))

#endif /* __MSG_QSH_H */

