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
#include <limits.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <errno.h>

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_prog.h"
#include "uti/sge_string.h"

#include "basis_types.h"
#include "startprog.h"
#include "msg_daemons_common.h"

#if defined(SOLARIS)
#   include "sge_smf.h"
#endif

static int do_wait(pid_t);

/*-----------------------------------------------------------------------
 * startprog
 * Start Sge program and wait until it exits. 
 * Usually it will be only useful for daemons, since it blocks the caller
 * until the child exits
 * Use "argv0" or "path" to build path to program
 * Note:  conf-struct may be uninitialized at this point
 * return 0 if ok
 *       -1 if fork() or exec() failed or child died through signal
 *       -2 if executable is not stat()able
 *       >0 the exit status of the child 
 *       exit status 8 is reserved for unsuccesfull exec() 
 *-----------------------------------------------------------------------*/
int startprog(char *argv0, char *path, char *name, ...)
{
 SGE_STRUCT_STAT sb;
 char prog_path[SGE_PATH_MAX];
 pid_t pid;
 int ret;
 char *ptr;
 va_list argnp;
 char *argv[256];
 int i;
 char *str;
 dstring ds;
 char buffer[128];
#if defined(SOLARIS)
 int err_length = 256;
 char err_str[err_length];
#endif

 DENTER(TOP_LAYER, "startprog");

 sge_dstring_init(&ds, buffer, sizeof(buffer));

 va_start(argnp, name);

 for (i=0; i<256; i++)
   argv[i] = NULL;
 
 for (i=1; i<256 && (str = va_arg(argnp, char*)); i++) {
   DPRINTF(("argv[%d] %s\n", i, str));   
   argv[i] = str;
 }   
 va_end(argnp);

 /* Check with $SGE_ROOT/bin/arch if argv0 == NULL */
 if (argv0) {
   sge_strlcpy(prog_path, argv0, sizeof(prog_path));
   if ((ptr = strrchr(prog_path, '/'))) {
       ptr++;
       *ptr = '\0';
       sge_strlcpy(prog_path, name, sizeof(prog_path));
       if (SGE_STAT(prog_path, &sb)) {
          ERROR((SGE_EVENT, MSG_FILE_STATFAILED_SS, 
               prog_path, strerror(errno)));
          DRETURN(-2);
       }
   } else {
      sge_strlcpy(prog_path, name, sizeof(prog_path));
   }
 } else {
    if (!path) {
       DRETURN(-2);
    }
    snprintf(prog_path, sizeof(prog_path), "%s/%s/%s", path, sge_get_arch(), name);
    if (SGE_STAT(prog_path, &sb)) {
       snprintf(prog_path, sizeof(prog_path), "%s/%s", path, name);
       if (SGE_STAT(prog_path, &sb)) {
          ERROR((SGE_EVENT, MSG_FILE_STATFAILED_SS, prog_path, strerror(errno)));
          DRETURN(-2);
       }
    }
 }

 argv[0] = prog_path;

 WARNING((SGE_EVENT, MSG_STARTUP_STARTINGPROGRAMMX_S, prog_path));

#if defined(SOLARIS)
 pid = sge_smf_contract_fork(err_str, err_length);
 if (pid < -1 && strlen(err_str) > 0) {
	 /* Print additional SMF related error message */
	 ERROR((SGE_EVENT, MSG_SMF_STARTPROG_FORK_FAILED_S, err_str));
 }
#else
 pid = fork();
#endif
 if (pid < 0) {
   ERROR((SGE_EVENT, MSG_PROC_CANTFORKPROCESSTOSTARTX_S, prog_path));
   DRETURN(-1);
 } else if (pid == 0) {
   /* child */
   if (getenv("SGE_DEBUG_LEVEL")) {
      putenv("SGE_DEBUG_LEVEL=0 0 0 0 0 0 0 0");
   }
   execvp(prog_path, argv);
   DEXIT;
   exit(8);
 } else {
    /* parent */
    ret = do_wait(pid);
    if (ret == -1) {
       CRITICAL((SGE_EVENT, MSG_PROC_CANTEXECPROCESSORPROCESSDIEDTHROUGHSIGNALX_S, prog_path));
    } else if (ret > 0) {
       CRITICAL((SGE_EVENT, MSG_PROC_CANTSTARTPROCESSX_S, prog_path));     
    }
    DRETURN(ret);
 }
 /* should never be reached */
 DRETURN(-1);
} 

/*-----------------------------------------------------------------------
 * do_wait
 * wait for child with given pid
 * return >= 0 exit status of child
 *       -1 if child exited with exit status 8 (exec failed)
 *          or child died through signal
 *-----------------------------------------------------------------------*/
static int do_wait(
pid_t pid 
) {
   pid_t npid;
   int status, exit_status, ret;

   DENTER(TOP_LAYER, "do_wait");

   /* This loop only ends if the process exited normally or
    * died through signal
    */
   do {
      npid = waitpid(pid, &status, 0);
   } while ((npid <= 0) || (!WIFSIGNALED(status) && !WIFEXITED(status)) ||
            (npid != pid));

   if (WIFEXITED(status))
      exit_status = WEXITSTATUS(status);
   else if (WIFSIGNALED(status))
      exit_status = 8;
   else {
      ERROR((SGE_EVENT, SFNMAX, MSG_PROC_WAITPIDRETURNEDUNKNOWNSTATUS));
      exit_status = 8;
   }
 
   ret = exit_status == 8 ? -1 : exit_status;
   
   DPRINTF(("exit status of child: %d\n",  ret));

   DRETURN(ret);
}
