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
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <pwd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "basis_types.h"

#include "uti/config_file.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_string.h"

#include "uti/sge_unistd.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_stdio.h"

#include "msg_common.h"
#include "msg_daemons_common.h"
#include "msg_qrsh.h"

#define MAKEEXITSTATUS(x) (x << 8)

static pid_t child_pid = 0;

/****** Interactive/qrsh/qrsh_error() ******************************************
*  NAME
*     qrsh_error() -- propagate qrsh startup error to shepherd and qrsh client
*
*  SYNOPSIS
*     static 
*     void qrsh_error(const char *fmt, ...) 
*
*  FUNCTION
*     Writes the passed error message to a special error file in the jobs
*     temporary directory.
*     Separate error files are written for jobs and tasks (started by 
*     qrsh -inherit).
*
*  INPUTS
*     const char *fmt - format string
*     ...             - arguments to be formatted using the format string
*
*******************************************************************************/
static void qrsh_error(const char *fmt, ...)
{
   char *tmpdir = NULL;
   char *taskid = NULL;
   int file;
   char fileName[SGE_PATH_MAX];

   va_list ap;
   char message[MAX_STRING_SIZE];

   va_start(ap, fmt);

   if (fmt == NULL || *fmt == '\0') {
      return;
   }

   vsnprintf(message, MAX_STRING_SIZE, fmt, ap);
   va_end(ap);

   if ((tmpdir = search_conf_val("qrsh_tmpdir")) == NULL) {
      fprintf(stderr, "%s\n", message);
      fprintf(stderr, MSG_CONF_NOCONFVALUE_S, "qrsh_tmpdir");
      fprintf(stderr, "\n");
      return;
   }

   taskid = search_conf_val("qrsh_task_id");

   if (taskid != NULL) {
      snprintf(fileName, SGE_PATH_MAX, "%s/qrsh_error.%s", tmpdir, taskid);
   } else {
      snprintf(fileName, SGE_PATH_MAX, "%s/qrsh_error", tmpdir);
   }

   if ((file = SGE_OPEN3(fileName, O_WRONLY | O_APPEND | O_CREAT, 00744)) == -1) {
      fprintf(stderr, "%s\n", message);
      fprintf(stderr, MSG_QRSH_STARTER_CANNOTOPENFILE_SS, fileName, strerror(errno));
      fprintf(stderr, "\n");
      return;
   }

   if (write(file, message, strlen(message)) != strlen(message)) {
      dstring ds = DSTRING_INIT;
      fprintf(stderr, MSG_FILE_CANNOT_WRITE_SS, fileName, sge_strerror(errno, &ds));
      sge_dstring_free(&ds);
   }

   close(file);
}

