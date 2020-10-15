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
#include <unistd.h>
#include <errno.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_spool.h"
#include "uti/sge_profiling.h"
#include "uti/sge_unistd.h"

#include "cull/cull_file.h"
#include "cull/cull_list.h"

#include "sgeobj/sge_str.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_suser.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_object.h"

#include "spool/sge_dirent.h"

#include "basis_types.h"
#include "sge.h"
#include "sge_job_qmaster.h"
#include "read_write_job.h"
#include "msg_common.h"
#include "msg_spoollib_classic.h"

static lList *ja_task_list_create_from_file(u_long32 job_id, 
                                            u_long32 ja_task_id,
                                            sge_spool_flags_t flags);

static lListElem *ja_task_create_from_file(u_long32 job_id,
                                           u_long32 ja_task_id,
                                           const char *pe_task_id,
                                           sge_spool_flags_t flags);

static int ja_task_write_to_disk(lListElem *ja_task, u_long32 job_id,
                                 const char *pe_task_id,
                                 sge_spool_flags_t flags); 

static int job_write_ja_task_part(lListElem *job, u_long32 ja_task_id,
                                  const char *pe_task_id,
                                  sge_spool_flags_t flags);

static int job_write_as_single_file(lListElem *job, u_long32 ja_task_id,
                                   sge_spool_flags_t flags);

static lListElem *job_create_from_file(u_long32 job_id, u_long32 task_id,
                                       sge_spool_flags_t flags);    

static int job_has_to_spool_one_file(const lListElem *job,
                                     const lList *pe_list,
                                     sge_spool_flags_t flags);

static lListElem *pe_task_create_from_file(u_long32 job_id,
                                           u_long32 ja_task_id,
                                           const char *pe_task_id,
                                           sge_spool_flags_t flags);

static int job_remove_script_file(u_long32 job_id);

/* Here we cache the path of the last task spool dir that has been created.
   In case a task spool dir is removed the cache is no longer a proof of the
   existence of the task spool dir and is reinitialized */
static char old_task_spool_dir[SGE_PATH_MAX] = "";

static lListElem *job_create_from_file(u_long32 job_id, u_long32 ja_task_id,
                                       sge_spool_flags_t flags)
{
   lListElem *job = NULL;
   char spool_path[SGE_PATH_MAX] = "";

   DENTER(TOP_LAYER, "job_create_from_file");

   sge_get_file_path(spool_path, JOB_SPOOL_DIR, FORMAT_DEFAULT, 
                     flags, job_id, ja_task_id, NULL);  

   if (sge_is_directory(spool_path)) {
      char spool_path_common[SGE_PATH_MAX];
      lList *ja_tasks = NULL;

      sge_get_file_path(spool_path_common, JOB_SPOOL_FILE, FORMAT_DEFAULT, 
                        flags, job_id, ja_task_id, NULL);  
      job = lReadElemFromDisk(NULL, spool_path_common, JB_Type, "job");
      if (job) {
         ja_tasks = ja_task_list_create_from_file(job_id, ja_task_id, flags); 
         if (ja_tasks) {
            lList *ja_task_list;

            ja_task_list = lGetList(job, JB_ja_tasks);
            if (ja_task_list) {
               lAddList(ja_task_list, &ja_tasks);
            } else {
               lSetList(job, JB_ja_tasks, ja_tasks);
            }
            ja_tasks = NULL;
            lPSortList(ja_tasks, "%I+", JAT_task_number); 
         } else {
            /*
             * This is no error! It only means that there is no enrolled
             * task in the spool area (all tasks are unenrolled)
             */
         }
      }
   } else {
      job = lReadElemFromDisk(NULL, spool_path, JB_Type, "job");
   }
   DRETURN(job);
}

