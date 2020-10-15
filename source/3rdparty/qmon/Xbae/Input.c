/*
 * Copyright(c) 1999 Andrew Lister
 *
 *                        All rights reserved
 * Permission to use, copy, modify and distribute this material for
 * any purpose and without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies, and that the name of the author not be used in advertising
 * or publicity pertaining to this material without the specific,
 * prior written permission of the author.
 *
 * THE AUTHOR MAKES NO REPRESENTATIONS AND EXTENDS NO WARRANTIES, EX-
 * PRESS OR IMPLIED, WITH RESPECT TO THE SOFTWARE, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR ANY PARTICULAR PURPOSE, AND THE WARRANTY AGAINST IN-
 * FRINGEMENT OF PATENTS OR OTHER INTELLECTUAL PROPERTY RIGHTS.  THE
 * SOFTWARE IS PROVIDED "AS IS", AND IN NO EVENT SHALL THE AUTHOR OR
 * ANY OF ITS AFFILIATES BE LIABLE FOR ANY DAMAGES, INCLUDING ANY
 * LOST PROFITS OR OTHER INCIDENTAL OR CONSEQUENTIAL DAMAGES RELAT-
 * ING TO THE SOFTWARE.
 *
 * Author: Andrew Lister
 *
 * $Id: Input.c,v 1.2 2002-04-18 16:31:29 svdavidson Exp $
 */

#include <stdio.h>

/*#define DEBUG*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* For memmove/bcopy */
#include <string.h>

#include <ctype.h>
#include "InputP.h"
#if XmVersion <= 1001
#include <Xm/XmP.h>
#else
#include <Xm/DrawP.h>
#endif

#if !defined(HAVE_MEMMOVE) && !defined(XBAE_NEED_BCOPY)
#define XBAE_NEED_BCOPY
#endif

#if !defined(XBAE_NEED_BCOPY) || defined(SVR4) || defined(VMS) || defined(__EMX__)
#define BCOPY(src, dest, n)     memmove((void *)(dest), (void *)(src), (n))
#else
#define BCOPY(src, dest, n)     bcopy((void *)(src), (void *)(dest), (n))
#endif

#define CHAR_ALPHA                'a' /* Alphabetic characters only */
#define CHAR_BOTH                'b' /* Both - either character or digit */
#define CHAR_CHARACTER                'c' /* Any character */
#define CHAR_DIGIT                'd' /* Digits only */
#define CHAR_SOPTION                '[' /* Start optional sequence */
#define CHAR_EOPTION                ']' /* End optional sequence */
#define CHAR_UPPER                'U' /* Uppercase character */
#define CHAR_LOWER                'L' /* Lowercase character */
#define CHAR_ESCAPE                '\\' /* Escape character */
#define CHAR_OR                        '|' /* separator between optional chars */

#define IS_LITERAL(c)                (((c) != CHAR_ALPHA && \
                                  (c) != CHAR_BOTH && \
                                  (c) != CHAR_CHARACTER && \
                                  (c) != CHAR_DIGIT && \
                                  (c) != CHAR_SOPTION && \
                                  (c) != CHAR_EOPTION && \
                                  (c) != CHAR_UPPER && \
                                  (c) != CHAR_LOWER && \
                                  (c) != CHAR_OR) || \
                                  (c) == CHAR_ESCAPE)
                                 
/* Declaration of Actions */

/* ARGSUSED */
static void ToggleOverwriteACT P((Widget, XEvent *,
                                  String *, Cardinal *nparams));

static void validate P((Widget, XtPointer, XtPointer));

/* Declaration of Methods */
static void Initialize P((Widget, Widget, ArgList, Cardinal *));
static Boolean SetValues P((Widget, Widget, Widget, ArgList, Cardinal *));
static void Destroy P((Widget));

/* Utility routines */
static void parsePattern P((XbaeInputWidget, String));
static int match P((XbaeInputWidget, char *, int));

