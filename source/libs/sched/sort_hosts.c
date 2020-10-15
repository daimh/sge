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
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_parse_num_par.h"

#include "sgeobj/sge_host.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_schedd_conf.h"

#include "sge.h"
#include "sort_hosts.h"
#include "sge_sched.h"

static const char load_ops[]={
        '+',
        '-',
        '*',
        '/',
        '&',
        '|',
        '^',
        '\0'
};


enum {
        LOAD_OP_NONE=-1,
        LOAD_OP_PLUS,
        LOAD_OP_MINUS,
        LOAD_OP_TIMES,
        LOAD_OP_DIV,
        LOAD_OP_AND,
        LOAD_OP_OR,
        LOAD_OP_XOR
};

/* prototypes */
static int get_load_value(double *dvalp, lListElem *global, lListElem *host, const lList *centry_list, const char *attrname);

/*************************************************************************

   sort_host_list

   purpose:
      sort host list according to a load evaluation formula.

   return values:
      0 on success; -1 otherwise

   input parameters:
      hl             :  the host list to be sorted (EH_Type)
      centry_list    :  the complex entry list (CE_Type)
      formula        :  the load evaluation formula (containing no blanks)

   output parameters:
      hl             :  the sorted host list

*************************************************************************/
int sort_host_list(lList *hl, lList *centry_list)
{
   lListElem *hlp = NULL;
   lListElem *global = host_list_locate(hl, SGE_GLOBAL_NAME);
   lListElem *template = host_list_locate(hl, SGE_TEMPLATE_NAME);
   const char *load_formula = sconf_get_load_formula();
   double load;

   DENTER(TOP_LAYER, "sort_host_list");

   for_each (hlp, hl) {
      if (hlp != global && hlp != template) { /* don't treat global or template */
         /* build complexes for that host */
         load = scaled_mixed_load(load_formula, global, hlp, centry_list);
         lSetDouble(hlp, EH_sort_value, load);
         DPRINTF(("%s: %f\n", lGetHost(hlp, EH_name), load));
      }
   }
   sge_free(&load_formula);

   if (lPSortList(hl,"%I+", EH_sort_value)) {
      DRETURN(-1);
   } else {
      DRETURN(0);
   }
}


