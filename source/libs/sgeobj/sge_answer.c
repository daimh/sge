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

#include <stdarg.h>
#include <string.h>

#include "cull/cull.h"

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_dstring.h"
#include "uti/sge_log.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "sge.h"

#define ANSWER_LAYER CULL_LAYER

static bool answer_is_recoverable(const lListElem *answer);
static bool answer_log(lListElem *answer, bool show_info);

/****** sgeobj/answer/-AnswerList *********************************************
*  NAME
*     AnswerList - Object used to return errors/warning/info
*
*  FUNCTION
*     Answer elements and lists are used to exchange information
*     about the level of success of an operation between caller
*     and callee. Especially in GDI answers, elements and lists are
*     used to transfer error information from server components
*     to clients.
*
*     Example:
*
*        void caller(void) {
*           lList *answer_list = NULL; 
*
*           callee(&answer_list);
*           if (answer_list_has_error(&answer_list)) {
*              try_to_handle_error();
*              lFreeList(&answer_list);
*           }
*        } 
*
*        void callee(lList **answer_list) {
*           char *s = malloc(256);
*
*           if (s == NULL) {
*              answer_list_add(answer_list, "no memory", 
*                              STATUS_ERROR, ANSWER_QUALITY_ERROR);
*              return;
*           }
*        }
*
*  SEE ALSO
*     sgeobj/answer/answer_has_quality() 
*     sgeobj/answer/answer_is_recoverable() 
*     sgeobj/answer/answer_exit_if_not_recoverable() 
*     sgeobj/answer/answer_get_quality_text() 
*     sgeobj/answer/answer_get_status() 
*     sgeobj/answer/answer_print_text() 
*     sgeobj/answer/answer_list_add() 
*     sgeobj/answer/answer_list_add_sprintf() 
*     sgeobj/answer/answer_list_has_quality() 
*     sgeobj/answer/answer_list_has_error() 
*     sgeobj/answer/answer_list_on_error_print_or_exit() 
*
*  NOTES
*     MT-NOTE: the answer list module is MT safe
******************************************************************************/

/****** sgeobj/answer/answer_has_quality() ************************************
*  NAME
*     answer_has_quality() -- Check for certain answer quality 
*
*  SYNOPSIS
*     bool answer_has_quality(const lListElem *answer, 
*                            answer_quality_t quality) 
*
*  FUNCTION
*     Return true (1) if "answer" has the given "quality" 
*
*  INPUTS
*     const lListElem *answer  - AN_Type element 
*     answer_quality_t quality - Quality id 
*
*  RESULT
*     bool - true or false 
*
*  NOTES
*     MT-NOTE: answer_has_quality() is MT safe
*******************************************************************************/
bool answer_has_quality(const lListElem *answer, answer_quality_t quality) 
{
   bool ret;

   DENTER(ANSWER_LAYER, "answer_has_quality");
   ret = (lGetUlong(answer, AN_quality) ==  quality) ? true : false;
   DEXIT;
   return ret;
}

/****** sgeobj/answer/answer_is_recoverable() *********************************
*  NAME
*     answer_is_recoverable() -- Check for recoverable error 
*
*  SYNOPSIS
*     int answer_is_recoverable(const lListElem *answer) 
*
*  FUNCTION
*     This function returns true (1) if "answer" is an error which the
*     calling function application may recover from.
*     The following errors are handled as nonrecoverable:
*
*        STATUS_NOQMASTER
*        STATUS_NOCOMMD
*        STATUS_ENOKEY 
*
*  INPUTS
*     const lListElem *answer - AN_Type element 
*
*  RESULT
*     int - true or false
*
*  NOTES
*     MT-NOTE: answer_is_recoverable() is MT safe
******************************************************************************/
static bool answer_is_recoverable(const lListElem *answer)
{
   bool ret = true;

   DENTER(ANSWER_LAYER, "answer_is_recoverable");
   if (answer != NULL) {
      const int max_non_recoverable = 4;
      const u_long32 non_recoverable[] = {
         STATUS_NOQMASTER,
         STATUS_NOCOMMD,
         STATUS_ENOKEY,
         STATUS_NOCONFIG
      };
      u_long32 status = lGetUlong(answer, AN_status);
      int i;

      for (i = 0; i < max_non_recoverable; i++) {
         if (status == non_recoverable[i]) {
            ret = false;
            break;
         }
      }
   }
   DEXIT;
   return ret;
}

