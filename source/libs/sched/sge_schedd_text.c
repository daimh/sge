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

#include "uti/sge_rmon.h"

#include "sched/sge_schedd_text.h"
#include "sched/msg_schedd.h"

#include "basis_types.h"

/*
** Prototype for internationalized messages
** translation (used in sge_schedd_text() )
*/
const char* sge_get_schedd_text(int number);

/****** sge_schedd_text/sge_get_schedd_text() **********************************
*  NAME
*     sge_get_schedd_text() -- transformes a id into a info message
*
*  SYNOPSIS
*     const char* sge_get_schedd_text(int nr) 
*
*  FUNCTION
*    transforms an id into an info message
*
*  INPUTS
*     int nr - info id
*
*  RESULT
*     const char* -  info message
*
*  NOTES
*     MT-NOTE: sge_get_schedd_text() is MT safe 
*
*  SEE ALSO
*     sge_schedd_text.h for a detailed description on how to extend this.
*
*******************************************************************************/
const char* sge_get_schedd_text( int nr ) 
{
 
   switch(nr)
   {  
      case SCHEDD_INFO_CANNOTRUNATHOST_SSS      :
         return MSG_SCHEDD_INFO_CANNOTRUNATHOST_SSS       ;  
 
      case SCHEDD_INFO_HASNOPERMISSION_SS        :
         return MSG_SCHEDD_INFO_HASNOPERMISSION_SS         ;   
 
      case SCHEDD_INFO_HASINCORRECTPRJ_SSS      :
         return MSG_SCHEDD_INFO_HASINCORRECTPRJ_SSS       ;  
 
      case SCHEDD_INFO_HASNOPRJ_SS              :
         return MSG_SCHEDD_INFO_HASNOPRJ_SS               ;
 
      case SCHEDD_INFO_EXCLPRJ_SSS               :
         return MSG_SCHEDD_INFO_EXCLPRJ_SSS; 
 
      case SCHEDD_INFO_QUEUENOTREQUESTABLE_S    :
         return MSG_SCHEDD_INFO_QUEUENOTREQUESTABLE_S     ;   
 
      case SCHEDD_INFO_NOTINHARDQUEUELST_S      :
         return MSG_SCHEDD_INFO_NOTINHARDQUEUELST_S       ;   
 
      case SCHEDD_INFO_NOTPARALLELQUEUE_S       :
         return MSG_SCHEDD_INFO_NOTPARALLELQUEUE_S        ;  
 
      case SCHEDD_INFO_NOTINQUEUELSTOFPE_SS     :
         return MSG_SCHEDD_INFO_NOTINQUEUELSTOFPE_SS      ; 
 
      case SCHEDD_INFO_NOTACKPTQUEUE_S          :
         return MSG_SCHEDD_INFO_NOTACKPTQUEUE_S           ;
 
      case SCHEDD_INFO_NOTINQUEUELSTOFCKPT_SS   :
         return MSG_SCHEDD_INFO_NOTINQUEUELSTOFCKPT_SS    ;  
 
      case SCHEDD_INFO_QUEUENOTINTERACTIVE_S    :
         return MSG_SCHEDD_INFO_QUEUENOTINTERACTIVE_S     ;   
 
      case SCHEDD_INFO_NOTASERIALQUEUE_S        :
         return MSG_SCHEDD_INFO_NOTASERIALQUEUE_S         ;  
 
      case SCHEDD_INFO_NOTPARALLELJOB_S         :
         return MSG_SCHEDD_INFO_NOTPARALLELJOB_S          ; 
 
      case SCHEDD_INFO_NOTREQFORCEDRES_SS       :
         return MSG_SCHEDD_INFO_NOTREQFORCEDRES_SS        ; 
 
      case SCHEDD_INFO_WOULDSETQEUEINALARM_DS   :
         return MSG_SCHEDD_INFO_WOULDSETQEUEINALARM_IS    ;   
 
      case SCHEDD_INFO_NOSLOTSINQUEUE_S         :
         return MSG_SCHEDD_INFO_NOSLOTSINQUEUE_S          ;   
 
      case SCHEDD_INFO_CANNOTRUNINQUEUE_SSS     :
         return MSG_SCHEDD_INFO_CANNOTRUNINQUEUE_SSS      ;   

      case SCHEDD_INFO_CANNOTRUNINQUEUECAL_SU   :
         return MSG_SCHEDD_INFO_CANNOTRUNINQUEUECAL_SU    ;

      case SCHEDD_INFO_NORESOURCESPE_           :
         return MSG_SCHEDD_INFO_NORESOURCESPE_            ;   

      case SCHEDD_INFO_TOTALPESLOTSNOTINRANGE_S :
         return MSG_SCHEDD_INFO_TOTALPESLOTSNOTINRANGE_S  ;

      case SCHEDD_INFO_CANNOTRUNGLOBALLY_SS     :
         return MSG_SCHEDD_INFO_CANNOTRUNGLOBALLY_SS      ;   
 
      case SCHEDD_INFO_NOFORCEDRES_SS           :
         return MSG_SCHEDD_INFO_NOFORCEDRES_SS            ;  
 
      case SCHEDD_INFO_NOGLOBFORCEDRES_S        :
         return MSG_SCHEDD_INFO_NOGLOBFORCEDRES_S         ;
 
      case SCHEDD_INFO_CKPTNOTFOUND_            :
         return MSG_SCHEDD_INFO_CKPTNOTFOUND_             ;   
 
      case SCHEDD_INFO_PESLOTSNOTINRANGE_SI      :
         return MSG_SCHEDD_INFO_PESLOTSNOTINRANGE_SI       ;  
 
      case SCHEDD_INFO_NOACCESSTOPE_S           :
         return MSG_SCHEDD_INFO_NOACCESSTOPE_S            ;   
 
      case SCHEDD_INFO_QUEUEINALARM_SS          :
         return MSG_SCHEDD_INFO_QUEUEINALARM_SS           ;       
 
      case SCHEDD_INFO_QUEUEOVERLOADED_SS       :
         return MSG_SCHEDD_INFO_QUEUEOVERLOADED_SS        ;   
 
      case SCHEDD_INFO_ALLALARMOVERLOADED_      :
         return MSG_SCHEDD_INFO_ALLALARMOVERLOADED_       ;   
 
      case SCHEDD_INFO_TURNEDOFF_               :
         return MSG_SCHEDD_INFO_TURNEDOFF_                ;  
 
      case SCHEDD_INFO_JOBLIST_                 :
         return MSG_SCHEDD_INFO_JOBLIST_                  ;   
 
      case SCHEDD_INFO_EXECTIME_                :
         return MSG_SCHEDD_INFO_EXECTIME_                 ;   
 
      case SCHEDD_INFO_JOBINERROR_              :
         return MSG_SCHEDD_INFO_JOBINERROR_               ;   
 
      case SCHEDD_INFO_JOBHOLD_                :
         return MSG_SCHEDD_INFO_JOBHOLD_                  ;   
 
      case SCHEDD_INFO_USRGRPLIMIT_             :
         return MSG_SCHEDD_INFO_USRGRPLIMIT_              ;     
 
      case SCHEDD_INFO_JOBDEPEND_               :
         return MSG_SCHEDD_INFO_JOBDEPEND_                ;      
 
      case SCHEDD_INFO_NOMESSAGE_               :
         return MSG_SCHEDD_INFO_NOMESSAGE_                ;   
 
      case SCHEDD_INFO_QUEUEFULL_S              :
         return MSG_SCHEDD_INFO_QUEUEFULL_S               ;
 
      case SCHEDD_INFO_QUEUESUSP_S              :
         return MSG_SCHEDD_INFO_QUEUESUSP_S               ;
 
      case SCHEDD_INFO_QUEUEDISABLED_S          :
         return MSG_SCHEDD_INFO_QUEUEDISABLED_S           ;
 
      case SCHEDD_INFO_QUEUENOTAVAIL_S          :
         return MSG_SCHEDD_INFO_QUEUENOTAVAIL_S           ;
      
      case SCHEDD_INFO_INSUFFICIENTSLOTS_       :
         return MSG_SCHEDD_INFO_INSUFFICIENTSLOTS_        ;

      case SCHEDD_INFO_PEALLOCRULE_S            :
         return MSG_SCHEDD_INFO_PEALLOCRULE_S             ;

      case SCHEDD_INFO_NOPEMATCH_               :
         return MSG_SCHEDD_INFO_NOPEMATCH_ ;  

      case SCHEDD_INFO_CLEANUPNECESSARY_S :
         return MSG_SCHEDD_INFO_CLEANUPNECESSARY_S;

      case SCHEDD_INFO_MAX_AJ_INSTANCES_:
         return MSG_SCHEDD_INFO_MAX_AJ_INSTANCES_;

      case SCHEDD_INFO_JOB_CATEGORY_FILTER_     :
         return MSG_SCHEDD_INFO_JOB_CATEGORY_FILTER_      ;

      case SCHEDD_INFO_CANNOTRUNRQS_SSS        :
         return MSG_SCHEDD_INFO_CANNOTRUNRQS_SSS         ;

      case SCHEDD_INFO_JOBDYNAMICALLIMIT_SS   :
         return MSG_SCHEDD_INFO_JOBDYNAMICALLIMIT_SS      ;

      case SCHEDD_INFO_CANNOTRUNRQSGLOBAL_SS:
         return MSG_SCHEDD_INFO_CANNOTRUNRQSGLOBAL_SS; 

      case SCHEDD_INFO_QINOTARRESERVED_SI:
         return MSG_SCHEDD_INFO_QINOTARRESERVED_SI; 

      case SCHEDD_INFO_QNOTARRESERVED_SI:
         return MSG_SCHEDD_INFO_QNOTARRESERVED_SI; 

      case SCHEDD_INFO_ARISINERROR_I:
         return MSG_SCHEDD_INFO_ARISINERROR_I; 

/* */

      case SCHEDD_INFO_CANNOTRUNATHOST          :
         return MSG_SCHEDD_INFO_CANNOTRUNATHOST           ;   
 
      case SCHEDD_INFO_HASNOPERMISSION          :
         return MSG_SCHEDD_INFO_HASNOPERMISSION           ;  
 
      case SCHEDD_INFO_HASINCORRECTPRJ          :
         return MSG_SCHEDD_INFO_HASINCORRECTPRJ           ;  
 
      case SCHEDD_INFO_HASNOPRJ                 :
         return MSG_SCHEDD_INFO_HASNOPRJ                  ;  
 
      case SCHEDD_INFO_EXCLPRJ                  :
         return MSG_SCHEDD_INFO_EXCLPRJ                   ;  
 
      case SCHEDD_INFO_QUEUENOTREQUESTABLE      :
         return MSG_SCHEDD_INFO_QUEUENOTREQUESTABLE       ;  
 
      case SCHEDD_INFO_NOTINHARDQUEUELST        :
         return MSG_SCHEDD_INFO_NOTINHARDQUEUELST         ;  
 
      case SCHEDD_INFO_NOTPARALLELQUEUE         :
         return MSG_SCHEDD_INFO_NOTPARALLELQUEUE          ;

      case SCHEDD_INFO_NOTINQUEUELSTOFPE        :
         return MSG_SCHEDD_INFO_NOTINQUEUELSTOFPE         ;  
 
      case SCHEDD_INFO_NOTACKPTQUEUE            :
         return MSG_SCHEDD_INFO_NOTACKPTQUEUE             ;   
 
      case SCHEDD_INFO_NOTINQUEUELSTOFCKPT      :
         return MSG_SCHEDD_INFO_NOTINQUEUELSTOFCKPT       ;  
 
      case SCHEDD_INFO_QUEUENOTINTERACTIVE      :
         return MSG_SCHEDD_INFO_QUEUENOTINTERACTIVE       ; 
 
      case SCHEDD_INFO_NOTASERIALQUEUE          :
         return MSG_SCHEDD_INFO_NOTASERIALQUEUE           ;

      case SCHEDD_INFO_NOTPARALLELJOB           :
         return MSG_SCHEDD_INFO_NOTPARALLELJOB            ;

      case SCHEDD_INFO_NOTREQFORCEDRES          :
         return MSG_SCHEDD_INFO_NOTREQFORCEDRES           ;   
 
      case SCHEDD_INFO_WOULDSETQEUEINALARM      :
         return MSG_SCHEDD_INFO_WOULDSETQEUEINALARM       ;     
 
      case SCHEDD_INFO_NOSLOTSINQUEUE           :
         return MSG_SCHEDD_INFO_NOSLOTSINQUEUE            ;   
 
      case SCHEDD_INFO_CANNOTRUNINQUEUE         :
         return MSG_SCHEDD_INFO_CANNOTRUNINQUEUE          ;   
 
      case SCHEDD_INFO_NORESOURCESPE        :
         return MSG_SCHEDD_INFO_NORESOURCESPE         ;  

      case SCHEDD_INFO_TOTALPESLOTSNOTINRANGE   :
         return MSG_SCHEDD_INFO_TOTALPESLOTSNOTINRANGE    ;
 
      case SCHEDD_INFO_CANNOTRUNGLOBALLY        :
         return MSG_SCHEDD_INFO_CANNOTRUNGLOBALLY         ;  
 
      case SCHEDD_INFO_NOFORCEDRES              :
         return MSG_SCHEDD_INFO_NOFORCEDRES               ;      
 
      case SCHEDD_INFO_NOGLOBFORCEDRES          :
         return MSG_SCHEDD_INFO_NOGLOBFORCEDRES           ; 
 
      case SCHEDD_INFO_CKPTNOTFOUND             :
         return MSG_SCHEDD_INFO_CKPTNOTFOUND              ;
 
      case SCHEDD_INFO_PESLOTSNOTINRANGE        :
         return MSG_SCHEDD_INFO_PESLOTSNOTINRANGE         ; 
 
      case SCHEDD_INFO_NOACCESSTOPE             :
         return MSG_SCHEDD_INFO_NOACCESSTOPE              ;  
 
      case SCHEDD_INFO_QUEUEINALARM             :
         return MSG_SCHEDD_INFO_QUEUEINALARM              ;    
 
      case SCHEDD_INFO_QUEUEOVERLOADED          :
         return MSG_SCHEDD_INFO_QUEUEOVERLOADED           ; 
 
      case SCHEDD_INFO_ALLALARMOVERLOADED       :
         return MSG_SCHEDD_INFO_ALLALARMOVERLOADED        ; 
 
      case SCHEDD_INFO_TURNEDOFF                :
         return MSG_SCHEDD_INFO_TURNEDOFF                 ;      
 
      case SCHEDD_INFO_JOBLIST                  :
         return MSG_SCHEDD_INFO_JOBLIST                   ;   
 
      case SCHEDD_INFO_EXECTIME                 :
         return MSG_SCHEDD_INFO_EXECTIME                  ;              
 
      case SCHEDD_INFO_JOBINERROR               :
         return MSG_SCHEDD_INFO_JOBINERROR                ;   
 
      case SCHEDD_INFO_JOBHOLD                  :
         return MSG_SCHEDD_INFO_JOBHOLD                   ; 
 
      case SCHEDD_INFO_USRGRPLIMIT              :
         return MSG_SCHEDD_INFO_USRGRPLIMIT               ;     
 
      case SCHEDD_INFO_JOBDEPEND                :
         return MSG_SCHEDD_INFO_JOBDEPEND                 ;      
 
      case SCHEDD_INFO_NOMESSAGE                :
         return MSG_SCHEDD_INFO_NOMESSAGE                 ;            
 
      case SCHEDD_INFO_QUEUEFULL                :
         return MSG_SCHEDD_INFO_QUEUEFULL                 ;     
 
      case SCHEDD_INFO_QUEUESUSP                :
         return MSG_SCHEDD_INFO_QUEUESUSP                 ;   
 
      case SCHEDD_INFO_QUEUEDISABLED            :
         return MSG_SCHEDD_INFO_QUEUEDISABLED             ;   
 
      case SCHEDD_INFO_QUEUENOTAVAIL            :
         return MSG_SCHEDD_INFO_QUEUENOTAVAIL             ; 

      case SCHEDD_INFO_INSUFFICIENTSLOTS        :
         return MSG_SCHEDD_INFO_INSUFFICIENTSLOTS         ;

      case SCHEDD_INFO_PEALLOCRULE              :
         return MSG_SCHEDD_INFO_PEALLOCRULE               ;  

      case SCHEDD_INFO_NOPEMATCH:
         return MSG_SCHEDD_INFO_NOPEMATCH;

      case SCHEDD_INFO_CLEANUPNECESSARY:
         return MSG_SCHEDD_INFO_CLEANUPNECESSARY;

      case SCHEDD_INFO_MAX_AJ_INSTANCES:
         return MSG_SCHEDD_INFO_MAX_AJ_INSTANCES;

      case SCHEDD_INFO_JOB_CATEGORY_FILTER:
         return MSG_SCHEDD_INFO_JOB_CATEGORY_FILTER;

      case SCHEDD_INFO_CANNOTRUNINQUEUECAL   :
         return MSG_SCHEDD_INFO_CANNOTRUNINQUEUECAL    ;

      case SCHEDD_INFO_CANNOTRUNRQS:
         return MSG_SCHEDD_INFO_CANNOTRUNRQS;

      case SCHEDD_INFO_JOBDYNAMICALLIMIT:
         return MSG_SCHEDD_INFO_JOBDYNAMICALLIMIT;

      case SCHEDD_INFO_CANNOTRUNRQSGLOBAL:
         return MSG_SCHEDD_INFO_CANNOTRUNRQSGLOBAL; 

      case SCHEDD_INFO_QINOTARRESERVED:
         return MSG_SCHEDD_INFO_QINOTARRESERVED; 

      case SCHEDD_INFO_QNOTARRESERVED:
         return MSG_SCHEDD_INFO_QNOTARRESERVED; 

      case SCHEDD_INFO_ARISINERROR:
         return MSG_SCHEDD_INFO_ARISINERROR; 

      default:
         return "";
   }
}

const char *sge_schedd_text(int number) {
   const char *error_text = NULL;

   DENTER(TOP_LAYER, "sge_schedd_text");

   error_text = sge_get_schedd_text(number);

   if (error_text == NULL) {
      DEXIT;
      return MSG_SYSTEM_GOTNULLASERRORTEXT;
   }
 
   if (strlen(error_text) == 0) {
      DEXIT;
      return MSG_SYSTEM_INVALIDERRORNUMBER;
   }

   DEXIT;
   return error_text;
}

