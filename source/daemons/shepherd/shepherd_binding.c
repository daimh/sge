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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Portions of this code are Copyright 2011 Univa Inc.
 *  Copyright (C) 2011 Dave Love, University of Liverpool
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "uti/sge_binding_hlp.h"
#include "uti/sge_dstring.h"
#include "uti/sge_string.h"

#include "shepherd_binding.h"
#include "err_trace.h"
#include "binding_support.h"

static bool binding_set_linear(int first_socket, int first_core,
               int number_of_cores, int offset, const binding_type_t type);

static bool binding_set_striding(int first_socket, int first_core,
               int number_of_cores, int offset, int n, const binding_type_t type);

#if HAVE_HWLOC
static bool bind_process_to_mask(const hwloc_bitmap_t cpuset);

static bool create_binding_env(hwloc_const_bitmap_t set);
#endif

static bool binding_explicit(const int* list_of_sockets, const int samount, 
                             const int* list_of_cores, const int camount,
                             const binding_type_t type);

/****** shepherd_binding/do_core_binding() *************************************
*  NAME
*     do_core_binding() -- Performs the core binding task.
*
*  SYNOPSIS
*     int do_core_binding(void) 
*
*  FUNCTION
*     Performs core binding on shepherd side. All information required for  
*     the binding is communicated from execd to shepherd in the config 
*     file value "binding". If there is "NULL" no core binding is done. 
* 
*     If there is any instruction the bookkeeping for these cores is already 
*     done. In case of Solaris the processor set is already created by 
*     execution daemon.  The binding is inherited from shepherd by the job
*     it starts.
* 
*  RESULT
*     bool - Returns true for success and false value in case of problems.
*
*  NOTES
*     MT-NOTE: do_core_binding() is not MT safe 
*
*******************************************************************************/
bool do_core_binding(void)
{
   if (HAVE_HWLOC) {
   /* Check if "binding" parameter in 'config' file 
    * is available and not set to "binding=no_job_binding".
    * If so, we do an early abortion. 
    */
   char *binding = get_conf_val("binding");
   binding_type_t type;

   if (binding == NULL || strcasecmp("NULL", binding) == 0) {
      shepherd_trace("do_core_binding: \"binding\" parameter not found in config file");
      return false;
   }
   
   if (strcasecmp("no_job_binding", binding) == 0) {
      shepherd_trace("do_core_binding: skip binding - no core binding configured");
      return false;
   }
   
   /* get the binding type (set = 0 | env = 1 | pe = 2) where default is 0 */
   type = binding_parse_type(binding); 

   /* do a binding accorting the strategy */
   if (strstr(binding, "linear") != NULL) {
      /* do a linear binding */ 
      int amount;
      int socket;
      int core;

      shepherd_trace("do_core_binding: do linear");
   
      /* get the number of cores to bind on */
      if ((amount = binding_linear_parse_number(binding)) < 0) {
         shepherd_trace("do_core_binding: couldn't parse the number of cores from config file");
         return false;
      }

      /* get the socket to begin binding with (choosen by execution daemon) */
      if ((socket = binding_linear_parse_socket_offset(binding)) < 0) {
         shepherd_trace("do_core_binding: couldn't get the socket number from config file");
         return false;
      }

      /* get the core to begin binding with (choosen by execution daemon)   */
      if ((core = binding_linear_parse_core_offset(binding)) < 0) {
         shepherd_trace("do_core_binding: couldn't get the core number from config file");
         return false;
      }

      /* perform core binding on current process */
      if (binding_set_linear(socket, core, amount, 1, type) == false) {
         /* core binding was not successful */
         if (type == BINDING_TYPE_SET) {
            shepherd_trace("do_core_binding: linear binding was not successful");
         } else if (type == BINDING_TYPE_ENV) {
            shepherd_trace("do_core_binding: couldn't set SGE_BINDING environment variable");
         } else if (type == BINDING_TYPE_PE) {
            shepherd_trace("do_core_binding: couldn't produce rankfile");
         }
      } else {
         if (type == BINDING_TYPE_SET) {
            shepherd_trace("do_core_binding: job successfully bound");
         } else if (type == BINDING_TYPE_ENV) {
            shepherd_trace("do_core_binding: SGE_BINDING environment variable created");
         } else if (type == BINDING_TYPE_PE) {
            shepherd_trace("do_core_binding: rankefile produced");
         }
      }

   } else if (strstr(binding, "striding") != NULL) {
      int amount = binding_striding_parse_number(binding);
      int stepsize = binding_striding_parse_step_size(binding);
      
      /* these are the real start parameters */
      int first_socket = 0, first_core = 0;
      
      shepherd_trace("do_core_binding: striding");

      if (amount <= 0) {
         shepherd_trace("do_core_binding: error parsing <amount>");
         return false;
      }

      if (stepsize < 0) {
         shepherd_trace("do_core_binding: error parsing <stepsize>");
         return false;
      }
      
      first_socket = binding_striding_parse_first_socket(binding);
      if (first_socket < 0) {
         shepherd_trace("do_core_binding: error parsing <socket>");
         return false;
      }
      
      first_core   = binding_striding_parse_first_core(binding);
      if (first_core < 0) {
         shepherd_trace("do_core_binding: error parsing <core>");
         return false;
      }

      /* last core has to be incremented because core 0 is first core to be used */
      if (stepsize == 0) {
         /* stepsize must be >= 1 */
         stepsize = 1;
      }

      shepherd_trace("do_core_binding: striding set binding: first_core: %d first_socket %d amount %d stepsize %d", 
         first_core, first_socket, amount, stepsize);

      /* get the first core and first socket which is available for striding    */

      /* perform core binding on current process                */

      if (binding_set_striding(first_socket, first_core, amount, 0, stepsize, type)) {
         shepherd_trace("do_core_binding: striding: binding done");
      } else {
         shepherd_trace("do_core_binding: striding: binding not done");
      }

   } else if (strstr(binding, "explicit") != NULL) {

      /* list with the sockets (first part of the <socket>,<core> tuples) */
      int* sockets = NULL;
      /* length of sockets list */
      int nr_of_sockets = 0;
      /* list with the cores to be bound on the sockets */
      int* cores = NULL;
      /* length of cores list */
      int nr_of_cores = 0;

      shepherd_trace("do_core_binding: explicit");
      
      /* get <socket>,<core> pairs out of binding string */ 
      if (binding_explicit_extract_sockets_cores(binding, &sockets, &nr_of_sockets,
            &cores, &nr_of_cores) == true) {

         if (nr_of_sockets == 0 && nr_of_cores == 0) {
            /* no cores and no sockets are found */
            shepherd_trace("do_core_binding: explicit: no socket or no core was specified");
         } else if (nr_of_sockets != nr_of_cores) {
            shepherd_trace("do_core_binding: explicit: unequal number of specified sockets and cores");
         } else {
            /* do core binding according the <socket>,<core> tuples */
            if (binding_explicit(sockets, nr_of_sockets, cores, nr_of_cores, type) == true) {
               shepherd_trace("do_core_binding: explicit: binding done");
            } else {
               shepherd_trace("do_core_binding: explicit: no core binding done");
            }
         }
         
         sge_free(&sockets);
         sge_free(&cores);

      } else {
         sge_free(&sockets);
         sge_free(&cores);    
         shepherd_trace("do_core_binding: explicit: couldn't extract <socket>,<core> pair");
      }

   } else {
   
      if (binding != NULL) {
         shepherd_trace("do_core_binding: WARNING: unknown \"binding\" parameter: %s", 
            binding);
      } else {
         shepherd_trace("do_core_binding: WARNING: binding was null!");
      }   

   }
   
   shepherd_trace("do_core_binding: finishing");
   }
   return true;
}


