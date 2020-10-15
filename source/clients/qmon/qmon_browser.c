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
#include <X11/Xos.h>

#include <Xm/Xm.h>
#if XmVersion <= 1001
#include <Xm/MwmUtil.h>
#endif

#include <Xmt/Xmt.h>
#include <Xmt/Cli.h>
#include <Xmt/Icon.h>
#include <Xmt/Create.h>
#include <Xmt/Chooser.h>
#include <Xmt/Dialogs.h>

#include "qmon_proto.h"
#include "qmon_rmon.h"
#include "qmon_util.h"
#include "qmon_browser.h"
#include "qmon_globals.h"
#include "qmon_init.h"
#include "qmon_file.h"
#include "qmon_message.h"

#include "uti/sge_prog.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"

#include "sge.h"
#include "basis_types.h"

extern sge_gdi_ctx_class_t *ctx;

static int BrowserState = 0;
static Widget BrowserShell = 0;
static Widget browser_object = 0;

/*-------------------------------------------------------------------------*/
static Widget qmonBrowserCreateDialog(Widget parent, char *title);
static void qmonBrowserClose(Widget w, XtPointer cld, XtPointer cad);
static void qmonBrowserClear(Widget w, XtPointer cld, XtPointer cad);
static void qmonBrowserSetState(Widget w, XtPointer cld, XtPointer cad);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* PUBLIC FUNCTIONS                                                       */
/*-------------------------------------------------------------------------*/
   
/*-------------------------------------------------------------------------*/
void qmonBrowserOpen(Widget w, XtPointer cld, XtPointer cad)
{
   
   DENTER(TOP_LAYER, "qmonBrowserOpen");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   if (!BrowserShell) {
      BrowserShell = qmonBrowserCreateDialog(AppShell, "Qmon Object Browser");
      /* 
      ** set the close button callback 
      ** set the icon and iconName
      */
      XmtCreatePixmapIcon(BrowserShell, qmonGetIcon("toolbar_browser"), None);
      XtVaSetValues(BrowserShell, XtNiconName, "qmon:Browser", NULL);
      XmtAddDeleteCallback(BrowserShell, XmDO_NOTHING, qmonBrowserClose,  NULL);

   } 

   xmui_manage(BrowserShell);

   /* set default cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;

}



/*-------------------------------------------------------------------------*/
void qmonBrowserShow(
StringConst s 
) {
   Widget browser;
   
   DENTER(GUI_LAYER, "qmonBrowserShow");

   if (!BrowserShell) {
      DEXIT;
      return;
   }
      
   browser = XmtNameToWidget(BrowserShell, "*browser");   
   if (s) {
      XmtCliPuts(s, browser);
      XmtCliFlush(browser);
   }   

   /*    XBell(XtDisplay(browser), 0); */

   DEXIT;
}

/*-------------------------------------------------------------------------*/
Boolean qmonBrowserObjectEnabled(
int obj_id 
) {
   return ((BrowserState & obj_id) == obj_id);    
}

/*-------------------------------------------------------------------------*/
void qmonBrowserMessages(Widget w, XtPointer cld, XtPointer cad)
{
   char *host = (char*)cld;
   char filename[SGE_PATH_MAX];
   lList *alp = NULL;
   const char *default_cell = ctx->get_default_cell(ctx);

   DENTER(GUI_LAYER, "qmonBrowserMessages");

   /*
   ** get the path to the messages file
   */
   sprintf(filename, "%s/%s/spool/", SGE_ROOT, default_cell);

   if (!host || !strcmp(host , "global"))
      strcat(filename, "qmaster");
   else
      strcat(filename, host);
   strcat(filename, "/");
   strcat(filename, ERR_FILE);

   alp = qmonReadFile(filename);
   qmonMessageBox(w, alp, 0);
   lFreeList(&alp);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
/* PRIVATE FUNCTIONS                                                       */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
static Widget qmonBrowserCreateDialog(
Widget parent,
char *title 
) {
   
   Widget browser_shell, browser, browser_close, browser_clear,
          browser_main_link;
   
   DENTER(TOP_LAYER, "qmonBrowserCreateDialog");

   browser_shell = XmtBuildQueryToplevel(parent, "browser_shell",
                            "browser_object", &browser_object,
                            "browser", &browser,
                            "browser_close", &browser_close,
                            "browser_clear", &browser_clear,
                            "browser_main_link", &browser_main_link,
                            NULL);

   /* set iconName */
   XtVaSetValues( browser_shell, 
/*                   XmNtitle, title, */
                  XmNallowShellResize, True,
                  NULL);
   
   /* attach callbacks */
   XtAddCallback(browser_main_link, XmNactivateCallback, 
                        qmonMainControlRaise, NULL); 
   XtAddCallback(browser_object, XmtNvalueChangedCallback, 
                        qmonBrowserSetState, (XtPointer) browser); 
   XtAddCallback(browser_close, XmNactivateCallback, 
                        qmonBrowserClose, NULL); 
   XtAddCallback(browser_clear, XmNactivateCallback, 
                        qmonBrowserClear, (XtPointer) browser);
   
   DEXIT;
   return browser_shell;
}

/*-------------------------------------------------------------------------*/
static void qmonBrowserClose(Widget w, XtPointer cld, XtPointer cad)
{
   
   DENTER(TOP_LAYER, "qmonBrowserClose");

   /* disable browsing */
   XmtChooserSetState(browser_object, 0, True);

   xmui_unmanage(BrowserShell);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonBrowserClear(Widget w, XtPointer cld, XtPointer cad)
{
   Widget browser = (Widget) cld;

   DENTER(GUI_LAYER, "qmonBrowserClear");

   XmtCliClear(browser);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonBrowserSetState(Widget w, XtPointer cld, XtPointer cad)
{
   XmtChooserCallbackStruct *cbs = (XmtChooserCallbackStruct*) cad;
   Widget browser = (Widget) cld;
   Boolean out, err;
   
   DENTER(GUI_LAYER, "qmonBrowserSetState");
   
   BrowserState = cbs->state;
   
   if (BrowserState & BROWSE_STDOUT)
      out = True;
   else
      out = False;
      
   if (BrowserState & BROWSE_STDERR)
      err = True;
   else
      err = False;
      
   XtVaSetValues( browser, 
                  XmtNdisplayStdout, out,
                  XmtNdisplayStderr, err,
                  NULL);
   DEXIT;
}
