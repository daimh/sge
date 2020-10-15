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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  
#include <pwd.h>
#include <sys/types.h>

#include "uti/sge_rmon.h"
#include "uti/setup_path.h"
#include "uti/sge_unistd.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_io.h"
#include "uti/sge_prog.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/parse.h"

#include "parse_job_cull.h"
#include "parse_qsub.h"
#include "read_defaults.h"
#include "sge_options.h"
#include "msg_common.h"
#include "msg_clients_common.h"

static char *get_cwd_defaults_file_path (lList **answer_list);
static void append_opts_from_default_files (u_long32 prog_number,
                                            lList **pcmdline, 
                                            lList **answer_list,
                                            char **envp,
                                            char **def_files);

/****** sge/opt/opt_list_append_opts_from_default_files() *********************
*  NAME
*     opt_list_append_opts_from_default_files() -- parse default files 
*
*  SYNOPSIS
*     void opt_list_append_opts_from_default_files(
*                                         lList **pcmdline, 
*                                         lList **answer_list
*                                         char **envp) 
*
*  FUNCTION
*     This function reads the 3 defaults files if they exist and parses them
*     into an options list. 
*
*  INPUTS
*     lList **pcmdline - pointer to SPA_Type list, if list is NULL, it is
*                        created if the files contain any options 
*     lList* - answer list, AN_Type or NULL if everything ok
*        possible errors:
*           STATUS_ENOSUCHUSER - could not retrieve passwd info on me.user_name
*           STATUS_EDISK       - home directory for user is missing or cwd 
*                                cannot be read or file could not be opened 
*                                (is just a warning)
*           STATUS_EEXIST      - (parse_script_file), (is just a warning)
*           STATUS_EUNKNOWN    - (parse_script_file), error opening or 
*                                reading from existing file, (is just a warning)
*                                plus all other error stati returned by 
*                                parse_script_file, see there
*     char **envp      - environment pointer 
*
*  NOTES
*     MT-NOTE: opt_list_append_opts_from_default_files() is MT safe
*******************************************************************************/
void opt_list_append_opts_from_default_files(u_long32 prog_number, 
                                             const char* cell_root,
                                             const char* user,
                                             lList **pcmdline, 
                                             lList **answer_list,
                                             char **envp) 
{
   dstring req_file = DSTRING_INIT;
   char *def_files[3 + 1];

   DENTER(TOP_LAYER, "opt_list_append_opts_from_default_files");

   lFreeList(answer_list);

   /* the sge root defaults file */
   get_root_file_path(&req_file, cell_root, SGE_COMMON_DEF_REQ_FILE);
   def_files[0] = strdup(sge_dstring_get_string(&req_file));

   /*
    * the defaults file in the user's home directory
    */
   get_user_home_file_path(&req_file, SGE_HOME_DEF_REQ_FILE, user, answer_list);
   def_files[1] = strdup(sge_dstring_get_string(&req_file));

   /*
    * the defaults file in the current working directory
    */
   def_files[2] = get_cwd_defaults_file_path(answer_list);


   def_files[3] = NULL;

   /*
    * now read all the defaults files, unaware of where they came from
    */
   append_opts_from_default_files(prog_number, pcmdline,  answer_list, envp, def_files); /* MT-NOTE !!!! */
    
   sge_dstring_free(&req_file);

   DRETURN_VOID;
}

/****** read_defaults/get_user_home_file_path() *****************************
*  NAME
*     get_user_home_file_path() -- get absolut path name to file in user
*                                  home
*
*  SYNOPSIS
*     char *get_user_home_file_path (lList **answer_list) 
*
*  FUNCTION
*     This function returns the path to the file in the user's home
*     directory
*
*  INPUTS
*     dstring              - computed absoult filename
*     const char *filename - file name
*     const char *user     - user name
*     lList* - answer list, AN_Type or NULL if everything ok
*        possible errors:
*           STATUS_ENOSUCHUSER - could not retrieve passwd info on me.user_name
*           STATUS_EDISK       - home directory for user is missing or cwd 
*                                cannot be read or file could not be opened 
*                                (is just a warning)
*           STATUS_EEXIST      - (parse_script_file), (is just a warning)
*           STATUS_EUNKNOWN    - (parse_script_file), error opening or 
*                                reading from existing file, (is just a warning)
*                                plus all other error stati returned by 
*                                parse_script_file, see there
*
*  RETURNS
*     bool - true or false
*
*     MT-NOTE: get_user_home_file_path() is MT safe
*******************************************************************************/
bool get_user_home_file_path(dstring *absolut_filename, const char *filename, const char *user, lList **answer_list)
{
   bool ret = false;

   DENTER (TOP_LAYER, "get_user_home_file_path");

   if (absolut_filename != NULL && filename != NULL) {

      sge_dstring_clear(absolut_filename);

      if (get_user_home(absolut_filename, user, answer_list)) {
         sge_dstring_append(absolut_filename, "/");
         sge_dstring_append(absolut_filename, filename); 
         ret = true;
      }
   }

   DRETURN(ret);
}

