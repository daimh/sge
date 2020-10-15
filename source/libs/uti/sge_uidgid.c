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

#include "uti/sge_uidgid.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <pthread.h>

#include "uti/sge_mtutil.h"
#include "uti/sge_rmon.h"
#include "uti/sge_stdio.h"
#include "uti/sge_unistd.h"
#include "uti/sge_log.h"
#include "uti/sge_arch.h"
#include "uti/sge_string.h"
#include "uti/msg_utilib.h"
#include "uti/sge_string.h"
#include "uti/sge_stdio.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_uidgid.h"

#include "msg_common.h"

#if defined( INTERIX )
#   include "wingrid.h"
#   include "../../../utilbin/sge_passwd.h"
#endif

#define UIDGID_LAYER CULL_LAYER 
#define MAX_LINE_LENGTH 10000

enum { SGE_MAX_USERGROUP_BUF = 255 };

typedef struct {
   pthread_mutex_t mutex;  
   const char *user_name;
   uid_t uid;
   gid_t gid;
   int ngroups;
   gid_t *groups;
   bool  initialized;
} admin_user_t;

struct uidgid_state_t {
   uid_t last_uid;
   char  last_username[SGE_MAX_USERGROUP_BUF]; 
   gid_t last_gid;
   char  last_groupname[SGE_MAX_USERGROUP_BUF]; 
};

static admin_user_t admin_user = {PTHREAD_MUTEX_INITIALIZER, NULL, (uid_t)-1, (gid_t)-1, false};

static pthread_once_t uidgid_once = PTHREAD_ONCE_INIT;
static pthread_key_t  uidgid_state_key;

static void uidgid_once_init(void);
static void uidgid_state_destroy(void* theState);
static void uidgid_state_init(struct uidgid_state_t* theState);

static void set_admin_user(const char *user_name, uid_t, gid_t);
static int  get_admin_user(uid_t*, gid_t*, int*, gid_t**);

static uid_t       uidgid_state_get_last_uid(void);
static const char* uidgid_state_get_last_username(void);
static gid_t       uidgid_state_get_last_gid(void);
static const char* uidgid_state_get_last_groupname(void);

static void uidgid_state_set_last_uid(uid_t uid);
static void uidgid_state_set_last_username(const char *user);
static void uidgid_state_set_last_gid(gid_t gid);
static void uidgid_state_set_last_groupname(const char *group);

static int get_file_line_size(FILE* fp); 

/****** uti/uidgid/uidgid_mt_init() ************************************************
*  NAME
*     uidgid_mt_init() -- Initialize user and group oriented functions for multi
*                         threading use.
*
*  SYNOPSIS
*     void uidgid_mt_init(void) 
*
*  FUNCTION
*     Set up user and group oriented functions. This function must be called at
*     least once before any of the user and group functions can be used. This
*     function is idempotent, i.e. it is safe to call it multiple times.
*
*     Thread local storage for the user and group state information is reserved. 
*
*  INPUTS
*     void - NONE 
*
*  RESULT
*     void - NONE
*
*  NOTES
*     MT-NOTE: uidgid_mt_init() is MT safe 
*
*******************************************************************************/
void uidgid_mt_init(void)
{
   pthread_once(&uidgid_once, uidgid_once_init);
}

/****** uti/uidgid/sge_is_start_user_superuser() *******************************
*  NAME
*     sge_is_start_user_superuser() -- return true/false is current real user
*                                     is superuser (root/Administrator)
*
*  SYNOPSIS
*     bool sge_is_start_user_superuser(void)
*
*  FUNCTION
*     Check the real user id to determine if it is the superuser. If so, return
*     true, else return false. This function relies on getuid == 0 for UNIX.
*     On INTERIX, this function determines if the user is the built-in local
*     admin user or the domain administrator.
*     Other members of the Administrators group do not have the permission
*     to "su" without password!
*
*  INPUTS
*     NONE
*
*  RESULT
*         true - root was start user
*         false - otherwise
*
*  NOTES
*     MT-NOTE: sge_is_start_user_superuser() is MT safe.
* 
*  SEE ALSO
*     uti/uidgid/sge_switch2admin_user()
*     uti/uidgid/sge_set_admin_username()
*     uti/uidgid/sge_switch2start_user()
*     uti/uidgid/sge_run_as_user()
******************************************************************************/
bool sge_is_start_user_superuser(void)
{
   bool   is_root = false;
   uid_t  start_uid; 

   DENTER(UIDGID_LAYER, "sge_is_start_user_superuser");

   start_uid = getuid();
   is_root = (start_uid == SGE_SUPERUSER_UID)?true:false;

   DEXIT;
   return is_root;
} /* sge_is_start_user_superuser() */           

/****** uti/uidgid/sge_set_admin_username() ***********************************
*  NAME
*     sge_set_admin_username() -- Set SGE/EE admin user
*
*  SYNOPSIS
*     int sge_set_admin_username(const char *user, char *err_str, size_t lstr)
*
*  FUNCTION
*     Set SGE/EE admin user. If 'user' is "none" then use the current
*     uid/gid. Ignore if current user is not root.
*
*  INPUTS
*     const char *user - admin user name
*     char *err_str    - error message
*     size_t lstr      - size of err_str
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Username does not exist
*        -2 - Admin user was already set
*
*  NOTES
*     MT-NOTE: sge_set_admin_username() is MT safe.
* 
*  SEE ALSO
*     uti/uidgid/sge_switch2admin_user()
*     uti/uidgid/sge_set_admin_username()
*     uti/uidgid/sge_switch2start_user()
*     uti/uidgid/sge_run_as_user()
******************************************************************************/
int sge_set_admin_username(const char *user, char *err_str, size_t lstr)
{
   struct passwd *admin;
   int ret, ngroups;
   uid_t uid;
   gid_t gid, *groups;
#if defined( INTERIX )
   char fq_name[1024];
#endif

   DENTER(UIDGID_LAYER, "sge_set_admin_username");

#if defined( INTERIX ) 
   /* For Interix: Construct full qualified admin user name.
    * As admin user is always local, use hostname as domain name.
    * If admin user is "none", it is a special case and 
    * full qualified name must not be constructed!
    */
   if(strcasecmp(user, "none") != 0) {
      wl_build_fq_local_name(user, fq_name);
      user = fq_name;
   }
#endif

   /*
    * Do only if admin user is not already set!
    */
   if (get_admin_user(&uid, &gid, &ngroups, &groups) != ESRCH) {
      DEXIT;
      return -2;
   }
   if (!user || user[0] == '\0') {
      if (err_str) {
         snprintf(err_str, lstr, SFNMAX, MSG_POINTER_SETADMINUSERNAMEFAILED);
      }
      DEXIT;
      return -1;
   }
 
   ret = 0;
   if (!strcasecmp(user, "none")) {
      set_admin_user("root", getuid(), getgid());
   } else {
      struct passwd pw_struct;
      int size = get_pw_buffer_size();
      char *buffer = sge_malloc(size);

      admin = sge_getpwnam_r(user, &pw_struct, buffer, size);
      if (admin) {
         set_admin_user(user, admin->pw_uid, admin->pw_gid);
      } else {
         if (err_str)
            snprintf(err_str, lstr, MSG_SYSTEM_ADMINUSERNOTEXIST_S, user);
         ret = -1;
      }
      sge_free(&buffer);
   }
   DEXIT;
   return ret;
} /* sge_set_admin_username() */           

