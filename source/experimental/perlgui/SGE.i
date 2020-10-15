%module SGE

%{

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
 *  Source License Version 1.2 (the \"License\"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an \"AS IS\" basis,
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


#include "sgermon.h"
#include "cull.h"
#include "gdi_qmod.h"
#include "basis_types.h"
#include "sge_all_listsL.h"
#include "sge_gdi.h"
#include "sge_gdi_attributes.h"
#include "sge_gdi_intern.h"
#include "sge_boundaries.h"
#include "sge_complex_schedd.h"
#include "sge_histdirL.h"
#include "sge_hostL.h"
#include "sge_idL.h"
#include "sge_jataskL.h"
#include "sge_jobL.h"
#include "sge_job_queueL.h"
#include "sge_job_refL.h"
#include "sge_job_reportL.h"
#include "sge_job_schedd.h"
#include "sge_krbL.h"
#include "sge_load_reportL.h"
#include "sge_manopL.h"
#include "sge_messageL.h"
#include "sge_multiL.h"
#include "sge_orderL.h"
#include "sge_orders.h"
#include "sge_paL.h"
#include "sge_peL.h"
#include "sge_pe_schedd.h"
#include "sge_permissionL.h"
#include "sge_ptfL.h"
#include "sge_qexec.h"
#include "sge_qexecL.h"
#include "sge_qsiL.h"
#include "sge_queueL.h"
#include "sge_rangeL.h"
#include "sge_range_schedd.h"
#include "sge_reportL.h"
#include "sge_requestL.h"
#include "sge_schedconfL.h"
#include "sge_schedd_text.h"
#include "sge_secL.h"
#include "sge_select_queue.h"
#include "sge_share_tree_nodeL.h"
#include "sge_static_load.h"
#include "sge_string.h"
#include "sge_stringL.h"
#include "sge_time_eventL.h"
#include "sge_ulongL.h"
#include "sge_usageL.h"
#include "sge_usermapL.h"
#include "sge_userprjL.h"
#include "sge_usersetL.h"
#include "commd.h"
#include "commd_message_flags.h"
#include "commlib.h"
#include "debit.h"
#include "sgeee.h"
/* #include "libintl.h" */
#include "load_correction.h"
#include "pack.h"
#include "pack_job_delivery.h"
#include "parse_qsubL.h"
#include "sge_feature.h"
#include "qmon_prefL.h"
#include "rmon.h"
#include "rmon_monitoring_level.h"
#include "scale_usage.h"
#include "schedd_conf.h"
#include "schedd_message.h"
#include "schedd_monitor.h"
#include "sig_handlers.h"
#include "slots_used.h"
#include "sort_hosts.h"
#include "subordinate_schedd.h"
#include "suspend_thresholds.h"
#include "valid_queue_user.h"
#include "zconf.h"
#include "zlib.h"
#include "cull.h"

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

SV* elem2hash(lListElem *ep);

SV* list2array(lList *lp) {
   lListElem *ep;
   AV* array = NULL;
   SV *elem;
   SV *ref = NULL;

   for_each(ep, lp) {
      if (!array)
         array = newAV();
      elem = elem2hash(ep);
      av_push(array, elem);
   }   
   ref = newRV_noinc((SV*)array); 
   return (SV*)ref; 
}   

SV* elem2hash(lListElem *ep) {
   static int nesting_level = 0;
   int i;
   struct _lList *tlp;
   HV* hash = NULL;
   SV* sv = NULL;
   SV* ref = NULL;
   char *key;
   int keylen;
   HE *entry;

   if (!ep)
      return NULL;

   hash = newHV();   
   for (i = 0; ep->descr[i].mt != lEndT; i++) {
      const char *str = NULL;
      key = lNm2Str(ep->descr[i].nm);
      keylen = strlen(key);
      sv = 0;
      switch (ep->descr[i].mt) {
      case lIntT:
         sv = newSViv(lGetPosInt(ep, i));
         break;
      case lUlongT:
         sv = newSViv(lGetPosUlong(ep, i));
         break;
      case lStringT:
         str = lGetPosString(ep, i);
         sv = newSVpv(str ? str : "", str ? strlen(str) : 0);
         break;
      case lListT:
         nesting_level++; 
         tlp = lGetPosList(ep, i); 
         if ((tlp != 0) && (lGetNumberOfElem(tlp) > 0))
            sv = list2array(tlp); 
         nesting_level--; 
         break;
      case lFloatT:
         sv = newSVnv((double)lGetPosFloat(ep, i));
         break;
      case lDoubleT:
         sv = newSVnv(lGetPosDouble(ep, i));
         break;
      case lLongT:
         sv = newSViv(lGetPosLong(ep, i));
         break;
      case lCharT:
         sv = newSViv(lGetPosChar(ep, i));
         break;
      case lRefT:
         break;
      default:
         unknownType("elem2hash");
      }
      if (sv != 0) hv_store(hash, key, keylen, sv, 0);
   }

   ref = newRV_noinc((SV*)hash); 
   return ref;
}

/* this function is used to get the C-Type (_enum_lMultiType) of the lMultitype Attribute */
/* from the the unique attribute ID (lDescr->nm) */
int lNm2Mt (int nm, struct _lDescr * descr) {
   int i, n;

   if ((n = lCountDescr(descr)) > 0)
      for (i = 0; i <= n; i++) {
         if (descr[i].nm == nm)
            return descr[i].mt;
      }
   return NoName;
}

struct descr_str_struct {
   char * prefix;
   struct _lDescr * type;
};


/* mapping array for lStr2Descr () !!! */
/* this is VERY IMPORTANT to transfer data from perl to C */
static struct descr_str_struct descr_str_array[] = {
   /* inspired by c4/gdilib/sge_boundaries.h */
      "JB_", JB_Type,
      "QU_", QU_Type,
      "EH_", EH_Type,
      "AH_", AH_Type,
      "SH_", SH_Type,
      "AN_", AN_Type,
      /* "HA", HA_Type, */
      "HL_", HL_Type,
      "HS_", HS_Type,
      /* "HT", HT_Type, */
      "EV_", EV_Type,
      "CE_", CE_Type,
      "CX_", CX_Type,
      "OR_", OR_Type,
      "OQ_", OQ_Type,
      "RQ_", RQ_Type,
      "RE_", RE_Type,
      "LR_", LR_Type,
      "US_", US_Type,
      "UE_", UE_Type,
      "RN_", RN_Type,
      "PN_", PN_Type,
      "AT_", AT_Type,
      "VA_", VA_Type,
      "MR_", MR_Type,
      "MO_", MO_Type,
      "ET_", ET_Type,
      "CONF_", CONF_Type,
      /* "CFSCHED_", CFSCHED_Type, */
      "CF_", CF_Type,
      "PE_", PE_Type,
      "QR_", QR_Type,
      "JQ_", JQ_Type,
      "ST_", ST_Type,
      "String", ST_Type,
      "JG_", JG_Type,
      "HD_", HD_Type,
      "HF_", HF_Type,
      "QAJ_", QAJ_Type,
      "SPA_", SPA_Type,
      "REP_", REP_Type,
      "SO_", SO_Type,
      "UA_", UA_Type,
      "STN_", STN_Type,
      "SC_", SC_Type,
      /* "YYYCONF_", YYYCONF_Type, */
      "JR_", JR_Type,
      "LIC_", LIC_Type,
      "JL_", JL_Type,
      "JP_", JP_Type,
      "UPU_", UPU_Type,
      "CK_", CK_Type,
      "KRB_", KRB_Type,
      "PA_", PA_Type,
      "CS_", CS_Type,
      "CO_", CO_Type,
      "JRE_", JRE_Type,
      "ID_", ID_Type,
      "MA_", MA_Type,
      "TE_", TE_Type,
      "CAL_", CAL_Type,
      "CA_", CA_Type,
      "TMR_", TMR_Type,
      "TM_", TM_Type,
      "RT_", RT_Type,
      "UPP_", UPP_Type,
      "JO_", JO_Type,
      "KTGT_", KTGT_Type,
      "SME_", SME_Type,
      "MES_", MES_Type,
      "JAT_", JAT_Type,
      "CT_", CT_Type,
      "PGR_", PGR_Type,
      "USR_", USR_Type,
      "JRL_", JRL_Type,
      "ULNG_", ULNG_Type,
      "PREF_", PREF_Type,
      "GRP_", GRP_Type,
      "PERM_", PERM_Type,
      "UME_", UME_Type,
      "LS_", LS_Type,
      "UM_", UM_Type,
      /* "SEC", SEC_Type, */
      /* inspired by all other headers in c4/gdilib */
      "UP_", UP_Type,
      "JC_", JC_Type,
/* inspired by all headers in c4/cull/src, uncommented since not used 
      "H_", HostT,
      "J_", JobT,
      "R_", JobRequestT,
      "O_", OwnerT,
      "Q_", QueueT,
      "C_", ComplexT,
      "A_", ComplexAttributeT,
      "N_", ComplexNameT, */
      0, 0
};

/* nmhhh, very strange but it works */
/* Houston, we have a problem ...   */
/* This sgee is VERY EXPERIMENTAL, but Lothar, Andreas & Andre say,  */
/* it's okay to do it like this     */
/* This function is used to resolve the type (JB_Type, QU_Type etc.) */
/* on the basis of one element's attribute name */
/* the reason why is that we don't save the Type of the list in the perl-cull */
/* (perhaps we should save the type instead of the list name) */


struct _lDescr * lStr2Descr (const char *str) {
   struct descr_str_struct *i;

   if (str)
      for (i = descr_str_array; (i->prefix) != 0; i++)
         if (strncmp(i->prefix, str, strlen(i->prefix)) == 0) return i->type;
   return 0;
}

/* this function converts a perl-array and a listname to a lList */
/* since the perl-lList-equivalent is stored as a perl-cull entry, with the */
/* listname as the key (perl-string), and the lListElem's as the value (perl-array), */
/* we have to submit both to transform it to a lList */
struct _lList * larray2List(SV *array_rv, char *listName) {
   struct _lList * argument;
   struct _lDescr * descr;
   int listType;
   struct _lList *list;
   struct _lListElem * element;
   int i, len_array, len_hvkey, mt, nm;
   unsigned long len_hvvalue;
   char * hvkey_str, * hvvalue_str;
   SV *sv, *hvvalue, *sv3;
   SV  **tv;
   AV *av, *av2, *av3;
   HV *hv, *hv2, *hv3;
   HE *he;

   if (SvROK(array_rv) && (SvTYPE(SvRV(array_rv)) == SVt_PVAV)) {
      descr = 0;
      element = NULL;
      list = NULL;

      /* get the Elements of the list */
      av = (AV*)SvRV(array_rv);
      len_array = av_len(av);

      /* cycle thru lListElem's */
      for (i = 0; i <= len_array; i++) {
         /* if there was an element before, add the former element to the list */
         if (element && list) lAppendElem(list, element);
         element = 0;
         /* get the new element */
         tv = av_fetch(av, i, 0);        
         /* test if it is really an element */
         if (SvROK(*tv) && (SvTYPE(SvRV(*tv)) == SVt_PVHV)) {
            hv = (HV *)SvRV(*tv);
            hv_iterinit(hv);

            /* go thru the attributes */
            while (he = hv_iternext(hv)) {

               /* get the attributes name (created with lNm2Str(lDescr->nm)) */
               hvkey_str = hv_iterkey(he, &len_hvkey);

               /* if we don't know the description of the list yet, */
               /* we probably did not create it at all, so we have to */
               /* detect the kind of list by the attribute's name */
               /* of the element. So add new list if needed */
               if (descr == 0) {
                  descr = lStr2Descr(hvkey_str);
                  list = lCreateList(listName, descr);
               }
               if (list) {
                  /* add new element to list if needed */
                  if (!element)
                     element = lCreateElem(descr);

                  /* get (the value of) the attribute itself */  
                  hvvalue = hv_iterval(hv, he);
                  /* get the unique attribute-ID from the attribute's name */
                  /* N.B. this function is not well tested, 'though it's a official one */
                  nm = lStr2Nm(hvkey_str);
                     if (nm != NoName) {
                        /* now the difficult part: */
                        /* find the corresponding C-Type (_enum_lMultiType), which is in the lDescriptor */
                        /* so that we can cast the perl-type to the correct C-Type */
                        mt = lNm2Mt(nm, descr);

                     if ((mt == lFloatT)) 
                        if (SvTYPE(hvvalue) == SVt_NV) lSetFloat(element, nm, (lFloat)SvNV(hvvalue));
                        else lSetFloat(element, nm, (lFloat)atof(SvPV(hvvalue, PL_na)));
                     if ((mt == lDoubleT)) 
                        if (SvTYPE(hvvalue) == SVt_NV) lSetDouble(element, nm, (lDouble)SvNV(hvvalue));
                        else lSetDouble(element, nm, (lDouble)atof(SvPV(hvvalue, PL_na)));
                     if ((mt == lUlongT)) 
                        if (SvTYPE(hvvalue) == SVt_IV) lSetUlong(element, nm, (lUlong)SvIV(hvvalue));
                        else lSetUlong(element, nm, (lUlong)atol(SvPV(hvvalue, PL_na)));
                     if ((mt == lLongT)) 
                        if (SvTYPE(hvvalue) == SVt_IV) lSetLong(element, nm, (lLong)SvIV(hvvalue));
                        else lSetLong(element, nm, (lLong)atol(SvPV(hvvalue, PL_na)));
                     if ((mt == lCharT)) 
                        if (SvTYPE(hvvalue) == SVt_IV) lSetChar(element, nm, (lChar)SvIV(hvvalue));
                        else lSetChar(element, nm, (lChar)atoi(SvPV(hvvalue, PL_na)));
                     if ((mt == lIntT)) 
                        if (SvTYPE(hvvalue) == SVt_IV) lSetInt(element, nm, (lInt)SvIV(hvvalue));
                        else lSetInt(element, nm, (lInt)atoi(SvPV(hvvalue, PL_na)));
                     if ((mt == lStringT)) {
                        hvvalue_str = SvPV(hvvalue, len_hvvalue);
                        if (len_hvvalue > 0) lSetString(element, nm, (lString)hvvalue_str);
                     }
                     if ((mt == lListT)) lSetList(element, nm, (struct _lList *)larray2List(hvvalue, hvkey_str));
                     if (mt == lRefT) printf("SGE.i:443: should not happen! got C reference while transforming cull list from Perl to C - ignore it\n");
                     if (mt == lEndT) printf("SGE.i:444: mhhh ... should not happen! got C 'end of cull list' while transforming cull list from Perl to C - ignore it\n");
                  }
               }
            }
         } else list = 0;
      }
      if (element && list) lAppendElem(list, element);
   } else list = 0;

   return list;
}



%}

%typemap(perl5, in) FILE * {
   $target = IoIFP(sv_2io($source));
}

%include "sge_gdi.h"
%include "cull_what.h"
%include "cull_list.h"
%include "cull_listP.h"
/* %include "sge_jobL.h" */
/* %include "sge_queueL.h" */
/* %include "cull_multitypeP.h"
*/

/* transform a single lList to a perl-cull, as in answerList = sge_gdi(SGE_GDI_MOD) */

%typemap(perl5, out) struct _lList * {
   if ($source) {
      $target = (SV*)list2array($source);
      argvi++;
      sv_2mortal($target);
   };
}

%typemap(perl5, out) struct _lList * perl_gdi_get(int target, lCondition* cp, lEnumeration *enp) {
   if ($source) {
      $target = (SV*)list2array($source);
      /* this is important, since perl_gdi_get creates a cull-list for us,
         but we can't free it, because we loose the reference to the list in perl */
      lFreeList($source);
      argvi++;
      sv_2mortal($target);
   };
}

%typemap(perl5, out) struct _lList * perl_gdi_change(int target, char * command_name, struct _lList * alList) {
   if ($source) {
      $target = (SV*)list2array($source);
      /* this is important, since perl_gdi_change creates an answer cull-list for us,
         but we can't free it, because we loose the reference to the list in perl */
      lFreeList($source);
      argvi++;
      sv_2mortal($target);
   };
}


/* transfer a perl-cull (just one entry) into a lList */
%typemap(perl5, in) struct _lList * {
   struct _lList *lp = NULL;
   int len_hvkey;
   HV *hv;
   HE *he;

   /* we have to get sure that we got really a perl cull as input */
   if (($source)) {
      lp = larray2List($source, "Perl CULL List");
      $target = lp;
   };
}

/* after we transformed the perl-cull into a lList, the C-function */
/* is executed, with the newly created lList as an argument. When it returns, */
/* the lList is not longer required, so we have to free it */
%typemap(perl5, freearg) struct _lList * {
   if ($source != 0)
      lFreeList ($source);
}

%inline %{

/* get a perl cull list representation of a C cull list */
/* note that this doesn't free (lFreeList()) the C cull list ! */
struct _lList * cull2perl(lList *list) {
   printf("executing cull2perl\n");
   return (struct _lList *)list;
}

/* get a C cull list representation of a perl cull list */
lList * perl2cull(struct _lList *list) {
   printf("executing perl2cull\n");
   return (lList *)list;
}

struct _lList * perl_gdi_change(int target, int command, struct _lList * alList) {
   struct _lList *lp = alList;

   if (command) {
      return sge_gdi((u_long32)target, command, &lp, NULL, NULL);
   } else
      return 0;
}

/* we have to wrap sge_gdi(SGE_GDI_GET), since is returns 2 lists */
struct _lList * perl_gdi_get(int target, lCondition* cp,  lEnumeration *enp) {
   struct _lList *lp = NULL;
   struct _lList *alp = NULL;

   if (!enp) {
      enp = lWhatAll();
      alp = sge_gdi((u_long32)target, SGE_GDI_GET, &lp, 0, enp);
      lFreeWhat(enp);
   } else
      alp = sge_gdi((u_long32)target, SGE_GDI_GET, &lp, 0, enp);

/* lWriteListTo(lp, stderr);  */
/* lWriteListTo(alp, stdout); */

   if (lp) {
      if (alp) lFreeList(alp);
      return lp;
   } else
      return alp;
}

void perl_lWriteListTo(struct _lList * alList) {
   lWriteListTo(alList, stdout);
}

SV* elem2hash(lListElem *ep);

SV* list2array(lList *lp);

/* things from sge_queueL.h */
/* enum { */
/*   BQ = 0x01,  /* batch Q */
/*   IQ = 0x02,  /* interactive Q */
/*   CQ = 0x04,  /* checkpointing Q */
/*   PQ = 0x08,  /* parallel Q */
/*   TQ = 0x10   /* transfer Q */
/*}; */

/* Q states moved over from def.h */
#define QALARM                               0x00000001
#define QSUSPEND_ALARM                       0x00000002
#define QDISABLED                            0x00000004
#define QENABLED                             0x00000008
#define QRUNNING                             0x00000080
#define QSUSPENDED                           0x00000100
#define QUNKNOWN                             0x00000400
#define QERROR                               0x00004000
#define QSUSPENDED_ON_SUBORDINATE            0x00008000
#define QCLEAN                               0x00010000
#define QCAL_DISABLED                        0x00020000
#define QCAL_SUSPENDED                       0x00040000

#define JIDLE                                0x00000000
#define JHELD                                0x00000010
#define JMIGRATING                           0x00000020
#define JQUEUED                              0x00000040
#define JRUNNING                             0x00000080
#define JSUSPENDED                           0x00000100
#define JTRANSITING                          0x00000200
#define JDELETED                             0x00000400
#define JWAITING                             0x00000800
#define JEXITING                             0x00001000
#define JWRITTEN                             0x00002000
#define JWAITING4OSJID                       0x00004000
#define JERROR                               0x00008000
#define JSUSPENDED_ON_THRESHOLD              0x00010000
#define JFINISHED                            0x00010000
#define JSLAVE                               0x00020000

%}
