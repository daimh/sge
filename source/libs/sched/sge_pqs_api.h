#ifndef _SGE_PQS_API_H_
#define _SGE_PQS_API_H_

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

/*
 * The SGE Parallel Environment Queue Sort (PQS) API
 *
 * WARNING: This is NOT a generalized scheduler API which allows you
 * replace the Grid Engine scheduler. If that's what you're looking
 * for you've come to the wrong place. This is an evolving API to
 * specifically choose (i.e. narrow down) and order the hosts for a
 * parallel job. Before even considering using this API, you should
 * investigate the built-in configuration capabilities such as the
 * sched_conf(5) load_formula, queue_sort_method, and configurable
 * load sensors (see sge_execd(8) and sge_conf(5)). Also, if you need
 * to change the rank of tasks for an MPI job, you should consider doing
 * this in the PE start_proc_args and/or the queue_conf(5) starter_method,
 * or a JSV.
 * The existing capabilities are quite flexible and should be able to
 * handle most of your needs. Also, don't count on this interface never
 * changing. This interface was introduced in Grid Engine 6.0 and is
 * very likely to be evolving over time.
 *
 * This programming interface allows a function from a dynamically
 * loaded library to specify the queues/hosts and order that should be used
 * to schedule a parallel job in Grid Engine. The input parameters include
 * the total number of requested execution slots and a list of queue/host
 * pairs which are candidates to schedule the job. The function should
 * determine which queue/hosts should be used for the job. This
 * is accomplished by specifying the sort order by setting a sequence
 * number in the passed pqs_queue_t structure. The Grid Engine scheduler
 * will sort the queue/host list according to the sequence number and
 * schedule the job to the available slots based on the sorted order.
 * When the job is dispatched, the $pe_hostfile described in sge_pe(5)
 * will contain the hosts/queues in the specified sorted order.
 *
 * The function may also determine that the job should not be scheduled
 * on any of the available resources by setting a return code (see below).
 *
 * This API is intended to allow the scheduling of resources to jobs to
 * be extended beyond the capabilities provided by the sched_conf(5)
 * load_formula and queue_sort_method. This mechanism is not intended
 * to replace the resource management capabilities of Grid Engine such
 * as consumable resources or resource reservation. Instead, it is
 * integrated with these capabilities.
 *
 * The function is called for every job with a list of suitable candidate
 * queues and host in the qlist member of the pqs_params_t structure.
 * Queues or hosts which are not currently a candidate for the job
 * due to resource constraints, will not be in the list. The number of
 * available slots on the queue is specified in the available_slots
 * member of the pqs_queue_t structure. If multiple queues are defined
 * for a host, there could be multiple queues in the list for the same
 * host. Keep in mind that this function may be called many times
 * during a single scheduling interval. Performing I/O or communication
 * in this function will have a negative peformance impact on the
 * scheduler.
 *
 * The function should return one of the following values defined in
 * this header file:
 *
 *   PQS_ASSIGNED
 *     Schedule the job according to the queue sort order specified in
 *     the new_seqno member of the pqs_queue_t structure.
 *
 *   PQS_NO_ASSIGNMENT_AT_SPECIFIED_TIME
 *     Do not schedule this job at this time.
 *
 *   PQS_NO_ASSIGNMENT_FOR_JOB_CATEGORY
 *     Jobs of this category can never be scheduled. This allows the
 *     Grid Engine scheduler to not bother with scheduling any more
 *     jobs with the same resources requirements as the current job.
 *
 *   PQS_NO_ASSIGNMENT_FOR_JOB
 *     Do not schedule this job at any time. This means that the
 *     job cannot ever be scheduled on these resources.
 *
 * The dynamic library and function are specified in the qsort_args
 * attribute of the PE configuration.
 *
 * qsort_args  <lib-qsort.so> <qsort-function> [ <arg1> ... ]
 *
 *      <lib-qsort.so>          The path of the qsort dynamic library
 *      <qsort-function>        The name of the qsort function
 *      <arg1> ...              Arguments passed to the qsort function
 *
 * The qsort_args string is passed to the function in the qsort_args
 * member of the pqs_params_t structure. Substitutions from the hard
 * requested resource list for the job will be made for any strings
 * of the form $<resource>, where <resource> is the full name of the
 * resource as defined in complex(5) list. The requested value in the
 * hard resource request will be substituted. If <resource> is not
 * requested in the job, a null string will be substituted. This
 * mechanism is intended to allow per-job resource information to be
 * easily passed to the qsort function.
 *
 * If a PE called testpe has the qsort_args attribute,
 *
 *    $ qconf -sp testpe
 *    pe_name     testpe
 *    ...
 *    qsort_args  libmyqsort.so my_qsort $myresource
 *    ...
 *
 * and myresources is defined as a complex,
 *
 *    $ qconf -sc
 *    ...
 *    myresource  myresource  INT     <=   YES   NO   0   0
 *    ...
 *
 * and the resource is available on the appropriate queues
 *
 *    qconf -sq all.q
 *    ...
 *    pe_list           testpe
 *    complex_values    myresource=100
 *    ...
 *
 * and a parallel job is submitted,
 *
 *    $ qsub -pe testpe 8 -l myresource=5 myjob.sh
 *
 * then the qsort_args passed to the my_qsort function will contain:
 *
 *    qsort_args = "libmyqsort.so my_qsort 5"
 *
 * and the qsort_argv argument vector passed to the my_qsort
 * function will contain:
 *
 *    qsort_argv[0] = "libmyqsort.so"
 *    qsort_argv[1] = "my_qsort"
 *    qsort_argv[2] = "5"
 *    qsort_argv[3] = NULL
 *
 *
 * Here's a sample PE qsort function. It reverses the sort order.
 *
 * #include "sge_pqs_api.h"
 *
 * int
 * my_qsort(pqs_params_t *p)
 * {
 *    int i, seq_no=1000;
 *    for(i=0; i<p->num_queues; i++)
 *       p->qlist[i].new_seqno = seq_no--;
 *    return PQS_ASSIGNED;
 * }
 *
 * If this function is in a file called my_qsort.c, here's how to create
 * a shared library (libmyqsort.so) using gcc:
 *
 *    $ gcc -fPIC -c my_qsort.c
 *    $ gcc -shared -Wl,-soname,libmyqsort.so -o libmyqsort.so my_qsort.o -lc
 *
 * Copy the shared library into the $SGE_ROOT/lib/<arch> (or fully qualify
 * the path in the qsort_argv).
 *
 *    $ cp libmyqsort.so $SGE_ROOT/lib/`$SGE_ROOT/util/arch`
 *
 * To use, define the PE and submit a job using that PE.
 *
 * WARNING: If you're not an experienced software developer, you
 * probably should not attempt to use this API. :)
 *
 */

