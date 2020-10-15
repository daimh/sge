/* 
 * Motif Tools Library, Version 3.1
 * $Id: Xmt.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Xmt.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _Xmt_h
#define _Xmt_h

/*
 * This header will contain whatever #ifdefs and #includes are needed to
 * make the Xmt library portable on as many platforms as possible
 */

#if !defined (XMT_HAS_STRERROR)
# if defined (XMT_HAS_SYS_ERRLIST)
extern char *sys_errlist[];
#   define strerror(err) sys_errlist[err]
# else
#   define strerror(err) "strerror is unsupported"
# endif /* XMT_HAS_SYS_ERRLIST */
#endif /* !XMT_HAS_STERROR */

/*
 * Standard X header files
 */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

/*
 * If the compiler can't handle prototypes, tell the Motif headers so
 */
#if NeedFunctionPrototypes == 0
#ifndef _NO_PROTO
#define _NO_PROTO
#endif
#endif

/*
 * The standard Motif header file
 */
#include <Xm/Xm.h>

/*
 * Specify what version of Xmt this is.
 */
#define XmtVERSION      3
#define XmtREVISION     1
#define XmtVersion      (XmtVERSION * 1000 + XmtREVISION)
#define XmtPatchlevel   0

/*
 * figure out what version of Xt we're using.
 */
#if defined(XtSpecificationRelease)
#  define X11R4
#else
#  undef X11R4
#endif
#if defined(XtSpecificationRelease) && XtSpecificationRelease >= 5
#  define X11R5
#else
#  undef X11R5
#endif

/* 
 * There were some useful Xrm functions, etc. added in R5.
 * Pre-R5, they can be approximated as follows:
 */
#ifndef X11R5
#  define XrmGetDatabase(dpy) ((dpy)->db)
#  define XrmSetDatabase(dpy, database) ((dpy)->db = (database))
#  define XtScreenDatabase(screen) XtDatabase(DisplayOfScreen(screen))
#  define XrmPermStringToQuark(str) XrmStringToQuark(str)
#  define XPointer caddr_t
#endif

/*
 * The MIT X11R5 implementation defines some useful C++
 * portability stuff.  We redefine it here in case we're using
 * X11R4 or some other implementation
 */
#ifndef _XFUNCPROTOBEGIN
#  ifdef __cplusplus	       	/* for C++ V2.0 */
#    define _XFUNCPROTOBEGIN extern "C" {
#    define _XFUNCPROTOEND }
#  else
#    define _XFUNCPROTOBEGIN
#    define _XFUNCPROTOEND
#  endif /* __cplusplus */
#endif /* _XFUNCPROTOBEGIN */

/*
 * const String s; is the same as char * const s; not const char *s;
 * The latter is really what we want.  As a workaround, we define
 * a new type StringConst which is a const char *.
 * Note that we can declare: const StringConst s;
 */
#if __STDC__ || defined(__cplusplus) || defined(c_plusplus)
typedef const char *StringConst;
#ifndef _Xconst
#define _Xconst const
#endif
#else
typedef char * StringConst;
#ifndef _Xconst
#define _Xconst /* as nothing */
#endif
#endif

/*
 * In R4 we get the database of a widget with XtDatabase(); in
 * R5 we use XtScreenDatabase().  So we define a macro that will
 * do the right thing for us.
 */
#ifndef X11R5
#define XmtDatabaseOfWidget(w) (XtDatabase(XtDisplayOfObject(w)))
#else
#define XmtDatabaseOfWidget(w) (XtScreenDatabase(XtScreenOfObject(w)))
#endif

/*
 * Standard C and K&R C handle varargs routines slightly differently.
 */
#if NeedVarargsPrototypes
# include <stdarg.h>
# define Va_start(a,b) va_start(a,b)
#else
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#endif

/*
 * In K&R C, function arguments are all made at least as long as an int.
 * In ANSI-C, using prototypes, it is possible to pass function arguments
 * in their "narrow" form.  This library might be compiled with an ANSI
 * compiler, but linked with code compiled with K&R, so we have to be
 * careful not to use narrow arguments in this way, unless we are
 * explicitly told not to worry about it.
 *
 * Client code generally doesn't need to worry about being compiled with
 * mixed compilers, so you shouldn't have to use these.  If you do, only
 * use them in the prototypes for function definitions and declarations,
 * not as variables or structure fields.
 */
#if NeedWidePrototypes /* almost always defined to True */
#define XmtWideBoolean int
#define XmtWideDimension unsigned int
#define XmtWidePosition int
#else
#define XmtWideBoolean Boolean
#define XmtWideDimension Dimension
#define XmtWidePosition Position
#endif

/*
 * if compiling with gcc version 2, there are some cool type-checking and
 * optimizations things we can do, through gcc extensions to the C grammar.
 * use -Wformat (or -Wall) to get warnings about bad printf style args.
 */
#if defined(__GNUC__) && __GNUC__ >= 2 && !defined(__STRICT_ANSI__)
/*
 * these first two are superseded in gcc 2.5, and are no longer
 * used anywhere in Xmt.  They remain here in case someone else
 * started using them, though.
 */
