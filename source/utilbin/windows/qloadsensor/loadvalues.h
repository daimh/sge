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

#ifndef __LOADVALUES_H
#define __LOADVALUES_H

#include "win32_type.h"
#include "pdhquery.h"

struct _loadvalues {
   /*
    * Don't change the position of the hostname attribute within this
    * structure. It has to be the first element.
    */
   TXCHAR hostname;      
   DOUBLE load_avg[3];   /* 1/5/15 minute load average */
   LONGLONG cpu;         /* cpu load */
   DWORD num_proc;       /* number of processors */
   DOUBLE swap_free;     /* the amount of swap memory */
   DOUBLE swap_used;     /* the amount of used spaw space */
   DOUBLE swap_total;    /* the total amount of swap space */
   DOUBLE mem_free;      /* the amount of free memory */
   DOUBLE mem_used;      /* the amount of used memory */
   DOUBLE mem_total;     /* the total amount of memory */
   DOUBLE virtual_free;  
   DOUBLE virtual_used;  
   DOUBLE virtual_total; 
};

typedef struct _loadvalues t_loadvalues;

extern t_loadvalues loadvalue;
extern HANDLE loadvalue_mutex;

extern BOOL disable_beginend;
extern BOOL disable_numproc;
extern BOOL disable_load;
extern BOOL disable_memory;
extern BOOL disable_cpu;
extern TXCHAR default_hostname;

void
loadvalue_set_adj(DOUBLE adj);

void
loadvalue_set_fac(DOUBLE fac);

void
loadvalue_prefix(void);

void
loadvalue_postfix(void);

void
loadvalue_print(t_loadvalues *loadvalue);

int
loadvalue_update_hostname(t_loadvalues *loadvalue);

int
loadvalue_update_num_proc(t_loadvalues *loadvalue);

int
loadvalue_update_memory(t_loadvalues *loadvalue);

int
loadvalue_update_cpuusage(t_loadvalues *loadvalue, t_pdhquery *query,
                          t_pdhcounterset *counter_cpuusage);

int
loadvalue_update_processor_queue_length(t_loadvalues *loadvalue, t_pdhquery *query,
                          t_pdhcounterset *counter_queue_length);

int
loadvalue_update_load(t_loadvalues *loadvalue, t_pdhquery *query,
                      t_pdhcounterset *counter_state,
                      t_pdhcounterset *counter_pid);

#endif /* __LOADVALUES_H */
