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
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include "cull/cull_list.h"

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"
#include "uti/sge_time.h"
#include "uti/sge_unistd.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_attr.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_calendar.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "sge.h"
#include "sge_c_gdi.h"
#include "sge_calendar_qmaster.h"
#include "sge_qmod_qmaster.h"
#include "msg_common.h"
#include "msg_qmaster.h"


enum {
   DOT = 1,
   COLON,
   EQUAL_SIGN,
   MINUS,
   COMMA,
   SPACE,
   NUMBER,
   STRING,
   NO_TOKEN,
   ERR_TOKEN
};

typedef struct {
   int token;
   char *text;
} token_set_t;


static token_set_t statev[] = {
   { QI_DO_ENABLE,   "on" },
   { QI_DO_DISABLE,  "off" },
   { QI_DO_SUSPEND, "suspended" },
   { -1, NULL },
};

typedef int (*cmp_func_t)(const lListElem *t1, const lListElem *t2); 

static char old_error[1000];
static char store[1000];
static int number;
static int token_is_valid = 0;
static char parse_error[MAX_STRING_SIZE];

/* parsing */
static int 
disabled_year_list(lList **alpp, const char *s, 
                   lList **cal, const char *cal_name);

static int disabled_week_list(lList **alpp, const char *s, lList **cal, const char *cal_name); 

static int 
state_at(time_t now, const lList *ycal, const lList *wcal, time_t *then);

static int scan(const char *s, token_set_t token_set[]);

static void join_wday_range(lList *week_day); 
static void extend_wday_range(lList *week_day);

static char *get_string(void);

static int get_number(void); 

static void eat_token(void);

static char *save_error(void);

static int cheap_scan(char *s, token_set_t tokenv[], int n);

static int disabled_year_entry(lListElem **calep);

static int year_day_range_list(lList **ydrl); 

static int year_day_range(lListElem **tmr);

static int year_day(lListElem **tm);

static int month(int *);

static int day(int *);

static int year(int *);

static int tm_yday_cmp(const lListElem *t1, const lListElem *t2);

static int tm_wday_cmp(const lListElem *t1, const lListElem *t2);

static int normalize_range_list(lList *rl, cmp_func_t cmp_func);

static bool in_range_list(lListElem *tm, lList *rl, cmp_func_t cmp_func); 

static int in_range(const lListElem *tm, const lListElem *r, cmp_func_t cmp_func);

static int daytime_range_list(lList **dtrl);

static int daytime_range(lListElem **tmr);

static void full_daytime_range(lList **dtrl);

static void full_weekday_range(lList **dtrl);

static void split_daytime_range(lList *dtrl, lListElem *tmr);

static int daytime(lListElem **tm);

static int hour(int *);

static int minute(int *);

static int seconds(int *);

static int action(int *sp);

static int range_number(int min, int max, int *ip, const char *name);

static int tm_daytime_cmp(const lListElem *t1, const lListElem *t2);

static u_long32 is_week_entry_active(lListElem *tm, lListElem *week_entry, time_t *limit, u_long32 *next_state, int rec_count);

static u_long32 is_year_entry_active(lListElem *tm, lListElem *year_entry, time_t *limit);

static time_t compute_limit(bool today, bool active, const lList *year_time, const lList *week_time, 
              const lList *day_time, const lListElem *now, bool *is_end_of_day_reached); 
                            
static int disabled_week_entry(lListElem **calep);

static int week_day_range_list(lList **wdrl);

static int week_day_range(lListElem **tmr);

static void split_wday_range(lList *wdrl, lListElem *tmr);

static int week_day(lListElem **tm);

static u_long32 calendar_get_current_state_and_end(const lListElem *this_elem, time_t *then, time_t *now);

lListElem *calendar_list_locate(lList *calendar_list, const char *cal_name) 
{
   return lGetElemStr(calendar_list, CAL_name, cal_name);
}

/*

   NAME
      state_at() - computes state using year and week calendar 
                   and optionally when this state will end

   RETURNS
      The actual state according this calendar
   
   PARAMETERS
      now        - actual time 
      ycal       - year calendar
      wcal       - week calendar
      next_event - if this pointer is not NULL then
                   state_at() will compute the next 
                   time when this calendar has to be checked again

   DESCRIPTION
      Here we have to answer two questions:
         1. Which one is the actual state?
            If both this day and the actual daytime are in an entry
            then this entry is active and the entries state is returned.
            In case there are more than one active entries the 'hardest'
            is dominant ( disabled < suspended < enabled )

         2. When will this state end?
            For all active entries we compute the end of the range. 
            For all inactive entries we compute the start of the range.
            We iterate through all these ranges beginning with that range
            which is closest to 'now' in the future.
          
            In some cases the state does not change but the range which is
            responsible for that state changes: 

               mon-fri=9-18 fri=18-19

   CALENDAR
      year
         CA_yday_range_list
            TMR_begin
               TM_mday
               TM_mon
               TM_year
            TMR_end
               TM_mday
               TM_mon
               TM_year
         CA_daytime_range_list
            TMR_begin
               TM_hour
               TM_min
               TM_sec
            TMR_end
               TM_hour
               TM_min
               TM_sec
         CA_state

      week
         CA_wday_range_list
            TMR_begin
               TM_wday
            TMR_end
               TM_wday
         CA_daytime_range_list
            TMR_begin
               TM_hour
               TM_min
               TM_sec
            TMR_end
               TM_hour
               TM_min
               TM_sec
         CA_state

   FUTURE ENHANCEMENTS
      The compution of 'next_event' could be improved. Actually 
      state_at() returns 24 o'clock of this day as the end of
      this state also when this state is valid beyond this day.

*/

/*
ycal,  CA_Type 
wcal,  CA_Type
*/
static int state_at(time_t now, const lList *ycal, const lList *wcal, time_t *next_event) {
   struct tm *tm_now;
   int state = 0, w_is_active = -1, y_is_active;
   lListElem *yc, *wc, *tm;
   time_t limit;
   time_t temp_next_event = 0;
   struct tm res;
   u_long32 next_state;

   DENTER(TOP_LAYER, "state_at");

   /* convert time_t format into struct tm format */
   tm_now = localtime_r(&now, &res);

   /* cullify struct tm */
   tm = lCreateElem(TM_Type);
   cullify_tm(tm, tm_now);

   DPRINTF(("now: sec: %d min: %d hour: %d mday: %d mon: %d year:%d wday: %d yday: %d isdst: %d\n",
         tm_now->tm_sec,
         tm_now->tm_min,
         tm_now->tm_hour,
         tm_now->tm_mday,
         tm_now->tm_mon,
         tm_now->tm_year,
         tm_now->tm_wday,
         tm_now->tm_yday,
         tm_now->tm_isdst)); 

   /* ycal */
   for_each (yc, ycal) {
      y_is_active = is_year_entry_active(tm, yc, &limit);
      if (state != y_is_active || state == QI_DO_NOTHING) {
         state |= y_is_active;
    
         if (temp_next_event == 0 || (limit != 0 && temp_next_event > limit)) {
            temp_next_event= limit;
         }
      }
      else {
         if (temp_next_event == 0 || (limit != 0 && temp_next_event < limit)) {
            temp_next_event= limit;
         }
      }
   }
   
   if (!state) { 
      int counter = 0;
      int max = lGetNumberOfElem(wcal);
      bool *visited = (bool*) malloc(max * sizeof(bool));
      bool isOverlapping;
     
      memset(visited, false, max * sizeof(bool));

      /* Week calendar */
      /* we can have overlapping ranges, which will not result in state changes. This code tries
         to figure out, when a range overlaps and we face no state change. */
      do {
         counter = 0;
         isOverlapping = false;
         
         for_each (wc, wcal) {
            if ((w_is_active = is_week_entry_active(tm, wc, &limit, &next_state, 0))) {
               state |= w_is_active;
            }   
            
            if (limit != 0 && state == next_state && temp_next_event >= limit) {
               if (!visited[counter]) {
                  isOverlapping = true;
                  visited[counter] = true;
                  temp_next_event += 1;
                  tm_now = localtime_r(&temp_next_event, &res);
                  cullify_tm(tm, tm_now);

                  temp_next_event = 0;
                  if ((w_is_active = is_week_entry_active(tm, wc, &limit, &next_state, 0))) {
                     state |= w_is_active;
                  }                 
               } else {
                  temp_next_event = 0;
                  isOverlapping = false;
                  break;
               }
            }
            
            if ((state != next_state) && (temp_next_event == 0 || ((limit != 0) && (temp_next_event > limit)))) {
               temp_next_event= limit;
            }   
            counter++;
         }
      } while(isOverlapping);

      sge_free(&visited);
   }  

   if (next_event != NULL) {
      *next_event = temp_next_event;
   }   
  
   DPRINTF(("got state %d from %s calendar. Now: %lu Next event: %lu\n", 
            state, (w_is_active==-1)?"year":"week", (unsigned long) now,
            (unsigned long) (next_event?*next_event:0)));

   lFreeElem(&tm);

   if ((state & QI_DO_ENABLE)) {
      DRETURN(QI_DO_ENABLE);
   }
   if ((state & QI_DO_SUSPEND)) {
      DRETURN(QI_DO_SUSPEND);
   }
   if ((state & QI_DO_DISABLE)) {
      DRETURN(QI_DO_DISABLE);
   }

   DRETURN(QI_DO_ENABLE);
}

