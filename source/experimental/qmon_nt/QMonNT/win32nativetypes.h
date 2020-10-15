#ifndef __WIN32NATIVETYPES_H__
#define __WIN32NATIVETYPES_H__
/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#ifdef WIN32NATIVE

typedef unsigned short u_short;
typedef unsigned short ushort;
typedef unsigned long  u_long;
typedef unsigned int   u_int;
typedef unsigned short uid_t;	/* FIXME: wrong typ! */
typedef unsigned short gid_t;	/* FIXME: wrong typ! */
typedef unsigned short rlim_t;	/* FIXME: wrong typ! */
typedef unsigned short sigset_t;/* FIXME: wrong typ! */

#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERR         3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */

#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)

#define strcasecmp( a, b) stricmp( a, b)
#define strncasecmp( a, b, n) strnicmp( a, b, n)

#endif /* WIN32NATIVE */

#endif /* __WIN32NATIVETYPES_H__ */
