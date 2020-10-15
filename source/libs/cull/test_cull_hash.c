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

/* #define HASH_STATISTICS */
#define XMALLINFO

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>

#ifdef MALLINFO
#include <malloc.h>
#endif

#include "basis_types.h"

#define __SGE_GDI_LIBRARY_HOME_OBJECT_FILE__
#include "cull/cull.h"

#include "uti/sge_profiling.h"

#ifdef TEST_USE_JOBL
#include "sge_jobL.h"
int NM_ULONG  = JB_job_number;
int NM_STRING = JB_owner;
lDescr *DESCR = JB_Type;

lNameSpace my_nmv[] = {
   {1, JBS, JBN },
   {0, 0, NULL}
};

#else
enum {
   TEST_ulong = 1,
   TEST_string
};

LISTDEF(TEST_Type)
   SGE_ULONG (TEST_ulong, CULL_DEFAULT)
   SGE_STRING (TEST_string, CULL_DEFAULT)
LISTEND

NAMEDEF(TEST_Name)
   NAME("TEST_ulong")
   NAME("TEST_string")
NAMEEND

#define TEST_Size sizeof(TEST_Name) / sizeof(char *)

int NM_ULONG  = TEST_ulong;
int NM_STRING = TEST_string;
lDescr *DESCR = TEST_Type;

lNameSpace my_nmv[] = {
   {1, TEST_Size, TEST_Name },
   {0, 0, NULL}
};
#endif

static void usage(const char *argv0) 
{
   fprintf(stderr, "usage: %s [<num_objects> <num_names> <uh> <nuh>]\n", argv0);
   fprintf(stderr, "<num_objects> = number of objects to be created\n");
   fprintf(stderr, "<num_names>   = number of entries in non unique hash\n");
   fprintf(stderr, "<uh>          = create unique hash\n");
   fprintf(stderr, "<nuh>         = create non unique hash\n");
   exit(EXIT_FAILURE);
}

static const char *random_string(int length)
{
   static char buf[1000];
   int i;

   for (i = 0; i < length; i++) {
      buf[i] = rand() % 26 + 64;
   }
   buf[i] = 0;

   return strdup(buf);
}

long clk_tck = 0;
const char **names = NULL;

const char *HEADER_FORMAT = "%s %s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s\n";
const char *DATA_FORMAT   = "%s %s %8.3lf %8.3lf %8.3lf %8.3lf %8.3lf %8.3lf %8.3lf (%6d) %8.3lf (%6d) %8ld\n";