static lList *ja_task_list_create_from_file(u_long32 job_id, 
                                            u_long32 ja_task_id,
                                            sge_spool_flags_t flags)
{
   lList *dir_entries = NULL;
   lList *ja_task_entries = NULL;
   lList *pe_task_entries = NULL;
   lList *ja_tasks = NULL;
   lList *pe_tasks = NULL;
   lListElem *dir_entry;
   char spool_dir_job[SGE_PATH_MAX];
   DENTER(TOP_LAYER, "ja_task_list_create_from_file");

   ja_tasks = lCreateList("ja_tasks", JAT_Type); 
   if (!ja_tasks) {
      DTRACE;
      goto error;
   }
   sge_get_file_path(spool_dir_job, JOB_SPOOL_DIR, FORMAT_DEFAULT, flags, 
                     job_id, ja_task_id, NULL);
   dir_entries = sge_get_dirents(spool_dir_job);
   for_each(dir_entry, dir_entries) {
      const char *entry;
 
      entry = lGetString(dir_entry, ST_name);
      if (strcmp(entry, ".") && strcmp(entry, "..") && 
          strcmp(entry, "common")) {
         char spool_dir_tasks[SGE_PATH_MAX];
         lListElem *ja_task_entry; 

         sprintf(spool_dir_tasks, SFN"/"SFN, spool_dir_job, entry);
         ja_task_entries = sge_get_dirents(spool_dir_tasks);
         for_each(ja_task_entry, ja_task_entries) {
            const char *ja_task_string;

            ja_task_string = lGetString(ja_task_entry, ST_name);
            if (strcmp(ja_task_string, ".") && strcmp(ja_task_string, "..")) {
               char spool_dir_pe_tasks[SGE_PATH_MAX];
               lListElem *pe_task_entry;
               u_long32 ja_task_id;
               lListElem *ja_task;

               ja_task_id = atol(ja_task_string);
               if (ja_task_id == 0) {
                  DTRACE;
                  goto error;
               }
               sprintf(spool_dir_pe_tasks, SFN"/"SFN, spool_dir_tasks,
                       ja_task_string);

               if (sge_is_directory(spool_dir_pe_tasks)) {
                  char spool_path_ja_task[SGE_PATH_MAX];

                  sge_get_file_path(spool_path_ja_task, TASK_SPOOL_FILE,
                                    FORMAT_DEFAULT, flags, job_id, ja_task_id, NULL);
                  ja_task = lReadElemFromDisk(NULL, spool_path_ja_task, JAT_Type, "ja_task");
                  pe_tasks = NULL;
                  pe_task_entries = sge_get_dirents(spool_dir_pe_tasks);
                  for_each(pe_task_entry, pe_task_entries) {
                     const char *pe_task_string;

                     pe_task_string = lGetString(pe_task_entry, ST_name);
                     if (strcmp(pe_task_string, ".") && 
                         strcmp(pe_task_string, "..") &&
                         strcmp(pe_task_string, "common")) {
                        lListElem *pe_task;
                        
                        pe_task = pe_task_create_from_file(job_id, ja_task_id, pe_task_string, flags);
                        if (pe_task) {
                           if (!pe_tasks) {
                              pe_tasks = lCreateList("pe_tasks", PET_Type); 
                           }
                           lAppendElem(pe_tasks, pe_task);
                        } else {
                           DTRACE;
                           goto error;
                        }
                     }
                  }
                  lFreeList(&pe_task_entries);
                  lSetList(ja_task, JAT_task_list, pe_tasks);
               } else {
                  ja_task = ja_task_create_from_file(job_id, ja_task_id, NULL, flags);
               }
               if (ja_task) {
                  lAppendElem(ja_tasks, ja_task);
               } else {
                  DTRACE;
                  goto error;
               }
            }
         } 
         lFreeList(&ja_task_entries);
      }
   }
   lFreeList(&dir_entries);

   if (!lGetNumberOfElem(ja_tasks)) {
      DTRACE;
      goto error; 
   } 
   DEXIT;
   return ja_tasks;
error:
   lFreeList(&ja_tasks);
   lFreeList(&dir_entries);
   lFreeList(&ja_task_entries);  
   lFreeList(&pe_task_entries);
   DEXIT;
   return NULL; 
}

static lListElem *ja_task_create_from_file(u_long32 job_id, 
                                           u_long32 ja_task_id, 
                                           const char *pe_task_id,
                                           sge_spool_flags_t flags) 
{
   lListElem *ja_task;
   char spool_path_ja_task[SGE_PATH_MAX];

   sge_get_file_path(spool_path_ja_task, TASK_SPOOL_DIR_AS_FILE,
                     FORMAT_DEFAULT, flags, job_id, ja_task_id, NULL);
   ja_task = lReadElemFromDisk(NULL, spool_path_ja_task, JAT_Type, "ja_task"); 
   return ja_task;
}

static lListElem *pe_task_create_from_file(u_long32 job_id,
                                           u_long32 ja_task_id,
                                           const char *pe_task_id,
                                           sge_spool_flags_t flags)
{
   lListElem *pe_task;
   char spool_path_pe_task[SGE_PATH_MAX];

   sge_get_file_path(spool_path_pe_task, PE_TASK_SPOOL_FILE,
                     FORMAT_DEFAULT, flags, job_id, ja_task_id, pe_task_id);
   pe_task = lReadElemFromDisk(NULL, spool_path_pe_task, PET_Type, "pe_task"); 
   return pe_task;
   
}

