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

#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <sys/types.h>

#include "uti/sge_rmon.h"
#include "uti/sge_stdio.h"
#include "uti/sge_log.h"
#include "uti/setup_path.h"
#include "uti/sge_string.h" 
#include "uti/sge_uidgid.h"
#include "uti/sge_unistd.h"
#include "uti/sge_hostname.h"

#include "comm/lists/cl_errors.h"

#include "sgeobj/sge_host.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_path_alias.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "msg_common.h"
#include "msg_daemons_common.h"

/****** sgeobj/path_alias/-PathAlias *******************************************
*  NAME
*     PathAlias - Path aliasing mechanism for SGE/EE
*
*  FUNCTION
*     Sometimes the directory structure on the different submit,
*     execution hosts is not exactly the same. This makes it 
*     necessary to map certain paths for specific hosts.
*
*     The administrators/users have the possibility to
*     activate path aliasing by creating one or more of the following
*     files:
*
*        $SGE_ROOT/$CELL/common/sge_aliases
*        $HOME/.sge_aliases 
*
*     The file format is documented in the ADOC comment for the
*     function path_alias_read_from_file().
*
*     The files are interpreted as follows:
*        - after a submit client (qsub, qmon, ...) has retrieved 
*          the current working directory, the cluster global
*          path aliasing file is read, if present. The user
*          path aliasing file is read afterwards, as if it were 
*          appended to the global file.
*        - as soon as both files are read, the path aliasing
*          information is passed along with the submitted job.
*        - On the execution host the aliasing information will be
*          evaluated. The leading part of the current working 
*          directory will be replaced if the execution host entry of
*          the path alias matches the executing host.
*
*  SEE ALSO
*     sgeobj/path_alias/path_alias_read_from_file()
*     sgeobj/path_alias/path_alias_list_initialize()
*     sgeobj/path_alias/path_alias_list_get_path()
******************************************************************************/

static int path_alias_read_from_file(lList **path_alias_list, lList **alpp, 
                                     char *file_name);

/****** sgeobj/path_alias/path_alias_read_from_file() *************************
*  NAME
*     path_alias_read_from_file() -- read file content to list
*
*  SYNOPSIS
*     #include <sgeobj/sge_path_alias.h>
*  
*     static int path_alias_read_from_file(lList **path_alias_list, 
*                                          lList **alpp, 
*                                          char *file_name) 
*
*  FUNCTION
*     Read and parse the file with the name "file_name" and append
*     entries into "path_alias_list". Errors will be logged in "alpp". 
*
*     File format:
*     - Blank lines and lines beginning with a # sign in the first
*       column are skipped.
*     - Each line - other than a blank line or a line preceded by # - 
*       must contain four strings separated by any number of blanks 
*       or tabs.
*     - The first string specifies a source path, the second a submit
*       host, the third an execution host, and the fourth the source 
*       path replacement.
*     - Both the submit and the execution host entries may consist
*       of only a * sign, which matches any host.
*
*  INPUTS
*     lList **path_alias_list - PA_Type list pointer
*     lList **alpp            - AN_Type list pointer 
*     char *file_name         - name of an alias file 
*
*  RESULT
*     static int - error state
*        -1 - Error
*         0 - OK 
*
*  NOTES
*     MT-NOTE: path_alias_read_from_file() is MT safe
******************************************************************************/
static int path_alias_read_from_file(lList **path_alias_list, lList **alpp,
                                     char *file_name)
{
   FILE *fd;
   char buf[10000];
   char err[MAX_STRING_SIZE];
   char origin[CL_MAXHOSTLEN];
   char submit_host[CL_MAXHOSTLEN];
   char exec_host[CL_MAXHOSTLEN];
   char translation[CL_MAXHOSTLEN];
   lListElem *pal;
   SGE_STRUCT_STAT sb;
   int ret = 0;

   DENTER(GDI_LAYER, "path_alias_read_from_file");

   if (!path_alias_list || !file_name) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return -1;
   }

   if ((SGE_STAT(file_name, &sb) != 0) && (errno == ENOENT)) {
      DEXIT;
      return 0;
   }    

   if (!(fd=(fopen(file_name, "r")))) {
      DEXIT;
      return -1;
   }

   while (fgets(buf, sizeof(buf), fd)) {
      char *crp;

      /* strip \n */
      if ((crp = strchr(buf, (int)'\n')))
         *crp = '\0';

      DPRINTF(("Path Alias: >%s<\n",buf));

      /* skip empty lines and comments */
      if (!strlen(buf) || (*buf == '#' )) {
         continue;
      }

      /*
       * reset
       */
      origin[0]      = '\0';
      submit_host[0] = '\0';
      exec_host[0]   = '\0';
      translation[0] = '\0';   

      /* See CL_MAXHOSTNAMELEN_LENGTH for width */
      sscanf(buf, "%256s %256s %256s %256s",
	     origin, submit_host, exec_host, translation);

      /*
       * check for correctness of path alias file
       */
      if (*origin == '\0' || *submit_host == '\0' || *exec_host == '\0' ||
            *translation == '\0') {
         snprintf(err, sizeof(err),
                  MSG_ALIAS_INVALIDSYNTAXOFPATHALIASFILEX_S, file_name);
         answer_list_add(alpp, err, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
         ret = -1;
         break;
      }

      /*
       * compress multiple slashes to one slash
       */
      sge_compress_slashes(origin);
      sge_compress_slashes(translation);
            
      
      pal = lAddElemStr(path_alias_list, PA_origin, origin, PA_Type);
      
      if (!pal) {
         answer_list_add(alpp, MSG_SGETEXT_NOMEM, STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         ret = -1;
         break;
      }

      /*
       * set the values of the element
       */
      lSetHost(pal, PA_submit_host, submit_host);
      if ( strcmp(submit_host, "*") && (sge_resolve_host(pal, PA_submit_host) != CL_RETVAL_OK)) {
         SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_SGETEXT_CANTRESOLVEHOST_S, submit_host));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = -1;
         break;
      }
      lSetHost(pal, PA_exec_host, exec_host);
      lSetString(pal, PA_translation, translation);

   } /* while (fgets) */

   FCLOSE(fd);

   DEXIT;
   return ret;
