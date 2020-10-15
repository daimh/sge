/* 
 * Motif Tools Library, Version 3.1
 * $Id: Color.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Color.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <ctype.h>
#include <Xmt/Xmt.h>
#include <Xmt/Lexer.h>
#include <Xmt/AppResP.h>
#include <X11/IntrinsicP.h>

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
extern long int strtol();
#endif

/*
** undefine if you don't want the full colormap fix
** contributed by Glenn Carr 
*/
#define CLOSEST_COLOR


/*
 * HSL color syntax: H / S / L, where H, S, and L are integers
 * with an optional leading '+' or '-'.  Whitespace is okay.
 */
#if NeedFunctionPrototypes
static Boolean ParseHSLColor(StringConst name, XColor *c,
			     XmtAppResources *appres, Boolean is_foreground)
#else
static Boolean ParseHSLColor(name, c, appres, is_foreground)
StringConst name;
XColor *c;
XmtAppResources *appres;
Boolean is_foreground;
#endif
{
    int h, s, l;
    unsigned r, g, b;
    unsigned base_h, base_s, base_l;
    String hue, saturation, lightness, dummy;

    hue = (String)name;
    h = strtol(hue, &saturation, 0);
    if (saturation == hue) return False;

    while(isspace(*saturation)) saturation++;
    if (*saturation++ != '/') return False;
    s = strtol(saturation, &lightness, 0);
    if (lightness == saturation) return False;

    while(isspace(*lightness)) lightness++;
    if (*lightness++ != '/') return False;
    l = strtol(lightness, &dummy, 0);
    if (dummy == lightness) return False;

    while(isspace(*dummy)) dummy++;
    if (*dummy) return False;

    if (is_foreground) {
	base_h = appres->foreground_hue;
	base_s = appres->foreground_saturation;
	base_l = appres->foreground_lightness;
    }
    else {
	base_h = appres->background_hue;
	base_s = appres->background_saturation;
	base_l = appres->background_lightness;
    }

    if ((hue[0] == '+') || (hue[0] == '-'))
	h = (h + base_h) % 360;
    if ((saturation[0] == '+') || (saturation[0] == '-')) {
	s = s + base_s;
	if (s < 0) s = 0;
	if (s > 100) s = 100;
    }
    if ((lightness[0] == '+') || (lightness[0] == '-')) {
	l = l + base_l;
	if (l < 0) l = 0;
	if (l > 100) l = 100;
    }

    XmtHSLToRGB(h, s, l, &r, &g, &b);
    c->red = r;
    c->green = g;
    c->blue = b;
    c->flags = DoRed | DoGreen | DoBlue;
    return True;
}

#ifdef CLOSEST_COLOR

/* Legal Garbage:  I borrowed the basis of this code from the...
 *
 * The Tk Toolkit
 * by John Ousterhout (and many others at Sun Microsystems and elsewhere)
 * john.ousterhout@eng.sun.com
 *
 * This software is copyrighted by the Regents of the University of
 * California, Sun Microsystems, Inc., and other parties.  The following
 * terms apply to all files associated with the software unless explicitly
 * disclaimed in individual files.
 * 
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 * 
 * IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY
 * DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE
 * IS PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE
 * NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS.
 * 
 * RESTRICTED RIGHTS: Use, duplication or disclosure by the government
 * is subject to the restrictions as set forth in subparagraph (c) (1) (ii)
 * of the Rights in Technical Data and Computer Software Clause as DFARS
 * 252.227-7013 and FAR 52.227-19.
 */

#if NeedFunctionPrototypes
static void
FindClosestColor(Screen * screen,
                 Colormap colormap,
                 XColor * pxcolorDesired,
                 XColor * pxcolorActual)