/****** uti/uidgid/sge_is_admin_user() ****************************************
*  NAME
*     sge_is_admin_user() -- Check if user is SGE admin user
*
*  SYNOPSIS
*     bool sge_is_admin_user(const char *username)
*
*  FUNCTION
*     Checks if the given user is the SGE admin user.
*
*  INPUTS
*     const char *username - given user name
*
*  RESULT
*     bool - true if the given user is the SGE admin user
*            false if not.
*
*  NOTES
*     MT-NOTE: sge_is_admin_user() is MT safe.
* 
*  SEE ALSO
******************************************************************************/
bool sge_is_admin_user(const char *username)
{
   bool       ret = false;
   const char *admin_user;
#ifdef INTERIX
   char       fq_name[1024];
#endif

   admin_user = bootstrap_get_admin_user();
   if(admin_user != NULL && username != NULL) {
#ifdef INTERIX
      /* For Interix: Construct full qualified admin user name.
       * As admin user is always local, use hostname as domain name.
       * If admin user is "none", it is a special case and
       * full qualified name must not be constructed!
       */
      wl_build_fq_local_name(admin_user, fq_name);
      admin_user = fq_name;
#endif
      ret = strcmp(username, admin_user)==0 ? true : false; 
   }

   return ret;
} /* sge_is_admin_user() */

/****** uti/uidgid/sge_switch2admin_user() ************************************
*  NAME
*     sge_switch2admin_user() -- Set euid/egid to admin uid/gid
*
*  SYNOPSIS
*     int sge_switch2admin_user(void)
*
*  FUNCTION
*     Set euid/egid to admin uid/gid. Silently ignore if our uid
*     is not root. Do nothing if out euid/egid is already the admin
*     uid/gid. If the admin user was not set with
*     sge_set_admin_username() the function will not return.
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - setegid()/seteuid() fails
*
*  NOTES
*     MT-NOTE: sge_switch2admin_user() is MT safe.
*
*  SEE ALSO
*     uti/uidgid/sge_switch2admin_user()
*     uti/uidgid/sge_set_admin_username()
*     uti/uidgid/sge_switch2start_user()
*     uti/uidgid/sge_run_as_user()
******************************************************************************/
int sge_switch2admin_user(void)
{
   uid_t uid;
   gid_t gid, *groups;
   int ret = 0;
   int ngroups;

   DENTER(UIDGID_LAYER, "sge_switch2admin_user");
#if !defined(INTERIX)
   /*
    * On Windows Vista (and probably later versions) we can't set the effective
    * user ID to somebody else during boot time, because the local Administrator
    * doesn't have his primary group set before booting finished.
    * This problem occurs solely when the execd is started by a RC script
    * during boot time.
    * But we don't need to switch to the SGE admin user anyway, as spooling
    * always has to be done locally, so we can just skip it always.
    */
   if (get_admin_user(&uid, &gid, &ngroups, &groups) == ESRCH) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_SWITCH_USER_NOT_INITIALIZED));
      abort();
   }

   if (!sge_is_start_user_superuser()) {
      DPRINTF(("%s", MSG_SWITCH_USER_NOT_ROOT));
      ret = 0;
      goto exit;
   } else {
      if (getegid() != gid) {
         if (setgroups(ngroups, groups) != 0 || setegid(gid) == -1) {
            DTRACE;
            ret = -1;
            goto exit;
         } 
      }
 
      if (geteuid() != uid) {
         if (sge_seteuid(uid) == -1) {
            DTRACE;
            ret = -1;
            goto exit;
         } 
      }
   }

exit:
#endif
   DPRINTF(("uid=%ld; gid=%ld; euid=%ld; egid=%ld auid=%ld; agid=%ld\n", 
            (long)getuid(), (long)getgid(), 
            (long)geteuid(), (long)getegid(),
            (long)uid, (long)gid));
   DEXIT;
   return ret;
} /* sge_switch_2admin_user() */

/****** uti/uidgid/sge_switch2start_user() ************************************
*  NAME
*     sge_switch2start_user() -- set euid/egid to start uid/gid
*
*  SYNOPSIS
*     int sge_switch2start_user(void)
*
*  FUNCTION
*     Set euid/egid to the uid/gid of that user which started the
*     application which calls this function. If our euid/egid is
*     already the start uid/gid don't do anything. If the admin user
*     was not set with sge_set_admin_username() the function will
*     not return.
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - setegid()/seteuid() fails
*
*  NOTES
*     MT-NOTE: sge_switch2start_user() is MT safe.
*
*  SEE ALSO
*     uti/uidgid/sge_switch2admin_user()
*     uti/uidgid/sge_set_admin_username()
*     uti/uidgid/sge_switch2start_user()
*     uti/uidgid/sge_run_as_user()
******************************************************************************/
int sge_switch2start_user(void)
{
#if !defined(INTERIX)
   uid_t uid, start_uid;
   gid_t gid, start_gid;
#endif
   gid_t *groups;
   int ret = 0, ngroups;

   DENTER(UIDGID_LAYER, "sge_switch2start_user");
#if !defined(INTERIX)
   /*
    * On Windows Vista (and probably later versions) we can't set the effective
    * user ID to somebody else during boot time, because the local Administrator
    * doesn't have his primary group set before booting finished.
    * This problem occurs solely when the execd is started by a RC script
    * during boot time.
    * But we don't need to switch to the SGE admin user anyway, as spooling
    * always has to be done locally, so we can just skip it always.
    */
 
   if (get_admin_user(&uid, &gid, &ngroups, &groups) == ESRCH) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_SWITCH_USER_NOT_INITIALIZED));
      abort();
   }

   start_uid = getuid();
   start_gid = getgid();

   if(!sge_is_start_user_superuser()) {
      DPRINTF(("%s", MSG_SWITCH_USER_NOT_ROOT));
      ret = 0;
      goto exit;
   } else {
      if (start_gid != getegid()) {
         if (setegid(start_gid) == -1) {
            DTRACE;
            ret = -1;
            goto exit;
         } 
      }
      if (start_uid != geteuid()) {
         if (sge_seteuid(start_uid) == -1) {
            DTRACE;
            ret = -1;
            goto exit;
         } 
      }
   }

exit:
   DPRINTF(("uid=%ld; gid=%ld; euid=%ld; egid=%ld auid=%ld; agid=%ld\n", 
            (long)getuid(), (long)getgid(), 
            (long)geteuid(), (long)getegid(),
            (long)uid, (long)gid));
#endif
   DEXIT;
   return ret;
} /* sge_switch2start_user() */ 

#if unused
/****** uti/uidgid/sge_run_as_user() ******************************************
*  NAME
*     sge_run_as_user() -- Set euid to uid
*
*  SYNOPSIS
*     int sge_run_as_user(void)
*
*  FUNCTION
*     Set euid to uid
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - setegid()/seteuid() failed
*
*  NOTES
*     MT-NOTE: sge_run_as_user() is MT safe
*
*  SEE ALSO
*     uti/uidgid/sge_switch2admin_user()
*     uti/uidgid/sge_set_admin_username()
*     uti/uidgid/sge_switch2start_user()
*     uti/uidgid/sge_run_as_user()
******************************************************************************/
int sge_run_as_user(void)
{
   int ret = 0;
 
   DENTER(UIDGID_LAYER, "sge_run_as_user");

   if(geteuid() != getuid()) {
      if (sge_seteuid(getuid())) {
         ret = -1;
      }
   }
 
   DEXIT;
   return ret;
} /* sge_run_as_user() */
#endif

/****** uti/uidgid/sge_user2uid() *********************************************
*  NAME
*     sge_user2uid() -- resolves user name to uid and gid 
*
*  SYNOPSIS
*     int sge_user2uid(const char *user, uid_t *puid, gid_t *pgid, int retries) 
*
*  FUNCTION
*     Resolves a username ('user') to it's uid (stored in 'puid') and
*     it's primary gid (stored in 'pgid').
*     'retries' defines the number of (e.g. NIS/DNS) retries.
*     If 'puid' is NULL the user name is resolved without saving it.
*
*  INPUTS
*     const char *user - username 
*     uid_t *puid      - uid pointer 
*     gid_t *pgid      - gid pointer
*     int retries      - number of retries 
*
*  NOTES
*     MT-NOTE: sge_user2uid() is MT safe.
*
*  RESULT
*     int - exit state 
*         0 - OK
*         1 - Error
******************************************************************************/
int sge_user2uid(const char *user, uid_t *puid, uid_t *pgid, int retries) 
{
   struct passwd *pw;
   struct passwd pwentry;
   char *buffer;
   int size;

   DENTER(UIDGID_LAYER, "sge_user2uid");

   size = get_pw_buffer_size();
   buffer = sge_malloc(size);

   do {
      DPRINTF(("name: %s retries: %d\n", user, retries));

      if (!retries--) {
         sge_free(&buffer);
         DEXIT;
         return 1;
      }
      if (getpwnam_r(user, &pwentry, buffer, size, &pw) != 0) {
         pw = NULL;
      }
   } while (pw == NULL);

   if(puid) {
      *puid = pw->pw_uid;
   }
   if(pgid) {
      *pgid = pw->pw_gid;
   }

   sge_free(&buffer);
   DEXIT; 
   return 0;
} /* sge_user2uid() */

