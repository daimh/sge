#ifndef __MSG_QMOD_H
#define __MSG_QMOD_H
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
** qmod.c
*/
#define MSG_QMOD_c_OPT_USAGE      _MESSAGE(11008, _("clear error state"))
#define MSG_QMOD_c_OPT_USAGE_Q    _MESSAGE(11009, _("clear queue error state"))
#define MSG_QMOD_c_OPT_USAGE_J    _MESSAGE(11010, _("clear job error state"))
#define MSG_QMOD_r_OPT_USAGE      _MESSAGE(11011, _("reschedule jobs (running in queue)"))
#define MSG_QMOD_r_OPT_USAGE_J    _MESSAGE(11012, _("reschedule jobs"))
#define MSG_QMOD_r_OPT_USAGE_Q    _MESSAGE(11013, _("reschedule all jobs in a queue"))
#define MSG_QMOD_d_OPT_USAGE      _MESSAGE(11014, _("disable"))
#define MSG_QMOD_e_OPT_USAGE      _MESSAGE(11015, _("enable"))
#define MSG_QMOD_f_OPT_USAGE      _MESSAGE(11016, _("force action"))
#define MSG_QMOD_s_OPT_USAGE      _MESSAGE(11018, _("suspend"))
#define MSG_QMOD_s_OPT_USAGE_J    _MESSAGE(11019, _("suspend jobs"))
#define MSG_QMOD_s_OPT_USAGE_Q    _MESSAGE(11020, _("suspend queues"))
#define MSG_QMOD_us_OPT_USAGE     _MESSAGE(11021, _("unsuspend"))
#define MSG_QMOD_us_OPT_USAGE_J   _MESSAGE(11022, _("unsuspend jobs"))
#define MSG_QMOD_us_OPT_USAGE_Q   _MESSAGE(11023, _("unsuspend queues"))

#define MSG_QMOD_err_OPT_ISAGE    _MESSAGE(11024, _("set error state"))
#define MSG_QMOD_o_OPT_ISAGE      _MESSAGE(11025, _("set orphaned state"))
#define MSG_QMOD_do_OPT_ISAGE     _MESSAGE(11026, _("delete orphaned  state"))
#define MSG_QMOD_u_OPT_ISAGE      _MESSAGE(11027, _("set unknown state"))
#define MSG_QMOD_du_OPT_ISAGE     _MESSAGE(11028, _("delete unknown state"))
#define MSG_QMOD_c_OPT_ISAGE      _MESSAGE(11029, _("set ambiguous state"))
#define MSG_QMOD_dc_OPT_ISAGE     _MESSAGE(11030, _("delete ambiguous state"))

#endif /* __MSG_QMOD_H */

