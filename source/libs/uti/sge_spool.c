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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>  

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_spool.h"
#include "uti/sge_stdio.h" 
#include "uti/sge_unistd.h"
#include "uti/sge_string.h"
#include "uti/msg_utilib.h"

#include "sge.h"
#include "msg_common.h"

#define MAX_JA_TASKS_PER_DIR  (4096l)
#define MAX_JA_TASKS_PER_FILE (1l)  

static int silent_flag = 0;

static washing_machine_t wtype;

static const char *spoolmsg_message[] = {
   "",
   "DO NOT MODIFY THIS FILE MANUALLY!",
   "",
   NULL
};

static void get_spool_dir_range(u_long32 ja_task_id, u_long32 *start, 
                                u_long32 *end)
{
   u_long32 row = (ja_task_id - 1) / sge_get_ja_tasks_per_directory(); 

   *start = row * sge_get_ja_tasks_per_directory() + 1;
   *end = (row + 1) * sge_get_ja_tasks_per_directory();
}

static void get_spool_dir_parts(u_long32 job_id, char *first, char *second, 
                                char *third)
{
   sprintf(third, "%04d", (int)(job_id % 10000l));
   job_id /= 10000l;
   sprintf(second, "%04d", (int)(job_id % 10000l));
   job_id /= 10000l;
   sprintf(first, "%02d", (int)(job_id % 10000l));  
}

/****** uti/spool/sge_get_ja_tasks_per_directory() *****************************
*  NAME
*     sge_get_ja_tasks_per_directory() -- Configured number of tasks per dir
*
*  SYNOPSIS
*     u_long32 sge_get_ja_tasks_per_directory(void) 
*
*  FUNCTION
*     Returns the configured number of tasks per directory
*
*  RESULT
*     u_long32 - the number
*
*  NOTES
*     MT-NOTE: sge_get_ja_tasks_per_directory() is not MT safe
*******************************************************************************/
u_long32 sge_get_ja_tasks_per_directory(void) {
   static u_long32 tasks_per_directory = 0;

   if (!tasks_per_directory) {
      char *env_string;
     
      env_string = getenv("SGE_MAX_TASKS_PER_DIRECTORY"); 
      if (env_string) {
         tasks_per_directory = (u_long32) strtol(env_string, NULL, 10); 
      }
   }
   if (!tasks_per_directory) {
      tasks_per_directory = MAX_JA_TASKS_PER_DIR;
   }
   return tasks_per_directory;
}
 
/****** uti/spool/sge_get_ja_tasks_per_file() **********************************
*  NAME
*     sge_get_ja_tasks_per_file() -- Configured number of tasks per file
*
*  SYNOPSIS
*     u_long32 sge_get_ja_tasks_per_file(void) 
*
*  FUNCTION
*     Returns the configured number of tasks per file
*
*  RESULT
*     u_long32 - the number
*
*  NOTES
*     MT-NOTE: sge_get_ja_tasks_per_file() is not MT safe
*******************************************************************************/
u_long32 sge_get_ja_tasks_per_file(void) {
   static u_long32 tasks_per_file = 0;
 
   if (!tasks_per_file) {
      char *env_string;
 
      env_string = getenv("SGE_MAX_TASKS_PER_FILE");
      if (env_string) {
         tasks_per_file = (u_long32) strtol(env_string, NULL, 10);
      }
   }
   if (!tasks_per_file) {
     tasks_per_file = MAX_JA_TASKS_PER_FILE;
   }
   return tasks_per_file;  
}

