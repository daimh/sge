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
 * Portions of this software are Copyright (c) 2011 Univa Corporation
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/resource.h>
#include "sge_uidgid.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <interix/interix.h>

#include "config_file.h"
#include "err_trace.h"
#include "wingrid.h"
#include "windows_gui.h"

/****** wingrid/wl_start_job_remote() ****************************************
*  NAME
*     wl_start_job_remote() -- Sends a job to the SGE Helper Service and
*                              requests its start
*
*  SYNOPSIS
*     int wl_start_job_remote(const char *filename, 
*                             char       **args,
*                             char       **env,
*                             const char *job_user,
*                             const char *user_passwd,
*                             int        *pwin32_exit_status,
*                             enum en_JobStatus *pjob_status,
*                             char       *err_str)
*
*  FUNCTION
*     Sends a job start request to the SGE Helper Service, waits blocking
*     for job end.
*
*  INPUTS
*     const char *filename     - The path of the job file 
*     char       **args        - The argument list of the job
*     char       **env         - The environment of the job
*     const char *job_user     - The name of the job user
*     const char *user_passwd  - The passwd of the job user
*
*  OUTPUTS
*     int  *pwin32_exit_status       - The exit status of the job
*     enum en_JobStatus *pJob_status - The status of the job
*     char *err_str                  - In case of error: Error description
*
*  RESULTS
*     int - 0: Job was started, job exit code received
*          >0: errno
*         254: Job already existed in job list, job was rejected
*         255: not all data was sent, but send() returned no error, job was
*              not started.
*
*  NOTES
*     MT-NOTE: 
******************************************************************************/
int wl_start_job_remote(const char *filename, 
                        char **args,
                        char **env,
                        const char *job_user,
                        const char *user_passwd,
                        int  *pwin32_exit_status,
                        enum en_JobStatus *pjob_status,
                        char *err_str)
{
   int            ret, comm_sock;
   int            i = 0;
   int            nargs = 0;
   char           str_nargs[10];
   char           job[4096];
   char           unix_conf[4096];
   char           domain[MAX_STRING_SIZE];
   const          size_t req_size = 32*1024;
   char           request[req_size];
   char           *ptr = request;
   char           str_nenv[10];
   int            ienv, nenv;
   int            nconf;
   char           *pconf = NULL;
   char           str_nconf[10];

   ret = wl_connect_to_service(&comm_sock, err_str, MAX_STRING_SIZE);
   if(ret != 0) {
      wl_disconnect_from_service(&comm_sock);
      return ret;
   }

   /* Clear request string */
   memset(request, 0, req_size);

   /* Set request type to 0 = req_job_start */
   ptr = request+4;

   /* Set job name */
   unixpath2win(filename, 0, job, 4095);
   strcpy(ptr, job);
   ptr += strlen(job)+1;

   /* Set number of args */
   while(args[nargs]!=NULL) {
      nargs++;
   }
   sprintf(str_nargs, "%d", nargs);
   strcpy(ptr, str_nargs);
   ptr += strlen(str_nargs)+1;

   /* Set args */
   for(i=0; i<nargs; i++) {
      strcpy(ptr, args[i]);
      ptr += strlen(args[i])+1;
   }

   /* Set number of config entries */
   nconf = 7;
   pconf = search_conf_val("pe_task_id");
   if(pconf) {
      nconf++;
   }
   pconf = search_conf_val("display_win_gui");
   if(pconf) {
      nconf++;
   }
   sprintf(str_nconf, "%d", nconf);

   strcpy(ptr, str_nconf);
   ptr += strlen(str_nconf)+1;

   /* Set config entries */
   strcpy(ptr, "stdout_path=");
   ptr += strlen("stdout_path=");
   pconf = search_conf_val("stdout_path");
   unixpath2win(pconf, 0, unix_conf, 4095);
   strcpy(ptr, unix_conf);
   ptr += strlen(unix_conf)+1;

   strcpy(ptr, "stdin_path=");
   ptr += strlen("stdin_path=");
   pconf = search_conf_val("stdin_path");
   unixpath2win(pconf, 0, unix_conf, 4095);
   strcpy(ptr, unix_conf);
   ptr += strlen(unix_conf)+1;
   
   strcpy(ptr, "stderr_path=");
   ptr += strlen("stderr_path=");
   pconf = search_conf_val("stderr_path");
   unixpath2win(pconf, 0, unix_conf, 4095);
   strcpy(ptr, unix_conf);
   ptr += strlen(unix_conf)+1;

   strcpy(ptr, "merge_stderr=");
   ptr += strlen("merge_stderr=");
   pconf = search_conf_val("merge_stderr");
   strcpy(ptr, pconf);
   ptr += strlen(pconf)+1;

   strcpy(ptr, "cwd=");
   ptr += strlen("cwd=");
   pconf = search_conf_val("cwd");
   unixpath2win(pconf, 0, unix_conf, 4095);
   strcpy(ptr, unix_conf);
   ptr += strlen(unix_conf)+1;

   strcpy(ptr, "job_id=");
   ptr += strlen("job_id=");
   pconf = search_conf_val("job_id");
   strcpy(ptr, pconf);
   ptr += strlen(pconf)+1;

   strcpy(ptr, "ja_task_id=");
   ptr += strlen("ja_task_id=");
   pconf = search_conf_val("ja_task_id");
   strcpy(ptr, pconf);
   ptr += strlen(pconf)+1;

   pconf = search_conf_val("pe_task_id");
   if(pconf) {
      strcpy(ptr, "pe_task_id=");
      ptr += strlen("pe_task_id=");
      strcpy(ptr, pconf);
      ptr += strlen(pconf)+1;
   }

   pconf = search_conf_val("display_win_gui");
   if(pconf) {
      strcpy(ptr, "display_win_gui=");
      ptr += strlen("display_win_gui=");
      strcpy(ptr, pconf);
      ptr += strlen(pconf)+1;
   }

   /* Set number of environment entries */
   for(ienv = 0, nenv = 0; env[ienv] != NULL; ienv++) {
      nenv++;
   }
   sprintf(str_nenv, "%d", nenv);
   strcpy(ptr, str_nenv);
   ptr += strlen(str_nenv)+1;

   /* Set environment entries */
   for(ienv = 0; env[ienv] != NULL; ienv++) {
      strcpy(ptr, env[ienv]);
      ptr += strlen(env[ienv])+1;
   }

   /* Set user name */
   strcpy(ptr, job_user);
   ptr += strlen(job_user)+1;

   /* Set password */
   strcpy(ptr, user_passwd); 
   ptr += strlen(user_passwd)+1;

   /* Set domain name */
   getpdomain(domain, MAX_STRING_SIZE);
   strcpy(ptr, domain);
   ptr += strlen(domain)+1;

   /* Set terminate char */
   *ptr = (char)EOF;

   ret = wl_request_job_start(comm_sock, request, ptr-request+1,
                              err_str, MAX_STRING_SIZE);

   wl_disconnect_from_service(&comm_sock);
   if(ret == 0) {
      if(strcmp(request, "NAK")==0) {
         ret = 254;
      } else {
         sscanf(request, "%d %u %[^\n]", 
            (int*)pjob_status, pwin32_exit_status, err_str);
      }
   }
   return ret;
}

