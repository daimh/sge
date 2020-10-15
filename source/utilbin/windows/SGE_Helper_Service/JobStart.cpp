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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <Psapi.h>
#include <Wtsapi32.h>
#include <AccCtrl.h>
#include <Aclapi.h>
#include <Sddl.h>
#include <tchar.h>


#include "Job.h"
#include "JobList.h"
#include "JobStart.h"
#include "SGE_Helper_Service.h"
#include "Communication.h"
#include "Logging.h"

#define DESKTOP_ALL (DESKTOP_READOBJECTS | DESKTOP_CREATEWINDOW | \
DESKTOP_CREATEMENU | DESKTOP_HOOKCONTROL | DESKTOP_JOURNALRECORD | \
DESKTOP_JOURNALPLAYBACK | DESKTOP_ENUMERATE | DESKTOP_WRITEOBJECTS | \
DESKTOP_SWITCHDESKTOP | STANDARD_RIGHTS_REQUIRED)

#define WINSTA_ALL (WINSTA_ENUMDESKTOPS | WINSTA_READATTRIBUTES | \
WINSTA_ACCESSCLIPBOARD | WINSTA_CREATEDESKTOP | WINSTA_WRITEATTRIBUTES | \
WINSTA_ACCESSGLOBALATOMS | WINSTA_EXITWINDOWS | WINSTA_ENUMERATE | \
WINSTA_READSCREEN | STANDARD_RIGHTS_REQUIRED)

#define GENERIC_ACCESS (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | \
GENERIC_ALL)

// function forward declarations
static BOOL GetLogonSID(HANDLE hToken, PSID &pSid);
static void FreeLogonSID(PSID &pSid);
static BOOL AddAceToWindowStation(HWINSTA hWinsta, PSID pSid);
static BOOL AddAceToDesktop(HDESK hDesk, PSID pSid);
static BOOL RemoveAceFromWindowStation(HWINSTA hWinsta, PSID pSid);
static BOOL RemoveAceFromDesktop(HDESK hDesk, PSID pSid);
static BOOL GetJobStartModeFromConf(char **conf, int nconf);
static BOOL GetModeFromEnv(const char *mode, char **env, int nenv);
static void WriteEnvToFile(char *pszEnv, char *pszFile);
static DWORD RedirectStdHandles(const C_Job& Job, HANDLE &hStdout, HANDLE &hStderr);
static HANDLE WINAPI GetInteractiveUserToken();
static BOOL IsSystemWindowsVista();
static int  GetModuleDir(char *szDirName, int nSize);
static void GetDirName(char *szDirName, char delim);
static int CreatePipeForStarter(char **pszPipeName, HANDLE &hPipe);
static int BuildCommandLineForStarter(const C_Job &Job, const char *pszPipeName,
                                      const char *pszCmdLineArgs, char **pszStarterCmdLine);
static int WritePasswordToPipe(const HANDLE &hPipe, const HANDLE &hProcess,
                               const char *szPassword, char *szError);
static HANDLE WINAPI GetRemoteDesktopUserToken();

// external variables
extern C_JobList       g_JobList;
extern C_Communication g_Communication;
extern HANDLE          g_hWinstaACLMutex;
extern HANDLE          g_hDeskACLMutex;
extern BOOL            g_bDoLogging;

// static variables
static char *szStarterError[] = {
   "Ok",
   "Missing command line arguments",
   "Can't create job process",
   "Got invalid job process handle",
   "Can't get exit code of job process"
};

/****** JobStarterThread() ****************************************************
*  NAME
*     JobStarterThread() -- starting point of job starter thread
*
*  SYNOPSIS
*     DWORD WINAPI JobStarterThread(LPVOID lpParameter)
*
*  FUNCTION
*     This is the starting point for the job starter thread
*     It searches a job with js_Received from the job list and tries to
*     start it. It sends a response to the sge_shepherd if job was
*     successfully or unsuccessfully started.
*
*  INPUTS
*     LPVOID lpParameter - unused
*
*  RESULT
*     DWORD - exit status of the job starter thread
*     0:  no errors
*     >0: GetLastError() value
*
*  NOTES
*******************************************************************************/
DWORD WINAPI JobStarterThread(LPVOID lpParameter)
{
   C_Job    *pJob = NULL;
   DWORD    ret = 1;

   // lock access to job list
   WaitForSingleObject(g_JobList.m_hJobListMutex, INFINITE);


   // get job from job list that still is to be executed
   pJob = g_JobList.GetFirstJobInReceivedState();
   if(pJob != NULL) {
      pJob->m_JobStatus = js_ToBeStarted;
   } else {
      WriteToLogFile("Didn't find a job to be started in joblist!");
   }

   // unlock access to job list
   ReleaseMutex(g_JobList.m_hJobListMutex);

   if(pJob != NULL) {
      WriteToLogFile("Going to start job %d", pJob->m_job_id);

      // start job
      ret = StartJob(*pJob);

      WriteToLogFile("StartJob() returned %d", ret);

      // send exit code to sge_shepherd
      g_Communication.SendExitStatus(*pJob);
      g_Communication.ShutdownSocket(&(pJob->m_comm_sock));
   }
   return ret;
}

