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
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "msg_utilbin.h"
#include "uti/sge_language.h"

int should_sleep = 0;

/* load profile */
int sleep_time = 0;
int working_time = 1;

/* options */
char cmin, cmax;
int silent = 0;
int nproc = 1;
int n = 1;

void usage(int exit_code);
void permutation(int n);
void go_sleep(int signo);


int main(int argc, char *argv[])
{
   int i;


#ifdef __SGE_COMPILE_WITH_GETTEXT__   
   /* init language output for gettext() , it will use the right language */
   sge_init_language_func((gettext_func_type)        gettext,
                         (setlocale_func_type)      setlocale,
                         (bindtextdomain_func_type) bindtextdomain,
                         (textdomain_func_type)     textdomain);
   sge_init_language(NULL,NULL);   
#endif /* __SGE_COMPILE_WITH_GETTEXT__  */


   while (argc>1 && argv[1][0] == '-') {
      if (!strcmp(argv[1], "-s")) {
         silent = 1;
         argc--;
         argv++;
      }
      if (!strcmp(argv[1], "-p")) {
         argc--;
         argv++;

         if (sscanf(argv[1], "%d", &sleep_time)!=1)
            usage(1);
         argc--;
         argv++;
      }
      if (!strcmp(argv[1], "-f")) {
         argc--;
         argv++;

         if (sscanf(argv[1], "%d", &nproc)!=1)
            usage(1);
         argc--;
         argv++;
      }
   }

   if (argc<2 || argc>3)
      usage(1);

   if (strlen(argv[1])!=3 || argv[1][1]!='-') 
      usage(1);


   cmin = argv[1][0];
   cmax = argv[1][2];

   if (cmin>=cmax) 
      usage(1);

   if (argc==3)
      if (sscanf(argv[2], "%d", &n)!=1 || n<1)
         usage(1);

   /* fork nproc-1 times */
   for (i=1; i<nproc && fork()>0; i++)
      ;

   /* install signal handler for SIGALRM */
   if (sleep_time) {
      signal(SIGALRM, go_sleep); 
/*       fprintf(stderr, "Working for %d seconds.\n", working_time); */
      alarm(working_time);
   }

   permutation(n);

   return 0;
}

void permutation(int n)
{
   char c;
   static char *s = NULL;
   static int i = 0;

   if (should_sleep && sleep_time) {
/*       fprintf(stderr, "Sleeping for %d seconds.\n", sleep_time); */
      sleep(sleep_time);

      signal(SIGALRM, go_sleep); 
/*       fprintf(stderr, "Working for %d seconds.\n", working_time); */
      alarm(working_time);

      should_sleep = 0;
   }

   if (!s) {
      s = sge_malloc(n+1);
      s[n]='\0';
   }
  
   for (c=cmin; c<=cmax; c++) {
      s[i] = c;
      if (n!=1) {
         i++;
         permutation(n-1);
         i--;
      } else 
         if (!silent)
            printf("%s\n", s);
   }
}

void usage(int exit_code)
{
   

   printf("%s\n permutation [options] range [number]\n",MSG_UTILBIN_USAGE);

   
   printf("%s\n", MSG_UTILBIN_PERMUSAGE1);
   printf("   -s         %s\n",MSG_UTILBIN_PERMUSAGE2);
   printf("   -p n       %s\n",MSG_UTILBIN_PERMUSAGE3);
   printf("   -f n       %s\n",MSG_UTILBIN_PERMUSAGE4);
   printf("%s\n", MSG_UTILBIN_PERMUSAGE5);
   
   exit(exit_code);
}

/* signal handler for SIGALRM */
void go_sleep(int signo)
{
   should_sleep = 1;
}