/****** sge_calendar/is_week_entry_active() ************************************
*  NAME
*     is_week_entry_active() -- computes the current queue state and the next change
*
*  SYNOPSIS
*     static u_long32 is_week_entry_active(lListElem *tm, lListElem 
*     *week_entry, time_t *limit, int rec_count) 
*
*  FUNCTION
*     Computes the current queue state based on its calendar setting. It also figures
*     out how long this state lasts and when the next change will be. The function
*     for the next change has a short comming, that it will generate an state end, when
*     the current day changes. 
*     Therefore this function calles itself recursivly, if such an event happens. It can
*     only happen, when the callendar contains a state changes during a day. In Theory, it
*     should not call itself more than 1 time. With the extra call, it should have found a
*     new state change on the next day, or the day, when the next change comes.
*
*
*  INPUTS
*     lListElem *tm         - TM_Type - represeting now (time)
*     lListElem *week_entry - CA_Type - calendar entries for the week setup
*     time_t    *limit      - next state changea (0, if there is no further state change)
*     int       rec_count   - current recoursion level
*
*  RESULT
*     static u_long32 - the current state
*
*  NOTES
*     MT-NOTE: is_week_entry_active() is MT safe 
*
*******************************************************************************/
static u_long32 is_week_entry_active(lListElem *tm, lListElem *week_entry, time_t *limit, u_long32 *next_state, int rec_count) {
   u_long32 state;
   bool in_wday_range, in_daytime_range = false;


   DENTER(TOP_LAYER, "is_week_entry_active");

   /* compute state */
   if ((in_wday_range=in_range_list(tm, lGetList(week_entry, CA_wday_range_list), tm_wday_cmp)) 
     && (in_daytime_range=in_range_list(tm, lGetList(week_entry, CA_daytime_range_list), tm_daytime_cmp))) {
       /* DPRINTF(("in_range_list(wday) = %d in_range_list(daytime) = %d state = %d\n", 
                    in_wday_range, in_daytime_range, lGetUlong(week_entry, CA_state))); */
      state = lGetUlong(week_entry, CA_state);
      *next_state = 0;
   } else {
      /* DPRINTF(("in_range_list(wday) = %d in_range_list(daytime) = %d\n", in_wday_range, in_daytime_range)); */
      state = 0;
      *next_state = lGetUlong(week_entry, CA_state);

   }

   if (limit != NULL) {  
      bool is_end_of_the_day_reached = false;
      
      *limit = compute_limit(in_wday_range, in_daytime_range,NULL, lGetList(week_entry, CA_wday_range_list),
            lGetList(week_entry, CA_daytime_range_list), tm, &is_end_of_the_day_reached);

      if (is_end_of_the_day_reached) {
         struct tm *tm_now;
         struct tm res;
         lListElem *new_tm;
     
         /* we have to add a second to get into the next time slot */
         (*limit)++;
         
         /* convert time_t format into struct tm format */
         tm_now = localtime_r(limit, &res);

         /* cullify struct tm */
         new_tm= lCreateElem(TM_Type);
         cullify_tm(new_tm, tm_now);
    
         /* more than 2 recorsion should never be needed, we allow 4 to be save.*/
         if (rec_count < 4) {
            state = is_week_entry_active(new_tm, week_entry, limit, next_state, rec_count++); 
         }
         else {
            ERROR((SGE_EVENT, SFNMAX, MSG_CALENDAR_CALCTERMINATED));
         }

         lFreeElem(&new_tm);
      }

      /* there is no other state change */
      if (*limit == 0) {
         *next_state = 0;   
      }
   }

   DRETURN(state);
}

/* returns state and time when state changes acording this entry */
/*
lListElem *tm,         TM_Type 
lListElem *year_entry, CA_Type 
*/

/****** sge_calendar/is_year_entry_active() ************************************
*  NAME
*     is_year_entry_active() -- computes the current queue state and the next change
*
*  SYNOPSIS
*     static u_long32 is_year_entry_active(lListElem *tm, lListElem 
*     *week_entry, time_t *limit) 
*
*  FUNCTION
*     Computes the current queue state based on its calendar setting. It also figures
*     out how long this state lasts and when the next change will be. The function
*     for the next change has a short comming, that it will generate an state end, when
*     the current day changes. 
*     Therefore this function calles itself recursivly, if such an event happens. It can
*     only happen, when the callendar contains a state changes during a day. In Theory, it
*     should not call itself more than 1 time. With the extra call, it should have found a
*     new state change on the next day, or the day, when the next change comes.
*
*     This function has problem, when two calendar entries overlap themselfs and one has
*     day changes, the otherone has not. It will report those day changes, even though there
*     are none. It makse a mistake in figuring out, which calendar setting is the more important
*     one.
*
*
*  INPUTS
*     lListElem *tm         - TM_Type - represeting now (time)
*     lListElem *week_entry - CA_Type - calendar entries for the week setup
*     time_t *limit         - next state changea (0, if there is no further state change)
*
*  RESULT
*     static u_long32 - the current state
*
*  NOTES
*     MT-NOTE: is_week_entry_active() is MT safe 
*
*******************************************************************************/
static u_long32 is_year_entry_active(lListElem *tm, lListElem *year_entry, time_t *limit) {
   u_long32 state;
   bool in_yday_range, in_daytime_range = false;

   DENTER(TOP_LAYER, "is_year_entry_active");

   /* compute state */
   if ((in_yday_range=in_range_list(tm, lGetList(year_entry, CA_yday_range_list), tm_yday_cmp)) 
     && (in_daytime_range=in_range_list(tm, lGetList(year_entry, CA_daytime_range_list), tm_daytime_cmp))) {
      DPRINTF(("in_range_list(yday) = %d in_range_list(daytime) = %d state = %d\n", 
            in_yday_range, in_daytime_range, (int) lGetUlong(year_entry, CA_state)));
      state = lGetUlong(year_entry, CA_state);
   } else {
      DPRINTF(("in_range_list(yday) = %d in_range_list(daytime) = %d\n", in_yday_range, in_daytime_range)); 
      state = in_daytime_range?QI_DO_ENABLE:0;
   }

   if (limit) {
      bool is_end_of_the_day_reached = false;

      *limit = compute_limit(in_yday_range, in_daytime_range, lGetList(year_entry, CA_yday_range_list), NULL,
            lGetList(year_entry, CA_daytime_range_list), tm, &is_end_of_the_day_reached);

      if (is_end_of_the_day_reached) {
         struct tm *tm_now;
         struct tm res;
         lListElem *new_tm;
         DPRINTF(("trying the next time slot\n"));     
         /* we have to add a second to get into the next time slot */
         (*limit)++;
         
         /* convert time_t format into struct tm format */
         tm_now = localtime_r(limit, &res);

         /* cullify struct tm */
         new_tm= lCreateElem(TM_Type);
         cullify_tm(new_tm, tm_now);
      
         state = is_year_entry_active(new_tm, year_entry, limit);

         lFreeElem(&new_tm);
      }
            
   }

   DRETURN(state);
}


/****** sge_calendar/calender_state_changes() **********************************
*  NAME
*     calender_state_changes() -- calendar states and calendar state list
*
*  SYNOPSIS
*     u_long32 calender_state_changes(const lListElem *cep, lList 
*     **state_changes_list, time_t *when) 
*
*  FUNCTION
*   Computes the current state and generates a calendar state list. Right now it
*   only does it for the current state and the next state. But it could be extended
*   to handle more states.
*
*  Based on the shortcomings of the methods it is using, it has to check, wether they
*  reported a state change and if not, call the methods again with a new time stamp. It
*  is starting with now and goes into the future, based on the reported state changes.
*
*  INPUTS
*     const lListElem *cep       - (in) calendar (CAL_Type)
*     lList **state_changes_list - (out) a pointer to a list pointer (CQU_Type)
*     time_t *when               - (out) when will the next change be, or 0
*     time_t *now                - (in) should be NULL, or the current time
*                                  (only for the test programm)
*
*  RESULT
*     u_long32 - new state (QI_DO_NOTHING, QI_DO_DISABLE, QI_DO_SUSPEND)
*
*  NOTES
*     MT-NOTE: calender_state_changes() is MT safe 
*
*******************************************************************************/
u_long32 calender_state_changes(const lListElem *cep, lList **state_changes_list, time_t *when, time_t *now) {
   time_t temp_when = 0;
   time_t temp_now= 0;
   time_t when1 = 0;
   u_long32 state0 = 0;
   u_long32 state1 = 0;
   u_long32 state2 = 0; 
   lListElem *state_change = NULL;

   if (cep == NULL || state_changes_list == NULL) {
      return 0;
   }

   /* build queue state change list */
   state0 = calendar_get_current_state_and_end(cep, &temp_when, now);
   *when = temp_when;   

   /* calculate the next state shift. */
   if (temp_when != 0) {
      int state_changes = 0;
      const int max_state_changes = 60; /* we want to limit this calculation to about month and for that we
                                          assume 2 state changes per day. This limit is only needed for a
                                          scheduler configuration like: 
                                          year cal: "NONE", week cal: "Mon-Wed=09:00-18:00=suspended Mon-Fri=suspended"
                                          year cal: 1.2.2004-1.4.2004=off 1.3.2004-1.5.2004=off
                                        */  

      /* we do have to do the calculation at least once, but there are cases, in which the 
        calendar_get_current_state_and_end function does not return the correct state switch
        (overlapping calendar configurations), therefor we have to run it multiple times. */
      do {
         *when = temp_when;
         temp_now = temp_when + 1;
         state1 = calendar_get_current_state_and_end(cep, &temp_when, &temp_now);
         state_changes++;
      } while ((temp_when != 0) && (state0 == state1) && (state_changes < max_state_changes));

      if (state0 == state1) {
         *when = temp_when;
      }

      when1 = temp_when;
      /* some problem with the next interval */
      if (when1 != 0) {
         do {
            when1 = temp_when;
            temp_now = temp_when + 1;
            state2 = calendar_get_current_state_and_end(cep, &temp_when, &temp_now);
            state_changes++;
         } while ((temp_when != 0) && (state2 == state1) && (state_changes < max_state_changes));
      }

      if (state1 == state2) {
         when1 = temp_when;
      }      
   }
   
   *state_changes_list = lCreateList("state_changes", CQU_Type);
   state_change = lCreateElem(CQU_Type);
   
   lSetUlong(state_change, CQU_state, state0);
   lSetUlong(state_change,  CQU_till, *when);
   lAppendElem(*state_changes_list, state_change);
      
   /* extend queue state change list */
   if (*when != 0) {
      state_change = lCreateElem(CQU_Type);
      lSetUlong(state_change,  CQU_state,state1);
      lSetUlong(state_change,  CQU_till, when1);
      lAppendElem(*state_changes_list, state_change);
   }
   
#if 0
   {
         struct tm *tm_limit;
         struct tm res; 
                  /* convert time_t format into struct tm format */
         tm_limit= localtime_r(when, &res);

         DPRINTF(("first change: state %d, time "sge_u32", sec:%d min:%d hour:%d mday:%d mon:%d year:%d wday:%d yday:%d isdst:%d\n",
            state0,
            (u_long32) when,
            tm_limit->tm_sec,
            tm_limit->tm_min,
            tm_limit->tm_hour,
            tm_limit->tm_mday,
            tm_limit->tm_mon,
            tm_limit->tm_year,
            tm_limit->tm_wday,
            tm_limit->tm_yday,
            tm_limit->tm_isdst));

         tm_limit= localtime_r(&when1, &res);

         DPRINTF(("second change: state %d, time "sge_u32", sec:%d min:%d hour:%d mday:%d mon:%d year:%d wday:%d yday:%d isdst:%d\n",
            state1,
            (u_long32) when1,
            tm_limit->tm_sec,
            tm_limit->tm_min,
            tm_limit->tm_hour,
            tm_limit->tm_mday,
            tm_limit->tm_mon,
            tm_limit->tm_year,
            tm_limit->tm_wday,
            tm_limit->tm_yday,
            tm_limit->tm_isdst));
           
   }
#endif

   return state0;
}