/****** StartJob() ************************************************************
*  NAME
*     StartJob() -- starts the job
*
*  SYNOPSIS
*     DWORD StartJob(C_Job &Job)
*
*  FUNCTION
*     Gives job user full access to the visible desktop, starts the job
*     waits for job end, reads usage data, withdraws access to visible
*     desktop from job user.
*
*  INPUTS
*     C_Job &Job - all informations about the job that is to be started
*
*  RESULT
*     DWORD - exit status of the job starter thread
*     0:  no errors
*     >0: GetLastError() value
*
*  NOTES
*******************************************************************************/
DWORD StartJob(C_Job &Job)
{
   STARTUPINFO         si;
   PROCESS_INFORMATION pi;
   DWORD               dwWait;
   HANDLE              hUserToken    = INVALID_HANDLE_VALUE;
   HANDLE              hSessionToken = INVALID_HANDLE_VALUE;
   HANDLE              hStdout       = INVALID_HANDLE_VALUE;
   HANDLE              hStderr       = INVALID_HANDLE_VALUE;
   HANDLE              hPipe         = INVALID_HANDLE_VALUE;
   char                *pszEnv = NULL;
   char                *pszCmdLine = NULL;
   char                szBuf[101];
   char                szError[4096];
   char                szErrorPart[4096];
   BOOL                bError      = TRUE;
   DWORD               dwError     = ERROR_SUCCESS;
   HWINSTA             hWinstaSave = NULL;
   HWINSTA             hWinsta     = NULL;
   HDESK               hDesk       = NULL;
   PSID                pSid        = NULL;
   BOOL                bResult     = FALSE;
   char                *pFileName  = NULL;
   const char          *pszCurDir  = NULL;
   DWORD               BytesRead   = 0;
   BOOL                bBackgndMode = FALSE;

   if(GetJobStartModeFromConf(Job.conf, Job.nconf) == FALSE) {
      Job.m_JobStatus = js_Failed;
      return 1;
   }

   // Put logging flag to environment, it will get copied into the job
   // environment below
   _snprintf(szBuf, 100, "%d", (int)g_bDoLogging);
   SetEnvironmentVariable("SGE_DO_LOGGING", szBuf);
   // Build environment as local Administrator
   Job.BuildEnvironment(pszEnv);
   Job.BuildCommandLine(pszCmdLine);
   pszCurDir = Job.GetConfValue("cwd");

   ZeroMemory(&si, sizeof(si));
   ZeroMemory(&pi, sizeof(pi));
   ZeroMemory(szErrorPart, sizeof(szErrorPart));

   WriteToLogFile("Logging on user %s+%s.",
      Job.domain != NULL ? Job.domain : "<null>",
      Job.user != NULL ? Job.user : "<null>");

   if (Job.pass == NULL) {
      WriteToLogFile("users password is NULL!");
   } else if (strlen(Job.pass) == 0) {
      WriteToLogFile("users password is empty!");
   }

   // Log on the job user to get the file handles of the
   // stdout and stderr file.
   if(!LogonUser(
         Job.user,
         Job.domain,
         Job.pass,
         LOGON32_LOGON_INTERACTIVE,
         LOGON32_PROVIDER_DEFAULT,
         &hUserToken)) {
      sprintf(szErrorPart, "LogonUser failed:");
      goto Cleanup;
   }

   bBackgndMode = GetModeFromEnv("SGE_BACKGND_MODE", Job.env, Job.nenv);
   if(bBackgndMode == FALSE) {
      // Save a handle to the caller's current window station.
      if((hWinstaSave = GetProcessWindowStation()) == NULL) {
         sprintf(szErrorPart, "GetProcessWindowStation failed:");
         goto Cleanup;
      }

      // Get a handle to the interactive window station.
      hWinsta = OpenWindowStation(
         "WinSta0",                   // the interactive window station
         FALSE,                       // handle is not inheritable
         READ_CONTROL | WRITE_DAC);   // rights to read/write the DACL

      if(hWinsta == NULL) {
         sprintf(szErrorPart, "OpenWindowStation failed:");
         goto Cleanup;
      }

      // To get the correct default desktop, set the caller's
      // window station to the interactive window station.
      if(!SetProcessWindowStation(hWinsta)) {
         sprintf(szErrorPart, "SetProcessWindowStation(hWinsta) failed:");
         goto Cleanup;
      }

      // Get a handle to the interactive desktop.
      hDesk = OpenDesktop(
         "Default",     // the interactive window station 
         0,             // no interaction with other desktop processes
         FALSE,         // handle is not inheritable
         READ_CONTROL | // request the rights to read and write the DACL
         WRITE_DAC | 
         DESKTOP_WRITEOBJECTS | 
         DESKTOP_READOBJECTS);

      // Restore the caller's window station.
      if(!SetProcessWindowStation(hWinstaSave)) {
         sprintf(szErrorPart, "SetProcessWindowStation(hWinstaSave) failed:");
         goto Cleanup;
      }

      if(hDesk == NULL) {
         sprintf(szErrorPart, "OpenDesktop failed:");
         goto Cleanup;
      }

      // Get the SID for the client's logon session.
      if(!GetLogonSID(hUserToken, pSid)) {
         sprintf(szErrorPart, "GetLogonSID failed:");
         goto Cleanup;
      }

      // Allow logon SID full access to interactive window station.
      if(!AddAceToWindowStation(hWinsta, pSid))  {
         sprintf(szErrorPart, "AddAceToWindowStation failed:");
         goto Cleanup;
      }

      // Allow logon SID full access to interactive desktop.
      if(!AddAceToDesktop(hDesk, pSid)) {
         sprintf(szErrorPart, "AddAceToDesktop failed:");
         goto Cleanup;
      }
   }

   // Impersonate job user to create the stdout/stderr files as the right user.
   if (!ImpersonateLoggedOnUser(hUserToken)) {
      sprintf(szErrorPart, "ImpersonateLoggedOnUser (JobUser) failed:");
      goto Cleanup;
   }

   // Redirect stdout and stderr
   if(RedirectStdHandles(Job, hStdout, hStderr)!=0) {
       sprintf(szErrorPart, "Redirecting File Handles failed:");
      goto Cleanup;
   }

   if (IsSystemWindowsVista() == TRUE) {
      RevertToSelf();

      hSessionToken = GetRemoteDesktopUserToken();
      if (hSessionToken == NULL) {
         // Get a token of the user of the currently logged on session to redirect
         // the GUI of the job to this window station and desktop.
         hSessionToken = GetInteractiveUserToken();
         if (hSessionToken == NULL) {
            sprintf(szErrorPart, "Getting Logged On User Token failed: ");
            goto Cleanup;
         }
      }
      // Impersonate client to ensure access to executable file.
      if(!ImpersonateLoggedOnUser(hSessionToken))  {
         sprintf(szErrorPart, "ImpersonateLoggedOnUser (DesktopUser) failed:");
         goto Cleanup;
      }
   }
   // Fill STARTUPINFO struct
   si.cb          = sizeof(STARTUPINFO); 
   si.dwFlags     |= STARTF_USESTDHANDLES;
   si.hStdOutput  = hStdout;
   si.hStdError   = hStderr;
   si.lpDesktop   = bBackgndMode==TRUE ? "" : "WinSta0\\Default";
   si.wShowWindow = IsSystemWindowsVista()==TRUE ? SW_HIDE : SW_SHOW;

   // To avoid a race condition with a signal here, lock Job list, check if
   // this job has been killed in the meanwhile. If job is not locked, start
   // it (job may not get killed in the meanwhile, because list is locked),
   // but unlock before the blocking wait. After the job has been started,
   // killing it will not lead to unexpected resultst.
   WaitForSingleObject(g_JobList.m_hJobListMutex, INFINITE);

   if(Job.m_JobStatus == js_Deleted) {
      ReleaseMutex(g_JobList.m_hJobListMutex);
      goto Cleanup;
   }

   Job.m_hJobObject = CreateJobObject(NULL, NULL);
   if (Job.m_hJobObject == NULL) {
      ReleaseMutex(g_JobList.m_hJobListMutex);
      goto Cleanup;
   }

   WriteToLogFile("Starting Job %s", pszCmdLine);
   if (IsSystemWindowsVista() == TRUE) {
      char *pszStarterCmdLine = NULL;
      char *pszPipeName = NULL;

      // We have to transfer the job users password to the SGE_Starter.exe,
      // a pipe seems to be the most secure way.
      if (CreatePipeForStarter(&pszPipeName, hPipe) != 0) {
         goto Cleanup;
      }

      if (BuildCommandLineForStarter(Job, pszPipeName, pszCmdLine, &pszStarterCmdLine) != 0) {
         goto Cleanup;
      }
      // Launch the process in the client's logon session.
      bResult = CreateProcessAsUser(hSessionToken,
         NULL,
         pszStarterCmdLine,
         NULL,
         NULL,
         TRUE,
         NORMAL_PRIORITY_CLASS|CREATE_PRESERVE_CODE_AUTHZ_LEVEL,
         pszEnv,
         pszCurDir,
         &si,
         &pi);

      free(pszStarterCmdLine); pszStarterCmdLine = NULL;
      free(pszPipeName); pszPipeName = NULL;
   } else {
      // Not on Vista or later
      bResult = CreateProcessAsUser(hUserToken,
         NULL,
         pszCmdLine,
         NULL,
         NULL,
         TRUE,
         NORMAL_PRIORITY_CLASS,//|CREATE_NO_WINDOW,//|CREATE_NEW_CONSOLE,
         pszEnv,
         pszCurDir,
         &si,
         &pi);
   }

   if(!bResult) {
      dwError = GetLastError();

      RevertToSelf();
      sprintf(szErrorPart, "CreateProcessAsUser failed, Command is \"%s\":", pszCmdLine);
      ReleaseMutex(g_JobList.m_hJobListMutex);

      SetLastError(dwError);
      goto Cleanup;
   }

   WriteToLogFile("Job started successfully.");
   AssignProcessToJobObject(Job.m_hJobObject, pi.hProcess);

   // End impersonation of client.
   RevertToSelf();

   Job.m_JobStatus = js_Started;
   Job.m_hProcess  = pi.hProcess;

   // unlock access to job list
   ReleaseMutex(g_JobList.m_hJobListMutex);

   // Wait blocking for job end
   if (bResult && pi.hProcess != INVALID_HANDLE_VALUE) {
      if (IsSystemWindowsVista() == TRUE) {
         // On Vista, send job user's password to SGE_Starter.exe, it needs it
         // to call CreateProcessWithLogon().
         if (WritePasswordToPipe(hPipe, pi.hProcess, Job.pass, szErrorPart) != 0) {
            // szErrorPart should already be filled by WritePasswordToPipe();
            goto Cleanup;
         }
         CloseHandle(hPipe);
         hPipe = INVALID_HANDLE_VALUE;
      }
      // Wait blocking for end of the main process of the job
      // (or for end of SGE_Starter.exe on Vista)
      WriteToLogFile("Waiting for job end in JobStarter.");
      dwWait = WaitForSingleObjectEx(pi.hProcess, INFINITE, FALSE);
      if(dwWait==WAIT_OBJECT_0) {
         // Make sure all child processes are killed
         if (Job.Terminate() != 0) {
            sprintf(szErrorPart, "Error terminating job");
            goto Cleanup;
         }
         // Read usage data from Windows job object and store it in our Job object
         if (Job.StoreUsage() != 0) {
            sprintf(szErrorPart, "Error storing job usage");
            goto Cleanup;
         }
      }
      if (IsSystemWindowsVista() == TRUE) {
         DWORD dwStarterExit;
         // Translate SGE_Starter.exe exit status
         dwStarterExit = (Job.dwExitCode & 0xffff0000) >> 16;
         Job.dwExitCode = Job.dwExitCode & 0x0000ffff;
         if (dwStarterExit != 0) {
            sprintf(szErrorPart, "SGE_Starter.exe failed, %d: %s",
               dwStarterExit, szStarterError[dwStarterExit]);
            goto Cleanup;
         }
      }
      Job.m_JobStatus = js_Finished;
      WriteToLogFile("Job ended.");
   }

   if(bBackgndMode == FALSE) {
      // Disallow logon SID full access to interactive desktop.
      if(!RemoveAceFromDesktop(hDesk, pSid)) {
         sprintf(szErrorPart, "RemoveAceFromDesktop failed:");
         goto Cleanup;
      }
      // Disallow logon SID full access to interactive window station.
      if(!RemoveAceFromWindowStation(hWinsta, pSid))  {
         sprintf(szErrorPart, "RemoveAceFromWindowStation failed:");
         goto Cleanup;
      }
   }
   if(bResult) {
      bError = FALSE;
   }

Cleanup: 
   if (bError == TRUE) {   
      char szLastError[501];

      dwError = GetLastError();

      if (dwError == 193) {
         // FormatMessage doesn't provide an error message for errno=193
         // The table of System Errors from the MSDN Libary tells us:
         // 193 Is not a valid application.  ERROR_BAD_EXE_FORMAT 
         strcpy(szLastError, "Is not a valid application.");
      } else {
         FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 
            NULL, dwError, 0, szLastError, 500, NULL);
      }

      if (strlen(szLastError) > 0) {
         if (szLastError[strlen(szLastError)-2] == '\r') {
             szLastError[strlen(szLastError)-2] = '\0';
         }
      } else {
         strcpy(szLastError, "(no error message available from system)");
      }

      sprintf(szError, "%s: %s (errno=%d)",
         szErrorPart,
         szLastError,
         dwError);
      WriteToLogFile(szError);

      Job.szError     = _strdup(szError);
      Job.m_JobStatus = js_Failed;
   }

   if (hStdout != INVALID_HANDLE_VALUE) {
      CloseHandle(hStdout);
   }
   if (hStderr != INVALID_HANDLE_VALUE) {
      CloseHandle(hStderr);
   }
   if (hPipe != INVALID_HANDLE_VALUE) {
      CloseHandle(hPipe);
   }
   if (hWinstaSave != NULL) {
      SetProcessWindowStation(hWinstaSave);
   }

   // Free the buffer for the logon SID.
   if (pSid) {
      FreeLogonSID(pSid);
   }

   // Close the handles to the interactive window station and desktop.
   if (hWinsta != NULL) {
      CloseWindowStation(hWinsta);
   }
   if (hDesk != NULL) {
      CloseDesktop(hDesk);
   }
   // Close the handle to the client's access token.
   if (hUserToken != INVALID_HANDLE_VALUE) {
      CloseHandle(hUserToken);  
   }
   // Close the handle to the session token.
   if (hSessionToken != INVALID_HANDLE_VALUE) {
      CloseHandle(hSessionToken);  
   }

   if (pi.hProcess != INVALID_HANDLE_VALUE) {
      CloseHandle(pi.hProcess); 
      pi.hProcess = INVALID_HANDLE_VALUE;
      Job.m_hProcess = INVALID_HANDLE_VALUE;
   }
   if (Job.m_hJobObject != NULL) {
      CloseHandle(Job.m_hJobObject);
      Job.m_hJobObject = NULL;
   }
   if(pi.hThread != INVALID_HANDLE_VALUE) {
      CloseHandle(pi.hThread); 
      pi.hThread = INVALID_HANDLE_VALUE;
   }

   free(pszEnv);
   free(pszCmdLine);
   return dwError;
}

