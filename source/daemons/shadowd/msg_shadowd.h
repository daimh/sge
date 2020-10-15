#ifndef __MSG_SHADOWD_H
#define __MSG_SHADOWD_H
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
** shadowd.c
*/
#define MSG_SHADOWD_CANTREADQMASTERSPOOLDIRFROMX_S       _MESSAGE(35000, _("can't read qmaster_spool_dir from "SFQ))
#define MSG_SHADOWD_FOUNDRUNNINGSHADOWDWITHPIDXNOTSTARTING_I   _MESSAGE(35001, _("found running shadowd with pid %d - not starting"))
#define MSG_SHADOWD_CANTCHANGETOQMASTERSPOOLDIRX_S       _MESSAGE(35002, _("can't change to qmaster spool directory: "SFN))
#define MSG_SHADOWD_CANTSWITCH_USER                      _MESSAGE(35003, _("can't switch user"))
#define MSG_SHADOWD_FAILEDTOLOCKQMASTERSOMBODYWASFASTER       _MESSAGE(35004, _("failed to lock qmaster -- somebody else was faster"))
#define MSG_SHADOWD_CANTSTARTQMASTER       _MESSAGE(35005, _("can't start qmaster"))
#define MSG_SHADOWD_DELAYINGSHADOWFUNCFORXSECONDS_U   _MESSAGE(35006, _("delaying shadow function for "sge_U32CFormat" seconds"))        
#define MSG_SHADOWD_CANTREADACTQMASTERFILEX_S       _MESSAGE(35007, _("can't read act_qmaster file "SFQ))
#define MSG_SHADOWD_CANTRESOLVEHOSTNAMEFROMACTQMASTERFILE_SS       _MESSAGE(35008, _("can't resolve hostname from act_qmaster file "SFQ": "SFQ))
#define MSG_SHADOWD_NOTASHADOWMASTERFILE_S       _MESSAGE(35009, _("this is not in shadow master file "SFQ))


#endif /* __MSG_SHADOWD_H */