/****** sge/opt/get_cwd_defaults_file_path() ***********************************
*  NAME
*     get_cwd_defaults_file_path() -- find cwd default file path
*
*  SYNOPSIS
*     char *get_cwd_defaults_file_path () 
*
*  FUNCTION
*     This function returns the path of the defaults file in the current working
*     directory
*
*  INPUTS
*     lList* - answer list, AN_Type or NULL if everything ok
*        possible errors:
*           STATUS_ENOSUCHUSER - could not retrieve passwd info on me.user_name
*           STATUS_EDISK       - home directory for user is missing or cwd 
*                                cannot be read or file could not be opened 
*                                (is just a warning)
*           STATUS_EEXIST      - (parse_script_file), (is just a warning)
*           STATUS_EUNKNOWN    - (parse_script_file), error opening or 
*                                reading from existing file, (is just a warning)
*                                plus all other error stati returned by 
*                                parse_script_file, see there
*     char * - cwd defaults file name with absolute path
*
*   MT-NOTE: get_cwd_defaults_file_path() is MT safe
*******************************************************************************/
static char *get_cwd_defaults_file_path(lList **answer_list)
{
   char cwd[SGE_PATH_MAX + 1];
   char str[MAX_STRING_SIZE];
   char *file = NULL;
   
   DENTER (TOP_LAYER, "get_cwd_defaults_file_name");

   if (!getcwd(cwd, sizeof(cwd))) {
      snprintf(str, sizeof(str), SFNMAX, MSG_FILE_CANTREADCURRENTWORKINGDIR);
      answer_list_add(answer_list, str, STATUS_EDISK, ANSWER_QUALITY_ERROR);
   }
   
   file = sge_malloc(strlen(cwd) + strlen(SGE_HOME_DEF_REQ_FILE) + 2);
   
   strcpy(file, cwd);
   if (*file && (file[strlen(file) - 1] != '/')) {
      strcat(file, "/");
   }
   strcat(file, SGE_HOME_DEF_REQ_FILE);
   
   DRETURN(file);
}

