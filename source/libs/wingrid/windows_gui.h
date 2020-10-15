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

enum en_JobStatus {
   js_Received           = 0,
   js_ToBeStarted        = 1,
   js_Started            = 2,
   js_Finished           = 3,
   js_Failed             = 4,
   js_Deleted            = 5,
   js_Invalid            = -1
};


int wl_request_job_usage(int comm_sock, char *job_results, int *resultslen,
                           char* errormsg, int errorlen);

int wl_getrusage_remote(const char *szjob_id, int *pstatus, 
                        struct rusage *prusage, char *pszerrormsg);

int wl_request_job_exit_status(int comm_sock, char *job_exit_status,
                           int *exit_status_len, char *errormsg, int errorlen);


int wl_start_job_remote(const char *filename, 
                        char **args,
                        char **env,
                        const char *job_user,
                        const char *user_passwd,
                        int  *pwin32_exit_status,
                        enum en_JobStatus *pjob_status,
                        char *err_str);

int wl_forward_signal_to_job(const char *szjob_id, 
                             int *postponed_signal,
                             char *errormsg, int errorlen);

int wl_read_port_from_registry(int *port);
int wl_connect_to_service(int *comm_sock, char *errormsg, int errorlen);
int wl_request_job_start(int comm_sock, char *message, int messagelen,
                         char* errormsg, int errorlen);
int wl_wait_for_answer(int comm_sock, char *buffer, int *bufferlen,
                       char *errormsg, int errorlen);
int wl_disconnect_from_service(int *comm_sock);

bool wl_get_GUI_mode(const char *conf_val);

