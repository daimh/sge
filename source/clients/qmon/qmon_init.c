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
#include <locale.h>

#include <Xm/Xm.h>

#include <Xmt/Xmt.h>
#include <Xmt/AppRes.h>
#include <Xmt/Pixmap.h>
#include <Xmt/Xpm.h>
#include <Xmt/Hash.h>
#include <Xmt/AppRes.h>
#include <Xmt/Converters.h>
#include <Xmt/Layout.h>
#include <Xmt/Procedures.h>
#include <Xmt/WidgetType.h>
#include <Xmt/Color.h>

#include "qmon_rmon.h"
#include "qmon_init.h"
#include "qmon_queue.h"
#include "qmon_submit.h"
#include "qmon_request.h"
#include "qmon_job.h"
#include "qmon_comm.h"
#include "qmon_timer.h"
#include "qmon_widgets.h"
#include "qmon_matrix.h"
#include "qmon_appres.h"
#include "qmon_globals.h"
#include "qmon_util.h"
#include "qmon_signal.h"

#include "uti/sge_prog.h"
#include "uti/sge_hostname.h"
#include "uti/sge_log.h"
#include "uti/sge_unistd.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_all_listsL.h" 

#include "gdi/qm_name.h"
#include "gdi/sge_gdi.h" 
#include "gdi/sge_gdiP.h"
#include "gdi/sge_gdi_ctx.h"
#include "gdi/msg_gdilib.h"
#include "gdi/sge_gdi2.h"

#include "comm/commlib.h" 

#include "msg_clients_common.h"
#include "msg_common.h"

extern sge_gdi_ctx_class_t *ctx;


static String icon_names[] = {
   "21cal",
   "21cal_ins",
   "shell",
   "shell_ins",
   "mailbox",
   "mailbox_ins",
   "env",
   "penv",
   "ar",
   "stderror",
   "stderror_ins",
   "stdoutput_ins",
   "script_file",
   "script_file_ins",
   "qsh_on",
   "qsh_on_ins",
   "qsh_off",
   "qsh_off_ins",
   "mcicon",
   "aclask",
   "dynamite",
   "days",
   "hours",
   "minutes",
   "seconds",
   "infinity",
   "memorybig",
   "int",
   "bool",
   "mem",
   "time",
   "str",
   "cstr",
   "host",
   "unknown",
   "resources",
   "resources_enabled",
   "logo",
   "toolbar_cluster",
   "toolbar_queue",
   "toolbar_pe",
   "toolbar_cplx",
   "toolbar_submit",
   "toolbar_user",
   "toolbar_job",
   "toolbar_host",
   "toolbar_browser",
   "toolbar_exit",
   "xterm",
   "xterm-axp",
   "xterm-sun",
   "xterm-hp",
   "xterm-dec",
   "xterm-sol",
   "xterm-linux",
   "xterm-sgi",
   "xterm-ibm",
   "leaf",
   "leaf_open",
   "leaf_plus",
   "leaf_minus"
};

/*-------------------------------------------------------------------------*/

/* 
 *  some nice things
 *  1. Load and process all images
 *  2. Check User Permissions and set sensitivity for dialog parts
 *  3. Add Functionality to change User (qmon_su) -> change sensitivity 
 *  4. Initialize What's Where's , default values for dialogs
 *  
 */

/*-------------------------------------------------------------------------*/
void qmonLoadIcons(void)
{
   int i;
   XmtImage *image;
   String pixmap_file;
   Arg args[10];
   Cardinal ac;
   String pixmapFilePath;
   
   DENTER(GUI_LAYER, "qmonLoadIcons");

   ac = 0;
   XtSetArg(args[ac], XmtNpixmapFilePath, &pixmapFilePath); ac++;
   XmtGetApplicationValues(AppShell, args, ac);

   for (i=0; i<XtNumber(icon_names); i++) {
      pixmap_file = XmtFindFile( AppShell, 
                                 "pixmaps", 
                                 icon_names[i], 
                                 ".xpm",
                                 NULL,
                                 pixmapFilePath,
                                 XmtSearchPathOnly );
      if (!pixmap_file) {
         fprintf(stderr, "Can't load icon %s. Pixmaps should reside in $SGE_ROOT/qmon/PIXMAPS.\n",
               icon_names[i]);
         DEXIT;
         qmonExitFunc(1);
      }
      DPRINTF(("%s\n", pixmap_file));
      image = XmtParseXpmFile(pixmap_file);
      if (!image) {
         fprintf(stderr, "Loading icon '%s' failed\n", pixmap_file);
         DEXIT;
         qmonExitFunc(1);
      }
      XtFree((char*) pixmap_file);
      XmtRegisterImage(icon_names[i], image);
      
      /*
      ** exits if an icon can't be loaded
      */
      qmonGetIcon(icon_names[i]);
   }
      
   DEXIT;
}
   

