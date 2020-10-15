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

#include <X11/IntrinsicP.h>

#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/ToggleB.h>

#include <Xmt/Xmt.h>
#include <Xmt/Create.h>
#include <Xmt/Chooser.h>
#include <Xmt/Layout.h>
#include <Xmt/Dialog.h>
#include <Xmt/Dialogs.h>
#include <Xmt/MsgLine.h>
#include <Xmt/InputField.h>
#include <Xmt/Procedures.h>


/*----------------------------------------------------------------------------*/
#include "qmon_proto.h"
#include "qmon_rmon.h"
#include "qmon_util.h"
#include "qmon_qaction.h"
#include "qmon_comm.h"
#include "qmon_load.h"
#include "qmon_quarks.h"
#include "qmon_message.h"
#include "qmon_init.h"
#include "qmon_manop.h"
#include "qmon_cq.h"
#include "qmon_pe.h"
#include "qmon_ckpt.h"
#include "qmon_globals.h"
#include "qmon_project.h"
#include "AskForTime.h"
#include "Matrix.h"

#include "uti/sge_string.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_attr.h"

#include "comm/commlib.h"

#include "gdi/sge_gdi_ctx.h"

#include "sge_cqueue_qconf.h"

extern sge_gdi_ctx_class_t *ctx;


/*-------------------------------------------------------------------------*/

/* 
 * typedefs for all the resources connected to the dialogs 
 * additional typedefs for transfer cull <--> structs
 */

/* queue configuration parameters -> gdilib/sge_queueL.h */

typedef struct _tQCEntry {
   char *cq_name;
   char *current_href;
   lList *cq_hostlist;
/*---- general queue config -----*/
   u_long32 qtype;
   bool qtype_tw;
   const char *processors;
   bool processors_tw;
   int priority;
   bool priority_tw;
   u_long32 job_slots;
   bool job_slots_tw;
   bool rerun;
   bool rerun_tw;
   const char *tmpdir;
   bool tmpdir_tw;
   const char *calendar;
   bool calendar_tw;
   const char *shell;
   bool shell_tw;
   u_long32 shell_start_mode;
   bool shell_start_mode_tw;
   const char *notify;
   bool notify_tw;
   u_long32 seq_no;
   bool seq_no_tw;
   u_long32 initial_state;
   bool initial_state_tw;
/*---- methods -----*/
   const char *prolog;
   bool prolog_tw;
   const char *epilog;
   bool epilog_tw;
   const char *starter_method;
   bool starter_method_tw;
   const char *suspend_method;
   bool suspend_method_tw;
   const char *resume_method;
   bool resume_method_tw;
   const char *terminate_method;
   bool terminate_method_tw;
/*---- checkpointing -----*/
   const char *min_cpu_interval;
   bool min_cpu_interval_tw;
   lList *ckpt_list;
   bool ckpt_list_tw;
/*---- pe -----*/
   lList *pe_list;
   bool pe_list_tw;
/*---- load thresholds ----*/
   lList *load_thresholds;
   bool load_thresholds_tw;
/*---- suspend thresholds ----*/
   lList *suspend_thresholds;
   bool suspend_thresholds_tw;
   const char *suspend_interval;
   bool suspend_interval_tw;
   u_long32 nsuspend;
   bool nsuspend_tw;
/*---- queue limits  ----*/
   const char *s_rt;
   bool s_rt_tw;
   const char *s_cpu;
   bool s_cpu_tw;
   const char *s_fsize;
   bool s_fsize_tw;
   const char *s_data;
   bool s_data_tw;
   const char *s_stack;
   bool s_stack_tw;
   const char *s_core;
   bool s_core_tw;
   const char *s_rss;
   bool s_rss_tw;
   const char *s_vmem;
   bool s_vmem_tw;
   const char *h_rt;
   bool h_rt_tw;
   const char *h_cpu;
   bool h_cpu_tw;
   const char *h_fsize;
   bool h_fsize_tw;
   const char *h_data;
   bool h_data_tw;
   const char *h_stack;
   bool h_stack_tw;
   const char *h_core;
   bool h_core_tw;
   const char *h_rss;
   bool h_rss_tw;
   const char *h_vmem;
   bool h_vmem_tw;
/*---- complexes  ----*/
   lList *consumable_config_list;
   bool consumable_config_list_tw;
/*---- user access ---*/
   lList *acl;
   bool acl_tw;
   lList *xacl;
   bool xacl_tw;
/*---- project access ---*/
   lList *prj;
   bool prj_tw;
   lList *xprj;
   bool xprj_tw;
/*---- owner list ----*/
   lList *owner_list;
   bool owner_list_tw;
/*---- subordinate list ----*/
   lList *subordinate_list;
   bool subordinate_list_tw;
}  tQCEntry;

   
typedef struct _tAction {
   String label;
   XtCallbackProc callback;
} tAction;



/*---- queue configuration -----*/
XtResource cq_resources[] = {
   { "cq_name", "cq_name", XtRString, 
      sizeof(char*), XtOffsetOf(tQCEntry, cq_name), 
      XtRImmediate, NULL },

   { "cq_hostlist", "cq_hostlist", QmonRHR_Type, 
      sizeof(lList *), XtOffsetOf(tQCEntry, cq_hostlist), 
      XtRImmediate, NULL },

   /*---- attributes  -----*/
   { "qtype", "qtype", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, qtype),
      XtRImmediate, NULL },
      
   { "qtype_tw", "qtype_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, qtype_tw),
      XtRImmediate, NULL },
      
   { "processors", "processors", XtRString, 
      sizeof(char*), XtOffsetOf(tQCEntry, processors), 
      XtRImmediate, NULL },
      
   { "processors_tw", "processors_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, processors_tw),
      XtRImmediate, NULL },
      
   { "priority", "priority", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, priority),
      XtRImmediate, NULL },
   
   { "priority_tw", "priority_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, priority_tw),
      XtRImmediate, NULL },
      
   { "job_slots", "job_slots", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, job_slots),
      XtRImmediate, NULL },
      
   { "job_slots_tw", "job_slots_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, job_slots_tw),
      XtRImmediate, NULL },
      
   { "rerun", "rerun", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, rerun),
      XtRImmediate, NULL },
      
   { "rerun_tw", "rerun_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, rerun_tw),
      XtRImmediate, NULL },
      
   { "tmpdir", "tmpdir", XtRString, 
      sizeof(char*), XtOffsetOf(tQCEntry, tmpdir), 
      XtRImmediate, NULL },
      
   { "tmpdir_tw", "tmpdir_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, tmpdir_tw),
      XtRImmediate, NULL },
      
   { "calendar", "calendar", XtRString, 
      sizeof(char*), XtOffsetOf(tQCEntry, calendar), 
      XtRImmediate, NULL },
      
   { "calendar_tw", "calendar_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, calendar_tw),
      XtRImmediate, NULL },
      
   { "shell", "shell", XtRString, 
      sizeof(char*), XtOffsetOf(tQCEntry, shell), 
      XtRImmediate, NULL },
      
   { "shell_tw", "shell_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, shell_tw),
      XtRImmediate, NULL },
      
   { "notify", "notify", XtRString, 
      sizeof(char*), XtOffsetOf(tQCEntry, notify), 
      XtRImmediate, NULL },

   { "notify_tw", "notify_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, notify_tw),
      XtRImmediate, NULL },
      
   { "seq_no", "seq_no", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, seq_no),
      XtRInt, 0 },

   { "seq_no_tw", "seq_no_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, seq_no_tw),
      XtRImmediate, NULL },
      
   { "initial_state", "initial_state", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, initial_state),
      XtRInt, 0 },

   { "initial_state_tw", "initial_state_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, initial_state_tw),
      XtRImmediate, NULL },
      
   { "shell_start_mode", "shell_start_mode", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, shell_start_mode),
      XtRInt, 0 },

   { "shell_start_mode_tw", "shell_start_mode_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, shell_start_mode_tw),
      XtRImmediate, NULL },
      
/*---- methods -----*/
   { "prolog", "prolog", XtRString,
      sizeof(char *), XtOffsetOf(tQCEntry, prolog),
      XtRImmediate, NULL },

   { "prolog_tw", "prolog_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, prolog_tw),
      XtRImmediate, NULL },
      
   { "epilog", "epilog", XtRString,
      sizeof(char *), XtOffsetOf(tQCEntry, epilog),
      XtRImmediate, NULL },

   { "epilog_tw", "epilog_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, epilog_tw),
      XtRImmediate, NULL },
      
   { "starter_method", "starter_method", XtRString,
      sizeof(char *), XtOffsetOf(tQCEntry, starter_method),
      XtRImmediate, NULL },

   { "starter_method_tw", "starter_method_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, starter_method_tw),
      XtRImmediate, NULL },
      
   { "suspend_method", "suspend_method", XtRString,
      sizeof(char *), XtOffsetOf(tQCEntry, suspend_method),
      XtRImmediate, NULL },

   { "suspend_method_tw", "suspend_method_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, suspend_method_tw),
      XtRImmediate, NULL },
      
   { "resume_method", "resume_method", XtRString,
      sizeof(char *), XtOffsetOf(tQCEntry, resume_method),
      XtRImmediate, NULL },

   { "resume_method_tw", "resume_method_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, resume_method_tw),
      XtRImmediate, NULL },
      
   { "terminate_method", "terminate_method", XtRString,
      sizeof(char *), XtOffsetOf(tQCEntry, terminate_method),
      XtRImmediate, NULL },

   { "terminate_method_tw", "terminate_method_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, terminate_method_tw),
      XtRImmediate, NULL },
      
/*---- checkpointing -----*/
   { "min_cpu_interval", "min_cpu_interval", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, min_cpu_interval),
      XtRImmediate, NULL },

   { "min_cpu_interval_tw", "min_cpu_interval_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, min_cpu_interval_tw),
      XtRImmediate, NULL },
      
   { "ckpt_list", "ckpt_list", QmonRSTR_Type,
      sizeof(lList *), XtOffsetOf(tQCEntry, ckpt_list),
      XtRImmediate, NULL },

   { "ckpt_list_tw", "ckpt_list_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, ckpt_list_tw),
      XtRImmediate, NULL },
      
/*---- pe -----*/
   { "pe_list", "pe_list", QmonRSTR_Type,
      sizeof(lList *), XtOffsetOf(tQCEntry, pe_list),
      XtRImmediate, NULL },
   
   { "pe_list_tw", "pe_list_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, pe_list_tw),
      XtRImmediate, NULL },
      
/*---- load thresholds ----*/
   { "load_thresholds", "load_thresholds", QmonRCE2_Type,
      sizeof(lList *), XtOffsetOf(tQCEntry, load_thresholds),
      XtRImmediate, NULL },

   { "load_thresholds_tw", "load_thresholds_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, load_thresholds_tw),
      XtRImmediate, NULL },
      
/*---- suspend thresholds ----*/
   { "suspend_thresholds", "suspend_thresholds", QmonRCE2_Type,
      sizeof(lList *), XtOffsetOf(tQCEntry, suspend_thresholds),
      XtRImmediate, NULL },

   { "suspend_thresholds_tw", "suspend_thresholds_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, suspend_thresholds_tw),
      XtRImmediate, NULL },
      
   { "suspend_interval", "suspend_interval", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, suspend_interval),
      XtRImmediate, NULL },

   { "suspend_interval_tw", "suspend_interval_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, suspend_interval_tw),
      XtRImmediate, NULL },
      
   { "nsuspend", "nsuspend", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, nsuspend),
      XtRImmediate, NULL },

   { "nsuspend_tw", "nsuspend_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, nsuspend_tw),
      XtRImmediate, NULL },
      