/****** spool/classic/job_write_spool_file() **********************************
*  NAME
*     job_write_spool_file() -- makes a job/task persistent 
*
*  SYNOPSIS
*     int job_write_spool_file(lListElem *job, u_long32 ja_taskid, 
*                              sge_spool_flags_t flags) 
*
*  FUNCTION
*     This function writes a job or a task of an array job into the spool 
*     area. It may be used within the qmaster or execd code.
*   
*     The result from this function looks like this within the spool area
*     of the master for the job 10001, the array job 10002.1-3, 
*     the tightly integrated job 20011 (two pe_tasks).
*     
*   
*     $SGE_ROOT/default/spool/qmaster/jobs
*     +---00
*         +---0001
*         |   +---0001                     (JB_Type file)
*         |   +---0002 
*         |       +---common               (JB_Type without JB_ja_tasks)
*         |       +---1-4096
*         |           +---1                (JAT_Type file) 
*         |           +---2                (JAT_Type file)
*         |           +---3                (JAT_Type file)
*         +---0002
*             +---0011
*                 +---common               (JB_Type without JB_ja_tasks)
*                 +---1-4096
*                     +---1
*                         +--- common      (JAT_Type file witout JAT_task_list)
*                         +--- 1.speedy    (PET_Type file)
*                         +--- 2.speedy    (PET_Type file)
*                         +--- past_usage  (PET_Type file)
*
*  INPUTS
*     lListElem *job          - full job (JB_Type) 
*     u_long32 ja_taskid      - 0 or a allowed array job task id 
*     const char *pe_task_id  - pe task id
*     sge_spool_flags_t flags - where/how should we spool the object 
*        SPOOL_HANDLE_AS_ZOMBIE   -> has to be used for zombie jobs 
*        SPOOL_WITHIN_EXECD       -> has to be used within the execd 
*        SPOOL_DEFAULT            -> if no other flags are needed
*
*  RESULT
*     int - 0 on success otherwise != 0 
******************************************************************************/
int job_write_spool_file(lListElem *job, u_long32 ja_taskid, 
                         const char *pe_task_id,
                         sge_spool_flags_t flags) 
{
   int ret = 0;
   int report_long_delays = flags & SPOOL_WITHIN_EXECD;
   u_long32 start = 0;
   
   DENTER(TOP_LAYER, "job_write_spool_file");

   if (report_long_delays) {
      start = sge_get_gmt();
   }

   if (job_has_to_spool_one_file(job, *object_type_get_master_list(SGE_TYPE_PE), 
                                        flags)) {
      ret = job_write_as_single_file(job, ja_taskid, flags); 
   } else {
      if (!(flags & SPOOL_ONLY_JATASK) && !(flags & SPOOL_ONLY_PETASK)) {
         ret = job_write_common_part(job, ja_taskid, flags);
      }
      if (!ret && !(flags & SPOOL_IGNORE_TASK_INSTANCES)) {
         ret = job_write_ja_task_part(job, ja_taskid, pe_task_id, flags); 
      }
   }

   if (report_long_delays) {
      u_long32 time = sge_get_gmt() - start;
      if (time > 30) {
         /* administrators need to be aware of suspicious spooling delays */
         WARNING((SGE_EVENT, MSG_CONFIG_JOBSPOOLINGLONGDELAY_UUI, 
         sge_u32c(lGetUlong(job, JB_job_number)), sge_u32c(ja_taskid), (int)time));
      }
   }

   DRETURN(ret);
}

static int job_has_to_spool_one_file(const lListElem *job, 
                                     const lList *pe_list,
                                     sge_spool_flags_t flags) 
{
   DENTER(TOP_LAYER, "job_has_to_spool_one_file");

   if ((flags & SPOOL_HANDLE_AS_ZOMBIE) || (flags & SPOOL_WITHIN_EXECD)) {
      DRETURN(1);
   } 
   
   if (job_might_be_tight_parallel(job, pe_list)
               || (job_get_submit_ja_tasks(job) > sge_get_ja_tasks_per_file())) {
      DRETURN(0);
   }

   DRETURN(1);
}

static int job_write_as_single_file(lListElem *job, u_long32 ja_task_id,
                                   sge_spool_flags_t flags) 
{
   int ret = 0;
   u_long32 job_id;
   char job_dir_third[SGE_PATH_MAX] = "";
   char spool_file[SGE_PATH_MAX] = "";
   char tmp_spool_file[SGE_PATH_MAX] = "";

   DENTER(TOP_LAYER, "job_write_as_single_file");
   job_id = lGetUlong(job, JB_job_number);

   sge_get_file_path(job_dir_third, JOB_SPOOL_DIR, FORMAT_THIRD_PART,
                     flags, job_id, ja_task_id, NULL);
   sge_mkdir(job_dir_third, 0755, false, false);
   sge_get_file_path(spool_file, JOB_SPOOL_DIR_AS_FILE, FORMAT_DEFAULT,
                     flags, job_id, ja_task_id, NULL);
   sge_get_file_path(tmp_spool_file, JOB_SPOOL_DIR_AS_FILE, FORMAT_DOT_FILENAME,
                     flags, job_id, ja_task_id, NULL);
   ret = lWriteElemToDisk(job, tmp_spool_file, NULL, "job");
   if (!ret && (rename(tmp_spool_file, spool_file) == -1)) {
      DTRACE;
      ret = 1;
   }

   DEXIT;
   return ret;  
}

