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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Portions of this software are Copyright (c) 2011 Univa Corporation
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
#include <Sddl.h>
#include <process.h>
#include <Userenv.h>

// Global Variables:
static char  *g_pCmdLine       = NULL;
static TCHAR g_szWindowClass[] = "SGE_Starter"; // the main window class name
static BOOL  g_bDoLogging = FALSE;

// Forward declarations of functions included in this code module:
ATOM                    MyRegisterClass(HINSTANCE hInstance);
BOOL                    InitInstance(HINSTANCE, int);
LRESULT CALLBACK        WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK        About(HWND, UINT, WPARAM, LPARAM);
int                     StartJob(int argc, char *argv[]);

/****** SGE_Starter/WriteToLogFile() *****************************************
*  NAME
*     WriteToLogFile() -- Appends a line to the log file.
*
*  SYNOPSIS
*     static int WriteToLogFile(const char *szMessage, ...)
*
*  FUNCTION
*     Appends a line to the log file. If the log file doesn't exist yet, it
*     creates the log file.
*     The log file path is currently fixed to "C:\Temp\SGE_Starter.log". If
*     opening this file is not possible, a new file named
*     "C:\Temp\SGE_Starter$TIMESTAMP.log" will be created, where $TIMESTAMP
*     is of the format "HHMMSS".
*
*  INPUTS
*     szMessage - Format string of the message to be added to the log file.
*     ...       - Arguments required by the format string. See printf(3C).
*
*  RESULTS
*     int - 0 if the line was successfully appended to the log file,
*           1 if an error occured,
*           2 if logging is disabled.
******************************************************************************/
static int WriteToLogFile(const char *szMessage, ...)
{
   int         ret = 1;
   FILE        *fp = NULL;
   DWORD       dwLastError;
   SYSTEMTIME  sysTime;
   va_list     args;
   char        Buffer[4096];
   static BOOL g_bFirstTime = TRUE;
   static char g_szTraceFile[5000];

   if (g_bDoLogging == FALSE) {
      return 2;
   }

   // We do not want to change LastError in here.
   dwLastError = GetLastError();
   GetLocalTime(&sysTime);

   // If we don't have a trace file yet, create it's name.
   if (g_bFirstTime == TRUE) {
      g_bFirstTime = FALSE;
      // The trace file name is $TEMP\SGE_Starter.$PID_$TIMESTAMP.log
      // e.g. "C:\Temp\SGE_Starter.18732_142316.log"
      // This should be unique enough for this purpose.
      strcpy(Buffer, "C:\\TEMP");
      GetEnvironmentVariable("TEMP", Buffer, 4095);
      _snprintf(g_szTraceFile, 4999, "%s\\SGE_Starter.%d_%02d%02d%02d.log", Buffer,
         _getpid(), sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
   }

   fp = fopen(g_szTraceFile, "a+");
   if(fp != NULL) {
      va_start(args, szMessage);
      _vsnprintf(Buffer, 4095, szMessage, args);
      fprintf(fp, "%02d:%02d:%02d [SGE_Starter] %s\n",
         sysTime.wHour, sysTime.wMinute, sysTime.wSecond, Buffer);
      fflush(fp);
      fclose(fp);
      ret = 0;
   }
   SetLastError(dwLastError);

   return ret;
}

/****** SGE_Starter/LogErrorMessageToFile() **********************************
*  NAME
*     LogErrorMessageToFile() -- Logs a user error message followed by the last
*                                system error message to the log file.
*
*  SYNOPSIS
*     static void LogErrorMessageToFile(const char *szErrorMessage)
*
*  FUNCTION
*     Logs a error message, followed by the system error message and the
*     GetLastError() number, to the log file.
*
*  INPUTS
*     const char *szErrorMessage - the user defined error message
*
*  RESULTS
*     void - none.
******************************************************************************/
static void LogErrorMessageToFile(const char *szErrorMessage)
{
   DWORD dwError;
   char  szLastError[501];
   char  szError[4096];

   dwError = GetLastError();

   if(dwError == 193) {
      // FormatMessage doesn't provide an error message for errno=193
      // The table of System Errors from the MSDN Libary tells us:
      // 193 Is not a valid application.  ERROR_BAD_EXE_FORMAT 
      strcpy(szLastError, "Is not a valid application.");
   } else {
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 
         NULL, dwError, 0, szLastError, 500, NULL);
   }

   if(strlen(szLastError) > 0) {
      if(szLastError[strlen(szLastError)-2] == '\r') {
         szLastError[strlen(szLastError)-2] = '\0';
      }
   } else {
      strcpy(szLastError, "(no error message available from system)");
   }

   sprintf(szError, "%s: %s (errno=%d)",
      szErrorMessage,
      szLastError,
      dwError);

   WriteToLogFile("Error: %s", szError);
}   

