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

#include <winsock2.h>
#include <stdio.h>
#include "Job.h"
#include "Logging.h"

/****** C_Job::C_Job() *********************************************************
*  NAME
*     C_Job::C_Job() -- constructor of the job object
*
*  SYNOPSIS
*     C_Job::C_Job()
*
*  FUNCTION
*     Initializes an empty job object.
*
*  NOTES
*******************************************************************************/
C_Job::C_Job()
{
   next = NULL;
   m_JobStatus  = js_Invalid;

   m_job_id     = 0;
   m_ja_task_id = 0;
   m_pe_task_id = NULL;
   m_comm_sock  = 0;
   m_hProcess   = INVALID_HANDLE_VALUE;
   m_hJobObject = NULL;
   m_ForwardedSignal = 0;

   // data members to start the job
   jobname = NULL;
   nargs   = 0;
   args    = NULL;
   nconf   = 0;
   conf    = NULL;
   nenv    = 0; 
   env     = NULL;
   user    = NULL;
   pass    = NULL;
   domain  = NULL;

   // data members that hold results and usage
   mem     = 0;
   cpu     = 0;
   vmem    = 0;

   dwExitCode = 0;    // Exit code of job, if it ran
   szError    = NULL; // Error message, in case of error
   lUserSec   = 0;    // User part of job run time
   lUserUSec  = 0;    // User part of job run time usec
   lKernelSec = 0;    // Kernel part of job run time
   lKernelUSec = 0;    // Kernel part of job run time usec
}

/****** C_Job::C_Job() *********************************************************
*  NAME
*     C_Job::C_Job() -- copy-constructor of the job object
*
*  SYNOPSIS
*     C_Job::C_Job(const C_Job& otherJob)
*
*  FUNCTION
*     Copies the content of otherJob to this job object.
*
*  NOTES
*******************************************************************************/
C_Job::C_Job(const C_Job& otherJob)
{
   int i;

   next         = otherJob.next;

   m_JobStatus  = otherJob.m_JobStatus;

   m_job_id     = otherJob.m_job_id;
   m_ja_task_id = otherJob.m_ja_task_id;
   m_pe_task_id = otherJob.m_pe_task_id ? _strdup(otherJob.m_pe_task_id) : NULL;
   m_comm_sock  = otherJob.m_comm_sock;
   m_hProcess   = otherJob.m_hProcess;
   m_hJobObject = otherJob.m_hJobObject;
   m_ForwardedSignal = otherJob.m_ForwardedSignal;

   // data members to start the job
   jobname = otherJob.jobname ? _strdup(otherJob.jobname) : NULL;

   nargs   = otherJob.nargs;
   args = new char*[nargs];
   for(i=0; i<nargs; i++) {
      args[i] = _strdup(otherJob.args[i]);
   }

   nconf   = otherJob.nconf;
   conf = new char*[nconf];
   for(i=0; i<nconf; i++) {
      conf[i] = _strdup(otherJob.conf[i]);
   }

   nenv    = otherJob.nenv;
   env = new char*[nenv];
   for(i=0; i<nenv; i++) {
      env[i] = _strdup(otherJob.env[i]);
   }

   user    = otherJob.user   ? _strdup(otherJob.user)   : NULL;
   pass    = otherJob.pass   ? _strdup(otherJob.pass)   : NULL;
   domain  = otherJob.domain ? _strdup(otherJob.domain) : NULL;

   // data members that hold results and usage
   mem     = otherJob.mem;
   cpu     = otherJob.cpu;
   vmem    = otherJob.vmem;

   dwExitCode  = otherJob.dwExitCode;   // Exit code of job
   szError     = otherJob.szError ? _strdup(otherJob.szError) : NULL;
   lUserSec    = otherJob.lUserSec;     // User part of job run time
   lUserUSec   = otherJob.lUserUSec;    // User part of job run time usec
   lKernelSec  = otherJob.lKernelSec;   // Kernel part of job run time
   lKernelUSec = otherJob.lKernelUSec;  // Kernel part of job run time usec
}

/****** C_Job::~C_Job() *********************************************************
*  NAME
*     C_Job::~C_Job() -- destructor of the job object
*
*  SYNOPSIS
*     C_Job::~C_Job()
*
*  FUNCTION
*     cleanes up the object, frees all allocated memory.
*
*  NOTES
*******************************************************************************/
C_Job::~C_Job()
{
   FreeAllocatedMembers();
}

