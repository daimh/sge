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

#include <grp.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <rpc/rpc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <sys/statvfs.h>
#include <interix/interix.h>
#include <interix/security.h>

#include "sge_rmon.h"
#include "sge_log.h"
#include "basis_types.h"
#include "../../../utilbin/sge_passwd.h"
#include "msg_wingrid.h"
#include "config_file.h"
#include "sge_bootstrap.h"
#include "wingrid.h"

#include "uti/sge_stdlib.h"

static bool wl_enable_windomacc = false;

/* ============================================================================
 * STATIC Functions
 * ==========================================================================*/
/****** wingrid/wl_copy_string_to_buffer() ***********************************
*  NAME
*     wl_copy_string_to_buffer() -- Copies one string to a position in a
*                                   buffer, sets buffer position and 
*                                   remaining size of buffer.
*
*  SYNOPSIS
*     static int wl_copy_string_to_buffer(const char *string, char *buf,
*                                          int *bufpos, int *bufremain)
*
*  FUNCTION
*     Copies a string into a buffer at given position. Checks before copying
*     if the string will fit into the buffer. After copying, sets the index
*     of the next free buffer element and the size of the remaining free
*     buffer, so a subsequent call to this function will copy the next
*     string to the next free location in the buffer.
*
*  INPUTS
*     const char *string    - The string that is to copy into the buffer.
*     char       *buf       - Pointer to beginning of the buffer.
*     int        *bufpos    - Pointer to the Index of the position in the buffer,
*                             where the string should be copied to.
*     int        *bufremain - Pointer to the number of remaining free bytes in
*                             the buffer before copying.
*
*  OUTPUTS
*     int        *bufpos    - Pointer to the Index of the next free position
*                             in the buffer.
*     int        *bufremain - Pointer to the number of remaining free bytes in
*                             the buffer after copying.
*
*  RESULTS
*     int - 0 if OK, 1 if the buffer is too small.
*
*  NOTES
*     MT-NOTE: wl_copy_string_to_buffer() is MT-safe
******************************************************************************/
static int wl_copy_string_to_buffer(const char *string, char *buf, 
                                    int *bufpos, int *bufremain)
{
   int stringlen;

   stringlen = strlen(string)+1; /* Include trailing '\0' */
   if(stringlen > *bufremain) {
      return 1;
   }
   memcpy(&buf[*bufpos], (unsigned char*)string, stringlen);
   *bufpos += stringlen;
   *bufremain -= stringlen;

   return 0;
}

/****** wingrid/wl_getpwuid_ex_r() *******************************************
*  NAME
*     wl_getpwuid_ex_r() -- MT safe version of extended Interix version of
*                           getpwuid()
*
*  SYNOPSIS
*     int wl_getpwuid_ex_r(uid_t uid, struct passwd *pwd, char *buffer,
*                          size_t bufsize, struct passwd **result, uint flags)
*
*  FUNCTION
*     MT safe function to retrieve passwd information for a specific UID.
*     Uses Interix' extended version of getpwuid() that allows to retrieve
*     full qualified Windows domain user names.
*     E.g.: "BOFUR+Administrator" instead of "Administrator"
*
*  INPUTS
*     uid_t           uid     - The UID of the passwd
*     struct passwd  *pwd     - Pointer of the passwd struct that is to be filled.
*     char           *buffer  - Buffer providing memory for the strings in passwd. 
*     size_t          bufsize - Size of the buffer.
*     uint            flags   - 0:           short name is retrieved
*                               PW_FULLNAME: FQDN name is retrieved
*
*  OUTPUTS
*     struct passwd **result  - Points to pwd after function call, 
*                               see getpwuid_r().
*
*  RESULTS
*     int - 0 if OK, 1 if the buffer is too small.
*
*  NOTES
*     MT-NOTE: wl_getpwuid_ex_r() is not MT-safe
*              It is safe it getpwuid_ex() is not called from any other place.
*
*  SEE ALSO
*     getpwuid_r(), getpwuid_ex()
******************************************************************************/
int wl_getpwuid_ex_r(uid_t uid, struct passwd *pwd, char *buffer,
                    size_t bufsize, struct passwd **result, uint flags)
{
   static pthread_mutex_t getpwuid_ex_mutex = PTHREAD_MUTEX_INITIALIZER;
   int ret;
   int bufpos;
   int bufremain;
   struct passwd *pwd_ex;

   pthread_mutex_lock(&getpwuid_ex_mutex);

   pwd_ex = getpwuid_ex(uid, flags);
   pwd->pw_uid    = pwd_ex->pw_uid;
   pwd->pw_gid    = pwd_ex->pw_gid;
   pwd->pw_change = pwd_ex->pw_change;
   pwd->pw_expire = pwd_ex->pw_expire;

   bufpos    = 0;
   bufremain = bufsize;

   pwd->pw_name = &buffer[bufpos];
   ret = wl_copy_string_to_buffer(pwd_ex->pw_name, buffer, &bufpos, &bufremain);
   if(ret==0) {
      pwd->pw_dir = &buffer[bufpos];
      ret = wl_copy_string_to_buffer(pwd_ex->pw_dir, buffer, &bufpos, &bufremain);
   }
   if(ret==0) {
      pwd->pw_shell = &buffer[bufpos];
      ret = wl_copy_string_to_buffer(pwd_ex->pw_shell, buffer, &bufpos, &bufremain);
   }
   if(ret==0) {
      pwd->pw_passwd = &buffer[bufpos];
      ret = wl_copy_string_to_buffer(pwd_ex->pw_passwd, buffer, &bufpos, &bufremain);
   }
   if(ret==0) {
      pwd->pw_gecos = &buffer[bufpos];
      ret = wl_copy_string_to_buffer(pwd_ex->pw_gecos, buffer, &bufpos, &bufremain);
   }
   result = &pwd;

   pthread_mutex_unlock(&getpwuid_ex_mutex);
   return ret;
}

