/* 
 * Motif Tools Library, Version 3.1
 * $Id: Lexer.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Lexer.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtLexer_h
#define _XmtLexer_h    

#include <ctype.h>

typedef enum {
    XmtLexerNone,
    
    XmtLexerString,
    XmtLexerIdent,
    XmtLexerKeyword,
    XmtLexerInteger,
    
    XmtLexerLParen,
    XmtLexerRParen,
    XmtLexerLBracket,
    XmtLexerRBracket,
    XmtLexerLBrace,
    XmtLexerRBrace,
    XmtLexerLess,
    XmtLexerGreater,
    XmtLexerPlus,
    XmtLexerMinus,
    XmtLexerAsterisk,
    XmtLexerSlash,
    XmtLexerBar,
    XmtLexerEqual,
    XmtLexerSharp,
    XmtLexerTwiddle,
    XmtLexerPercent,
    XmtLexerDollar,
    XmtLexerPeriod,
    XmtLexerCaret,
    XmtLexerColon,
    XmtLexerSemicolon,
    XmtLexerComma,
    
    XmtLexerEndOfString,
    XmtLexerError
} XmtLexerToken;

typedef struct {
    _Xconst char *c;
    XmtLexerToken token;
    int intval;
    String strval;
    String *keywords;  /* must be in alphabetical order */
    int num_keywords;
} XmtLexerRec, *XmtLexer;

#define XmtLexerIntValue(l) ((l)->intval)
#define XmtLexerStrValue(l) ((l)->strval)
#define XmtLexerStrLength(l) ((l)->intval)
#define XmtLexerKeyValue(l) ((l)->intval)
#define XmtLexerGetToken(l) \
    (((l)->token != XmtLexerNone)?(l)->token:_XmtLexerGetToken(l))
#define XmtLexerConsumeToken(l) ((l)->token = XmtLexerNone)
#define XmtLexerSkipWhite(l) while (isspace((int)*(l)->c)) (l)->c++
/* ConsumeToken immediately followed by GetToken */
#define XmtLexerNextToken(l) _XmtLexerGetToken(l)    

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern XmtLexer XmtLexerCreate(String *, int);
extern void XmtLexerDestroy(XmtLexer);
extern void XmtLexerInit(XmtLexer, StringConst);
extern XmtLexerToken _XmtLexerGetToken(XmtLexer);
extern XmtLexerToken XmtLexerScan(XmtLexer, StringConst, XmtWideBoolean);
extern Boolean XmtLexerGetArgList(XmtLexer, String *, Cardinal, Cardinal *);
#else
extern XmtLexer XmtLexerCreate();
extern void XmtLexerDestroy();
extern void XmtLexerInit();
extern XmtLexerToken _XmtLexerGetToken();
extern XmtLexerToken XmtLexerScan();
extern Boolean XmtLexerGetArgList();
#endif
_XFUNCPROTOEND
		  
#endif
