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
 *  Portions of this code are Copyright 2011 Univa Inc.
 *   Copyright (C) 2011 Dave Love, University of Liverpool
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef WINDOWS
#include <unistd.h>

#include "uti/sge_rmon.h"
#include "uti/sge_os.h"
#include "uti/sge_language.h"
#include "uti/sge_prog.h"
#include "uti/sge_binding_hlp.h"
#include "uti/sge_string.h"

#include "sgeobj/sge_host.h"
#include "sgeobj/sge_binding.h"
#include "binding_support.h"

#include "basis_types.h"
#include "msg_utilbin.h"
#else
#include "msg_utilbin.h"
#include <windows.h>
#include <io.h>
#endif

/* In POSIX, but possibly not on all relevant platforms -- let's see.  */
#ifndef WINDOWS
#include <sys/utsname.h>

/* no dstring with Windows native compiler */
void fill_socket_core_topology(dstring* msocket, dstring* mcore, dstring* mthread, dstring* mtopology);
#endif

void usage(void);
void print_mem_load(char *, char *, int, double, char*);
void check_core_binding(void);

void test_binding(void);

static bool can_bind, has_topology;

void usage()
{
   fprintf(stderr, "%s loadcheck [-cb] | [-int] [-loadval name]\n", MSG_UTILBIN_USAGE);
   fprintf(stderr, "\t\t[-cb] \t\t\t\t Shows core binding and processor topology related information.\n");
   fprintf(stderr, "\t\t[-int]\t\t\t\t Print as integer\n");
   fprintf(stderr, "\t\t[-loadval]\t\t\t Select specific load value\n");
   exit(1);
}
   