/* helper for core_binding */

/****** shepherd_binding/binding_set_linear() ***************************************
*  NAME
*     binding_set_linear() -- Bind current process linear to chunk of cores.
*
*  SYNOPSIS
*     bool binding_set_linear(int first_socket, int first_core, int 
*     number_of_cores, int offset)
*
*  FUNCTION
*     Binds current process (shepherd) to a set of cores. All processes 
*     started by the current process inherit the core binding.
*     
*     The core binding is done in a linear manner, that means that 
*     the process is bound to 'number_of_cores' cores using one core
*     after another starting at socket 'first_socket' (usually 0) and 
*     core = 'first_core' (usually 0) + 'offset'. If the core number 
*     is higher than the number of cores which are provided by socket 
*     'first_socket' then the next socket is taken (the core number 
*      defines how many cores are skiped).
*
*  INPUTS
*     int first_socket    - The first socket (starting at 0) to bind to. 
*     int first_core      - The first core to bind. 
*     int number_of_cores - The number_of_cores of cores to bind to.
*     int offset          - The user specified core number offset. 
*     binding_type_t type - The type of binding ONLY FOR EXECD ( set | env | pe )
*                           
*  RESULT
*     bool - true if binding for current process was done, false if not
*
*  NOTES
*     MT-NOTE: binding_set_linear() is not MT safe 
*
*******************************************************************************/
static bool binding_set_linear(int first_socket, int first_core,
               int number_of_cores, int offset, const binding_type_t type)
{

#if HAVE_HWLOC
   /* sets bitmask in a linear manner        */ 
   /* first core is on exclusive host 0      */ 
   /* first core could be set from scheduler */ 
   /* offset is the first core to start with (makes sense only with
      exclusive host) */
   dstring error = DSTRING_INIT;

   if (has_core_binding() == true) {
      /* bitmask for processors to turn on and off */
      hwloc_bitmap_t cpuset = hwloc_bitmap_alloc();
         
      if (has_topology_information()) {
         /* number of cores set in processor binding mask */
         int cores_set;
         /* next socket to use */
         int next_socket = first_socket;
         /* the number of cores of the next socket */
         int socket_number_of_cores;
         /* next core to use */
         int next_core = first_core + offset;
         /* maximal number of sockets on this system */
         int max_number_of_sockets = get_number_of_sockets();
         hwloc_obj_t this_core;

         /* strategy: go to the first_socket and the first_core + offset and 
            fill up socket and go to the next one. */ 
               
         /* TODO maybe better to search for using a core exclusively? */
            
         while (get_number_of_cores(next_socket) <= next_core) {
            /* TODO which kind of warning when first socket does not
               offer this? */
            /* move on to next socket - could be that we have to deal
               only with cores instead of <socket><core> tuples */
            next_core -= get_number_of_cores(next_socket);
            next_socket++;
            if (next_socket >= max_number_of_sockets) {
               /* we are out of sockets - we do nothing */
               hwloc_bitmap_free(cpuset);
               return false;
            }
         }  
         this_core =
            hwloc_get_obj_below_by_type(sge_hwloc_topology,
                                        HWLOC_OBJ_SOCKET, next_socket,
                                        HWLOC_OBJ_CORE, next_core);
         hwloc_bitmap_or(cpuset, cpuset, this_core->cpuset);

         /* collect the other processor ids with the strategy */
         for (cores_set = 1; cores_set < number_of_cores; cores_set++) {
            next_core++;
            /* jump to next socket when it is needed */
            /* maybe the next socket could offer 0 cores (I can't see when, 
               but just to be sure) */
            while ((socket_number_of_cores = get_number_of_cores(next_socket))
                        <= next_core) {
               next_socket++;
               next_core = next_core - socket_number_of_cores;
               if (next_socket >= max_number_of_sockets) {
                  /* we are out of sockets - we do nothing */
                  hwloc_bitmap_free(cpuset);
                  return false;
               }
            }
            this_core =
               hwloc_get_obj_below_by_type(sge_hwloc_topology,
                                           HWLOC_OBJ_SOCKET, next_socket,
                                           HWLOC_OBJ_CORE, next_core);
            hwloc_bitmap_or(cpuset, cpuset, this_core->cpuset);
         }

         /* check what to do with the processor ids (set, env or pe) */
         if (type == BINDING_TYPE_PE) {
               
            /* is done outside */

         } else if (type == BINDING_TYPE_ENV) {
               
            /* set the environment variable                    */
            /* this does not show up in "environment" file !!! */
            if (create_binding_env(cpuset) == true) {
               shepherd_trace("binding_set_linear: SGE_BINDING env var created");
            } else {
               shepherd_trace("binding_set_linear: problems while creating SGE_BINDING env");
            }
             
         } else {

             /* bind SET process to mask */ 
            if (bind_process_to_mask(cpuset) == false) {
               /* there was an error while binding */ 
               hwloc_bitmap_free(cpuset);
               return false;
            }
         }

         hwloc_bitmap_free(cpuset);

      } else {
            
         /* TODO DG strategy without topology information but with 
            working library? */
         shepherd_trace("binding_set_linear: no information about topology");
         return false;
      }
         

   } else {

      shepherd_trace("binding_set_linear: binding not supported: %s",
                     sge_dstring_get_string(&error));

      sge_dstring_free(&error);
   }
#endif  /* HAVE_HWLOC */
   return true;
}

