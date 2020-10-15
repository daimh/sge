#ifndef __SGE_ANSWER_H
#define __SGE_ANSWER_H
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

#include "sge.h"
#include "sgeobj/sge_answer_AN_L.h"

typedef enum {
   ANSWER_QUALITY_CRITICAL = 0,
   ANSWER_QUALITY_ERROR = 1,
   ANSWER_QUALITY_WARNING = 2,
   ANSWER_QUALITY_INFO = 3,
   ANSWER_QUALITY_END = 4  /* needs to be last element */
} answer_quality_t;

/* 
 * valid values for AN_status 
 */
typedef enum {
   STATUS_OK = 1,            /* everything was fine */
   STATUS_ESEMANTIC,         /* semantic error */
   STATUS_EEXIST,            /* elem does not exist OR it exists for a
                              * "add" request */
   STATUS_EUNKNOWN,          /* unknown error occured */
   STATUS_ENOIMP,            /* command not implemented for target */
   STATUS_ENOKEY,            /* missing key field in case of add,del,mod */
   STATUS_ESYNTAX,           /* syntax error parsing a_source field */
   STATUS_EDENIED2HOST,      /* operation denied to this host */
   STATUS_ENOMGR,            /* operation needs manager privileges */
   STATUS_ENOOPR,            /* operation needs operator privileges */
   STATUS_NOQMASTER,         /* failed to reach sge_qmaster */
   STATUS_NOCOMMD,           /* failed to reach commd */
   STATUS_NOCONFIG,          /* missing dir/conf (cell, common, bootstrap) */
   STATUS_EDISK,             /* disk operation failed */
   STATUS_ENOSUCHUSER,       /* can't resolve user */
   STATUS_NOSUCHGROUP,       /* can't resolve group */
   STATUS_EMALLOC,           /* can't allocate memory */
   STATUS_ENOTOWNER,         /* need to be owner for this operation */
   STATUS_ESUBHLIC,          /* too few submit host licenses */
   STATUS_DENIED,            /* not allowed to do whatever you try */
   STATUS_EVERSION,          /* qmaster GDI version differs from clients
                              * GDI version */
   STATUS_ERROR1,            /* general error 1 */
   STATUS_ERROR2,            /* general error 2 */
   STATUS_ERROR3,            /* general error 3 */
   STATUS_OK_DOAGAIN=24,     /* 
                              * everything was fine but transaction
                              * was not completly finished. 
                              */
   STATUS_NOTOK_DOAGAIN=25   /*
                              * transaction was rejected. Try again later 
                              * (value will be used as return value for
                              * qsub)
                              */
} an_status_t;


bool answer_has_quality(const lListElem *answer, answer_quality_t quality);

void answer_exit_if_not_recoverable(const lListElem *answer);

const char *answer_get_quality_text(const lListElem *answer);

u_long32 answer_get_status(const lListElem *answer);

void answer_print_text(const lListElem *answer, 
                       FILE *stream,
                       const char *prefix,
                       const char *suffix);

void answer_to_dstring(const lListElem *answer, dstring *diag);

void answer_list_to_dstring(const lList *alp, dstring *diag);

bool answer_list_add_sprintf(lList **answer_list, u_long32 status, 
                             answer_quality_t quality, const char *fmt, ...)
  __attribute__ ((format (printf, 4, 5)));

bool answer_list_has_quality(lList **answer_list, 
                             answer_quality_t quality);

void answer_list_remove_quality(lList *answer_list, answer_quality_t quality);


bool answer_list_has_status(lList **answer_list, 
                            u_long32 status);

bool answer_list_has_error(lList **answer_list);

void answer_list_on_error_print_or_exit(lList **answer_list, FILE *stream);

int answer_list_print_err_warn(lList **answer_list,
                               const char *critical_prefix,
                               const char *err_prefix,
                               const char *warn_prefix);

bool answer_list_output(lList **answer_list);

bool answer_list_log(lList **answer_list, bool is_free_list, bool show_info);

int answer_list_handle_request_answer_list(lList **answer_list, FILE *stream);

bool answer_list_add(lList **answer_list, const char *text,
                     u_long32 status, answer_quality_t quality);

bool answer_list_add_elem(lList **answer_list, lListElem *answer);

void answer_list_replace(lList **answer_list, lList **new_list);

void answer_list_append_list(lList **answer_list, lList **new_list);

int show_answer(lList *alp);

int show_answer_list(lList *alp);


#endif /* __SGE_ANSWER_H */