/****** SGE_Starter/LogProcessOwnerToFile() **********************************
*  NAME
*     LogProcessOwnerToFile() -- Logs user, domain and SID to the log file
*
*  SYNOPSIS
*     static void LogProcessOwnerToFile()
*
*  FUNCTION
*     Logs the user, domain and SID of the owner of the current process to
*     the log file.
*     Used for debugging purposes only.
*
*  RESULTS
*     void - none.
******************************************************************************/
static void LogProcessOwnerToFile()
{
   // Who am I?
   HANDLE hProcessToken;
   char TokenBuffer[50000];
   DWORD TokenLength = 0;
   char Sid[1000];
   DWORD dwSidSize = sizeof(Sid);
   char  szDomainName[5000];
   DWORD dwNameSize = sizeof(szDomainName);
   SID_NAME_USE Type;
   char szDomainUser[5000];

   if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hProcessToken) == FALSE) {
      WriteToLogFile("Cant OpenProcessToken, %d", GetLastError());
   }
   if (GetTokenInformation(hProcessToken, TokenUser, TokenBuffer, 50000, &TokenLength) == FALSE) {
      WriteToLogFile("Can't GetTokenInformation(), %d", GetLastError());
   }

   PSID pSid = ((TOKEN_USER*)TokenBuffer)->User.Sid;

   LPTSTR pszStringSid;
   ConvertSidToStringSid(pSid, &pszStringSid);
   WriteToLogFile("SID = %s", pszStringSid);
   LocalFree(pszStringSid);

   DWORD dwUserSize = 5000;
   if (LookupAccountSid(NULL, pSid, szDomainUser, &dwUserSize, szDomainName, &dwNameSize, &Type) == FALSE) {
      WriteToLogFile("Cant LookupAccountSid");
   }
   WriteToLogFile("szDomainUser = %s", szDomainUser);
   WriteToLogFile("szDomainName = %s", szDomainName);
}


/****** SGE_Starter/ParseCmdLine() *******************************************
*  NAME
*     ParseCmdLine() -- Splits the command line into separate options.
*
*  SYNOPSIS
*     static int ParseCmdLine(char *lpCmdLine, int *argc, char **argv)
*
*  FUNCTION
*     Splits the command line into the seperate command line options, like
*     it is done by the shell.
*
*  INPUTS
*     char *lpCmdLine - The command line. This string will be modified
*                       by this function.
*  OUTPUTS
*     int  *argc  - Number of arguments in the argument list.
*     char **argv - Pointer to an array of strings. The array must be
*                   already allocated!
*                   The entries of this array are just pointers to places in
*                   the modified lpCmdLine string, so don't modify lpCmdLine
*                   afterwards.
*                   The entry 0 is always "Dummy", because in the
*                   main function this would be the binary name, the
*                   first command line argument is in entry 1.
*
*  RESULTS
*     void - none.
******************************************************************************/
static void ParseCmdLine(char *lpCmdLine, int *argc, char **argv)
{
   const char cTerm = '\0';
   const char cQuote = '\"';
   char       *pCurChar = lpCmdLine;

   *argc = 0;
   argv[(*argc)++] = "Dummy";

   pCurChar = lpCmdLine;
   while (*pCurChar) {
      // Skip leading whitespaces
      while (isspace(*pCurChar)) {
         pCurChar++;
      }

      if (*pCurChar == cQuote) { // Quoted arguments
         pCurChar++;  // Skip quote

         // Here the current argument begins
         argv[(*argc)++] = pCurChar;

         // Search for closing quote
         while (*pCurChar &&
            !(*pCurChar == cQuote && (isspace(pCurChar[1]) || pCurChar[1] == cTerm))) {
            pCurChar++; 
         }

         // Terminate the current argument and jump to the next one
         if (*pCurChar != NULL) {
            *pCurChar = cTerm;
            if (pCurChar[1] != NULL) {
               pCurChar += 2;
            }
         }
      } else {  // Unquoted argument
         // Here the current argument begins
         argv[(*argc)++] = pCurChar;

         // Search trailing whitespace
         while (*pCurChar && !isspace(*pCurChar)) {
            pCurChar++;
         }
         // Terminate the current argument and jump to the next one
         if (*pCurChar && isspace(*pCurChar)) {
            *pCurChar = cTerm;
            pCurChar++;
         }
      }
   }
}

