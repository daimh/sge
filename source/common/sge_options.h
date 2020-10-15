#ifndef __SGE_OPTIONS_H
#define __SGE_OPTIONS_H
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
/* Portions of this code are Copyright 2011 Univa Inc. */
/*___INFO__MARK_END__*/

#include "uti/sge_prog.h" 

enum {
   NO_OPT = 0,
   a_OPT,
   A_OPT,
   c_OPT,
   cat_OPT,
   cl_OPT,
   cell_OPT,
   C_OPT,
   e_OPT,
   pe_OPT,
   f_OPT,
   h_OPT,

   hard_OPT,
   i_OPT,
   j_OPT,
   js_OPT,
   jsv_OPT,
   l_OPT,
   m_OPT,
   M_OPT,
   N_OPT,
   o_OPT,
   p_OPT,
   q_OPT,

   r_OPT,
   res_OPT,
   s_OPT,
   shell_OPT,
   soft_OPT,
   sync_OPT,
   S_OPT,
   t_OPT,
   u_OPT,
   v_OPT,
   verify_OPT,
   V_OPT,
   JOB_ID_OPR,
   JOB_TASK_OPR,

   SCRIPT_OPR,
   help_OPT,
   cwd_OPT,
   ext_OPT,
   notify_OPT,
   now_OPT,
   b_OPT,
   wd_OPT,

   masterq_OPT,
   d_OPT,
   us_OPT,
   hold_jid_OPT,
   hold_jid_ad_OPT,
   JQ_DEST_OPR,
   ac_OPT,
   ah_OPT,
   am_OPT,
   ao_OPT,
   aq_OPT,

   au_OPT,
   Au_OPT,
   Aq_OPT,
   cq_OPT,
   dc_OPT,
   dh_OPT,
   dm_OPT,
   do_OPT,
   dq_OPT,
   du_OPT,
   ke_OPT,

   mc_OPT,
   mconf_OPT,
   mq_OPT,
   sc_OPT,
   sconf_OPT,
   sh_OPT,
   sm_OPT,

   so_OPT,
   sq_OPT,
   sql_OPT,
   su_OPT,
   sul_OPT,
   jid_OPT,
   gc_OPT,  /* dummy from qconf to qmaster to get complex */
   ae_OPT,
   Ae_OPT,

   as_OPT,
   de_OPT,
   ds_OPT,
   Mc_OPT,
   me_OPT,
   Me_OPT,
   sel_OPT,
   se_OPT,
   ss_OPT,
   km_OPT,

   ks_OPT,
   ap_OPT, /* add pe object */
   mp_OPT, /* mod pe object */
   dp_OPT, /* del pe object */
   sp_OPT, /* show pe object */
   spl_OPT, /* show pe object list */
   sconfl_OPT,  /* show list of local configurations */
   dconf_OPT,   /* delete local configuration */
   starthist_OPT,  /* flush history */

   Mq_OPT,
   aconf_OPT,
   nostart_commd_OPT,
   Aconf_OPT,
   Mconf_OPT,
   clear_OPT,

   AT_OPT,
   Ap_OPT, /* add pe object from file */
   Mp_OPT, /* mod pe object from file */
   tsm_OPT, 
   msconf_OPT,   /* modify SGE scheduler configuration */
   Msconf_OPT,   /* mofify SGE scheduler configuration from file*/
   aus_OPT,     /* SGE add user */
   Aus_OPT,     /* SGE add user from file */
   mus_OPT,     /* SGE modify user (qconf) or mail options (submit) */
   Mus_OPT,     /* SGE modify user from file (qconf) or mail list (submit) */
   dus_OPT,     /* SGE delete user */
   sus_OPT,     /* SGE show user */

   susl_OPT,    /* SGE show user list */
   aprj_OPT,     /* SGE add project */
   Aprj_OPT,     /* SGE add project from file */
   Mprj_OPT,     /* SGE modify project from file */
   mprj_OPT,     /* SGE modify project */
   dprj_OPT,     /* SGE delete project */
   sprj_OPT,     /* SGE show project */
   sprjl_OPT,    /* SGE show project list */
   mstree_OPT,   /* SGE modify sharetree */
   Mstree_OPT,   /* SGE modify sharetree from file*/
   astree_OPT,   /* SGE add sharetree */
   Astree_OPT,   /* SGE add sharetree from file*/
   dstree_OPT,   /* SGE delete sharetree */
   sstree_OPT,   /* SGE show sharetree */
   sst_OPT,      /* SGE show a formated sharetree */

   mu_OPT,       /* edit userset object (not only SGE) */
   Mu_OPT,       /* modify userset from file */
   dl_OPT,       /* SGE deadline initiation */
   P_OPT,        /* SGE Project */
   ot_OPT,       /* SGE override tickets option */