/****** sgeobj/answer/answer_exit_if_not_recoverable() ************************
*  NAME
*     answer_exit_if_not_recoverable() -- Exit on certain errors 
*
*  SYNOPSIS
*     void answer_exit_if_not_recoverable(const lListElem *answer) 
*
*  FUNCTION
*     This function checks if "answer" is an unrecoverable error.
*     (answer_is_recoverable() is used to check this.) The error
*     text will then be printed to stderr and the calling
*     process will be terminated with exit code 1. 
*
*  INPUTS
*     const lListElem *answer - AN_Type element 
*
*  NOTES
*     This function may never return. 
*
*  NOTES
*     MT-NOTE: answer_exit_if_not_recoverable() is MT safe
******************************************************************************/
void answer_exit_if_not_recoverable(const lListElem *answer)
{
   DENTER(ANSWER_LAYER, "answer_exit_if_not_recoverable");
   if (!answer_is_recoverable(answer)) {
      fprintf(stderr, "%s: %s\n", answer_get_quality_text(answer),
              lGetString(answer, AN_text));
      DEXIT;
      SGE_EXIT(NULL, 1);
   }
   DEXIT;
}

/****** sgeobj/answer/answer_get_quality_text() *******************************
*  NAME
*     answer_get_quality_text() -- Get quality text 
*
*  SYNOPSIS
*     const char* answer_get_quality_text(const lListElem *answer) 
*
*  FUNCTION
*     Returns a string representation for the quality of the "answer" 
*
*  INPUTS
*     const lListElem *answer - AN_Type list 
*
*  RESULT
*     const char* - String
*
*  NOTES
*     MT-NOTE: answer_get_quality_text() is MT safe
******************************************************************************/
const char *answer_get_quality_text(const lListElem *answer) 
{
   const char *quality_text[] = {
      "CRITICAL",
      "ERROR",
      "WARNING",
      "INFO"
   };
   u_long32 quality;

   DENTER(ANSWER_LAYER, "answer_get_quality_text");
   quality = lGetUlong(answer, AN_quality);
   if (quality >= ANSWER_QUALITY_END) {
      quality = ANSWER_QUALITY_CRITICAL;
   }
   DEXIT;
   return quality_text[quality];
}

/****** sgeobj/answer/answer_get_status() *************************************
*  NAME
*     answer_get_status() -- Return the error status.
*
*  SYNOPSIS
*     u_long32 answer_get_status(const lListElem *answer) 
*
*  FUNCTION
*     Return the error status of "answer". 
*
*  INPUTS
*     const lListElem *answer - AN_Type element 
*
*  RESULT
*     u_long32 - error status
*
*  NOTES
*     MT-NOTE: answer_get_status() is MT safe
******************************************************************************/
u_long32 answer_get_status(const lListElem *answer) 
{
   u_long32 ret;

   DENTER(ANSWER_LAYER, "answer_get_status");
   ret = lGetUlong(answer, AN_status);
   DEXIT;
   return ret;
}

