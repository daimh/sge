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
/* Portions of this code are Copyright (c) 2011 Univa Corporation. */
/*___INFO__MARK_END__*/
/*
 * Based on David Flanagan's Xmt libary's Hash.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <sys/times.h>

#include "uti/sge_rmon.h"
#include "uti/sge_htable.h"
#include "uti/sge_log.h"

#ifdef SGE_USE_PROFILING
#include "uti/sge_profiling.h"
#endif

/****** uti/htable/--Hashtable ***********************************************
*  NAME
*     htable -- A Hashtable Implementation for Grid Engine 
*
*  SYNOPSIS
*     htable sge_htable_create(int size, 
*                   int (*hash_func)(const void *), 
*                   int (*compare_func)(const void *, const void *));
*
*     void sge_htable_destroy(htable ht);
*
*     void sge_htable_store(htable ht, const void* key, 
*                           const void* data);
*
*     int sge_htable_lookup(htable ht, const void* key, 
*                           const void** data);
*
*     void sge_htable_delete(htable ht, const void* key);
*
*     void sge_htable_for_each(htable ht, 
*                              sge_htable_for_each_proc proc);
*
*  FUNCTION
*     This module provides a hash table implementation for Grid Engine.
*
*     Hash tables are used to have very fast access to objects stored in 
*     data structures like linked lists without having to traverse the 
*     whole list when searching a specific element.
*
*     An element in a hash table is characterized by a unique key.
*
*     This hash table implementation dynamically adjusts the size of the
*     hash table when necessary.
*
*  NOTES
*     MT-NOTE: This module is MT safe.
*
*  SEE ALSO
*     cull/htable/--Introduction
******************************************************************************/
typedef struct _Bucket {        /* Stores one entry. */
    const void* key;
    const void* data;
    struct _Bucket *next;
} Bucket;

typedef struct _htable_rec {
    Bucket **table;           /* Pointer to array of hash entries. */
    long size;                /* log2 of the size */
    long mask;                /* Current size of hash table minus 1. */
    long numentries;          /* Number of entries currently in table. */
    const void *(*dup_func)(const void *); /* pointer to function duplicating key */
    int (*hash_func)(const void *);        /* pointer to hash function */
    int (*compare_func)(const void *, const void *);   /* pointer to compare function */
} htable_rec;

#define HASH_RESIZE_UP_THRESHOLD 0
#define HASH_RESIZE_DOWN_THRESHOLD 1

/****** uti/htable/sge_htable_resize() ******************************************
*  NAME
*     sge_htable_resize() -- Resize the hash table
*
*  SYNOPSIS
*     static void sge_htable_resize(htable ht, int grow) 
*
*  FUNCTION
*     Hash tables are dynamically resized if necessary.
*     If the number of elements in a has table becomes too big, the hash
*     algorithm can no longer provide efficient access to the stored 
*     objects. On the other hand, storing only a few elements in a much 
*     too big hash table wastes memory. Therefore the whole table can 
*     be resized. If the hash table has to grow, it is doubled in size.
*     If it has to be shrunk, it is halfed in size.
*
*     Resizing implies rehashing all stored objects.
*
*  INPUTS
*     htable ht             - the hashtable  to resize 
*     int grow              - true or false
*           true  = double size of the table,
*           false = shrink table to half the size
*
*  NOTE
*     If the system is running in log_level log_debug, statistics is 
*     output before and after resizing the hash table, along with timing 
*     information.
*
*  SEE ALSO
*     uti/htable/sge_htable_statistics()
******************************************************************************/

