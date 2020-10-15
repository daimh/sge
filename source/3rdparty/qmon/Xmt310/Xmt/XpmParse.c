/* 
 * Motif Tools Library, Version 3.1
 * $Id: XpmParse.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: XpmParse.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

/*
 * This file is derived in part from the XPM 3.0 distribution by
 * Arnaud Le Hors.  See the file COPYRIGHT for the Groupe Bull copyright.
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#ifdef VMS
#include <perror.h>
#endif
#include <Xmt/Xmt.h>
#include <Xmt/Xpm.h>

/*
 * minimal portability layer between ansi and KR C 
 */

/* forward declaration of functions with prototypes */

#if NeedFunctionPrototypes              /* ANSI || C++ */
#define FUNC(f, t, p) extern t f p
#define LFUNC(f, t, p) static t f p
#else					/* K&R */
#define FUNC(f, t, p) extern t f()
#define LFUNC(f, t, p) static t f()
#endif					/* end of K&R */

/* Return ErrorStatus codes:
 * null     if full success
 * positive if partial success
 * negative if failure
 */
#define XpmSuccess       0
#define XpmOpenFailed   -1
#define XpmFileInvalid  -2
#define XpmNoMemory     -3

/* character constants */
#define EOL '\n'
#define TAB '\t'
#define SPC ' '

/* value for the lexer type field */
/* where the lexer reads its characters from */
#define XPMARRAY 0
#define XPMFILE  1
#define XPMPIPE  2  /* XXX what is this for?  Synonym for file for now */
#define XPMSTRING 3

typedef struct {
    unsigned int type;
    union {
	FILE *file;
	char **data;
    }     stream;
    _Xconst char *cptr;
    unsigned int line;
    char Bos, Eos;
    Boolean InsideString;		/* whether we are in a string or not */
} LexerData;

typedef struct {
    char *type;				/* key word */
    char Bos;				/* character begining strings */
    char Eos;				/* character ending strings */
    char *Strs;				/* strings separator */
    char *Dec;				/* data declaration string */
    char *Boa;				/* string begining assignment */
    char *Eoa;				/* string ending assignment */
} LexerTypeData;

static LexerTypeData lexer_types[] =
{
 {"", '\0', '\n', "", "", "", ""},	/* Natural type */
 {"C", '"', '"', ",\n", "static char *", "[] = {\n", "};\n"},
 {"Lisp", '"', '"', "\n", "(setq ", " '(\n", "))\n"},
 {NULL, 0, 0, NULL, NULL, NULL, NULL}
};

/*
 * these strings come directly before the optional default color string
 * for each of the visuals.  The string "s" comes before the optional
 * symbolic color name.  They correspond to the XmtXpmVisualClass enumerated
 * values.
 */
static char *xpmColorKeys[] =
{
 "m",					/* key #0: mono visual */
 "g4",					/* key #1: 4 grays visual */
 "g",					/* key #2: gray visual */
 "c",					/* key #3: color visual */
};

LFUNC(ParseImage, int, (LexerData * data,
			 XmtImage * xmtimage_return));
LFUNC(FreeColorTable, void, (XmtXpmColorTable color_table, int ncolors));
LFUNC(InitXmtImage, void, (XmtImage * xmpdata));

/* lexer */
LFUNC(xpmNextString, void, (LexerData * lexer));
LFUNC(xpmNextUI, int, (LexerData * lexer, unsigned int *ui_return));
LFUNC(xpmGetC, int, (LexerData * lexer));
LFUNC(xpmUngetC, int, (int c, LexerData * lexer));
LFUNC(xpmNextWord, unsigned int, (LexerData * lexer, char *buf));
LFUNC(InitFileLexer, int, (StringConst filename, LexerData * lexer));
LFUNC(InitArrayLexer, void, (char **data, LexerData * lexer));
LFUNC(InitStringLexer, void, (StringConst string, LexerData * lexer));
LFUNC(XpmLexerClose, void, (LexerData * lexer));
LFUNC(atoui, unsigned int, (char *p, unsigned int l, unsigned int *ui_return));