static void do_test(bool unique_hash, bool non_unique_hash, 
                    int num_objects, int num_names)
{
   lList *lp = NULL;
   lList *copy;
   lListElem *ep;
   clock_t now, start;
   struct tms tms_buffer;
#ifdef MALLINFO
   struct mallinfo meminfo;
#endif
   int i;
#ifdef HASH_STATISTICS
   cull_htable cht;
   dstring stat_dstring = DSTRING_INIT;
#endif

   /* measurement data */
   double  prof_create,  /* create objects */
           prof_copy,    /* copy list including all hashtables */
           prof_rau,     /* random access by unique attrib */
           prof_inu,     /* iterate by non unique attrib */
           prof_curo,    /* change unique attrib of random object */
           prof_cnuro,   /* change non unique attrib of random object */
           prof_dru,     /* delete random object by unique attribute */
           prof_dinu;    /* delete by iterating over non unique attrib */
   int     objs_dru,     /* objects deleted by random unique */
           objs_dinu;    /* objects deleted by iterate non unique */

   /* create list and hash tables */
   lp = lCreateList("test list ", DESCR);
   if (unique_hash) {
      cull_hash_new(lp, NM_ULONG, true);
   }
   if (non_unique_hash) {
      cull_hash_new(lp, NM_STRING, false);
   }
#ifdef HASH_STATISTICS
   cht = lp->descr[1].ht;
   printf("%s\n", cull_hash_statistics(cht, &stat_dstring));
#endif
   start = times(&tms_buffer);

   /* TEST: build objects */
   for (i = 0; i < num_objects; i++) {
      ep = lAddElemUlong(&lp, NM_ULONG, i, DESCR);
      lSetString(ep, NM_STRING, names[rand() % num_names]);
   }
   /* measure time */
   now = times(&tms_buffer);
   prof_create = (now - start) * 1.0 / clk_tck;

#ifdef HASH_STATISTICS
   printf("%s\n", cull_hash_statistics(cht, &stat_dstring));
#endif


   /* measure memory usage */
#ifdef MALLINFO
   meminfo = mallinfo();
#endif
  
   /* TEST: copy list */
   start = times(&tms_buffer);
   copy = lCopyList("copy", lp);

   /* measure time */
   now = times(&tms_buffer);
   prof_copy = (now - start) * 1.0 / clk_tck;
   lFreeList(&copy);

   /* TEST: random access by unique attrib */
   start = times(&tms_buffer);
   for (i = 0; i < num_objects; i++) {
      ep = lGetElemUlong(lp, NM_ULONG, rand() % num_objects);
   }

   /* measure time */
   now = times(&tms_buffer);
   prof_rau = (now - start) * 1.0 / clk_tck;
   start = now;

   /* TEST: iterate by non unique attrib */
   for (i = 0; i < num_names; i++) {
      const void *iterator = NULL;
      lListElem *next_ep;

      next_ep = lGetElemStrFirst(lp, NM_STRING, names[i], &iterator);
      while ((ep = next_ep) != NULL) {
         next_ep = lGetElemStrNext(lp, NM_STRING, names[i], &iterator);
      }
   }

   /* measure time */
   now = times(&tms_buffer);
   prof_inu = (now - start) * 1.0 / clk_tck;
   start = now;

   /* TEST: change unique attribute of random object */
   /* measure time */
   for (i = 0; i < num_objects; i++) {
      /* object to access */
      int unique = rand() % num_objects;
      /* new value to set, make sure we stay unique */
      int newval = unique + num_objects;

      /* search object */
      ep = lGetElemUlong(lp, NM_ULONG, unique);

      /* set a new value */
      lSetUlong(ep, NM_ULONG, newval);

      /* restore old value */
      lSetUlong(ep, NM_ULONG, unique);
   }

   now = times(&tms_buffer);
   prof_curo = (now - start) * 1.0 / clk_tck;
   start = now;

   /* TEST: change non unique attribute of random object */
   for (i = 0; i < num_objects; i++) {
      /* search object */
      ep = lGetElemUlong(lp, NM_ULONG, rand() % num_objects);

      /* set a new value */
      lSetString(ep, NM_STRING, names[rand() % num_names]);
   }
   /* measure time */
   now = times(&tms_buffer);
   prof_cnuro = (now - start) * 1.0 / clk_tck;
   start = now;

   /* TEST: delete random object */
   objs_dru = 0;
   for (i = 0; i < num_objects / 2; i++) {
      /* search object */
      ep = lGetElemUlong(lp, NM_ULONG, rand() % num_objects);
      /* if same rand showed up earlier, object does no longer exist! */
      if (ep != NULL) {
         lRemoveElem(lp, &ep);
         objs_dru++;
      }
   }   
   /* measure time */
   now = times(&tms_buffer);
   prof_dru = (now - start) * 1.0 / clk_tck;
   start = now;

   /* TEST: delete all objects having the same non unique attribute */
   objs_dinu = 0;
   for (i = 0; i < num_names; i++) {
      const void *iterator = NULL;
      lListElem *next_ep;

      next_ep = lGetElemStrFirst(lp, NM_STRING, names[i], &iterator);
      while ((ep = next_ep) != NULL) {
         next_ep = lGetElemStrNext(lp, NM_STRING, names[i], &iterator);
         lRemoveElem(lp, &ep);
         objs_dinu++;
      }
   }

   /* measure time */
   now = times(&tms_buffer);
   prof_dinu = (now - start) * 1.0 / clk_tck;
   start = now;

   printf(DATA_FORMAT, 
          unique_hash ? " * " : "   ",
          non_unique_hash ? " * " : "   ",
          prof_create, 
          prof_copy, 
          prof_rau, prof_inu, 
          prof_curo, prof_cnuro,
          prof_dru, objs_dru, prof_dinu, objs_dinu,
#ifdef MALLINFO
          (meminfo.usmblks + meminfo.uordblks) / 1024
#else
          0L
#endif
          );

#ifdef HASH_STATISTICS
printf("%s\n", cull_hash_statistics(cht, &stat_dstring));
   sge_dstring_free(&stat_dstring);
#endif
   lFreeList(&lp);
}

int main(int argc, char *argv[])
{
   int num_objects;
   int num_names;
   int i;
   bool uh, nuh;

   if (argc != 1 && argc < 5) {
      usage(argv[0]);
   }

   /* initialize globals */
   lInit(my_nmv);
   clk_tck = sysconf(_SC_CLK_TCK); /* JG: TODO: sge_sysconf? */
   prof_mt_init();

   /* we need random numbers */
   srand(time(0));

   if (argc == 1) {
      num_objects = 1000;
      num_names   = 10;
      uh          = true;
      nuh         = true;
   } else {
      /* parse commandline options */
      num_objects = atoi(argv[1]);
      num_names   = atoi(argv[2]);
      uh          = atoi(argv[3]) == 0 ? false : true;
      nuh         = atoi(argv[4]) == 0 ? false : true;
   }

   /* create name array */
   names = sge_malloc (num_names * sizeof(const char *));

   /* build random names */
   for (i = 0; i < num_names; i++) {
      const char *name = random_string(10);
      names[i] = name;
   }

   /* output header */
   printf(HEADER_FORMAT, "uh ", "nuh", 
          "create", "copy", "rau", "inu", "curo", "cnuro", 
          "dru", "(objs)", "dinu", "(objs)", "mem(kB)");

   /* do tests */
   do_test(uh, nuh, num_objects, num_names);
   
   /* free names */
   for (i = 0; i < num_names; i++) {
      sge_free(&(names[i]));
   }

   return EXIT_SUCCESS;
}