/****** uti/uidgid/sge_group2gid() ********************************************
*  NAME
*     sge_group2gid() -- Resolve a group name to its gid 
*
*  SYNOPSIS
*     int sge_group2gid(const char *gname, gid_t *gidp, int retries) 
*
*  FUNCTION
*     Resolves a groupname ('gname') to its gid (stored in 'gidp').
*     'retries' defines the number of (e.g. NIS/DNS) retries.
*     If 'gidp' is NULL the group name is resolved without saving it.
*
*  INPUTS
*     const char *gname - group name 
*     gid_t *gidp       - gid pointer 
*     int retries       - number of retries  
*
*  NOTES
*     MT-NOTE: sge_group2gid() is MT safe.
*
*  RESULT
*     int - exit state 
*         0 - OK
*         1 - Error
******************************************************************************/
int sge_group2gid(const char *gname, gid_t *gidp, int retries) 
{
   struct group *gr;
   struct group grentry;
   char *buffer;
   int size;

   DENTER(UIDGID_LAYER, "sge_group2gid");

   size = get_group_buffer_size();
   buffer = sge_malloc(size);

   do {
      if (!retries--) {
         sge_free(&buffer);
         DEXIT;
         return 1;
      }
#if defined(INTERIX)
      if (getgrnam_nomembers_r(gname, &grentry, buffer, size, &gr) != 0)
#else
      if (getgrnam_r(gname, &grentry, buffer, size, &gr) != 0)
#endif
      {
         if (errno == ERANGE) {
            retries++;
            size += 1024;
            buffer = sge_realloc(buffer, size, 1);
         }
         gr = NULL;
      }
   } while (gr == NULL);
   
   if (gidp) {
      *gidp = gr->gr_gid;
   }

   sge_free(&buffer);
   DEXIT; 
   return 0;
} /* sge_group2gid() */

/****** uti/uidgid/sge_uid2user() *********************************************
*  NAME
*     sge_uid2user() -- Resolves uid to user name. 
*
*  SYNOPSIS
*     int sge_uid2user(uid_t uid, char *dst, size_t sz, int retries) 
*
*  FUNCTION
*     Resolves uid to user name. if 'dst' is NULL the function checks
*     only if the uid is resolvable. 
*
*  INPUTS
*     uid_t uid   - user id 
*     char *dst   - buffer for the username 
*     size_t sz   - buffer size
*     int retries - number of retries 
*
*  NOTES
*     MT-NOTE: sge_uid2user() is MT safe.
*
*  RESULT
*     int - error state
*         0 - OK
*         1 - Error
******************************************************************************/
int sge_uid2user(uid_t uid, char *dst, size_t sz, int retries)
{
   const char *last_username;

   DENTER(UIDGID_LAYER, "sge_uid2user");

   last_username = uidgid_state_get_last_username();

   if (!last_username[0] || (uidgid_state_get_last_uid() != uid)) {
      struct passwd *pw;
      struct passwd pwentry;
      int size;
      char *buffer;

      size = get_pw_buffer_size();
      buffer = sge_malloc(size);
      
      /* max retries that are made resolving user name */
      while (getpwuid_r(uid, &pwentry, buffer, size, &pw) != 0 || !pw) {
         if (!retries--) {
            ERROR((SGE_EVENT, MSG_SYSTEM_GETPWUIDFAILED_US, 
                  sge_u32c(uid), strerror(errno)));
            sge_free(&buffer);
            DEXIT;
            return 1;
         }
         sleep(1);
      }
      /* cache user name */
      uidgid_state_set_last_username(pw->pw_name);
      uidgid_state_set_last_uid(uid);

      sge_free(&buffer);
   }

   if (dst) {
      sge_strlcpy(dst, uidgid_state_get_last_username(), sz);
   }

   DEXIT; 
   return 0;
} /* sge_uid2user() */

/****** uti/uidgid/sge_gid2group() ********************************************
*  NAME
*     sge_gid2group() -- Resolves gid to group name. 
*
*  SYNOPSIS
*     int sge_gid2group(gid_t gid, char *dst, size_t sz, int retries) 
*
*  FUNCTION
*     Resolves gid to group name. if 'dst' is NULL the function checks
*     only if the gid is resolvable. 
*
*  INPUTS
*     uid_t gid   - group id 
*     char *dst   - buffer for the group name 
*     size_t sz   - buffer size
*     int retries - number of retries 
*
*  NOTES
*     MT-NOTE: sge_gid2group() is MT safe.
*
*  RESULT
*     int - error state
*         0 - OK
*         1 - Error
******************************************************************************/
int sge_gid2group(gid_t gid, char *dst, size_t sz, int retries)
{
   struct group *gr;
   struct group grentry;
   const char *last_groupname;

   DENTER(UIDGID_LAYER, "sge_gid2group");

   last_groupname = uidgid_state_get_last_groupname();

   if (!last_groupname[0] || uidgid_state_get_last_gid() != gid) {
      int size = get_group_buffer_size();
      char *buf = sge_malloc(size);
      char **pbuf = &buf;

      gr = sge_getgrgid_r(gid, &grentry, pbuf, size, retries);
      /* Bugfix: Issuezilla 1256
       * We need to handle the case when the OS is unable to resolve the GID to
       * a name. [DT] */
      if (gr == NULL) {
         sge_free(pbuf);
         DEXIT;
         return 1;
      }
      
      /* cache group name */
      uidgid_state_set_last_groupname(gr->gr_name);
      uidgid_state_set_last_gid(gid);

      sge_free(&buf);
   }
   
   if (dst != NULL) {
      sge_strlcpy(dst, uidgid_state_get_last_groupname(), sz);
   }

   DEXIT; 
   return 0;
} /* sge_gid2group() */



int _sge_gid2group(gid_t gid, gid_t *last_gid, char **groupnamep, int retries)
{
   struct group *gr;
   struct group grentry;

   DENTER(TOP_LAYER, "_sge_gid2group");

   if (!groupnamep || !last_gid) {
      DEXIT;
      return 1;
   }

   if ( !(*groupnamep) || *last_gid != gid) {
      char *buf = NULL;
      int size = 0;
      
      size = get_group_buffer_size();
      buf = sge_malloc(size);
      
     /* max retries that are made resolving group name */
#if defined (INTERIX)
      while (getgrgid_nomembers_r(gid, &grentry, buf, size, &gr) != 0)
#else
      while (getgrgid_r(gid, &grentry, buf, size, &gr) != 0)
#endif
      {
         if (!retries--) {
            sge_free(&buf);
            
            DEXIT;
            return 1;
         }
         
         sleep(1);
      }
      
      /* Bugfix: Issuezilla 1256
       * We need to handle the case when the OS is unable to resolve the GID to
       * a name. [DT] */
      if (gr == NULL) {
         sge_free(&buf);
         DEXIT;
         return 1;
      }
      
      /* cache group name */
      *groupnamep = sge_strdup(*groupnamep, gr->gr_name);
      *last_gid = gid;

      sge_free(&buf);
   }
   
   DEXIT; 
   return 0;
} /* _sge_gid2group() */

