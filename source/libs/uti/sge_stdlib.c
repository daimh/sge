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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#if __linux__
#include <errno.h>
#include <sys/prctl.h>
#endif

#include "uti/sge_rmon.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_dstring.h"
#include "uti/sge_log.h" 
#include "uti/msg_utilib.h"
#include "uti/sge_uidgid.h"

bool sge_dumpable = false;

/****** uti/stdlib/sge_malloc() ***********************************************
*  NAME
*     sge_malloc() -- replacement for malloc()
*
*  SYNOPSIS
*     void* sge_malloc(size_t size)
*
*  FUNCTION
*     Allocates a memory block. Aborts in case of error.
*
*  INPUTS
*     size_t size - size in bytes
*
*  RESULT
*     void* - pointer to memory block
*
*  NOTES
*     MT-NOTE: sge_malloc() is MT safe
******************************************************************************/
void *sge_malloc(size_t size)
{
   void *cp = NULL;

   DENTER_(BASIS_LAYER, "sge_malloc");

   if (!size) {
      DRETURN_(NULL);
   }

   cp = malloc(size);
   if (!cp) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_MEMORY_MALLOCFAILED));
      DEXIT_;
      abort();
   }

   DRETURN_(cp);
}

/****** uti/stdlib/sge_realloc() **********************************************
*  NAME
*     sge_realloc() -- replacement for realloc 
*
*  SYNOPSIS
*     char* sge_realloc(char *ptr, int size, int abort) 
*
*  FUNCTION
*     Reallocates a memory block. Aborts in case of an error. 
*
*  INPUTS
*     char *ptr - pointer to a memory block
*     int size  - new size
*     int abort - do abort when realloc fails?
*
*  RESULT
*     char* - pointer to the (new) memory block
*
*  NOTES
*     MT-NOTE: sge_realloc() is MT safe
******************************************************************************/
void *sge_realloc(void *ptr, int size, int do_abort)
{
   void *cp = NULL;

   DENTER_(BASIS_LAYER, "sge_realloc");

   /* if new size is 0, just free the currently allocated memory */
   if (size == 0) {
      sge_free(&ptr);
      DRETURN_(NULL);
   }

   cp = realloc(ptr, size);
   if (cp == NULL) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_MEMORY_REALLOCFAILED));
      if (do_abort) {
         DEXIT_;
         abort();
      } else {
         sge_free(&ptr);
      }
   }

   DRETURN_(cp);
}

/****** uti/stdlib/sge_free() *************************************************
*  NAME
*     sge_free() -- replacement for free 
*
*  SYNOPSIS
*     void sge_free(void *cp)
*
*  FUNCTION
*     Replacement for free function. Accepts NULL pointers.
*
*  INPUTS
*     void *cp - pointer to a pointer of a memory block
*
*  NOTES
*     MT-NOTE: sge_free() is MT safe
******************************************************************************/
void sge_free(void *cp) 
{
   void **mem = (void **)cp;

   if (mem != NULL && *mem != NULL) {
      free(*mem);
      *mem = NULL;
   }
}  

/****** uti/stdlib/sge_putenv() ***********************************************
*  NAME
*     sge_putenv() -- put an environment variable to environment
*
*  SYNOPSIS
*     static int sge_putenv(const char *var) 
*
*  FUNCTION
*     Duplicates the given environment variable and calls the system call
*     putenv.
*
*  INPUTS
*     const char *var - variable to put in the form <name>=<value>
*
*  RESULT
*     static int - 1 on success, else 0
*
*  SEE ALSO
*     uti/stdlib/sge_setenv() 
*     uti/stdlib/sge_getenv()
*
*  NOTES
*     MT-NOTE: sge_putenv() is MT safe
*******************************************************************************/
int sge_putenv(const char *var)
{
   char *duplicate;

   if(var == NULL) {
      return 0;
   }

   duplicate = strdup(var);

   if(duplicate == NULL) {
      return 0;
   }

   if(putenv(duplicate) != 0) {
      return 0;
   }

   return 1;
}

/****** uti/stdlib/sge_setenv() ***********************************************
*  NAME
*     sge_setenv() -- Change or add an environment variable 
*
*  SYNOPSIS
*     int sge_setenv(const char *name, const char *value) 
*
*  FUNCTION
*     Change or add an environment variable 
*
*  INPUTS
*     const char *name  - variable name 
*     const char *value - new value 
*
*  RESULT
*     int - error state
*         1 - success
*         0 - error 
*
*  SEE ALSO
*     uti/stdlib/sge_putenv() 
*     uti/stdlib/sge_getenv()
*     uti/stdio/addenv()
*
*  NOTES
*     MT-NOTE: sge_setenv() is MT safe
*******************************************************************************/
int sge_setenv(const char *name, const char *value)
{
   int ret = 0;

   if (name != NULL && value != NULL) {
      dstring variable = DSTRING_INIT;

      sge_dstring_sprintf(&variable, "%s=%s", name, value);
      ret = sge_putenv(sge_dstring_get_string(&variable));
      sge_dstring_free(&variable);
   }
   return ret;
}

