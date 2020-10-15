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

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>

#include "uti/sge_rmon.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"

#include "cull/cull.h"

#include "sgeobj/sge_config.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_attr.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/msg_sgeobjlib.h"

/* 
**
** DESCRIPTION
**    read_config_list() reads the file into a configuration list 
**    of type 'delimitor' putting the name of the configuraion
**    attribute into the 'nm1'-field and the value into 'nm2'.
**    'delimitor' is used to separate the name and value.
**    It may be NULL as it gets passed to sge_strtok().
**    If 'flag' has set the RCL_NO_VALUE bitmask then
**    also lines are accepted having only a name but no value.
**    
**
**
** RETURN
**    A NULL pointer as answer list signals success.
*/
int read_config_list(FILE *fp, lList **lpp, lList **alpp, lDescr *dp, int nm1,
                     int nm2, int nm3, char *delimitor, int flag, char *buffer,
                     int buffer_size) 
{
   lListElem *ep;
   char *name; 
   char *value;
   char *tmp;
   char *s;
   struct saved_vars_s *last = NULL;
   int force_value;
  
   DENTER(TOP_LAYER, "read_config_list");

   force_value = ((flag&RCL_NO_VALUE)==0);

   while (fgets(buffer, buffer_size, fp)) {
      if (last) {
         sge_free_saved_vars(last);
         last = NULL;
      }
      /*
      ** skip empty and comment lines
      */
      if (buffer[0] == '#')
         continue;
      for(s = buffer; *s == ' ' || *s == '\t'; s++)
         ;
      if (*s == '\n')
         continue;

      /*
      ** get name and value
      */
      if (*s != '"' || !(name = sge_strtok_r(s+1, "\"", &last))) {
         if (!(name = sge_strtok_r(buffer, delimitor, &last)))
            break;
      }
      value = sge_strtok_r(NULL, "\n", &last);

      /* handle end of sub-list */
      if (nm3 && strcmp(name, "}") == 0 && !value) {
         break;
      }

      if (!value && force_value)
         break;

      /* skip leading delimitors */
      if (value) {
         while (*value && (delimitor? 
            (NULL != strchr(delimitor, *value)):
            isspace((int) *value)))
            value++;
      }

      if(!value || !(*value)) {
         if (force_value) {
            SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_CONFIGNOARGUMENTGIVEN_S , name));
            answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
            DEXIT;
            goto Error;
         }
         value = NULL;
      } else {
         /* skip trailing delimitors */
         tmp = &value[strlen(value)-1];

         while (tmp>value && (delimitor?
            (NULL != strchr(delimitor, *tmp)):
            isspace((int) *tmp))) {
            *tmp = '\0';
            tmp--;
         }
      }

      /* handle sub-list */
      if (nm3 && value && strcmp(value, "{") == 0) {
         lList *slpp = NULL;
         if (read_config_list(fp, &slpp, alpp, dp, nm1, nm2, nm3, delimitor, flag, buffer, buffer_size)<0) {
            DEXIT;
            goto Error;
         }
         ep = lAddElemStr(lpp, nm1, name, dp);
         if (!ep) { 
            ERROR((SGE_EVENT, MSG_GDI_CONFIGADDLISTFAILED_S , name));
            answer_list_add(alpp, SGE_EVENT, STATUS_EDISK, ANSWER_QUALITY_ERROR);
            DEXIT;
            goto Error;
         } 
         lSetList(ep, nm3, slpp);
      } else {
         ep = lAddElemStr(lpp, nm1, name, dp);
         if (!ep) { 
            ERROR((SGE_EVENT, MSG_GDI_CONFIGADDLISTFAILED_S , name));
            answer_list_add(alpp, SGE_EVENT, STATUS_EDISK, ANSWER_QUALITY_ERROR);
            DEXIT;
            goto Error;
         } 

         lSetString(ep, nm2, value);
      }
   }
   
   if (last) {
      sge_free_saved_vars(last);
   }   
   DEXIT;
   return 0; 

Error:
   if (last)
      sge_free_saved_vars(last);
   return -1;
}