/****** sge/opt/append_opts_from_default_files() *******************************
*  NAME
*     append_opts_from_default_files() -- parse default files 
*
*  SYNOPSIS
*     void append_opts_from_default_files(lList **pcmdline, 
*                                         lList **answer_list
*                                         char **envp,
*                                         char *def_files) 
*
*  FUNCTION
*     This function reads the defaults files pointed to by def_files[] if they
*     exist and parses them into an options list. 
*
*  INPUTS
*     lList **pcmdline - pointer to SPA_Type list, if list is NULL, it is
*                        created if the files contain any options 
*     lList* - answer list, AN_Type or NULL if everything ok
*        possible errors:
*           STATUS_ENOSUCHUSER - could not retrieve passwd info on me.user_name
*           STATUS_EDISK       - home directory for user is missing or cwd 
*                                cannot be read or file could not be opened 
*                                (is just a warning)
*           STATUS_EEXIST      - (parse_script_file), (is just a warning)
*           STATUS_EUNKNOWN    - (parse_script_file), error opening or 
*                                reading from existing file, (is just a warning)
*                                plus all other error stati returned by 
*                                parse_script_file, see there
*     char **envp      - environment pointer 
*     char **def_files - paths to default files
*
*******************************************************************************/
static void append_opts_from_default_files(u_long32 prog_number, 
                                           lList **pcmdline, 
                                           lList **answer_list,
                                           char **envp,
                                           char **def_files) 
{
   lList *alp;
   lListElem *aep;
   char **pstr;
   char **ppstr;
   SGE_STRUCT_STAT buf;
   int do_exit = 0;
   
   DENTER(TOP_LAYER, "append_opts_from_default_files");

   for (pstr = def_files; *pstr; pstr++) {
      int already_read;

      if (SGE_STAT(*pstr, &buf)<0) {
         DPRINTF(("-- defaults file %s does not exist\n", *pstr));
         continue;
      }

      already_read = 0; 
      for (ppstr = def_files; *ppstr != *pstr; ppstr++) {
         if (!sge_filecmp(*ppstr, *pstr)) {
            DPRINTF(("-- skipping %s as defaults file - already read as %s\n", 
               *pstr, *ppstr));
            already_read = 1; 
            break;
         }
      }
      if (already_read) {
         continue;
      }
      DPRINTF(("-- defaults file: %s\n", *pstr));

      alp = parse_script_file(prog_number, *pstr, "", pcmdline, envp, 
         FLG_HIGHER_PRIOR | FLG_USE_NO_PSEUDOS);

      for_each(aep, alp) {
         u_long32 status;
         answer_quality_t quality;

         status = lGetUlong(aep, AN_status);
         quality = (answer_quality_t)lGetUlong(aep, AN_quality);

         if (quality == ANSWER_QUALITY_ERROR) {
            DPRINTF(("%s", lGetString(aep, AN_text)));
            if (status == STATUS_EDISK) {
               /*
               ** we turn this error into a warning here
               */
               quality = ANSWER_QUALITY_WARNING;
            } else {
               do_exit = 1;
            }
         } else {
            DPRINTF(("Warning: Error: %s\n", lGetString(aep, AN_text)));
         }
         answer_list_add(answer_list, lGetString(aep, AN_text), status, 
                         quality);
      }
      lFreeList(&alp);

      if (do_exit) {
         for (pstr = def_files; *pstr; pstr++) {
            sge_free(pstr);
         }
         DRETURN_VOID;
      }
   }

   for (pstr = def_files; *pstr; pstr++) {
      sge_free(pstr);
   }
   
   DRETURN_VOID;
}
/****** sge/opt/opt_list_append_opts_from_qsub_cmdline() **********************
*  NAME
*     opt_list_append_opts_from_qsub_cmdline() -- parse opts from cmd line 
*
*  SYNOPSIS
*     void opt_list_append_opts_from_qsub_cmdline(lList **opts_cmdline, 
*                                                 lList **answer_list, 
*                                                 char **argv, 
*                                                 char **envp) 
*
*  FUNCTION
*     Parse options from the qsub commandline given by "argv" and store
*     the parsed objects in "opts_cmdline". If an error occures store
*     the error/warning messages in the "answer_list". 
*     "envp" is a pointer to the process environment.
*     
*
*  INPUTS
*     lList **opts_cmdline - command line options 
*     lList **answer_list  - AN_Type list 
*     char **argv          - Argumente 
*     char **envp          - Environment 
*
*  RESULT
*     void - None
*******************************************************************************/
void opt_list_append_opts_from_qsub_cmdline(u_long32 prog_number, 
                                            lList **opts_cmdline,
                                            lList **answer_list,
                                            char **argv,
                                            char **envp)
{
   lFreeList(answer_list);
   *answer_list = cull_parse_cmdline(prog_number, argv, envp, opts_cmdline, FLG_USE_PSEUDOS);
}

/****** sge/opt/opt_list_append_opts_from_qalter_cmdline() ********************
*  NAME
*     opt_list_append_opts_from_qalter_cmdline() -- parse opts from cmd line 
*
*  SYNOPSIS
*     void opt_list_append_opts_from_qalter_cmdline(lList **opts_cmdline, 
*                                                   lList **answer_list, 
*                                                   char **argv, 
*                                                   char **envp) 
*
*  FUNCTION
*     Parse options from the qalter commandline given by "argv" and store
*     the parsed objects in "opts_cmdline". If an error occures store
*     the error/warning messages in the "answer_list". 
*     "envp" is a pointer to the process environment.
*     
*
*  INPUTS
*     lList **opts_cmdline - command line options 
*     lList **answer_list  - AN_Type list 
*     char **argv          - Argumente 
*     char **envp          - Environment 
*
*  RESULT
*     void - None
*******************************************************************************/
void opt_list_append_opts_from_qalter_cmdline(u_long32 prog_number, 
                                              lList **opts_cmdline,
                                              lList **answer_list,
                                              char **argv,
                                              char **envp)
{
   lFreeList(answer_list);
   *answer_list = cull_parse_cmdline(prog_number, argv, envp, opts_cmdline, 
                                     FLG_USE_PSEUDOS | FLG_QALTER);
}

