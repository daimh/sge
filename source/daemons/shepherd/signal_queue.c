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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <ctype.h>

#include "uti/sge_signal.h"

#include "signal_queue.h"
#include "err_trace.h"

#define SGE_MAXSIG 100

/* ring buffer to queue signals */
static int sig_queue[SGE_MAXSIG];
static int n_sigs = 0;
static int next_sig = 0;
static int free_sig = 0; 


#define NEXT_INDEX(i) (((i+1)>SGE_MAXSIG-1)?0:(i+1))

#ifdef DEBUG
void report_signal_queue()
{
   char str[256];
   int n, i;
 
   if (n_sigs==0) {
      shepherd_trace("no signals in queue");
      return;
   }

   i=next_sig;

   for (n=n_sigs; n; n--) {
      shepherd_trace("%d. %d", i, sig_queue[i]);
      i = NEXT_INDEX(i);
   }

   return;
}
#endif

int shepherd_sys_str2signal(char *override_signal)
{
   if (!isdigit(override_signal[0]))
      override_signal = &override_signal[3];
   return sge_sys_str2signal(override_signal);
}

/****** shepherd/signal/queue/add_signal() ************************************
*  NAME
*     add_signal() -- store signal in queue 
*
*  SYNOPSIS
*     int add_signal(int signal) 
*
*  FUNCTION
*     Store an additional signal in queue. 
*
*  INPUTS
*     int signal - sginal number 
*
*  RESULT
*     int - error state
*        0 - successfull
*       -1 - buffer is full 
*******************************************************************************/
int add_signal(int signal)
{
   int ret = -1;

   if (n_sigs != SGE_MAXSIG) {
      ret = 0;

      n_sigs++;
      sig_queue[free_sig] = signal;
      free_sig = NEXT_INDEX(free_sig);

      shepherd_trace("queued signal %s", sge_sys_sig2str(signal));
   } 
   return ret;
}  

/****** shepherd/signal_queue/get_signal() ************************************
*  NAME
*     get_signal() -- get signal from queue 
*
*  SYNOPSIS
*     int get_signal() 
*
*  FUNCTION
*     Get signal from queue. If tehre are no more signals return -1.
*
*  INPUTS
*
*  RESULT
*     int - 
*        >0 - signal number
*        -1 - There are no more signals 
*******************************************************************************/
int get_signal(void)
{
   int signal = -1;

   if (n_sigs != 0) {
      n_sigs--;
      signal = sig_queue[next_sig];
      next_sig = NEXT_INDEX(next_sig);
   }

   return signal;
}

/****** shepherd/signal_queue/pending_sig() ***********************************
*  NAME
*     pending_sig() -- look for given signal in queue 
*
*  SYNOPSIS
*     int pending_sig(int sig) 
*
*  FUNCTION
*     Try to find signal in queue.
*
*  INPUTS
*     int sig - signal number 
*
*  RESULT
*     int - result
*        0 - not found
*        1 - found 
*******************************************************************************/
int pending_sig(int sig) 
{
   int ret = 0;
   int n, i;

   for (n = n_sigs, i = next_sig; n; n--, i = NEXT_INDEX(i)) {
      if (sig_queue[i] == sig) {
         ret = 1;
         break;
      }
   }
   return ret;
}

/****** shepherd/signal_queue/get_n_sigs() ************************************
*  NAME
*     get_n_sigs() -- return number of queues signals 
*
*  SYNOPSIS
*     int get_n_sigs(void) 
*
*  FUNCTION
*     Return number of queues signals 
*
*  RESULT
*     int - number of signals
*******************************************************************************/
int get_n_sigs(void)
{
   return n_sigs; 
}

/****** shepherd/signal_queue/clear_queued_signals() **************************
*  NAME
*     clear_queued_signals() -- clear signal queue 
*
*  SYNOPSIS
*     void clear_queued_signals(void) 
*
*  FUNCTION
*     Clear signal queue 
*******************************************************************************/
void clear_queued_signals(void)
{
   n_sigs = next_sig = free_sig = 0; 
}