/****** wingrid/wl_getrusage_remote() ****************************************
*  NAME
*     wl_getrusage_remote() -- Retrieves the usage of a job from 
*                              SGE Helper Service
*
*  SYNOPSIS
*     int wl_getrusage_remote(const char *szjob_id, int *pstatus,
*                             struct rusage *prusage, char *pszerrormsg)
*
*  FUNCTION
*     Retrieves the job usage from the SGE Helper Service and fills
*     a rusage struct with this information.
*
*  INPUTS
*     const char *szjob_id       - Job ID as string
*
*  OUTPUTS
*     int           *pstatus     - Receives status information, see wait(3).
*     struct rusage *prusage     - Receives job usage data.
*     char          *pszerrormsg - Buffer of size MAX_STRING_SIZE that
*                                  receives the error description.
*
*  RESULTS
*     int - 0: Usage was successfully received
*          >0: errno
*
*  NOTES
*     MT-NOTE: 
******************************************************************************/
int wl_getrusage_remote(const char *szjob_id, int *pstatus,
                        struct rusage *prusage, char *pszerrormsg)
{
   int ret, comm_sock;
   char job_usage[4096];
   int usagelen = sizeof(job_usage);

   memset(job_usage, 0, 4096);

   ret = wl_connect_to_service(&comm_sock, pszerrormsg, MAX_STRING_SIZE);
   if (ret != 0) {
      return ret;
   }

