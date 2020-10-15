#ifndef __SGE_CALENDAR_TM_L_H
#define __SGE_CALENDAR_TM_L_H

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
 * this data structure is used for 
 *    yeardays 
 *        TM_mday
 *        TM_mon
 *        TM_year
 *    weekdays 
 *        TM_wday
 *    daytimes 
 *        TM_hour
 *        TM_min
 *        TM_sec
 *        
 *   TM_Type borrows most fields and meaning from struct tm
 */
enum {
   TM_mday = TM_LOWERBOUND,
   TM_mon,
   TM_year,
   TM_sec,
   TM_min,
   TM_hour,
   TM_wday,
   TM_yday,
   TM_isdst
};

LISTDEF(TM_Type)
   JGDI_MAPPED_OBJ(java.util.Calendar)
   SGE_ULONG(TM_mday, CULL_DEFAULT)         /* 1-32 */
   SGE_ULONG(TM_mon, CULL_DEFAULT)          /* 0-11 */
   SGE_ULONG(TM_year, CULL_DEFAULT)         /* The number of years since 1900. */
   SGE_ULONG(TM_sec, CULL_DEFAULT)          /* 0-59 */
   SGE_ULONG(TM_min, CULL_DEFAULT)          /* 0-59 */
   SGE_ULONG(TM_hour, CULL_DEFAULT)         /* 0-23 */
   SGE_ULONG(TM_wday, CULL_DEFAULT)         /* 0-6 */
   SGE_ULONG(TM_yday, CULL_DEFAULT)         /* ?? */
   SGE_ULONG(TM_isdst, CULL_DEFAULT)        /* 1 or 0 */
LISTEND 

NAMEDEF(TMN)
   NAME("TM_mday")
   NAME("TM_mon")
   NAME("TM_year")
   NAME("TM_sec")
   NAME("TM_min")
   NAME("TM_hour")
   NAME("TM_wday")
   NAME("TM_yday")
   NAME("TM_isdst")
NAMEEND

#define TMS sizeof(TMN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_CALENDARL_H */
