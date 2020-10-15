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

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/sge_parse_num_par.h"

#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_id.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "sge_options.h"
#include "parse_qsub.h"
#include "usage.h"
#include "msg_common.h"

/*-------------------------------------------------------------------------*/
/* use cstring_list_parse_from_string() if you need a parsing function */
static void sge_parse_string_list(lList **lp, const char *str, int field, 
                           lDescr *descr) {
   const char *cp;

   DENTER(TOP_LAYER, "sge_parse_string_list");

   cp = sge_strtok(str, ",");
   lAddElemStr(lp, field, cp, descr);
   while((cp = sge_strtok(NULL, ","))) {
      lAddElemStr(lp, field, cp, descr);
   }

   DEXIT;
}

/***************************************************************************/
/* MT-NOTE: sge_add_noarg() is MT safe */
lListElem *sge_add_noarg(
lList **popt_list,
u_long32 opt_number,
const char *opt_switch,
const char *opt_switch_arg 
) {
   lListElem *ep;

   if (!popt_list) {
      return NULL;
   }
   if (!*popt_list) {
      *popt_list = lCreateList("option list", SPA_Type);
      if (!*popt_list) {
         return NULL;
      }
   }

   ep = lCreateElem(SPA_Type);
   if (!ep) {
      return NULL;
   }
   lSetUlong(ep, SPA_number, opt_number);
   lSetString(ep, SPA_switch, opt_switch);
   lSetString(ep, SPA_switch_arg, opt_switch_arg);
   lSetUlong(ep, SPA_occurrence, BIT_SPA_OCC_NOARG);
   lAppendElem(*popt_list, ep);
   return ep;
}

/***************************************************************************/

/* MT-NOTE: sge_add_arg() is MT safe */
lListElem *sge_add_arg(
lList **popt_list,
u_long32 opt_number,
u_long32 opt_type,
const char *opt_switch,
const char *opt_switch_arg
) {
   lListElem *ep;
   
   DENTER(TOP_LAYER, "sge_add_arg");
   
   if (popt_list == NULL) {
       DEXIT;
       return NULL;
   }

   ep = lAddElemStr(popt_list, SPA_switch, opt_switch, SPA_Type);

   if (ep != NULL) {
      lSetUlong(ep, SPA_number, opt_number);
      lSetUlong(ep, SPA_argtype, opt_type);
      lSetString(ep, SPA_switch_arg, opt_switch_arg);
      lSetUlong(ep, SPA_occurrence, BIT_SPA_OCC_ARG);
   }

   DEXIT;
   return ep;
}

/***************************************************************************/


/****
 **** parse_noopt
 ****
 **** parse a option from the commandline (sp). The option
 **** is given by shortopt and longopt (optional). 
 **** The parsed option is stored in ppcmdline (SPA_Type).
 **** An errormessage is appended to the answer-list (alpp).
 **** The function returns a pointer to the next argument.
 ****/
char **parse_noopt(char **sp, const char *shortopt, const char *longopt,
                   lList **ppcmdline, lList **alpp _UNUSED) {

   DENTER (TOP_LAYER, "parse_noopt");

   if ( (!strcmp(shortopt, *sp)) || (longopt && !strcmp(longopt, *sp)) ) {
      if(!lGetElemStr(*ppcmdline, SPA_switch, shortopt)) {
         sge_add_noarg(ppcmdline, 0, shortopt, NULL);
      }
      sp++;
   }
   DEXIT;
   return sp;
}

/****
 **** parse_until_next_opt
 ****
 **** parse an option from the commandline (sp). The option 
 **** is given by shortopt and longopt (optional).
 **** Arguments are parsed until another option is reached
 **** (beginning with '-'). The parsed option is stored
 **** in ppcmdline (SPA_Type). An errormessage will be
 **** appended to the answer-list (alpp).
 **** The function returns a pointer to the next argument.
 **** If shortopt or longopt contains a '*' as its last (!)
 **** character, then all options matching the first given
 **** charcters are valid (e.g. 'OAport' matches 'OA*')
 ****/
