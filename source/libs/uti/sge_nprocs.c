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
#include "msg_utilib.h"

#include <unistd.h>

#if defined(DARWIN)
#   include <mach/host_info.h>
#   include <mach/mach_host.h>
#   include <mach/mach_init.h>
#   include <mach/machine.h>
#endif

#if defined(__sgi)
#   include <sys/types.h>
#   include <sys/sysmp.h>
#endif

#if defined(ALPHA)
#   include <sys/sysinfo.h>
#   include <machine/hal_sysinfo.h>
#endif

#if defined(__hpux)
    /* needs to be copiled with std C compiler ==> no -Aa switch no gcc */
#   include <stdlib.h>
#   include <sys/pstat.h>
#endif

#if defined(FREEBSD)
#   include <sys/types.h>
#   include <sys/sysctl.h>
#endif

#ifdef NPROCS_TEST
#   include <stdio.h>
#endif

#ifndef NPROCS_TEST
#include "sge_os.h"
#else 
int sge_nprocs (void);
#endif

/****** uti/os/sge_nprocs() ***************************************************
*  NAME
*     sge_nprocs() -- Number of processors in this machine 
*
*  SYNOPSIS
*     int sge_nprocs() 
*
*  FUNCTION
*     Use this function to get the number of processors in 
*     this machine 
*
*  RESULT
*     int - number of procs
* 
*  NOTES
*     MT-NOTE: sge_nprocs() is MT safe (SOLARIS, NEC, IRIX, ALPHA, HPUX, LINUX)
******************************************************************************/
int sge_nprocs()
{
   int nprocs=1; /* default */


#if defined(_SC_NPROCESSORS_ONLN) /* Solaris, AIX, Linux, NetBSD */
   nprocs = sysconf(_SC_NPROCESSORS_ONLN);
#endif

#if defined(DARWIN)
  struct host_basic_info cpu_load_data;

  mach_msg_type_number_t host_count = sizeof(cpu_load_data)/sizeof(integer_t);
  mach_port_t host_priv_port = mach_host_self();

  host_info(host_priv_port, HOST_BASIC_INFO , (host_info_t)&cpu_load_data, &host_count);

  nprocs =  cpu_load_data.avail_cpus;

#endif


#ifdef __sgi
   nprocs = sysmp(MP_NPROCS);
#endif

#if defined(ALPHA)
   int start=0;

   getsysinfo(GSI_CPUS_IN_BOX,(char*)&nprocs,sizeof(nprocs),&start);
#endif

#if defined(__hpux)
   union pstun pstatbuf;
   struct pst_dynamic dinfo;

   pstatbuf.pst_dynamic = &dinfo;
   if (pstat(PSTAT_DYNAMIC,pstatbuf,sizeof(dinfo),NULL,NULL)==-1) {
          perror(MSG_PERROR_PSTATDYNAMIC);
          exit(1);
   }
   nprocs = dinfo.psd_proc_cnt;
#endif

#if defined(FREEBSD)
   size_t nprocs_len = sizeof(nprocs);

   if (sysctlbyname("hw.ncpu", &nprocs, &nprocs_len, NULL, 0) == -1) {
      nprocs = -1;
   }
#endif

#if defined(INTERIX)
/* TODO: HP: don't set nprocs==-1 to 0, overwrite it with value from
 *       external load sensor.
 */
   nprocs = -1;
#endif

   if (nprocs <= 0) {
      nprocs = 1;
   }

   return nprocs;
}

#ifdef NPROCS_TEST
void main(
int argc,
char **argv 
) {
   printf(MSG_INFO_NUMBOFPROCESSORS_I, sge_nprocs());
   printf("\n");
   exit(0);
}
#endif