/****** SGE_Starter/ReadPasswordFromPipe() ***********************************
*  NAME
*     ReadPasswordFromPipe() -- Reads the user password from the pipe to
*                               the SGE_Helper_Service.exe
*
*  SYNOPSIS
*     static int ReadPasswordFromPipe(const char *szPipeName, char *szPass,
*                                     int nBufLen)
*
*  FUNCTION
*     Reads the users password from the SGE_Helper_Service.exe through a
*     named pipe whose name was provided in the command line.
*
*  INPUTS
*     const char *szPipeName - The name of the named pipe.
*     char       *szPass     - A buffer large enough to hold the users
*                              password. Clear this buffer securely after
*                              finished using it!
*     int        nBufLen     - Size of the password buffer in bytes.
*
*  RESULTS
*     int - 0 if the password was read successfully,
*           1 if the named pipe couldn't be opened,
*           2 if the named pipe couldn't be set into read mode,
*           3 if the password couldn't be read from the open pipe.
******************************************************************************/
static int ReadPasswordFromPipe(const char *szPipeName, char *szPass, int nBufLen)
{
   char   szErrorPart[4096];
   DWORD  dwRead;
   DWORD  dwMode = PIPE_READMODE_BYTE;
   HANDLE hPipe = INVALID_HANDLE_VALUE;
   int    nRet = 0;

   try {
      hPipe = CreateFile( 
         szPipeName, 
         GENERIC_READ|GENERIC_WRITE,
         0,
         NULL,
         OPEN_EXISTING,
         0,
         NULL);

      if (hPipe == INVALID_HANDLE_VALUE) {
         sprintf(szErrorPart, "Couldn't open pipe %s", szPipeName);
         throw 1;
      }

      if (SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL) == FALSE) {
         sprintf(szErrorPart, "Couldn't set pipe read mode");
         throw 2;
      }

      if (ReadFile(hPipe, szPass, nBufLen, &dwRead, NULL) == FALSE) {
         sprintf(szErrorPart, "Couldn't read from pipe");
         throw 3;
      }
   }
   catch (int nRetVal) {
      nRet = nRetVal;
      LogErrorMessageToFile(szErrorPart);
   }

   if (hPipe != INVALID_HANDLE_VALUE) {
      CloseHandle(hPipe);
   }
   return nRet;
}

/****** SGE_Starter/ConvertStringToWideChar() ********************************
*  NAME
*     ConvertStringToWideChar() -- Converts an ANSI string to wide char
*
*  SYNOPSIS
*     static int ConvertStringToWideChar(const char *szInput,
*                                        LPWSTR &szwOutput, int &nOutBufLen)
*
*  FUNCTION
*     Converts an ANSI string to wide characters. This format is needed by
*     all ...W() Windows API functions.
*
*  INPUTS
*     const char *szInput    - The string in ANSI format (a normal C string).
*     LPWSTR     &szwOutput  - A reference to the pointer to the buffer that
*                              will be filled with wide char string.
*                              The buffer gets allocated inside this function!
*                              You have to free() it after using it.
*     int        &nOutBufLen - A reference to the length of the buffer. Gets
*                              filled inside this function.
*
*  RESULTS
*     int - 0 if the string was converted successfully,
*           1 if there was an error allocating the buffer for the wide char
*             string.
******************************************************************************/
static int ConvertStringToWideChar(const char *szInput, LPWSTR &szwOutput, int &nOutBufLen)
{
   // Converting from ANSI to wide character strings is a bit complicated -
   // first ask the converter function for the needed buffer size, then
   // allocate the buffer, and then convert the string.
   nOutBufLen = MultiByteToWideChar(CP_ACP, 0, szInput, (int)strlen(szInput)+1, 0, 0);
   szwOutput = (LPWSTR)malloc(nOutBufLen*sizeof(wchar_t));
   if (szwOutput == NULL) {
      return 1;
   }
   MultiByteToWideChar(CP_ACP, 0, szInput, (int)strlen(szInput)+1, szwOutput, nOutBufLen);
   return 0;
}