/****** C_Job::ParseCommand() **************************************************
*  NAME
*     C_Job::ParseCommand() -- parses the command received from sge_shepherd
*
*  SYNOPSIS
*     en_request_type C_Job::ParseCommand(char *command, C_Job &Job)
*
*  FUNCTION
*     Parses the command received from sge_shepherd. Depending on the type of
*     the request, it fills different parts of the Job object with the data
*     from the command.
*
*  INPUTS
*     char *command - the command received from sge_shepherd
*
*  RESULT
*     en_request_type - type of the request
*     req_job_start:      command contains a job that is to be started
*     req_send_job_usage: request to send usage data of a job that has been
*                         started before
*
*  NOTES
*******************************************************************************/
en_request_type C_Job::ParseCommand(char *command)
{
   int i;
   en_request_type request_type;

   request_type = *((en_request_type*)command);
   command += sizeof(en_request_type);

   switch(request_type) {
      case req_job_start:
         jobname = _strdup(command);
         command += strlen(command)+1;

         sscanf(command, "%d", &(nargs));
         command += strlen(command)+1;
         args = new char*[nargs];

         for(i=0; i<nargs; i++) {
            args[i] = _strdup(command);
            command += strlen(command)+1;
         }

         sscanf(command, "%d", &(nconf));
         command += strlen(command)+1;
         conf = new char*[nconf];

         for(i=0; i<nconf; i++) {
            conf[i] = _strdup(command);
            command += strlen(command)+1;
         }

         sscanf(command, "%d", &(nenv));
         command += strlen(command)+1;
         env = new char*[nenv];

         for(i=0; i<nenv; i++) {
            env[i] = _strdup(command);
            command += strlen(command)+1;
         }

         user = _strdup(command);
         command += strlen(command)+1;

         pass = _strdup(command);
         command += strlen(command)+1;

         domain = _strdup(command);

         m_job_id     = atol(GetConfValue("job_id"));
         m_ja_task_id = atol(GetConfValue("ja_task_id"));
         m_pe_task_id = GetConfValue("pe_task_id");
         break;

      case req_send_job_usage:
         sscanf(command, "%d", &(nconf));
         command += strlen(command)+1;
         conf = new char*[nconf];

         for(i=0; i<nconf; i++) {
            conf[i] = _strdup(command);
            command += strlen(command)+1;
         }
         m_job_id     = atol(GetConfValue("job_id"));
         m_ja_task_id = atol(GetConfValue("ja_task_id"));
         m_pe_task_id = GetConfValue("pe_task_id");
         break;

      case req_forward_signal:
         sscanf(command, "%d", &m_job_id);
         command += strlen(command)+1;

         sscanf(command, "%d", &m_ForwardedSignal);
         break;

      default:
         request_type = req_error;
   }

   m_JobStatus  = js_Received;

   return request_type;
}

/****** C_Job::GetConfValue() const ********************************************
*  NAME
*    C_Job::GetConfValue() const -- retrieves a specific configuration value
*                                   from job object
*
*  SYNOPSIS
*    char* C_Job::GetConfValue(const char *pszName)
*
*  FUNCTION
*    retrieves a specific configuration value from the configuration list in
*    the job object.
*
*  INPUTS
*    const char *pszName - the name of the configuration value
*
*  RESULT
*     char* - pointer to the configuration value in the job object. If the job
*             object is deleted, this pointer becomes invalid!
*
*  NOTES
*******************************************************************************/
const char* C_Job::GetConfValue(const char *pszName) const
{
   int  i;
   char *pToken, *pConf, *pTmp, *pRet = NULL;

   for(i=0; i<nconf; i++) {
      pConf = conf[i];
      pTmp = _strdup(pConf);

      pToken = strtok(pTmp, "=");
      if(strcmp(pToken, pszName) == 0) {
         pRet = strchr(pConf, '=');
         pRet++;
         free(pTmp);
         break;
      }
      free(pTmp);
   }
   return pRet;
}
 
