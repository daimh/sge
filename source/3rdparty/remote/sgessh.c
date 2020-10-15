#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>

#if defined SOLARIS || HPUX || NECSX5 || CRAY
#define _PATH_NOLOGIN "/etc/nologin"
#define _PATH_BSHELL "/bin/sh"
#define _PATH_DEFPATH "/usr/bin:/bin"
#else
#include <paths.h>
#endif

#include <sge_unistd.h>
#include <setosjobid.h>
#include <setrlimits.h>
#include <config_file.h>
#include <sge_uidgid.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

extern int foreground;
bool  g_new_interactive_job_support = false; /* This is needed in err_trace.c */

static char *s_qsub_gid = NULL;
static char start_dir[MAXPATHLEN] = "";

int sgessh_readconfig(void)
{
  char err_str[2048];

  read_config("config");

  if (sge_set_admin_username(get_conf_val("admin_user"), err_str, sizeof(err_str)))
  {
    perror(err_str);
    exit(1);
  }

  getcwd(start_dir, MAXPATHLEN);

  s_qsub_gid = get_conf_val("qsub_gid");

  return 0;
}



int sgessh_do_setusercontext(struct passwd *pwd)
{
   char work_dir[MAXPATHLEN];
   gid_t add_grp_id;

   getcwd(work_dir, MAXPATHLEN);
   chdir(start_dir);

   sge_switch2admin_user();
   foreground = 0; /* setosjobid shall write to shepherd trace file */
   setosjobid(0, &add_grp_id, pwd);
   setrlimits(0);
   sge_switch2start_user();

   if (*pwd->pw_shell == '\0')
     pwd->pw_shell = _PATH_BSHELL;

#if BSD > 43
   if (setlogin(pwd->pw_name) < 0)
     syslog(LOG_ERR, "setlogin() failed: %m");
#endif

   if(s_qsub_gid != NULL && strcmp(s_qsub_gid, "no") != 0)
      pwd->pw_gid = atoi(s_qsub_gid);

   (void) setgid((gid_t)pwd->pw_gid);

#if !defined(INTERIX) /* EB: TODO: There is no initgroups() in INTERIX */
   initgroups(pwd->pw_name, pwd->pw_gid);
#endif

#if (SOLARIS || ALPHA || LINUX || DARWIN)
   /* add Additional group id to current list of groups */
   if (add_grp_id) {
      char err_str[1024];
      bool skip_silently = false;
      const char *tmp_str = search_conf_val("skip_ngroups_max_silently");

      if (tmp_str != NULL && strcmp(tmp_str, "yes") == 0) {
         skip_silently = true;
      }
      if (sge_add_group(add_grp_id, err_str, sizeof(err_str), skip_silently) == -1) {
         perror(err_str);
      }
   }
#endif

   setuid((uid_t)pwd->pw_uid);

   chdir(work_dir);

   return 0;

}
