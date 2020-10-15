#ifndef __MSG_EVCLIB_H
#define __MSG_EVCLIB_H
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
** sge_event_client.c
*/ 
#define MSG_COMMD_FAILEDTOSENDACKEVENTDELIVERY        _MESSAGE(65000, _("failed to send ACK_EVENT_DELIVERY"))
#define MSG_EVENT_HIGHESTEVENTISXWHILEWAITINGFORY_UU  _MESSAGE(65001, _("highest event number is "sge_U32CFormat" while waiting for "sge_U32CFormat))
#define MSG_EVENT_SMALLESTEVENTXISGRTHYWAITFOR_UU     _MESSAGE(65002, _("smallest event number "sge_U32CFormat" is greater than number "sge_U32CFormat" I'm waiting for"))
#define MSG_EVENT_EVENTSWITHNOINCREASINGNUMBERS       _MESSAGE(65003, _("got events with not increasing numbers"))
#define MSG_LIST_FAILEDINCULLUNPACKREPORT             _MESSAGE(65004, _("Failed in cull_unpack report"))
#define MSG_EVENT_ILLEGAL_ID_OR_NAME_US               _MESSAGE(65005, _("Illegal id "sge_U32CFormat" or name "SFQ" in event client registration"))
#define MSG_EVENT_UNINITIALIZED_EC                    _MESSAGE(65006, _("event client not properly initialized (ec_prepare_registration)"))
#define MSG_EVENT_ILLEGALEVENTID_I                    _MESSAGE(65007, _("illegal event id %d"))
#define MSG_EVENT_ILLEGALFLUSHTIME_I                  _MESSAGE(65008, _("illegal flush time %d - must be in the range [0:63]"))
#define MSG_EVENT_NOTREGISTERED                       _MESSAGE(65009, _("event client not registered"))
#define MSG_EVENT_HAVETOHANDLEEVENTS                  _MESSAGE(65010, _("you have to handle the events sgeE_QMASTER_GOES_DOWN, sgeE_SHUTDOWN and sgeE_ACK_TIMEOUT"))

#endif /* __MSG_EVCLIB_H */
