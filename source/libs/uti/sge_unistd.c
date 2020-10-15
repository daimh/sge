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

#include "sge_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h> 

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_log.h"
#include "uti/msg_utilib.h"
#include "uti/sge_prog.h"

#include "gdi/sge_gdi_ctx.h"

typedef enum {
   FILE_TYPE_NOT_EXISTING,
   FILE_TYPE_FILE,
   FILE_TYPE_DIRECTORY
} file_type_t; 

/* MT-NOTE: This module is MT safe */

static int sge_domkdir(const char *, int, bool, bool);

static file_type_t sge_get_file_type(const char *name);  

static file_type_t sge_get_file_type(const char *name)
{
   SGE_STRUCT_STAT stat_buffer;
   file_type_t ret = FILE_TYPE_NOT_EXISTING;
 
   if (SGE_STAT(name, &stat_buffer)) {
      ret = FILE_TYPE_NOT_EXISTING;
   } else {
      if (S_ISDIR(stat_buffer.st_mode)) {
         ret = FILE_TYPE_DIRECTORY;
      } else if (S_ISREG(stat_buffer.st_mode)) {
         ret = FILE_TYPE_FILE;
      } else {
         ret = FILE_TYPE_NOT_EXISTING;
      }
   }
   return ret;
}             

static int sge_domkdir(const char *path_, int fmode, bool exit_on_error, bool may_not_exist) 
{
   SGE_STRUCT_STAT statbuf;
 
   DENTER(TOP_LAYER, "sge_domkdir");
 
   if (mkdir(path_, (mode_t) fmode)) {
      if (errno == EEXIST) {
         if (may_not_exist) {
            DEXIT;
            return -1;
         } else {
            DEXIT;
            return 0;
         }
      }
 
      if (!SGE_STAT(path_, &statbuf) && S_ISDIR(statbuf.st_mode)) {
         /*
          * may be we do not have permission, 
          * but directory already exists 
          */
         DEXIT;
         return 0;
      }
 
      if (exit_on_error) {
         CRITICAL((SGE_EVENT, MSG_FILE_CREATEDIRFAILED_SS, path_, 
                   strerror(errno)));
         SGE_EXIT(NULL, 1);
      } else {
         ERROR((SGE_EVENT, MSG_FILE_CREATEDIRFAILED_SS, path_, 
                strerror(errno)));
         DEXIT;
         return -1;
      }
   }
#if defined( INTERIX )
   /*
    * mkdir is buggy, chown dir manually
    */
   /* Flawfinder: ignore */
   chown(path_, geteuid(), getegid());
#endif
 
   DEXIT;
   return 0;
}         

/****** uti/unistd/sge_unlink() ***********************************************
*  NAME
*     sge_unlink() -- delete a name and possibly the file it refers to
*
*  SYNOPSIS
*     bool sge_unlink(const char *prefix, const char *suffix) 
*
*  FUNCTION
*     Replacement for unlink(). 'prefix' and 'suffix' will be combined
*     to a filename. This file will be deleted. 'prefix' may be NULL.
*
*  INPUTS
*     const char *prefix - pathname or NULL
*     const char *suffix - filename 
*
*  RESULT
*     int - error state
*         true  - OK
*         false - Error
******************************************************************************/
bool sge_unlink(const char *prefix, const char *suffix) 
{
   int status;
   char file[SGE_PATH_MAX];
 
   DENTER(TOP_LAYER, "sge_unlink");
 
   if (!suffix) {
      ERROR((SGE_EVENT, SFNMAX, MSG_POINTER_SUFFIXISNULLINSGEUNLINK));
      DEXIT;
      return false;
   }
 
   if (prefix) {
      snprintf(file, sizeof(file), "%s/%s", prefix, suffix);
   } else {
      snprintf(file, sizeof(file), "%s", suffix);
   }
 
   DPRINTF(("file to unlink: \"%s\"\n", file));
   status = unlink(file);
 
   if (status) {
      ERROR((SGE_EVENT, MSG_FILE_UNLINKFAILED_SS, file, strerror(errno)));
      DEXIT;
      return false;
   } else {
      DEXIT;
      return true;
   }
}  

