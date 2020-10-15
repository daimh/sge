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

/*****************************************************************************
*  GUI_test.exe is a binary that tests if the Window that it opens is
*  beeing displayed on the visible desktop.
*
*  Purpose: This binary allows the testsuite to test the
*           display_win_gui complex variable on Windows hosts.
*
*  GUI_test.cpp : Defines the entry point for the application.
******************************************************************************/
#include <windows.h>

// Global Variables:
TCHAR     g_szWindowClass[] = "GUI_TEST"; // the main window class name

// Forward declarations of functions included in this code module:
ATOM				   MyRegisterClass(HINSTANCE hInstance);
BOOL				   InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

/****** GUI_test/WinMain() ***************************************************
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
	MSG msg;

	MyRegisterClass(hInstance);
	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) {
		return FALSE;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {
	   TranslateMessage(&msg);
	   DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

/****** GUI_test/MyRegisterClass() *******************************************
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

/****** GUI_test/InitInstance() **********************************************
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

   hWnd = CreateWindow(g_szWindowClass, "GUI_test", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd) {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}
/****** GUI_test/WndProc() ***************************************************
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
   HWINSTA     hWinsta;
   HDESK       hDesk;
   char        szDeskName[100];
   char        szWinstaName[100];
   int         ret = 0;

	switch (message) 
	{
      // This message is sent when CreateWindow() finishes
   	case WM_CREATE:
         // Check if we are on the visible Desktop
         hWinsta = GetProcessWindowStation();
         hDesk   = GetThreadDesktop(GetCurrentThreadId());

         GetUserObjectInformation(hWinsta, UOI_NAME, szWinstaName, 100, NULL);
         GetUserObjectInformation(hDesk, UOI_NAME, szDeskName, 100, NULL);

         if (strcmp(szWinstaName, "WinSta0") == 0
            && strcmp(szDeskName, "Default") == 0) {
            ret = 57;
         }
         PostQuitMessage(ret);
		   break;
      // This message is sent whenever a region of the window 
      // has to be repainted
	   case WM_PAINT:
		   hdc = BeginPaint(hWnd, &ps);
		   // TODO: Add any drawing code here...
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
