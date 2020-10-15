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
 *   Portions of this code are Copyright 2011 Univa Inc.
 *   Copyright (C) 2011 Dave Love, University of Liverpool
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "uti/sge_binding_hlp.h"

#include <pthread.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"

#include "sgeobj/sge_binding.h"
#include "sgeobj/sge_answer.h"

#include "binding_support.h"
#include "msg_common.h"

/*
 * these sockets cores or threads are currently in use from SGE
 * access them via getExecdTopologyInUse() because of initialization
 */
static char* logical_used_topology = NULL;

static int logical_used_topology_length = 0;

/* creates a string with the topology used from a single job */
static bool create_topology_used_per_job(char** accounted_topology,
               int* accounted_topology_length, char* logical_used_topology,
               char* used_topo_with_job, int logical_used_topology_length);

static bool get_free_sockets(const char* topology, const int topology_length,
               int** sockets, int* sockets_size);

static int account_cores_on_socket(char** topology, const int topology_length,
               const int socket_number, const int cores_needed, int** list_of_sockets,
               int* list_of_sockets_size, int** list_of_cores, int* list_of_cores_size);

static bool get_socket_with_most_free_cores(const char* topology, const int topology_length,
               int* socket_number);

static bool account_all_threads_after_core(char** topology, const int core_pos);


#if HAVE_HWLOC
static int initialized = 0;

hwloc_topology_t sge_hwloc_topology = 0;
#endif

/* Intended to be called at the start of the program, with topology
   shared between any threads.  */
void
init_topology(void)
{
#if HAVE_HWLOC
  initialized = 1;
  if (hwloc_topology_init(&sge_hwloc_topology) != 0 ||
      hwloc_topology_load(sge_hwloc_topology) != 0)
     sge_hwloc_topology = 0;
#endif
  return;
}

#if HAVE_HWLOC
static int get_total_number_of(hwloc_obj_type_t obj_type) {
  int number = 0;

  if (!has_topology_information()) return 0;
  number = hwloc_get_nbobjs_by_type (sge_hwloc_topology, obj_type);
  if (-1 == number) {
    /* Only for things like L1, L2 caches, not the types we're
       interested in?  */
    hwloc_obj_t obj;
    number = 0;
    for (obj = hwloc_get_next_obj_by_type(sge_hwloc_topology, obj_type, NULL);
         NULL != obj;
         obj = hwloc_get_next_obj_by_type(sge_hwloc_topology, obj_type, obj))
      if (obj_type == obj->type) ++number;
  }
  return number;
}

static unsigned count_type_under(hwloc_obj_t top, hwloc_obj_type_t type)
{
   unsigned i, count = 0;
   if (!top) return 0;
   if (top->type == type) ++count;
   for (i = 0; i < top->arity; ++i)
      count += count_type_under(top->children[i], type);
    return count;
}
#endif

/****** binding_support/has_topology_information() *********************************
*  NAME
*     has_topology_information() -- Checks if current arch offers topology.
*
*  SYNOPSIS
*     bool has_topology_information()
*
*  FUNCTION
*     Checks if current architecture (on which this function is called)
*     offers processor topology information or not.
*
*  RESULT
*     bool - true if the arch offers topology information false if not
*
*  NOTES
*     MT-NOTE: has_topology_information() is not MT safe
*
*******************************************************************************/
bool has_topology_information(void)
{
#if HAVE_HWLOC
   const struct hwloc_topology_support *support;

   if (!initialized) init_topology();
   if (!sge_hwloc_topology) return false;
   support = hwloc_topology_get_support(sge_hwloc_topology);
   if (support->discovery->pu)
     return true;
#endif
   return false;
}


/****** binding_support/has_core_binding() *****************************************
*  NAME
*     has_core_binding() -- Check if core binding system call is supported.
*
*  SYNOPSIS
*     bool has_core_binding()
*
*  FUNCTION
*     Checks if core binding is supported on the machine or not. If it is
*     supported this does not mean that topology information (about socket
*     and core amount) is available (which is needed for internal functions
*     in order to perform a correct core binding).
*
*  RESULT
*     bool - True if core binding could be done. False if not.
*
*  NOTES
*     MT-NOTE: has_core_binding() is not MT safe
*
*******************************************************************************/
bool has_core_binding(void)
{
#if HAVE_HWLOC
   const struct hwloc_topology_support *support;

   if (!initialized) init_topology();
   if (!sge_hwloc_topology) return false;
   support = hwloc_topology_get_support(sge_hwloc_topology);
   if (support->cpubind->set_proc_cpubind) return true;
#endif
   return false;
}

/****** binding_support/get_total_number_of_threads() ***********************
*  NAME
*     get_total_number_of_threads() -- The total number of hw supported threads.
*
*  SYNOPSIS
*     int get_total_number_of_threads()
*
*  FUNCTION
*     Returns the total number of threads all CPUs on the host support.
*
*  RESULT
*     int - Total number of hardware supported threads the system supports.
*
*  NOTES
*     MT-NOTE: get_total_number_of_threads() is MT safe
*
*******************************************************************************/
int get_total_number_of_threads(void) {
#if HAVE_HWLOC
   return get_total_number_of(HWLOC_OBJ_PU);
#else
   return 0;
#endif
}