/****** C_Job::Serialize() const ***********************************************
*  NAME
*    C_Job::Serialize() const -- Writes this C_Job object to a file
*
*  SYNOPSIS
*    int C_Job::Serialize(HANDLE hFile) const
*
*  FUNCTION
*    Writes this C_Job object to a file.
*
*  INPUTS
*    HANDLE hFile - Handle of the file opened for writing.
*
*  RESULT
*     int - Number of Bytes written to the file if writing succeeded,
*           -1 if writing failed.
*******************************************************************************/
int C_Job::Serialize(HANDLE hFile) const
{
   BOOL  bRet;
   int   i;
   char  *pszBuffer;
   char  szTemp[1000];
   DWORD dwWritten = 0;
   DWORD dwBufLen  = 100000;

   pszBuffer = new char[dwBufLen];
   ZeroMemory(pszBuffer, dwBufLen);

   sprintf(szTemp, "%d\n",  m_JobStatus);     strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%ld\n", m_job_id);        strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%ld\n", m_ja_task_id);    strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%s\n",  m_pe_task_id ? m_pe_task_id : "<null>"); strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%d\n",  m_comm_sock);     strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%p\n",  m_hProcess); strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%d\n", m_ForwardedSignal); strcat(pszBuffer, szTemp);

   sprintf(szTemp, "%s\n",  jobname ? jobname : "<null>");           strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%d\n",  nargs);        strcat(pszBuffer, szTemp);
   for(i=0; i<nargs; i++) {
      sprintf(szTemp, "%s\n", args[i]);    strcat(pszBuffer, szTemp);
   }

   sprintf(szTemp, "%d\n",  nconf);        strcat(pszBuffer, szTemp);
   for(i=0; i<nconf; i++) {
      sprintf(szTemp, "%s\n", conf[i]);    strcat(pszBuffer, szTemp);
   }

   sprintf(szTemp, "%d\n",  nenv);         strcat(pszBuffer, szTemp);
   for(i=0; i<nenv; i++) {
      sprintf(szTemp, "%s\n", env[i]);     strcat(pszBuffer, szTemp);
   }

   sprintf(szTemp, "%s\n",  user ? user : "<null>");     strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%s\n",  pass ? pass : "<null>");     strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%s\n",  domain ? domain : "<null>"); strcat(pszBuffer, szTemp);

   sprintf(szTemp, "%ld\n", mem);          strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%ld\n", cpu);          strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%ld\n", vmem);         strcat(pszBuffer, szTemp);

   sprintf(szTemp, "%ld\n", dwExitCode);   strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%s\n",  szError);      strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%ld\n", lUserSec);     strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%ld\n", lUserUSec);    strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%ld\n", lKernelSec);   strcat(pszBuffer, szTemp);
   sprintf(szTemp, "%ld\n", lKernelUSec);  strcat(pszBuffer, szTemp);

   dwBufLen = (DWORD)strlen(pszBuffer);
   bRet = WriteFile(hFile, pszBuffer, dwBufLen, &dwWritten, NULL);
   delete pszBuffer;

   return bRet ? dwWritten : -1;
}

