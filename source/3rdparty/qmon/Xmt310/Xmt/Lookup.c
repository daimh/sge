/* 
 * Motif Tools Library, Version 3.1
 * $Id: Lookup.c,v 1.2 2002-08-22 15:06:11 andre Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Lookup.c,v $
 * Revision 1.2  2002-08-22 15:06:11  andre
 * AA-2002-08-22-0  I18N:      bunch of fixes for l10n
 *                  Bugtraq:   #4733802, #4733201, #4733089, #4733043,
 *                             #4731976, #4731990, #4731967, #4731958,
 *                             #4731944, #4731935, #4731273, #4729700
 *
 * Revision 1.1.1.1  2001/07/18 11:06:02  root
 * Initial checkin.
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <stdio.h>
#include <Xmt/Xmt.h>
#include <Xmt/LookupP.h>

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
extern char *getenv();
#endif

/* a dummy predicate for getting language string components */
#if NeedFunctionPrototypes
static Boolean always_true(String s)
#else
static Boolean always_true(s)
String s;
#endif
{
    return True;
}


/*
 * This utility is used internally to look up things under special
 * "branches" of the database.  The Pixmap and Bitmap converters use
 * it to look under _Pixmaps_ and _Bitmaps_, the string localization
 * code uses it to look under _Messages_, the font list converter uses
 * it to look under _FontLists_, and the ColorTable converter uses it
 * to look under _ColorTables_
 *
 * The "path" argument specifies how the db query should be specified--
 * each character in this string specifies one quark in the query, as
 * follows:
 *
 * P  _Pixmaps_
 * B  _Bitmaps_
 * M  _Messages_
 * F  _FontLists_
 * T  _ColorTables_
 * l  the 'language' part of the language string.
 * t  the 'territory' part of the language string.
 * c  the 'codeset' part of the language string.
 * f  the 'fontFamily' app-resource.
 * p  the 'palette' app-resource.
 * C  the customization string.
 * V  the visual type.  One of "color", "gray", "monochrome".
 * D  the screen depth.  An integer.
 * Z  the screen resolution.  One of "high", "medium" and "low".
 *
 * Note that many of these chars are the same as the substitution
 * characters used by XmtFindFile().
 */

