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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "uti/sge_os.h"
#include "uti/sge_string.h"
#include "uti/sge_unistd.h"

#include "gdi/sge_gdi2.h"

volatile int should_stop = 0;

/* cpu load profile */
int working_time = 100;
int nproc = 1;

/* mem load profile */
unsigned long stack_mb = 0;
unsigned long stack_kb = 0;

unsigned long malloc_mb = 0;
unsigned long malloc_kb = 0;

/* io load profile */
int in = -1;
int out = -1;

/* cheat profile */   
int block_sigxcpu = 0;
int block_sigxfsz = 0;
int change_pgrp = 0;
int do_daemonize = 0;

/* options */

void work(char *mmp);
static void reserve_stack_mem_mb(int n);
static void reserve_stack_mem_kb(int n);


static void usage(int exit_code);
void stop_work(int signo);

static void usage(int exit_code)
{
   FILE *fp;

   if (exit_code)
      fp = stderr;
   else 
      fp = stdout;

   fprintf(fp, "usage:\n");
   fprintf(fp, "work [options]\n");
   fprintf(fp, "  options are:\n");
   fprintf(fp, "cpu\n");
   fprintf(fp, "   [-f n]            do work 'n' times using 'n' processes, default 1 process\n");
   fprintf(fp, "   [-w n]            work 'n' seconds, default 100 seconds\n");

   fprintf(fp, "io\n");
   fprintf(fp, "   [-in fname]       read from <fname>\n");
   fprintf(fp, "   [-out fname]      write to <fname>\n");

   fprintf(fp, "mem\n");
   fprintf(fp, "   [-stackK n]       force stack size with at least 'n' KB\n");
   fprintf(fp, "   [-stackM n]       force stack size with at least 'n' MB\n");
   fprintf(fp, "   [-mallocK n]      force malloc() with 'n' KB\n");
   fprintf(fp, "   [-mallocM n]      force malloc() with 'n' MB\n");
   fprintf(fp, "cheat options:\n");
   fprintf(fp, "   [-block_sigxcpu]  ignore SIGXCPU\n");
   fprintf(fp, "   [-block_sigxfsz]  ignore SIGXFSZ\n");
   fprintf(fp, "   [-change_pgrp]    change process group\n");  
   fprintf(fp, "   [-daemonize]      daemonize\n");

   exit(exit_code);
}

int main(int argc, char *argv[])
{
   int i;
   void *kmp = NULL, *mmp = NULL; /* pointers for mallocing */

   while (argc>1 && argv[1][0] == '-') {
      /* ---------------------- cpu load cmd line options ------- */
      if (!strcmp(argv[1], "-w")) {
         argc--;
         argv++;
         if (argc== 1 || sscanf(argv[1], "%d", &working_time)!=1)
            usage(1);
   
/*          fprintf(stderr, "-w = %d\n", working_time); */
         argc--;
         argv++;
         continue;
      }
      if (!strcmp(argv[1], "-f")) {
         argc--;
         argv++;

         if (argc== 1 || sscanf(argv[1], "%d", &nproc)!=1)
            usage(1);
/*          fprintf(stderr, "-f = %d\n", nproc); */
         argc--;
         argv++;
         continue;
      }
      /* ---------------------- cheat cmd line options ------- */
      if (!strcmp(argv[1], "-block_sigxcpu")) {
         block_sigxcpu = 1;
         argc--;
         argv++;
         continue;
      }
      if (!strcmp(argv[1], "-block_sigxfsz")) {
         block_sigxfsz = 1;
         argc--;
         argv++;
         continue;
      }
      if (!strcmp(argv[1], "-change_pgrp")) {
         change_pgrp = 1;
         argc--;
         argv++;
         continue;
      }
      if (!strcmp(argv[1], "-daemonize")) {
         do_daemonize = 1;
         argc--;
         argv++;
         continue;
      }

      /* ---------------------- malloc cmd line options ------- */
      if (!strcmp(argv[1], "-mallocM")) {
         argc--;
         argv++;
         if (argc== 1 || sscanf(argv[1], "%ld", &malloc_mb)!=1)
            usage(1);
         argc--;
         argv++;
         continue;
      }

      if (!strcmp(argv[1], "-mallocK")) {
         argc--;
         argv++;
         if (argc== 1 || sscanf(argv[1], "%ld", &malloc_kb)!=1)
            usage(1);
         argc--;
         argv++;
         continue;
      }

      /* ---------------------- io cmd line options ------- */
      if (!strcmp(argv[1], "-in")) {
         argc--;
         argv++;
         if (argc==1)
            usage(1);
         if ((in = open(argv[1], O_RDONLY, 0644))<0) {
            fprintf(stderr, "cannot read from \"%s\": %s\n", 
               argv[1], strerror(errno));
         }
         argc--;
         argv++;
         continue;
      }

      if (!strcmp(argv[1], "-out")) {
         argc--;
         argv++;
         if (argc== 1)
            usage(1);
         if ((in = open(argv[1], O_CREAT|O_APPEND|O_WRONLY, 0644))<0) {
            fprintf(stderr, "cannot write to \"%s\": %s\n", 
               argv[1], strerror(errno));
         }
         argc--;
         argv++;
         continue;
      }

      /* ---------------------- stack cmd line options ------- */
      if (!strcmp(argv[1], "-stackM")) {
         argc--;
         argv++;
         if (argc== 1 || sscanf(argv[1], "%ld", &stack_mb)!=1)
            usage(1);
         argc--;
         argv++;
         continue;
      }

      if (!strcmp(argv[1], "-stackK")) {
         argc--;
         argv++;
         if (argc== 1 || sscanf(argv[1], "%ld", &stack_kb)!=1)
            usage(1);
         argc--;
         argv++;
         continue;
      }
      if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-help")) {
         usage(0);
      }
      fprintf(stderr, "unknown option \"%s\"\n", argv[1]);
      usage(1);
   }

