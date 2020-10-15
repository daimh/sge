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
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(SOLARIS64) || defined(SOLARISAMD64)
#   include <sys/pset.h>
#elif defined(ALPHA)
#   include <sys/processor.h>
#elif defined(__sgi)
#   include <sys/sysmp.h>
#   include <sys/prctl.h>
#   include <sys/schedctl.h>
#endif

#include "uti/sge_uidgid.h"
#include "uti/sge_nprocs.h"
#include "uti/config_file.h"
#include "uti/sge_stdio.h"

#include "sge_pset.h"
#include "basis_types.h"
#include "execution_states.h"
#include "err_trace.h"

#define PROC_SET_OK            0
#define PROC_SET_WARNING       1
#define PROC_SET_ERROR        -1
#define PROC_SET_BUSY         -2

#if defined(__sgi)
#   define SGE_MPSET_MULTIPLIER   100
#endif

#if defined(ALPHA)
typedef long sbv_t;
#elif defined(__sgi)
/* 
 * declaration of sbv_t from sys/runq_private.h
 * including sys/runq_private.h lead to compile errors.
 */
typedef unsigned long long sbv_t;
#endif

#if defined(ALPHA)
int destroy_pset(int pset_id, int number);
int create_pset(void);
void print_pset_error(int ret);
#endif

#if defined(__sgi) || defined(ALPHA) 
static int range2proc_vec(char *, sbv_t *, char *);
#endif


#if defined(__sgi) || defined(ALPHA) || defined(SOLARIS64) || defined(SOLARISAMD64)
static int free_processor_set(char *err_str);
static int set_processor_range(char *crange, int proc_set_num, char *err_str);
#endif

void sge_pset_create_processor_set(void) 
{
#if defined(__sgi) || defined(ALPHA) || defined(SOLARIS64) || defined(SOLARISAMD64)
   char err_str[2*SGE_PATH_MAX+128];

   /* SGI IRIX processor set stuff */
   if (strcasecmp("UNDEFINED",get_conf_val("processors"))) {
      int ret;

      errno = 0;
      if (sge_switch2start_user()) {
         shepherd_trace("can't switch user in sge_pset_create_processor_set");
         shepherd_state = SSTATE_PROCSET_NOTSET;
         shepherd_error(1, strerror(errno));
         return;
      }
      if ((ret=set_processor_range(get_conf_val("processors"),
                 (int) strtol(get_conf_val("job_id"), NULL, 10),
                 err_str)) != PROC_SET_OK) {
         sge_switch2admin_user();
         if (ret == PROC_SET_WARNING) /* not critical - e.g. not root */
            shepherd_trace("warning: processor set not set in set_processor_range");
         else { /* critical --> use err_str to indicate error */
            shepherd_trace("critical error in set_processor_range - bailing out");
            shepherd_state = SSTATE_PROCSET_NOTSET;
            shepherd_error(1, err_str);
         }
      } else {
         sge_switch2admin_user();
      }
   }
#endif

}

void sge_pset_free_processor_set(void)
{
#if defined(__sgi) || defined(ALPHA) || defined(SOLARIS64) || defined(SOLARISAMD64)
   /* SGI IRIX processor set stuff */
   if (strcasecmp("UNDEFINED",get_conf_val("processors"))) {
      char err_str[2*SGE_PATH_MAX+128];
      int ret;

      errno = 0;
      if (sge_switch2start_user()) {
         shepherd_trace("failed to switch user in free_processor_set: %s",
                        strerror(errno));
         shepherd_state = SSTATE_PROCSET_NOTFREED;
         shepherd_error(1, strerror(errno));
         return;
      }
      if ((ret=free_processor_set(err_str)) != PROC_SET_OK) {
         sge_switch2admin_user();
         switch (ret) {
         case PROC_SET_WARNING: /* not critical - e.g. not root */
            shepherd_trace("warning: processor set not freed in free_processor_set - "
                           "did no exist, probably");
            break;
         case PROC_SET_ERROR: /* critical - err_str indicates error */
            shepherd_trace("critical error in free_processor_set - bailing out");
            shepherd_state = SSTATE_PROCSET_NOTFREED;
            shepherd_error(1, err_str);
            break;
         case PROC_SET_BUSY: /* still processes running in processor set */
            shepherd_trace("error in releasing processor set");
            shepherd_state = SSTATE_PROCSET_NOTFREED;
            shepherd_error(1, err_str);
            break;
         default: /* should not occur */
            sprintf(err_str,
               "internal error after free_processor_set - ret=%d", ret);
            shepherd_state = SSTATE_PROCSET_NOTFREED;
            shepherd_error(1, err_str);
            break;
         }
      } else {
         sge_switch2admin_user();
      }
   }
#endif
}