/****** uti/uidgid/get_pw_buffer_size() ****************************************
*  NAME
*     get_pw_buffer_size() -- get the buffer size required for getpw*_r
*
*  SYNOPSIS
*     int get_pw_buffer_size(void) 
*
*  FUNCTION
*     Returns the buffer size required for functions like getpwnam_r.
*     It can either be retrieved via sysconf, or a big (20k) buffer
*     size is taken.
*
*  RESULT
*     int - buffer size in bytes
*
*  NOTES
*     MT-NOTE: get_pw_buffer_size() is MT safe 
*
*  SEE ALSO
*     uti/uidgid/get_group_buffer_size()
*******************************************************************************/
int get_pw_buffer_size(void)
{
   enum { buf_size = 20480 };  /* default is 20 KB */
   
   int sz = buf_size;

#ifdef _SC_GETPW_R_SIZE_MAX
   if ((sz = (int)sysconf(_SC_GETPW_R_SIZE_MAX)) == -1) {
      sz = buf_size;
   } else {
      sz = MAX(sz, buf_size);
   }
#endif

   return sz;
}

/****** uti/uidgid/get_group_buffer_size() ****************************************
*  NAME
*     get_group_buffer_size() -- get the buffer size required for getgr*_r
*
*  SYNOPSIS
*     int get_group_buffer_size(void) 
*
*  FUNCTION
*     Returns the buffer size required for functions like getgrnam_r.
*     It can either be retrieved via sysconf, or a big (20k) buffer
*     size is taken.
*
*  RESULT
*     int - buffer size in bytes
*
*  NOTES
*     MT-NOTE: get_group_buffer_size() is MT safe 
*
*  SEE ALSO
*     uti/uidgid/get_pw_buffer_size()
*******************************************************************************/
int get_group_buffer_size(void)
{
   enum { buf_size = 20480 };  /* default is 20 KB */
   
   int sz = buf_size;

#ifdef _SC_GETGR_R_SIZE_MAX
   if ((sz = (int)sysconf(_SC_GETGR_R_SIZE_MAX)) == -1) {
      sz = buf_size;
   } else {
      sz = MAX(sz, buf_size);
   }
#endif

   return sz;
}

/****** uti/uidgid/sge_set_uid_gid_addgrp() ***********************************
*  NAME
*     sge_set_uid_gid_addgrp() -- Set uid and gid of calling process
*
*  SYNOPSIS
*     int sge_set_uid_gid_addgrp(const char *user, 
*                                const char *intermediate_user,
*                                int min_gid, int min_uid, int add_grp,
*                                char *err_str, int use_qsub_gid, 
*                                gid_t qsub_gid)
*
*  FUNCTION
*     Set uid and gid of calling process. This can be done only by root.
*
*  INPUTS
*     const char *user              - ???
*     const char *intermediate_user - ???
*     int min_gid                   - ???
*     int min_uid                   - ???
*     int add_grp                   - ???
*     char *err_str                 - ???
*     int use_qsub_gid              - ???
*     gid_t qsub_gid                - ???
*     bool skip_silently            - skip silently when add_grp could not 
*                                     be added due to NGROUP_MAX limit
*
*  NOTES
*     MT-NOTE: sge_set_uid_gid_addgrp() is MT safe
*
*     TODO: This function needs to be rewritten from scratch! It calls
*     'initgroups()' which is not part of POSIX. The call to 'initgroups()'
*     shall be replaced by a combination of 'getgroups()/getegid()/setgid()'.
*      
*     This function is used by 'shepherd' only anyway. Hence it shall be
*     considered to move it from 'libuti' to 'shepherd'.
* 
*  RESULT
*     int - error state
*         0 - OK
*        -1 - we can't switch to user since we are not root
*         1 - we can't switch to user or we can't set add_grp
*         2 - can't read sgepasswd file
*         3 - no password entry in sgepasswd file
*         4 - switch to user failed, likely wrong password for this user
******************************************************************************/
static int _sge_set_uid_gid_addgrp(const char *user, const char *intermediate_user,
                            int min_gid, int min_uid, int add_grp,
                            char *err_str, size_t lstr,
                            int use_qsub_gid, gid_t qsub_gid,
                            char *buffer, int size, bool skip_silently);

int sge_set_uid_gid_addgrp(const char *user, const char *intermediate_user,
                           int min_gid, int min_uid, int add_grp,
                           char *err_str, size_t lstr,
                           int use_qsub_gid, gid_t qsub_gid, bool skip_silently)
{
   int ret;
   char *buffer;
   int size;

   size = get_pw_buffer_size();
   buffer = sge_malloc(size);

   ret = _sge_set_uid_gid_addgrp(user, intermediate_user, min_gid, min_uid, add_grp, 
                                 err_str, lstr, use_qsub_gid, qsub_gid,
                                 buffer, size, skip_silently);

   sge_free(&buffer);
   return ret;
}

