#ifndef __MSG_WINGRID_H
#define __MSG_WINGRID_H
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
** libs/wingrid/misc.c
*/
#define MSG_WIN_CANT_GET_PASSWD_INFO         _MESSAGE(212002, _("Can't get account information from system for user id %d"))
#define MSG_WIN_CHECK_SETUSER_FAILED         _MESSAGE(212003, _("setuser("SFN", SU_CHECK) failed, reason: '"SFN"' (errno %d)"))
#define MSG_WIN_SETUSER_FAILED               _MESSAGE(212004, _("setuser("SFN", SU_COMPLETE) failed, reason: '"SFN"' (errno %d)"))
#define MSG_WIN_CANT_SETGID_IN_SETUSER       _MESSAGE(212005, _("setgid(%d) failed in function wl_setuser(), reason: '"SFN"' (errno %d)"))
#define MSG_WIN_CANT_SETUID_IN_SETUSER       _MESSAGE(212006, _("setuid(%d) failed in function wl_setuser(), reason: '"SFN"' (errno %d)"))

#endif
