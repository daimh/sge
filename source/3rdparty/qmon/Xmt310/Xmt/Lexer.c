/* 
 * Motif Tools Library, Version 3.1
 * $Id: Lexer.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Lexer.c,v $
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

#if NeedFunctionPrototypes
XmtLexer XmtLexerCreate(String *keywords, int num_keywords)
#else
XmtLexer XmtLexerCreate(keywords, num_keywords)
String *keywords;
int num_keywords;
#endif
{
    XmtLexer lexer = XtNew(XmtLexerRec);

    lexer->c = NULL;
    lexer->token = XmtLexerNone;
    lexer->intval = 0;
    lexer->strval = NULL;
    lexer->keywords = keywords;
    lexer->num_keywords = num_keywords;
    return lexer;
}

#if NeedFunctionPrototypes
void XmtLexerDestroy(XmtLexer lexer)
#else
void XmtLexerDestroy(lexer)
XmtLexer lexer;
#endif
{
    XtFree((char *)lexer);
}

#if NeedFunctionPrototypes
void XmtLexerInit(XmtLexer lexer, StringConst str)
#else
void XmtLexerInit(lexer, str)
XmtLexer lexer;
StringConst str;
#endif
{
    lexer->c = str;
    lexer->token = XmtLexerNone;
    lexer->intval = 0;
    lexer->strval = NULL;
}

#if NeedFunctionPrototypes
XmtLexerToken _XmtLexerGetToken(XmtLexer l)
#else
XmtLexerToken _XmtLexerGetToken(l)
XmtLexer l;
#endif
{
    int total;
    int len;
    _Xconst char *mark;
    int keyword;

    l->token = XmtLexerNone;

    if (l->c == NULL) {
	l->token = XmtLexerEndOfString;
	return l->token;
    }
    
    /* skip whitespace */
    while (isspace((int)*l->c)) l->c++;

    /* check for end-of-string */
    if (*l->c == '\0') {
	l->token = XmtLexerEndOfString;
	return l->token;
    }

    /* check punctuation */
    switch (*l->c) {
    case '(':  l->token = XmtLexerLParen; break;
    case ')':  l->token = XmtLexerRParen; break;
    case '[':  l->token = XmtLexerLBracket; break;
    case ']':  l->token = XmtLexerRBracket; break;
    case '{':  l->token = XmtLexerLBrace; break;
    case '}':  l->token = XmtLexerRBrace; break;
    case '<':  l->token = XmtLexerLess; break;
    case '>':  l->token = XmtLexerGreater; break;
    case '+':  l->token = XmtLexerPlus; break;
    case '-':  l->token = XmtLexerMinus; break;
    case '*':  l->token = XmtLexerAsterisk; break;
    case '/':  l->token = XmtLexerSlash; break;
    case '|':  l->token = XmtLexerBar; break;
    case '=':  l->token = XmtLexerEqual; break;
    case '#':  l->token = XmtLexerSharp; break;
    case '~':  l->token = XmtLexerTwiddle; break;
    case '%':  l->token = XmtLexerPercent; break;
    case '$':  l->token = XmtLexerDollar; break;
    case '.':  l->token = XmtLexerPeriod; break;
    case '^':  l->token = XmtLexerCaret; break;
    case ':':  l->token = XmtLexerColon; break;
    case ';':  l->token = XmtLexerSemicolon; break;
    case ',':  l->token = XmtLexerComma; break;
    }

    if (l->token != XmtLexerNone) {
	l->c++;
	return l->token;
    }

    if (isdigit((int)*l->c)) {
	total = 0;
	while (isdigit((int)*l->c)) {
	    total *= 10;
	    total += (int)(*l->c - '0');
	    l->c++;
	}
	l->token = XmtLexerInteger;
	l->intval = total;
    }
    else if (*l->c == '"') { /* its a string */
	l->c++;
	mark = l->c;
	while((*l->c != '\0') && (*l->c != '"')) {
	    if ((*l->c == '\\') && (*(l->c+1) != '\0'))
		l->c++;
	    l->c++;
	}
	if (*l->c == '\0') {
	    XmtWarningMsg("XmtLexer", "badString",
			  "unterminated string.");
	    l->token = XmtLexerError;
	}
	else {
	    len = l->c - mark;
	    l->c++;
	    l->strval = XtMalloc(len+1);
	    strncpy(l->strval, mark, len);
	    l->strval[len] = '\0';
	    l->intval = len;
	    l->token = XmtLexerString;
	}
    }
    else if (isalnum(*l->c) || (*l->c == '_')) {
	/* its an ident.  Figure out how long and copy it. */
	mark = l->c;
	for(len=0; *l->c &&
	    (isalnum((int)*l->c) || (*l->c == '_'));
	    len++,l->c++);
	l->strval = XtMalloc(len+1);
	strncpy(l->strval, mark, len);
	l->strval[len] = '\0';
	l->intval = len;

	/* now go see if it is a keyword. */
	keyword = XmtBSearch(l->strval, l->keywords, l->num_keywords);

	if (keyword != -1) {
	    l->intval = keyword;
	    l->token = XmtLexerKeyword;
	    XtFree(l->strval);
	    l->strval = l->keywords[keyword];
	}
	else
	    l->token = XmtLexerIdent;
    }
    else {  /* otherwise it is an unrecognized character. */
	XmtWarningMsg("XmtLexer", "badChar",
		      "unrecognized character `%c'.",
		      *l->c);
	l->c++;
	l->token = XmtLexerError;
    }
    
    return l->token;
}

