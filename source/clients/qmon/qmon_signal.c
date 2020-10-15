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
#include <signal.h>
#include <Xm/Xm.h>
#include <Xmt/Xmt.h>
#include <Xmt/Dialogs.h>

#include "qmon_rmon.h"
#include "qmon_signal.h"
#include "qmon_globals.h"
#include "qmon_init.h"

static int qmon_shutdown = 0;
extern XtSignalId sigint_id;

void qmonSIGALRM(int dummy)
{
   static char message[BUFSIZ];
   
   sprintf(message, "SIGALRM arrived");
   XtAppAddTimeOut(AppContext, 0, qmonSignalMsg, message);
}

int do_qmon_shutdown(void) {
   return qmon_shutdown;
}

void qmonSIGPIPE(int dummy)
{

#if 0
   static char message[BUFSIZ];
   
   sprintf(message, "SIGPIPE arrived");
   XtAppAddTimeOut(AppContext, 0, qmonSignalMsg, message);
#endif   
}

/*-------------------------------------------------------------------------*/
void qmonSignalMsg(
XtPointer cld,
XtIntervalId *id 
) {
   XmtDisplayWarning(AppShell, (char*)cld, "SignalWarning");
}

void qmonSIGINT(int dummy) {
   qmon_shutdown = 1;

   if (sigint_id != 0) {
      XtNoticeSignal(sigint_id);
   }
}

/*-------------------------------------------------------------------------*/
void qmonInstSignalHandler(void)
{
   signal(SIGALRM, qmonSIGALRM );
   signal(SIGPIPE, qmonSIGPIPE );
   signal(SIGINT,  qmonSIGINT  ); 
   signal(SIGTERM,  qmonSIGINT  ); 
}


