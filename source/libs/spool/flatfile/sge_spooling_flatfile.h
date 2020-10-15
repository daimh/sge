#ifndef __SGE_SPOOLING_FLATFILE_H 
#define __SGE_SPOOLING_FLATFILE_H 
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

#include "cull.h"

#include "spool/sge_spooling.h"
#include "spool/sge_spooling_utilities.h"

#define SHARETREE_FILE            "sharetree"

/****** spool/flatfile/--Spooling-Flatfile ************************************
*
*  NAME
*     Flat file spooling - spooling and output of data in flat files
*
*  FUNCTION
*     The module provides functions and a spooling framework instantiation
*     for data input/output in flat files.
*
*     It can be used for spooling of data, for information output (e.g. qstat)
*     and input/output as used by qconf.
*
*     The output format can be influenced by the use of a spool_flatfile_instr
*     structure.
*
*  SEE ALSO
*     spool/flatfile/-Spooling-Flatfile-Typedefs
*     spool/flatfile/spool_flatfile_write_object()
*     spool/flatfile/spool_flatfile_write_list()
*     spool/flatfile/spool_flatfile_read_object()
*     spool/flatfile/spool_flatfile_read_list()
*     spool/flatfile/spool_flatfile_align_object()
*     spool/flatfile/spool_flatfile_align_list()
****************************************************************************
*/


/*
 * spooling framework functions
 */

#ifdef SPOOLING_classic
const char *get_spooling_method(void);
#else
const char *get_classic_spooling_method(void);
#endif

lListElem *
spool_classic_create_context(lList **answer_list, const char *args);

bool 
spool_classic_default_startup_func(lList **answer_list, 
                                    const lListElem *rule, bool check);
bool 
spool_classic_common_startup_func(lList **answer_list, 
                                   const lListElem *rule, bool check);

bool 
spool_classic_default_list_func(lList **answer_list, 
                                 const lListElem *type, 
                                 const lListElem *rule,
                                 lList **list, 
                                 const sge_object_type object_type);
lListElem *
spool_classic_default_read_func(lList **answer_list, 
                                 const lListElem *type, 
                                 const lListElem *rule,
                                 const char *key, 
                                 const sge_object_type object_type);
bool 
spool_classic_default_write_func(lList **answer_list, 
                                  const lListElem *type, 
                                  const lListElem *rule, 
                                  const lListElem *object, 
                                  const char *key, 
                                  const sge_object_type object_type);
bool 
spool_classic_default_delete_func(lList **answer_list, 
                                   const lListElem *type, 
                                   const lListElem *rule,
                                   const char *key, 
                                   const sge_object_type object_type);
#endif /* __SGE_SPOOLING_FLATFILE_H */    
