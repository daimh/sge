/* 
 * Motif Tools Library, Version 3.1
 * $Id: HelpNode.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: HelpNode.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#ifdef VMS
#include <perror.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>    
#include <X11/Xos.h>
#include <Xmt/Xmt.h>
#include <Xmt/Help.h>
#include <Xmt/Hash.h>
#include <Xmt/AppResP.h>

#if NeedFunctionPrototypes
void XmtHelpDefineNode(Widget w, StringConst name, XmtHelpNode *node)
#else
void XmtHelpDefineNode(w, name, node)
Widget w;
StringConst name;
XmtHelpNode *node;
#endif
{
    XmtAppResources *app = XmtGetApplicationResources(w);

    if (app->help_node_table == NULL)
	app->help_node_table = XmtHashTableCreate(5);
	
    XmtHashTableStore(app->help_node_table,
		      (XtPointer)XrmStringToQuark(name),
		      (XtPointer)node);
}
    

/*
 * Parse a help file, defining any nodes we find
 * The node syntax, sort of.  Keywords begin with a
 * '.' as the first character on a line.
 * 
 * help-file:: {{include} node}
 * include:: <.Include line>
 * node:: <.Node line> {attributes} body
 * attributes: .Title | .Keywords | .Crossrefs | .Subsections | .Format
 * body:: .Filename | .Body lines .End
 *
 *   .Include <filename>  [parse some other file in the same way]
 *   .Node <nodename>
 *   .Title <node-title>
 *   .Keywords <list-of-keywords>
 *   .Crossrefs <list-of-crossrefs>
 *   .Subsections <list-of-subnodes>
 *   .Format <format> [Text and XmString are currently supported formats]
 *   .Filename <filename> [optional straight text file, instead of .Body/.End]
 *   .Body
 *   <any number of lines of text>
 *   .End
 *
 * A help node begins with .Node, has any attributes in any order, and
 * ends with either a .File line, or a .Body line followed by the body
 * and a .End line.
 */

#define LINELEN 512

#if NeedFunctionPrototypes
static void ParseStringList(String line, String** list_return,
			    short *len_return)
#else
static void ParseStringList(line, list_return, len_return)
String line;
String **list_return;
short int *len_return;
#endif
{
    String *list = NULL;
    short num = 0;
    short max = 0;
    register char *p, *q;
    int len;
    String word;

    p = line;
    while(*p) {
	/* skip leading whitespace */
	while(isspace(*p)) p++;
	if (!*p) break;

	/* we're at the begining of a word.  Go find comma or EOL or EOS */
	for(q = p; *q && (*q != '\n') && (*q != ','); q++);

	/* make more space in the list, if needed */
	if (num == max) {
	    max += 5;
	    list = (String *) XtRealloc((char *)list, max * sizeof(String));
	}

	/* copy the word from the line */
	len = q - p;
	word = strncpy(XtMalloc(len+1), p, len);
	word[len] = '\0';
	list[num++] = word;

	/* if we're at a comma or newline, skip over it */
	if ((*q == ',') || (*q == '\n')) q++;

	p = q;
    }

    /* free up any excess allocated memory */
    if (num < max) list = (String *) XtRealloc((char *)list,
					       num*sizeof(String));
    
    /* and return the list and list length */
    *list_return = list;
    *len_return = num;
}

#if NeedFunctionPrototypes
static void FreeStringList(String *list, short num)
#else
static void FreeStringList(list, num)
String *list;
short int num;
#endif
{
    int i;

    for(i=0; i < num; i++) XtFree(list[i]);
    XtFree((char *)list);
}

#if NeedFunctionPrototypes
static XmtHelpNode *ParseNode(FILE *f)
#else
static XmtHelpNode *ParseNode(f)
FILE *f;
#endif
{
    XmtHelpNode *node;
    char line[LINELEN];
    char buf[LINELEN];
    char *s;
    int len;
    
    /*
     * We've already read the .Node line, so read any number of
     * attributes, then look for a filename specified by .Filename,
     * or for the node body specified by a .Body/.End pair.  In this
     * latter case, just save the offset and the length of the text
     * to be read later when needed
     */

    /*
     * first, allocate and initialize the node
     */
    node = (XmtHelpNode *) XtCalloc(1, sizeof(XmtHelpNode));
    node->format = XmtHelpFormatString;

    /* parse any number of attribute lines */
    while(fgets(line, LINELEN-1, f)) {
	if (strncmp(line, ".Title", 6) == 0) {
	    s = &line[6];
	    while(isspace(*s)) s++;
	    len = strlen(s) - 1;  /* minus 1 to remove newline */
	    s[len] = '\0';
	    node->title = strcpy(XtMalloc(len+1), s);
	}
	else if (strncmp(line, ".Keywords", 9) == 0)
	    ParseStringList(&line[9], &node->keywords, &node->num_keywords);
	else if (strncmp(line, ".Crossrefs", 10) == 0)
	    ParseStringList(&line[10], &node->crossrefs, &node->num_crossrefs);
	else if (strncmp(line, ".Subsections", 12) == 0)
	    ParseStringList(&line[12], &node->subnodes, &node->num_subnodes);
	else if (strncmp(line, ".Format", 7) == 0) {
	    sscanf(&line[7], "%s", buf);
	    if (strncmp(buf, "XmString", 8) == 0)
		node->format = XmtHelpFormatXmString;
	}
	else
	   break;
    }

    /* Now look for a .Filename line or a .Body line */
    if (strncmp(line, ".Filename", 9) == 0) {
	sscanf(&line[9], "%s", buf);
	node->body = XtNewString(buf);
	node->is_filename = True;
	return node;
    }
    else if (strncmp(line, ".Body", 5) == 0) {
	node->file = f;
	node->offset = ftell(f);
	/* scan 'till .End */
	while(fgets(line, LINELEN-1, f)) {
	    if ((line[0] == '.') && (strncmp(line, ".End", 4) == 0)) break;
	}
	node->length = ftell(f) - strlen(line) - node->offset;
	return node;
    }
    else {  /* nothing found; an error */
	if (node->title) XtFree(node->title);
	if (node->keywords) FreeStringList(node->keywords, node->num_keywords);
	if (node->crossrefs) FreeStringList(node->crossrefs,
					    node->num_crossrefs);
	if (node->subnodes) FreeStringList(node->subnodes, node->num_subnodes);
	XtFree((char *)node);
	return NULL;
    }
}


