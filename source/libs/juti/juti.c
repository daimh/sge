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

#ifdef WINDOWS

#include <windows.h>

#else

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <termios.h>

#endif

void setEcho(int flag) 
{
#ifdef WINDOWS
   HANDLE hConsole;
   DWORD  dwMode;

   hConsole = CreateFile("CONIN$", 
              GENERIC_READ|GENERIC_WRITE, 
              FILE_SHARE_READ|FILE_SHARE_WRITE,
              NULL,
              OPEN_EXISTING,
              0,
              NULL);

   if(hConsole != INVALID_HANDLE_VALUE) {
      GetConsoleMode(hConsole, &dwMode);
      if(flag) {
         dwMode |= ENABLE_ECHO_INPUT;
      } else {
         dwMode &= ~ENABLE_ECHO_INPUT;
      }
      SetConsoleMode(hConsole, dwMode);
      CloseHandle(hConsole);
   }
#else /* unix */
   static struct termios init_set;
   static int initialized = 0;

   struct termios new_set;
   
   if(initialized == 0) {
      tcgetattr(fileno(stdin), &init_set);
      initialized = 1;
   }

   new_set = init_set;
   if(flag) {   
      tcsetattr(fileno(stdin), TCSAFLUSH, &init_set);
   } else {
      new_set.c_lflag &= ~ECHO;
      tcsetattr(fileno(stdin), TCSAFLUSH, &new_set);
   }

#endif
}

