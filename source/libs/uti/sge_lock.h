#ifndef _SGE_LOCK_H_
#define _SGE_LOCK_H_

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
#include "uti/sge_rmon.h"

#if 0
#define SGE_DEBUG_LOCK_TIME 
#endif

#if 1
#define SGE_USE_LOCK_FIFO
#endif

#undef LOCK_READ
#undef LOCK_WRITE

typedef enum {
   LOCK_READ  = 1, /* shared  */
   LOCK_WRITE = 2  /* exclusive */
} sge_lockmode_t;

typedef u_long32 sge_locker_t;

typedef enum {
   /* 
    * global lock 
    */
   LOCK_GLOBAL  = 0, 

   LOCK_MASTER_CONF = 1,

   NUM_OF_LOCK_TYPES = 2
} sge_locktype_t;

void 
sge_lock(sge_locktype_t aType, sge_lockmode_t aMode, const char *func);

void 
sge_unlock(sge_locktype_t aType, sge_lockmode_t aMode, const char *func);

sge_locker_t 
sge_locker_id(void);

#if defined(SGE_LOCK)
#error "SGE_LOCK already defined!"
#endif

#define SGE_LOCK(type, mode) \
{ \
   sge_lock(type, mode, SGE_FUNC); \
}

#if defined(SGE_UNLOCK)
#error "SGE_UNLOCK already defined!"
#endif

#define SGE_UNLOCK(type, mode) \
{ \
   sge_unlock(type, mode, SGE_FUNC); \
}

#endif /* _SGE_LOCK_H_ */
