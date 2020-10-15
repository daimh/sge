/* 
 * Motif Tools Library, Version 3.1
 * $Id: FontListCvt.c,v 1.2 2002-08-22 15:06:10 andre Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: FontListCvt.c,v $
 * Revision 1.2  2002-08-22 15:06:10  andre
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
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <Xmt/Xmt.h>
#include <Xmt/ConvertersP.h>
#include <Xmt/LookupP.h>

/* ARGSUSED */
#if NeedFunctionPrototypes
Boolean XmtConvertStringToXmFontList(Display *dpy,
                                     XrmValue *args, Cardinal *num_args,
                                     XrmValue *from, XrmValue *to,
                                     XtPointer *converter_data)
#else
Boolean XmtConvertStringToXmFontList(dpy, args, num_args,
                                     from, to, converter_data)
Display *dpy;
XrmValue *args;
Cardinal *num_args;
XrmValue *from;
XrmValue *to;
XtPointer *converter_data;
#endif
{
    Screen *screen = *(Screen **)args[0].addr;
    String copy = XtNewString((String) from->addr);
    String s, fs, bs;
    String next_entry;
    String tag;
    String tmp;
    XmFontType font_type = XmFONT_IS_FONT;
    XmFontListEntry entry;
    XmFontList fontlist = NULL;
    int j;
    static Boolean fontdebug = False;
    
    if (getenv("XMTDEBUGFONT"))
       fontdebug = True;

    /*
     * This XmFontList converter does not handle XFontSets defined in
     * X11R5 and Motif 1.2.  This is to make it easier to write, and
     * for portablity to R4 and 1.1.  Also, this converter uses the 1.1
     * XmFontList API.  Anyone using multi-charset Asian languages will
     * not want to use this converter.
     * This converter also does not support the (undocumented?) features
     * of the Motif converter that allows quotes in font names.
     * The main reason to use the converter is because it handles
     * font list indirection.  And because it doesn't have the bug
     * that 1.2.3 does...
     *
     * The syntax is:
     *    font-name [ `=' tag ] { `,' font-name [ `=' tag ] }
     * 
     * and we extend this also to allow:
     *    `$' symbolic-font-list-name
     * A symbolic name like this will be looked up under _Fonts_,
     * depending on display resolution, and other factors.
     */
    
    s = copy;

    /* handle a symbolic font list name by recursing */
    if (s[0] == '$') {
        String value;
        XrmValue new_from;

        /* get symbolic name */
        s++;
        while(isspace(*s)) s++;
        
        /*
         * lookup value of that symbolic name under:
         *   _FontLists_.fontFamily.language.territory.codeset
         * Note that we don't have to free the return value.
         */
        value = _XmtLookupResource(screen, "Ffltc", s);
        
        /* if no value defined, warn and fail */
        if (!value) {
            XmtWarningMsg("XmtConvertStringToFontList", "nosymbol",
                          "No font list with name '%s' defined in resource file under _FontLists_",
                          s);
            goto fail;
        }

        /*
         * Otherwise, recurse to convert the definition we've found 
         * The programmer must be smart enough to avoid infinite recursion
         */
        XtFree(copy);
        new_from.addr = (XPointer) value;
        new_from.size = strlen(value) + 1;
        return XmtConvertStringToXmFontList(dpy, args, num_args,
                                            &new_from, to, converter_data);
    }
    
    if (fontdebug) {
       printf("FontListCvt:\n->%s<-\n------------------------\n", 
                  s ? s : "empty" );    
    }
       

    /* Otherwise, it is not a symbolic font name */
    while(s && *s) {
        /* skip white-space */
        while(isspace(*s)) s++;
        if (!*s) break;

        /* isolate the first entry, and remember start of next entry */
        next_entry = strchr(s, ',');
        if (next_entry) {
            *next_entry = '\0';
            next_entry++;
        }

        bs = s;
        j = 0;
#if 0
        while (bs && *bs) {
           bs_next = strchr(bs, ';');
           if (bs_next) {
               *bs_next = '\0';
               bs_next++;
           }
#endif
           if (fontdebug) {       
              printf("baselist[%d]: '%s'\n", j, bs);
           }  
           /*
            * see if it is a fontset
            */
           fs = strchr(bs, ':');
           if (fs)
               font_type = XmFONT_IS_FONTSET;
           else     
               font_type = XmFONT_IS_FONT;
               
           if (fontdebug) {       
              printf("font_type: '%s'\n", (font_type==XmFONT_IS_FONT) ? 
                           "XmFONT_IS_FONT": "XmFONT_IS_FONTSET");
           }  
           /*
            * find the start of the font tag, if any
            * and isolate the font name from it.
            * remove whitespace from beginning and end of tag
            */
           if (font_type == XmFONT_IS_FONT) 
               tag = strchr(bs, '=');
           else
               tag = strchr(bs, ':');

           if (tag) {
               *tag = '\0';
               tag++;
               while(isspace(*tag)) tag++;
               for(tmp=tag; *tmp && !isspace(*tmp); tmp++)  ;
               *tmp = '\0';
           }
           else {
   #if (XmVersion < 1002)
               tag = XmSTRING_DEFAULT_CHARSET;
   #else
               tag = XmFONTLIST_DEFAULT_TAG;
   #endif            
           }

           /* remove any whitespace from the end of the font name */
           tmp = bs + strlen(bs) - 1;
           while (isspace(*tmp)) tmp--;
           *(++tmp) = '\0';

#if 0
           /*
           ** fontset support it is a bit tricky, but needed for multibyte chars
           ** FIXME: figure out how to do it
           */
           if ((font_type == XmFONT_IS_FONTSET))  {
              XFontSet fontset;
              char **missing_list;
              int missing_count;
              int i;
              char *def_string = NULL; 
              if (fontdebug)
                 printf("FontSet: %s\n--------------\n", bs);

              fontset = XCreateFontSet(dpy, bs, &missing_list, &missing_count,
                                              &def_string);
              for (i=0; i<missing_count; i++) {
                  printf("missing_list[%d]: '%s'\n", i, missing_list[i]);
/*                        XtFree(missing_list[i]); */
              }
/*                    XtFree((char *)missing_list); */
           }
#if 1           
           else {
              XFontStruct *font;
              font = XLoadQueryFont(dpy, s);
              if (!font) {
                 XmtWarningMsg("XmtConvertStringToFontList", "badfont",
                          "unknown font '%s'.\n\tUsing default.",
                          s);
                 /* this default is from the X11R5 font converter */
                 font = XLoadQueryFont(dpy, "-*-*-*-R-*-*-*-120-*-*-*-*-ISO8859-1");

                 /* if still no font, then we fail */
                 if (!font) {
                    XmtWarningMsg("XmtConvertStringToFontList", "nodefault",
                              "cannot load any default font.");
                    goto fail;
                 }
              }     
           }
#endif

#endif
           if (fontdebug) {
               printf("bs: '%s (%s)'\n", bs, tag);
           }    
           entry = XmFontListEntryLoad(dpy, bs, font_type, tag ? tag : XmFONTLIST_DEFAULT_TAG);
           fontlist = XmFontListAppendEntry(fontlist, entry);
           XmFontListEntryFree(&entry);
#if 0
         bs = bs_next;
         j++;
      }
#endif

#if 0
        font = XLoadQueryFont(dpy, s);
        if (!font) {
            XmtWarningMsg("XmtConvertStringToFontList", "badfont",
                          "unknown font '%s'.\n\tUsing default.",
                          s);
            /* this default is from the X11R5 font converter */
            font = XLoadQueryFont(dpy, "-*-*-*-R-*-*-*-120-*-*-*-*-ISO8859-1");

            /* if still no font, then we fail */
            if (!font) {
                XmtWarningMsg("XmtConvertStringToFontList", "nodefault",
                              "cannot load any default font.");
                goto fail;
            }
        }

        /* and use this font and tag to create or append to the font list */
        if (!fontlist)
            fontlist = XmFontListCreate(font, tag);
        else 
            fontlist = XmFontListAdd(fontlist, font, tag);

#endif

        /* finally, move on to the next entry, and start the loop over */
        s = next_entry;
    }


    if (fontlist) {
        /* FIXME */
        if (fontdebug) {
                XmFontContext context;
                XmFontListEntry entry;
                XtPointer f;
                XmFontType type;
                char *tag;
                XmFontListInitFontContext(&context, fontlist);
                do {
                        entry = XmFontListNextEntry(context);
                        f = XmFontListEntryGetFont(entry, &type);
                        tag = XmFontListEntryGetTag(entry);
                        if (type == XmFONT_IS_FONT) {
                                printf("XmFONT_IS_FONT: %s\n", tag ? tag : "no tag");
                        } else {        
                                printf("XmFONT_IS_FONTSET: %s\n", tag ? tag : "no tag");
                                printf("BaseFonts: %s\n", f ? XBaseFontNameListOfFontSet((XFontSet)f): "--none--");
                        }        
                        XtFree(tag);
                                
                }while (entry);        
                        
                XmFontListFreeFontContext(context);
        }
        
        /* FIXME */
        XtFree(copy);
        done(XmFontList, fontlist);  /* this macro returns */
    }

 fail:
    XtDisplayStringConversionWarning(dpy, copy, XmRFontList);
    XtFree(copy);
    return False;
}


