#ifndef __PARSE_QCONF_H
#define __PARSE_QCONF_H
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

#include "spool/sge_spooling_utilities.h"
#include "gdi/sge_gdi_ctx.h"

typedef struct object_info_entry {
   u_long32 target;
   char *object_name;
   lDescr *cull_descriptor;
   char *attribute_name;
   int nm_name;
   spooling_field *fields;
   const struct spool_flatfile_instr *instr;
   bool (*pre_gdi_function)(lList *list, lList **answer_list);
} object_info_entry;

int sge_parse_qconf(sge_gdi_ctx_class_t *ctx, char **argv);


#endif /* __PARSE_QCONF_H */