/****** C_Job::Unserialize() ***************************************************
*  NAME
*    C_Job::Unserialize() -- Reads this C_Job object from a file
*
*  SYNOPSIS
*    int C_Job::Unserialize(HANDLE hFile)
*
*  FUNCTION
*    Reads this C_Job object from a file.
*
*  INPUTS
*    HANDLE hFile - Handle of the file opened for reading.
*
*  RESULT
*     int - Number of Bytes read from the file if reading succeeded,
*           -1 if reading failed.
*******************************************************************************/
int C_Job::Unserialize(HANDLE hFile)
{
   BOOL  bRet;
   int   i;
   char  *pszBuffer;
   char  *pszTemp;
   DWORD dwRead   = 0;
   DWORD dwBufLen = 100000;

   pszBuffer = new char[dwBufLen];
   ZeroMemory(pszBuffer, dwBufLen);

   FreeAllocatedMembers();
   bRet = ReadFile(hFile, pszBuffer, dwBufLen, &dwRead, NULL);

   if(bRet) {
      pszTemp = strtok(pszBuffer, "\n");
      sscanf(pszTemp, "%d\n",  &m_JobStatus);   pszTemp = strtok(NULL, "\n");
      sscanf(pszTemp, "%ld\n", &m_job_id);      pszTemp = strtok(NULL, "\n");
      sscanf(pszTemp, "%ld\n", &m_ja_task_id);  pszTemp = strtok(NULL, "\n");

      if(strcmp(pszTemp, "<null>")!=0) { m_pe_task_id = _strdup(pszTemp); }
                                                   pszTemp = strtok(NULL, "\n");
      sscanf(pszTemp, "%d\n", &m_comm_sock);       pszTemp = strtok(NULL, "\n");
      sscanf(pszTemp, "%p\n", &m_hProcess);        pszTemp = strtok(NULL, "\n");
      sscanf(pszTemp, "%d\n", &m_ForwardedSignal); pszTemp = strtok(NULL, "\n");

      if(strcmp(pszTemp, "<null>")!=0) { jobname = _strdup(pszTemp); }
                                                pszTemp = strtok(NULL, "\n");
      sscanf(pszTemp, "%d\n",  &nargs);         pszTemp = strtok(NULL, "\n");
      args = new char*[nargs];
      for(i=0; i<nargs; i++) {
         args[i] = _strdup(pszTemp);             pszTemp = strtok(NULL, "\n");
      }

      sscanf(pszTemp, "%d\n",  &nconf);         pszTemp = strtok(NULL, "\n");
      conf = new char*[nconf];
      for(i=0; i<nconf; i++) {
         conf[i] = _strdup(pszTemp);             pszTemp = strtok(NULL, "\n");
      }

      sscanf(pszTemp, "%d\n",  &nenv);          pszTemp = strtok(NULL, "\n");
      env = new char*[nenv];
      for(i=0; i<nenv; i++) {
         env[i] = _strdup(pszTemp);              pszTemp = strtok(NULL, "\n");
      }

      if(strcmp(pszTemp, "<null>")!=0) { user = _strdup(pszTemp); }
                                                pszTemp = strtok(NULL, "\n");
      if(strcmp(pszTemp, "<null>")!=0) { pass = _strdup(pszTemp); }
                                                pszTemp = strtok(NULL, "\n");
      if(strcmp(pszTemp, "<null>")!=0) { domain = _strdup(pszTemp); }
                                                pszTemp = strtok(NULL, "\n");

      sscanf(pszTemp, "%ld\n", &mem);           pszTemp = strtok(NULL, "\n");
      sscanf(pszTemp, "%ld\n", &cpu);           pszTemp = strtok(NULL, "\n");
      sscanf(pszTemp, "%ld\n", &vmem);          pszTemp = strtok(NULL, "\n");

      sscanf(pszTemp, "%ld\n", &dwExitCode);    pszTemp = strtok(NULL, "\n");
      if(strcmp(pszTemp, "<null>")!=0) { szError = _strdup(pszTemp); }
                                                pszTemp = strtok(NULL, "\n");
      sscanf(pszTemp, "%ld\n", &lUserSec);      pszTemp = strtok(NULL, "\n");
      sscanf(pszTemp, "%ld\n", &lUserUSec);     pszTemp = strtok(NULL, "\n");
      sscanf(pszTemp, "%ld\n", &lKernelSec);    pszTemp = strtok(NULL, "\n");
      sscanf(pszTemp, "%ld\n", &lKernelUSec);
   }
   delete pszBuffer;

   return bRet ? dwRead : -1;
}

/****** C_Job::FreeAllocatedMembers() ******************************************
*  NAME
*    C_Job::FreeAllocatedMembers() -- Frees all dynamically allocated buffers
*
*  SYNOPSIS
*    void C_Job::FreeAllocatedMembers()
*
*  FUNCTION
*    Frees all dynamically allocated buffers of the object.
* 
*  RESULT
*     void - none
*******************************************************************************/
void C_Job::FreeAllocatedMembers()
{
   int i;

   free(jobname);
   jobname = NULL;

   for(i=0; i<nargs; i++) {
      free(args[i]);
   }
   free(args);
   args = NULL;
   nargs = 0;

   for(i=0; i<nconf; i++) {
      free(conf[i]);
   }
   free(conf);
   conf = NULL;
   nconf = 0;

   for(i=0; i<nenv; i++) {
      free(env[i]);
   }
   free(env);
   env = NULL;
   nenv = 0;

   free(user);
   user = NULL;

   if (pass != NULL) {
      SecureZeroMemory(pass, strlen(pass));
   }
   free(pass);
   pass = NULL;

   free(domain);
   domain = NULL;

   free(szError);
   szError = NULL;
}