/*---- queue limits  ----*/
   { "h_rt", "h_rt", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, h_rt),
      XtRImmediate, NULL },

   { "h_rt_tw", "h_rt_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, h_rt_tw),
      XtRImmediate, NULL },

   { "h_cpu", "h_cpu", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, h_cpu),
      XtRImmediate, NULL },

   { "h_cpu_tw", "h_cpu_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, h_cpu_tw),
      XtRImmediate, NULL },

   { "h_fsize", "h_fsize", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, h_fsize),
      XtRImmediate, NULL },

   { "h_fsize_tw", "h_fsize_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, h_fsize_tw),
      XtRImmediate, NULL },

   { "h_data", "h_data", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, h_data),
      XtRImmediate, NULL },

   { "h_data_tw", "h_data_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, h_data_tw),
      XtRImmediate, NULL },

   { "h_stack", "h_stack", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, h_stack),
      XtRImmediate, NULL },

   { "h_stack_tw", "h_stack_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, h_stack_tw),
      XtRImmediate, NULL },

   { "h_core", "h_core", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, h_core),
      XtRImmediate, NULL },

   { "h_core_tw", "h_core_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, h_core_tw),
      XtRImmediate, NULL },

   { "h_rss", "h_rss", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, h_rss),
      XtRImmediate, NULL },

   { "h_rss_tw", "h_rss_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, h_rss_tw),
      XtRImmediate, NULL },

   { "h_vmem", "h_vmem", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, h_vmem),
      XtRImmediate, NULL },

   { "h_vmem_tw", "h_vmem_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, h_vmem_tw),
      XtRImmediate, NULL },

   { "s_rt", "s_rt", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, s_rt),
      XtRImmediate, NULL },

   { "s_rt_tw", "s_rt_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, s_rt_tw),
      XtRImmediate, NULL },

   { "s_cpu", "s_cpu", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, s_cpu),
      XtRImmediate, NULL },

   { "s_cpu_tw", "s_cpu_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, s_cpu_tw),
      XtRImmediate, NULL },

   { "s_fsize", "s_fsize", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, s_fsize),
      XtRImmediate, NULL },

   { "s_fsize_tw", "s_fsize_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, s_fsize_tw),
      XtRImmediate, NULL },

   { "s_data", "s_data", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, s_data),
      XtRImmediate, NULL },

   { "s_data_tw", "s_data_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, s_data_tw),
      XtRImmediate, NULL },

   { "s_stack", "s_stack", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, s_stack),
      XtRImmediate, NULL },

   { "s_stack_tw", "s_stack_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, s_stack_tw),
      XtRImmediate, NULL },

   { "s_core", "s_core", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, s_core),
      XtRImmediate, NULL },

   { "s_core_tw", "s_core_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, s_core_tw),
      XtRImmediate, NULL },

   { "s_rss", "s_rss", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, s_rss),
      XtRImmediate, NULL },

   { "s_rss_tw", "s_rss_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, s_rss_tw),
      XtRImmediate, NULL },

   { "s_vmem", "s_vmem", XtRString,
      sizeof(char*), XtOffsetOf(tQCEntry, s_vmem),
      XtRImmediate, NULL },

   { "s_vmem_tw", "s_vmem_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, s_vmem_tw),
      XtRImmediate, NULL },

/*---- complexes ----*/
   { "consumable_config_list", "consumable_config_list", QmonRCE2_Type,
      sizeof(lList *), XtOffsetOf(tQCEntry, consumable_config_list),
      XtRImmediate, NULL },
      
   { "consumable_config_list_tw", "consumable_config_list_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, consumable_config_list_tw),
      XtRImmediate, NULL },
      
/*---- subordinates ----*/
   { "subordinate_list", "subordinate_list", QmonRSO_Type,
      sizeof(lList *), XtOffsetOf(tQCEntry, subordinate_list),
      XtRImmediate, NULL },
      
   { "subordinate_list_tw", "subordinate_list_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, subordinate_list_tw),
      XtRImmediate, NULL },
      
/*---- access  ----*/
   { "acl", "acl", QmonRUS_Type,
      sizeof(lList *), XtOffsetOf(tQCEntry, acl),
      XtRImmediate, NULL },
   
   { "acl_tw", "acl_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, acl_tw),
      XtRImmediate, NULL },
      
   { "xacl", "xacl", QmonRUS_Type,
      sizeof(lList *), XtOffsetOf(tQCEntry, xacl),
      XtRImmediate, NULL },
      
   { "xacl_tw", "xacl_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, xacl_tw),
      XtRImmediate, NULL },
      
/*---- projects  ----*/
   { "prj", "prj", QmonRPR_Type,
      sizeof(lList *), XtOffsetOf(tQCEntry, prj),
      XtRImmediate, NULL },
   
   { "prj_tw", "prj_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, prj_tw),
      XtRImmediate, NULL },
      
   { "xprj", "xprj", QmonRPR_Type,
      sizeof(lList *), XtOffsetOf(tQCEntry, xprj),
      XtRImmediate, NULL },
      
   { "xprj_tw", "xprj_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, xprj_tw),
      XtRImmediate, NULL },
      
/*---- owners  ----*/
   { "owner_list", "owner_list", QmonRUS_Type,
      sizeof(lList *), XtOffsetOf(tQCEntry, owner_list),
      XtRImmediate, NULL },
      
   { "owner_list_tw", "owner_list_tw", XtRInt,
      sizeof(int), XtOffsetOf(tQCEntry, owner_list_tw),
      XtRImmediate, NULL },
      
};