/* Internal callbacks */
static void checkInput P((Widget, XtPointer, XtPointer));

static char defaultTranslations[] =
"<Key>osfInsert : ToggleOverwrite()\n\
<Key>KP_Insert : ToggleOverwrite()";

static XtActionsRec actions[] =
{
    {"ToggleOverwrite", ToggleOverwriteACT},
};

#define offset(field)        XtOffsetOf(XbaeInputRec, field)

static XtResource resources[] = {
    { XmNalignment, XmCAlignment, XmRAlignment, sizeof(unsigned char),
      offset(input.alignment), XmRImmediate,
      (XtPointer)XmALIGNMENT_BEGINNING },
    { XmNautoFill, XmCBoolean, XmRBoolean, sizeof(Boolean),
      offset(input.auto_fill), XmRImmediate, (XtPointer)False },
    { XmNconvertCase, XmCBoolean, XmRBoolean, sizeof(Boolean),
      offset(input.convert_case), XmRImmediate, (XtPointer)True },
    { XmNoverwriteMode, XmCBoolean, XmRBoolean, sizeof(Boolean),
      offset(input.overwrite_mode), XmRImmediate, (XtPointer)False },
    { XmNpattern, XmCString, XmRString, sizeof(String),
      offset(input.pattern), XmRImmediate, (XtPointer)NULL },
    { XmNvalidateCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
      offset(input.validate_callback), XmRCallback, (XtPointer)NULL }
};

#if XmVersion > 1001
static XmPrimitiveClassExtRec primitiveClassExtRec = {
    NULL,                                /* next_extension        */
    NULLQUARK,                                /* record_type                */
    XmPrimitiveClassExtVersion,                /* version                */
    sizeof(XmPrimitiveClassExtRec),        /* record_size                */
    XmInheritBaselineProc,                /* widget_baseline        */
    XmInheritDisplayRectProc,                /* widget_display_rect        */
    (XmWidgetMarginsProc)_XtInherit        /* widget_margins        */
};
#endif

XbaeInputClassRec xbaeInputClassRec =
{
    { /* core_class fields         */
         (WidgetClass) &xmTextClassRec,        /* superclass                */
         "XbaeInput",                        /* class_name                */
         sizeof(XbaeInputRec),                /* widget_size               */
         NULL,                                /* class_initialize        */
         NULL,                                /* class_part_initialize*/
         False,                                /* class_inited                */
         (XtInitProc)Initialize,                /* initialize                */
         NULL,                                /* initialize_hook        */
         XtInheritRealize,                /* realize                   */
        actions,                        /* actions                   */
         XtNumber(actions),                /* num_actions                */
         resources,                        /* resources                */
         XtNumber(resources),                /* num_resources        */
         NULLQUARK,                        /* xrm_class                */
         True,                                /* compress_motion        */
         XtExposeCompressMaximal,        /* compress_exposure        */
         True,                                /* compress_enterleave        */
         False,                                /* visible_interest        */
         (XtWidgetProc)Destroy,                /* destroy                */
         XtInheritResize,                /* resize                */
         XtInheritExpose,                /* expose                */
         (XtSetValuesFunc)SetValues,        /* set_values                */
         NULL,                                /* set_values_hook        */
         XtInheritSetValuesAlmost,        /* set_values_almost        */
         NULL,                                /* get_values_hook        */
         XtInheritAcceptFocus,                /* accept_focus                */
         XtVersion,                        /* version                */
         NULL,                                /* callback_private        */
         XtInheritTranslations,                /* tm_table                */
         XtInheritQueryGeometry,                /* query_geometry        */
         XtInheritDisplayAccelerator,        /* display_accelerator        */
         NULL                                /* extension                */
    },
    { /* Primitive class         */
        XmInheritBorderHighlight,        /* border_highlight        */
        XmInheritBorderUnhighlight,        /* border_unhighlight        */
        defaultTranslations,                /* translations                */
        NULL,                                /* arm_and_activate        */
        NULL,                                /* syn resources        */
        0,                                /* num_syn_resources        */
#if XmVersion <= 1001
        NULL,                                /* extension                */
#else
        &primitiveClassExtRec,                /* extension                */
#endif
    },
    { /* Text class */
        NULL,                                /* extension                */
    },
    { /* Input class */
        NULL,                                /* extension                */
    }
};

