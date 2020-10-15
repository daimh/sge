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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/times.h>

#include <Xmt/Xmt.h>
#include <Xmt/Dialogs.h>

#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_comm.h"
#include "qmon_timer.h"
#include "qmon_appres.h"
#include "qmon_globals.h"
#include "qmon_init.h"

#include "uti/sge_unistd.h"
#include "uti/sge_time.h"

#include "comm/commlib.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_id.h"

#include "gdi/qm_name.h"
#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi.h"

extern sge_gdi_ctx_class_t *ctx;

#define for_each2(ep1, lp1, ep2, lp2) \
   for (ep1=lFirst(lp1), ep2=lFirst(lp2); ep1 && ep2;\
      ep1=lNext(ep1), ep2=lNext(ep2) )

/* is the connection to commd & qmaster ok */
static int contact_ok = 1;

/* 
** QMASTER LISTS 
*/
static tQmonMirrorList QmonMirrorList[] = {
   { 0, 0, 0, NULL, 0, NULL, NULL },   /* DUMMY   */
   { 0, SGE_AH_LIST, ADMINHOST_T, NULL, 0, NULL, NULL },
   { 0, SGE_SH_LIST, SUBMITHOST_T, NULL, 0, NULL, NULL },
   { 0, SGE_EH_LIST, EXECHOST_T, NULL, 0, NULL, NULL },
   { 0, SGE_CQ_LIST, CQUEUE_T, NULL, 0, NULL, NULL },
   { 0, SGE_JB_LIST, JOB_T, NULL, 0, NULL, NULL },
   { 0, SGE_EV_LIST, EVENT_T, NULL, 0, NULL, NULL },
   { 0, SGE_CE_LIST, CENTRY_T, NULL, 0, NULL, NULL },
   { 0, SGE_ORDER_LIST, ORDER_T, NULL, 0, NULL, NULL },
   { 0, SGE_MASTER_EVENT, MASTER_EVENT_T, NULL, 0, NULL, NULL },
   { 0, SGE_CONF_LIST, CONFIG_T, NULL, 0, NULL, NULL },
   { 0, SGE_UM_LIST, MANAGER_T, NULL, 0, NULL, NULL },
   { 0, SGE_UO_LIST, OPERATOR_T, NULL, 0, NULL, NULL },
   { 0, SGE_PE_LIST, PE_T, NULL, 0, NULL, NULL },
   { 0, SGE_SC_LIST, SC_T, NULL, 0, NULL, NULL },
   { 0, SGE_UU_LIST, USER_T, NULL, 0, NULL, NULL },
   { 0, SGE_US_LIST, USERSET_T, NULL, 0, NULL, NULL },
   { 0, SGE_PR_LIST, PROJECT_T, NULL, 0, NULL, NULL },
   { 0, SGE_STN_LIST, SHARETREE_T, NULL, 0, NULL, NULL },
   { 0, SGE_CK_LIST, CKPT_T, NULL, 0, NULL, NULL },
   { 0, SGE_CAL_LIST, CALENDAR_T, NULL, 0, NULL, NULL },
   { 0, SGE_SME_LIST, JOB_SCHEDD_INFO_T, NULL, 0, NULL, NULL },
   { 0, SGE_ZOMBIE_LIST, ZOMBIE_T, NULL, 0, NULL, NULL },
   { 0, SGE_USER_MAPPING_LIST, USER_MAPPING_T, NULL, 0, NULL, NULL },
   { 0, SGE_HGRP_LIST, HGROUP_T, NULL, 0, NULL, NULL },
   { 0, SGE_RQS_LIST, RQS_T, NULL, 0, NULL, NULL },
   { 0, SGE_AR_LIST, AR_T, NULL, 0, NULL, NULL },
};
   
