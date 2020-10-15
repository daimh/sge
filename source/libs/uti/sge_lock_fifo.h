#ifndef _SGE_LOCK_FIFO_H_
#define _SGE_LOCK_FIFO_H_

#include <pthread.h>

#include "basis_types.h"

#include "uti/sge_lock.h"

typedef struct sge_fifo_elem_t__ {
   /*
    * is the waiting thread a reader or writer
    */
   bool is_reader;

   /*
    * has this thread already been signaled
    */
   bool is_signaled;

   /*
    * condition to wakeup a waiting thread
    */
   pthread_cond_t cond;
} sge_fifo_elem_t;

typedef struct sge_fifo_rw_lock_t__ {
   /* 
    * mutex to guard this structure
    */
   pthread_mutex_t mutex;

   /*
    * condition to wakeup a waiting thread which got not
    * no position in the queue of waiting threads
    */
   pthread_cond_t cond;

   /*
    * fifo array where information about waiting threads is stored.
    */
   sge_fifo_elem_t *array;

   /* 
    * position of the next thread which gets the lock 
    */
   int head;            

   /* 
    * position in the array where the next thread will be placed which has to wait 
    */
   int tail;          

   /* 
    * maximum array size 
    */ 
   int size;            

   /* 
    * number of reader threads currently active 
    */
   int reader_active;   

   /* 
    * number of waiting threads in the queue which try to get a read lock 
    */
   int reader_waiting;  
   
   /*
    * number of writer threads currently active (maximum is 1)
    */
   int writer_active;

   /*
    * number of waiting threads in the queue which try to get the write lock
    */
   int writer_waiting;  

   /*
    * number of threads which do neither get a lock nor get a free position in the array
    */
   int waiting;       

   /*
    * number of waiting threads which have been signaled so that they wake up (maximum is 1)
    */
   int signaled;
} sge_fifo_rw_lock_t;

bool 
sge_fifo_lock_init(sge_fifo_rw_lock_t *lock);

bool 
sge_fifo_lock(sge_fifo_rw_lock_t *lock, bool is_reader);

bool 
sge_fifo_ulock(sge_fifo_rw_lock_t *lock, bool is_reader);

void 
sge_fifo_debug(sge_locktype_t aType);

void 
sge_debug_time(sge_locktype_t aType);

#endif
