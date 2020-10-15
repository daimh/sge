/* 
 * Motif Tools Library, Version 3.1
 * $Id: ShellUtil.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: ShellUtil.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/ConvertersP.h>
#include <X11/IntrinsicP.h>  /* for faster subclass checking */
#include <Xm/AtomMgr.h>
#include <Xm/Protocols.h>

#if NeedFunctionPrototypes
Widget XmtGetApplicationShell(Widget w)
#else
Widget XmtGetApplicationShell(w)
Widget w;
#endif
{
    if (w == NULL) return NULL;
    while(XtParent(w)) w = XtParent(w);
    return w;
}

#if NeedFunctionPrototypes
Widget XmtGetTopLevelShell(Widget w)
#else
Widget XmtGetTopLevelShell(w)
Widget w;
#endif
{
    if (w == NULL) return NULL;
    while(!XtIsTopLevelShell(w)) w = XtParent(w);
    return w;
}

#if NeedFunctionPrototypes
Widget XmtGetShell(Widget w)
#else
Widget XmtGetShell(w)
Widget w;
#endif
{
    /*
     * XXX  what happens when we go up from a menu button?
     * do we find the menu shell or the main shell?  This is
     * important for callback functions from menu buttons
     */
    if (w == NULL) return NULL;
    while (!XtIsShell(w)) w = XtParent(w);
    return w;
}

#if NeedFunctionPrototypes
void XmtAddDeleteCallback(Widget shell, int response,
			  XtCallbackProc proc, XtPointer data)
#else
void XmtAddDeleteCallback(shell, response, proc, data)
Widget shell;
int response;
XtCallbackProc proc;
XtPointer data;
#endif
{
    static Atom wm_delete_window = 0;

    if (!wm_delete_window) {
	wm_delete_window = XmInternAtom(XtDisplay(shell),
					"WM_DELETE_WINDOW", False);
    }

    XtVaSetValues(shell, XmNdeleteResponse, response, NULL);
    XmAddWMProtocolCallback(shell, wm_delete_window, proc, data);
}

#if NeedFunctionPrototypes
void XmtAddSaveYourselfCallback(Widget shell,
				XtCallbackProc proc, XtPointer data)
#else
void XmtAddSaveYourselfCallback(shell, proc, data)
Widget shell;
XtCallbackProc proc;
XtPointer data;
#endif
{
    static Atom wm_save_yourself;

    if (!XtIsApplicationShell(shell)) {
	XmtWarningMsg("XmtAddSaveYourselfCallback", "badShell",
		      "specified shell '%s' is not an ApplicationShell.",
		      XtName(shell));
	return;
    }

    if (!wm_save_yourself) {
	wm_save_yourself = XmInternAtom(XtDisplay(shell),
					"WM_SAVE_YOURSELF", False);
    }
    else {
	XmtWarningMsg("XmtAddSaveYourselfCallback", "multiple",
		      "only one callback may be registered per process.");
	return;
    }

    XmAddWMProtocolCallback(shell, wm_save_yourself, proc, data);
}

#if NeedFunctionPrototypes
void XmtIconifyShell(Widget w)
#else
void XmtIconifyShell(w)
Widget w;
#endif
{
    Widget shell = XmtGetShell(w);

    if (!XtIsTopLevelShell(shell)) {
	XmtWarningMsg("XmtIconifyShell", "badShell",
		      "Shell widget '%s' is not a TopLevelShell.",
		      XtName(shell));
	return;
    }
    XtVaSetValues(shell, XtNiconic, True, NULL);
}

#if NeedFunctionPrototypes
void XmtDeiconifyShell(Widget w)
#else
void XmtDeiconifyShell(w)
Widget w;
#endif
{
    Widget shell = XmtGetShell(w);

    if (!XtIsTopLevelShell(shell)) {
	XmtWarningMsg("XmtDeiconifyShell", "badShell",
		      "Shell widget '%s' is not a TopLevelShell.",
		      XtName(shell));
	return;
    }
    XtVaSetValues(shell, XtNiconic, False, NULL);
    if (!XtIsRealized(shell)) return;
    XMapWindow(XtDisplay(shell), XtWindow(shell));
}

#if NeedFunctionPrototypes
void XmtRaiseShell(Widget w)
#else
void XmtRaiseShell(w)
Widget w;
#endif
{
    Widget shell = XmtGetShell(w);

    if (!XtIsRealized(shell)) return;
    XmtDeiconifyShell(shell);
    XRaiseWindow(XtDisplay(shell), XtWindow(shell));
}

