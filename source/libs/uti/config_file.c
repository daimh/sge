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
#include <ctype.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_stdio.h"
#include "uti/sge_log.h"
#include "uti/sge_parse_num_par.h"
#include "uti/config_file.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_arch.h"

#include "basis_types.h"
#include "err_trace.h"

#include "msg_daemons_common.h"


char err_msg[MAX_STRING_SIZE] = { '0' };

void set_error(const char *err_str) 
{
   if (err_str) {
      sge_strlcpy(err_msg, err_str, sizeof(err_msg));
   }
}

/* handle the config file
   This file is used to transfer parameters to the shepherd */
typedef struct config_entry {
   char *name;
   char *value;
   struct config_entry *next;
} config_entry;

/*
 * MT-NOTE: libs/uti/config_file.c is not MT safe due to access to global 
 * MT-NOTE: variables. But currently it is used only in execd and shepherd 
 */
static config_entry *config_list = NULL;

/* these variables may get used to replace variables in pe_start */
char *pe_variables[] = {
   "pe_hostfile",
   "host",
   "ja_task_id",
   "job_owner",
   "job_id",
   "job_name",
   "pe",
   "pe_slots",
   "processors",
   "queue",
   "sge_cell",
   "sge_root",
   "stdin_path",
   "stdout_path",
   "stderr_path",
   "merge_stderr",
   "fs_stdin_host",
   "fs_stdin_path",
   "fs_stdin_tmp_path",
   "fs_stdin_file_staging",
   "fs_stdout_host",
   "fs_stdout_path",
   "fs_stdout_tmp_path",
   "fs_stdout_file_staging",
   "fs_stderr_host",
   "fs_stderr_path",
   "fs_stderr_tmp_path",
   "fs_stderr_file_staging",
   NULL
};

/* these variables may get used to replace variables in prolog/epilog */
char *prolog_epilog_variables[] = {
   "host",
   "ja_task_id",
   "job_owner",
   "job_id",
   "job_name",
   "processors",
   "queue",
   "sge_cell",
   "sge_root",
   "stdin_path",
   "stdout_path",
   "stderr_path",
   "merge_stderr",
   "fs_stdin_host",
   "fs_stdin_path",
   "fs_stdin_tmp_path",
   "fs_stdin_file_staging",
   "fs_stdout_host",
   "fs_stdout_path",
   "fs_stdout_tmp_path",
   "fs_stdout_file_staging",
   "fs_stderr_host",
   "fs_stderr_path",
   "fs_stderr_tmp_path",
   "fs_stderr_file_staging",
   NULL
};


/* these variables may get used to replace variables in the allocation rule of a pe */
char *pe_alloc_rule_variables[] = {
   "pe_slots",
   "fill_up",
   "round_robin",
   NULL
};

char *ckpt_variables[] = {
   "host",
   "ja_task_id",
   "job_owner",
   "job_id",
   "job_name",
   "queue",
   "job_pid",
   "ckpt_dir",
   "ckpt_signal",
   "sge_cell",
   "sge_root",
   NULL
};

char *ctrl_method_variables[] = {
   "host",
   "ja_task_id",
   "job_owner",
   "job_id",
   "job_name",
   "queue",
   "job_pid",
   "sge_cell",
   "sge_root",
   NULL
};

void (*config_errfunc)(const char *) = NULL;

/*****************************************************
 read configuration file to memory.
 returns:
 - 0 Ok
 - 1 systemerror -> errno is valid
 - 2 malloc error

 MT-NOTE: read_config() is not MT safe
 *****************************************************/
int read_config(const char *fname) {
   FILE *fp;
   char buf[100000], *name, *value;

   delete_config();

   fp = fopen(fname, "r");
   if (!fp) {
      return 1;
   }

   while (fgets(buf, sizeof(buf), fp)) {
      struct saved_vars_s *context;
      context = NULL;
      name = sge_strtok_r(buf, " =", &context);
      if (!name) {
         sge_free_saved_vars(context);
         break;
      }   
      value = sge_strtok_r(NULL, "\n", &context);

      if (add_config_entry(name, value)) {
         sge_free_saved_vars(context);
         return 2;
      }
      sge_free_saved_vars(context);
   }
   FCLOSE(fp);

   return 0;
FCLOSE_ERROR:
   return 1;
}