/****** sge_calendar/compute_limit() *******************************************
*  NAME
*     compute_limit() -- compute the next state change
*
*  SYNOPSIS
*     static time_t compute_limit(bool today, bool active, const lList 
*     *year_time, const lList *week_time, const lList *day_time, const 
*     lListElem *now, bool *is_end_of_day_reached) 
*
*  FUNCTION
*     This function computes the next state change based on the three booleans:
*     today, active, and is_full_day. 
*
*     There are some special calendar states, which result in special code to 
*     handle thus states. They are all special to the week calendar.
*
*     - A week calendar can specify a range beyond the week borders. For example
*       Wed-Mon. This will be broken down to two structures:
*        - Wed-Sat (day 3 till 6)
*        - Sun-Mon. (day 0 till 1)
*       Since it is one range, the actual break down will look as follows:
*        - Wed-Mon (day 3 till 8)
*        - Sun-Mon (day 0 till 1)
*       The first range, which is too big, allows us to recognize the whole range,
*       thus we are not issuing a state change at 0 a.m. on Sunday but 0 a.m on
*       Tuesday.
*
*    - An other special case is a calendar, which disables a queue for a whole week.
*      There is special code to recognize that.
*
*  INPUTS
*     bool today                  - does the state change happen today?
*     bool active                 - is a calendar entry active?
*     const lList *year_time      - year_time structure or NULL, if week_time is set
*     const lList *week_time      - week_time strucutre or NULL, if year_time is set
*     const lList *day_time       - day_time changes, if the exist
*     const lListElem *now        - a now cull representation
*     bool *is_end_of_day_reached - (out) is the end of day reached? 
*
*  RESULT
*     static time_t - time of the next change or end of the day
*
*  NOTES
*     MT-NOTE: compute_limit() is MT safe 
*
*******************************************************************************/
static time_t compute_limit(bool today, bool active, const lList *year_time, const lList *week_time, 
                            const lList *day_time, const lListElem *now, bool *is_end_of_day_reached) {

   lListElem *lep = NULL;
   bool end_of_day = false;
   struct tm tm_limit;
   time_t limit;
   bool is_full_day = false;
   bool is_new_cal_entry = true;
   lListElem *time;   
   lListElem *new_now = (lListElem *) now; /* this is a dirty hack to get rid of the const. new_now will not be modified, 
                                              but might be overriden with a temp object, which needs to be freed within 
                                              the function. But a const lListElem cannot be freed. */
   bool is_new_now_copy = false;

   DENTER(TOP_LAYER, "compute_limit");

   if (day_time == NULL) {
      DPRINTF(("no day time calendar is set "));
      DRETURN(0);
   }

   if (year_time != NULL && week_time != NULL) {
      DPRINTF(("year and week calendar are set. It should only be one of them"));
      DRETURN(0);
   }

   if (year_time == NULL && week_time == NULL) {
      DPRINTF(("we have no a calendar time definition. It is most likely set to \"off\" or \"suspended\""));
      DRETURN(0);
   }

   time = lFirst(day_time);

   { /* should only be one entry in case that the calendar is valid for whole days*/
      lListElem *end = lFirst(lGetList(time, TMR_end));
      lListElem *begin = lFirst(lGetList(time, TMR_begin));
      
      if ( (lGetUlong(end, TM_sec) == 0)   &&
           (lGetUlong(end, TM_min) == 0)   &&
           (lGetUlong(end, TM_hour) == 24) &&
           (lGetUlong(begin, TM_sec) == 0) &&
           (lGetUlong(begin, TM_min) == 0) &&
           (lGetUlong(begin, TM_hour) == 0)) {
         is_full_day = true;     
      }     
   }

   /* do we look at a full day calendar or is the next entry not today? We then have to figure out the
      exect next date, when the calendar will be active /inactive again */
   if (is_full_day || !today) {
      if (active) { /* a calendar is active */
         if (year_time != NULL) {
            for_each(time, year_time) {
               if (in_range(now, time, tm_yday_cmp)) {
                  lep = lCopyElem(lFirst(lGetList(time, TMR_end))); 
                  if (lep == NULL) {
                      lep = lCopyElem(lFirst(lGetList(time, TMR_begin))); 
                  }
                  break;
               }
            }
         }
         else if (week_time != NULL) {
            bool is_allways_inactive = true;
            
            for_each(time, week_time) {
               lList *endList = lGetList(time, TMR_end);
               lList *beginList = lGetList(time, TMR_begin);
               if ( is_full_day &&
                    (endList != NULL) && (beginList != NULL) &&
                    (lGetUlong(lFirst(endList), TM_wday) - lGetUlong(lFirst(beginList), TM_wday) == 6)
                   ) {
                   is_allways_inactive &= true; /* the calendar is disabled for the whole week. */
               }
               else {
                  is_allways_inactive &= false;
               }
            
               if (in_range(now, time, tm_wday_cmp)) {
                  lListElem *end = lFirst(endList);
                  u_long32 day;
                  
                  if (end == NULL) { /* we might only have one day specified. If so, the beginList contains the end */
                     end = lFirst(beginList);
                  }
                  
                  day = lGetUlong(now, TM_mday);
                  
                  day += (lGetUlong(end, TM_wday) - lGetUlong(now, TM_wday));
                  lep = lCopyElem(now);
                  lSetUlong(lep, TM_mday, day);
                  break;
               }
            }
            if (is_allways_inactive) { /* the calendar is disabled for the whole week. No need to compute anything further */
               *is_end_of_day_reached = false;
               lFreeElem(&lep);
               DRETURN(0);
            }
         }
         else {
            DPRINTF(("year_time and week_time calendar are set, this should not be. "));
         }
         lSetUlong(lep, TM_sec, 59); /* we cannot have 24:0:0. That would be equal to 0:0:0 and therefor be the */
         lSetUlong(lep, TM_min, 59); /* beginning of the day and not its end */
         lSetUlong(lep, TM_hour, 23);
         end_of_day=true;
      }
      else { /* a calendar is inactive */
         lListElem *begin = NULL;
         if (year_time != NULL) {   /* year calenar */
            for_each(time, year_time) {
               begin = lFirst(lGetList(time, TMR_begin));
               if (tm_yday_cmp(now, begin) < 0 && 
                  (!lep || tm_yday_cmp(lep, begin) > 0)) {  
                  lFreeElem(&lep);
                  lep = lCopyElem(begin);           
               }   
            }
            if (lep == NULL) {
               is_new_cal_entry = false;
            }
         }
         else if (week_time != NULL) { /* week calendar */
            bool is_next_week = false;
            for_each(time, week_time) {
               begin = lFirst(lGetList(time, TMR_begin));
               if (tm_wday_cmp(now, begin) < 0 && 
                  (!lep || tm_wday_cmp(lep, begin) > 0)) {     
                  lFreeElem(&lep);
                  lep = lCopyElem(begin);  
               }
            }   
            if (lep == NULL) {
               is_next_week = true;
               for_each(time, week_time) {
                  begin = lFirst(lGetList(time, TMR_begin));
                  if (!lep || tm_wday_cmp(lep, begin) > 0) {     
                     lFreeElem(&lep);
                     lep = lCopyElem(begin);  
                  }
               }              
            }
            if (is_next_week) {
               int days = lGetUlong(now, TM_mday);
               days += (7 - lGetUlong(now, TM_wday)) + lGetUlong(lep, TM_wday);
               lSetUlong(lep, TM_mday, days);
            }
            else {
               lSetUlong(lep, TM_mday, lGetUlong(now, TM_mday) + (lGetUlong(lep, TM_wday) - lGetUlong(now, TM_wday)));
            }
            lSetUlong(lep, TM_mon, lGetUlong(now, TM_mon));
            lSetUlong(lep, TM_year, lGetUlong(now, TM_year));
         }
         else {      /* error in the data structure. should not happen */
            DPRINTF(("year_time and week_time calendar are not set"));
         }
      }

      if (lep != NULL) { /* we do not know about summer or winter time, let the system choose */
         lSetUlong(lep, TM_isdst, -1);
      }

      if (is_new_cal_entry) {
         uncullify_tm(lep, &tm_limit);
         if (is_full_day) {/* a full day calendar, nothing else is needed. */
            lFreeElem(&lep);
         }
         else {
            today = true; /* we have to enable the calendar for today */
            new_now = lep; /* set the next calendar state change as today */
            lep = NULL;
            is_new_now_copy = true; /* new_now needs to be freed later */
         }
         
      }
   }

   /* the current calendar changes the state during the day, we have to compute the exact time, when
      that happens */
   if (!is_full_day && new_now != NULL && today) {
      
      if (active) {
         /* inside of range - seek nearest end of range */
         /* seek end of active daytime range */ 
         for_each (time, day_time) {
            if (in_range(new_now, time, tm_daytime_cmp)) {
               lep = lCopyElem(lFirst(lGetList(time, TMR_end))); 
               break;
            }
         }
      } else {
         lListElem *begin;
         /* outside of range - seek nearest begin of range */
         for_each (time, day_time) {
            begin = lFirst(lGetList(time, TMR_begin));
            if (year_time != NULL) {
               if (tm_daytime_cmp(new_now, begin) < 0 && 
                  (!lep || tm_daytime_cmp(lep, begin) > 0)) {
                  lFreeElem(&lep);
                  lep = lCopyElem(begin); 
               }
            } 
            else { /* a day starts at 0:0:0 o'clock, therefore we have to check,
                      if we hit the start date right away or if we are smaller */
               if (tm_daytime_cmp(new_now, begin) <= 0 && 
                  (!lep || tm_daytime_cmp(lep, begin) > 0)) {
                  lFreeElem(&lep);
                  lep = lCopyElem(begin); 
               }
            }
         }
      }

      
      /* it might happen, that we are already to late for todays state shift, though we assume
         a state shift at the end of the day and record it. The function will be called again,
         to compute the state shift for the "next" day */
      if (!lep) {
         DPRINTF(("reached end of the day\n"));  
         *is_end_of_day_reached=true;
         lep = lCopyElem(new_now);
         lSetUlong(lep, TM_hour, 24);
         lSetUlong(lep, TM_min, 0);
         lSetUlong(lep, TM_sec, 0);
      }

      /* convert from cull struct tm into time_t */
      end_of_day = (lGetUlong(lep, TM_hour)==24) ? true : false;
      if (end_of_day) {
         lSetUlong(lep, TM_hour, 23);
         lSetUlong(lep, TM_min, 59);
         lSetUlong(lep, TM_sec, 59);
      }
      uncullify_tm(new_now, &tm_limit);
      tm_limit.tm_hour = lGetUlong(lep, TM_hour);
      tm_limit.tm_min = lGetUlong(lep, TM_min);
      tm_limit.tm_sec = lGetUlong(lep, TM_sec);
      lFreeElem(&lep);
      is_new_cal_entry = true;
   }

   if (is_new_now_copy) {
       lFreeElem(&new_now);
   }         

   if (is_new_cal_entry) {
#if 0
      DPRINTF(("sec:%d min:%d hour:%d mday:%d mon:%d year:%d wday:%d yday:%d isdst:%d\n",
         tm_limit.tm_sec,
         tm_limit.tm_min,
         tm_limit.tm_hour,
         tm_limit.tm_mday,
         tm_limit.tm_mon,
         tm_limit.tm_year,
         tm_limit.tm_wday,
         tm_limit.tm_yday,
         tm_limit.tm_isdst)); 
#endif
      limit = mktime(&tm_limit);

      DPRINTF(("limit: "sge_u32"\n", (u_long32) limit)); 
      if (end_of_day) {
         limit += 1;
      }
   }
   else { /* the current state will last for ever */
      DPRINTF(("no new calendar entry\n"));
      limit = 0;
   }

   DRETURN(limit);

}