/****** uti/spool/sge_get_file_path() *****************************************
*  NAME
*     sge_get_file_path() -- Return SGE/EE specific file/pathname 
*
*  SYNOPSIS
*     char* sge_get_file_path(char *buffer, sge_file_path_id_t id, 
*                             sge_file_path_format_t format_flags, 
*                             sge_spool_flags_t spool_flags, 
*                             u_long32 ulong_val1, u_long32 ulong_val2,
*                             const char *string_val1) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     char *buffer                        - buffer for file/pathname 
*     sge_file_path_id_t id               - type of file/pathname 
*     sge_file_path_format_t format_flags - format of returned string 
*     sge_spool_flags_t spool_flags       - context where the name is
*                                           needed 
*     u_long32 ulong_val1                 - 1st ulong 
*     u_long32 ulong_val2                 - 2nd ulong 
*     const char *string_val1             - 1st string
*
*  RESULT
*     char* - equivalent with 'buffer' 
*
*  NOTES
*     MT-NOTE: sge_get_file_path() is not MT safe due to get_spool_dir_range()
*
*  SEE ALSO
*     uti/spool/sge_file_path_id_t
*     uti/spool/sge_file_path_format_t
*     uti/spool/sge_spool_flags_t 
******************************************************************************/
char *sge_get_file_path(char *buffer, sge_file_path_id_t id,
                        sge_file_path_format_t format_flags,
                        sge_spool_flags_t spool_flags,
                        u_long32 ulong_val1, u_long32 ulong_val2,
                        const char *string_val1)
{
   int handle_as_zombie = spool_flags & SPOOL_HANDLE_AS_ZOMBIE;
   int first_part = format_flags & FORMAT_FIRST_PART;
   int second_part = format_flags & FORMAT_SECOND_PART;
   int third_part = format_flags & FORMAT_THIRD_PART;
   int insert_dot = format_flags & FORMAT_DOT_FILENAME;
   int in_execd = spool_flags & SPOOL_WITHIN_EXECD;
   char *spool_dir = (handle_as_zombie ? ZOMBIE_DIR : JOB_DIR);

   if (id == JOBS_SPOOL_DIR) {
      sprintf(buffer, SFN, spool_dir);
   } else if (id == JOB_SPOOL_DIR || id == JOB_SPOOL_FILE ||
              id == TASKS_SPOOL_DIR || id == TASK_SPOOL_DIR_AS_FILE ||
              id == TASK_SPOOL_DIR || id == JOB_SPOOL_DIR_AS_FILE ||
              id == TASK_SPOOL_FILE || id == PE_TASK_SPOOL_FILE) {
      char job_dir[SGE_PATH_MAX] = "";
      char file_prefix[SGE_PATH_MAX] = "";
      char id_range[SGE_PATH_MAX] = "";
      char job_dir_first[SGE_PATH_MAX] = "";
      char job_dir_second[SGE_PATH_MAX] = "";
      char job_dir_third[SGE_PATH_MAX] = "";

      get_spool_dir_parts(ulong_val1, job_dir_first, job_dir_second,
                          job_dir_third);

      if (first_part) {
         ;
      } else if (second_part) {
         sprintf(job_dir, "%s", job_dir_first);
      } else if (third_part) {
         sprintf(job_dir, "%s/%s", job_dir_first, job_dir_second);
      } else {
         if (id == JOB_SPOOL_DIR_AS_FILE && insert_dot) {
            if (in_execd) {
               sprintf(job_dir, "%s/%s/.%s."sge_u32, job_dir_first,
                       job_dir_second, job_dir_third, ulong_val2);
            } else {
               sprintf(job_dir, "%s/%s/.%s", job_dir_first,
                       job_dir_second, job_dir_third);
            }
         } else {
            if (in_execd) {
               sprintf(job_dir, "%s/%s/%s."sge_u32, job_dir_first,
                       job_dir_second, job_dir_third, ulong_val2);
            } else {
               sprintf(job_dir, "%s/%s/%s", job_dir_first,
                       job_dir_second, job_dir_third);
            }
         }
      }
      if (insert_dot && (id == JOB_SPOOL_FILE || id == TASK_SPOOL_DIR_AS_FILE ||
                         id == TASK_SPOOL_FILE || id == PE_TASK_SPOOL_FILE)) {
         strcpy(file_prefix, "."); 
      }
      if (id == TASKS_SPOOL_DIR || id == TASK_SPOOL_DIR_AS_FILE ||
          id == TASK_SPOOL_DIR || id == TASK_SPOOL_FILE || 
          id == PE_TASK_SPOOL_FILE) {
         u_long32 start, end;
         get_spool_dir_range(ulong_val2, &start, &end);
         sprintf(id_range, sge_u32"-"sge_u32, start, end);
      }
      if (id == JOB_SPOOL_DIR || id == JOB_SPOOL_DIR_AS_FILE) {
         sprintf(buffer, "%s/%s", spool_dir, job_dir);
      } else if (id == JOB_SPOOL_FILE) {
         sprintf(buffer, "%s/%s/%s%s", spool_dir, job_dir, 
            file_prefix, "common");
      } else if (id == TASKS_SPOOL_DIR) {
         sprintf(buffer, "%s/%s/%s", spool_dir, job_dir, id_range);
      } else if (id == TASK_SPOOL_DIR_AS_FILE || id == TASK_SPOOL_DIR) {
         sprintf(buffer, "%s/%s/%s/%s"sge_u32, spool_dir, job_dir, 
                 id_range, file_prefix, ulong_val2);
      } else if (id == TASK_SPOOL_FILE) {
         sprintf(buffer, "%s/%s/%s/"sge_u32"/%s%s", spool_dir, job_dir, 
                 id_range, ulong_val2, file_prefix, "common");
      } else if (id == PE_TASK_SPOOL_FILE) {
         sprintf(buffer, "%s/%s/%s/"sge_u32"/%s%s", spool_dir, job_dir, 
                 id_range, ulong_val2, file_prefix, string_val1);
      }
   } else if (id == JOB_SCRIPT_DIR) { 
      sprintf(buffer, "%s", EXEC_DIR); 
   } else if (id == JOB_SCRIPT_FILE) {
      sprintf(buffer, "%s/"sge_u32, EXEC_DIR, ulong_val1); 
   } else if (id == JOB_ACTIVE_DIR && in_execd) {
      sprintf(buffer, ACTIVE_DIR"/"sge_u32"."sge_u32, ulong_val1, ulong_val2);
   } else {
      buffer[0] = '\0';
   }

   return buffer;
}