WidgetClass xbaeInputWidgetClass = (WidgetClass) &xbaeInputClassRec;

/*
 * Action routines
 */

/*ARGSUSED*/
static void
validate(w, cd, cb)
Widget w;
XtPointer cd;
XtPointer cb;
{
    XmAnyCallbackStruct *cbs = (XmAnyCallbackStruct *)cb;
    XmTextVerifyCallbackStruct *vcbs = (XmTextVerifyCallbackStruct *)cb;
    XbaeInputWidget iw = (XbaeInputWidget)w;
    XbaeInputValidateCallbackStruct call_data;
    String pattern = NULL;
    String value;
    int ret;

    if (!iw->input.pattern)
        return;

    value = XmTextGetString(w);

    ret = match(iw, value, (int)True);

    if (iw->input.auto_fill && iw->input.literal_pending)
    {
        /*
         * Fill in any literals at the end of the string
         */
        int i, j = strlen(value);
        
        value = XtRealloc(value, (Cardinal)(strlen(value) + 1 +
                                            iw->input.literal_count));

        for (i = 0; i < iw->input.literal_count; i++)
        {
            if (*iw->input.literal_pending == CHAR_ESCAPE)
                iw->input.literal_pending++;

            value[i + j] = *iw->input.literal_pending;
            iw->input.literal_pending++;
        }
        value[i + j] = '\0';
        XmTextSetString(w, value);
    }

    /*
     * Check that the string matches exactly with the pattern
     */
    call_data.doit = ret;

    if (iw->input.validate_callback)
    {
        if (iw->input.pattern)
            pattern = XtNewString(iw->input.pattern);
        else
            pattern = NULL;

        call_data.reason = cbs->reason;
        call_data.event = cbs->event;
        call_data.pattern = pattern;
        call_data.value = value;

        XtCallCallbackList(w, iw->input.validate_callback,
                           (XtPointer)&call_data);
    }

    if (cbs->reason == XmCR_LOSING_FOCUS)
        vcbs->doit = call_data.doit;

    if (!call_data.doit)
    {
        XBell(XtDisplay(w), 0);
        XmProcessTraversal(w, XmTRAVERSE_CURRENT);

        if (cbs->reason == XmCR_ACTIVATE)
            XmProcessTraversal(w, XmTRAVERSE_CURRENT);
        else                        /* XmCR_LOSING_FOCUS */
        {
            /*
             * Should handle what's required - ringing the bell, preventing
             * traversal, etc by setting the XmTextVerifyCallbackStruct's
             * doit flag
             */
            vcbs->doit = call_data.doit;
        }
    }
    else if (iw->input.validate_callback && call_data.value != value)
        /* changed in callback */
    {
        XtFree(value);
        value = call_data.value;
        XmTextSetString(w, value);
    }

    if (pattern)
        XtFree(pattern);
    if (value)
        XtFree(value);
}

/* ARGSUSED */
static void
ToggleOverwriteACT(w, event, params, nparams)
Widget w;
XEvent *event;
String *params;
Cardinal *nparams;
{
    XbaeInputWidget iw = (XbaeInputWidget)w;

#ifdef DEBUG
    printf ("Toggle\n");
#endif

    if (iw->input.overwrite_mode)
        iw->input.overwrite_mode = False;
    else
        iw->input.overwrite_mode = True;

    XtCallActionProc(w, "toggle-overstrike", NULL, NULL, 0);    
}