static int normalize_range_list(lList *rl, cmp_func_t cmp_func) {
   lListElem *r1, *r2, *r;
   lListElem *q1, *q2, *q;
/*    lListElem *t1, *t2, *t3, *t4; */
/*    int i1, i2, i3, i4; */

   DENTER(TOP_LAYER, "normalize_range_list");

/*    i1 = i2 = i3 = i4 = -1; */

   for_each(r, rl) {

      r1 = lFirst(lGetList(r, TMR_begin));
      r2 = lFirst(lGetList(r, TMR_end));

      for (q = lNext(r); q; q = lNext(q)) {

         q1 = lFirst(lGetList(q, TMR_begin));
         q2 = lFirst(lGetList(q, TMR_end));

         /* overlapping ranges ? */
         if (in_range(r1, q, cmp_func)         || /* r1 in q */
             (r2 && in_range(r2, q, cmp_func)) || /* r2 in q */
             in_range(q1, r, cmp_func)         || /* q1 in r */
             (q2 && in_range(q2, r, cmp_func)))   /* q2 in r */ {

/*             DPRINTF(("%d %d %d %d\n", i1,i2,i3,i4)); */
/*             t1=lFirst(lGetList(r, TMR_begin)); */
/*             t2=lFirst(lGetList(r, TMR_end)); */
/*             t3=lFirst(lGetList(q, TMR_begin)); */
/*             t4=lFirst(lGetList(q, TMR_end)); */

/*            DPRINTF(("overlapping ranges %d:%d:%d-%d:%d:%d and %d:%d:%d-%d:%d:%d\n",
               lGetUlong(t1, TM_hour),
               lGetUlong(t1, TM_min),
               lGetUlong(t1, TM_sec),

               lGetUlong(t2, TM_hour),
               lGetUlong(t2, TM_min),
               lGetUlong(t2, TM_sec),

               lGetUlong(t3, TM_hour),
               lGetUlong(t3, TM_min),
               lGetUlong(t3, TM_sec),

               lGetUlong(t4, TM_hour),
               lGetUlong(t4, TM_min),
               lGetUlong(t4, TM_sec)));*/

            /* combine both overlapping ranges */
            /* r1 = min(r1, q1) */
            if (cmp_func(r1, q1) > 0) {
               lSwapList(r, TMR_begin, q, TMR_begin);
            }   

            /* r2 = max(r2, q2) */
            if ( (q2 && !r2) || cmp_func(r2, q2)<0) {
               lSwapList(r, TMR_end, q, TMR_end);
            }   
            
/*             t1=lFirst(lGetList(r, TMR_begin)); */
/*             t2=lFirst(lGetList(r, TMR_end)); */

/*            DPRINTF(("resulting range %d:%d:%d-%d:%d:%d\n",
               lGetUlong(t1, TM_hour),
               lGetUlong(t1, TM_min),
               lGetUlong(t1, TM_sec),

               lGetUlong(t2, TM_hour),
               lGetUlong(t2, TM_min),
               lGetUlong(t2, TM_sec))); */

            /* remove q */
            lRemoveElem(rl, &q);

            /* prepare next loop iteration */
            q = r;

            /* r1, r2 are no longer valid after swapping */
            r1 = lFirst(lGetList(r, TMR_begin));
            r2 = lFirst(lGetList(r, TMR_end));
         } 
      }
   }

   DRETURN(0);
}

/*
lListElem *tm, TM_Type 
*/
static bool in_range_list(lListElem *tm, lList *rl, cmp_func_t cmp_func) {
   lListElem *r;

   DENTER(TOP_LAYER, "in_range_list");

   if (!rl) {
      DRETURN(true);
   }

   for_each(r, rl) {
      if (in_range(tm, r, cmp_func)) {
         DRETURN(true);
      }
   }

   DRETURN(false);
}

/*
lListElem *tm, TM_Type
*/
static int in_range(const lListElem *tm, const lListElem *r, cmp_func_t cmp_func) {
   lListElem *t1, *t2;

   DENTER(TOP_LAYER, "in_range");

   t1 = lFirst(lGetList(r, TMR_begin));
   t2 = lFirst(lGetList(r, TMR_end));

   if (t2) {
      if (cmp_func(t1, tm)<=0 && cmp_func(tm, t2) <= 0) { /* mon-fri */
#if 0
      DPRINTF(("%d:%d:%d is in range %d:%d:%d-%d:%d:%d\n",
         lGetUlong(tm, TM_hour),
         lGetUlong(tm, TM_min),
         lGetUlong(t1, TM_sec),

         lGetUlong(t1, TM_hour),
         lGetUlong(t1, TM_min),
         lGetUlong(t1, TM_sec),

         lGetUlong(t2, TM_hour),
         lGetUlong(t2, TM_min),
         lGetUlong(t2, TM_sec)));
#endif
         DRETURN(1);
      }

   } else {
      if (!cmp_func(t1, tm)) { /* must be equal for hit */
         DRETURN(1);
      }
   }

   DRETURN(0);
}

/* disabled_year_list := disabled_year_entry[<space>disabled_year_entry] */
static int disabled_year_list(lList **alpp, const char *s, lList **cal, const char *cal_name) { 
   lListElem *calep;

   static token_set_t token_set[] = {
      { DOT,             "." }, 
      { COLON,           ":" }, 
      { EQUAL_SIGN,      "=" },
      { MINUS,           "-" },
      { COMMA,           "," },
      { SPACE,           " " },
      { STRING,          NULL },
      { NUMBER,          NULL },
      { 0,               NULL }
   };

   DENTER(TOP_LAYER, "disabled_year_list");

   if (cal != NULL) {
      *cal = NULL;
   }   

   if (!s || !strcasecmp(s, "none")) {
      DRETURN(0);
   }

   scan(s, token_set);

   if (disabled_year_entry(cal?&calep:NULL))
      goto ERROR;

   if (cal != NULL) {
      *cal = lCreateList("year list", CA_Type);
      lAppendElem(*cal, calep);
   }
   while (scan(NULL, NULL) == SPACE) {
      eat_token();

      while (scan(NULL, NULL) == SPACE) {
         eat_token();
      }
      
      if (disabled_year_entry(cal?&calep:NULL)) {
         goto ERROR;
      }   
      if (cal != NULL) {
         lAppendElem(*cal, calep); 
      }
   }  

   /* complain about still unused tokens */
   if (scan(NULL, NULL)!=NO_TOKEN) {
      snprintf(parse_error, sizeof(parse_error),
               MSG_TOKEN_UNRECOGNIZEDSTRING_S, get_string());
      goto ERROR;
   }

   DRETURN(0);

ERROR:
   lFreeList(cal);
   SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_ANSWER_ERRORINDISABLYEAROFCALENDARXY_SS, 
         save_error(), cal_name));
   answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
   DRETURN(-1);
}