   ret = wl_request_job_usage(comm_sock, job_usage, &usagelen,
                              pszerrormsg, MAX_STRING_SIZE);
   wl_disconnect_from_service(&comm_sock);
   if (ret != 0) {
      return ret;
   }
   memset(prusage, 0, sizeof(struct rusage));

   job_usage[usagelen]='\0';
   sscanf(job_usage, "%ld %ld %ld %ld %d", 
      (long int*)&(prusage->ru_stime.tv_sec),
      (long int*)&(prusage->ru_stime.tv_usec),
      (long int*)&(prusage->ru_utime.tv_sec),
      (long int*)&(prusage->ru_utime.tv_usec),
      pstatus);

   return ret;
}

/****** wingrid/wl_request_job_usage() ***************************************
*  NAME
*     wl_request_job_usage() -- Requests the usage of a job from 
*                               SGE Helper Service
*
*  SYNOPSIS
*     int wl_request_job_usage(int comm_sock, char *job_results, 
*                              int *resultslen, char* errormsg, int errorlen)
*
*  FUNCTION
*     Requests the job usage from the SGE Helper Service.
*
*  INPUTS
*     int  comm_sock    - socket fd of the connection to SGE Helper Service
*     char *job_results - buffer for the usage data
*     int  *resultslen  - size of buffer for usage data
*     char *errormsg    - buffer for error message
*     int  errorlen     - length of errormsg buffer
*
*  OUTPUTS
*     char *job_results - the serialized usage data
*     int  *resultslen  - size of usage data
*     char *errormsg    - in case of error: the error description
*
*  RESULTS
*     int - 0: Usage was successfully received
*          >0: errno
*         255: not all data was sent, but send() returned no error.
*
*  NOTES
*     MT-NOTE: 
******************************************************************************/
int wl_request_job_usage(int comm_sock, char *job_results, int *resultslen,
                          char* errormsg, int errorlen)
{
   int ret;
   char request[4096];
   char unix_conf[4096];
   char str_nconf[10];
   char *ptr = request;
   char *pconf = NULL;

   memset(request, 0, 4096);
   request[0] = (char)1;
   ptr = request+4;
   
   /* Set number of config entries */
   pconf = search_conf_val("pe_task_id");
   if(pconf) {
      sprintf(str_nconf, "%d", 7);
   } else {
      sprintf(str_nconf, "%d", 6);
   }
   strcpy(ptr, str_nconf);
   ptr += strlen(str_nconf)+1;

   /* Set config entries */
   strcpy(ptr, "stdout_path=");
   ptr += strlen("stdout_path=");
   pconf = search_conf_val("stdout_path");
   unixpath2win(pconf, 0, unix_conf, 4095);
   strcpy(ptr, unix_conf);
   ptr += strlen(unix_conf)+1;

   strcpy(ptr, "stdin_path=");
   ptr += strlen("stdin_path=");
   pconf = search_conf_val("stdin_path");
   unixpath2win(pconf, 0, unix_conf, 4095);
   strcpy(ptr, unix_conf);
   ptr += strlen(unix_conf)+1;
   
   strcpy(ptr, "stderr_path=");
   ptr += strlen("stderr_path=");
   pconf = search_conf_val("stderr_path");
   unixpath2win(pconf, 0, unix_conf, 4095);
   strcpy(ptr, unix_conf);
   ptr += strlen(unix_conf)+1;

   strcpy(ptr, "cwd=");
   ptr += strlen("cwd=");
   pconf = search_conf_val("cwd");
   unixpath2win(pconf, 0, unix_conf, 4095);
   strcpy(ptr, unix_conf);
   ptr += strlen(unix_conf)+1;

   strcpy(ptr, "job_id=");
   ptr += strlen("job_id=");
   pconf = search_conf_val("job_id");
   strcpy(ptr, pconf);
   ptr += strlen(pconf)+1;

   strcpy(ptr, "ja_task_id=");
   ptr += strlen("ja_task_id=");
   pconf = search_conf_val("ja_task_id");
   strcpy(ptr, pconf);
   ptr += strlen(pconf)+1;

   pconf = search_conf_val("pe_task_id");
   if(pconf) {
      strcpy(ptr, "pe_task_id=");
      ptr += strlen("pe_task_id=");
      strcpy(ptr, pconf);
      ptr += strlen(pconf)+1;
   }
   *ptr = (char)EOF;

   ret = send(comm_sock, request, ptr-request+1, 0);
   if(ret == -1) {
      snprintf(errormsg, errorlen, "send() returned '%s'", strerror(errno));
      return errno;
   }
   if(ret != ptr-request+1) {
      snprintf(errormsg, errorlen, "Not all data sent, cancelling job start.");
      return 255;
   }

   ret = wl_wait_for_answer(comm_sock, job_results, resultslen,
                            errormsg, errorlen);

   return ret;
}

