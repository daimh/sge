#ifndef MSG_DAEMONS_COMMON_H
#define MSG_DAEMONS_COMMON_H
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



/*
** global deamon messages
*/
#define MSG_MASTER                         _MESSAGE(27001, _("master"))
#define MSG_STARTUP_STARTINGUP_SSS         _MESSAGE(27002, _("starting up "SFN" "SFN" ("SFN")"))
#define MSG_SHADOWD_CONTROLLEDSHUTDOWN_S   _MESSAGE(27003, _("controlled shutdown "SFN))
#define MSG_SHADOWD_CONTROLLEDSHUTDOWN_SU  _MESSAGE(27000, _("controlled shutdown "SFN" (exit state = "sge_U32CFormat")"))
#define MSG_ERROR_CANTSWITCHTOADMINUSER    _MESSAGE(27004, _("can't switch to admin_user"))

/*
** path_aliases.c
*/
#define MSG_ALIAS_INVALIDSYNTAXOFPATHALIASFILEX_S    _MESSAGE(27005, _("invalid syntax of path alias file "SFQ))
#define MSG_ALIAS_CANTREAD_SS                        _MESSAGE(27006, _("can't read path aliasing file "SFQ": "SFN))


/*
** unparse_job_cull.c
*/
#define MSG_LIST_ERRORFORMATINGJIDPREDECESSORLISTASHOLDJID       _MESSAGE(27007, _("Error formatting jid_request_list as -hold_jid"))
#define MSG_LIST_ERRORFORMATINGJOBIDENTIFIERLISTASJID       _MESSAGE(27008, _("Error formatting job_identifier_list as -jid"))
#define MSG_PARSE_ERRORUNPARSINGMAILOPTIONS       _MESSAGE(27009, _("Error unparsing mail options"))
#define MSG_LIST_ERRORFORMATTINGMAILLISTASM       _MESSAGE(27010, _("Error formatting mail list as -M"))
#define MSG_PROC_INVALIDPROIRITYMUSTBELESSTHAN1025       _MESSAGE(27011, _("Invalid priority; must be less than 1025"))
#define MSG_PROC_INVALIDPRIORITYMUSTBEGREATERTHANMINUS1024       _MESSAGE(27012, _("Invalid priority; must be greater than -1024"))
#define MSG_LIST_ERRORFORMATINGHARDQUEUELISTASQ       _MESSAGE(27013, _("Error formatting hard_queue_list as -q"))
#define MSG_LIST_ERRORFORMATINGSOFTQUEUELISTASQ       _MESSAGE(27014, _("Error formatting soft_queue_list as -q"))
/* #define MSG_LIST_ERRORFORMATINGQSARGSLIST       _message(27015, _("Error formatting qs_args list")) __TS Removed automatically from testsuite!! TS__*/
#define MSG_LIST_ERRORFORMATINGSHELLLIST       _MESSAGE(27016, _("Error formatting shell_list"))
#define MSG_LIST_ERRORFORMATINGENVIRONMENTLISTASV       _MESSAGE(27017, _("Error formatting environment list as -v"))
#define MSG_LIST_ERRORFORMATINGJOBARGUMENTS       _MESSAGE(27018, _("Error formatting job arguments"))
#define MSG_JOB_INVALIDVALUEFORCHECKPOINTATTRIBINJOB_U       _MESSAGE(27019, _("Invalid value for checkpoint attribute in job "sge_U32CFormat))
#define MSG_LIST_ERRORFORMATINGHARDRESOURCELISTASL       _MESSAGE(27020, _("Error formatting hard_resource_list as -l"))
#define MSG_JOB_JOBHASPEWITHNORANGES       _MESSAGE(27021, _("Job has parallel environment with no ranges"))
#define MSG_LIST_ERRORFORMATINGRANGESINPE       _MESSAGE(27022, _("Error formatting ranges in -pe"))
#define MSG_LIST_ERRORFORMATINGPATHLIST       _MESSAGE(27023, _("Error formatting path_list"))
#define MSG_LIST_ERRORFORMATINGJIDPREDECESSORLISTASHOLDJIDAD       _MESSAGE(27024, _("Error formatting jid_request_list as -hold_jid_ad"))


