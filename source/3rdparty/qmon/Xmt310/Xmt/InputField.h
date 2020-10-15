/* 
 * Motif Tools Library, Version 3.1
 * $Id: InputField.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: InputField.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtInputField_h
#define _XmtInputField_h

#include <Xm/Text.h>

externalref WidgetClass xmtInputFieldWidgetClass;
typedef struct _XmtInputFieldClassRec *XmtInputFieldWidgetClass;
typedef struct _XmtInputFieldRec *XmtInputFieldWidget;

typedef struct {
    StringConst input;
                   /* this string can be replaced, but not modified or freed */
    Boolean okay;  /* set to False if the string is bad */
                   /* or, set to True to stop error handling */
} XmtInputFieldCallbackStruct;

externalref _Xconst char XmtInputFieldStrings[];
#ifndef XmtNautoDelete
#define XmtNautoDelete ((char*)&XmtInputFieldStrings[0])
#endif
#ifndef XmtNautoInsert
#define XmtNautoInsert ((char*)&XmtInputFieldStrings[11])
#endif
#ifndef XmtNbeepOnError
#define XmtNbeepOnError ((char*)&XmtInputFieldStrings[22])
#endif
#ifndef XmtNbufferSymbolName
#define XmtNbufferSymbolName ((char*)&XmtInputFieldStrings[34])
#endif
#ifndef XmtNerrorBackground
#define XmtNerrorBackground ((char*)&XmtInputFieldStrings[51])
#endif
#ifndef XmtNerrorCallback
#define XmtNerrorCallback ((char*)&XmtInputFieldStrings[67])
#endif
#ifndef XmtNerrorForeground
#define XmtNerrorForeground ((char*)&XmtInputFieldStrings[81])
#endif
#ifndef XmtNerrorString
#define XmtNerrorString ((char*)&XmtInputFieldStrings[97])
#endif
#ifndef XmtNhighlightOnError
#define XmtNhighlightOnError ((char*)&XmtInputFieldStrings[109])
#endif
#ifndef XmtNinput
#define XmtNinput ((char*)&XmtInputFieldStrings[126])
#endif
#ifndef XmtNinputCallback
#define XmtNinputCallback ((char*)&XmtInputFieldStrings[132])
#endif
#ifndef XmtNmatchAll
#define XmtNmatchAll ((char*)&XmtInputFieldStrings[146])
#endif
#ifndef XmtNoverstrike
#define XmtNoverstrike ((char*)&XmtInputFieldStrings[155])
#endif
#ifndef XmtNpattern
#define XmtNpattern ((char*)&XmtInputFieldStrings[166])
#endif
#ifndef XmtNreplaceOnError
#define XmtNreplaceOnError ((char*)&XmtInputFieldStrings[174])
#endif
#ifndef XmtNtargetSymbolName
#define XmtNtargetSymbolName ((char*)&XmtInputFieldStrings[189])
#endif
#ifndef XmtNverifyCallback
#define XmtNverifyCallback ((char*)&XmtInputFieldStrings[206])
#endif
#ifndef XmtCAutoDelete
#define XmtCAutoDelete ((char*)&XmtInputFieldStrings[221])
#endif
#ifndef XmtCAutoInsert
#define XmtCAutoInsert ((char*)&XmtInputFieldStrings[232])
#endif
#ifndef XmtCBeepOnError
#define XmtCBeepOnError ((char*)&XmtInputFieldStrings[243])
#endif
#ifndef XmtCBufferSymbolName
#define XmtCBufferSymbolName ((char*)&XmtInputFieldStrings[255])
#endif
#ifndef XmtCErrorBackground
#define XmtCErrorBackground ((char*)&XmtInputFieldStrings[272])
#endif
#ifndef XmtCErrorForeground
#define XmtCErrorForeground ((char*)&XmtInputFieldStrings[288])
#endif
#ifndef XmtCErrorString
#define XmtCErrorString ((char*)&XmtInputFieldStrings[304])
#endif
#ifndef XmtCHighlightOnError
#define XmtCHighlightOnError ((char*)&XmtInputFieldStrings[316])
#endif
#ifndef XmtCInput
#define XmtCInput ((char*)&XmtInputFieldStrings[333])
#endif
#ifndef XmtCMatchAll
#define XmtCMatchAll ((char*)&XmtInputFieldStrings[339])
#endif
#ifndef XmtCOverstrike
#define XmtCOverstrike ((char*)&XmtInputFieldStrings[348])
#endif
#ifndef XmtCPattern
#define XmtCPattern ((char*)&XmtInputFieldStrings[359])
#endif
#ifndef XmtCReplaceOnError
#define XmtCReplaceOnError ((char*)&XmtInputFieldStrings[367])
#endif
#ifndef XmtCTargetSymbolName
#define XmtCTargetSymbolName ((char*)&XmtInputFieldStrings[382])
#endif

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern Widget XmtCreateInputField(Widget, StringConst, ArgList, Cardinal);
extern void XmtRegisterInputField(void);
extern String XmtInputFieldGetString(Widget);
extern void XmtInputFieldSetString(Widget, StringConst);
#else
extern Widget XmtCreateInputField();
extern void XmtRegisterInputField();
extern String XmtInputFieldGetString();
extern void XmtInputFieldSetString();
#endif
_XFUNCPROTOEND    

#endif /* _XmtInputField_h */