/****** binding_support/get_total_number_of_cores() ********************************
*  NAME
*     get_total_number_of_cores() -- Fetches the total number of cores on system.
*
*  SYNOPSIS
*     int get_total_number_of_cores()
*
*  FUNCTION
*     Returns the total number of cores.
*
*  RESULT
*     int - Total number of cores installed on the system.
*
*  NOTES
*     MT-NOTE: get_total_number_of_cores() is MT safe
*
*******************************************************************************/
int get_total_number_of_cores(void)
{
#if HAVE_HWLOC
   return get_total_number_of(HWLOC_OBJ_CORE);
#else
   return 0;
#endif
}


/****** binding_support/get_number_of_cores() **************************************
*  NAME
*     get_number_of_cores() -- Get number of cores per socket.
*
*  SYNOPSIS
*     int get_number_of_cores(int socket_number)
*
*  FUNCTION
*     Returns the number of cores for a specific socket.
*
*  INPUTS
*     int socket_number - Logical socket number starting at 0.
*
*  RESULT
*     int - Amount of cores for the given socket or 0.
*
*  NOTES
*     MT-NOTE: get_number_of_cores() is MT safe
*
*******************************************************************************/
int get_number_of_cores(int socket_number)
{
#if HAVE_HWLOC
  hwloc_obj_t socket = hwloc_get_obj_by_type(sge_hwloc_topology,
                                             HWLOC_OBJ_SOCKET, socket_number);
  if (socket)
    return count_type_under(socket, HWLOC_OBJ_CORE);
  else
#endif
    return 0;
}

/****** binding_support/get_number_of_threads() **************************************
*  NAME
*     get_number_of_threads() -- Get number of threads a specific core supports.
*
*  SYNOPSIS
*     int get_number_of_threads(int socket_number, int core_number)
*
*  FUNCTION
*     Returns the number of threads a specific core supports.
*
*  INPUTS
*     int socket_number - Logical socket number starting at 0.
*     int core_number   - Logical core number on socket starting at 0.
*
*  RESULT
*     int - Amount of threads a specific core supports.
*
*  NOTES
*     MT-NOTE: get_number_of_threads() is MT safe
*
*******************************************************************************/
int get_number_of_threads(int socket_number, int core_number) {
#if HAVE_HWLOC
   hwloc_obj_t core =
     hwloc_get_obj_below_by_type(sge_hwloc_topology, HWLOC_OBJ_SOCKET,
                                 socket_number, HWLOC_OBJ_CORE, core_number);
   return count_type_under(core, HWLOC_OBJ_PU);
#else
   return 0;
#endif
}


/****** binding_support/get_number_of_sockets() ************************************
*  NAME
*     get_number_of_sockets() -- Get the number of available sockets.
*
*  SYNOPSIS
*     int get_number_of_sockets()
*
*  FUNCTION
*     Returns the number of sockets available on this system.
*
*  RESULT
*     int - The number of available sockets on system. 0 in case of
*                  of an error.
*
*  NOTES
*     MT-NOTE: get_number_of_sockets() is not MT safe
*
*******************************************************************************/
int get_number_of_sockets(void)
{
#if HAVE_HWLOC
   return get_total_number_of(HWLOC_OBJ_SOCKET);
#else
   return 0;
#endif
}

/****** binding_support/get_processor_ids() ******************************
*  NAME
*     get_processor_ids() -- Get internal processor ids for a specific core.
*
*  SYNOPSIS
*     bool get_processor_ids(int socket_number, int core_number, int**
*     proc_ids, int* amount)
*
*  FUNCTION
*     Get the internal processor ids for a given core (specified by a socket,
*     core pair).
*
*  INPUTS
*     int socket_number - Logical socket number (starting at 0 without holes)
*     int core_number   - Logical core number on the socket (starting at 0 without holes)
*
*  OUTPUTS
*     int** proc_ids    - Array of internal processor ids.
*     int* amount       - Size of the proc_ids array.
*
*  RESULT
*     bool - Returns true when processor ids where found otherwise false.
*
*  NOTES
*     MT-NOTE: get_processor_ids() is MT safe
*
*******************************************************************************/
bool get_processor_ids(int socket_number, int core_number, int** proc_ids, int* amount)
{
#if HAVE_HWLOC
   int i, count;
   hwloc_obj_t pu, parent;
   struct hwloc_obj **children;
   hwloc_obj_t core =
      hwloc_get_obj_below_by_type(sge_hwloc_topology, HWLOC_OBJ_SOCKET,
                                  socket_number, HWLOC_OBJ_CORE, core_number);
   if (core)
      pu = hwloc_get_obj_below_by_type(sge_hwloc_topology, HWLOC_OBJ_CORE,
                                       core->logical_index, HWLOC_OBJ_PU, 0);
   else
      return false;
   parent = pu->parent;
   count = parent->arity;
   if (count <= 0) return false;
   children = parent->children;
   (*amount) = count;
   (*proc_ids) = (int *) sge_malloc(count * sizeof(int));
   for (i = 0; i < count; i++)
      (*proc_ids)[i] = children[i]->os_index;
   return true;
#else
   return false;
#endif
}