/****** Interactive/qrsh/setEnvironment() ***************************************
*
*  NAME
*     setEnvironment() -- set environment from file
*
*  SYNOPSIS
*     static char *setEnvironment(const char *jobdir, char **wrapper);
*
*  FUNCTION
*     Reads environment variables and their values from file <envFileName>
*     and sets them in the actual process environment.
*     The file format conforms to the sge environment file format:
*     Each line contains a tuple:
*        <name>=<value>
*     Special handling for variable PWD: tries to change to named
*     directory.
*     Special handling for variable QRSH_COMMAND: is the command to be executed
*     by qrsh_starter. The value of this variable will be returned as command,
*     or NULL, if an error occurs.
*     Special handling for variable QRSH_WRAPPER: this is a wrapper to be called
*     instead of a shell to execute the command.
*     If this variable is contained in the environment, it will be returned in
*     the parameter wrapper. Memory will be allocated to hold the variable, it 
*     is in the responsibility of the caller to free this memory.
*     Special handling for variable DISPLAY: if it is already set, do not 
*     overwrite it. Usually  it is not set, but if ssh is used as transport
*     mechanism for qrsh, the ssh -X option can be used to enable 
*     X11 forwarding.
*
*  INPUTS
*     jobdir - the jobs spool directory
*     wrapper - buffer to take the path and name of a wrapper script
*
*  RESULT
*     command, if all actions could be performed
*     NULL,    if an error occured; possible errors are:
*                 - the environment file cannot be opened
*                 - a PWD entry is found, but changing to the named directory fails
*                 - necessary memory cannot be allocated
*                 - the variable QRSH_COMMAND is not found
*
****************************************************************************
*/
static char *setEnvironment(const char *jobdir, char **wrapper)
{
   char envFileName[SGE_PATH_MAX];
   FILE *envFile = NULL;
   char *line = NULL;
   char *command   = NULL;
   SGE_STRUCT_STAT statbuf;
   int size;
   bool set_display = true;

   *wrapper = NULL;

   /* don't set DISPLAY, if it is already set (e.g. by ssh) */
   if (getenv("DISPLAY") != NULL) {
      set_display = false;
   }

   snprintf(envFileName, SGE_PATH_MAX, "%s/environment", jobdir);
  
   /* check if environment file exists and
    * retrieve file size. We will take file size as maximum possible line length
    */
   if (SGE_STAT(envFileName, &statbuf) != 0) {
      qrsh_error(MSG_QRSH_STARTER_CANNOTOPENFILE_SS, envFileName, strerror(errno));
      return NULL;
   } 
   
   size = statbuf.st_size;
   line = (char *)malloc(size + 1);
   if (line == NULL) {
      qrsh_error(MSG_QRSH_STARTER_MALLOCFAILED_S, strerror(errno));
      return NULL;
   }

   /* open sge environment file */
   if ((envFile = fopen(envFileName, "r")) == NULL) {
      qrsh_error(MSG_QRSH_STARTER_CANNOTOPENFILE_SS, envFileName, strerror(errno));
      sge_free(&line);
      return NULL;
   }

   /* set all environment variables, change to directory named by PWD */
   while (fgets(line, size, envFile) != NULL) {
      /* clean trailing garbage (\n, \r, EOF ...) */
      char *c = &line[strlen(line)];
      while (iscntrl(*(--c))) {
         *c = 0;
      }

      /* skip setting of display variable */
      if (strncmp(line, "DISPLAY=", 8) == 0 && !set_display) {
         continue;
      }
      
      if (strncmp(line, "QRSH_COMMAND=", 13) == 0) {
         if ((command = (char *)malloc(strlen(line) - 13 + 1)) == NULL) {
            qrsh_error(MSG_QRSH_STARTER_MALLOCFAILED_S, strerror(errno));
            sge_free(&line);
            FCLOSE(envFile);
            return NULL;
         }
         {
            const char *new_line = unescape_env_value(line);
            strcpy(command, new_line + 13); /* RATS: ignore */
            sge_free(&new_line);
         }
      } else if (strncmp(line, "QRSH_WRAPPER=", 13) == 0) {
         if (*(line + 13) == 0) {
            fprintf(stderr, "%s\n", MSG_QRSH_STARTER_EMPTY_WRAPPER);
         } else {
            if ((*wrapper = (char *)malloc(strlen(line) - 13 + 1)) == NULL) {
               qrsh_error(MSG_QRSH_STARTER_MALLOCFAILED_S, strerror(errno));
               sge_free(&line);
               FCLOSE(envFile); 
               return NULL;
            }
            strcpy(*wrapper, line + 13); /* RATS: ignore */
         }
      } else {
         const char *new_line = unescape_env_value(line);
         int put_ret;
         /* set variable */
         put_ret = sge_putenv(new_line);
         sge_free(&new_line);
         if (put_ret == 0) {
            sge_free(&line);
            FCLOSE(envFile); 
            return NULL;
         }
      }
   }

   sge_free(&line);
   FCLOSE(envFile); 

   /* 
    * Use starter_method if it is supplied
    * and not overridden by QRSH_WRAPPER
    */
    
   if (*wrapper == NULL) {
      char *starter_method = get_conf_val("starter_method");
      if (starter_method != NULL && strcasecmp(starter_method, "none") != 0) { 
         char buffer[128];
         *wrapper = starter_method;
         snprintf(buffer, 128, "%s=%s", "SGE_STARTER_SHELL_PATH", ""); sge_putenv(buffer);
         snprintf(buffer, 128, "%s=%s", "SGE_STARTER_SHELL_START_MODE", "unix_behavior"); sge_putenv(buffer);
         snprintf(buffer, 128, "%s=%s", "SGE_STARTER_USE_LOGIN_SHELL", "false"); sge_putenv(buffer);
      } 
   }
   
   return command;
FCLOSE_ERROR:
   qrsh_error(MSG_FILE_NOCLOSE_SS, envFileName, strerror(errno));
   return NULL;
}