/* disabled_year_entry := year_day_range_list[=daytime_range_list][=state] */
static int disabled_year_entry(lListElem **cal) {
   lList *ydrl = NULL,
         *dtrl = NULL;
   int state = QI_DO_DISABLE;
  
   DENTER(TOP_LAYER, "disabled_year_entry");

   if (scan(NULL, NULL)==NUMBER) {
      if (year_day_range_list(&ydrl))
         goto ERROR;
     
      if (scan(NULL, NULL)!=EQUAL_SIGN)
         goto SUCCESS;
      eat_token();
   }

   if (scan(NULL, NULL)==NUMBER) {
      if (daytime_range_list(&dtrl))
         goto ERROR;
      normalize_range_list(dtrl, tm_daytime_cmp);
      if (scan(NULL, NULL)!=EQUAL_SIGN)
         goto SUCCESS;
      eat_token();
   }
   
   if (scan(NULL, NULL)==STRING) {
      if (action(&state)) {
         DRETURN(-1);
      }
   } else {
      snprintf(parse_error, sizeof(parse_error), SFNMAX,
               MSG_ANSWER_GOTEQUALWITHOUTDAYTIMERANGEORSTATE );
      goto ERROR;
   }

SUCCESS:
   if (cal) {
      *cal = lCreateElem(CA_Type);
      lSetList(*cal, CA_yday_range_list, ydrl);
      if (!dtrl)
         full_daytime_range(&dtrl);
      lSetList(*cal, CA_daytime_range_list, dtrl);
      DPRINTF(("state = %d\n", state));
      lSetUlong(*cal, CA_state, state);
   }

   DRETURN(0);

ERROR:
   lFreeList(&ydrl);
   lFreeList(&dtrl);
   DRETURN(-1);
}

static void full_daytime_range(lList **dtrl) {
   lListElem *tmr;

   DENTER(TOP_LAYER, "full_daytime_range");

   if (!*dtrl)
      *dtrl = lCreateList("full day", TMR_Type);

/*    DPRINTF(("FULL_DAYTIME_RANGE\n")); */

   tmr = lCreateElem(TMR_Type);
   lAddSubUlong(tmr, TM_hour, 0,  TMR_begin, TM_Type);
   lAddSubUlong(tmr, TM_hour, 24, TMR_end,   TM_Type);

   lAppendElem(*dtrl, tmr);   

   DRETURN_VOID;
}

static void full_weekday_range(lList **dtrl) {
   lListElem *tmr;

   DENTER(TOP_LAYER, "full_weekday_range");

   if (!*dtrl)
      *dtrl = lCreateList("full week", TMR_Type);

/*    DPRINTF(("FULL_WEEKDAY_RANGE\n")); */

   tmr = lCreateElem(TMR_Type);
   lAddSubUlong(tmr, TM_wday, 0,  TMR_begin, TM_Type);
   lAddSubUlong(tmr, TM_wday, 6, TMR_end,   TM_Type);

   lAppendElem(*dtrl, tmr);   

   DRETURN_VOID;
}


/* year_day_range := year_day_range[,year_day_range] */
static int year_day_range_list(lList **ydrl) { 
   lListElem *tmr;

   DENTER(TOP_LAYER, "year_day_range_list");

   if (year_day_range(&tmr)) {
      DRETURN(-1);
   }
   if (ydrl != NULL) {
      *ydrl = lCreateList("year_day_range_list", TMR_Type);
      lAppendElem(*ydrl, tmr);
   }

   while (scan(NULL, NULL)==COMMA) {
      eat_token();
      if (year_day_range(&tmr)) {
         if (ydrl != NULL) {
            lFreeList(ydrl);
         }
         DRETURN(-1);
      }
      if (ydrl != NULL) {
         lAppendElem(*ydrl, tmr);
      }
   }

   DRETURN(0);
}

static int year_day_range(lListElem **tmr) { 
   lListElem *t1, *t2 = NULL;
 
   DENTER(TOP_LAYER, "year_day_range");

   if (year_day(&t1)) {
      DRETURN(-1);
   }

   if (scan(NULL, NULL)==MINUS) {   
      eat_token();

      if (year_day(&t2)) {
         lFreeElem(&t1);
         DRETURN(-1);
      }   
      if (tm_yday_cmp(t1, t2)>0) {
         snprintf(parse_error, sizeof(parse_error), SFNMAX,
                  MSG_ANSWER_FIRSTYESTERDAYINRANGEMUSTBEBEFORESECONDYESTERDAY);
         lFreeElem(&t1);
         DRETURN(-1);   
      }
   }

   if (tmr) {
      lList *tmlp;
      *tmr = lCreateElem(TMR_Type);

      tmlp = lCreateList("tm_list", TM_Type);
      lAppendElem(tmlp, t1);
      t1 = NULL;
      lSetList(*tmr, TMR_begin, tmlp);

      if (t2) {
         tmlp = lCreateList("tm_list", TM_Type);
         lAppendElem(tmlp, t2);
         t2 = NULL;
         lSetList(*tmr, TMR_end, tmlp);
      }
   }

   lFreeElem(&t1);
   lFreeElem(&t2);


   DRETURN(0);
}

static int year_day(lListElem **tm) { 
   int y, m, d;

   DENTER(TOP_LAYER, "year_day");

   if (day(&d)) {
      DRETURN(-1);
   }

   if (scan(NULL, NULL)!=DOT) {
      snprintf(parse_error, sizeof(parse_error), SFNMAX,
               MSG_PARSE_MISSINGPOINTAFTERDAY);
      DRETURN(-1);
   }
   eat_token();

   if (month(&m)) {
      DRETURN(-1);
   }

   if (scan(NULL, NULL)!=DOT) {
      snprintf(parse_error, sizeof(parse_error), SFNMAX,
               MSG_PARSE_MISSINGPOINTAFTERMONTH );
      DRETURN(-1);
   }
   eat_token();

   if (year(&y)) {
      DRETURN(-1);
   }

   if (tm) {
      *tm = lCreateElem(TM_Type);
      lSetUlong(*tm, TM_year, y - 1900);
      lSetUlong(*tm, TM_mon, m);
      lSetUlong(*tm, TM_mday, d);
   }

   DRETURN(0);
}

static int range_number(int min, int max, int *ip, const char *name) {
   
   DENTER(TOP_LAYER, "range_number");

   if (scan(NULL, NULL)==NUMBER) {
      int this_number = get_number();
      eat_token();

      if (this_number > max || this_number < min) {
         snprintf(parse_error, sizeof(parse_error),
                  MSG_PARSE_WOUTSIDEOFRANGEXYZ_SIIS, get_string(),
                  min, max, name);
         DRETURN(-1);
      } else {
         if (ip)
            *ip = this_number; 
         DRETURN(0);
      }
   }

   snprintf(parse_error, sizeof(parse_error), MSG_PARSE_XISNOTAY_SS,
            get_string(), name);
   DRETURN(-1);
}


static int month(int *mp) {
   int m;

   static token_set_t monthv[] = {
      { 1,  "january" },
      { 2,  "february" },
      { 3,  "march" },
      { 4,  "april" },
      { 5,  "may" },
      { 6,  "june" },
      { 7,  "july" },
      { 8,  "august" },
      { 9,  "september" },
      { 10, "october" },
      { 11, "november" },
      { 12, "december" },
      { -1, NULL }
   };

   DENTER(TOP_LAYER, "month");

   if (scan(NULL, NULL)==STRING) {
      char *s = get_string();
      if ((m = cheap_scan(s, monthv, 3))<0) {
         DRETURN(-1);
      }
      eat_token();
   } else {
      if (range_number(1, 12, &m, "month")<0) {
         DRETURN(-1);
      }
   }
   *mp = m - 1;

   DRETURN(0);
}

static int day(int *dp)
{
   return range_number(1, 31, dp, "day");
}

static int year(int *yp) { 
   return range_number(1970, 2037, yp, "year"); 
}

/* daytime_range_list   := daytime_range[,daytime_range] ... */
static int daytime_range_list(lList **dtrl) { 
   lListElem *tmr;

   DENTER(TOP_LAYER, "daytime_range_list");

   if (daytime_range(&tmr)) {
      DRETURN(-1);
   }
   if (dtrl) {
      *dtrl = lCreateList("daytime_range_list", TMR_Type);
      lAppendElem(*dtrl, tmr);
      split_daytime_range(*dtrl, tmr);   
   }

   while (scan(NULL, NULL)==COMMA) {
      eat_token();
      if (daytime_range(&tmr)) {
         if (dtrl != NULL) {
            lFreeList(dtrl);
         }
         DRETURN(-1);
      }
      if (dtrl != NULL) {
         lAppendElem(*dtrl, tmr);
         split_daytime_range(*dtrl, tmr);   
      }
   }

   DRETURN(0);
}

static void split_daytime_range(lList *dtrl, lListElem *tmr) {
   lListElem *t2, *t1, *t3, *t4, *tmr2;

   DENTER(TOP_LAYER, "split_daytime_range");

   if ((t2=lFirst(lGetList(tmr, TMR_end)))) {
      t1=lFirst(lGetList(tmr, TMR_begin));

      if (tm_daytime_cmp(t1, t2)>0) {
         /* split it into two ranges */
         tmr2 = lCreateElem(TMR_Type);
         lAddSubUlong(tmr2, TM_hour, 0, TMR_begin, TM_Type);
         lAddSubUlong(tmr2, TM_hour, 24, TMR_end, TM_Type);
         lSwapList(tmr, TMR_end, tmr2, TMR_end);
         lAppendElem(dtrl, tmr2);
      
         t1=lFirst(lGetList(tmr, TMR_begin));
         t2=lFirst(lGetList(tmr, TMR_end));
         t3=lFirst(lGetList(tmr2, TMR_begin));
         t4=lFirst(lGetList(tmr2, TMR_end));

         DPRINTF(("split %d:%d:%d-%d:%d:%d into %d:%d:%d-%d:%d:%d and %d:%d:%d-%d:%d:%d\n",
            (int)lGetUlong(t1, TM_hour),
            (int)lGetUlong(t1, TM_min),
            (int)lGetUlong(t1, TM_sec),

            (int)lGetUlong(t4, TM_hour),
            (int)lGetUlong(t4, TM_min),
            (int)lGetUlong(t4, TM_sec),

            (int)lGetUlong(t1, TM_hour),
            (int)lGetUlong(t1, TM_min),
            (int)lGetUlong(t1, TM_sec),

            (int)lGetUlong(t2, TM_hour),
            (int)lGetUlong(t2, TM_min),
            (int)lGetUlong(t2, TM_sec),

            (int)lGetUlong(t3, TM_hour),
            (int)lGetUlong(t3, TM_min),
            (int)lGetUlong(t3, TM_sec),

            (int)lGetUlong(t4, TM_hour),
            (int)lGetUlong(t4, TM_min),
            (int)lGetUlong(t4, TM_sec)));
      }
   }

   DRETURN_VOID;
}

