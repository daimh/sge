/* 
 * Motif Tools Library, Version 3.1
 * $Id: Include.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Include.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <stdio.h>
#include <errno.h>
#ifdef VMS
#include <perror.h>
#endif
#include <Xmt/Xmt.h>
#include <Xmt/Include.h>
#include <Xmt/Hash.h>
#include <Xmt/Lexer.h>
#include <Xmt/AppResP.h>
#include <X11/IntrinsicP.h>

#if NeedFunctionPrototypes
static void load_file_error(String filename)
#else
static void load_file_error(filename)
String filename;
#endif
{
    XmtWarningMsg("XmtLoadResourceFile", "io",
		  "can't load file '%s':\n\t%s.",
		  filename, strerror(errno));
}    


/* forward declaration */
#if NeedFunctionPrototypes
static void LoadResourceFileList(XrmDatabase *, Widget, StringConst);
#else
static void LoadResourceFileList();
#endif


#if NeedFunctionPrototypes
static Boolean LoadResourceFile(XrmDatabase *db, Widget w,StringConst filename,
				Boolean user, Boolean override)
#else
static Boolean LoadResourceFile(db, w, filename, user, override)
XrmDatabase *db;
Widget w;
StringConst filename;
Boolean user;
Boolean override;
#endif
{
    static XrmName required_names[2];
    static XrmClass required_classes[2];
    static Boolean required_quarks_inited = False;
    XmtAppResources *app = XmtGetApplicationResources(w);
    XrmRepresentation type;
    XrmValue value;
    XrmDatabase newdb = NULL;
    XrmDatabase userdb = NULL;
    String full_filename;
    String user_filename;
    Boolean found = False;
    XrmQuark fileq;
    XtPointer dummy = NULL;

    if (!filename || filename[0] == '\0') return False;
    
    /* find our application shell */
    w = XmtGetApplicationShell(w);

    /*
     * one time initialization
     */
    if (required_quarks_inited == False) {
	required_names[0] = XrmPermStringToQuark(XmtNxmtRequires);
	required_names[1] = NULLQUARK;
	required_classes[0] = XrmPermStringToQuark(XmtCXmtRequires);
	required_classes[1] = NULLQUARK;
	required_quarks_inited = True;
    }

    /* test that the file is not already included */
    fileq = XrmStringToQuark(filename);
    if (XmtHashTableLookup(app->loaded_files_table,
			   (XtPointer)fileq, &dummy))
	return True;
    
    /*
     * If the filename begins with / or ./ or ../, then we don't search for it.
     * Otherwise, we've got to deal with paths.
     */
    if ((filename[0] == '/') ||
	 ((filename[0] == '.') && (filename[1] == '/')) ||
	 ((filename[0]=='.') && (filename[1]=='.') && (filename[2]=='/'))) {
	found = True;
	newdb = XrmGetFileDatabase(filename);
	if (!newdb) load_file_error((String)filename);
    }
    else {
	/*
	 * go look for a application or system file by the specified name.
	 * This call searches resourceFilePath under the directory configDir.
	 * If resourceFilePath is undefined or if no file is found there, it
	 * searches configFilePath, if it differs from resourceFilePath, or
	 * if configFilePath is undefined it uses a default path under teh
	 * directory configDir.
	 * If still no file, it searches XFILESEARCHPATH or the /usr/lib/X11
	 * default path.
	 */
	full_filename = XmtFindFile(w, "app-defaults", filename,
				    APPDEFAULTSSUFFIX,
				    NULL, app->resource_file_path,
				    XmtSearchAppPath | XmtSearchSysPath);
	
	if (full_filename) {
	    newdb = XrmGetFileDatabase(full_filename);
	    if (!newdb) load_file_error(full_filename);
	    else found = True;
	    XtFree(full_filename);
	}

	/* if user is True, go look for a user resource file
	 * and override the application file with it.
	 */
	if (user) {
	    user_filename = XmtFindFile(w, "app-defaults", filename,
					APPDEFAULTSSUFFIX,
					NULL, NULL, XmtSearchUserPath);
	    if (user_filename) {
		userdb = XrmGetFileDatabase(user_filename);
		if (!userdb) load_file_error(user_filename);
		else {
		    XrmMergeDatabases(userdb, &newdb);
		    found = True;
		}
		XtFree(user_filename);
	    }
	}
    }

    /*
     * if no file was found, warn
     */
    if (!found)
	XmtWarningMsg("XmtLoadResourceFile", "notFound",
		      "file '%s' not found.", filename);

    /* if there's no db at this point, quit */
    if (!newdb) return False;

    /*
     * remember that we loaded this file
     */
    XmtHashTableStore(app->loaded_files_table, (XtPointer)fileq, (XtPointer)1);

    /*
     * check the new db for recursive include files
     */
    if (XrmQGetResource(newdb, required_names, required_classes,
			&type, &value) == True)
	LoadResourceFileList(db, w, value.addr);

    /*
     * Finally, augment or override the db with the new db
     * Note that this destroys newdb, so we don't have to do so explicitly.
     */
#ifndef X11R5
    if (!override)
	XmtWarningMsg("XmtLoadResourceFile", "cantAugment",
		      "the #augment directive only works with X11R5\n\tand later releases.  Overriding instead.");
    XrmMergeDatabases(newdb, db);
#else
    XrmCombineDatabase(newdb, db, override);
#endif

    return True;
}