/*
**
** DESCRIPTION
**    'get_conf_sublist' searches in the configurations list 
**    for a config value with 'key' as name. 'name_nm' is
**    used as field for the key and 'value_nm' is taken
**    for value.
** 
**    If an alpp gets passed then an answer element 
**    gets added in case of error.
**
** RETURN
**    Returns sub-list value or NULL if such a list
**    does not exist.
*/
lList *get_conf_sublist(lList **alpp, lList *lp, int name_nm, int value_nm,
                        const char *key) 
{
   lList *value;
   lListElem *ep;

   DENTER(CULL_LAYER, "get_conf_sublist");
   
   if (!(ep=lGetElemStr(lp, name_nm, key))) {
      if (alpp) {
         char error[1000];
         snprintf(error, sizeof(error), MSG_GDI_CONFIGMISSINGARGUMENT_S, key);
         answer_list_add(alpp, error, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      }
      DEXIT;
      return NULL;
   }

   value = lGetList(ep, value_nm);
   /* DPRINTF(("%s = %s\n", key, value?value:"<null ptr>")); */

   DEXIT;
   return value;
}

/*
**
** DESCRIPTION
**    'get_conf_value' searches in the configurations list 
**    for a config value with 'key' as name. 'name_nm' is
**    used as field for the key and 'value_nm' is taken
**    for value.
** 
**    If an alpp gets passed then an answer element 
**    gets added in case of error.
**
** RETURN
**    Returns string value or NULL if such a value
**    does not exist.
*/
char *get_conf_value(lList **alpp, lList *lp, int name_nm, int value_nm,
                           const char *key) {
   char *value;
   lListElem *ep;

   DENTER(CULL_LAYER, "get_conf_value");
   
   if (!(ep=lGetElemStr(lp, name_nm, key))) {
      if (alpp) {
         char error[1000];
         snprintf(error, sizeof(error), MSG_GDI_CONFIGMISSINGARGUMENT_S, key);
         answer_list_add(alpp, error, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      }
      DEXIT;
      return NULL;
   }

   /* FIX_CONST */
   value = (char*) lGetString(ep, value_nm);
   DPRINTF(("%s = %s\n", key, value?value:"<null ptr>"));

   DEXIT;
   return value;
}

/****
 **** set_conf_string
 ****
 **** 'set_conf_string' searches in the configuration list
 **** (pointed to by clpp) for a string-config value with 'key'
 **** as name.
 **** If the value is found, it is stored in the lListElem
 **** 'ep' in the fied specified by 'name_nm'.
 **** If the config-string is not found, an error message
 **** is created.
 ****
 **** The function returns false on error, otherwise true.
 ****/
bool set_conf_string(
lList **alpp,
lList **clpp,
int fields[],
const char *key,
lListElem *ep,
int name_nm 
) {
   const char *str;
   int pos;
   int dataType;

   DENTER(TOP_LAYER, "set_conf_string");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }

   pos = lGetPosViaElem(ep, name_nm, SGE_NO_ABORT);
   dataType = lGetPosType(lGetElemDescr(ep),pos);
   switch (dataType) {
      case lStringT:
         DPRINTF(("set_conf_string: lStringT data type (Type: %s)\n",lNm2Str(name_nm)));
         lSetString(ep, name_nm, str);
         break;
      case lHostT:
         DPRINTF(("set_conf_string: lHostT data type (Type: %s)\n",lNm2Str(name_nm)));
         lSetHost(ep, name_nm, str);
         break;
      default:
         DPRINTF(("!!!!!!!!!set_conf_string: unexpected data type !!!!!!!!!!!!!!!!!\n"));
         break;
   } 
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   DEXIT;
   return true;
}

/****
 **** set_conf_bool
 ****
 **** 'set_conf_bool' searches in the configuration list
 **** (pointed to by clpp) for a string-config value with 'key'
 **** as name.
 **** If the value is found, it is stored in the lListElem
 **** 'ep' in the field specified by 'name_nm'.
 **** the strings true and false are stored as the constants true/false
 **** in a u_long32 field
 **** If the config-string is not found, an error message
 **** is created.
 ****
 **** The function returns false on error, otherwise true.
 ****/
bool set_conf_bool(
lList **alpp,
lList **clpp,
int fields[],
const char *key,
lListElem *ep,
int name_nm 
) {
   const char *str;

   DENTER(CULL_LAYER, "set_conf_boolean");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   if (!object_parse_bool_from_string(ep, NULL, name_nm, str)) {
      DEXIT;
      return false;
   }
      
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   DEXIT;
   return true;
}

bool set_conf_centry_type(
lList **alpp,
lList **clpp,
int fields[],
const char *key,
lListElem *ep,
int name_nm 
) {
   const char *str;
   u_long32 type;

   DENTER(CULL_LAYER, "set_conf_centry_type");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   if (!ulong_parse_centry_type_from_string(&type, alpp, str)) {
      DEXIT;
      return false;
   } else {
      lSetUlong(ep, name_nm, type);
   }

   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   DEXIT;
   return true;
}

bool set_conf_centry_relop(
lList **alpp,
lList **clpp,
int fields[],
const char *key,
lListElem *ep,
int name_nm 
) {
   const char *str;
   u_long32 type;

   DENTER(CULL_LAYER, "set_conf_centry_relop");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   if (!ulong_parse_centry_relop_from_string(&type, alpp, str)) {
      DEXIT;
      return false;
   } else {
      lSetUlong(ep, name_nm, type);
   }

   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   DEXIT;
   return true;
}

bool set_conf_centry_requestable(
lList **alpp,
lList **clpp,
int fields[],
const char *key,
lListElem *ep,
int name_nm 
) {
   const char *str;
   u_long32 flag;

   DENTER(CULL_LAYER, "set_conf_centry_relop");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   if (!strcasecmp(str, "y") || !strcasecmp(str, "yes")) {
      flag = REQU_YES;
   } else if (!strcasecmp(str, "n") || !strcasecmp(str, "no")) {
      flag = REQU_NO;
   } else if (!strcasecmp(str, "f") || !strcasecmp(str, "forced")) {
      flag = REQU_FORCED;
   } else {
      answer_list_add_sprintf(alpp, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                              MSG_INVALID_CENTRY_REQUESTABLE_S, str);
      DEXIT;
      return false;
   }
   lSetUlong(ep, name_nm, flag);

   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   DEXIT;
   return true;
}

/****
 **** set_conf_ulong
 ****
 **** 'set_conf_ulong' searches in the configuration list
 **** (pointed to by clpp) for a ulong-config value with 'key'
 **** as name.
 **** If the value is found, it is stored in the lListElem
 **** 'ep' in the fied specified by 'name_nm'.
 **** If the config-value is not found, or it is not an
 **** integer, an error message is created.
 ****
 **** The function returns false on error, otherwise true.
 ****/
bool set_conf_ulong(
lList **alpp,
lList **clpp,
int fields[],
const char *key,
lListElem *ep,
int name_nm 
) {
   const char *str;

   DENTER(CULL_LAYER, "set_conf_ulong");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   if (!object_parse_ulong32_from_string(ep, alpp, name_nm, str)) {
      DEXIT;
      return false;
   }
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   DEXIT;
   return true;
}

/****
 **** set_conf_double
 ****
 **** 'set_conf_double' searches in the configuration list
 **** (pointed to by clpp) for a double-config value with 'key'
 **** as name.
 **** If the value is found, it is stored in the lListElem
 **** 'ep' in the field specified by 'name_nm'.
 **** If operation_nm != 0, the double value can be preceded by
 **** +,-,= or nothing. This sets a flag specified by operation_nm.
 **** If the config-value is not found, or it is not a
 **** double, an error message is created.
 ****
 **** The function returns false on error, otherwise true.
 ****/
bool set_conf_double(
lList **alpp,
lList **clpp,
int fields[],
const char *key,
lListElem *ep,
int name_nm,
int operation_nm 
) {
   const char *str;
   double dval;

   DENTER(CULL_LAYER, "set_conf_double"); 

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   /*
   ** only treated if operation_nm != 0
   */
   if (operation_nm) {
      while (isspace((int) *str))
         str++;
      switch (*str) {
         case '+':   
            lSetUlong(ep, operation_nm, MODE_ADD);
            str++;
            break;
         case '-':
            lSetUlong(ep, operation_nm, MODE_SUB);
            str++;
            break;
         case '=':
            lSetUlong(ep, operation_nm, MODE_SET);
            str++;
            break;
         default:
            lSetUlong(ep, operation_nm, MODE_RELATIVE);
      }
   }

   if ( (sscanf(str, "%lf", &dval)!=1) || ( strncasecmp(str,"inf",3) == 0 ) ) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_CONFIGARGUMENTNOTDOUBLE_SS , 
               key, str));
      answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      DEXIT;
      return false;
   }

   lSetDouble(ep, name_nm, dval);
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   DEXIT;
   return true;
}