static void
Initialize(request, new, args, num_args)
Widget request, new;
ArgList args;
Cardinal *num_args;
{
    XbaeInputWidget iw = (XbaeInputWidget)new;

    iw->input.literal_pending = NULL;
    iw->input.literal_count = 0;
    iw->input.last_insert = 0;
    /*
     * Check for a pattern associated with the text field
     */
    if (iw->input.pattern)
    {
        iw->input.pattern = XtNewString(iw->input.pattern);
        parsePattern(iw, iw->input.pattern);
        XtAddCallback(new, XmNmodifyVerifyCallback, checkInput, NULL);
    }
    else
    {
        iw->input.pattern_length = 0;
        iw->input.pattern = NULL;
    }

#if 0
    XtAugmentTranslations(new, XtParseTranslationTable
                          (defaultTranslations));
    XtVaSetValues(((XmTextRec *)iw)->text,
                  XmNtranslations, default_text_translations,
                  NULL);
#endif
    /*
     * By default, overwrite mode is off on the text field.  By calling
     * the text widget's action routine it will be turned on.
     */
    if (iw->input.overwrite_mode == True)
        XtCallActionProc(new, "toggle-overstrike", NULL, NULL, 0);

    XtAddCallback(new, XmNlosingFocusCallback, validate, NULL);
    XtAddCallback(new, XmNactivateCallback, validate, NULL);
}

static Boolean
SetValues(current, request, new, args, num_args)
Widget current, request, new;
ArgList args;
Cardinal *num_args;
{
    XbaeInputWidget c = (XbaeInputWidget)current;
    XbaeInputWidget n = (XbaeInputWidget)new;

#define current(field)        (c->input.field)
#define new(field)        (n->input.field)
    
#define NE(field)        (current(field) != new(field))
#define EQ(field)        (current(field) == new(field))

    if (NE(pattern))
    {
        if (new(pattern) == NULL) 
            XtRemoveCallback(new, XmNmodifyVerifyCallback, checkInput, NULL);
        else
            new(pattern) = XtNewString(new(pattern));

        if (current(pattern) == NULL)
            XtAddCallback(new, XmNmodifyVerifyCallback, checkInput, NULL);
        else
            XtFree(current(pattern));
        
        parsePattern(n, new(pattern));
    }
    

    if (NE(overwrite_mode))
        XtCallActionProc(new, "toggle-overstrike", NULL, NULL, 0);        

    if (NE(auto_fill))
    {
        /*
         * Call match to set any pending literals.  The string up to the
         * insertion position is what needs to be checked.
         */
        char *text;
        XmTextPosition pos = XmTextGetCursorPosition(new);

        text = XmTextGetString(new);
        if (text && *text)
        {
            *(text + pos) = 0;
            (void)match(n, text, (int)False);
        }
        else
            parsePattern(n, new(pattern));

        XtFree(text);
    }
    return False;
#undef Current
#undef New
#undef NE
#undef EQ
}

static void
Destroy(w)
Widget w;
{
    XbaeInputWidget iw = (XbaeInputWidget)w;

    if (iw->input.pattern)
        XtFree(iw->input.pattern);
}