/****** GetLogonSID() *********************************************************
*  NAME
*     GetLogonSID() -- retrieve SID of logged on user
*
*  SYNOPSIS
*     static BOOL GetLogonSID(HANDLE hToken, PSID &pSid)
*
*  FUNCTION
*     Retrieves the SID of the logged on user represented by the logon
*     token.
*
*  INPUTS
*     HANDLE hToken - token of the logged on user
*     
*  OUTPUTS
*     PSID &pSid - SID of the logged on user
*
*  RESULT
*     BOOL - true if SID could be retrieved, false if not
*
*  NOTES
*******************************************************************************/
static BOOL GetLogonSID(HANDLE hToken, PSID &pSid) 
{
   BOOL          bRet = FALSE;
   DWORD         dwIndex;
   DWORD         dwLength = 0;
   PTOKEN_GROUPS ptg      = NULL;

   // Get required buffer size and allocate the TOKEN_GROUPS buffer.
   if(!GetTokenInformation(hToken, TokenGroups,
                           (LPVOID)ptg, 0, &dwLength)) {
      if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
         goto Cleanup;
      }
      ptg = (PTOKEN_GROUPS)HeapAlloc(GetProcessHeap(), 
                                     HEAP_ZERO_MEMORY, dwLength);
      if(ptg == NULL) {
         goto Cleanup;
      }
   }

   // Get the token group information from the access token.
   if(!GetTokenInformation(hToken, TokenGroups,
                           (LPVOID)ptg, dwLength, &dwLength)) {
      goto Cleanup;
   }

   // Loop through the groups to find the logon SID.
   for(dwIndex = 0; dwIndex < ptg->GroupCount; dwIndex++) {
      if((ptg->Groups[dwIndex].Attributes&SE_GROUP_LOGON_ID) == SE_GROUP_LOGON_ID) {
         // Found the logon SID; make a copy of it.
         dwLength = GetLengthSid(ptg->Groups[dwIndex].Sid);
         pSid = (PSID)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength);

         if(pSid == NULL) {
             goto Cleanup;
         }
         if(!CopySid(dwLength, pSid, ptg->Groups[dwIndex].Sid)) {
             HeapFree(GetProcessHeap(), 0, (LPVOID)pSid);
             goto Cleanup;
         }
         break;
      }
   }
   bRet = TRUE;

Cleanup: 
   // Free the buffer for the token groups.
   if(ptg != NULL) {
      HeapFree(GetProcessHeap(), 0, (LPVOID)ptg);
   }
   return bRet;
}

/****** FreeLogonSID() ****************************************************++++
*  NAME
*     FreeLogonSID() -- frees the buffer of the SID 
*
*  SYNOPSIS
*     static void FreeLogonSID(PSID &pSid)
*
*  FUNCTION
*     Frees the buffer of the SID allocated by GetLogonSID()
*
*  INPUTS
*     PSID &pSid - the SID to be freed.
*     
*  RESULT
*     void - no result
*
*  NOTES
*******************************************************************************/
static void FreeLogonSID(PSID &pSid)
{
    HeapFree(GetProcessHeap(), 0, (LPVOID)pSid);
}

