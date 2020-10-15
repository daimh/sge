#ifndef __MSG_QHOST_H
#define __MSG_QHOST_H
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
 *  Portions of this code are Copyright 2011 Univa Inc.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/


#include "basis_types.h"


/*
** qhost.c
*/
#define MSG_HEADER_HOSTNAME              "HOSTNAME"
#define MSG_HEADER_ARCH                  "ARCH"
#define MSG_HEADER_NPROC                 "NCPU"
#define MSG_HEADER_LOAD                  "LOAD"
#define MSG_HEADER_MEMTOT                "MEMTOT"
#define MSG_HEADER_MEMUSE                "MEMUSE"
#define MSG_HEADER_SWAPTO                "SWAPTO"
#define MSG_HEADER_SWAPUS                "SWAPUS"
#define MSG_QHOST_h_OPT_USAGE        _MESSAGE(9001, _("display only selected hosts"))
#define MSG_QHOST_q_OPT_USAGE        _MESSAGE(9002, _("display queues hosted by host"))
#define MSG_QHOST_j_OPT_USAGE        _MESSAGE(9003, _("display jobs hosted by host"))
#define MSG_QHOST_l_OPT_USAGE        _MESSAGE(9004, _("request the given resources"))
#define MSG_QHOST_F_OPT_USAGE        _MESSAGE(9005, _("show (selected) resources"))
#define MSG_QHOST_u_OPT_USAGE        _MESSAGE(9006, _("show only jobs for user"))
#define MSG_QHOST_ncb_OPT_USAGE      _MESSAGE(9007, _("suppress host topology based information"))

#endif /* __MSG_QHOST_H */

