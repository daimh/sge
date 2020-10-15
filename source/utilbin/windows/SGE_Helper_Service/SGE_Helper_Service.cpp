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
#include "JobList.h"
#include "JobStart.h"
#include "SGE_Helper_Service.h"
#include "ServiceInstaller.h"
#include "Communication.h"
#include "Logging.h"

// constants
// name, display name and description of the service in "Services" dialog
// of control panel
static const char g_szServiceName[] = "SGE_Helper_Service.exe";
static const char g_szServiceDisplayName[] = "Sun Grid Engine Helper Service";
static const char g_szServiceDescription[] = "Enables Sun Grid Engine "
                               "execution host functionality to let jobs "
                               "display their GUI to the visible desktop.";

// internal name of the service handler
static const char g_szServiceHandlerName[] = "Sun Grid Engine Helper Service Handler";

// global variables
HANDLE          g_hWinstaACLMutex = NULL;
HANDLE          g_hDeskACLMutex = NULL;
C_Communication g_Communication;
C_JobList       g_JobList;
BOOL            g_bAcceptJobs    = TRUE;
int             g_Port = 0;
BOOL            g_bDoLogging = FALSE;

// module variables
static SERVICE_STATUS_HANDLE g_hServiceStatus = NULL;
static BOOL                  g_bRunning       = TRUE;