/****** wingrid/wl_handle_ls_results() **************************************
*  NAME
*     wl_handle_ls_results() -- Parse result of external load sensor
*
*  SYNOPSIS
*     bool wl_handle_ls_results(const char *name, const char *value,
*                               const char *host, char *error_string)
*
*  FUNCTION
*     Parses the result of an external loadsensor. Returns true if the
*     loadsensors reported load, returns false if the loadsensor
*     reported an error.
*
*  RESULT
*     bool - true if load is OK, false if load is an error report.
*
*  NOTES
*     MT-NOTE: wl_handle_ls_results() is MT safe
******************************************************************************/
/* returns true if name/value/host is a loadvalue */
/* error_string should have a size of at least 4 * MAX_STRING_SIZE */
bool wl_handle_ls_results(const char *name, 
                          const char *value, 
                          const char *host,
                          char *error_string) 
{
   bool ret = false; 
   
   if (strcmp(host, "_sge_pseudo_host") == 0) {
      if (strncmp(name, "error", 5) == 0) {
         if (error_string != NULL) {
            sprintf(error_string, "loadsensor report: %s", value);
         }
      }
   } else {
      ret = true;
   }
   return ret;
}

/* ============================================================================
 * SUPERUSER
 * ==========================================================================*/
/****** wingrid/wl_get_superuser_id() ***************************************
*  NAME
*     wl_get_superuser_id() -- return UID of local Administrator
*
*  SYNOPSIS
*     uid_t wl_get_superuser_id()
*
*  FUNCTION
*     Return UID of local Administrator.
*     It is under Windows, unlike Unix, not always 0.
*     Only the local Administrator is comparable to root, the Domain
*     Administrator has only limited abilities.
*
*  RESULT
*     uid_t - the UID of the local Administrator.
*
*  NOTES
*     MT-NOTE: wl_get_superuser_id() is MT safe
******************************************************************************/
uid_t wl_get_superuser_id()
{
   return 197108;
}

gid_t wl_get_superuser_gid()
{
   return 197121;
}

/****** wingrid/wl_get_superuser_name() **************************************
*  NAME
*     wl_get_superuser_name() -- return name of local Administrator
*
*  SYNOPSIS
*     int wl_get_superuser_name(char *buf, int bufsize)
*
*  FUNCTION
*     Return name of local Administrator.
*     The name depends on the language of Windows and could be set manually
*     by the Administrator.
*
*  INPUTS
*     bufsize - sife of buffer that is to receive superuser name.
*
*  OUTPUTS
*     char *buf - Pointer to a buffer that is to receive superuser name.
*
*  RESULT
*     int - 0: OK
*           1: buffer to small for superuser name
*
*  NOTES
*     MT-NOTE: wl_get_superuser_name() is MT safe
******************************************************************************/
int wl_get_superuser_name(char *buf, int bufsize)
{
   struct passwd  pwd, *pwdresult;
   char           buffer[4096];
   int            ret;
   uid_t          uid = wl_get_superuser_id();

   ret = wl_getpwuid_ex_r(uid, &pwd, buffer, sizeof(buffer), &pwdresult, 0);
   if(ret == 0) {
      strlcpy(buf, pwd.pw_name, bufsize);
   }
   return ret;
}