#if defined(__sgi) || defined(ALPHA) || defined(SOLARIS64) || defined(SOLARISAMD64)  
/****** shepherd/pset/set_processor_range() ***********************************
*  NAME
*     set_processor_range() -- sets processor range according to string 
*
*  SYNOPSIS
*     int set_processor_range(char *crange, 
*                             int proc_set_num, 
*                             char *err_str) 
*
*  FUNCTION
*     Sets processor range according to string specification.
*     The unique processor set number will be stored in the file
*     "processor_set_number" located in the current working directory.
*
*     Format:
*        n|[n][-[m]],...  , n,m  being int >= 0.
*        no blanks are allowed in between (this is supposed to be 
*        handled by the queue configuration parsing routine)
*
*  INPUTS
*     char *crange     - String specifier of the range. Will be 
*                        modified via strtok internally.
*     int proc_set_num - The base for a unique processor set number.
*                        This number is already supposed to be unique.
*                        for the job (currently the job_id).
*                        set_processor_range() manipulates it to make
*                        sure that it is a unique processor set number. 
*     char *err_str    - The error message string to be used by the
*                        calling routine retuns value != PROC_SET_OK 
*                        Also used for trace messages internally.
*                        Passed to invoked subroutines.
*
*  RESULT
*     int - error state
*        PROC_SET_OK      - Ok
*        PROC_SET_ERROR   - A critical error occurred; either during 
*                           execution of sysmp() calls or as returned 
*                           from range2proc_vec().
*        PROC_SET_WARNING - A non-critical error occurred (e.g. the 
*                           procedure is executed as unpriveliged user)
******************************************************************************/
static int set_processor_range(char *crange, int proc_set_num, char *err_str) 
{
#if defined(__sgi) || defined(ALPHA) 
   int ret;
#endif
   FILE *fp;
#if defined(__sgi) || defined(ALPHA)
   sbv_t proc_vec;
#endif

#if defined(__sgi) || defined(ALPHA)
   if ((ret=range2proc_vec(crange, &proc_vec, err_str)))
      return ret;
#endif

#if defined(ALPHA)
   /* It is not possible to bind processor #0 to other psets than pset #0
    * So if we get a pset with #0 contained in the range we do nothing. 
    * The process gets not bound to a processor but it is guaranteed
    * that no other job will get processor #0 exclusively. It is upon 
    * the administrator to prevent overlapping of the psets in different
    * queues 
    */
   if (!(proc_vec & 1)) { /* processor #0 not contained */
      if ((proc_set_num = create_pset())<0) {
         print_pset_error(proc_set_num); /* prints error to stdout */
         shepherd_trace("MPPS_CREATE: failed to setup a new processor set");
         return PROC_SET_ERROR;
      }

      if (assign_cpu_to_pset(proc_vec, proc_set_num, 0)<0) {
         print_pset_error(proc_set_num); /* prints error to stdout */
         shepherd_trace("MPPS_CREATE: failed assigning processors to processor set");
         return PROC_SET_ERROR;
      }
   } else {
      /* use default pset (id #0) */
      proc_set_num = 0;
   }
#elif defined(SOLARIS64) || defined(SOLARISAMD64)
   /*
    * We do not create a processor set here
    * The system administrator is responsible to do this
    * We read one id from crange. This is the processor-set id we should use.
    */
   if (crange) {
      char *tok, *next;

      if ((tok=strtok(crange, " \t\n"))) {
         proc_set_num = (int) strtol(tok, &next, 10);
         if (next == tok) {
            sprintf(err_str, "wrong processor set id format: %20.20s", crange);
            shepherd_trace(err_str);
            return PROC_SET_ERROR;
         }
      } 
   }
#endif

   /* dump to file for later use */
   if ((fp = fopen("processor_set_number","w"))) {
      fprintf(fp,"%d\n",proc_set_num);
      FCLOSE(fp);
   } else {
      shepherd_trace("MPPS_CREATE: failed creating file processor_set_number");
      return PROC_SET_ERROR;
   }

#if defined(ALPHA)
   /* Now let's assign ourselves to the previously created processor set */
   if (proc_set_num) {
      pid_t pid_list[1];
      pid_list[0] = getpid();
      if (assign_pid_to_pset(pid_list, 1, proc_set_num, PSET_EXCLUSIVE)<0) {
         print_pset_error(proc_set_num); /* prints error to stdout */
         shepherd_trace("MPPS_CREATE: failed assigning processors to processor set");
         return PROC_SET_ERROR;
      }
   }
#elif defined(SOLARIS64) || defined(SOLARISAMD64)
   if (proc_set_num) {
      int local_ret;

      sprintf(err_str,"pset_bind: try to use processorset %d", proc_set_num);
      shepherd_trace(err_str);
      if (pset_bind(proc_set_num, P_PID, P_MYID, NULL)) {
         switch (errno) {
         case EFAULT:
            shepherd_trace("pset_bind: The location pointed to by opset was not"
               " NULL and not writable by the user");
            local_ret = PROC_SET_ERROR;
            break;
         case EINVAL:
            shepherd_trace("pset_bind: invalid processor set was specified");
            local_ret = PROC_SET_ERROR;
            break;
         case EPERM:
            shepherd_trace("pset_bind: The effective user of the calling "
               "process is not super-user");
            local_ret = PROC_SET_ERROR;
            break;
         default:
            sprintf(err_str,"pset_bind: unexpected error - errno=%d", errno);
            shepherd_trace(err_str);
            local_ret = PROC_SET_ERROR;
            break;
         }
         return local_ret;
      }
   }
#endif

   return PROC_SET_OK;
FCLOSE_ERROR:
   shepherd_trace("MPPS_CREATE: failed creating file processor_set_number");
   return PROC_SET_ERROR;
}