static int _sge_set_uid_gid_addgrp(const char *user, const char *intermediate_user,
                            int min_gid, int min_uid, int add_grp,
                            char *err_str, size_t lstr,
                            int use_qsub_gid, gid_t qsub_gid,
                            char *buffer, int size, bool skip_silently)
{
#if !(defined(WIN32) || defined(INTERIX)) /* var not needed */
   int status;
#endif
   struct passwd *pw;
   struct passwd pw_struct;
   gid_t old_grp_id;
 
   sge_switch2start_user();
 
   if (!sge_is_start_user_superuser()) {
      snprintf(err_str, lstr, SFNMAX, MSG_SYSTEM_CHANGEUIDORGIDFAILED);
      return -1;
   }
 
   if (intermediate_user) {
      user = intermediate_user;            
   }

   if (!(pw = sge_getpwnam_r(user, &pw_struct, buffer, size))) {
      snprintf(err_str, lstr, MSG_SYSTEM_GETPWNAMFAILED_S , user);
      return 1;
   }

   /*
    * preserve the old primary gid for initgroups()
    * see cr 6590010
    */
   old_grp_id = pw->pw_gid;

   /*
    *  Should we use the primary group of qsub host? (qsub_gid)
    */
   if (use_qsub_gid) {
      pw->pw_gid = qsub_gid;
   }
 
#if !defined(INTERIX)
   if (!intermediate_user) {
      errno = 0;
      /*
       *  It should not be necessary to set min_gid/min_uid to 0
       *  for being able to run prolog/epilog/pe_start/pe_stop
       *  as root
       */
      if (pw->pw_gid < min_gid) {
        snprintf(err_str, lstr, MSG_SYSTEM_GIDLESSTHANMINIMUM_SUI ,
                 user, sge_u32c( pw->pw_gid), min_gid);
         return 1;
      }
      if (sge_setgid(pw->pw_gid)) {
         snprintf(err_str, lstr, MSG_SYSTEM_SETGIDFAILED_US,
                  sge_u32c(pw->pw_gid), strerror(errno));
         return 1;
      }
   } else {
      if (setegid(pw->pw_gid)) {
         snprintf(err_str, lstr, MSG_SYSTEM_SETEGIDFAILED_US,
                  sge_u32c(pw->pw_gid), strerror(errno));
         return 1;
      }
   }
#endif

#if !(defined(WIN32) || defined(INTERIX)) /* initgroups not called */
   status = initgroups(pw->pw_name, old_grp_id);
 
   /* Why am I doing it this way?  Good question,
      an even better question would be why vendors
      can't get their act together on what is returned,
      at least get it right in the man pages!
      on error here's what I get:
      (subject to change with OS releases)
      OS      return       errno
      AIX     1            1
      ULTRIX  1            1
      OSF/1   1            1
      IRIX   -1            1   (returns #groups if successful)
      SUNOS  -1            1
      SOLARIS-1
      UGH!!!
    */      

#if defined(SVR3) || defined(__sun__)
   if (status < 0) {
      snprintf(err_str, lstr, MSG_SYSTEM_INITGROUPSFAILED_I, status);
      return 1;
   }
#else
   if (status) {
      snprintf(err_str, lstr, MSG_SYSTEM_INITGROUPSFAILED_I, status);
      return 1;
   }
#endif
#endif /* WIN32 */
 
#if defined(SOLARIS) || defined(ALPHA) || defined(LINUX) || defined(FREEBSD) || defined(DARWIN)
   /* add Additional group id to current list of groups */
   if (add_grp) {
      if (sge_add_group(add_grp, err_str, lstr, skip_silently) == -1) {
         return 5;
      }
   }
#endif
 
   if (!intermediate_user) {
      if (pw->pw_uid < min_uid) {
         snprintf(err_str, lstr, MSG_SYSTEM_UIDLESSTHANMINIMUM_SUI,
                 user, sge_u32c(pw->pw_uid), min_uid);
         return 1;
      }

#if defined( INTERIX )
      /*
       * Do only if windomacc=true is set and user is not the superuser.
       * For non-interactive jobs, user shouldn't be the superuser.
       * For qrsh, user usually is the superuser.
       */
      if (wl_use_sgepasswd()==true 
          && wl_is_user_id_superuser(pw->pw_uid)==false) {
         char *pass = NULL;
         char buf[1000] = "\0";
         int  res;

         err_str[0] = '\0';
         res = uidgid_read_passwd(pw->pw_name, &pass, err_str, lstr);

         if(res != 0) {
            /* map uidgid_read_passwd() return value to
             * sge_set_uid_gid_addgrp() return value.
             */
            res++;
            sge_free(&pass);
            return res;
         }

         if(wl_setuser(pw->pw_uid, pw->pw_gid, pass, err_str) != 0) {
            sge_free(&pass);
            snprintf(buf, sizeof(buf), MSG_SYSTEM_SETUSERFAILED_UU,
                     sge_u32c(pw->pw_uid), sge_u32c(pw->pw_gid));
            sge_strlcat(err_str, buf, sizeof(err_str));
            return 4;
         }
         sge_free(&pass);
      }
      else
#endif
      {
         errno = 0;
         if (use_qsub_gid) {
            if (sge_setgid(pw->pw_gid)) {
               snprintf(err_str, lstr, MSG_SYSTEM_SETGIDFAILED_US,
                        sge_u32c(pw->pw_gid), strerror(errno));
               return 1;
            }
         }
         if (sge_setuid(pw->pw_uid)) {
            snprintf(err_str, lstr, MSG_SYSTEM_SETUIDFAILED_US,
                     sge_u32c(pw->pw_uid), strerror(errno));
            return 1;
         }
      }
   } else {
      errno = 0;
      if (use_qsub_gid) {
         if (sge_setegid(pw->pw_gid)) {
            snprintf(err_str, lstr, MSG_SYSTEM_SETEGIDFAILED_US,
                     sge_u32c(pw->pw_gid), strerror(errno));
            return 1;
         }
      }

      if (sge_seteuid(pw->pw_uid)) {
         snprintf(err_str, lstr, MSG_SYSTEM_SETEUIDFAILED_US,
                  sge_u32c(pw->pw_uid), strerror(errno));
         return 1;
      }
   }
 
   return 0;
} /* _sge_set_uid_gid_addgrp() */ 

/****** uti/uidgid/sge_add_group() ********************************************
*  NAME
*     sge_add_group() -- Add a gid to the list of additional group ids
*
*  SYNOPSIS
*     int sge_add_group(gid_t add_grp_id, char *err_str)
*
*  FUNCTION
*     Add a gid to the list of additional group ids. If 'add_grp_id' 
*     is 0 don't add value to group id list (but return successfully).
*     If an error occurs, a descriptive string will be written to 
*     err_str.
*
*  INPUTS
*     gid_t add_grp_id   - new gid
*     char *err_str      - if points to a valid string buffer
*                          error descriptions 
*                          will be written here
*     size_t lstr        - size of err_str
*     bool skip_silently - skip silently if setting the group is skipped
*                          because this would exceed the _SC_NGROUPS_MAX
*                          limit.
*
*  NOTE
*     MT-NOTE: sge_add_group() is MT safe
*
*  RESULT
*     int - error state
*         0 - Success
*        -1 - Error
******************************************************************************/
int sge_add_group(gid_t add_grp_id, char *err_str, size_t lstr, bool skip_silently)
{
   u_long32 max_groups;
   gid_t *list;
   int groups;

   if(err_str != NULL) {
      err_str[0] = 0;
   }

   if (add_grp_id == 0) {
      return 0;
   }

   max_groups = sysconf(_SC_NGROUPS_MAX);
   if (max_groups <= 0) {
      if(err_str != NULL) {
         snprintf(err_str, lstr, MSG_SYSTEM_ADDGROUPIDFORSGEFAILED_UUS,
                  sge_u32c(getuid()), sge_u32c(geteuid()),
                  MSG_SYSTEM_INVALID_NGROUPS_MAX);
      }
      return -1;
   }
 
/*
 * INSURE detects a WRITE_OVERFLOW when getgroups was invoked (LINUX).
 * Is this a bug in the kernel or in INSURE?
 */
#if __linux__
   list = (gid_t*) malloc(2*max_groups*sizeof(gid_t));
#else
   list = (gid_t*) malloc(max_groups*sizeof(gid_t));
#endif
   if (list == NULL) {
      if(err_str != NULL) {
         int error = errno;
         snprintf(err_str, lstr, MSG_SYSTEM_ADDGROUPIDFORSGEFAILED_UUS,
                  sge_u32c(getuid()), sge_u32c(geteuid()), strerror(error));
      }
      return -1;
   }
 
   groups = getgroups(max_groups, list);
   if (groups == -1) {
      if(err_str != NULL) {
         int error = errno;
         snprintf(err_str, lstr, MSG_SYSTEM_ADDGROUPIDFORSGEFAILED_UUS,
                  sge_u32c(getuid()), sge_u32c(geteuid()), strerror(error));
      }
      sge_free(&list);
      return -1;
   }   
#if !defined(INTERIX)

   if (groups < max_groups) {
      list[groups] = add_grp_id;
      groups++;
      groups = setgroups(groups, list);
      if (groups == -1) {
         if (err_str != NULL) {
            int error = errno;
            snprintf(err_str, lstr, MSG_SYSTEM_ADDGROUPIDFORSGEFAILED_UUS,
                     sge_u32c(getuid()), sge_u32c(geteuid()), strerror(error));
         }
         sge_free(&list);
         return -1;
      }
   } else if (skip_silently == false) {
      if (err_str != NULL) {
         snprintf(err_str, lstr, MSG_SYSTEM_ADDGROUPIDFORSGEFAILED_UUS,
                  sge_u32c(getuid()), sge_u32c(geteuid()),
                  MSG_SYSTEM_USER_HAS_TOO_MANY_GIDS);
      }
      sge_free(&list);
      return -1;
   } else {
      sge_free(&list);
      return 0; 
   } 
#endif
   sge_free(&list);
   return 0;
}  

/****** uti/uidgid/sge_getpwnam_r() ********************************************
*  NAME
*     sge_getpwnam_r() -- Return password file entry for a given user name. 
*
*  SYNOPSIS
*     struct passwd* sge_getpwnam_r(const char*, struct passwd*, char*, int) 
*
*  FUNCTION
*     Search user database for a name. This function is just a wrapper for
*     'getpwnam_r()', taking into account some additional possible errors.
*     For a detailed description see 'getpwnam_r()' man page.
*
*  INPUTS
*     const char *name  - points to user name 
*     struct passwd *pw - points to structure which will be updated upon success 
*     char *buffer      - points to memory referenced by 'pw'
*     size_t buflen     - size of 'buffer' in bytes 
*
*  RESULT
*     struct passwd* - Pointer to entry matching user name upon success,
*                      NULL otherwise.
*
*  NOTES
*     MT-NOTE: sge_getpwnam_r() is MT safe. 
*
*******************************************************************************/
struct passwd *sge_getpwnam_r(const char *name, struct passwd *pw, 
                              char *buffer, size_t bufsize)
{
   struct passwd *res = NULL;
   int i = MAX_NIS_RETRIES;
 
   DENTER(UIDGID_LAYER, "sge_getpwnam_r");

   while (i-- && !res) {
      if (getpwnam_r(name, pw, buffer, bufsize, &res) != 0) {
         res = NULL;
      }
   }
 
   /* sometime on failure struct is non NULL but name is empty */
   if (res && !res->pw_name) {
      res = NULL;
   }
 
   DRETURN(res);
} /* sge_getpwnam_r() */