/*
** startprog.c
*/
#define MSG_STARTUP_STARTINGPROGRAMMX_S       _MESSAGE(27026, _("starting program: "SFN))
#define MSG_PROC_CANTFORKPROCESSTOSTARTX_S       _MESSAGE(27027, _("can't fork process to start: "SFN))
#define MSG_PROC_CANTEXECPROCESSORPROCESSDIEDTHROUGHSIGNALX_S       _MESSAGE(27028, _("cannot exec process or process died through signal: "SFN))
#define MSG_PROC_CANTSTARTPROCESSX_S       _MESSAGE(27029, _("couldn't start process: "SFN))
#define MSG_PROC_WAITPIDRETURNEDUNKNOWNSTATUS       _MESSAGE(27030, _("waitpid() returned unknown status"))
#define MSG_SMF_STARTPROG_FORK_FAILED_S    _MESSAGE(27031, _("sge_smf_contract_fork() for startprog failed: "SFQ))


/*
 * setup_qmaster.c
 */
#define MSG_QMASTER_FOUNDRUNNINGQMASTERONHOSTXNOTSTARTING_S    _MESSAGE(27038, _("found running qmaster on host "SFQ"- not starting"))


/*
** mail.c
*/
#define MSG_SMF_MAIL_FORK_FAILED_S     _MESSAGE(27101, _("sge_smf_contract_fork() for mail failed: "SFQ))
#define MSG_MAIL_EMPTYUSERHOST         _MESSAGE(27042, _("sending mail is not possible since user and host are empty"))
#define MSG_MAIL_MAILUSER_SSSS         _MESSAGE(27043, _("sending "SFN" mail to user "SFQ"|mailer "SFQ"|"SFQ))
#define MSG_MAIL_MAILUSERHOST_SSSSS    _MESSAGE(27044, _("sending "SFN" mail to user \""SFN"@"SFN"\"|mailer "SFQ"|"SFQ))
#define MSG_MAIL_NOPIPE_S              _MESSAGE(27045, _("pipe() for mail failed: "SFQ))
#define MSG_MAIL_NOFORK_S              _MESSAGE(27046, _("fork() for mail failed: "SFQ))
#define MSG_MAIL_NODUP_S               _MESSAGE(27047, _("dup() failed: "SFQ))
#define MSG_MAIL_NOEXEC_SS             _MESSAGE(27048, _("exec of mailer "SFQ" failed: "SFQ))
#define MSG_MAIL_NOMAIL1               _MESSAGE(27049, _("wait for mailer returned 0 - killing") ) 
#define MSG_MAIL_NOMAIL2               _MESSAGE(27050, _("mailer had timeout - killing"))
#define MSG_MAIL_NOMAIL3_I             _MESSAGE(27051, _("mailer was stopped due to signal %d - killing"))
#define MSG_MAIL_NOSUBJ                _MESSAGE(27052, _("<no subject>"))


/*
** admin_mail.c 
*/
#define MSG_MAIL_PARSE_S           _MESSAGE(27053, _("Error parsing mail list "SFQ))

/* CR: don't localize mail subject, until we send it in Mime format!
 *  The message definition is not l10n'ed (no _() macro used)!!!     
 */
#define MSG_MAIL_SUBJECT_SUU       SFN": Job-array task "sge_U32CFormat"."sge_U32CFormat" failed"
#define MSG_MAIL_SUBJECT_SU        SFN": Job " sge_U32CFormat " failed"


#define MSG_MAIL_BODY_USSSSSSS     "Job " sge_U32CFormat " caused action: "SFN"\n User        = "SFN"\n Queue       = "SFN"\n Start Time  = "SFN"\n End Time    = "SFN"\nfailed "SFN": "SFN2
#define MSG_GFSTATE_PEJOB_U        _MESSAGE(27054, _("PE Job "sge_U32CFormat" will be deleted"))
#define MSG_GFSTATE_QUEUE_S        _MESSAGE(27055, _("Queue "SFQ" set to ERROR"))
#define MSG_GFSTATE_HOST_S         _MESSAGE(27056, _("All Queues on host "SFQ" set to ERROR"))
#define MSG_GFSTATE_JOB_UU         _MESSAGE(27057, _("Job-array task "sge_U32CFormat"."sge_U32CFormat" set to ERROR"))
#define MSG_GFSTATE_JOB_U          _MESSAGE(27058, _("Job "sge_U32CFormat" set to ERROR"))