static unsigned int
#if NeedFunctionPrototypes
atoui(register char *p, unsigned int l, unsigned int *ui_return)
#else
atoui(p, l, ui_return)
register char *p;
unsigned int l;
unsigned int *ui_return;
#endif
{
    register int n, i;

    n = 0;
    for (i = 0; i < l; i++)
	if (*p >= '0' && *p <= '9')
	    n = n * 10 + *p++ - '0';
	else
	    break;

    if (i != 0 && i == l) {
	*ui_return = n;
	return 1;
    } else
	return 0;
}

/*
 * skip to the end of the current string and the beginning of the next one
 */
#if NeedFunctionPrototypes
static void xpmNextString(LexerData *lexer)
#else
static void xpmNextString(lexer)
LexerData *lexer;
#endif
{
    int c;

    switch (lexer->type) {
    case XPMARRAY:
	lexer->cptr = (lexer->stream.data)[++lexer->line];
	break;
    case XPMSTRING:
	while(*lexer->cptr && *lexer->cptr != '\n') lexer->cptr++;
	if (*lexer->cptr) lexer->cptr++;
	break;
    case XPMFILE:
    case XPMPIPE:
	if (lexer->Eos)
	    while ((c = xpmGetC(lexer)) != lexer->Eos && c != EOF);
	if (lexer->Bos)			/* if not natural XPM2 */
	    while ((c = xpmGetC(lexer)) != lexer->Bos && c != EOF);
	break;
    }
}

/*
 * skip whitespace and compute the following unsigned int,
 * returns 1 if one is found and 0 if not
 */
static int
#if NeedFunctionPrototypes
xpmNextUI(LexerData *lexer, unsigned int *ui_return)
#else
xpmNextUI(lexer, ui_return)
LexerData *lexer;
unsigned int *ui_return;
#endif
{
    char buf[BUFSIZ];
    int l;

    l = xpmNextWord(lexer, buf);
    return atoui(buf, l, ui_return);
}

/*
 * return the current character, skipping comments
 */
#if NeedFunctionPrototypes
static int xpmGetC(LexerData *lexer)
#else
static int xpmGetC(lexer)
LexerData *lexer;
#endif
{
    int c;

    switch (lexer->type) {
    case XPMARRAY:
    case XPMSTRING:
	return (*lexer->cptr++);
    case XPMFILE:
    case XPMPIPE:
	c = getc(lexer->stream.file);

	if (lexer->Bos && lexer->Eos
	    && (c == lexer->Bos || c == lexer->Eos)) {
	    /* if not natural XPM2 */
	    lexer->InsideString = !lexer->InsideString;
	    return (c);
	}
	/* comment is started with any slash outside of a string */
	if (!lexer->InsideString && c == '/') {;
	    /*
	     * skip to the end of the comment
	     */
	    do {
		c = getc(lexer->stream.file);
		if (c == '*') {
		    c = getc(lexer->stream.file);
		    if (c == '/') break;
		}
	    } while (c != EOF);
	    c = xpmGetC(lexer);
	}
	return (c);
    }
    return 0;
}

/*
 * push the given character back
 */
#if NeedFunctionPrototypes
static int xpmUngetC(int c, LexerData *lexer)
#else
static int xpmUngetC(c, lexer)
int c;
LexerData *lexer;
#endif
{
    switch (lexer->type) {
    case XPMARRAY:
    case XPMSTRING:
	return (*--lexer->cptr);
    case XPMFILE:
    case XPMPIPE:
	if (lexer->Bos && (c == lexer->Bos || c == lexer->Eos))
	    /* if not natural XPM2 */
	    lexer->InsideString = !lexer->InsideString;
	return (ungetc(c, lexer->stream.file));
    }
    return 0;  /* make compiler happy */
}

/*
 * skip whitespace and return the following word
 */