/****** AddAceToWindowStation() ************************************************
*  NAME
*     AddAceToWindowStation() -- adds the ACE of the job user to the ACL of the 
*                                visible window station.
*
*  SYNOPSIS
*     static BOOL AddAceToWindowStation(HWINSTA hWinsta, PSID pSid)
*
*  FUNCTION
*    Adds the ACE (Access Control Entry) of the job user to the ACL
*    (Access Control List) of the visible window station.
*
*  INPUTS
*     HWINSTA hWinsta - Handle of the visible window station
*     PSID    pSid    - SID (Security Identifier) of the job user
*     
*  RESULT
*     BOOL - true if adding succeeded, false if it failed
*
*  NOTES
*******************************************************************************/
static BOOL AddAceToWindowStation(HWINSTA hWinsta, PSID pSid)
{
   ACCESS_ALLOWED_ACE   *pAce;
   ACL_SIZE_INFORMATION aclSizeInfo;
   BOOL                 bDaclExist;
   BOOL                 bDaclPresent;
   BOOL                 bRet = FALSE;
   DWORD                dwNewAclSize;
   DWORD                dwSidSize = 0;
   DWORD                dwSdSizeNeeded;
   PACL                 pAcl;
   PACL                 pNewAcl;
   PSECURITY_DESCRIPTOR pSd = NULL;
   PSECURITY_DESCRIPTOR pSdNew = NULL;
   PVOID                pTempAce;
   SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
   unsigned int         i;

   if(WaitForSingleObject(g_hWinstaACLMutex, INFINITE) == WAIT_OBJECT_0) {
      __try
      {
         // Obtain the DACL for the window station.
         if(!GetUserObjectSecurity(hWinsta, &si, pSd, dwSidSize, &dwSdSizeNeeded)) {
            if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
               pSd = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(),
                                       HEAP_ZERO_MEMORY, dwSdSizeNeeded);
            }

            if (pSd == NULL) {
               __leave;
            }

            pSdNew = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(),
                                       HEAP_ZERO_MEMORY, dwSdSizeNeeded);

            if(pSdNew == NULL) {
               __leave;
            }

            dwSidSize = dwSdSizeNeeded;
            if(!GetUserObjectSecurity(hWinsta, &si, pSd, dwSidSize, &dwSdSizeNeeded)) {
               __leave;
            }
         } else {
            __leave;
         }

         // Create a new DACL.
         if(!InitializeSecurityDescriptor(pSdNew, SECURITY_DESCRIPTOR_REVISION)) {
            __leave;
         }

         // Get the DACL from the security descriptor.
         if(!GetSecurityDescriptorDacl(pSd, &bDaclPresent, &pAcl, &bDaclExist)) {
            __leave;
         }

         // Initialize the ACL.
         ZeroMemory(&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
         aclSizeInfo.AclBytesInUse = sizeof(ACL);

         // Call only if the DACL is not NULL.
         if (pAcl != NULL) {
            // get the file ACL size info
            if(!GetAclInformation(pAcl, (LPVOID)&aclSizeInfo,
                  sizeof(ACL_SIZE_INFORMATION), AclSizeInformation)) {
               __leave;
            }
         }

         if (bDaclPresent == TRUE) {
            // Check if object already has this ACL - if yes, don't set it again!
            if (aclSizeInfo.AceCount != 0) {
               for (i=0; i<aclSizeInfo.AceCount; i++) {
                  // Get an ACE
                  if (GetAce(pAcl, i, &pTempAce) != TRUE) {
                     __leave;
                  }
                  if (EqualSid((PSID)&((ACCESS_ALLOWED_ACE*)pTempAce)->SidStart, pSid) == TRUE) {
                     bRet = TRUE;
                     __leave; // this SID already exists
                  }
               }
            }
         }
         

         // Compute the size of the new ACL.
         dwNewAclSize = aclSizeInfo.AclBytesInUse 
                        + (2*sizeof(ACCESS_ALLOWED_ACE)) 
                        + (2*GetLengthSid(pSid)) - (2*sizeof(DWORD));

         // Allocate memory for the new ACL.
         pNewAcl = (PACL)HeapAlloc(GetProcessHeap(),  
                           HEAP_ZERO_MEMORY, dwNewAclSize);

         if(pNewAcl == NULL) {
            __leave;
         }

         // Initialize the new DACL.
         if(!InitializeAcl(pNewAcl, dwNewAclSize, ACL_REVISION)) {
            __leave;
         }

         // If DACL is present, copy it to a new DACL.
         if(bDaclPresent) {
            // Copy the ACEs to the new ACL.
            if(aclSizeInfo.AceCount) {
               for(i=0; i < aclSizeInfo.AceCount; i++) {
                  if(!GetAce(pAcl, i, &pTempAce)) {
                     __leave;
                  }

                  // Add the ACE to the new ACL.
                  if(!AddAce(pNewAcl, ACL_REVISION, MAXDWORD,
                        pTempAce, ((PACE_HEADER)pTempAce)->AceSize)) {
                     __leave;
                  }
               }
            }
         }

         // Add the first ACE to the window station.
         pAce = (ACCESS_ALLOWED_ACE *)HeapAlloc(
               GetProcessHeap(),
               HEAP_ZERO_MEMORY,
               sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pSid) -
                     sizeof(DWORD));

         if (pAce == NULL)
            __leave;

         pAce->Header.AceType  = ACCESS_ALLOWED_ACE_TYPE;
         pAce->Header.AceFlags = CONTAINER_INHERIT_ACE |
                                    INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;
         pAce->Header.AceSize  = (WORD)(sizeof(ACCESS_ALLOWED_ACE) +
                                    GetLengthSid(pSid) - sizeof(DWORD));
         pAce->Mask            = GENERIC_ACCESS;

         if (!CopySid(GetLengthSid(pSid), &pAce->SidStart, pSid))
            __leave;

         if (!AddAce(
               pNewAcl,
               ACL_REVISION,
               MAXDWORD,
               (LPVOID)pAce,
               pAce->Header.AceSize)
         )
            __leave;

         // Add the second ACE to the window station.
         pAce->Header.AceFlags = NO_PROPAGATE_INHERIT_ACE;
         pAce->Mask            = WINSTA_ALL;

         if (!AddAce(
               pNewAcl,
               ACL_REVISION,
               MAXDWORD,
               (LPVOID)pAce,
               pAce->Header.AceSize)
         )
            __leave;

         // Set a new DACL for the security descriptor.
         if (!SetSecurityDescriptorDacl(
               pSdNew,
               TRUE,
               pNewAcl,
               FALSE)
         )
            __leave;

         // Set the new security descriptor for the window station.
         if(!SetUserObjectSecurity(hWinsta, &si, pSdNew)) {
            DWORD dwError;
            char  szLastError[501];

            dwError = GetLastError();
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0, szLastError, 500, NULL);
            __leave;
         }
         // Indicate success.
         bRet = TRUE;
      }
      __finally
      {
         // Free the allocated buffers.

         if (pAce != NULL)
            HeapFree(GetProcessHeap(), 0, (LPVOID)pAce);

         if (pNewAcl != NULL)
            HeapFree(GetProcessHeap(), 0, (LPVOID)pNewAcl);

         if (pSd != NULL)
            HeapFree(GetProcessHeap(), 0, (LPVOID)pSd);

         if (pSdNew != NULL)
            HeapFree(GetProcessHeap(), 0, (LPVOID)pSdNew);
      }
      ReleaseMutex(g_hWinstaACLMutex);
   }
   return bRet;
}