/****** uti/unistd/sge_sleep() ************************************************
*  NAME
*     sge_sleep() -- sleep for x microseconds 
*
*  SYNOPSIS
*     void sge_sleep(int sec, int usec) 
*
*  FUNCTION
*     Delays the calling application for 'sec' seconds and 'usec'
*     microseconds 
*
*  INPUTS
*     int sec  - seconds 
*     int usec - microseconds 
******************************************************************************/
void sge_sleep(int sec, int usec) 
{
   struct timeval timeout;
 
   timeout.tv_sec = sec;
   timeout.tv_usec = usec;
 
   select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout);
}       

/****** uti/unistd/sge_chdir_exit() *******************************************
*  NAME
*     sge_chdir_exit() -- Replacement for chdir() 
*
*  SYNOPSIS
*     int sge_chdir_exit(const char *path, int exit_on_error) 
*
*  FUNCTION
*     Change working directory 
*
*  INPUTS
*     const char *path  - pathname 
*     int exit_on_error - exit in case of errors 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - ERROR ('exit_on_error'==1 the function may not return)
*
*  SEE ALSO
*     uti/unistd/sge_chdir()
******************************************************************************/
int sge_chdir_exit(const char *path, int exit_on_error) 
{
   DENTER(BASIS_LAYER, "sge_chdir");
 
   if (chdir(path)) {
      if (exit_on_error) {
         CRITICAL((SGE_EVENT, MSG_FILE_NOCDTODIRECTORY_S , path));
         SGE_EXIT(NULL, 1);
      } else {
         ERROR((SGE_EVENT, MSG_FILE_NOCDTODIRECTORY_S , path));
         return -1;
      }
   }
 
   DEXIT;
   return 0;
}           

/****** uti/unistd/sge_chdir() ************************************************
*  NAME
*     sge_chdir() --  Replacement for chdir()
*
*  SYNOPSIS
*     int sge_chdir(const char *dir) 
*
*  FUNCTION
*     Change working directory 
*
*  INPUTS
*     const char *dir - pathname 
*
*  RESULT
*     int - error state
*        0 - success
*        != 0 - error 
*
*  NOTE
*     Might be used in shepherd because it does not use CRITICAL/ERROR.
*     TODO: pass a dstring for the return of error messages.
*
*  SEE ALSO
*     uti/unistd/sge_chdir_exit()
******************************************************************************/
int sge_chdir(const char *dir)
{
   if (dir != NULL) {
      SGE_STRUCT_STAT statbuf;

      /*
       * force automount
       */
      SGE_STAT(dir, &statbuf);
      return chdir(dir);
   }

   /* on error return -1 */
   return -1;
}

/****** uti/unistd/sge_exit() *************************************************
*  NAME
*     sge_exit() -- Wrapped exit Function 
*
*  SYNOPSIS
*     void sge_exit(int i) 
*
*  FUNCTION
*     Calls 'exit_func' if installed. Stops monitoring with DCLOSE 
*
*  INPUTS
*     sge_gdi_ctx_class_t **ref_ctx - address of the context, the context is freed in exit_func
*     int i          - exit state 
*
*  SEE ALSO
*     uti/unistd/sge_install_exit_func()
******************************************************************************/
void sge_exit(void **ref_ctx, int i) 
{
   sge_exit_func_t exit_func = NULL;

   if (ref_ctx && *ref_ctx) {
      sge_gdi_ctx_class_t *ctx = *ref_ctx;
      exit_func = ctx->get_exit_func(ctx);
   }   
   if (exit_func) {
      exit_func(ref_ctx, i);
   }
   exit(i);
}
 

