/* sge_execvlp.c -- execvp cum execlp and routines to sanitize environment
   Copyright (C) 2011 Dave Love, University of Liverpool <d.love@liv.ac.uk>

   This file is free software: you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 3 of
   the License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this file.  If not, a copy can be downloaded
   from http://www.gnu.org/licenses/lgpl.html.
*/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <regex.h>
#include "uti2/sge_execvlp.h"
#include "sgeobj/sge_var.h"
#include "uti/sge_string.h"

extern char **environ;

static char **
newargs (const char *file, char *const argv[])
{
   int i, na = 0;
   char **newargv;

   while (argv[na++]) ;
   newargv = malloc ((na + 1) * sizeof (char *));
   if (!newargv) return NULL;
   newargv[0] = "/bin/sh";
   newargv[1] = (char *) file;
   for (i = 2; i < na + 1; i++)
      newargv[i] = argv[i-1];
   return newargv;
}

/****** uti/sge_execvlp() ****************************************
*
*  NAME
*     sge_execvlp -- like execve, but search the path
*
*  SYNOPSIS
*     #include "uti/execvlp.h"
*     int sge_execvlp (const char *file, char *const argv[], char *const envp[])
*
*  FUNCTION
*     A version of execve that does a path search for the executable
*     like execlp/execvp.
*
*  INPUTS
*     file - name of executable
*     argv - null-terminated list of arguments
*     envp - null-terminated list of environment elements
*
*  RESULT
*     on success, the function will not return (it execs)
*     -1 if the exec fails
******************************************************************************/
int
sge_execvlp (const char *file, char *const argv[], char *const envp[])
{
   char *path;
   char execpath[SGE_PATH_MAX];
   char *component;
   int first = 1;

   path = getenv ("PATH");
   if (!path)                    /* use file directly if no path */
      return execve (file, argv, envp);

   if (strchr (file, '/')) {	/* directory path -- don't search */
      int late_errno = errno;

      errno = 0;
      execve (file, argv, envp);
      if (errno == ENOEXEC) {
	 char **newargv;

         errno = late_errno;
         newargv = newargs (file, argv);
         if (!newargv) return -1;
	 execve (newargv[0], newargv, envp);
	 free (newargv);
      }
      return -1;
   }

   /* Else try path components */
   path = strdup (path);
   while ((component = strtok ((first ? path : NULL), ":"))) {
      int late_errno = errno;

      first = 0;
      execpath[0] = '\0';
      if (strlen (component) != 0) { /* else current directory */
         if ((sge_strlcat(execpath, component, sizeof(execpath))
              >= sizeof(execpath))
             || (sge_strlcat(execpath, "/", sizeof(execpath))
                 >= sizeof(execpath)))
            continue;
      }
      if (sge_strlcat(execpath, file, sizeof(execpath)) >= sizeof(execpath))
         continue;
      errno = 0;
      execve (execpath, argv, envp);
      if (errno == ENOEXEC) {
	 char **newargv;

         newargv = newargs (file, argv);
         if (!newargv) return -1;
	 execve (newargv[0], newargv, envp);
	 free (newargv);
         errno = late_errno;
	 sge_free(&path);
	 return -1;
      }
      if (errno != ENOENT)
	 return -1;
   }
   sge_free(&path);
   return -1;
}

/* Fixme:  Instead of semi-hardwiring (parts of) environment variables
   below, we should make them execd parameters, but that requires (a)
   infrastructure to allow list-valued execd parameters, (b) ensuring
   it gets set up.  */

/* Check whether an environment string (foo=bar) sets a variable in
   the unsafe list or with a bad value.  In retrospect, this stuff
   should probably just have been lifted from sudo.  */
static bool
var_unsafe(char *var, regex_t lc_regex)
{
   const char *unsafe[] = {
      /* These variables are regarded as unsafe in suid programs by
         GNU libc (2.11.1).
         Fixme:  Try to extend this for other systems.  */
      "GCONV_PATH", "GETCONF_DIR", "HOSTALIASES",
      /* We get everything with an LD_ prefix anyhow. */
      /*
        "LD_AUDIT", "LD_DEBUG", "LD_DEBUG_OUTPUT",
        "LD_DYNAMIC_WEAK", "LD_LIBRARY_PATH", "LD_ORIGIN_PATH",
        "LD_PRELOAD", "LD_PROFILE", "LD_SHOW_AUXV", "LD_USE_LOAD_BIAS",
      */
      "LOCALDOMAIN", "LOCPATH", "MALLOC_TRACE", "NIS_PATH",
      "NLSPATH", "RESOLV_HOST_CONF", "RES_OPTIONS", "TMPDIR", "TZDIR",
      /* Others not from glibc */
      /* LD_LIBRARY_PATH &c, and DYLD_LIBRARY_PATH taken care of by
	 regexps elsewhere.  */
      /* Fixme:  Check if IFS actually needs to be set on relevant systems
         <http://www.dwheeler.com/secure-programs/Secure-Programs-HOWTO/environment-variables.html#ENV-VAR-SOLUTION>.  */
      "IFS",   /* for shell scripts, but not inherited by the ones I've tried */
      /* From sudo, but maybe not all relevant here.  (Various ones
         are elided, such as for SecurID, obsolete krb4, and those
         which can be removed by initial sanitization in shells or
         command-line options like perl -T):  */
      "SHELLOPTS", "PS4", "PATH_LOCALE", "PATH_LOCALE",
      "TERMINFO", "TERMINFO_DIRS", "TERMPATH", "TERMCAP",
      "ENV", "BASH_ENV", "KRB5_CONFIG", "KRB5_KTNAME", "JAVA_TOOL_OPTIONS",
#ifdef _AIX
      "AUTHSTATE",
#endif
      ""};
   int i;

   for (i=0; strlen (unsafe[i]) > 0; i++)
      if (strncmp (var, unsafe[i], strlen (unsafe[i])) == 0)
         return true;

   /* Check locale variable values per
      <http://www.dwheeler.com/secure-programs/Secure-Programs-HOWTO/locale.html> */
   {
      const char *locale[] = {"LANGUAGE", "LANG", "LINGUAS", ""};
      for (i=0; strlen (locale[i]) > 0; i++)
         if (strncmp (var, locale[i], strlen (locale[i])) == 0
             || strncmp (var, "LC_", 3) == 0)
            if (regexec (&lc_regex, var, 0, NULL, 0) != 0) /* invalid locale */
               return true;
   }
   return false;
}