/****** wingrid/wl_request_job_exit_status() *********************************
*  NAME
*     wl_request_job_exit_status() -- Requests the exit status of a job from 
*                                     SGE Helper Service
*
*  SYNOPSIS
*     int wl_request_job_usage(int comm_sock, char *job_exit_status,
*                          int *exit_status_len, char* errormsg, int errorlen)
*
*  FUNCTION
*     Requests the job exit status from the SGE Helper Service.
*
*  INPUTS
*     int  comm_sock        - socket fd of the connection to SGE Helper Service
*     char *job_exit_status - buffer for the exit status
*     int  *exit_status_len - size of buffer for exit status
*     char *errormsg        - buffer for error message
*     int  errorlen         - length of errormsg buffer
*
*  OUTPUTS
*     char *job_exit_status - the serialized exit status
*     int  *exit_status_len - size of exit status
*     char *errormsg        - in case of error: the error description
*
*  RESULTS
*     int - 0: Usage was successfully received
*          >0: errno
*         255: not all data was sent, but send() returned no error.
*
*  NOTES
*     MT-NOTE: 
******************************************************************************/
int wl_request_job_exit_status(int comm_sock, char *job_exit_status, 
                           int *exit_status_len, char *errormsg, int errorlen)
{
   int ret;
   char request[4096];
   char unix_conf[4096];
   char str_nconf[10];
   char *ptr = request;
   char *pconf = NULL;

   memset(request, 0, 4096);
   request[0] = (char)1;
   ptr = request+4;

   /* Set number of config entries */
   pconf = search_conf_val("pe_task_id");
   if(pconf) {
      sprintf(str_nconf, "%d", 7);
   } else {
      sprintf(str_nconf, "%d", 6);
   }
   strcpy(ptr, str_nconf);
   ptr += strlen(str_nconf)+1;

   /* Set config entries */
   strcpy(ptr, "stdout_path=");
   ptr += strlen("stdout_path=");
   pconf = search_conf_val("stdout_path");
   unixpath2win(pconf, 0, unix_conf, 4095);
   strcpy(ptr, unix_conf);
   ptr += strlen(unix_conf)+1;

   strcpy(ptr, "stdin_path=");
   ptr += strlen("stdin_path=");
   pconf = search_conf_val("stdin_path");
   unixpath2win(pconf, 0, unix_conf, 4095);
   strcpy(ptr, unix_conf);
   ptr += strlen(unix_conf)+1;
   
   strcpy(ptr, "stderr_path=");
   ptr += strlen("stderr_path=");
   pconf = search_conf_val("stderr_path");
   unixpath2win(pconf, 0, unix_conf, 4095);
   strcpy(ptr, unix_conf);
   ptr += strlen(unix_conf)+1;

   strcpy(ptr, "cwd=");
   ptr += strlen("cwd=");
   pconf = search_conf_val("cwd");
   unixpath2win(pconf, 0, unix_conf, 4095);
   strcpy(ptr, unix_conf);
   ptr += strlen(unix_conf)+1;

   strcpy(ptr, "job_id=");
   ptr += strlen("job_id=");
   pconf = search_conf_val("job_id");
   strcpy(ptr, pconf);
   ptr += strlen(pconf)+1;

   strcpy(ptr, "ja_task_id=");
   ptr += strlen("ja_task_id=");
   pconf = search_conf_val("ja_task_id");
   strcpy(ptr, pconf);
   ptr += strlen(pconf)+1;

   pconf = search_conf_val("pe_task_id");
   if(pconf) {
      strcpy(ptr, "pe_task_id=");
      ptr += strlen("pe_task_id=");
      strcpy(ptr, pconf);
      ptr += strlen(pconf)+1;
   }
   *ptr = (char)EOF;

   ret = send(comm_sock, request, ptr-request+1, 0);
   if(ret == -1) {
      snprintf(errormsg, errorlen, "send() returned '%s'", strerror(errno));
      return errno;
   }
   if(ret != ptr-request+1) {
      snprintf(errormsg, errorlen, "Not all data sent, cancelling job start.");
      return 255;
   }

   ret = wl_wait_for_answer(comm_sock, job_exit_status, exit_status_len,
                            errormsg, errorlen);
   return ret;
}

