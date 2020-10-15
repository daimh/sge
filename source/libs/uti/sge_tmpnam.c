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
 *   Copyright: 2003 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "uti/sge_tmpnam.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "uti/sge_rmon.h"
#include "uti/sge_dstring.h"
#include "uti/sge_string.h"
#include "uti/sge_unistd.h"
#include "uti/msg_utilib.h"

#include "basis_types.h"

static int elect_path(dstring *aBuffer);
static int spawn_file(dstring *aBuffer, dstring *error_message);

/****** uti/sge_tmpnam/sge_mkstemp() *******************************************
*  NAME
*     sge_mkstemp() -- SGE version of mkstemp()
*
*  SYNOPSIS
*     int sge_mkstemp(char *aBuffer, size_t size, dstring *error_message)
*
*  FUNCTION
*     SGE-specific version of mkstemp(3) that tries TMPDIR as well as
*     P_tmpdir before /tmp, returns an error message, and guarantees
*     read and write access for the user only.
*
*     The 'aBuffer' argument points to an array of at least
*     'size' in length.  'aBuffer' will contain the generated
*     filename upon successful completion, and the file descriptor for
*     the opened file is returned.  If the function fails, -1 will be
*     returned, with an error message in error_message.
*
*  INPUTS
*     char *aBuffer - Array to hold filename
*     size_t size - size of aBuffer
*     dstring error_message - Error message
*
*  RESULT
*     char* - Points to 'aBuffer' if successful, NULL otherwise
*
*  NOTE
*     MT-NOTE: sge_mkstemp() is MT safe.
******************************************************************************/
int sge_mkstemp(char *aBuffer, size_t size, dstring *error_message)
{
   dstring s = DSTRING_INIT;
   int fd;

   DENTER(TOP_LAYER, "sge_mkstemp");

   if (aBuffer == NULL) {
      sge_dstring_sprintf(error_message, "%s", MSG_TMPNAM_GOT_NULL_PARAMETER);
      DEXIT;
      return -1;
   }

   if (elect_path(&s) < 0) {
      sge_dstring_sprintf(error_message, "%s", MSG_TMPNAM_CANNOT_GET_TMP_PATH);
      sge_dstring_free(&s);
      DEXIT;
      return -1;
   }

   if ((sge_dstring_get_string(&s))[sge_dstring_strlen(&s)-1] != '/') {
      sge_dstring_append_char(&s, '/');
   }

   if ((fd = spawn_file(&s, error_message)) < 0) {
      sge_dstring_free(&s);
      DEXIT;
      return -1;
   }

   sge_strlcpy(aBuffer, sge_dstring_get_string(&s), size);
   sge_dstring_free(&s);

   DPRINTF(("sge_mkstemp: returning %s\n", aBuffer));
   DEXIT;
   return fd;
}

static int elect_path(dstring *aBuffer)
{
   const char *d;

   d = getenv("TMPDIR");
   if ((d != NULL) && sge_is_directory(d)) {
      sge_dstring_append(aBuffer, d);
      return 0;
   } else if (sge_is_directory(P_tmpdir)) {
      sge_dstring_append(aBuffer, P_tmpdir);
      return 0;
   } else if (sge_is_directory("/tmp")) {
      sge_dstring_append(aBuffer, "/tmp/");
      return 0;
   }
   return -1;
}


static int spawn_file(dstring *aBuffer, dstring *error_message) {
   int my_errno;
   int mkstemp_return;
   char tmp_file_string[256];
   char tmp_string[SGE_PATH_MAX];

   /*
    * generate template filename for mkstemp()
    */
   snprintf(tmp_file_string, 256, "pid-%u-XXXXXX", (unsigned int)getpid());

   /*
    * check final length of path
    */
   if (sge_dstring_strlen(aBuffer) + strlen(tmp_file_string) >= SGE_PATH_MAX) {
      sge_dstring_append(aBuffer, tmp_file_string);
      sge_dstring_sprintf(error_message, MSG_TMPNAM_SGE_MAX_PATH_LENGTH_US,
                          sge_u32c(SGE_PATH_MAX), sge_dstring_get_string(aBuffer));
      return -1;
   }

   /*
    * now build full path string for mkstemp()
    */
   snprintf(tmp_string, SGE_PATH_MAX, "%s%s", sge_dstring_get_string(aBuffer), tmp_file_string);

   /*
    * generate temp file by call to mkstemp()
    */
   errno = 0;
   mkstemp_return = mkstemp(tmp_string);
   my_errno = errno;
   if (-1 == mkstemp_return
       /* POSIX doesn't guarantee the mode.  */
       || fchmod(mkstemp_return, 0600)) {
      sge_dstring_sprintf(error_message, MSG_TMPNAM_GOT_SYSTEM_ERROR_SS,
                          strerror(my_errno),
                          sge_dstring_get_string(aBuffer));
      return -1;
   }

   /*
    * finally copy the resulting path to aBuffer
    */
   sge_dstring_sprintf(aBuffer, "%s", tmp_string);
   return mkstemp_return;
}