/****
 **** set_conf_deflist
 ****
 **** 'set_conf_deflist' searches in the configuration list
 **** (pointed to by clpp) for a definition-list-config 
 **** value with 'key' as name.
 **** If the value is found, it is stored in the lListElem
 **** 'ep' in the fied specified by 'name_nm'.
 **** The definition-list is tokenized with 
 **** cull_parse_definition_list().
 **** The sub-list is created of type 'descr' and 
 **** interpreted by 'interpretation_rule'.
 ****
 **** The function returns false on error, otherwise true.
 ****/
bool set_conf_deflist(
lList **alpp,
lList **clpp,
int fields[],
const char *key,
lListElem *ep,
int name_nm,
lDescr *descr,
int *interpretation_rule 

) {
   lList *tmplp = NULL;
   char *str;

   DENTER(CULL_LAYER, "set_conf_deflist");


   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DRETURN((fields?true:false));
   }

   if (cull_parse_definition_list(str, &tmplp, key, descr, 
         interpretation_rule) != 0) {
      DRETURN(false);
   }

   lSetList(ep, name_nm, tmplp);
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   DRETURN(true);
}

/****
 **** set_conf_timestring
 ****
 **** 'set_conf_timestring' searches in the configuration list
 **** (pointed to by clpp) for a timestring-config value 
 **** with 'key' as name.
 **** If the value is found, it is stored in the lListElem
 **** 'ep' in the fied specified by 'name_nm'.
 **** If the config-string is not found, or it is not a
 **** valid time-string, an error message is created.
 ****
 **** The function returns false on error, otherwise true.
 ****/