/****** sgeobj/answer/answer_print_text() *************************************
*  NAME
*     answer_print_text() -- Prints error text 
*
*  SYNOPSIS
*     void answer_print_text(const lListElem *answer, 
*                            FILE *stream
*                            const char *prefix,
*                            const char *suffix) 
*
*  FUNCTION
*     Prints "prefix", the error text of "answer" and "suffix" 
*     to "stream".
*
*  INPUTS
*     const lListElem *answer - AN_Type element 
*     FILE *stream            - Output stream 
*     const char *prefix      - Introductional message
*     const char *prefix      - Final message
*
*  NOTES
*     MT-NOTE: answer_print_text() is MT safe
******************************************************************************/
void answer_print_text(const lListElem *answer, 
                       FILE *stream, 
                       const char *prefix,
                       const char *suffix)
{
   const char *text = NULL;

   DENTER(ANSWER_LAYER, "answer_print_text");
   text = lGetString(answer, AN_text);

   if (prefix != NULL) {
      fprintf(stream, "%s", prefix);
   }
   if (text != NULL) {
      fprintf(stream, "%s", text);
   }
   if (suffix != NULL) {
      fprintf(stream, "%s", suffix);
   }
   fprintf(stream, "\n");
   DEXIT;
}

/****** sgeobj/answer/answer_to_dstring() ************************************
*  NAME
*     answer_to_dstring() -- Copy answer to dstring without newline
*
*  SYNOPSIS
*     void answer_to_dstring(const lListElem *answer, dstring *diag) 
*
*  FUNCTION
*     Copy answer text into dstring without newline character.
*
*  INPUTS
*     const lListElem *answer - AN_Type element
*
*  OUTPUT
*     dstring *diag           - destination dstring
*
*  RESULT
*     void - 
*
*  NOTES
*     MT-NOTE: answer_to_dstring() is MT safe
*******************************************************************************/
void answer_to_dstring(const lListElem *answer, dstring *diag)
{
   if (diag) {
      if (!answer) {
         sge_dstring_copy_string(diag, MSG_ANSWERWITHOUTDIAG);
      } else {
         const char *s, *t;
         s = lGetString(answer, AN_text);
         if ((t=strchr(s, '\n'))) {
            sge_dstring_sprintf_append(diag, "%.*s", (int) (t-s), s);
         }
         else {
            sge_dstring_append(diag, s);
         }   
      }
   }
}

/****** sgeobj/answer/answer_list_to_dstring() *********************************
*  NAME
*     answer_list_to_dstring() -- Copy answer to dstring without newline
*
*  SYNOPSIS
*     void answer_list_to_dstring(const lList *alp, dstring *diag) 
*
*  FUNCTION
*     Copy answer list text into dstring with each element separated by a
*     newline character.
*
*  INPUTS
*     const lList *alp        - AN_Type list
*     dstring *diag           - destination dstring
*
*  RESULT
*     void - 
*
*  NOTES
*     MT-NOTE: answer_list_to_dstring() is MT safe
*******************************************************************************/
void answer_list_to_dstring(const lList *alp, dstring *diag)
{
   if (diag) {
      if (!alp || (lGetNumberOfElem (alp) == 0)) {
         sge_dstring_copy_string(diag, MSG_ANSWERWITHOUTDIAG);
      } else {
         lListElem *aep = NULL;
         
         sge_dstring_clear(diag);
         
         for_each(aep, alp) {
            const char *s;

            s = lGetString(aep, AN_text);
            sge_dstring_append(diag, s);

            if (strchr(s, '\n') == NULL) {
               sge_dstring_append_char(diag, '\n');
            }
         }
      }
   }
}