static int job_write_ja_task_part(lListElem *job, u_long32 ja_task_id,
                                  const char *pe_task_id,
                                  sge_spool_flags_t flags)
{
   lListElem *ja_task, *next_ja_task;
   u_long32 job_id;
   int ret = 0;
   DENTER(TOP_LAYER, "job_write_ja_task_part"); 

   job_id = lGetUlong(job, JB_job_number);
   if (ja_task_id != 0) {
      next_ja_task = lGetElemUlong(lGetList(job, JB_ja_tasks),
                                   JAT_task_number, ja_task_id);
   } else {
      next_ja_task = lFirst(lGetList(job, JB_ja_tasks));
   }
   while ((ja_task = next_ja_task)) {
      if (ja_task_id != 0) {
         next_ja_task = NULL;
      } else {
         next_ja_task = lNext(ja_task);
      }

      if ((flags & SPOOL_WITHIN_EXECD) ||
         job_is_enrolled(job, lGetUlong(ja_task, JAT_task_number))) {
         if (job_might_be_tight_parallel(job, *object_type_get_master_list(SGE_TYPE_PE))) {
            flags |= SPOOL_HANDLE_PARALLEL_TASKS;
         }

         ret = ja_task_write_to_disk(ja_task, job_id, pe_task_id, flags);
         if (ret) {
            DTRACE;
            break;
         }
      }
   }
   DRETURN(ret);
}

int job_write_common_part(lListElem *job, u_long32 ja_task_id,
                                 sge_spool_flags_t flags) 
{
   int ret = 0;
   u_long32 job_id;
   char spool_dir[SGE_PATH_MAX];
   char spoolpath_common[SGE_PATH_MAX], tmp_spoolpath_common[SGE_PATH_MAX];
   lList *ja_tasks;

   DENTER(TOP_LAYER, "job_write_common_part");

   job_id = lGetUlong(job, JB_job_number);
   sge_get_file_path(spool_dir, JOB_SPOOL_DIR, FORMAT_DEFAULT,
                     flags, job_id, ja_task_id, NULL);
   sge_mkdir(spool_dir, 0755, false, false);
   sge_get_file_path(spoolpath_common, JOB_SPOOL_FILE, FORMAT_DEFAULT,
                     flags, job_id, ja_task_id, NULL);
   sge_get_file_path(tmp_spoolpath_common, JOB_SPOOL_FILE,
                     FORMAT_DOT_FILENAME, flags, job_id, ja_task_id, NULL);

   ja_tasks = NULL;
   lXchgList(job, JB_ja_tasks, &ja_tasks);
   ret = lWriteElemToDisk(job, tmp_spoolpath_common, NULL, "job");
   lXchgList(job, JB_ja_tasks, &ja_tasks);

   if (!ret && (rename(tmp_spoolpath_common, spoolpath_common) == -1)) {
      DTRACE;
      ret = 1;
   }

   DEXIT;
   return ret;
}