/****** wingrid/wl_forward_signal_to_job() ***********************************
*  NAME
*     wl_forward_signal_to_job() -- Forwards a signal to the SGE Helper Service
*                                   which delivers it to the job
*
*  SYNOPSIS
*     int wl_forward_signal_to_job(const char *szjob_id, int *postponed_signal,
*                                                  char *errormsg, int errorlen)
*
*  FUNCTION
*     Forwards a signal to the SGE Helper Service
*
*  INPUTS
*     const char *szjob_id   - the job ID as string
*     int  *postponed_signal - array of signals that should be forwarded
*                              (currently only signal on index 0 will be forwarded)
*     char *errormsg         - buffer for error message
*     int  errorlen          - length of errormsg buffer
*
*  OUTPUTS
*     char *errormsg        - in case of error: the error description
*
*  RESULTS
*     int - 0: Usage was successfully received
*          >0: errno
*         255: not all data was sent, but send() returned no error.
*
*  NOTES
*     MT-NOTE: 
******************************************************************************/
int wl_forward_signal_to_job(const char *szjob_id, 
                             int *postponed_signal,
                             char *errormsg, int errorlen)

{
   int  ret;
   char request[4096];
   char str_signal[10];
   char str_answer[50];
   int  answer_len = 50;
   char *ptr = request;
   int  comm_sock;
   
   ret = wl_connect_to_service(&comm_sock, errormsg, errorlen);
   if(ret != 0) {
      return ret;
   }

   memset(request, 0, 4096);
   request[0] = (char)2;
   ptr = request+4;

   strcpy(ptr, szjob_id);
   ptr += strlen(szjob_id)+1;

   sprintf(str_signal, "%d", *postponed_signal);
   strcpy(ptr, str_signal);
   ptr += strlen(str_signal);

   *ptr = (char)EOF;

   ret = send(comm_sock, request, ptr-request+1, 0);
   if(ret == -1) {
      snprintf(errormsg, errorlen, "send() returned '%s'", strerror(errno));
      return errno;
   }
   if(ret != ptr-request+1) {
      snprintf(errormsg, errorlen, "Not all data sent, cancelling job start.");
      return 255;
   }

   ret = wl_wait_for_answer(comm_sock, str_answer, &answer_len,
                            errormsg, errorlen);
   wl_disconnect_from_service(&comm_sock);

   return ret;
}