/****** uti/spool/sge_spoolmsg_write() ****************************************
*  NAME
*     sge_spoolmsg_write() -- add a comment in a file
*
*  SYNOPSIS
*     int sge_spoolmsg_write(FILE *file, char comment_char)
*
*  FUNCTION
*     This function writes an additional comment into a file. First
*     character in a comment line is 'comment_char'.
*
*  INPUTS
*     FILE *file        - file descriptor
*     char comment_char - first character in a comment line
*
*  RESULT
*     -1 on error else 0
*
*  NOTES
*     MT-NOTE: sge_spoolmsg_write() is not MT safe due to FPRINTF() macro
******************************************************************************/
int sge_spoolmsg_write(FILE *file, const char comment_char, const char *version){
   int i;
 
   FPRINTF((file, "%c Version: %s\n", comment_char, version));
   i = 0;
   while(spoolmsg_message[i]) {
      FPRINTF((file, "%c %s\n", comment_char, spoolmsg_message[i]));
      i++;
   }
 
   return 0;
FPRINTF_ERROR:
   return -1;
}

void sge_spoolmsg_append(dstring *ds, const char comment_char, const char *version)
{
   int i = 0;

   sge_dstring_sprintf_append(ds, "%c Version: %s\n", comment_char, version);
   while(spoolmsg_message[i]) {
      sge_dstring_sprintf_append(ds, "%c %s\n", comment_char, spoolmsg_message[i]);
      i++;
   }

   return;
}

