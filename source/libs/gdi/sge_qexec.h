#ifndef _SGE_QEXEC_H_
#define _SGE_QEXEC_H_
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

#ifdef  __cplusplus
extern "C" {
#endif

#include "sgeobj/sge_qexec_RT_L.h"

#include "gdi/sge_gdi_ctx.h"

typedef char *sge_tid_t;

enum {                           /* possible values for RT_state */
   RT_STATE_WAIT4ACK,            /* started - we are waiting for the exit msg
                                  * for task */
   RT_STATE_EXITED,              /* we got the exit message - keep in our list 
                                  * to prevent task ids used multiple */
   RT_STATE_WAITED               /* user reaped task by calling sge_qwaittid() */
};

#if 0
/* put these values into task environment list 'envlp' in order 
   to overwrite default behaviour */ 
#define OVERWRITE_TASK_ID_NAME   "TASK_ID"
#define OVERWRITE_PROGRAM_NAME   "PROGRAM_NAME"
#define OVERWRITE_STDOUT         "STDOUT_PATH"
#define OVERWRITE_STDERR         "STDERR_PATH"
#define OVERWRITE_MERGE          "STDOUTERR_MERGE"
#define OVERWRITE_QUEUE          "QUEUE_NAME"
#define OVERWRITE_NO_ACK         "NO_ACK"

/* meaning should be analog to macros that come with waitpid(2) */ 
#define QEXITSTATUS(status) (status)
#define QIFEXITED(status)   (0)
#define QIFSIGNALED(status) (0)
#define QTERMSIG(status)    (0)
#endif
sge_tid_t sge_qexecve(sge_gdi_ctx_class_t *context, 
                      const char *hostname, const char *queuename, const char *cwd, 
                      const lList *environment, const lList *path_aliases);

int sge_qwaittid(sge_gdi_ctx_class_t *context, sge_tid_t tid, int *status, int options);

const char *qexec_last_err(void);

#ifdef  __cplusplus
}
#endif

#endif /* _SGE_QEXEC_H_ */