/* daytime_range  := daytime-daytime */
static int daytime_range(lListElem **tmr) {
   lListElem *t1 = NULL, *t2 = NULL;
  
   DENTER(TOP_LAYER, "daytime_range");

   if (daytime(&t1)) {
      goto ERROR;
   }
   if (scan(NULL, NULL)!=MINUS) {
      snprintf(parse_error, sizeof(parse_error), SFNMAX,
               MSG_PARSE_MISSINGDASHINDAYTIMERANGE);
      goto ERROR;
   }
   eat_token();
   if (daytime(&t2)) {
      goto ERROR;
   }
   if (!tm_daytime_cmp(t1, t2)) {
      snprintf(parse_error, sizeof(parse_error), SFNMAX,
               MSG_PARSE_RANGEBEGISEQUALTOEND);
      goto ERROR;
   }

   if (tmr) {
      lList *tmlp;

      *tmr = lCreateElem(TMR_Type);

      tmlp = lCreateList("tm_list", TM_Type);
      lAppendElem(tmlp, t1);
      t1 = NULL;
      lSetList(*tmr, TMR_begin, tmlp);

      tmlp = lCreateList("tm_list", TM_Type);
      lAppendElem(tmlp, t2);
      t2 = NULL;
      lSetList(*tmr, TMR_end, tmlp);
   }

   lFreeElem(&t1);
   lFreeElem(&t2);

   DRETURN(0);

ERROR:
   lFreeElem(&t1);
   lFreeElem(&t2);
   DRETURN(-1);
}


/* daytime := hour[:minute][:second] */
static int daytime(lListElem **tm) { 
   int h, m = 0, s = 0;

   DENTER(TOP_LAYER, "daytime");

   if (hour(&h)) {
      DRETURN(-1);
   } 

   if (scan(NULL, NULL)!=COLON) {
      goto SUCCESS;
   }
   eat_token();
   if (minute(&m)) {
      DRETURN(-1);
   }

   if (scan(NULL, NULL)!=COLON) {
      goto SUCCESS;
   }
   eat_token();
   if (seconds(&s)) {
      DRETURN(-1);
   }

SUCCESS:
   if (h==24) {
      if (m || s) {
         snprintf(parse_error, sizeof(parse_error), SFNMAX,
                  MSG_PARSE_DAYTIMESBEYOND24HNOTALLOWED);
         DRETURN(-1);
      }
   }

   if (tm) {
      *tm = lCreateElem(TM_Type);
      lSetUlong(*tm, TM_hour, h);
      lSetUlong(*tm, TM_min, m);
      lSetUlong(*tm, TM_sec, s);
   }

   DRETURN(0);
}

static int hour(int *hp) { 
   return range_number(0, 24, hp, MSG_PARSE_HOURSPEC); 
}

static int minute(int *mp) { 
   return range_number(0, 59, mp, MSG_PARSE_MINUTESPEC); 
}

static int seconds(int *sp) {
   return range_number(0, 59, sp, MSG_PARSE_SECONDSSPEC); 
}

static int action(int *sp) {
   int state;
   char *s;
   DENTER(TOP_LAYER, "action");

   if (scan(NULL, NULL)!=STRING) {
      snprintf(parse_error, sizeof(parse_error),
               MSG_PARSE_XISNOTASTATESPECIFIER_S, get_string());
      DRETURN(-1);
   } 

   s = get_string();
   if ((state = cheap_scan(s, statev, 3))<0) {
      snprintf(parse_error, sizeof(parse_error),
               MSG_PARSE_XISNOTASTATESPECIFIER_S, s);
      DRETURN(-1);
   }
   eat_token();
   *sp = state;

   DRETURN(0);
}

static int disabled_week_list(lList **alpp, const char *s, lList **cal, const char *cal_name) {
   lListElem *calep;
   static token_set_t token_set[] = {
      { COLON,        ":" }, 
      { EQUAL_SIGN,   "=" },
      { MINUS,        "-" },
      { COMMA,        "," },
      { SPACE,        " " },
      { STRING,       NULL },
      { NUMBER,       NULL },
      { 0,            NULL }
   };
   
   DENTER(TOP_LAYER, "disabled_week_list");

   if (cal) {
      *cal = NULL;
   }   

   if (!s || !strcasecmp(s, "none")) {
      DRETURN(0);
   }

   scan(s, token_set);

   if (disabled_week_entry(cal?&calep:NULL)) {
      goto ERROR;
   }   

   if (cal) {
      *cal = lCreateList("week list", CA_Type);
      lAppendElem(*cal, calep);
   }

   /* if we have a space, there is at least one more calender spec. */
   while (scan(NULL, NULL) == SPACE) {
      eat_token();

      /* there can be multiple spaces between calender def. eat them all */
      while (scan(NULL, NULL) ==SPACE) {
         eat_token();
      }   

      if (disabled_week_entry(cal?&calep:NULL)) {
         goto ERROR;
      }
      if (cal) {
         lAppendElem(*cal, calep);
      }   
   }  

   /* complain about still unused tokens */
   if (scan(NULL, NULL)!=NO_TOKEN) {
      snprintf(parse_error, sizeof(parse_error), SFNMAX,
               MSG_PARSE_UNRECOGNIZEDTOKENATEND);
      goto ERROR;
   }

   DRETURN(0);

ERROR:
   lFreeList(cal);
   SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_PARSE_ERRORINDISABLEDWEEKOFCALENDAR_SS, 
        cal_name, save_error()));
   answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
   DRETURN(-1);
}

/* disabled_week_entry := week_day_range_list[=daytime_range_list][=state] */
static int disabled_week_entry(lListElem **cal) {
   /* default values */
   lList *wdrl = NULL,
         *dtrl = NULL;
       int state = QI_DO_DISABLE;

   DENTER(TOP_LAYER, "disabled_week_entry");

   if (scan(NULL, NULL)==STRING &&
       cheap_scan(get_string(), statev, 3)<0) {

      if (week_day_range_list(&wdrl)) {
         goto ERROR;
      }   
     
      if (scan(NULL, NULL)!=EQUAL_SIGN) {
         goto SUCCESS;
      }   
      eat_token();
   }

   if (scan(NULL, NULL)==NUMBER) {
      if (daytime_range_list(&dtrl)) {
         goto ERROR;
      }   
      normalize_range_list(dtrl, tm_daytime_cmp);
      if (scan(NULL, NULL)!=EQUAL_SIGN) {
         goto SUCCESS;
      }   
      eat_token();
   }

   if (scan(NULL, NULL)==STRING) {
      state = 0;
      if (action(&state)) {
         goto ERROR;
      }   
   } else {
      snprintf(parse_error, sizeof(parse_error), SFNMAX,
               MSG_ANSWER_GOTEQUALWITHOUTDAYTIMERANGEORSTATE);
      goto ERROR;
   }

SUCCESS:
   if (cal) {
      *cal = lCreateElem(CA_Type);
      
      if (!wdrl) {
         full_weekday_range(&wdrl);
      }   
      
      lSetList(*cal, CA_wday_range_list, wdrl);
      
      if (!dtrl) {
         full_daytime_range(&dtrl);
      }   
      lSetList(*cal, CA_daytime_range_list, dtrl);
      lSetUlong(*cal, CA_state, state);
   }

   DRETURN(0);

ERROR:
   lFreeList(&wdrl);
   lFreeList(&dtrl);
   DRETURN(-1);
}

/* week_day_range := week_day_range[,week_day_range] */
static int week_day_range_list(lList **wdrl) { 
   lListElem *tmr;

   DENTER(TOP_LAYER, "week_day_range_list");
   if (week_day_range(&tmr)) {
      DRETURN(-1);
   }

   if (wdrl) {
      *wdrl = lCreateList("week_day_range_list", TMR_Type);
      lAppendElem(*wdrl, tmr);
      split_wday_range(*wdrl, tmr);
   }

   

   while (scan(NULL, NULL)==COMMA) {
      eat_token();
      if (week_day_range(&tmr)) {
         lFreeList(wdrl);
         DRETURN(-1);
      }
      if (wdrl) {
         lAppendElem(*wdrl, tmr);
         split_wday_range(*wdrl, tmr);
      }
   }  

   if (!wdrl) {
      DRETURN(-1);
   }
   /* prepare the structures for later use */
   join_wday_range(*wdrl); 
   extend_wday_range(*wdrl);

   DRETURN(0);
}