/****** C_Job::BuildCommandLine() **********************************************
*  NAME
*    C_Job::BuildCommandLine()-- builds one command line from the job arguments
*
*  SYNOPSIS
*    void C_Job::BuildCommandLine(char *&szCmdLine) const
*
*  FUNCTION
*    build one command line from the job arguments
*
*  OUTPUTS
*     char *&szCmdLine - all arguments as one command line. Memory for the
*                        command line gets allocated in this function - free
*                        it after usage.
*     
*  RESULT
*     void - no result
*
*  NOTES
*******************************************************************************/
void C_Job::BuildCommandLine(char *&szCmdLine) const
{
   int    i;
   size_t nCmdLineSize = 0;
/*
   nCmdLineSize += strlen("SGE_Redirector.exe ");
   nCmdLineSize += strlen(Job.GetConfValue("stderr_path"));
   nCmdLineSize ++;
   nCmdLineSize += strlen(Job.GetConfValue("stdout_path"));
   nCmdLineSize ++;
*/
   nCmdLineSize += strlen(jobname);
   for(i=1; i<nargs; i++) {
      nCmdLineSize++;
      nCmdLineSize += strlen(args[i]);
   }

   szCmdLine = (char*)malloc(nCmdLineSize+1);
   ZeroMemory(szCmdLine, nCmdLineSize+1);
/*
   strcat(szCmdLine, "SGE_Redirector.exe ");
   strcat(szCmdLine, Job.GetConfValue("stdout_path"));
   strcat(szCmdLine, " ");
   strcat(szCmdLine, Job.GetConfValue("stderr_path"));
   strcat(szCmdLine, " ");
*/
   strcat(szCmdLine, jobname);
   for(i=1; i<nargs; i++) {
      strcat(szCmdLine, " ");
      strcat(szCmdLine, args[i]);
   }
}

/****** C_Job::BuildEnvironment() **********************************************
*  NAME
*    C_Job::BuildEnvironment() -- builds the job environment from the environment
*                                 variable list in the job object
*
*  SYNOPSIS
*    void BuildEnvironment(char *&psEnv) const
*
*  FUNCTION
*    build the environment in a single block of memory from the environment
*    variable list of the job object.
*
*  OUTPUTS
*     char *&pszEnv - the environment in a single block of memory. Memory
*                     gets allocated in this function - free it after usage.
*     
*  RESULT
*     void - no result
*
*  NOTES
*******************************************************************************/
void C_Job::BuildEnvironment(char *&pszEnv) const
{
   C_MapStringToString mapSysEnv;
   C_MapStringToString mapMergedEnv;

   // Merge pathes, let job environment win over system environment for all other variables
   BuildSysEnvTable(mapSysEnv);
   MergeSysEnvTableWithJobEnvTable(mapSysEnv, mapMergedEnv);
   BuildEnvironmentFromTable(mapMergedEnv, pszEnv);
}

/****** C_Job::BuildTableFromJobEnv() const ************************************
*  NAME
*    C_Job::BuildTableFromJobEnv() const -- Builds a StringToStringMap from
*                                           the job environment
*
*  SYNOPSIS
*    void C_Job::BuildTableFromJobEnv(C_MapStringToString &mapJobEnv) const
*
*  FUNCTION
*    Builds a C_MapStringToString map object from the job environment.
*
*  INPUTS
*    C_MapStringToString &mapJobEnv - Reference to the map object
*
*  RESULT
*    void - none
*******************************************************************************/
void C_Job::BuildTableFromJobEnv(C_MapStringToString &mapJobEnv) const
{
   int     i;
   char    *pTmp;
   char    *szKey, *szValue;

   for(i=0; i<nenv; i++) {
      pTmp    = _strdup(env[i]);
      szKey   = _strdup(strtok(pTmp, "="));
      szValue = _strdup(strtok(NULL, ""));
      mapJobEnv.SetAt(_strupr(szKey), szValue);
      free(pTmp);
   }
}