#if NeedFunctionPrototypes
String _XmtLookupResource(Screen *screen, StringConst path, StringConst object)
#else
String _XmtLookupResource(screen, path, object)
Screen *screen;
StringConst path;
StringConst object;
#endif
{
    Display *dpy = DisplayOfScreen(screen);
    Visual *visual = DefaultVisualOfScreen(screen);
    int depth = DefaultDepthOfScreen(screen);
    XrmDatabase database = XtScreenDatabase(screen);

    static Boolean inited;
    static XrmQuark qP, qB, qM, qF, qT, qf, qp, ql, qt, qc, qC;
    static Boolean debug;

    String visual_sub, size_sub;
    char depth_sub[4];
    int screen_width;

    XrmQuark names[20], classes[20];
    int n;
    XrmQuark type;
    XrmValue value;

    if (!inited) {
	String language, territory, codeset, customization;
	String appname, appclass;
	char resname[200], resclass[200];
	String type;
	XrmValue resvalue;

	qP = XrmPermStringToQuark("_Pixmaps_");
	qB = XrmPermStringToQuark("_Bitmaps_");
	qM = XrmPermStringToQuark("_Messages_");
	qF = XrmPermStringToQuark("_FontLists_");
	qT = XrmPermStringToQuark("_ColorTables_");

	XtGetApplicationNameAndClass(dpy, &appname, &appclass);
	sprintf(resname, "%s.fontFamily", appname);
	sprintf(resclass, "%s.FontFamily", appclass);
	if (XrmGetResource(database, resname, resclass, &type, &resvalue))
	    qf = XrmStringToQuark((String)resvalue.addr);

	sprintf(resname, "%s.palette", appname);
	sprintf(resclass, "%s.Palette", appclass);
	if (XrmGetResource(database, resname, resclass, &type, &resvalue))
	    qp = XrmStringToQuark((String)resvalue.addr);

	/* figure out the components of the language string */
	language = XtResolvePathname(dpy, NULL, NULL, NULL,
				     "%l", NULL, 0, always_true);
/* printf("++++++++++++ language: '%s'\n", language);                  */
	territory = XtResolvePathname(dpy, NULL, NULL, NULL,
				      "%t", NULL, 0, always_true);
/* printf("++++++++++++ territory: '%s'\n", territory);                  */
	codeset = XtResolvePathname(dpy, NULL, NULL, NULL,
				    "%c", NULL, 0, always_true);
/* printf("++++++++++++ codeset: '%s'\n", codeset);                  */
	customization = XtResolvePathname(dpy, NULL, NULL, NULL,
					  "%C", NULL, 0, always_true);
/* printf("++++++++++++ customization: '%s'\n", customization);                  */
	
	if (language && *language) {
      /*
      ** FIXME
      ** XtResolvePathname workaround for Solaris
      */
      if (strchr(language, '/')) {
         int i;
         static char trimmed_language[1024];
         for (i=0; language[i+1] && i<1024; i++) {
            if (language[i+1] == '/')
               break;
            trimmed_language[i] = language[i+1];
         }   
         trimmed_language[++i] = '\0';
         ql = XrmStringToQuark(trimmed_language);
       } else {   
         ql = XrmStringToQuark(language);
       }  
   }      
	if (territory && *territory) {
      if (strchr(territory, '/')) {
         int i;
         static char trimmed_territory[1024];
         for (i=0; territory[i] && i<1024; i++) {
            trimmed_territory[i] = territory[i];
         }   
         trimmed_territory[++i] = '\0';
         qt = XrmStringToQuark(trimmed_territory);
      } else   
         qt = XrmStringToQuark(territory);
   }      
	if (codeset && *codeset) {
      /*
      ** FIXME
      ** XtResolvePathname workaround for Solaris
      */
      if (strchr(codeset, '/')) {
         int i;
         static char trimmed_codeset[1024];
         for (i=0; codeset[i] && i<1024; i++) {
            trimmed_codeset[i] = codeset[i];
         }   
         trimmed_codeset[++i] = '\0';
         qc = XrmStringToQuark(trimmed_codeset);
      } else   
         qc = XrmStringToQuark(codeset);
   }   
	if (customization && *customization)
	    qC = XrmStringToQuark(customization);

	XtFree(language);
	XtFree(territory);
	XtFree(codeset);
	XtFree(customization);

#ifndef NDEBUG
	debug = (getenv("XMTDEBUGLOOKUP") != NULL);
#endif	

	inited = True;
    }

    n = 0;
    while (*path) {
	XrmQuark q;
	
	switch(*path) {
	case 'P': q = qP; break;
	case 'B': q = qB; break;
	case 'M': q = qM; break;
	case 'F': q = qF; break;
	case 'T': q = qT; break;
	case 'l': q = ql; break;
	case 't': q = qt; break;
	case 'c': q = qc; break;
	case 'f': q = qf; break;
	case 'p': q = qp; break;
	case 'C': q = qC; break;
	case 'V':
	    /* these aren't precomputed, because they depend on Screen */
	    if (visual->map_entries == 2) visual_sub = "monochrome";
	    else if ((visual->class == StaticGray) ||
		     (visual->class == GrayScale)) visual_sub = "gray";
	    else visual_sub = "color";
	    q = XrmPermStringToQuark(visual_sub);
	    break;
	case 'D': 
	    sprintf(depth_sub, "%d", depth);
	    q = XrmStringToQuark(depth_sub);
	    break;
	case 'Z':
	    screen_width = DisplayWidth(dpy, XScreenNumberOfScreen(screen));
	    if (screen_width < 750) size_sub = "small";
	    else if (screen_width > 1150) size_sub = "large";
	    else size_sub = "medium";
	    q = XrmPermStringToQuark(size_sub);
	    break;
	default:  q = NULLQUARK; break;
	}

	/* the language and customization quarks may not be defined, so check*/
	if (q) {
	    names[n] = classes[n] = q;
	    n++;
	}
	path++;
    }

    /* once the fixed quarks are set up, add quarks from the object name */
    XrmStringToQuarkList(object, &names[n]);
    for(;names[n] != NULLQUARK; n++) classes[n] = names[n];
    classes[n] = NULLQUARK;

#ifndef NDEBUG
    /* if we are debugging, print a message */
    if (debug) {
	int i;
	printf("Looking for: ");
	for(i=0; names[i] != NULLQUARK; i++)
	    printf("%s%s", (i==0)?"":".", XrmQuarkToString(names[i]));
	printf("\n");
    }
#endif	

    /* and go look up the resource */
    if (XrmQGetResource(database, names, classes, &type, &value))
	return ((String) value.addr);
    else
	return NULL;
}
