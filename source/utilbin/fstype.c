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
 *  Portions of this software are Copyright (c) 2011 Univa Corporation
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <errno.h>
#include <string.h>

#if defined(DARWIN) || defined(FREEBSD) || defined(NETBSD)
#  include <sys/param.h>
#  include <sys/mount.h>
#elif defined(LINUX)
#  include <sys/vfs.h>
#  include "sge_string.h"
#  include <mntent.h>
#else
#  include <sys/types.h>
#  include <sys/statvfs.h>
#endif

#if defined(SOLARIS)
#include <kstat.h>
#include <nfs/nfs.h>
#include <nfs/nfs_clnt.h>
#endif

#if defined(INTERIX)
#  include "wingrid.h"
#endif

#define BUF_SIZE 8 * 1024

/* fixme:  what about others?  Lustre, Ceph, GFS, GPFS & Gluster are
   supposed to be POSIX-compliant, so presumably are OK.  */

int main(int argc, char *argv[]) {

   int ret=1;

   if (argc < 2) {
      printf("Usage: fstype <directory>\n");
      return 1;
   } else {
#if defined(LINUX)
   struct statfs buf;
   FILE *fd = NULL;
   ret = statfs(argv[1], &buf);
#elif defined(DARWIN) || defined(FREEBSD) || (defined(NETBSD) && !defined(ST_RDONLY))
   struct statfs buf;
   ret = statfs(argv[1], &buf);
#elif defined(INTERIX)
   struct statvfs buf;
   ret = wl_statvfs(argv[1], &buf);
#elif defined(SOLARIS)
   struct statvfs buf;
   struct mntinfo_kstat mnt_info;
   minor_t fsid;
   kstat_ctl_t    *kc = NULL;
   kstat_t        *ksp;
   kstat_named_t  *knp;

   ret = statvfs(argv[1], &buf);
   /*
      statfs returns dev_t (32bit - 14bit major + 18bit minor number)
      the kstat_instance is the minor number
   */
   fsid = (minor_t)(buf.f_fsid & 0x3ffff);

   if (strcmp(buf.f_basetype, "nfs") == 0) {
            kc = kstat_open();
            for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
               if (ksp->ks_type != KSTAT_TYPE_RAW)
			         continue;
		         if (strcmp(ksp->ks_module, "nfs") != 0)
			         continue;
		         if (strcmp(ksp->ks_name, "mntinfo") != 0)
			         continue;
               if (kstat_read(kc, ksp, &mnt_info) == -1) {
                  kstat_close(kc);
                  printf("error\n");
                  return 2;
               }
               if (fsid  == ksp->ks_instance) {
                  if ( mnt_info.mik_vers >= 4 ) {
                     snprintf(buf.f_basetype, sizeof buf.f_basetype,
                              "nfs%i", mnt_info.mik_vers);
                  }
                  break;
               }
            }
            ret = kstat_close(kc);
   }
#else
   struct statvfs buf;
   ret = statvfs(argv[1], &buf);
#endif

   if(ret!=0) {
      printf("Error: %s\n", strerror(errno));
      return 2;
   }

#if defined (DARWIN) || defined(FREEBSD) || defined(NETBSD)
   printf("%s\n", buf.f_fstypename);
#elif defined(LINUX)
   /* 0x6969 is NFS_SUPER_MAGIC (see statfs(2) man page) */
   /* See also more values in linux/magic.h (which we can't include).  */
   if (buf.f_type == 0x6969) {
      /*
       * Linux is not able to detect the right nfs version form the statfs struct.
       * f_type always returns nfs, even when it's a nfs4. We are looking into
       * the /etc/mtab (or equivalent) file until we found a better solution
       * to do this.
       */
      fd = setmntent(_PATH_MOUNTED, "r");
      if (fd == NULL) {
         fprintf(stderr, "file system type could not be detected\n");
         printf("unknown fs\n");
         return 1;
      } else {
         bool found_line = false;
	 struct mntent *ent;
         sge_strip_white_space_at_eol(argv[1]);
         sge_strip_slash_at_eol(argv[1]);

         while ((ent = getmntent(fd)) != NULL) {

            /* search only in valid lines that contain NFS4 mounts */
            if (ent->mnt_dir != NULL && ent->mnt_type != NULL && strcmp(ent->mnt_type, "nfs4") == 0) {
               /* search mountpoint in given path */
               char *pos = strstr(argv[1], ent->mnt_dir);
               if (pos == argv[1]) {
                  /* we found the mountpoint at the start of the given path, this is it! */
                  found_line = true;
                  printf ("%s\n", ent->mnt_type);
                  break;
               }
            }
         }

         (void) endmntent(fd);
         if (found_line == false) { /*if type could not be detected via /etc/mtab, then we have to print out "nfs"*/
            printf("nfs\n");
         }
      }
   } else {
      switch (buf.f_type) {
      case 0x52654973:
         printf("reiserfs\n");
         break;
      case 0x517B:
         printf("smb\n");
         break;
      case 0x5346414F:
         printf("afs\n");
         break;
      case 0x73757245:
         printf("coda\n");      /* is it POSIX-compliant?  */
         break;
      default:
         printf("%lx\n", (long unsigned int)buf.f_type);
      }
   }
#elif defined(INTERIX)
   printf("%s\n", buf.f_fstypename);
#else
   printf("%s\n", buf.f_basetype);
#endif
   }
   return 0;
}