/****** shepherd_binding/binding_set_striding() *************************************
*  NAME
*     binding_set_striding() -- Binds current process to cores.
*
*  SYNOPSIS
*     bool binding_set_striding(int first_socket, int first_core, int
*     number_of_cores, int offset, int stepsize)
*
*  FUNCTION
*     Performs a core binding for the calling process according to the 
*     'striding' strategy. The first core used is specified by first_socket
*     (beginning with 0) and first_core (beginning with 0). If first_core is 
*     greater than available cores on first_socket, the next socket is examined 
*     and first_core is reduced by the skipped cores. If the first_core could 
*     not be found on system (because it was to high) no binding will be done.
*     
*     If the first core was choosen the next one is defined by the step size 'n' 
*     which is incremented to the first core found. If the socket has not the 
*     core (because it was the last core of the socket for example) the next 
*     socket is examined.
*
*     If the system is out of cores and there are still some cores to select 
*     (because of the number_of_cores parameter) no core binding will be performed.
*    
*  INPUTS
*     int first_socket    - first socket to begin with  
*     int first_core      - first core to start with  
*     int number_of_cores - total number of cores to be used
*     int offset          - core offset for first core (increments first core used) 
*     int stepsize        - step size
*     int type            - type of binding (set or env or pe)
*
*  RESULT
*     bool - Returns true if the binding was performed, otherwise false.
*
*  NOTES
*     MT-NOTE: binding_set_striding() is MT safe 
*
*******************************************************************************/
static bool
binding_set_striding(int first_socket, int first_core, int number_of_cores,
                          int offset, int stepsize, const binding_type_t type)
{
   /* n := take every n-th core */ 
   bool bound = false;

#if HAVE_HWLOC
   dstring error = DSTRING_INIT;

   if (has_core_binding() == true) {

      sge_dstring_free(&error);

         /* bitmask for processors to turn on and off */
         hwloc_bitmap_t cpuset = hwloc_bitmap_alloc();

         /* when library offers architecture: 
            - get virtual processor ids in the following manner:
              * on socket "first_socket" choose core number "first_core + offset"
              * then add n: if core is not available go to next socket
              * ...
         */
         if (has_topology_information()) {
            /* number of cores set in processor binding mask */
            int cores_set = 0;
            /* next socket to use */
            int next_socket = first_socket;
            /* next core to use */
            int next_core = first_core + offset;
            /* maximal number of sockets on this system */
            int max_number_of_sockets = get_number_of_sockets();
            hwloc_obj_t core;
            
            /* check if we are already out of range */
            if (next_socket >= max_number_of_sockets) {
               shepherd_trace("binding_set_striding: already out of sockets");
               hwloc_bitmap_free(cpuset);
               return false;
            }   

            while (get_number_of_cores(next_socket) <= next_core) {
               /* move on to next socket - could be that we have to deal only with cores 
                  instead of <socket><core> tuples */
               next_core -= get_number_of_cores(next_socket);
               next_socket++;
               if (next_socket >= max_number_of_sockets) {
                  /* we are out of sockets - we do nothing */
                  shepherd_trace("binding_set_striding: first core: out of sockets");
                  hwloc_bitmap_free(cpuset);
                  return false;
               }
            }  
            core = hwloc_get_obj_below_by_type(sge_hwloc_topology,
                                               HWLOC_OBJ_SOCKET, next_socket,
                                               HWLOC_OBJ_CORE, next_core);
            hwloc_bitmap_or(cpuset, cpuset, core->cpuset);
            
            /* collect the rest of the processor ids */ 
            for (cores_set = 1; cores_set < number_of_cores; cores_set++) {
               /* calculate next_core number */ 
               next_core += stepsize;
               
               /* check if we are already out of range */
               if (next_socket >= max_number_of_sockets) {
                  shepherd_trace("binding_set_striding: out of sockets");
                  hwloc_bitmap_free(cpuset);
                  return false;
               }   

               while (get_number_of_cores(next_socket) <= next_core) {
                  /* move on to next socket - could be that we have to deal only with cores 
                     instead of <socket><core> tuples */
                  next_core -= get_number_of_cores(next_socket);
                  next_socket++;
                  if (next_socket >= max_number_of_sockets) {
                     /* we are out of sockets - we do nothing */
                     shepherd_trace("binding_set_striding: out of sockets!");
                     hwloc_bitmap_free(cpuset);
                     return false;
                  }
                  core = hwloc_get_obj_below_by_type(sge_hwloc_topology,
                                                     HWLOC_OBJ_SOCKET,
                                                     next_socket,
                                                     HWLOC_OBJ_CORE,
                                                     next_core);
                  hwloc_bitmap_or(cpuset, cpuset, core->cpuset);
               }    
                
            } /* collecting processor ids */
           
            if (type == BINDING_TYPE_PE) {
            
               /* rankfile is created: do nothing */

            } else if (type == BINDING_TYPE_ENV) {

               /* set the environment variable */
               if (create_binding_env(cpuset) == true) {
                  shepherd_trace("binding_set_striding: SGE_BINDING env var created");
               } else {
                  shepherd_trace("binding_set_striding: problems while creating SGE_BINDING env");
               }

            } else {
               
               /* bind process to mask */ 
               if (bind_process_to_mask(cpuset) == true) {
                  /* there was an error while binding */ 
                  bound = true;
               }
            }
         
            hwloc_bitmap_free(cpuset);
            
         } else {
            /* setting bitmask without topology information which could 
               not be right? */
            shepherd_trace("binding_set_striding: bitmask without topology information");
            return false;
         }

   } else {
      /* has no core binding feature */
      sge_dstring_free(&error);
      
      return false;
   }
   
#endif  /* HAVE_HWLOC */
   return bound;
}