/****** wingrid/wl_is_user_id_superuser()*************************************
*  NAME
*     wl_is_user_id_superuser() -- check if UID is superuser
*                                  (= local Administrator)
*
*  SYNOPSIS
*     bool wl_is_user_id_superuser(int uid)
*
*  FUNCTION
*     Check if UID is superuser (=local Administrator)
*
*  INPUTS
*     uid - the UID that is to be checked.
*
*  RESULT
*     bool - true: UID is superuser, false: UID is not superuser
*
*  NOTES
*     MT-NOTE: wl_is_user_id_superuser() is MT safe
******************************************************************************/
bool wl_is_user_id_superuser(int uid)
{
   /* Only the local Administrator is the 'real' superuser! */
   return (uid==wl_get_superuser_id());
}

/* ============================================================================
 * SETUSER
 * ==========================================================================*/
/****** wingrid/wl_setuser() **********************************************
*  NAME
*     wl_setuser() --  change effective and real uid and gid of process
*
*  SYNOPSIS
*     int wl_setuser(int uid, int gid)
*
*  FUNCTION
*     Change effective and real uid and gid of process
*
*  INPUTS
*     int uid - UID that the process should be changed to.
*     int gid - GID that the process should be changed to.
*
*  RESULTS
*     int - 0 if uid and gid changed correctly, else errno is set and:
*           1 can't read passwd information
*           2
*
*  NOTES
*     MT-NOTE: wl_setuser() is MT safe
******************************************************************************/
int wl_setuser(int uid, int gid, const char *pass, char* err_str)
{
   char   buf[16000];
   int    ret=0;
   int    try=0;
   struct passwd pwd, *ppwd=NULL;

   /* Do this only for domain users */
   if(uid>999999) { 
      /*
       * Get username of UID
       * (Use not MT safe getpwuid_ex() for full qualified name)
       */
      ret = wl_getpwuid_ex_r(uid, &pwd, buf, sizeof(buf), &ppwd, 0); 
      if(ret != 0) {
         sprintf(err_str, MSG_WIN_CANT_GET_PASSWD_INFO, uid);
      } else {
         /* 
          * Read password from file
          */
         if(pwd.pw_name==NULL || 
            (pwd.pw_name!=NULL && pwd.pw_name[0]=='\0')) {
            sprintf(err_str, MSG_WIN_CANT_GET_PASSWD_INFO, uid);
            ret = 1;
         } else {
            ret = setuser(pwd.pw_name, pass, SU_CHECK);
            if(ret != 0) {
               sprintf(err_str, MSG_WIN_CHECK_SETUSER_FAILED, pwd.pw_name,
                       strerror(errno)?strerror(errno):"<NULL>", errno);
            } else {
               do {
                  try++;
                  ret = setuser(pwd.pw_name, pass, SU_COMPLETE);

                  if(ret != 0) {
                     sleep(1);
                  } 
               } while(ret != 0 && try < 5);

               if(ret != 0) {
                  sprintf(err_str, MSG_WIN_SETUSER_FAILED, pwd.pw_name,
                          strerror(errno)?strerror(errno):"<NULL>", errno);
               }
            }
         }
      }
   }
   else
   {
      ret = setgid(gid);
      if(ret != 0) {
         sprintf(err_str, MSG_WIN_CANT_SETGID_IN_SETUSER, gid, 
                 strerror(errno)?strerror(errno):"<NULL>", errno);
      } else {
         ret = setuid(uid);
         if(ret != 0) {
            sprintf(err_str, MSG_WIN_CANT_SETUID_IN_SETUSER, uid,
                    strerror(errno)?strerror(errno):"<NULL>", errno);
         }
      }
   }
   return ret;
}

bool wl_use_sgepasswd()
{
   return wl_enable_windomacc;
}

