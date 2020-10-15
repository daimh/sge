#ifndef __MSG_QRSH_H
#define __MSG_QRSH_H
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

#define MSG_QRSH_STARTER_CANNOTOPENFILE_SS   _MESSAGE(15000, _("qrsh_starter: cannot open file "SFN": "SFN))
#define MSG_QRSH_STARTER_CANNOTREADCONFIGFROMFILE_S _MESSAGE(15001, _("qrsh_starter: cannot read job configuration from file "SFN))
#define MSG_QRSH_STARTER_CANNOTCHANGEDIR_SS  _MESSAGE(15002, _("qrsh_starter: cannot change to directory "SFN": "SFN))
#define MSG_QRSH_STARTER_MALLOCFAILED_S      _MESSAGE(15003, _("qrsh_starter: malloc failed: "SFN))
#define MSG_QRSH_STARTER_CANNOTWRITEPID_SS   _MESSAGE(15005, _("qrsh_starter: cannot write pid file "SFN": "SFN))
#define MSG_QRSH_STARTER_CANNOTFORKCHILD_S   _MESSAGE(15006, _("qrsh_starter: cannot fork child process: "SFN))
#define MSG_QRSH_STARTER_CANNOTGETLOGIN_S    _MESSAGE(15007, _("qrsh_starter: cannot get login name: "SFN))
#define MSG_QRSH_STARTER_CANNOTGETUSERINFO_S _MESSAGE(15008, _("qrsh_starter: cannot get user information: "SFN))
#define MSG_QRSH_STARTER_CANNOTDETERMSHELL_S _MESSAGE(15009, _("qrsh_starter: cannot determine login shell, using "SFQ))
#define MSG_QRSH_STARTER_EMPTY_WRAPPER       _MESSAGE(15010, _("qrsh_starter: environment variable QRSH_WRAPPER has no value"))
#define MSG_QRSH_STARTER_EXECCHILDFAILED_S   _MESSAGE(15011, _("qrsh_starter: executing child process "SFN" failed: "SFN))
#define MSG_QRSH_STARTER_INVALIDCOMMAND      _MESSAGE(15012, _("qrsh_starter: received invalid command to execute"))
#define MSG_QRSH_STARTER_NOCWDINCONFIG       _MESSAGE(15013, _("qrsh_starter: can't get cwd from config"))

#endif /* __MSG_QRSH_H */