/****** sge_calendar/join_wday_range() *****************************************
*  NAME
*     join_wday_range() -- join overlapping ranges
*
*  SYNOPSIS
*     static void join_wday_range(lList *week_day) 
*
*  FUNCTION
*     A user can define overlapping ranges in the week calendar. This function
*     will join them into one definition.
*
*  INPUTS
*     lList *week_day - week calendar def. list
*
*  NOTES
*     MT-NOTE: join_wday_range() is MT safe 
*
*******************************************************************************/
static void join_wday_range(lList *week_day) 
{
   lListElem *day_range1;
   lListElem *day_range2;
   lListElem *next1;
   lListElem *next2;

   next1 = lFirst(week_day);

   /* join ranges, which overlap or touch */
   while ((day_range1 = next1) != NULL) {
      u_long32 begin1;
      u_long32 end1;
      lList *beginList1;
      lList *endList1;     
      
      next1 = lNext(next1);

      beginList1 = lGetList(day_range1, TMR_begin);
      endList1 = lGetList(day_range1, TMR_end);

      begin1 = lGetUlong(lFirst(beginList1), TM_wday);
      if (endList1 != NULL) {
         end1 = lGetUlong(lFirst(endList1), TM_wday);
      }
      else {
         end1 = begin1;
      }
      next2 = lFirst(week_day);
      while((day_range2 = next2) != NULL) {
         u_long32 begin2;
         u_long32 end2;
         lList *beginList2;
         lList *endList2;
      
         next2 = lNext(next2);

         if (day_range2 == day_range1) {
            continue;
         }
   
         beginList2 = lGetList(day_range2, TMR_begin);
         endList2 = lGetList(day_range2, TMR_end);

         begin2 = lGetUlong(lFirst(beginList2), TM_wday);
         if (endList2 != NULL) {
            end2 = lGetUlong(lFirst(endList2), TM_wday);
         }
         else {
            end2 = begin2;
         }         
        
         if (begin2 <= (end1 +1) && (begin1 <= begin2)) {
            if (end1 < end2) {
               if (endList1) {
                  lSetUlong(lFirst(endList1), TM_wday, end2);
               }
               else {
                  lSetList(day_range1, TMR_end, lCopyList("", endList2));
               }
            }
            
            if (next1 == day_range2) {
               next1 = next2;
            }
            lRemoveElem(week_day, &day_range2);
            day_range2 = NULL;
         }
        
      }
   }
}

/****** sge_calendar/extend_wday_range() ***************************************
*  NAME
*     extend_wday_range() -- extend ranges over the week border....
*
*  SYNOPSIS
*     static void extend_wday_range(lList *week_day) 
*
*  FUNCTION
*     extend the range over the one week border. The week ends on Sunday, but the
*      range can go till Monday or longer. But the beginning till the end will never
*      contain more than 7 days (a week).  This is needed!!
*
*  INPUTS
*     lList *week_day - calendar list for a week
*
*  NOTES
*     MT-NOTE: extend_wday_range() is MT safe 
*
*******************************************************************************/
static void extend_wday_range(lList *week_day) 
{
   lListElem *day_range1;
   lListElem *day_range2;
   lListElem *next1;
   lListElem *next2;

   next1 = lFirst(week_day);
   while ((day_range1 = next1) != NULL) {
      u_long32 begin1;
      u_long32 end1;
      lList *beginList1;
      lList *endList1;     
      
      next1 = lNext(next1);

      beginList1 = lGetList(day_range1, TMR_begin);
      endList1 = lGetList(day_range1, TMR_end);

      begin1 = lGetUlong(lFirst(beginList1), TM_wday);
      if (endList1 != NULL) {
         end1 = lGetUlong(lFirst(endList1), TM_wday);
      }
      else {
         end1 = begin1;
      }

      next2 = lFirst(week_day);
      while((day_range2 = next2) != NULL) {
         u_long32 begin2;
         u_long32 end2;
         lList *beginList2;
         lList *endList2;
      
         next2 = lNext(next2);

         if (day_range2 == day_range1) {
            continue;
         }
   
         beginList2 = lGetList(day_range2, TMR_begin);
         endList2 = lGetList(day_range2, TMR_end);

         begin2 = lGetUlong(lFirst(beginList2), TM_wday);
         if (endList2 != NULL) {
            end2 = lGetUlong(lFirst(endList2), TM_wday);
         }
         else {
            end2 = begin2;
         }         
        
         if ((end1 == 6) && (begin2 == 0)) {
            int tempEnd = end1 + 1 + end2;
            if (endList1 == 0) {
               endList1 = lCopyList("",beginList1);
               lSetList(day_range1, TMR_end, endList1);
            }
            lSetUlong(lFirst(endList1), TM_wday, tempEnd);
         }
      }
   }


}
   

static void split_wday_range(lList *wdrl, lListElem *tmr) {
   lListElem *t2, *t1, /* *t3, *t4, */ *tmr2;

   DENTER(TOP_LAYER, "split_wday_range");

   if ((t2=lFirst(lGetList(tmr, TMR_end)))) {
      t1=lFirst(lGetList(tmr, TMR_begin));

      if (tm_wday_cmp(t1, t2) > 0) {
         /* split it into two ranges */
         tmr2 = lCreateElem(TMR_Type);
         lAddSubUlong(tmr2, TM_wday, 0, TMR_begin, TM_Type);
         lAddSubUlong(tmr2, TM_wday, 6, TMR_end, TM_Type);
         lSwapList(tmr, TMR_end, tmr2, TMR_end);
         lAppendElem(wdrl, tmr2);

/*          t3=lFirst(lGetList(tmr2, TMR_begin)); */
/*          t4=lFirst(lGetList(tmr2, TMR_end)); */

#if 0
         t1=lFirst(lGetList(tmr, TMR_begin));
         t2=lFirst(lGetList(tmr, TMR_end));
      DPRINTF(("splitted wday %d-%d into %d-%d and %d-%d\n",
         lGetUlong(t1, TM_wday),
         lGetUlong(t4, TM_wday),

         lGetUlong(t1, TM_wday),
         lGetUlong(t2, TM_wday),

         lGetUlong(t3, TM_wday),
         lGetUlong(t4, TM_wday)));
#endif
      }
   }

   DRETURN_VOID;
}

/* week_day_range  := week_day[-week_day] */
static int week_day_range(lListElem **tmr) {
   lListElem *t1 = NULL;
   lListElem *t2 = NULL;

   DENTER(TOP_LAYER, "week_day_range");

   if (week_day(&t1)) 
      goto ERROR;

   if (scan(NULL, NULL)==MINUS) {   
      eat_token();

      if (week_day(&t2))
         goto ERROR;

      if (tm_wday_cmp(t1, t2)==0) {
         snprintf(parse_error, sizeof(parse_error), SFNMAX,
                  MSG_PARSE_FOUNDUSELESSWEEKDAYRANGE);
         goto ERROR;
      }
   }


   if (tmr) {
      lList *tmlp;
      *tmr = lCreateElem(TMR_Type);

      tmlp = lCreateList("tm_list", TM_Type);
      lAppendElem(tmlp, t1);
      t1 = NULL;
      lSetList(*tmr, TMR_begin, tmlp);

      if (t2) {
         tmlp = lCreateList("tm_list", TM_Type);
         lAppendElem(tmlp, t2);
         t2 = NULL;
         lSetList(*tmr, TMR_end, tmlp);
      }
   }
   lFreeElem(&t1);
   lFreeElem(&t2);

   DRETURN(0);

ERROR:
   lFreeElem(&t1);
   lFreeElem(&t2);
   DRETURN(-1);
}

static int week_day(lListElem **tm) {
   int wday;
   char *s;

   static token_set_t weekdayv[] = {
      { 0, "sunday" },
      { 1, "monday" },
      { 2, "tuesday" },
      { 3, "wednesday" },
      { 4, "thursday" },
      { 5, "friday" },
      { 6, "saturday" },
      { -1, NULL }
   };

   DENTER(TOP_LAYER, "week_day");

   if (scan(NULL, NULL)!=STRING) {
      snprintf(parse_error, sizeof(parse_error), SFNMAX,
               MSG_PARSE_EXPECTEDSTRINGFORWEEKDAY);
      DRETURN(-1);
   }
   s = get_string();
   if ((wday = cheap_scan(s, weekdayv, 3))<0) {
      snprintf(parse_error, sizeof(parse_error),
               MSG_PARSE_XISNOTAWEEKDAY_S, s);
      DRETURN(-1);
   }
   eat_token();

   if (tm) {
      *tm = lCreateElem(TM_Type);
      lSetUlong(*tm, TM_wday, wday);
   }

   DRETURN(0);
}

static char *get_string() {
   return store;
}

static int get_number() {
   return number;
}
static char *save_error() {
   sge_strlcpy(old_error, parse_error, sizeof(old_error));
   return old_error;
}

/* ------------------------------------------------------------

   eat_token() informs parsing system that the current token
   is proceed

 */
static void eat_token() {
/*    DENTER(TOP_LAYER, "eat_token"); */
/*    DPRINTF(("token \"%s\"\n", store)); */
   token_is_valid = 0;
/*    DRETURN_VOID; */
}

static int scan(const char *s, token_set_t token_set[]) {
   static const char *t = NULL;
   static token_set_t *ts = NULL;
   static int token;

   int i, j, k, len = 0;
   int found;

   DENTER(CULL_LAYER, "scan");

   if (s) {                     /* initialize scan() with a new string to parse */
      t = s;
      token_is_valid = 0;
      old_error[0] = parse_error[0] = '\0';
   }
   if (token_set) {
      ts = token_set;
   }   

   if (token_is_valid) {        /* no need for a new token to parse */
      DRETURN(token);
   }

   if (t && !*t) {
      token_is_valid = 1;
      token = NO_TOKEN;
      DRETURN(token);
   }

   /* try all possible tokens */
   for (i = 0; ts[i].token; i++) {
      switch (ts[i].token) {

         case NUMBER: {
                     int old_number = 0;
                     /* parse number 0-9* and store it */
                     for (j=0; t[j] && isdigit((int) t[j]); j++) {
                        number =  old_number * 10 + (t[j]-'0');
                        if (number<old_number) {
                           ERROR((SGE_EVENT, SFNMAX, MSG_PARSE_OVERFLOWERRORWHILEPARSING));
                           token = ERR_TOKEN;
                           token_is_valid = 1;
                           DRETURN(token);
                        } else 
                           old_number = number;
                     }
                     if ((found = (j!=0))) {
                        len = j; 
                        strncpy(store, t, len);
                        store[len] = '\0';
                     }
                  }
            break;                   

         case STRING:   /* strings and shortcut strings */
                  /* parse string a-z,A-Z[a-z,A-Z,0-9]* and store it */
                  j = k = 0;
                  if (isalpha((int)t[j])) {
                     store[k++] = t[j];
                     for (j++; t[j] && isalnum((int)t[j]); j++) {
                        store[k++] = t[j];
                     }
                     store[k] = '\0';
                  }

                  len = j; 
                  if (ts[i].token == STRING) {
                     found = (j!=0);
                     break;
                  }
         
                  found = !strcasecmp(ts[i].text, store);
            break;
         default:
               len = strlen(ts[i].text);
               found = !strncasecmp(ts[i].text, t, len);
               strncpy(store, t, len);
               store[len] = '\0';
         break;
      }
      
      if (found) {
         t += len;
         token_is_valid = 1;
         DRETURN((token = ts[i].token));
      }
   }

   /* nothing found */
   token_is_valid = 1;
   token = ERR_TOKEN;
   DRETURN(token);
}

