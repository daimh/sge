#ifndef __PATH_ALIASES_H
#define __PATH_ALIASES_H
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

#include "uti/sge_dstring.h"

#include "sgeobj/sge_path_alias_PA_L.h"

#define PATH_ALIAS_COMMON_FILE    "common/sge_aliases"
#define PATH_ALIAS_HOME_FILE      ".sge_aliases"

int path_alias_list_initialize(lList **path_alias_list,
                               lList **alpp,
                               const char *cell_root,
                               const char *user,
                               const char *host);

void path_alias_list_get_path(const lList *path_aliases, lList **alpp,
                              const char *inpath, const char *myhost,
                              dstring *outpath);

bool path_verify(const char *path, lList **answer_list, const char *name, bool absolute);
bool path_list_verify(const lList *path_list, lList **answer_list, const char *name);
bool path_alias_verify(const lList *path_aliases, lList **answer_list);

#endif /* __PATH_ALIASES_H */
