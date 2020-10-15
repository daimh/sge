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
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#include "uti/sge_rmon.h"

#include "cull/cull.h"

#include "uti/sge_log.h"
#include "uti/sge_unistd.h"
#include "uti/msg_utilib.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_str.h"

#include "spool/sge_dirent.h"

#include "msg_common.h"

/****
 **** sge_get_dirents
 ****
 **** Returns the entries of the specified directory
 **** (without . and ..) as lList of type ST_Type.
 **** The lList has to be freed from the caller.
 **** On any error, NULL is returned.
 ****/
lList *sge_get_dirents(const char *path) {
   lList *entries = NULL;
   DIR *cwd;
   SGE_STRUCT_DIRENT *dent;
   char dirent[SGE_PATH_MAX*2];

   DENTER(TOP_LAYER, "sge_get_dirents");

   cwd = opendir(path);

   if (cwd == (DIR *) 0) {
      ERROR((SGE_EVENT, MSG_FILE_CANTOPENDIRECTORYX_SS, path, strerror(errno)));
      return (NULL);
   }

   while (SGE_READDIR_R(cwd, (SGE_STRUCT_DIRENT *)dirent, &dent)==0 && dent!=NULL) {
      if (!dent->d_name)
         continue;              
      if (!dent->d_name[0])
         continue;             
      if (strcmp(dent->d_name, "..") == 0 || strcmp(dent->d_name, ".") == 0)
         continue;
      lAddElemStr(&entries, ST_name, dent->d_name, ST_Type);
   }
   closedir(cwd);

   DRETURN(entries);
}

u_long32 sge_count_dirents(char *directory_name) 
{
   lList *dir_entries;
   lListElem *dir_entry;
   u_long32 entries = 0;

   dir_entries = sge_get_dirents(directory_name);
   for_each(dir_entry, dir_entries) {
      const char *entry;

      entry = lGetString(dir_entry, ST_name);
      if (strcmp(entry, ".") && strcmp(entry, "..")) {
         entries++;
      }
   }
   lFreeList(&dir_entries);
   return entries;
}

int has_more_dirents(char *directory_name, u_long32 number_of_entries)
{
   lList *dir_entries;
   lListElem *dir_entry;
   u_long32 entries = 0;
   int ret = 0;
 
   dir_entries = sge_get_dirents(directory_name);
   for_each(dir_entry, dir_entries) {
      const char *entry;
 
      entry = lGetString(dir_entry, ST_name);
      if (strcmp(entry, ".") && strcmp(entry, "..")) {
         entries++;
         if (entries > number_of_entries) {
            ret = 1;
            break;
         }
      }
   }
   lFreeList(&dir_entries);
   return ret;  
}