/****** Interactive/qrsh/readConfig() *****************************************
*  NAME
*     readConfig() -- read the jobs configuration
*
*  SYNOPSIS
*     static int readConfig(const char *jobdir) 
*
*  FUNCTION
*     Reads the jobs configuration (<job spool dir>/config).
*
*  INPUTS
*     const char *jobdir - the jobs spool directory
*
*  RESULT
*     static int - 0, if an error occured
*                  1, if function completed without errors
*
*******************************************************************************/
static int readConfig(const char *jobdir)
{
   char configFileName[SGE_PATH_MAX];

   snprintf(configFileName, SGE_PATH_MAX, "%s/config", jobdir);

   /* read jobs config file */
   if(read_config(configFileName) != 0) {
      qrsh_error(MSG_QRSH_STARTER_CANNOTREADCONFIGFROMFILE_S, configFileName);
      return 0;
   }

   return 1;
}

/****** Interactive/qrsh/changeDirectory() *****************************************
*  NAME
*     changeDirectory() -- change to directory named in job config
*
*  SYNOPSIS
*     static int changeDirectory(void) 
*
*  FUNCTION
*     Reads the target working directory for a qrsh job from the jobs 
*     configuration and tries to 
*     change the current working directory.
*
*  RESULT
*     static int - 0, if an error occured
*                  1, if function completed without errors
*  SEE ALSO
*     Interactive/qrsh/readConfig()
*
*******************************************************************************/
static int changeDirectory(void) 
{
   char *cwd = NULL;

   /* get jobs target directory */
   cwd = get_conf_val("cwd");

   if(cwd == NULL) {
      qrsh_error(MSG_QRSH_STARTER_NOCWDINCONFIG);
      return 0;
   }

   /* change to dir cwd */
   if (sge_chdir(cwd) == -1) {
      fprintf(stderr, MSG_QRSH_STARTER_CANNOTCHANGEDIR_SS, cwd, strerror(errno));
      fprintf(stderr, "\n");
      return 0;
   }

   return 1;
}


/****** Interactive/qrsh/write_pid_file() ***************************************
*
*  NAME
*     write_pid_file()  -- write a pid to file pid in $TMPDIR
*
*  SYNOPSIS
*     static int write_pid_file(pid_t pid);
*
*  FUNCTION
*     Writes the given pid to a file named pid in the directory
*     contained in environment variable TMPDIR
*
*  INPUTS
*     pid - the pid to write
*
*  RESULT
*     1, if all actions could be performed
*     0, if an error occured. Possible error situations are:
*        - the environement variable TMPDIR cannot be read
*        - the file cannot be opened
*
****************************************************************************
*/
static int write_pid_file(pid_t pid) 
{
   char *pid_file_name = NULL;
   int pid_file;
   char pid_str[20];

   if((pid_file_name = search_conf_val("qrsh_pid_file")) == NULL) {
      qrsh_error(MSG_CONF_NOCONFVALUE_S, "qrsh_pid_file");
      return 0;
   }

   if((pid_file = SGE_OPEN3(pid_file_name, O_WRONLY | O_APPEND | O_CREAT, 00744)) == -1) {
      dstring ds = DSTRING_INIT;
      qrsh_error(MSG_QRSH_STARTER_CANNOTWRITEPID_SS, pid_file_name, sge_strerror(errno, &ds));
      sge_dstring_free(&ds);
      return 0;
   }

   snprintf(pid_str, 20, pid_t_fmt, pid);

   if (write(pid_file, pid_str, strlen(pid_str)) != strlen(pid_str)) {
      dstring ds = DSTRING_INIT;
      qrsh_error(MSG_FILE_CANNOT_WRITE_SS, pid_file_name, sge_strerror(errno, &ds));
      sge_dstring_free(&ds);
   }

   SGE_CLOSE(pid_file);
   return 1;
}