static void
parsePattern(iw, pattern)
XbaeInputWidget iw;
String pattern;
{
    char *ptr = pattern;
    Boolean optional = False;

    iw->input.pattern_length = 0;

    if (!ptr || !*ptr)
        return;
    
    while (*ptr)
    {
        if (*ptr == CHAR_SOPTION)
        {
            if (optional)
            {
                XtAppWarningMsg(
                    XtWidgetToApplicationContext((Widget) iw),
                    "pattern", "badPattern", "XbaeInput",
                    "XbaeInput: Nested optionals in pattern",
                    (String *) NULL, (Cardinal *) NULL);
                break;
            }
                
            /*
             * If there is an optional character, then that character
             * does not need to be counted as part of the length
             */
            optional = True;
        }
        else if (*ptr == CHAR_EOPTION)
        {
            if (!optional)
            {
                XtAppWarningMsg(
                    XtWidgetToApplicationContext((Widget) iw),
                    "pattern", "badPattern", "XbaeInput",
                    "XbaeInput: Error in pattern",
                    (String *) NULL, (Cardinal *) NULL);
                break;
            }            
            optional = False;
        }
        else
        {
            /*
             * If the first character is a literal, set literal_pending
             * to True
             */
            if (iw->text.cursor_position == 0 &&
                       iw->input.pattern_length == 0 && IS_LITERAL(*ptr))
            {
                char *orig = ptr;

                if (*ptr == CHAR_ESCAPE)
                    ptr++;

                iw->input.literal_pending = ptr;
                iw->input.literal_count = 1;

                ptr++;

                while (*ptr && IS_LITERAL(*ptr))
                {
                    if (*ptr == CHAR_ESCAPE)
                        ptr++;
                    iw->input.literal_count++;
                    ptr++;
                }
                ptr = orig;
            }
            iw->input.pattern_length++;
        }
        ptr++;
    }
    XtVaSetValues((Widget)iw, XmNmaxLength, iw->input.pattern_length, NULL);
}