/****** C_Job::IsEnvAPath() const **********************************************
*  NAME
*    C_Job::IsEnvAPath() const -- checks if the environment key is 
*                                 either PATH or LD_LIBRARY_PATH
*
*  SYNOPSIS
*    BOOL C_Job::IsEnvAPath(const char *szKey) const
*
*  FUNCTION
*    Checks if the given environment key is either PATH or LD_LIBRARY_PATH.
*
*  INPUTS
*    const char *szKey - The environment key
*
*  RESULT
*    BOOL - TRUE if it is either PATH or LD_LIBRARY_PATH,
*           FALSE if not
*******************************************************************************/
BOOL C_Job::IsEnvAPath(const char *szKey) const
{
   static char *szPathes[] = {"PATH", "LD_LIBRARY_PATH"};

   return strcmp(szPathes[0], szKey) == 0 ||
          strcmp(szPathes[1], szKey) == 0;
}

/****** C_Job::PathDelimiter() const *******************************************
*  NAME
*    C_Job::PathDelimiter() const -- Returns the right path delimiter for the
*                                    given path format (Unix or Windows)
*
*  SYNOPSIS
*    void C_Job::PathDelimiter(const char *szPath) const
*
*  FUNCTION
*    Returns the right path delimiter for the given path format (Unix or
*    Windows). Default is Windows path format, if it can't be determined.
*
*  INPUTS
*    const char *szPath - The path defining the format and therefore
*                         the delimiter
*
*  RESULT
*    char* - The path delimiter
*******************************************************************************/
char* C_Job::PathDelimiter(const char *szPath) const
{
   if (strstr(szPath, "/") != NULL) {
      return ":";
   } else {
      return ";";
   }
}

/****** C_Job::MergeSysEnvTableWithJobEnvTable() const *************************
*  NAME
*    C_Job::MergeSysEnvTableWithJobEnvTable() const --
*                                            Merges the two environment tables
*
*  SYNOPSIS
*    void C_Job::MergeSysEnvTableWithJobEnvTable(C_MapStringToString &mapSysEnv,  
*                                        C_MapStringToString &mapMergedEnv) const
*
*  FUNCTION
*    Merges the two environments from the system and this job object.
*
*  INPUTS
*    C_MapStringToString &mapSysEnv    - Copy of the system environment
* 
*  OUTPUTS
*    C_MapStringToString &mapSysEnv    - Modified copy of the system environemnt.
*                                       All variables that were merged to the
*                                       merged env were removed from the copy of
*                                       the system environment.
*    C_MapStringToString &mapMergedEnv - The merged environment.
*
*  RESULT
*    void - none
*******************************************************************************/
void C_Job::MergeSysEnvTableWithJobEnvTable(C_MapStringToString &mapSysEnv,
                                            C_MapStringToString &mapMergedEnv) const
{
   char *szSysValue;
   // Copy whole job env into merged env
   BuildTableFromJobEnv(mapMergedEnv);
// TODO: Take care of case of the keys (=variables)!
   // Search for all duplicates in the maps, 
   // remove duplicate ordinary variables from system env, merge pathes

   t_MapElem *pElem = mapMergedEnv.m_pFirst;

   while (pElem != NULL) {
      szSysValue = mapSysEnv.Lookup(pElem->szKey);
      if (szSysValue != NULL) {
         if (IsEnvAPath(pElem->szKey)) {
            size_t new_length = strlen(pElem->szValue)
                             + strlen(PathDelimiter(pElem->szValue))
                             + strlen(szSysValue) + 1;
            char* new_szValue = (char*)malloc(new_length);
            char *old_szValue = pElem->szValue;

            strcpy(new_szValue, old_szValue);
            strcat(new_szValue, PathDelimiter(old_szValue));
            strcat(new_szValue, szSysValue);

            pElem->szValue = new_szValue;
            free(old_szValue);
            old_szValue = NULL;
         }
         mapSysEnv.RemoveKey(pElem->szKey);
      }
      pElem = pElem->next;
   }

   // Append remainder of system env to merged env
   mapMergedEnv.Append(mapSysEnv);
}