/*
** config_file.c
*/
#define MSG_CONF_NOCONFVALUE_S        _MESSAGE(27059, _("can't get configuration value for "SFQ))
#define MSG_CONF_ATLEASTONECHAR       _MESSAGE(27060, _("variables need at least one character"))
#define MSG_CONF_REFVAR_S             _MESSAGE(27061, _("referenced variable %20.20s... exceeds maximum length (%d)"))
#define MSG_CONF_UNKNOWNVAR_S         _MESSAGE(27062, _("unknown variable "SFQ))

/* 
 * sge_category.c
 */ 

/*
 * Additional messages
 */
/* CR: don't localize mail subject, until we send it in Mime format!
 *  The message definition is not l10n'ed (no _() macro used)!!!     
 */
#define MSG_MAIL_SUBJECT_JA_TASK_SUSP_UUS       "Job-array task "sge_U32CFormat"."sge_U32CFormat" ("SFN") Suspended"
#define MSG_MAIL_SUBJECT_JOB_SUSP_US            "Job "sge_U32CFormat" ("SFN") Suspended"
#define MSG_MAIL_SUBJECT_JA_TASK_CONT_UUS       "Job-array task "sge_U32CFormat"."sge_U32CFormat" ("SFN") Continued"
#define MSG_MAIL_SUBJECT_JOB_CONT_US            "Job "sge_U32CFormat" ("SFN") Continued"



#define MSG_MAIL_BODY_SSSSS                     _MESSAGE(27065, _(SFN"\n Master queue    = "SFN"\n Owner           = "SFN"\n Submission time = "SFN"\n Start time      = "SFN))
#define MSG_MAIL_TYPE_SUSP                      _MESSAGE(27066, _("job suspend"))
#define MSG_MAIL_TYPE_CONT                      _MESSAGE(27067, _("job continue"))
#define MSG_MAIL_UNKNOWN_REASON                 _MESSAGE(27068, _("<unknown reason>"))
#define MSG_MAIL_TYPE_ADMIN                     _MESSAGE(27069, _("admin"))
#define MSG_MAIL_TYPE_START                     _MESSAGE(27070, _("job start")) 
#define MSG_MAIL_TYPE_ARSTART                   _MESSAGE(27071, _("advance_reservation start")) 
#define MSG_MAIL_TYPE_AREND                     _MESSAGE(27072, _("advance_reservation finish")) 
#define MSG_MAIL_TYPE_ARDELETE                  _MESSAGE(27073, _("advance_reservation delete")) 
#define MSG_MAIL_TYPE_ARERROR                   _MESSAGE(27074, _("advance_reservation error")) 
#define MSG_MAIL_TYPE_AROK                      _MESSAGE(27075, _("advance_reservation error resolved")) 


/* CR: don't localize mail subject, until we send it in Mime format!
 *  The message definition is not l10n'ed (no _() macro used)!!!     
 */
#define MSG_MAIL_SUBJECT_JA_TASK_COMP_UUS       "Job-array task "sge_U32CFormat"."sge_U32CFormat" ("SFN") Complete"
#define MSG_MAIL_SUBJECT_JOB_COMP_US            "Job "sge_U32CFormat" ("SFN") Complete"