#ifdef SIGXFSZ
   if (block_sigxfsz) {
      sigset_t sigset;
      sigemptyset(&sigset);
      sigaddset(&sigset, SIGXFSZ);
      sigprocmask(SIG_BLOCK, &sigset, NULL);
      printf("blocking signal SIGXFSZ\n");
      fflush(stdout);
   }
#endif
#ifdef SIGXCPU
   if (block_sigxcpu) {
      sigset_t sigset;
      sigemptyset(&sigset);
      sigaddset(&sigset, SIGXCPU);
      sigprocmask(SIG_BLOCK, &sigset, NULL);
      printf("blocking signal SIGXCPU\n");
      fflush(stdout);
   }
#endif
   
   if (change_pgrp) {
      u_long old_pgrp, new_pgrp;
      old_pgrp = GETPGRP;
      SETPGRP;
      new_pgrp = GETPGRP;
      printf("changed pgrp from %d to %d\n", 
         (int)old_pgrp, (int)new_pgrp);
      fflush(stdout);
   }

   if (do_daemonize) {
      int keep_open[3];
      dup2(0, 3);
      dup2(1, 4);
      dup2(2, 5);
      keep_open[0] = 3;
      keep_open[1] = 4;
      keep_open[2] = 5;

      sge_daemonize(keep_open, 3, NULL);
      dup2(3, 0);
      dup2(4, 1);
      dup2(5, 2);
   }

   if (malloc_kb) {
      printf("going to allocate %ld kb via malloc\n", malloc_kb); fflush(stdout);
      if (!(kmp = malloc(1024*malloc_kb))) {
         printf("failed getting %ld kb mem via malloc\n", malloc_kb); fflush(stdout);
         return 1;
      } else
         printf("got %ld kb mem via malloc\n", malloc_kb); fflush(stdout);
   }
   if (malloc_mb) { 
      printf("going to allocate %ld mb via malloc\n", malloc_mb); fflush(stdout);
      if (!(mmp = malloc(1024*1024*malloc_mb))) {
         printf("failed getting %ld mb mem via malloc\n", malloc_mb); fflush(stdout);
         return 1;
      } else 
         printf("got %ld mb mem via malloc\n", malloc_mb); fflush(stdout);
   }

   if (stack_kb) {
      printf("going to allocate %ld kb on stack ", stack_kb); fflush(stdout);
      reserve_stack_mem_kb(stack_kb);
      printf("got %ld kb mem on stack\n", stack_kb); fflush(stdout);
   }
   if (stack_mb) {
      printf("going to allocate %ld mb on stack ", stack_mb); fflush(stdout);
      reserve_stack_mem_mb(stack_mb);
      printf("got %ld mb mem on stack\n", stack_mb); fflush(stdout);
   }


   fprintf(stderr, "Forking %d times.\n", nproc-1);
   /* fork nproc-1 times */
   if (nproc) {
      for (i=1; i<nproc && fork()>0; i++)
         ;
   }

   if (working_time) {
      /* install signal handler for SIGALRM */
      signal(SIGALRM, stop_work); 
   /*       fprintf(stderr, "Working for %d seconds.\n", working_time); */
      alarm(working_time);
      work(mmp);
   }
  
   if (in>=0)
      close(in);
   if (out>=0)
      close(out);
   if (kmp)
      sge_free(&kmp);
   if (mmp)
      sge_free(&mmp);

   return 0;
}

void work(char *mmp) {
   float a = 0, b = 0.1;
   char c;
   char *miter = NULL, *miter_max = NULL;
   char buffer[2];

   if (mmp) {
      miter = mmp;
      miter_max = &mmp[1024*1024*malloc_mb - 1];
   }

   while (!should_stop) {
      a += b;
      if (mmp) {
         c = *miter + 1;
         *miter = c;
         miter += 8;
         if (miter>miter_max)
         miter =  mmp;
      }
      if (in >= 0) {
         if (read(in, buffer, 1) == -1) {
            dstring ds = DSTRING_INIT;
            fprintf(stderr, "error reading from file: %s\n", sge_strerror(errno, &ds));
            sge_dstring_free(&ds);
            close(in); in = -1;
         }
      }
      if (out >= 0) {
         if (write(out, buffer, 1) != 1) {
            dstring ds = DSTRING_INIT;
            fprintf(stderr, "error writing to file: %s\n", sge_strerror(errno, &ds));
            sge_dstring_free(&ds);
            close(out); out = -1;
         }
      }
   }
}

/* signal handler for SIGALRM */
void stop_work(int signo)
{
   should_stop = 1;
}

static void reserve_stack_mem_kb(int n)
{
   char stack_mem[1024];   
   stack_mem[0] = '\0';
   if (n==1) {
      printf(".%s\n", stack_mem); fflush(stdout);
      return;
   }
   printf("."); fflush(stdout);
   reserve_stack_mem_kb(n-1);
   return;
}


static void reserve_stack_mem_mb(int n)
{
   char stack_mem[1024*1024];   
   stack_mem[0] = '\0';
   if (n==1) {
      printf(".%s\n", stack_mem); fflush(stdout);
      return;
   }
   printf("."); fflush(stdout);
   reserve_stack_mem_mb(n-1);
   return;
}