static unsigned int
#if NeedFunctionPrototypes
xpmNextWord(LexerData *lexer, char *buf)
#else
xpmNextWord(lexer, buf)
LexerData *lexer;
char *buf;
#endif
{
    register unsigned int n = 0;
    int c;

    switch (lexer->type) {
    case XPMARRAY:
    case XPMSTRING:
	while (isspace(c = *lexer->cptr) && c != lexer->Eos)
	    lexer->cptr++;
	do {
	    c = *lexer->cptr++;
	    buf[n++] = c;
	} while (!isspace(c) && c != lexer->Eos && c != '\0');
	n--;
	lexer->cptr--;
	break;
    case XPMFILE:
    case XPMPIPE:
	while (isspace(c = xpmGetC(lexer)) && c != lexer->Eos);
	while (!isspace(c) && c != lexer->Eos && c != EOF) {
	    buf[n++] = c;
	    c = xpmGetC(lexer);
	}
	xpmUngetC(c, lexer);
	break;
    }
    return (n);
}

/*
 * open the given file to be read as an LexerData which is returned.
 */
static int
#if NeedFunctionPrototypes
InitFileLexer(StringConst filename, LexerData *lexer)
#else
InitFileLexer(filename, lexer)
StringConst filename;
LexerData *lexer;
#endif
{
    if (!filename || !(lexer->stream.file = fopen(filename, "r"))) {
	XmtWarningMsg("XpmInitFileLexer", "io",
		      "can't read file '%s':\n\t%s",
 		      filename, strerror(errno));
	return (XpmOpenFailed);
    }
    lexer->type = XPMFILE;
    lexer->InsideString = False;
    return (XpmSuccess);
}

/*
 * open the given array to be read or written as an LexerData which is returned
 */
#if NeedFunctionPrototypes
static void InitArrayLexer(char **data, LexerData *lexer)
#else
static void InitArrayLexer(data, lexer)
char **data;
LexerData *lexer;
#endif
{
    lexer->type = XPMARRAY;
    lexer->stream.data = data;
    lexer->cptr = *data;
    lexer->line = 0;
    lexer->Bos = lexer->Eos = '\0';
    lexer->InsideString = False;
}

#if NeedFunctionPrototypes
static void InitStringLexer(StringConst str, LexerData *lexer)
#else
static void InitStringLexer(str, lexer)
StringConst str;
LexerData *lexer;
#endif
{
    lexer->type = XPMSTRING;
    lexer->cptr = str;
    lexer->Bos = '\0';
    lexer->Eos = '\n';
    lexer->InsideString = False;
}

/*
 * close the file related to the LexerData if any
 */
#if NeedFunctionPrototypes
static void XpmLexerClose(LexerData *lexer)
#else
static void XpmLexerClose(lexer)
LexerData *lexer;
#endif
{
    switch (lexer->type) {
    case XPMARRAY:
    case XPMSTRING:
	break;
    case XPMFILE:
    case XPMPIPE:
	if (lexer->stream.file != (stdout) && lexer->stream.file != (stdin))
	    fclose(lexer->stream.file);
	break;
    }
}


/* Copyright 1990,91 GROUPE BULL -- See licence conditions in file COPYRIGHT */
/*****************************************************************************\
* parse.c:                                                                    *
*                                                                             *
*  XPM library                                                                *
*  Parse an XPM file or array and store the found informations                *
*  in an an XmtImage structure which is returned.                      *
*                                                                             *
*  Developed by Arnaud Le Hors                                                *
\*****************************************************************************/

/*
 * Free the computed color table
 */

#if NeedFunctionPrototypes
static void FreeColorTable(XmtXpmColorTable color_table, int ncolors)
#else
static void FreeColorTable(color_table, ncolors)
XmtXpmColorTable color_table;
int ncolors;
#endif
{
    int color, visual;

    if (color_table) {
	for (color = 0; color < ncolors; color++) {
	    if (color_table[color].symbolic_name)
		XtFree(color_table[color].symbolic_name);
	    for (visual = 0; visual < NVISUALS; visual++)
		if (color_table[color].default_colors[visual])
		    XtFree(color_table[color].default_colors[visual]);
	}
	XtFree((char *)color_table);
    }
}


/*
 * Free the XmtImage pointers which have been allocated,
 * and the XmtImage structure itself.
 */
#if NeedFunctionPrototypes
void XmtFreeXmtImage(XmtImage *xmtimage)
#else
void XmtFreeXmtImage(xmtimage)
XmtImage *xmtimage;
#endif
{
    if (xmtimage->color_table)
	FreeColorTable(xmtimage->color_table, xmtimage->ncolors);
    if (xmtimage->data)
	XtFree((char *)xmtimage->data);
    XtFree((char *)xmtimage);
}

