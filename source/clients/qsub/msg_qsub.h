#ifndef __MSG_QSUB_H
#define __MSG_QSUB_H
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

#define MSG_QSUB_WARNING_S    _MESSAGE(21000, _("qsub: warning: "))
#define MSG_QSUB_WAITINGFORIMMEDIATEJOBTOBESCHEDULED    _MESSAGE(21001, _("Waiting for immediate job to be scheduled."))
#define MSG_QSUB_YOURQSUBREQUESTCOULDNOTBESCHEDULEDDTRYLATER    _MESSAGE(21003, _("Your qsub request could not be scheduled; try again later."))
#define MSG_QSUB_YOURIMMEDIATEJOBXHASBEENSUCCESSFULLYSCHEDULED_S    _MESSAGE(21004, _("Your immediate job "SFN" has been successfully scheduled."))
#define MSG_QSUB_COULDNOTINITIALIZEENV_S    _MESSAGE(21005, _("Unable to initialize environment because of error: "SFN"\nExiting."))
#define MSG_QSUB_COULDNOTRUNJOB_S    _MESSAGE(21006, _("Unable to run job: "SN_UNLIMITED"\nExiting."))
#define MSG_QSUB_COULDNOTWAITFORJOB_S    _MESSAGE(21007, _("Unable to wait for job because of error: "SFN"\nExiting."))
#define MSG_QSUB_JOBNEVERRAN_S    _MESSAGE(21008, _("Unable to run job "SFN))
#define MSG_QSUB_JOBRECEIVEDSIGNAL_SS    _MESSAGE(21009, _("Job "SFN" exited because of signal "SFN))
#define MSG_QSUB_JOBFINISHUNCLEAR_S    _MESSAGE(21010, _("No information available on job "SFN"'s exit status."))
#define MSG_QSUB_COULDNOTFINALIZEENV_S    _MESSAGE(21011, _("Unable to finalize environment because of error: "SFN"\nExiting."))
#define MSG_QSUB_YOURJOBHASBEENSUBMITTED_SS    _MESSAGE(21012, _("Your job "SFN" ("SFQ") has been submitted."))
#define MSG_QSUB_JOBEXITED_SI    _MESSAGE(210013, _("Job "SFN" exited with exit code %d."))
#define MSG_QSUB_INTERRUPTED    _MESSAGE(210015, _("Interrupted!"))
#define MSG_QSUB_TERMINATING    _MESSAGE(210016, _("Please wait while qsub shuts down."))
#define MSG_QSUB_COULDNOTREADSCRIPT_S    _MESSAGE(210017, _("Unable to read script file because of error: "))

#endif /* __MSG_QSUB_H */

