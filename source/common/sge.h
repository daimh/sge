#ifndef __SGE_H
#define __SGE_H
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

#define MAX_SEQNUM        9999999

/* template/global/default/queue names */
#define SGE_TEMPLATE_NAME        "template"
#define SGE_GLOBAL_NAME          "global"

/* attribute names of sge objects */
#define SGE_ATTR_LOAD_FORMULA          "load_formula"
#define SGE_ATTR_LOAD_SCALING          "load_scaling"
#define SGE_ATTR_COMPLEX_VALUES        "complex_values"
#define SGE_ATTR_LOAD_VALUES           "load_values"
#define SGE_ATTR_USER_LISTS            "user_lists"
#define SGE_ATTR_XUSER_LISTS           "xuser_lists"
#define SGE_ATTR_SLOTS                 "slots"
#define SGE_ATTR_H_RT                  "h_rt"
#define SGE_ATTR_S_RT                  "s_rt"

/* attribute values for certain object attributes */

#define DEFAULT_ACCOUNT           "sge"
#define DEFAULT_CELL              "default"
#define ACTIVE_DIR                "active_jobs"

/* These files exist in the qmaster and execd spool area */
#define EXEC_DIR                  "job_scripts"
#define ERR_FILE                  "messages"

/* Possibly supported by other compilers too.  */
#ifndef __attribute__
/* According to glibc ansidecl.h, this gives us at least attributes
   format (on pointer), unused, noreturn, malloc, pure.  Check before
   adding others.  */
# if (__GNUC__ * 1000 + __GNUC_MINOR__ < 3001) || __STRICT_ANSI__
#  define __attribute__(x)
# endif
#endif

#define _UNUSED __attribute__((unused))

#endif /* __SGE_H */