#define gcc_const_func const
#define gcc_volatile_func volatile
#define gcc_printf_func(format_arg_num, first_vararg_num)\
   __attribute__((format (printf, format_arg_num, first_vararg_num)))
#if defined(__GNUC_MINOR__) && __GNUC_MINOR__ >= 5 && !defined(__STRICT_ANSI__)
/*
 * In gcc 2.5 and later, we can define these to help the compiler optimize.
 * They are actually used in Xmt, so compiling with gcc 2.5 gives a minor
 * optimization boost.
 */
#define gcc_const_attribute __attribute__((const))
#define gcc_volatile_attribute __attribute__((noreturn))
#else
/* if not gcc 2.5 or later, these two aren't defined */
#define gcc_const_attribute
#define gcc_volatile_attribute
#endif
#else
/* if not gcc 2.0 or later , none of these are defined */
#define gcc_const_func
#define gcc_volatile_func
#define gcc_printf_func(a,b)
#define gcc_const_attribute
#define gcc_volatile_attribute
#endif

/*
 * In X11R5, Intrinsic.h includes Xosdefs.h which defines the symbols
 * X_NOT_STDC_ENV and X_NOT_POSIX_ENV for systems without the standard
 * C and Posix header files.  These are useful to know when we can
 * to include <stdlib.h>, for example, and when we should just declare
 * the functions we want explicitly.
 *
 * X_NOT_STDC_ENV means does not have ANSI C header files.  Lack of this
 * symbol does NOT mean that the system has stdarg.h.
 *
 * X_NOT_POSIX means does not have POSIX header files.  Lack of this
 * symbol does NOT mean that the POSIX environment is the default.
 * You may still have to define _POSIX_SOURCE to get it.
 *
 * For X11R4, we just include the contents of the X11R5 file here
 * See the file COPYRIGHT for the applicable MIT copyright notice.
 */
#ifndef X11R5
#ifndef _XOSDEFS_H_
#define _XOSDEFS_H_

#ifdef NOSTDHDRS
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif

#ifdef sony
#ifndef SYSTYPE_SYSV
#define X_NOT_POSIX
#endif
#endif

#ifdef UTEK
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif

#ifdef CRAY
#define X_NOT_POSIX
#endif

#ifdef vax
#ifndef ultrix			/* assume vanilla BSD */
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif
#endif

#ifdef luna
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif

#ifdef Mips
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif
  
#ifdef USL
#ifdef SYSV /* (release 3.2) */
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif
#endif

#ifdef SYSV386
#ifdef SYSV
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif
#endif

#ifdef MOTOROLA
#ifdef SYSV
#define X_NOT_STDC_ENV
#endif
#endif
#endif /* _XOSDEFS_H_ */
#endif

/*
 * In Unix, app-defaults files have the suffix ".ad".  In VMS
 * they have the suffix ".dat".  We define a symbolic constant
 * to take care of this difference and use it in ContextHelp.c
 * and Include.c
 */
#ifndef APPDEFAULTSSUFFIX
#ifndef VMS
#define APPDEFAULTSSUFFIX ".ad"
#else
#define APPDEFAULTSSUFFIX ".dat"
#endif
#endif

/*
 * some special representation types used in the library
 */

/* A special type used in a number of places; different from XtRString */
#ifndef XmtRBuffer
#define XmtRBuffer "Buffer"
#endif

/* Xt doesn't define an XtRDouble */
#ifndef XmtRDouble
#define XmtRDouble "Double"
#endif

/*
 * This macro is an analog to XtOffsetOf.  Instead of returning the
 * offset of a field in a structure of the specified type, however,
 * it returns the size of that field.  It is particularly useful when
 * declaring resource lists with XmtRBuffer resources, because we can't
 * just use sizeof() with fields like "char name[40];"
 */
#define XmtSizeOf(type, field) sizeof(((type *)0)->field)

/*
 * DECWindows can't handle XtRemoveCallback, so we make these patches:
 */
#if DECWINDOWS_CALLBACK_HACK
#define XtAddCallback _XmtDECAddCallback
#define XtRemoveCallback _XmtDECRemoveCallback
#if NeedFunctionPrototypes
extern void _XmtDECAddCallback(Widget, String, XtCallbackProc, XtPointer);
extern void _XmtDECRemoveCallback(Widget, String, XtCallbackProc, XtPointer);
#else
extern void _XmtDECAddCallback();
extern void _XmtDECRemoveCallback();
#endif
#endif

/*
 * Here is some portability stuff for Motif 1.1
 */
#if XmVersion == 1001
#define XmGetPixmapByDepth(s,i,f,b,d) XmGetPixmap(s,i,f,b)
#endif

/*
 * Some systems (e.g. SunOS 4.1.3_U1) still don't have memmove()
 */
#ifdef NO_MEMMOVE
#undef memmove
#define memmove(a,b,n) bcopy(b,a,n)
#endif

/*
 * Some common Xmt functions
 */
#include <Xmt/Util.h>

#endif /* _Xmt_h */
