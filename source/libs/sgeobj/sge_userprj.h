#ifndef __SGE_USERPRJ_H 
#define __SGE_USERPRJ_H 
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

#include "sgeobj/sge_userprj_PR_L.h"
#include "sgeobj/sge_userprj_UU_L.h"
#include "sgeobj/sge_userprj_UPU_L.h"
#include "sgeobj/sge_userprj_UPP_L.h"

lListElem *prj_list_locate(const lList *prj_list,
                           const char *prj_name);

lListElem *user_list_locate(const lList *user_list,
                            const char *user_name);

const char *prj_list_append_to_dstring(const lList *this_list, dstring *string);

bool prj_list_do_all_exist(const lList *this_list, lList **answer_list,
                           const lList *userprj_list);

lListElem *getUserTemplate(void);
lListElem *getPrjTemplate(void);

#endif /* __SGE_USERPRJ_H */