/*-------------------------------------------------------------------------*/
Pixmap qmonGetIcon(
String name 
) {
   Pixmap pix = None;
   
   DENTER(GUI_LAYER, "qmonGetIcon");

   pix = XmtGetPixmap(AppShell, NULL, name);

   if (pix == None) {
      /* pix = defaultpix; */
      fprintf(stderr, "Couldn't load pixmap '%s'\n", name);
      fprintf(stderr, "There are not enough colors. Try qmon -cmap\n");
      DEXIT;
      qmonExitFunc(1);
   }

   DEXIT;
   return pix;
}



/*-------------------------------------------------------------------------*/
void qmonInitSge(sge_gdi_ctx_class_t **ctx_ref, char *progname, int usage) 
{
   int error = 0;
   int endless_loop = 0;
   lList *alp = NULL;
   char* env_var = NULL;
   const char *mastername = NULL;
   u_long32 sge_qmaster_port = 0;
   sge_gdi_ctx_class_t *ctx = NULL;

   DENTER(GUI_LAYER, "qmonInitSge");
   
   strcpy(SGE_EVENT,"");

   env_var = getenv("SGE_QMON_TEST_QMASTER_ISALIVE");
   if (env_var != NULL) {
      endless_loop = atoi(env_var);
   }
   log_state_set_log_gui(1);
   error = sge_gdi2_setup(ctx_ref, QMON, MAIN_THREAD, &alp);

   if (*ctx_ref != NULL) {
      ctx = *ctx_ref;
      mastername = ctx->get_sge_root(ctx);
      sge_qmaster_port = ctx->get_sge_qmaster_port(ctx);
   }   
   if (error != AE_OK) {

      answer_list_output(&alp);
      
      if ( mastername != NULL) {
         error=ctx->is_alive(ctx);
         /* For the default case, just print a simple message */
         if (error == CL_RETVAL_CONNECT_ERROR ||
             error == CL_RETVAL_CONNECTION_NOT_FOUND) {
            SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_UNABLE_TO_CONNECT_SUS,
                                   prognames[QMASTER], 
                                   sge_qmaster_port, 
                                   mastername));
         }
         /* For unusual errors, give more detail */
         else {
            SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_CANT_SEND_MSG_TO_PORT_ON_HOST_SUSS,
                                   prognames[QMASTER], 
                                   sge_qmaster_port, 
                                   mastername, 
                                   cl_get_error_text(error)));
         }
         fprintf(stderr, "%s\n", SGE_EVENT);
         if (error != CL_RETVAL_OK) {
            /*
            ** re-read act_qmaster file
            */
            ctx->get_master(ctx, true);
         }
      }
      if (endless_loop == 0) {
         qmonExitFunc(1);
      }
   }

   while (endless_loop > 0) {
      static int nr_of_errors = 0;
      error=ctx->is_alive(ctx);
      printf("checking isalive qmaster (errors=%d, frequency=%d) ...\n", nr_of_errors, endless_loop);
      if (do_qmon_shutdown()) {
         qmonExitFunc(0);
      }
      sleep(endless_loop);
      if (error != CL_RETVAL_OK) {
         /*
         ** re-read act_qmaster file
         */
         ctx->get_master(ctx, true);
      }
   }
   log_state_set_log_gui(0);

   DEXIT;
}

/*-------------------------------------------------------------------------
 Function installed to be called just before exit() is called.
 clean up
 -------------------------------------------------------------------------*/
void qmonExitFunc(
int i 
) {
   DENTER(GUI_LAYER, "qmonExitFunc");
   cl_com_cleanup_commlib();
   DCLOSE;
   exit(i);
}