bool set_conf_timestr(
lList **alpp,
lList **clpp,
int fields[],
const char *key,
lListElem *ep,
int name_nm 
) {
   const char *str;

   DENTER(CULL_LAYER, "set_conf_timestring");

   if (key == NULL) {
      DEXIT;
      return false;
   }

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   if(!parse_ulong_val(NULL, NULL, TYPE_TIM, str, NULL, 0)) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_CONFIGARGUMENTNOTTIME_SS , key, str));
      answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      DEXIT;
      return false;
   }

   lSetString(ep, name_nm, str);
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   DEXIT;
   return true;
}

/****
 **** set_conf_memstr
 ****
 **** 'set_conf_memstr' searches in the configuration list
 **** (pointed to by clpp) for a memstring-config value 
 **** with 'key' as name.
 **** If the value is found, it is stored in the lListElem
 **** 'ep' in the fied specified by 'name_nm'.
 **** If the config-string is not found, or it is not a
 **** valid mem-string, an error message is created.
 ****
 **** The function returns false on error, otherwise true.
 ****/
bool set_conf_memstr(
lList **alpp,
lList **clpp,
int fields[],
const char *key,
lListElem *ep,
int name_nm 
) {
   const char *str;

   DENTER(CULL_LAYER, "set_conf_memstr");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   if(!parse_ulong_val(NULL, NULL, TYPE_MEM, str, NULL, 0)) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_CONFIGARGUMENTNOMEMORY_SS , key, str));
      answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      DEXIT;
      return false;
   }

   lSetString(ep, name_nm, str);
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   DEXIT;
   return true;
}

/****
 **** set_conf_enum
 ****
 **** 'set_conf_enum' searches in the configuration list
 **** (pointed to by clpp) for a enumstring-config value 
 **** with 'key' as name.
 **** If the value is found, it is stored in the lListElem
 **** 'ep' in the field specified by 'name_nm'.
 **** If the config-string is not found, or it is not a
 **** valid enum-string, an error message is created.
 ****
 **** The function returns false on error, otherwise true.
 ****/
