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

#include "win32_type.h"
#include "simplelog.h"

DWORD error_id = 0;
TXCHAR error_message = "";
FILE *error_filestream = NULL;
BOOL error_traingenabled = FALSE;

void
error_set(DWORD id, LPSTR message) 
{
   error_id = id;
   strcpy(error_message, message);
   DPRINTF(("id=%d message=\"%s\"\n", id, message));
}

void
error_print(void)
{    
   FILE *out = stdout;

   if (error_id != 0) {
      fprintf(out, "_sge_pseudo_host:error(%d):%s\n", error_id, error_message);
      fflush(out);
   }
   error_id = 0; 
}

int 
error_printf(const char *fmt, ...)
{
   int ret = 0;

   if (error_traingenabled) {
      FILE *out = error_filestream ? error_filestream : stderr;
      va_list args;

      va_start(args, fmt);
      ret = vfprintf(out, fmt, args);
      fflush(out);
      va_end(args);
   }
   return ret;
} 

void
error_set_tracingenabled(BOOL new_state)
{
   error_traingenabled = new_state;
}

int
error_open_tracefile(TXCHAR tracefile) 
{
   int ret = 0;

   error_filestream = fopen(tracefile, "a+");
   if (error_filestream == NULL) {
      fprintf(stderr, "unable to open file \"%s\"\n", tracefile);
      fflush(stderr);
      ret = 1;
   }
   return ret;
}

int 
error_close_tracefile(void)
{
   int ret = 0;
   int local_ret;
   
   local_ret = fclose(error_filestream);
   if (local_ret != 0) {
      ret = 1;
   }
   return ret;
}