/****** binding_support/get_topology() ***********************************
*  NAME
*     get_topology() -- Creates the topology string for the current host.
*
*  SYNOPSIS
*     bool get_topology(char** topology, int* length)
*
*  FUNCTION
*     Creates the topology string for the current host. When created,
*     it has to be freed from outside.
*
*  INPUTS
*     char** topology - The topology string for the current host.
*     int* length     - The length of the topology string.
*
*  RESULT
*     bool - when true the topology string could be generated (and memory
*            is allocated otherwise false
*
*  NOTES
*     MT-NOTE: get_topology() is MT safe
*
*******************************************************************************/
bool get_topology(char** topology, int* length)
{
   bool success = false;

   if (HAVE_HWLOC) {
   /* initialize length of topology string */
   (*length) = 0;

   /* check if topology is supported via hwloc */
   if (has_topology_information()) {
      int num_sockets;

      /* topology string */
      dstring d_topology = DSTRING_INIT;

      /* build the topology string */
      if ((num_sockets = get_number_of_sockets())) {
         int num_cores, ctr_cores, ctr_sockets, ctr_threads;
         char* s = "S"; /* socket */
         char* c = "C"; /* core   */
         char* t = "T"; /* thread */

         for (ctr_sockets = 0; ctr_sockets < num_sockets; ctr_sockets++) {

            /* append new socket */
            sge_dstring_append_char(&d_topology, *s);
            (*length)++;

            /* for each socket get the number of cores */
            if ((num_cores = get_number_of_cores(ctr_sockets))) {
               /* for thread counting */
               int* proc_ids = NULL;
               int number_of_threads = 0;

               /* check each core */
               for (ctr_cores = 0; ctr_cores < num_cores; ctr_cores++) {
                  sge_dstring_append_char(&d_topology, *c);
                  (*length)++;
                  /* check if the core has threads */
                  if (get_processor_ids(ctr_sockets, ctr_cores, &proc_ids,
                                        &number_of_threads)
                        && number_of_threads > 1) {
                     /* print the threads */
                     for (ctr_threads = 0; ctr_threads < number_of_threads;
                          ctr_threads++) {
                        sge_dstring_append_char(&d_topology, *t);
                        (*length)++;
                     }
                  }
                  sge_free(&proc_ids);
               }
            }
         } /* for each socket */

         if ((*length) != 0) {
            /* convert d_topolgy into topology */
            (*length)++; /* we need `\0` at the end */

            /* copy element */
            (*topology) = sge_strdup(NULL, sge_dstring_get_string(&d_topology));
            success = true;
         }

         sge_dstring_free(&d_topology);
      }

   }
   }
   return success;
}

/****** sge_binding/getExecdTopologyInUse() ************************************
*  NAME
*     getExecdTopologyInUse() -- Creates a string which represents the used topology.
*
*  SYNOPSIS
*     bool getExecdTopologyInUse(char** topology)
*
*  FUNCTION
*
*     Checks all jobs (with going through active jobs directories) and their
*     usage of the topology (binding). Afterwards global "logical_used_topology"
*     string is up to date (which is also updated when a job ends and starts) and
*     a copy is made available for the caller.
*
*     Note: The memory is allocated within this function and
*           has to be freed from the caller afterwards.
*  INPUTS
*     char** topology - out: the current topology in use by jobs
*
*  RESULT
*     bool - true if the "topology in use" string could be created
*
*  NOTES
*     MT-NOTE: getExecdTopologyInUse() is not MT safe
*******************************************************************************/
bool get_execd_topology_in_use(char** topology)
{
   bool retval = false;

   /* topology must be a NULL pointer */
   if ((*topology) != NULL) {
      return false;
   }

   if (logical_used_topology_length == 0 || logical_used_topology == NULL) {
      /* initialize without any usage */
      get_topology(&logical_used_topology, &logical_used_topology_length);
   }

   if (logical_used_topology_length > 0) {
      /* copy the string */
      (*topology) = sge_strdup(NULL, logical_used_topology);
      retval = true;
   }

   return retval;
}

/* gets the positions in the topology string from a given <socket>,<core> pair */
static int get_position_in_topology(const int socket, const int core, const char* topology,
   const int topology_length);

/* accounts all occupied resources given by a topology string into another one */
static bool account_job_on_topology(char** topology, const int topology_length,
   const char* job, const int job_length);

/* DG TODO length should be an output */
static bool is_starting_point(const char* topo, const int length, const int pos,
   const int amount, const int stepsize, char** topo_account);


/****** sge_binding/account_job() **********************************************
*  NAME
*     account_job() -- Accounts core binding from a job on host global topology.
*
*  SYNOPSIS
*     bool account_job(char* job_topology)
*
*  FUNCTION
*      Accounts core binding from a job on host global topology.
*
*  INPUTS
*     char* job_topology - Topology used from core binding.
*
*  RESULT
*     bool - true when successful otherwise false
*
*  NOTES
*     MT-NOTE: account_job() is not MT safe
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
bool account_job(const char* job_topology)
{
   if (logical_used_topology_length == 0 || logical_used_topology == NULL) {

      /* initialize without any usage */
      if (!get_topology(&logical_used_topology,
                        &logical_used_topology_length))
         return false;
   }
   return account_job_on_topology(&logical_used_topology,
                                  strlen(logical_used_topology),
                                  job_topology, strlen(job_topology));
}