/****** sgeobj/answer/answer_list_add_sprintf() *******************************
*  NAME
*     answer_list_add_sprintf() -- Format add an answer to an answer list
*
*  SYNOPSIS
*     bool answer_list_add_sprintf(lList **answer_list, u_long32 status, 
*                                  answer_quality_t quality, const char *fmt, 
*                                  ...) 
*
*  FUNCTION
*     This function creates a new answer element having the properties
*     "status", "quality", and a message text created from fmt and the 
*     following variable argument list.
*
*     The new element will be appended to "answer_list".
*     
*     If "answer_list" is NULL, no action is performed.
*
*     If the list pointed to by "answer_list" is NULL, a new list will be
*     created.
*
*  INPUTS
*     lList **answer_list      - AN_Type list
*     u_long32 status          - answer status
*     answer_quality_t quality - answer quality
*     const char *fmt          - format string to create message (printf)
*     ...                      - arguments used for formatting message
*
*  RESULT
*     bool - true on success, else false
*
*  SEE ALSO
*     sgeobj/answer/answer_list_add()
*
*  NOTES
*     MT-NOTE: answer_list_add_sprintf() is MT safe
******************************************************************************/
bool 
answer_list_add_sprintf(lList **answer_list, u_long32 status, 
                        answer_quality_t quality, const char *fmt, ...)
{
   bool ret = false;

   DENTER(ANSWER_LAYER, "answer_list_add");
   if (answer_list != NULL) {
      dstring buffer = DSTRING_INIT;
      const char *message;
      va_list ap;

      va_start(ap, fmt);
      message = sge_dstring_vsprintf(&buffer, fmt, ap);
      va_end(ap);

      if (message != NULL) {
         ret = answer_list_add(answer_list, message, status, quality);
      }

      sge_dstring_free(&buffer);
   }

   DRETURN(ret);
}

/****** sgeobj/answer/answer_list_has_quality() *******************************
*  NAME
*     answer_list_has_quality() -- Contains list  
*
*  SYNOPSIS
*     int answer_list_has_quality(lList **answer_list, 
*                                 answer_quality_t quality) 
*
*  FUNCTION
*     The function returns true (1) if the "answer_list" contains
*     at least one answer element with the given "quality". 
*
*  INPUTS
*     lList **answer_list      - AN_Type list 
*     answer_quality_t quality - quality value 
*
*  RESULT
*     bool - true or false
*
*  NOTES
*     MT-NOTE: answer_list_has_quality() is MT safe
******************************************************************************/
bool answer_list_has_quality(lList **answer_list, answer_quality_t quality)
{
   bool ret = false;

   DENTER(ANSWER_LAYER, "answer_list_has_quality");
   if (answer_list != NULL) {
      lListElem *answer;   /* AN_Type */

      for_each(answer, *answer_list) {
         if (answer_has_quality(answer, quality)) {
            ret = true;
            break;
         }
      }
   }
   DRETURN(ret);
}

/****** sge_answer/answer_list_remove_quality() ********************************
*  NAME
*     answer_list_remove_quality() -- Remove elements from list
*
*  SYNOPSIS
*     void answer_list_remove_quality(lList *alp, answer_quality_t quality) 
*
*  FUNCTION
*     The function removes all answer list elements with the given quality from
*     the list.
*
*  INPUTS
*     lList *answer_list       - AN_Type list   
*     answer_quality_t quality - quality value
*
*  RESULT
*     void - 
*
*  NOTES
*     MT-NOTE: answer_list_remove_quality() is MT safe 
*******************************************************************************/
void answer_list_remove_quality(lList *answer_list, answer_quality_t quality)
{
   lListElem *aep, *nxt = lFirst(answer_list);

   DENTER(ANSWER_LAYER, "answer_list_remove_quality");

   while ((aep=nxt)) {
      nxt=lNext(aep);
      if (lGetUlong(aep, AN_quality) == quality) {
         lRemoveElem(answer_list, &aep);
      }
   }

   DRETURN_VOID;
}


/****** sge_answer/answer_list_has_status() ************************************
*  NAME
*     answer_list_has_status() -- status contains in list
*
*  SYNOPSIS
*     bool answer_list_has_status(lList **answer_list, u_long32 status) 
*
*  FUNCTION
*     The function returns true if the "answer_list" contains at least
*     one answer element with the given status
*
*  INPUTS
*     lList **answer_list - AN_Type list
*     u_long32 status     - expected status
*
*  RESULT
*     bool - true or false
*
*  NOTES
*     MT-NOTE: answer_list_has_status() is MT safe 
*******************************************************************************/
bool answer_list_has_status(lList **answer_list, u_long32 status)
{
   bool ret = false;

   DENTER(ANSWER_LAYER, "answer_list_has_status");

   if (answer_list != NULL) {
      lListElem *answer;   /* AN_Type */

      for_each(answer, *answer_list) {
         if (answer_get_status(answer) == status) {
            ret = true;
            break;
         }
      }
   }
   DRETURN(ret);
}