/*
 * Scan from the current lexer location until an unquoted character
 * from the delimiters string is encountered.  Copy the string and return
 * XmtLexerString.  If include is True, include the delimiter, otherwise 
 * don't.  If the end of string is found return XmtLexerEndOfString.
 */

#if NeedFunctionPrototypes
XmtLexerToken XmtLexerScan(XmtLexer l, StringConst delimiters,
			   XmtWideBoolean include)
#else
XmtLexerToken XmtLexerScan(l, delimiters, include)
XmtLexer l;
StringConst delimiters;
int include;
#endif
{
    _Xconst char *c, *d;
    Boolean instring = False;

    if (l->c == '\0') {
	l->token = XmtLexerEndOfString;
	return XmtLexerEndOfString;
    }
    
    for(c = l->c; *c; c++) {
    	if (!instring) {
	    for(d = delimiters; *d && (*d != *c); d++);
	    if (*d) break;
	}
	if ((*c == '\\') && *(c+1)) c++;
	else if (*c == '"') instring = !instring;
    }

    if (*c == '\0') {
	l->c = c;
	l->token = XmtLexerEndOfString;
	return XmtLexerEndOfString;
    }

    /*
     * if include is True, we include the terminating delimiter.
     */
    if (include) c++;  

    l->intval = c - l->c;
    l->strval = XtMalloc(l->intval +1);
    strncpy(l->strval, l->c, l->intval);
    l->strval[l->intval] = '\0';
    l->c = c;
    l->token = XmtLexerString;
    return XmtLexerString;
}

#if NeedFunctionPrototypes
static String GetArg(XmtLexer l)
#else
static String GetArg(l)
XmtLexer l;
#endif
{
    _Xconst char *c, *s;
    register char *t;
    Boolean instring;
    int parenlevel;
    int len;
    char *arg;
    
    /*
     * This procedure breaks the XmtLexer abstraction and scans the
     * string directly.  It should only be called when the last token
     * has been consumed.
     */

    /* skip whitespace */
    XmtLexerSkipWhite(l);

    /* special case for procedures with 0 args */
    if (*l->c == ')') {
	return NULL;
    }

    /* scan to an unquoted, unnested comma or right paren */
    instring = False;
    parenlevel = 0;
    for(c = l->c; *c; c++) {
	if (*c == '\0') break;
	else if (!instring && parenlevel<=0 && (*c == ',' || *c == ')')) break;
	else if (*c == '"') instring = !instring;
	else if (!instring && *c == '(') parenlevel++;
	else if (!instring && *c == ')') parenlevel--;
	else if ((*c == '\\') && (*(c+1) != '\0')) c++;
    }

    /* back up over any whitespace */
    while(isspace(*(c-1))) c--;

    /* allocate space for a copy of the arg */
    len = (int) (c - l->c);
    arg = XtMalloc(len+1);

    /* copy it, unquoting things */
    for(s = l->c, t = arg; s < c; s++) {
	if (*s == '\\') *t++ = *++s;
	else *t++ = *s;
    }

    /* and terminate the string */
    *t = '\0';

    /* put the lexer back in a known state */
    l->c = c;
    l->token = XmtLexerNone;

    return arg;
}

#if NeedFunctionPrototypes
Boolean XmtLexerGetArgList(XmtLexer l, String *args,
			   Cardinal max_args, Cardinal *num_args)
#else
Boolean XmtLexerGetArgList(l, args, max_args, num_args)
XmtLexer l;
String *args;
Cardinal max_args;
Cardinal *num_args;
#endif
{
    XmtLexerToken tok;
    
    /* if no '(', then no arguments, but this is not an error. */
    if (XmtLexerGetToken(l) != XmtLexerLParen) {
	*num_args = 0;
	return True;
    }

    /* otherwise eat the lparen */
    XmtLexerConsumeToken(l);

    /* now loop through the arguments */
    *num_args = 0;
    while(1) {
	/* make sure we haven't overflowed */
	if (*num_args >= max_args) {
	    XmtWarningMsg("_XmtParseArgList", "tooMany",
			  "too many arguments in argument list.");
	    goto error;
	}

	/* get the next argument */
	args[*num_args] = GetArg(l);
	if (args[*num_args]) *num_args += 1;

	/* consume a comma or a right paren */
	tok = XmtLexerGetToken(l);
	if (tok == XmtLexerRParen) {
	    XmtLexerConsumeToken(l);
	    return True;
	}
	else if (tok == XmtLexerComma)
	    XmtLexerConsumeToken(l);
	else {
	    XmtWarningMsg("_XmtParseArgList", "syntax",
			  "syntax error in argument list");
	    goto error;
	}
    }

 error:
    /* read past closing ')' */
    while(((tok = XmtLexerGetToken(l)) != XmtLexerEndOfString) &&
	  (tok != XmtLexerRParen))
	XmtLexerConsumeToken(l);
    XmtLexerConsumeToken(l);
    return False;
}
