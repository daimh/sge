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
#include <fcntl.h>
#include <sys/types.h>

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"

#include "lock.h"

/*-------------------------------------------------------------
 * Name:   qmaster_lock
 * Return: 0 if creation of file was successfull
 *         -1 if file couldn't be created
 *-------------------------------------------------------------*/
int qmaster_lock(
char *file 
) {
   int fd;

   DENTER(TOP_LAYER, "qmaster_lock");

   fd = SGE_OPEN3(file, O_RDWR | O_CREAT | O_EXCL, 0600);
   if (fd == -1) {
      DEXIT;
      return -1;
   }
   else {
      close(fd);
      DEXIT;
      return 0;
   }
}

/*-------------------------------------------------------------
 * Name:   qmaster_unlock
 * Return: 0 if file could be unlinked
 *         -1 if file couldn't be unlinked
 *-------------------------------------------------------------*/
int qmaster_unlock(
char *file 
) {
   int ret;

   DENTER(TOP_LAYER, "qmaster_unlock");

   ret = unlink(file);

   DEXIT;
   return ret;
}

/*-------------------------------------------------------------
 * Name:   isLocked
 * Return: 1 if file exists
 *         0 if file doesn't exist
 *-------------------------------------------------------------*/
int isLocked(
char *file 
) {
   int ret;
   SGE_STRUCT_STAT sb;

   DENTER(TOP_LAYER, "isLocked");

   ret = SGE_STAT(file, &sb);
   ret = ret ? 0 : 1;

   DEXIT;
   return ret;
}
