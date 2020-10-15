#ifndef __SGE_CENTRY_H 
#define __SGE_CENTRY_H 
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

#include "sgeobj/sge_centry_CE_L.h"
#include "sgeobj/sge_ct_SCT_L.h"
#include "sgeobj/sge_ct_REF_L.h"
#include "sgeobj/sge_ct_CT_L.h"
#include "sgeobj/sge_ct_CCT_L.h"
#include "sgeobj/sge_ct_CTI_L.h"

/* 
 * This is the list type we use to hold the complex list in qmaster.
 *
 * We also use it for the queue information which administrator defined 
 * complexes aply to this queue. In this case CX_entries is unused. 
 * At the moment this applies only for the gdi. Internal the old list is
 * used.  
 */

/* relops in CE_relop */
enum {
   CMPLXEQ_OP = 1, /* == */
   CMPLXGE_OP,     /* >= */
   CMPLXGT_OP,     /* > */
   CMPLXLT_OP,     /* < */
   CMPLXLE_OP,     /* <= */
   CMPLXNE_OP,     /* != */
   CMPLXEXCL_OP    /* EXCL */
};

enum {
   REQU_NO = 1,
   REQU_YES,
   REQU_FORCED
};

enum {
   CONSUMABLE_NO = 0,
   CONSUMABLE_YES,
   CONSUMABLE_JOB
};

/* bit mask for CE_dominant */
enum {
   DOMINANT_LAYER_GLOBAL = 0x0001,
   DOMINANT_LAYER_HOST = 0x0002,
   DOMINANT_LAYER_QUEUE = 0x0004,
   DOMINANT_LAYER_RQS = 0x0008,
   DOMINANT_LAYER_MASK = 0x00ff,        /* all layers */

   DOMINANT_TYPE_VALUE = 0x0100,        /* value from complex template */
   DOMINANT_TYPE_FIXED = 0x0200,        /* fixed value from object
                                         * configuration */
   DOMINANT_TYPE_LOAD = 0x0400,         /* load value */
   DOMINANT_TYPE_CLOAD = 0x0800,        /* corrected load value */
   DOMINANT_TYPE_CONSUMABLE = 0x1000,   /* consumable */
   DOMINANT_TYPE_MASK = 0xff00          /* all types */
};

/* tag level*/
enum{
   NO_TAG = 0,
   QUEUE_TAG,
   HOST_TAG,
   GLOBAL_TAG,
   PE_TAG,     /* not really used as a tag */
   RQS_TAG,    /* not really used as a tag */
   MAX_TAG
};

#define CENTRY_LEVEL_TO_CHAR(level) "NQHGPLM"[level]

/* Mapping list for generating a complex out of a queue */
struct queue2cmplx {
   char *name;    /* name of the centry element, not the shortcut */
   int  field;    /* name of the element in the queue structure */
   int  cqfld;    /* cluster queue field */
   int  valfld;   /* value field in cluster queue sublist */
   int  type;     /* type of the element in the queue strcuture */
};
extern const int max_host_resources;
extern const struct queue2cmplx host_resource[]; 
extern const int max_queue_resources;
extern const struct queue2cmplx queue_resource[];

int 
get_rsrc(const char *name, bool queue, int *field, int *cqfld, int *valfld, int *type);

int
centry_fill_and_check(lListElem *this_elem, lList** answer_list, bool allow_empty_boolean,
                      bool allow_neg_consumable);

const char *
map_op2str(u_long32 op);

const char *
map_type2str(u_long32 type);

const char *
map_req2str(u_long32 op);

const char *
map_consumable2str(u_long32 op);

lListElem *
centry_create(lList **answer_list, const char *name);

bool
centry_is_referenced(const lListElem *centry, lList **answer_list,
                     const lList *master_cqueue_list,
                     const lList *master_exechost_list,
                     const lList *master_lirs_list);

bool
centry_print_resource_to_dstring(const lListElem *this_elem, 
                                 dstring *string);

lList **
centry_list_get_master_list(void);

lListElem *
centry_list_locate(const lList *this_list, 
                   const char *name);

bool
centry_elem_validate(lListElem *centry, lList *centry_list, lList **answer_list);


bool
centry_list_sort(lList *this_list);

bool
centry_list_init_double(lList *this_list);

int
centry_list_fill_request(lList *centry_list, lList **answer_list, lList *master_centry_list,
                         bool allow_non_requestable, bool allow_empty_boolean,
                         bool allow_neg_consumable);

bool
centry_list_are_queues_requestable(const lList *this_list);

const char *
centry_list_append_to_dstring(const lList *this_list, dstring *string); 

int
centry_list_append_to_string(lList *this_list, char *buff, u_long32 max_len);

lList *
centry_list_parse_from_string(lList *complex_attributes,
                              const char *str, bool check_value);

void
centry_list_remove_duplicates(lList *this_list);

double 
centry_urgency_contribution(int slots, const char *name, double value, 
                            const lListElem *centry);

bool
centry_list_do_all_exists(const lList *this_list, lList **answer_list,
                          const lList *centry_list);

bool
centry_list_is_correct(lList *this_list, lList **answer_list);

int ensure_attrib_available(lList **alpp, lListElem *ep, int nm);

bool
validate_load_formula(const char *formula, lList **answer_list, lList *centry_list, const char *name);

bool load_formula_is_centry_referenced(const char *load_formula, const lListElem *centry);

#endif /* __SGE_CENTRY_H */