/****** wingrid/wl_read_port_from_registry() ********************************
*  NAME
*     wl_read_port_from_registry() -- reads SGE Helper Service's port
*                                     from the registry
*
*  SYNOPSIS
*     int wl_read_port_from_registry(int *pPort) 
*
*  FUNCTION
*     Read SGE Helper Service's port from the Windows registry
*
*  OUTPUTS
*     int *pPort - the port that gets read from the registry
*
*  RESULT
*     int - 0: port read from registry.
*          >0: port not read
*
*  NOTES
*     MT-NOTE: wl_read_port_from_registry() is MT safe
******************************************************************************/
int wl_read_port_from_registry(int *pPort)
{
   size_t   size=sizeof(*pPort);
   int      type;
   int      ret;

   ret = getreg("\\Registry\\Machine\\Software\\Sun Microsystems\\"
                "N1 Grid Engine\\Helper Service\\Port", &type, pPort, &size);
   return ret;
}

/****** wingrid/wl_connect_to_service() *************************************
*  NAME
*     wl_connect_to_service() -- establishes connection to
*                                SGE Helper Service
*
*  SYNOPSIS
*     int wl_connect_to_service(int *comm_sock, char *errormsg, int errorlen)
*
*  FUNCTION
*     Establishes connection to SGE Helper Service
*
*  INPUTS
*     char *errormsg - buffer for error message
*     int  error_len - length of errormsg buffer
*
*  OUTPUTS
*     int  *comm_sock - socket fd of the established connection
*     char *errormsg  - in case of error: the error description
*
*  RESULT
*     int - 0: connection established
*          >0: errno
*
*  NOTES
*     MT-NOTE: wl_connect_to_service() is MT safe
******************************************************************************/
int wl_connect_to_service(int *comm_sock, char *errormsg, int errorlen)
{
   static int         port = 0;
   const char         *ip_addr = "127.0.0.1";
   struct sockaddr_in saddr;
   int                last_error = 0;
   int                ret = 0;
   int                try = 0;

   if (port == 0) {
      ret = wl_read_port_from_registry(&port);
      if (ret != 0) {
         last_error = errno;
         snprintf(errormsg, errorlen, "can't read server port from registry! "
            "%d: %s", errno, strerror(errno));
         return last_error;
      }
   }

   saddr.sin_family      = AF_INET;
   saddr.sin_port        = htons(port);
   saddr.sin_addr.s_addr = inet_addr(ip_addr);

   *comm_sock = socket(AF_INET, SOCK_STREAM, 0);
   if (*comm_sock == -1) {
      last_error = errno;
      snprintf(errormsg, errorlen, "socket() returned '%s'", strerror(errno));
      return last_error;
   }

   while ((ret = connect(*comm_sock, (struct sockaddr*)&saddr, 
                         sizeof(saddr))) == -1 && try < 3) {
      /*
       * Server is perhaps overloaded, give it some time...
       */
      last_error = errno;
      sleep(1);
      try++;
   }

   if (ret == -1) {
      snprintf(errormsg, errorlen, "connect(%s, %d) failed: \"%s\" (errno=%d)",
              ip_addr, port, strerror(last_error), last_error);
      return last_error;
   }
   return 0;
}

/****** wingrid/wl_request_job_start() ***************************************
*  NAME
*     wl_request_job_start() -- sends job start request to
*                               SGE Helper Service
*
*  SYNOPSIS
*     int wl_request_job_start(int comm_sock, char *message, int messagelen,
*                              char* errormsg, int errorlen)
*
*  FUNCTION
*     Sends job start request to the SGE Helper Service, waits blocking
*     for job end.
*
*  INPUTS
*     int  comm_sock  - The socket fd of the connection to SGE Helper Service
*     char *message   - The serialized job
*     int  messagelen - length of message
*     char *errormsg  - buffer for error message
*     int  error_len  - length of errormsg buffer
*
*  OUTPUTS
*     char *errormsg  - in case of error: the error description
*
*  RESULT
*     int - 0: request sent
*          >0: errno
*         255: not all data was sent, but send() returned no error.
*
*  NOTES
*     MT-NOTE: wl_request_job_start() is MT safe
******************************************************************************/
int wl_request_job_start(int comm_sock, char *message, int messagelen,
                         char* errormsg, int errorlen)
{
   int ret;

   ret = send(comm_sock, message, messagelen, 0);
   if(ret == -1) {
      snprintf(errormsg, errorlen, "send() returned '%s'", strerror(errno));
      return errno;
   }
   if(ret != messagelen) {
      snprintf(errormsg, errorlen, "Not all data sent, cancelling job start.");
      return 255;
   }

   memset(message, 0, messagelen);
   ret = wl_wait_for_answer(comm_sock, message, &messagelen,
                            errormsg, errorlen);
   return ret;
}