/****** C_Job::BuildSysEnvTable() const ****************************************
*  NAME
*    C_Job::BuildSysEnvTable() const -- Copies the system environment to a 
*                                       StringToString map
*
*  SYNOPSIS
*    void C_Job::BuildSysEnvTable(C_MapStringToString &mapSysEnv) const
*
*  FUNCTION
*    Copies the systen environment to a C_MapStringToString map object.
*
*  OUTPUT
*    C_MapStringToString &mapSysEnv - Reference to the map object
*
*  RESULT
*    void - none
*******************************************************************************/
void C_Job::BuildSysEnvTable(C_MapStringToString &mapSysEnv) const
{
   char *pszEnv;
   char *pLine;
   char *pToken;
   char *pKey;

   pszEnv = GetEnvironmentStrings();

   pLine = pszEnv;
   while(*pLine != '\0') {
      pKey = strtok(pLine, "=");
      pToken = pKey+strlen(pKey)+1;
      mapSysEnv.SetAt(_strupr(pKey), pToken);
      pLine = pToken+strlen(pToken)+1;
   }
   FreeEnvironmentStrings(pszEnv);
}

/****** C_Job::BuildEnvironmentFromTable() *************************************
*  NAME
*    void C_Job::BuildEnvironmentFromTable() const - Creates the job environment
*                                                    from a StringToString map.
*
*  SYNOPSIS
*    void C_Job::BuildEnvironmentFromTable(const C_MapStringToString &mapMergedEnv, 
*                                          char *&pszEnv) const
*
*  FUNCTION
*    Builds the job environment from a C_MapStringToString map object.
*
*  INPUTS
*    C_MapStringToString &mapMergedEnv - Reference to the map object containing
*                                       the merged environment (merged of 
*                                       system and job environment)
*
*  OUTPUTS
*    char *&pszEnv - Reference to a pointer pointing to the environment buffer
*                    that gets allocated in this function. After usage, free 
*                    it with free().
*
*  RESULT
*    void - none
*******************************************************************************/
void C_Job::BuildEnvironmentFromTable(const C_MapStringToString &mapMergedEnv, 
                                      char *&pszEnv) const
{
   char     *ptr;
   size_t   nEnvSize = 0;

   t_MapElem *pElem = mapMergedEnv.m_pFirst;

   while (pElem != NULL) {
      nEnvSize += strlen(pElem->szKey) + strlen("=") + strlen(pElem->szValue) + 1;
      pElem = pElem->next;
   }
   nEnvSize++;

   // Allocate environment buffer, copy system and job environment
   // to buffer
   pszEnv = (char*)malloc(nEnvSize);
   ptr    = pszEnv;
   pElem  = mapMergedEnv.m_pFirst;
   while (pElem != NULL) {
      sprintf(ptr, "%s=%s", pElem->szKey, pElem->szValue);
      ptr += strlen(ptr)+1;
      pElem = pElem->next;
   }
   *ptr = '\0';
}

/****** C_Job::Terminate() *****************************************************
*  NAME
*     C_Job::Terminate() -- terminates all processes in the Windows job object
*                           associated to this C_Job object.
*
*  SYNOPSIS
*     int C_Job::Terminate()
*
*  FUNCTION
*    Terminates all processes in the Windows job object associated to this
*    C_Job object. This avoids getting zombies and orphans of this job.
*
*  RESULT
*     int
*     0:  All processes where terminated successfully.
*     >0: value of GetLastError()
*
*  NOTES
*******************************************************************************/
int C_Job:: Terminate()
{
   if (TerminateJobObject(m_hJobObject, 999) == FALSE) {
      return GetLastError();
   }
   return 0;
}