/* ARGSUSED */
#if NeedFunctionPrototypes
static void FreeConvertedXmFontList(XtAppContext app, XrmValue *to,
                                    XtPointer closure,
                                    XrmValue *args, Cardinal *num_args)
#else
static void FreeConvertedXmFontList(app, to, closure, args, num_args)
XtAppContext app;
XrmValue *to;
XtPointer closure;
XrmValue *args;
Cardinal *num_args;
#endif
{
    XmFontList fontlist = *((XmFontList *) to->addr);
    Screen *screen = *(Screen **)args[0].addr;
    Display *dpy = DisplayOfScreen(screen);
    XmFontContext context;
    XFontStruct *font;
    XmStringCharSet tag;
    
    if (XmFontListInitFontContext(&context, fontlist)) {
        while(XmFontListGetNextFont(context, &tag, &font))
            XFreeFont(dpy, font);
        XmFontListFreeFontContext(context);
    }
    XmFontListFree(fontlist);
}

#if NeedFunctionPrototypes
void XmtRegisterXmFontListConverter(void)
#else
void XmtRegisterXmFontListConverter()
#endif
{
    static Boolean registered = False;

    if (!registered) {
        XtSetTypeConverter(XtRString, XmRFontList,
                           XmtConvertStringToXmFontList,
                           (XtConvertArgRec *)screenConvertArg, (Cardinal) 1,
                           XtCacheByDisplay,
                           FreeConvertedXmFontList);
        registered = True;
    }
}