/****** AddAceToDesktop() ******************************************************
*  NAME
*     AddAceToDesktop() -- adds the ACE of the job user to the ACL of the 
*                          visible desktop.
*
*  SYNOPSIS
*     static BOOL AddAceToDesktop(HDESK hDesk, PSID pSid)
*
*  FUNCTION
*    Adds the ACE (Access Control Entry) of the job user to the ACL
*    (Access Control List) of the visible desktop.
*
*  INPUTS
*     HDESK hDesk - Handle of the visible desktop
*     PSID  pSid  - SID (Security Identifier) of the job user
*     
*  RESULT
*     BOOL - true if adding succeeded, false if it failed
*
*  NOTES
*******************************************************************************/
static BOOL AddAceToDesktop(HDESK hDesk, PSID pSid)
{
   ACL_SIZE_INFORMATION aclSizeInfo;
   BOOL                 bDaclExist;
   BOOL                 bDaclPresent;
   BOOL                 bRet      = FALSE;
   DWORD                dwSidSize = 0;
   DWORD                dwNewAclSize;
   DWORD                dwSdSizeNeeded;
   PVOID                pTempAce;
   PACL                 pAcl;
   PACL                 pNewAcl;
   PSECURITY_DESCRIPTOR pSd    = NULL;
   PSECURITY_DESCRIPTOR pSdNew = NULL;
   SECURITY_INFORMATION si     = DACL_SECURITY_INFORMATION;
   unsigned int         i;

   if(WaitForSingleObject(g_hDeskACLMutex, INFINITE) == WAIT_OBJECT_0) {
      __try
      {
         // Obtain the security descriptor for the desktop object.
         if(!GetUserObjectSecurity(hDesk, &si, pSd, dwSidSize, &dwSdSizeNeeded)) {
            if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
               pSd = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(),
                                       HEAP_ZERO_MEMORY, dwSdSizeNeeded);
               if(pSd == NULL) {
                  __leave;
               }

               pSdNew = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(),
                                          HEAP_ZERO_MEMORY, dwSdSizeNeeded);
               if(pSdNew == NULL) {
                  __leave;
               }

               dwSidSize = dwSdSizeNeeded;
               if(!GetUserObjectSecurity(hDesk, &si, pSd,
                                       dwSidSize, &dwSdSizeNeeded)) {
                  __leave;
               }
            } else {
               __leave;
            }
         }

         // Create a new security descriptor.
         if(!InitializeSecurityDescriptor(pSdNew, SECURITY_DESCRIPTOR_REVISION)) {
            __leave;
         }

         // Obtain the DACL from the security descriptor.
         if (!GetSecurityDescriptorDacl(pSd, &bDaclPresent, &pAcl, &bDaclExist)) {
            __leave;
         }

         // Initialize.
         ZeroMemory(&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
         aclSizeInfo.AclBytesInUse = sizeof(ACL);

         if(pAcl != NULL) {
            // Determine the size of the ACL information.
            if (!GetAclInformation(pAcl, (LPVOID)&aclSizeInfo,
                     sizeof(ACL_SIZE_INFORMATION), AclSizeInformation)) {
               __leave;
            }
         }

         if (bDaclPresent) {
            // Checks if object already has this ACL - if yes, don't add it again!
            if (aclSizeInfo.AceCount) {
               for (i=0; i<aclSizeInfo.AceCount; i++) {
                  // Get an ACE.
                  if (!GetAce(pAcl, i, &pTempAce)) {
                     __leave;
                  }

                  if (EqualSid((PSID)&((ACCESS_ALLOWED_ACE*)pTempAce)->SidStart, pSid)) {
                     bRet = TRUE;
                     __leave; // this SID already exists
                  }
               }
            }
         }

         // Compute the size of the new ACL and allocate buffer
         dwNewAclSize = aclSizeInfo.AclBytesInUse
                        + sizeof(ACCESS_ALLOWED_ACE)
                        + GetLengthSid(pSid) - sizeof(DWORD);

         pNewAcl = (PACL)HeapAlloc(GetProcessHeap(), 
                        HEAP_ZERO_MEMORY, dwNewAclSize);

         if(pNewAcl == NULL) {
            __leave;
         }

         if(!InitializeAcl(pNewAcl, dwNewAclSize, ACL_REVISION)) {
            __leave;
         }

         // If DACL is present, copy it to a new DACL.
         if(bDaclPresent) {
            // Copy the ACEs to the new ACL.
            if(aclSizeInfo.AceCount) {
               for(i=0; i < aclSizeInfo.AceCount; i++) {
                  // Get an ACE.
                  if(!GetAce(pAcl, i, &pTempAce)) {
                     __leave;
                  }

                  // Add the ACE to the new ACL.
                  if(!AddAce(pNewAcl, ACL_REVISION, MAXDWORD, pTempAce,
                                          ((PACE_HEADER)pTempAce)->AceSize)) {
                     __leave;
                  }
               }
            }
         }

         // Add ACE to the DACL, set new DACL to the new security descriptor,
         // set new security descriptor for the desktop object.
         if(!AddAccessAllowedAce(pNewAcl, ACL_REVISION, DESKTOP_ALL, pSid)) {
            __leave;
         }
         if(!SetSecurityDescriptorDacl(pSdNew, TRUE, pNewAcl, FALSE)) {
            __leave;
         }
         if(!SetUserObjectSecurity(hDesk, &si, pSdNew)) {
            __leave;
         }
         bRet = TRUE;
      }
      __finally
      {
         // Free buffers.
         if(pNewAcl != NULL) {
            HeapFree(GetProcessHeap(), 0, (LPVOID)pNewAcl);
         }

         if(pSd != NULL) {
            HeapFree(GetProcessHeap(), 0, (LPVOID)pSd);
         }

         if(pSdNew != NULL) {
            HeapFree(GetProcessHeap(), 0, (LPVOID)pSdNew);
         }
      }
      ReleaseMutex(g_hDeskACLMutex);
   }
   return bRet;
}

/****** RemoveAceFromWindowStation() ******************************************
*  NAME
*     RemoveAceFromWindowStation() -- removes the ACE of the job user from the
*                                     ACL of the visible window station.
*
*  SYNOPSIS
*     static BOOL RemoveAceFromWindowStation(HWINSTA hWinsta, PSID pSid)
*
*  FUNCTION
*    Removes the ACE (Access Control Entry) of the job user from the ACL
*    (Access Control List) of the visible window station.
*
*  INPUTS
*     HWINSTA hWinsta - Handle of the visible window station
*     PSID    pSid    - SID (Security Identifier) of the job user
*     
*  RESULT
*     BOOL - true if removing succeeded, false if it failed
*
*  NOTES
*******************************************************************************/
static BOOL RemoveAceFromWindowStation(HWINSTA hWinsta, PSID pSid)
{
   SECURITY_DESCRIPTOR  *pSD = NULL;
   BOOL                 bSecRet;
   BOOL                 bDaclPresent = TRUE;
   BOOL                 bDaclDefaulted = FALSE;
   DWORD                SDLength = 0;
   DWORD                SDLengthNeeded = 0;
   PACL			    		pWinstaDacl;    
   LPVOID               pWinstaAce;
   SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
   BOOL                 bRet = TRUE;
   BOOL                 bEqual;
   PSID                 pListSid;
   int                  nDeleted = 0;

   if(WaitForSingleObject(g_hWinstaACLMutex, INFINITE) == WAIT_OBJECT_0) {
      __try
      {
         // Obtain DACL from Windows station, search for ACE, remove ACE from DACL
         bSecRet = GetUserObjectSecurity(hWinsta, &si, pSD, SDLength, &SDLengthNeeded);
         if(!bSecRet) {
            pSD = (SECURITY_DESCRIPTOR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SDLengthNeeded);
         }
         bSecRet = GetUserObjectSecurity(hWinsta, &si, pSD, SDLengthNeeded, &SDLengthNeeded);
         bSecRet = GetSecurityDescriptorDacl(pSD, &bDaclPresent, &pWinstaDacl, &bDaclDefaulted);

         for(int i=pWinstaDacl->AceCount-1; i>=0; i--) {
            bSecRet = GetAce(pWinstaDacl, i, &pWinstaAce);
            if(((ACCESS_ALLOWED_ACE*)pWinstaAce)->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) {
               pListSid = (PSID)&(((ACCESS_ALLOWED_ACE*)pWinstaAce)->SidStart);
               bEqual = TRUE;
               bSecRet = IsValidSid(pSid);
               bSecRet = IsValidSid(pListSid);
               DWORD dwSidLength = GetLengthSid(pSid);
               DWORD dwListSidLength = GetLengthSid(pListSid);

               for(DWORD j=0; j<dwSidLength && j<dwListSidLength; j++) {
                  if(*((BYTE*)pListSid+j) != *((BYTE*)pSid+j)) {
                     bEqual = FALSE;
                     break;
                  }
               }
               if(bEqual) {
                  DeleteAce(pWinstaDacl, i);
                  nDeleted++;
                  if(nDeleted == 2) {
                     break;
                  }
               }
            }
         }
         SetUserObjectSecurity(hWinsta, &si, pSD);
      }

      __finally
      {
         if(pSD != NULL) {
            HeapFree(GetProcessHeap(), 0, (LPVOID)pSD);
         }
      }

      ReleaseMutex(g_hWinstaACLMutex);
   }         
   return bRet;
}