static int ja_task_write_to_disk(lListElem *ja_task, u_long32 job_id,
                                 const char *pe_task_id,
                                 sge_spool_flags_t flags)
{
   int handle_pe_tasks = flags & SPOOL_HANDLE_PARALLEL_TASKS;
   int ret = 0;
   DENTER(TOP_LAYER, "ja_task_write_to_disk");

   /* this is a tightly integrated parallel job */
   if (handle_pe_tasks) {
      char task_spool_dir[SGE_PATH_MAX];
      char task_spool_file[SGE_PATH_MAX];
      char tmp_task_spool_file[SGE_PATH_MAX];
      lListElem *pe_task = NULL;
      lListElem *next_pe_task = NULL;
      u_long32 ja_task_id = lGetUlong(ja_task, JAT_task_number);
      lList *pe_task_list = lGetList(ja_task, JAT_task_list);

      sge_get_file_path(task_spool_dir, TASK_SPOOL_DIR, FORMAT_DEFAULT, flags,
                        job_id, ja_task_id, NULL);
      sge_get_file_path(task_spool_file, TASK_SPOOL_FILE, FORMAT_DEFAULT, flags,
                        job_id, ja_task_id, NULL);
      sge_get_file_path(tmp_task_spool_file, TASK_SPOOL_FILE, 
                        FORMAT_DOT_FILENAME, flags, job_id, ja_task_id, NULL);

      /* create task spool directory if necessary */
      if ((flags & SPOOL_WITHIN_EXECD) || 
          strcmp(old_task_spool_dir, task_spool_dir)) {
         strcpy(old_task_spool_dir, task_spool_dir);
         sge_mkdir(task_spool_dir, 0755, false, false);
      }

      /* spool ja_task */
      if (!(flags & SPOOL_ONLY_PETASK)) {
         lList *tmp_task_list = NULL;

         /* we do *not* want to spool the pe_tasks together with the ja_task */
         lXchgList(ja_task, JAT_task_list, &tmp_task_list);
         /* spool ja_task to temporary file */
         ret = lWriteElemToDisk(ja_task, tmp_task_spool_file, NULL, "ja_task");
         lXchgList(ja_task, JAT_task_list, &tmp_task_list);
         /* make spooling permanent = COMMIT */
         if (!ret && (rename(tmp_task_spool_file, task_spool_file) == -1)) {
            DTRACE;
            goto error;
         }
      }

      /* spool pe_task(s) */
      if (!(flags & SPOOL_ONLY_JATASK)) {
         if (pe_task_id != 0) {
            next_pe_task = lGetElemStr(pe_task_list, PET_id, pe_task_id);
         } else {
            next_pe_task = lFirst(pe_task_list);
         }
         while ((pe_task = next_pe_task)) {
            char pe_task_spool_file[SGE_PATH_MAX];
            char tmp_pe_task_spool_file[SGE_PATH_MAX];
            const char* pe_task_id_string = lGetString(pe_task, PET_id);

            if (pe_task_id != 0) {
               next_pe_task = NULL;
            } else {
               next_pe_task = lNext(pe_task);
            }

            DTRACE;

            sge_get_file_path(pe_task_spool_file, PE_TASK_SPOOL_FILE, 
                              FORMAT_DEFAULT, flags, job_id, ja_task_id, 
                              pe_task_id_string);
            sge_get_file_path(tmp_pe_task_spool_file, PE_TASK_SPOOL_FILE, 
                              FORMAT_DOT_FILENAME, flags, job_id, ja_task_id, 
                              pe_task_id_string);

            /* spool pe_task to temporary file */
            ret = lWriteElemToDisk(pe_task, tmp_pe_task_spool_file, NULL, "pe_task");
            /* make spooling permanent = COMMIT */
            if (!ret && (rename(tmp_pe_task_spool_file, pe_task_spool_file) == -1)) {
               DTRACE;
               goto error;
            }
      
            DTRACE;
         }
      }
   } else {
      /* this is a non tightly parallel array task */
      char task_spool_dir[SGE_PATH_MAX];
      char task_spool_file[SGE_PATH_MAX];
      char tmp_task_spool_file[SGE_PATH_MAX];

      sge_get_file_path(task_spool_dir, TASKS_SPOOL_DIR, FORMAT_DEFAULT, flags,
                        job_id, lGetUlong(ja_task, JAT_task_number), NULL);
      sge_get_file_path(task_spool_file, TASK_SPOOL_DIR_AS_FILE, 
                        FORMAT_DEFAULT, flags, job_id, 
                        lGetUlong(ja_task, JAT_task_number), NULL);
      sge_get_file_path(tmp_task_spool_file, TASK_SPOOL_DIR_AS_FILE, 
                        FORMAT_DOT_FILENAME, flags, job_id, 
                        lGetUlong(ja_task, JAT_task_number), NULL);

      /* create task spool directory if necessary */
      if ((flags & SPOOL_WITHIN_EXECD) ||
          strcmp(old_task_spool_dir, task_spool_dir)) {
         strcpy(old_task_spool_dir, task_spool_dir);
         sge_mkdir(task_spool_dir, 0755, false, false);
      }

      /* spool ja_task to temporary file */
      ret = lWriteElemToDisk(ja_task, tmp_task_spool_file, NULL, "ja_task");
      /* make spooling permanent = COMMIT */
      if (!ret && (rename(tmp_task_spool_file, task_spool_file) == -1)) {
         DTRACE;
         goto error;
      }    
   }

error:
   DRETURN(ret);
}