/****** sgeobj/answer/answer_list_has_error() *********************************
*  NAME
*     answer_list_has_error() -- Is an "error " in the list 
*
*  SYNOPSIS
*     bool answer_list_has_error(lList **answer_list) 
*
*  FUNCTION
*     The function returns true (1) if the "answer_list" containes
*     at least one error answer element 
*
*  INPUTS
*     lList **answer_list - AN_Type list 
*
*  RESULT
*     bool - true or false
*
*  NOTES
*     MT-NOTE: answer_list_has_error() is MT safe
******************************************************************************/
bool answer_list_has_error(lList **answer_list)
{
   bool ret = false;

   DENTER(ANSWER_LAYER, "answer_list_has_error");
   if ((answer_list_has_quality(answer_list, ANSWER_QUALITY_ERROR) == true) ||
       (answer_list_has_quality(answer_list, ANSWER_QUALITY_CRITICAL) == true)) {
       ret = true;
   }
   DRETURN(ret);
}               

/****** sgeobj/answer/answer_list_on_error_print_or_exit() ********************
*  NAME
*     answer_list_on_error_print_or_exit() -- Print and/or exit 
*
*  SYNOPSIS
*     void answer_list_on_error_print_or_exit(lList **answer_list, 
*                                             FILE *stream) 
*
*  FUNCTION
*     The error texts of all answer elements within "answer_list" are
*     printed to "stream". If an unrecoverable error is detected
*     the calling process will be terminated. 
*
*  INPUTS
*     lList **answer_list - AN_Type list 
*     FILE *stream        - output stream 
*
*  NOTES
*     MT-NOTE: answer_list_on_error_print_or_exit() is MT safe
******************************************************************************/
void answer_list_on_error_print_or_exit(lList **answer_list, FILE *stream)
{
   lListElem *answer;   /* AN_Type */

   DENTER(ANSWER_LAYER, "answer_list_on_error_print_or_exit");
   for_each(answer, *answer_list) {
      answer_exit_if_not_recoverable(answer);
      answer_print_text(answer, stream, NULL, NULL);
   }
   DEXIT;
}

/****** sgeobj/answer/answer_list_print_err_warn() ****************************
*  NAME
*     answer_list_print_err_warn() -- Print and/or exit 
*
*  SYNOPSIS
*     int answer_list_print_err_warn(lList **answer_list, 
*                                    const char *err_prefix, 
*                                    const char *warn_prefix) 
*
*  FUNCTION
*     Prints all messages contained in "answer_list". All error
*     messages will be printed to stderr with an an initial "err_prefix".
*     All warning and info messages will be printed to stdout with
*     the prefix "warn_prefix". 
*
*     If the "answer_list" contains at least one error then this
*     function will return with a positive return value. This value
*     is the errror status of the first error message.
*
*     If there is no error contained in 'answer_list' than this function 
*     will return with a value of 0. 
*
*     "*answer_list" will be freed.
*
*  INPUTS
*     lList **answer_list     - AN_Type list 
*     const char *err_prefix  - e.g. "qsub: "
*     const char *warn_prefix - e.g. MSG_WARNING
*
*  NOTES
*     MT-NOTE: answer_list_print_err_warn() is MT safe
******************************************************************************/
int answer_list_print_err_warn(lList **answer_list, 
                               const char *critical_prefix,
                               const char *err_prefix,
                               const char *warn_prefix)
{
   int do_exit = 0;
   lListElem *answer;   /* AN_Type */
   u_long32 status = 0;

   DENTER(ANSWER_LAYER, "answer_list_print_err_warn");
   for_each(answer, *answer_list) {
      if (answer_has_quality(answer, ANSWER_QUALITY_CRITICAL)) {
         answer_print_text(answer, stderr, critical_prefix, NULL);
         if (do_exit == 0) {
            status = answer_get_status(answer);
            do_exit = 1;
         }
      } else if (answer_has_quality(answer, ANSWER_QUALITY_ERROR)) {
         answer_print_text(answer, stderr, err_prefix, NULL);
         if (do_exit == 0) {
            status = answer_get_status(answer);
            do_exit = 1;
         }
      } else if (answer_has_quality (answer, ANSWER_QUALITY_WARNING)) {
         answer_print_text(answer, stdout, warn_prefix, NULL);
      }
      else {
         answer_print_text(answer, stdout, NULL, NULL);
      }
   }
   lFreeList(answer_list);
   DEXIT;
   return (int)status;
}