/*
 * Intialize the XmtImage pointers to Null to know
 * which ones must be XtFreed later on.
 */
#if NeedFunctionPrototypes
static void InitXmtImage(XmtImage *xmtimage)
#else
static void InitXmtImage(xmtimage)
XmtImage *xmtimage;
#endif
{
    xmtimage->ncolors = 0;
    xmtimage->color_table = NULL;
    xmtimage->data = NULL;
}


/*
 * This function parses an Xpm file or data and store the found informations
 * in an an XmtImage structure which is returned.
 */
static int
#if NeedFunctionPrototypes
ParseImage(LexerData *lexer, XmtImage *xmtimage_return)
#else
ParseImage(lexer, xmtimage_return)
LexerData *lexer;
XmtImage *xmtimage_return;
#endif
{
    /* variables to return */
    unsigned int width, height;
    unsigned int ncolors = 0;
    unsigned int cpp;         /* chars per pixel */
    unsigned int x_hotspot, y_hotspot;
    Boolean has_hotspot;
    XmtXpmColorTable color_table = NULL;
    unsigned char *data = NULL;

    /* calculation variables */
    unsigned int rncolors = 0;
    unsigned int key;			/* color key */
    char symbol[20];    /* must be bigger than cpp */
    char *symbols = NULL;
    char buf[BUFSIZ];  /* where lexer returns tokens */
    unsigned char *iptr;
    unsigned int color, i, x, y, len;
    int status;

    /*
     * read hints: width, height, ncolors, chars_per_pixel 
     */
    if (!(xpmNextUI(lexer, &width) && xpmNextUI(lexer, &height)
	  && xpmNextUI(lexer, &rncolors) && xpmNextUI(lexer, &cpp))){
	status = XpmFileInvalid;
	XmtWarningMsg("XmtParseXmtImage", "header",
		      "syntax error in XPM header\n\tShould be: width height number_of_colors chars_per_pixel.");
	goto error;
    }

    ncolors = rncolors;
    if (ncolors > 256) {
	XmtWarningMsg("XmtParseXmtImage", "tooMany",
		      "image contains too many colors");
	status = XpmFileInvalid;
	goto error;
    }

    /*
     * read hotspot coordinates if any 
     */
    x_hotspot = y_hotspot = 0;
    has_hotspot = xpmNextUI(lexer, &x_hotspot) && xpmNextUI(lexer, &y_hotspot);

    /*
     * read colors 
     */
    color_table = (XmtXpmColorTable) XtCalloc(ncolors, sizeof(XmtXpmColor));
    symbols = XtMalloc(ncolors * cpp);

    for (color = 0; color < ncolors; color++) {
	xpmNextString(lexer);		/* skip the line */
	/*
	 * read pixel symbol
	 */
	for (i = 0; i < cpp; i++)
	    symbols[color*cpp + i] = xpmGetC(lexer);

	/*
	 * read color keys and values 
	 */
	while ((len = xpmNextWord(lexer, buf))) {
	    for (key = 0; key < NVISUALS; key++)
		if (!strncmp(xpmColorKeys[key], buf, len))
		    break;
	    if (key < NVISUALS) {  /* a default color key */
		len = xpmNextWord(lexer, buf);
		color_table[color].default_colors[key] = XtMalloc(len + 1);
		strncpy(color_table[color].default_colors[key], buf, len);
		color_table[color].default_colors[key][len] = '\0';
	    }
	    else if (len == 1 && buf[0] == 's') { /* symbolic name */
		len = xpmNextWord(lexer, buf);
		color_table[color].symbolic_name = XtMalloc(len+1);
		strncpy(color_table[color].symbolic_name, buf, len);
		color_table[color].symbolic_name[len] = '\0';
	    }
	    else {
		status = XpmFileInvalid;
		buf[len] = '\0';
		XmtWarningMsg("XmtParseXmtImage", "badkey",
			      "Bad key '%s' in color specification.  Use 's', 'c', 'm', 'g', or 'g4'.",
			      buf);
		goto error;  /* key without value */
	    }
	}
    }

    /*
     * read pixels and index them on color number 
     */
    data = (unsigned char *) XtMalloc(sizeof(unsigned char) * width * height);
    iptr = data;

    for (y = 0; y < height; y++) {
	xpmNextString(lexer);
	for (x = 0; x < width; x++, iptr++) {
	    for (i = 0; i < cpp; i++)
		symbol[i] = xpmGetC(lexer);
	    for (color = 0; color < ncolors; color++) {
		for(i=0; i < cpp; i++)
		    if (symbol[i] != symbols[color*cpp + i]) break;
		if (i == cpp) break;
	    }
	    if (color == ncolors) { /* no color matches */
		status = XpmFileInvalid;
		if ((symbol[0] == '\n') ||
		    ((cpp == 2) && (symbol[1] == '\n')))
		    XmtWarningMsg("XmtParseXmtImage", "short",
				  "Line %d is too short.", y);
		else
		    XmtWarningMsg("XmtParseXmtImage", "unknown",
				  "Unknown color symbol %c%c at pixel (%d,%d)",
				  symbol[0], (cpp==2)?symbol[1]:' ', x, y);
		goto error; 
	    }
	    *iptr = (unsigned char) color;
	}
    }

    XtFree(symbols);
    
    /*
     * store stuff in the XmtImage structure 
     */
    xmtimage_return->width = width;
    xmtimage_return->height = height;
    xmtimage_return->ncolors = ncolors;
    xmtimage_return->color_table = color_table;
    xmtimage_return->data = data;
    xmtimage_return->has_hotspot = has_hotspot;
    xmtimage_return->hotspot.x = x_hotspot;
    xmtimage_return->hotspot.y = y_hotspot;

    return (XpmSuccess);

 error:
    if (color_table) FreeColorTable(color_table, ncolors);
    if (symbols) XtFree(symbols);
    if (data) XtFree((char *)data); 
    return(status);
}