/*
 * The pqs_queue_t structure defines a queue/host resource which
 * is a candidate for scheduling the parallel job. The new sort
 * order should be set by setting new_seqno in ascending order.
 */

typedef struct {
   const char *queue_name;      /* queue name (input) */
   const char *host_name;       /* host name (input) */
   int  host_seqno;             /* host sequence number (input) */
   int  queue_seqno;            /* queue sequence number (input) */
   int  available_slots;        /* slots available on host/queue (input) */
   int  soft_violations;        /* number of soft resource request violations */
   int  new_seqno;              /* new sequence number for ordering (output) */
} pqs_queue_t;

/*
 * The pqs_params_t structure defines the parameters required
 * to sort the candidate queue/host list for a job.
 */

typedef struct {
   int job_id;                  /* job ID (input)                         */
   int task_id;                 /* task ID (input)                        */
   int slots;                   /* requested PE slots (input)             */
   int num_queues;              /* number of queues in qlist (input)      */
   pqs_queue_t *qlist;          /* array of schedulable queues (input)    */
   const char *pe_name;         /* PE name (input)                        */
   const char *pe_qsort_args;   /* PE qsort_args (input)                  */
   const char **pe_qsort_argv;  /* PE qsort_args converted to an          */
                                /*   argument vector with job environment */
                                /*   variable substitutions (input)       */
} pqs_params_t;

/*
 * The dynamic PE queue sort function
 */

typedef int (*pqs_qsort_t)(pqs_params_t *pqs_params, int verify, char *err_str, int err_str_size);

/*
 * Return codes for the function
 */

#define PQS_ASSIGNED 0
#define PQS_NO_ASSIGNMENT_AT_SPECIFIED_TIME 1
#define PQS_NO_ASSIGNMENT_FOR_JOB_CATEGORY -1
#define PQS_NO_ASSIGNMENT_FOR_JOB -2

#endif /* _SGE_PQS_API_H_ */