/*-------------------------------------------------------------------------*/
void qmonExitCB(
Widget w,
XtPointer cld,
XtPointer cad 
) {
   DENTER(GUI_LAYER, "qmonExitCB");

   sge_gdi2_shutdown((void**)&ctx);

   DCLOSE;
   exit(0);

   DEXIT;
}

#if 1
/*-------------------------------------------------------------------------
    The following is the Intrinsics default language procedure,
    adapted to use only LC_MESSAGES instead of LC_ALL
-------------------------------------------------------------------------*/
static String myXtDefaultLanguageProc(
Display *dpy,     /* unused */
String xnl,
XtPointer closure  /* unused */
) {
   char *languste = NULL;

   if (getenv("XMTDEBUGFINDFILE")) {
      printf("xnl = '%s'\n", xnl);
   }

   if (!strcasecmp(xnl, "relabel"))
      xnl = "C";

   if (! setlocale(LC_ALL, xnl))
      XtWarning("locale not supported by C library, locale unchanged");

   if (! XSupportsLocale()) {
      XtWarning("locale not supported by Xlib, locale set to C");
      setlocale(LC_ALL, "C");
   }
   
   setlocale(LC_NUMERIC, "C");
   
   if (! XSetLocaleModifiers(""))
      XtWarning("X locale modifiers not supported, using default");

   languste = setlocale(LC_MESSAGES, NULL);

   if (getenv("XMTDEBUGFINDFILE")) {
      printf("languste: '%s'\n", languste);
   }   
   
   return languste; /* re-query in case overwritten */
}
#endif 

/*----------------------------------------------------------------------------*/
Widget XmtInitialize(
XtAppContext *app,
String app_class,
XrmOptionDescList options,
Cardinal num_options,
int *argc_in_out,
String *argv_in_out,
String *fallbacks,
ArgList args,
Cardinal num_args 
) {
   Widget top;
   Display *dpy;
   Colormap colormap;
   int own_colormap = 0;
   int i;
   ArgList new_args = NULL;

   DENTER(GUI_LAYER, "XmtInitialize");

   XtSetLanguageProc(NULL, myXtDefaultLanguageProc, NULL);

   for (i=0; i<*argc_in_out; i++) {
      if (!strcmp(argv_in_out[i], "-cmap")) {
         own_colormap = 1;
         DPRINTF(("-cmap set\n"));
      }
      if (!strcmp(argv_in_out[i], "-nologo")) {
         nologo = 1;
         DPRINTF(("-nologo set\n"));
      }
      if (!strcmp(argv_in_out[i], "-qmon_debug")) {
         qmon_debug = 1;
         DPRINTF(("-qmon_debug set\n"));
      }
      if (!strcmp(argv_in_out[i], "-help")) {
         DPRINTF(("-help set\n"));
         helpset = 1;
      }
   }
   
   if (own_colormap) {
      XmtPatchVisualInheritance();

      /* first four initialization steps */
      XtToolkitInitialize();
      *app = XtCreateApplicationContext();
      XtAppSetFallbackResources(*app, fallbacks);
      dpy = XtOpenDisplay(*app, NULL, NULL, app_class, NULL, 0, 
                           argc_in_out, argv_in_out);
      if (dpy == NULL) 
         XtError("cannot open display");
   
      /* get a new colormap */
      colormap = XCopyColormapAndFree(dpy, 
                     DefaultColormap(dpy, DefaultScreen(dpy)));
      
      if (colormap) {
         new_args = (ArgList) XtMalloc(sizeof(Arg) *(num_args+1));
         for(i=0; i<num_args; i++) {
            new_args[i].name = args[i].name;
            new_args[i].value = args[i].value;
         }
         XtSetArg(new_args[num_args], XmNcolormap, colormap); num_args++;
      }
      else 
         XtError("cannot create colormap");
      top = XtAppCreateShell(NULL, app_class, applicationShellWidgetClass, dpy,
                        new_args, num_args);
   } 
   else {
DTRACE;
      top = XtAppInitialize(  app, 
                           app_class, 
                           options, num_options,
                           argc_in_out, argv_in_out,
                           fallbacks,
                           args, num_args);
DTRACE;
      new_args = args;
   }
   /*
    * Parse Xmt specific command line
    */
   XmtParseCommandLine(top, argc_in_out, argv_in_out);
   
   /*
    * get application resources 
    */
   qmonGetApplicationResources(top, new_args, num_args);


   /*
   ** set the multiclick time
   **
   */
   XtSetMultiClickTime(XtDisplay(top), MULTI_CLICK_TIME);

#ifdef LINUX                    /* why? */
   XtSetLanguageProc(*app, NULL, NULL); 
#endif   

   /*
    * check if qmon_version is set correctly in the app-defaults file
    */
   if (QMON_VERSION != 5300) {
      fprintf(stderr, "Wrong Version of Application Defaults file\n");
      DEXIT;
      qmonExitFunc(1);
   } 

   /* 
    * Register Xmt Pixmap and Bitmap converters 
    * (specify pixmaps or bitmaps in resource file)
    */
   XmtRegisterPixelConverter();    /* this routine is buggy */
   XmtRegisterBitmapConverter();
   XmtRegisterBitmaskConverter();
   XmtRegisterPixmapConverter();
   XmtRegisterColorTableConverter();
   XmtRegisterWidgetConverter();
   XmtRegisterCallbackConverter();
   XmtRegisterXmStringConverter();
   XmtRegisterXmFontListConverter();
   XmtRegisterMenuItemsConverter();
   XmtRegisterCallbackConverter();

   XmtRegisterMotifWidgets();
   XmtRegisterXmtWidgets();

   XmtRegisterXtProcedures();
   XmtRegisterXmtProcedures();


   XmtRegisterLayoutParser();
   XmtRegisterLayoutCreateMethod();

#if 1
   /*
   ** register xmt improved icons
   */
   XmtRegisterImprovedIcons(top, NULL);
#endif

   /*
    * Register adapted Qmon Widgets 
    */
   QmonRegisterWidgets();
   QmonRegisterMatrixWidgets();
   
   /*
   ** register callbacks 
   */
   XmtVaRegisterCallbackProcedures(
         "DeleteItems", DeleteItems, XtRWidget,
         NULL);

   /*
    * Register the shell and look up the app-resources 
    */
   XmtInitializeApplicationShell(top, new_args, num_args);

   DEXIT;
   return top;

}
   
