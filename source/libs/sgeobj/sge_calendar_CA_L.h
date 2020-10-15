#ifndef __SGE_CALENDAR_CA_L_H
#define __SGE_CALENDAR_CA_L_H

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

#include "cull/cull.h"
#include "sgeobj/sge_boundaries.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */   

/* 
 * this data structure is used for parsing calendar functionality
 */
enum {
   CA_yday_range_list = CA_LOWERBOUND,
   CA_wday_range_list,
   CA_daytime_range_list,
   CA_state
};

LISTDEF(CA_Type)
   JGDI_OBJ(ParsedCalendar)
   SGE_LIST(CA_yday_range_list, TMR_Type, CULL_DEFAULT)       /* TMR_Type with begin/end of type *
                                       * TM_Type using *
                                       * TM_mday/TM_mon/TM_year */
   SGE_LIST(CA_wday_range_list, TMR_Type, CULL_DEFAULT)       /* TMR_Type with begin/end of type *
                                       * TM_Type using TM_wday */
   SGE_LIST(CA_daytime_range_list, TMR_Type, CULL_DEFAULT)    /* TMR_Type with begin/end of type *
                                       * TM_Type using *
                                       * TM_sec/TM_min/TM_hour */
   SGE_ULONG(CA_state, CULL_DEFAULT)
LISTEND 

NAMEDEF(CAN)
   NAME("CA_yday_range_list")
   NAME("CA_wday_range_list")
   NAME("CA_daytime_range_list")
   NAME("CA_state")
NAMEEND

#define CAS sizeof(CAN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_CALENDARL_H */