/****** sgeobj/answer/answer_list_handle_request_answer_list() ****************
*  NAME
*     answer_list_handle_request_answer_list() -- handle res. of request
*
*  SYNOPSIS
*     int answer_list_handle_request_answer_list(lList **answer_list, 
*                                                FILE *stream) 
*
*  FUNCTION
*     Processes the answer list that results from a GDI request
*     (sge_gdi or sge_gdi_multi).
*     Outputs any errors and warnings and returns the first error
*     or warning status code.
*     The answer list is freed.
*
*  INPUTS
*     lList **answer_list - answer list to process
*     FILE *stream        - output stream
*
*  RESULT
*     int - first error or warning status code or STATUS_OK
*
*  NOTES
*     MT-NOTE: answer_list_handle_request_answer_list() is MT safe
******************************************************************************/
int answer_list_handle_request_answer_list(lList **answer_list, FILE *stream) {
   int ret = STATUS_OK;

   DENTER(ANSWER_LAYER, "answer_list_handle_request_answer_list");
   if(answer_list != NULL && *answer_list != NULL) {
      lListElem *answer;

      for_each(answer, *answer_list) {
         if(answer_has_quality(answer, ANSWER_QUALITY_CRITICAL) ||
            answer_has_quality(answer, ANSWER_QUALITY_ERROR) ||
            answer_has_quality(answer, ANSWER_QUALITY_WARNING)) {
            answer_print_text(answer, stream, NULL, NULL);
            if(ret == STATUS_OK) {
               ret = lGetUlong(answer, AN_status);
            }
         }
      }
      lFreeList(answer_list);
   } else {
      fprintf(stream, "%s\n", MSG_ANSWER_NOANSWERLIST);
      return STATUS_EUNKNOWN;
   }
   DEXIT;
   return ret;
}

/****** sgeobj/answer/answer_list_add() ***************************************
*  NAME
*     answer_list_add() -- Add an answer to an answer list
*
*  SYNOPSIS
*     int answer_list_add(lList **answer_list,
*                         const char *text,
*                         u_long32 status,
*                         answer_quality_t quality)
*
*  FUNCTION
*     This function creates a new answer element (using "quality",
*     "status" and "text"). The new element will be appended to
*     "answer_list"
*
*     If "answer_list" is NULL, no action is performed.
*
*     If the list pointed to by "answer_list" is NULL, a new list will be
*     created.
*
*  INPUTS
*     lList **answer_list      - AN_Type list
*     const char *text         - answer text
*     u_long32 status          - answer status
*     answer_quality_t quality - answer quality
*
*  RESULT
*     int - error state
*        true  - OK
*        false - error occured
*
*  SEE ALSO
*     sgeobj/answer/answer_list_add_sprintf()
*
*  NOTES
*     MT-NOTE: answer_list_add() is MT safe
******************************************************************************/
bool
answer_list_add(lList **answer_list, const char *text,
                u_long32 status, answer_quality_t quality)
{
   bool ret = false;

   DENTER(ANSWER_LAYER, "answer_list_add");

   if (answer_list != NULL) {
      lListElem *answer = lCreateElem(AN_Type);

      if (answer != NULL) {
         lSetString(answer, AN_text, text);
         lSetUlong(answer, AN_status, status);
         lSetUlong(answer, AN_quality, quality);

         if (*answer_list == NULL) {
            *answer_list = lCreateList("", AN_Type);
         }

         if (*answer_list != NULL) {
            lAppendElem(*answer_list, answer);
            ret = true;
         }
      }

      if (!ret) {
         lFreeElem(&answer);
      }
   }
   DRETURN(ret);
}

