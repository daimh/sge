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

#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <float.h>
#include <stdio.h>

#include "uti/sge_rmon.h"
#include "uti/sge_lock.h"
#include "uti/sge_lock_fifo.h"
#include "uti/sge_mtutil.h"
#include "uti/msg_lcklib.h"


#ifdef SGE_DEBUG_LOCK_TIME
static double reader_min[NUM_OF_LOCK_TYPES] = {DBL_MAX, DBL_MAX};
static double reader_max[NUM_OF_LOCK_TYPES] = {0.0, 0.0};
static double reader_all[NUM_OF_LOCK_TYPES] = {0.0, 0.0};
static double reader_count[NUM_OF_LOCK_TYPES] = {0.0, 0.0};
static double writer_min[NUM_OF_LOCK_TYPES] = {DBL_MAX, DBL_MAX};
static double writer_max[NUM_OF_LOCK_TYPES] = {0.0, 0.0};
static double writer_all[NUM_OF_LOCK_TYPES] = {0.0, 0.0};
static double writer_count[NUM_OF_LOCK_TYPES] = {0.0, 0.0};
#endif

#if 0
#define PRINT_LOCK
#endif

/****** sge_lock/Introduction ****************************************************
*  NAME
*     Grid Engine Locking API
*
*  FUNCTION
*     The Grid Engine Locking API is a mediator between a lock service provider
*     and a lock client. A lock service provider offers a particular lock
*     implementation by registering a set of callbacks. A lock client does acquire
*     and release a lock using the respective API functions.
*
*     A lock service provider (usually a daemon) needs to register three
*     different callbacks:
*
*       + a lock callback, which is used by the API lock function
*
*       + an unlock callback, which is used by the API unlock function
*
*       + an ID callback, which is used by the API locker ID function
*     
*     Lock service provider has to register these callbacks *before* lock client
*     uses the lock/unlock API functions. Otherwise the lock/unlock operations do
*     have no effect at all.
*
*     Locktype denotes the entity which will be locked/unlocked (e.g. Global
*     Lock. Lockmode denotes in which mode the locktype will be locked/unlocked.
*     Locker ID unambiguously identifies a lock client.
*
*     Adding a new locktype does recquire two steps:
*
*     1. Add an enumerator to 'sge_locktype_t'. Do not forget to update
*        'NUM_OF_TYPES'.
*
*     2. Add a description to 'locktype_names'.
*
*  SEE ALSO
*     sge_lock/sge_lock.h
*******************************************************************************/
#ifdef SGE_USE_LOCK_FIFO
static sge_fifo_rw_lock_t Global_Lock;
static sge_fifo_rw_lock_t Master_Conf_Lock;

/* watch out. The order in this array has to be the same as in the sge_fifo_rw_lock_t type */
static sge_fifo_rw_lock_t *SGE_RW_Locks[NUM_OF_LOCK_TYPES] = {
   &Global_Lock, 
   &Master_Conf_Lock,
};

#else
static pthread_rwlock_t Global_Lock;
static pthread_rwlock_t Master_Conf_Lock;

/* watch out. The order in this array has to be the same as in the sge_locktype_t type */
static pthread_rwlock_t *SGE_RW_Locks[NUM_OF_LOCK_TYPES] = {
   &Global_Lock, 
   &Master_Conf_Lock,
};
#endif


/* 'locktype_names' has to be in sync with the definition of 'sge_locktype_t' */
static const char* locktype_names[NUM_OF_LOCK_TYPES] = {
   "global",          /* LOCK_GLOBAL */
   "master_config",   /* LOCK_MASTER_CONF */ 
};

static pthread_once_t lock_once = PTHREAD_ONCE_INIT;
static void lock_once_init(void);
/* lock service provider */
static sge_locker_t id_callback_impl(void);

static sge_locker_t (*id_callback)(void) = id_callback_impl;