FCLOSE_ERROR:
   SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_FILE_NOCLOSE_SS, file_name,
                  strerror(errno)));
   answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
   return -1;
}

/****** sgeobj/path_alias/path_alias_list_initialize() ************************
*  NAME
*     path_alias_list_initialize() -- initialize path_alias_list 
*
*  SYNOPSIS
*     int path_alias_list_initialize(lList **path_alias_list, 
*                                    lList **alpp, 
*                                    const char *user, 
*                                    const char *host) 
*
*  FUNCTION
*     Intitialize "path_alias_list" according to the different
*     path aliasing files. 
*
*     The following files will be used if available:
*
*        $SGE_ROOT/$CELL/common/sge_aliases
*        $HOME/.sge_aliases 
*
*  INPUTS
*     lList **path_alias_list - PA_Type list pointer
*     lList **alpp            - AN_Type list pointer 
*     const char *user        - username
*     const char *host        - hostname 
*
*  RESULT
*     int - return state
*        -1 - error
*         0 - OK
*
*  NOTES
*     MT-NOTE: path_alias_list_initialize() is MT safe
******************************************************************************/
int path_alias_list_initialize(lList **path_alias_list, 
                               lList **alpp,
                               const char *cell_root,
                               const char *user,
                               const char *host) 
{
   char filename[2][SGE_PATH_MAX];
   char err[MAX_STRING_SIZE];
   DENTER(TOP_LAYER, "path_alias_list_initialize");

   /* 
    * find names of different sge_path_alias files:
    *    global
    *    home directory
    */
   {
      struct passwd *pwd;
      struct passwd pw_struct;
      char *buffer;
      int size;

      size = get_pw_buffer_size();
      buffer = sge_malloc(size);
      pwd = sge_getpwnam_r(user, &pw_struct, buffer, size);

      if (!pwd) {
         snprintf(err, sizeof(err), MSG_USER_INVALIDNAMEX_S, user);
         answer_list_add(alpp, err, STATUS_ENOSUCHUSER, ANSWER_QUALITY_ERROR);
         sge_free(&buffer);
         DRETURN(-1);
      }
      if (!pwd->pw_dir) {
         snprintf(err, sizeof(err), MSG_USER_NOHOMEDIRFORUSERX_S, user);
         answer_list_add(alpp, err, STATUS_EDISK, ANSWER_QUALITY_ERROR);
         sge_free(&buffer);
         DRETURN(-1);
      }
      snprintf(filename[0], SGE_PATH_MAX, "%s/%s", cell_root, PATH_ALIAS_COMMON_FILE);
      snprintf(filename[1], SGE_PATH_MAX, "%s/%s", pwd->pw_dir, PATH_ALIAS_HOME_FILE);

      sge_free(&buffer);
   }

   /*
    * read files
    */
   {
      int i;

      for (i=0; i<2; i++) {
         if (path_alias_read_from_file(path_alias_list, 
                                       alpp, filename[i]) != 0) {
            snprintf(err, sizeof(err), MSG_ALIAS_CANTREAD_SS,
                     filename[i], strerror(errno));
            answer_list_add(alpp, err, STATUS_EDISK, ANSWER_QUALITY_ERROR);
            DEXIT;
            return -1;
         }
      }
   }

   /*
    * remove the unnecessary hosts from the list
    */
   {
      lCondition *where = NULL;

      where = lWhere("%T(%I == %s || %I == %s)", PA_Type, 
                     PA_submit_host, "*", PA_submit_host, host);
      *path_alias_list = lSelectDestroy(*path_alias_list, where);
      lFreeWhere(&where);
   }

   DEXIT;
   return 0;
}

