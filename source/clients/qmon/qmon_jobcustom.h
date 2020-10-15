#ifndef _QMON_JOBCUSTOM_H_
#define _QMON_JOBCUSTOM_H_
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

#include <Xm/Xm.h>
#include "qmon_proto.h"
#include "qmon_cull.h"

typedef String (*tFieldPrintFunc)(lListElem *ep, lListElem *jat, lList *jal, int nm); 

typedef struct _tJobField {
   int show;
   int nm;
   String name;
   short width;
   int max_length;
   tFieldPrintFunc printJobField;
} tJobField;

enum fill_mode {
   FILL_ALL,
   FILL_SELECTED
};

void qmonPopupJCU(Widget parent, XtPointer cld, XtPointer cad);
void qmonCreateJCU(Widget parent, XtPointer cld);
String* PrintJobField(lListElem *ep, lListElem *jat, lList *jal, int cols);
lList* qmonJobFilterResources(void);
lList* qmonJobFilterOwners(void);
int qmonJobFilterArraysCompressed(void);

int match_queue(lList **queue_list, lList *request_list, lList *complex_list, lList *exec_host_list);
int match_job(lList **job_list, lList *owner_list, lList *queue_list, lList *complex_list, lList *exec_host_list, lList *request_list);

#endif /* _QMON_JOBCUSTOM_H_ */