/****** Interactive/qrsh/forward_signal() ***************************************
*
*  NAME
*     forward_signal() -- forward a signal to qrsh_starter's child
*
*  SYNOPSIS
*     static void forward_signal(int sig);
*
*  FUNCTION
*     Forwards the signal <sig> to the process group given
*     in the global variable <child_pid>.
*
*  INPUTS
*     sig - the signal to forward
*
****************************************************************************
*/
static void forward_signal(int sig)
{
   if(child_pid > 0) {
      kill(-child_pid, sig);
   }
}

/****** Interactive/qrsh/split_command() *******************************************
*  NAME
*     split_command() -- split commandline into tokens
*
*  SYNOPSIS
*     static int split_command(char *command, char ***cmdargs) 
*
*  FUNCTION
*     The command to be executed by qrsh_starter may contain multiple 
*     arguments, quotes, double quotes, back quotes etc. within the 
*     arguments ...
*     To preserve all this information, qrsh writes the command line arguments
*     to an environment variable QRSH_COMMAND and separates the arguments by
*     the character with code 255 (0xff).
*     split_command splits the resulting string into the original arguments
*     and writes them to a string array.
*
*  INPUTS
*     char *command   - arguments separated by 0xff
*     char ***cmdargs - pointer to string array to be filled with arguments
*
*  RESULT
*     static int - the number of arguments or 0 if an error occured
*
*  SEE ALSO
*     Interactive/qrsh/join_command()
*
*******************************************************************************/
static int split_command(char *command, char ***cmdargs) {
   /* count number of arguments */
   int counter = 1;
   char *s = command;
   int argc;
   char **args;
   int i,end;
   char delimiter[2];

   snprintf(delimiter, 2, "%c", 0xff);

   while(*s) {
      if(*s++ == delimiter[0]) {
         counter++;
      }
   }

   /* copy arguments */
   argc = 0;
   args = (char **)malloc(counter * sizeof(char *));
   
   if(args == NULL) {
      return 0;
   }

   /* do not use strtok(), strtok() is seeing 2 or more delimiters as one !!! */
   s=command;
   args[argc++] = s;
   end =  strlen(command);
   for(i = 0; i < end ; i++) {
      if (s[i] == delimiter[0]) {
         s[i] = 0;
            args[argc++] = &s[i+1];
      }
   } 

#if 0
   /* debug code */
   fflush(stderr);
   fprintf(stdout, "counter = %d\n", counter);
   for(i = 0; i < argc; i++) {
      fprintf(stdout, "split_command: args[%d] = %s\n", i, args[i]);
   }
   fflush(stdout);
#endif

   *cmdargs = args;
   return argc;
}

/****** Interactive/qrsh/join_command() ********************************************
*  NAME
*     join_command() -- join arguments to a single string
*
*  SYNOPSIS
*     static char* join_command(int argc, char **argv) 
*
*  FUNCTION
*     Joins arguments given in an argument vector (string array) to a single
*     character string where the arguments are separated by a single blank.
*     This is used, to pass an argument vector as one argument to a call
*     <shell> -c <commandline>.
*
*  INPUTS
*     int argc    - argument count
*     char **argv - argument vector
*
*  RESULT
*     static char* - the resulting commandline or NULL, if an error occured.
*
*  SEE ALSO
*     Interactive/qrsh/split_command()
*
*******************************************************************************/
static char *join_command(int argc, char **argv) {
   int i;
   int length = 0;
   char *buffer;
   
   /* calculate needed size */
   for(i = 0; i < argc; i++) {
      length += strlen(argv[i]);
   }

   /* add spaces and \0 */
   length += argc;

   buffer = malloc(length * sizeof(char));

   if(buffer == NULL) {
      return 0;
   }

   strcpy(buffer, argv[0]);     /* RATS: ignore */
   for(i = 1; i < argc; i++) {
      strcat(buffer, " "); strcat(buffer, argv[i]);  /* RATS: ignore */
   }

   return buffer;
}