#if NeedFunctionPrototypes
XmtImage *XmtParseXpmFile(StringConst filename)
#else
XmtImage *XmtParseXpmFile(filename)
StringConst filename;
#endif
{
    LexerData lexer;
    char buf[BUFSIZ];
    XmtImage *image;
    int status;
    int format;
    char formatname[4];
    
    status = InitFileLexer(filename, &lexer);
    if (status != XpmSuccess) return NULL;

    image = XtNew(XmtImage);
    InitXmtImage(image);

    /*
     * parse the file headers
     */
    fgets(buf, BUFSIZ-1, lexer.stream.file);
    if ((sscanf(buf, "/* %3s */", formatname) == 1) &&
	(strcmp(formatname, "XPM") == 0)) {
	format = 1;
	lexer.Bos = lexer_types[format].Bos;
	lexer.Eos = '\0';
	xpmNextString(&lexer);	/* skip the assignment line */
	lexer.InsideString = True;
	lexer.Eos = lexer_types[format].Eos;
	status = ParseImage(&lexer, image);
    }
    else
	status = XpmFileInvalid;
	
    XpmLexerClose(&lexer);
    if (status != XpmSuccess) {
	XmtFreeXmtImage(image);
	return NULL;
    }
    return image;
}

#if NeedFunctionPrototypes
XmtImage *XmtParseXpmData(String *data)
#else
XmtImage *XmtParseXpmData(data)
String *data;
#endif
{
    LexerData lexer;
    int status;
    XmtImage *image;

    InitArrayLexer(data, &lexer);
    image = XtNew(XmtImage);
    InitXmtImage(image);

    status = ParseImage(&lexer, image);

    if (status != XpmSuccess) {
	XmtFreeXmtImage(image);
	return NULL;
    }
    return image;
}

#if NeedFunctionPrototypes
XmtImage *XmtParseXpmString(StringConst string)
#else
XmtImage *XmtParseXpmString(string)
StringConst string;
#endif
{
    LexerData lexer;
    int status;
    XmtImage *image;

    InitStringLexer(string, &lexer);
    image = XtNew(XmtImage);
    InitXmtImage(image);

    status = ParseImage(&lexer, image);

    if (status != XpmSuccess) {
	XmtFreeXmtImage(image);
	return NULL;
    }
    return image;
}