/*-------------------------------------------------------------------------*/
void qmonCreateGC(
Widget top 
) {
   XGCValues values;
   unsigned long valuemask;
   Pixel fg, bg, qfg, qbg, q_running_color,
         q_suspend_color, q_alarm_color, q_error_color, q_disable_color,
         q_caldisable_color, q_calsuspend_color;
   Display *dpy = XtDisplay(top);
   Window root = RootWindow(dpy, DefaultScreen(dpy));
   Font default_font;

   DENTER(GUI_LAYER, "qmonCreateGC");
   
   XmtAllocWidgetColor(top, "$fg_gc_foreground", &fg);
   XmtAllocWidgetColor(top, "$fg_gc_background", &bg);
   XmtAllocWidgetColor(top, "$qb_gc_foreground", &qfg);
   XmtAllocWidgetColor(top, "$qb_gc_background", &qbg);
   XmtAllocWidgetColor(top, "$q_running_color", &q_running_color);
   XmtAllocWidgetColor(top, "$q_suspend_color", &q_suspend_color);
   XmtAllocWidgetColor(top, "$q_alarm_color", &q_alarm_color);
   XmtAllocWidgetColor(top, "$q_error_color", &q_error_color);
   XmtAllocWidgetColor(top, "$q_disable_color", &q_disable_color);
   XmtAllocWidgetColor(top, "$q_caldisable_color", &q_caldisable_color);
   XmtAllocWidgetColor(top, "$q_calsuspend_color", &q_calsuspend_color);
   
   default_font = XLoadFont(dpy,"-*-courier-bold-o-*--*-140-*");
                  
   values.foreground = bg;
   values.background = fg;
   values.font = default_font;
   valuemask = GCForeground | GCBackground | GCFont;
   bg_gc = XCreateGC( dpy, root, valuemask, &values);
                        
   values.foreground = fg;
   values.background = bg;
   values.font = default_font;
   valuemask = GCForeground | GCBackground | GCFont;
   fg_gc = XCreateGC( dpy, root, valuemask, &values);

   values.foreground = qfg;
   values.background = qbg;
   values.font = default_font;
   valuemask = GCForeground | GCBackground | GCFont;
   qb_gc = XCreateGC( dpy, root, valuemask, &values);

   values.foreground = q_running_color;
   values.background = q_running_color;
   values.font = default_font;
   valuemask = GCForeground | GCBackground | GCFont;
   running_gc = XCreateGC( dpy, root, valuemask, &values);

   values.foreground = q_alarm_color;
   values.background = q_alarm_color;
   values.font = default_font;
   valuemask = GCForeground | GCBackground | GCFont;
   alarm_gc = XCreateGC( dpy, root, valuemask, &values);
   
   values.foreground = q_error_color;
   values.background = q_error_color;
   values.font = default_font;
   valuemask = GCForeground | GCBackground | GCFont;
   error_gc = XCreateGC( dpy, root, valuemask, &values);
   
   values.foreground = q_suspend_color;
   values.background = q_suspend_color;
   values.font = default_font;
   valuemask = GCForeground | GCBackground | GCFont;
   suspend_gc = XCreateGC( dpy, root, valuemask, &values);
   
   values.foreground = q_disable_color;
   values.background = q_disable_color;
   values.font = default_font;
   valuemask = GCForeground | GCBackground | GCFont;
   disable_gc = XCreateGC( dpy, root, valuemask, &values);

   values.foreground = q_caldisable_color;
   values.background = q_caldisable_color;
   values.font = default_font;
   valuemask = GCForeground | GCBackground | GCFont;
   caldisable_gc = XCreateGC( dpy, root, valuemask, &values);

   values.foreground = q_calsuspend_color;
   values.background = q_calsuspend_color;
   values.font = default_font;
   valuemask = GCForeground | GCBackground | GCFont;
   calsuspend_gc = XCreateGC( dpy, root, valuemask, &values);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void qmonAllocColor(
Widget top 
) {
   DENTER(GUI_LAYER, "qmonAllocColor");

   /* 
   ** XmtAllocColor returns:
   ** 0  successful
   ** 1  colorname unrecognized or malformed
   ** 2  colormap was full
   */
   
   if ( XmtAllocWidgetColor(top, "$warning_color", &WarningPixel) == 2) {
      XmtWarningMsg("XmtAllocWidgetColor", "colormap_full", 
                     "Can't allocate color, colormap full");
   }

   if ( XmtAllocWidgetColor(top, "$q_selected_color", &QueueSelectedPixel) 
         == 2) {
      XmtWarningMsg("XmtAllocWidgetColor", "colormap_full", 
                     "Can't allocate color, colormap full");
   }

   if ( XmtAllocWidgetColor(top, "$job_suspend_color", &JobSuspPixel) 
         == 2) {
      XmtWarningMsg("XmtAllocWidgetColor", "colormap_full", 
                     "Can't allocate color, colormap full");
   }

   if ( XmtAllocWidgetColor(top, "$job_suspend_on_subordinate_color", 
            &JobSosPixel) 
         == 2) {
      XmtWarningMsg("XmtAllocWidgetColor", "colormap_full", 
                     "Can't allocate color, colormap full");
   }

   if ( XmtAllocWidgetColor(top, "$job_delete_color", &JobDelPixel) 
         == 2) {
      XmtWarningMsg("XmtAllocWidgetColor", "colormap_full", 
                     "Can't allocate color, colormap full");
   }

   if ( XmtAllocWidgetColor(top, "$job_hold_color", &JobHoldPixel) 
         == 2) {
      XmtWarningMsg("XmtAllocWidgetColor", "colormap_full", 
                     "Can't allocate color, colormap full");
   }

   if ( XmtAllocWidgetColor(top, "$job_error_color", &JobErrPixel) 
         == 2) {
      XmtWarningMsg("XmtAllocWidgetColor", "colormap_full", 
                     "Can't allocate color, colormap full");
   }

   if ( XmtAllocWidgetColor(top, "$tooltip_fg", &TooltipForeground) 
         == 2) {
      XmtWarningMsg("XmtAllocWidgetColor", "colormap_full", 
                     "Can't allocate color, colormap full");
   }

   if ( XmtAllocWidgetColor(top, "$tooltip_bg", &TooltipBackground) 
         == 2) {
      XmtWarningMsg("XmtAllocWidgetColor", "colormap_full", 
                     "Can't allocate color, colormap full");
   }


   DEXIT;
}

