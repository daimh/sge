#ifndef __MSG_QALTER_H
#define __MSG_QALTER_H
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
** qalter.c
*/
#define MSG_QALTER                                    _MESSAGE(3000, _("qalter: "))
#define MSG_QALTERWARNING                             _MESSAGE(3001, _("qalter: warning: "))
#define MSG_JOB_NOJOBATTRIBUTESELECTED                  _MESSAGE(3002, _("no job attribute selected"))
#define MSG_ANSWER_FAILDTOBUILDREDUCEDDESCRIPTOR        _MESSAGE(3003, _("failed to build reduced descriptor"))
#define MSG_ANSWER_ALLANDJOBIDSARENOTVALID              _MESSAGE(3004, _("\'all\' AND jobids are not valid"))
#define MSG_ANSWER_0ISNOTAVALIDJOBID                    _MESSAGE(3005, _("0 is not a valid jobid"))

#endif /* __MSG_QALTER_H */