/****** uti/uidgid/sge_getgrgid_r() ********************************************
*  NAME
*     sge_getgrgid_r() -- Return group information for a given group ID.
*
*  SYNOPSIS
*     struct group* sge_getgrgid_r(gid_t gid, struct group *pg,
*                                  char *buffer, size_t bufsize, int retries)
*
*  FUNCTION
*     Search account database for a group. This function is just a wrapper for
*     'getgrgid_r()', taking into account some additional possible errors.
*     For a detailed description see 'getgrgid_r()' man page.
*
*  INPUTS
*     gid_t gid         - group ID
*     struct group *pg  - points to structure which will be updated upon success
*     char *buffer      - points to memory referenced by 'pg'
*     size_t buflen     - size of 'buffer' in bytes 
*     int retries       - number of retries to connect to NIS
*
*  RESULT
*     struct group*  - Pointer to entry matching group information upon success,
*                      NULL otherwise.
*
*  NOTES
*     MT-NOTE: sge_getpwnam_r() is MT safe. 
*
*******************************************************************************/
struct group *sge_getgrgid_r(gid_t gid, struct group *pg, 
                             char **buffer, size_t bufsize, int retries)
{
   struct group *res = NULL;

   DENTER(UIDGID_LAYER, "sge_getgrgid_r");

   while(retries-- && !res) {
#if defined(INTERIX)
      if (getgrgid_nomembers_r(gid, pg, *buffer, bufsize, &res) != 0)
#else
      if (getgrgid_r(gid, pg, *buffer, bufsize, &res) != 0)
#endif
      {
         if (errno == ERANGE) {
            retries++;
            bufsize += 1024;
            *buffer = sge_realloc(*buffer, bufsize, 1);
         }
         res = NULL;
      }
   }

   /* could be that struct is not NULL but group nam is empty */
   if (res && !res->gr_name) {
      res = NULL;
   }

   DEXIT;
   return res;
} /* sge_getgrgid_r() */

/****** uti/uidgid/sge_is_user_superuser() *************************************
*  NAME
*     sge_is_user_superuser() -- check if provided user is the superuser
*
*  SYNOPSIS
*     bool sge_is_user_superuser(const char *name); 
*
*  FUNCTION
*     Checks platform independently if the provided user is the superuser.
*
*  INPUTS
*     const char *name - name of the user to check
*
*  RESULT
*     bool - true if it is the superuser,
*            false if not.
*
*  NOTES
*     MT-NOTE: sge_is_user_superuser() is MT safe. 
*
*******************************************************************************/
bool sge_is_user_superuser(const char *name)
{
   bool ret = false;

#if defined(INTERIX)
   char buffer[1000];
   char *plus_sign;

   wl_get_superuser_name(buffer, 1000);

   /* strip Windows domain name from user name */
   plus_sign = strstr(buffer, "+");
   if (plus_sign!=NULL) {
      plus_sign++;
      sge_strlcpy(buffer, plus_sign, sizeof(buffer));
   }
   if ((strncmp(name, buffer, 1000) == 0) || (strcmp(name, "root") == 0)) {
      ret = true;
   }
#else
   ret = (strcmp(name, "root") == 0) ? true : false;
#endif

   return ret;
}

/****** uti/uidgid/uidgid_state_get_*() ************************************
*  NAME
*     uidgid_state_set_*() - read access to lib/uti/sge_uidgid.c global variables
*
*  FUNCTION
*     Provides access to per thread global variable.
*
******************************************************************************/
static uid_t uidgid_state_get_last_uid(void)
{ 
   GET_SPECIFIC(struct uidgid_state_t, uidgid_state, uidgid_state_init, uidgid_state_key, "uidgid_state_get_last_uid");
   return uidgid_state->last_uid;
}

static const char *uidgid_state_get_last_username(void)
{ 
   GET_SPECIFIC(struct uidgid_state_t, uidgid_state, uidgid_state_init, uidgid_state_key, "uidgid_state_get_last_username");
   return uidgid_state->last_username;
}

static gid_t uidgid_state_get_last_gid(void)
{ 
   GET_SPECIFIC(struct uidgid_state_t, uidgid_state, uidgid_state_init, uidgid_state_key, "uidgid_state_get_last_gid");
   return uidgid_state->last_gid;
}

static const char *uidgid_state_get_last_groupname(void)
{ 
   GET_SPECIFIC(struct uidgid_state_t, uidgid_state, uidgid_state_init, uidgid_state_key, "uidgid_state_get_last_groupname");
   return uidgid_state->last_groupname;
}

/****** uti/uidgid/uidgid_state_set_*() ************************************
*  NAME
*     uidgid_state_set_*() - write access to lib/uti/sge_uidgid.c global variables
*
*  FUNCTION
*     Provides access to per thread global variable.
*
******************************************************************************/
static void uidgid_state_set_last_uid(uid_t uid)
{ 
   GET_SPECIFIC(struct uidgid_state_t, uidgid_state, uidgid_state_init, uidgid_state_key, "uidgid_state_set_last_uid");
   uidgid_state->last_uid = uid;
}

static void uidgid_state_set_last_username(const char *user)
{ 
   GET_SPECIFIC(struct uidgid_state_t, uidgid_state, uidgid_state_init, uidgid_state_key, "uidgid_state_set_last_username");
   sge_strlcpy(uidgid_state->last_username, user, SGE_MAX_USERGROUP_BUF);
}

static void uidgid_state_set_last_gid(gid_t gid)
{ 
   GET_SPECIFIC(struct uidgid_state_t, uidgid_state, uidgid_state_init, uidgid_state_key, "uidgid_state_set_last_gid");
   uidgid_state->last_gid = gid;
}

static void uidgid_state_set_last_groupname(const char *group)
{ 
   GET_SPECIFIC(struct uidgid_state_t, uidgid_state, uidgid_state_init, uidgid_state_key, "uidgid_state_set_last_groupname");
   sge_strlcpy(uidgid_state->last_groupname, group, SGE_MAX_USERGROUP_BUF);
}

/****** uti/uidgid/set_admin_user() ********************************************
*  NAME
*     set_admin_user() -- Cache user and group info for admin user.
*
*  SYNOPSIS
*     static void set_admin_user(const char *user_name, uid_t theUID, gid_t theGID)
*
*  FUNCTION
*     Initialize the admin_user structure.
*
*  INPUTS
*     const char *user_name - uer name of admin user
*     uid_t theUID - user id of admin user
*     gid_t theGID - group id of admin user
*
*  RESULT
*     static void - none
*
*  NOTES
*     MT-NOTE: set_admin_user() is MT safe. 
*
*******************************************************************************/
static void set_admin_user(const char *user_name, uid_t theUID, gid_t theGID)
{
   uid_t uid = theUID;
   gid_t gid = theGID;
   gid_t dummy[1];

   DENTER(UIDGID_LAYER, "set_admin_user");

   sge_mutex_lock("admin_user_mutex", SGE_FUNC, __LINE__, &admin_user.mutex);
   errno = 0;
   admin_user.user_name = user_name;
   admin_user.uid = uid;
   admin_user.gid = gid;
   /* Cache the supplementary groups to avoid access to the groups
      database.  */
   admin_user.ngroups = 1;
   getgrouplist(user_name, gid, dummy, &admin_user.ngroups);
   if (admin_user.ngroups <= 0) goto err;
   admin_user.groups = sge_malloc(admin_user.ngroups * sizeof(gid_t));
   if (getgrouplist(user_name, gid, admin_user.groups, &admin_user.ngroups)
       < 0) goto err;
   if (admin_user.ngroups <= 0) goto err;
   admin_user.initialized = true;
   sge_mutex_unlock("admin_user_mutex", SGE_FUNC, __LINE__, &admin_user.mutex);

   DPRINTF(("auid=%ld; agid=%ld\n", (long)uid, (long)gid));

   DEXIT;
   return;
 err:
   CRITICAL((SGE_EVENT, MSG_SYSTEM_ADMIN_INFO_S, strerror(errno)));
   DEXIT;
   sge_exit(NULL, 1);
   return;
} /* set_admin_user() */