int main(int argc, char *argv[])
{
   double avg[3];
   int loads;
   char *name = NULL;
#ifndef WINDOWS
   dstring msocket   = DSTRING_INIT;
   dstring mcore     = DSTRING_INIT;
   dstring mthread   = DSTRING_INIT;
   dstring mtopology = DSTRING_INIT;
#endif	/* WINDOWS */

#ifdef SGE_LOADMEM
   sge_mem_info_t mem_info;
#endif

#ifdef SGE_LOADCPU
   double total = 0.0;
#endif

   int i, pos = 0, print_as_int = 0, precision = 0, core_binding = 0;
   char *m = "";

#ifndef WINDOWS
   DENTER_MAIN(TOP_LAYER, "loadcheck");
#endif

#ifdef __SGE_COMPILE_WITH_GETTEXT__   
   /* init language output for gettext() , it will use the right language */
   sge_init_language_func((gettext_func_type)        gettext,
                         (setlocale_func_type)      setlocale,
                         (bindtextdomain_func_type) bindtextdomain,
                         (textdomain_func_type)     textdomain);
   sge_init_language(NULL,NULL);   
#endif /* __SGE_COMPILE_WITH_GETTEXT__  */
#ifndef WINDOWS
   can_bind = has_core_binding();
   has_topology = has_topology_information();
#endif
   if (argc == 2 && !strcmp(argv[1], "-cb")) {
      core_binding = 1;
   } else {
      for (i = 1; i < argc;) {
         if (!strcmp(argv[i], "-int"))
            print_as_int = 1;
         else if (!strcmp(argv[i], "-loadval")) {
            if (i + 1 < argc)
               pos=i+1;
            else
               usage();
            i++;
         }
         else
            usage();
         i++;
      }
   }   
   
   if (core_binding) {
#ifndef WINDOWS
      check_core_binding();
      DEXIT;
#endif
      return 1;
   } else if (print_as_int) {
      m = "";
      precision = 0;
   }   
   else {
      m = "M";
      precision = 6;
   }   

   if ((pos && !strcmp("arch", argv[pos])) || !pos) {
      const char *arch = "";
#if defined(WINDOWS)
      arch = "win32-x86";
#else
      arch = sge_get_arch();
#endif 
      printf("arch            %s\n", arch);
   }
      
   if ((pos && !strcmp("num_proc", argv[pos])) || !pos) {
      int nprocs = 1;
#if defined(WINDOWS)
      SYSTEM_INFO system_info;
      char        buf[100];

      GetSystemInfo(&system_info);
      nprocs = system_info.dwNumberOfProcessors;
      sprintf(buf, "num_proc        %d\n", nprocs);
      fflush(stdout);
      _write(1, (const void*)buf, (unsigned int)strlen(buf));
      _write(1, (const void*)"\0x0a", (unsigned int)1);
#else
      nprocs = sge_nprocs();
      printf("num_proc        %d\n", nprocs);
#endif	/* WINDOWS */
   }

#ifndef WINDOWS
   if (true == has_topology) {
      fill_socket_core_topology(&msocket, &mcore, &mthread, &mtopology);
      if ((pos && !strcmp("m_socket", argv[pos])) || !pos) {
        printf("m_socket        %s\n", sge_dstring_get_string(&msocket));
      }
      if ((pos && !strcmp("m_core", argv[pos])) || !pos) {
        printf("m_core          %s\n", sge_dstring_get_string(&mcore));
      }
      if ((pos && !strcmp("m_thread", argv[pos])) || !pos) {
        printf("m_thread        %s\n", sge_dstring_get_string(&mthread));
      }
      if ((pos && !strcmp("m_topology", argv[pos])) || !pos) {
        printf("m_topology      %s\n", sge_dstring_get_string(&mtopology));
      }
   }
   else
#endif	/* !WINDOWS */
     {
      if ((pos && !strcmp("m_socket", argv[pos])) || !pos) {
        printf("m_socket        -\n");
      }
      if ((pos && !strcmp("m_core", argv[pos])) || !pos) {
        printf("m_core          -\n");
      }
      if ((pos && !strcmp("m_thread", argv[pos])) || !pos) {
        printf("m_thread        -\n");
      }
      if ((pos && !strcmp("m_topology", argv[pos])) || !pos) {
        printf("m_topology      -\n");
      }   
   }

#if defined(WINDOWS)
   loads = 0;
   avg[0] = avg[1] = avg[2] = 0;
#else
   loads = sge_getloadavg(avg, 3);
#endif

   if (loads>0 && ((pos && !strcmp("load_short", argv[pos])) || !pos)) 
      printf("load_short      %.2f\n", avg[0]);
   if (loads>1 && ((pos && !strcmp("load_medium", argv[pos])) || !pos)) 
      printf("load_medium     %.2f\n", avg[1]);
   if (loads>2 && ((pos && !strcmp("load_long", argv[pos])) || !pos))
      printf("load_long       %.2f\n", avg[2]);
      
   if (pos)
      name = argv[pos];
   else
      name = NULL;

#ifdef SGE_LOADMEM
   /* memory load report */
   memset(&mem_info, 0, sizeof(sge_mem_info_t));
   if (sge_loadmem(&mem_info)) {
      fprintf(stderr, "%s\n", MSG_SYSTEM_RETMEMORYINDICESFAILED);
#ifndef WINDOWS
      DEXIT;
      sge_dstring_free(&mcore);
      sge_dstring_free(&msocket);
      sge_dstring_free(&mthread);
      sge_dstring_free(&mtopology);
#endif
      return 1;
   }

   print_mem_load(LOAD_ATTR_MEM_FREE, name, precision, mem_info.mem_free, m); 
   print_mem_load(LOAD_ATTR_SWAP_FREE, name, precision, mem_info.swap_free, m); 
   print_mem_load(LOAD_ATTR_VIRTUAL_FREE, name, precision, mem_info.mem_free  + mem_info.swap_free, m); 

   print_mem_load(LOAD_ATTR_MEM_TOTAL, name, precision, mem_info.mem_total, m); 
   print_mem_load(LOAD_ATTR_SWAP_TOTAL, name, precision, mem_info.swap_total, m); 
   print_mem_load(LOAD_ATTR_VIRTUAL_TOTAL, name, precision, mem_info.mem_total + mem_info.swap_total, m);

   print_mem_load(LOAD_ATTR_MEM_USED, name, precision, mem_info.mem_total - mem_info.mem_free, m); 
   print_mem_load(LOAD_ATTR_SWAP_USED, name, precision, mem_info.swap_total - mem_info.swap_free, m); 
   print_mem_load(LOAD_ATTR_VIRTUAL_USED, name, precision,(mem_info.mem_total + mem_info.swap_total) - 
                                          (mem_info.mem_free  + mem_info.swap_free), m); 
#  ifdef IRIX
   print_mem_load(LOAD_ATTR_SWAP_USED, name, precision, mem_info.swap_rsvd, m); 
#  endif
#endif /* SGE_LOADMEM */

#ifdef SGE_LOADCPU
   loads = sge_getcpuload(&total);
   sleep(1);
   loads = sge_getcpuload(&total);

   if (loads != -1) {
      print_mem_load("cpu", name,  1, total, "%");
   }
#endif /* SGE_LOADCPU */
#ifndef WINDOWS
   DEXIT;
   sge_dstring_free(&mcore);
   sge_dstring_free(&msocket);
   sge_dstring_free(&mthread);
   sge_dstring_free(&mtopology);
#endif	/* WINDOWS */
   return 0;
}