/****** shepherd/pset/free_processor_set() ************************************
*  NAME
*     free_processor_set() -- Release the previously occupied proc set. 
*
*  SYNOPSIS
*     int free_processor_set(char *err_str) 
*
*  FUNCTION
*     Release the previously occupied processor set. The unique 
*     processor set number is read from the file "processor_set_number"
*     which has to be located in the current working directory.
*
*  INPUTS
*     char *err_str - The error message string to be used by the calling
*                     routine if return value != PROC_SET_OK. Also used
*                     for trace messages internally 
*
*  RESULT
*     int - Error state
*        PROC_SET_OK      - Ok
*        PROC_SET_BUSY    - The processor set is still in use, i.e.
*                           processes originating from the job have not 
*                           finished.
*        PROC_SET_ERROR   - A critical error occurred. During execution
*                           of sysmp() calls.
*        PROC_SET_WARNING - A non-critical error occurred (e.g. the
*                           procedure is executed as unpriviliged user)
******************************************************************************/
static int free_processor_set(char *err_str) 
{
   FILE *fp;
   int proc_set_num;

   /* read unique processor set number from file */
   if ((fp = fopen("processor_set_number","r"))) {
      fscanf(fp, "%d", &proc_set_num);
      FCLOSE_IGNORE_ERROR(fp);
   } else {
      shepherd_trace("MPPS_CREATE: failed reading from file processor_set_number");
      return PROC_SET_ERROR;
   }

#if defined(ALPHA)
   if (proc_set_num) {
      int ret;
      pid_t pid_list[1];
      pid_list[0] = getpid();

      /* assign shepherd back to default processor set */
      if ((ret=assign_pid_to_pset(pid_list, 1, 0, 0))<0) {
         print_pset_error(ret); /* prints error to stdout */
         shepherd_trace("MPPS_CREATE: failed assigning processors to processor set");
         return PROC_SET_ERROR;
      }

      if ((ret = destroy_pset(proc_set_num, 1))==PROCESSOR_SET_ACTIVE) {
         print_pset_error(ret);
         shepherd_trace("MPPS_CREATE: failed assigning processors to processor set");
         return PROC_SET_ERROR;
      }
   }
#elif defined(SOLARIS64) || defined(SOLARISAMD64)
   /*
    * We do not release a processor set here
    * The system administrator is responsible to do this
    */
#endif
   return PROC_SET_OK;
}