static void sge_htable_resize(htable ht, int grow)    
{
   Bucket **otable;
   int otablesize;
   Bucket *bucket, *next, **head;
   int i;
   
#ifdef SGE_USE_PROFILING
   clock_t start = 0;
#endif   
   char buffer[1024];
   dstring buffer_wrapper;

   DENTER_(BASIS_LAYER, "sge_htable_resize");

   sge_dstring_init(&buffer_wrapper, buffer, sizeof(buffer));

#ifdef SGE_USE_PROFILING
   if(prof_is_active(SGE_PROF_HT_RESIZE) && log_state_get_log_level() >= LOG_DEBUG) {
      struct tms t_buf;
      DEBUG((SGE_EVENT, "hash stats before resizing: %s",
             sge_htable_statistics(ht, &buffer_wrapper)));
      start = times(&t_buf);
   } 
#endif   

   otable = ht->table;
   otablesize = 1 << ht->size;

   if (grow) {
      ht->size++;
   } else if (ht->size > 2) {
      ht->size--;
   } else {
      DRETURN_VOID_;
   }   
    
   ht->table = (Bucket **) calloc(1<<ht->size, sizeof(Bucket *));
   ht->mask = (1<<ht->size) - 1;

   for(i=0; i < otablesize; i++) {
      for(bucket = otable[i]; bucket; bucket = next) {
         next = bucket->next;
         head = &(ht->table[ht->hash_func(bucket->key) & ht->mask]);
         bucket->next = *head;
         *head = bucket;
      }
   }
   sge_free(&otable);

#ifdef SGE_USE_PROFILING
   if(prof_is_active(SGE_PROF_HT_RESIZE) && log_state_get_log_level() >= LOG_DEBUG) {
      struct tms t_buf;
      DEBUG((SGE_EVENT, "resizing of hash table took %.3fs",
             (times(&t_buf) - start) * 1.0 / sysconf(_SC_CLK_TCK)));
      DEBUG((SGE_EVENT, "hash stats after resizing: %s",
             sge_htable_statistics(ht, &buffer_wrapper)));
   }
#endif   
   
   DRETURN_VOID_;
}

/****** uti/htable/sge_htable_create() ****************************************
*  NAME
*     sge_htable_create() -- Create a new hash table
*
*  SYNOPSIS
*     htable sge_htable_create(int size, 
*                    int (*hash_func)(const void *),
*                    int (*compare_func)(const void *, const void *)
*
*  FUNCTION
*     Creates an empty hash table and initializes its data structures.
*
*  INPUTS
*     int size            - Initial table size will be 2^n 
*     int (*hash_func)    - pointer to hash function 
*     int (*compare_func) - pointer to compare function 
*
*  RESULT
*     htable - the created hash table
*
*  EXAMPLE
*     htable MyHashTable = sge_htable_create(5, hash_func_u_long32, 
*                                             hash_compare_u_long32);
******************************************************************************/
htable sge_htable_create(int size, 
                          const void *(*dup_func)(const void *), 
                          int (*hash_func)(const void *), 
                          int (*compare_func)(const void *, const void *))
{
    htable ht = (htable) malloc(sizeof(htable_rec));

    ht->size = size;
    ht->mask = (1<<size)-1;
    ht->table = (Bucket **)calloc(ht->mask + 1, sizeof(Bucket *));
    ht->numentries = 0;
    ht->dup_func = dup_func;
    ht->hash_func = hash_func;
    ht->compare_func = compare_func;
    return ht;
}

/****** uti/htable/sge_htable_destroy() ***************************************
*  NAME
*     sge_htable_destroy() -- Destroy a hash table
*
*  SYNOPSIS
*     void sge_htable_destroy(htable ht) 
*
*  FUNCTION
*     Destroys a hash table and frees all used memory.
*
*  INPUTS
*     htable ht - the hash table to destroy
*
*  NOTES
*     The objecs managed by the hash table are not destroyed and have 
*     to be handled separately.
******************************************************************************/
void sge_htable_destroy(htable ht)
{
    int i;
    Bucket *bucket, *next;

    for (i = 0; i < ht->mask+1; i++) {
        for (bucket = ht->table[i]; bucket; bucket = next) {
            next = bucket->next;
            if(bucket->key != NULL) {
               sge_free(&(bucket->key));
            }
            sge_free(&bucket);
        }
    }
    sge_free(&(ht->table));
    sge_free(&ht);
}