/****** sgeobj/path_alias/path_alias_list_get_path() **************************
*  NAME
*     path_alias_list_get_path() -- map path according alias table 
*
*  SYNOPSIS
*     void path_alias_list_get_path(const lList *path_aliases,
*                                   lList **alpp,
*                                   const char *inpath,
*                                   const char *myhost,
*                                   char *outpath,
*                                   int outmax)
*
*  FUNCTION
*     "path_aliases" is used to map "inpath" for the host "myhost"
*     into its alias path which will be written into the buffer 
*     "outpath" of size "outmax". 
*
*  INPUTS
*     const lList *path_aliases - alias table (PA_Type) 
*     lList **alpp              - AN_Type list pointer 
*     const char *inpath        - input path 
*     const char *myhost        - hostname 
*     char *outpath             - result path 
*     int outmax                - size of "outpath" 
*
*  NOTES
*     MT-NOTE: path_alias_list_get_path() is MT safe
*******************************************************************************/
void path_alias_list_get_path(const lList *path_aliases, lList **alpp _UNUSED,
                              const char *inpath, const char *myhost,
                              dstring *outpath)
{
   lListElem *pap;
   const char *origin;
   const char *translation;
   const char *exec_host;
   dstring the_path = DSTRING_INIT;
 
   DENTER(TOP_LAYER, "path_alias_list_get_path");

   sge_dstring_copy_string(outpath, inpath);
   sge_dstring_copy_dstring(&the_path, outpath); 

   if (path_aliases && lGetNumberOfElem(path_aliases) > 0) { 
      for_each(pap, path_aliases) {
         size_t orign_str_len = 0; 
         origin = lGetString(pap, PA_origin);
         orign_str_len = strlen(origin);
         exec_host = lGetHost(pap, PA_exec_host);
         translation = lGetString(pap, PA_translation);

         if (strncmp(origin, sge_dstring_get_string(&the_path), 
             orign_str_len )) {
            /* path leaders aren't the same ==> no match */
            continue;
         }
 
         /* the paths are ok, what about the exec hosts ? */
         /* if exec_host is a '*' we have a match */
         if (*exec_host != '*') {
            /* no '*', so we have to look closer   */
            /* resolv the exec host from the alias */
            if (sge_resolve_host(pap, PA_exec_host) != CL_RETVAL_OK) {
               ERROR((SGE_EVENT, MSG_SGETEXT_CANTRESOLVEHOST_S, exec_host)); 
               continue;
            }
            exec_host = lGetHost(pap, PA_exec_host);

            /* and compare it to the executing host */
            if (sge_hostcmp(exec_host, myhost))
               continue;

         }
 
         /* copy the alias as leading part of cwd */
         sge_dstring_copy_string(outpath, translation);
 
         /* now append the trailer of the original cwd */
         {  
            const char *path = sge_dstring_get_string(&the_path);
            sge_dstring_append(outpath, path + orign_str_len );
         }

         DPRINTF(("Path "SFQ" has been aliased to "SFQ"\n", inpath, sge_dstring_get_string(outpath))); 
 
         /* and we have to start all over again for subsequent aliases */
         sge_dstring_copy_dstring(&the_path, outpath);
      }
   } else {
      DPRINTF(("\"path_aliases\" contains no elements\n"));
   }

   sge_dstring_free(&the_path);

   DRETURN_VOID;

}