#endif 

#if defined(__sgi) || defined(ALPHA) 
/****** shepherd/pset/range2proc_vec() ****************************************
*  NAME
*     range2proc_vec() -- Computes bit vector (prcessor set spec.)
*
*  SYNOPSIS
*     static int range2proc_vec(char *crange, sbv_t *proc_vec, char *err_str) 
*
*  FUNCTION
*     Computes bit vector with bits set corresponding to string 
*     specification of processor range. 
*
*  INPUTS
*     char *crange    - String specifier of the range. Will be modified
*                       internally. Format:
*                          n|[n][-[m]],...  , n,m  being int >= 0.
*                          no blanks are allowed in between 
*     sbv_t *proc_vec - a bit vector of type sbv_t with all bits set
*                       contained in the range description and all
*                       other bits zero. 
*     char *err_str   - The error message string to be used by the
*                       calling routine if return value != PROC_SET_OK.
*                       Also used for trace messages internally. 
*
*  RESULT
*     static int - Error state
*        PROC_SET_OK    - Ok
*        PROC_SET_ERROR - Invalid range value in range description.
******************************************************************************/
static int range2proc_vec(char *crange, sbv_t *proc_vec, char *err_str) 
{
   char *tok, *next, *p=crange;
   int min, max, i;
   int dash_used;
   int sbvlen;

   *proc_vec = (sbv_t) 0;

   /* compute max number of processors and thus significant length of
    * proc_vec
    */
   sbvlen = sge_nprocs() - 1; /* LSB corresponds to proc. 0 */

   /* loop trough range string with "," as token delimiter
    * Set processor vector for each token = range definition element.
    */
   while ((tok=strtok(p,","))) {
      if (p) p=NULL;
      
      /* for each token parse range, i.e. find
       * whether min or max value is set and whether a "-" sign
       * was used
       */
      min = -1;
      max = -1;
      dash_used = 0;
      while (*tok) {
         next = NULL;
         if (*tok == '-') {
            dash_used = 1;
            if (min == -1)
               min = 0;
         } else { /* should be a number */
            if (min == -1 && !dash_used ) {
               min = (int) strtol(tok, &next, 10);
               if (next == tok || min < 0 || min > sbvlen) {
                  sprintf(err_str, "range2proc_vec: wrong processor range format: %20.20s", crange);
                  shepherd_trace(err_str);
                  return PROC_SET_ERROR;
               }
            } else if (max == -1 && dash_used ) {
               max = (int) strtol(tok, &next, 10);
               if (next == tok || max < 0 || max > sbvlen) {
                  sprintf(err_str, "range2proc_vec: wrong processor range format: %20.20s", crange);
                  shepherd_trace(err_str);
                  return PROC_SET_ERROR;
               }
            }
         }

         /* proceed either by one char in case of a "-" or by the
          * width of the number field
          */
         if (next)
            tok = next;
         else
            tok++;
      }

      /* fill out full range specification "n-m" according to findings */
      if (!dash_used )
         max = min;
      else {
         if (min == -1) min = 0;
         if (max == -1) max = sbvlen;
      }

      /* set processor vector as defined by range specification */
      for(i=min; i<=max; i++)
         *proc_vec |= (sbv_t) 1<<i;
   }

   return PROC_SET_OK;
}

#endif