bool set_conf_enum(lList **alpp, lList **clpp, int fields[], const char *key,
                  lListElem *ep, int name_nm, const char **enum_strings) 
{
   const char *str;
   u_long32 uval = 0;

   DENTER(CULL_LAYER, "set_conf_enum");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   if(!sge_parse_bitfield_str(str, enum_strings, &uval, key, alpp, false)) {
      DEXIT;
      return false;
   }
   
   if(!uval) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_GDI_CONFIGINVALIDQUEUESPECIFIED ));
      answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      DEXIT;
      return false;
   }

   lSetUlong(ep, name_nm, uval);
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);


   DEXIT;
   return true;
}

bool set_conf_enum_none(lList **alpp, lList **clpp, int fields[], const char *key,
                  lListElem *ep, int name_nm, const char **enum_strings) 
{
   const char *str;
   u_long32 uval = 0;

   DENTER(TOP_LAYER, "set_conf_enum_none");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   if(!sge_parse_bitfield_str(str, enum_strings, &uval, key, alpp, true)) {
      DEXIT;
      return false;
   }
   
   lSetUlong(ep, name_nm, uval);
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);


   DEXIT;
   return true;
}

/****
 **** set_conf_list
 ****
 **** 'set_conf_list' searches in the configuration list
 **** (pointed to by clpp) for a list-config value with 
 **** 'key' as name.
 **** If the value is found, it is stored in the lListElem
 **** 'ep' in the fied specified by 'name_nm'.
 **** The definition-list is tokenized with 
 **** lString2List().
 **** The sub-list is created of type 'descr' and 
 **** the strings are stored in the field 'sub_name_nm'.
 ****
 **** The function returns false on error, otherwise true.
 ****/
bool set_conf_list(lList **alpp, lList **clpp, int fields[], const char *key, 
                  lListElem *ep, int name_nm, lDescr *descr, int sub_name_nm) 
{
   lList *tmplp = NULL;
   const char *str;
   const char *tmp_str = NULL;
   char delims[] = "\t \v\r,"; 

   DENTER(TOP_LAYER, "set_conf_list");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   lString2List(str, &tmplp, descr, sub_name_nm, delims); 

   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   

   if (tmplp != NULL) {
      int pos, dataType;
      lListElem *lep = lFirst(tmplp);

      pos = lGetPosViaElem(lep, sub_name_nm, SGE_NO_ABORT);
      dataType = lGetPosType(lGetElemDescr(lep),pos);
      switch (dataType) {
         case lStringT:
            DPRINTF(("set_conf_list: lStringT data type (Type: %s)\n",lNm2Str(name_nm)));
            tmp_str = lGetString(lep, sub_name_nm);
            break;
         case lHostT:
            DPRINTF(("set_conf_list: lHostT data type (Type: %s)\n",lNm2Str(name_nm)));
            tmp_str = lGetHost(lep, sub_name_nm);
            break;
         default:
            DPRINTF(("!!!!!!!!!set_conf_string: unexpected data type !!!!!!!!!!!!!!!!!\n"));
            break;
      }
      if (tmp_str && strcasecmp("NONE", tmp_str)) {
         lSetList(ep, name_nm, tmplp);
         DEXIT;
         return true;
      } else {
         lFreeList(&tmplp);
      }
   }

   DEXIT;
   return true;
}

#if unused
bool set_conf_str_attr_list(lList **alpp, lList **clpp, int fields[], 
                            const char *key, lListElem *ep, int name_nm, 
                            lDescr *descr, int sub_name_nm) 
{
   bool ret;
   lList *tmplp = NULL;
   const char *str;
   lList *lanswer_list = NULL;

   DENTER(TOP_LAYER, "set_conf_str_attr_list");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   ret = str_attr_list_parse_from_string(&tmplp, &lanswer_list, str,
                                         HOSTATTR_ALLOW_AMBIGUITY);
   if (!ret) {
      const char *text = lGetString(lFirst(lanswer_list), AN_text);

      sprintf(SGE_EVENT, "%s - %s", key, text); 
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      return ret;
   }
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   if (tmplp != NULL) {
      lSetList(ep, name_nm, tmplp);
      DEXIT;
      return true;
   }

   DEXIT;
   return true;
}