#if NeedFunctionPrototypes
void XmtHelpParseFile(Widget w, StringConst filename)
#else
void XmtHelpParseFile(w, filename)
Widget w;
StringConst filename;
#endif
{
    XmtAppResources *app = XmtGetApplicationResources(w);
    String fullname;
    FILE *f;
    char line[LINELEN];
    char name[LINELEN];
    XmtHelpNode *node;


    fullname = XmtFindFile(w, "help", filename, ".txt",
			   NULL, app->help_file_path, XmtSearchEverywhere);

    if (!fullname || (f = fopen(fullname, "r")) == NULL) {
	XmtWarningMsg("XmtHelpParseFile", "notFound",
		      "can't find a readable help file named '%s'",
		      filename);
	XtFree(fullname);
	return;
    }

    XtFree(fullname);

    while(fgets(line, LINELEN-1, f)) {
	/*
	 * a help file is a sequence of include statements and node
	 * definitions.  Look for one or the other and discard anything else
	 */
	if (line[0] != '.') continue;
	if (strncmp(line, ".Include", 8) == 0) {
	    sscanf(&line[8], "%s", name);
	    XmtHelpParseFile(w, name);
	}
	else if (strncmp(line, ".Node", 5) == 0) {
	    /* a node definition */
	    sscanf(&line[5], "%s", name);
	    node = ParseNode(f);
	    if (node != NULL)
		XmtHelpDefineNode(w, name, node);
	    else { /* error */
		XmtWarningMsg("XmtHelpParseFile", "badNode",
			      "Node '%s':\n\tExpected .Filename or .Body following .Node",
			      name);
	    }
	}
    }
}

#if NeedFunctionPrototypes
XmtHelpNode *XmtHelpLookupNode(Widget w, StringConst name)
#else
XmtHelpNode *XmtHelpLookupNode(w, name)
Widget w;
StringConst name;
#endif
{
    XmtAppResources *app = XmtGetApplicationResources(w);
    XrmQuark q = XrmStringToQuark(name);
    XmtHelpNode *node;

    if (!app->help_file_loaded) {
	app->help_file_loaded = True;
	if (app->help_file) XmtHelpParseFile(w, app->help_file);
    }

    if (app->help_node_table == NULL) return NULL;
    
    if (XmtHashTableLookup(app->help_node_table,
			   (XtPointer)q, (XtPointer *)&node))
	return node;
    else
	return NULL;
}

#if NeedFunctionPrototypes
String XmtHelpNodeGetBody(XmtHelpNode *node)
#else
String XmtHelpNodeGetBody(node)
XmtHelpNode *node;
#endif
{
    if (node->body && !node->is_filename)
	return XtNewString(node->body);
    else if (node->body) { /* return the entire contents of the named file */
	FILE *f;
	struct stat stats;
	int size;
	String text;

	/* open the file */
	if ((f = fopen(node->body, "r")) == NULL) {
	    XmtWarningMsg("XmtHelpNodeGetBody", "fopen",
			  "Can't open file '%s': %s",
			  node->body, strerror(errno));
	    return NULL;
	}

	/* find out how long it is */
	if (fstat(fileno(f), &stats) == -1) {
	    XmtWarningMsg("XmtHelpNodeGetBody", "fstat",
			  "Can't stat file '%s': %s",
			  node->body, strerror(errno));
	    fclose(f);
	    return NULL;
	}

	size = (int) stats.st_size;
	text = (String) XtMalloc(size+1);
	if ((size = read(fileno(f), text, size)) == -1) {
	    /* I don't really think this will ever fail, so no warning msg */
	    XtFree(text);
	    text = NULL;
	}
	fclose(f);
	text[size] = '\0';
	return text;
    }
    else if (node->file) { /* return a portion of the already opened file */
	String text;
	int len;
	
	text = XtMalloc(node->length + 1);
	if (fseek(node->file, node->offset, 0) != 0) {
	    XmtWarningMsg("XmtHelpNodeGetBody", "fseek",
			  "fseek: %s", strerror(errno));
	    return NULL;
	}
	if ((len = fread(text, sizeof(char), node->length, node->file))==EOF) {
	    XmtWarningMsg("XmtHelpNodeGetBody", "fread",
			  "fread: %s", strerror(errno));
	    XtFree(text);
	    return NULL;
	}
	text[len] = '\0';
	return text;
    }
    else
	return NULL;
}
