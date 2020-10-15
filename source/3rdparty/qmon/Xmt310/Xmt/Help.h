/* 
 * Motif Tools Library, Version 3.1
 * $Id: Help.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Help.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtHelp_h
#define _XmtHelp_h

#include <stdio.h>

/*
 * Each piece of online help is an XmtHelpNode.  Each node has a name
 * and is stored by name in a hash table.  Help nodes can be used by
 * both the XmtHelpDialog context help widget, or the XmtHelpBrowser
 * widget.  The XmtHelpDialog just displays a single node, and the
 * XmtHelpBrowser widget organizes nodes into a hierarchial document.
 */
 
typedef struct _XmtHelpNode {
    String title;       /* the node title; the section name */
    String *keywords;   /* keywords for this node; used in index */
    String *crossrefs;  /* See also nodes */
    String *subnodes;   /* list of subsection nodes */
    short num_keywords;
    short num_crossrefs;
    short num_subnodes;
    String body;        /* literal text for the node... */
    FILE *file;         /*   or a file pointer to read the text from */
    long offset;        /*   and an offset to find the text at */
    int length;         /*   and a length of the text block */
    unsigned char format; /* how to display body, an XmtHelpFormat value */
    Boolean is_filename;/* if the body field is really a filename */
} XmtHelpNode;

/* defined help formats */
#define XmtHelpFormatString 1    /* plain text; display with XmText */
#define XmtHelpFormatXmString 2  /* an XmString; display with an XmLabel */

#define XmtHelpNodeGetTitle(node) node->title

/*
 * names and classes for the database resources we
 * read when looking up context help for a widget.
 */
#define XmtNxmtHelp		"xmtHelp"
#define XmtCXmtHelp		"XmtHelp"
#define XmtNxmtHelpTitle	"xmtHelpTitle"
#define XmtCXmtHelpTitle	"XmtHelpTitle"
#define XmtNxmtHelpNode		"xmtHelpNode"
#define XmtCXmtHelpNode		"XmtHelpNode"


_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void XmtHelpGetContextHelp(Widget, String *, String *);
extern void XmtHelpDisplayContextHelp(Widget);
extern void XmtHelpDoContextHelp(Widget);
extern void XmtHelpContextHelpCallback(Widget, XtPointer, XtPointer);
extern void XmtHelpInstallContextHelp(Widget, XtCallbackProc, XtPointer);
extern void XmtHelpDefineNode(Widget, StringConst, XmtHelpNode *);
extern XmtHelpNode *XmtHelpLookupNode(Widget, StringConst);
extern String XmtHelpNodeGetBody(XmtHelpNode *);
extern void XmtHelpParseFile(Widget, StringConst);
#else
extern void XmtHelpGetContextHelp();
extern void XmtHelpDisplayContextHelp();
extern void XmtHelpDoContextHelp();
extern void XmtHelpContextHelpCallback();
extern void XmtHelpInstallContextHelp();
extern void XmtHelpDefineNode();
extern XmtHelpNode *XmtHelpLookupNode();
extern String XmtHelpNodeGetBody();
extern void XmtHelpParseFile();
#endif
_XFUNCPROTOEND


#endif /* _XmtHelp_h */
