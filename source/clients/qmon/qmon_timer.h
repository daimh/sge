#ifndef QMON_TIMER_H
#define QMON_TIMER_H
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

#include <Xm/Xm.h>
#include "qmon_proto.h"
#include "qmon_cull.h"

typedef void (*tUpdateProc)(void);
typedef struct _tUpdateRec {
   long id;
   tUpdateProc proc;
   struct _tUpdateRec *next;
} tUpdateRec;

typedef struct _tQmonPoll {
   long type;
   long timercount;
   long fetch_frequency;
   long fetch;
   tUpdateRec *update_proc_list;
} tQmonPoll;

typedef struct _tTimer {
      XtAppContext timerapp;
      unsigned long timeout;
      XtTimerCallbackProc timerproc;
      XtIntervalId timerid;
      XtPointer timerdata;
} tTimer;



void qmonStartPolling(XtAppContext app);
void qmonStopPolling(void);
void qmonStartTimer(long timertype);
void qmonStopTimer(long timertype);
void qmonListTimerProc(XtPointer cld, XtIntervalId *id);
int  qmonTimerAddUpdateProc(long type, String name, tUpdateProc proc);
void qmonTimerRmUpdateProc(long type, String name);

void qmonTimerCheckInteractive(Widget w, XtPointer cld, XtPointer cad); 

#endif /* QMON_TIMER_H */