/****** uti/uidgid/get_admin_user() ********************************************
*  NAME
*     get_admin_user() -- Get user and group id of admin user.
*
*  SYNOPSIS
*     static int get_admin_user(uid_t* theUID, gid_t* theGID) 
*
*  FUNCTION
*     Get user and group id of admin user. 'theUID' and 'theGID' will contain
*     the user and group id respectively, upon successful completion.
*
*     If the admin user has not been set by a call to 'set_admin_user()'
*     previously, an error is returned. In case of an error, the locations
*     pointed to by 'theUID' and 'theGID' remain unchanged.
*
*  OUTPUTS
*     uid_t* theUID - pointer to user id storage.
*     gid_t* theGID - pointer to group id storage.
*
*  RESULT
*     int - Returns ESRCH, if no admin user has been initialized. 
*
*  EXAMPLE
*
*     uid_t uid;
*     gid_t gid;
*
*     if (get_admin_user(&uid, &gid) == ESRCH) {
*        printf("error: no admin user\n");
*     } else {
*        printf("uid = %d, gid =%d\n", (int)uid, (int)gid);
*     }
*       
*  NOTES
*     MT-NOTE: get_admin_user() is MT safe.
*
*******************************************************************************/
static int get_admin_user(uid_t* theUID, gid_t* theGID, int *ngroups, gid_t **groups)
{
   uid_t uid;
   gid_t gid;
   bool init = false;
   int res = ESRCH;

   DENTER(UIDGID_LAYER, "get_admin_user");

   sge_mutex_lock("admin_user_mutex", SGE_FUNC, __LINE__, &admin_user.mutex);
   uid = admin_user.uid;
   gid = admin_user.gid;
   *ngroups = admin_user.ngroups;
   *groups = admin_user.groups;
   init = admin_user.initialized;
   sge_mutex_unlock("admin_user_mutex", SGE_FUNC, __LINE__, &admin_user.mutex);

   if (init == true) {
      *theUID = uid;
      *theGID = gid;
      res = 0;
   }

   DEXIT;
   return res;
} /* get_admin_user() */

/****** uti/uidgid/get_admin_user_name() ***************************************
*  NAME
*     get_admin_user_name() -- Returns the admin user name
*
*  SYNOPSIS
*     const char* get_admin_user_name(void) 
*
*  FUNCTION
*     Returns the admin user name. 
*
*  INPUTS
*     void - None 
*
*  RESULT
*     const char* - Admin user name
*
*  NOTES
*     MT-NOTE: get_admin_user_name() is MT safe 
*******************************************************************************/
const char *get_admin_user_name(void) {
   return  admin_user.user_name;
}

/****** uti/uidgid/sge_has_admin_user() ****************************************
*  NAME
*     sge_has_admin_user() -- is there a admin user configured and set
*
*  SYNOPSIS
*     bool sge_has_admin_user(void) 
*
*  FUNCTION
*     Returns if there is a admin user setting configured and set. 
*
*  INPUTS
*     void - None 
*
*  RESULT
*     bool - result
*        true  - there is a setting
*
*  NOTES
*     MT-NOTE: sge_has_admin_user() is MT safe 
*******************************************************************************/
bool 
sge_has_admin_user(void) {
   bool ret = true;
   uid_t uid;
   gid_t gid, *groups;
   int ngroups;

   DENTER(TOP_LAYER, "sge_has_admin_user");
   ret = (get_admin_user(&uid, &gid, &ngroups, &groups) == ESRCH) ? false : true;
   DRETURN(ret);
}

/****** uti/uidgid/uidgid_once_init() ******************************************
*  NAME
*     uidgid_once_init() -- One-time user and group function initialization.
*
*  SYNOPSIS
*     static uidgid_once_init(void) 
*
*  FUNCTION
*     Create access key for thread local storage. Register cleanup function.
*
*     This function must be called exactly once.
*
*  INPUTS
*     void - none
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: uidgid_once_init() is MT safe. 
*
*******************************************************************************/
static void uidgid_once_init(void)
{
   pthread_key_create(&uidgid_state_key, uidgid_state_destroy);
}

/****** uti/uidgid/uidgid_state_destroy() **************************************
*  NAME
*     uidgid_state_destroy() -- Free thread local storage
*
*  SYNOPSIS
*     static void uidgid_state_destroy(void* theState) 
*
*  FUNCTION
*     Free thread local storage.
*
*  INPUTS
*     void* theState - Pointer to memory which should be freed.
*
*  RESULT
*     static void - none
*
*  NOTES
*     MT-NOTE: uidgid_state_destroy() is MT safe.
*
*******************************************************************************/
static void uidgid_state_destroy(void* theState)
{
   sge_free(&theState);
}

/****** uti/uidgid/uidgid_state_init() *****************************************
*  NAME
*     uidgid_state_init() -- Initialize user and group function state.
*
*  SYNOPSIS
*     static void cull_state_init(struct cull_state_t* theState) 
*
*  FUNCTION
*     Initialize user and group function state.
*
*  INPUTS
*     struct cull_state_t* theState - Pointer to user and group state structure.
*
*  RESULT
*     static void - none
*
*  NOTES
*     MT-NOTE: cull_state_init() is MT safe.
*
*******************************************************************************/
static void uidgid_state_init(struct uidgid_state_t* theState)
{
   memset(theState, 0, sizeof(struct uidgid_state_t));
}

/* Not MT-Safe */
const char*
sge_get_file_passwd(void)
{  
   static char file[SGE_PATH_MAX] = "";

   DENTER(TOP_LAYER, "sge_get_file_passwd");
   if (file[0] == '\0') {
      const char *sge_root = sge_get_root_dir(0, NULL, 0, 1);
      const char *sge_cell = sge_get_default_cell();

      snprintf(file, sizeof(file), "%s/%s/common/sgepasswd", sge_root, sge_cell);
   }
   DEXIT;
   return file;
}

/****** uti/uidgid/password_get_size()****************************************
*  NAME
*     password_get_size() - count number of lines in sgepasswd file
*
*  SYNOPSIS
*     static int password_get_size(const char *filename)
*
*  FUNCTION
*     This function counts the lines in the sgepasswd file, it also checks
*     that each line is not too long.  If any line has
*     more than MAX_LINE_LENGTH characters the function returns error state -1
*
*  INPUTS
*     const char *filename - name of the file on which the function is run
*
*  RESULT
*     int - returns number of lines in sgepasswd file if function has run
*           successfully
*         -1 - if there is a line in a file that is longer than MAX_LINE_LENGTH
*
*  NOTES
*     MT-NOTE: password_get_size() is not MT safe
*
*******************************************************************************/
static int
password_get_size(const char *filename)
{
   size_t ret = 0;
   FILE   *fp = NULL;
   char   input[MAX_LINE_LENGTH];
   bool   do_loop = true;

   DENTER(TOP_LAYER, "password_get_size");
   fp = fopen(filename, "r");
   if (fp != NULL) {
      while (do_loop) {
         if (get_file_line_size(fp) > MAX_LINE_LENGTH) {
            ret = -1;
            break;
         }
         if (fscanf(fp, "%[^\n]\n", input) == 1) {
            ret++;
         } else {
            do_loop = false;
         }
      }
      FCLOSE(fp);
   }
FCLOSE_ERROR:
   DRETURN(ret);
}