// function forward declarations
static void  WINAPI SunGridEngineHelperServiceStart(DWORD dwArgc, LPTSTR *lpszArgv);
static DWORD WINAPI SunGridEngineHelperServiceHandlerEx(DWORD dwControl, 
                          DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
static void PrintHelp(const char *pszExecutableName, const char *pszServiceName);
static void ServiceError();


/****** main() ****************************************************************
*  NAME
*     main() -- starting point of the executable
*
*  SYNOPSIS
*     int main(int argc, char *argv[]);
*
*  FUNCTION
*     A Windows Service works this way:
*     When the service is started, either automatically at boot time or manually,
*     main() gets called by the ServiceControlManager (SCM).
*
*     main() has to register the Control Dispatcher Thread to the SCM. The Control
*     Dispatcher Thread gets called by the SCM, 
*
*  INPUTS
*     int  argc    - number of command line arguments
*     char *argv[] - array of arguments
*
*  RESULT
*     int - exit status of the executable
*     0: no errors
*     1: installation or uninstallation of service failed
* TODO: Document all error codes
*
*  NOTES
*******************************************************************************/
int main(int argc, char *argv[])
{
   int ret = 0;

   SERVICE_TABLE_ENTRY DispatchTable[] = {
      {(LPSTR)g_szServiceName, SunGridEngineHelperServiceStart},
      {NULL,                   NULL}};

   if(argc==2) {
      /*
       * Newer Microsoft compilers warn if "strnicmp" is used, you have to use "_strnicmp" to
       * avoid this warning. This is for all libc functions.
       */
      if(_strnicmp(argv[1], "-install", 8)==0) {
         ret = InstallService(argv[0], g_szServiceName,
                              g_szServiceDisplayName, g_szServiceDescription);
      } else if(_strnicmp(argv[1], "-uninstall", 10)==0) {
         ret = UninstallService(g_szServiceName);
      } else if((g_Port=atoi(argv[1]))!=0) {
         if(!StartServiceCtrlDispatcher(DispatchTable)) {
            SvcDebugOut("[%s] StartServiceCtrlDispatcher error = %d\n",
               g_szServiceDisplayName, GetLastError());
            ret = 7;
         }
      } else {
         PrintHelp(argv[0], g_szServiceDisplayName);
      }
   } else if(argc==1) {
      // Start Execd Helper Service
      if(!StartServiceCtrlDispatcher(DispatchTable)) {
         SvcDebugOut("[%s] StartServiceCtrlDispatcher error = %d\n",
            g_szServiceDisplayName, GetLastError());
         ret = 7;
      }
   } else {
      PrintHelp(argv[0], g_szServiceDisplayName);
   }
	return ret;
}

/****** SunGridEngineHelperServiceHandlerEx() *********************************
*  NAME
*     SunGridEngineHelperServiceHandlerEx() -- handler for the service that is
*                                      invoked by the service control manager
*
*  SYNOPSIS
*     DWORD WINAPI SunGridEngineHelperServiceHandlerEx(DWORD  dwControl,
*                                                     DWORD  dwEventType,
*                                                     LPVOID lpEventData,
*                                                     LPVOID lpContext)
*
*  FUNCTION
*     This handler is invoked by the service control manager when the service
*     is to be started, stopped or altered.
*     See HandlerEx() in Microsoft PlatformSDK documentation.
*  
*     Pause and continue are used to shutdown the service gracefully:
*     First call pause, the service goes to pause mode and doesn't accept any
*     new jobs. After this, call resume. Resume will fail as long as there
*     are jobs running - if no job is left running, resume will succeed and
*     stop the service.
*
*  INPUTS
*     DWORD  dwControl   - action that is to be performed
*     DWORD  dwEventType - more specific information for some actions
*     LPVOID lpEventData - more specific information for some actions
*     LPVOID lpContext   - more specific information for some actions
*
*  RESULT
*     DWORD - NO_ERROR if no errors occured, see HandlerEx() in Microsoft
*             PlatformSDK documentation for list of errors.
*
*  NOTES
*******************************************************************************/
static DWORD WINAPI SunGridEngineHelperServiceHandlerEx(DWORD  dwControl,
                                                       DWORD  dwEventType,
                                                       LPVOID lpEventData,
                                                       LPVOID lpContext)
{
   SERVICE_STATUS  SvcStatus;

   SvcStatus.dwServiceType             = SERVICE_WIN32_OWN_PROCESS 
                                         |SERVICE_INTERACTIVE_PROCESS;
   SvcStatus.dwControlsAccepted        = SERVICE_ACCEPT_SHUTDOWN  
                                         |SERVICE_ACCEPT_STOP 
                                         |SERVICE_ACCEPT_PAUSE_CONTINUE;
   SvcStatus.dwWin32ExitCode           = NO_ERROR;
   SvcStatus.dwServiceSpecificExitCode = 0;
   SvcStatus.dwCheckPoint              = 0;
   SvcStatus.dwWaitHint                = 1000;

   switch(dwControl) {
      case SERVICE_CONTROL_STOP:
         SvcStatus.dwCurrentState = SERVICE_STOPPED; 

         if(!SetServiceStatus(g_hServiceStatus, &SvcStatus)) { 
            ServiceError();
         } 
         g_bRunning = FALSE;
         break;

      case SERVICE_CONTROL_PAUSE:
         g_bAcceptJobs = FALSE;
         SvcStatus.dwCurrentState = SERVICE_PAUSED; 

         if(!SetServiceStatus(g_hServiceStatus, &SvcStatus)) { 
            ServiceError();
         } 
         break;

      case SERVICE_CONTROL_CONTINUE:
         if(g_JobList.IsEmpty()) {
            g_bRunning = FALSE;
            SvcStatus.dwCurrentState = SERVICE_STOPPED;
         } else {
            SvcStatus.dwCurrentState = SERVICE_PAUSED; 
         }

         if(!SetServiceStatus(g_hServiceStatus, &SvcStatus)) { 
            ServiceError();
         } 
         break;

      case SERVICE_CONTROL_INTERROGATE:
         break; 

      default: 
         SvcDebugOut("[%s] Unrecognized opcode %ld\n",  
                       g_szServiceDisplayName, dwControl); 
   }
   return NO_ERROR;
}

/****** SunGridEngineHelperServiceStart() *********************************************
*  NAME
*     SunGridEngineHelperServiceStart() -- starting point of the service
*
*  SYNOPSIS
*     void WINAPI SunGridEngineHelperServiceStart(DWORD dwArgc, LPTSTR *lpszArgv)
*
*  FUNCTION
*     This is the starting point of the service that is called by the
*     service manager when the service is started.
*
*  INPUTS
*     DWORD  dwArgc    - number of arguments
*     LPTSTR *lpszArgv - argument list
*
*  RESULT
*     void - no result
*
*  NOTES
*******************************************************************************/
static void WINAPI SunGridEngineHelperServiceStart(DWORD dwArgc, LPTSTR *lpszArgv)
{
   SERVICE_STATUS        ServiceStatus;
   DWORD                 dwStatus = 0;
   int                   ret;
   char                  job_result[] = "done";

   // Parse command line options
   for (DWORD i=1; i<dwArgc; i++) {
      if (strcmp(lpszArgv[i], "-log") == 0) {
         g_bDoLogging = TRUE;
         break;
      }
   }
   WriteToLogFile("Starting %s.", g_szServiceDisplayName);

   ServiceStatus.dwServiceType             = SERVICE_WIN32_OWN_PROCESS
                                             |SERVICE_INTERACTIVE_PROCESS;
   ServiceStatus.dwCurrentState            = SERVICE_RUNNING;
   ServiceStatus.dwControlsAccepted        = SERVICE_ACCEPT_SHUTDOWN
                                             |SERVICE_ACCEPT_STOP  
                                             |SERVICE_ACCEPT_PAUSE_CONTINUE;
   ServiceStatus.dwWin32ExitCode           = NO_ERROR;
   ServiceStatus.dwServiceSpecificExitCode = 0;
   ServiceStatus.dwCheckPoint              = 0;
   ServiceStatus.dwWaitHint                = 1000;

   g_hServiceStatus = RegisterServiceCtrlHandlerEx(g_szServiceName, 
                              SunGridEngineHelperServiceHandlerEx, NULL);

   if(g_hServiceStatus == (SERVICE_STATUS_HANDLE)0) { 
      SvcDebugOut("[%s] RegisterServiceCtrlHandler failed %d\n",  
                             g_szServiceDisplayName, GetLastError()); 
      return; 
   } 

   if(!SetServiceStatus(g_hServiceStatus, &ServiceStatus)) {
      dwStatus = GetLastError(); 
      SvcDebugOut("[%s] SetServiceStatus error %ld\n", 
                         g_szServiceDisplayName, dwStatus); 
   }

   WriteToLogFile("%s started successfully.", g_szServiceDisplayName);

   g_hWinstaACLMutex = CreateMutex(NULL, FALSE, "WinstaACLMutex");
   g_hDeskACLMutex   = CreateMutex(NULL, FALSE, "DeskACLMutex");

   // listen on port
   ret = g_Communication.InitComm();
   if(ret != 0) {
      g_bRunning = FALSE;
   }

   // Main loop
   while(g_bRunning == TRUE) {
      ret = g_Communication.DoComm();
      if(ret != 0) {
         g_bRunning = FALSE;
      }
   }
   g_Communication.ExitComm();

   CloseHandle(g_hWinstaACLMutex);
   CloseHandle(g_hDeskACLMutex);
}

/****** PrintHelp() ***********************************************************
*  NAME
*     PrintHelp() - prints usage information to stdout
*
*  SYNOPSIS
*     void PrintHelp(const char *pszExecutableName, const char *pszServiceName)
*
*  FUNCTION
*     Prints usage information to stdout
*
*  INPUTS
*     const char* pszExecutableName - name of the executable (use argv[0])
*     const char* pszServiceName    - display name of the service
*
*  RESULT
*     void - no result
*
*  NOTES
*******************************************************************************/
static void PrintHelp(const char *pszExecutableName, const char *pszServiceName)
{
   printf("%s -install | -uninstall | -help\n"
          "  -install     Installs %s\n"
          "  -uninstall   Uninstalls %s\n"
          "  -help        Displays this help\n",
          pszExecutableName, pszServiceName, pszServiceName);
}

/****** SvcDebugOut() *********************************************************
*  NAME
*     SvcDebugOutput() - prints error messages to the debugger
*
*  SYNOPSIS
*     void SvcDebugOut(LPSTR String, ...)
*
*  FUNCTION
*     Prints error messages to the debugger
*
*  INPUTS
*     LPSTR  String      - format string of error message
*     ...                - parameters of error message
*
*  RESULT
*     void - no result
*
*  NOTES
*******************************************************************************/
void SvcDebugOut(LPSTR String, ...)
{
   char    Buffer[1024];
   va_list args;

   va_start(args, String);
   _vsnprintf(Buffer, 1023, String, args);

   OutputDebugStringA(Buffer);
}

/****** ServiceError() ********************************************************
*  NAME
*     ServiceError() -- Writes the last error to the debugger
*
*  SYNOPSIS
*     static void ServiceError()
*
*  FUNCTION
*     Prints the last error (GetLastError()) to the debugger.
*
*  RESULT
*     void - no result
*******************************************************************************/
static void ServiceError()
{
   DWORD dwStatus = GetLastError(); 
   SvcDebugOut("[%s] SetServiceStatus error %ld\n", 
      g_szServiceDisplayName, dwStatus); 
}