/****** sge_binding/account_job_on_topology() **********************************
*  NAME
*     account_job_on_topology() -- Marks occupied resources.
*
*  SYNOPSIS
*     static bool account_job_on_topology(char** topology, int*
*     topology_length, const char* job, const int job_length)
*
*  FUNCTION
*     Marks occupied resources from one topology string (job), which
*     is usually a job, into another topology string (topology) which
*     is usually the execution daemon local topology string.
*
*  INPUTS
*     char** topology      - (in/out) topology on which the accounting is done
*     int* topology_length - (in)  length of the topology stirng
*     const char* job      - (in) topology string from the job
*     const int job_length - (in) length of the topology string from the job
*
*  RESULT
*     static bool - true in case of success
*
*  NOTES
*     MT-NOTE: account_job_on_topology() is MT safe
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static bool account_job_on_topology(char** topology, const int topology_length,
   const char* job, const int job_length)
{
   int i;

   /* parameter validation */
   if (topology_length != job_length ||  job_length <= 0
      || topology == NULL || (*topology) == NULL || job == NULL) {
      return false;
   }

   /* go through topology and account */
   for (i = 0; i < job_length && job[i] != '\0'; i++) {
      if (job[i] == 'c') {
         (*topology)[i] = 'c';
      } else if (job[i] == 's') {
         (*topology)[i] = 's';
      } else if (job[i] == 't') {
         (*topology)[i] = 't';
      }
   }

   return true;
}



/****** sge_binding/binding_explicit_check_and_account() ***********************
*  NAME
*     binding_explicit_check_and_account() -- Checks if a job can be bound.
*
*  SYNOPSIS
*     bool binding_explicit_check_and_account(const int* list_of_sockets, const
*     int samount, const int** list_of_cores, const int score, char**
*     topo_used_by_job, int* topo_used_by_job_length)
*
*  FUNCTION
*     Checks if the job can bind to the given <socket>,<core> pairs.
*     If so, these cores are marked as used and true is returned. Also a
*     topology string is returned where all cores consumed by the job are
*     marked with lower case letters.
*
*  INPUTS
*     const int* list_of_sockets   - List of sockets to be used
*     const int samount            - Size of list_of_sockets
*     const int** list_of_cores    - List of cores (on sockets) to be used
*     const int score              - Size of list_of_cores
*
*  OUTPUTS
*     char** topo_used_by_job      -  Topology with resources job consumes marked.
*     int* topo_used_by_job_length -  Topology string length.
*
*  RESULT
*     bool - True if the job can be bound to the topology, false if not.
*
*  NOTES
*     MT-NOTE: binding_explicit_check_and_account() is MT safe
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
bool binding_explicit_check_and_account(const int* list_of_sockets, const int samount,
   const int* list_of_cores, const int score, char** topo_used_by_job,
   int* topo_used_by_job_length)
{
   int i;

   /* position of <socket>,<core> in topology string */
   int pos;
   /* status if accounting was possible */
   bool possible = true;

   /* input parameter validation */
   if (samount != score || samount <= 0 || list_of_sockets == NULL
         || list_of_cores == NULL) {
      return false;
   }

   /* check if the topology which is used already is accessable */
   if (logical_used_topology == NULL) {
      /* we have no topology string at the moment (should be initialized before) */
      if (!get_topology(&logical_used_topology, &logical_used_topology_length)) {
         /* couldn't even get the topology string */
         return false;
      }
   }

   /* create output string */
   get_topology(topo_used_by_job, topo_used_by_job_length);

   /* go through the <socket>,<core> pair list */
   for (i = 0; i < samount; i++) {

      /* get position in topology string */
     if ((pos = get_position_in_topology(list_of_sockets[i], list_of_cores[i],
        logical_used_topology, logical_used_topology_length)) < 0) {
        /* the <socket>,<core> does not exist */
        possible = false;
        break;
     }

      /* check if this core is available (DG TODO introduce threads) */
      if (logical_used_topology[pos] == 'C') {
         /* do temporarily account it */
         (*topo_used_by_job)[pos] = 'c';
         /* thread binding: account threads here */
         account_all_threads_after_core(topo_used_by_job, pos);
      } else {
         /* core not usable -> early abort */
         possible = false;
         break;
      }
   }

   /* do accounting if all cores can be used */
   if (possible) {
      if (account_job_on_topology(&logical_used_topology, logical_used_topology_length,
         *topo_used_by_job, *topo_used_by_job_length) == false) {
         possible = false;
      }
   }

   /* free memory when unsuccessful */
   if (possible == false) {
      sge_free(topo_used_by_job);
      *topo_used_by_job_length = 0;
   }

   return possible;
}

/****** sge_binding/free_topology() ********************************************
*  NAME
*     free_topology() -- Free cores used by a job on module global accounting string.
*
*  SYNOPSIS
*     bool free_topology(const char* topology, const int topology_length)
*
*  FUNCTION
*     Frees global resources (cores, sockets, or threads) which are marked as
*     beeing used (lower case letter, like 'c' 's' 't') in the given
*     topology string.
*
*  INPUTS
*     const char* topology      - Topology string with the occupied resources.
*     const int topology_length - Length of the topology string
*
*  RESULT
*     bool - true in case of success; false in case of a topology mismatch
*
*  NOTES
*     MT-NOTE: free_topology() is MT safe
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
bool free_topology(const char* topology, const int topology_length)
{
   /* free cores, sockets and threads in global accounting */
   int i;
   int size = topology_length;

   if (topology_length < 0) {
      /* size not known but we stop at \0 */
      size = 1000000;
   }

   for (i = 0; i < size && i < logical_used_topology_length &&
      topology[i] != '\0' && logical_used_topology[i] != '\0'; i++) {

      if (topology[i] == 'c') {
         if (logical_used_topology[i] != 'c' && logical_used_topology[i] != 'C') {
            /* topology type mismatch: input parameter is not like local topology */
            return false;
         } else {
            logical_used_topology[i] = 'C';
         }
      } else if (topology[i] == 't') {
         if (logical_used_topology[i] != 't' && logical_used_topology[i] != 'T') {
            /* topology type mismatch: input parameter is not like local topology */
            return false;
         } else {
            logical_used_topology[i] = 'T';
         }
      } else if (topology[i] == 's') {
         if (logical_used_topology[i] != 's' && logical_used_topology[i] != 'S') {
            /* topology type mismatch: input parameter is not like local topology */
            return false;
         } else {
            logical_used_topology[i] = 'S';
         }
      }

   }

   return true;
}