/****** sge/opt/opt_list_append_opts_from_script() ****************************
*  NAME
*     opt_list_append_opts_from_script() -- parse opts from scriptfile 
*
*  SYNOPSIS
*     void opt_list_append_opts_from_script(lList **opts_scriptfile, 
*                                           lList **answer_list, 
*                                           const lList *opts_cmdline, 
*                                           char **envp) 
*
*  FUNCTION
*     This function parses the commandline options which are embedded
*     in scriptfile (jobscript) and stores the parsed objects in
*     opts_scriptfile. The filename of the scriptfile has to be
*     contained in the list "opts_cmdline" which has been previously i
*     created with opt_list_append_opts_from_*_cmdline(). "answer_list"
*     will be used to store error/warning messages.
*     "envp" is a pointer to the process environment.
*
*  INPUTS
*     lList **opts_scriptfile   - embedded command line options 
*     lList **answer_list       - AN_Type list 
*     const lList *opts_cmdline - Argumente 
*     char **envp               - Environment 
*
*  RESULT
*     void - None
*******************************************************************************/
void opt_list_append_opts_from_script(u_long32 prog_number, 
                                      lList **opts_scriptfile, 
                                      lList **answer_list,
                                      const lList *opts_cmdline,
                                      char **envp) 
{ 
   lListElem *script_option = NULL;
   lListElem *c_option = NULL;
   const char *scriptfile = NULL;
   const char *prefix = NULL;
 
   script_option = lGetElemStr(opts_cmdline, SPA_switch, STR_PSEUDO_SCRIPT);
   if (script_option != NULL) {
      scriptfile = lGetString(script_option, SPA_argval_lStringT);
   }
   c_option = lGetElemStr(opts_cmdline, SPA_switch, "-C");
   if (c_option != NULL) {
      prefix = lGetString(c_option, SPA_argval_lStringT);
   } else {
      prefix = default_prefix;
   }
   lFreeList(answer_list);
   *answer_list = parse_script_file(prog_number, scriptfile, prefix, opts_scriptfile, 
                                    envp, FLG_DONT_ADD_SCRIPT);
}

/****** sge/opt/opt_list_append_opts_from_script_path() ************************
*  NAME
*     opt_list_append_opts_from_script_path() -- parse opts from scriptfile 
*
*  SYNOPSIS
*     void opt_list_append_opts_from_script_path(lList **opts_scriptfile,
*                                           char *path, lList **answer_list, 
*                                           const lList *opts_cmdline, 
*                                           char **envp) 
*
*  FUNCTION
*     This function parses the commandline options which are embedded
*     in scriptfile (jobscript) and stores the parsed objects in
*     opts_scriptfile. The filename of the scriptfile has to be
*     contained in the list "opts_cmdline" which has been previously i
*     created with opt_list_append_opts_from_*_cmdline(). If the filename of
*     the scriptfile is not an absolute path, "path" will be prepended to it.
*     "answer_list" will be used to store error/warning messages.
*     "envp" is a pointer to the process environment.
*
*  INPUTS
*     lList **opts_scriptfile   - embedded command line options 
*     const char *path          - the root path for the script file
*     lList **answer_list       - AN_Type list 
*     const lList *opts_cmdline - Argumente 
*     char **envp               - Environment 
*
*  RESULT
*     void - None
*******************************************************************************/
void opt_list_append_opts_from_script_path(u_long32 prog_number,
                                           lList **opts_scriptfile,
                                           const char *path,
                                           lList **answer_list,
                                           const lList *opts_cmdline,
                                           char **envp)
{ 
   lListElem *script_option = NULL;
   lListElem *c_option = NULL;
   const char *scriptfile = NULL;
   char *scriptpath = NULL;
   const char *prefix = NULL;
 
   script_option = lGetElemStr(opts_cmdline, SPA_switch, STR_PSEUDO_SCRIPT);
   
   if (script_option != NULL) {
      scriptfile = lGetString(script_option, SPA_argval_lStringT);
      
      /* If the scriptfile path isn't absolute (which includes starting with
         $HOME), make it absolute relative to the given path.
         If the script or path is NULL, let parse_script_file() catch it. */
      if ((scriptfile != NULL) && (path != NULL) && (scriptfile[0] != '/') &&
          (strncmp (scriptfile, "$HOME/", 6) != 0) &&
          (strcmp (scriptfile, "$HOME") != 0)) {
         /* Malloc space for the path, the filename, the \0, and perhaps a / */
         scriptpath = sge_malloc(sizeof(char) * (strlen(path) + strlen(scriptfile) + 2));
         strcpy (scriptpath, path);
         
         /* If the last character is not a slash, add one. */
         if (scriptpath[strlen (scriptpath) - 1] != '/') {
            strcat (scriptpath, "/");
         }
         
         strcat (scriptpath, scriptfile);
      } else if (scriptfile) {
         scriptpath = strdup (scriptfile);
      }
   }
   
   c_option = lGetElemStr(opts_cmdline, SPA_switch, "-C");
   
   if (c_option != NULL) {
      prefix = lGetString(c_option, SPA_argval_lStringT);
   } else {
      prefix = default_prefix;
   }
   
   lFreeList(answer_list);
   
   *answer_list = parse_script_file(prog_number, scriptpath, prefix, opts_scriptfile, 
                                    envp, FLG_DONT_ADD_SCRIPT);
   
   sge_free(&scriptpath);
}

