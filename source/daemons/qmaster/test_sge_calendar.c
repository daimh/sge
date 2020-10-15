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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include "uti/sge_rmon.h"
#include "uti/sge_time.h"
#include "uti/sge_unistd.h"
#include "uti/sge_log.h"
#include "uti/sge_lock.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_calendar.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_qinstance_state.h"

#include "spool/sge_spooling.h"

#include "sge.h"
#include "sge_utility_qmaster.h"
#include "sge_persistence_qmaster.h"
#include "sge_event_master.h"
#include "sge_c_gdi.h"
#include "sge_calendar_qmaster.h"
#include "sge_qmod_qmaster.h"
#include "sge_qinstance_qmaster.h"
#include "msg_common.h"
#include "msg_qmaster.h"

/* new data types */

typedef struct {
   char *year_cal; /* year calendar definition */ 
   char *week_cal; /* week calendar definition */
   char *description; /* a calendar description for the test output */
} cal_entry_t;

typedef struct {
   int         cal_nr; /* the calendar to test */
   struct tm   now;    /* the current date */
   struct tm   result1;  /* the expected state change date */
   int         state1;   /* the expected current state */
   struct tm   result2;  /* the expected state change date */
   int         state2;   /* the expected state change state */
} date_entry_t;

typedef struct {
   int         cal_nr;     /* the calendar to test */
   struct tm   start_time; /* start of time frame */
   u_long32    duration;   /* duration of time frame */
   bool        open;       /* calendar open or closed */
} time_frame_entry_t;

/* global test variables */

/* should the look ahead list be printed? The list is not tested yet */
static int is_print_look_ahead = 0;

/**
 *
 * Calendar definitions for testing
 */
static cal_entry_t calendars[] = { 
/*year calendar*/                  {"1.2.2004-1.3.2004=suspended","NONE",
                                    "queue is suspended in March 2004"},
                                    
                                   {"1.2.2004-1.3.2004=off","NONE",
                                    "queue is off in March 2004"},
                                    
                                   {"1.2.2004-1.4.2004=off 1.3.2004-1.5.2004=off","NONE",
                                    "queue is off from March till June 2004, using 2 calendar entries"}, 
                                    
                                   {"1.2.2004-1.4.2004=suspended 1.3.2004-1.5.2004=off","NONE",
                                    "two overlapping calendar entries, one off, one suspended"}, 
                                    
                                   {"1.2.2004-1.4.2004=9:0-18:0=suspended","NONE",
                                    "queue is suspended in March 2004 during the day"},
                                    
                                   {"1.2.2004-1.4.2004=18:0-9:0=suspended","NONE",
                                    "queue is enabled in March 2004 during the day"}, 
                                    
                                   {"1.2.2004-1.6.2004=18:0-9:0=suspended 1.3.2004-1.5.2004=suspended","NONE",
                                    "queue is supended during the night, and turned suspended for 2 month."},   


/* no calendar */                  {"NONE", "NONE",
                                    "no calendar defined"},
                                    
                                    
/* week calendar*/                 {"NONE", "Mon-Sun=suspended",
                                    "queue is always disabled"},       

                                   {"NONE", "Mon-Sun=09:00-18:00=suspended",
                                    "queue is disabled during the day"},
                                    
                                   {"NONE", "Mon-sun=18:00-09:00=suspended",
                                    "queue is disabled during the night"},
                                    
                                   {"NONE", "Mon,Wed,Fri=09:00-18:00=suspended",
                                    "queue is disabled on Monday, Friday, and Wednesday during the day"},

                                   {"NONE", "Mon-Wed=09:00-18:00=suspended Mon-Fri=suspended",
                                    "queue is disabled on Monday till Wednesday during the day"},


/*mixed calendars */               {"1.2.2004-1.3.2004=suspended","Mon-Sun=09:00-18:00=suspended",
                                     "queue is disabled during the day, except from 2/1/2004 till 3/1/2004. During that time it disabled for the whole day."},
                                     
                                   {"24.12.2004-26.12.2004=on", "Mon-Fri=06:00-18:00=off Mon-Fri=09:00-18:00=suspended",
                                    "queue is only enabled on the none working hours and Christmas"},
                                  
                                   {"1.2.2004-1.3.2004=suspended", "Mon-Sun=suspended Mon-Sun=09:00-18:00=suspended",   
                                   "queue is always disabled"}, 

                                   {"NONE", "Sun-Wed=on Wed-Sat=on", 
                                   "queue is always enabled"}, 
                                   
                                   {"1.1.2004-1.2.2004=suspended 1.2.2004-1.3.2004=suspended 1.3.2004-1.4.2004=suspended 1.4.2004-30.4.2004=suspended 1.5.2004-1.6.2004=suspended", "NONE",
                                   "queue is always disabled"},  

                                   {"NONE", "Mon-Wed=on Wed-Fri,Wed-Sat,Sun=on",  
                                   "queue is always enabled"}, 

                                   {"1.2.2004-1.3.2004=on", "Mon-Wed=on Wed-Sun=on Mon-Sun=09:00-18:00=on",    
                                   "queue is always enabled"},  
                                  
                                   {"NONE", "09:00-18:00=suspended",
                                    "queue is suspended from 9 to 6 every day"},
                                 
                                   {"NONE","Sun-Sat=suspended Wed-Fri=on",
                                    "queue is always suspended except Wednesday till Friday"},

/* disabling queues */             {"off","NONE","queue is always off"},
                                   {"suspended", "NONE","queue is always suspended"},
                                   {"NONE", "off", "queue is always off"},
                                   {"NONE", "suspended", "queue is always suspended"},

/* issue 1787 */                   {"NONE","mon=0:0:0-21:0:0", "queue is off every monday from 0 to 21 hours"},


/* end of definition */            {NULL, NULL}
                                };