int job_remove_spool_file(u_long32 jobid, u_long32 ja_taskid, 
                          const char *pe_task_id,
                          sge_spool_flags_t flags)
{
   char spool_dir[SGE_PATH_MAX] = "";
   char spool_dir_second[SGE_PATH_MAX] = "";
   char spool_dir_third[SGE_PATH_MAX] = "";
   char spoolpath_common[SGE_PATH_MAX] = "";
   int within_execd = flags & SPOOL_WITHIN_EXECD;
   int handle_as_zombie = flags & SPOOL_HANDLE_AS_ZOMBIE;
   int one_file;
   lList *master_list = handle_as_zombie ? 
                        *(object_type_get_master_list(SGE_TYPE_ZOMBIE)) : 
                        *(object_type_get_master_list(SGE_TYPE_JOB));
   lListElem *job = job_list_locate(master_list, jobid);
   int try_to_remove_sub_dirs = 0;
   dstring error_msg;
   char error_msg_buffer[SGE_PATH_MAX];

   DENTER(TOP_LAYER, "job_remove_spool_file");

   sge_dstring_init(&error_msg, error_msg_buffer, sizeof(error_msg_buffer));

   one_file = job_has_to_spool_one_file(job, *object_type_get_master_list(SGE_TYPE_PE), 
                                         flags);
   if (ja_taskid != 0 && pe_task_id != NULL && !one_file) {
       char pe_task_spool_file[SGE_PATH_MAX];

       sge_get_file_path(pe_task_spool_file, PE_TASK_SPOOL_FILE, 
                         FORMAT_DEFAULT, flags, jobid, ja_taskid, pe_task_id);
      
       DPRINTF(("try to remove "SFN"\n", pe_task_spool_file));
       if (sge_is_file(pe_task_spool_file) &&
           !sge_unlink(NULL, pe_task_spool_file)) {
         ERROR((SGE_EVENT, MSG_JOB_CANNOT_REMOVE_SS, 
                MSG_JOB_PE_TASK_SPOOL_FILE, pe_task_spool_file));
         DTRACE;
      }
   }

   if (ja_taskid != 0 && pe_task_id == NULL && !one_file) {
      char task_spool_dir[SGE_PATH_MAX];
      char task_spool_file[SGE_PATH_MAX];
      int remove_task_spool_file = 0;

      sge_get_file_path(task_spool_dir, TASKS_SPOOL_DIR, FORMAT_DEFAULT, flags,
                        jobid, ja_taskid, NULL);
      sge_get_file_path(task_spool_file, TASK_SPOOL_DIR_AS_FILE, 
                        FORMAT_DEFAULT, flags, jobid, ja_taskid, NULL);

      if (within_execd) {
         remove_task_spool_file = 1;
      } else {
         remove_task_spool_file = job_is_enrolled(job, ja_taskid);
      }
      DPRINTF(("remove_task_spool_file = %d\n", remove_task_spool_file));;

      if (remove_task_spool_file) {
         DPRINTF(("removing "SFN"\n", task_spool_file));
         if (sge_is_directory(task_spool_file)) {
            if (sge_rmdir(task_spool_file, &error_msg)) {
               ERROR((SGE_EVENT, MSG_JOB_CANNOT_REMOVE_SS, MSG_JOB_TASK_SPOOL_FILE, error_msg_buffer));
            } 
         } else {
            if (!sge_unlink(NULL, task_spool_file)) {
               ERROR((SGE_EVENT, MSG_JOB_CANNOT_REMOVE_SS, MSG_JOB_TASK_SPOOL_FILE, task_spool_file));
            }
         }

         /*
          * The following rmdir call may fail.  We can ignore
          * ENOTEMPTY as that's only an indicator that another task is
          * running which has been spooled in the directory.
          */  
         DPRINTF(("try to remove "SFN"\n", task_spool_dir));
         errno = 0;
         if (rmdir(task_spool_dir)) {
            if (errno != ENOTEMPTY)
               ERROR((SGE_EVENT, MSG_JOB_CANNOT_REMOVE_SS,
                      MSG_JOB_TASK_SPOOL_FILE, strerror(errno)));
         } 

         /* 
          * a task spool directory has been removed: reinit 
          * old_task_spool_dir to ensure mkdir() is performed 
          */
         old_task_spool_dir[0] = '\0';
      }
   }

   sge_get_file_path(spool_dir, JOB_SPOOL_DIR, 
                     FORMAT_DEFAULT, flags, jobid, ja_taskid, NULL);
   sge_get_file_path(spool_dir_third, JOB_SPOOL_DIR, 
                     FORMAT_THIRD_PART, flags, jobid, ja_taskid, NULL);
   sge_get_file_path(spool_dir_second, JOB_SPOOL_DIR, 
                     FORMAT_SECOND_PART, flags, jobid, ja_taskid, NULL);
   sge_get_file_path(spoolpath_common, JOB_SPOOL_FILE, 
                     FORMAT_DEFAULT, flags, jobid, ja_taskid, NULL);
   try_to_remove_sub_dirs = 0;
   if (!one_file) {
      if (ja_taskid == 0) { 
         DPRINTF(("removing "SFN"\n", spoolpath_common));
         if (!sge_unlink(NULL, spoolpath_common)) {
            ERROR((SGE_EVENT, MSG_JOB_CANNOT_REMOVE_SS, MSG_JOB_JOB_SPOOL_FILE, spoolpath_common)); 
         }
         DPRINTF(("removing "SFN"\n", spool_dir));
         if (sge_rmdir(spool_dir, NULL)) {
            ERROR((SGE_EVENT, MSG_JOB_CANNOT_REMOVE_SS, MSG_JOB_JOB_SPOOL_DIRECTORY, spool_dir));
         }
         try_to_remove_sub_dirs = 1;
      }
   } else {
      DPRINTF(("removing "SFN"\n", spool_dir));
      if (!sge_unlink(NULL, spool_dir)) {
         ERROR((SGE_EVENT, MSG_JOB_CANNOT_REMOVE_SS, MSG_JOB_JOB_SPOOL_FILE,
                spool_dir));
      }
      try_to_remove_sub_dirs = 1;
   }
   /*
    * The following rmdir calls may fail.  We can ignore ENOTEMPTY as
    * that's only an indicator that another task is running which has
    * been spooled in the directory.
    */  
   if (try_to_remove_sub_dirs) {
      DPRINTF(("try to remove "SFN"\n", spool_dir_third));

      errno = 0;
      if (rmdir(spool_dir_third)) {
         if (errno != ENOTEMPTY)
            ERROR((SGE_EVENT, MSG_JOB_CANNOT_REMOVE_SS,
                   MSG_JOB_TASK_SPOOL_FILE, strerror(errno)));
         DPRINTF(("try to remove "SFN"\n", spool_dir_second));
         errno = 0;
         if (rmdir(spool_dir_second))
            if (errno != ENOTEMPTY)
               ERROR((SGE_EVENT, MSG_JOB_CANNOT_REMOVE_SS,
                      MSG_JOB_TASK_SPOOL_FILE, strerror(errno)));
      }
   }

   DEXIT;
   return 0;
}

