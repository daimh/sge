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
/*
** Test prog to check if Motif under Solaris passes the
** Multibyte English test with its default FontList and String Converter
** To test, follow these steps:
** - install Multibyte English locale support (mbe.27.tar.Z)
** - set the environment variables according to the delivered documentation
**   (logout, relogin by switching to en_FW.MBE as language in CDE dtlogin)
** - copy the file qmon/Xmlocale to $HOME
** - compile xmlocale: aimk -nocore xmlocale
** - test it: SOLARIS64/xmlocale  (Label is visible in red)
**   remove ~/Xmlocale and test again, this should fail (Label should be black)
*/

#include <stdio.h>
#include <stdlib.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>

String fallbacks[] = { "*fontList: 9x14=tag", NULL};

void exitCB(Widget w, XtPointer cld, XtPointer cad)
{
   printf("Goodbye\n");
   exit(0);
}

int main(int argc, char **argv)
{
   Widget toplevel, rc, pb, label;
   XtAppContext app;
   XmString xstr;

   XtSetLanguageProc(NULL, NULL, NULL);

   toplevel = XtVaAppInitialize(&app, "Xmlocale", NULL, 0, 
                                &argc, argv, NULL, NULL);
   
   rc = XtVaCreateWidget("RowCol", xmRowColumnWidgetClass,
                         toplevel, NULL);
   label = XtVaCreateManagedWidget("Label", xmLabelWidgetClass, 
                                rc, NULL);
   xstr = XmStringCreateLocalized("ÑsTTÉírÉèoÉÖeÉît");
   pb = XtVaCreateManagedWidget("PB", xmPushButtonWidgetClass, 
                                rc, XmNlabelString, xstr, NULL);
   XmStringFree(xstr);
   XtAddCallback(pb, XmNactivateCallback, exitCB, NULL);
   XtManageChild(rc);
   XtRealizeWidget(toplevel);
   XtAppMainLoop(app);
   return 0;
}