static String keywords[] = {
    "augment",
    "override"
};

#define AUGMENT 0
#define OVERRIDE 1

#if NeedFunctionPrototypes
static void LoadResourceFileList(XrmDatabase *db, Widget w, StringConst list)
#else
static void LoadResourceFileList(db, w, list)
XrmDatabase *db;
Widget w;
StringConst list;
#endif
{
    XmtLexer l;
    XmtLexerToken tok;
    Boolean override = True;

    l = XmtLexerCreate(keywords, XtNumber(keywords));
    XmtLexerInit(l, list);

    /*
     * Parse the following grammar:
     * ((directive | file) [','])*
     * directive: ("#augment" | "#override")
     * file: STRING | '<'[^> \t]*'>'
     */
    while((tok = XmtLexerGetToken(l)) != XmtLexerEndOfString) {
	switch(tok) {
	case XmtLexerSharp:
	    if (XmtLexerNextToken(l) != XmtLexerKeyword) goto syntax;
	    if (XmtLexerKeyValue(l) == AUGMENT) override = False;
	    else override = True;
	    XmtLexerConsumeToken(l);
	    break;
	case XmtLexerString:
	    (void)LoadResourceFile(db, w, XmtLexerStrValue(l), True, override);
	    XtFree(XmtLexerStrValue(l));
	    if (XmtLexerNextToken(l) == XmtLexerComma) XmtLexerConsumeToken(l);
	    break;
	case XmtLexerLess:
	    XmtLexerConsumeToken(l);
	    XmtLexerSkipWhite(l);
	    if (XmtLexerScan(l, "> \t", False) != XmtLexerString) goto syntax;
	    (void)LoadResourceFile(db,w, XmtLexerStrValue(l), False, override);
	    XtFree(XmtLexerStrValue(l));
	    if (XmtLexerNextToken(l) != XmtLexerGreater) goto syntax;
	    if (XmtLexerNextToken(l) == XmtLexerComma) XmtLexerConsumeToken(l);
	    break;
	default:
	    goto syntax;
	}
    }
    XmtLexerDestroy(l);
    return;
    
 syntax:
    XmtWarningMsg("XmtLoadResourceFileList", "syntax",
		  "syntax error in xmtRequires resource:\n\t%s",
		  list);
    XmtLexerDestroy(l);
    return;
}

#if NeedFunctionPrototypes
Boolean XmtLoadResourceFile(Widget w, StringConst filename,
			    XmtWideBoolean user, XmtWideBoolean override)
#else
Boolean XmtLoadResourceFile(w, filename, user, override)
Widget w;
StringConst filename;
int user;
int override;
#endif
{
    XrmDatabase db;
    
    w = XmtGetApplicationShell(w); 
    db = XmtDatabaseOfWidget(w);
    return LoadResourceFile(&db, w, filename, user, override);
}

#if NeedFunctionPrototypes
void XmtLoadResourceFileList(Widget w, StringConst list)
#else
void XmtLoadResourceFileList(w, list)
Widget w;
StringConst list;
#endif
{
    XrmDatabase db;
    
    w = XmtGetApplicationShell(w); 
    db = XmtDatabaseOfWidget(w);
    LoadResourceFileList(&db, w, list);
}