static int cheap_scan(char *s, token_set_t tokenv[], int n) {
   int i;
   int len;
   int match_all_chars = 0;

   DENTER(TOP_LAYER, "cheap_scan");

   if ((len=strlen(s)) < n) {
      match_all_chars = 1;
   }   

   for (i=0; tokenv[i].text; i++) {
      if (match_all_chars ? 
          !strcasecmp(tokenv[i].text, s):
          !strncasecmp(tokenv[i].text, s, len)) {
         
/*             DPRINTF(("recognized \"%s\" == \"%s\" with %d\n", tokenv[i].text, s, tokenv[i].token));  */
               
         DRETURN(tokenv[i].token);
      }
   }

   DRETURN(tokenv[i].token);
}

static int tm_yday_cmp(const lListElem *t1, const lListElem *t2) {
   int t;

/*   DPRINTF(("tm_yday_cmp(y%d:m%d:d%d, y%d:m%d:d%d)\n", 
      lGetUlong(t1, TM_year),
      lGetUlong(t1, TM_mon),
      lGetUlong(t1, TM_mday),
      lGetUlong(t2, TM_year),
      lGetUlong(t2, TM_mon),
      lGetUlong(t2, TM_mday))); */

   if ((t=(lGetUlong(t1, TM_year) - lGetUlong(t2, TM_year))))
      return t;
   if ((t=(lGetUlong(t1, TM_mon) - lGetUlong(t2, TM_mon))))
      return t;
   return lGetUlong(t1, TM_mday) - lGetUlong(t2, TM_mday);
}

static int tm_wday_cmp(const lListElem *t1, const lListElem *t2) {
   return lGetUlong(t1, TM_wday) - lGetUlong(t2, TM_wday);
}

static int tm_daytime_cmp(const lListElem *t1, const lListElem *t2) {
   int t;

   if ((t=(lGetUlong(t1, TM_hour) - lGetUlong(t2, TM_hour)))) {
      return t;
   }   
   if ((t=(lGetUlong(t1, TM_min) - lGetUlong(t2, TM_min)))) {
      return t;
   }   
   return lGetUlong(t1, TM_sec) - lGetUlong(t2, TM_sec);
}

void 
cullify_tm(lListElem *tm_ep, struct tm *tm_now) 
{
   lSetUlong(tm_ep, TM_mday,  tm_now->tm_mday);
   lSetUlong(tm_ep, TM_mon,   tm_now->tm_mon);
   lSetUlong(tm_ep, TM_year,  tm_now->tm_year);
   lSetUlong(tm_ep, TM_sec,   tm_now->tm_sec);
   lSetUlong(tm_ep, TM_min,   tm_now->tm_min);
   lSetUlong(tm_ep, TM_hour,  tm_now->tm_hour);
   lSetUlong(tm_ep, TM_wday,  tm_now->tm_wday);
   lSetUlong(tm_ep, TM_yday,  tm_now->tm_yday);
   lSetUlong(tm_ep, TM_isdst, tm_now->tm_isdst);
}

void 
uncullify_tm(const lListElem *tm_ep, struct tm *tm_now) 
{
   tm_now->tm_mday =  lGetUlong(tm_ep, TM_mday);
   tm_now->tm_mon  =  lGetUlong(tm_ep, TM_mon);
   tm_now->tm_year =  lGetUlong(tm_ep, TM_year);
   tm_now->tm_sec  =  lGetUlong(tm_ep, TM_sec);
   tm_now->tm_min  =  lGetUlong(tm_ep, TM_min);
   tm_now->tm_hour =  lGetUlong(tm_ep, TM_hour);
   tm_now->tm_wday =  lGetUlong(tm_ep, TM_wday);
   tm_now->tm_yday =  lGetUlong(tm_ep, TM_yday);
   tm_now->tm_isdst = lGetUlong(tm_ep, TM_isdst);
}

bool 
calendar_parse_year(lListElem *cal, lList **answer_list) 
{
   bool ret = true;
   lList *yc = NULL;

   DENTER(TOP_LAYER, "calendar_parse_year");
   if (disabled_year_list(answer_list, lGetString(cal, CAL_year_calendar), 
                          &yc, lGetString(cal, CAL_name))) {
      ret = false;               
   } else {
      lXchgList(cal, CAL_parsed_year_calendar, &yc);
      lFreeList(&yc); 
   }

   DRETURN(ret);
}

bool calendar_parse_week(lListElem *cal, lList **answer_list) {
   bool ret = true;
   lList *wc = NULL;

   DENTER(TOP_LAYER, "calendar_parse_week");
   if (disabled_week_list(answer_list, lGetString(cal, CAL_week_calendar), 
            &wc, lGetString(cal, CAL_name))) {
      ret = false;
   } else {
      lXchgList(cal, CAL_parsed_week_calendar, &wc);
      lFreeList(&wc); 
   }
   DRETURN(ret);
}

/****** sge_calendar/calendar_get_current_state_and_end() **********************
*  NAME
*     calendar_get_current_state_and_end() -- generates the TODO orders
*
*  SYNOPSIS
*     u_long32 calendar_get_current_state_and_end(const lListElem *cep, time_t 
*     *then, time_t *now) 
*
*  FUNCTION
*     It calles the routins to compute the current state and next change. The current
*     state is than changed into todo orders.
*
*  INPUTS
*     const lListElem *cep - (in) calendar (CAL_Type)
*     time_t *then         - (out) next state change
*     time_t *now          - (in) now
*
*  RESULT
*     u_long32 - what to do? what is the current state
*
*  NOTES
*     MT-NOTE: calendar_get_current_state_and_end() is MT safe 
*
*******************************************************************************/
static u_long32 calendar_get_current_state_and_end(const lListElem *cep, time_t *then, time_t *now) {
   u_long32 new_state;
   lList *year_list = NULL;
   lList *week_list = NULL;
   
   DENTER(TOP_LAYER, "calendar_get_current_state_and_end");

   DPRINTF(("cal: %s\n", lGetString(cep, CAL_name)));

   if (cep != NULL) {
      year_list = lGetList(cep, CAL_parsed_year_calendar);
      week_list = lGetList(cep, CAL_parsed_week_calendar);
   }
   
   if (now == NULL) {
      new_state = state_at((time_t)sge_get_gmt(), year_list, week_list, then);
   } else {
      new_state = state_at(*now, year_list, week_list, then);
   }
   
   switch (new_state) {
   case QI_DO_DISABLE:
      new_state = QI_DO_CAL_DISABLE;
      break;
   case QI_DO_SUSPEND:
      new_state = QI_DO_CAL_SUSPEND;
      break;
   default:
      new_state = QI_DO_NOTHING;
      break;
   }

   DRETURN(new_state);
}

bool 
calendar_is_referenced(const lListElem *calendar, lList **answer_list,
                       const lList *master_cqueue_list)
{
   bool ret = false;
   lListElem *cqueue = NULL, *cal = NULL;
 
   /*
    * fix for bug 6422335
    * check the cq configuration for calendar references instead of qinstances
    */
   const char *calendar_name = lGetString(calendar, CAL_name);     
   if (calendar_name != NULL) {
      for_each (cqueue, master_cqueue_list) {
         for_each (cal, lGetList(cqueue, CQ_calendar)) {
            const char *qcal_name = lGetString(cal, ASTR_value);
            if (qcal_name && !strcmp(qcal_name, calendar_name)) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                       ANSWER_QUALITY_INFO,
                                       MSG_CALENDAR_REFINQUEUE_SS,
                                       calendar_name,
                                       lGetString(cqueue, CQ_name));
               ret = true;
               break;
            }
         }
      }
   }
   return ret;
}

/* -----------------------------
   
   build up a generic cal object

   returns 
      NULL on error

*/
lListElem* sge_generic_cal(char *cal_name) {
   lListElem *calp;

   DENTER(TOP_LAYER, "sge_generic_cal");

   calp = lCreateElem(CAL_Type);

   lSetString(calp, CAL_name, cal_name?cal_name:"template");

   DRETURN(calp);
}

/****** sge_calendar/calendar_open_in_time_frame() *****************************
*  NAME
*     calendar_open_in_time_frame() -- check if calender is open in given time
*     frame
*
*  SYNOPSIS
*     bool calendar_open_in_time_frame(const lListElem *cep, u_long32 
*     start_time, u_long32 duration) 
*
*  FUNCTION
*     Returns the state (only open or closed) of a calendar in a given time
*     frame
*
*  INPUTS
*     const lListElem *cep - calendar object (CAL_Type)
*     u_long32 start_time  - time frame start
*     u_long32 duration    - time frame duration
*
*  RESULT
*     bool - true if open
*            false if closed
*
*  NOTES
*     MT-NOTE: calendar_open_in_time_frame() is MT safe 
*******************************************************************************/
bool calendar_open_in_time_frame(const lListElem *cep, u_long32 start_time, u_long32 duration)
{
   bool ret = true;
   u_long32 state;
   lList *year_list = NULL;
   lList *week_list = NULL;
   time_t next_change;
   time_t start = (time_t)start_time;
   time_t end = (time_t)duration_add_offset(start_time, duration);

   DENTER(TOP_LAYER, "calendar_open_in_time_frame");

   if (cep != NULL) {
      year_list = lGetList(cep, CAL_parsed_year_calendar);
      week_list = lGetList(cep, CAL_parsed_week_calendar);
   }

   state = state_at(start, year_list, week_list, &next_change);
   while (state == QI_DO_ENABLE && next_change != 0 && next_change <= end) {
      start = next_change;
      state = state_at(start, year_list, week_list, &next_change);
   }

   if (state != QI_DO_ENABLE) {
      ret = false;
   }
  
   DRETURN(ret);
}