static int job_remove_script_file(u_long32 job_id)
{
   char script_file[SGE_PATH_MAX] = "";
   int ret = 0;
   DENTER(TOP_LAYER, "job_remove_script_file");

   PROF_START_MEASUREMENT(SGE_PROF_JOBSCRIPT);
   sge_get_file_path(script_file, JOB_SCRIPT_FILE, FORMAT_DEFAULT,
                     SPOOL_DEFAULT, job_id, 0, NULL); 
   if (unlink(script_file)) {
      if (errno!=ENOENT) {
         ERROR((SGE_EVENT, MSG_CONFIG_FAILEDREMOVINGSCRIPT_SS,
              strerror(errno), script_file)); 
         DTRACE;
         ret = 1;
      }
   } else {
      INFO((SGE_EVENT, MSG_CONFIG_REMOVEDSCRIPTOFBADJOBFILEX_S, script_file));
   }
   PROF_STOP_MEASUREMENT(SGE_PROF_JOBSCRIPT);
   DEXIT;
   return ret;
}

int job_list_read_from_disk(lList **job_list, char *list_name, int check,
                                sge_spool_flags_t flags, 
                                int (*init_function)(lListElem*)) 
{
   char first_dir[SGE_PATH_MAX] = ""; 
   lList *first_direnties; 
   lListElem *first_direntry;
   char path[SGE_PATH_MAX];
   int handle_as_zombie = (flags & SPOOL_HANDLE_AS_ZOMBIE) > 0;

   DENTER(TOP_LAYER, "job_read_job_list_from_disk"); 
   sge_get_file_path(first_dir, JOBS_SPOOL_DIR, FORMAT_FIRST_PART, 
                     flags, 0, 0, NULL);  
   first_direnties = sge_get_dirents(first_dir);

   if (first_direnties && !sge_silent_get()) {
      printf(MSG_CONFIG_READINGIN_S, list_name);
      printf("\n");
   }

   sge_status_set_type(STATUS_DOTS);
   for (;
        (first_direntry = lFirst(first_direnties)); 
        lRemoveElem(first_direnties, &first_direntry)) {
      char second_dir[SGE_PATH_MAX] = "";
      lList *second_direnties;
      lListElem *second_direntry;
      const char *first_entry_string;


      first_entry_string = lGetString(first_direntry, ST_name);
      sprintf(path, "%s/%s", first_dir, first_entry_string);
      if (!sge_is_directory(path)) {
         ERROR((SGE_EVENT, MSG_CONFIG_NODIRECTORY_S, path)); 
         break;
      }
   
      sprintf(second_dir, SFN"/"SFN, first_dir, first_entry_string); 
      second_direnties = sge_get_dirents(second_dir);
      for (;
           (second_direntry = lFirst(second_direnties));
           lRemoveElem(second_direnties, &second_direntry)) {
         char third_dir[SGE_PATH_MAX] = "";
         lList *third_direnties;
         lListElem *third_direntry;
         const char *second_entry_string;

         second_entry_string = lGetString(second_direntry, ST_name);
         sprintf(path, "%s/%s/%s", first_dir, first_entry_string,
                 second_entry_string);
         if (!sge_is_directory(path)) {
            ERROR((SGE_EVENT, MSG_CONFIG_NODIRECTORY_S, path));
            break;
         } 

         sprintf(third_dir, SFN"/"SFN, second_dir, second_entry_string);
         third_direnties = sge_get_dirents(third_dir);
         for (;
              (third_direntry = lFirst(third_direnties));
              lRemoveElem(third_direnties, &third_direntry)) {
            lListElem *job, *ja_task;
            char *lasts = NULL;
            char job_dir[SGE_PATH_MAX] = "";
            char fourth_dir[SGE_PATH_MAX] = "";
            char job_id_string[SGE_PATH_MAX] = "";
            char *ja_task_id_string;
            u_long32 job_id, ja_task_id;
            int all_finished;

            sge_status_next_turn();
            sprintf(fourth_dir, SFN"/"SFN, third_dir,
                    lGetString(third_direntry, ST_name));
            sprintf(job_id_string, SFN SFN SFN, 
                    lGetString(first_direntry, ST_name),
                    lGetString(second_direntry, ST_name),
                    lGetString(third_direntry, ST_name)); 
            job_id = (u_long32) strtol(job_id_string, NULL, 10);
            strtok_r(job_id_string, ".", &lasts);
            ja_task_id_string = strtok_r(NULL, ".", &lasts);
            if (ja_task_id_string) {
               ja_task_id = (u_long32) strtol(ja_task_id_string, NULL, 10);
            } else {
               ja_task_id = 0;
            }
            sge_get_file_path(job_dir, JOB_SPOOL_DIR, FORMAT_DEFAULT,
                              flags, job_id, ja_task_id, NULL);

            /* check directory name */
            if (strcmp(fourth_dir, job_dir)) {
               fprintf(stderr, "%s %s\n", fourth_dir, job_dir);
               DPRINTF(("Invalid directory "SFN"\n", fourth_dir));
               continue;
            }

            /* read job */
            job = job_create_from_file(job_id, ja_task_id, flags);
            if (!job) {
               job_remove_script_file(job_id);
               continue;
            }

            /* check for scriptfile before adding job */
            all_finished = 1;
            for_each (ja_task, lGetList(job, JB_ja_tasks)) {
               if (lGetUlong(ja_task, JAT_status) != JFINISHED) {
                  all_finished = 0;
                  break;
               }
            }
            if (check && !all_finished && lGetString(job, JB_script_file)) {
               char script_file[SGE_PATH_MAX];
               SGE_STRUCT_STAT stat_buffer;

               sge_get_file_path(script_file, JOB_SCRIPT_FILE, FORMAT_DEFAULT,
                                 flags, job_id, 0, NULL);
               if (SGE_STAT(script_file, &stat_buffer)) {
                  ERROR((SGE_EVENT, MSG_CONFIG_CANTFINDSCRIPTFILE_U,
                         sge_u32c(lGetUlong(job, JB_job_number))));
                  job_list_add_job(object_type_get_master_list(SGE_TYPE_JOB), "job list", job, 0);
                  job_remove_spool_file(job_id, 0, NULL, SPOOL_DEFAULT);
                  lRemoveElem( *(object_type_get_master_list(SGE_TYPE_JOB)), &job);
                  continue;
               }
            }  
 
            /* check if filename has same name which is stored job id */
            if (lGetUlong(job, JB_job_number) != job_id) {
               ERROR((SGE_EVENT, MSG_CONFIG_JOBFILEXHASWRONGFILENAMEDELETING_U,
                     sge_u32c(job_id)));
               job_remove_spool_file(job_id, 0, NULL, flags);
               /* 
                * script is not deleted here, 
                * since it may belong to a valid job 
                */
            } 

            if (init_function) {
               init_function(job);
            }

            lSetList(job, JB_jid_successor_list, NULL); 
            job_list_add_job(job_list, list_name, job, 0);
            
            if (!handle_as_zombie) {
               job_list_register_new_job(*(object_type_get_master_list(SGE_TYPE_JOB)), mconf_get_max_jobs(), 1);
               suser_register_new_job(job, mconf_get_max_u_jobs(), 1);
            }
         }
         lFreeList(&third_direnties);
      }
      lFreeList(&second_direnties);
   } 
   lFreeList(&first_direnties);

   if (*job_list) {
      sge_status_end_turn();
   }      

   DEXIT;
   return 0;
}