/*-------------------------------------------------------------------------*/
static Widget qmonQCCreate(Widget parent);
static void qmonQCToggleAction(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCAccessToggle(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCAccessAdd(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCAccessRemove(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCPEAdd(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCPERemove(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCCkptAdd(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCCkptRemove(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCProjectToggle(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCProjectAdd(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCProjectRemove(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCOwnerAdd(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCCheckName(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCUpdate(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCCalendar(Widget w, XtPointer cld, XtPointer cad);

static void qmonQCTime(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCMem(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCSOQ(Widget w, XtPointer cld, XtPointer cad);
static void qmonInitQCEntry(tQCEntry *data);
static void qmonQCSetData(tQCEntry *data, StringConst qname, StringConst href);

static void qmonQCAdd(Widget w, XtPointer cld, XtPointer cad);

static void updateQCA(void);
static void updateQCP(void);
static void updateQCPE(void);
static void updateQCCkpt(void);

static Boolean qmonCullToCQ(lListElem *qep, tQCEntry *data, const char *href);
static Boolean qmonCQToCull(tQCEntry *data, lListElem *qep, const char *href);
static Boolean qmonCQDeleteHrefAttr(lListElem *qep, const char *href);
static void qmonCQAddHref(Widget w, XtPointer cld, XtPointer cad);
static void qmonCQDeleteHref(Widget w, XtPointer cld, XtPointer cad);
static void qmonCQAddHost(Widget w, XtPointer cld, XtPointer cad);
static void qmonCQHrefSelect(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCToggleTW(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCSetAllTogglesSensitive(Widget w, XtPointer cld, XtPointer cad);

/*-------------------------------------------------------------------------*/
static Widget cq_dialog = 0;
static Widget cq_title = 0;
static Widget cq_apply = 0;
static Widget cq_folder = 0;
static Widget cq_name = 0;
static Widget cq_hostlist_w = 0;
static Widget cq_host_add_w = 0;
static Widget cq_host_delete_w = 0;
static Widget cq_host_input_w = 0;
static Widget cq_href_add_w = 0;
static Widget cq_href_delete_w = 0;
static Widget cq_href_input_w = 0;
static Widget cq_href_select_list_w = 0;
static Widget subordinates_attached = 0;
static Widget access_list = 0;
static Widget access_allow = 0;
static Widget access_deny = 0;
static Widget access_toggle = 0;
static Widget project_list = 0;
static Widget project_allow = 0;
static Widget project_deny = 0;
static Widget project_toggle = 0;
static Widget owner_list = 0;
static Widget owner_new = 0;
static Widget ckpt_reference_list = 0;
static Widget ckpt_list = 0;
static Widget pe_reference_list = 0;
static Widget pe_list = 0;
static Widget suspend_interval = 0;
static Widget access_allow_tw, access_deny_tw, ckpt_reference_list_tw,
          min_cpu_interval_tw, consumable_config_list_tw, subordinate_list_tw,
          calendar_tw, initial_state_tw, job_slots_tw, notify_tw, priority_tw,
          processors_tw, qtype_tw, rerun_tw, seq_no_tw, shell_start_mode_tw,
          shell_tw, tmpdir_tw, h_core_tw, h_cpu_tw, h_data_tw, h_fsize_tw,
          h_rss_tw, h_rt_tw, h_stack_tw, h_vmem_tw, s_core_tw, s_cpu_tw,
          s_data_tw, s_fsize_tw, s_rss_tw, s_rt_tw, s_stack_tw, s_vmem_tw,
          load_thresholds_tw, suspend_interval_tw, suspend_nsuspend_tw,
          suspend_thresholds_tw, epilog_tw, prolog_tw, resume_method_tw,
          starter_method_tw, suspend_method_tw, terminate_method_tw,
          owner_list_tw, pe_reference_list_tw, project_allow_tw,
          project_deny_tw;

static int dialog_mode = QC_ADD;
static int dont_close = 0;

/*
**  hold the tQCEntry data
*/
static tQCEntry current_entry; 
static lListElem *current_qep = NULL;
static char *old_href = HOSTREF_DEFAULT;

/*
** this depends on the order of the sub dialogs in the
** option menu button !!!
*/
   
enum {
   QC_ALMOST = -2,
   QC_ALL,
   QC_GENERAL,
   QC_CHECKPOINT,
   QC_PE,
   QC_LIMIT,
   QC_LOADTHRESHOLD, 
   QC_COMPLEXES,
   QC_USERSET,
   QC_PROJECT,
   QC_OWNER,
   QC_SUBORDINATES,
   QC_SUSPENDTHRESHOLD 
};





/*-------------------------------------------------------------------------*/
/* P U B L I C                                                             */ 
/*-------------------------------------------------------------------------*/
void qmonQCPopup(Widget w, XtPointer cld, XtPointer cad)
{
   Widget layout;
   XmString title;
   const char *qname = NULL;
   XmString xmhost = NULL;
   tQCAction *qc_action = (tQCAction *)cld;
   
   DENTER(TOP_LAYER, "qmonQCPopup");

   /*
   ** popup the dialog in the specified mode
   */
   if (qc_action) {
      dialog_mode = qc_action->action;
      qname = qc_action->qname;
   } else {
      dialog_mode = QC_ADD;
   }   

   /* 
   ** set busy cursor 
   */
   XmtDisplayBusyCursor(w);

   /* 
   ** create the dialog if necessary 
   */
   if (!cq_dialog) {
      layout = XmtGetTopLevelShell(w);
      cq_dialog = qmonQCCreate(layout);
      /*
      ** initialize current entry to zero
      */
      qmonInitQCEntry(&current_entry);
   }

   /*
   ** set the dialog title
   */
   if (dialog_mode == QC_ADD) {
      title = XmtCreateLocalizedXmString(cq_dialog, "@{@fBQueue Configuration - Add}");
   } else if (dialog_mode == QC_CLONE) {
      title = XmtCreateLocalizedXmString(cq_dialog, "@{@fBQueue Configuration - Clone}");
   } else {
      char buf[BUFSIZ];
      sprintf(buf, "%s %s", XmtLocalize(w, "Modify", "@{Modify}"), qname); 
      title = XmtCreateXmString(buf); 
   }
   
   XtVaSetValues( cq_dialog,
                  XmNdialogTitle, title,
                  NULL);
   XtVaSetValues( cq_title,
                  XmtNlabelString, title,
                  NULL);
   XmStringFree(title);

   /*
   ** we open the qconf dialog with the template queue displayed 
   ** the Apply ad Reset buttons are set insensitive
   */
   old_href = HOSTREF_DEFAULT;
   if (!qname) {
      qmonQCSetData(&current_entry, "template", HOSTREF_DEFAULT );
   } else {
      qmonQCSetData(&current_entry, qname, HOSTREF_DEFAULT);
   }
   XmtDialogSetDialogValues(cq_dialog, &current_entry);

   xmhost = XmStringCreateLocalized((char *) HOSTREF_DEFAULT);
   XmListSelectItem(cq_href_select_list_w, xmhost, True);
   XmStringFree(xmhost);

   if (dialog_mode == QC_ADD) {
      XtSetSensitive(cq_apply, False);
      XtSetSensitive(cq_name, True);
   } else if (dialog_mode == QC_CLONE) {
      XmtInputFieldSetString(cq_name, "");
      XtSetSensitive(cq_apply, False);
      XtSetSensitive(cq_name, True);
   } else {   
      XtSetSensitive(cq_name, False);
   }   
   qmonQCUpdate(w, NULL, NULL);
   
   /*
   ** display the correct dialog
   */
   qmonQCToggleAction(cq_dialog, NULL, NULL);

   /* 
   ** pop the dialog up, Motif 1.1 gets confused if we don't popup the shell 
   */
   XtManageChild(cq_dialog);

   /* 
   **  stop the polling of lists totally
   */
/*    qmonStopPolling(); */

   /* set normal cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
void qmonQCPopdown(Widget w, XtPointer cld, XtPointer cad)
{
   
   DENTER(TOP_LAYER, "qmonQCPopdown");

   if (cq_dialog && XtIsManaged(cq_dialog)) {

      lFreeElem(&current_qep);
   
      XtUnmanageChild(cq_dialog);
      XtPopdown(XtParent(cq_dialog));

      /*
      ** Sync the display
      */
      XSync(XtDisplay(cq_dialog), 0);
      XmUpdateDisplay(cq_dialog);

      /*
      ** force update
      */
      qmonCQUpdate(w, NULL, NULL);

   }
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
/* P R I V A T E                                                           */
/*-------------------------------------------------------------------------*/
static Widget qmonQCCreate(
Widget parent 
) {

   Widget dialog;
   Widget cq_cancel, cq_update, cq_main_link;
   Widget qtype, notify, notifyPB, calendar, calendarPB;
   Widget min_cpu_intervalPB, min_cpu_interval;
   Widget load_thresholds, load_delete;
   Widget suspend_thresholds, suspend_delete,
          suspend_intervalPB;
   Widget limits_hard, limits_soft;
   Widget complexes_dialog, complexes_ccl, consumable_delete;
   Widget access_add, access_remove, access_dialog;
   Widget project_add, project_remove, project_dialog, project_config;
   Widget owner_add, owner_remove;
   Widget pe_add, pe_remove, pe_dialog;
   Widget ckpt_add, ckpt_remove, ckpt_dialog;
   Widget h_corePB, h_cpuPB, h_dataPB, h_fsizePB, h_rssPB, h_rtPB, h_stackPB,
          h_vmemPB, s_corePB, s_cpuPB, s_dataPB, s_fsizePB, s_rssPB, s_rtPB,
          s_stackPB, s_vmemPB;
   Widget h_core, h_cpu, h_data, h_fsize, h_rss, h_rt, h_stack,
          h_vmem, s_core, s_cpu, s_data, s_fsize, s_rss, s_rt,
          s_stack, s_vmem;

   DENTER(TOP_LAYER, "qmonQCCreate");

   /*
   ** the resource file for cq_dialog_shell must contain 
   ** cq_dialog as unmanaged XmtLayout child otherwise
   ** it is managed by XmtBuildQueryDialog and managed again
   ** in qmonQCPopup
   */
   dialog = XmtBuildQueryDialog(parent, "cq_dialog_shell",
                              cq_resources, XtNumber(cq_resources),
                              "cq_folder", &cq_folder,
                              "cq_name", &cq_name,
                              "cq_apply", &cq_apply,
                              "cq_cancel", &cq_cancel,
                              "cq_update", &cq_update,
                              "cq_main_link", &cq_main_link,
                              "cq_title", &cq_title,
                              "cq_href_select_list", &cq_href_select_list_w,
                              "cq_href_input", &cq_href_input_w,
                              "cq_href_add", &cq_href_add_w,
                              "cq_href_delete", &cq_href_delete_w,
                              "cq_hostlist", &cq_hostlist_w,
                              "cq_host_input", &cq_host_input_w,
                              "cq_host_add", &cq_host_add_w,
                              "cq_host_delete", &cq_host_delete_w,
                              /* General Configuration */
                              "qtype", &qtype,
                              "notify", &notify,
                              "notifyPB", &notifyPB,
                              "calendar", &calendar,
                              "calendarPB", &calendarPB,
                              /* checkpoint_config */
                              "min_cpu_intervalPB", &min_cpu_intervalPB,
                              "min_cpu_interval", &min_cpu_interval,
                              "ckpt_list", &ckpt_list,
                              "ckpt_add", &ckpt_add,
                              "ckpt_remove", &ckpt_remove,
                              "ckpt_dialog", &ckpt_dialog,
                              "ckpt_reference_list", &ckpt_reference_list,
                              /* pe config */
                              "pe_list", &pe_list,
                              "pe_add", &pe_add,
                              "pe_remove", &pe_remove,
                              "pe_dialog", &pe_dialog,
                              "pe_reference_list", &pe_reference_list,
                              /* load_threshold_config */
                              "load_thresholds", &load_thresholds,
                              "load_delete", &load_delete,
                              /* suspend_threshold_config */
                              "suspend_thresholds", &suspend_thresholds,
                              "suspend_delete", &suspend_delete,
                              "suspend_interval", &suspend_interval,
                              "suspend_intervalPB", &suspend_intervalPB,
                              /* limit_config */ 
                              "limits_hard", &limits_hard,
                              "limits_soft", &limits_soft,
                              /* complexes_config */
                              "complexes_dialog", &complexes_dialog,
                              "consumable_delete", &consumable_delete,
                              "complexes_ccl", &complexes_ccl,
                              /* subordinates_config */
                              "subordinates_attached", &subordinates_attached,
                              /* access_config */
                              "access_list", &access_list,
                              "access_allow", &access_allow,
                              "access_deny", &access_deny,
                              "access_add", &access_add,
                              "access_remove", &access_remove,
                              "access_toggle", &access_toggle,
                              "access_dialog", &access_dialog,
                              /* project_config */
                              "project_list", &project_list,
                              "project_allow", &project_allow,
                              "project_deny", &project_deny,
                              "project_add", &project_add,
                              "project_remove", &project_remove,
                              "project_toggle", &project_toggle,
                              "project_dialog", &project_dialog,
                              "project_config", &project_config,
                              /* owner_config */
                              "owner_new", &owner_new,
                              "owner_add", &owner_add,
                              "owner_remove", &owner_remove,
                              "owner_list", &owner_list,
                              /* toggles */
                              "access_allow_tw", &access_allow_tw,
                              "access_deny_tw", &access_deny_tw,
                              "ckpt_reference_list_tw", &ckpt_reference_list_tw,
                              "min_cpu_interval_tw", &min_cpu_interval_tw,
                              "consumable_config_list_tw", &consumable_config_list_tw,
                              "subordinate_list_tw", &subordinate_list_tw,
                              "calendar_tw", &calendar_tw,
                              "initial_state_tw", &initial_state_tw,
                              "job_slots_tw", &job_slots_tw,
                              "notify_tw", &notify_tw,
                              "priority_tw", &priority_tw,
                              "processors_tw", &processors_tw,
                              "qtype_tw", &qtype_tw,
                              "rerun_tw", &rerun_tw,
                              "seq_no_tw", &seq_no_tw,
                              "shell_start_mode_tw", &shell_start_mode_tw,
                              "shell_tw", &shell_tw,
                              "tmpdir_tw", &tmpdir_tw,
                              "h_core_tw", &h_core_tw,
                              "h_cpu_tw", &h_cpu_tw,
                              "h_data_tw", &h_data_tw,
                              "h_fsize_tw", &h_fsize_tw,
                              "h_rss_tw", &h_rss_tw,
                              "h_rt_tw", &h_rt_tw,
                              "h_stack_tw", &h_stack_tw,
                              "h_vmem_tw", &h_vmem_tw,
                              "s_core_tw", &s_core_tw,
                              "s_cpu_tw", &s_cpu_tw,
                              "s_data_tw", &s_data_tw,
                              "s_fsize_tw", &s_fsize_tw,
                              "s_rss_tw", &s_rss_tw,
                              "s_rt_tw", &s_rt_tw,
                              "s_stack_tw", &s_stack_tw,
                              "s_vmem_tw", &s_vmem_tw,
                              "h_corePB", &h_corePB,
                              "h_cpuPB", &h_cpuPB,
                              "h_dataPB", &h_dataPB,
                              "h_fsizePB", &h_fsizePB,
                              "h_rssPB", &h_rssPB,
                              "h_rtPB", &h_rtPB,
                              "h_stackPB", &h_stackPB,
                              "h_vmemPB", &h_vmemPB,
                              "s_corePB", &s_corePB,
                              "s_cpuPB", &s_cpuPB,
                              "s_dataPB", &s_dataPB,
                              "s_fsizePB", &s_fsizePB,
                              "s_rssPB", &s_rssPB,
                              "s_rtPB", &s_rtPB,
                              "s_stackPB", &s_stackPB,
                              "s_vmemPB", &s_vmemPB,
                              "h_core", &h_core,
                              "h_cpu", &h_cpu,
                              "h_data", &h_data,
                              "h_fsize", &h_fsize,
                              "h_rss", &h_rss,
                              "h_rt", &h_rt,
                              "h_stack", &h_stack,
                              "h_vmem", &h_vmem,
                              "s_core", &s_core,
                              "s_cpu", &s_cpu,
                              "s_data", &s_data,
                              "s_fsize", &s_fsize,
                              "s_rss", &s_rss,
                              "s_rt", &s_rt,
                              "s_stack", &s_stack,
                              "s_vmem", &s_vmem,
                              "load_thresholds_tw", &load_thresholds_tw,
                              "suspend_interval_tw", &suspend_interval_tw,
                              "suspend_nsuspend_tw", &suspend_nsuspend_tw,
                              "suspend_thresholds_tw", &suspend_thresholds_tw,
                              "epilog_tw", &epilog_tw,
                              "prolog_tw", &prolog_tw,
                              "resume_method_tw", &resume_method_tw,
                              "starter_method_tw", &starter_method_tw,
                              "suspend_method_tw", &suspend_method_tw,
                              "terminate_method_tw", &terminate_method_tw,
                              "owner_list_tw", &owner_list_tw,
                              "pe_reference_list_tw", &pe_reference_list_tw,
                              "project_allow_tw", &project_allow_tw,
                              "project_deny_tw", &project_deny_tw,
                              NULL); 


   /*
   ** callbacks
   */
   XtAddCallback(cq_main_link, XmNactivateCallback, 
                  qmonMainControlRaise, NULL); 
   XtAddCallback(cq_apply, XmNactivateCallback, 
                  qmonQCAdd, NULL); 
   XtAddCallback(cq_update, XmNactivateCallback, 
                  qmonQCUpdate, NULL); 

   XtAddCallback(cq_cancel, XmNactivateCallback, 
                  qmonQCPopdown, NULL);
/*    XtAddCallback(cq_reset, XmNactivateCallback,  */
/*                   qmonQCResetAll, NULL); */

/*    XtAddCallback(cq_qhostname, XmtNverifyCallback,  */
/*                      qmonQCCheckHost, NULL); */
   XtAddCallback(cq_name, XmNvalueChangedCallback, 
                     qmonQCCheckName, NULL);
   
   XtAddCallback(cq_host_input_w, XmtNinputCallback, 
                  qmonCQAddHost, NULL); 
   XtAddCallback(cq_host_add_w, XmNactivateCallback, 
                  qmonCQAddHost, NULL); 
   XtAddCallback(cq_host_delete_w, XmNactivateCallback, 
                     DeleteItems, (XtPointer) cq_hostlist_w);

   XtAddCallback(cq_href_input_w, XmtNinputCallback, 
                  qmonCQAddHref, NULL); 
   XtAddCallback(cq_href_add_w, XmNactivateCallback, 
                  qmonCQAddHref, NULL); 
   XtAddCallback(cq_href_delete_w, XmNactivateCallback, 
                     qmonCQDeleteHref, (XtPointer) cq_href_select_list_w);
   XtAddCallback(cq_href_select_list_w, XmNbrowseSelectionCallback,
                     qmonCQHrefSelect, NULL);

   /* 
   ** General Config
   */
   XtAddCallback(notifyPB, XmNactivateCallback, 
                     qmonQCTime, (XtPointer)notify);
   XtAddCallback(calendarPB, XmNactivateCallback, 
                     qmonQCCalendar, (XtPointer)calendar);


   /*
   ** Load Thresholds
   */
#if 0   
   XtAddCallback(load_delete, XmNactivateCallback,
                  qmonLoadDelLines, (XtPointer) load_thresholds); 
   XtAddCallback(load_thresholds, XmNenterCellCallback,
                  qmonLoadNoEdit, NULL);
   XtAddCallback(load_thresholds, XmNselectCellCallback,
                  qmonLoadSelectEntry, NULL);
#endif
   XtAddCallback(load_thresholds, XmNlabelActivateCallback,
                  qmonLoadNames, NULL);

   /*
   ** Suspend Thresholds
   */
#if 0
   XtAddCallback(suspend_delete, XmNactivateCallback,
                  qmonLoadDelLines, (XtPointer) suspend_thresholds); 
   XtAddCallback(suspend_thresholds, XmNenterCellCallback,
                  qmonLoadNoEdit, NULL);
   XtAddCallback(suspend_thresholds, XmNselectCellCallback,
                  qmonLoadSelectEntry, NULL);
#endif
   XtAddCallback(suspend_thresholds, XmNlabelActivateCallback,
                  qmonLoadNames, NULL);
   XtAddCallback(suspend_intervalPB, XmNactivateCallback,
                  qmonQCTime, (XtPointer) suspend_interval); 

   /*
   ** Checkpointing
   */
   XtAddCallback(min_cpu_intervalPB, XmNactivateCallback,
                  qmonQCTime, (XtPointer) min_cpu_interval); 
   XtAddCallback(ckpt_add, XmNactivateCallback, 
                     qmonQCCkptAdd, NULL);
   XtAddCallback(ckpt_remove, XmNactivateCallback, 
                     qmonQCCkptRemove, NULL);
   XtAddCallback(ckpt_dialog, XmNactivateCallback, 
                     qmonPopupCkptConfig, NULL);

   /*
   ** PE
   */
   XtAddCallback(pe_add, XmNactivateCallback, 
                     qmonQCPEAdd, NULL);
   XtAddCallback(pe_remove, XmNactivateCallback, 
                     qmonQCPERemove, NULL);
   XtAddCallback(pe_dialog, XmNactivateCallback, 
                     qmonPopupPEConfig, NULL);

   /*
   ** Complexes & Consumables
   */
#if 0
   XtAddCallback(consumable_delete, XmNactivateCallback,
                  qmonLoadDelLines, (XtPointer) complexes_ccl); 
   XtAddCallback(complexes_ccl, XmNenterCellCallback,
                  qmonLoadNoEdit, NULL);
   XtAddCallback(complexes_ccl, XmNselectCellCallback,
                  qmonLoadSelectEntry, NULL);
#endif
   XtAddCallback(complexes_ccl, XmNlabelActivateCallback,
                  qmonLoadNames, NULL);

   /*
   ** Limits
   */
   XtAddCallback(h_rtPB, XmNactivateCallback, 
                     qmonQCTime, (Widget)h_rt);
   XtAddCallback(h_cpuPB, XmNactivateCallback, 
                     qmonQCTime, (Widget)h_cpu);
   XtAddCallback(h_corePB, XmNactivateCallback, 
                     qmonQCMem, (Widget)h_core);
   XtAddCallback(h_dataPB, XmNactivateCallback, 
                     qmonQCMem, (Widget)h_data);
   XtAddCallback(h_fsizePB, XmNactivateCallback, 
                     qmonQCMem, (Widget)h_fsize);
   XtAddCallback(h_rssPB, XmNactivateCallback, 
                     qmonQCMem, (Widget)h_rss);
   XtAddCallback(h_stackPB, XmNactivateCallback, 
                     qmonQCMem, (Widget)h_stack);
   XtAddCallback(h_vmemPB, XmNactivateCallback, 
                     qmonQCMem, (Widget)h_vmem);

   XtAddCallback(s_rtPB, XmNactivateCallback, 
                     qmonQCTime, (Widget)s_rt);
   XtAddCallback(s_cpuPB, XmNactivateCallback, 
                     qmonQCTime, (Widget)s_cpu);
   XtAddCallback(s_corePB, XmNactivateCallback, 
                     qmonQCMem, (Widget)s_core);
   XtAddCallback(s_dataPB, XmNactivateCallback, 
                     qmonQCMem, (Widget)s_data);
   XtAddCallback(s_fsizePB, XmNactivateCallback, 
                     qmonQCMem, (Widget)s_fsize);
   XtAddCallback(s_rssPB, XmNactivateCallback, 
                     qmonQCMem, (Widget)s_rss);
   XtAddCallback(s_stackPB, XmNactivateCallback, 
                     qmonQCMem, (Widget)s_stack);
   XtAddCallback(s_vmemPB, XmNactivateCallback, 
                     qmonQCMem, (Widget)s_vmem);

   /*
   ** Subordinates
   */
   XtAddCallback(subordinates_attached, XmNdefaultActionCallback, 
                     qmonQCSOQ, NULL);

   /*
   ** Access & Xacess
   */
   XtAddCallback(access_toggle, XmtNvalueChangedCallback, 
                     qmonQCAccessToggle, NULL);
   XtAddCallback(access_add, XmNactivateCallback, 
                     qmonQCAccessAdd, NULL);
   XtAddCallback(access_remove, XmNactivateCallback, 
                     qmonQCAccessRemove, NULL);
   XtAddCallback(access_dialog, XmNactivateCallback, 
                     qmonPopupManopConfig, (XtPointer)2);

   /*
   ** Project & Xproject
   */
   XtAddCallback(project_toggle, XmtNvalueChangedCallback, 
                     qmonQCProjectToggle, NULL);
   XtAddCallback(project_add, XmNactivateCallback, 
                     qmonQCProjectAdd, NULL);
   XtAddCallback(project_remove, XmNactivateCallback, 
                     qmonQCProjectRemove, NULL);
   XtAddCallback(project_dialog, XmNactivateCallback, 
                     qmonPopupProjectConfig, NULL);

   /*
   ** Owner
   */
#if 0
   XtAddCallback(owner_add, XmNactivateCallback, 
                     qmonQCOwnerAdd, NULL);
   XtAddCallback(owner_remove, XmNactivateCallback, 
                     DeleteItems, (XtPointer) owner_list);
#endif
   XtAddCallback(owner_new, XmNinputCallback, 
                     qmonQCOwnerAdd, NULL);
                              
   
   /*
   ** register callback procedures
   */
   XmtVaRegisterCallbackProcedures(
         "ToggleSensitive", qmonQCToggleTW, XtRWidget,
         NULL);



#if 0
   /* 
   ** start the needed timers and the corresponding update routines 
   */
   XtAddCallback(XtParent(dialog), XmNpopupCallback, 
                     qmonQCStartUpdate, NULL);
   XtAddCallback(XtParent(dialog), XmNpopdownCallback,
                     qmonQCStopUpdate, NULL);
                     
#endif

   XtAddEventHandler(XtParent(dialog), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
   XtAddEventHandler(XtParent(dialog), StructureNotifyMask, False, 
                        SetMaxShellSize, NULL);

   DEXIT;
   return dialog;
}


/*-------------------------------------------------------------------------*/
static void qmonQCUpdate(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonQCUpdate");
   
   qmonMirrorMultiAnswer(USERSET_T | PROJECT_T | PE_T | CKPT_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   updateQCA();
   updateQCP();
   updateQCPE();
   updateQCCkpt();

   DEXIT;
}



/*-------------------------------------------------------------------------*/
static void qmonQCToggleAction(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonQCToggleAction");

   /*
   ** set the cq_apply button label and the corresponding callback
   */
   switch (dialog_mode) {
      case QC_ADD:
      case QC_CLONE:
         /* manage the layout */
/*          if (!XtIsManaged(cq_layout)) */
/*             XtManageChild(cq_layout); */
         XtSetSensitive(cq_name, True);
         XtVaSetValues( cq_name,
                        XmNeditable, True,
                        NULL);
         break;
      case QC_MODIFY:
         /* manage the layout */
/*          if (!XtIsManaged(cq_layout)) */
/*             XtManageChild(cq_layout); */
         XtSetSensitive(cq_name, False);
         XtVaSetValues( cq_name,
                        XmNeditable, False,
                        NULL);
         break;
      case QC_DELETE:
         /* manage the layout */
/*          if (XtIsManaged(cq_layout)) */
/*             XtUnmanageChild(cq_layout); */
         XtSetSensitive(cq_name, False);
         break;
   }

   DEXIT;
}   

#if 0
/*-------------------------------------------------------------------------*/
static void qmonQCResetAll(Widget w, XtPointer cld, XtPointer cad)
{
   XmString xmhost = NULL;

   DENTER(GUI_LAYER, "qmonQCResetAll");
   
   /*
   ** fill all pages with template values, leave qname and hostname 
   ** unchanged
   */
   qmonQCSetData(&current_entry, "template", HOSTREF_DEFAULT);
   XmtDialogSetDialogValues(cq_dialog, &current_entry);

   xmhost = XmStringCreateLocalized((char *) HOSTREF_DEFAULT);
   XmListSelectItem(cq_href_select_list_w, xmhost, True);
   XmStringFree(xmhost);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQCClone(Widget w, XtPointer cld, XtPointer cad)
{
   Boolean status = False;
   lList *ql = NULL;
   lListElem *qep = NULL;
   int n, i;
   StringConst *strs = NULL;
   static char buf[BUFSIZ];
   lList *alp = NULL;
   XmString xmhost = NULL;
   
   DENTER(GUI_LAYER, "qmonQCClone");
   
   qmonMirrorMultiAnswer(CQUEUE_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }

   ql = qmonMirrorList(SGE_CQ_LIST);
   n = lGetNumberOfElem(ql);
   if (n>0) {
      strs = (StringConst*)XtMalloc(sizeof(String)*n); 
      for (qep=lFirst(ql), i=0; i<n; qep=lNext(qep), i++) {
        /*
        ** we get only references don't free, the strings
        */
        strs[i] = lGetString(qep, CQ_name);
      }
    
      strcpy(buf, "");
      /* FIX_CONST_GUI */
      status = XmtAskForItem(w, NULL, "@{Get Queue Attributes from}",
                        "@{Available Queues}", (String*)strs, n,
                        False, buf, BUFSIZ, NULL); 
      
      if (status) {
         qmonQCSetData(&current_entry, buf, HOSTREF_DEFAULT);
         XmtDialogSetDialogValues(cq_dialog, &current_entry);

         xmhost = XmStringCreateLocalized((char *) HOSTREF_DEFAULT);
         XmListSelectItem(cq_href_select_list_w, xmhost, True);
         XmStringFree(xmhost);
      }
      /*
      ** don't free referenced strings, they are in the queue list
      */
      XtFree((char*)strs);
   }
   else
      qmonMessageShow(w, True, "@{No queues to clone}");
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQCCheckHost(Widget w, XtPointer cld, XtPointer cad)
{
   XmtInputFieldCallbackStruct *cbs = (XmtInputFieldCallbackStruct*)cad;
   static char unique[CL_MAXHOSTLEN];
   int ret;
   String name = NULL;
   
   DENTER(GUI_LAYER, "qmonQCCheckHost");

   name = XmtInputFieldGetString(cq_name);

   if (cbs->input && cbs->input[0]!='\0' && name && strcmp(name, "template")) {
      ret = sge_resolve_hostname((char*)cbs->input, unique, EH_name, sizeof(unique));

      switch ( ret ) {
         case CL_RETVAL_GETHOSTNAME_ERROR:
/*             cbs->okay = False; */
/*             qmonMessageShow(w, True, "Can't resolve host '%s'\n",  */
/*                               cbs->input);   */
            break;
         case CL_RETVAL_OK:
            cbs->input = unique;
         default:
            DPRINTF(("sge_resolve_hostname() failed resolving: %s\n", cl_get_error_text(ret)));
      }
   }

   DEXIT;
}
#endif
/*-------------------------------------------------------------------------*/
static void qmonQCCheckName(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonQCCheckName");

   XtSetSensitive(cq_apply, True);
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonQCAdd(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;
   XmString xmhost = NULL;
   lListElem *copy = NULL;
   u_long32 gdi_command = 0;
   
   DENTER(TOP_LAYER, "qmonQCAdd");
   
   xmhost = XmStringCreateLocalized((char *) HOSTREF_DEFAULT);
   XmListSelectItem(cq_href_select_list_w, xmhost, True);
   XmStringFree(xmhost);

   if (qmon_debug) {
      lWriteElemTo(current_qep, stdout);
   }

   copy = lCopyElem(current_qep);

   if (dialog_mode == QC_ADD || dialog_mode == QC_CLONE) {
      gdi_command = SGE_GDI_ADD | SGE_GDI_SET_ALL;
   } else {
      gdi_command = SGE_GDI_MOD | SGE_GDI_SET_ALL;
   }   
   cqueue_add_del_mod_via_gdi(ctx, copy, &alp, gdi_command); 
   qmonMessageBox(w, alp, 0);
   
   if ( lFirst(alp) && lGetUlong(lFirst(alp), AN_status) == STATUS_OK ) {
      qmonCQUpdate(w, NULL, NULL);
      dont_close = 0;
      /*
      ** reset dialog 
      */
      if (dialog_mode == QC_ADD || dialog_mode == QC_CLONE)
         XtSetSensitive(cq_apply, False);
   } else {
      dont_close = 1;
   }   
   lFreeList(&alp);
   lFreeElem(&copy);

   if (!dont_close)
      qmonQCPopdown(w, NULL, NULL);

   DEXIT;
}
      
/*-------------------------------------------------------------------------*/
static void qmonQCSetData(
tQCEntry *data,
StringConst qname,
StringConst href
) {
   lList *ql;
   lList *alp = NULL;
   lList *href_list = NULL; 
   
   DENTER(GUI_LAYER, "qmonQCSetData");

   if (!data) {
      DEXIT;
      return;
   }
   
   qmonMirrorMulti(CQUEUE_T);
   ql = qmonMirrorList(SGE_CQ_LIST);

   if (current_qep) {
      lFreeElem(&current_qep);
   }   
   if (!strcmp(qname, "template")) {
         current_qep = cqueue_create(&alp, "template" );
         cqueue_set_template_attributes(current_qep, &alp);
   } else {
      current_qep = lCopyElem(cqueue_list_locate(ql, qname));
   }  
   cqueue_find_used_href(current_qep, &alp, &href_list);   
   UpdateXmListFromCull(cq_href_select_list_w, XmFONTLIST_DEFAULT_TAG, 
                           href_list, HR_name);

   if (qmon_debug) {
      lWriteElemTo(current_qep, stdout);   
   }   

   qmonCullToCQ(current_qep, data, href);

   lFreeList(&href_list);
   lFreeList(&alp);

   DEXIT;
}   
   
#if 0
/*-------------------------------------------------------------------------*/
static Boolean check_qname(
char *name 
) {
   DENTER(GUI_LAYER, "check_qname");

   if (strchr(name, '/')) {
      DEXIT;
      return False;
   }

   DEXIT;
   return True;
}
#endif

/*-------------------------------------------------------------------------*/
static void qmonInitQCEntry(
tQCEntry *data 
) {
   DENTER(GUI_LAYER, "qmonInitQCEntry");

   memset((void*)data, 0, sizeof(tQCEntry));

   DEXIT;

}


/*-------------------------------------------------------------------------*/
/* C H E C K P O I N T    P A G E                                          */
/*-------------------------------------------------------------------------*/
static void qmonQCCkptAdd(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount, i;
   String text;
   
   DENTER(GUI_LAYER, "qmonQCCkptAdd");

   XtVaGetValues( ckpt_list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount) {
      XmtLayoutDisableLayout(cq_dialog);

      for (i=0; i<selectedItemCount; i++) {
         if (!XmStringGetLtoR(selectedItems[i], XmFONTLIST_DEFAULT_TAG, &text))
            continue;
         XmListAddItemUniqueSorted(ckpt_reference_list, text);
         XtFree(text); 
      }
      XmtLayoutEnableLayout(cq_dialog);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQCCkptRemove(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount;
   
   DENTER(GUI_LAYER, "qmonQCCkptRemove");

   XtVaGetValues( ckpt_reference_list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount)
      XmListDeleteItems(ckpt_reference_list, selectedItems, selectedItemCount); 

   DEXIT;
}


/*-------------------------------------------------------------------------*/
/* P E   P A G E                                                           */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static void qmonQCPEAdd(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount, i;
   String text;
   
   DENTER(GUI_LAYER, "qmonQCPEAdd");

   XtVaGetValues( pe_list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount) {
      XmtLayoutDisableLayout(cq_dialog);

      for (i=0; i<selectedItemCount; i++) {
         if (!XmStringGetLtoR(selectedItems[i], XmFONTLIST_DEFAULT_TAG, &text))
            continue;
         XmListAddItemUniqueSorted(pe_reference_list, text);
         XtFree(text); 
      }
      XmtLayoutEnableLayout(cq_dialog);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQCPERemove(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount;
   
   DENTER(GUI_LAYER, "qmonQCPERemove");

   XtVaGetValues( pe_reference_list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount)
      XmListDeleteItems(pe_reference_list, selectedItems, selectedItemCount); 

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQCMem(Widget w, XtPointer cld, XtPointer cad) 
{
   Widget target = (Widget) cld;
   char stringval[BUFSIZ];
   Boolean status = False;
   String value;

   DENTER(GUI_LAYER, "qmonQCLimitInputMem");
   
   value = XmtInputFieldGetString(target);
   sge_strlcpy(stringval, value, BUFSIZ);
   status = XmtAskForMemory(w, NULL, "@{Enter a memory value}",
                  stringval, sizeof(stringval), NULL);
   if (stringval[0] == '\0')
      status = False;

   if (status) {
      XmtInputFieldSetString(target, stringval);
   }
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonQCTime(Widget w, XtPointer cld, XtPointer cad)
{
   Widget input_field = (Widget) cld;
   char stringval[256];
   Boolean status = False;
   String current;
   Boolean mode = True;

   DENTER(GUI_LAYER, "qmonQCTime");

   if (input_field == suspend_interval)
      mode = False;

   current = XmtInputFieldGetString(input_field);
   sge_strlcpy(stringval, current, sizeof(stringval));
   status = XmtAskForTime(w, NULL, "@{Enter time}",
               stringval, sizeof(stringval), NULL, mode);
   if (stringval[0] == '\0')
      status = False;

   if (status)
      XmtInputFieldSetString(input_field, stringval);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQCCalendar(Widget w, XtPointer cld, XtPointer cad)
{
   Widget input_field = (Widget) cld;
   char buf[256];
   Boolean status = False;
   lList *cl = NULL;
   lListElem *cep = NULL;
   StringConst *strs = NULL;
   int n, i;

   DENTER(GUI_LAYER, "qmonQCCalendar");

   qmonMirrorMulti(CALENDAR_T);
   cl = qmonMirrorList(SGE_CAL_LIST);
   n = lGetNumberOfElem(cl);
   if (n>0) {
      strs = (StringConst*)XtMalloc(sizeof(String)*n); 
      for (cep=lFirst(cl), i=0; i<n; cep=lNext(cep), i++) {
        /*
        ** we get only references don't free, the strings
        */
        strs[i] = (StringConst)lGetString(cep, CAL_name);
      }
    
      strcpy(buf, "");
      /* FIX_CONST_GUI */
      status = XmtAskForItem(w, NULL, "@{Get calendar}",
                        "@{Available Calendars}", (String*)strs, n,
                        False, buf, 256, NULL); 


      if (status) {
         XmtInputFieldSetString(input_field, buf);
      }
      /*
      ** don't free referenced strings, they are in the queue list
      */
      XtFree((char*)strs);
   }
   else
      qmonMessageShow(w, True, "@{No calendars to select}");
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
/* L I M I T    P A G E                                                    */
/*-------------------------------------------------------------------------*/
#if 0
static void qmonQCLimitNoEdit(Widget w, XtPointer cld, XtPointer cad)
{
   XbaeMatrixEnterCellCallbackStruct *cbs =
         (XbaeMatrixEnterCellCallbackStruct*) cad;

   DENTER(GUI_LAYER, "qmonQCLimitNoEdit");

   switch( cbs->column ) {
      case 0:
         cbs->doit = False;
         break;
      default:
         cbs->doit = True;       /* Default behaviour */
         break;
    }
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQCLimitCheck(Widget w, XtPointer cld, XtPointer cad)
{
   XbaeMatrixLeaveCellCallbackStruct *cbs =
         (XbaeMatrixLeaveCellCallbackStruct*) cad;
   u_long32 uval;
   char buf[BUFSIZ];
   static Boolean show_message = True;

   DENTER(GUI_LAYER, "qmonQCLimitCheck");

   if (cbs->column == 1) {
      if (!parse_ulong_val(NULL, &uval, TYPE_TIM, cbs->value, buf, BUFSIZ-1)) {
         if (show_message)
            qmonMessageShow(w, True, "@{qaction.novalidtime.No valid time format: hh:mm:ss or\nINFINITY required !}");
         show_message = False;
         cbs->doit = False;
      }
      else
         show_message = True;
   }
  
   DEXIT;
}


#endif


/*-------------------------------------------------------------------------*/
/* C O M P L E X E S    P A G E                                            */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/* S U B O R D I N A T E S   P A G E                                       */
/*-------------------------------------------------------------------------*/
static void qmonQCSOQ(Widget w, XtPointer cld, XtPointer cad)
{
   XbaeMatrixDefaultActionCallbackStruct *cbs =
         (XbaeMatrixDefaultActionCallbackStruct*) cad;
   Boolean status = False;
   lList *ql = NULL;
   lListElem *qep = NULL;
   int n, i;
   StringConst *strs = NULL;
   static char buf[BUFSIZ];
   
   DENTER(GUI_LAYER, "qmonQCSOQ");
   
   if (cbs->column == 0) {
      qmonMirrorMulti(CQUEUE_T);
      ql = qmonMirrorList(SGE_CQ_LIST);
      n = lGetNumberOfElem(ql);
      if (n>0) {
         strs = (StringConst*)XtMalloc(sizeof(String)*n); 
         for (qep=lFirst(ql), i=0; i<n; qep=lNext(qep), i++) {
           /*
           ** we get only references don't free, the strings
           */
           strs[i] = (StringConst)lGetString(qep, CQ_name);
         }
       
         strcpy(buf, "");
         /* FIX_CONST_GUI */
         status = XmtAskForItem(w, NULL, "@{Get Queue}",
                           "@{Available Queues}", (String*)strs, n,
                           False, buf, BUFSIZ, NULL); 
         
         if (status) {
             XbaeMatrixSetCell(w, cbs->row, cbs->column, buf);
         }
         /*
         ** don't free referenced strings, they are in the queue list
         */
         XtFree((char*)strs);
      }
      else
         qmonMessageShow(w, True, "@{No queues to select}");
   } 
   DEXIT;
}


/*-------------------------------------------------------------------------*/
/* A C C E S S L I S T     P A G E                                         */
/*-------------------------------------------------------------------------*/
static void qmonQCAccessToggle(Widget w, XtPointer cld, XtPointer cad)
{
   XmtChooserCallbackStruct *cbs = (XmtChooserCallbackStruct*) cad;
   
   DENTER(GUI_LAYER, "qmonQCAccessToggle");

   if (cbs->state) {
      XtSetSensitive(access_allow, False);
      XtSetSensitive(access_deny, True);
   }
   else {
      XtSetSensitive(access_allow, True);
      XtSetSensitive(access_deny, False);
   }
  
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQCAccessAdd(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount, i;
   Widget list;
   int state;
   String text;
   
   DENTER(GUI_LAYER, "qmonQCAccessAdd");

   XtVaGetValues( access_list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount) {
      XmtLayoutDisableLayout(cq_dialog);
      state = XmtChooserGetState(access_toggle);

      if (state)
         list = access_deny;
      else
         list = access_allow;
         
      for (i=0; i<selectedItemCount; i++) {
         if (!XmStringGetLtoR(selectedItems[i], XmFONTLIST_DEFAULT_TAG, &text))
            continue;
         XmListAddItemUniqueSorted(list, text);
         XtFree(text); 
      }
      XmtLayoutEnableLayout(cq_dialog);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQCAccessRemove(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount;
   int state;
   Widget list;
   
   DENTER(GUI_LAYER, "qmonQCAccessRemove");

   state = XmtChooserGetState(access_toggle);

   if (state)
      list = access_deny;
   else
      list = access_allow;
         
   XtVaGetValues( list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount)
      XmListDeleteItems(list, selectedItems, selectedItemCount); 

   DEXIT;
}

/*-------------------------------------------------------------------------*/
/* P R O J E C T     P A G E                                               */
/*-------------------------------------------------------------------------*/
static void qmonQCProjectToggle(Widget w, XtPointer cld, XtPointer cad)
{
   XmtChooserCallbackStruct *cbs = (XmtChooserCallbackStruct*) cad;
   
   DENTER(GUI_LAYER, "qmonQCProjectToggle");

   if (cbs->state) {
      XtSetSensitive(project_allow, False);
      XtSetSensitive(project_deny, True);
   }
   else {
      XtSetSensitive(project_allow, True);
      XtSetSensitive(project_deny, False);
   }
  
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQCProjectAdd(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount, i;
   Widget list;
   int state;
   String text;
   
   DENTER(GUI_LAYER, "qmonQCProjectAdd");

   XtVaGetValues( project_list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount) {
      XmtLayoutDisableLayout(cq_dialog);
      state = XmtChooserGetState(project_toggle);

      if (state)
         list = project_deny;
      else
         list = project_allow;
         
      for (i=0; i<selectedItemCount; i++) {
         if (!XmStringGetLtoR(selectedItems[i], XmFONTLIST_DEFAULT_TAG, &text))
            continue;
         XmListAddItemUniqueSorted(list, text);
         XtFree(text); 
      }
      XmtLayoutEnableLayout(cq_dialog);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQCProjectRemove(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount;
   int state;
   Widget list;
   
   DENTER(GUI_LAYER, "qmonQCProjectRemove");

   state = XmtChooserGetState(project_toggle);

   if (state)
      list = project_deny;
   else
      list = project_allow;
         
   XtVaGetValues( list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount)
      XmListDeleteItems(list, selectedItems, selectedItemCount); 

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQCOwnerAdd(Widget w, XtPointer cld, XtPointer cad)
{
   XmString xnew = NULL;
   String new = NULL;

   DENTER(GUI_LAYER, "qmonQCOwnerAdd");
   
   new = XmtInputFieldGetString(owner_new);

   if (new && new[0] != '\0') {
      xnew = XmtCreateXmString(new);
      XmListAddItem(owner_list, xnew, 0); 
      XmStringFree(xnew);
      XmtInputFieldSetString(owner_new, "");
   }

   DEXIT;
}




/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

#if 0

/*-------------------------------------------------------------------------*/
void updateQCQ(void)
{
   lList *qlf;
   static lCondition *where = NULL;
   
   DENTER(GUI_LAYER, "updateQCQ");

   if (cq_dialog && XtIsManaged(cq_dialog)) {
      /* disable/enable redisplay while updating */
      XmtLayoutDisableLayout(cq_dialog);
      /* list with template sorted alphabetically */
      qlf = lCopyList("qlf", qmonMirrorList(SGE_CQ_LIST));
      lPSortList(qlf, "%I+", QU_qname);
      UpdateXmListFromCull(cq_queue_list, XmFONTLIST_DEFAULT_TAG, qlf, 
                              QU_qname);
                              
      if (!where) {
         where = lWhere("%T(%I != %s)", QU_Type, QU_qname, "template");
      }
      qlf = lSelectDestroy(qlf, where);
      UpdateXmListFromCull(cq_queue_set, XmFONTLIST_DEFAULT_TAG, qlf, QU_qname);
      XmtLayoutEnableLayout(cq_dialog);

      lFreeList(&qlf);
   }
   
   DEXIT;
}

#endif

/*-------------------------------------------------------------------------*/
static void updateQCA(void)
{
   lList *al;
   
   DENTER(GUI_LAYER, "updateQCA");
   
   /* What about sorting */
   al = qmonMirrorList(SGE_US_LIST);
   lPSortList(al, "%I+", US_name);
   
   /* disable/enable redisplay while updating */
   XmtLayoutDisableLayout(cq_dialog);
   UpdateXmListFromCull(access_list, XmFONTLIST_DEFAULT_TAG, al, US_name);
   XmtLayoutEnableLayout(cq_dialog);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void updateQCP(void)
{
   lList *pl;
   
   DENTER(GUI_LAYER, "updateQCP");
   
   /* What about sorting */
   pl = qmonMirrorList(SGE_PR_LIST);
   
   /* disable/enable redisplay while updating */
   XmtLayoutDisableLayout(cq_dialog);
   lPSortList(pl, "%I+", PR_name);
   UpdateXmListFromCull(project_list, XmFONTLIST_DEFAULT_TAG, pl, PR_name);
   XmtLayoutEnableLayout(cq_dialog);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void updateQCPE(void)
{
   lList *pl;
   
   DENTER(GUI_LAYER, "updateQCPE");
   
   /* What about sorting */
   pl = qmonMirrorList(SGE_PE_LIST);
   
   /* disable/enable redisplay while updating */
   XmtLayoutDisableLayout(cq_dialog);
   lPSortList(pl, "%I+", PE_name);
   UpdateXmListFromCull(pe_list, XmFONTLIST_DEFAULT_TAG, pl, PE_name);
   XmtLayoutEnableLayout(cq_dialog);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void updateQCCkpt(void)
{
   lList *pl;
   
   DENTER(GUI_LAYER, "updateQCCkpt");
   
   /* What about sorting */
   pl = qmonMirrorList(SGE_CK_LIST);
   
   /* disable/enable redisplay while updating */
   XmtLayoutDisableLayout(cq_dialog);
   lPSortList(pl, "%I+", CK_name);
   UpdateXmListFromCull(ckpt_list, XmFONTLIST_DEFAULT_TAG, pl, CK_name);
   XmtLayoutEnableLayout(cq_dialog);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
static Boolean qmonCullToCQ(
lListElem *qep,
tQCEntry *data,
const char *href
) {
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonCullToCQ");
   
   if (!qep || !data) 
      goto error;

DTRACE;
   
   data->cq_name = (char *)lGetString(qep, CQ_name);
   data->cq_hostlist = lGetList(qep, CQ_hostlist);
   
   ulng_attr_list_find_value_href(lGetList(qep, CQ_seq_no), &alp, href, &(data->seq_no), &(data->seq_no_tw));
   ulng_attr_list_find_value_href(lGetList(qep, CQ_nsuspend), &alp, href, &(data->nsuspend), &(data->nsuspend_tw));
   ulng_attr_list_find_value_href(lGetList(qep, CQ_job_slots), &alp, href, &(data->job_slots), &(data->job_slots_tw));

   qtlist_attr_list_find_value_href(lGetList(qep, CQ_qtype), &alp, href, &(data->qtype), &(data->qtype_tw));

   {
      /*
      ** this is important here otherwise the macro breaks
      */
/*       lBool rerun; */
      bool rerun;
      bool_attr_list_find_value_href(lGetList(qep, CQ_rerun), &alp, href, 
                                       &rerun, &(data->rerun_tw));
      if (rerun)
         data->rerun = 1;
      else
         data->rerun = 0;
   }

   str_attr_list_find_value_href(lGetList(qep, CQ_tmpdir), &alp, href, &(data->tmpdir), &(data->tmpdir_tw));
   str_attr_list_find_value_href(lGetList(qep, CQ_shell), &alp, href, &(data->shell), &(data->shell_tw));
   str_attr_list_find_value_href(lGetList(qep, CQ_calendar), &alp, href, &(data->calendar), &(data->calendar_tw));
   {
      const char *priority = NULL;
      str_attr_list_find_value_href(lGetList(qep, CQ_priority), &alp, href, &priority, &(data->priority_tw));
      if (priority) {
         data->priority = atoi(priority);
      } else {
         data->priority = 0;
      } 
   }
   str_attr_list_find_value_href(lGetList(qep, CQ_processors), &alp, href, &(data->processors), &(data->processors_tw));
   str_attr_list_find_value_href(lGetList(qep, CQ_prolog), &alp, href, &(data->prolog), &(data->prolog_tw));
   str_attr_list_find_value_href(lGetList(qep, CQ_epilog), &alp, href, &(data->epilog), &(data->epilog_tw));
   {
      const char *shell_start_mode = NULL;
      str_attr_list_find_value_href(lGetList(qep, CQ_shell_start_mode), &alp, href, &shell_start_mode, 
                                       &(data->shell_start_mode_tw));
      if (shell_start_mode && !strcasecmp(shell_start_mode, "unix_behavior"))
         data->shell_start_mode = 3; 
      else if (shell_start_mode && !strcasecmp(shell_start_mode, "script_from_stdin"))
         data->shell_start_mode = 2; 
      else if (shell_start_mode && !strcasecmp(shell_start_mode, "posix_compliant"))
         data->shell_start_mode = 1; 
      else 
         data->shell_start_mode = 0; 
   }
   str_attr_list_find_value_href(lGetList(qep, CQ_starter_method), &alp, href, 
                                 &(data->starter_method), &(data->starter_method_tw));
   str_attr_list_find_value_href(lGetList(qep, CQ_suspend_method), &alp, href, 
                                 &(data->suspend_method), &(data->suspend_method_tw));
   str_attr_list_find_value_href(lGetList(qep, CQ_resume_method), &alp, href, 
                                 &(data->resume_method), &(data->resume_method_tw));
   str_attr_list_find_value_href(lGetList(qep, CQ_terminate_method), &alp, href, 
                                 &(data->terminate_method), &(data->terminate_method_tw));
   {
      const char *initial_state = NULL;
      str_attr_list_find_value_href(lGetList(qep, CQ_initial_state), &alp, href, 
                                    &initial_state, &(data->initial_state_tw));
      if (initial_state && !strcasecmp(initial_state, "disabled"))
         data->initial_state = 2; 
      else if (initial_state && !strcasecmp(initial_state, "enabled"))
         data->initial_state = 1; 
      else 
         data->initial_state = 0; 
   }      
   strlist_attr_list_find_value_href(lGetList(qep, CQ_pe_list), &alp, href, &(data->pe_list), &(data->pe_list_tw));
   strlist_attr_list_find_value_href(lGetList(qep, CQ_ckpt_list), &alp, href, &(data->ckpt_list), &(data->ckpt_list_tw));

   usrlist_attr_list_find_value_href(lGetList(qep, CQ_owner_list), &alp, href, 
                                     &(data->owner_list), &(data->owner_list_tw));
   usrlist_attr_list_find_value_href(lGetList(qep, CQ_acl), &alp, href, &(data->acl), &(data->acl_tw));
   usrlist_attr_list_find_value_href(lGetList(qep, CQ_xacl), &alp, href, &(data->xacl), &(data->xacl_tw));

   prjlist_attr_list_find_value_href(lGetList(qep, CQ_projects), &alp, href, &(data->prj), &(data->prj_tw));
   prjlist_attr_list_find_value_href(lGetList(qep, CQ_xprojects), &alp, href, &(data->xprj), &(data->xprj_tw));
   
   celist_attr_list_find_value_href(lGetList(qep, CQ_load_thresholds), &alp, href, 
                                    &(data->load_thresholds), &(data->load_thresholds_tw));
   celist_attr_list_find_value_href(lGetList(qep, CQ_suspend_thresholds), &alp, href, 
                                    &(data->suspend_thresholds), &(data->suspend_thresholds_tw));
   celist_attr_list_find_value_href(lGetList(qep, CQ_consumable_config_list), &alp, href, 
                                    &(data->consumable_config_list), &(data->consumable_config_list_tw));

   solist_attr_list_find_value_href(lGetList(qep, CQ_subordinate_list), &alp, href, 
                                    &(data->subordinate_list), &(data->subordinate_list_tw));

   mem_attr_list_find_value_href(lGetList(qep, CQ_s_fsize), &alp, href, &(data->s_fsize), &(data->s_fsize_tw));
   mem_attr_list_find_value_href(lGetList(qep, CQ_s_data), &alp, href, &(data->s_data), &(data->s_data_tw));
   mem_attr_list_find_value_href(lGetList(qep, CQ_s_stack), &alp, href, &(data->s_stack), &(data->s_stack_tw));
   mem_attr_list_find_value_href(lGetList(qep, CQ_s_core), &alp, href, &(data->s_core), &(data->s_core_tw));
   mem_attr_list_find_value_href(lGetList(qep, CQ_s_rss), &alp, href, &(data->s_rss), &(data->s_rss_tw));
   mem_attr_list_find_value_href(lGetList(qep, CQ_s_vmem), &alp, href, &(data->s_vmem), &(data->s_vmem_tw));
   time_attr_list_find_value_href(lGetList(qep, CQ_s_rt), &alp, href, &(data->s_rt), &(data->s_rt_tw));
   time_attr_list_find_value_href(lGetList(qep, CQ_s_cpu), &alp, href, &(data->s_cpu), &(data->s_cpu_tw));

   mem_attr_list_find_value_href(lGetList(qep, CQ_h_fsize), &alp, href, &(data->h_fsize), &(data->h_fsize_tw));
   mem_attr_list_find_value_href(lGetList(qep, CQ_h_data), &alp, href, &(data->h_data), &(data->h_data_tw));
   mem_attr_list_find_value_href(lGetList(qep, CQ_h_stack), &alp, href, &(data->h_stack), &(data->h_stack_tw));
   mem_attr_list_find_value_href(lGetList(qep, CQ_h_core), &alp, href, &(data->h_core), &(data->h_core_tw));
   mem_attr_list_find_value_href(lGetList(qep, CQ_h_rss), &alp, href, &(data->h_rss), &(data->h_rss_tw));
   mem_attr_list_find_value_href(lGetList(qep, CQ_h_vmem), &alp, href, &(data->h_vmem), &(data->h_vmem_tw));
   time_attr_list_find_value_href(lGetList(qep, CQ_h_rt), &alp, href, &(data->h_rt), &(data->h_rt_tw));
   time_attr_list_find_value_href(lGetList(qep, CQ_h_cpu), &alp, href, &(data->h_cpu), &(data->h_cpu_tw));

   inter_attr_list_find_value_href(lGetList(qep, CQ_suspend_interval), &alp, href, &(data->suspend_interval), 
                                       &(data->suspend_interval_tw));
   inter_attr_list_find_value_href(lGetList(qep, CQ_min_cpu_interval), &alp, href, &(data->min_cpu_interval), 
                                       &(data->min_cpu_interval_tw));
   inter_attr_list_find_value_href(lGetList(qep, CQ_notify), &alp, href, &(data->notify), &(data->notify_tw));
   

   lFreeList(&alp);
   DRETURN(True);

   error:
      fprintf(stderr, "qmonCullToCQ failure\n");
      DRETURN(False);
}

/*-------------------------------------------------------------------------*/
static Boolean qmonCQToCull(
tQCEntry *data,
lListElem *qep, 
const char *href
) {
   lList *alp = NULL;
   
   DENTER(GUI_LAYER, "qmonCQToCull");

   if (!qep || !data || !data->cq_name)
      goto error;

   lSetString(qep, CQ_name, qmon_trim(data->cq_name));
   lSetList(qep, CQ_hostlist, data->cq_hostlist);

   ulng_attr_list_add_set_del(lGetListRef(qep, CQ_seq_no), &alp, href, &(data->seq_no), !data->seq_no_tw);

   ulng_attr_list_add_set_del(lGetListRef(qep, CQ_nsuspend), &alp, href, &(data->nsuspend), !data->nsuspend_tw);
      
   ulng_attr_list_add_set_del(lGetListRef(qep, CQ_job_slots), &alp, href, &(data->job_slots), !data->job_slots_tw);
   
   qtlist_attr_list_add_set_del(lGetListRef(qep, CQ_qtype), &alp, href, &(data->qtype), !data->qtype_tw);


   {
      /*
      ** this is important here otherwise the macro breaks
      */
/*       lBool rerun = data->rerun ? True : False; */
      bool rerun = data->rerun ? True : False;
      bool_attr_list_add_set_del(lGetListRef(qep, CQ_rerun), &alp, href, 
                                    &rerun, !data->rerun_tw);
   }

   str_attr_list_add_set_del(lGetListRef(qep, CQ_tmpdir), &alp, href, &(data->tmpdir), !data->tmpdir_tw);
   str_attr_list_add_set_del(lGetListRef(qep, CQ_shell), &alp, href, &(data->shell), !data->shell_tw);

   str_attr_list_add_set_del(lGetListRef(qep, CQ_calendar), &alp, href, &(data->calendar), !data->calendar_tw);

   {
      char buf[BUFSIZ];
      const char *priority;
      sprintf(buf, "%d", data->priority);
      priority = strdup(buf); 
      str_attr_list_add_set_del(lGetListRef(qep, CQ_priority), &alp, href, &priority, !data->priority_tw);
      sge_free(&priority);
   }

   str_attr_list_add_set_del(lGetListRef(qep, CQ_processors), &alp, href, &(data->processors), !data->processors_tw);

   str_attr_list_add_set_del(lGetListRef(qep, CQ_prolog), &alp, href, &(data->prolog), !data->prolog_tw);

   str_attr_list_add_set_del(lGetListRef(qep, CQ_epilog), &alp, href, &(data->epilog), !data->epilog_tw);
   
   {
      const char* modes[] = {"NONE", "posix_compliant", "script_from_stdin", "unix_behavior"};
      str_attr_list_add_set_del(lGetListRef(qep, CQ_shell_start_mode), &alp, href, &modes[data->shell_start_mode], !data->shell_start_mode_tw);
   }

   str_attr_list_add_set_del(lGetListRef(qep, CQ_starter_method), &alp, href, &(data->starter_method), !data->starter_method_tw);
   str_attr_list_add_set_del(lGetListRef(qep, CQ_suspend_method), &alp, href, &(data->suspend_method), !data->suspend_method_tw);
   str_attr_list_add_set_del(lGetListRef(qep, CQ_resume_method), &alp, href, &(data->resume_method), !data->resume_method_tw);
   str_attr_list_add_set_del(lGetListRef(qep, CQ_terminate_method), &alp, href, &(data->terminate_method), !data->terminate_method_tw);
   {
      const char *initial_states[] = {"default", "enabled", "disabled"};
      str_attr_list_add_set_del(lGetListRef(qep, CQ_initial_state), &alp, href, &initial_states[data->initial_state], !data->initial_state_tw); 
   }      

   strlist_attr_list_add_set_del(lGetListRef(qep, CQ_pe_list), &alp, href, &(data->pe_list), !data->pe_list_tw);
   strlist_attr_list_add_set_del(lGetListRef(qep, CQ_ckpt_list), &alp, href, &(data->ckpt_list), !data->ckpt_list_tw);
   usrlist_attr_list_add_set_del(lGetListRef(qep, CQ_owner_list), &alp, href, &(data->owner_list), !data->owner_list_tw);
   usrlist_attr_list_add_set_del(lGetListRef(qep, CQ_acl), &alp, href, &(data->acl), !data->acl_tw);
   usrlist_attr_list_add_set_del(lGetListRef(qep, CQ_xacl), &alp, href, &(data->xacl), !data->xacl_tw);

   prjlist_attr_list_add_set_del(lGetListRef(qep, CQ_projects), &alp, href, &(data->prj), !data->prj_tw);
   prjlist_attr_list_add_set_del(lGetListRef(qep, CQ_xprojects), &alp, href, &(data->xprj), !data->xprj_tw);

   celist_attr_list_add_set_del(lGetListRef(qep, CQ_load_thresholds), &alp, href, &(data->load_thresholds), !data->load_thresholds_tw);
   celist_attr_list_add_set_del(lGetListRef(qep, CQ_suspend_thresholds), &alp, href, &(data->suspend_thresholds), !data->suspend_thresholds_tw);
   celist_attr_list_add_set_del(lGetListRef(qep, CQ_consumable_config_list), &alp, href, &(data->consumable_config_list), !data->consumable_config_list_tw);

   solist_attr_list_add_set_del(lGetListRef(qep, CQ_subordinate_list), &alp, href, &(data->subordinate_list), !data->subordinate_list_tw);

   mem_attr_list_add_set_del(lGetListRef(qep, CQ_s_fsize), &alp, href, &(data->s_fsize), !data->s_fsize_tw);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_s_data), &alp, href, &(data->s_data), !data->s_data_tw);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_s_stack), &alp, href, &(data->s_stack), !data->s_stack_tw);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_s_core), &alp, href, &(data->s_core), !data->s_core_tw);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_s_rss), &alp, href, &(data->s_rss), !data->s_rss_tw);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_s_vmem), &alp, href, &(data->s_vmem), !data->s_vmem_tw);
   time_attr_list_add_set_del(lGetListRef(qep, CQ_s_rt), &alp, href, &(data->s_rt), !data->s_rt_tw);
   time_attr_list_add_set_del(lGetListRef(qep, CQ_s_cpu), &alp, href, &(data->s_cpu), !data->s_cpu_tw);

   mem_attr_list_add_set_del(lGetListRef(qep, CQ_h_fsize), &alp, href, &(data->h_fsize), !data->h_fsize_tw);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_h_data), &alp, href, &(data->h_data), !data->h_data_tw);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_h_stack), &alp, href, &(data->h_stack), !data->h_stack_tw);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_h_core), &alp, href, &(data->h_core), !data->h_core_tw);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_h_rss), &alp, href, &(data->h_rss), !data->h_rss_tw);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_h_vmem), &alp, href, &(data->h_vmem), !data->h_vmem_tw);
   time_attr_list_add_set_del(lGetListRef(qep, CQ_h_rt), &alp, href, &(data->h_rt), !data->h_rt_tw);
   time_attr_list_add_set_del(lGetListRef(qep, CQ_h_cpu), &alp, href, &(data->h_cpu), !data->h_cpu_tw);

   inter_attr_list_add_set_del(lGetListRef(qep, CQ_suspend_interval), &alp, href, &(data->suspend_interval), !data->suspend_interval_tw); 
   inter_attr_list_add_set_del(lGetListRef(qep, CQ_min_cpu_interval), &alp, href, &(data->min_cpu_interval), !data->min_cpu_interval_tw); 
   inter_attr_list_add_set_del(lGetListRef(qep, CQ_notify), &alp, href, &(data->notify), !data->notify_tw);



   if (qmon_debug) {
      printf("-----> alp\n");
      lWriteListTo(alp, stdout);
/*       printf("-----> qep\n"); */
/*       lWriteElemTo(qep, stdout); */
   }   
   
   lFreeList(&alp);
   
   
   DEXIT;
   return True;

   error:
      fprintf(stderr, "qmonQCToCull failure\n");
      DEXIT;
      return False;

}



/*-------------------------------------------------------------------------*/
static Boolean qmonCQDeleteHrefAttr(
lListElem *qep, 
const char *href
) {
   lList *alp = NULL;
   
   DENTER(GUI_LAYER, "qmonCQDeleteHrefAttr");

   if (!qep)
      goto error;

   ulng_attr_list_add_set_del(lGetListRef(qep, CQ_seq_no), &alp, href, NULL, True);

   ulng_attr_list_add_set_del(lGetListRef(qep, CQ_nsuspend), &alp, href, NULL, True);
      
   ulng_attr_list_add_set_del(lGetListRef(qep, CQ_job_slots), &alp, href, NULL, True);
   
   qtlist_attr_list_add_set_del(lGetListRef(qep, CQ_qtype), &alp, href, NULL, True);

   bool_attr_list_add_set_del(lGetListRef(qep, CQ_rerun), &alp, href, NULL, True);

   str_attr_list_add_set_del(lGetListRef(qep, CQ_tmpdir), &alp, href, NULL, True);

   str_attr_list_add_set_del(lGetListRef(qep, CQ_shell), &alp, href, NULL, True);

   str_attr_list_add_set_del(lGetListRef(qep, CQ_calendar), &alp, href, NULL, True);

   str_attr_list_add_set_del(lGetListRef(qep, CQ_priority), &alp, href, NULL, True);

   str_attr_list_add_set_del(lGetListRef(qep, CQ_processors), &alp, href, NULL, True);

   str_attr_list_add_set_del(lGetListRef(qep, CQ_prolog), &alp, href, NULL, True);

   str_attr_list_add_set_del(lGetListRef(qep, CQ_epilog), &alp, href, NULL, True);
   
   str_attr_list_add_set_del(lGetListRef(qep, CQ_shell_start_mode), &alp, href, NULL, True);

   str_attr_list_add_set_del(lGetListRef(qep, CQ_starter_method), &alp, href, NULL, True);
   str_attr_list_add_set_del(lGetListRef(qep, CQ_suspend_method), &alp, href, NULL, True);
   str_attr_list_add_set_del(lGetListRef(qep, CQ_resume_method), &alp, href, NULL, True);
   str_attr_list_add_set_del(lGetListRef(qep, CQ_terminate_method), &alp, href, NULL, True);
   str_attr_list_add_set_del(lGetListRef(qep, CQ_initial_state), &alp, href, NULL, True); 

   strlist_attr_list_add_set_del(lGetListRef(qep, CQ_pe_list), &alp, href, NULL, True);
   strlist_attr_list_add_set_del(lGetListRef(qep, CQ_ckpt_list), &alp, href, NULL, True);
   usrlist_attr_list_add_set_del(lGetListRef(qep, CQ_owner_list), &alp, href, NULL, True);
   usrlist_attr_list_add_set_del(lGetListRef(qep, CQ_acl), &alp, href, NULL, True);
   usrlist_attr_list_add_set_del(lGetListRef(qep, CQ_xacl), &alp, href, NULL, True);

   prjlist_attr_list_add_set_del(lGetListRef(qep, CQ_projects), &alp, href, NULL, True);
   prjlist_attr_list_add_set_del(lGetListRef(qep, CQ_xprojects), &alp, href, NULL, True);

   celist_attr_list_add_set_del(lGetListRef(qep, CQ_load_thresholds), &alp, href, NULL, True);
   celist_attr_list_add_set_del(lGetListRef(qep, CQ_suspend_thresholds), &alp, href, NULL, True);
   celist_attr_list_add_set_del(lGetListRef(qep, CQ_consumable_config_list), &alp, href, NULL, True);

   solist_attr_list_add_set_del(lGetListRef(qep, CQ_subordinate_list), &alp, href, NULL, True);

   mem_attr_list_add_set_del(lGetListRef(qep, CQ_s_fsize), &alp, href, NULL, True);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_s_data), &alp, href, NULL, True);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_s_stack), &alp, href, NULL, True);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_s_core), &alp, href, NULL, True);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_s_rss), &alp, href, NULL, True);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_s_vmem), &alp, href, NULL, True);
   time_attr_list_add_set_del(lGetListRef(qep, CQ_s_rt), &alp, href, NULL, True);
   time_attr_list_add_set_del(lGetListRef(qep, CQ_s_cpu), &alp, href, NULL, True);

   mem_attr_list_add_set_del(lGetListRef(qep, CQ_h_fsize), &alp, href, NULL, True);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_h_data), &alp, href, NULL, True);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_h_stack), &alp, href, NULL, True);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_h_core), &alp, href, NULL, True);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_h_rss), &alp, href, NULL, True);
   mem_attr_list_add_set_del(lGetListRef(qep, CQ_h_vmem), &alp, href, NULL, True);
   time_attr_list_add_set_del(lGetListRef(qep, CQ_h_rt), &alp, href, NULL, True);
   time_attr_list_add_set_del(lGetListRef(qep, CQ_h_cpu), &alp, href, NULL, True);

   inter_attr_list_add_set_del(lGetListRef(qep, CQ_suspend_interval), &alp, href, NULL, True); 
   inter_attr_list_add_set_del(lGetListRef(qep, CQ_min_cpu_interval), &alp, href, NULL, True); 
   inter_attr_list_add_set_del(lGetListRef(qep, CQ_notify), &alp, href, NULL, True);

   lFreeList(&alp);
   
   if (qmon_debug) {
      lWriteElemTo(qep, stdout);
   }   
   
   DEXIT;
   return True;

   error:
      fprintf(stderr, "qmonCQDeleteHrefAttr failure\n");
      DEXIT;
      return False;

}

/*-------------------------------------------------------------------------*/
static void qmonCQAddHref(Widget w, XtPointer cld, XtPointer cad)
{
   char *host;
   XmString xmhost;
   
   DENTER(GUI_LAYER, "qmonCQAddHref");

   host = XmtInputFieldGetString(cq_href_input_w);

   if (host && host[0] != '\0') {
      XmListAddItemUniqueSorted(cq_href_select_list_w, host);
      xmhost = XmStringCreateSimple(host);
      XmListSelectItem(cq_href_select_list_w, xmhost, True);
      XmStringFree(xmhost);
   }

   XmtInputFieldSetString(cq_href_input_w, "");
   

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCQDeleteHref(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount;
   Widget list = (Widget) cld;
   char *href = NULL;
   int i;
   
   DENTER(GUI_LAYER, "qmonCQDeleteHref");

   
   /*
   ** delete
   */
   XtVaGetValues( list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);
   
   if (selectedItemCount) {
      for (i=0; i<selectedItemCount; i++) {
         XmString deleteme = XmStringCopy(selectedItems[i]);
         XmString xmhost = XmStringCreateLocalized((char *) HOSTREF_DEFAULT);
         XmListSelectItem(list, xmhost, True);
         XmStringFree(xmhost);
         XmStringGetLtoR(deleteme, XmFONTLIST_DEFAULT_TAG, &href);
         if (!strcmp(href, HOSTREF_DEFAULT)) 
            continue;
         qmonCQDeleteHrefAttr(current_qep, href);
         XmListDeleteItem(list, deleteme);
         XmStringFree(deleteme);
         XtFree((char*)href);
      } 
   }   

   DEXIT;
}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
static void qmonCQAddHost(Widget w, XtPointer cld, XtPointer cad)
{
   char *host;
   
   DENTER(GUI_LAYER, "qmonCQAddHost");

   host = XmtInputFieldGetString(cq_host_input_w);

   if (host && host[0] != '\0') {
      XmListAddItemUniqueSorted(cq_hostlist_w, host);
   }

   XmtInputFieldSetString(cq_host_input_w, "");

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCQHrefSelect(Widget w, XtPointer cld, XtPointer cad)
{
   XmListCallbackStruct *cbs = (XmListCallbackStruct*) cad;
   char *href;
   
   DENTER(GUI_LAYER, "qmonCQHrefSelect");

   if (! XmStringGetLtoR(cbs->item, XmFONTLIST_DEFAULT_TAG, &href)) {
      fprintf(stderr, "XmStringGetLtoR failed\n");
      DEXIT;
      return;
   }
   
   /* FIXME */
   XmtDialogGetDialogValues(cq_dialog, &current_entry);
   qmonCQToCull(&current_entry, current_qep, old_href);

   DPRINTF(("Save href entry for %s\nSwitching to %s\n", old_href, href));

   qmonCullToCQ(current_qep, &current_entry, href);
   XmtDialogSetDialogValues(cq_dialog, &current_entry);
   old_href = href;

#if 1
   if (!strcmp(href, HOSTREF_DEFAULT)) {
      qmonQCSetAllTogglesSensitive(w, (XtPointer) False, NULL);
   } else {
      qmonQCSetAllTogglesSensitive(w, (XtPointer) True, NULL);
   }
#endif   

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQCToggleTW(Widget w, XtPointer cld, XtPointer cad)
{
   Widget target;
   Boolean set = XmToggleButtonGetState(w);

   if (w == seq_no_tw) {
      target = XmtNameToWidget(w, "^seq_no"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^seq_no_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == processors_tw) {
      target = XmtNameToWidget(w, "^processors"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^processors_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == tmpdir_tw) {
      target = XmtNameToWidget(w, "^tmpdir"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^tmpdir_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == shell_tw) {
      target = XmtNameToWidget(w, "^shell"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^shell_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == calendar_tw) {
      target = XmtNameToWidget(w, "^calendar"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^calendar_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^calendarPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == notify_tw) {
      target = XmtNameToWidget(w, "^notify"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^notify_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^notifyPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == priority_tw) {
      target = XmtNameToWidget(w, "^priority"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^priority_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == job_slots_tw) {
      target = XmtNameToWidget(w, "^job_slots"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^job_slots_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == shell_start_mode_tw) {
      target = XmtNameToWidget(w, "^shell_start_mode"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^shell_start_mode_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == initial_state_tw) {
      target = XmtNameToWidget(w, "^initial_state"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^initial_state_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == rerun_tw) {
      target = XmtNameToWidget(w, "^rerun"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^rerun_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == qtype_tw) {
      target = XmtNameToWidget(w, "^qtype"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^qtype_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == prolog_tw) {
      target = XmtNameToWidget(w, "^prolog"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^prolog_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == epilog_tw) {
      target = XmtNameToWidget(w, "^epilog"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^epilog_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == starter_method_tw) {
      target = XmtNameToWidget(w, "^starter_method"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^starter_method_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == suspend_method_tw) {
      target = XmtNameToWidget(w, "^suspend_method"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^suspend_method_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == resume_method_tw) {
      target = XmtNameToWidget(w, "^resume_method"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^resume_method_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == terminate_method_tw) {
      target = XmtNameToWidget(w, "^terminate_method"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^terminate_method_label"); 
      XtSetSensitive(target, set);
   }   

   if (w == ckpt_reference_list_tw) {
      target = XmtNameToWidget(w, "^ckpt_reference_list_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^ckpt_reference_listSW"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^ckpt_listSW"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^ckpt_add"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^ckpt_remove"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^ckpt_dialog"); 
      XtSetSensitive(target, set);
   }   

   if (w == min_cpu_interval_tw) {
      target = XmtNameToWidget(w, "^min_cpu_interval_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^min_cpu_interval"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^min_cpu_intervalPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == pe_reference_list_tw) {
      target = XmtNameToWidget(w, "^pe_reference_list_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^pe_reference_listSW"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^pe_listSW"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^pe_add"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^pe_remove"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^pe_dialog"); 
      XtSetSensitive(target, set);
   }   

   if (w == load_thresholds_tw) {
      target = XmtNameToWidget(w, "^load_thresholds_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^load_thresholds"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^load_thresholds_remove"); 
      XtSetSensitive(target, set);
   }   

   if (w == suspend_thresholds_tw) {
      target = XmtNameToWidget(w, "^suspend_thresholds_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^suspend_thresholds"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^suspend_thresholds_remove"); 
      XtSetSensitive(target, set);
   }   

   if (w == suspend_interval_tw) {
      target = XmtNameToWidget(w, "^suspend_interval_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^suspend_interval"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^suspend_intervalPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == suspend_nsuspend_tw) {
      target = XmtNameToWidget(w, "^suspend_nsuspend_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^suspend_nsuspend"); 
      XtSetSensitive(target, set);
   }   

   if (w == h_rt_tw ) {
      target = XmtNameToWidget(w, "^h_rt_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_rt"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_rtPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == h_cpu_tw ) {
      target = XmtNameToWidget(w, "^h_cpu_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_cpu"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_cpuPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == h_fsize_tw ) {
      target = XmtNameToWidget(w, "^h_fsize_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_fsize"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_fsizePB"); 
      XtSetSensitive(target, set);
   }   

   if (w == h_data_tw ) {
      target = XmtNameToWidget(w, "^h_data_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_data"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_dataPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == h_stack_tw ) {
      target = XmtNameToWidget(w, "^h_stack_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_stack"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_stackPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == h_core_tw ) {
      target = XmtNameToWidget(w, "^h_core_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_core"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_corePB"); 
      XtSetSensitive(target, set);
   }   

   if (w == h_rss_tw ) {
      target = XmtNameToWidget(w, "^h_rss_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_rss"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_rssPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == h_vmem_tw ) {
      target = XmtNameToWidget(w, "^h_vmem_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_vmem"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^h_vmemPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == s_rt_tw ) {
      target = XmtNameToWidget(w, "^s_rt_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_rt"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_rtPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == s_cpu_tw ) {
      target = XmtNameToWidget(w, "^s_cpu_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_cpu"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_cpuPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == s_fsize_tw ) {
      target = XmtNameToWidget(w, "^s_fsize_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_fsize"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_fsizePB"); 
      XtSetSensitive(target, set);
   }   

   if (w == s_data_tw ) {
      target = XmtNameToWidget(w, "^s_data_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_data"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_dataPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == s_stack_tw ) {
      target = XmtNameToWidget(w, "^s_stack_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_stack"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_stackPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == s_core_tw ) {
      target = XmtNameToWidget(w, "^s_core_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_core"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_corePB"); 
      XtSetSensitive(target, set);
   }   

   if (w == s_rss_tw ) {
      target = XmtNameToWidget(w, "^s_rss_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_rss"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_rssPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == s_vmem_tw ) {
      target = XmtNameToWidget(w, "^s_vmem_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_vmem"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^s_vmemPB"); 
      XtSetSensitive(target, set);
   }   

   if (w == consumable_config_list_tw ) {
      target = XmtNameToWidget(w, "^consumable_config_list_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^complexes_ccl"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^complexes_ccl_remove"); 
      XtSetSensitive(target, set);
   }   

   if (w == subordinate_list_tw ) {
      target = XmtNameToWidget(w, "^subordinate_list_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^subordinates_attached"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^subordinates_attached_remove"); 
      XtSetSensitive(target, set);
   }   

   if (w == access_allow_tw ) {
      target = XmtNameToWidget(w, "^access_allow_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^access_allowSW"); 
      XtSetSensitive(target, set);
   }   

   if (w == access_deny_tw ) {
      target = XmtNameToWidget(w, "^access_deny_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^access_denySW"); 
      XtSetSensitive(target, set);
   }   

   if (w == project_allow_tw ) {
      target = XmtNameToWidget(w, "^project_allow_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^project_allowSW"); 
      XtSetSensitive(target, set);
   }   

   if (w == project_deny_tw ) {
      target = XmtNameToWidget(w, "^project_deny_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^project_denySW"); 
      XtSetSensitive(target, set);
   }   

   if (w == owner_list_tw ) {
      target = XmtNameToWidget(w, "^owner_list_label"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^owner_listSW"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^owner_new"); 
      XtSetSensitive(target, set);
      target = XmtNameToWidget(w, "^owner_remove"); 
      XtSetSensitive(target, set);
   }   
}

/*-------------------------------------------------------------------------*/
static void qmonQCSetAllTogglesSensitive(Widget w, XtPointer cld, XtPointer cad)
{
   Widget target;
   Boolean set = (cld == NULL) ? False : True;

   DENTER(GUI_LAYER, "qmonQCSetAllTogglesSensitive");

   target = XmtNameToWidget(w, "*tmpdir_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*shell_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*notify_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*processors_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*calendar_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*seq_no_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*qtype_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*shell_start_mode_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*initial_state_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*priority_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*job_slots_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*rerun_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*prolog_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*epilog_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*starter_method_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*suspend_method_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*resume_method_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*terminate_method_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*min_cpu_interval_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*ckpt_reference_list_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*min_cpu_interval_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*ckpt_reference_list_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*pe_reference_list_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*pe_reference_list_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*load_thresholds_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*suspend_thresholds_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*suspend_nsuspend_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*suspend_interval_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*load_thresholds_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*suspend_thresholds_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*suspend_interval_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*s_fsize_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*s_data_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*s_stack_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*s_core_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*s_rss_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*s_vmem_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*s_rt_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*s_cpu_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*h_fsize_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*h_data_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*h_stack_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*h_core_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*h_rss_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*h_vmem_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*h_rt_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*h_cpu_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*consumable_config_list_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*consumable_config_list_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*subordinate_list_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*subordinate_list_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*access_allow_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*access_deny_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*access_allow_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*access_deny_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*project_allow_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*project_deny_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*project_allow_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*project_deny_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*owner_list_tw");
   XtSetSensitive(target, set);
   target = XmtNameToWidget(w, "*owner_list_tw");
   XtSetSensitive(target, set);

   DEXIT;
}

