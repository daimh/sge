#ifndef __SGE_FLATFILE_H 
#define __SGE_FLATFILE_H 
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


/****** spool/flatfile/-Spooling-Flatfile-Typedefs ***************************
*
*  NAME
*     Typedefs -- type definitions for spooling utility functions
*
*  SYNOPSIS
*     <to be documented after the module is finished>
*
*  FUNCTION
*     spool_flatfile_destination
*        Used to specify the destination of an output function, e.g. 
*        streams like stdin or stdout, temporary file or named file.
*
*     spool_flatfile_format
*        Format to use for output, e.g. ASCII, XML, CULL.
*
*     spool_flatfile_instr
*        Instruction for spooling. 
*        Describes which fields to spool, formatting, spooling of sublists ...
*
*  NOTES
*     May not allow really comprehensive output in all possible variations,
*     but it seems to be sufficient for all spooling and output done in
*     Grid Engine.
*
*  SEE ALSO
****************************************************************************
*/

#include "cull/cull.h"

#include "spool/sge_spooling_utilities.h"

#define MAX_LINE_LENGTH 80

typedef enum {
   SP_DEST_STDOUT,
   SP_DEST_STDERR,
   SP_DEST_TMP,
   SP_DEST_SPOOL
} spool_flatfile_destination;

typedef enum {
   SP_FORM_ASCII,
   SP_FORM_XML,
   SP_FORM_CULL
} spool_flatfile_format;

/*
 * - output field name and value or only field name
 * - output a header that contains all field names of a sub list
 * - alignment: if field names are shown, align all values horizontally.
 * - delimiter for use between field name and value, if alignment is active,
 *   it may be repeated
 * - delimiter between fields
 * - delimiter between records
 * - for nested structures: record begin and record end sign
 *
 * parsing of such spooled data will accept any whitespaces between tokens
 */

typedef struct recursion_info {
   int recursion_field; /* The field that holds the recursive elements */
   int id_field; /* The field that is used to id recursive elements for building
                  * a tree structure */
   int supress_field; /* The field to only print for the root element */
} recursion_info;

/* JG: TODO: we need a check function:
 * - delimiters may not contain whitespace, exception \n
 */
typedef struct spool_flatfile_instr {
   const spool_instr *spool_instr;
   bool show_field_names;
   bool show_field_header;
   bool show_footer;
   bool align_names;
   bool align_data;
   bool record_start_end_newline;
   bool show_empty_fields;
   bool ignore_list_name;
   const char name_value_delimiter;
   const char field_delimiter;
   const char record_delimiter;
   const char record_start;
   const char record_end;
   const struct spool_flatfile_instr *sub_instr;
   const recursion_info recursion_info;
} spool_flatfile_instr;

typedef struct flatfile_info {
   spooling_field *fields;
   const spool_flatfile_instr *instr;
} flatfile_info;

extern const spool_flatfile_instr qconf_sub_name_value_space_sfi;
extern const spool_flatfile_instr qconf_sfi;
extern const spool_flatfile_instr qconf_sub_comma_list_sfi;
extern const spool_flatfile_instr qconf_name_value_list_sfi;
extern const spool_flatfile_instr qconf_sub_name_value_comma_sfi;
extern const spool_flatfile_instr qconf_sub_comma_sfi;
extern const spool_flatfile_instr qconf_param_sfi;
extern const spool_flatfile_instr qconf_sub_param_sfi;
extern const spool_flatfile_instr qconf_comma_sfi;
extern const spool_flatfile_instr qconf_ce_sfi;
extern const spool_flatfile_instr qconf_ce_list_sfi;
extern const spool_flatfile_instr qconf_sub_rqs_sfi;
extern const spool_flatfile_instr qconf_sub_spool_usage_sfi;
extern const spool_flatfile_instr qconf_rqs_sfi;
extern const spool_flatfile_instr qconf_sub_name_value_comma_braced_sfi;


const char *
spool_flatfile_write_object(lList **answer_list, const lListElem *object,
                            bool is_root, const spooling_field *fields,
                            const spool_flatfile_instr *instr,
                            const spool_flatfile_destination destination,
                            const spool_flatfile_format format, 
                            const char *filepath, bool print_header);

const char *
spool_flatfile_write_list(lList **answer_list,
                          const lList *list,
                          const spooling_field *fields,
                          const spool_flatfile_instr *instr,
                          const spool_flatfile_destination destination,
                          const spool_flatfile_format format,
                          const char *filepath, bool print_header);

lListElem *
spool_flatfile_read_object(lList **answer_list, const lDescr *descr, lListElem *root,
                           const spooling_field *fields_in, int fields_out[],
                           bool parse_values, const spool_flatfile_instr *instr,
                           const spool_flatfile_format format,
                           FILE *file,
                           const char *filepath);
lList *
spool_flatfile_read_list(lList **answer_list, const lDescr *descr, 
                         const spooling_field *fields_in, int fields_out[],
                         bool parse_values, const spool_flatfile_instr *instr,
                         const spool_flatfile_format format,
                         FILE *file,
                         const char *filepath);

bool 
spool_flatfile_align_object(lList **answer_list,
                            spooling_field *fields);

bool
spool_flatfile_align_list(lList **answer_list, const lList *list, 
                          spooling_field *fields, int padding);

int spool_get_unprocessed_field(spooling_field in[], int out[], lList **alpp);
int spool_get_number_of_fields(const spooling_field fields[]);

#endif /* __SGE_FLATFILE_H */