/****** RemoveAceFromDesktop() ************************************************
*  NAME
*     RemoveAceFromDesktop() -- removes the ACE of the job user from the
*                               ACL of the visible desktop
*
*  SYNOPSIS
*     static BOOL RemoveAceFromDesktop(HDESK hDesk, PSID pSid)
*
*  FUNCTION
*    Removes the ACE (Access Control Entry) of the job user from the ACL
*    (Access Control List) of the visible desktop
*
*  INPUTS
*     HDESK hDesk - Handle of the visible desktop
*     PSID  pSid  - SID (Security Identifier) of the job user
*     
*  RESULT
*     BOOL - true if removing succeeded, false if it failed
*
*  NOTES
*******************************************************************************/
static BOOL RemoveAceFromDesktop(HDESK hDesk, PSID pSid)
{
   SECURITY_DESCRIPTOR  *pSD = NULL;
   BOOL                 bSecRet;
   BOOL                 bDaclPresent = TRUE;
   BOOL                 bDaclDefaulted = FALSE;
   DWORD                SDLength = 0;
   DWORD                SDLengthNeeded = 0;
   PACL			    		pDeskDacl;    
   LPVOID               pDeskAce;
   SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
   BOOL                 bRet = TRUE;
   BOOL                 bEqual;
   PSID                 pListSid;

   if(WaitForSingleObject(g_hDeskACLMutex, INFINITE) == WAIT_OBJECT_0) {
      __try
      {
         // Obtain DACL from Windows station, search for ACE, remove ACE from DACL
         bSecRet = GetUserObjectSecurity(hDesk, &si, pSD, SDLength, &SDLengthNeeded);
         if(!bSecRet) {
            pSD = (SECURITY_DESCRIPTOR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SDLengthNeeded);
         }
         bSecRet = GetUserObjectSecurity(hDesk, &si, pSD, SDLengthNeeded, &SDLengthNeeded);
         bSecRet = GetSecurityDescriptorDacl(pSD, &bDaclPresent, &pDeskDacl, &bDaclDefaulted);

         for(DWORD i=0; i<pDeskDacl->AceCount; i++) {
            bSecRet = GetAce(pDeskDacl, i, &pDeskAce);
            if(((ACCESS_ALLOWED_ACE*)pDeskAce)->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) {
               pListSid = (PSID)&(((ACCESS_ALLOWED_ACE*)pDeskAce)->SidStart);
               bEqual = TRUE;
               bSecRet = IsValidSid(pSid);
               bSecRet = IsValidSid(pListSid);
               DWORD dwSidLength = GetLengthSid(pSid);
               DWORD dwListSidLength = GetLengthSid(pListSid);

               for(DWORD j=0; j<dwSidLength && j<dwListSidLength; j++) {
                  if(*((BYTE*)pListSid+j) != *((BYTE*)pSid+j)) {
                     bEqual = FALSE;
                     break;
                  }
               }
               if(bEqual) {
                  DeleteAce(pDeskDacl, i);
                  break;
               }
            }
         }
         SetUserObjectSecurity(hDesk, &si, pSD);
      }
      __finally
      {
         if(pSD != NULL) {
            HeapFree(GetProcessHeap(), 0, (LPVOID)pSD);
         }
      }
      ReleaseMutex(g_hDeskACLMutex);
   }
   return bRet;
}

static void GetShellProcess(TCHAR shell[_MAX_PATH])
{
   HKEY  hKey   = NULL;
   DWORD dwType = REG_SZ;
   DWORD dwSize = _MAX_PATH * sizeof(TCHAR);
   LONG  lRet   = 0;

   // initialize to "explorer.exe"
   _tcscpy(shell, _T("explorer.exe"));

   // gets the default shell process
   RegOpenKeyEx(HKEY_LOCAL_MACHINE,
      _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
      0, KEY_READ, &hKey);

   if (hKey != NULL) {
      lRet = RegQueryValueEx(hKey, _T("Shell"), NULL, &dwType, (LPBYTE)shell, &dwSize);
      if (lRet == ERROR_SUCCESS) {
         _tcslwr(shell);
      } 
      RegCloseKey(hKey);
   }
}

typedef struct sEnumData {
   DWORD dwSessionID;
   DWORD dwPID;
   BOOL (WINAPI* ProcessIdToSessionId)(DWORD, DWORD*);
   TCHAR szShell[_MAX_PATH];
} tEnumData;

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
   char       *szBasename;
   char       szFilename[_MAX_PATH];
   DWORD      dwSessionID;
   DWORD      dwPID = 0;
   DWORD      dwSize = 0;
   tEnumData  *pEnumData = (tEnumData*)lParam;
   HANDLE     hProcess;
   HMODULE    hModule;

   GetWindowThreadProcessId(hWnd, &dwPID);
   hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, dwPID);
   if (hProcess) {
      if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &dwSize)) {
         GetModuleFileNameEx(hProcess, hModule, szFilename, sizeof(szFilename));
         // get basename of module path
         {
            char *cp;
            
            cp = strrchr(szFilename, '\\');
            if (cp != NULL) {
               cp++;
               *cp = '\0';
               szBasename = cp;
            } else {
               szBasename = szFilename;
            }
         }
         
         if (_stricmp(szBasename, pEnumData->szShell) == 0) {
            // Found the explorer.exe
            if (pEnumData->ProcessIdToSessionId != NULL) {
               pEnumData->ProcessIdToSessionId(dwPID, &dwSessionID);
               if (dwSessionID == pEnumData->dwSessionID) {
                  pEnumData->dwPID = dwPID;
                  return FALSE;
               }
            }
            return FALSE;
         }
      }
      CloseHandle(hProcess);
   } else {
      WriteToLogFile("Cant open Process wit PID %ld", dwPID);
      return FALSE;
   }
   return TRUE;
}


static DWORD GetShellProcessPidForSession(DWORD dwSessionID)
{
   tEnumData myEnumData;

   // init struct that is to be filled by callback function EnumWindowsProc
   myEnumData.ProcessIdToSessionId = (BOOL(WINAPI*)(DWORD, DWORD*))
      GetProcAddress(GetModuleHandle(_T("KERNEL32.DLL")), "ProcessIdToSessionId");
   myEnumData.dwSessionID = dwSessionID;
   myEnumData.dwPID = 0;
   GetShellProcess(myEnumData.szShell);

   if (EnumWindows(EnumWindowsProc, (LPARAM)&myEnumData) != TRUE) {
      return myEnumData.dwPID;
   }
   return 0;
}

static DWORD GetRemoteDesktopSessionID()
{
   BOOL bRet = FALSE;
   DWORD i = 0;
   DWORD dwEntries = 0;
   WTS_SESSION_INFO *pWTSSession = NULL;

   bRet = WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pWTSSession, &dwEntries);
   if (bRet == FALSE) {
      return (DWORD)-1;
   }

   // Loop over all sessions to find a Remote Desktop
   for (i=0; i<dwEntries; i++) {
      DWORD dwOutBytes = 0;
      LPTSTR szOutput = NULL;
      USHORT SessionType = 0;

      bRet = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                                        pWTSSession[i].SessionId,
                                        WTSClientProtocolType,
                                        &szOutput,
                                        &dwOutBytes);

      if (szOutput != NULL) {
         if (bRet == TRUE) {
            SessionType = *((USHORT*)szOutput);
            if (SessionType == 2) {
               WTSFreeMemory((PVOID)szOutput);
               return pWTSSession[i].SessionId;
            }
         }
         WTSFreeMemory((PVOID)szOutput);
      }
   }
   return (DWORD)-1;
}

static HANDLE WINAPI GetRemoteDesktopUserToken()
{
   BOOL   bRet = FALSE;
   DWORD  dwSID = (DWORD)-1;
   HANDLE hUserToken = NULL;
   HANDLE hDuplicatedToken = NULL;

   dwSID = GetRemoteDesktopSessionID();
   if (dwSID == -1) {
      return NULL;
   }

   bRet = WTSQueryUserToken(dwSID, &hUserToken);
   if (bRet == FALSE) {
      return NULL;
   }

   DuplicateTokenEx(hUserToken, TOKEN_ALL_ACCESS, NULL,
                    SecurityImpersonation, TokenPrimary,
                    &hDuplicatedToken);
   CloseHandle(hUserToken);
   hUserToken = NULL;

   return hDuplicatedToken;
}

static HANDLE WINAPI GetInteractiveUserToken()
{
   DWORD  dwSessionID;
   DWORD  dwShellInteractivePID;
   HANDLE hProcess      = NULL;
   HANDLE hToken        = NULL;
   HANDLE hPrimaryToken = NULL;

   // WTSEnumerateSessions() might be a solution here
   // TODO: return after first error. This means here return if
   //       dwSessionID is invalid !!!
   dwSessionID = WTSGetActiveConsoleSessionId();

   // TODO: WTSQueryUserToken() will fail when no user is logged into the
   //       system for vista hosts. This means starting a GUI job without
   //       any user logged into the system will fail.

   // try to get the token of the user of the interactive console session
   // from the Windows Terminal Services
   if (WTSQueryUserToken(dwSessionID, &hToken) == FALSE) {
      // it didn't work, so try to get the process id of the shell of
      // the interactively logged on user
      dwShellInteractivePID = GetShellProcessPidForSession(dwSessionID);
      if (dwShellInteractivePID == 0) {
         // no chance to get the right user token
         return NULL;
      }

      // open the shell process and get the user token
      hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwShellInteractivePID);
      if (hProcess == NULL ||
          OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken) == FALSE) {
         return NULL;
      }
      CloseHandle(hProcess);
   }
   // here we have the impersonation token of the user, create a primary
   // user token out of it
   if (DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation,
       TokenPrimary, &hPrimaryToken) == FALSE) {
      CloseHandle(hToken);
      return NULL;
   }
   CloseHandle(hToken);

   return hPrimaryToken;
}