/****** uti/htable/sge_htable_for_each() **************************************
*  NAME
*     sge_htable_for_each() -- Apply an action on all elements
*
*  SYNOPSIS
*     void sge_htable_for_each(htable table, 
*                              sge_htable_for_each_proc proc) 
*
*  FUNCTION
*     Calls a certain function for all elements in a hash table.
*
*  INPUTS
*     htable table                  - the hash table
*     sge_htable_for_each_proc proc - func to call for each element 
******************************************************************************/
void sge_htable_for_each(htable table, sge_htable_for_each_proc proc)
{
    int i;
    Bucket *bucket;

    for(i=0; i < table->mask+1; i++) {
        for (bucket = table->table[i]; bucket; bucket = bucket->next)
            (*proc)(table, bucket->key, &bucket->data);
    }
}    

/****** uti/htable/sge_htable_store() *******************************************
*  NAME
*     sge_htable_store() -- Store a new element in a hash table
*
*  SYNOPSIS
*     void sge_htable_store(htable table, const void* key, 
*                           const void* data) 
*
*  FUNCTION
*     Stores a new element in a hash table.
*     If there already exists an element with the same key in the table, 
*     it will be replaced by the new element.
*
*     If the number of elements in the table exceeds the table size, the
*     hash table will be resized.
*
*  INPUTS
*     htable table     - table to hold the new element
*     const void* key  - unique key 
*     const void* data - data to store, usually a pointer to an object
*
*  SEE ALSO
*     uti/htable/sge_htable_resize()
******************************************************************************/
void sge_htable_store(htable table, const void* key, const void* data)
{
    Bucket **head;
    Bucket *bucket;

    head = &(table->table[table->hash_func(key) & table->mask]);
    for (bucket = *head; bucket; bucket = bucket->next) {
        if(table->compare_func(bucket->key, key) == 0) {
            bucket->data = data;
            return;
        }
    }
    bucket = (Bucket *)malloc(sizeof(Bucket));
    bucket->key = table->dup_func(key);
    bucket->data = data;
    bucket->next = *head;
    *head = bucket;
    table->numentries++;
    if (table->numentries > (table->mask << HASH_RESIZE_UP_THRESHOLD))
        sge_htable_resize(table, True);
}

/****** uti/htable/sge_htable_lookup() ***************************************
*  NAME
*     sge_htable_lookup() -- search for an element
*
*  SYNOPSIS
*     int sge_htable_lookup(htable table, const void* key, 
*                         const void** data) 
*
*  FUNCTION
*     Search for a certain object characterized by a unique key in the 
*     hash table.
*     If an element can be found, it is returned in data.
*
*  INPUTS
*     htable table      - the table to search
*     const void* key   - unique key to search
*     const void** data - object if found, else NULL
*
*  RESULT
*     int - true, when an object was found, else false
******************************************************************************/
int sge_htable_lookup(htable table, const void* key, const void** data)
{
    Bucket *bucket;

    for (bucket = table->table[table->hash_func(key) & table->mask]; 
         bucket; 
         bucket = bucket->next) {
        if(table->compare_func(bucket->key, key) == 0) {
            *data = (void *)bucket->data;
            return True;
        }
    }
    return False;
}

/****** uti/htable/sge_htable_delete() ****************************************
*  NAME
*     sge_htable_delete() -- Delete an element in a hash table
*
*  SYNOPSIS
*     void sge_htable_delete(htable table, const void* key) 
*
*  FUNCTION
*     Deletes an element in a hash table.
*     If the number of elements falls below a certain threshold
*     (half the size of the hash table), the hash table is resized 
*     (shrunk).
*
*  INPUTS
*     htable table    - hash table that contains the element
*     const void* key - key of the element to delete
*
*  NOTES
*     Only deletes the entry in the hash table. The object itself 
*     is not deleted.
*
*  SEE ALSO
*     uti/htable/sge_htable_resize()
******************************************************************************/
void sge_htable_delete(htable table, const void* key)
{
    Bucket *bucket, **prev;

    for (prev = &(table->table[table->hash_func(key) & table->mask]); 
         (bucket = *prev); 
         prev = &bucket->next) {
        if (table->compare_func(bucket->key, key) == 0) {
            *prev = bucket->next;
            if (bucket->key != NULL) {
               sge_free(&(bucket->key));
            }
            sge_free(&bucket);
            table->numentries--;
            if (table->numentries < (table->mask >> HASH_RESIZE_DOWN_THRESHOLD))
                sge_htable_resize(table, False);
            return;
        }
    }
}