/*************************************************************************
   scaled_mixed_load:

   purpose:
      compute scaled and weighted load for a particular host according
      to a load formula and that host's load and load scaling lists.

   return value:
      load value to be used for sorting the host list. in case of
      errors, a value of ERROR_LOAD_VAL is returned thereby ensuring
      that hosts with incorrect load reporting are considered heavily
      loaded.

   input parameters:
      load_list      : the host's load list  -> to be passed further
      scaling_list   : the host's load scaling list   -> to be passed
                       further
      host_cplx      : the entries list of the host complex -> to be
                       passed further
      lc_factor      : factor that is used to implement load correction: 
                       0 means no load correction, 
                       n load correction for n new jobs
*************************************************************************/
double scaled_mixed_load(const char* load_formula, lListElem *global,
                         lListElem *host, const lList *centry_list)
{
   char *cp = NULL;
   char *tf = NULL; 
   char *ptr = NULL;
   char *ptr2 = NULL;
   char *par_name = NULL;
   char *op_ptr=NULL;

   double val=0, val2=0;
   double load=0;
   int op_pos, next_op=LOAD_OP_NONE;
   char *lasts = NULL;

   DENTER(TOP_LAYER, "scaled_mixed_load");

   /* we'll use strtok ==> we need a safety copy */
   if ((tf = strdup(load_formula)) == NULL) {
      DRETURN(ERROR_LOAD_VAL);
   }

   /* 
    * + and - have the lowest precedence. all else are equal,
    * thus formula is delimited by + or - signs
    * if the load formula begins with a "-" we need to multiply the
    * first load value with -1
    */
   if (tf[0] == '-') {
      next_op = LOAD_OP_MINUS;
   }

   for (cp=strtok_r(tf, "+-", &lasts); cp; cp = strtok_r(NULL, "+-", &lasts)) {

      /* ---------------------------------------- */
      /* get scaled load value                    */
      /* determine 1st components value           */
      if (!(val = strtod(cp, &ptr)) && ptr == cp) {
         /* it is not an integer ==> it's got to be a load value */
         if (!(par_name = sge_delim_str(cp, &ptr, load_ops)) ||
               get_load_value(&val, global, host, centry_list, par_name)) {
            sge_free(&par_name);
            sge_free(&tf);

            DRETURN(ERROR_LOAD_VAL);
         }
         sge_free(&par_name);
      }

      /* ---------------------------------------- */
      /* for the load value                       */
      /* *ptr now contains the delimiting character for val */
      if (*ptr) {
         /* if the delimiter is not \0 it's got to be a operator -> find it */
         if (!(op_ptr=strchr(load_ops,(int) *ptr))) {
            sge_free(&tf);
            DRETURN(ERROR_LOAD_VAL);
         }
         op_pos = (int) (op_ptr - load_ops);

         /* ------------------------------- */
         /* look for a weighting factor  */
         /* determine 2nd component's value */
         ptr++;
         if (!(val2 = strtod(ptr,&ptr2)) && ptr2 == ptr) {
            /* it is not an integer ==> it's got to be a load value */
            if (!(par_name = sge_delim_str(ptr,NULL,load_ops)) ||
               get_load_value(&val2, global, host, centry_list, par_name)) {
               sge_free(&par_name);
               sge_free(&tf);
               DRETURN(ERROR_LOAD_VAL);
            }
            sge_free(&par_name);
         }

         /* ------------------------------- */
         /*  apply according load operator  */
         switch (op_pos) {
            case LOAD_OP_TIMES:
               val *= val2;
               break;
            case LOAD_OP_DIV:
               val /= val2;
               break;
            case LOAD_OP_AND: {
               u_long32 tmp;
               tmp = (u_long32)val & (u_long32)val2;
               val = (double)tmp;
               break;
            }
            case LOAD_OP_OR: {
               u_long32 tmp;
               tmp = (u_long32)val | (u_long32)val2; 
               val = (double)tmp;
               break;
            }
            case LOAD_OP_XOR: {
               u_long32 tmp;
               tmp = (u_long32)val ^ (u_long32)val2;
               val = (double)tmp;
               break;
            }
         }     /* switch (op_pos) */
      }     /* if (*ptr) */

      /* now we have the intermediate result from the subexpression in
       * between a + or - operator in val. next we've to add or
       * subtract from the current result value.
       */

      /* next_op is the next operation from the last run of the while loop.
       * thus next_op now is the operation to applied
       */
      switch (next_op) {
         case LOAD_OP_NONE:
            /* this is the first run -> just set load */
            load = val;
            break;
         case LOAD_OP_PLUS:
            load += val;
            break;
         case LOAD_OP_MINUS:
            load -= val;
            break;
      }

      /* determine next_op from the safety copy of the stripped formula */
      if (load_formula[cp-tf+strlen(cp)] == '+') {
         next_op = LOAD_OP_PLUS;
      }   
      else {
         next_op = LOAD_OP_MINUS;
      }   
   }

   sge_free(&tf);

   DRETURN(load);
}



/***********************************************************************

   get_load_value

 ***********************************************************************/
static int get_load_value(double *dvalp, lListElem *global, lListElem *host, const lList *centry_list, const char *attrname) 
{
   lListElem *cep;

   DENTER(TOP_LAYER, "get_load_value");
   
   /* search complex */
   if (strchr(attrname, '$')) {
      attrname++;
   }

   if(!(cep = get_attribute_by_name(global, host, NULL, attrname, centry_list, DISPATCH_TIME_NOW, 0))){
      /* neither load or consumable available for that host */
      DRETURN(1);
   }

   if (lGetUlong(cep, CE_pj_dominant) & DOMINANT_TYPE_VALUE) {
      *dvalp = lGetDouble(cep, CE_doubleval);
   } else {
      *dvalp = lGetDouble(cep, CE_pj_doubleval);
   }

   lFreeElem(&cep);

   /*
    * No value available.
    */

   DRETURN(0);
}