/****** uti/spool/sge_readpid() ***********************************************
*  NAME
*     sge_readpid() -- Read pid from file
*
*  SYNOPSIS
*     pid_t sge_readpid(const char *fname)
*
*  FUNCTION
*     Read pid from file 'fname'. The pidfile may be terminated with
*     a '\n'. Empty lines at the beginning of the file are ignored.
*     Whitespaces at the beginning of the line are ignored.
*     Any characters or lines after a valid pid are ignored.
*
*  INPUTS
*     const char *fname - filename
*
*  RESULT
*     pid_t - process id
*  
*  NOTES
*     MT-NOTE: sge_readpid() is MT safe.
******************************************************************************/ 
pid_t sge_readpid(const char *fname)
{
   FILE *fp;
   char buf[512], *cp;
   pid_t pid;
 
   DENTER(TOP_LAYER, "sge_readpid");
 
   if (!(fp = fopen(fname, "r"))) {
      DEXIT;
      return 0;
   }
 
   pid = 0;
   while (fgets(buf, sizeof(buf), fp))
   {
      char *pos = NULL;

      /*
       * set chrptr to the first non blank character
       * If line is empty continue with next line
       */
       if(!(cp = strtok_r(buf, " \t\n", &pos))) {
          continue;
       }
 
       /* Check for negative numbers */
       if (!isdigit((int) *cp)) {
          pid = 0;
       } else {
          pid = atoi(cp);
       }
       break;
   }
 
   FCLOSE(fp);
 
   DEXIT;
   return pid;
FCLOSE_ERROR:
   DEXIT;
   return 0;
} /* sge_readpid() */        

/****** uti/spool/sge_write_pid() *********************************************
*  NAME
*     sge_write_pid() -- Write pid into file
*
*  SYNOPSIS
*     void sge_write_pid(const char *pid_log_file)
*
*  FUNCTION
*     Write pid into file
*
*  INPUTS
*     const char *pid_log_file - filename
*  
*  NOTES
*     MT-NOTE: sge_write_pid() is MT safe
******************************************************************************/
void sge_write_pid(const char *pid_log_file)
{
   int pid, ret;
   FILE *fp;
 
   DENTER(TOP_LAYER, "sge_write_pid");
 
   errno = 0;
   close(ret=creat(pid_log_file, 0644));
   if (-1 == ret) {
      /* fixme: stop afterwards? */
      ERROR((SGE_EVENT, MSG_FILE_CANNOT_CREATE_SS, pid_log_file,
             strerror(errno)));
   }
#if defined( INTERIX )
   /*
    * Interix has a bug if the file is created on a NFS mapped drive.
    */ 
   /* Flawfinder: ignore */   
   chown(pid_log_file, geteuid(), -1);
#endif
   if ((fp = fopen(pid_log_file, "w")) == NULL) {
      /* fixme: stop afterwards? */
      ERROR((SGE_EVENT, MSG_FILE_FOPENFAILED_SS, pid_log_file,
             strerror(errno)));
   } else {
      pid = getpid();
      FPRINTF((fp, "%d\n", pid));
      FCLOSE(fp);
   }
FPRINTF_ERROR:
FCLOSE_ERROR:
   /* EB: TODO: CLEANUP: make it possible that calling function handles this error */
   DEXIT;
   return;
}  

 
/****** uti/spool/sge_get_confval() *******************************************
*  NAME
*     sge_get_confval() -- Get config value for
*
*  SYNOPSIS
*     char* sge_get_confval(const char *conf_val, const char *fname)
*
*  FUNCTION
*     Get config value for entry 'conf_val' from file 'fname'.
*
*  INPUTS
*     const char *conf_val - is case insensitive name
*     const char *fname    - filename
*
*  RESULT
*     char* - pointer to internal static buffer
*
*  NOTES
*     Lines may be up to 1024 characters long. Up to 1024 characters of the
*     config value are copied to the static buffer.
*
*  NOTES
*     MT-NOTE: sge_get_confval() is MT safe
******************************************************************************/
char *sge_get_confval(const char *conf_val, const char *fname)
{
   static char valuev[1][1025];
   bootstrap_entry_t namev[1];

   namev[0].name = conf_val;
   namev[0].is_required = true;
   if (sge_get_confval_array(fname, 1, 1, namev, valuev, NULL)) {
      return NULL;
   } else {
      return valuev[0];
   }
}