#if NeedFunctionPrototypes
void XmtLowerShell(Widget w)
#else
void XmtLowerShell(w)
Widget w;
#endif
{
    Widget shell = XmtGetShell(w);

    if (!XtIsRealized(shell)) return;
    XLowerWindow(XtDisplay(shell), XtWindow(shell));
}


#if NeedFunctionPrototypes
void XmtSetFocusToShell(Widget w)
#else
void XmtSetFocusToShell(w)
Widget w;
#endif
{
    Widget shell = XmtGetShell(w);

    if (!XtIsRealized(shell)) return;
    XmtRaiseShell(shell);
    XSetInputFocus(XtDisplay(shell), XtWindow(shell), RevertToParent,
		   XtLastTimestampProcessed(XtDisplay(shell)));
}    

#if NeedFunctionPrototypes
void XmtWarpToShell(Widget w)
#else
void XmtWarpToShell(w)
Widget w;
#endif
{
    Widget shell = XmtGetShell(w);
    int width, height;

    if (!XtIsRealized(shell)) return;
    XmtRaiseShell(shell);
    XtVaGetValues(shell, XtNwidth, &width, XtNheight, &height, NULL);
    XWarpPointer(XtDisplay(shell), None, XtWindow(shell),
		 0,0,0,0, width/2, height/2);
    XSetInputFocus(XtDisplay(shell), XtWindow(shell), RevertToParent,
		   XtLastTimestampProcessed(XtDisplay(shell)));
}

#if NeedFunctionPrototypes
void XmtMoveShellToPointer(Widget w)
#else
void XmtMoveShellToPointer(w)
Widget w;
#endif
{
    Widget shell = XmtGetShell(w);
    Display *dpy = XtDisplay(shell);
    int width, height;
    int x,y;
    int screennum = XScreenNumberOfScreen(XtScreen(shell));
    int rootwidth = DisplayWidth(dpy, screennum);
    int rootheight = DisplayHeight(dpy, screennum);
    Window root, child;
    int dummyx, dummyy;
    unsigned mask;

    XmtRaiseShell(shell);
    XtVaGetValues(shell, XtNwidth, &width, XtNheight, &height, NULL);
    XQueryPointer(dpy, XtWindow(shell), &root, &child, &x, &y,
		  &dummyx, &dummyy, &mask);
    x -= width/2;
    y -= height/2;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > rootwidth - width) x = rootwidth - width;
    if (y > rootheight - height) y = rootheight - height;

    XtVaSetValues(shell, XtNx, x, XtNy, y, NULL);
    XSetInputFocus(XtDisplay(shell), XtWindow(shell), RevertToParent,
		   XtLastTimestampProcessed(XtDisplay(shell)));
}


typedef enum {
    FocusNone,
    FocusSetInput,
    FocusWarp,
    FocusMove,
    FocusStyleUnset  /* a invalid default value */
} FocusStyle;

#define XmtNfocusStyle "focusStyle"
#define XmtCFocusStyle "FocusStyle"
#define XmtRFocusStyle "FocusStyle"

static String focus_names[] = { "focus", "move", "none", "warp"};
static Cardinal focus_values[] = {FocusSetInput, FocusMove, FocusNone, FocusWarp};

static XtResource focus_resources[] = {
  {XmtNfocusStyle, XmtCFocusStyle, XmtRFocusStyle,
       sizeof(FocusStyle), 0, XmtRFocusStyle, (XtPointer) FocusStyleUnset}
};
    

#if NeedFunctionPrototypes
void XmtFocusShell(Widget w)
#else
void XmtFocusShell(w)
Widget w;
#endif
{
    static Boolean got_focus_style;
    static FocusStyle focus_style;
    Widget shell = XmtGetShell(w);

    if (!got_focus_style) {
	XmtRegisterEnumConverter(XmtRFocusStyle, focus_names, focus_values,
				 XtNumber(focus_names), NULL);
	XtGetApplicationResources(shell, (XtPointer)&focus_style,
				  focus_resources, 1, NULL, 0);
	if (focus_style == FocusStyleUnset){
	    if (XmIsMotifWMRunning(shell)) focus_style = FocusSetInput;
	    else focus_style = FocusNone;
	}
	got_focus_style = True;
    }

    switch(focus_style) {
    case FocusNone:
    default:
	XmtRaiseShell(shell);
	break;
    case FocusSetInput:
	XmtSetFocusToShell(shell);
	break;
    case FocusWarp:
	XmtWarpToShell(shell);
	break;
    case FocusMove:
	XmtMoveShellToPointer(shell);
	break;
    }
}
