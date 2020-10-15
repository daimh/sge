#ifndef __SGE_SHARETREE_PRINTING_H
#define __SGE_SHARETREE_PRINTING_H
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

#endif /* __SGE_SHARETREE_PRINTING_H */

#include "cull.h"

#include "sge_dstring.h"

typedef struct {
   bool name_format;
   bool format_times;
   const char *delim;
   const char *line_delim;
   const char *rec_delim;
   const char *str_format;
   const char *field_names;
   const char *line_prefix;
} format_t;

void 
print_hdr(dstring *out, const format_t *format);

void
sge_sharetree_print(dstring *out, lList *sharetree, const lList *users, 
                    const lList *projects, const lList *usersets,
                    bool group_nodes, bool decay_usage, 
                    const char **names, const format_t *format);