/****** uti/htable/sge_htable_statistics() ************************************
*  NAME
*     sge_htable_statistics() -- Get some statistics for a hash table
*
*  SYNOPSIS
*     const char* sge_htable_statistics(htable ht) 
*
*  FUNCTION
*     Returns a constant string containing statistics for a hash table 
*     in the following format:
*     "size: %ld, %ld entries, chains: %ld empty, %ld max, %.1f avg"
*     size is the size of the hash table (number of hash chains)
*     entries is the number of objects stored in the hash table
*     Information about hash chains:
*        empty is the number of empty hash chains
*        max is the maximum number of objects in a hash chain
*        avg is the average number of objects for all occupied 
*        hash chains
*     
*     The string returned is a static buffer, subsequent calls to the 
*     function will overwrite this buffer.
*
*  INPUTS
*     htable ht - Hash table for which statistics shall be generated
*     dstring *buffer - buffer to be provided by caller
*
*  RESULT
*     const char* - the string described above
******************************************************************************/
const char *sge_htable_statistics(htable ht, dstring *buffer)
{
   long size  = 0;
   long empty = 0;
   long max   = 0;
   long i;
 
   /* count empty hash chains and maximum chain length */
   size = 1 << ht->size;
   
   for(i = 0; i < size; i++) {
      long count = 0;
      if(ht->table[i] == NULL) {
         empty++;
      } else {
         Bucket *b = ht->table[i];
         do {
            count++;
         } while((b = b->next) != NULL);

         if(count > max) {
            max = count;
         }
      }
   }

   sge_dstring_sprintf_append(buffer, 
           "size: %ld, %ld entries, chains: %ld empty, %ld max, %.1f avg", 
           size, ht->numentries,
           empty, max, 
           (size - empty) > 0 ? ht->numentries * 1.0 / (size - empty) : 0);

   return sge_dstring_get_string(buffer);
}

/****** uti/htable/-Dup-Functions() *******************************************
*  NAME
*     dup_func_<type>() -- function duplicate the key
*
*  SYNOPSIS
*     const void *dup_func_<type>(const void *key) 
*
*  FUNCTION
*     The hash table cannot rely on the key data to remain valid over 
*     the programs execution time. Therefore copies of keys are stored 
*     in the bucket object. To allow duplication of keys with types 
*     unknown to the hash table implementation, a duplication function 
*     must be specified when a hash table is created.
*
*  INPUTS
*     const void *key - pointer to the key to duplicate
*
*  RESULT
*     const void * - the duplicated key
*
*  NOTES
*     The following data types are provided with this module:
*        - strings (char *)
*        - u_long32
*
*  SEE ALSO
*     uti/htable/sge_htable_create()
******************************************************************************/
const void *dup_func_u_long32(const void *key) 
{
   u_long32 *dup_key = NULL;
   u_long32 *cast = (u_long32 *)key;

   if((dup_key = (u_long32 *)malloc(sizeof(u_long32))) != NULL) {
      *dup_key = *cast;
   }

   return dup_key;
}

const void *dup_func_u_long64(const void *key) 
{
   u_long64 *dup_key = NULL;
   u_long64 *cast = (u_long64 *)key;

   if((dup_key = (u_long64 *)malloc(sizeof(u_long64))) != NULL) {
      *dup_key = *cast;
   }

   return dup_key;
}

const void *dup_func_long(const void *key)
{
   long *dup_key  = NULL;
   long *cast = (long*)key;

   if((dup_key = (long*) malloc(sizeof(long))) != NULL) {
      *dup_key = *cast;
   }
   return dup_key;
}