/****** Interactive/qrsh/startJob() ***************************************
*
*  NAME
*     startJob() -- start a shell with commands to execute
*
*  SYNOPSIS
*     static int startJob(char *command, char *wrapper, int noshell);
*
*  FUNCTION
*     Starts the commands and arguments to be executed as 
*     specified in parameter <command>. 
*     If the parameter noshell is set to 1, the command is directly called
*     by exec.
*     If a wrapper is specified (parameter wrapper, set by environment
*     variable QRSH_WRAPPER), this wrapper is called and is passed the 
*     command to execute as commandline parameters.
*     If neither noshell nor wrapper is set, a users login shell is called
*     with the parameters -c <command>.
*     The child process creates an own process group.
*     The pid of the child process is written to a pid file in $TMPDIR.
*
*  INPUTS
*     command - commandline to be executed
*     wrapper - name and path of a wrapper script
*     noshell - if != 0, call the command directly without shell
*
*  RESULT
*     status of the child process after it terminated
*     or EXIT_FAILURE, if the process of starting the child 
*     failed because of one of the following error situations:
*        - fork failed
*        - the pid of the child process cannot be written to pid file
*        - the name of actual user cannot be determined
*        - info about the actual user cannot be determined (getpwnam)
*        - necessary memory cannot be allocated
*        - executing the shell failed
*
*  SEE ALSO
*     Interactive/qrsh/write_pid_file()
*     Interactive/qrsh/split_command()
*     Interactive/qrsh/join_command()
*
****************************************************************************
*/
static int startJob(char *command, char *wrapper, int noshell)
{

   child_pid = fork();
   if(child_pid == -1) {
      qrsh_error(MSG_QRSH_STARTER_CANNOTFORKCHILD_S, strerror(errno));
      return EXIT_FAILURE;
   }

   if(child_pid) {
      /* parent */
      int status;

#if defined(LINUX)
      int ttyfd;
#endif

      signal(SIGINT,  forward_signal);
      signal(SIGQUIT, forward_signal);
      signal(SIGTERM, forward_signal);

      /* preserve pseudo terminal */
      /* fixme:  why only Linux? */
#if defined(LINUX)
      ttyfd = open("/dev/tty", O_RDWR);
      if (ttyfd != -1) {
         tcsetpgrp(ttyfd, child_pid);
         close(ttyfd); 
      }
#endif

      while(waitpid(child_pid, &status, 0) != child_pid && errno == EINTR);
      return(status);
   } else {
      /* child */
      char *buffer = NULL;
      int size;
      struct passwd pw_struct;
      char *shell    = NULL;
      char *userName = NULL;
      int    argc = 0;
      const char **args = NULL;
      char *cmd = NULL;
      int cmdargc;
      char **cmdargs = NULL;
      int i;

      if(!write_pid_file(getpid())) {
         exit(EXIT_FAILURE);
      }

      cmdargc = split_command(command, &cmdargs);

      if(cmdargc == 0) {
         qrsh_error(MSG_QRSH_STARTER_INVALIDCOMMAND);
         exit(EXIT_FAILURE);
      }

      if(!noshell) {
         struct passwd *pw = NULL;

         if((userName = search_conf_val("job_owner")) == NULL) {
            qrsh_error(MSG_QRSH_STARTER_CANNOTGETLOGIN_S, strerror(errno));
            exit(EXIT_FAILURE);
         }

         size = get_pw_buffer_size();
         buffer = sge_malloc(size);

         if ((pw = sge_getpwnam_r(userName, &pw_struct, buffer, size)) == NULL) {
            qrsh_error(MSG_QRSH_STARTER_CANNOTGETUSERINFO_S, strerror(errno));
            exit(EXIT_FAILURE);
         }
         
         shell = pw->pw_shell;
         
         if(shell == NULL) { 
            qrsh_error(MSG_QRSH_STARTER_CANNOTDETERMSHELL_S, "/bin/sh");
            shell = "/bin/sh";
         } 
      }
     
      if((args = malloc((cmdargc + 3) * sizeof(char *))) == NULL) {
         qrsh_error(MSG_QRSH_STARTER_MALLOCFAILED_S, strerror(errno));
         exit(EXIT_FAILURE);
      }         
    
      if(wrapper == NULL) {
         if(noshell) {
            cmd = cmdargs[0];
            for(i = 0; i < cmdargc; i++) {
               args[argc++] = cmdargs[i];
            }
         } else {
            cmd = shell;
            args[argc++] = sge_basename(shell, '/');
            args[argc++] = "-c";
            args[argc++] = join_command(cmdargc, cmdargs);
         }
      } else {
         cmd = wrapper;
         args[argc++] = sge_basename(wrapper, '/');
         for(i = 0; i < cmdargc; i++) {
            args[argc++] = cmdargs[i];
         }
      }

      args[argc++] = NULL;

#if 0
{
   /* debug code */
   int i;
   
   fflush(stdout) ; fflush(stderr);
   printf("execvp(%s,", cmd);
   for(i = 0; args[i] != NULL; i++) {
      printf(" \"%s\"", args[i]);
   }
   printf(")\n");
   fflush(stdout) ; fflush(stderr); 
} 
#endif

      SETPGRP;
      execvp(cmd, (char *const *)args);
      /* exec failed */
      fprintf(stderr, MSG_QRSH_STARTER_EXECCHILDFAILED_S, args[0], strerror(errno));
      fprintf(stderr, "\n");
      exit(EXIT_FAILURE);
   }

   /* will never be reached */
   return EXIT_FAILURE; 
}

