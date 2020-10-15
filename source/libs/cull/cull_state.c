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

#include "cull_state.h"

#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "uti/sge_stdlib.h"

#include "cull/pack.h"


/* struct to store ALL state information of cull lib */
typedef struct {
   int               lerrno;            /* cull errno               */
   char              noinit[50];        /* cull error buffer        */
   const lSortOrder* global_sort_order; /* qsort() by-pass argument */
   const lNameSpace* name_space;        /* name vector              */
} cull_state_t;

static pthread_once_t cull_once = PTHREAD_ONCE_INIT;
static pthread_key_t cull_state_key;  

static void          cull_once_init(void);
static void          cull_state_destroy(void* theState);
static cull_state_t* cull_state_getspecific(pthread_key_t aKey);
static void          cull_state_init(cull_state_t *theState);


/****** cull/state/cull_state_get_????() ************************************
*  NAME
*     cull_state_get_????() - read access to cull state.
*
*  FUNCTION
*     Provides access to thread local storage.
*
******************************************************************************/
int cull_state_get_lerrno(void)
{
   cull_state_t *cull_state = NULL;

   pthread_once(&cull_once, cull_once_init);
 
   cull_state = cull_state_getspecific(cull_state_key);

   return cull_state->lerrno;
}

const char *cull_state_get_noinit(void)
{
   cull_state_t *cull_state = NULL;

   pthread_once(&cull_once, cull_once_init);
 
   cull_state = cull_state_getspecific(cull_state_key);

   return cull_state->noinit;
}

const lSortOrder *cull_state_get_global_sort_order(void)
{
   cull_state_t *cull_state = NULL;

   pthread_once(&cull_once, cull_once_init);
 
   cull_state = cull_state_getspecific(cull_state_key);

   return cull_state->global_sort_order;
}

const lNameSpace *cull_state_get_name_space(void)
{
   cull_state_t *cull_state = NULL;

   pthread_once(&cull_once, cull_once_init);
 
   cull_state = cull_state_getspecific(cull_state_key);

   return cull_state->name_space;
}

/****** cull/list/cull_state_set_????() ************************************
*  NAME
*     cull_state_set_????() - write access to cull state.
*
*  FUNCTION
*     Provides access to thread local storage.
*
******************************************************************************/
void cull_state_set_lerrno( int i)
{
   cull_state_t *cull_state = NULL;

   pthread_once(&cull_once, cull_once_init);
 
   cull_state = cull_state_getspecific(cull_state_key);

   cull_state->lerrno = i;

   return;
}

void cull_state_set_noinit( char *s)
{
   cull_state_t *cull_state = NULL;

   pthread_once(&cull_once, cull_once_init);
 
   cull_state = cull_state_getspecific(cull_state_key);

   strcpy(cull_state->noinit, s);

   return;
}

void cull_state_set_global_sort_order( const lSortOrder *so)
{
   cull_state_t *cull_state = NULL;

   pthread_once(&cull_once, cull_once_init);
 
   cull_state = cull_state_getspecific(cull_state_key);

   cull_state->global_sort_order = so;

   return;
}

void cull_state_set_name_space(const lNameSpace  *ns)
{
   cull_state_t *cull_state = NULL;

   pthread_once(&cull_once, cull_once_init);
 
   cull_state = cull_state_getspecific(cull_state_key);

   cull_state->name_space = ns;

   return;
}

/****** cull/state/cull_once_init() ********************************************
*  NAME
*     cull_once_init() -- One-time CULL initialization.
*
*  SYNOPSIS
*     static cull_once_init(void) 
*
*  FUNCTION
*     Create access key for thread local storage. Register cleanup function.
*
*     This function must be called exactly once.
*
*  INPUTS
*     void - none
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: cull_once_init() is MT safe. 
*
*******************************************************************************/
static void cull_once_init(void)
{
   pthread_key_create(&cull_state_key, cull_state_destroy);
   return;
} /* cull_once_init() */

/****** cull/state/cull_state_destroy() ****************************************
*  NAME
*     cull_state_destroy() -- Free thread local storage
*
*  SYNOPSIS
*     static void cull_state_destroy(void* theState) 
*
*  FUNCTION
*     Free thread local storage.
*
*  INPUTS
*     void* theState - Pointer to memory which should be freed.
*
*  RESULT
*     static void - none
*
*  NOTES
*     MT-NOTE: cull_state_destroy() is MT safe.
*
*******************************************************************************/
static void cull_state_destroy(void* theState)
{
   sge_free(&theState);
}

/****** cull/state/cull_state_getspecific() ************************************
*  NAME
*     cull_state_getspecific() -- Get thread local cull state 
*
*  SYNOPSIS
*     static cull_state_t* cull_state_getspecific(pthread_key_t aKey) 
*
*  FUNCTION
*     Return thread local cull state. 
*
*     If a given thread does call this function for the first time, no thread
*     local cull state is available for this particular thread. In this case the
*     thread local cull state is allocated and set.
*
*  INPUTS
*     pthread_key_t aKey - Key for thread local cull state 
*
*  RESULT
*     static cull_state_t* - Pointer to thread local cull state
*
*  NOTES
*     MT-NOTE: cull_state_getspecific() is MT safe 
*
*******************************************************************************/
static cull_state_t* cull_state_getspecific(pthread_key_t aKey)
{
   cull_state_t *cull_state = pthread_getspecific(aKey);

   if (cull_state == NULL) { 
      int res = EINVAL;

      cull_state = (cull_state_t*)sge_malloc(sizeof(cull_state_t));
      cull_state_init(cull_state);
      res = pthread_setspecific(cull_state_key, (const void*)cull_state);

      if (0 != res) {
         fprintf(stderr, "pthread_set_specific(%s) failed: %s\n", "cull_state_getspecific", strerror(res));
         abort();
      }
   }
   
   return cull_state;
} /* cull_state_getspecific() */

/****** cull/state/cull_state_init() *******************************************
*  NAME
*     cull_state_init() -- Initialize CULL state.
*
*  SYNOPSIS
*     static void cull_state_init(cull_state_t *theState) 
*
*  FUNCTION
*     Initialize CULL state.
*
*  INPUTS
*     struct cull_state_t* theState - Pointer to CULL state structure.
*
*  RESULT
*     static void - none
*
*  NOTES
*     MT-NOTE: cull_state_init() is MT safe. 
*
*******************************************************************************/
static void cull_state_init(cull_state_t *theState)
{
   theState->lerrno = 0;
   theState->noinit[0] = '\0';
   theState->global_sort_order = NULL;
   theState->name_space = NULL;
}