void wl_set_use_sgepasswd(bool use_it)
{
   wl_enable_windomacc = use_it;
}

/* ============================================================================
 * User Name modifications
 * ==========================================================================*/
/****** wingrid/wl_strip_hostname() ******************************************
*  NAME
*     wl_strip_hostname() -- strip hostname from full qualified username  
*
*  SYNOPSIS
*     char *wl_strip_hostname(char *user_name)
*
*  FUNCTION
*     Strips the hostname from the full qualified username, i.e.
*     returns "username" for "HOST+username".
*
*  INPUTS
*     char *user_name - Pointer to a dynamically allocated buffer that
*                       contains the full qualified username. This buffer
*                       gets freed in this function.
*
*  RESULTS
*     char * - Pointer to a dynamically allocated buffer that contains
*              the stripped user name. This buffer must be freed after usage.
*
*  NOTES
*     MT-NOTE: wl_strip_hostname() is MT-safe.
*
*  SEE ALSO
******************************************************************************/
char *wl_strip_hostname(char *user_name)
{
   char *token;
   char *ret   = strdup(user_name);
   char *lasts = NULL;

   token = strtok_r(user_name, "+", &lasts);
   if (token != NULL) {
      token = strtok_r(NULL, " ", &lasts);
      if (token != NULL) {
         ret = strdup(token);
      }
   }
   free(user_name);
   user_name = NULL;
   return ret;
}

/****** wingrid/wl_build_fq_local_name() *************************************
*  NAME
*     wl_build_fq_local_name() -- build full qualified local user name
*
*  SYNOPSIS
*     int wl_build_fq_local_name(const char *user, char *fq_name)
*
*  FUNCTION
*     Builds a full qualified Interix user name from the given short 
*     user name. A full qualified username is of the form
*     DOMAIN+USER, where DOMAIN is in this case the name of the local host.
*
*  INPUTS
*     const char *user - short name of the user
*
*  OUTPUTS
*     char *fq_name - buffer that is to receive the full qualified name.
*                     It's size must be 1024.
*
*  RESULTS
*     int - 0 if full qualified name was successfully build
*           1 if the user name was not provided in short format
*
*  NOTES
*     MT-NOTE: wl_build_fq_local_name() is MT-safe.
*
*  SEE ALSO
******************************************************************************/
int wl_build_fq_local_name(const char *user, char *fq_name)
{
   char hostname[128];
   int  ret = 1;

   /*
    * do only if user name is not already full qualified
    */
   if(user != NULL && fq_name != NULL && strstr(user, "+")==NULL) {
      gethostname(hostname, 128);
      snprintf(fq_name, 1023, "%s+%s", hostname, user);
      ret = 0;
   }
   return ret;
}

/* ============================================================================
 * REPLACEMENTS of buggy libc functions
 * ==========================================================================*/
/* ----------------------------------------------------------------------------
 * STAT
 * --------------------------------------------------------------------------*/
/****** wingrid/wl_stat() / wingrid/wl_statvfs()***************************
*  NAME
*     wl_stat() / wl_statvfs() - obtain info about files
*
*  SYNOPSIS
*     int wl_stat(const char *path, struct stat *buf)
*     int wl_statvfs(const char *path, struct statvfs *buf)
*
*  FUNCTION
*     Obtain information about the file pointed to by path.
*     This functions are replacements for the libc functions of Interix
*     that don't work on network drives.
*     Limitation of this functions: The process must have the permissions
*     to open the file for reading.
*
*  INPUTS
*     const char     *path - path of the file to obtain info about.
*
*  OUTPUTS
*     struct stat    *buf  - Information bout the file.
*     struct statvfs *buf  - 
*
*  RESULT
*     int - 0 on successful completion, otherwise -1 and errno is set.
*
*  NOTES
*     MT-NOTE: wl_stat() and wl_statvfs are MT safe.
******************************************************************************/
int wl_stat(const char *path, struct stat *buf)
{
   /* stat() doesn't work on NFS drives, have to use fstat() */
   int fd;
   int ret=-1;

   if((fd = open(path, O_RDONLY))!=-1) {
      ret = fstat(fd, buf);
      close(fd);
   }
   return ret;
}