/****** GetJobStartModeFromConf() *********************************************
*  NAME
*     GetJobStartModeFromConf() -- searches conf for display_win_gui and
*                                   retrieves the corresponding value.
*
*  SYNOPSIS
*     static BOOL GetJobStartModeFromConf(char **conf, int nconf)
*
*  FUNCTION
*    Searchs conf for the complex variable display_win_gui and retrieves
*    it's value.
*
*  INPUTS
*     char **conf - the configuration that is to be searched
*     int  nconf  - number of entries in configuration
*     
*  RESULT
*     BOOL - TRUE if the value of the complex variable display_win_gui is "1",
*            else FALSE.
*
*  NOTES
*******************************************************************************/
static BOOL GetJobStartModeFromConf(char **conf, int nconf)
{
   int             i;
   char            *ptr;
   char            *tmp;
   BOOL            bRet = FALSE;
   BOOL            bFound = FALSE;

   for(i=0; i<nconf && bFound==FALSE; i++) {
      tmp = _strdup(conf[i]);
      ptr = strtok(tmp, "=");
      if(ptr && _stricmp(ptr, "display_win_gui")==0) {
         ptr=strtok(NULL, "=");
         if(ptr && _stricmp(ptr, "1")==0) {
            bRet = TRUE;
            bFound = TRUE;
         } 
      }
      free(tmp);
   }
   return bRet;
}

/****** GetModeFromEnv() ******************************************************
*  NAME
*     GetModeFromEnv() -- searches env for given variable and retrieves
*                         the corresponding value.
*
*  SYNOPSIS
*     static BOOL GetModeFromEnv(const char *mode, char **env, int nenv)
*
*  FUNCTION
*    Searchs env for the given variable and retrieves it's value.
*    
*
*  INPUTS
*     char **env - the environment that is to be searched
*     int  nenv  - number of entries in environment
*     
*  RESULT
*     BOOL - TRUE if the value of the environment variable is "1"
*            or "TRUE" (not case sensitive), else FALSE.
*
*  NOTES
*******************************************************************************/
static BOOL GetModeFromEnv(const char *mode, char **env, int nenv)
{
   int             i;
   char            *ptr;
   char            *tmp;
   BOOL            bRet = FALSE;
   BOOL            bFound = FALSE;

   for(i=0; i<nenv && bFound==FALSE; i++) {
      tmp = _strdup(env[i]);
      ptr = strtok(tmp, "=");
      if(ptr && _stricmp(ptr, mode)==0) {
         ptr=strtok(NULL, "=");
         if(ptr && (_stricmp(ptr, "TRUE")==0 || _stricmp(ptr, "1")==0)) {
            bRet = TRUE;
            bFound = TRUE;
         } 
      }
      free(tmp);
   }
   return bRet;
}