bool set_conf_strlist_attr_list(lList **alpp, lList **clpp, int fields[], 
                                const char *key, lListElem *ep, int name_nm, 
                                lDescr *descr, int sub_name_nm) 
{
   bool ret;
   lList *tmplp = NULL;
   const char *str;
   lList *lanswer_list = NULL;

   DENTER(TOP_LAYER, "set_conf_strlist_attr_list");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }

   ret = strlist_attr_list_parse_from_string(&tmplp, &lanswer_list, str,
                                             HOSTATTR_ALLOW_AMBIGUITY);
   if (!ret) {
      const char *text = lGetString(lFirst(lanswer_list), AN_text);

      sprintf(SGE_EVENT, "%s - %s", key, text); 
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      return ret;
   }
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   if (tmplp != NULL) {
      lSetList(ep, name_nm, tmplp);
      DEXIT;
      return true;
   }

   DEXIT;
   return true;
}

bool set_conf_usrlist_attr_list(lList **alpp, lList **clpp, int fields[], 
                                const char *key, lListElem *ep, int name_nm, 
                                lDescr *descr, int sub_name_nm) 
{
   bool ret;
   lList *tmplp = NULL;
   const char *str;
   lList *lanswer_list = NULL;

   DENTER(TOP_LAYER, "set_conf_usrlist_attr_list");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }

   ret = usrlist_attr_list_parse_from_string(&tmplp, &lanswer_list, str,
                                             HOSTATTR_ALLOW_AMBIGUITY);
   if (!ret) {
      const char *text = lGetString(lFirst(lanswer_list), AN_text);

      sprintf(SGE_EVENT, "%s - %s", key, text); 
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      return ret;
   }
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   if (tmplp != NULL) {
      lSetList(ep, name_nm, tmplp);
      DEXIT;
      return true;
   }

   DEXIT;
   return true;
}

bool set_conf_prjlist_attr_list(lList **alpp, lList **clpp, int fields[], 
                                const char *key, lListElem *ep, int name_nm, 
                                lDescr *descr, int sub_name_nm) 
{
   bool ret;
   lList *tmplp = NULL;
   const char *str;
   lList *lanswer_list = NULL;

   DENTER(TOP_LAYER, "set_conf_prjlist_attr_list");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }

   ret = prjlist_attr_list_parse_from_string(&tmplp, &lanswer_list, str,
                                             HOSTATTR_ALLOW_AMBIGUITY);
   if (!ret) {
      const char *text = lGetString(lFirst(lanswer_list), AN_text);

      sprintf(SGE_EVENT, "%s - %s", key, text); 
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      return ret;
   }
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   if (tmplp != NULL) {
      lSetList(ep, name_nm, tmplp);
      DEXIT;
      return true;
   }

   DEXIT;
   return true;
}

bool set_conf_celist_attr_list(lList **alpp, lList **clpp, int fields[], 
                               const char *key, lListElem *ep, int name_nm, 
                               lDescr *descr, int sub_name_nm) 
{
   bool ret;
   lList *tmplp = NULL;
   const char *str;
   lList *lanswer_list = NULL;

   DENTER(TOP_LAYER, "set_conf_celist_attr_list");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }

   ret = celist_attr_list_parse_from_string(&tmplp, &lanswer_list, str,
                                            HOSTATTR_ALLOW_AMBIGUITY);
   if (!ret) {
      const char *text = lGetString(lFirst(lanswer_list), AN_text);

      sprintf(SGE_EVENT, "%s - %s", key, text); 
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      return ret;
   }
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   if (tmplp != NULL) {
      lSetList(ep, name_nm, tmplp);
      DEXIT;
      return true;
   }

   DEXIT;
   return true;
}

bool set_conf_solist_attr_list(lList **alpp, lList **clpp, int fields[], 
                               const char *key, lListElem *ep, int name_nm, 
                               lDescr *descr, int sub_name_nm) 
{
   bool ret;
   lList *tmplp = NULL;
   const char *str;
   lList *lanswer_list = NULL;

   DENTER(TOP_LAYER, "set_conf_solist_attr_list");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }

   ret = solist_attr_list_parse_from_string(&tmplp, &lanswer_list, str,
                                            HOSTATTR_ALLOW_AMBIGUITY);
   if (!ret) {
      const char *text = lGetString(lFirst(lanswer_list), AN_text);

      sprintf(SGE_EVENT, "%s - %s", key, text); 
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      return ret;
   }
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   if (tmplp != NULL) {
      lSetList(ep, name_nm, tmplp);
      DEXIT;
      return true;
   }

   DEXIT;
   return true;
}

