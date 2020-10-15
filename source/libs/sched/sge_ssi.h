#ifndef _SGE_SSI_H_
#define _SGE_SSI_H_
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


#include "evc/sge_event_client.h"

/****** schedlib/ssi/-Simple-Scheduler-Interface-Typedefs ***************************************
*
*  NAME
*     -Simple-Scheduler-Interface-Typedefs -- typedefs for the SSI
*
*  SYNOPSIS
*     typedef struct {
*        int procs;
*        const char *host_name;
*     } task_map;
*
*  FUNCTION
*     With a task_map a jobs structure is described.
*     A job can be spawned over an arbitrary number of hosts.
*     A job has an arbitrary number of tasks per host.
*     An array of task_map is used to pass information to ssi functions.
*     It can contain any number of entries, the last entry has to contain
*     0 as procs.
*
*  SEE ALSO
*     schedlib/ssi/--Simple-Scheduler-Interface
*     schedlib/ssi/sge_ssi_job_start()
****************************************************************************
*/


typedef struct {
   int procs;
   const char *host_name;
} task_map;

/****** schedlib/ssi/--Simple-Scheduler-Interface ***************************************
*
*  NAME
*     Simple-Scheduler-Interface -- Interface for custom schedulers
*
*  FUNCTION
*     SGE provides a very simple interface to custom schedulers.
*     Such scheduler can be created using the event client or the
*     event mirror interface.
*     The interface provides functions to start a job and to
*     delete a job.
*
*     It was created to allow an easier integration of the MAUI scheduler 
*     into Grid Engine.
*
*  SEE ALSO
*     schedlib/ssi/sge_ssi_job_start()
*     schedlib/ssi/sge_ssi_job_cancel()
*
****************************************************************************
*/

bool sge_ssi_job_start(sge_evc_class_t *evc, const char *job_identifier, const char *pe, task_map tasks[]);
bool sge_ssi_job_cancel(sge_evc_class_t *evc, const char *job_identifier, bool reschedule); 

#endif /* _SGE_SSI_H_ */
