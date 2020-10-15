#ifndef __MSG_QQUOTA_H
#define __MSG_QQUOTA_H
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
** qquota.c
*/

#define MSG_QQUOTA_h_OPT_USAGE        _MESSAGE(10000, _("display only selected hosts"))
#define MSG_QQUOTA_l_OPT_USAGE        _MESSAGE(10001, _("request the given resources"))
#define MSG_QQUOTA_u_OPT_USAGE        _MESSAGE(10002, _("display only selected users"))
#define MSG_QQUOTA_pe_OPT_USAGE       _MESSAGE(10003, _("display only selected parallel environments"))
#define MSG_QQUOTA_P_OPT_USAGE        _MESSAGE(10004, _("display only selected projects"))
#define MSG_QQUOTA_q_OPT_USAGE        _MESSAGE(10005, _("display only selected queues"))

#define MSG_QQUOTA_HELP_WCPROJECT     _MESSAGE(10010, _("wildcard expression matching a project"))
#define MSG_QQUOTA_HELP_WCPE          _MESSAGE(10011, _("wildcard expression matching a parallel environment"))

#endif /* __MSG_QQUOTA_H */