void print_mem_load(
char *name,
char *thisone,
int precision,
double value,
char *m 
) {

   if ((thisone && !strcmp(name, thisone)) || !thisone)
      printf("%-15s %.*f%s\n", name, precision, value, m);
}

#ifndef WINDOWS
/****** loadcheck/check_core_binding() *****************************************
*  NAME
*     check_core_binding() -- Checks core binding functionality on current host. 
*
*  SYNOPSIS
*     void check_core_binding() 
*
*  FUNCTION
*     Checks core binding functionality on current host. 
*
*  INPUTS
*
*  RESULT
*     void - No result
*
*******************************************************************************/
void check_core_binding()
{
  if (HAVE_HWLOC) {
     printf("Your SGE has core binding functionality (built with hwloc).\n");
     test_binding();
  } else {
     printf("Your SGE has no core binding functionality (not built with hwloc).\n");
  }
}

void test_binding()
{
   dstring error  = DSTRING_INIT;
   char* topology = NULL;
   int length     = 0;
   int s, c;
   struct utsname name;

   if (uname(&name) != -1) {
      printf("Your kernel version is: %s\n", name.release);
   }

   if (!has_core_binding()) {
      printf("Your system seems not to offer core binding capabilities!");
   }

   if (!has_topology_information()) {
      printf("No topology information could by retrieved!\n");
   } else {
      /* get number of sockets */
      printf("Number of sockets:\t\t%d\n", get_number_of_sockets());
      /* get number of cores   */
      printf("Number of cores:\t\t%d\n", get_total_number_of_cores());
      /* the number of threads must be shown as well */
      printf("Number of threads:\t\t%d\n", get_total_number_of_threads());
      /* get topology */
      get_topology(&topology, &length);
      printf("Topology:\t\t\t%s\n", topology);
      sge_free(&topology); 
      printf("Mapping of logical socket and core numbers to internal\n");

      /* for each socket,core pair get the internal processor number */
      /* try multi-mapping */
      for (s = 0; s < get_number_of_sockets(); s++) {
         for (c = 0; c < get_number_of_cores(s); c++) {
            int* proc_ids  = NULL;
            int amount     = 0;
            if (get_processor_ids(s, c, &proc_ids, &amount)) {
               int i = 0;
               printf("Internal processor ids for socket %5d core %5d: ", s , c);
               for (i = 0; i < amount; i++) {
                  printf(" %5d", proc_ids[i]);
               }
               printf("\n");
               sge_free(&proc_ids);
            } else {
               printf("Couldn't get processor ids for socket %5d core %5d\n", s, c);
            }
         }
      }
   }   

   sge_dstring_free(&error);

   return;
}

/****** loadcheck/fill_socket_core_topology() **********************************
*  NAME
*     fill_socket_core_topology() -- Get load values regarding processor topology. 
*
*  SYNOPSIS
*     void fill_socket_core_topology(dstring* msocket, dstring* mcore, dstring* 
*     mtopology) 
*
*  FUNCTION
*     Gets the values regarding processor topology. 
*
*  OUTPUTS 
*     dstring* msocket   - The number of sockets the host have.
*     dstring* mcore     - The number of cores the host have.
*     dstring* mtopology - The topology the host have. 
*
*  RESULT
*     void - nothing 
*
*******************************************************************************/
void fill_socket_core_topology(dstring* msocket, dstring* mcore, dstring* mthread, dstring* mtopology)
{
   int ms, mc, mt;
   char* topo = NULL;
   int length = 0;

   ms = get_number_of_sockets();
   mc = get_total_number_of_cores();
   mt = get_total_number_of_threads();
   if (!get_topology(&topo, &length) || topo == NULL) {
      topo = sge_strdup(NULL, "-");
   }
   sge_dstring_sprintf(msocket, "%d", ms);
   sge_dstring_sprintf(mcore, "%d", mc);
   sge_dstring_sprintf(mthread, "%d", mt);
   sge_dstring_append(mtopology, topo);
   sge_free(&topo);
}
#endif	/* WINDOWS */
