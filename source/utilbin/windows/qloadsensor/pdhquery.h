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

#ifndef __PDH_QUERY
#define __PDH_QUERY

#include <windows.h>
#include <pdh.h>

#include "win32_type.h"
#include "qloadsensor.h"

struct _pdhquery {
   HQUERY query;
};

typedef struct _pdhquery t_pdhquery;

struct _pdhcounterset {
   TXCHAR object;
   TXCHAR instance;
   TXCHAR counter;

   TXCHAR trans_object;
   TXCHAR trans_instance;
   TXCHAR trans_counter;
   TXCHAR wild_counter_path;

   DWORD number_of_counters;
   HCOUNTER *counter_handles;
   TXCHAR *pdh_name;
};

typedef struct _pdhcounterset t_pdhcounterset;

DWORD
pdhquery_initialize(t_pdhquery *query);

DWORD 
pdhquery_free(t_pdhquery *query);

DWORD 
pdhquery_add_counterset(t_pdhquery *query, t_pdhcounterset *counterset);

DWORD 
pdhquery_remove_counterset(t_pdhquery *query, t_pdhcounterset *counterset);

DWORD
pdhquery_update(t_pdhquery *query);



DWORD 
pdhcounterset_initialize(t_pdhcounterset *counterset, LPSTR object, LPSTR instance, LPSTR counter);

DWORD 
pdhcounterset_free(t_pdhcounterset *counterset);

#endif /* __PDH_QUERY */
