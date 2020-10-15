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

#include <ctype.h>
#include <string.h>

#include "cull/cull_list.h"

#include "comm/cl_communication.h" 

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"

#include "sgeobj/parse.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_id.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "msg_common.h"
#include "msg_qmaster.h"

/* 7-bit character test, equivalent to isascii, if that was portable.  */
#define	asciichar(c) (((c) & ~0x7f) == 0)

/****** sge_utility/verify_str_key() *******************************************
*  NAME
*     verify_str_key() -- Generic function for verifying object names
*
*  SYNOPSIS
*     an_status_t verify_str_key(lList **alpp, const char *str, size_t 
*     str_length, const char *name, int table) 
*
*  FUNCTION
*     Verifies object names. The following verification tables are
*     used
*
*        QSUB_TABLE    job account strings 
*           (see qsub(1) -A for characters not allowed)
*        QSUB_TABLE    job name
*           (see qsub(1) -N for characters not allowed)
*        KEY_TABLE     parallel environment names
*           (see sge_pe(5))
*        KEY_TABLE     calendar names 
*           (see calendar_conf(5))
*        KEY_TABLE     cluster queue names 
*           (see queue_conf(5))
*        KEY_TABLE     project names
*           (see project(5))
*        KEY_TABLE     userset names
*           (see access_list(5))
*        KEY_TABLE     department names
*           (see access_list(5))
*        KEY_TABLE     checkpoint interface name
*           (see checkpoint(5))
*        KEY_TABLE     user name
*           (see user(5))
*        KEY_TABLE     hostgroup names
*           (see hostgroup(5))
*        WC_TABLE      wc_pe_names (see sge_types(5))
*
*        KEY_TABLE     event client name (internal purposes only)
*        KEY_TABLE     JAPI session key (internal purposes only)
*
*           Note, there is test_sge_utility
*
*  INPUTS
*     lList **alpp      - answer list
*     const char *str   - string to be verified
*     size_t str_length - length of the string to be verified
*     const char *name  - verbal description of the object
*     unsigned table    - verification table to be used
*
*  RESULT
*     an_status_t - STATUS_OK upon success
*
*  NOTES
*     MT-NOTE: verify_str_key() is MT safe 
* 
*  SEE ALSO
*     There is a module test (test_sge_utility) for verify_str_key().
*******************************************************************************/
an_status_t verify_str_key(
   lList **alpp, const char *str, size_t str_length, const char *name, unsigned table) 
{
   static const char *begin_strings[3][3];
   static const char *mid_strings[3][20];

   static char begin_chars[3][3] =
      { { '.', '#', 0 },
        { 0, 0, 0 },
        { '.', '#', 0 } };

   static const char mid_characters[3][20] =
      { { '\n', '\t', '\r', ' ', '/', ':', '\'', '\"', '\\', '[', ']',
          '{', '}', '|', '(', ')', '@', '%', ',', 0},  /* KEY_TABLE  */
        { '\n', '\t', '\r', '/', ':', '@', '\\', '*', '?', 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0}, /* QSUB_TABLE */
        { '\n', '\t', '\r', ' ', '/', ':', '\'', '\"', '\\', '{', '}',
          '|', '(', ')', '@', '%', ',', 0, 0, 0}}; /* WC_TABLE */
   static const char* keyword[] = { "NONE", "ALL", "TEMPLATE", NULL };
   static const char* keyword_strings[4];

   static int initialized = 0;
   char forbidden_char;
   const char* forbidden_string;
   int i;

   /* Presumably the programming error will be obvious.  */
   if (table > WC_TABLE) return STATUS_EUNKNOWN;

   table = table -1;
   if (!initialized) {
      begin_strings[0][0] = MSG_GDI_KEYSTR_DOT;
      begin_strings[0][1] = MSG_GDI_KEYSTR_HASH;
      begin_strings[0][2] = NULL;
      begin_strings[1][0] = NULL;
      begin_strings[1][1] = NULL;
      begin_strings[1][2] = NULL;
      begin_strings[2][0] = MSG_GDI_KEYSTR_DOT;
      begin_strings[2][1] = MSG_GDI_KEYSTR_HASH;
      begin_strings[2][2] = NULL;

      mid_strings[0][0] = MSG_GDI_KEYSTR_RETURN;
      mid_strings[0][1] = MSG_GDI_KEYSTR_TABULATOR;
      mid_strings[0][2] = MSG_GDI_KEYSTR_CARRIAGERET;
      mid_strings[0][3] = MSG_GDI_KEYSTR_SPACE;
      mid_strings[0][4] = MSG_GDI_KEYSTR_SLASH;
      mid_strings[0][5] = MSG_GDI_KEYSTR_COLON;
      mid_strings[0][6] = MSG_GDI_KEYSTR_QUOTE;
      mid_strings[0][7] = MSG_GDI_KEYSTR_DBLQUOTE;
      mid_strings[0][8] = MSG_GDI_KEYSTR_BACKSLASH;
      mid_strings[0][9] = MSG_GDI_KEYSTR_BRACKETS;
      mid_strings[0][10] = MSG_GDI_KEYSTR_BRACKETS;
      mid_strings[0][11] = MSG_GDI_KEYSTR_BRACES;
      mid_strings[0][12] = MSG_GDI_KEYSTR_BRACES;
      mid_strings[0][13] = MSG_GDI_KEYSTR_PIPE;
      mid_strings[0][14] = MSG_GDI_KEYSTR_PARENTHESIS;
      mid_strings[0][15] = MSG_GDI_KEYSTR_PARENTHESIS;
      mid_strings[0][16] = MSG_GDI_KEYSTR_AT;
      mid_strings[0][17] = MSG_GDI_KEYSTR_PERCENT;
      mid_strings[0][18] = MSG_GDI_KEYSTR_COMMA;
      mid_strings[0][19] = NULL;
      mid_strings[1][0] = MSG_GDI_KEYSTR_RETURN;
      mid_strings[1][1] = MSG_GDI_KEYSTR_TABULATOR;
      mid_strings[1][2] = MSG_GDI_KEYSTR_CARRIAGERET;
      mid_strings[1][3] = MSG_GDI_KEYSTR_SLASH;
      mid_strings[1][4] = MSG_GDI_KEYSTR_COLON;
      mid_strings[1][5] = MSG_GDI_KEYSTR_AT;
      mid_strings[1][6] = MSG_GDI_KEYSTR_BACKSLASH;
      mid_strings[1][7] = MSG_GDI_KEYSTR_ASTERISK;
      mid_strings[1][8] = MSG_GDI_KEYSTR_QUESTIONMARK;
      mid_strings[1][9] = NULL;
      mid_strings[2][0] = MSG_GDI_KEYSTR_RETURN;
      mid_strings[2][1] = MSG_GDI_KEYSTR_TABULATOR;
      mid_strings[2][2] = MSG_GDI_KEYSTR_CARRIAGERET;
      mid_strings[2][3] = MSG_GDI_KEYSTR_SPACE;
      mid_strings[2][4] = MSG_GDI_KEYSTR_SLASH;
      mid_strings[2][5] = MSG_GDI_KEYSTR_COLON;
      mid_strings[2][6] = MSG_GDI_KEYSTR_QUOTE;
      mid_strings[2][7] = MSG_GDI_KEYSTR_DBLQUOTE;
      mid_strings[2][8] = MSG_GDI_KEYSTR_BACKSLASH;
      mid_strings[2][9] = MSG_GDI_KEYSTR_BRACES;
      mid_strings[2][10] = MSG_GDI_KEYSTR_BRACES;
      mid_strings[2][11] = MSG_GDI_KEYSTR_PIPE;
      mid_strings[2][12] = MSG_GDI_KEYSTR_PARENTHESIS;
      mid_strings[2][13] = MSG_GDI_KEYSTR_PARENTHESIS;
      mid_strings[2][14] = MSG_GDI_KEYSTR_AT;
      mid_strings[2][15] = MSG_GDI_KEYSTR_PERCENT;
      mid_strings[2][16] = MSG_GDI_KEYSTR_COMMA;
      mid_strings[2][17] = NULL;

      keyword_strings[0] = MSG_GDI_KEYSTR_KEYWORD;
      keyword_strings[1] = MSG_GDI_KEYSTR_KEYWORD;
      keyword_strings[2] = MSG_GDI_KEYSTR_KEYWORD;
      keyword_strings[3] = NULL;

      initialized = 1;
   }

   if (str == NULL) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_KEYSTR_NULL_S, name));
      answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      return STATUS_EUNKNOWN;
   }

   /* check string length first, if too long -> error */
   if (strlen(str) > str_length) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_KEYSTR_LENGTH_U, sge_u32c(str_length)));
      answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      return STATUS_EUNKNOWN;
   }

   /* check first character */
   i = -1;
   while ((forbidden_char = begin_chars[table][++i])) {
      if (str[0] == forbidden_char) {
         if (isprint((int) forbidden_char)) {
            SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_KEYSTR_FIRSTCHAR_SC,
                           begin_strings[table][i], begin_chars[table][i]));
         } else {
            SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_KEYSTR_FIRSTCHAR_S, 
                           begin_strings[table][i]));
         }
         answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
         return STATUS_EUNKNOWN;
      }
   }

   /* check all characters in str */
   i = -1;
   while ((forbidden_char = mid_characters[table][++i])) {
      if (strchr(str, forbidden_char)) {
         if (isprint((int) forbidden_char)) {
            SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_KEYSTR_MIDCHAR_SC,
                           mid_strings[table][i], mid_characters[table][i]));
         } else {
            SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_KEYSTR_MIDCHAR_S, 
                           mid_strings[table][i]));
         }
         answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
         return STATUS_EUNKNOWN;
      }
   }

   /* Check for non-ASCII/non-graphic after more specific tests above
      for non-printing ones.  */
   for (i=0; i<strlen(str); ++i) {
      if (!(isgraph(str[i]) && asciichar(str[i]))) {
         char mesg[128];
         snprintf (mesg, sizeof(mesg), "Non-graphic character (hex %x)",
                   (unsigned char) str[i]);
         SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_KEYSTR_MIDCHAR_S,
                                mesg));
         return STATUS_EUNKNOWN;
      }
   }

   /* reject invalid keywords */
   i = -1;
   while ((forbidden_string = keyword[++i])) {
      if (!strcasecmp(str, forbidden_string)) {
         SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_KEYSTR_KEYWORD_SS, 
                        keyword_strings[i],
            forbidden_string));
         answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
         return STATUS_EUNKNOWN;
      }
   }

   return STATUS_OK;
}

/****** sge_utility/verify_host_name() *****************************************
*  NAME
*     verify_host_name() -- verify a hostname
*
*  SYNOPSIS
*     bool 
*     verify_host_name(lList **answer_list, const char *host_name) 
*
*  FUNCTION
*     Verifies if a hostname is correct (regarding maximum length etc.).
*
*  INPUTS
*     lList **answer_list   - answer list to pass back error messages
*     const char *host_name - the hostname to verify
*
*  RESULT
*     bool - true on success,
*            false on error with error message in answer_list
*
*  NOTES
*     MT-NOTE: verify_host_name() is MT safe 
*******************************************************************************/
bool verify_host_name(lList **answer_list, const char *host_name)
{
   bool ret = true;

   if (host_name == NULL || *host_name == '\0') {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR, 
                              "%s", MSG_HOSTNAME_NOT_EMPTY);
      ret = false;
   }

   if (ret) {
      if (strlen(host_name) > CL_MAXHOSTNAMELEN_LENGTH) {
         answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR, 
                                 "%s", MSG_HOSTNAME_NOT_EMPTY);
      }
   }

   /* TODO: further verification (e.g. character set) */

   return ret;
}