#if HAVE_HWLOC
/****** shepherd_binding/bind_process_to_mask() *************************************
*  NAME
*     bind_process_to_mask() -- Binds current process to a given cpuset (mask).
*
*  SYNOPSIS
*     static bool bind_process_to_mask(const hwloc_bitmap_t cpuset)
*
*  FUNCTION
*     Binds current process to a given cpuset. 
*
*  INPUTS
*     const hwloc_bitmap_t cpuset - Processors to bind processes to
*
*  RESULT
*     static bool - true if successful, false otherwise
*
*  NOTES
*     MT-NOTE: bind_process_to_mask() is not MT safe 
*
*******************************************************************************/
static bool bind_process_to_mask(const hwloc_bitmap_t cpuset)
{
   /* we only need core binding capabilites, no topology is required */
   if (!has_core_binding()) return false;
   /* Try strict binding first; fall back to non-strict if it isn't
      available.  */
   if (!hwloc_set_cpubind(sge_hwloc_topology, cpuset, HWLOC_CPUBIND_STRICT) ||
       !hwloc_set_cpubind(sge_hwloc_topology, cpuset, 0)) {
      /* Set the environment variable as for the env type.  Done for
         for conveniece, e.g. with runtimes like GCC's libgomp which
         require an environment variable to be set for thread affinity
         rather than using the core binding in effect.  */
     /* This does not show up in "environment" file!  */
      if (create_binding_env(cpuset) == true)
         shepherd_trace("bind_process_to_mask: SGE_BINDING env var created");
      return true;
      }
   return false;
}
#endif  /* HAVE_HWLOC */