/****** uti/sanitize_environment() *****************************************
*  NAME
*     sanitize_environment() -- remove sensitive variables from the environment before calling programs
*
*  SYNOPSIS
*     #include "uti/execvlp.h"
*     void sanitize_environment(char *env[])
*
*  FUNCTION
*     Remove variables from the environment which represent a security
*     risk when a program is called under a privileged id with an
*     environment supplied by the user, such as remote
*     communication daemons.  (For instance, if the user can get a
*     dynamically-linked privileged program run in an ELF environment
*     with arbitrary LD_LIBRARY_PATH or LD_PRELOAD, all bets are off.)
*
*  SEE ALSO
*  qrsh_starter
*******************************************************************************/
void
sanitize_environment (char *env[])
{
   int i = 0, j;
   regex_t lc_regex;
   const char *sharedlib = var_get_sharedlib_path_name ();
   /* Valid locale -- see var_unsafe.  */
   const char *lc_pattern = "^[A-Za-z_]+=[A-Za-z][-A-Za-z0-9_,+@.=]*$";

   if (!env) return;
   /* We only need to bother if we're privileged in some way.  */
   if ((geteuid () != SGE_SUPERUSER_UID)
       && (getegid () != SGE_SUPERUSER_GID)
       && (geteuid () == getuid ())
       && (getegid () == getgid ()))
      return;

   if (regcomp (&lc_regex, lc_pattern, REG_NOSUB | REG_EXTENDED))
      exit(1);

   while (env[i]) {
      /* In an ELF system, sensitive variables include
         LD_LIBRARY_PATH, plus variations like LD_LIBRARY_PATH64, and
         LD_PRELOAD.  For safety, we'll zap anything with an LD_
         prefix.  (We could test the __ELF__ of the system, but this
         is safer.)  Similarly check var_get_sharedlib_path_name as a
         prefix.  */
      if ((strncmp (env[i], "LD_", 3) == 0)
          /* Known in Tru64, but sudo has it generally */
	  || (strncmp (env[i], "_RLD_", 5) == 0)
#ifdef __APPLE__
	  /* DYLD_{FALLBACK_,}FRAMEWORK_PATH,
	     {FALLBACK_,}DYLD_LIBRARY_PATH seem to be relevant.  */
	  || (strncmp (env[i], "DYLD_", 5) == 0)
#elif defined _AIX
	  || (strncmp (env[i], "LDR_", 4) == 0)
#elif defined __hpux__
          /* _HP_DLDOPTS is loader options; guess a prefix.  */
	  || (strncmp (env[i], "_HP_DLD", 7) == 0)
#endif
          || (strncmp (env[i], sharedlib, strlen (sharedlib)) == 0)
          || var_unsafe (env[i], lc_regex)) {
         /* Shuffle env elements down over the unsafe one.  */
         for (j=i; env[j]; j++)
            env[j] = env[j+1];
      } else {
         i++;
      }
   }
   regfree (&lc_regex);
   return;
}

/****** uti/sge_copy_sanitize_env() *****************************************
*  NAME
*     sge_copy_sanitize_environment() -- make a copy of environment with sensitive variables removed
*
*  SYNOPSIS
*     #include "uti/execvlp.h"
*     char **sge_copy_sanitize_env(char *env[])
*
*  FUNCTION
*     Remove variables from the environment which represent a security
*     risk if when a program is called under a privileged id with an
*     environment supplied by the user, for example, remote
*     communication daemons.  (If the user can get a
*     dynamically-linked privileged program run in an ELF environment,
*     say, with arbitrary LD_LIBRARY_PATH or LD_PRELOAD, all bets are
*     off.)  Also try to protect shell scripts from IFS, at least.
*******************************************************************************/
char **sge_copy_sanitize_env (char *const env[])
{
   int l=0;
   size_t bytes;
   char **newenv;

   while (env[l] != 0)
      l++;
   bytes = (l+1)*sizeof (char *);
   newenv = malloc (bytes);
   memcpy (newenv, env, bytes);
   sanitize_environment (newenv);
   return newenv;
}

#if TEST
int main(int argc, char* argv[])
{
   char *args[] = {""};
   sge_execvlp ("env", args, sge_copy_sanitize_env(environ));
   exit (0);
}
#endif