/******************************************************/
int add_config_entry(const char *name, const char *value)
{
   config_entry *new;

   if ((new = (config_entry *)malloc(sizeof(config_entry))) == NULL) {
      return 1;
   }
   
   if ((new->name = strdup(name)) == NULL) {
      sge_free(&new);
      return 1;
   }
  
   if (value != NULL) {
      if ((new->value = strdup(value)) == NULL) {
         sge_free(&(new->name));
         sge_free(&(new));
         return 1;
      }
   } else {
      new->value = NULL;
   }
  
   new->next = config_list;
   config_list = new;

   return 0;
}

/****************************************************/

static config_entry *find_conf_entry(const char *name, config_entry *ptr) 
{
   while (ptr) {
      if (!strcmp(ptr->name, name))
         return ptr;

      ptr = ptr->next;
   }
   return NULL;
}

/***************************************************/
char *get_conf_val(const char *name)
{
   config_entry *ptr = config_list;
   char err_str[MAX_STRING_SIZE];
   
   ptr = find_conf_entry(name, config_list);
   if (ptr)
      return ptr->value;
   
   snprintf(err_str, sizeof(err_str), MSG_CONF_NOCONFVALUE_S, name);
   if (config_errfunc)
      config_errfunc(err_str);
   return NULL;
}

/*****************************************************************************
 * set_conf_val
 *   Sets the value of a config entry.
 *   If the config entry already exists, it replaces the value.
 *   If the config entry does not exist, it creates a new config entry.
 *
 * Parameters:
 *   name:  Name of the config entry
 *   value: Value of the config entry
 *****************************************************************************/
void set_conf_val(const char* name, const char* value)
{
   config_entry* pConfigEntry;

   if (name == NULL || value == NULL) {
      return;
   }
   
   pConfigEntry = find_conf_entry(name, config_list);
   if (pConfigEntry != NULL) {
      /* avoid overwriting by itself */
      if (pConfigEntry->value != value) {
         sge_free(&(pConfigEntry->value));
         pConfigEntry->value = strdup(value);
      }
   } else {
      add_config_entry(name, value);
   }
}

/***************************************************/
char *search_conf_val(const char *name) {
   config_entry *ptr = config_list;
   
   ptr = find_conf_entry(name, config_list);
   if (ptr != NULL) {
      return ptr->value;
   } else {
      return NULL;
   }
}

/**************************************************
   return NULL if conf value does not exist or
   if "none" is it's value
*/
char *search_nonone_conf_val(const char *name)
{
   char *s;

   s = search_conf_val(name);
   if (s != NULL && strcasecmp("none", s) == 0) {
      return NULL;
   } else {
      return s;
   }
}

/**************************************************/
void delete_config()
{
   config_entry *next;

   while (config_list) {
      next = config_list->next;
      if (config_list->name) {
         sge_free(&(config_list->name));
      }
      if (config_list->value) {
         sge_free(&(config_list->value));
      }
      sge_free(&config_list);
      config_list = next;
   }
}

/* JG: TODO: Cleanup:
 * config_file shouldn't be part of utilib, but this is necessary 
 * due to dependencies with other modules.
 * To resolve it:
 * replace_params calls a function checking if a config value is 
 * available (but this should only be called in shepherd).
 * In qmaster replace_params is also called, but qmaster has no 
 * config list - and would not need the checking function 
 * Solution: Pass a function pointer for a checking function.
 * qmaster (resp. the functions validating pe and ckpt etc. in libgdi)
 * pass NULL as checking function, schepherd etc. passes a function pointer.
 * replace_params should be moved to sge_string (libuti),
 * config_file could be moved to libspool.
 */

