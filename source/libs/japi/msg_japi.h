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

#ifndef _MSG_JAPI_H
#define	_MSG_JAPI_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "basis_types.h"
   
#define MSG_JAPI_NO_EVENT_CLIENT   _MESSAGE(45501, _("The event client has not been started."))
#define MSG_JAPI_EVENT_CLIENT_ALREADY_STARTED   _MESSAGE(45502, _("The event client has already been started."))
#define MSG_JAPI_EC_GET_PROBLEM _MESSAGE(45503, _("Problem receiving events from qmaster."))
#define MSG_JAPI_DISCONNECTED   _MESSAGE(45504, _("The qmaster has become unreachable.  Attempting to reconnect."))
#define MSG_JAPI_RECONNECTED    _MESSAGE(45505, _("Reconnected to qmaster."))
#define MSG_JAPI_QMASTER_DOWN   _MESSAGE(45506, _("The qmaster has gone down.  Waiting to reconnect."))
#define MSG_JAPI_NO_HANDLE_S    _MESSAGE(45507, _("Unable to open a connection to the qmaster: "SFN))
#define MSG_JAPI_NEGATIVE_TIMEOUT  _MESSAGE(45508, _("Negative timeout values are not allowed."))
#define MSG_JAPI_EC_THREAD_NOT_STARTED_S  _MESSAGE(45509, _("Couldn't create event client thread: "SFN))
#define MSG_JAPI_KILLED_EVENT_CLIENT   _MESSAGE(45510, _("Event client got shutdown signal."))
#ifdef ENABLE_PERSISTENT_JAPI_SESSIONS
#endif
#define MSG_JAPI_CANNOT_CLOSE_COMMLIB_S  _MESSAGE(45514, _("Unable to shutdown commlib handle: "SFN))
#define MSG_JAPI_BAD_GDI_ANSWER_LIST  _MESSAGE(45515, _("Error reading answer list from qmaster"))
#define MSG_JAPI_JOB_ALREADY_EXISTS_S  _MESSAGE(45516, _("Job with given job id already exists: "SFQ))
#define MSG_JAPI_TASK_REF_TWICE_UU  _MESSAGE(45517, _("Task "sge_U32CFormat" of jobid "sge_U32CFormat" referenced twice"))
#define MSG_JAPI_BAD_JOB_ID_S  _MESSAGE(45518, _("Job id, "SFQ", is not a valid job id"))
#define MSG_JAPI_BAD_BULK_JOB_ID_S  _MESSAGE(45519, _("Job id, "SFQ", is not a valid bulk job id"))
#define MSG_JAPI_QMASTER_TIMEDOUT _MESSAGE(45520, _("Timed out at qmaster. Waiting to reconnect."))
#ifdef	__cplusplus
}
#endif

#endif	/* _MSG_JAPI_H */
