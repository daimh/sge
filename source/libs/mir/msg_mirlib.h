#ifndef __MSG_MIRLIB_H
#define __MSG_MIRLIB_H
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

/* sge_mirror.c */
#define MSG_MIRROR_INVALID_OBJECT_TYPE_SI          _MESSAGE(67000, _(SFN": invalid object type %d"))
#define MSG_MIRROR_QMASTERALIVETIMEOUTEXPIRED      _MESSAGE(67001, _("qmaster alive timeout expired"))
#define MSG_MIRROR_CALLBACKFAILED_S                _MESSAGE(67002, _("callback function for event "SFQ" failed"))

#define MSG_MIRROR_NOTINITIALIZED                  _MESSAGE(67003, _("event client not yet initialized"))
#define MSG_MIRROR_BADARG                          _MESSAGE(67004, _("bad argument"))
#define MSG_MIRROR_TIMEOUT                         _MESSAGE(67005, _("timeout"))
#define MSG_MIRROR_DUPLICATEKEY                    _MESSAGE(67006, _("duplicate key"))
#define MSG_MIRROR_KEYNOTFOUND                     _MESSAGE(67007, _("key not found"))
#define MSG_MIRROR_CALLBACKFAILED                  _MESSAGE(67008, _("callback failed"))
#define MSG_MIRROR_PROCESSERRORS                   _MESSAGE(67009, _("errors processing events"))
#define MSG_MIRROR_OK                              _MESSAGE(67010, _("ok"))

#define MSG_JOB_RECEIVEDINVALIDUSAGEEVENTFORJOB_S  _MESSAGE(67012, _("received invalid job usage event for job "SFN))
#define MSG_JOB_CANTFINDJOBFORUPDATEIN_SS          _MESSAGE(67014, _("can't find job "SFN" for update in function "SFN))
#define MSG_JOB_CANTFINDJATASKFORUPDATEIN_SS       _MESSAGE(67015, _("can't find array task "SFN" for update in function "SFN))
#define MSG_JOB_CANTFINDPETASKFORUPDATEIN_SS       _MESSAGE(67016, _("can't find parallel task "SFN" for update in function "SFN))
#define MSG_CQUEUE_CANTFINDFORUPDATEIN_SS          _MESSAGE(67017, _("can't find cluster queue "SFN" for update in function "SFN))
#define MSG_QINSTANCE_CANTFINDFORUPDATEIN_SS       _MESSAGE(67018, _("can't find queue instance "SFN" for update in function "SFN))

#endif /* __MSG_MIRLIB_H */