/* ---------------------------------------------------------------------------*/
/*                   Bookkeeping of cores in use by SGE                       */
/* ---------------------------------------------------------------------------*/

bool get_linear_automatic_socket_core_list_and_account(const int amount,
      int** list_of_sockets, int* samount, int** list_of_cores, int* camount,
      char** topo_by_job, int* topo_by_job_length)
{
   /* return value: if it is possible to fit the request on the host  */
   bool possible       = true;

   /* temp topology string where accounting is done on     */
   char* tmp_topo_busy = NULL;

   /* number of cores we could account already             */
   int used_cores      = 0;

   /* the numbers of the sockets which are completely free */
   int* sockets        = NULL;
   int sockets_size    = 0;

   /* tmp counter */
   int i;

   /* get the topology which could be used by the job */
   tmp_topo_busy = (char *) calloc(logical_used_topology_length, sizeof(char));
   memcpy(tmp_topo_busy, logical_used_topology, logical_used_topology_length*sizeof(char));

   /* 1. Find all free sockets and try to fit the request on them     */
   if (get_free_sockets(tmp_topo_busy, logical_used_topology_length, &sockets,
         &sockets_size) == true) {

      /* there are free sockets: use them */
      for (i = 0; i < sockets_size && used_cores < amount; i++) {
         int needed_cores = amount - used_cores;
         used_cores += account_cores_on_socket(&tmp_topo_busy, logical_used_topology_length,
                           sockets[i], needed_cores, list_of_sockets, samount,
                           list_of_cores, camount);
      }

      sge_free(&sockets);
   }

   /* 2. If not all cores fit there - fill up the rest of the sockets */
   if (used_cores < amount) {

      /* the socket which offers some cores */
      int socket_free = 0;
      /* the number of cores we still need */
      int needed_cores = amount - used_cores;

      while (needed_cores > 0) {
         /* get the socket with the most free cores */
         if (get_socket_with_most_free_cores(tmp_topo_busy, logical_used_topology_length,
               &socket_free) == true) {

            int accounted_cores = account_cores_on_socket(&tmp_topo_busy,
                                    logical_used_topology_length, socket_free,
                                    needed_cores, list_of_sockets, samount,
                                    list_of_cores, camount);

            if (accounted_cores < 1) {
               /* there must be a bug in one of the last two functions! */
               possible = false;
               break;
            }

            needed_cores -= accounted_cores;

          } else {
            /* we don't have free cores anymore */
            possible = false;
            break;
          }
       }

   }

   if (possible == true) {
      /* calculate the topology used by the job out of */
      create_topology_used_per_job(topo_by_job, topo_by_job_length,
         logical_used_topology, tmp_topo_busy, logical_used_topology_length);

      /* make the temporary accounting permanent */
      memcpy(logical_used_topology, tmp_topo_busy, logical_used_topology_length*sizeof(char));
   }

   sge_free(&tmp_topo_busy);

   return possible;
}

static bool get_socket_with_most_free_cores(const char* topology, const int topology_length,
               int* socket_number)
{
   /* get the socket which offers most free cores */
   int highest_number_of_cores = 0;
   *socket_number              = 0;
   int current_socket          = -1;
   int i;
   /* number of unbound cores on the current socket */
   int current_free_cores      = 0;

   /* go through the topology, remember the socket with the highest amount
      of free cores so far and update it when it is neccessary */
   for (i = 0; i < topology_length && topology[i] != '\0'; i++) {

      if (topology[i] == 'S' || topology[i] == 's') {
         /* we are on a new socket */
         current_socket++;
         /* reset core counter */
         current_free_cores = 0;
      } else if (topology[i] == 'C') {
         current_free_cores++;

         /* remember if the socket offers more free cores */
         if (current_free_cores > highest_number_of_cores) {
            highest_number_of_cores = current_free_cores;
            *socket_number          = current_socket;
         }

      }

   }

   if (highest_number_of_cores <= 0) {
      /* there is no core free */
      return false;
   } else {
      /* we've found the socket which offers most free cores (socket_number) */
      return true;
   }
}

static bool account_all_threads_after_core(char** topology, const int core_pos)
{
   /* we need the position after the C in the topology string (example: "SCTTSCTT"
      or "SCCSCC") */
   int next_pos = core_pos + 1;

   /* check correctness of input values */
   if (topology == NULL || (*topology) == NULL || core_pos < 0 || strlen(*topology) <= core_pos) {
      return false;
   }

   /* check if we are at the last core of the string without T's at the end */
   if (next_pos >= strlen(*topology)) {
      /* there is no thread on the last core to account: thats a success anyway */
      return true;
   } else {
      /* set all T's at the current position */
      while ((*topology)[next_pos] == 'T' || (*topology)[next_pos] == 't') {
         /* account the thread */
         (*topology)[next_pos] = 't';
         next_pos++;
      }
   }

   return true;
}