/****** uti/unistd/sge_mkdir() ************************************************
*  NAME
*     sge_mkdir() -- Create a directory (and necessary parents)
*
*  SYNOPSIS
*     int sge_mkdir(const char *path, int fmode, bool exit_on_error, bool may_not_exist) 
*
*  FUNCTION
*     Create a directory 
*
*  INPUTS
*     const char *path  - path 
*     int fmode         - file mode 
*     bool exit_on_error - as it says 
*     bool may_not_exist - if true an error is returned if the last component
*                         of the path exists
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error (The function may never return)
******************************************************************************/
int sge_mkdir(const char *path, int fmode, bool exit_on_error, bool may_not_exist) 
{
   int i = 0, res=0;
   char path_[SGE_PATH_MAX];
 
   DENTER(TOP_LAYER, "sge_mkdir");
   if (!path) {
      if (exit_on_error) {
         CRITICAL((SGE_EVENT, SFNMAX, MSG_VAR_PATHISNULLINSGEMKDIR));
         DCLOSE;
         SGE_EXIT(NULL, 1);
      } else {
         ERROR((SGE_EVENT, SFNMAX, MSG_VAR_PATHISNULLINSGEMKDIR));
         DEXIT;
         return -1;
      }
   }

   DPRINTF(("Making dir \"%s\"\n", path));

   memset(path_, 0, sizeof(path_));
   while ((unsigned char) path[i]) {
      path_[i] = path[i];
      if ((path[i] == '/') && (i != 0)) {
         path_[i] = (unsigned char) 0;
         res = sge_domkdir(path_, fmode, exit_on_error, false);
         if (res) {
            DPRINTF(("retval = %d\n", res));
            DEXIT;
            return res;
         }
      }
      path_[i] = path[i];
      i++;
   }
 
   i = sge_domkdir(path_, fmode, exit_on_error, may_not_exist);
 
   DPRINTF(("retval = %d\n", i));
   DEXIT;
   return i;
}   

int sge_mkdir2(const char *base_dir, const char *name, int fmode, bool exit_on_error)
{
   dstring path = DSTRING_INIT;
   int ret;

   DENTER(TOP_LAYER, "sge_mkdir2");
   
   if (base_dir == NULL || name == NULL) {
      if (exit_on_error) {
         CRITICAL((SGE_EVENT, SFNMAX, MSG_VAR_PATHISNULLINSGEMKDIR));
         DCLOSE;
         SGE_EXIT(NULL, 1);
      } else {
         ERROR((SGE_EVENT, SFNMAX, MSG_VAR_PATHISNULLINSGEMKDIR));
         DRETURN(-1);
      }
   }
  
   sge_dstring_sprintf(&path, "%s/%s", base_dir, name);

   ret = sge_mkdir(sge_dstring_get_string(&path), fmode, exit_on_error, false); 
   sge_dstring_free(&path);

   DRETURN(ret);
}