/*
   replace_params()

DESCRIPTION 

   can be used to 
      check if all variables contained in src are allowed
   or to 
      build a new string in dst by replacing allowed variables
      from config_list 

   a variable starts with a "$" sign 
   the name contains of chars from isalnum() or "_" 

   the allowed variable names are given using a string array 
   that contains accessable variables or all variable from 
   config list are allowed if NULL is passed

ERROR

   0  no error    

   there are two classes of errors:
      -1 a lack of an __allowed__ variable in the config_list   - execd damaged
      1  an syntax error or a request of a not allowed variable - pe damaged
  
REM
   
   may be the config_list should be passed as an argument

*/
int replace_params(
const char *src,
char *dst,
int dst_len,
char **allowed  
) {
   char err_str[MAX_STRING_SIZE];
   char name[256];
   int name_len;
   const char *sp;
   size_t dp_pos = 0;

   size_t max_dst_len = dst_len - 1;
   char **spp, *value = NULL;
   int just_check = 0;
   char root[SGE_PATH_MAX];

   /* does caller just want to validate */
   if (!dst) {
      just_check = 1;
   }

   /* handle NULL string as src */
   if (src == NULL) {
      sp = "";
   } else {
      sp=src; 
   }
   while (*sp) {
      switch (*sp) {
      case '$':

         /* eat '$' */
         sp++;

         /* get variable length */
         name_len = 0;
         /* here get variable names defined */
         while (isalnum((int) sp[name_len]) || sp[name_len]=='_') { 
            name_len++;
         }

         if (name_len==0) {
            snprintf(err_str, sizeof(err_str), SFNMAX, MSG_CONF_ATLEASTONECHAR);
            if (config_errfunc)
               config_errfunc(err_str);
            return 1;
         }

         if (name_len>(sizeof(name)-1)) {
            snprintf(err_str, sizeof(err_str), MSG_CONF_REFVAR_S, sp,
                     (int) sizeof(name)-1);
            if (config_errfunc)
               config_errfunc(err_str);
            return 1;
         }

         /* copy variable name in local buffer */
         strncpy(name, sp, name_len);
         name[name_len] = '\0';

         /* eat variable name */
         sp += name_len;

         /* check if this variable is allowed */
         if (allowed) {
            for (spp=allowed; *spp && strcmp(*spp, name); spp++)
               ;
               
            if (!*spp) {
               /* this variable may be known by us but not by the user */
               snprintf(err_str, sizeof(err_str), MSG_CONF_UNKNOWNVAR_S, name);
               if (config_errfunc)
                  config_errfunc(err_str);
               return 1;
            } 
         }

         /* check if this variable is in the config_entry list */
         /* sge_root, sge_cell are special -- not in the config file */
         if (!just_check) {
            value=get_conf_val(name);
            if (!value) {
               if (strcmp(name, "sge_root") == 0) {
                  sge_get_root_dir(0, root, sizeof root, 0);
                  value = root;
               }
               else if (strcmp(name, "sge_cell") ==0)
                  value = (char *) sge_get_default_cell();
               else
                  /* error func is called by get_conf_val() */
                  return -1;/* Arghh! we have promised to serve such a variable */
            }
            /* copy value into dst buffer */
            while (*value && dp_pos < max_dst_len) {
               dst[dp_pos++] = *value++;
            }
         }
         break;

      default:
         if (!just_check && dp_pos < max_dst_len) {
            dst[dp_pos++] = *sp; 
         }
         sp++;
         break;
      }
   }

   if (!just_check) {
      dst[dp_pos] = '\0';
   }
   return 0;
}

bool parse_time_param(const char *input, const char *variable, u_long32 *value)
{
   bool ret = false;

   DENTER(BASIS_LAYER, "parse_time_param");

   if (input != NULL && variable != NULL && value != NULL) {
      int var_len = strlen(variable);
      
      /* Test that 'variable' is the left side of the = in 'input.' */
      /* We don't have to guard against an overrun in input[var_len] because we
       * know that input is at least as long as var_len when we pass the
       * strncasecmp(), so the worst that input[var_len] could be is \0. */
      if ((strncasecmp(input, variable, var_len) == 0) &&
         ((input[var_len] == '=') || (input[var_len] == '\0'))) {
         const char *s;

         /* yes, this variable is set */
         ret = true;

         /* search position of = */
         s = strchr(input, '=');

         /* no value contained in input -> value = 0 */
         if (s == NULL) {
            *value = 0;
         } else {
            /* skip = */
            s++;

            if (!extended_parse_ulong_val(NULL, value, TYPE_TIM, s, NULL, 0, 0, false)) {
               *value = 0;
               ret = false;
            }
         }

         DPRINTF(("%s = "sge_U32CFormat"\n", variable, sge_u32c(*value)));
      }
   }

   DEXIT;
   return ret;
}


