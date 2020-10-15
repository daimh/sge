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

#include "uti/sge_rmon.h"
#include "uti/sge_bitfield.h"
#include "uti/sge_dstring.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_string.h"
#include "uti/sge_prog.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_mailrec.h"

#include "symbols.h"
#include "get_path.h"

#include "msg_common.h"

/****** sgeobj/mailrec/mailrec_parse() ****************************************
*  NAME
*     mailrec_parse() -- Parse a list of mail recipients 
*
*  SYNOPSIS
*     int mailrec_parse(lList **lpp, const char *mail_str) 
*
*  FUNCTION
*     Parse a list of mail recipients.
*     user[@host][,user[@host],...] 
*
*  INPUTS
*     lList **lpp          - MR_Type list 
*     const char *mail_str - stringlist of mail recipients 
*
*  RESULT
*     int - error state
*        0 - success
*       >0 - error
*
*  SEE ALSO
*     sgeobj/mailrec/mailrec_unparse() 
*
*  NOTES
*     MT-NOTE: mailrec_parse() is MT safe
*******************************************************************************/
int mailrec_parse(lList **lpp, const char *mail_str) 
{
   const char *user;
   const char *host;
   char **str_str;
   char **pstr;
   lListElem *ep, *tmp;
   char *mail;
   struct saved_vars_s *context;

   DENTER(TOP_LAYER, "mailrec_parse");

   if (!lpp) {
      DEXIT;
      return 1;
   }

   mail = sge_strdup(NULL, mail_str);
   if (!mail) {
      *lpp = NULL;
      DEXIT;
      return 2;
   }
   str_str = string_list(mail, ",", NULL);
   if (!str_str || !*str_str) {
      *lpp = NULL;
      sge_free(&mail);
      DEXIT;
      return 3;
   }

   if (!*lpp) {
      *lpp = lCreateList("mail_list", MR_Type);
      if (!*lpp) {
         sge_free(&mail);
         sge_free(&str_str);
         DEXIT;
         return 4;
      }
   }

   for (pstr = str_str; *pstr; pstr++) {
      context = NULL;
      user = sge_strtok_r(*pstr, "@", &context);
      host = sge_strtok_r(NULL, "@", &context);
      if ((tmp=lGetElemStr(*lpp, MR_user, user))) {
         if (!sge_strnullcmp(host, lGetHost(tmp, MR_host))) {
            /* got this mail adress twice */
            sge_free_saved_vars(context);
            continue;
         }
      }

      /* got a new adress - add it */
      ep = lCreateElem(MR_Type);
      lSetString(ep, MR_user, user);
      if (host) 
         lSetHost(ep, MR_host, host);
      lAppendElem(*lpp, ep);

      sge_free_saved_vars(context);
   }

   sge_free(&mail);
   sge_free(&str_str);
   DEXIT;
   return 0;
}

/****** sgeobj/mailrec/mailrec_unparse() **************************************
*  NAME
*     mailrec_unparse() -- Build a string of mail reipients 
*
*  SYNOPSIS
*     int mailrec_unparse(lList *head, char *mail_str, 
*                         unsigned int mail_str_len) 
*
*  FUNCTION
*     Build a string of mail reipients ("user@host,user,...") 
*
*  INPUTS
*     lList *head               - MR_Type list
*     char *mail_str            - buffer to be filled 
*     unsigned int mail_str_len - size of buffer 
*
*  RESULT
*     int - error state
*        0 - success
*       >0 - error
*
*  SEE ALSO
*     sgeobj/mailrec/mailrec_parse() 
*******************************************************************************/
int mailrec_unparse(lList *head, char *mail_str, unsigned int mail_str_len)
{
   int len=0;
   int comma_needed = 0; /* whether we need to insert a comma */
   char tmpstr[1000];    /* need 1000 for brain damaged mail addresse(e)s */
   lListElem *elem;
   const char *h;
   const char *u;

   if (!head) {
      sge_strlcpy(mail_str, MSG_NONE, mail_str_len);
      return 0;
   }

   *mail_str = '\0';

   for_each(elem,head) {
      if (!(u = lGetString(elem, MR_user)))
         u = MSG_SMALLNULL;

      if (!(h = lGetHost(elem, MR_host)))
         snprintf(tmpstr, sizeof(tmpstr), "%s", u);
      else
         snprintf(tmpstr, sizeof(tmpstr), "%s@%s", u, h);

      if (strlen(tmpstr)+len+1+comma_needed > mail_str_len)
         return 1;              /* forgot the rest */

      if (comma_needed)
         strcat(mail_str, ",");
      else
         comma_needed = 1;      /* need comma after first mailaddress */

      sge_strlcat(mail_str, tmpstr, mail_str_len);
   }
   return 0;
}

bool
sge_mailopt_to_dstring(u_long32 opt, dstring *string)
{
   bool success = true;

   DENTER(TOP_LAYER, "sge_mailopt_to_dstring");
   if (VALID(MAIL_AT_ABORT, opt)) {
      sge_dstring_append_char(string, 'a');
   } 
   if (VALID(MAIL_AT_BEGINNING, opt)) {
      sge_dstring_append_char(string, 'b');
   } 
   if (VALID(MAIL_AT_EXIT, opt)) {
      sge_dstring_append_char(string, 'e');
   } 
   if (VALID(NO_MAIL, opt)) {
      sge_dstring_append_char(string, 'n');
   } 
   if (VALID(MAIL_AT_SUSPENSION, opt)) {
      sge_dstring_append_char(string, 's');
   } 
   DRETURN(success);
}

/***********************************************************************/
/* MT-NOTE: sge_parse_mail_options() is MT safe */
int 
sge_parse_mail_options(lList **alpp, const char *mail_str, u_long32 prog_number)
{
   int i, j;
   int mail_opt = 0;

   DENTER(TOP_LAYER, "sge_parse_mail_options");

   i = strlen(mail_str);

   for (j = 0; j < i; j++) {
      if ((char) mail_str[j] == 'a') {
         mail_opt = mail_opt | MAIL_AT_ABORT;
      } else if ((char) mail_str[j] == 'b') {
         mail_opt = mail_opt | MAIL_AT_BEGINNING;
      } else if ((char) mail_str[j] == 'e') {
         mail_opt = mail_opt | MAIL_AT_EXIT;
      } else if ((char) mail_str[j] == 'n') {
         mail_opt = mail_opt | NO_MAIL;
      } else if ((char) mail_str[j] == 's') {
         if (prog_number == QRSUB) {
            answer_list_add_sprintf(alpp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR,
                   MSG_PARSE_XOPTIONMUSTHAVEARGUMENT_S, "-m");
            DRETURN(0);
         }
         mail_opt = mail_opt | MAIL_AT_SUSPENSION;
      } else {
         DRETURN(0);
      }
   }

   DRETURN(mail_opt);

}