/****** RedirectStdHandles() **************************************************
*  NAME
*     RedirectStdHandles() -- Redirects stdout and stderr
*
*  SYNOPSIS
*    static DWORD RedirectStdHandles(const C_Job &Job, 
*                                    HANDLE &hStdout, HANDLE &hStderr)
*
*  FUNCTION
*    Redirects stdout and stderr of the job to files. The file names are
*    retrieved from the job's environment.
*
*  INPUTS
*     C_Job &Job - The job object of the job for whom the standard handles
*                  are to be redirected.
*
*  OUTPUTS
*     HANDLE &hStdout - The redirected stdout handle
*     HANDLE &hStderr - The redirected stderr handle
*     
*  RESULT
*     DWORD - 0: OK
*             1: Can't create stdout file
*             2: Can't create stderr file
*
*  NOTES
*******************************************************************************/
static DWORD RedirectStdHandles(const C_Job &Job, HANDLE &hStdout, HANDLE &hStderr)
{
   const char *pszStdout, *pszStderr, *pszMerge;
   int        iMerge = 0;
   int        ret = 0;
   SECURITY_ATTRIBUTES secAttr;

   ZeroMemory(&secAttr, sizeof(secAttr));
   secAttr.nLength        = sizeof(secAttr);
   secAttr.bInheritHandle = TRUE;

   try {
      pszStdout = Job.GetConfValue("stdout_path");
      pszStderr = Job.GetConfValue("stderr_path");
      pszMerge  = Job.GetConfValue("merge_stderr");
      if(pszMerge != NULL) {
         sscanf(pszMerge, "%d", &iMerge);
      }

      hStdout = CreateFile(
                  pszStdout,
                  GENERIC_WRITE,
                  FILE_SHARE_READ|FILE_SHARE_WRITE,
                  &secAttr,
                  OPEN_ALWAYS,
                  FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,
                  NULL);
      if(hStdout == INVALID_HANDLE_VALUE) {
         throw 1;
      }
      SetFilePointer(hStdout, 1, NULL, FILE_END);
      SetHandleInformation(hStdout, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

      if(iMerge == 0) {
         hStderr = CreateFile(
                     pszStderr,
                     GENERIC_WRITE,
                     FILE_SHARE_READ|FILE_SHARE_WRITE,
                     &secAttr,
                     OPEN_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,
                     NULL);
      } else {
         DuplicateHandle(GetCurrentProcess(), hStdout, 
                        GetCurrentProcess(), &hStderr,
                        0, TRUE, DUPLICATE_SAME_ACCESS);
      }
      if(hStderr == INVALID_HANDLE_VALUE) {
         throw 2;
      }
      SetFilePointer(hStderr, 1, NULL, FILE_END);
      SetHandleInformation(hStderr, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
   }
   catch(int ret_val) {
      ret = ret_val;
   }
   return ret;
}

/****** IsSystemWindowsVista() ************************************************
*  NAME
*     IsSystemWindowsVista() -- checks if current OS is Vista or later
*
*  SYNOPSIS
*     static BOOL IsSystemWindowsVista()
*
*  FUNCTION
*     Checks if the OS is Windows Vista or later.
*
*  RESULT
*     BOOL - TRUE if the current OS is Windows Vista or later,
*            FALSE if it is a earlier Windows version.
*
*  NOTES
*******************************************************************************/
static BOOL IsSystemWindowsVista()
{
   OSVERSIONINFOEX osvi;
   DWORDLONG       dwlCondMask = 0;
 
   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   osvi.dwMajorVersion = 6;

   VER_SET_CONDITION(dwlCondMask, VER_MAJORVERSION, VER_GREATER_EQUAL);

   return VerifyVersionInfo(&osvi, VER_MAJORVERSION, dwlCondMask);
}

/****** GetDirName() **********************************************************
*  NAME
*     GetDirName() -- gets the directory part of an absolute or relative path
*
*  SYNOPSIS
*     static void GetDirName(char *szDirName, char delim)
*
*  FUNCTION
*     Gets the directory part of a an absolute or relative path
*
*  INPUTS
*     char *szDirName - the absolute or relative path
*     char delim      - delimiter, must be '\\' on Windows
*
*  RESULT
*     VOID - none
*
*  NOTES
*******************************************************************************/
static void GetDirName(char *szDirName, char delim)
{
   char *cp;

   cp = strrchr(szDirName, delim);
   if (cp != NULL) {
      szDirName[cp - szDirName] = '\0';
   }
}


/****** GetModuleDir() ********************************************************
*  NAME
*     GetModuleDir() -- Retrieves the directory where this program module
*                       (i.e. EXE file) is located
*
*  SYNOPSIS
*     static int GetModuleDir(char *szDirName, int nSize)
*
*  FUNCTION
*     Retrieves the directory where this program module (i.e. EXE file) is
*     located.
* 
*  INPUTS
*     char *szDirName - A buffer to receive the directory.
*     int  nSize      - Size of the buffer
*
*  RESULT
*     int - 0: OK
*           1: The buffer is too small
*
*  NOTES
*******************************************************************************/
static int GetModuleDir(char *szDirName, int nSize)
{
   int  ret;

   // Get full path of this executable
   ret = GetModuleFileName(NULL, szDirName, nSize);
   if (ret == nSize) {
      return 1;
   }

   // Cut off name of executable
   GetDirName(szDirName, '\\');
   return 0;
}

/****** BuildCommandLineForStarter() ******************************************
*  NAME
*     BuildCommandLineForStarter() -- Composes the command line to start
*                                     SGE_Starter.exe
*
*  SYNOPSIS
*     static int BuildCommandLineForStarter(const C_Job &Job,
*                                           const char *pszPipeName,
*                                           const char *pszCmdLineArgs,
*                                           char *szStarterCmdLine)
*
*  FUNCTION
*     Builds the command line that is provided to our child process
*     SGE_Starter.exe.
*     SGE_Starter.exe is needed on Vista and later to get access
*     to the visible desktop.
*
*  INPUTS
*     const C_Job &Job           - Needed for the job user and the job
*                                  users domain
*     const char *pszPipeName    - The name of the named pipe we open to the
*                                  SGE_Starter.exe in order to transfer the job
*                                  users password
*     const char *pszCmdLineArgs - The command line arguments for the job itself
*     
*  OUTPUTS
*     char **szStarterCmdLine    - Receives a buffer with the command line.
*                                  The user must free this buffer after using it!
*
*  RESULT
*     int - 0: no errors
*           1: Error in GetModuleDir()
*           2: Error in malloc()
*
*  NOTES
*******************************************************************************/
static int BuildCommandLineForStarter(const C_Job &Job,
                                      const char *pszPipeName,
                                      const char *pszCmdLineArgs,
                                      char **pszStarterCmdLine)
{
   int        nBufSize;
   const int  nModuleDirSize = 5000;
   char       szModuleDir[nModuleDirSize];
   char       szStarter[] = "\\SGE_Starter.exe";

   // We expect the SGE_Starter.exe to be located in the same directory
   // as SGE_Helper_Service.exe, so get this directory.
   if (GetModuleDir(szModuleDir, nModuleDirSize) != 0) {
      return 1;
   }

   // Estimate size of buffer
   nBufSize = (int)(strlen(szModuleDir) + strlen(szStarter) + strlen(Job.user)
            + strlen(Job.domain) + strlen(pszPipeName) + strlen(pszCmdLineArgs)
            + 50);  //Just add 50 Bytes for all spaces and quotation marks
   *pszStarterCmdLine = (char*)malloc(nBufSize);
   if (*pszStarterCmdLine == NULL) {
      return 2;
   }

   strcpy(*pszStarterCmdLine, "\"");
   strcat(*pszStarterCmdLine, szModuleDir);
   strcat(*pszStarterCmdLine, szStarter);
   strcat(*pszStarterCmdLine, "\" \"");
   strcat(*pszStarterCmdLine, Job.user);
   strcat(*pszStarterCmdLine, "\" \"");
   strcat(*pszStarterCmdLine, Job.domain);
   strcat(*pszStarterCmdLine, "\" \"");
   strcat(*pszStarterCmdLine, pszPipeName);
   strcat(*pszStarterCmdLine, "\" \"");
   strcat(*pszStarterCmdLine, pszCmdLineArgs);
   strcat(*pszStarterCmdLine, "\"");

   return 0;
}

/****** WritePasswordToPipe() *************************************************
*  NAME
*     WritePasswordToPipe() -- Writes the job user's password over the named
*                              pipe to the SGE_Starter.exe
*
*  SYNOPSIS
*     static int WritePasswordToPipe(const HANDLE &hPipe,
*                   const HANDLE &hProcess, const char *szPassword)
*
*  FUNCTION
*     Sends the job users password over the named pipe to the SGE_Starter.exe.
*     The pipe must already be created and the SGE_Starter.exe must be started,
*     but the pipe must not be connected (i.e. this process must not have
*     accepted the connection, the other process may already have requested the
*     connection) and the SGE_Starter.exe may already have exited.
*
*     This function waits until the SGE_Starter.exe connects to this pipe and
*     checks if the SGE_Starter.exe already exited. If the SGE_Starter.exe 
*     connects, this function writes the password to it and makes sure the
*     password is written before the function exists.
*
*  INPUTS
*     const HANDLE &hPipe      - Handle of the named pipe. Has to be created,
*                                but not yet connected.
*     const HANDLE &hProcess   - Handle of the SGE_Starter.exe process. Has
*                                to be started, may already have exited.
*     const char *szPassword   - The password of the job user.
*
*  RESULT
*     int - 0: no errors
*           1: Child died before connecting to pipe
*           2: Can't write to pipe
*           3: Child died before all data was written
*
*  NOTES
*******************************************************************************/
static int WritePasswordToPipe(const HANDLE &hPipe, const HANDLE &hProcess,
                               const char *szPassword, char *szError)
{
   BOOL   bPipeIsConnected = FALSE;
   DWORD  dwWritten;
   DWORD  dwWait = 1;  // 1 is not a return value of WaitForSingleObjectEx()
   OVERLAPPED ov;

   ZeroMemory(&ov, sizeof(ov));

   // Wait until our child has connected to our pipe, but make sure
   // our child didn't die before!
   while (bPipeIsConnected == FALSE) {
      // Check if child has connected
      if (ConnectNamedPipe(hPipe, &ov) == FALSE) {
         if (GetLastError() == ERROR_IO_PENDING) {
            // We just have to wait...
         } else if (GetLastError() == ERROR_PIPE_CONNECTED) {
            // Our child already is connected!
            bPipeIsConnected = TRUE;
            break;
         } else {
            // A severe error occured!
            break;
         }
      } else {
         // Child process has connected to pipe!
         bPipeIsConnected = TRUE;
         break;
      }
      // Check if child is still alive and sleep for 100 ms
      dwWait = WaitForSingleObjectEx(hProcess, 100, FALSE);
      if (dwWait == WAIT_OBJECT_0) {
         // Child already died!
         break;
      }
   }

   // Pipe is not connected, return error
   if (bPipeIsConnected == FALSE) {
      strcpy(szError, "SGE_Starter.exe didn't start the job");
      WriteToLogFile("SGE_Starter.exe didn't start the job");
      return 1;
   }
   
   // Pipe is connected, write to pipe
   if (WriteFile(hPipe, szPassword, (DWORD)strlen(szPassword), &dwWritten, &ov) == FALSE
      || dwWritten != strlen(szPassword)) {
      strcpy(szError, "Couldn't write all data to pipe");
      WriteToLogFile("Couldn't write all data to pipe");
      return 2;
   }

   // As the pipe is asynchronous, we will see here if the child process really
   // read all bytes or if it died before
   if (FlushFileBuffers(hPipe) == FALSE) {
      strcpy(szError, "Flushing the pipe failed");
      WriteToLogFile("Flushing the pipe failed");
      return 3;
   }

   return 0;
}

/****** CreatePipeForStarter() ************************************************
*  NAME
*     CreatePipeForStarter() -- Creates a named pipe between SGE_Helper_Service.exe
*                               and it's subproces SGE_Starter.exe
*
*  SYNOPSIS
*     static int CreatePipeForStarter(char *szPipeName, HANDLE &hPipe)
*
*  FUNCTION
*     Creates a pipe between this process and the subprocess SGE_Starter.exe.
*     This pipe is for transfering the job users password.
*
*  INPUTS
*     char **pszPipeName  - Receives a buffer with the pipe name.
*                           This buffer must be freed after using it!
*     HANDLE &hPipe       - The handle of the newly created pipe.
*
*  RESULT
*     int - 0: no errors
*           1: Can't create pipe
*           2: Error in malloc()
*
*  NOTES
*******************************************************************************/
static int CreatePipeForStarter(char **pszPipeName, HANDLE &hPipe)
{
   char       szFixedPart[] = "\\\\.\\pipe\\SGE_Helper_Service_";
   SYSTEMTIME sysTime;

   *pszPipeName = (char*)malloc(strlen(szFixedPart)+10);
   if (*pszPipeName == NULL) {
      return 2;
   }

   GetLocalTime(&sysTime);
   sprintf(*pszPipeName, "%s%02d%02d%02d%02d", szFixedPart,
      sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);

   hPipe = CreateNamedPipe(
      *pszPipeName,
      PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,
      PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT,
      PIPE_UNLIMITED_INSTANCES,
      4096,
      4096,
      1000,
      NULL);

   return (hPipe == INVALID_HANDLE_VALUE) ? 1 : 0;
}


/******************************************************************************
* Test/Debugging functions
******************************************************************************/
static void WriteEnvToFile(char *pszEnv, char *pszFile) 
{
   FILE *fp;
   char *ptr = pszEnv;

   fp = fopen(pszFile, "w+");
   if(fp) {
      while(*ptr != '\0' || *(ptr+1) != '\0') {
         if(*ptr=='\0') {
            fwrite("\n", 1, 1, fp);
         } else {
            fwrite(ptr, 1, 1, fp);
         }
         ptr++;
      }
      fclose(fp);
   }
}
