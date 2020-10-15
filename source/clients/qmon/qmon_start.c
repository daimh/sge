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
#include <unistd.h>


#include <Xm/Xm.h>
#include <Xm/MenuShell.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>

#include <Xmt/Xmt.h>
#include <Xmt/WorkingBox.h>

#include "qmon_rmon.h"
#include "qmon_start.h"
#include "qmon_browser.h"


#if 0

/*-------------------------------------------------------------------------*/
static void get_pipe_input(XtPointer cld, int *fid, XtInputId *id);
static pid_t qmonForkProcess(XtAppContext app, Widget parent);

/*-------------------------------------------------------------------------*/
static void get_pipe_input(
XtPointer cld,
int *fid,
XtInputId *id 
) {
   char buf[BUFSIZ];
   int  nbytes;

   if ((nbytes=read(*fid,buf,BUFSIZ)) == -1)
      perror("get_pipe_input");

   fprintf(stderr, "Parent: %s", buf);
}

static pid_t qmonForkProcess(
XtAppContext app,
Widget parent 
) {
   int pfd1[2];
   pid_t pid;
   static int count = 0;
   
   DENTER(GUI_LAYER, "qmonForkProcess");

   if ( pipe(pfd1) < 0 ) {
      fprintf(stderr, "pipe() failed\n");
      exit(1);
   }
   
   if ((pid = fork()) == -1) {
      fprintf(stderr, "fork() failed\n");
   }

   if (pid == 0) { /* child */
      close(pfd1[0]);
      close(1);
      dup(pfd1[1]);
      while(1) {
         if (! (count++ % 1000000) )
            write(1, "Hello, I'm the child\n", 
                     strlen("Hello, I'm the child\n")+1);
      }
   }
   else {
      close(pfd1[1]);
      XtAppAddInput(app,pfd1[0],(XtPointer)XtInputReadMask,
                          get_pipe_input, NULL);
   }

   DEXIT;
   return getpid();
}

#endif


/***************************************************************************/
Widget qmonStartupWindow(
Widget parent 
) {
   Widget StartupShell, StartupRC, StartupLabel;
   Position x, y;

   DENTER(TOP_LAYER, "qmonStartupWindow");

#if 0
/*
   fprintf(stderr, "Parent pid = %d\n", 
            qmonForkProcess(XtWidgetToApplicationContext(parent), parent) );
*/
/*
   StartupShell = XtVaCreateWidget("StartupShell",
                                    topLevelShellWidgetClass,
                                    parent,
                                    XtNtransient,True,
                                    XtNsaveUnder, True,
                                    NULL);
   StartupDialog = XtVaCreateManagedWidget("StartupDialog",
                                    xmtWorkingBoxWidgetClass,
                                    StartupShell,
                                    XmtNmessage, "WELCOME TO SGE",
                                    XmtNshowScale, False,
                                    XmtNshowButton, False,
                                    NULL);
*/
#endif

   x = (DisplayWidth(XtDisplay(parent), DefaultScreen(XtDisplay(parent))) 
         - 719)/2;
   y = (DisplayHeight(XtDisplay(parent), DefaultScreen(XtDisplay(parent)))
         - 367)/2;
   StartupShell = XtVaCreateWidget( "StartupShell",
                                    xmMenuShellWidgetClass,
                                    parent,
                                    XmNwidth, 465,
                                    XmNheight, 730,
                                    XmNx, x,
                                    XmNy, y,
                                    NULL);
   StartupRC = XtVaCreateManagedWidget( "StartupRC",
                                        xmRowColumnWidgetClass,
                                        StartupShell,
                                        XmNx, x,
                                        XmNy, y,
                                        XmNwidth, 465,
                                        XmNheight, 700,
                                        NULL);
   StartupLabel = XtVaCreateManagedWidget( "StartupLabel",
                                        xmLabelWidgetClass,
                                        StartupRC,
                                        XmNalignment, XmALIGNMENT_CENTER,
                                        XmNmarginWidth, 0,
                                        XmNmarginHeight, 0,
                                        XmNshadowThickness, 1,
                                        NULL); 
   XtRealizeWidget(StartupShell);
   XtPopup(StartupShell, XtGrabNone);
   XmtWaitUntilMapped(StartupShell);

   DEXIT;
   return StartupShell;
}