/****** sge/opt/opt_list_merge_command_lines() ********************************
*  NAME
*     opt_list_merge_command_lines() -- merge commandlines together
*
*  SYNOPSIS
*     void opt_list_merge_command_lines(lList **opts_all, 
*                                       lList **opts_defaults, 
*                                       lList **opts_scriptfile, 
*                                       lList **opts_cmdline) 
*
*  FUNCTION
*     Merge "opts_defaults", "opts_scriptfile" and "opts_cmdline" into
*     "opts_all".
*
*     Options to a sge submit can come from different sources:
*      - default settings (sge/sge_request)
*      - special comments in scriptfiles (override default settings)
*      - command line options (override default settings and special 
*        comments) 
*
*  INPUTS
*     lList **opts_all        - destination commandline 
*     lList **opts_defaults   - opts from default files 
*     lList **opts_scriptfile - opts from the script 
*     lList **opts_cmdline    - commandline options 
*
*  RESULT
*     void - None
*******************************************************************************/
void opt_list_merge_command_lines(lList **opts_all,
                                  lList **opts_defaults,
                                  lList **opts_scriptfile,
                                  lList **opts_cmdline)
{
   /*
    * Order is very important here
    */
   if (*opts_defaults != NULL) {
      if (*opts_all == NULL) {
         *opts_all = *opts_defaults;
      } else {
         lAddList(*opts_all, opts_defaults);
      }
      *opts_defaults = NULL;
   }
   if (*opts_scriptfile != NULL) {
      if (*opts_all == NULL) {
         *opts_all = *opts_scriptfile;
      } else {
         /* Override the queue (-q) values from defaults */
         lOverrideStrList(*opts_all, *opts_scriptfile, SPA_switch, "-q");
      }
      *opts_scriptfile = NULL;
   }
   if (*opts_cmdline != NULL) {
      if (*opts_all == NULL) {
         *opts_all = *opts_cmdline;
      } else {
         /* Override queue (-q) values from both defaults and scriptfile */
         lOverrideStrList(*opts_all, *opts_cmdline, SPA_switch, "-q");
      }
      *opts_cmdline = NULL;
   }

   /* If -ar was requested add -w if it was not explicitly set */
   if (lGetElemStr(*opts_all, SPA_switch, "-ar") != NULL) {
      if (lGetElemStr(*opts_all, SPA_switch, "-w") == NULL) {
         lListElem *ep_opt = sge_add_arg(opts_all, r_OPT, lIntT, "-w", "e");
         lSetInt(ep_opt, SPA_argval_lIntT, ERROR_VERIFY);
      }
   }
}

/****** sge/opt/opt_list_has_X() **********************************************
*  NAME
*     opt_list_has_X() -- is a certain option contained in list?
*
*  SYNOPSIS
*     bool opt_list_has_X(lList *opts, const char *option) 
*
*  FUNCTION
*     This function returns true (1) if the given 'option' 
*     (e.g. "-help") is contained in the list 'opts'.
*
*  INPUTS
*     lList *opts        - SPA_Type list 
*     const char *option - switch name  
*
*  RESULT
*     bool - found switch?
*        true - yes
*        false - no
*
*  SEE ALSO
*     sge/opt/opt_list_is_X_true()
*******************************************************************************/
bool opt_list_has_X(lList *opts, const char *option) 
{
   lListElem *opt;
   bool ret = false;

   opt = lGetElemStr(opts, SPA_switch, option);
   if (opt != NULL) {
      ret = true;
   }
   return ret;
}