/****** sge_path_alias/path_verify() *******************************************
*  NAME
*     path_verify() -- verify a path string
*
*  SYNOPSIS
*     bool 
*     path_verify(const char *path, lList **answer_list, const char *name,
*                 bool absolute)
*
*  FUNCTION
*     Verifies if a path string has valid contents.
*
*  INPUTS
*     const char *path    - the string to verify
*     lList **answer_list - answer list to pass back error messages
*     const char *name    - name of the path to check, e.g. "prolog"
*                           for error output
*     bool absolute       - does it have to be an absolute path?
*
*  RESULT
*     bool - true on success,
*            false on error with error message in answer_list
*
*  NOTES
*     MT-NOTE: path_verify() is MT safe 
*******************************************************************************/
bool 
path_verify(const char *path, lList **answer_list, const char *name, bool absolute)
{
   bool ret = true;

   if (path == NULL || *path == '\0') {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR, 
                              "%s", MSG_PATH_ALIAS_INVALID_PATH);
      ret = false;
   }

   if (ret) {
      if (strlen(path) > SGE_PATH_MAX) {
         answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR, 
                                 MSG_PATH_TOOLONG_I, SGE_PATH_MAX);
         ret = false;
      }
   }

   /* check for absolute path */
   if (absolute) {
      if (ret && path[0] != '/') {
         answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR, 
                                 MSG_CONF_THEPATHGIVENFORXMUSTSTARTWITHANY_S, name);
         ret = false;
      }
   }

   /* 
    * TODO: we have to do additional checks here 
    * we could use a variant of verify_str_key, using different valid character definition
    * verify_str_key will be extended to do length check
    * shall we also check for absolute vs. relative path?
    * filename vs. directory path?
    */

   return ret;
}

/****** sge_path_alias/path_alias_verify() *************************************
*  NAME
*     path_alias_verify() -- verify path alias list
*
*  SYNOPSIS
*     bool 
*     path_alias_verify(const lList *path_aliases, lList **answer_list) 
*
*  FUNCTION
*     Verify a path alias list, as sent with job or PE task start orders
*     to sge_execd.
*
*  INPUTS
*     const lList *path_aliases - path alias list
*     lList **answer_list       - answer list to pass back error messages
*
*  RESULT
*     bool - true on success,
*            false on error with error message in answer_list
*
*  NOTES
*     MT-NOTE: path_alias_verify() is MT safe 
*
*  SEE ALSO
*     sge_path_alias/path_verify()
*******************************************************************************/
bool 
path_alias_verify(const lList *path_aliases, lList **answer_list)
{
   bool ret = true;
   const lListElem *ep;

   for_each (ep, path_aliases) {
      /* 
       * PA_origin and PA_translation may not be NULL or empty string 
       * they have to be valid paths.
       */
      if (ret) {
         ret = path_verify(lGetString(ep, PA_origin), answer_list, "path_alias: origin", false);
      }
      if (ret) {
         ret = path_verify(lGetString(ep, PA_translation), answer_list, "path_alias: translation", false);
      }

       /*
       * PA_submit_host and PA_exec_host have to be either '*' or a valid
       * hostname (no need to resolve them).
       */
      if (ret) {
         ret = verify_host_name(answer_list, lGetHost(ep, PA_submit_host));
      }
      if (ret) {
         ret = verify_host_name(answer_list, lGetHost(ep, PA_exec_host));
      }

      if (!ret) {
         break;
      }
   }

   return ret;
}

/****** sge_path_alias/path_list_verify() **************************************
*  NAME
*     path_list_verify() -- verify a path list
*
*  SYNOPSIS
*     bool 
*     path_list_verify(const lList *path_list, lList **answer_list, 
*                      const char *name) 
*
*  FUNCTION
*     Verify a path list, e.g. the path specification in JB_stdout_path_list,
*     coming from a qsub -o <path_list>.
*
*  INPUTS
*     const lList *path_list - the path list to verify
*     lList **answer_list    - answer list to pass back error messages
*     const char *name       - name of the checked attribute for error output
*
*  RESULT
*     bool - true: everything ok, else false
*
*  NOTES
*     MT-NOTE: path_list_verify() is MT safe 
*******************************************************************************/
bool 
path_list_verify(const lList *path_list, lList **answer_list, const char *name)
{
   bool ret = true;
   const lListElem *ep;

   for_each (ep, path_list) {
      const char *host;

      ret = path_verify(lGetString(ep, PN_path), answer_list, name, false);
      if (!ret) {
         break;
      }

      host = lGetHost(ep, PN_host);
      if (host != NULL) {
         ret = verify_host_name(answer_list, host);
         if (!ret) {
            break;
         }
      }

      host = lGetHost(ep, PN_file_host);
      if (host != NULL) {
         ret = verify_host_name(answer_list, host);
         if (!ret) {
            break;
         }
      }
   }

   return ret;
}