static int account_cores_on_socket(char** topology, const int topology_length,
               const int socket_number, const int cores_needed, int** list_of_sockets,
               int* list_of_sockets_size, int** list_of_cores, int* list_of_cores_size)
{
   int i;
   /* socket number we are at the moment */
   int current_socket_number = -1;
   /* return value */
   int retval;

   /* try to use as many cores as possible on a specific socket
      but not more */

   /* jump to the specific socket given by the "socket_number" */
   for (i = 0; i < topology_length && (*topology)[i] != '\0'; i++) {
      if ((*topology)[i] == 'S' || (*topology)[i] == 's') {
         current_socket_number++;
         if (current_socket_number >= socket_number) {
            /* we are at the beginning of socket #"socket_number" */
            break;
         }
      }
   }

   /* check if we reached that socket or if it was out of range */
   if (socket_number != current_socket_number) {

      /* early abort because we couldn't find the socket we were
         searching for */
      retval = 0;

   } else {

      /* we are at a 'S' or 's' and going to the next 'S' or 's'
         and collecting all cores in between */

      int core_counter = 0;   /* current core number on the socket */
      i++;                    /* just forward to the first core on the socket */
      retval  = 0;            /* need to initialize the number of cores we found */

      for (; i < topology_length && (*topology)[i] != '\0'; i++) {
         if ((*topology)[i] == 'C') {
            /* take this core */
            (*list_of_sockets_size)++;    /* the socket list is growing */
            (*list_of_cores_size)++;      /* the core list is growing */
            *list_of_sockets = sge_realloc(*list_of_sockets, (*list_of_sockets_size)
                                           * sizeof(int), 1);
            *list_of_cores   = sge_realloc(*list_of_cores, (*list_of_cores_size)
                                           * sizeof(int), 1);
            /* store the logical <socket,core> tuple inside the lists */
            (*list_of_sockets)[(*list_of_sockets_size) - 1]   = socket_number;
            (*list_of_cores)[(*list_of_cores_size) - 1]       = core_counter;
            /* increase the number of cores we've collected so far */
            retval++;
            /* move forward to the next core */
            core_counter++;
            /* do accounting */
            (*topology)[i] = 'c';
            /* thread binding: accounting is done here */
            account_all_threads_after_core(topology, i);

         } else if ((*topology)[i] == 'c') {
            /* this core is already in use */
            /* move forward to the next core */
            core_counter++;
         } else if ((*topology)[i] == 'S' || (*topology)[i] == 's') {
            /* we are already on another socket which we can not use */
            break;
         }

         if (retval >= cores_needed) {
            /* we have already collected as many cores we need to collect */
            break;
         }
      }

   }

   return retval;
}


static bool get_free_sockets(const char* topology, const int topology_length,
               int** sockets, int* sockets_size)
{
   /* temporary counter */
   int i, j;
   /* this number of sockets we discovered already */
   int socket_number  = 0;

   (*sockets) = NULL;
   (*sockets_size) = 0;

   /* go through the whole topology and check if there are some sockets
      completely unbound */
   for (i = 0; i < topology_length && topology[i] != '\0'; i++) {

      if (topology[i] == 'S' || topology[i] == 's') {

         /* we're on a new socket: check all cores (and skip threads) after it */
         bool free = true;

         /* check the topology till the next socket (or end) */
         for (j = i + 1; j < topology_length && topology[j] != '\0'; j++) {
            if (topology[j] == 'c') {
               /* this socket has at least one core in use */
               free = false;
            } else if (topology[j] == 'S' || topology[j] == 's') {
               break;
            }
         }

         /* fast forward */
         i = j;

         /* check if this socket had a core in use */
         if (free == true) {
            /* this socket can be used completely */
            (*sockets) = sge_realloc(*sockets, ((*sockets_size)+1)*sizeof(int), 1);
            (*sockets)[(*sockets_size)] = socket_number;
            (*sockets_size)++;
         }

         /* increment the number of sockets we discovered so far */
         socket_number++;

      } /* end if this is a socket */

   }

   /* it was successful when we found at least one socket not used by any job */
   if ((*sockets_size) > 0) {
      /* we also have to free the list outside afterwards */
      return true;
   } else {
      return false;
   }
}



