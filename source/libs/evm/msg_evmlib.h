#ifndef __MSG_EVMLIB_H
#define __MSG_EVMLIB_H
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
** sge_event_master.c
*/
#define MSG_EVE_REINITEVENTCLIENT_S   _MESSAGE(66000, _("reinitialization of "SFQ))
#define MSG_EVE_UNKNOWNEVCLIENT_US    _MESSAGE(66001, _("no event client known with id "sge_U32CFormat" to "SFN))
#define MSG_EVE_CLIENTREREGISTERED_SSSU _MESSAGE(66002, _("event client "SFQ" ("SFN"/"SFN"/"sge_U32CFormat") reregistered - it will need a total update"))
#define MSG_EVE_REG_SUU               _MESSAGE(66003, _(SFQ" registers as event client with id "sge_U32CFormat" event delivery interval "sge_U32CFormat))
#define MSG_EVE_UNREG_SU              _MESSAGE(66004, _("event client "SFQ" with id "sge_U32CFormat" deregistered"))
#define MSG_EVE_EVENTCLIENT           _MESSAGE(66005, _("event client"))
#define MSG_EVE_ILLEGALIDREGISTERED_U _MESSAGE(66007, _("illegal event client id "sge_U32CFormat" for registration"))

#define MSG_EVE_INVALIDSUBSCRIPTION   _MESSAGE(66008, _("invalid subscription information"))
#define MSG_EVE_INVALIDINTERVAL_U     _MESSAGE(66009, _("invalid event interval "sge_U32CFormat))

#define MSG_EVE_TOTALUPDATENOTHANDLINGEVENT_I _MESSAGE(66010, _("event number %d is not handled by sge_total_update_event"))

#define MSG_COM_ACKTIMEOUT4EV_ISIS        _MESSAGE(66011, _("acknowledge timeout after %d seconds for event client ("SFN":%d) on host "SFQ))
#define MSG_COM_NOSHUTDOWNPERMS           _MESSAGE(66012, _("shutdown requires manager privileges"))
#define MSG_COM_SHUTDOWNNOTIFICATION_SUS  _MESSAGE(66013, _("sent shutdown notification to event client " SFN " with id " sge_U32CFormat " on host " SFN))
#define MSG_EVE_QMASTERISGOINGDOWN        _MESSAGE(66014, _("do not accept new event clients. Qmaster is going down"))
#define MSG_COM_KILLED_SCHEDULER          _MESSAGE(66015, _("sent shutdown notification to scheduler"))
#define MSG_WRONG_USER_FORFIXEDID         _MESSAGE(66016, _("only a manager can register event clients with a fixed id"))
#define MSG_TO_MANY_DYNAMIC_EC_U          _MESSAGE(66017, _("cannot register event client. Only "sge_U32CFormat" event clients are allowed in the system"))
#define MSG_SET_MAXDYNEVENTCLIENT_U       _MESSAGE(66018, _("max dynamic event clients is set to "sge_U32CFormat))
#define MSG_EVE_INCOMPLETEEVENTCLIENT     _MESSAGE(66020, _("wrong event client version"))
#define MSG_COM_ACKTIMEOUT4EV_SIS         _MESSAGE(66021, _("removing event client ("SFN":%d) on host "SFQ" after acknowledge timeout from event client list"))

#endif /* __MSG_EVMLIB_H */