static int
match(iw, string, exact)
XbaeInputWidget iw;
char *string;
int exact;
{
    char *tptr, *sptr, *ntptr;
    char *lastopt = NULL;        /* last optional block */
    char *lastreq = NULL;        /* last required block */
    int optional = 0;
    /*int pend_rew = 0;*/
    int equal = 1;
    int escaped;
    int set_lastopt = 0;

    tptr = iw->input.pattern;
    sptr = string;

#ifdef DEBUG    
    printf("checking '%s' against '%s'\n", string ? string : "(null)",
           iw->input.pattern ? iw->input.pattern : "(null)");
#endif

    if (!tptr || !sptr || !*tptr || !*sptr)
    {
        /* check for leading literals */
        if (tptr)
        {
            if (IS_LITERAL(*tptr))
            {
                if (*tptr == CHAR_ESCAPE)
                    tptr++;
                iw->input.literal_pending = tptr++;
                iw->input.literal_count = 1;
                while (*tptr && IS_LITERAL(*tptr))
                {
                    if (*tptr == CHAR_ESCAPE)
                        tptr++;
                    iw->input.literal_count++;
                    tptr++;
                }
            }
        }
        /*
         * No pattern or no string - must match!
         */
        return True;
    }

    while (*sptr && *tptr)
    {
        escaped = 0;
        do
        {
            ntptr = tptr + 1;        /* next tptr */
            switch (*tptr)
            {
                case CHAR_SOPTION:        /* start of optional sequence */
                    optional = 1;
                    /*if (!lastopt)*/        /* XXX -cg */
                    /*
                       Trouble is with this removed we don't rewind far enough
                       back should another thing fail further along.  Also
                       presents problems for autofill since the position of
                       the filled chars may move!  Need either another loop
                       which rewinds or a stack of lastopt pointers which can
                       be popped. Thus the beauty of recursion.
                     */
                    /*lastopt = sptr;*/
                    lastopt = NULL;
                    set_lastopt = 1;
                    break;
                case CHAR_EOPTION:        /* start of optional sequence */
                    optional = 0;
                    if (*(tptr + 1) != CHAR_SOPTION)
                        lastreq = tptr + 1;
                    set_lastopt = 0;
                    break;
                case CHAR_ALPHA:        /* alphabetic */
                    equal = (int)isalpha(*sptr);
                    if (equal && set_lastopt)
                        lastopt = sptr;
                    set_lastopt = 0;
                    break;
                case CHAR_BOTH:                /* both: alphabetic or digit */
                    equal = (int)isalnum(*sptr);
                    if (equal && set_lastopt)
                        lastopt = sptr;
                    set_lastopt = 0;
                    break;
                case CHAR_CHARACTER:        /* any character */
                    equal = True;
                    if (equal && set_lastopt)
                        lastopt = sptr;
                    set_lastopt = 0;
                    break;
                case CHAR_DIGIT:        /* any digit */
                    equal = (int)isdigit(*sptr);
                    if (equal && set_lastopt)
                        lastopt = sptr;
                    set_lastopt = 0;
                    break;
                case CHAR_UPPER:
                    if (iw->input.convert_case && islower(*sptr))
                        *sptr = toupper(*sptr);
                    equal = (int)(isalpha(*sptr) && isupper(*sptr));
                    if (equal && set_lastopt)
                        lastopt = sptr;
                    set_lastopt = 0;
                    break;
                case CHAR_LOWER:
                    if (iw->input.convert_case && isupper(*sptr))
                        *sptr = tolower(*sptr);
                    equal = (int)(isalpha(*sptr) && islower(*sptr));
                    if (equal && set_lastopt)
                        lastopt = sptr;
                    set_lastopt = 0;
                    break;
                case CHAR_OR:
                    /*
                       If tptr is | then we already had success on the previous
                       char so skip it and then next.
                    */
                    tptr = ntptr + 1;
                    equal = 0;
                    continue;
                case CHAR_ESCAPE:
                    tptr++;
                    ntptr++;
                    escaped = 1;
                    /* And fall through to literal */
                default:                /* treat as a literal */
                    equal = (int)(*sptr == *tptr);
                    if (equal && set_lastopt)
                        lastopt = sptr;
                    set_lastopt = 0;
                    break;
            }
        } while (!equal && *ntptr == CHAR_OR && (tptr = ntptr + 1));
        if (!lastreq && !optional)
            lastreq = tptr;
        if (!equal)
        {
#ifdef DEBUG
            int z, w;
            char *z1;
            puts("string failed at:");
            for (z1 = string, z = 0; *z1; z1++, z++)
            {
                if (z1 == sptr)
                    w = z;
                putchar(*z1);
            }
            putchar('\n');
            for (z = 0; z < w; z++)
                putchar(' ');
            puts("^");
            puts("pattern was at:");
            for (z1 = iw->input.pattern, z = 0; *z1; z1++, z++)
            {
                if (z1 == tptr)
                    w = z;
                putchar(*z1);
            }
            putchar('\n');
            for (z = 0; z < w; z++)
                putchar(' ');
            puts("^");
            printf("opt %d, lastopt %s\n",
                   optional, lastopt ? lastopt : "(null)");
#endif
            if (optional)
            {
                /*
                 * Discard the optional part and move forward to the
                 * required bit, if there is one - if there isn't one
                 * it is a failure
                 */
                while (*tptr &&
                       (*tptr != CHAR_EOPTION || *(tptr-1) == CHAR_ESCAPE))
                    tptr++;
                if (!*tptr)
                    (void)fprintf(stderr, "Mismatched parentheses\n");
                else
                {
                    tptr++;        /* move past the ']' */
#ifdef OLD
                    if (!lastopt)
                    {
                        equal = False;
                        break;
                    }
#endif
                    /* rewind if necessary */
                    if (lastopt)
                    {
                        /*if (pend_rew)*/
                        {
#ifdef DEBUG
                            int z, w, w1;
                            char *z1;
                            puts("Rewinding:");
                            for (z1 = string, z = 0; *z1; z1++, z++)
                            {
                                if (z1 == sptr)
                                    w = z;
                                if (z1 == lastopt)
                                    w1 = z;
                                putchar(*z1);
                            }
                            putchar('\n');
                            for (z = 0; z < w; z++)
                            {
                                if (z == w1)
                                    putchar('^');
                                else
                                    putchar(' ');
                            }
                            puts("<");
#endif
                            sptr = lastopt;
                            lastopt = NULL;
                            /*pend_rew = 0;*/
                        }
                        /*
                        else
                        {
#ifdef DEBUG
                            puts("rewind pending...");
#endif
                            pend_rew = 1;
                        }
                        */
                    }
                    equal = True; /* hope for the best! */
/*                    if (*tptr == CHAR_SOPTION)
                        lastopt = tptr;
                        else*/
                }
                optional = 0;
            }
            /*
            else if (pend_rew)
            {
#ifdef DEBUG
                int z, w, w1;
                char *z1;
                puts("Rewinding:");
                for (z1 = string, z = 0; *z1; z1++, z++)
                {
                    if (z1 == sptr)
                        w = z;
                    if (z1 == lastopt)
                        w1 = z;
                    putchar(*z1);
                }
                putchar('\n');
                for (z = 0; z < w; z++)
                {
                    if (z == w1)
                        putchar('^');
                    else
                        putchar(' ');
                }
                puts("<");
#endif
                sptr = lastopt;
                lastopt = NULL;
                pend_rew = 0;
            }
        */
            else
            {
                /*
                 * Failed in a required part.  Here, if there was a
                 * previous optional part, that needs to be thrown away
                 * and the section checked again
                 */
                if (lastreq && lastopt)
                {
                    sptr = lastopt;
                    tptr = lastreq;
#ifdef DEBUG
#if 0
                    m = mmatch;
                    s = smatch;
#endif
#endif
                    lastopt = NULL;
                }
                else
                {
                    equal = False;
                    break;
                }
            }
            continue;
        }
        if (escaped || (*tptr != CHAR_EOPTION && *tptr != CHAR_SOPTION))
        {
#ifdef DEBUG
            printf("%.*s matches %.*s\n", sptr - string +1, string /*strbuf*/,
                   tptr - iw->input.pattern +1, iw->input.pattern/*matchbuf*/);
#if 0
            s++;
#endif
#endif
            sptr++;
        }
        tptr++;
#ifdef DEBUG
#if 0
        m++;
#endif
#endif
    }

    if (optional && *tptr && *tptr == CHAR_EOPTION) /* step over last ']' */
        tptr++;

    /*
     * Determine whether any (and how many) literals are about to be inserted
     */
    iw->input.literal_count = 0;
    iw->input.literal_pending = NULL;

    while (*tptr && IS_LITERAL(*tptr) && equal)
    {
        if (*tptr == CHAR_ESCAPE)
            tptr++;

        if (iw->input.literal_count == 0)
            iw->input.literal_pending = tptr;
        
        iw->input.literal_count++;
        tptr++;
    }

    /*
     * Ignore any trailing optional bits
     */
    while (*tptr && *tptr == CHAR_SOPTION)
    {
        while (*tptr != CHAR_EOPTION)
            tptr++;
        tptr++;
    }
    /*
       XXX Might have to skip literals again here for final valid check, not
       to set literal_pending/literal_count though, unless we want to fill
       all trailing literals upon final validation, even if they span an
       optional bit.
    */
        
    if (*sptr && !*tptr)
    {
#ifdef DEBUG
        printf("Couldn't fit all chars into pattern. Sorry about that\n");
#endif
        equal = False;
    }

    if (*tptr && !*sptr && exact)
        equal = False;

#ifdef DEBUG
    printf("----\n");
#endif

    return equal;
}