/* Enable daemon core dumps after setuid etc. calls in routines below,
   which typically disable dumps.  Currently only done for Linux, but
   may be usefully extended for other systems.  */
static void
make_dumpable(void)
{
#if __linux__
   DENTER(TOP_LAYER, "make_dumpable");
   if (true == sge_dumpable) {
      errno = 0;
      int ret = prctl(PR_SET_DUMPABLE, 1, 42, 42, 42);
      if (-1 == ret) {
         ERROR((SGE_EVENT, MSG_PRCTL_FAILED, strerror(errno)));
      }
   }
   DRETURN_VOID;
#endif
}

/****** uti/stdlib/sge_setuid() *************************************************
*  NAME
*     sge_setuid() -- set user id
*
*  SYNOPSIS
*     int sge_setuid(uid_t uid)
*
*  FUNCTION
*     Call setuid and maybe call prctl, or similar, afterwards.
*     prctl is called under Linux to allow core dumps subsequently.
*     That is only done if global sge_dumpable is non-zero.
*
*  INPUTS
*     uid_t uid - uid to set
*
*  RESULT
*     Per setuid
*
*  NOTES
*     MT-NOTE: MT safe as long as sge_dumpable is only set before
*     threads are started.
*******************************************************************************/
int sge_setuid(uid_t uid) {
   int ret = setuid(uid);
   make_dumpable();
   return ret;
}

/****** uti/stdlib/sge_seteuid() *************************************************
*  NAME
*     sge_seteuid() -- set effective user id
*
*  SYNOPSIS
*     int sge_seteuid(uid_t uid)
*
*  FUNCTION
*     Call seteuid and maybe call prctl, or similar, afterwards.
*     prctl is called under Linux to allow core dumps subsequently.
*     That is only done if global sge_dumpable is non-zero.
*
*  INPUTS
*     uid_t uid - uid to set
*
*  RESULT
*     Per seteuid
*
*  NOTES
*     MT-NOTE: MT safe as long as sge_dumpable is only set before
*     threads are started.
*******************************************************************************/
int sge_seteuid(uid_t uid) {
   int ret = seteuid(uid);
   make_dumpable();
   return ret;
}

/****** uti/stdlib/sge_setgid() *************************************************
*  NAME
*     sge_setgid() -- set group id
*
*  SYNOPSIS
*     int sge_setgid(gid_t gid)
*
*  FUNCTION
*     Call setgid and maybe call prctl, or similar, afterwards.
*     prctl is called under Linux to allow core dumps subsequently.
*     That is only done if global sge_dumpable is non-zero.
*
*  INPUTS
*     gid_t gid - gid to set
*
*  RESULT
*     Per setuid
*
*  NOTES
*     MT-NOTE: MT safe as long as sge_dumpable is only set before
*     threads are started.
*******************************************************************************/
int sge_setgid(gid_t gid) {
   int ret = setgid(gid);
   make_dumpable();
   return ret;
}

/****** uti/stdlib/sge_setegid() *************************************************
*  NAME
*     sge_setegid() -- set effective group id
*
*  SYNOPSIS
*     int sge_setegid(gid_t gid)
*
*  FUNCTION
*     Call setegid and maybe call prctl, or similar, afterwards.
*     prctl is called under Linux to allow core dumps subsequently.
*     That is only done if global sge_dumpable is non-zero.
*
*  INPUTS
*     gid_t gid - gid to set
*
*  RESULT
*     Per setegid
*
*  NOTES
*     MT-NOTE: MT safe as long as sge_dumpable is only set before
*     threads are started.
*******************************************************************************/
int sge_setegid(gid_t gid) {
   int ret = setegid(gid);
   make_dumpable();
   return ret;
}

/****** uti/stdlib/sge_maybe_set_dumpable() ************************************
*  NAME
*     sge_maybe_set_dumpable() -- maybe allow core dumps after sge_setuid & al
*
*  SYNOPSIS
*     void sge_maybe_set_dumpable(void gid)
*
*  FUNCTION
*     Conditionally allows dumping core after calls to sge_setuid & al.
*     The condition is that SGE_ENABLE_COREDUMP is set in the environment.
*
*  NOTES
*     MT-NOTE: sge_maybe_set_dumpable() is not MT safe
*/
void sge_maybe_set_dumpable(void) {
  if (getenv("SGE_ENABLE_COREDUMP") != NULL)
     sge_dumpable = true;
  return;
}