#else
static void
FindClosestColor(screen, colormap, pxcolorDesired, pxcolorActual)
Screen * screen;
Colormap colormap;
XColor * pxcolorDesired;
XColor * pxcolorActual;
#endif
{
    float           closestDistance, distance, tmp;
    Display        *dpy = DisplayOfScreen(screen);
    XtAppContext    appContext = XtDisplayToApplicationContext(dpy);
    Visual         *pvisual = XDefaultVisualOfScreen(screen);
    static XColor  *pxcolors;
    int             i, closest;
    int             nColors = pvisual->map_entries;

    /*
     * Get all the colors from the colormap.  These should all be
     * allocated, otherwise we wouldn't be here.
     */
    if (NULL == pxcolors) {
        pxcolors = calloc(nColors, sizeof(*pxcolors));
        for (i = 0; i < nColors; i++)
            pxcolors[i].pixel = i;
    }
    XQueryColors(dpy, colormap, pxcolors, pvisual->map_entries);

    while (nColors > 0) {
        closestDistance = 1e30;
        closest = 0;
        for (i = 0; i < nColors; i++) {
            /*
             * Use Euclidean distance in RGB space, weighted by Y (of YIQ)
             * as the objective function;  this accounts for differences
             * in the color sensitivity of the eye.
             */
            tmp = .30 * (((int) pxcolorDesired->red) - (int) pxcolors[i].red);
            distance = tmp * tmp;
            tmp = .61 * (((int) pxcolorDesired->green) - (int) pxcolors[i].green);
            distance += tmp * tmp;
            tmp = .11 * (((int) pxcolorDesired->blue) - (int) pxcolors[i].blue);
            distance += tmp * tmp;
            if (distance < closestDistance) {
                closest = i;
                closestDistance = distance;
            }
        }
        if (XAllocColor(dpy, colormap, &pxcolors[closest])) {
            *pxcolorActual = pxcolors[closest];
            return;
        }
        pxcolors[closest] = pxcolors[nColors - 1];
        nColors -= 1;
    }

    /*
     * Should never get here, but just in case...
     */
    {
        char            szColor[64];
        Cardinal        num_params = 1;
        String          params[1]; 

        params[0] = szColor;

        sprintf(szColor, "#%04x%04x%04x",
                pxcolorDesired->red,
                pxcolorDesired->green,
                pxcolorDesired->blue);

        XtAppWarningMsg(appContext, "badValue", "cvtStringToPixel",
                        "FindClosestColor",
                        "Can't find approximation for color \"%s\"",
                        params, &num_params);
    }

}                               /* FindClosestColor */

#endif
/*
 * This function allocates the named color, using the specified
 * colormap, visual, and color table.  The widget argument is
 * used only to obtain the display, and application resoruces.
 * If the colormap, visual, or color table are unspecified, the
 * widget is used to obtain default values.
 * 
 * Return codes:
 *   0 -> successful allocation
 *   1 -> unknown or malformed color name
 *   2 -> colormap full
 */

#if NeedFunctionPrototypes
int XmtAllocColor(Widget w, Colormap cmap, Visual *visual,
		  XmtColorTable ctable, StringConst name,
		  Pixel *pixel_return)