/****** SGE_Starter/WinMain() ************************************************
*  NAME
*     WinMain() -- Entry point of this executable
*
*  SYNOPSIS
*     int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
*                          LPTSTR lpCmdLine, int nCmdShow)
*
*  FUNCTION
*     Entry point of this executable. Initializes this instance of the
*     application and does the message loop.
*
*  INPUTS
*     See WinAPI documentation.
*
*  RESULTS
*     int - Depends on how this application gets terminated.
*           See WinAPI documentation for details.
******************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
   MSG  msg;
   char szBuf[101];

   GetEnvironmentVariable("SGE_DO_LOGGING", szBuf, 100);
   sscanf(szBuf, "%d", (int*)&g_bDoLogging);

   g_pCmdLine = _strdup(lpCmdLine);

	MyRegisterClass(hInstance);
	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {
	   TranslateMessage(&msg);
	   DispatchMessage(&msg);
	}
   free((void*)g_pCmdLine);
	return (int)msg.wParam;
}

/****** SGE_Starter/MyRegisterClass() ****************************************
*  NAME
*     MyRegisterClass() -- Registers a 'personalized' window class
*
*  SYNOPSIS
*     ATOM MyRegisterClass(HINSTANCE hInstance)
*
*  FUNCTION
*     Registers a special window class for this application at the system.
*
*  INPUTS
*     HINSTANCE hInstance - Handle to this instance of the application.
*     See WinAPI documentation for details.
*
*  RESULTS
*     ATOM - Class-Atom of the registered class.
*     See WinAPI documentation for details.
******************************************************************************/
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

   ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			 = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	 = (WNDPROC)WndProc;
	wcex.hInstance		 = hInstance;
	wcex.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszClassName = g_szWindowClass;

	return RegisterClassEx(&wcex);
}

/****** SGE_Starter/WndProc() ************************************************
*  NAME
*     WndProc() -- The message handler of the main window
*
*  SYNOPSIS
*     LRESULT CALLBACK WndProc(HWND hWnd, UINT message, 
*                              WPARAM wParam, LPARAM lParam)
*
*  FUNCTION
*     Processes messages for the main window.
*     Exits this application with return code 57 if the main window is visible
*     on the default desktop of the default window station ("WinSta0").
*
*  INPUTS
*     HWND     hWnd      - Handle of the main window
*     UINT     message   - Message-ID
*     WPARAM   wParam    - First parameter of the message
*     LPARAM   lParam    - Second parameter of the message
*     See WinAPI documentation for details.
*
*  RESULTS
*     LRESULT - 0 if the message was handled by this function,
*               the result of DefWindowProc() if it was handled by
*               the default handler.
******************************************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC         hdc;
   int         ret  = 0;
   int         argc = 0;
   char        *argv[1000];

	switch (message) 
	{
      // This message is sent when CreateWindow() finishes
   	case WM_CREATE:
         WriteToLogFile("");
         WriteToLogFile("----------- starting ------------");
#ifdef DEBUG_XXY
         LogProcessOwnerToFile();
#endif
         ParseCmdLine(g_pCmdLine, &argc, argv);
         ret = StartJob(argc, argv);   // Forward exit status of job
         WriteToLogFile("----------- ending --------------");
         PostQuitMessage(ret);
		   break;
      // This message is sent whenever a region of the window 
      // has to be repainted
	   case WM_PAINT:
		   hdc = BeginPaint(hWnd, &ps);
		   EndPaint(hWnd, &ps);
		   break;
      // This message is sent when the Window is to be destroyed
	   case WM_DESTROY:
		   PostQuitMessage(0);
		   break;
	   default:
		   return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

/****** SGE_Starter/InitInstance() **********************************************
*  NAME
*     InitInstance() -- Initializes this instance of this application
*
*  SYNOPSIS
*     BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
*
*  FUNCTION
*     Creates and shows the main Windows of this application.
*
*  INPUTS
*     HINSTANCE hInstance - Handle of this instance of this application.
*     int       nCmdShow  - Show mode of the main window,
*                           e.g. maximized, normal, show, hide, ...
*     See WinAPI documentation for details.
*
*  RESULTS
*     BOOL - TRUE if OK, FALSE is creation of main windows failed.
******************************************************************************/
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hWnd = CreateWindow(g_szWindowClass, "SGE_Starter", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd) {
      return FALSE;
   }
   // We don't want to see this window on the desktop!
   //ShowWindow(hWnd, nCmdShow);
   //UpdateWindow(hWnd);

   return TRUE;
}

