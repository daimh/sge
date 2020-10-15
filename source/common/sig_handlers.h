#ifndef __SIG_HANDLERS_H
#define __SIG_HANDLERS_H
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

#include <signal.h>

#ifdef  __cplusplus
extern "C" {
#endif

void sge_setup_sig_handlers(int me_who);
void sge_reap(int dummy);

extern sigset_t default_mask;
extern sigset_t omask;
extern sigset_t io_mask;

extern struct sigaction sigterm_vec, sigterm_ovec;
extern struct sigaction sigalrm_vec, sigalrm_ovec;
extern struct sigaction sigcld_vec, sigcld_ovec;

extern volatile int shut_me_down;
extern volatile int sge_sig_handler_dead_children;
extern volatile int sge_sig_handler_in_main_loop;
extern volatile int sge_sig_handler_sigpipe_received;

#ifdef  __cplusplus
}
#endif

#endif /* __SIG_HANDLERS_H */
