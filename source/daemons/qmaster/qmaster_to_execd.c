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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_prog.h"

#include "sgeobj/sge_all_listsL.h"

#include "comm/commlib.h"

#include "gdi/sge_gdi2.h"

#include "sge.h"
#include "qmaster_to_execd.h"
#include "msg_qmaster.h"


static int host_notify_about_X(sge_gdi_ctx_class_t *ctx,
                               lListElem *host,
                               u_long32 x,
                               int tag,
                               int progname_id);

/****** qmaster/host/host_notify_about_X() *************************************
*  NAME
*     host_notify_about_X() -- Send X to comproc 
*
*  SYNOPSIS
*     static int host_notify_about_X(lListElem *host, 
*                                    u_long32 x, 
*                                    int tag, 
*                                    int progname_id) 
*
*  FUNCTION
*     Sends the information "x" which will be tagged with "tag" to
*     the comproc on "host" which is identified by "progname_id". 
*
*     Following combinations are meaningfull in the moment:
*
*        x              tag                  progname_id
*        -------------- -------------------- ------------
*        0 or 1         TAG_KILL_EXECD       EXECD               
*        featureset id  TAG_NEW_FEATURES     EXECD
*        *              TAG_GET_NEW_CONF     EXECD
*
*  INPUTS
*     lListElem *host  - EH_Type element 
*     u_long32 x       - data 
*     int tag          - tag for data 
*     int progname_id  - programm name id 
*
*  RESULT
*     int - error state
*         0 - no error
*        -1 - error
*        -2 - comproc with id "progname_id" not known on "host" 
*
*  NOTES
*     We send an unacknowledged request for the moment. 
*     I would have a better feelin if we make some sort of acknowledgement. 
*
*  SEE ALSO
*     qmaster/host/host_notify_about_featureset()
*******************************************************************************/
static int host_notify_about_X(sge_gdi_ctx_class_t *ctx, 
                               lListElem *host,
                               u_long32 x,
                               int tag,
                               int progname_id)
{
   const char *hostname = NULL;
   sge_pack_buffer pb;
   int ret = -1;

   DENTER(TOP_LAYER, "host_notify_about_X");

   hostname = lGetHost(host, EH_name);
   if (progname_id == EXECD) {
      unsigned long last_heard_from;
      u_short id = 1;
      const char *commproc = prognames[progname_id];
      cl_commlib_get_last_message_time(cl_com_get_handle(prognames[QMASTER], 0),
                                        (char*)hostname, (char*)commproc,id, 
                                        &last_heard_from);
      if (!last_heard_from) {
         ERROR((SGE_EVENT, MSG_NOXKNOWNONHOSTYTOSENDCONFNOTIFICATION_SS,
                commproc, hostname));
         DRETURN(-2);
      }
   }

   if (init_packbuffer(&pb, 256, 0) == PACK_SUCCESS) {
      u_long32 dummy = 0;

      packint(&pb, x);
      if (gdi2_send_message_pb(ctx, 0, prognames[progname_id], 1, hostname,
                               tag, &pb, &dummy) == CL_RETVAL_OK) {
         ret = 0;
      }
      clear_packbuffer(&pb);
   }

   DRETURN(ret);
}

/****** qmaster/host/host_notify_about_new_conf() *****************************
*  NAME
*     host_notify_about_new_conf() -- Notify execd about new config. 
*
*  SYNOPSIS
*     int host_notify_about_new_conf(lListElem *host) 
*
*  FUNCTION
*     Notify execd that there is a new configuration available.
*
*  INPUTS
*     lListElem *host - EH_Type element 
*
*  RESULT
*     int - see host_notify_about_X() 
*
*  SEE ALSO
*     qmaster/host/host_notify_about_X()
*******************************************************************************/
int host_notify_about_new_conf(sge_gdi_ctx_class_t *ctx, lListElem *host) 
{
   return host_notify_about_X(ctx, host, 0, TAG_GET_NEW_CONF, EXECD);
}

/****** qmaster/host/host_notify_about_kill() *********************************
*  NAME
*     host_notify_about_kill() -- Send kill command to execd 
*
*  SYNOPSIS
*     int host_notify_about_kill(lListElem *host, int kill_command) 
*
*  FUNCTION
*     Send the given "kill_command" to the execution "host". 
*
*  INPUTS
*     lListElem *host - EH_Type 
*     int kill_command   - command 
*
*  RESULT
*     int - see host_notify_about_X() 
*
*  SEE ALSO
*     qmaster/host/host_notify_about_X()
*******************************************************************************/
int host_notify_about_kill(sge_gdi_ctx_class_t *ctx, lListElem *host, int kill_command)
{
   return host_notify_about_X(ctx, host, kill_command, TAG_KILL_EXECD, EXECD);
}

int host_notify_about_full_load_report(sge_gdi_ctx_class_t *ctx, lListElem *host)
{
   return host_notify_about_X(ctx, host, 0, TAG_FULL_LOAD_REPORT, EXECD);
}