/****** uti/spool/sge_get_confval_array() *************************************
*  NAME
*     sge_get_confval_array() - Read configuration file entries
*
*  SYNOPSIS
*     int sge_get_confval_array(const char *fname, int n, 
*                               const char *name[], 
*                               char value[][1025],
*                               dstring *error_dstring) 
*
*  FUNCTION
*     Reads in an array of configuration file entries
*
*  RESULT
*     int - 0 on success
*
*  BUGS
*     Function can not differ multiple similar named entries.
*
*  NOTES
*     MT-NOTE: sge_get_confval_array() is MT safe
******************************************************************************/
int sge_get_confval_array(const char *fname, int n, int nmissing, bootstrap_entry_t name[], 
                          char value[][1025], dstring *error_dstring) 
{
   FILE *fp;
   char buf[1024], *cp;
   int i;
   bool *is_found = NULL;
   
   DENTER(TOP_LAYER, "sge_get_confval_array");

   if (!(fp = fopen(fname, "r"))) {
      if (error_dstring == NULL){
         CRITICAL((SGE_EVENT, MSG_FILE_FOPENFAILED_SS, fname, strerror(errno)));
      }
      else {
         sge_dstring_sprintf(error_dstring, MSG_FILE_FOPENFAILED_SS, 
                             fname, strerror(errno));
      }
      DEXIT;
      return n;
   }
   is_found = malloc(sizeof(bool) * n);
   memset(is_found, false, n * sizeof(bool));
   
   while (fgets(buf, sizeof(buf), fp)) {
      char *pos = NULL;

      /* set chrptr to the first non blank character
       * If line is empty continue with next line
       */
      if(!(cp = strtok_r(buf, " \t\n", &pos))) {
          continue;
      }    

      /* allow commentaries */
      if (cp[0] == '#') {
          continue;
      }    
  
      /* search for all requested configuration values */ 
      for (i=0; i<n; i++) {
         if (cp && (strcasecmp(name[i].name, cp) == 0) &&
             ((cp = strtok_r(NULL, " \t\n", &pos)) != NULL)) {
             if (!cp) break;
             /* fixme value elements are 1025 long in caller */
             strncpy(value[i], cp, 512);
             cp = value[i];
             is_found[i] = true;
             if (name[i].is_required) {
               --nmissing; 
             }
             break;
         }
      }
   }
   if (nmissing != 0) {
      for (i=0; i<n; i++) {
         if (!is_found[i] && name[i].is_required) {
            if (error_dstring == NULL){
               CRITICAL((SGE_EVENT, MSG_UTI_CANNOTLOCATEATTRIBUTE_SS, name[i].name, fname));
            }
            else {
               sge_dstring_sprintf(error_dstring, MSG_UTI_CANNOTLOCATEATTRIBUTE_SS, 
                                   name[i].name, fname);
            }
            
            break;
         }
      }
   }
   
   sge_free(&is_found);
   FCLOSE(fp);
   DEXIT;
   return nmissing;
FCLOSE_ERROR:
   DEXIT;
   return 0;
} /* sge_get_confval_array() */



/****** uti/spool/sge_status_set_type() ***************************************
*  NAME
*     sge_status_set_type() -- set display mode
*
*  SYNOPSIS
*     void sge_status_set_type(washing_machine_t type)
*
*  FUNCTION
*     With 'STATUS_ROTATING_BAR' each call of
*     sge_status_next_turn() will show a rotating bar.
*     In 'STATUS_DOTS'-mode each call will show more
*     dots in a line.
*
*  INPUTS
*     washing_machine_t type - display type
*        STATUS_ROTATING_BAR
*        STATUS_DOTS
*
*  NOTES
*     MT-NOTE: sge_status_set_type() is not MT safe
******************************************************************************/
void sge_status_set_type(washing_machine_t type)
{
   wtype = type;
}              

/****** uti/spool/sge_status_next_turn() **************************************
*  NAME
*     sge_status_next_turn() -- show next turn
*
*  SYNOPSIS
*     void sge_status_next_turn(void)
*
*  FUNCTION
*     Show next turn of rotating washing machine.
*
*  NOTES
*     MT-NOTE: sge_status_next_turn() is not MT safe
******************************************************************************/
void sge_status_next_turn(void)
{
   static int cnt = 0;
   static const char s[] = "-\\/";
   static const char *sp = NULL;
 
   cnt++;
   if ((cnt % 100) != 1) {
      return;
   }
 
   switch (wtype) {
   case STATUS_ROTATING_BAR:
      {
 
         if (!sge_silent_get()) {
            if (!sp || !*sp) {
               sp = s;
            }
            printf("%c\b", *sp++);
            fflush(stdout);
         }
      }
      break;
   case STATUS_DOTS:
      if (!sge_silent_get()) {
         printf(".");
         fflush(stdout);
      }
      break;
   default:
      break;
   }
}                                                                               

