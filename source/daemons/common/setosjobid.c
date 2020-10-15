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
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

/* for service provider info (SPI) entries and projects */
#if defined(IRIX)
#   include <sys/extacct.h>
#   include <proj.h>
#endif

#include "uti/config_file.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_stdio.h"

#include "basis_types.h"
#include "err_trace.h"
#include "setosjobid.h"

void setosjobid(pid_t sid _UNUSED, gid_t *add_grp_id_ptr, struct passwd *pw _UNUSED)
{
   FILE *fp=NULL;

   shepherd_trace("setosjobid: uid = "gid_t_fmt", euid = "uid_t_fmt, getuid(), geteuid());

#  if defined(SOLARIS) || defined(ALPHA) || defined(LINUX) || defined(FREEBSD) || defined(DARWIN)
      /* Read SgeId from config-File and create Addgrpid-File */
      {  
         char *cp;
         if ((cp = search_conf_val("add_grp_id")))
            *add_grp_id_ptr = atol(cp);
         else
            *add_grp_id_ptr = 0;
      }
      if ((fp = fopen("addgrpid", "w")) == NULL) {
         shepherd_error(1, "can't open \"addgrpid\" file");   
      }
      fprintf(fp, gid_t_fmt"\n", *add_grp_id_ptr);
      FCLOSE(fp);   
# elif defined(HP1164) || defined(AIX)
    {
      if ((fp = fopen("addgrpid", "w")) == NULL) {
         shepherd_error(1, "can't open \"addgrpid\" file");
      }
      fprintf(fp, pid_t_fmt"\n", getpgrp());
      FCLOSE(fp);
    }
#  else
   {
      char osjobid[100];
      if ((fp = fopen("osjobid", "w")) == NULL) {
         shepherd_error(1, "can't open \"osjobid\" file");
      }

      if(sge_switch2start_user() == 0) {
#     if defined(IRIX)
      {
         /* The following block contains the operations necessary for
          * IRIX6.2 (and later) to set array session handles (ASHs) and
          * service provider info (SPI) records
          */
         struct acct_spi spi;
         int ret;
         char *cp;

         shepherd_trace("in irix code");
         /* get _local_ array session id */
         if ((ret=newarraysess())) {
            shepherd_error(1, "error: can't create ASH; errno=%d", ret);
         }

         /* retrieve array session id we just assigned to the process and
          * write it to the os-jobid file
          */
         sprintf(osjobid, "%lld", getash());
         shepherd_trace(osjobid); 
         /* set service provider information (spi) record */
         strncpy(spi.spi_company, "SGE", 8);
         strncpy(spi.spi_initiator, get_conf_val("spi_initiator"), 8);
         strncpy(spi.spi_origin, get_conf_val("queue"),16);
         strcpy(spi.spi_spi, "Job ");
         strncat(spi.spi_spi, get_conf_val("job_id"),11);
         if ((ret=setspinfo(&spi))) {
            shepherd_error(1, "error: can't set SPI; errno=%d", ret);
         }
         
         if ((cp = search_conf_val("acct_project"))) {
            prid_t proj; 
            if (strcasecmp(cp, "none") && ((proj = projid(cp)) >= 0)) {
               shepherd_trace("setting project \"%s\" to id %lld", cp, proj);
               if (setprid(proj) == -1)
                  shepherd_trace("failed setting project id");
            }
            else {   
               shepherd_trace("can't get id for project \"%s\"", cp);
            }
         } else {
            shepherd_trace("can't get configuration entry for projects");
         }
      }
#     else
         /* write a default os-jobid to file */
         snprintf(osjobid, sizeof(osjobid), pid_t_fmt, sid);
#     endif
         sge_switch2admin_user();
      } 
      else /* not running as super user --> we want a default os-jobid */
         sprintf(osjobid, "0");
      
      if(fprintf(fp, "%s\n", osjobid) < 0)
         shepherd_trace("error writing osjobid file");
         
      FCLOSE(fp); /* Close os-jobid file */   
   }
#  endif
   return;
FCLOSE_ERROR:
   shepherd_error(1, "can't close file"); 
}