   /* added for checkpointing */
   ackpt_OPT,    /* add ckpt element */
   Ackpt_OPT,    /* add ckpt element from file */
   dckpt_OPT,    /* delete ckpt element */
   mckpt_OPT,    /* modify ckpt element */
   Mckpt_OPT,    /* modify ckpt element from file */
   sckpt_OPT,    /* show ckpt element */
   sckptl_OPT,   /* show all ckpt elements */
   ckptobj_OPT,  /* -ckpt in qsub */

   dul_OPT,      /* "-dul <user_set>," in qconf */
   display_OPT,  /* -display option for qsh */
   sss_OPT,      /* show scheduler state */
   sds_OPT,      /* show deficient configurations */
   ssconf_OPT,   /* show scheduler configuration */

   /* calendar management */
   acal_OPT,     /* add new calendar interactively */
   Acal_OPT,     /* add new calendar from file */
   mcal_OPT,     /* modify calendar interactively */
   Mcal_OPT,     /* modify calendar from file */
   dcal_OPT,     /* remove calendar */
   scal_OPT,     /* show calendar */
   scall_OPT,    /* show calendar list */
   w_OPT,        /* warn mode concerning verification of schedulability */ 

   /* share tree node */
   astnode_OPT,  /* SGE add share tree node */
   dstnode_OPT,  /* SGE delete share tree node */
   mstnode_OPT,  /* SGE modify share tree node */
   sstnode_OPT,  /* SGE show share tree node */
   rsstnode_OPT, /* SGE show share tree node and its children */

   /* verbosity */
   verbose_OPT,  /* verbose option for q(r)sh */
   inherit_OPT,  /* inherit option for qrsh, inherit existing job $JOB_ID */
   nostdin_OPT,  /* nostdin option for qrsh, pass as -n option to rsh */
   noshell_OPT,  /* noshell option for qrsh, pass as noshell option to qrsh_starter */
   pty_OPT,      /* pty option for qrsh, start job in a pty */

   /* add/set/delete/modify sge objects */
   mattr_OPT,     /* modify a sublist of an object */
   rattr_OPT,     /* overwrite a sublist */
   dattr_OPT,     /* delete some elements of a sublist */
   aattr_OPT,     /* add a element to a sublist */
   Mattr_OPT,     /* modifiy a sublist from file */
   Rattr_OPT,     /* overwrite a sublist from file */
   Dattr_OPT,     /* aelete a sublist from file */
   Aattr_OPT,     /* add a element to a sublist from file */
   sobjl_OPT,     /* show object list which matches conf value */
   purge_OPT,     /* delete element which value matches given string */
      
#ifndef __SGE_NO_USERMAPPING__
   /* added for user mapping */
   aumap_OPT,    /* add new user mapping entry */
   Aumap_OPT,    /* add new user mapping entry from file */
   dumap_OPT,    /* delete user mapping entry  */
   mumap_OPT,    /* modify user mapping entry  */
   sumap_OPT,    /* show user mapping entry    */
   sumapl_OPT,   /* show user mapping entry list */
   Mumap_OPT,    /* modify user mapping entry from file */ 
#endif

   /* added for host groups */
   ahgrp_OPT,    /* add new host group entry */
   Ahgrp_OPT,    /* add new host group entry from file */
   dhgrp_OPT,    /* delete host group entry  */
   mhgrp_OPT,    /* modify host group entry */
   shgrp_OPT,    /* show host group entry */
   shgrp_tree_OPT,        /* show host group entry as tree*/
   shgrp_resolved_OPT,    /* show host group entry with resolved hostlist */
   shgrpl_OPT,   /* show host group entry list  */
   Mhgrp_OPT,    /* modify host group entry from file */

   /* added for event clients */
   secl_OPT,     /* show event client list */
   kec_OPT,      /* kill event client */

   cu_OPT,       /* sharetree - clear all user/project usage */
   R_OPT,        /* sharetree - clear all user/project usage */

   /* added for resource quota sets */
   srqs_OPT,     /* show resource quota set */
   srqsl_OPT,    /* show resource quota set list */
   arqs_OPT,     /* add resource quota set */
   Arqs_OPT,     /* add resource quota set from file */
   mrqs_OPT,     /* modfiy resource quota set */
   Mrqs_OPT,     /* modify resource quota set from file */
   drqs_OPT,     /* delete resource quota set */
   ar_OPT,       /* advanced resservation option */
   he_OPT,       /* error handling for qrsub */ 
   explain_OPT,  /* explain error in qrstat */
   xml_OPT,      /* generate xml outout */
   terse_OPT,    /* tersed output */
   at_OPT,       /* add/start thread */
   kt_OPT,       /* kill/terminate thread */

   tc_OPT,        /* task concurrency */

   /* added for job to core binding */
   binding_OPT,   /* requests job binding strategy */
   suspend_remote_OPT   /* parameter for qrsh to toggle the suspend behavior */
};

/* macros used in parsing */
#define VALID_OPT(opt,who) (sge_options[opt][who])

extern unsigned short sge_options[][ALL_OPT + 1]; 

#endif /* __SGE_OPTIONS_H */