/****** sge_binding/get_striding_first_socket_first_core_and_account() ********
*  NAME
*     get_striding_first_socket_first_core_and_account() -- Checks if and where
*                                                           striding would fit.
*
*  SYNOPSIS
*     bool getStridingFirstSocketFirstCore(const int amount, const int
*     stepsize, int* first_socket, int* first_core)
*
*  FUNCTION
*     This operating system independent function checks (depending on
*     the underlaying topology string and the topology string which
*     reflects already execution units in use) if it is possible to
*     bind the job in a striding manner to cores on the host.
*
*     This function requires the topology string and the string with the
*     topology currently in use.
*
*  INPUTS
*     const int amount    - Amount of cores to allocate.
*     const int stepsize  - Distance of the cores to allocate.
*     const int start_at_socket - First socket to begin the search with (usually at 0).
*     const int start_at_core   - First core to begin the search with (usually at 0).
*     int* first_socket   - out: First socket when striding is possible (return value).
*     int* first_core     - out: First core when striding is possible (return value).
*
*  RESULT
*     bool - if true striding is possible at <first_socket, first_core>
*
*  NOTES
*     MT-NOTE: getStridingFirstSocketFirstCore() is not MT safe
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
bool get_striding_first_socket_first_core_and_account(const int amount, const int stepsize,
   const int start_at_socket, const int start_at_core, const bool automatic,
   int* first_socket, int* first_core, char** accounted_topology,
   int* accounted_topology_length)
{
   /* return value: if it is possible to fit the request on the host */
   bool possible   = false;

   /* position in topology string */
   int i = 0;

   /* socket and core counter in order to find the first core and socket */
   int sc = -1;
   int cc = -1;

   /* these core and socket counters are added later on .. */
   int found_cores   = 0;
   int found_sockets = 0; /* first socket is given implicitely */

   /* temp topology string where accounting is done on */
   char* tmp_topo_busy;

   /* initialize socket and core where the striding will fit */
   *first_socket   = 0;
   *first_core     = 0;

   if (start_at_socket < 0 || start_at_core < 0) {
      /* wrong input parameter */
      return false;
   }

   if (logical_used_topology == NULL) {
      /* we have no topology string at the moment (should be initialized before) */
      if (!get_topology(&logical_used_topology, &logical_used_topology_length)) {
         /* couldn't even get the topology string */
         return false;
      }
   }
   /* temporary accounting string -> account on this and
      when eventually successful then copy this string back
      to global topo_busy string */
   tmp_topo_busy = (char *) calloc(logical_used_topology_length + 1, sizeof(char));
   memcpy(tmp_topo_busy, logical_used_topology, logical_used_topology_length*sizeof(char));

   /* we have to go to the first position given by the arguments
      (start_at_socket and start_at_core) */
   for (i = 0; i < logical_used_topology_length; i++) {

      if (logical_used_topology[i] == 'C' || logical_used_topology[i] == 'c') {
         /* found core   -> update core counter   */
         cc++;
      } else if (logical_used_topology[i] == 'S' || logical_used_topology[i] == 's') {
         /* found socket -> update socket counter */
         sc++;
         /* we're changing socket -> no core found on this one yet */
         cc = -1;
      } else if (logical_used_topology[i] == '\0') {
         /* we couldn't find start socket start string */
         possible = false;
         sge_free(&tmp_topo_busy);
         return possible;
      }

      if (sc == start_at_socket && cc == start_at_core) {
         /* we found our starting point (we remember 'i' for next loop!) */
         break;
      }
   }

   /* check if we found the socket and core we want to start searching */
   if (sc != start_at_socket || cc != start_at_core) {
      /* could't find the start socket and start core */
      sge_free(&tmp_topo_busy);
      return false;
   }

   /* check each position of the topology string */
   /* we reuse 'i' from last loop -> this is the position where we begin */
   for (; i < logical_used_topology_length && logical_used_topology[i] != '\0'; i++) {

      /* this could be optimized (with increasing i in case if it is not
         possible) */
      if (is_starting_point(logical_used_topology, logical_used_topology_length, i, amount, stepsize,
            &tmp_topo_busy)) {
         /* we can do striding with this as starting point */
         possible = true;
         /* update place where we can begin */
         *first_socket = start_at_socket + found_sockets;
         *first_core   = start_at_core + found_cores;
         /* return the accounted topology */
         create_topology_used_per_job(accounted_topology, accounted_topology_length,
            logical_used_topology, tmp_topo_busy, logical_used_topology_length);
         /* finally do execution host wide accounting */
         /* DG TODO mutex */
         memcpy(logical_used_topology, tmp_topo_busy, logical_used_topology_length*sizeof(char));

         break;
      } else {

         /* else retry and update socket and core number to start with */

         if (logical_used_topology[i] == 'C' || logical_used_topology[i] == 'c') {
            /* jumping over a core */
            found_cores++;
            /* a core is a valid starting point for binding in non-automatic case */
            /* if we have a fixed start socket and a start core we do not retry
               it with the next core available (when introducing T's this have to
               be added there too) */
            if (automatic == false) {
               possible = false;
               break;
            }

         } else if (logical_used_topology[i] == 'S' || logical_used_topology[i] == 's') {
            /* jumping over a socket */
            found_sockets++;
            /* we are at core 0 on the new socket */
            found_cores = 0;
         }
         /* at the moment we are not interested in threads or anything else */

      }

   } /* end go through the whole topology string */

   sge_free(&tmp_topo_busy);
   return possible;
}


static bool create_topology_used_per_job(char** accounted_topology, int* accounted_topology_length,
            char* logical_used_topology, char* used_topo_with_job, int logical_used_topology_length)
{
   /* tmp counter */
   int i;

   /* length of output string remains the same */
   (*accounted_topology_length) = logical_used_topology_length;

   /* copy string of current topology in use */
   (*accounted_topology) = calloc(logical_used_topology_length+1, sizeof(char));
   if ((*accounted_topology) == NULL) {
      /* out of memory */
      return false;
   }

   memcpy((*accounted_topology), logical_used_topology, sizeof(char)*logical_used_topology_length);

   /* revert all accounting from other jobs */
   for (i = 0; i < logical_used_topology_length; i++) {
      if ((*accounted_topology)[i] == 'c') {
         (*accounted_topology)[i] = 'C';
      } else if ((*accounted_topology)[i] == 's') {
         (*accounted_topology)[i] = 'S';
      } else if ((*accounted_topology)[i] == 't') {
         (*accounted_topology)[i] = 'T';
      }
   }

   /* account all the resources the job consumes: these are all occupied
      resources in used_topo_with_job String that are not occupied in
      logical_used_topology String */
   for (i = 0; i < logical_used_topology_length; i++) {

      if (used_topo_with_job[i] == 'c' && logical_used_topology[i] == 'C') {
         /* this resource is from job exclusively used */
         (*accounted_topology)[i] = 'c';
      }

      if (used_topo_with_job[i] == 't' && logical_used_topology[i] == 'T') {
         /* this resource is from job exclusively used */
         (*accounted_topology)[i] = 't';
      }

      if (used_topo_with_job[i] == 's' && logical_used_topology[i] == 'S') {
         /* this resource is from job exclusively used */
         (*accounted_topology)[i] = 's';
      }

   }

   return true;
}

