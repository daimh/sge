#ifndef __MSG_LCKLIB_H__
#define __MSG_LCKLIB_H__

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
 *   Copyright: 2003 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "basis_types.h"

/*
 * sge_mtutil.c
 */
#define MSG_LCK_MUTEXLOCKFAILED_SSS   _MESSAGE(71000, _(SFQ" failed to lock "SFQ" - error: "SFQ))
#define MSG_LCK_MUTEXUNLOCKFAILED_SSS _MESSAGE(71001, _(SFQ" failed to unlock "SFQ" - error: "SFQ))

#define MSG_LCK_RWLOCKFORWRITINGFAILED_SSS _MESSAGE(71003, _(SFQ" failed to lock "SFQ" for writing - error: "SFQ))
#define MSG_LCK_RWLOCKUNLOCKFAILED_SSS     _MESSAGE(71004, _(SFQ" failed to unlock "SFQ" - error: "SFQ))

#endif /* __MSG_LCKLIB_H__ */