char **parse_until_next_opt(
char **sp,
const char *shortopt,
const char *longopt,
lList **ppcmdline,
lList **alpp 
) {
char **rp;
stringT str;
lListElem *ep; /* SPA_Type */

   DENTER (TOP_LAYER, "parse_until_next_opt");

   rp = sp;
   if ( (!strcmp(shortopt, *sp)) || (longopt && !strcmp(longopt, *sp)) 
        || ((shortopt[strlen(shortopt)-1] == '*')
           && !strncmp(shortopt, *sp, strlen(shortopt)-1)) 
        || (longopt && (longopt[strlen(longopt)-1] == '*')
           && !strncmp(longopt, *sp, strlen(longopt)-1)) ) {
      if(!*(++rp) || (**rp == '-') || (!**rp)) {
         snprintf(str, sizeof(str), MSG_PARSE_XOPTIONMUSTHAVEARGUMENT_S, *sp);
         answer_list_add(alpp, str, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
         DEXIT;
         return rp;
      }
      ep = sge_add_arg(ppcmdline, 0, lListT, shortopt, NULL);
      while (*rp && **rp != '-') {
         /* string at *rp is argument to current option */
         lAddSubStr(ep, ST_name, *rp, SPA_argval_lListT, ST_Type);
         rp++;
      }
   }
   DEXIT;
   return rp;
}


/****
 **** parse_until_next_opt2
 ****
 **** parse an option from the commandline (sp). The option 
 **** is given by shortopt and longopt (optional).
 **** Arguments are parsed until another option is reached
 **** (beginning with '-'). The parsed option is stored
 **** in ppcmdline (SPA_Type). An errormessage will be
 **** appended to the answer-list (alpp).
 **** The function returns a pointer to the next argument.
 ****/
char **parse_until_next_opt2(char **sp, const char *shortopt,
                             const char *longopt, lList **ppcmdline,
                             lList **alpp _UNUSED) {
   char **rp;
   lListElem *ep; /* SPA_Type */

   DENTER (TOP_LAYER, "parse_until_next_opt2");

   rp = sp;
   if ( (!strcmp(shortopt, *sp)) || (longopt && !strcmp(longopt, *sp)) ) {
      ++rp;
      
#if 0
      string str;
      if(!*() || (**rp == '-')) {
         sprintf(str, MSG_PARSE_XOPTIONMUSTHAVEARGUMENT_S, *sp);
         answer_list_add(alpp, str, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
         DEXIT;
         return rp;
      }
#endif
      ep = sge_add_arg(ppcmdline, 0, lListT, shortopt, NULL);
      while (*rp && **rp != '-') {
         /* string at *rp is argument to current option */
         lAddSubStr(ep, ST_name, *rp, SPA_argval_lListT, ST_Type);
         rp++;
      }
   }
   DEXIT;
   return rp;
}

/****
 **** parse_param
 ****
 **** parse a list of parameters from the commandline (sp).
 **** Parameters are parsed until the next option ("-...")
 **** is reached. The parsed parameters are stored in
 **** ppcmdline (SPA_Type).
 **** The function returns a pointer to the next argument.
 ****/
char **parse_param(char **sp, const char *opt, lList **ppcmdline,
                   lList **alpp _UNUSED) {
char **rp;
lListElem *ep = NULL; /* SPA_Type */

   DENTER (TOP_LAYER, "parse_param");

   rp = sp;
   while( (*rp) && (**rp != '-') ) {
      /* string under rp is parameter, no option! */
      if(!ep)
         ep = sge_add_arg(ppcmdline, 0, lListT, opt, NULL);
      lAddElemStr(lGetListRef(ep, SPA_argval_lListT), ST_name, *rp, ST_Type);
      rp++;
   }
   DEXIT;
   return rp;
}

/****
 **** parse_flag
 ****
 **** look in the ppcmdline-list for a flag given
 **** by 'opt'. If it is found, true is returned and
 **** flag is set to 1. If it is not found, false
 **** is returned and flag will be untouched.
 ****
 **** If the switch occures more than one time, every
 **** occurence is removed from the ppcmdline-list.
 ****
 **** The answerlist ppal is not used yet.
 ****/
bool parse_flag(lList **ppcmdline, const char *opt, lList **ppal _UNUSED,
                u_long32 *pflag) {
lListElem *ep;
char* actual_opt;

   DENTER(BASIS_LAYER, "parse_flag");

   if((ep = lGetElemStrLike(*ppcmdline, SPA_switch, opt))) {
      actual_opt = sge_strdup(NULL, lGetString(ep, SPA_switch));
      while(ep) {
         /* remove _all_ flags of same type */
         lRemoveElem(*ppcmdline, &ep);
         ep = lGetElemStrLike(*ppcmdline, SPA_switch, actual_opt);
      }
      sge_free(&actual_opt);
      *pflag = 1;
      DEXIT;
      return true;
   } else {
      DEXIT;
      return false;
   }
}

/****
 **** parse_multi_stringlist
 ****
 **** looks in the ppcmdline-list for a option given
 **** by 'opt'. If it is found, true is returned,
 **** otherwise false.
 **** Arguments after the option switch are parsed
 **** into the ppdestlist-list (given field and type).
 **** There can be multiple occurrences of this switch.
 **** The arguments are collected. 
 **** The arguments can be either space- or comma-separated.
 ****/ 
bool parse_multi_stringlist(lList **ppcmdline, const char *opt,
                            lList **ppal _UNUSED, lList **ppdestlist,
                            lDescr *type, int field) {
   lListElem *ep, *sep;

   DENTER(TOP_LAYER, "parse_multi_stringlist");

   if((ep = lGetElemStr(*ppcmdline, SPA_switch, opt))) {
      while(ep) {
         /* collect all opts of same type, this is what 'multi' means in funcname!  */
         for_each(sep, lGetList(ep, SPA_argval_lListT)) {
            sge_parse_string_list(ppdestlist, lGetString(sep, ST_name), field, type);
         }
         lRemoveElem(*ppcmdline, &ep);
         ep = lGetElemStr(*ppcmdline, SPA_switch, opt);
      }
      DRETURN(true);
   } else {
      DRETURN(false);
   }
}

bool parse_multi_jobtaskslist(
lList **ppcmdline,
const char *opt,
lList **alpp,
lList **ppdestlist,
bool include_names,
u_long32 action
) {
   lListElem *ep, *sep, *idp;
   bool ret = false;
   bool is_run_once = false;

   DENTER(TOP_LAYER, "parse_multi_jobtaskslist");
   while ((ep = lGetElemStr(*ppcmdline, SPA_switch, opt))) {
      lListElem *arrayDef = lNext(ep);
      lList *arrayDefList = NULL;

      ret = true;
      is_run_once = true;
      if ((arrayDef != NULL) && lGetUlong(arrayDef, SPA_number) ==  t_OPT) {
         arrayDefList = lGetList(arrayDef, SPA_argval_lListT);
      }
      for_each(sep, lGetList(ep, SPA_argval_lListT)) {
         lList *tempArrayList = NULL;
     
         if ((arrayDefList != NULL) && (lNext(sep) == NULL)) {
            tempArrayList = arrayDefList;
         }   
         if (sge_parse_jobtasks(ppdestlist, &idp, lGetString(sep, ST_name), NULL,
             include_names, tempArrayList) == -1) {
            answer_list_add_sprintf(alpp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR,
                                    MSG_JOB_XISINVALIDJOBTASKID_S, lGetString(sep, ST_name));

            lRemoveElem(*ppcmdline, &ep);
            DRETURN(false);
         }
         lSetUlong(idp, ID_action, action);
      }
      if (arrayDefList != NULL) {
         lRemoveElem(*ppcmdline, &arrayDef);
         arrayDef = NULL;
         arrayDefList = NULL;
      }
      lRemoveElem(*ppcmdline, &ep);
   }
   
   if (is_run_once && (ep = lGetElemUlong(*ppcmdline, SPA_number, t_OPT )) != NULL) {
      answer_list_add_sprintf(alpp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR,
                              MSG_JOB_LONELY_TOPTION_S, lGetString(ep, SPA_switch_arg));
      while ((ep = lGetElemUlong(*ppcmdline, SPA_number, t_OPT )) != NULL) {
         lRemoveElem(*ppcmdline, &ep);
      }  
      
      DRETURN(false);
   }
   
   DRETURN(ret);
}

int parse_string(lList **ppcmdline, const char *opt, lList **ppal _UNUSED, char **str) {
   lListElem *ep, *ep2;

   DENTER(TOP_LAYER, "parse_string");

   if((ep = lGetElemStr(*ppcmdline, SPA_switch, opt))) {
      ep2 = lFirst(lGetList(ep, SPA_argval_lListT));
      if (ep2)
         *str = sge_strdup(NULL, lGetString(ep2, ST_name));
      else
         *str = NULL;   
      
      if(lGetNumberOfElem(lGetList(ep, SPA_argval_lListT)) > 1) {
         lRemoveElem(lGetList(ep, SPA_argval_lListT), &ep2);
      } else {
         lRemoveElem(*ppcmdline, &ep);
      }
      
      DEXIT;
      return true;
   } else {
      DEXIT;
      return false;
   }
}

int 
parse_u_long32(lList **ppcmdline, const char *opt, lList **ppal _UNUSED, u_long32 *value) 
{
   bool ret = false;
   lListElem *ep = NULL;

   DENTER(TOP_LAYER, "parse_u_long32");
   ep = lGetElemStr(*ppcmdline, SPA_switch, opt);
   if(ep != NULL) {
      *value = lGetUlong(ep, SPA_argval_lUlongT); 

      lRemoveElem(*ppcmdline, &ep);
      ret = true;
   }
   DRETURN(ret);
}

int 
parse_u_longlist(lList **ppcmdline, const char *opt, lList **ppal _UNUSED, lList **value) 
{
   bool ret = false;
   lListElem *ep = NULL;

   DENTER(TOP_LAYER, "parse_u_longlist");
   ep = lGetElemStr(*ppcmdline, SPA_switch, opt);
   if(ep != NULL) {
      *value = NULL;
      lXchgList(ep, SPA_argval_lListT, value);

      lRemoveElem(*ppcmdline, &ep);
      ret = true;
   }
   DRETURN(ret);
}


u_long32 
parse_group_options(lList *string_list, lList **answer_list) 
{
   u_long32 group_opt = GROUP_DEFAULT;
   lListElem *str_elem;

   DENTER(TOP_LAYER, "sge_parse_group_options");

   for_each(str_elem, string_list) {
      const char *letter_string = lGetString(str_elem, ST_name);
      size_t i, len;
      len = strlen(letter_string);

      for (i = 0; i < len; i++) {
         char letter = letter_string[i];

         if (letter == 'd') {
            group_opt |= GROUP_NO_TASK_GROUPS;
         } else if (letter == 'c') {
            group_opt |= GROUP_CQ_SUMMARY;
         } else if (letter == 't') {
            group_opt |= GROUP_NO_PETASK_GROUPS;
         } else {
            sprintf(SGE_EVENT, MSG_QSTAT_WRONGGCHAR_C, letter);
            answer_list_add(answer_list, SGE_EVENT, 
                            STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
         }
      }
   }
   DEXIT; 
   return (group_opt);
}

/* ------------------------------------------ 
   JG: TODO: make ADOC header
   parses an enumeration of specifiers into value
   the first specifier is interpreted
   as 1, second as 2, third as 4 ..
   
   return value

   0 ok
   -1 error

*/
bool 
sge_parse_bitfield_str(const char *str, const char *set_specifier[], 
                       u_long32 *value, const char *name, lList **alpp,
                       bool none_allowed) 
{
   const char *s;
   const char **cpp;
   u_long32 bitmask;
   /* isspace() character plus "," */
   static const char delim[] = ", \t\v\n\f\r";
   DENTER(TOP_LAYER, "sge_parse_bitfield_str");
   
   *value = 0;

   if (none_allowed && !strcasecmp(str, "none")) {
      DEXIT;
      return true;
   }

   for (s = sge_strtok(str, delim); s; s=sge_strtok(NULL, delim)) {

      bitmask = 1;
      for (cpp=set_specifier; *cpp != NULL; cpp++) {
         if (!strcasecmp(*cpp, s)) {

            if ( *value & bitmask ) {
               /* whops! same specifier, already supplied! */
               SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_READCONFIGFILESPECGIVENTWICE_SS, *cpp, name)); 
               answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
               DEXIT;
               return false;
            }

            *value |= bitmask;
            break;
         }
         else   
            bitmask <<= 1;
      }

      if ( *cpp == NULL ) {
         /* whops! unknown specifier */
         SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_READCONFIGFILEUNKNOWNSPEC_SS, s, name)); 
         answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
         DEXIT;
         return false;
      }

   }

   if (*value == 0) {
      /* empty or no specifier for userset type */
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_READCONFIGFILEEMPTYSPEC_S, name));
      answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      DEXIT;
      return false;

   }
   DEXIT;
   return true;
}