bool answer_list_add_elem(lList **answer_list, lListElem *answer)
{
   bool ret = false;

   DENTER(ANSWER_LAYER, "answer_list_add_elem");
   if (answer_list != NULL) {
      if (*answer_list == NULL) {
         *answer_list = lCreateList("", AN_Type);
      }
      if (*answer_list != NULL) {
         lAppendElem(*answer_list, answer);
         ret = true;
      }
   }
   DEXIT;
   return ret;
}

/****** sgeobj/answer/answer_list_replace() ***********************************
*  NAME
*     answer_list_replace() -- replace an answer list 
*
*  SYNOPSIS
*     void answer_list_replace(lList **answer_list, lList **new_list) 
*
*  FUNCTION
*     free *answer_list and replace it by *new_list. 
*
*  INPUTS
*     lList **answer_list - AN_Type 
*     lList **new_list    - AN_Type 
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: answer_list_replace() is MT safe
*******************************************************************************/
void answer_list_replace(lList **answer_list, lList **new_list)
{
   DENTER(ANSWER_LAYER, "answer_list_replace");
   if (answer_list != NULL) {
      lFreeList(answer_list);

      if (new_list != NULL) {
         *answer_list = *new_list; 
         *new_list = NULL;
      } else {
         *answer_list = NULL; 
      }
   }
   DEXIT;
}

/****** sgeobj/answer/answer_list_append_list() *******************************
*  NAME
*     answer_list_append_list() -- Append two lists 
*
*  SYNOPSIS
*     void answer_list_append_list(lList **answer_list, lList **new_list) 
*
*  FUNCTION
*     Append "new_list" after "answer_list". *new_list will be NULL afterwards 
*
*  INPUTS
*     lList **answer_list - AN_Type list 
*     lList **new_list    - AN_Type list 
*
*  RESULT
*     void - None 
*******************************************************************************/
void answer_list_append_list(lList **answer_list, lList **new_list)
{
   DENTER(ANSWER_LAYER, "answer_list_append_list");
   if (answer_list != NULL && new_list != NULL) {
      if (*answer_list == NULL && *new_list != NULL) {
         *answer_list = lCreateList("", AN_Type);
      }
      if (*new_list != NULL) {
         lAddList(*answer_list, new_list);
      }
   }
   DEXIT;
}

/****** sgeobj/answer/answer_list_log() ****************************
*  NAME
*     answer_list_log() -- output and free answer_list
*
*  SYNOPSIS
*     bool
*     answer_list_log(lList **answer_list, bool is_free_list, bool show_info)
*
*  FUNCTION
*     Prints all messages contained in "answer_list". 
*     The ERROR, WARNING and INFO macros will be used for output.
*
*     If the "answer_list" contains at least one error then this
*     function will return true.
*
*     If there is no error contained in 'answer_list' then this function 
*     will return with a value of false. 
*
*     "*answer_list" will only be freed and set to NULL, if is_free_list is
*     true.
*
*  INPUTS
*     lList **answer_list     - AN_Type list 
*     bool is_free_list       - if true, frees the answer list
*     bool show_info          - log also info messages
*
*  NOTES
*     MT-NOTE: answer_list_print_err_warn() is MT safe
******************************************************************************/
bool answer_list_log(lList **answer_list, bool is_free_list, bool show_info) {

   bool ret = false;
   lListElem *answer;   /* AN_Type */

   DENTER(ANSWER_LAYER, "answer_list_log");

   if (answer_list != NULL && *answer_list != NULL) {
      for_each(answer, *answer_list) {
         ret = answer_log(answer, show_info);
      }
      if (is_free_list) {
         lFreeList(answer_list);
      }
   }

   DEXIT;
   return ret;
}