/****** Interactive/qrsh/writeExitCode() ***************************************
*
*  NAME
*    writeExitCode() -- write exit code of child process to file
*
*  SYNOPSIS
*     static int writeExitCode(int myExitCode, int programExitCode)
*
*  FUNCTION
*     If myExitCode != EXIT_SUCCESS, that means, if an error occured in
*     qrsh_starter, write this exit code to file,
*     else write the exit code of the child process (programExitCode).
*     The exit code is written to a file "qrsh_exit_code" in the
*     directory $TMPDIR.
*
*  INPUTS
*     myExitCode      - status of qrsh_starter
*     programExitCode - status of the child process
*
*  RESULT
*     EXIT_SUCCESS, if all actions could be performed,
*     EXIT_FAILURE, if one of the following errors occured:
*        - the environment variable TMPDIR cannot be read
*        - the file $TMPDIR/qrsh_exit_code cannot be written
*
****************************************************************************
*/
static int writeExitCode(int myExitCode, int programExitCode) 
{   
   int exitCode;
   char exitCode_str[20];
   char *tmpdir = NULL;
   char *taskid = NULL;
   int  file;
   char fileName[SGE_PATH_MAX];

   if(myExitCode != EXIT_SUCCESS) {
      exitCode = MAKEEXITSTATUS(myExitCode);
   } else {
      exitCode = programExitCode;
   }

   if((tmpdir = search_conf_val("qrsh_tmpdir")) == NULL) {
      qrsh_error(MSG_CONF_NOCONFVALUE_S, "qrsh_tmpdir");
      return EXIT_FAILURE;
   }
  
   taskid = get_conf_val("pe_task_id");
   
   if(taskid != NULL) {
      snprintf(fileName, SGE_PATH_MAX, "%s/qrsh_exit_code.%s", tmpdir, taskid);
   } else {
      snprintf(fileName, SGE_PATH_MAX, "%s/qrsh_exit_code", tmpdir);
   }

   if((file = SGE_OPEN3(fileName, O_WRONLY | O_APPEND | O_CREAT, 00744)) == -1) {
      dstring ds = DSTRING_INIT;
      qrsh_error(MSG_QRSH_STARTER_CANNOTOPENFILE_SS, fileName, sge_strerror(errno, &ds));
      sge_dstring_free(&ds);
      return EXIT_FAILURE;
   }
 
   snprintf(exitCode_str, 20, "%d", exitCode);
   if (write(file, exitCode_str, strlen(exitCode_str)) != strlen(exitCode_str)) {
      dstring ds = DSTRING_INIT;
      qrsh_error(MSG_FILE_CANNOT_WRITE_SS, fileName, sge_strerror(errno, &ds));
      sge_dstring_free(&ds);
   }
   SGE_CLOSE(file);
   
   return EXIT_SUCCESS;
}