/****** SGE_Starter/StartJob() **************************************************
*  NAME
*     InitInstance() -- Initializes this instance of this application
*
*  SYNOPSIS
*     BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
*
*  FUNCTION
*     Creates and shows the main Windows of this application.
*
*  INPUTS
*     HINSTANCE hInstance - Handle of this instance of this application.
*     int       nCmdShow  - Show mode of the main window,
*                           e.g. maximized, normal, show, hide, ...
*     See WinAPI documentation for details.
*
*  RESULTS
*     BOOL - TRUE if OK, FALSE is creation of main windows failed.
******************************************************************************/
int StartJob(int argc, char *argv[])
{
   char                szCmdLine[20000];
   char                szErrorPart[4096];
   char                szPass[1000];
   STARTUPINFO         si; 
   PROCESS_INFORMATION pi; 
   BOOL                bResult     = FALSE;
   DWORD               dwError     = ERROR_SUCCESS;
   DWORD               dwExitCode  = 0;
   HANDLE              hStdout     = INVALID_HANDLE_VALUE;
   HANDLE              hStderr     = INVALID_HANDLE_VALUE;
   int                 nRet        = 0;
   int                 nLen        = 0;
   int                 nPassLen    = 0;
   LPCWSTR             szwUser     = NULL;
   LPCWSTR             szwDomain   = NULL;
   LPCWSTR             szwPass     = NULL;
   LPWSTR              szwCmdLine  = NULL;

   ZeroMemory(&si, sizeof(si));
   ZeroMemory(&pi, sizeof(pi));
   ZeroMemory(szCmdLine, sizeof(szCmdLine));

/*
 * Arguments:
 * 0 = binary name
 * 1 = Job user name
 * 2 = Job user domain name
 * 3 = Name of pipe to retrieve password
 * 4... = cmd line
 */
   try {
      if (argc < 4) {   
         sprintf(szErrorPart, "Missing command line arguments");
         throw 1;
      }

      // The command line arguments of the job were the last arguments in
      // our command line - just copy them to the command line of the job.
      for (int i=4; i<argc; i++) {
         strcat(szCmdLine, argv[i]);
         strcat(szCmdLine, " ");
      }

      // Fill the STARTUPINFO struct for the job
      // The standard handles were already redirected by the SGE_Helper_Service.
      si.cb           = sizeof(STARTUPINFO);
      si.dwFlags      |= STARTF_USESTDHANDLES;
      si.hStdOutput   = GetStdHandle(STD_OUTPUT_HANDLE);
      si.hStdError    = GetStdHandle(STD_ERROR_HANDLE);
      si.wShowWindow  = SW_SHOW;
      si.lpDesktop    = NULL;

      // CreateProcessWithLogonA() (using ANSI strings) is known to be buggy in
      // some Windows versions, so we use CreateProcessWithLogonW() (which uses
      // wide character strings). Therefore we have to convert all strings that
      // we want to use as parameters to CreateProcessWithLogonW() to wide strings.
      ConvertStringToWideChar(argv[1], (LPWSTR&)szwUser, nLen);
      ConvertStringToWideChar(argv[2], (LPWSTR&)szwDomain, nLen);
      ReadPasswordFromPipe(argv[3], szPass, sizeof(szPass));
      ConvertStringToWideChar(szPass, (LPWSTR&)szwPass, nPassLen);
      SecureZeroMemory(szPass, sizeof(szPass));
      ConvertStringToWideChar(szCmdLine, szwCmdLine, nLen);
      
	  LPVOID pszEnv = NULL;

      WriteToLogFile("Try to get user env...");
	  
	  HANDLE hToken;

      WriteToLogFile("Calling LogonUserW...");
	  bResult = LogonUserW(
          szwUser,
          szwDomain,
          szwPass,
          LOGON32_LOGON_INTERACTIVE,
          LOGON32_PROVIDER_DEFAULT,
		  &hToken); 
      if (!bResult) {
	      WriteToLogFile("LogonUserW failed for %s", szwUser);
	  };
	 
	
	// as SGE_Starter.cpp is started as SYSTEM user it gets
	// "HOME=/" set. This can confuse e.g. the Python function 
	// os.path.expanduser("~/...")
	//
	// I'm too lazy to figure out the correct value and removing
	// it is better than to have a wrong value...
	WriteToLogFile("Removing HOME environment variable");
	SetEnvironmentVariable("HOME", NULL);
	
        // PATH is also weird and filled with stuff like:
	// "C:\Windows\SUA\windows\system32" and "C"
        // even worse the rather important "C:\Windows\System32" is missing..
	// Let Windows just set the fresh default path
	WriteToLogFile("Removing PATH environment variable");
	SetEnvironmentVariable("PATH", NULL);
	
    WriteToLogFile("CreateEnvironmentBlock...");
    // TRUE: inherit environment from this process
    if (!CreateEnvironmentBlock(&pszEnv, hToken, TRUE)) {
	    WriteToLogFile("CreateEnvironmentBlock failed for %s", szwUser);
	    pszEnv = NULL;
	} else {
	  WriteToLogFile("Got user env!");
	};
	  
      WriteToLogFile("Creating Process with command line \"%s\"", szCmdLine);
      bResult = CreateProcessWithLogonW(
         szwUser,
         szwDomain,
         szwPass,
         LOGON_WITH_PROFILE,
         NULL,
         szwCmdLine,
         NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT,
         pszEnv,
         NULL,
         (LPSTARTUPINFOW)&si,
         &pi);

      CloseHandle(hToken);
	  if (!DestroyEnvironmentBlock(pszEnv)) {
	      WriteToLogFile("Could not destroy environment block!");
	  };

      // Free all allocated memory immediately (esp. the password buffer), but
      // don't let the SecureZeroMemory() function overwrite our LastError.
      dwError = GetLastError();
      free((void*)szwUser);
      free((void*)szwDomain);
      SecureZeroMemory((PVOID)szwPass, nPassLen);
      free((void*)szwPass);
      free((void*)szwCmdLine);
      
      if (!bResult) {
         sprintf(szErrorPart, "CreateProcessAsUser failed, Command is \"%s\"", szCmdLine);
         // Re-set the LastError of CreateProcessWithLogonW()
         SetLastError(dwError);
         throw 2;
      }
      WriteToLogFile("Successfully created process");
 
      // Process should already be in Job object because of inheritation
      if (pi.hProcess == INVALID_HANDLE_VALUE) {
         sprintf(szErrorPart, "Got invalid process handle");
         throw 3;
      }

      // Wait for job to finish
      DWORD dwWait = 0;
      WriteToLogFile("Waiting for job end.");
      dwWait = WaitForSingleObjectEx(pi.hProcess, INFINITE, FALSE);
      if(dwWait==WAIT_OBJECT_0) {
         if (GetExitCodeProcess(pi.hProcess, &dwExitCode) == FALSE) {
            throw 4;
         }
         nRet = (int)dwExitCode;
         WriteToLogFile("The SGE_Helper_Service should get the complete usage of the job");
      } else {
         WriteToLogFile("Waiting for job end failed!");
         throw 5;
      }

      WriteToLogFile("Job ended.");
      CloseHandle(pi.hProcess); 
   }
   catch (int nRetVal) {
      LogErrorMessageToFile(szErrorPart);
      // In order to transfer the job exit code and the SGE_Starter.exe
      // exit code to the SGE_Helper_Service.exe, we place the job
      // exit code into the lower 16 bit and the SGE_Starter.exe exit code
      // into the higher 16 bit of our return value.
      nRet = (nRetVal << 16) + dwExitCode;
   }

   if(pi.hThread != INVALID_HANDLE_VALUE) {
      CloseHandle(pi.hThread); 
   }
   return nRet;
}
