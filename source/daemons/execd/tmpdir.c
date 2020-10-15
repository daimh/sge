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

#include <errno.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_unistd.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_string.h"

#include "sgeobj/sge_qinstance.h"

#include "tmpdir.h"
#include "msg_common.h"
#include "msg_execd.h"

/*******************************************************/
char *sge_make_tmpdir(lListElem *qep, u_long32 jobid, u_long32 jataskid, uid_t uid, gid_t gid, char *tmpdir, size_t ltmpdir)
{
   const char *t;

   DENTER(TOP_LAYER, "sge_make_tmpdir");

   t = lGetString(qep, QU_tmpdir);
   if (t == NULL) {
      DRETURN(NULL);
   }

   /* Note could have multiple instantiations of same job, */
   /* on same machine, under same queue */
   snprintf(tmpdir, ltmpdir, "%s/"sge_u32"."sge_u32".%s", t, jobid,
            jataskid, lGetString(qep, QU_qname));

   DPRINTF(("making TMPDIR=%s\n", tmpdir));

   sge_switch2start_user();
   sge_mkdir(tmpdir, 0755, false, false);

   /*
    * chown is considered to be a security flaw, as an attacker might move the 
    * directory between the mkdir and chown.
    * This is both nearly impossible here and would have no effect.
    * Make flawfinder ignore it
    */
   /* Flawfinder: ignore */
   if (chown(tmpdir, uid, gid) != 0) {
      dstring ds = DSTRING_INIT;
      ERROR((SGE_EVENT, MSG_FILE_NOCHOWN_SS, tmpdir, sge_strerror(errno, &ds)));
      sge_dstring_free(&ds);
      unlink(tmpdir);
      DRETURN(NULL);
   }

   sge_switch2admin_user();

   DRETURN(tmpdir);
}

/************************************************************************/
int sge_remove_tmpdir(const char *dir, const char *job_owner, u_long32 jobid, u_long32 jataskid, const char *queue_name)
{
   stringT tmpstr;
   char err_str_buffer[1024];
   dstring err_str;

   DENTER(TOP_LAYER, "sge_remove_tmpdir");

   sge_dstring_init(&err_str, err_str_buffer, sizeof(err_str_buffer));

   if (!dir) {
      DRETURN(0);
   }

   snprintf(tmpstr, sizeof(tmpstr), "%s/"sge_u32"."sge_u32".%s", dir, jobid,
            jataskid, queue_name);
   DPRINTF(("recursively unlinking \"%s\"\n", tmpstr));
   sge_switch2start_user();
   if (sge_rmdir(tmpstr, &err_str)) {
      ERROR((SGE_EVENT, MSG_FILE_RECURSIVERMDIR_SS, 
             tmpstr, err_str_buffer));
      sge_switch2admin_user();
      DRETURN(-1);
   }
   sge_switch2admin_user();

   DRETURN(0);
}

char *sge_get_tmpdir(lListElem *qep, u_long32 jobid, u_long32 jataskid, char *tmpdir, size_t ltmpdir)
{
   const char *t;

   DENTER(TOP_LAYER, "sge_get_tmpdir");

   if (!(t=lGetString(qep, QU_tmpdir))) {
      DEXIT;
      return NULL;
   }

   snprintf(tmpdir, ltmpdir, "%s/"sge_u32"."sge_u32".%s", t, jobid,
            jataskid, lGetString(qep, QU_qname));

   DPRINTF(("TMPDIR=%s\n", tmpdir));

   DEXIT;
   return tmpdir;
}
