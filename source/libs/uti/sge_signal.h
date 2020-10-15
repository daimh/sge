#ifndef __SGE_SIGNAL_H
#define __SGE_SIGNAL_H
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

#ifndef __BASIS_TYPES_H
#   include "basis_types.h"
#endif

#include <signal.h>

#define SGE_SIGHUP                         901
#define SGE_SIGINT                         902
#define SGE_SIGQUIT                        903
#define SGE_SIGILL                         904
#define SGE_SIGTRAP                        905
#define SGE_SIGABRT                        906
#define SGE_SIGIOT                         907
#define SGE_SIGEMT                         908
#define SGE_SIGFPE                         909
#define SGE_SIGKILL                        910
#define SGE_SIGBUS                         911
#define SGE_SIGSEGV                        912
#define SGE_SIGPIPE                        914
#define SGE_SIGALRM                        915
#define SGE_SIGTERM                        916
#define SGE_SIGURG                         917
#define SGE_SIGSTOP                        918
#define SGE_SIGTSTP                        919
#define SGE_SIGCONT                        920
#define SGE_SIGCHLD                        921
#define SGE_SIGTTIN                        922
#define SGE_SIGTTOU                        923
#define SGE_SIGIO                          924
#define SGE_SIGXCPU                        925
#define SGE_SIGXFSZ                        926
#define SGE_SIGVTALRM                      927
#define SGE_SIGPROF                        928
#define SGE_SIGWINCH                       929
#define SGE_SIGUSR1                        931
#define SGE_SIGUSR2                        932
#define SGE_MIGRATE			   933

/* Not all systems have all signals. Fill this in if not known. */
#define SIGUNKNOWN                         0

typedef struct sig_mapT {
   u_long32 sge_sig;
   int sig;
   char *signame;
} sig_mapT;

int sge_unmap_signal(u_long32 sge_sig);

u_long32 sge_map_signal(int sys_sig);

u_long32 sge_str2signal(const char *str);

const char *sge_sig2str(u_long32 sge_sig);

const char *sge_sys_sig2str(u_long32 sig);

u_long32 sge_sys_str2signal(const char *str);

typedef void (*err_func_t)(char *s);
 
void sge_set_def_sig_mask(sigset_t*, err_func_t);
void sge_unblock_all_signals(void);

int sge_thread_block_all_signals(sigset_t *oldsigmask);

#endif /* __SGE_SIGNAL_H */