/****** sgeobj/answer/answer_log() ****************************
*  NAME
*     answer_log() -- output answer
*
*  SYNOPSIS
*     bool
*     answer_log(lListElem *answer)
*
*  FUNCTION
*     Prints the message contained in "answer". 
*     The CRITICAL, ERROR, WARNING and INFO macros will be used for output.
*
*  INPUTS
*     lListElem *answer       - AN_Type element 
*
*  RESULT
*     bool - true if answer is an error, false otherwise and if answer == NULL 
*
*  NOTES
*     MT-NOTE: answer_log() is MT safe
******************************************************************************/
static bool answer_log(lListElem *answer, bool show_info) {

   bool ret = false;

   DENTER(ANSWER_LAYER, "answer_log");

   if (!answer) {
      DRETURN(ret);
   }

   switch (lGetUlong(answer, AN_quality)) {
      case ANSWER_QUALITY_CRITICAL:
         CRITICAL((SGE_EVENT, SFNMAX, lGetString(answer, AN_text)));
         ret = true;
         break;
      case ANSWER_QUALITY_ERROR:
         ERROR((SGE_EVENT, SFNMAX, lGetString(answer, AN_text)));
         ret = true;
         break;
      case ANSWER_QUALITY_WARNING:
         WARNING((SGE_EVENT, SFNMAX, lGetString(answer, AN_text)));
         break;
      case ANSWER_QUALITY_INFO:
         if (show_info) {
            INFO((SGE_EVENT, SFNMAX, lGetString(answer, AN_text)));
         }
         break;
      default:
         break;
   }

   DRETURN(ret);
}

/****** sgeobj/answer/answer_list_output() ****************************
*  NAME
*     answer_list_output() -- output and free answer_list
*
*  SYNOPSIS
*     bool
*     answer_list_output(lList **answer_list)
*
*  FUNCTION
*     Prints all messages contained in "answer_list". 
*     The ERROR, WARNING and INFO macros will be used for output.
*
*     If the "answer_list" contains at least one error then this
*     function will return true.
*
*     If there is no error contained in 'answer_list' then this function 
*     will return with a value of false. 
*
*     "*answer_list" will be freed and set to NULL.
*
*  INPUTS
*     lList **answer_list     - AN_Type list 
*
*  NOTES
*     MT-NOTE: answer_list_output() is MT safe
******************************************************************************/
bool answer_list_output(lList **answer_list) {
   return answer_list_log(answer_list, true, true);
}


int show_answer(lList *alp) 
{
   lListElem *aep = NULL;
   int ret = 0;
   
   DENTER(TOP_LAYER, "show_answer");
   
   if (alp != NULL) {
    
      for_each(aep,alp) {
         answer_exit_if_not_recoverable(aep);
         if (lGetUlong(aep, AN_status) != STATUS_OK) {
            ret = 1;
         }
      }
      aep = lLast(alp);
      if (lGetUlong(aep, AN_quality) != ANSWER_QUALITY_END) {
         fprintf(stderr, "%s\n", lGetString(aep, AN_text));
      }
   }
   
   DRETURN(ret);
}

int show_answer_list(lList *alp) 
{
   lListElem *aep = NULL;
   int ret = 0;
   
   DENTER(TOP_LAYER, "show_answer_list");
   
   if (alp != NULL) {
      for_each(aep,alp) {
         if (lGetUlong(aep, AN_quality) == ANSWER_QUALITY_END) {
            continue;
         }

         answer_exit_if_not_recoverable(aep);
         if (lGetUlong (aep, AN_status) != STATUS_OK) {
            ret = 1;
         }
         fprintf(stderr, "%s\n", lGetString(aep, AN_text));
      }
   }
   
   DRETURN(ret);
}