int wl_statvfs(const char *path, struct statvfs *buf)
{
   /* statvfs() doesn't work on NFS drives, have to use fstatvfs() */
   int fd;
   int ret=-1;
            
   if((fd = open(path, O_RDONLY))!=-1) {
      ret = fstatvfs(fd, buf);
      close(fd);
   }
   return ret;
}

/* ----------------------------------------------------------------------------
 * XDR
 * --------------------------------------------------------------------------*/
/****** wingrid/wl_xdrmem_create() ******************************************
*  NAME
*     wl_xdrmem_create() -- Initialize XDR stream
*
*  SYNOPSIS
*     void wl_xdrmem_create(XDR *xdrs, const caddr_t addr,
*                     const uint_t size, const enum xdr_op op)
*
*  FUNCTION
*     Initializes XDR stream for usage.
*
*  INPUTS
*     XDR               *xdrs - XDR stream
*     const caddr_t     addr  - Buffer
*     const uint_t      size  - Size of buffer
*     const enum xdr_op op    - Direction of XDR stream (encode or decode)
*
*  OUTPUTS
*     XDR *xdrs - Initialized XDR stream
*
*  NOTES
*     MT-NOTE: wl_xdrmem_create() is MT safe
******************************************************************************/
void wl_xdrmem_create(XDR *xdrs, const caddr_t addr,
                      const uint_t size, const enum xdr_op op)
{
   if(xdrs!=NULL) {
      xdrs->x_op      = op;
      xdrs->x_ops     = NULL;
      xdrs->x_public  = addr;
      xdrs->x_private = NULL;
      xdrs->x_base    = 0;
      xdrs->x_handy   = 0;
   }
}

/****** wingrid/wl_xdr_destroy() ********************************************
*  NAME
*     wl_xdr_destroy() -- Destroy XDR stream
*
*  SYNOPSIS
*     void wl_xdr_destroy(XDR *xdrs)
*
*  FUNCTION
*     Destroys XDR stream. Doesn't free any buffers!
*
*  INPUTS
*     XDR               *xdrs - XDR stream
*
*  NOTES
*     MT-NOTE: wl_xdr_destroy() is MT safe
******************************************************************************/
void wl_xdr_destroy(XDR *xdrs)
{
}

/****** wingrid/wl_xdr_double() *********************************************
*  NAME
*     wl_xdr_double() -- Convert a double
*
*  SYNOPSIS
*     bool_t wl_xdr_double(XDR *xdrs, double *dp)
*
*  FUNCTION
*     Encodes or decodes a variable of type double.
*
*  INPUTS
*     XDR    *xdrs - If decoding: Stream that contains the encoded value
*     double *dp   - If encoding: Pointer to the variable that is to encode
*
*  OUTPUTS
*     XDR    *xdrs - If encoding: Stream that contains the encoded value
*     double *dp   - If decoding: Pointer to the variable that receives value
*
*  NOTES
*     MT-NOTE: wl_xdr_double() is MT safe
******************************************************************************/
bool_t wl_xdr_double(XDR *xdrs, double *dp)
{
   int i;

   if(xdrs==NULL || dp==NULL) {
      return FALSE;
   }

   if(xdrs->x_op==XDR_ENCODE) 
   {
      for(i=0; i<8; i++) {
         ((char*)xdrs->x_public)[i+xdrs->x_handy] = ((char*)dp)[7-i];
      }
      xdrs->x_handy += 8;
   }
   else if(xdrs->x_op==XDR_DECODE)
   {
      for(i=0; i<8; i++) {
         ((char*)dp)[7-i] = ((char*)xdrs->x_public)[i+xdrs->x_handy];
      }
      xdrs->x_handy += 8;
   }
   return TRUE;  
}

/****** wingrid/wl_xdr_getpos() *********************************************
*  NAME
*     wl_xdr_getpos() -- Get current position in stream
*
*  SYNOPSIS
*     uint_t wl_xdr_getpos(const XDR *xdrs)
*
*  FUNCTION
*     Returns position in XDR stream.
*
*  INPUTS
*     const XDR *xdrs - Stream on which position is to be returned.
*
*  RESULTS
*     uint_t - Current position on stream.
*
*  NOTES
*     MT-NOTE: wl_xdr_getpos() is MT safe
******************************************************************************/
uint_t wl_xdr_getpos(const XDR *xdrs)
{
   return xdrs->x_handy;
}