bool parse_bool_param(const char *input, const char *variable, bool *value)
{
   bool ret = false;

   DENTER(BASIS_LAYER, "parse_bool_param");

   if (input != NULL && variable != NULL && value != NULL) {
      int var_len = strlen(variable);
      
      /* 
       * Test that 'variable' is the left side of the = in 'input' or
       * 'input' only contains 'variable'.
       * We don't have to guard against an overrun in input[var_len] because we
       * know that input is at least as long as var_len when we pass the
       * strncasecmp(), so the worst that input[var_len] could be is \0. 
       */
      if ((strncasecmp(input, variable, var_len) == 0) &&
         ((input[var_len] == '=') || (input[var_len] == '\0'))) {
         const char *s;

         /* yes, this variable is set */
         ret = true;

         /* search position of = */
         s = strchr(input, '=');

         /* only boolean variable contained in input -> value = true */
         if (s == NULL) {
            *value = true;
         } else {
            /* skip = */
            s++;
            /* parse value */
            if (*s == '1' || strcasecmp(s, "true") == 0) {
               *value = true;
            } else {
               *value = false;
            }
         }

         DPRINTF(("%s = %s\n", variable, value ? "true" : "false"));
      }
   }

   DEXIT;
   return ret;
}

bool parse_int_param(const char *input, const char *variable, 
                     int *value, int type)
{
   bool ret = false;

   DENTER(BASIS_LAYER, "parse_int_param");

   if (input != NULL && variable != NULL && value != NULL) {
      int var_len = strlen(variable);

      if ((strncasecmp(input, variable, var_len) == 0) &&
         ((input[var_len] == '=') || (input[var_len] == '\0'))) {
         const char *s;

         /* yes, this variable is set */
         ret = true;

         /* search position of = */
         s = strchr(input, '=');

         /* no value contained in input -> value = 0 */
         if (s == NULL) {
            *value = 0;
         } else {
            u_long32 new_value;
            /* skip = */
            s++;
            /* parse value */
            if (parse_ulong_val(NULL, &new_value, type, s, NULL, 0)) {
               *value = new_value;
            } else {
               *value = 0;
            }
         }

         DPRINTF(("%s = %d\n", variable, *value));
      }
   }

   DEXIT;
   return ret;
}

/****** uti/parse_script_params() *****************************************
*  NAME
*     parse_script_params() -- Parse prolog/epilog/pe_start/pe_stop/mailer
*                              line from config
*
*  SYNOPSIS
*     char *parse_script_params(char **script_file)
*
*  FUNCTION
*     Parses the config value for prolog, epilog, pe_start, pe_stop, or mailer.
*     Retrieves the target user (as whom the script should be run) and
*     eats the target user from the config value string, so that the path of
*     the script file remains.
*
*  INPUTS
*     char **script_file - Pointer to the string containing the conf value.
*                          syntax: [<user>@]<path>, e.g. joe@/home/joe/script
*
*  OUTPUT
*     char **script_file - Pointer to the string containing the script path.
*
*  RESULT
*     char* - If one is given, the name of the user.
*             Else NULL.
*******************************************************************************/
char*
parse_script_params(char **script_file)
{
   char* target_user = NULL;
   char* s;

   /* syntax is [<user>@]<path> <arguments> */
   s = strpbrk(*script_file, "@ ");
   if (s && *s == '@') {
      *s = '\0';
      target_user = *script_file;
      *script_file = &s[1];
   }
   return target_user;
}

#if 0
bool parse_string_param(const char *input, const char *variable, char *value)
{
   bool ret = false;

   DENTER(BASIS_LAYER, "parse_string_param");
   if (input != NULL && variable != NULL && value != NULL) {
      int var_len = strlen(variable);
      /* Test that 'variable' is the left side of the = in 'input.' */
      /* We don't have to guard against an overrun in input[var_len] because we
       * know that input is at least as long as var_len when we pass the
       * strncasecmp(), so the worst that input[var_len] could be is \0. */
      if ((strncasecmp(input, variable, var_len) == 0) &&
         ((input[var_len] == '=') || (input[var_len] == '\0'))) {
         const char *s;
         ret = false;
         /* search position of = */
         s = strchr(input, '=');
         /* only variable name in input */
         if (s != NULL) {
            dstring l = DSTRING_INIT;
            s++;                /* skip = */
            if (strncmp("\"", s, 1) == 0) {
              s++;
              if ((l = strnchr(s, "=")) != NULL) {

              }
            }
         }
         DPRINTF(("%s = %s\n", variable, sge_dstring_get_string (value)));
      }
   }
   DEXIT;
   return ret;
}
#endif