/****** sge_lock/sge_lock() ****************************************************
*  NAME
*     sge_lock() -- Acquire lock
*
*  SYNOPSIS
*     void sge_lock(sge_locktype_t aType, sge_lockmode_t aMode)
*
*  FUNCTION
*     Acquire lock. If the lock is already held, block the caller until lock
*     becomes available again.
*
*     Instead of using this function directly the convenience macro
*     'SGE_LOCK(type, mode)' could (and should) be used. 
*     
*
*  INPUTS
*     sge_locktype_t aType - lock to acquire
*     sge_lockmode_t aMode - lock mode
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: sge_lock() is MT safe 
*******************************************************************************/
#ifdef SGE_LOCK_DEBUG
void sge_lock(sge_locktype_t aType, sge_lockmode_t aMode, const char *func)
{
   int res = -1;

   DENTER(TOP_LAYER, "sge_lock");

   pthread_once(&lock_once, lock_once_init);

#ifdef PRINT_LOCK
   {
      struct timeval now;
      gettimeofday(&now, NULL);
      printf("%ld lock %lu:%lus %s(%d)\n", (long int) pthread_self(),now.tv_sec, now.tv_usec, locktype_names[aType], aMode); 
   }   
#endif   

   if (aMode == LOCK_READ) {
      DLOCKPRINTF(("%s() about to lock rwlock \"%s\" for reading\n", func, locktype_names[aType]));
#ifdef SGE_USE_LOCK_FIFO
      res = sge_fifo_lock(SGE_RW_Locks[aType], true) ? 0 : 1;
#else
      res = pthread_rwlock_rdlock(SGE_RW_Locks[aType]);
#endif
      DLOCKPRINTF(("%s() locked rwlock \"%s\" for reading\n", func, locktype_names[aType]));
   } else if (aMode == LOCK_WRITE) {
       DLOCKPRINTF(("%s() about to lock rwlock \"%s\" for writing\n", func, locktype_names[aType]));
#ifdef SGE_USE_LOCK_FIFO
      res = sge_fifo_lock(SGE_RW_Locks[aType], false) ? 0 : 1;
#else
      res = pthread_rwlock_wrlock(SGE_RW_Locks[aType]);
#endif
      DLOCKPRINTF(("%s() locked rwlock \"%s\" for writing\n", func, locktype_names[aType]));
   } else {
      DLOCKPRINTF(("wrong lock type for global lock\n")); 
   }   

   if (res != 0) {
      DLOCKPRINTF((MSG_LCK_RWLOCKFORWRITINGFAILED_SSS, func, locktype_names[aType], strerror(res)));
      abort();
   }

#ifdef PRINT_LOCK
   {
      struct timeval now;
      gettimeofday(&now, NULL);
      printf("%ld got lock %lu:%lus %s(%d)\n", (long int) pthread_self(),now.tv_sec, now.tv_usec, locktype_names[aType], aMode); 
   }   
#endif   

   DRETURN_VOID;
} /* sge_lock */

#else
void sge_lock(sge_locktype_t aType, sge_lockmode_t aMode, const char *func)
{
   int res = -1;

#ifdef SGE_DEBUG_LOCK_TIME
   struct timeval before;
   struct timeval after;
   double time;
#endif

   DENTER(BASIS_LAYER, "sge_lock");
   
   pthread_once(&lock_once, lock_once_init);

#ifdef SGE_DEBUG_LOCK_TIME
   gettimeofday(&before, NULL);
#endif

   if (aMode == LOCK_READ) {
#ifdef SGE_USE_LOCK_FIFO
      res = sge_fifo_lock(SGE_RW_Locks[aType], true) ? 0 : 1;
#else
      res = pthread_rwlock_rdlock(SGE_RW_Locks[aType]);
#endif
   } else if (aMode == LOCK_WRITE) {
#ifdef SGE_USE_LOCK_FIFO
      res = sge_fifo_lock(SGE_RW_Locks[aType], false) ? 0 : 1;
#else
      res = pthread_rwlock_wrlock(SGE_RW_Locks[aType]);
#endif
   } else {
      DLOCKPRINTF(("wrong lock type for global lock\n")); 
   } 

   if (res != 0) {
      DLOCKPRINTF((MSG_LCK_RWLOCKFORWRITINGFAILED_SSS, func, locktype_names[aType], strerror(res)));
      abort();
   }

#ifdef SGE_DEBUG_LOCK_TIME
   gettimeofday(&after, NULL);
   time = after.tv_usec - before.tv_usec;
   time = after.tv_sec - before.tv_sec + (time/1000000);

   if (aMode == LOCK_READ) {
      if (time < reader_min[aType]) {
         reader_min[aType] = time;
      }  
      if (time > reader_max[aType]) {
         reader_max[aType] = time;
      } 
      reader_all[aType] += time;
      reader_count[aType]++; 
   } else {
      if (time < writer_min[aType]) {
         writer_min[aType] = time;
      }  
      if (time > writer_max[aType]) {
         writer_max[aType] = time;
      } 
      writer_all[aType] += time;
      writer_count[aType]++; 
   }
#endif

   DRETURN_VOID;
} /* sge_lock */
#endif

