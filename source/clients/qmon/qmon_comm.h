#ifndef _QMON_COMM_H_
#define _QMON_COMM_H_
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

#include "qmon_proto.h"
#include "qmon_cull.h"

#include "gdi/sge_gdi.h"

#define ADMINHOST_T           (1<<0)
#define SUBMITHOST_T          (1<<1)
#define EXECHOST_T            (1<<2)
#define CQUEUE_T              (1<<3)
#define JOB_T                 (1<<4)
#define EVENT_T               (1<<5)
#define CENTRY_T              (1<<6)
#define ORDER_T               (1<<7)
#define MASTER_EVENT_T        (1<<8)
#define CONFIG_T              (1<<9)
#define MANAGER_T             (1<<10)
#define OPERATOR_T            (1<<11)
#define PE_T                  (1<<12)
#define SC_T                  (1<<13)
#define USER_T                (1<<14)
#define USERSET_T             (1<<15)
#define PROJECT_T             (1<<16)
#define SHARETREE_T           (1<<17)
#define CKPT_T                (1<<18)
#define CALENDAR_T            (1<<19)
#define JOB_SCHEDD_INFO_T     (1<<20)
#define ZOMBIE_T              (1<<21)
#define USER_MAPPING_T        (1<<22)
#define HGROUP_T              (1<<23)
#define RQS_T                 (1<<24)
#define AR_T                  (1<<25)

#define QU_TEMPLATE              "template"
#define EH_GLOBAL_HOST_TEMPLATE  "global"
#define EH_EXEC_HOST_TEMPLATE    "template" 


typedef struct _tQmonMirrorList {
   int id;
   u_long32 type;
   u_long32 selector;
   lList *lp;
   u_long32 last_update;
   lCondition *where;
   lEnumeration *what;
} tQmonMirrorList;

void qmonMirrorListInit(void);
lList* qmonMirrorList(int type);
lList** qmonMirrorListRef(int type);
int qmonMirrorMulti(u_long32 selector);
int qmonMirrorMultiAnswer(u_long32 selector, lList**answerp);
void qmonShowMirrorList(Widget w, XtPointer cld, XtPointer cad);

lList* qmonDelList(int type, lList **local, int nm, lList **lpp, lCondition *where, lEnumeration *what); 

#if 0
lList* qmonDelJobList(int type, lList **local, int nm, lList **lpp, lCondition *where, lEnumeration *what); 
#endif

lList* qmonModList(int type, lList **local, int nm, lList **lpp, lCondition *where, lEnumeration *what); 
lList* qmonAddList(int type, lList **local, int nm, lList **lpp, lCondition *where, lEnumeration *what); 
lList* qmonChangeStateList(int type, lList *lp, int force, int action);
u_long32 l2s(u_long32 ltype);


#endif /* _QMON_COMM_H_ */
