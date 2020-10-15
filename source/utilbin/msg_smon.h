#ifndef __MSG_SMON_H
#define __MSG_SMON_H
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

#define MSG_USAGE                               _MESSAGE(47231, _("usage:"))
#define MSG_SGESHAREMON_NOSHARETREE             _MESSAGE(47232, _("No share tree"))
#define MSG_SGESHAREMON_c_OPT_USAGE             _MESSAGE(47233, _("number of collections (default is infinite)"))
#define MSG_SGESHAREMON_d_OPT_USAGE             _MESSAGE(47234, _("delimiter between columns (default is <TAB>)"))
#define MSG_SGESHAREMON_f_OPT_USAGE             _MESSAGE(47235, _("list of fields to print"))
#define MSG_SGESHAREMON_h_OPT_USAGE             _MESSAGE(47236, _("print a header containing the field names"))
#define MSG_SGESHAREMON_i_OPT_USAGE             _MESSAGE(47237, _("collection interval in seconds (default is 15)"))
#define MSG_SGESHAREMON_l_OPT_USAGE             _MESSAGE(47238, _("delimiter between nodes (default is <CR>)"))
#define MSG_SGESHAREMON_m_OPT_USAGE             _MESSAGE(47239, _("output file fopen mode (default is \"w\")"))
#define MSG_SGESHAREMON_n_OPT_USAGE             _MESSAGE(47240, _("use name=value format"))
#define MSG_SGESHAREMON_o_OPT_USAGE             _MESSAGE(47241, _("output file"))
#define MSG_SGESHAREMON_r_OPT_USAGE             _MESSAGE(47242, _("delimiter between collection records (default is <CR>)"))
#define MSG_SGESHAREMON_s_OPT_USAGE             _MESSAGE(47243, _("format of displayed strings (default is %s)"))
#define MSG_SGESHAREMON_t_OPT_USAGE             _MESSAGE(47244, _("show formatted times"))
#define MSG_SGESHAREMON_u_OPT_USAGE             _MESSAGE(47245, _("show decayed usage (since timestamp) in nodes"))
#define MSG_SGESHAREMON_x_OPT_USAGE             _MESSAGE(47246, _("exclude non-leaf nodes"))
#define MSG_ERROR_XISNOTAVALIDINTERVAL_S        _MESSAGE(47247, _(""SFN" is not a valid interval"))
#define MSG_ERROR_XISNOTAVALIDCOUNT_S           _MESSAGE(47248, _(""SFN" is not a valid count"))
#define MSG_WRITE_FAILED                        _MESSAGE(47249, _("write failed"))

#endif /* __MSG_SMON_H */