/****** shepherd_binding/binding_explicit() *****************************************
*  NAME
*     binding_explicit() -- Binds current process to specified CPU cores. 
*
*  SYNOPSIS
*     bool binding_explicit(int* list_of_cores, int camount, int* 
*     list_of_sockets, int samount) 
*
*  FUNCTION
*     Binds the current process to the cores specified by a <socket>,<core>
*     tuple. The tuple is given by a list of sockets and a list of cores. 
*     The elements on the same position of these lists are reflecting 
*     a tuple. Therefore the length of the lists must be the same.
*
*  INPUTS
*     int* list_of_sockets - List of sockets in the same order as list of cores. 
*     int samount          - Length of the list of sockets. 
*     int* list_of_cores   - List of cores in the same order as list of sockets. 
*     int camount          - Length of the list of cores. 
*     int type             - Type of binding ( set | env | pe ).
*
*  RESULT
*     bool - true when the current process was bound as specified with the
*            input parameter
*
*  NOTES
*     MT-NOTE: binding_explicit() is not MT safe 
*
*******************************************************************************/
static bool binding_explicit(const int* list_of_sockets, const int samount, 
   const int* list_of_cores, const int camount, const binding_type_t type)
{
   /* return value: successful bound or not */ 
   bool bound = false;

#if HAVE_HWLOC
   /* check if we have exactly the same number of sockets as cores */
   if (camount != samount) {
      shepherd_trace("binding_explicit: bug: number of sockets != number of cores");
      return false;
   }

   if (list_of_sockets == NULL || list_of_cores == NULL) {
      shepherd_trace("binding_explicit: wrong input values");
   }   
   
   if (has_core_binding() == true) {
      
      if (has_topology_information()) {
         /* bitmask for processors to turn on and off */
         hwloc_bitmap_t cpuset = hwloc_bitmap_alloc();

         /* processor id counter */
         int pr_id_ctr;

         /* Fetch for each socket,core tuple the processor id. 
            If this is not possible for one do nothing and return false. */ 

         /* go through all socket,core tuples and get the processor id */
         for (pr_id_ctr = 0; pr_id_ctr < camount; pr_id_ctr++) { 
            hwloc_obj_t core =
              hwloc_get_obj_below_by_type(sge_hwloc_topology,
                                          HWLOC_OBJ_SOCKET,
                                          list_of_sockets[pr_id_ctr],
                                          HWLOC_OBJ_CORE,
                                          list_of_cores[pr_id_ctr]);
            if (!core) {
               hwloc_bitmap_free(cpuset);
               return false;
            }
            hwloc_bitmap_or(cpuset, cpuset, core->cpuset);
         }

         if (type == BINDING_TYPE_PE) {
            
            /* rankfile is created */

         } else if (type == BINDING_TYPE_ENV) {
            /* set the environment variable */
            if (create_binding_env(cpuset) == true) {
               shepherd_trace("binding_explicit: SGE_BINDING env var created");
            } else {
               shepherd_trace("binding_explicit: problems while creating SGE_BINDING env");
            }
         } else {
            /* do the core binding for the current process with the mask */
            if (bind_process_to_mask(cpuset) == true) {
               /* there was an error while binding */ 
               bound = true;
            } else {
               /* couldn't be bound return false */
               shepherd_trace("binding_explicit: bind_process_to_mask was not successful");
            }   
         }

         hwloc_bitmap_free(cpuset);
         /* Fixme:  Maybe free topology at this stage, but it probably
            doesn't use a significant amount of space.  */
      } else {
         /* has no topology information */
         shepherd_trace("binding_explicit: no topology information");
      }  

   } else {
      /* has no core binding ability */
      shepherd_trace("binding_explicit: host does not support core binding");
   }   
#endif  /* HAVE_HWLOC */
   return bound;
}