/****** uti/spool/sge_status_end_turn() ***************************************
*  NAME
*     sge_status_end_turn() -- remove washing machine from display
*
*  SYNOPSIS
*     void sge_status_end_turn(void)
*
*  FUNCTION
*     Last turn of washing machine.
*
*  NOTES
*     MT-NOTE: sge_status_end_turn() is not MT safe
******************************************************************************/
void sge_status_end_turn(void)
{
   switch (wtype) {
   case STATUS_ROTATING_BAR:
      if (!sge_silent_get()) {
         printf(" \b");
         fflush(stdout);
      }
      break;
   case STATUS_DOTS:
      if (!sge_silent_get()) {
         printf("\n");
         fflush(stdout);
      }
      break;
   default:
      break;
   }
}  

/****** uti/spool/sge_silent_set() ********************************************
*  NAME
*     sge_silent_set() -- Enable/disable silence during spool ops 
*
*  SYNOPSIS
*     void sge_silent_set(int i) 
*
*  FUNCTION
*     Enable/disable silence during spool operations. Silence means
*     that no messages are printed to stdout. 
*
*  INPUTS
*     int i - 0 or 1 
*
*  SEE ALSO
*     uti/spool/sge_silent_get() 
*
*  NOTES
*     MT-NOTE: sge_silent_set() is not MT safe
******************************************************************************/
void sge_silent_set(int i) 
{
   silent_flag = i;
   return;
}
 
/****** uti/spool/sge_silent_get() ********************************************
*  NAME
*     sge_silent_get() -- Show whether silence is enable/disabled 
*
*  SYNOPSIS
*     int sge_silent_get() 
*
*  FUNCTION
*     Show whether silence is enable/disabled 
*
*  RESULT
*     int - 0 or 1
*
*  SEE ALSO
*     uti/spool/sge_silent_set() 
*
*  NOTES
*     MT-NOTE: sge_silent_get() is not MT safe
******************************************************************************/
int sge_silent_get()
{
   return silent_flag;
} 

