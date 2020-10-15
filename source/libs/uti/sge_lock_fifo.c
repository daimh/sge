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
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <float.h>

#include "uti/sge_lock_fifo.h"
#include "uti/msg_lcklib.h"
#include "uti/sge_rmon.h"

#define FIFO_LOCK_QUEUE_LENGTH 64 

/****** lib/lock/sge_fifo_lock_init() *****************************************
*  NAME
*     sge_fifo_lock_init() -- initialize a fifo read/write lock 
*
*  SYNOPSIS
*     bool sge_fifo_lock_init(sge_fifo_rw_lock_t *lock) 
*
*  FUNCTION
*     This function is used to initialize a fifo read/write lock.
*
*     On success the function returns true. If the lock object can't be 
*     initialized then the function will return with false. 
*
*  INPUTS
*     sge_fifo_rw_lock_t *lock - fifo lock object
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_fifo_lock_init() is MT safe 
*
*  SEE ALSO
*     lib/lock/sge_fifo_lock_init 
*     lib/lock/sge_fifo_lock
*     lib/lock/sge_fifo_unlock
*******************************************************************************/
bool 
sge_fifo_lock_init(sge_fifo_rw_lock_t *lock)
{
   bool ret = true;
   int lret = 0;

   lret = pthread_mutex_init(&(lock->mutex), NULL);
   if (lret == 0) {
      lock->array = (sge_fifo_elem_t *)malloc(sizeof(sge_fifo_elem_t) * FIFO_LOCK_QUEUE_LENGTH);

      if (lock->array != NULL) {
         int i;

         for (i = 0; i < FIFO_LOCK_QUEUE_LENGTH; i++) {
            lock->array[i].is_reader = false;
            lock->array[i].is_signaled = false;
            lret = pthread_cond_init(&(lock->array[i].cond), NULL);
            if (lret != 0) {
               ret = false;
               break;
            }
         }
         if (lret == 0) {
            lret = pthread_cond_init(&(lock->cond), NULL);
            if (lret == 0) {
               lock->head = 0;
               lock->tail = 0;
               lock->size = FIFO_LOCK_QUEUE_LENGTH;
               lock->reader_active = 0;
               lock->reader_waiting = 0;
               lock->writer_active = 0;
               lock->writer_waiting = 0;
               lock->waiting = 0;
               lock->signaled = 0;
            } else {
               ret = false;
            }
         } else {
            /* has already been handled in the for loop above */
         }
      } else {
         ret = false;
      }
   } else {
      ret = false;
   }
   return ret;
}