bool set_conf_qtlist_attr_list(lList **alpp, lList **clpp, int fields[], 
                               const char *key, lListElem *ep, int name_nm, 
                               lDescr *descr, int sub_name_nm) 
{
   bool ret;
   lList *tmplp = NULL;
   const char *str;
   lList *lanswer_list = NULL;

   DENTER(TOP_LAYER, "set_conf_qtlist_attr_list");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }

   ret = qtlist_attr_list_parse_from_string(&tmplp, &lanswer_list, str,
                                            HOSTATTR_ALLOW_AMBIGUITY);
   if (!ret) {
      const char *text = lGetString(lFirst(lanswer_list), AN_text);

      sprintf(SGE_EVENT, "%s - %s", key, text); 
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      return ret;
   }
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   if (tmplp != NULL) {
      lSetList(ep, name_nm, tmplp);
      DEXIT;
      return true;
   }

   DEXIT;
   return true;
}

bool set_conf_ulng_attr_list(lList **alpp, lList **clpp, int fields[], 
                             const char *key, lListElem *ep, int name_nm, 
                             lDescr *descr, int sub_name_nm) 
{
   bool ret;
   lList *tmplp = NULL;
   const char *str;
   lList *lanswer_list = NULL;

   DENTER(TOP_LAYER, "set_conf_ulng_attr_list");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   ret = ulng_attr_list_parse_from_string(&tmplp, &lanswer_list, str,
                                          HOSTATTR_ALLOW_AMBIGUITY);
   if (!ret) {
      const char *text = lGetString(lFirst(lanswer_list), AN_text);

      sprintf(SGE_EVENT, "%s - %s", key, text); 
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      return ret;
   }
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   if (tmplp != NULL) {
      lSetList(ep, name_nm, tmplp);
      DEXIT;
      return true;
   }

   DEXIT;
   return true;
}

bool set_conf_bool_attr_list(lList **alpp, lList **clpp, int fields[], 
                             const char *key, lListElem *ep, int name_nm, 
                             lDescr *descr, int sub_name_nm) 
{
   bool ret;
   lList *tmplp = NULL;
   const char *str;
   lList *lanswer_list = NULL;

   DENTER(TOP_LAYER, "set_conf_bool_attr_list");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   ret = bool_attr_list_parse_from_string(&tmplp, &lanswer_list, str,
                                          HOSTATTR_ALLOW_AMBIGUITY);
   if (!ret) {
      const char *text = lGetString(lFirst(lanswer_list), AN_text);

      sprintf(SGE_EVENT, "%s - %s", key, text); 
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      return ret;
   }
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   if (tmplp != NULL) {
      lSetList(ep, name_nm, tmplp);
      DEXIT;
      return true;
   }

   DEXIT;
   return true;
}

bool set_conf_time_attr_list(lList **alpp, lList **clpp, int fields[], 
                             const char *key, lListElem *ep, int name_nm, 
                             lDescr *descr, int sub_name_nm) 
{
   bool ret;
   lList *tmplp = NULL;
   const char *str;
   lList *lanswer_list = NULL;

   DENTER(TOP_LAYER, "set_conf_time_attr_list");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   ret = time_attr_list_parse_from_string(&tmplp, &lanswer_list, str,
                                          HOSTATTR_ALLOW_AMBIGUITY);
   if (!ret) {
      const char *text = lGetString(lFirst(lanswer_list), AN_text);

      sprintf(SGE_EVENT, "%s - %s", key, text); 
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      return ret;
   }
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   if (tmplp != NULL) {
      lSetList(ep, name_nm, tmplp);
      DEXIT;
      return true;
   }

   DEXIT;
   return true;
}

bool set_conf_mem_attr_list(lList **alpp, lList **clpp, int fields[], 
                             const char *key, lListElem *ep, int name_nm, 
                             lDescr *descr, int sub_name_nm) 
{
   bool ret;
   lList *tmplp = NULL;
   const char *str;
   lList *lanswer_list = NULL;

   DENTER(TOP_LAYER, "set_conf_mem_attr_list");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   ret = mem_attr_list_parse_from_string(&tmplp, &lanswer_list, str,
                                          HOSTATTR_ALLOW_AMBIGUITY);
   if (!ret) {
      const char *text = lGetString(lFirst(lanswer_list), AN_text);

      sprintf(SGE_EVENT, "%s - %s", key, text); 
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      return ret;
   }
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   if (tmplp != NULL) {
      lSetList(ep, name_nm, tmplp);
      DEXIT;
      return true;
   }

   DEXIT;
   return true;
}