/****** Interactive/qrsh/--qrsh_starter ***************************************
*
*  NAME
*     qrsh_starter -- start a command special correct environment
*
*  SYNOPSIS
*     qrsh_starter <environment file> <noshell>
*     int main(int argc, char **argv[])
*
*  FUNCTION
*     qrsh_starter is used to start a command, optionally with additional
*     arguments, in a special environment.
*     The environment is read from the given <environment file>.
*     The command to be executed is read from the environment variable
*     QRSH_COMMAND and executed either standalone, passed to a wrapper
*     script (environment  variable QRSH_WRAPPER) or (default) in a users login
*     shell (<shell> -c <command>).
*     On exit of the command, or if an error occurs, an exit code is written
*     to the file $TMPDIR/qrsh_exit_code.
*
*     qrsh_starter is called from qrsh to start the remote processes in 
*     the correct environment.
*
*  INPUTS
*     environment file - file with environment information, each line 
*                        contains a tuple <name>=<value>
*     noshell          - if this parameter is passed, the command will be
*                        executed standalone
*
*  RESULT
*     EXIT_SUCCESS, if all actions could be performed,
*     EXIT_FAILURE, if an error occured
*
*  EXAMPLE
*     setenv QRSH_COMMAND "echo test"
*     env > ~/myenvironment
*     rsh <hostname> qrsh_starter ~/myenvironment 
*
*  SEE ALSO
*     Interactive/qsh/--Interactive
*
****************************************************************************
*/
int main(int argc, char *argv[])
{
   int   exitCode = 0;
   char *command  = NULL;
   char *wrapper = NULL;
   int  noshell  = 0;

   /* check for correct usage */
   if(argc < 2) {
      fprintf(stderr, "usage: %s <job spooldir> [noshell]\n", argv[0]);
      exit(EXIT_FAILURE);        
   }

   /* check for noshell */
   if(argc > 2) {
      if(strcmp(argv[2], "noshell") == 0) {
         noshell = 1;
      }
   }

   if(!readConfig(argv[1])) {
      writeExitCode(EXIT_FAILURE, 0);
      exit(EXIT_FAILURE);
   }

   /* setup environment */
   command = setEnvironment(argv[1], &wrapper);
   if(command == NULL) {
      writeExitCode(EXIT_FAILURE, 0);
      exit(EXIT_FAILURE);
   }   

   if(!changeDirectory()) {
      writeExitCode(EXIT_FAILURE, 0);
      exit(EXIT_FAILURE);
   }

   /* start job */
   exitCode = startJob(command, wrapper, noshell);

   /* JG: TODO: At this time, we could already pass the exitCode to qrsh.
    *           Currently, this is done by shepherd, but only after 
    *           qrsh_starter and rshd exited.
    *           If we pass exitCode to qrsh, we also have to implement the
    *           shepherd_about_to_exit mechanism here.
    */

   /* write exit code and exit */
   return writeExitCode(EXIT_SUCCESS, exitCode);
}
