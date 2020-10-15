#ifndef __JAPIP_H

#include "cull.h"

struct drmaa_job_template_s {
   lList *strings;        /* VA_Type  */
   lList *string_vectors; /* NSV_Type */
};

/* 
 * This iterator is returned by 
 *   drmaa_run_bulk_jobs()             - vector of vector of job ids
 */
struct drmaa_bulk_jobid_iterator_s {
   u_long32 jobid;
   int start;
   int end;
   int incr;
   /* next position of iterator */
   int next_pos;
};

/* 
 * This iterator is returned by 
 *   japi_get_vector_attribute()       - vector of attribute values
 *   japi_get_attribute_names()        - vector of attribute name 
 *   japi_get_vector_attribute_names() - vector of attribute name 
 *   japi_wait()                       - vector of rusage strings 
 */
struct drmaa_string_array_iterator_s {
   lList *strings;  /* STR_Type  */
   /* next position of iterator */
   lListElem *next_pos;
};

/*
 * Transparent use of two different iterators 
 */
enum { JAPI_ITERATOR_BULK_JOBS, JAPI_ITERATOR_STRINGS };
struct drmaa_attr_names_s {
   int iterator_type; 
   union {
      struct drmaa_bulk_jobid_iterator_s ji;
      struct drmaa_string_array_iterator_s si;
   } it;
};
struct drmaa_attr_values_s {
   int iterator_type; 
   union {
      struct drmaa_bulk_jobid_iterator_s ji;
      struct drmaa_string_array_iterator_s si;
   } it;
};
struct drmaa_job_ids_s {
   int iterator_type; 
   union {
      struct drmaa_bulk_jobid_iterator_s ji;
      struct drmaa_string_array_iterator_s si;
   } it;
};

#endif /* __JAPIP_H */