bool set_conf_inter_attr_list(lList **alpp, lList **clpp, int fields[], 
                              const char *key, lListElem *ep, int name_nm, 
                              lDescr *descr, int sub_name_nm) 
{
   bool ret;
   lList *tmplp = NULL;
   const char *str;
   lList *lanswer_list = NULL;

   DENTER(TOP_LAYER, "set_conf_inter_attr_list");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   ret = inter_attr_list_parse_from_string(&tmplp, &lanswer_list, str,
                                           HOSTATTR_ALLOW_AMBIGUITY);

   if (!ret) {
      const char *text = lGetString(lFirst(lanswer_list), AN_text);

      sprintf(SGE_EVENT, "%s - %s", key, text); 
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      return ret;
   }
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   if (tmplp != NULL) {
      lSetList(ep, name_nm, tmplp);
      DEXIT;
      return true;
   }

   DEXIT;
   return true;
}
#endif

/****
 **** set_conf_subordlist
 ****
 **** 'set_conf_subordlist' searches in the configuration list
 **** (pointed to by clpp) for a subordinate-list-config 
 **** value with 'key' as name.
 **** The subordinate-list looks like:
 **** name1=val1,name2,name3,name4=val2
 **** The divider between two entries are ',', space or tab,
 **** between name and value are '=' or ':'.
 **** If the list is found, it is stored in the lListElem
 **** 'ep' in the fied specified by 'name_nm'.
 **** The sub-list is created of type 'descr', the name of
 **** the subord-list-field is stored in the field
 **** 'subname_nm', it's value in 'subval_nm'.
 **** If the subordinate list is 'NONE', no list will
 **** be created (null-pointer!).
 ****
 **** The function returns false on error, otherwise true.
 ****/
bool set_conf_subordlist(
lList **alpp,
lList **clpp,
int fields[],
const char *key,
lListElem *ep,
int name_nm,
lDescr *descr,
int subname_nm,
int subval_nm 
) {
   lList *tmplp = NULL;
   lListElem *tmpep;
   const char *str;
   const char *s;
   char *endptr;

   DENTER(CULL_LAYER, "set_conf_subordlist");

   if(!(str=get_conf_value(fields?NULL:alpp, *clpp, CF_name, CF_value, key))) {
      DEXIT;
      return fields?true:false;
   }
   lString2List(str, &tmplp, descr, subname_nm, ", \t");
   for_each (tmpep, tmplp) {
      s = sge_strtok(lGetString(tmpep, subname_nm), ":=");
      lSetString(tmpep, subname_nm, s);
      if (!(s=sge_strtok(NULL, ":=")))
         continue;
      lSetUlong(tmpep, subval_nm, strtol(s, &endptr, 10));
      if (*endptr) {
         SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_CONFIGREADFILEERRORNEAR_SS , key, endptr));
         answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
         DEXIT;
         return false;
      }
   }
   
   if (!strcasecmp("NONE", lGetString(lFirst(tmplp), subname_nm))) {
      lFreeList(&tmplp);
   }   

   lSetList(ep, name_nm, tmplp);
   lDelElemStr(clpp, CF_name, key);
   add_nm_to_set(fields, name_nm);

   DEXIT;
   return true;
}


/* 
   Append 'name_nm' into the int array 'fields'.
   In case the 'name_nm' is already contained
   in 'fields' -1 is returned. 
*/

int add_nm_to_set(
int fields[],
int name_nm 
) {
   int i = 0;

   DENTER(CULL_LAYER, "add_nm_to_set");

   if (!fields) {
      DEXIT;
      return 0; /* we leave here in most cases */
   }

   /* seek end and check whether 'name_nm' 
      is already in 'fields' */
   while (fields[i]!=NoName && fields[i]!=name_nm)
      i++;
   
   if (fields[i]==name_nm) {
      DEXIT;
      return -1;
   }

   fields[i] = name_nm;      
   fields[++i] = NoName;      

   DEXIT;
   return 0;
}