/****** sge_binding/is_starting_point() ****************************************
*  NAME
*     is_starting_point() -- Checks if 'pos' is a valid first core for striding.
*
*  SYNOPSIS
*     bool is_starting_point(const char* topo, const int length, const int pos,
*     const int amount, const int stepsize)
*
*  FUNCTION
*     Checks if 'pos' is a starting point for binding the 'amount' of cores
*     in a striding manner on the host. The topo string contains 'C's for unused
*     cores and 'c's for cores in use.
*
*  INPUTS
*     const char* topo   - String representing the topology currently in use.
*     const int length   - Length of topology string.
*     const int pos      - Position within the topology string.
*     const int amount   - Amount of cores to bind to.
*     const int stepsize - Step size when binding in a striding manner.
*
*  OUTPUTS
*     char* topo_account - Here the accounting is done on.
*
*  RESULT
*     bool - true if striding with the given parameters is possible.
*
*  NOTES
*     MT-NOTE: is_starting_point() is not MT safe
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static bool is_starting_point(const char* topo, const int length, const int pos,
   const int amount, const int stepsize, char** topo_account) {

   /* go through the topology (in use) string with the beginning at pos
      and try to fit all cores in there */
   int i;
   /* core counter in order to fulfill the stepsize property */
   int found_cores = 1;
   /* so many cores we have collected so far */
   int accounted_cores = 0;
   /* return value */
   bool is_possible = false;

   /* stepsize must be 1 or greater */
   if (stepsize < 1) {
      return false;
   }
   /* position in string must be smaller than string length */
   if (pos >= length) {
      return false;
   }
   /* topology string must not be NULL */
   if (topo == NULL) {
      return false;
   }
   /* amount must be 1 or greater */
   if (amount < 1) {
      return false;
   }

   /* fist check if this is a valid core */
   if (topo[pos] != 'C' || topo[pos] == '\0') {
      /* not possible this is not a valid free core (could be a socket,
         thread, or core in use) */
      return false;
   }

   /* we count this core */
   accounted_cores++;
   /* this core is used */
   (*topo_account)[pos] = 'c';
   /* thread binding: account following threads */
   account_all_threads_after_core(topo_account, pos);

   if (accounted_cores == amount) {
      /* we have all cores and we are still within the string */
      is_possible = true;
      return is_possible;
   }

   /* go to the remaining topology which is in use */
   for (i = pos + 1; i < length && topo[i] != '\0'; i++) {

      if (topo[i] == 'C') {
         /* we found an unused core */
         if (found_cores >= stepsize) {
            /* this core we need and it is free - good */
            found_cores = 1;
            /* increase the core counter */
            accounted_cores++;
            /* this core is used */
            (*topo_account)[i] = 'c';
            /* thread binding: bind following threads */
            account_all_threads_after_core(topo_account, i);

         } else if (found_cores < stepsize) {
            /* this core we don't need */
            found_cores++;
         }
      } else if (topo[i] == 'c') {
         /* this is a core in use */
         if (found_cores >= stepsize) {
            /* this core we DO NEED but it is busy */
            return false;
         } else if (found_cores < stepsize) {
            /* this core we don't need */
            found_cores++;
         }
      }

      /* accounted cores */
      if (accounted_cores == amount) {
         /* we have all cores and we are still within the string */
         is_possible = true;
         break;
      }
   }

   /* using this core as first core is possible */
   return is_possible;
}

static int get_position_in_topology(const int socket, const int core,
   const char* topology, const int topology_length)
{

   int i;
   /* position of <socket>,<core> in the topology string */
   int retval = -1;

   /* current position */
   int s = -1;
   int c = -1;
   int t = -1;

   if (topology_length <= 0 || socket < 0 || core < 0 || topology == NULL) {
      return false;
   }

   for (i = 0; i < topology_length && topology[i] != '\0'; i++) {
      if (topology[i] == 'S') {
         /* we've got a new socket */
         s++;
         /* invalidate core counter */
         c = -1;
      } else if (topology[i] == 'C') {
         /* we've got a new core */
         c++;
         /* invalidate thread counter */
         t = -1;
      } else if (topology[i] == 'T') {
         /* we've got a new thread */
         t++;
      }
      /* check if we are at the position seeking for */
      if (socket == s && core == c) {
         retval = i;
         break;
      }
   }

   return retval;
}

bool initialize_topology(void) {

   /* this is done when execution daemon starts        */

   if (logical_used_topology == NULL) {
      if (get_topology(&logical_used_topology, &logical_used_topology_length)) {
         return true;
      }
   }

   return false;
}


/* ---------------------------------------------------------------------------*/
/*               End of bookkeeping of cores in use by GE                     */
/* ---------------------------------------------------------------------------*/
