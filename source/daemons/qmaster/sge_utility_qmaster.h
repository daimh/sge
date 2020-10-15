#ifndef __GDI_UTILITY_QMASTER_H
#define __GDI_UTILITY_QMASTER_H
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/


#include "cull/cull.h"
#include "sgeobj/sge_answer.h"
#include "gdi/sge_gdi_ctx.h"

int attr_mod_procedure(lList **alpp, lListElem *qep, lListElem *new_queue, int nm, char *attr_name, char *variables[]);

int attr_mod_ctrl_method(lList **alpp, lListElem *qep, lListElem *new_queue, int nm, char *attr_name);

int attr_mod_zerostr(lListElem *qep, lListElem *new_queue, int nm, char *attr_name);

int attr_mod_str(lList **alpp, lListElem *qep, lListElem *new_queue, int nm, char *attr_name);

int attr_mod_double(lListElem *qep, lListElem *new_queue, int nm, char *attr_name);

int attr_mod_bool(lListElem *qep, lListElem *new_queue, int nm, char *attr_name);

int attr_mod_ulong(lListElem *qep, lListElem *new_queue, int nm, char *attr_name);

int attr_mod_mem_str(lList **alpp, lListElem *qep, lListElem *new_queue, int nm, char *attr_name);

int attr_mod_time_str(lList **alpp, lListElem *qep, lListElem *new_queue, int nm, char *attr_name, int enable_infinity);

bool cqueue_mod_sublist(lListElem *this_elem, lList **answer_list,
                   lListElem *reduced_elem, int sub_command,
                   int attribute_name, int sublist_host_name,
                   int sublist_value_name, int subsub_key,
                   const char *attribute_name_str,
                   const char *object_name_str);

int multiple_occurances(lList **alpp, lList *lp1, lList *lp2, int nm, const char *name, const char *obj_name);

void normalize_sublist(lListElem *ep, int nm);

bool attr_mod_sub_list(lList **alpp, lListElem *this_elem, int this_elem_name,
                  int this_elem_primary_key, lListElem *delta_elem,
                  int sub_command, const char *sub_list_name,
                  const char *object_name,
                  int no_info, bool *changed);

#endif /* __GDI_UTILITY_QMASTER_H  */