/****** C_Job::StoreUsage() ****************************************************
*  NAME
*     C_Job::StoreUsage() -- retrieves job usage from the system and stores it
*                            in the C_Job object.
*
*  SYNOPSIS
*     int C_Job::StoreUsage()
*
*  FUNCTION
*    retrieves job usage from the system and stores it in the C_Job object.
*
*  RESULT
*     DWORD 
*     0: the usage was retrieved successfully
*     1: can't get the exit code of the job 
*     2: can't get the usage of the job
*
*  NOTES
*     Call this function only when the main process of the job has terminated.
*******************************************************************************/
int C_Job::StoreUsage()
{
   JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION JobInfo;
   DWORD                                         dwReturnLength = 0;
   int                                           ret = 0;

   try {
      if(GetExitCodeProcess(m_hProcess, &dwExitCode) == FALSE) {
         throw 1;
      }

      ZeroMemory(&JobInfo, sizeof(JobInfo));
      if (QueryInformationJobObject(m_hJobObject, JobObjectBasicAndIoAccountingInformation,
             &JobInfo, sizeof(JobInfo), &dwReturnLength) == FALSE) {
         throw 2;
      }
      lUserSec    = (long)((_int64)JobInfo.BasicInfo.TotalUserTime.QuadPart/10000000);
      lUserUSec   = (long)((_int64)JobInfo.BasicInfo.TotalUserTime.QuadPart%10000000) / 10;
      lKernelSec  = (long)((_int64)JobInfo.BasicInfo.TotalKernelTime.QuadPart/1000000);
      lKernelUSec = (long)((_int64)JobInfo.BasicInfo.TotalKernelTime.QuadPart%1000000) / 10;
   }
   catch (int retval) {
      ret = retval;
   }
   return ret;
}

C_MapStringToString::C_MapStringToString()
{
   m_pFirst = NULL;
}

C_MapStringToString::~C_MapStringToString()
{
   t_MapElem *pElem = m_pFirst;
   t_MapElem *pNext = NULL;

   while (pElem != NULL) {
      pNext = pElem->next;
      free(pElem->szKey);
      free(pElem->szValue);
      delete pElem;
      pElem = pNext;
   }
}

void C_MapStringToString::DumpToLogFile()
{
   t_MapElem *pElem = m_pFirst;

   WriteToLogFile("Map content: Elem, Key, Value, next");
   WriteToLogFile("Start");

   while (pElem != NULL) {
      WriteToLogFile("%p, %s, %s, %p", pElem, pElem->szKey, pElem->szValue, pElem->next);
      pElem = pElem->next;
   }

   WriteToLogFile("End");
}

void C_MapStringToString::SetAt(const char *szKey, const char *szValue)
{
   // Search for the key - if it already exists, replace it's value
   t_MapElem *pElem = m_pFirst;
   while (pElem != NULL) {
      if (strcmp(pElem->szKey, szKey) == 0) {
         free(pElem->szValue);
         pElem->szValue = _strdup(szValue);
         break;
      }
      pElem = pElem->next;
   }

   if (pElem == NULL) {
      // Key does not exist -> create elem, fill and append it
      t_MapElem *pElem = new t_MapElem;
      pElem->szKey   = _strdup(szKey);
      pElem->szValue = _strdup(szValue);
      pElem->next    = m_pFirst;
      m_pFirst = pElem;
   }
}

void C_MapStringToString::Append(const C_MapStringToString &mapOther)
{
   t_MapElem *pOther = mapOther.m_pFirst;
   t_MapElem *pOtherCopy = NULL;

   while (pOther != NULL) {
      pOtherCopy          = new t_MapElem;
      pOtherCopy->szKey   = _strdup(pOther->szKey);
      pOtherCopy->szValue = _strdup(pOther->szValue);
      pOtherCopy->next    = m_pFirst;
      m_pFirst            = pOtherCopy;
      pOther = pOther->next;
   }
}

void C_MapStringToString::RemoveKey(const char *szKey)
{
   t_MapElem *pPrev = NULL;
   t_MapElem *pElem = m_pFirst;
   while (pElem != NULL) {
      if (strcmp(pElem->szKey, szKey) == 0) {
         if (pPrev == NULL) {
            m_pFirst = pElem->next;
            free(pElem->szKey);
            free(pElem->szValue);
            delete pElem;
            pElem = m_pFirst;
         } else {
            pPrev->next = pElem->next;
            free(pElem->szKey);
            free(pElem->szValue);
            delete pElem;
            pElem = pPrev;
         }
      }
      pPrev = pElem;
      pElem = pElem->next;
   }
}

char *C_MapStringToString::Lookup(const char *szKey)
{
   char      *szRet = NULL;
   t_MapElem *pElem = m_pFirst;

   while (pElem != NULL) {
      if (strcmp(pElem->szKey, szKey) == 0) {
         szRet = pElem->szValue;
         break;
      }
      pElem = pElem->next;
   }
   return szRet;
}
