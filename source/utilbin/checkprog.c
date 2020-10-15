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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "basis_types.h"
#include "msg_utilbin.h"

#include "uti/sge_os.h"
#include "uti/sge_smf.h"

void sge_exit_wrapper(int);
void usage(void);
int main(int, char *[]);

/*----------------------------------------------------------------------
 * sge_exit_wrapper
 * wrapper for standalone program
 *----------------------------------------------------------------------*/
void sge_exit_wrapper(
int i 
) {
   exit(i);
}
   
/*----------------------------------------------------------------------*/
void usage_checkprog(void)
{
   fprintf(stderr, "\n%s\n", MSG_COMMAND_USAGECHECKPROG);
   /*fprintf(stderr, "check the first 8 letters of process basename\n\n");
   fprintf(stderr, "exit status: 0 if process was found\n");
   fprintf(stderr, "             1 if process was not found\n");
   fprintf(stderr, "             2 if ps program couldn't be spawned\n"); */
   exit(2);
}

/*----------------------------------------------------------------------*/
void usage_getprogs(void)
{
   fprintf(stderr, "\n%s\n", MSG_COMMAND_USAGEGETPROGS );
   /*
   fprintf(stderr, "check and list pids of \"processname\"\n\n");
   fprintf(stderr, "exit status: 0 if process(es) were found\n");
   fprintf(stderr, "             1 if process(es) was not found\n");
   fprintf(stderr, "             2 if ps program couldn't be spawned\n");
   */
   exit(2);
}

   
/*----------------------------------------------------------------------*/
int main(int argc, char *argv[]) 
{
   int res;
   pid_t pid = 0;
   pid_t pids[10000];
   char *ptr;
   int checkit, i;

      
   ptr = strrchr(argv[0], '/');
   if (ptr)
      ptr++;
   else
      ptr = argv[0];   
      
   if (!strcmp(ptr, "checkprog"))
      checkit = 1;
   else if (!strcmp(ptr, "getprogs"))
      checkit = 0;
   else {
      fprintf(stderr, "%s\n", MSG_COMMAND_CALLCHECKPROGORGETPROGS);
      exit(1);
   } 
      
            
   if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-help"))) {
      if (checkit)
         usage_checkprog();
      else
         usage_getprogs();    
   }

   if (checkit ) {
	   if( argc == 3 )		
		  if( atoi(argv[1]) == 0) {
            usage_checkprog();
		  }
   }
   else if (!checkit && argc != 2)
      usage_getprogs();
   

   if (checkit) {
		
		int printpid = 0;
		char* process_name = NULL;
		
		switch( argc ) {
			case 2:
				if(strcmp(argv[1], "-ppid" ) == 0) {
					printpid = 1;
				} else {
					usage_checkprog();
				}
				break;
			case 3:
			   pid = atoi(argv[1]);
			   if(pid == 0) {
				   usage_checkprog();
			   }
			   process_name = argv[2];
			   break;
			default:
			   usage_checkprog();
		}
		
   
		if( printpid ) {
			pid = getppid();
			printf("%ld\n", (long)pid );
			res = 0;
		} else {
#if defined(SOLARIS)
                        /* Init shared SMF libs if necessary */
                        if (sge_smf_used() == 1 && sge_smf_init_libs() != 0) {
                           fprintf(stderr, MSG_COMMAND_SMF_INIT_FAILED);
                           fprintf(stderr, "\n");
                           exit(1);
                        }
#endif
			res = sge_checkprog(pid, process_name, PSCMD);
	
			if (res == 1)
				printf(MSG_PROC_PIDNOTRUNNINGORWRONGNAME_IS, (int) pid, argv[2]);
			else if (res == 0)
				printf(MSG_PROC_PIDISRUNNINGWITHNAME_IS , (int) pid, argv[2]);
			else if (res == -1)
				 printf("%s", MSG_COMMAND_SPANPSFAILED);
				 
			if (res == -1)
				res = 2;
         printf("\n");
		}
   }
   else {
      res = sge_get_pids(pids, 10000, argv[1], PSCMD);
      if (res == -1)
         printf(MSG_COMMAND_RUNPSCMDFAILED_S , PSCMD);
      else if (res == 0)
         printf(MSG_PROC_FOUNDNOPROCESS_S , argv[1]);
      else {
         printf(MSG_PROC_FOUNDPIDSWITHNAME_S , argv[1]);
         for (i = 0; i < res; i++)
            printf(pid_t_fmt"\n", pids[i]);
      }

      if (res == -1)
         res = 2;
      else if (res == 0)
         res = 1;
      else
         res = 0;
      printf("\n");
   }            
   return res;
}