/****** uti/uidgid/get_file_line_size()****************************************
*  NAME
*     get_file_line_size() - helper function that counts the characters in
*                            current line of file
*  SYNOPSIS
*     int get_file_line_size(FILE* fp)
*
*  FUNCTION
*     This helper function counts the characters in current line of file,
*     it restores file position pointer to the position that was before
*     entering function. Function  can be called on any opened file with read
*     permissions.
*
*  INPUTS
*     FILE* fp - handler of the file in which the characters in current line
*                are counted
*
*  RESULT
*     int - returns number of characters in current line of file,
*           if number of characters is bigger than MAX_LENGTH_LINE,
*           it returns MAX_LINE_LENGTH+1 value
*
*  NOTES
*     MT-NOTE: get_file_line_size() is not MT safe
*
*******************************************************************************/

static int
get_file_line_size(FILE *fp)
{
   fpos_t pos;
   char   tmp = '\0';
   int    i = 0;

   fgetpos(fp,&pos);
   while ((tmp != '\n') && (i <= MAX_LINE_LENGTH)) {
      if (fscanf(fp, "%c", &tmp) == 1) {
         i++;
      } else {
         break;
      }
  }
  fsetpos(fp, &pos);
  return i;
}

/* Fixme:  See issue 386 concerning the need to use a CULL list (and
   to lock the file?).  This is mitigated by checking the number of
   records in the loop below.  */

/****** uti/uidgid/password_read_file()*****************************************
*  NAME
*     password_read_file() - read in sgepasswd file
*
*  SYNOPSIS
*     int password_read_file(char **users[], char**encryped_pwds[],
*                            const char *filename)
*
*  FUNCTION
*     This function reads usernames and encrypted passwords from sgepasswd file
*     and saves them in arrays that are part if function output, if the file
*     could not be opened for reading or the file contains corrupted line,
*     (too long line, line that doesn't contain username and password),
*     the proper error state is returned
*
*  INPUTS
*     const char* filename - name with path of the sgepasswd file that is
*                            going to be read
*
*  OUTPUTS
*     char** users[] - array of strings to which are written the usernames
*                      that are read from sgepasswd, if sgepasswd could not be
*                      opened there are reserved 2 entries with NULL values,
*                      if the sgepasswd file is corrupted, no entries are
*                      reserved
*
*     char** encrypted_pwds[] - array of strings to which are written encrypted
*                               passwords that are read from sgepasswd
*                               if sgepasswd could not be opened there are
*                               reserved 2 entries with NULL values,
*                               if the sgepasswd file is corrupted,
*                               no entries are reserved
*
*  RESULT
*     int - error state
*         0 - OK, no errors
*         1 - sgepasswd could not be opened for reading
*         2 - sgepasswd file is corrupted
*
*  NOTES
*     MT-NOTE: password_read_file() is not MT safe
*
*******************************************************************************/
int
password_read_file(char **users[], char**encryped_pwds[], const char *filename)
{
   struct saved_vars_s *context;
   int    j;
   int    ret = 0;
   FILE   *fp = NULL;
   char   *uname = NULL;
   char   *pwd = NULL;
   char   input[MAX_LINE_LENGTH];
   bool   do_loop = true;
   size_t size = 0;
   int    i = 0;

   DENTER(TOP_LAYER, "password_read_file");
   fp = fopen(filename, "r");
   if (fp != NULL) {
      size = password_get_size(filename);
      if (size == -1){
         /*file corrupted*/
         ret = 2;
         size = 0;
         do_loop = false;
      }
      size = size + 2;
      *users = malloc(size * sizeof(char*));
      *encryped_pwds = malloc(size * sizeof(char*));

      while (do_loop) {
         uname = NULL;
         pwd = NULL;
         context = NULL;

         /* Check records in case file changed since reading initially.  */
         if (i >= size) {
            ret = 2;
            break;
         }

         if (fscanf(fp, "%[^\n]\n", input) == 1) {
            uname = sge_strtok_r(input, " ", &context);
            pwd = sge_strtok_r(NULL, " ", &context);
            if ((uname == NULL) || (pwd == NULL)){
                do_loop = false;
                /*file corrupted*/
                ret = 2;
                do_loop = false;
            } else {
               (*users)[i] = strdup(uname);
               (*encryped_pwds)[i] = strdup(pwd);
               i++;
            }
         } else {
            do_loop = false;
         }
         sge_free_saved_vars(context);
      }
      if (ret == 2) {
         for (j=0; j<i; j++) {
            sge_free(&((*users)[j]));
            sge_free(&((*encryped_pwds)[j]));
         }
         sge_free(users);
         sge_free(encryped_pwds);
         DPRINTF(("sgepasswd file is corrupted"));
      } else {
         (*users)[i] = NULL;
         (*encryped_pwds)[i] = NULL; i++;
         (*users)[i] = NULL;
         (*encryped_pwds)[i] = NULL; i++;
      }

      FCLOSE(fp);
   } else {
      *users = malloc(2 * sizeof(char*));
      *encryped_pwds = malloc(2 * sizeof(char*));
      (*users)[0] = NULL;
      (*encryped_pwds)[0] = NULL;
      (*users)[1] = NULL;
      (*encryped_pwds)[1] = NULL;

      /* Can't read passwd file */
      ret = 1;
   }
   DEXIT;
   return ret;
FCLOSE_ERROR:
   DEXIT;
   return 1;
}

/* Not MT-Safe */
int
password_find_entry(char *users[], char *encrypted_pwds[], const char *user)
{
   int ret = -1;
   size_t i = 0;

   DENTER(TOP_LAYER, "password_find_entry");
   while (users[i] != NULL) {
      if (!strcmp(users[i], user)) {
         ret = i;
         break;
      }
      i++;
   }
   return ret;
}

#if defined(INTERIX)
/* Not MT-Safe */
/* Read password for user from sgepasswd file, decrypt password */
int uidgid_read_passwd(const char *user, char **pass, char *err_str, size_t lstr)
{
   int  i;
   int  ret = 1;
   char **users = NULL;
   char **encrypted_pwd = NULL;
   char *buffer_decr = NULL;
   const char *passwd_file = NULL;
  
   unsigned char *buffer_deco = NULL;
   size_t buffer_deco_length = 0;
   size_t buffer_decr_size = 0;
   size_t buffer_decr_length = 0;

   /*
    * Read password table
    */
   passwd_file = sge_get_file_passwd();
   ret = password_read_file(&users, &encrypted_pwd, passwd_file);
   if(ret != 0) {
      snprintf(err_str, lstr, MSG_SYSTEM_READ_SGEPASSWD_SSI,
               passwd_file, strerror(errno)?strerror(errno):"<NULL>", errno);
      ret = 1;
   } else {
      /*
       * Search user in table, decrypt users password.
       */
      i = password_find_entry(users, encrypted_pwd, user);
      if(i == -1) {
         snprintf(err_str, lstr, MSG_SYSTEM_NO_PASSWD_ENTRY_SS, user, passwd_file);
         ret = 2;
      } else {
#if defined(SECURE)
         buffer_deco_length = strlen(encrypted_pwd[i]);
         buffer_decode_hex((unsigned char*)encrypted_pwd[i],
                            &buffer_deco_length, &buffer_deco);
         ret = buffer_decrypt((const char*)buffer_deco, buffer_deco_length,
                              &buffer_decr, &buffer_decr_size, &buffer_decr_length, err_str, lstr);
#else
         ret = 1;
#endif
         if (ret == 1) {
             ret = 3; //password is not correct need to distinguish from passwd_file_error
         }
         *pass = buffer_decr; 
         sge_free(&buffer_deco);
      }
   }
   return ret;
}
#endif /* #if defined(INTERIX) */