#else
int XmtAllocColor(w, cmap, visual, ctable, name, pixel_return)
Widget w;
Colormap cmap;
Visual *visual;
XmtColorTable ctable;
StringConst name;
Pixel *pixel_return;
#endif
{
    Widget shell = XmtGetApplicationShell(w);
    Display *dpy = XtDisplay(shell);
    Colormap root_colormap = shell->core.colormap;
    XmtAppResources *appres = XmtGetApplicationResources(shell);
    Boolean is_foreground = (name[0] == '+');
    XColor c1, c2;

#ifdef CLOSEST_COLOR
    Screen* screen = XtScreen(w);
#endif


    /* get default values for unspecified values */
    while(!XtIsWidget(w)) w = XtParent(w);
    if (cmap == None) cmap = w->core.colormap;
    if (visual == NULL) visual = XmtGetVisual(w);
    if (ctable == NULL) ctable = appres->colortable;

    /*
     * first thing we do is look for a monochrome "fallback" color.
     * If the first character in the color name is '+', it means
     * that the color is a foreground color.  If the 1st char is '-',
     * it means that the color is a background color.  If this is a
     * monochrome visual, return the foreground or background color.
     * Otherwise, ignore this first character.
     */
    if ((name[0] == '+') || (name[0] == '-')) {
	if ((visual->map_entries == 2) && (cmap == root_colormap)) {
	    if (name[0] == '+') *pixel_return = appres->foreground;
	    else *pixel_return = appres->background;
	    return 0;
	}
	name++;
    }

    /*
     * Now parse some other color types, depending on the (possibly new)
     * first character in the color name:
     *   '#' implies a standard X hexidecimal RGB spec, parsed with
     *       XParseColor for compatibility with X11R4.
     *   '%' implies an Xmt HSL color, which can include H,S,L deltas.
     *   '$' implies an Xmt symbolic color, looked up in the color table
     * If a color does not begin with one of these three characters, then
     * it is first compared to the constants XtDefaultForeground and
     * XtDefaultBackground, for compatibility with the Xt Pixel converter.
     * Otherwise, it is a color name, or and Xcms color specification,
     * and is converted with XAllocNamedColor()
     */
    if (name[0] == '#') {
	if (XParseColor(dpy, cmap, name, &c1) == 0) return 1;
#ifdef CLOSEST_COLOR
        if (XAllocColor(dpy, cmap, &c1) == 0)
            FindClosestColor(screen, cmap, &c1, &c1);
#else
        if (XAllocColor(dpy, cmap, &c1) == 0) return 2;
#endif
	*pixel_return = c1.pixel;
	return 0;
    }
    else if (name[0] == '%') {
	if (ParseHSLColor(&name[1], &c1, appres, is_foreground) == False)
	    return 1;
#ifdef CLOSEST_COLOR
        if (XAllocColor(dpy, cmap, &c1) == 0)
            FindClosestColor(screen, cmap, &c1, &c1);
#else
        if (XAllocColor(dpy, cmap, &c1) == 0) return 2;
#endif
	*pixel_return = c1.pixel;
	return 0;
    }
    else if (name[0] == '$') {  /* lookup symbolic colors by recursing */
	String newname;
	if (!ctable) return 1;
	newname = XmtLookupColorName(ctable, &name[1]);
	if (newname == NULL) return 1;
	return XmtAllocColor(w, cmap, visual, ctable, newname, pixel_return);
    }
    else {
	if (strcmp(name, XtDefaultBackground) == 0) {
	    if (cmap == root_colormap) {
		*pixel_return = appres->background;
		return 0;
	    }
	    else return XmtAllocColor(w, cmap, visual, ctable,
				       "#ffffffffffff", pixel_return);
	}
	if (strcmp(name, XtDefaultForeground) == 0) {
	    if (cmap == root_colormap) {
		*pixel_return = appres->foreground;
		return 0;
	    }
	    else return XmtAllocColor(w, cmap, visual, ctable, "#000",
				       pixel_return);
	}
	if (XAllocNamedColor(dpy, cmap, name, &c1, &c2)) {
	    *pixel_return = c1.pixel;
	    return 0;
	}
	else {
#ifdef CLOSEST_COLOR
            if (XLookupColor(dpy, cmap, name, &c1, &c2)) {
                FindClosestColor(screen, cmap, &c2, &c2);
                *pixel_return = c2.pixel;
                return 0;
            }
#else
            if (XLookupColor(dpy, cmap, name, &c1, &c2)) return 2;
#endif
	    else return 1;
	}
    }
}

/*
 * Call XmtAllocColor, using default colormap, visual, & color table.
 */
#if NeedFunctionPrototypes
int XmtAllocWidgetColor(Widget w, StringConst name, Pixel *pixel_return)
#else
int XmtAllocWidgetColor(w, name, pixel_return)
Widget w;
StringConst name;
Pixel *pixel_return;
#endif
{
    return XmtAllocColor(w, None, NULL, NULL, name, pixel_return);
}

/*
 * This function calls XFreeColors() to free most allocated values,
 * but doesn't free the foreground and background colors in the
 * application's default colormap--XmtAllocColor() can return these
 * pixel values without actually allocating them.
 * The widget argument is used to obtain the display, and the app
 * resources needed to deterermine whether the pixel should be freed.
 * If no colormap is specified, the widget's colormap will be used.
 * The only real reason to specify separate widget and colormap arguments
 * is for the String-to-Pixel converter which needs to cache on a
 * per-colormap basis without caching on a per-widget basis.  It passes
 * the widget colormap, but always passes the application shell widget
 */
 