/****** sge_lock/sge_unlock() **************************************************
*  NAME
*     sge_unlock() -- Release lock
*
*  SYNOPSIS
*     void sge_unlock(sge_locktype_t aType, sge_lockmode_t aMode)
*
*  FUNCTION
*     Release lock. 
*
*     Instead of using this function directly the convenience macro
*     'SGE_UNLOCK(type, mode)' could (and should) be used. 
*
*  INPUTS
*     sge_locktype_t aType - lock to release
*     sge_lockmode_t aMode - lock mode in which the lock has been acquired 
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: sge_unlock() is MT safe 
*******************************************************************************/
#ifdef SGE_LOCK_DEBUG
void sge_unlock(sge_locktype_t aType, sge_lockmode_t aMode, const char *func)
{
   int res = -1;
   DENTER(TOP_LAYER, "sge_unlock");

   pthread_once(&lock_once, lock_once_init);
#ifdef SGE_USE_LOCK_FIFO
   res = sge_fifo_ulock(SGE_RW_Locks[aType], (bool)(aMode == LOCK_READ)) ? 0 : 1;
#else
   res = pthread_rwlock_unlock(SGE_RW_Locks[aType]);
#endif
   if (res != 0) {
      DLOCKPRINTF((MSG_LCK_RWLOCKUNLOCKFAILED_SSS, func, locktype_names[aType], strerror(res)));
      abort();
   }
   DLOCKPRINTF(("%s() unlocked rwlock \"%s\"\n", func, locktype_names[aType]));

#ifdef PRINT_LOCK
   {
      struct timeval now;
      gettimeofday(&now, NULL);
      printf("%ld unlock %lu:%lus %s(%d)\n", (long int) pthread_self(),now.tv_sec, now.tv_usec, locktype_names[aType], aMode); 
   }   
#endif

   DRETURN_VOID;
} /* sge_unlock */
#else
void sge_unlock(sge_locktype_t aType, sge_lockmode_t aMode, const char *func)
{
   int res = -1;
   
   DENTER(BASIS_LAYER, "sge_unlock");

   pthread_once(&lock_once, lock_once_init);

#ifdef SGE_USE_LOCK_FIFO
   res = sge_fifo_ulock(SGE_RW_Locks[aType], (bool)(aMode == LOCK_READ)) ? 0 : 1;
#else
   res = pthread_rwlock_unlock(SGE_RW_Locks[aType]);
#endif
   if (res != 0) {
      DLOCKPRINTF((MSG_LCK_RWLOCKUNLOCKFAILED_SSS, func, locktype_names[aType], strerror(res)));
      abort();
   }

   DRETURN_VOID;
} /* sge_unlock */


#endif

/****** sge_lock/sge_locker_id() ***********************************************
*  NAME
*     sge_locker_id() -- Locker identifier 
*
*  SYNOPSIS
*     sge_locker_t sge_locker_id(void) 
*
*  FUNCTION
*     Return an unambiguous identifier for the locker.  
*
*  INPUTS
*     void - none 
*
*  RESULT
*     sge_locker_t - locker identifier 
*
*  NOTES
*     There is a 1 to 1 mapping between a locker id an a thread. However the 
*     locker id and the thread id may be different.
*
*     MT-NOTE: sge_locker_id() is MT safe
*******************************************************************************/
sge_locker_t sge_locker_id(void)
{
   sge_locker_t id = 0;

   if (NULL != id_callback) {
      id = (sge_locker_t)id_callback();
   }

   return id;
} /* sge_locker_id */

/****** libs/lck/lock_once_init() **************************
*  NAME
*     lock_once_init() -- setup lock service 
*
*  SYNOPSIS
*     static void lock_once_init(void) 
*
*  FUNCTION
*     Determine number of locks needed. Create and initialize the respective
*     mutexes. Register the callbacks required by the locking API 
*
*  INPUTS
*     void - none 
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: lock_once_init() is NOT MT safe. 
*
*     Currently we do not use so called recursive mutexes. This may change
*     *without* warning, if necessary!
*
*  SEE ALSO
*     libs/lck/sge_lock.c
*
*******************************************************************************/
static void lock_once_init(void)
{
#ifdef SGE_USE_LOCK_FIFO
   sge_fifo_lock_init(&Global_Lock);
   sge_fifo_lock_init(&Master_Conf_Lock);
#else
   pthread_rwlock_init(&Global_Lock, NULL); 
   pthread_rwlock_init(&Master_Conf_Lock, NULL);
#endif
   return;
} /* prog_once_init() */

/****** libs/lck/id_callback_impl() *********************************
*  NAME
*     id_callback_impl() -- locker ID callback 
*
*  SYNOPSIS
*     static sge_locker_t id_callback_impl(void) 
*
*  FUNCTION
*     Return ID of current locker. 
*
*  INPUTS
*     void - none 
*
*  RESULT
*     sge_locker_t - locker id
*
*  NOTES
*     MT-NOTE: id_callback() is MT safe. 
*
*******************************************************************************/
static sge_locker_t id_callback_impl(void)
{
   return (sge_locker_t)pthread_self();
} /* id_callback */

#ifdef SGE_DEBUG_LOCK_TIME
void sge_debug_time(sge_locktype_t aType) 
{
      fprintf(stderr, "reader_min   = %f\n", reader_min[aType]);
      fprintf(stderr, "reader_max   = %f\n", reader_max[aType]);
      fprintf(stderr, "reader_avg   = %f\n", reader_all[aType]/reader_count[aType]);
      fprintf(stderr, "reader_count = %f\n", reader_count[aType]);
      fprintf(stderr, "writer_min   = %f\n", writer_min[aType]);
      fprintf(stderr, "writer_max   = %f\n", writer_max[aType]);
      fprintf(stderr, "writer_avg   = %f\n", writer_all[aType]/writer_count[aType]);
      fprintf(stderr, "writer_count = %f\n", writer_count[aType]);
}
#endif
