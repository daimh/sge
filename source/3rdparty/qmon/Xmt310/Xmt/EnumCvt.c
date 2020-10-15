/* 
 * Motif Tools Library, Version 3.1
 * $Id: EnumCvt.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: EnumCvt.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/ConvertersP.h>

/* ARGSUSED */
#if NeedFunctionPrototypes
Boolean XmtConvertStringToEnum(Display *dpy,
                               XrmValuePtr args, Cardinal *num_args,
                               XrmValuePtr from, XrmValuePtr to, XtPointer *data)
#else
Boolean XmtConvertStringToEnum(dpy, args, num_args, from, to, data)
Display *dpy;
XrmValuePtr args;
Cardinal *num_args;
XrmValuePtr from;
XrmValuePtr to;
XtPointer *data;
#endif
{
    String type = *(String *)args[0].addr;
    String *names = *(String **)args[1].addr;
/*     Cardinal *values = *(Cardinal**)args[2].addr; */
/*     Cardinal num = (Cardinal)*(XtPointer*)args[3].addr; */
    Cardinal *values = (Cardinal*)*(XtPointer*)args[2].addr;
    Cardinal num = (Cardinal)*(XtPointer*)args[3].addr;
    String *prefixes = *(String **)args[4].addr;
    String target = (String) from->addr;
    String prefix;
    int i;
    static Cardinal value;  /* static for converter return */

    /*
     * First, strip any of the listed prefixes off of target.
     * All of the prefixes are optional, but they must occur in the
     * order that they appear in the array in.
     */
    if (prefixes) {
        while (*prefixes) {
            prefix = *prefixes;
            for(i=0; prefix[i] && (target[i] == prefix[i]); i++);
            if (prefix[i] == '\0') target += i;
            prefixes++;
        }
    }

    /*
     * now see if the target appears in the names array.
     * If not, print a warning and fail.
     */
    i = XmtBSearch(target, names, num);

/* printf("XmtBSearch(target, names, num) => %d\n", i); */

    if (i == -1) {
        XtDisplayStringConversionWarning(dpy, (String)from->addr, type);
        return False;
    }
    
    /*
     * converted value is values[i].
     * We return it in different ways depending on the desired size to.size.
     * Note how this code compares to the done() macro in ConvertersP.h
     */
    value = values[i];
    if (to->addr != NULL) {
        switch (to->size) {
        case sizeof(char):
            *(char *)to->addr = value;
            break;
#ifdef CRAY /* short and int are the same size (8 bytes) on the Cray */
        case sizeof(int):
            *(int *)to->addr = value;
            break;
#else
        case sizeof(short):
            *(short *)to->addr = value;
            break;
        case sizeof(Cardinal):
            *(Cardinal *)to->addr = value;
            break;
#endif
        default:
            if (to->size > sizeof(Cardinal)) {
                to->size = sizeof(Cardinal);
                *(Cardinal *)to->addr = value;
            }
            else {
                to->size = sizeof(Cardinal);
                return False;
            }
            break;
        }
    }
    else {
        to->addr = (XtPointer)&value;  /* value is a static variable */
        to->size = sizeof(value);
    }
    return True;
}


#if NeedFunctionPrototypes
void XmtRegisterEnumConverter(StringConst type, String *names, Cardinal *values,
                              Cardinal num, String *prefixes)
#else
void XmtRegisterEnumConverter(type, names, values, num, prefixes)
StringConst type;
String *names;
Cardinal *values;
Cardinal num;
String *prefixes;
#endif
{
    XtConvertArgRec args[5];

    /*
     * XtSetTypeConverter copies these args, so it is
     * okay to use an automatic array.
     */
    args[0].address_mode = XtImmediate;
    args[0].address_id = (XtPointer) type;
    args[0].size = sizeof(type);
    
    args[1].address_mode = XtImmediate;
    args[1].address_id = (XtPointer) names;
    args[1].size = sizeof(names);
        
    args[2].address_mode = XtImmediate;
    args[2].address_id = (XtPointer) values;
    args[2].size = sizeof(values);

    args[3].address_mode = XtImmediate;
    args[3].address_id = (XtPointer) num;
    args[3].size = sizeof(num);

    args[4].address_mode = XtImmediate;
    args[4].address_id = (XtPointer) prefixes;
    args[4].size = sizeof(prefixes);
        
    XtSetTypeConverter(XtRString, (String)type, XmtConvertStringToEnum,
                       args, 5, XtCacheNone, NULL);
}