#define MSG_MAIL_BODY_COMP_SSSSSSSSSSSI         _MESSAGE(27076, _(SFN"\n User             = "SFN"\n Queue            = "SFN"\n Host             = "SFN"\n Start Time       = "SFN"\n End Time         = "SFN"\n User Time        = "SFN"\n System Time      = "SFN"\n Wallclock Time   = "SFN"\n CPU              = "SFN"\n Max vmem         = "SFN"\n Exit Status      = %d"))
#define MSG_MAIL_TYPE_COMP                      _MESSAGE(27077, _("job completion"))
#define MSG_MAIL_ACTION_MIGR                    _MESSAGE(27078, _("Migrates"))
#define MSG_MAIL_ACTION_RESCH                   _MESSAGE(27079, _("Rescheduled"))
#define MSG_MAIL_ACTION_ERR                     _MESSAGE(27080, _("Set in error state"))
#define MSG_MAIL_ACTION_ERR_COMMENT             _MESSAGE(27081, _("\nUse \"qmod -c <jobid>\" to clear job error state\nonce the problem is fixed."))
#define MSG_MAIL_ACTION_ABORT                   _MESSAGE(27082, _("Aborted"))

/* CR: don't localize mail subject, until we send it in Mime format!
 *  The message definition is not l10n'ed (no _() macro used)!!!     
 */
#define MSG_MAIL_SUBJECT_JA_TASK_STATE_UUSS     "Job-array task "sge_U32CFormat"."sge_U32CFormat" ("SFN") "SFN
#define MSG_MAIL_SUBJECT_JOB_STATE_USS          "Job "sge_U32CFormat" ("SFN") "SFN

#define MSG_MAIL_BODY_STATE_SSSSSSSSSSSSS       _MESSAGE(27087, _(SFN"\n Exit Status      = "SFN"\n Signal           = "SFN"\n User             = "SFN"\n Queue            = "SFN"\n Host             = "SFN"\n Start Time       = "SFN"\n End Time         = "SFN"\n CPU              = "SFN"\n Max vmem         = "SFN"\nfailed "SFN" because:\n"SFN SFN))
#define MSG_MAIL_TYPE_STATE                     _MESSAGE(27088, _("job abortion/end"))
/* #define MSG_MAIL_TYPE_ABORT                     _message(27089, _("job abortion")) __TS Removed automatically from testsuite!! TS__*/
#define MSG_MAIL_UNKNOWN_NAME                   _MESSAGE(27090, _("<unknown>"))
#define MSG_MAIL_ACTION_APPERROR                _MESSAGE(27091, _("Rescheduled due to application error"))

/* 
** qmaster_heartbeat.c 
*/
#define MSG_HEART_CANNOTOPEN_SS             _MESSAGE(27041, _("can't open heartbeat file "SFQ": "SFN))
#define MSG_HEART_CANNOT_FSEEK_SS           _MESSAGE(27092, _("can't seek to the beginning of heartbeat file "SFQ": "SFN))
#define MSG_HEART_NO_FILENAME               _MESSAGE(27093, _("no heartbeat file name specified"))
#define MSG_HEART_WRITE_ERROR_SS            _MESSAGE(27094, _("can't write data to heartbeat file "SFQ": "SFN))
#define MSG_HEART_CLOSE_ERROR_SS            _MESSAGE(27095, _("error closing heartbeat file "SFQ": "SFN))
#define MSG_HEART_WRITE_TIMEOUT_S           _MESSAGE(27096, _("got timeout error while write data to heartbeat file "SFQ))
#define MSG_HEART_CANNOT_READ_FILE_S        _MESSAGE(27097, _("can't read act_qmaster file: "SFN))
#define MSG_HEART_ACT_QMASTER_FILE_CHANGED  _MESSAGE(27098, _("act_qmaster file has been changed by another qmaster"))
#define MSG_HEART_READ_TIMEOUT_S            _MESSAGE(27099, _("got timeout error while read data from heartbeat file "SFQ))
#define MSG_HEART_CANT_SIGNAL               _MESSAGE(27100, _("can't send signal to signal thread"))

/* procfs.c */
#define MSG_NO_SMAPS                        _MESSAGE(27102, _("memory accounting inaccurate with USE_SMAPS=false"))
#define MSG_NO_SWAP                         _MESSAGE(27103, _("memory accounting can't include swap: using (unrealistic) VMsize"))

#endif /* MSG_DAEMONS_COMMON_H */