/****** sge/opt/opt_list_is_X_true() ******************************************
*  NAME
*     opt_list_is_X_true() -- check the state of a boolean switch 
*
*  SYNOPSIS
*     bool opt_list_is_X_true(lList *opts, const char *option) 
*
*  FUNCTION
*     This function returns true (1) if the given 'option'
*     (e.g. "-b") is contained in the list 'opts' and if
*     it was set to 'true'. If the value of the boolean switch
*     is false than the function will also return false (0).
*
*  INPUTS
*     lList *opts        - SPA_Type list 
*     const char *option - switch name 
*
*  RESULT
*     bool - found switch with value 'true'
*        true - yes
*        false - no 
*
*  SEE ALSO
*     sge/opt/opt_list_has_X()
******************************************************************************/
bool opt_list_is_X_true(lList *opts, const char *option) 
{
   lListElem *opt;
   bool ret = false;

   opt = lGetElemStr(opts, SPA_switch, option);
   if (opt != NULL) {
      ret = (lGetInt(opt, SPA_argval_lIntT) == 1) ? true : false;
   }
   return ret;
}

/****** read_defaults/get_root_file_path() *************************************
*  NAME
*     get_root_file_path() -- creates absolute filename for file in SGE_ROOT
*
*  SYNOPSIS
*     const char* get_root_file_path(dstring *absolut_filename, const char 
*     *cell_root, const char *filename) 
*
*  FUNCTION
*     Sets the absolut filename of a file in SGE_ROOT in the given dstring
*
*  INPUTS
*     dstring *absolut_filename - created absolut filename
*     const char *cell_root     - sge root patch
*     const char *filename      - file name
*
*  RESULT
*     const char* - pointer to filename in absolut_filename
*
*  NOTES
*     MT-NOTE: get_root_file_path() is MT safe 
*
*******************************************************************************/
const char *get_root_file_path(dstring *absolut_filename, const char *cell_root, const char *filename)
{
   DENTER (TOP_LAYER, "get_root_file_path");

   sge_dstring_sprintf(absolut_filename, "%s/%s", cell_root, filename);

   DRETURN(sge_dstring_get_string(absolut_filename));
}

/****** read_defaults/get_user_home() ******************************************
*  NAME
*     get_user_home() -- get absoult filename in users home dir
*
*  SYNOPSIS
*     bool get_user_home(dstring *home_dir, const char *user, lList 
*     **answer_list) 
*
*  FUNCTION
*     Sets the absolut filename of a file in the users home directory
*
*  INPUTS
*     dstring *home_dir   - created absolut filename
*     const char *user    - user
*     lList **answer_list - answer list
*
*  RESULT
*     bool - true on success
*            false on error
*
*  NOTES
*     MT-NOTE: get_user_home() is MT safe 
*
*******************************************************************************/
bool get_user_home(dstring *home_dir, const char *user, lList **answer_list)
{
   bool ret = true;

   DENTER(TOP_LAYER, "get_user_home");

   if (home_dir != NULL) {
      struct passwd *pwd;
      struct passwd pw_struct;
      char *buffer;
      int size;

      size = get_pw_buffer_size();
      buffer = sge_malloc(size);
      pwd = sge_getpwnam_r(user, &pw_struct, buffer, size);
      if (!pwd) {
         answer_list_add_sprintf(answer_list, STATUS_ENOSUCHUSER, 
                         ANSWER_QUALITY_ERROR, MSG_USER_INVALIDNAMEX_S, user);
         ret = false;
      }
      if (ret && !pwd->pw_dir) {
         answer_list_add_sprintf(answer_list, STATUS_EDISK, ANSWER_QUALITY_ERROR,
                                 MSG_USER_NOHOMEDIRFORUSERX_S, user);
         ret = false;
      } 
      if (ret) {
         sge_dstring_copy_string(home_dir, pwd->pw_dir);
      }
      sge_free(&buffer);
   } else {
      /* should never happen */
      ret = false;
   }

   DRETURN(ret); 
}
