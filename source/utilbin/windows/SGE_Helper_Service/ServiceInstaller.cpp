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

#include <windows.h>
#include <stdio.h>

#include "ServiceInstaller.h"


/****** InstallService() ******************************************************
*  NAME
*     InstallService() -- Installs the SGE Helper Service
*
*  SYNOPSIS
*     int InstallService(const char *pszBinaryPathName, 
*                        const char *pszServiceName, 
*                        const char *pszDisplayName, 
*                        const char *pszDescription) 
*
*  FUNCTION
*     Installs the SGE Helper Service to the system.
*
*  INPUTS
*     const char *pszBinaryPathName - path of SGE_Helper_Service.exe
*     const char *pszServiceName    - internal name of the service
*     const char *pszDisplayName    - display name in the service control manager
*     const char *pszDescription    - description in the service control manager
*
*  RESULT
*     int - did the installation succeed?
*     0:  no errors
*     >0: installation failed
*
*  NOTES
*******************************************************************************/
int InstallService(const char *pszBinaryPathName, 
                   const char *pszServiceName, 
                   const char *pszDisplayName, 
                   const char *pszDescription) 
{ 
   SC_HANDLE           schService;
   SC_HANDLE           schSCManager;
   SERVICE_DESCRIPTION ServiceDescription;
   LPSTR               lpszBinaryPathName = NULL;
   LPTSTR              lptstrFilePart = NULL;
   DWORD               dwBufSize = 0;
   int                 ret = 0;

   try {
      dwBufSize = GetFullPathName(pszBinaryPathName, 0, NULL, &lptstrFilePart);
      if(dwBufSize == 0) {
         throw 1;
      }
      lpszBinaryPathName = (LPSTR)malloc(dwBufSize + 20);
      if(lpszBinaryPathName == NULL) {
         throw 2;
      }
      dwBufSize = GetFullPathName(pszBinaryPathName, dwBufSize + 19, lpszBinaryPathName, &lptstrFilePart);
      if(dwBufSize == 0) {
         throw 3;
      }
      schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
      if(schSCManager == NULL) {
         throw 4;
      }

      schService = CreateService( 
         schSCManager,                 // SCManager database 
         pszServiceName,               // name of service 
         pszDisplayName,               // service name to display 
         SERVICE_ALL_ACCESS,           // desired access 
         SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS,    // service type 
         SERVICE_AUTO_START,           // start type 
         SERVICE_ERROR_NORMAL,         // error control type 
         lpszBinaryPathName,           // service's binary 
         NULL,                         // no load ordering group 
         NULL,                         // no tag identifier 
         NULL,                         // no dependencies 
         NULL,                         // LocalSystem account 
         NULL);                        // no password 
    
      if (schService == NULL) {
         throw 5;
      }

      printf("Service successfully installed.\n");

      ServiceDescription.lpDescription = (LPTSTR)pszDescription;
      ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &ServiceDescription);
   }
   catch(int retval) {
      DWORD     dwLastError;
      char      ErrorMessage[501];

      dwLastError = GetLastError();
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, ErrorMessage, 500, NULL);

      printf("Service installation failed.\n");
      printf("Error: %d: %s\n", dwLastError, ErrorMessage);
      ret = retval;
   }

   if(lpszBinaryPathName != NULL) {
      free(lpszBinaryPathName);
   }
   if(schService != NULL) {
      CloseServiceHandle(schService); 
   }
   if(schSCManager != NULL) {
      CloseServiceHandle(schSCManager);
   }
   return ret;
}

/****** UninstallService() ****************************************************
*  NAME
*     UninstallService() -- Uninstalls the SGE Helper Service
*
*  SYNOPSIS
*     int UninstallService(const char *pszServiceName)
*
*  FUNCTION
*     Uninstalls the SGE Helper Service from the system.
*
*  INPUTS
*     const char *pszServiceName - internal name of the service
*
*  RESULT
*     int - did the uninstallation succeed?
*     0:  no errors
*     >0: uninstallation failed
*
*  NOTES
*******************************************************************************/
int UninstallService(const char *pszServiceName) 
{ 
   SC_HANDLE schService   = NULL;
   SC_HANDLE schSCManager = NULL;
   DWORD     dwLastError;
   char      ErrorMessage[501];
   int       ret = 0;

   try {
      schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
      if(schSCManager == NULL) {
         throw 4;
      }
      schService = OpenService(schSCManager, pszServiceName, SERVICE_ALL_ACCESS);
      if(schService == NULL) {
         throw 5;
      }
      if(!DeleteService(schService)) {
         throw 6;
      }
      printf("Service successfully uninstalled.\n");
   }
   catch(int retval) {
      dwLastError = GetLastError();
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, ErrorMessage, 500, NULL);

      printf("Service uninstallation failed.\n");
      printf("Error: %d: %s\n", dwLastError, ErrorMessage);
      ret = retval;
   }

   if(schService != NULL) {
      CloseServiceHandle(schService); 
   }
   if(schSCManager != NULL) {
      CloseServiceHandle(schSCManager);
   }
   return ret;
}