/****** lib/lock/sge_fifo_lock() *********************************************
*  NAME
*     sge_fifo_lock() -- acquire a read/write lock
*
*  SYNOPSIS
*     bool sge_fifo_lock(sge_fifo_rw_lock_t *lock, bool is_reader) 
*
*  FUNCTION
*     A call to this function acquires either a read or a write lock
*     depending on the value of "is_reader".
*
*     If the value of "is_reader" is "true" the function returns as soon
*     as it gets the read lock. This is the case if there is noone 
*     currently holding the write lock and if there was noone previously
*     trying to get the write lock.
*
*     If the value of "is_reader" is "false" then the function returns
*     as soon as it gets the write lock. This is only the case if there
*     is noone holding a read or write lock and only if there was noone 
*     else who tried to get the read or write lock.
*
*     A thread my hold multiple concurrent read locks. If so the
*     corresponding sge_fifo_unlock() function has to be called once
*     for each lock obtained.
*
*     Multiple threads might obtain a read lock whereas only one thread
*     can have the write lock at the same time.
*
*     Threads which can't acquire a read or write lock block till the
*     lock is available. A certain number of blocking threads (defined by
*     the define FIFO_LOCK_QUEUE_LENGTH) wait in a queue so that
*     each of those threads has a chance to get the lock. 
*
*     If more than FIFO_LOCK_QUEUE_LENGTH threads try to get the lock 
*     it might happen that then there are threads which will never 
*     get the lock (This behaviour depends on the implementation
*     of the pthread library).
*
*     A read/write lock has to be initialized with sge_fifo_lock_init()
*     before it can be used with this function.
*
*  INPUTS
*     sge_fifo_rw_lock_t *lock - lock object 
*     bool is_reader           - try to get the read lock (true) 
*                                or write lock (false)
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error occured
*
*  NOTES
*     MT-NOTE: sge_fifo_lock() is MT safe 
*
*  SEE ALSO
*     lib/lock/sge_fifo_lock_init
*     lib/lock/sge_fifo_lock
*     lib/lock/sge_fifo_unlock
*******************************************************************************/
bool 
sge_fifo_lock(sge_fifo_rw_lock_t *lock, bool is_reader)
{
   bool ret = true;
   int lret = 0;

   /* lock the lock-structure */
   lret = pthread_mutex_lock(&(lock->mutex));
   if (lret == 0) {
      bool do_wait = false;
      bool do_wait_in_queue = false;

      /*
       * If the current thread has to wait later on and if there is
       * no place available in the list of waiting threads then wait
       * till there is space in the queue.
       *
       * read lock:
       *    if the queue is full and this readers can't
       *    get the lock because a writer has it already then
       *    this thread will wait either:
       *     - till there is a place in the queue or 
       *     - till the writer released the lock so that
       *       this reader can have it
       *
       * write lock: 
       *    if the queue is full then wait till there is space 
       *    available
       */
      do {
         do_wait = (bool)((lock->reader_waiting + lock->writer_waiting) == FIFO_LOCK_QUEUE_LENGTH);
         if (do_wait) {
            lock->waiting++;
            pthread_cond_wait(&(lock->cond), &(lock->mutex));
            lock->waiting--;
         }
      } while (do_wait);

      /* 
       * Append the thread to the queue if it is necessary
       *
       * read lock:
       *    if there is currently a writer active or waiting or another thread is currently 
       *    waking up (because is was signaled) then this reader has to wait in 
       *    queue. If there are other readers active or none is active then this 
       *    reader can continue.
       *
       * write lock:
       *    the writer has to wait in queue if there is an active reader or 
       *    writer or if someone is currently waking up...
       */
      if (is_reader) {
         do_wait_in_queue = (bool)(lock->writer_active + lock->writer_waiting + lock->signaled> 0);
      } else {
         do_wait_in_queue = (bool)((lock->writer_active + lock->reader_active + lock->signaled > 0));
      }
      if (do_wait_in_queue) {
         int index;

         /* 
          * position the tail pointer behind the element which
          * will be filled now. This will be the place where the
          * next waiting thread will be stored.
          */
         index = lock->tail;
         lock->tail++;

         /* 
          * check if the new tail is behind the position of the
          * allocated array. Move then to the first array element.
          */
         if (lock->tail == lock->size) {
            lock->tail = 0;
         }

         /* store information about the thread which will wait */
         lock->array[index].is_reader = is_reader;
         lock->array[index].is_signaled = false;

         /* 
          * block this thread now till it gets a signal to continue.
          * The signal will be sent by an unlock call of another
          * reader or writer which hat the lock before.
          */
         while (lock->array[index].is_signaled == false) {
            if (is_reader) {
               lock->reader_waiting++;
            } else {
               lock->writer_waiting++;
            }
            pthread_cond_wait(&(lock->array[index].cond), &(lock->mutex));
            if (is_reader) {
               lock->reader_waiting--;
            } else {
               lock->writer_waiting--;
            }
         }

         /*
          * remove this thread from the signaled threads counter 
          */
         if (lock->array[index].is_signaled == true) {
            lock->signaled--;
         }

         /*
          * This thread will get the lock because it is the first in
          * the queue. Remove the information about this thread from the 
          * queue. 
          */
         index = lock->head;
         lock->head++;

         /* 
          * check if the new head is behind the position of the
          * allocated array. Move then to the first array element.
          */
         if (lock->head == lock->size) {
            lock->head = 0;
         }

         /*
          * if this thread is a reader and if there is at least one 
          * additional thread in the queue and if that thread is 
          * also a reader then wake it so that they can do work
          * simultaniously
          */

         if (lock->array[index].is_reader == true && lock->reader_waiting > 0 &&
             lock->array[lock->head].is_reader == true) {
            lock->array[lock->head].is_signaled = true;
            lock->signaled++;
            pthread_cond_signal(&(lock->array[lock->head].cond));
         }

         /*
          * there is now space in the queue available. if there
          * are threads waiting outside the queue then notify  
          * one so that it can append at the end.
          */
         if (lock->waiting > 0) {
            pthread_cond_signal(&(lock->cond));
         }

         /*
          * this is not necessary but it might make debugging easier.
          * preinitialize the array element with predefined values.
          * which indicate that this entry is 'empty'
          */
         lock->array[index].is_reader = false;
         lock->array[index].is_signaled = false;
      }

      /*
       * now the thread has the lock. increase the counter.
       */
      if (is_reader) {
         lock->reader_active++;
      } else {
         lock->writer_active++;
      }

      /* unlock the lock-structure */
      lret = pthread_mutex_unlock(&(lock->mutex));
      if (lret != 0) {
         ret = false;
      }
   } else {
      ret = false;
   }
   return ret;
}

/****** lib/lock/sge_fifo_ulock() *****************************************
*  NAME
*     sge_fifo_ulock() -- release a read or write lock 
*
*  SYNOPSIS
*     bool sge_fifo_ulock(sge_fifo_rw_lock_t *lock, bool is_reader) 
*
*  FUNCTION
*     Releases a read or write lock previously obtained with 
*     sge_fifo_lock() or sge_fifo_unlock() 
*
*  INPUTS
*     sge_fifo_rw_lock_t *lock - lock object 
*     bool is_reader           - type of lock to be released 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error 
*
*  NOTES
*     MT-NOTE: sge_fifo_ulock() is MT safe 
*
*  SEE ALSO
*     lib/lock/sge_fifo_lock_init
*     lib/lock/sge_fifo_lock
*     lib/lock/sge_fifo_unlock
*******************************************************************************/
bool 
sge_fifo_ulock(sge_fifo_rw_lock_t *lock, bool is_reader)
{
   bool ret = true;
   int lret = 0;

   /* lock the lock-structure */
   lret = pthread_mutex_lock(&(lock->mutex));
   if (lret == 0) {

      /*
       * decrease the counter.
       */
      if (is_reader) {
         lock->reader_active--;
      } else {
         lock->writer_active--;
      }

      /* 
       * notify the next waiting thread if there is one
       */
      if ((lock->reader_active + lock->writer_active + lock->signaled) == 0 &&
          (lock->reader_waiting + lock->writer_waiting > 0)) {
         lock->array[lock->head].is_signaled = true;
         lock->signaled++;
         pthread_cond_signal(&(lock->array[lock->head].cond));
      }

      /* unlock the lock-structure */
      lret = pthread_mutex_unlock(&(lock->mutex));
      if (lret != 0) {
         ret = false;
      }
   } else {
      ret = false;
   }
   return ret;
}