#if NeedFunctionPrototypes
void XmtFreeColor(Widget w, Colormap cmap, Pixel color)
#else
void XmtFreeColor(w, cmap, color)
Widget w;
Colormap cmap;
Pixel color;
#endif
{
    Widget shell = XmtGetApplicationShell(w);
    Display *dpy = XtDisplay(shell);
    Colormap root_colormap = shell->core.colormap;
    XmtAppResources *appres = XmtGetApplicationResources(shell);

    if (cmap == None) {
	while(!XtIsWidget(w)) w = XtParent(w);
	cmap = w->core.colormap;
    }

    /*
     * XmtAllocColor() sometimes doesn't actually allocate a color,
     * but instead just returns the default fg or bg color.  In this
     * case, there is nothing to free.
     */
    if ((cmap == root_colormap) &&
	((color == appres->background) || (color == appres->foreground)))
	return;

    XFreeColors(dpy, cmap, &color, 1, 0L);
}

/*
 * A simpler version
 */
#if NeedFunctionPrototypes
void XmtFreeWidgetColor(Widget w, Pixel color)
#else
void XmtFreeWidgetColor(w, color)
Widget w;
Pixel color;
#endif
{
    XmtFreeColor(w, w->core.colormap, color);
}

/*
 * Like XmtAllocColor, but stores the color into an already-allocated
 * read/write color cell.  Since read/write Visuals are not monochrome,
 * it ignores the +/- fallback syntax, and also does not handle
 * XtDefault[Fore|Back]Ground.  Returns 0 on success, or 1 if 
 * the color name was unknown or malformed.  It never returns 2,
 * like XmtAllocColor() can. Note that it takes a roundtrip
 * to the server to check color names, and then another request
 * to store the color--we need to do this to prevent X errors on
 * bad color names.  Also, note that this function takes a Visual *
 * argument that is currently unused.  This is to allow a future extension
 * that will allow different colors to be specified depending on visual
 * type (e.g. for gray scale).
 */
/* ARGSUSED */
#if NeedFunctionPrototypes
int XmtStoreColor(Widget w, Colormap cmap, Visual *visual,
		  XmtColorTable ctable, StringConst name, Pixel pixel)
#else
int XmtStoreColor(w, cmap, visual, ctable, name, pixel)
Widget w;
Colormap cmap;
Visual *visual;
XmtColorTable ctable;
StringConst name;
Pixel pixel;
#endif
{
    Display *dpy;
    XmtAppResources *appres = XmtGetApplicationResources(w);
    Boolean is_foreground = (name[0] == '+');
    XColor c1, c2;

    /* set up default values for unspecified arguments */
    while(!XtIsWidget(w)) w = XtParent(w);
    dpy = XtDisplay(w);
    if (cmap == None) cmap = w->core.colormap;
    if (visual == NULL) visual = XmtGetVisual(w);
    if (ctable == NULL) ctable = appres->colortable;

    c1.flags = DoRed | DoGreen | DoBlue;
    
    /* ignore foreground/background fallbacks */
    if ((name[0] == '-') || (name[0] == '+')) name++;

    if (name[0] == '#') {
	if (XParseColor(dpy, cmap, name, &c1) == 0) return 1;
	XStoreColor(dpy, cmap, &c1);
	return 0;
    }
    else if (name[0] == '%') {
	if (ParseHSLColor(&name[1], &c1, appres, is_foreground) == False)
	    return 1;
	XStoreColor(dpy, cmap, &c1);
	return 0;
    }
    else if (name[0] == '$') {
	String newname;
	if (!ctable) return 1;
	/*
	 * handle symbolic colors by recursing, since the XmtColorTable
	 * API doesn't include a function for storing colors in
	 * read/write cells.
	 */
	newname = XmtLookupColorName(ctable, &name[1]);
	if (newname == NULL) return 1;
	return XmtStoreColor(w, cmap, visual, ctable, newname, pixel);
    }
    else {
	if (XLookupColor(dpy, cmap, name, &c2, &c1) == 0) return 1;
	XStoreColor(dpy, cmap, &c1);
	return 0;
    }
}

#if NeedFunctionPrototypes
int XmtStoreWidgetColor(Widget w, StringConst name, Pixel pixel)
#else
int XmtStoreWidgetColor(w, name, pixel)
Widget w;
StringConst name;
Pixel pixel;
#endif
{
    return XmtStoreColor(w, None, NULL, NULL, name, pixel);
}