static char *sge_gdi_list_types[] = {
   "UNKNOWN",
   "ADMINHOST",
   "SUBMITHOST",
   "EXECHOST",
   "CQUEUE",
   "JOB",
   "EVENT",
   "CENTRY",
   "ORDER",
   "MASTER_EVENT",
   "CONFIG",
   "MANAGER",
   "OPERATOR",
   "PE",
   "SC",
   "USER",
   "USERSET",
   "PROJECT",
   "SHARETREE",
   "CHECKPOINT",
   "CALENDAR",
   "JOB_SCHEDD_INFO",
   "ZOMBIE_JOBS",
   "USER_MAPPING",
   "HGROUP",
   "RQS",
   "AR"
};



/*-------------------------------------------------------------------------*/
void qmonMirrorListInit(void)
{
   
   DENTER(GUI_LAYER, "qmonMirrorListInit");

   QmonMirrorList[SGE_AH_LIST].what = lWhat("%T(ALL)", AH_Type);
   QmonMirrorList[SGE_SH_LIST].what = lWhat("%T(ALL)", SH_Type);
   QmonMirrorList[SGE_EH_LIST].what = lWhat("%T(ALL)", EH_Type);
   QmonMirrorList[SGE_CQ_LIST].what = lWhat("%T(ALL)", CQ_Type);
   QmonMirrorList[SGE_JB_LIST].what = lWhat("%T(ALL)", JB_Type);
   QmonMirrorList[SGE_EV_LIST].what = lWhat("%T(ALL)", EV_Type);
   QmonMirrorList[SGE_CE_LIST].what = lWhat("%T(ALL)", CE_Type);
   QmonMirrorList[SGE_ORDER_LIST].what = lWhat("%T(ALL)", OR_Type);
   QmonMirrorList[SGE_MASTER_EVENT].what = lWhat("%T(ALL)", EV_Type);
   QmonMirrorList[SGE_CONF_LIST].what = lWhat("%T(ALL)", CONF_Type);
   QmonMirrorList[SGE_UM_LIST].what = lWhat("%T(ALL)", UM_Type);
   QmonMirrorList[SGE_UO_LIST].what = lWhat("%T(ALL)", UO_Type);
   QmonMirrorList[SGE_PE_LIST].what = lWhat("%T(ALL)", PE_Type);
   QmonMirrorList[SGE_SC_LIST].what = lWhat("%T(ALL)", SC_Type);
   QmonMirrorList[SGE_UU_LIST].what = lWhat("%T(ALL)", UU_Type);
   QmonMirrorList[SGE_US_LIST].what = lWhat("%T(ALL)", US_Type);
   QmonMirrorList[SGE_PR_LIST].what = lWhat("%T(ALL)", PR_Type);
   QmonMirrorList[SGE_STN_LIST].what = lWhat("%T(ALL)", STN_Type);
   QmonMirrorList[SGE_CK_LIST].what = lWhat("%T(ALL)", CK_Type);
   QmonMirrorList[SGE_CAL_LIST].what = lWhat("%T(ALL)", CAL_Type);
   QmonMirrorList[SGE_SME_LIST].what = lWhat("%T(ALL)", SME_Type);
   QmonMirrorList[SGE_ZOMBIE_LIST].what = lWhat("%T(ALL)", JB_Type);
   QmonMirrorList[SGE_USER_MAPPING_LIST].what = lWhat("%T(ALL)", CU_Type);
   QmonMirrorList[SGE_HGRP_LIST].what = lWhat("%T(ALL)", HGRP_Type);
   QmonMirrorList[SGE_RQS_LIST].what = lWhat("%T(ALL)", RQS_Type);
   QmonMirrorList[SGE_AR_LIST].what = lWhat("%T(ALL)", AR_Type);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
lList* qmonMirrorList(
int type 
) {
   return QmonMirrorList[type].lp;
}   
   
/*-------------------------------------------------------------------------*/
lList** qmonMirrorListRef(
int type 
) {
   DENTER(GUI_LAYER, "qmonMirrorListRef");

   DPRINTF(("Get reference to %s\n", sge_gdi_list_types[type]));

   DEXIT;
   return &(QmonMirrorList[type].lp);
}


/*-------------------------------------------------------------------------*/
int qmonMirrorMulti(
u_long32 selector
) {
   return qmonMirrorMultiAnswer(selector, NULL);
}   


/*-------------------------------------------------------------------------*/
int qmonMirrorMultiAnswer(
u_long32 selector,
lList **answerp
) {
   lList *mal = NULL;
   lList *alp = NULL;
   lListElem *aep = NULL;
   int i;
   int count = 0;
   int current = 0;
   int index;
   char msg[BUFSIZ];
   state_gdi_multi state = STATE_GDI_MULTI_INIT;

   DENTER(GUI_LAYER, "qmonMirrorMultiAnswer");

   sge_stopwatch_start(0);

   for (i=0; i<XtNumber(QmonMirrorList); i++) {
      if (selector & (1<<i))
         count++;
   }

   if (ctx->is_alive(ctx) != CL_RETVAL_OK) {
      snprintf(msg, sizeof(msg), "%s",
               XmtLocalize(AppShell, "cannot reach qmaster", "cannot reach qmaster"));
      contact_ok = XmtDisplayErrorAndAsk(AppShell, "nocontact",
                                                msg, "@{Retry}", "@{Abort}",
                                                XmtYesButton, NULL);
      /*
      ** we don't want to retry, so go down
      */
      if (!contact_ok) {
         DEXIT;
         qmonExitFunc(1);
      }
      /*
      ** re-read act_qmaster file
      */
      ctx->get_master(ctx, true);
   }

   for (i=0; i<XtNumber(QmonMirrorList); i++) {
      if (selector & (1<<i)) {
         int mode = (++current == count) ? SGE_GDI_SEND : SGE_GDI_RECORD;
         index = i + 1;
         QmonMirrorList[index].id = ctx->gdi_multi(ctx, &alp, mode,
                                 QmonMirrorList[index].type, 
                                 SGE_GDI_GET,
                                 NULL, 
                                 QmonMirrorList[index].where, 
                                 QmonMirrorList[index].what,
                                 false, &state, true);
         if (mode == SGE_GDI_SEND) {
            ctx->gdi_wait(ctx, &alp, &mal, &state);
         }
         if (answer_list_has_error(&alp)) {
            goto error;
         }   
      }   
   }      

   for (i=0; i<XtNumber(QmonMirrorList); i++) {
      if (selector & (1<<i)) {
         index = i + 1;
         sge_gdi_extract_answer(&alp, SGE_GDI_GET, QmonMirrorList[index].type,
                        QmonMirrorList[index].id, mal, 
                        &(QmonMirrorList[index].lp));
         for_each (aep, alp) {
            if (lGetUlong(aep, AN_status) == STATUS_EVERSION) {
               fprintf(stderr, "%s\n", lGetString(aep, AN_text));
               DEXIT;
               qmonExitFunc(1);
            }   
            if (lGetUlong(aep, AN_status) != STATUS_OK) {
               if (!answerp) {
                  fprintf(stderr, "%s\n", lGetString(aep, AN_text));
               }   
               goto error;
            }   
         }      
         lFreeList(&alp);
      }
   }
   lFreeList(&mal);
   sge_stopwatch_log(0, "qmonMirrorMulti:");

   DEXIT;
   return 0;

   error:
      lFreeList(&mal);
      if (answerp) {
         *answerp = alp;
      }
      else {
         lFreeList(&alp);
      }   
      DEXIT;
      return -1;
}

/*-------------------------------------------------------------------------*/
lList* qmonDelList(
int type,
lList **local,
int nm,
lList **lpp,
lCondition *where,
lEnumeration *what 
) {

   lList *alp = NULL;
   lListElem *alep = NULL;
   lListElem *ep = NULL;
   const lDescr *listDescriptor = NULL;
   int dataType;

   DENTER(GUI_LAYER, "qmonDelList");

   if (!lpp) {
      answer_list_add(&alp, "lpp is NULL", 
                      STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      DEXIT;
      return alp;
   }

   /* 
    * get the answer list for error reporting 
    * alp contains several answer elements for 
    * SGE_GDI_DEL
    */
   sge_stopwatch_start(0);
   
   alp = ctx->gdi(ctx, type, SGE_GDI_DEL, lpp, where, what, false); 

   if (type == SGE_JB_LIST) {
#if 0
      /* modify local list */
      for_each2(alep, alp, ep, *lpp) {
         if ( lGetUlong(alep, AN_status) == STATUS_OK) {
            u_long32 jid = atol(lGetString(ep, ID_str));
            DPRINTF(("Job: " sge_u32 "gets removed\n", jid));
            if (local) {
               job_list_locate(local, jid); 
            }
            else {
               sep = job_list_locate(qmonMirrorList(SGE_JB_LIST), jid);
               if (sep) {
                  lListElem *jatep;
                  for_each (jatep, lGetList(sep, JB_ja_tasks)) {
                     state = lGetUlong(jatep, JAT_state);
                     state |= JDELETED;
                     lSetUlong(jatep, JAT_state, state);
                  }
               }
            }
         }
      }

/*
printf("Jobs after delete_________________\n");
      for_each(ep, *local)
         printf("Job: %d\n", lGetUlong(ep, JB_job_number));
printf("__________________________________\n");
*/
#endif
   }
   else { /* type != SGE_JB_LIST */
      /* modify local list */
      for_each2(alep, alp, ep, *lpp) {
         if ( lGetUlong(alep, AN_status) == STATUS_OK) {
            listDescriptor = lGetListDescr(*local);
            dataType = lGetPosType(listDescriptor, lGetPosInDescr(listDescriptor, nm));
            switch (dataType) {
               case lStringT:
                  lDelElemStr(local, nm, lGetString(ep, nm)); 
                  break;
               case lHostT:
                  lDelElemHost(local, nm, lGetHost(ep, nm));
                  break;
               default:
                  answer_list_add(&alp, "data type not lStringT or lHostT", 
                                  STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
                  DPRINTF(("qmonDelList: data type not lStringT or lHostT\n"));
                  break;
            }
         }
      }
   }

   sge_stopwatch_log(0, "SGE_GDI_DEL:");
   
   DEXIT;
   return alp; 

}


/*-------------------------------------------------------------------------*/
lList* qmonAddList(
int type,
lList **local,
int nm,
lList **lpp,
lCondition *where,
lEnumeration *what 
) {

   lList *alp = NULL;
   lListElem *alep = NULL, *ep = NULL;
   

   DENTER(GUI_LAYER, "qmonAddList");

   if (!what ) 
      goto error;
   
   sge_stopwatch_start(0);
   
   if (type == SGE_JB_LIST || type == SGE_AR_LIST) {
      alp = ctx->gdi(ctx, type, SGE_GDI_ADD | SGE_GDI_RETURN_NEW_VERSION, 
                        lpp, where, what, false);
   } else {
      alp = ctx->gdi(ctx, type, SGE_GDI_ADD, lpp, where, what, false); 
   }

   for_each2(alep, alp, ep, *lpp) {
      if ( lGetUlong(alep, AN_status) == STATUS_OK) {
         if (!*local)
            *local = lCreateList(lGetListName(*lpp), lGetListDescr(*lpp));
         lAppendElem(*local, lCopyElem(ep)); 
      }
   }

   sge_stopwatch_log(0, "SGE_GDI_ADD:");
   
   DEXIT;
   return alp; 

   error:
      fprintf(stderr, "error: qmonAddList\n");
      DEXIT;
      return NULL;

}


/*-------------------------------------------------------------------------*/
lList* mod_gdi(
u_long32 target,
u_long32 cmd,
lList **lpp,
lCondition *cp,
lEnumeration *enp 
) {
   /* Note: I think that sge_gdi should be generalized to reduce the list
      for you, when an enumeration is supplied on a sge_gdi(SGE_GDI_MOD),
      but for now, I'll do it here. */
   lList *ans;
   lList *lp = NULL;

   if (enp) {
      lp = lSelect("", *lpp, NULL, enp);
   }   

   ans = ctx->gdi(ctx, target, SGE_GDI_MOD, lp ? &lp : lpp, cp, enp, false);

   lFreeList(&lp);

   return ans;
}


/*-------------------------------------------------------------------------*/
lList* qmonModList(
int type,
lList **local,
int nm,
lList **lpp,
lCondition *where,
lEnumeration *what 
) {

   lList *alp = NULL;
   lListElem *alep = NULL;
   lListElem *ep = NULL;
   lListElem *prev = NULL;
   lListElem *rem = NULL;
   const lDescr *listDescriptor = NULL;
   int dataType;

   
   DENTER(GUI_LAYER, "qmonModList");

   if (!what ) 
      goto error;
   
   /* 
    * get the answer list for error reporting 
    * alp contains several answer elements for 
    * SGE_GDI_MOD
    */
   sge_stopwatch_start(0);

   alp = ctx->gdi(ctx, type, SGE_GDI_MOD, lpp, where, what, false); 

   if (!(type == SGE_SC_LIST || type == SGE_STN_LIST)) {
      /* right now we change the whole element */
      for_each2(alep, alp, ep, *lpp) {
         if ( lGetUlong(alep, AN_status) == STATUS_OK) {
            if (!*local && *lpp)
               *local = lCreateList(lGetListName(*lpp), lGetListDescr(*lpp));
            else {
               if (nm == JB_job_number) {
                  rem = job_list_locate(*local, lGetUlong(ep, nm));
               } else {
                  listDescriptor = lGetListDescr(*local);
                  dataType = lGetPosType(listDescriptor, lGetPosInDescr(listDescriptor, nm));
                  rem = NULL;
                  switch (dataType) {
                     case lStringT:
                        rem = lGetElemStr(*local, nm, lGetString(ep, nm));
                        break;
                     case lHostT:
                        rem = lGetElemHost(*local, nm, lGetHost(ep, nm));
                        break;
                     default:
                        DPRINTF(("qmonModList: data type not lStringT or lHostT\n"));
                        goto error;
                  }
               } 
            }
            if (rem) {
               prev = lPrev(prev);
               lRemoveElem(*local, &rem);
            }
            lInsertElem(*local, prev, lCopyElem(ep)); 
         }
      }
   } 
   sge_stopwatch_log(0, "SGE_GDI_MOD:");
   
   DEXIT;
   return alp; 

   error:
      fprintf(stderr, "error: qmonModList\n");
      DEXIT;
      return NULL;

}


/*-------------------------------------------------------------------------*/
lList* qmonChangeStateList(
int type,
lList *lp,
int force,
int action 
) {
   lList *id_list = NULL;
   lList *alp = NULL;
   
   DENTER(GUI_LAYER, "qmonChangeStateList");

   if (!lp) {
      DEXIT;
      return NULL;
   }
      
   /* 
    * get the answer list for error reporting 
    * alp contains several answer elements for 
    */
   sge_stopwatch_start(0);
  
   if (id_list_build_from_str_list(&id_list, &alp, lp, action, force)) {
      alp = ctx->gdi(ctx, SGE_CQ_LIST, SGE_GDI_TRIGGER, &id_list, NULL, NULL, false);
      lFreeList(&id_list);
   }

   qmonMirrorMultiAnswer(l2s(type), &alp);
   
   sge_stopwatch_log(0, "CHANGE_STATE:");
   
   DEXIT;
   return alp; 
}



/*-------------------------------------------------------------------------*/
void qmonShowMirrorList(Widget w, XtPointer cld, XtPointer cad)
{
   int type = (int)(long) cld;
   
   DENTER(GUI_LAYER, "qmonShowMirrorList");

   printf("___________________________________________________________\n");
   printf("___________________________________________________________\n");
   lWriteListTo( qmonMirrorList(type), stdout);
   printf("___________________________________________________________\n");
   printf("___________________________________________________________\n");

   DEXIT;
}

u_long32 l2s(
u_long32 ltype 
) {
   return  QmonMirrorList[ltype].selector;
}   
