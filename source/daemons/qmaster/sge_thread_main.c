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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <signal.h>
#include <fcntl.h>

#include "uti/sge_rmon.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/setup_path.h"
#include "uti/sge_bootstrap.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_conf.h"

#include "gdi/sge_security.h"

#include "comm/cl_commlib.h"

#include "basis_types.h"
#include "sge_event_master.h"
#include "sge_reporting_qmaster.h"
#include "sge_qmaster_timed_event.h"
#include "sge_host_qmaster.h"
#include "sge_qmaster_threads.h"
#include "sge_thread_main.h"
#include "sge_thread_signaler.h"

main_control_t Main_Control = {
   0,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL
};

int 
sge_qmaster_shutdown_via_signal_thread(int i)
{
   int ret = 0;

   DENTER(TOP_LAYER, "sge_shutdown_qmaster_via_signal_thread");

   sge_signaler_initiate_termination();
   sge_qmaster_set_exit_state(i);

   DRETURN(ret);
}

int
sge_qmaster_get_exit_state(void) 
{
   return Main_Control.exit_state;
}

void
sge_qmaster_set_exit_state(int new_state) 
{
   Main_Control.exit_state = new_state;
}

bool
sge_qmaster_do_final_spooling(void) 
{
   /*
    * If the exit_state is 100 than another qmaster has unexpectedly taken
    * over! Final spooling should not be done.
    */
   return (Main_Control.exit_state == 100) ? false : true;
}

