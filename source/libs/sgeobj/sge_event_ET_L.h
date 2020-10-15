#ifndef __SGE_EVENT_ET_L_H
#define __SGE_EVENT_ET_L_H

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

#include "uti/sge_monitor.h"
#include "cull/cull.h"
#include "sgeobj/sge_boundaries.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */

enum {
   ET_number = ET_LOWERBOUND,/* number of the event */
   ET_timestamp,             /* unix time stamp (gmt) when the event occured */
   ET_type,                  /* type the event */
   ET_intkey,                /* a int key for use by a specific event type */
   ET_intkey2,               /* a int key for use by a specific event type */
   ET_strkey,                /* a str key for use by a specific event type */
   ET_strkey2,               /* a str key for use by a specific event type */
   ET_new_version            /* new version of the changed object */
                             /* JG: TODO: we should have different fields for 
                              *           objects (SGE_OBJECT) and
                              *           lists   (SGE_LIST), as we now have
                              *           proper handling for objects.
                              */
};

LISTDEF(ET_Type)
   JGDI_OBJ(Event)
   SGE_ULONG(ET_number, CULL_DEFAULT)
   SGE_ULONG(ET_timestamp, CULL_DEFAULT)
   SGE_ULONG(ET_type, CULL_DEFAULT)
   SGE_ULONG(ET_intkey, CULL_DEFAULT)
   SGE_ULONG(ET_intkey2, CULL_DEFAULT)
   SGE_STRING(ET_strkey, CULL_DEFAULT)
   SGE_STRING(ET_strkey2, CULL_DEFAULT)
   SGE_LIST(ET_new_version, CULL_ANY_SUBTYPE, CULL_DEFAULT)
LISTEND 

NAMEDEF(ETN)
   NAME("ET_number")
   NAME("ET_timestamp")
   NAME("ET_type")
   NAME("ET_intkey")
   NAME("ET_intkey2")
   NAME("ET_strkey")
   NAME("ET_strkey2")
   NAME("ET_new_version")
NAMEEND

#define ETS sizeof(ETN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_EVENTL_H */