#if HAVE_HWLOC
/****** shepherd_binding/create_binding_env() ****************************
*  NAME
*     create_binding_env() -- Creates SGE_BINDING env variable.
*
*  SYNOPSIS
*     bool create_binding_env(hwloc_const_bitmap_t set)
*
*  FUNCTION
*     Creates the SGE_BINDING environment variable.
*     This environment variable contains a space-separated list of
*     internal processor ids given as input parameter.
*
*  INPUTS
*     hwloc_const_bitmap_t set - CPU set to use
*
*  RESULT
*     bool - true when SGE_BINDING env var could be generated false if not
*
*  NOTES
*     MT-NOTE: create_binding_env() is MT safe
*
*******************************************************************************/
static bool
create_binding_env(hwloc_const_bitmap_t set)
{
   bool retval          = true;
   dstring sge_binding  = DSTRING_INIT;
   dstring proc         = DSTRING_INIT;
   unsigned i;
   bool first           = true;

   hwloc_bitmap_foreach_begin(i, set)
      if (first) {
        first = false;
        sge_dstring_sprintf(&proc, "%d", i);
      } else {
        sge_dstring_sprintf(&proc, " %d", i);
      }
      sge_dstring_append_dstring(&sge_binding, &proc);
   hwloc_bitmap_foreach_end();

   if (sge_setenv("SGE_BINDING", sge_dstring_get_string(&sge_binding)) != 1) {
      /* settting env var was not successful */
      retval = false;
      shepherd_trace("create_binding_env: Couldn't set environment variable!");
   }

   sge_dstring_free(&sge_binding);
   sge_dstring_free(&proc);

   return retval;
}
#endif  /* HAVE_HWLOC */