const void *dup_func_pointer(const void *key)
{
   char **dup_key  = NULL;
   char **cast = (char **)key;

   if((dup_key = (char **) malloc(sizeof(char *))) != NULL) {
      *dup_key = *cast;
   }
   return dup_key;
}

const void *dup_func_string(const void *key)
{
   return strdup((const char *)key);
}   


/****** uti/htable/-Hash-Functions() ******************************************
*  NAME
*     hash_func_<type>() -- Hash functions for selected data types
*
*  SYNOPSIS
*     int hash_func_<type>(const void *key) 
*
*  FUNCTION
*     Depending on the data type of the hash key, different hash 
*     functions have to be used.
*     Which hash function to use has to be specified when creating a new
*     hash table.
*
*  INPUTS
*     const void *key - pointer to key for which to compute a hash value
*
*  RESULT
*     int - the hash value
*
*  NOTES
*     The following data types are provided with this module:
*        - strings (char *)
*        - u_long32
*
*  SEE ALSO
*     uti/htable/sge_htable_create()
******************************************************************************/
int hash_func_u_long32(const void *key) 
{
   u_long32 *cast = (u_long32 *)key;
   return (int)*cast;
}

int hash_func_u_long64(const void *key) 
{
   u_long64 *cast = (u_long64 *)key;
   return (int)*cast;
}

int hash_func_long(const void *key)
{
   long *cast = (long*)key;
   return (int)*cast;
}

int hash_func_pointer(const void *key)
{
   char **cast = (char **)key;
   long tmp = (long)*cast;
   tmp = tmp >> 7;
/*    printf("====> %p -> %lx -> %x\n", cast, tmp, (int)tmp); */
   return (int)tmp;
}

int hash_func_string(const void *key)
{
   int hash = 0;
   const char *c = key;

   if(c != NULL) {
      do {
         hash += (hash << 3) + *c;
      } while (*c++ != 0);
   }

   return hash;
}   

/****** uti/htable/-Compare-Functions() ***************************************
*  NAME
*     compare_func_<type>() -- Compare functions for selected data types
*
*  SYNOPSIS
*     int HashCompare_<type>(const void *a, const void *b) 
*
*  FUNCTION
*     Depending on the data type of the hash key, different functions
*     have to be used to compare two elements.
*     Which compare function to use has to be specified when creating 
*     a new hash table. Syntax and return value are similar to strcmp.
*
*  INPUTS
*     const void *a - pointer to first element
*     const void *b - pointer to second element
*
*  RESULT
*     int - 0, if the elements are equal
*           > 0, if the first element is bigger than the second
*           < 0, if the first element is smaller than the second
*
*  NOTES
*     The following data types are provided with this module:
*        - strings (char *)
*        - u_long32
*
*  SEE ALSO
*     uti/htable/sge_htable_create()
******************************************************************************/
int hash_compare_u_long32(const void *a, const void *b)
{
   u_long32 *cast_a = (u_long32 *)a;
   u_long32 *cast_b = (u_long32 *)b;
   return *cast_a - *cast_b;
}

int hash_compare_u_long64(const void *a, const void *b)
{
   u_long64 *cast_a = (u_long64 *)a;
   u_long64 *cast_b = (u_long64 *)b;
   return *cast_a - *cast_b;
}

int hash_compare_long(const void *a, const void *b) 
{
   long *cast_a = (long*)a;
   long *cast_b = (long*)b;
   return (int)(*cast_a - *cast_b);
}

int hash_compare_pointer(const void *a, const void *b) 
{
   char **cast_a = (char **)a;
   char **cast_b = (char **)b;
/* printf("++++> %p - %p\n", *cast_a, *cast_b); */
   if (*cast_a != *cast_b) {
      return 1;
   } else {
      return 0;
   }
}

int hash_compare_string(const void *a, const void *b)
{
   return strcmp((const char *)a, (const char *)b);
}

int hash_compute_size(int number_of_elem)
{
   int size = 0;
   while (number_of_elem > 0) {
      size++;
      number_of_elem = number_of_elem >> 1;
   }

   return size;
}