/****** uti/unistd/sge_rmdir() ************************************************
*  NAME
*     sge_rmdir() -- Recursive rmdir
*
*  SYNOPSIS
*     int sge_rmdir(const char *cp, dstring *error)  
*
*  FUNCTION
*     Remove a directory tree. In case of errors a message may be found
*     in 'error' afterwards.
*
*  INPUTS
*     const char *cp  - path 
*     dstring *error  - destination for error message if non-NULL
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error 
******************************************************************************/
int sge_rmdir(const char *cp, dstring *error) 
{
   SGE_STRUCT_STAT statbuf;
   SGE_STRUCT_DIRENT *dent;
   DIR *dir;
   char dirent[SGE_PATH_MAX*2];
   char fname[SGE_PATH_MAX];

   DENTER(TOP_LAYER, "sge_rmdir");
 
   if (!cp) {
      sge_dstring_sprintf(error, "%s", MSG_POINTER_NULLPARAMETER);
      DEXIT;
      return -1;
   }
 
   if (!(dir = opendir(cp))) {
      sge_dstring_sprintf(error, MSG_FILE_OPENDIRFAILED_SS , cp, strerror(errno));
      DEXIT;
      return -1;
   }
 
   while (SGE_READDIR_R(dir, (SGE_STRUCT_DIRENT *)dirent, &dent)==0 && dent!=NULL) {
      if (strcmp(dent->d_name, ".") && strcmp(dent->d_name, "..")) {
 
         snprintf(fname, sizeof(fname), "%s/%s", cp, dent->d_name);

#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	 switch (dent->d_type) {
	 case DT_UNKNOWN:
	    break;
	 case DT_DIR:
	    if (sge_rmdir(fname, error)) {
	       sge_dstring_sprintf(error, "%s", MSG_FILE_RECURSIVERMDIRFAILED );
	       closedir(dir);
	       DEXIT;
	       return -1;
	    }
	    continue;
	 default:
	    if (unlink(fname)) {
	       sge_dstring_sprintf(error, MSG_FILE_UNLINKFAILED_SS,
				   fname, strerror(errno));
	       closedir(dir);
	       DEXIT;
	       return -1;
	    }
	    continue;
	 }
#endif /* HAVE_STRUCT_DIRENT_D_TYPE */

#ifndef WIN32 /* lstat not called */
         if (SGE_LSTAT(fname, &statbuf)) {
            sge_dstring_sprintf(error, MSG_FILE_STATFAILED_SS , fname, strerror(errno));
            closedir(dir);
            DEXIT;
            return -1;
         }
#else
         /* so symbolic links under Windows */
         if (SGE_STAT(fname, &statbuf)) {
            sge_dstring_sprintf(error, MSG_FILE_STATFAILED_SS , fname, strerror(errno));
            closedir(dir);
            DEXIT;
            return -1;
         }
#endif /* WIN32 */
 
    if (S_ISDIR(statbuf.st_mode) && !S_ISLNK(statbuf.st_mode)) 
    {
    if (sge_rmdir(fname, error)) {
               sge_dstring_sprintf(error, "%s", MSG_FILE_RECURSIVERMDIRFAILED );
               closedir(dir);
               DEXIT;
               return -1;
            }
         }
         else {                                                
#ifdef TEST
            printf("unlink %s\n", fname);
#else
            if (unlink(fname)) {
               sge_dstring_sprintf(error, MSG_FILE_UNLINKFAILED_SS,
                      fname, strerror(errno));
               closedir(dir);
               DEXIT;
               return -1;
            }
#endif
         }
      }
   }
 
   closedir(dir);
 
#ifdef TEST
   printf("rmdir %s\n", cp);
#else
   if (rmdir(cp)) {
      sge_dstring_sprintf(error, MSG_FILE_RMDIRFAILED_SS , cp, strerror(errno));
      DEXIT;
      return -1;
   }
#endif
 
   return 0;
}
 
int sge_is_executable(const char *name) 
{
   SGE_STRUCT_STAT stat_buffer;
   int ret = SGE_STAT(name, &stat_buffer);
   if (!ret) { 
      return (int)(stat_buffer.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH));
   } else {
      return 0;
   }
}


/****** uti/unistd/sge_is_directory() *****************************************
*  NAME
*     sge_is_directory() -- Does 'name' exist and is it a directory? 
*
*  SYNOPSIS
*     int sge_is_directory(const char *name) 
*
*  FUNCTION
*     Does 'name' exist and is it a directory?  
*
*  INPUTS
*     const char *name  - directory name 
*
*  RESULT
*     int - error state
*         0 - No
*         1 - Yes 
******************************************************************************/
int sge_is_directory(const char *name)
{
   return (sge_get_file_type(name) == FILE_TYPE_DIRECTORY);
}
 
/****** uti/unistd/sge_is_file() **********************************************
*  NAME
*     sge_is_file() -- Does 'name' exist and is it a file? 
*
*  SYNOPSIS
*     int sge_is_file(const char *name) 
*
*  FUNCTION
*     Does 'name' exist and is it a file?  
*
*  INPUTS
*     const char *name  - filename 
*
*  RESULT
*     int - error state
*         0 - No
*         1 - Yes 
******************************************************************************/
int sge_is_file(const char *name)
{
   return (sge_get_file_type(name) == FILE_TYPE_FILE);
}                         
 
#ifdef TEST
int main(int argc, char **argv)
{
   char err_str[1024];
 
   if (argc!=2) {
      fprintf(stderr, "usage: rmdir <dir>\n");
      exit(1);
   }
   if (sge_rmdir(argv[1], err_str)) {
      fprintf(stderr, "%s", err_str);
      return 1;
   }
   return 0;
}
#endif   