/**
 *
 * Test definitions
 *
 * If no state change is expected, the result has to be set to: 
 * "0,0,1, 1,0,70, 0,0,0". This coresponds to the time in sec of 0. 
 *
 * The time/date definition is: sec, min, hour, day(starting with 1), 
 * month(starting with 0) year (since 1900), 0, 0, 0.
 *
 * A -1 in state2 means that this entry does not exist.
 */
static date_entry_t tests[] = { {0, {0,0,0, 1,0,104, 0,0,0}, {0,0,0, 1,1,104, 0,0,0}, QI_DO_NOTHING, {0,0,0, 2,2,104, 0,0,0}, QI_DO_CAL_SUSPEND},
                                {0, {0,0,0, 1,1,104, 0,0,0}, {0,0,0, 2,2,104, 0,0,0}, QI_DO_CAL_SUSPEND, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING},
                                {0, {0,0,0, 2,2,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING, {0,0,1, 1,0, 70, 0,0,0}, -1},

                                {1, {0,0,0, 1,0,104, 0,0,0}, {0,0,0, 1,1,104, 0,0,0}, QI_DO_NOTHING, {0,0,0, 2,2,104, 0,0,0}, QI_DO_CAL_DISABLE},
                                {1, {0,0,0, 1,1,104, 0,0,0}, {0,0,0, 2,2,104, 0,0,0}, QI_DO_CAL_DISABLE, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING},
                                {1, {0,0,0, 2,2,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING, {0,0,1, 1,0, 70, 0,0,0}, -1}, 

                                {2, {0,0,0, 1,0,104, 0,0,0}, {0,0,0, 1,1,104, 0,0,0}, QI_DO_NOTHING, {0,0,0, 2,4,104, 0,0,1}, QI_DO_CAL_DISABLE},
                                {2, {0,0,0, 1,1,104, 0,0,0}, {0,0,0, 2,4,104, 0,0,1}, QI_DO_CAL_DISABLE, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING},
                                {2, {0,0,0, 1,2,104, 0,0,0}, {0,0,0, 2,4,104, 0,0,1}, QI_DO_CAL_DISABLE, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING},
                                {2, {0,0,0, 1,3,104, 0,0,0}, {0,0,0, 2,4,104, 0,0,1}, QI_DO_CAL_DISABLE, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING},
                                {2, {0,0,0, 2,4,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING, {0,0,1, 1,0, 70, 0,0,0}, -1}, 

                                {3, {0,0,0, 1,0,104, 0,0,0}, {0,0,0, 1,1,104, 0,0,0}, QI_DO_NOTHING, {0,0,0, 2,3,104, 0,0,1}, QI_DO_CAL_SUSPEND},
                                {3, {0,0,0, 1,1,104, 0,0,0}, {0,0,0, 2,3,104, 0,0,1}, QI_DO_CAL_SUSPEND, {0,0,0, 2,4,104, 0,0,1}, QI_DO_CAL_DISABLE},
                                {3, {0,0,0, 1,2,104, 0,0,0}, {0,0,0, 2,3,104, 0,0,1}, QI_DO_CAL_SUSPEND, {0,0,0, 2,4,104, 0,0,1}, QI_DO_CAL_DISABLE},
                                {3, {0,0,0, 2,3,104, 0,0,0}, {0,0,0, 2,4,104, 0,0,1}, QI_DO_CAL_DISABLE, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING},
                                {3, {0,0,0, 2,4,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING, {0,0,1, 1,0, 70, 0,0,0}, -1},    
                                
                                {4, {0,0, 0, 1,0,104, 0,0,0}, {0,0, 9, 1,1,104, 0,0,0}, QI_DO_NOTHING, {0,0,18, 1,1,104, 0,0,0}, QI_DO_CAL_SUSPEND},
                                {4, {0,0, 0, 2,2,104, 0,0,0}, {0,0, 9, 2,2,104, 0,0,0}, QI_DO_NOTHING, {0,0,18, 2,2,104, 0,0,0}, QI_DO_CAL_SUSPEND},
                                {4, {0,0,10, 2,2,104, 0,0,0}, {0,0,18, 2,2,104, 0,0,0}, QI_DO_CAL_SUSPEND, {0,0,9, 3,2,104, 0,0,0}, QI_DO_NOTHING},
                                {4, {0,0,19, 2,2,104, 0,0,0}, {0,0, 9, 3,2,104, 0,0,0}, QI_DO_NOTHING, {0,0,18, 3,2,104, 0,0,0}, QI_DO_CAL_SUSPEND},
                                {4, {0,0,0, 2,4,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING, {0,0,1, 1,0, 70, 0,0,0}, -1}, 
                          
                                {5, {0,0, 0, 1,0,104, 0,0,0}, {0,0,18, 1,1,104, 0,0,0}, QI_DO_NOTHING, {0,0, 9, 2,1,104, 0,0,0}, QI_DO_CAL_SUSPEND},
                                {5, {0,0,20, 1,2,104, 0,0,0}, {0,0, 9, 2,2,104, 0,0,0}, QI_DO_CAL_SUSPEND, {0,0,18, 2,2,104, 0,0,0}, QI_DO_NOTHING},
                                {5, {0,0,10, 2,2,104, 0,0,0}, {0,0,18, 2,2,104, 0,0,0}, QI_DO_NOTHING, {0,0,9, 3,2,104, 0,0,0}, QI_DO_CAL_SUSPEND},
                                {5, {0,0,0, 2,4,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING, {0,0,1, 1,0, 70, 0,0,0}, -1},      

                                {6, {0,0, 0, 1,2,104, 0,0,0}, {0,0, 9, 2,4,104, 0,0,1}, QI_DO_CAL_SUSPEND, {0,0,18, 2,4,104, 0,0,1}, QI_DO_NOTHING},
                                
                                {7, {0,0, 0, 1,2,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING, {0,0,1, 1,0, 70, 0,0,0}, -1}, 
                               
                                {8, {0,0, 0, 1,2,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_CAL_SUSPEND, {0,0,1, 1,0, 70, 0,0,0}, -1}, 
                               
                                {9, {0,0, 0, 1,1,104, 0,0,0}, {0,0, 9, 1,1,104, 0,0,0}, QI_DO_NOTHING,     {0,0,18, 1,1,104, 0,0,0}, QI_DO_CAL_SUSPEND},
                                {9, {0,0,10, 1,1,104, 0,0,0}, {0,0,18, 1,1,104, 0,0,0}, QI_DO_CAL_SUSPEND, {0,0, 9, 2,1,104, 0,0,0}, QI_DO_NOTHING},
                                {9, {0,0,20, 1,1,104, 0,0,0}, {0,0, 9, 2,1,104, 0,0,0}, QI_DO_NOTHING,     {0,0,18, 2,1,104, 0,0,0}, QI_DO_CAL_SUSPEND},
                               
                                {10, {0,0, 0, 1,1,104, 0,0,0}, {0,0, 9, 1,1,104, 0,0,0}, QI_DO_CAL_SUSPEND, {0,0,18, 1,1,104, 0,0,0}, QI_DO_NOTHING},
                                {10, {0,0,20, 1,1,104, 0,0,0}, {0,0, 9, 2,1,104, 0,0,0}, QI_DO_CAL_SUSPEND, {0,0,18, 2,1,104, 0,0,0}, QI_DO_NOTHING}, 
                      
                                {11, {0,0, 0,22,8,104, 0,0,1}, {0,0, 9,22,8,104, 0,0,1}, QI_DO_NOTHING,     {0,0,18,22,8,104, 0,0,1}, QI_DO_CAL_SUSPEND},
                                {11, {0,0,10,22,8,104, 0,0,1}, {0,0,18,22,8,104, 0,0,1}, QI_DO_CAL_SUSPEND, {0,0, 9,24,8,104, 0,0,1}, QI_DO_NOTHING},
                                {11, {0,0,20,22,8,104, 0,0,1}, {0,0, 9,24,8,104, 0,0,1}, QI_DO_NOTHING,     {0,0,18,24,8,104, 0,0,1}, QI_DO_CAL_SUSPEND},
                                {11, {0,0,20,24,8,104, 0,0,1}, {0,0, 9,27,8,104, 0,0,1}, QI_DO_NOTHING,     {0,0,18,27,8,104, 0,0,1}, QI_DO_CAL_SUSPEND},
                                {11, {0,0,20,20,8,104, 0,0,1}, {0,0, 9,22,8,104, 0,0,1}, QI_DO_NOTHING,     {0,0,18,22,8,104, 0,0,1}, QI_DO_CAL_SUSPEND}, 

                                {12, {0,0, 0,20,8,104, 0,0,1}, {0,0, 0,25,8,104, 0,0,1}, QI_DO_CAL_SUSPEND, {0,0, 0,27,8,104, 0,0,1}, QI_DO_NOTHING}, 
                                
                                {13, {0,0, 0,20,8,104, 0,0,1}, {0,0, 9,20,8,104, 0,0,1}, QI_DO_NOTHING, {0,0,18,20,8,104, 0,0,1}, QI_DO_CAL_SUSPEND}, 
                                {13, {0,0, 0, 2,1,104, 0,0,0}, {0,0, 0, 2,2,104, 0,0,0}, QI_DO_CAL_SUSPEND, {0,0, 9, 2,2,104, 0,0,0}, QI_DO_NOTHING}, 
                                {13, {0,0,10, 1,0,104, 0,0,0}, {0,0,18, 1,0,104, 0,0,0}, QI_DO_CAL_SUSPEND, {0,0, 9, 2,0,104, 0,0,0}, QI_DO_NOTHING}, 

                                {14, {0,0, 0,24,11,104, 0,0,0}, {0,0, 6,27,11,104, 0,0,0}, QI_DO_NOTHING, {0,0, 9,27,11,104, 0,0,0}, QI_DO_CAL_DISABLE}, 
                                {15, {0,0, 0, 1,2,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_CAL_SUSPEND, {0,0,1, 1,0, 70, 0,0,0}, -1},  
                                {16, {0,0, 0, 1,2,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING, {0,0,1, 1,0, 70, 0,0,0}, -1}, 
                                {17, {0,0, 0, 2,0,104, 0,0,0}, {0,0,0, 2,5, 104, 0,0,1},QI_DO_CAL_SUSPEND , {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING }, 
                                {18, {0,0, 0, 1,2,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING, {0,0,1, 1,0, 70, 0,0,0}, -1},
                                {19, {0,0, 0, 1,2,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_NOTHING, {0,0,1, 1,0, 70, 0,0,0}, -1},

                                {20, {0,0, 0, 1,1,104, 0,0,0}, {0,0, 9, 1,1,104, 0,0,0}, QI_DO_NOTHING,     {0,0,18, 1,1,104, 0,0,0}, QI_DO_CAL_SUSPEND},
                                {20, {0,0,10, 1,1,104, 0,0,0}, {0,0,18, 1,1,104, 0,0,0}, QI_DO_CAL_SUSPEND, {0,0, 9, 2,1,104, 0,0,0}, QI_DO_NOTHING},
                                {20, {0,0,20, 1,1,104, 0,0,0}, {0,0, 9, 2,1,104, 0,0,0}, QI_DO_NOTHING,     {0,0,18, 2,1,104, 0,0,0}, QI_DO_CAL_SUSPEND},

                                {21, {0,0, 0,20,8,104, 0,0,1}, {0,0, 0,22,8,104, 0,0,1}, QI_DO_CAL_SUSPEND,     {0,0, 0,25,8,104, 0,0,1}, QI_DO_NOTHING},

                                {22, {0,0, 0, 1,2,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_CAL_DISABLE, {0,0,1, 1,0, 70, 0,0,0}, -1}, 
                                {23, {0,0, 0, 1,2,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_CAL_SUSPEND, {0,0,1, 1,0, 70, 0,0,0}, -1},
                                {24, {0,0, 0, 1,2,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_CAL_DISABLE, {0,0,1, 1,0, 70, 0,0,0}, -1},
                                {25, {0,0, 0, 1,2,104, 0,0,0}, {0,0,1, 1,0, 70, 0,0,0}, QI_DO_CAL_SUSPEND, {0,0,1, 1,0, 70, 0,0,0}, -1},
                               
                                {26, {0,0, 0, 1,2,104, 0,0,0}, {0,0,21, 1,2, 104, 0,0,0}, QI_DO_CAL_DISABLE, {0,0,0, 8,2, 104, 0,0,0}, QI_DO_NOTHING}, 
                                {26, {0,0, 10, 1,2,104, 0,0,0}, {0,0,21, 1,2, 104, 0,0,0}, QI_DO_CAL_DISABLE, {0,0,0, 8,2, 104, 0,0,0}, QI_DO_NOTHING},
                                {26, {0,0, 22, 1,2,104, 0,0,0}, {0,0,0, 8,2, 104, 0,0,0}, QI_DO_NOTHING, {0,0,21, 8,2, 104, 0,0,0}, QI_DO_CAL_DISABLE}, 
                                {26, {0,0, 12, 3,2,104, 0,0,0}, {0,0,0, 8,2, 104, 0,0,0}, QI_DO_NOTHING, {0,0,21, 8,2, 104, 0,0,0}, QI_DO_CAL_DISABLE},  
                                
                                {-1, {0,0,0, 0,0,104, 0,0,0}, {0,0,0, 0,0,104, 0,0,0}, -1, {0,0,0, 0,0,104, 0,0,0}, -1}
                                  };

static time_frame_entry_t time_frame_tests[] = {
/*year calendar*/               {0, {0,0,0, 1,0,104, 0,0,0}, 3600, true},
                                {0, {0,30,23, 31,0,104, 0,0,0}, 3600, false},
                                {0, {0,0,12, 31,0,105, 0,0,0}, 3600, true},
                                {1, {0,0,0, 1,0,104, 0,0,0}, 3600, true},
                                {1, {0,30,23, 31,0,104, 0,0,0}, 3600, false},
                                {1, {0,0,12, 31,0,105, 0,0,0}, 3600, true},
                                {2, {0,0,0, 1,0,104, 0,0,0}, 3600, true},
                                {2, {0,30,23, 31,0,104, 0,0,0}, 3600, false},
                                {2, {0,0,12, 31,0,104, 0,0,0}, 6048000, false}, /* 70 days */
/* no calendar */               {7, {0,30,23, 31,0,104, 0,0,0}, 3600, true},
/* week calendar*/              {8, {0,0,0, 1,0,104, 0,0,0}, 3600, false},
                                {8, {0,30,23, 31,0,104, 0,0,0}, 3600, false},
                                {8, {0,0,12, 31,0,105, 0,0,0}, 3600, false},
                                {9, {1,0,18, 12,5,105, 0,0,1}, 53999, false}, /* 15 hours minus one second */
                                {9, {1,0,18, 12,5,105, 0,0,1}, 53998, true}, /* 15 hours minus two seconds */
                                {9, {0,0,18, 12,5,105, 0,0,1}, 53999, false}, /* 15 hours minus one seconds */
/*mixed calendars */            {13, {1,0,18, 2,2,104, 0,0,0}, 3600, true},
                                {13, {1,0,18, 15,1,104, 0,0,0}, 3600, false},
                                {-1, {0,0,0, 0,0,104, 0,0,0}, 0, false}
                              };


/* test functions */
static int test(void *context, date_entry_t *test, cal_entry_t *calendar, int test_nr); 
static int test_state_change_list(date_entry_t *test, lList *state_changes);
static int test_state_change(lListElem *stateObject, u_long32 state, struct tm *time, int elemNr);

static int test_time_frame(void *context, time_frame_entry_t *test, cal_entry_t *calendar, int test_nr); 

/* setup functions */
static lListElem *createCalObject(void *context, cal_entry_t *calendar);

/* output functions */
static void printDateError(time_t *when, struct tm *time);



/****** test_sge_calendar/test_state_change() **********************************
*  NAME
*     test_state_change() -- tests a single state change from the state change list
*
*  SYNOPSIS
*     static int test_state_change(lListElem *stateObject, u_long32 state, 
*     struct tm *time, int elemNr) 
*
*  FUNCTION
*     tests a single state change from the state change list
*
*  INPUTS
*     lListElem *stateObject - a state change object
*     u_long32 state         - expected state
*     struct tm *time        - expected time
*     int elemNr             - element nr for output
*
*  RESULT
*     static int - 0 okay / 1 test failed
*
*  NOTES
*     MT-NOTE: test_state_change() is MT safe 
*
*******************************************************************************/
static int test_state_change(lListElem *stateObject, u_long32 state, struct tm *time, int elemNr) 
{
   int ret = 0;

   if (lGetUlong(stateObject, CQU_state) != state) {
      ret = 1;
      printf("wrong state in state list (elem %d): expected %d, got %d\n", elemNr, (int) state, (int) lGetUlong(stateObject, CQU_state));
   }
   else {
      time_t now  = mktime(time);
      time_t result = (time_t) lGetUlong(stateObject, CQU_till);
      if (result != now) {
         ret = 1;
         printf("state list elem %d: ", elemNr);
         printDateError(&result, time);
      }
   }
   
   return ret;
}

/****** test_sge_calendar/test_state_change_list() *****************************
*  NAME
*     test_state_change_list() -- evaluates the state change list
*
*  SYNOPSIS
*     static int test_state_change_list(date_entry_t *test, lList 
*     *state_changes) 
*
*  FUNCTION
*     evaluates the state change list. The state change list is limited to
*     two states.
*
*  INPUTS
*     date_entry_t *test   - test to perform
*     lList *state_changes - state change list
*
*  RESULT
*     static int -  0 okay / 1 test failed
*
*  NOTES
*     MT-NOTE: test_state_change_list() is MT safe 
*
*******************************************************************************/
static int test_state_change_list(date_entry_t *test, lList *state_changes) 
{
   int ret = 0;
   int nr;
   if (is_print_look_ahead != 0) {
      lWriteListTo(state_changes, stdout);
   }   

   if (test->state2 != -1) {
      if ((nr = lGetNumberOfElem(state_changes)) != 2) {
         printf("wrong number of elemens in state change list. expected: %d, got: %d\n", 2, nr);
         ret = 1;
      }
      else {
         lListElem *state = lFirst(state_changes);
         ret |= test_state_change(state, test->state1, &(test->result1), 1);

         state = lNext(state);
         ret |= test_state_change(state, test->state2, &(test->result2), 2);
      }      
      
   }
   else {
      if ((nr = lGetNumberOfElem(state_changes)) != 1) {
         printf("wrong number of elemens in state change list. expected: %d, got: %d\n", 1, nr);
         ret = 1;
      }
      else {
         lListElem *state = lFirst(state_changes);
         ret |= test_state_change(state, test->state1, &(test->result1), 1);
      }
   }
   
   return ret;
}

/****** test_sge_calendar/printDateError() *************************************
*  NAME
*     printDateError() -- print date information in case of an error
*
*  SYNOPSIS
*     static void printDateError(time_t *when, struct tm *time) 
*
*  FUNCTION
*     print date information in case of an error
*
*  INPUTS
*     time_t *when    - result time
*     struct tm *time - expected time
*
*  NOTES
*     MT-NOTE: printDateError() is MT safe 
*
*******************************************************************************/
static void printDateError(time_t *when, struct tm *time) 
{
   struct tm *result;
   struct tm res;

   result = localtime_r(when, &res);
   
   printf("wrong change date:\n");
   printf("expected: sec:%d min:%d hour:%d mday:%d mon:%d year:%d wday:%d yday:%d isdst:%d\n",
      time->tm_sec,
      time->tm_min,
      time->tm_hour,
      time->tm_mday,
      time->tm_mon,
      time->tm_year,
      time->tm_wday,
      time->tm_yday,
      time->tm_isdst);
   printf("got     : sec:%d min:%d hour:%d mday:%d mon:%d year:%d wday:%d yday:%d isdst:%d\n",
      result->tm_sec,
      result->tm_min,
      result->tm_hour,
      result->tm_mday,
      result->tm_mon,
      result->tm_year,
      result->tm_wday,
      result->tm_yday,
      result->tm_isdst);      
}

/****** test_sge_calendar/createCalObject() ************************************
*  NAME
*     createCalObject() -- creates a calendar object from the cal data structure
*
*  SYNOPSIS
*     static lListElem* createCalObject(cal_entry_t *calendar) 
*
*  FUNCTION
*     creates a calendar object from the cal data structure
*
*  INPUTS
*     cal_entry_t *calendar - calendar definition
*
*  RESULT
*     static lListElem* - calendar object or NULL
*
*  NOTES
*     MT-NOTE: createCalObject() is MT safe 
*
*******************************************************************************/
static lListElem *createCalObject(void *context, cal_entry_t *calendar) 
{
   monitoring_t monitor;
   lListElem *sourceCal = NULL;
   lListElem *destCal = NULL;
   lList *answerList = NULL;
   
   sge_monitor_init(&monitor, "cal_test", NONE_EXT, NO_WARNING, NO_ERROR);
   
   sourceCal = lCreateElem(CAL_Type);

   lSetString(sourceCal, CAL_name, "test");
   lSetString(sourceCal, CAL_year_calendar, calendar->year_cal);
   lSetString(sourceCal, CAL_week_calendar, calendar->week_cal);

   destCal = lCreateElem(CAL_Type);
   
   if (0 != calendar_mod(context, &answerList, destCal, sourceCal, 1, "", "", NULL, 0, &monitor)) {
      lWriteListTo(answerList, stdout);
      lFreeElem(&destCal);
      lFreeList(&answerList);
   }
  
   lFreeElem(&sourceCal);
   
   sge_monitor_free(&monitor);

   return destCal;
}


/****** test_sge_calendar/test() ***********************************************
*  NAME
*     test() --  performs a single test
*
*  SYNOPSIS
*     static int test(date_entry_t *test, cal_entry_t *calendar, int test_nr) 
*
*  FUNCTION
*     performs a single test
*
*  INPUTS
*     date_entry_t *test    - test to perform
*     cal_entry_t *calendar - calendar to use
*     int test_nr           - test nr for output
*
*  RESULT
*     static int -  0 okay / 1 test failed
*
*  NOTES
*     MT-NOTE: test() is MT safe 
*
*******************************************************************************/
static int test(void *context, date_entry_t *test, cal_entry_t *calendar, int test_nr) 
{
   lListElem *destCal = NULL;
   int ret = 1;

   /* test output*/
   printf("\n==> Test Nr:     %d(%d)\n", test_nr, test->cal_nr);
   printf("==> Description: %s\n", calendar->description);
   printf("==> Time:        %d/%d/%d %d:%d:%d  (wday:%d yday:%d Summer time: %s)\n\n",
      (test->now.tm_mon + 1),      
      test->now.tm_mday,
      (test->now.tm_year + 1900),
   
      test->now.tm_hour,
      test->now.tm_min,
      test->now.tm_sec,
      
      test->now.tm_wday,
      test->now.tm_yday,
      (test->now.tm_isdst?"true":"false"));
   printf("==> year cal: \"%s\" week cal: \"%s\"\n", calendar->year_cal, calendar->week_cal);  

   /* start test */
   if ((destCal = createCalObject(context, calendar)) != NULL) {
      u_long32 current_state;
      time_t when = 0;
      time_t now  = mktime(&test->now);
      lList *state_changes_list = NULL;

      if (test->state1 == (current_state = calender_state_changes(destCal, &state_changes_list, &when, &now))) {
         if (when == mktime(&test->result1)) {
            if ((ret = test_state_change_list(test, state_changes_list)) == 0) {
               printf("==> Test is okay\n");
            }
         } else {
            printDateError(&when, &(test->result1));
         }
      } else {
         printf("wrong state: expected %d, got %d\n", test->state1, (int) current_state);
      }
      lFreeList(&state_changes_list);
   }
   
   /* test cleanup */
   printf("----------------\n");
   lFreeElem(&destCal);
   
   return ret;
}

/****** test_sge_calendar/main() ***********************************************
*  NAME
*     main() -- calendar test
*
*  SYNOPSIS
*     int main(int argc, char* argv[]) 
*
*  FUNCTION
*     calendar test
*
*  INPUTS
*     int argc     - nr. of args 
*     char* argv[] - args
*
*  RESULT
*     int -  nr of failed tests
*
*******************************************************************************/
int main(int argc, char* argv[])
{
   int test_counter = 0;
   int i = 0;
   int failed = 0;
   void *context = NULL;
   int cal_index = 0;

   lInit(nmv);
   obj_mt_init();
   
   printf("==> Calendar test <==\n");
   printf("---------------------\n");

   while ((cal_index = tests[i].cal_nr) != -1) {
      if (test(context, &(tests[i]), 
               &(calendars[cal_index]), 
               i) != 0) {
         failed++; 
      }   
      i++;
   }
   test_counter+=i;

   i=0;
   while ((cal_index = time_frame_tests[i].cal_nr) != -1) {
      if (test_time_frame(context, &(time_frame_tests[i]),
                          &(calendars[cal_index]),
                          i) != 0) {
         failed++;
      }
      i++;
   }
   test_counter+=i;
   
   if (failed == 0) {
      printf("\n==> All tests are okay <==\n");
   }
   else {
      printf("\n==> %d/%d test(s) failed <==\n", failed, test_counter);
   }
   
   return failed;
}

static int test_time_frame(void *context, time_frame_entry_t *test, cal_entry_t *calendar, int test_nr)
{
   int ret = 0;
   lListElem *destCal = NULL;
   struct tm *end_tm;
   struct tm res;
   u_long32 start_time = (u_long32)mktime(&test->start_time);
   time_t end_time = (time_t)duration_add_offset(start_time, test->duration);

   end_tm = localtime_r(&end_time, &res);

    /* test output*/
   printf("\n==> Test Nr:     %d(%d)\n", test_nr, test->cal_nr);
   printf("==> Description: %s\n", calendar->description);
   printf("==> Start Time:        %d/%d/%d %d:%d:%d  (wday:%d yday:%d Summer time: %s)\n",
      (test->start_time.tm_mon + 1),      
      test->start_time.tm_mday,
      (test->start_time.tm_year + 1900),
   
      test->start_time.tm_hour,
      test->start_time.tm_min,
      test->start_time.tm_sec,
      
      test->start_time.tm_wday,
      test->start_time.tm_yday,
      (test->start_time.tm_isdst?"true":"false"));
   printf("==> End Time:          %d/%d/%d %d:%d:%d  (wday:%d yday:%d Summer time: %s)\n",
      (end_tm->tm_mon + 1),      
      end_tm->tm_mday,
      (end_tm->tm_year + 1900),
   
      end_tm->tm_hour,
      end_tm->tm_min,
      end_tm->tm_sec,
      
      end_tm->tm_wday,
      end_tm->tm_yday,
      (end_tm->tm_isdst?"true":"false"));
   printf("==> year cal: \"%s\" week cal: \"%s\"\n", calendar->year_cal, calendar->week_cal);  

   if ((destCal = createCalObject(context, calendar)) != NULL) {
      bool result = calendar_open_in_time_frame(destCal, start_time, test->duration);
      if (test->open != result) {
         printf("wrong state for time frame: expected %d, got %d\n", test->open, result);
         ret++;
      } else {
         printf("==> Test is okay\n");
      }
   }

   /* test cleanup */
   printf("----------------\n");
   lFreeElem(&destCal);

   return ret;
}