/****** wingrid/wl_wait_for_answer() *****************************************
*  NAME
*     wl_wait_for_answer() -- waits for an answer from
*                             SGE Helper Service
*
*  SYNOPSIS
*     int wl_wait_for_answer(int comm_sock, char *buffer, int *bufferlen,
*                            char* errormsg, int errorlen)
*
*  FUNCTION
*     Waits blocking for an answer from the SGE Helper Service.
*     The SGE Helper Service gives answers to requests, either an
*     ACK/NAK or the requested information.
*
*  INPUTS
*     int  comm_sock  - The socket fd of the connection to SGE Helper Service
*     char *buffer    - A buffer for the answer
*     int  *bufferlen - The size of the buffer
*     char *errormsg  - buffer for error message
*     int  error_len  - length of errormsg buffer
*
*  OUTPUTS
*     char *buffer    - the answer from the SGE Helper Service
*     int  *bufferlen - the size of the answer
*     char *errormsg  - in case of error: the error description
*
*  RESULT
*     int - 0: answer received
*          >0: errno
*
*  NOTES
*     MT-NOTE: wl_wait_for_answer() is MT safe
*****************************************************************************/
int wl_wait_for_answer(int comm_sock, char *buffer, int *bufferlen,
                       char *errormsg, int errorlen)
{
   int idx = 0;
   int ret = 0;

   while((ret=recv(comm_sock, buffer+idx, *bufferlen-idx, 0))>0) {
      idx += ret;
      if(buffer[idx-1] == EOF) {
         buffer[idx-1] = '\0';
         break;
      }
   }
   if(ret == -1) {
      snprintf(errormsg, errorlen, "recv() returned '%s'", strerror(errno));
      return errno;
   }
   return 0;
}

/****** wingrid/wl_disconnect_from_service() *********************************
*  NAME
*     wl_disconnect_from_service() -- disconnects from
*                                     SGE Helper Service
*
*  SYNOPSIS
*     int wl_disconnect_from_service(int *comm_sock)
*
*  FUNCTION
*     Disconnects from SGE Helper Service
*
*  INPUTS
*     int  *comm_sock  - The socket fd of the connection to SGE Helper Service
*
*  OUTPUTS
*     int  *comm_sock  - -1, if the function succeeded.
*                        Otherwise comm_sock is unmodified.
*
*  RESULT
*     int - 0: successfully disconnected
*          >0: errno
*
*  NOTES
*     MT-NOTE: wl_disconnect_from_service() is MT safe
*****************************************************************************/
int wl_disconnect_from_service(int *comm_sock)
{
   if(comm_sock && *comm_sock >= 0) {
      if(shutdown(*comm_sock, SHUT_RDWR) == -1) {
         return errno;
      }
      if(close(*comm_sock) == -1) {
         return errno;
      }
      *comm_sock = -1;
   }
   return 0; 
}


/****** wingrid/wl_get_GUI_mode() ********************************************
*  NAME
*     wl_get_GUI_mode() -- Converts configuration value to bool
*
*  SYNOPSIS
*     bool wl_get_GUI_mode(const char *conf_val)
*
*  FUNCTION
*     Converts the result of get_conf_val("display_win_gui") to a bool value
*
*  INPUTS
*     const char *conf_val - The result of get_conv_val("display_win_gui"),
*                            which might be "1", "0" or NULL.
*
*  RESULT
*     bool - true:  if *conv_val is "1", 
*            false: otherwise
*
*  NOTES
*     MT-NOTE: wl_wait_for_answer() is MT safe
*****************************************************************************/
bool wl_get_GUI_mode(const char *conf_val)
{
   bool ret = false;

   if(conf_val) {
      ret = (atoi(conf_val) == 1);
   }
   return ret;
}