/*ARGSUSED*/
static void
checkInput(w, cd, cb)
Widget w;
XtPointer cd;
XtPointer cb;
{
    XbaeInputWidget iw = (XbaeInputWidget)w;
    XmTextVerifyCallbackStruct *cbs = (XmTextVerifyCallbackStruct *)cb;
    int i, j, pos;
    int length;
    char *text;
    
    /*
     * Ignore events generated by convenience routines
     */
    if (!cbs->event)
        return;
    /*
     * Was it a backspace?
     */
    if (cbs->startPos < cbs->currInsert || cbs->text->length == 0 ||
        !cbs->text->ptr || *cbs->text->ptr == 0)
    {
        /*
         * Allow a backspace or delete to do it's bit.
         */        
        return;
    }
    
    /*
     * This is pretty lame but Motif doesn't seem to call the modifyVerifyCB
     * if the text is pasted from the clipboard.  The logic to insert
     * characters below should handle it if pasting worked correctly.
     */
    if (cbs->text->length > 1)
    {
        cbs->doit = False;
        return;
    }

    /*
     * Has the length exceeded the pattern?
     */
    if (cbs->text->length + cbs->startPos > iw->input.pattern_length)
    {
        cbs->doit = False;
        return;
    }

    /*
     * Otherwise, something was inserted...
     */

    /*
     * Construct what the string looks like at the moment (+1 for NUL char)
     */
    text = XmTextGetString(w);    
    pos = cbs->startPos;

    /*
     * Copy the characters forward if inserting or at end of string.
     * Include the NUL terminator in the move.
     */
    if (pos >= (length = strlen(text)) || iw->input.overwrite_mode == False)
    {
        /*
         * The new length will be the current length of the string + 1 for
         * a NUL + the length of the newly inserted string
         */
        length += 1 + cbs->text->length;
        text = XtRealloc((void *)text, length);
        for (j = length - 1; j > pos; j--)
            text[j] = text[j - 1];
    }

    /*
     * If the insertion point has changed or we are overwriting chars,
     * any pending literals may also have changed
     */
    if (iw->input.last_insert != pos ||
        (pos < strlen(text) && iw->input.overwrite_mode == True))
    {
        char *buf = XtNewString(text);
        buf[pos] = 0;
        (void)match(iw, buf, (int)False);
        XtFree(buf);
    }

    /*
     * The string needs to reflect what the string will look like
     * after the text has been inserted so a bit of mucking about is
     * needed to get that
     */
    for (i = 0; i < cbs->text->length; i++, pos++)
    {        
        if (iw->input.auto_fill && iw->input.literal_pending &&
            cbs->text->ptr[i] != *iw->input.literal_pending)
        {
            int k;

            for (k = 0; k < iw->input.literal_count; k++)
            {
                /*
                 * If in insert mode or at the end of the string,
                 * extra space needs to be added
                 */
                if (!iw->input.overwrite_mode || pos >= strlen(text))
                {
                    length++;
                    text = XtRealloc((void *)text, length);
                    for (j = length - 1; j > pos; j--)
                        text[j] = text[j - 1];
                }
                /*
                 * The callback's ptr will always need to be changed though
                 */
                cbs->text->length++;
                cbs->text->ptr = XtRealloc((void *)cbs->text->ptr,
                                           cbs->text->length + 1);
                for (j = cbs->text->length; j > i; j--)
                    cbs->text->ptr[j] = cbs->text->ptr[j - 1];

                if (iw->input.overwrite_mode)
                    cbs->endPos++;
/*            (void)BCOPY(cbs->text->ptr + i, cbs->text->ptr + i + 1,
            cbs->text->length - i);*/
                
                cbs->text->ptr[i] = *iw->input.literal_pending;
                text[pos] = *iw->input.literal_pending;
                pos++;
                i++;
                iw->input.literal_pending++;
                if (*iw->input.literal_pending == CHAR_ESCAPE)
                    iw->input.literal_pending++;
            }
        }

        text[pos] = cbs->text->ptr[i];

        if (match(iw, text, (int)False) == 0)
        {
            cbs->doit = False;
            XtFree(text);
            return;
        }
        /*
         * match() may change the current character so it needs to
         * be copied back
         */
        cbs->text->ptr[i] = text[pos];
    }
    iw->input.last_insert = pos;

    XtFree(text);
}

Widget
XbaeCreateInput(parent, name, args, ac) 
Widget parent;
String name;
ArgList args;
Cardinal ac;
{ 
    return XtCreateWidget(name, xbaeInputWidgetClass, parent, args, ac); 
} 