/****** uti/spool/sge_get_management_entry() *************************************
*  NAME
*     sge_get_management_entry() - Read management.properties file entries
*
*  SYNOPSIS
*     int sge_get_management_entry(const char *fname, int n, 
*                               const char *name[], 
*                               char value[][1025],
*                               dstring *error_dstring) 
*
*  FUNCTION
*     Reads in an array of configuration file entries
*
*  RESULT
*     int - 0 on success
*
*  BUGS
*     Function can not differ multiple similar named entries.
*
*  NOTES
*     MT-NOTE: sge_get_management_entry() is MT safe
******************************************************************************/
int sge_get_management_entry(const char *fname, int n, int nmissing, bootstrap_entry_t name[], 
                          char value[][SGE_PATH_MAX], dstring *error_dstring) 
{
   FILE *fp;
   char buf[SGE_PATH_MAX], *cp;
   int i;
   bool *is_found = NULL;
   
   DENTER(TOP_LAYER, "sge_get_management_entry");

   if (!(fp = fopen(fname, "r"))) {
      if (error_dstring == NULL){
         CRITICAL((SGE_EVENT, MSG_FILE_FOPENFAILED_SS, fname, strerror(errno)));
      }
      else {
         sge_dstring_sprintf(error_dstring, MSG_FILE_FOPENFAILED_SS, 
                             fname, strerror(errno));
      }
      DEXIT;
      return n;
   }
   is_found = malloc(sizeof(bool) * n);
   memset(is_found, false, n * sizeof(bool));
   
   while (fgets(buf, sizeof(buf), fp)) {
      char *pos = NULL;

      /* set chrptr to the first non blank character
       * If line is empty continue with next line
       */
      if(!(cp = strtok_r(buf, " \t\n", &pos))) {
          continue;
      }    

      /* allow commentaries */
      if (cp[0] == '#') {
          continue;
      }    
  
      /* search for all requested configuration values */ 
      for (i=0; i<n; i++) {
         char *nam = strtok_r(cp, "=", &pos);
         char *val = strtok_r(NULL, "\n", &pos);
         if (nam != NULL && strcasecmp(name[i].name, nam) == 0) {
                DPRINTF(("nam = %s\n", nam));
                if (val != NULL) {
                   DPRINTF(("val = %s\n", val));
                   sge_strlcpy(value[i], val, SGE_PATH_MAX);
                } else {
                   sge_strlcpy(value[i], "", SGE_PATH_MAX);
                }
                is_found[i] = true;
                if (name[i].is_required) {
                  --nmissing; 
                }
                break;
         }
      }
   }
   if (nmissing != 0) {
      for (i=0; i<n; i++) {
         if (!is_found[i] && name[i].is_required) {
            if (error_dstring == NULL){
               CRITICAL((SGE_EVENT, MSG_UTI_CANNOTLOCATEATTRIBUTEMAN_SS, name[i].name, fname));
            }
            else {
               sge_dstring_sprintf(error_dstring, MSG_UTI_CANNOTLOCATEATTRIBUTEMAN_SS, 
                                   name[i].name, fname);
            }
            
            break;
         }
      }
   }
   
   sge_free(&is_found);
   FCLOSE(fp);
   DEXIT;
   return nmissing;
FCLOSE_ERROR:
   DEXIT;
   return 0;
} /* sge_get_management_entry() */

/****** uti/spool/sge_get_active_job_file_path() ********************************
*  NAME
*     sge_get_active_job_file_path() -- Create paths in active_jobs dir
*
*  SYNOPSIS
*     const char* sge_get_active_job_file_path(dstring *buffer, 
*        u_long32 job_id, u_long32 ja_task_id, const char *pe_task_id, 
*        const char *filename) 
*
*  FUNCTION
*     Creates paths in the execd's active_jobs directory.
*     Both directory and file paths can be created.
*     The result is placed in a buffer provided by the caller.
* 
*     WARNING: Do only use in shepherd and execution daemon!
*
*  INPUTS
*     dstring *buffer        - buffer to hold the generated path
*     u_long32 job_id        - job id 
*     u_long32 ja_task_id    - array task id
*     const char *pe_task_id - optional pe task id
*     const char *filename   - optional file name
*
*  RESULT
*     const char* - pointer to the string buffer on success, else NULL
*
*  EXAMPLE
*     To create the relative path to a jobs/tasks environment file, the 
*     following call would be used:
*
*     char buffer[SGE_PATH_MAX]
*     sge_get_active_job_file_path(buffer, SGE_PATH_MAX, 
*                                  job_id, ja_task_id, pe_task_id,
*                                  "environment");
*     
*
*  NOTES
*     JG: TODO: The function might be converted to or might use a more 
*     general path creating function (utilib).
*
*  SEE ALSO
*     execd/sge_make_ja_task_active_dir()
*     execd/sge_make_pe_task_active_dir()
*******************************************************************************/
const char *sge_get_active_job_file_path(dstring *buffer, u_long32 job_id, 
   u_long32 ja_task_id, const char *pe_task_id, const char *filename) 
{
   DENTER(TOP_LAYER, "sge_get_active_job_file_path");

   if (buffer == NULL) {
      DRETURN(NULL);
   }

   sge_dstring_sprintf(buffer, "%s/"sge_u32"."sge_u32, ACTIVE_DIR, job_id, ja_task_id);

   if (pe_task_id != NULL) {
      sge_dstring_append_char(buffer, '/');
      sge_dstring_append(buffer, pe_task_id);
   }

   if (filename != NULL) {
      sge_dstring_append_char(buffer, '/');
      sge_dstring_append(buffer, filename);
   }

   DRETURN(sge_dstring_get_string(buffer));
}


