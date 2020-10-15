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

#ifndef NO_SGE_COMPILE_DEBUG
#define NO_SGE_COMPILE_DEBUG
#endif

#define FLATFILE_LAYER BASIS_LAYER

/* system */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(ALPHA)
   extern void flockfile(FILE *);
   extern void funlockfile(FILE *);
#endif

#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_stdio.h"
#include "uti/sge_string.h"
#include "uti/sge_tmpnam.h"
#include "uti/sge_spool.h"
#include "uti/sge_unistd.h"
#include "uti/sge_profiling.h"

#include "sgeobj/sge_config.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_sharetree.h"
#include "sgeobj/sge_all_listsL.h"

#include "spool/sge_spooling_utilities.h"
#include "spool/msg_spoollib.h"
#include "spool/flatfile/msg_spoollib_flatfile.h"
#include "spool/flatfile/sge_spooling_flatfile_scanner.h"
#include "spool/flatfile/sge_flatfile.h"

#include "msg_common.h"

const spool_flatfile_instr qconf_sub_name_value_space_sfi = 
{
   NULL,
   false,
   false,
   false,
   false,
   false,
   false,
   true,
   false,
   '\0',
   '=',
   ' ',
   '\0',
   '\0',
   &qconf_sub_name_value_space_sfi,
   { NoName, NoName, NoName }
};

const spool_flatfile_instr qconf_sfi = 
{
   NULL,
   true,
   false,
   false,
   true,
   false,
   false,
   true,
   false,
   ' ',
   '\n',
   '\0',
   '\0',
   '\0',
   &qconf_sub_name_value_space_sfi,
   { NoName, NoName, NoName }
};

const spool_flatfile_instr qconf_sub_comma_list_sfi = 
{
   NULL,
   false,
   false,
   false,
   false,
   false,
   false,
   true,
   false,
   '\0',
   ',',
   '\0',
   '\0',
   '\0',
   NULL,
   {NoName, NoName, NoName}
};

const spool_flatfile_instr qconf_name_value_list_sfi = 
{
   NULL,
   true,
   false,
   false,
   false,
   false,
   false,
   true,
   false,
   '=',
   '\n',
   ',',
   '\0',
   '\0',
   &qconf_sub_comma_list_sfi,
   {
      STN_children,
      STN_id,
      STN_version
   }
};

const spool_flatfile_instr qconf_sub_name_value_comma_sfi = 
{
   NULL,
   false,
   false,
   false,
   false,
   false,
   false,
   true,
   false,
   '\0',
   '=',
   ',',
   '\0',
   '\0',
   NULL,
   { NoName, NoName, NoName }
};

const spool_flatfile_instr qconf_sub_comma_sfi = 
{
   NULL,
   false,
   false,
   false,
   false,
   false,
   false,
   true,
   false,
   '\0',
   '\0',
   ',',
   '\0',
   '\0',
   &qconf_sub_name_value_space_sfi,
   { NoName, NoName, NoName }
};

const spool_flatfile_instr qconf_param_sfi = 
{
   NULL,
   true,
   false,
   false,
   false,
   false,
   false,
   true,
   false,
   '\0',
   '\n',
   '\0',
   '\0',
   '\0',
   &qconf_sub_comma_sfi,
   { NoName, NoName, NoName }
};

const spool_flatfile_instr qconf_sub_param_sfi = 
{
   NULL,
   false,
   false,
   false,
   false,
   true,
   false,
   true,
   false,
   '\0',
   ' ',
   '\0',
   '\0',
   '\n',
   NULL,
   { NoName, NoName, NoName }
};

const spool_flatfile_instr qconf_comma_sfi = 
{
   NULL,
   true,
   false,
   false,
   true,
   false,
   false,
   true,
   false,
   ' ',
   '\n',
   '\0',
   '\0',
   '\0',
   &qconf_sub_name_value_comma_sfi,
   { NoName, NoName, NoName }
};

const spool_flatfile_instr qconf_ce_sfi = 
{
   NULL,
   true,
   false,
   false,
   true,
   false,
   false,
   true,
   false,
   ' ',
   '\n',
   '\0',
   '\0',
   '\0',
   &qconf_sub_name_value_space_sfi,
   { NoName, NoName, NoName }
};

const spool_flatfile_instr qconf_ce_list_sfi = 
{
   NULL,
   false,
   true,
   true,
   false,
   true,
   false,
   true,
   false,
   '\0',
   ' ',
   '\0',
   '\0',
   '\n',
   NULL,
   { NoName, NoName, NoName }
};

const spool_flatfile_instr qconf_sub_rqs_sfi = 
{
   NULL,
   true,
   false,
   false,
   false,
   false,
   false,
   false,
   true,
   '\0',
   ' ',
   '\0',
   '\0',
   '\n',
   &qconf_sub_name_value_comma_sfi,
   { NoName, NoName, NoName }
};

const spool_flatfile_instr qconf_sub_spool_usage_sfi = 
{
   NULL,
   false,
   false,
   false,
   false,
   false,
   false,
   false,
   true,
   '\0',
   ' ',
   '\0',
   '\0',
   ';',
   &qconf_sub_name_value_comma_sfi,
   { NoName, NoName, NoName }
};

const spool_flatfile_instr qconf_rqs_sfi = 
{
   NULL,
   true,
   false,
   false,
   false,
   false,
   true,
   true,
   true,
   ' ',
   '\n',
   '\0',
   '{',
   '}',
   &qconf_sub_rqs_sfi,
   { NoName, NoName, NoName }
};

const spool_flatfile_instr qconf_sub_name_value_comma_braced_sfi = 
{
   NULL,
   false,
   false,
   false,
   false,
   false,
   false,
   true,
   false,
   '\0',
   '=',
   ',',
   '[',
   ']',
   &qconf_sub_name_value_comma_sfi,
   { NoName, NoName, NoName }
};


static void spool_flatfile_add_line_breaks (dstring *buffer);

#ifdef DEBUG_FLATFILE
bool flatfile_debugging = true;

#define FF_DEBUG(msg) \
if (flatfile_debugging) {\
   debug_flatfile(msg, spool_line, *token, spool_text, end_token); \
}

static void debug_flatfile(const char *msg, int line, int token, 
                           const char *buffer, const char *end_token)
{
   const char *text;
   const char *et;

   DENTER(FLATFILE_LAYER, "debug_flatfile");

   if (token == 0) {
      text = "<EOF>";
   } else if (*buffer == '\n') {
      text = "<NEWLINE>";
   } else {
      text = buffer;
   }

   if (end_token == NULL) {
      et = "<NULL>";
   } else {
      et = end_token;
   }
   
   DPRINTF(("%-20s: line %4d, token %2d, text "SFQ", end_token = "SFQ"\n",
            msg, line, token, text, et));

   DEXIT;
}
#else
#define FF_DEBUG(msg)
#endif



static const char *output_delimiter(const char c)
{
   static char buffer[2] = { '\0', '\0' };
   const char *ret;

   switch (c) {
      case '\n':
         ret = "<NEWLINE>";
         break;
      default:
         buffer[0] = c;
         ret = buffer;
         break;
   }

   return ret;
}

static char *get_end_token(char *buffer, int size, const char *end_token,
                           const char new_end_token)
{
   char new_buffer[2] = { '\0', '\0' };

   if (end_token != NULL) {
      sge_strlcpy(buffer, end_token, size);
   } else {
      *buffer = '\0';
   }

   if (new_end_token != '\0') {
      new_buffer[0] = new_end_token;
   }

   sge_strlcat(buffer, new_buffer, size);

   return buffer;
}

static bool check_end_token(const char *end_token, const char act_char)
{
   bool ret = false;

   if (end_token !=  NULL && act_char != '\0') {
      if (strchr(end_token, act_char) != NULL) {
         ret = true;
      }
   }

   return ret;
}

static bool is_delimiter(int token)
{
   bool ret = false;
   
   if (token == SPFT_DELIMITER || token == SPFT_NEWLINE || 
       token == SPFT_WHITESPACE) {
       ret = true;
   }

   return ret;
}


static bool 
spool_flatfile_write_object_fields(lList **answer_list, const lListElem *object,
                                   dstring *buffer, 
                                   const spool_flatfile_instr *instr,
                                   const spooling_field *fields, bool recurse,
                                   bool root);

static bool
spool_flatfile_write_list_fields(lList **answer_list, const lList *list, 
                                 dstring *buffer, 
                                 const spool_flatfile_instr *instr,
                                 const spooling_field *fields, bool recurse, const char *list_name);
int
spool_flatfile_open_file(lList **answer_list,
                         const spool_flatfile_destination destination,
                         const char *filepath_in,
                         const char **filepath_out);

static bool
spool_flatfile_close_file(lList **answer_list, int fd, const char *filepath,
                          const spool_flatfile_destination destination);

static const char *
spool_flatfile_write_data(lList **answer_list, const void *data, int data_len, 
                          const spool_flatfile_destination destination, 
                          const char *filepath);

static lListElem *
_spool_flatfile_read_object(lList **answer_list, const lDescr *descr, lListElem *root,
                            const spool_flatfile_instr *instr,
                            const spooling_field *fields, int fields_out[], int *token,
                            const char *end_token, bool parse_values);

static void
_spool_flatfile_read_live_object(lList **answer_list, lListElem **object,
                                 const lDescr *descr, lListElem *root,
                                 const spool_flatfile_instr *instr, 
                                 const spooling_field *fields, int fields_out[],
                                 int *token, const char *end_token,
                                 bool parse_values);

static lList *
_spool_flatfile_read_list(lList **answer_list, const lDescr *descr, 
                          const spool_flatfile_instr *instr,
                          const spooling_field *fields, int fields_out[], int *token,
                          const char *end_token, bool parse_values, const char *list_name);

static spooling_field *get_recursion_field_list (const spool_flatfile_instr *instr);

/****** spool/flatfile/spool_flatfile_align_object() ********************
*  NAME
*     spool_flatfile_align_object() -- align object output
*
*  SYNOPSIS
*     bool 
*     spool_flatfile_align_object(lList **answer_list,
*                                 spooling_field *fields) 
*
*  FUNCTION
*     Computes the maximum length of the field names stored in <fields>
*     and sets the width value for all fields stored in <fields> to this 
*     maximum.
*
*  INPUTS
*     lList **answer_list     - answer list used to report errors
*     spooling_field *fields  - field description used for alignment
*
*  RESULT
*     bool - true on success, false if an error occured
*
*  SEE ALSO
*     spool/flatfile/spool_flatfile_align_list()
*******************************************************************************/
bool 
spool_flatfile_align_object(lList **answer_list, spooling_field *fields)
{
   int i;
   int width = 0;

   DENTER(FLATFILE_LAYER, "spool_flatfile_align_object");

   SGE_CHECK_POINTER_FALSE(fields, answer_list);

   for (i = 0; fields[i].nm != NoName; i++) {
      width = MAX(width, fields[i].name ? strlen(fields[i].name) : 0);
   }

   for (i = 0; fields[i].nm != NoName; i++) {
      fields[i].width = width;
   }

   DEXIT;
   return true;
}


/****** spool/flatfile/spool_flatfile_align_list() **********************
*  NAME
*     spool_flatfile_align_list() -- align list data for table output
*
*  SYNOPSIS
*     bool 
*     spool_flatfile_align_list(lList **answer_list, const lList *list, 
*                               spooling_field *fields) 
*
*  FUNCTION
*     Computes the maximum width of field name and field contents for 
*     fields described in <fields> and data in <list>.
*     Stores the computed maxima in <fields>.
*
*  INPUTS
*     lList **answer_list    - answer list for error reporting
*     const lList *list      - list with data to align
*     spooling_field *fields - field description
*
*  RESULT
*     bool -  true on success, else false
*
*  NOTES
*     Sublists are not regarded.
*
*  SEE ALSO
*     spool/flatfile/spool_flatfile_align_object()
*******************************************************************************/
bool 
spool_flatfile_align_list(lList **answer_list, const lList *list, 
                          spooling_field *fields, int padding)
{
   dstring buffer = DSTRING_INIT;
   const lListElem *object;
   int i;

   DENTER(FLATFILE_LAYER, "spool_flatfile_align_list");

   SGE_CHECK_POINTER_FALSE(list, answer_list);
   SGE_CHECK_POINTER_FALSE(fields, answer_list);

   for (i = 0; fields[i].nm != NoName; i++) {
      fields[i].width = fields[i].name ? strlen(fields[i].name) : 0;
   }

   for_each(object, list) {
      for (i = 0; fields[i].nm != NoName; i++) {
         const char *value;
         
         sge_dstring_clear(&buffer);
         value = object_append_field_to_dstring(object, answer_list, 
                                                &buffer, fields[i].nm, '\0');
         fields[i].width = MAX(fields[i].width,
                               (value ? strlen(value) : 0) + padding);
      }
   }

   sge_dstring_free(&buffer);

   DRETURN(true);
}

/****** spool/flatfile/spool_flatfile_write_list() **********************
*  NAME
*     spool_flatfile_write_list() -- write (spool) a complete list
*
*  SYNOPSIS
*     const char* 
*     spool_flatfile_write_list(lList **answer_list, const lList *list, 
*                               const spooling_field *fields_in, 
*                               const spool_flatfile_instr *instr, 
*                               const spool_flatfile_destination destination, 
*                               const spool_flatfile_format format, 
*                               const char *filepath) 
*
*  FUNCTION
*     Writes all data of a list according to the directives given with the
*     parameters.
*
*     Which fields to write can either be passed to the function by setting
*     the parameter <fields_in>, or the function will generate this information
*     using the spooling instructions passed in <instr>.
*
*     <destination> defines the spooling destination, e.g. to stdout, to 
*     a temporary file, to a named file (name is passed in <filepath>.
*  
*     The data format to use (e.g. ASCII format) is passed in <format>.
*
*     On success, the function returns the name of the output file/stream.
*     It is in the responsibility of the caller to free the memory used
*     by the file/stream name.
*
*  INPUTS
*     lList **answer_list                          - for error reporting
*     const lList *list                            - list to write
*     const spooling_field *fields_in              - optional, field description
*     const spool_flatfile_instr *instr            - spooling instructions
*     const spool_flatfile_destination destination - destination
*     const spool_flatfile_format format           - format
*     const char *filepath                         - if destination == 
*                                                    SP_DEST_SPOOL, path to the 
*                                                    spool file
*
*  RESULT
*     const char* - on success the name of the spool file / stream, else NULL
*
*  SEE ALSO
*     spool/flatfile/spool_flatfile_write_object()
*******************************************************************************/
const char *
spool_flatfile_write_list(lList **answer_list,
                          const lList *list,
                          const spooling_field *fields_in,
                          const spool_flatfile_instr *instr,
                          const spool_flatfile_destination destination,
                          const spool_flatfile_format format,
                          const char *filepath, bool print_header)
{
   dstring char_buffer = DSTRING_INIT;
   const char *result = NULL;
   const void *data = NULL;
   size_t data_len  = 0;
   const spooling_field *fields = NULL;
   spooling_field *my_fields = NULL;

   DENTER(FLATFILE_LAYER, "spool_flatfile_write_list");

   SGE_CHECK_POINTER_NULL(list, answer_list);
   SGE_CHECK_POINTER_NULL(instr, answer_list);

   /* if fields are passed, use them, else retrieve them from instructions */
   if (fields_in != NULL) {
      fields = fields_in;
   } else { 
      my_fields = spool_get_fields_to_spool(answer_list, lGetListDescr(list), 
                                            instr->spool_instr);
      if (my_fields == NULL) {
         /* message generated in spool_get_fields_to_spool */
         sge_dstring_free(&char_buffer);
         DRETURN(NULL);
      }

      if (format == SP_FORM_ASCII) {
         if (instr->align_names || instr->align_data) {
            if (!spool_flatfile_align_list(answer_list, list, my_fields, 0)) {
               /* message generated in spool_flatfile_align_object */
               my_fields = spool_free_spooling_fields(my_fields);
               sge_dstring_free(&char_buffer);
               DRETURN(NULL);
            }
         }
      }

      fields = my_fields;
   }

   switch (format) {
      case SP_FORM_ASCII:
         if (print_header) {
            dstring ds = DSTRING_INIT;
            sge_spoolmsg_append(&char_buffer, COMMENT_CHAR, feature_get_product_name(FS_VERSION, &ds));
            sge_dstring_free(&ds);
         }

         if (instr->show_field_header) {
            int i = 0;
            int len = 0;

            sge_dstring_append_char(&char_buffer, COMMENT_CHAR);
            
            for (i = 0; fields[i].nm != NoName; i++) {
               len += fields[i].width + (i?1:0);
               sge_dstring_sprintf_append(&char_buffer, "%-*s",
                                          fields[i].width + (i?1:0),
                                          fields[i].name);
            }

            sge_dstring_append_char(&char_buffer, '\n');
            sge_dstring_append_char(&char_buffer, COMMENT_CHAR);
            
            for (i = 0; i < len; i++) {
               sge_dstring_append_char(&char_buffer, '-');
            }
            
            sge_dstring_append_char(&char_buffer, '\n');
         }
         
         if (!spool_flatfile_write_list_fields(answer_list, list, &char_buffer, 
                                              instr, fields, false, NULL)) {
            /* in case of errors, messages are in answer_list,
             * clear data - we don't want to write erroneous data */
            sge_dstring_clear(&char_buffer); 
         }

         if (instr->record_end != '\n') {
            sge_dstring_append_char(&char_buffer, '\n');
         }
         
         if (instr->show_footer) {
            sge_dstring_append_char(&char_buffer, '#');
            sge_dstring_append_char(&char_buffer, ' ');
            sge_dstring_append(&char_buffer, MSG_COMPLEX_STARTSCOMMENTBUTNOSAVE);
            sge_dstring_append_char(&char_buffer, '\n');
         }

         if (!print_header && instr->show_field_names && (getenv("SGE_SINGLE_LINE") == NULL)) {
            spool_flatfile_add_line_breaks(&char_buffer);
         }
         
         data = sge_dstring_get_string(&char_buffer);
         data_len = sge_dstring_strlen(&char_buffer);

         break;
      case SP_FORM_XML:
      case SP_FORM_CULL:
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_NOTYETIMPLEMENTED_S, 
                                 "XML and CULL spooling");
         break;
   }      

   if (data == NULL || data_len == 0) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR,
                              "%s", MSG_FLATFILE_NODATATOSPOOL);
      sge_dstring_free(&char_buffer);
      if(my_fields != NULL) {
         my_fields = spool_free_spooling_fields(my_fields);
      }
      DRETURN(NULL);
   }

   result = spool_flatfile_write_data(answer_list, data, data_len, destination, 
                                      filepath);

   /* cleanup */
   sge_dstring_free(&char_buffer);

   /* if we created our own fields */
   if (my_fields != NULL) {
      my_fields = spool_free_spooling_fields(my_fields);
   }

   DRETURN(result);
}

/****** spool/flatfile/spool_flatfile_write_object() ********************
*  NAME
*     spool_flatfile_write_object() -- write (spool) an object 
*
*  SYNOPSIS
*     const char * 
*     spool_flatfile_write_object(lList **answer_list, const lListElem *object,
*                                 const spooling_field *fields_in, 
*                                 const spool_flatfile_instr *instr, 
*                                 const spool_flatfile_destination destination,
*                                 const spool_flatfile_format format, 
*                                 const char *filepath) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     lList **answer_list                          - ??? 
*     const lListElem *object                      - ??? 
*     const spooling_field *fields_in              - ??? 
*     const spool_flatfile_instr *instr            - ??? 
*     const spool_flatfile_destination destination - ??? 
*     const spool_flatfile_format format           - ??? 
*     const char *filepath                         - ??? 
*
*  RESULT
*     const char * - 
*
*  EXAMPLE
*     ??? 
*
*  NOTES
*     ??? 
*
*  BUGS
*     ??? 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
const char * 
spool_flatfile_write_object(lList **answer_list, const lListElem *object,
                            bool is_root, const spooling_field *fields_in,
                            const spool_flatfile_instr *instr,
                            const spool_flatfile_destination destination,
                            const spool_flatfile_format format, 
                            const char *filepath, bool print_header)
{
   dstring char_buffer = DSTRING_INIT;
   const char *result = NULL;
   const void *data = NULL;
   size_t data_len  = 0;
   spooling_field *my_fields = NULL;

   DENTER(FLATFILE_LAYER, "spool_flatfile_write_object");

   SGE_CHECK_POINTER_NULL(object, answer_list);
   SGE_CHECK_POINTER_NULL(instr, answer_list);

   /* if no fields are passed, retrieve them from instructions */
   if (fields_in == NULL) {
      my_fields = spool_get_fields_to_spool(answer_list, 
                                            object_get_type(object), 
                                            instr->spool_instr);
      if (my_fields == NULL) {
         /* message generated in spool_get_fields_to_spool */
         DRETURN(NULL);
      }

      if (format == SP_FORM_ASCII) {
         if (instr->align_names) {
            if (!spool_flatfile_align_object(answer_list, my_fields)) {
               /* message generated in spool_flatfile_align_object */
               my_fields = spool_free_spooling_fields(my_fields);
               DRETURN(NULL);
            }
         }
      }

      fields_in = my_fields;
   }   

   switch (format) {
      case SP_FORM_ASCII:
         if (print_header) {
            dstring ds = DSTRING_INIT;
            sge_spoolmsg_append(&char_buffer, COMMENT_CHAR, feature_get_product_name(FS_VERSION, &ds));
            sge_dstring_free(&ds);
         }

         if (!spool_flatfile_write_object_fields(answer_list, object, 
                                                &char_buffer, instr, fields_in,
                                                false, is_root)) {
            /* in case of errors, messages are in answer_list,
             * clear data - we don't want to write erroneous data */
            sge_dstring_clear(&char_buffer); 
         }

         sge_dstring_append_char(&char_buffer, '\n');

         if (!print_header && instr->show_field_names && (getenv("SGE_SINGLE_LINE") == NULL)) {
            spool_flatfile_add_line_breaks(&char_buffer);
         }

         data     = sge_dstring_get_string(&char_buffer);
         data_len = sge_dstring_strlen(&char_buffer);

         break;
      case SP_FORM_XML:
      case SP_FORM_CULL:
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, "not yet implemented");
         break;
   }      

   if (data_len == 0) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR,
                              "%s", MSG_FLATFILE_NODATATOSPOOL);
      sge_dstring_free(&char_buffer);
      if (my_fields != NULL) {
         my_fields = spool_free_spooling_fields(my_fields);
         fields_in = NULL;
      }
      DRETURN(NULL);
   } 

   result = spool_flatfile_write_data(answer_list, data, data_len, destination, 
                                      filepath);

   /* cleanup */
   sge_dstring_free(&char_buffer);

   /* if we created our own fields */
   if (my_fields != NULL) {
      my_fields = spool_free_spooling_fields(my_fields);
      fields_in = NULL;
   }

   DRETURN(result);
}

/****** spool/flatfile/spool_flatfile_open_file() ***********************
*  NAME
*     spool_flatfile_open_file() -- open spooling file or stream
*
*  SYNOPSIS
*     static FILE * 
*     spool_flatfile_open_file(lList **answer_list, 
*                              const spool_flatfile_destination destination, 
*                              const char *filepath_in, 
*                              const char **filepath_out) 
*
*  FUNCTION
*     Opens a file or stream as described by <destination>.
*
*     Streams are locked to handle concurrent access by multiple threads.
*     
*     If <destination> is SP_DEST_TMP, a temporary file is opened.
*
*     If <destination> is SP_DEST_SPOOL, the file specified by 
*     <filepath_in> is opened.
*
*     The name of the file/stream opened is returned in <filepath_out>.
*     It is in the responsibility of the caller to free the memory allocated
*     by <filepath_out>.
*
*     spool_flatfile_close_file shall be used to close a file opened using
*     spool_flatfile_open_file.
*
*  INPUTS
*     lList **answer_list                          - for error reporting 
*     const spool_flatfile_destination destination - destination
*     const char *filepath_in                      - optional filename
*     const char **filepath_out                    - returned filename
*
*  RESULT
*     static FILE * - on success a file handle, else NULL
*
*  SEE ALSO
*     spool/flatfile/spool_flatfile_close_file()
*******************************************************************************/
int 
spool_flatfile_open_file(lList **answer_list,
                         const spool_flatfile_destination destination,
                         const char *filepath_in,
                         const char **filepath_out)
{
   int fd = -1;

   DENTER(FLATFILE_LAYER, "spool_flatfile_open_file");

   *filepath_out = NULL;

   switch (destination) {
      case SP_DEST_SPOOL:
         /* check file name */
         if (filepath_in == NULL || filepath_in[0] == '\0') {
            answer_list_add_sprintf(answer_list, STATUS_ESEMANTIC, 
                                    ANSWER_QUALITY_ERROR, 
                                    "%s", MSG_INVALIDFILENAMENULLOREMPTY);
            DRETURN(fd);
         }
   
         /* open file */
         fd = open(filepath_in, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
         if (fd == -1) {
            answer_list_add_sprintf(answer_list, STATUS_EDISK,
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_ERROROPENINGFILEFORWRITING_SS, 
                                    filepath_in, strerror(errno));
         }

         *filepath_out = strdup(filepath_in);
         break;
      case SP_DEST_TMP:
         {
            char buffer[SGE_PATH_MAX];
            dstring tmp_name_error = DSTRING_INIT;

            /* get filename for temporary file, pass buffer to make it
             * thread safe.
             */
            errno = 0;
            fd = sge_mkstemp(buffer, sizeof(buffer), &tmp_name_error);
            sge_dstring_free(&tmp_name_error);
            if ((fd == -1) || (chmod(buffer,
                                     S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|
                                     S_IROTH|S_IWOTH))) {
               answer_list_add_sprintf(answer_list, STATUS_EDISK,
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_ERROROPENINGFILEFORWRITING_SS,
                                       filepath_in, strerror(errno));
               DRETURN(fd);
            }
            *filepath_out = strdup(buffer);
         }   
         break;
      case SP_DEST_STDOUT:
         fd = 1;

         fflush(stdout);
         *filepath_out = strdup("<stdout>");
         break;
      case SP_DEST_STDERR:
         fd = 2;

         fflush(stderr);
         *filepath_out = strdup("<stderr>");
         break;
   }

   DRETURN(fd);
}

/****** spool/flatfile/spool_flatfile_close_file() **********************
*  NAME
*     spool_flatfile_close_file() -- close spool file / stream
*
*  SYNOPSIS
*     static bool 
*     spool_flatfile_close_file(lList **answer_list, FILE *file, 
*                               const char *filepath, 
*                               const spool_flatfile_destination destination) 
*
*  FUNCTION
*     Closes the given file or stream.
*     Streams (stdout, strerr) are not really closed, but just unlocked.
*
*  INPUTS
*     lList **answer_list                          - to return errors
*     FILE *file                                   - file handle to close
*     const char *filepath                         - filename
*     const spool_flatfile_destination destination - destination
*
*  RESULT
*     static bool - true on success, else false
*
*  SEE ALSO
*     spool/flatfile/spool_flatfile_open_file()
*******************************************************************************/
static bool 
spool_flatfile_close_file(lList **answer_list, int fd, const char *filepath,
                          const spool_flatfile_destination destination)
{
   switch (destination) {
      case SP_DEST_TMP:
      case SP_DEST_SPOOL:
         if (close(fd) == -1) {
            fd = -1;
            answer_list_add_sprintf(answer_list, STATUS_EDISK,
                                    ANSWER_QUALITY_ERROR,
                                    MSG_ERRORCLOSINGFILE_SS,
                                    filepath != NULL ? filepath : "<null>",
                                    strerror(errno));
            return false;
         }
         break;
      case SP_DEST_STDOUT:
         fflush(stdout);
         break;
      case SP_DEST_STDERR:
         fflush(stderr);
         break;
   }

   return true;
}

static const char *
spool_flatfile_write_data(lList **answer_list, const void *data, int data_len, 
                          const spool_flatfile_destination destination, 
                          const char *filepath)
{
   int fd = -1;
   const char *result = NULL;

   DENTER(FLATFILE_LAYER, "spool_flatfile_write_data");

   SGE_CHECK_POINTER_NULL(data, answer_list);

   PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);

   /* open/get filehandle */
   fd = spool_flatfile_open_file(answer_list, destination, filepath, &result);
   if (fd == -1) {
      /* message generated in spool_flatfile_open_file */
      PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      DRETURN(NULL);
   }

   /* write data */
   if (write(fd, (char *)data, strlen((char *)data)) != data_len)
   {
      answer_list_add_sprintf(answer_list, STATUS_EDISK,
                              ANSWER_QUALITY_ERROR, MSG_ERRORWRITINGFILE_SS,
                              result, strerror(errno));
      spool_flatfile_close_file(answer_list, fd, result, destination);
      unlink(filepath);
      sge_free(&result);
      PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      DRETURN(NULL);
   }

   /* close file */
   if (!spool_flatfile_close_file(answer_list, fd, result, destination)) {
      /* message generated in spool_flatfile_close_file */
      unlink(filepath);
      sge_free(&result);
      PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      DRETURN(NULL);
   }
   PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);

   DRETURN(result);
}

static bool 
spool_flatfile_write_object_fields(lList **answer_list, const lListElem *object,
                                   dstring *buffer, 
                                   const spool_flatfile_instr *instr,
                                   const spooling_field *fields, bool recurse,
                                   bool root)
{
   int i;
   bool first_field = true;
   dstring field_name = DSTRING_INIT;
   dstring field_buffer = DSTRING_INIT;
   const lDescr *descr;

   int supress_field;
   char field_delimiter;
   char name_value_delimiter;
   bool show_field_names;
   bool record_start_end_newline;
   bool show_empty_fields;
   bool align_data;

   DENTER(FLATFILE_LAYER, "spool_flatfile_write_object_fields");

   SGE_CHECK_POINTER_FALSE(object, answer_list);
   SGE_CHECK_POINTER_FALSE(buffer, answer_list);
   SGE_CHECK_POINTER_FALSE(instr, answer_list);
   SGE_CHECK_POINTER_FALSE(fields, answer_list);

   supress_field = instr->recursion_info.supress_field;
   field_delimiter = instr->field_delimiter;
   name_value_delimiter = instr->name_value_delimiter;
   show_field_names = instr->show_field_names;
   record_start_end_newline = instr->record_start_end_newline;
   show_empty_fields = instr->show_empty_fields;
   align_data = instr->align_data;
 
   descr = lGetElemDescr(object);
 
   /* loop over all fields */
   for (i = 0; fields[i].nm != NoName; i++) {
      const int nm = fields[i].nm;
      const char *name = fields[i].name;
      const char *value = NULL;
      int pos;


      /* If this isn't the root node, and this is the field we're supposed to
       * supress, skip it. */
      if (!root && (supress_field == nm)) {
         continue;
      }
      
      pos = lGetPosInDescr(descr, fields[i].nm);
      if (pos < 0) {
         answer_list_add_sprintf(answer_list, STATUS_ESEMANTIC,
                                 ANSWER_QUALITY_WARNING, 
                                 MSG_NMNOTINELEMENT_S,
                                 lNm2Str(nm));
         continue;
      }

      sge_dstring_clear(&field_name);
      sge_dstring_clear(&field_buffer);
         
      /* if not first field, output field_delimiter */
      if (!first_field || recurse) {
         sge_dstring_append_char(&field_buffer, field_delimiter);
      } 

      /* if show_field_names, output field name */
      if (show_field_names && (name != NULL)) {

         /* if record_start_end_newline indent three spaces */
         if (record_start_end_newline) {
            sge_dstring_append(&field_name, "   ");
         }

         /* respect alignment */
         if (fields[i].width > 0) {
            sge_dstring_sprintf_append(&field_name, "%-*s", fields[0].width,
                                       name);
         } else {
            sge_dstring_append(&field_name, name);
         }
         sge_dstring_append_dstring(&field_buffer, &field_name);

         /* output name-value delimiter */
         if (name_value_delimiter != '\0') {
            sge_dstring_append_char(&field_buffer, name_value_delimiter);
         } else {
            sge_dstring_append_char(&field_buffer, ' ');
         }
      }

      /* output value */
      if (fields[i].write_func != NULL) {
         if (fields[i].write_func(object, nm, &field_buffer, answer_list) == 0
             && !show_empty_fields) {
            continue;
         }
      } else if (mt_get_type(descr[pos].mt) == lListT) {
         const spool_flatfile_instr *sub_instr = NULL;
         const bool recurse_field = (instr->recursion_info.recursion_field
                                       == nm) ? true : false;
         const spooling_field *sub_fields = NULL;
         
         sub_instr = instr;
         sub_fields = fields;
         if (!recurse_field) {
            sub_instr = (spool_flatfile_instr *)fields[i].clientdata;

            /* if no field specific sub_instr exists, use default from inst */
            if (sub_instr == NULL) {
               sub_instr = instr->sub_instr;
            }

            sub_fields = fields[i].sub_fields;
         }

         if (sub_instr == NULL || sub_fields == NULL) {
            if (sub_instr->show_empty_fields) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_WARNING, 
                                       MSG_DONTKNOWHOWTOSPOOLSUBLIST_SS,
                                       lNm2Str(nm), SGE_FUNC);
               sge_dstring_append(&field_buffer, NONE_STR);
            } else {
               continue;
            }
         } else {
            lList *sub_list = lGetList(object, nm);      

            /* Bugfix: Issuezilla #1137
             * If a list field has no name and no value, it should be ignored
             * altogether.  I'm using this funny "if" arrangement to get the
             * desired effect with the minimum amount of overhead. */
            if ((sub_list == NULL) || (lGetNumberOfElem(sub_list) == 0)) {
               if (name != NULL) {
                  if (sub_instr->show_empty_fields) {
                     sge_dstring_append(&field_buffer, NONE_STR);
                  } else {
                     continue;
                  }
               }
            } else if (!spool_flatfile_write_list_fields(answer_list, sub_list, 
                                                    &field_buffer, sub_instr,
                                                    sub_fields,
                                                    recurse_field, sge_dstring_get_string(&field_name))) {
               /* error handling has been done in spool_flatfile_write_list_fields */
               continue;
            }
         }
      } else {
         dstring tmp_buffer = DSTRING_INIT;

         value = object_append_field_to_dstring(object, answer_list, 
                                                &tmp_buffer, fields[i].nm, 
                                                '\0');
         if (!show_empty_fields && !strcasecmp(value, NONE_STR)) {
            sge_dstring_free(&tmp_buffer);
            continue;
         } else if (align_data && (fields[i + 1].nm != NoName)) {
            /* If asked to align the data and this isn't the last field, pad with
             * spaces. Testing for i+1 is always valid because the last element
             * is NoName, and we can't get to here if the current element is
             * NoName, i.e. we're at the last element. */
            sge_dstring_sprintf_append(&field_buffer, "%-*s", fields[i].width,
                                       value);
         } else {
            sge_dstring_append(&field_buffer, value);
         }
         sge_dstring_free(&tmp_buffer);
      }
      sge_dstring_append_dstring(buffer, &field_buffer);

      first_field = false;
   }

   sge_dstring_free(&field_name);
   sge_dstring_free(&field_buffer);

   DRETURN(true);
}

static bool
spool_flatfile_write_list_fields(lList **answer_list, const lList *list, 
                                 dstring *buffer, 
                                 const spool_flatfile_instr *instr,
                                 const spooling_field *fields, bool recurse, const char *list_name)
{
   lListElem *ep;
   bool first = true;
   bool first_start = true;
   const spooling_field *my_fields = fields;
   bool ret = true;

   bool field_delimiter;
   bool record_delimiter;
   bool record_start;
   bool record_end;
   bool ignore_list_name;
   bool record_start_end_newline;

   DENTER(FLATFILE_LAYER, "spool_flatfile_write_list_fields");

   SGE_CHECK_POINTER_FALSE(list, answer_list);
   SGE_CHECK_POINTER_FALSE(buffer, answer_list);
   SGE_CHECK_POINTER_FALSE(instr, answer_list);

   field_delimiter = (instr->field_delimiter == '\0')? false: true;
   record_delimiter = (instr->record_delimiter == '\0')? false: true;
   record_start = (instr->record_start == '\0')? false: true;
   record_end = (instr->record_end == '\0')? false: true;
   ignore_list_name = instr->ignore_list_name;
   record_start_end_newline = instr->record_start_end_newline;
  
   /* If recursion is enabled, only write out a single id field for each element
    * in the list.  Then recursively write out the entire element. */
   if (recurse) {
      my_fields = get_recursion_field_list(instr);
   }

   for_each(ep, list) {
      /* from second record on write record delimiter */
      if (!first) {
         if (ignore_list_name && list_name != NULL) {
            sge_dstring_append(buffer, list_name);
            if (field_delimiter) {
               sge_dstring_append_char(buffer, instr->field_delimiter);
            }
         }
         if (record_delimiter) {
            sge_dstring_append_char(buffer, instr->record_delimiter);
         }
      } else {
         first = false;
      }

      /* if record_start, output record_start */
      if (record_start) {
         if (record_start_end_newline && !first_start) {
            sge_dstring_append_char(buffer, '\n');
         } else {
            first_start = false;
         }

         sge_dstring_append_char(buffer, instr->record_start);

         if (record_start_end_newline) {
            sge_dstring_append_char(buffer, '\n');
         }
      }
         
      if (!spool_flatfile_write_object_fields(answer_list, ep, buffer, 
                                              instr, my_fields, false, false)) {
         sge_dstring_free(buffer); 
         ret = false;
      } else if (record_end) {
         /* if record_end, output record end, else record_delimiter */
         sge_dstring_append_char(buffer, instr->record_end);
      }
   }

   /* Recursively write out the sub objects. */
   if (recurse) {

      if (ret && instr->recursion_info.recursion_field != NoName) {
         for_each(ep, list) {         
            if (!spool_flatfile_write_object_fields(answer_list, ep, buffer, 
                                                    instr, fields, true, false)) {
               sge_dstring_free(buffer); 
               ret = false;
               break;
            }
         }
      }
      sge_free(&my_fields);
   }
   
   DRETURN(ret);
}

/****** spool/flatfile/spool_flatfile_read_object() *********************
*  NAME
*     spool_flatfile_read_object() -- read an object from file / stream
*
*  SYNOPSIS
*     lListElem * 
*     spool_flatfile_read_object(lList **answer_list, const lDescr *descr, 
*                                const spooling_field *fields_in, 
*                                int fields_out[], 
*                                const spool_flatfile_instr *instr, 
*                                const spool_flatfile_format format, 
*                                FILE *file, const char *filepath) 
*
*  FUNCTION
*     Read an object of type <descr> from the stream <file> or a file described
*     by <filepath>.
*
*     <fields_in> names the fields that can be contained in the input.
*
*     The fields actually read are stored in <fields_out>.
*
*     <format> and <instr> describe the data format to expect.
*
*  INPUTS
*     lList **answer_list                - to report any errors
*     const lDescr *descr                - object type to read
*     lListElem *root                    - 
*     const spooling_field *fields_in    - fields that can be contained in input
*     int fields_out[]                   - field actually read
*     const spool_flatfile_instr *instr  - spooling instruction
*     const spool_flatfile_format format - spooling format
*     FILE *file                         - filehandle to read from
*     const char *filepath               - if <file> == NULL, <filepath> is 
*                                          opened
*
*  RESULT
*     lListElem * - on success the read object, else NULL
*
*  SEE ALSO
*     spool/flatfile/spool_flatfile_write_object()
*     spool/flatfile/spool_flatfile_read_list()
*******************************************************************************/
lListElem * 
spool_flatfile_read_object(lList **answer_list, const lDescr *descr, 
                           lListElem *root, const spooling_field *fields_in,
                           int fields_out[], bool parse_values,
                           const spool_flatfile_instr *instr,
                           const spool_flatfile_format format,
                           FILE *file,
                           const char *filepath)
{
   bool file_opened = false;
   int token;
   lListElem *object = NULL;
   const spooling_field *fields = fields_in;
   spooling_field *my_fields = NULL;

   DENTER(FLATFILE_LAYER, "spool_flatfile_read_object");

   SGE_CHECK_POINTER_NULL(descr, answer_list);
   SGE_CHECK_POINTER_NULL(instr, answer_list);

   /* if no file handle is passed, try to open file for reading */
   if (file == NULL) {
      SGE_CHECK_POINTER_NULL(filepath, answer_list);

      if (sge_is_file(filepath) == 0) {
         answer_list_add_sprintf(answer_list, STATUS_EDISK,
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_ERROROPENINGFILEFORREADING_SS,
                                 filepath, strerror(errno));
         DRETURN(NULL);
      }

      file = fopen(filepath, "r");
      if (file == NULL) {
         answer_list_add_sprintf(answer_list, STATUS_EDISK,
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_ERROROPENINGFILEFORREADING_SS,
                                 filepath, strerror(errno));
         DRETURN(NULL);
      }
      file_opened = true;
   }

   /* initialize scanner */
   token = spool_scanner_initialize(file);

   if (token == SPFT_ERROR_NO_MEMORY) {
      /* messages generated in spool_get_fields_to_spool */
      spool_scanner_shutdown();
      answer_list_add_sprintf(answer_list, STATUS_EDISK,
                              ANSWER_QUALITY_ERROR, 
                              "%s", MSG_SGETEXT_NOMEM);
      if (file_opened) {
         FCLOSE(file);
      }
      DRETURN(NULL);
   }

   /* if no fields are passed, retrieve them from instructions */
   if (fields == NULL) {
      my_fields = spool_get_fields_to_spool(answer_list, descr, 
                                         instr->spool_instr);
      if (my_fields == NULL) {
         /* messages generated in spool_get_fields_to_spool */
         spool_scanner_shutdown();
         if (file_opened) {
            FCLOSE(file);
         }
         DRETURN(NULL);
      }

      fields = my_fields;
   }

   object = _spool_flatfile_read_object(answer_list, descr, root, instr, 
                                        fields, fields_out, &token, NULL,
                                        parse_values);
   if (object == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_FLATFILE_ERROR_READINGFILE_S, 
                              filepath);
   }
   spool_scanner_shutdown();

   /* if we opened the file, we also have to close it */
   if (file_opened) {
      FCLOSE(file);
   }

   /* if we created our own fields */
   if (my_fields != NULL) {
      my_fields = spool_free_spooling_fields(my_fields);
   }

   DRETURN(object);
FCLOSE_ERROR:
   lFreeElem(&object);
   DRETURN(NULL);
}

static lListElem *
_spool_flatfile_read_object(lList **answer_list, const lDescr *descr, 
                            lListElem *root, const spool_flatfile_instr *instr, 
                            const spooling_field *fields, int fields_out[],
                            int *token, const char *end_token,
                            bool parse_values)
{
   lListElem *object = NULL;
   int *my_fields_out = NULL;

   /* BUGFIX: Issuezilla #732
    * If we're not given a fields_out array, create one for internal use. */
   if (fields_out != NULL) {
      my_fields_out = fields_out;
   }
   else {
      my_fields_out = (int *)malloc ((spool_get_number_of_fields (fields) + 1) *
                                                                  sizeof (int));
      my_fields_out[0] = NoName;
   }

   _spool_flatfile_read_live_object(answer_list, &object, descr, root, instr,
                                    fields, my_fields_out, token, end_token,
                                    parse_values);

   if (answer_list_has_error(answer_list)) {
      lFreeElem(&object);
   }

   if (fields_out == NULL) {
      sge_free(&my_fields_out);
   }
   
   return object;
}

static void
_spool_flatfile_read_live_object(lList **answer_list, lListElem **object,
                                 const lDescr *descr, lListElem *root,
                                 const spool_flatfile_instr *instr, 
                                 const spooling_field *fields, int fields_out[],
                                 int *token, const char *end_token,
                                 bool parse_values)
{
   int field_index = -1;

   dstring buffer = DSTRING_INIT;
   bool stop = false;

   DENTER(FLATFILE_LAYER, "_spool_flatfile_read_live_object");
   
FF_DEBUG("reading object");

   while (*token != 0 && !stop) {
      int nm = NoName;
      int pos, type;
      bool field_end  = false;
      bool record_end = false;
      bool field_has_name = false;
        
FF_DEBUG("reading field");

      /* check for list end condition */
      if (is_delimiter(*token) && check_end_token(end_token, *spool_text)) {
FF_DEBUG("detected end_token");
         stop = true;
         continue;
      }

      /* skip newlines */
      while (*token == SPFT_NEWLINE) {
FF_DEBUG("skip newline");
         /* DT: TODO: There is a potential problem here that just hasn't come up
          * yet.  If an object type with a NULL field name were to disable value
          * parsing, it wouldn't work because the field name is always expected
          * to be a WORD.  Therefore, with a NULL field name value parsing is
          * always assumed to be false.
          * To fix this, we would need to allow text to be pushed back onto the
          * spool_text so that we could read the name as a WORD, and if it turns
          * out to be a field with a NULL name with value parsing disabled we
          * could push the WORD back onto the spool_text and read it again with
          * value parsing disabled. */
         *token = spool_lex();
      }

      /* check for eof */
      if (*token == 0) {
FF_DEBUG("eof detected");
         continue;
      }

      if (*object == NULL) {
         *object = lCreateElem(descr);
         if (*object == NULL) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    "%s", MSG_ERRORCREATINGOBJECT);
            stop = true;
            continue;
         }
      }

      /* read field name from file or from field list */
      if (instr->show_field_names) {
         /* read field name from file */
FF_DEBUG("read field name");
         if (*token != SPFT_WORD) {
            answer_list_add_sprintf(answer_list, STATUS_ESYNTAX,
                                    ANSWER_QUALITY_ERROR,
                                    MSG_PARSINGOBJECTNOATTRIBUTE_D,
                                    spool_line);
            stop = true;
            continue;
         }
   
         /* search field name in field array */
         for (field_index = 0; fields[field_index].nm != NoName; field_index++) {
            if (sge_strnullcmp(spool_text, fields[field_index].name) == 0) {
               nm = fields[field_index].nm;
               break;
            }
         }

         /* Not found -> Search for a nameless field in the field array */
         if (nm == NoName) {
            field_has_name = false;
            
            for (field_index = 0; fields[field_index].nm != NoName; field_index++) {
               if (fields[field_index].name == NULL) {
                  nm = fields[field_index].nm;
                  break;
               }
            }
            
            /* not found -> error */
            if (nm == NoName) {
               answer_list_add_sprintf(answer_list, STATUS_ESYNTAX,
                                       ANSWER_QUALITY_ERROR,
                                       MSG_UNKNOWNATTRIBUTENAME_S, spool_text);
               stop = true;
               continue;
            }
         } else {
            field_has_name = true;
         }

         if (field_has_name) {
            if (isspace(instr->name_value_delimiter)) {
FF_DEBUG("return whitespace");
               spool_return_whitespace = true;
            }
            *token = spool_lex();
            spool_return_whitespace = false;

            /* do we have a special delimiter between attrib name and value? */
            if (instr->name_value_delimiter != '\0') {
FF_DEBUG("read name_value_delimiter");
               if (!is_delimiter(*token) || 
                   *spool_text != instr->name_value_delimiter) {
                  answer_list_add_sprintf(answer_list, STATUS_ESYNTAX,
                                          ANSWER_QUALITY_ERROR,
                                          MSG_PARSINGOBJECTNAMEVALUESEP_SD,
                                          output_delimiter(instr->name_value_delimiter),
                                          spool_line);
                  stop = true;
                  continue;
               }

               if (!parse_values) {
                  spool_finish_line = 1;
               }

               *token = spool_lex();
               spool_finish_line = 0;
            }
         }
      } else {
FF_DEBUG("eval next field");
         /* get next field from field array */   
         /* field_index starts at -1 so that this works on the first pass */
         nm = fields[++field_index].nm;

         /* last field reached */
         if (nm == NoName) {
            stop = true;
            continue;
         }
      }

FF_DEBUG(lNm2Str(nm));

      /* check if nm is an attribute of current object type */
      pos = lGetPosInDescr(descr, nm);
      if (pos < 0) {
         answer_list_add_sprintf(answer_list, STATUS_ESYNTAX,
                                 ANSWER_QUALITY_ERROR,
                                 MSG_ATTRIBUTENOTINOBJECT_S,
                                 (fields[field_index].name != NULL) ?
                                 fields[field_index].name : lNm2Str(nm));
         stop = true;
         continue;
      }

      /* if list of read fields is requested in fields_out, store this info */
      if (fields_out != NULL) {
         /* BUGFIX: Issuezilla #732
          * If add_nm_to_set returns -1, it means that the field already exists
          * in the field list.  In this case, return an error. */
         if (add_nm_to_set(fields_out, nm) == -1) {
            answer_list_add_sprintf(answer_list, STATUS_ESYNTAX,
                                    ANSWER_QUALITY_ERROR,
                                    MSG_FLATFILE_DUPLICATEATTRIB_S,
                                    (fields[field_index].name != NULL) ?
                                    fields[field_index].name : lNm2Str(nm));
            stop = true;
            continue;
         }
      }

      type = mt_get_type(descr[pos].mt);
      
      /* now read the data */
      if ((type != lListT) || (fields[field_index].read_func != NULL)) {
         bool found_value = false;
         
         /* read field data and append until field/record end */
         sge_dstring_clear(&buffer);
         spool_return_whitespace = true;

         while (*token != 0 && !field_end && !record_end) {
FF_DEBUG("reading value");
            if (is_delimiter(*token)) {
               /* check for field end */
               if (*spool_text == instr->field_delimiter) {
FF_DEBUG("detected field_delimiter");
                  field_end = true;
                  continue;
               }
               /* check for record end */
               if (*spool_text == instr->record_end) {
FF_DEBUG("detected record_end");
                  record_end = true;
                  continue;
               }
               /* check for record end */
               if (*spool_text == instr->record_delimiter) {
FF_DEBUG("detected record_delimiter");
                  record_end = true;
                  continue;
               }
               /* check for external end condition */
               if (check_end_token(end_token, *spool_text)) {
FF_DEBUG("detected end_token");
                  record_end = true;
                  continue;
               }
               /* if it's a space that doesn't have a special meaning,
                * ignore it */
               if (!found_value && (*spool_text == ' ')) {
                  *token = spool_lex();
               }
            }
            
            /* store data */
            sge_dstring_append(&buffer, spool_text);
            found_value = true;
            
            *token = spool_lex();
         }
         spool_return_whitespace = false;
         
         if (fields[field_index].read_func == NULL) {
                        
            if(type == lUlongT) {            
               char *end_ptr = NULL;
               double dbl_value;
               
               dbl_value = strtod(sge_dstring_get_string(&buffer), &end_ptr);
               if ( dbl_value < 0) {
                  answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                       ANSWER_QUALITY_ERROR,
                                       MSG_MUST_BE_POSITIVE_VALUE_S,
                                       fields[field_index].name);
                  sge_dstring_free(&buffer);
                  DRETURN_VOID;
               }
            }
            if (object_parse_field_from_string(*object, answer_list, nm, 
                                        sge_dstring_get_string(&buffer)) == 0) {
               stop = true;
               continue;
            }
         } else {
            if (fields[field_index].read_func(*object, nm,
                                               sge_dstring_get_string(&buffer),
                                               answer_list) == 0) {
               stop = true;
               continue;
            }
         }
      } else { /* if (type == lListT) */
         lList *list;
         const lDescr *sub_descr;

FF_DEBUG("reading list");
         /* check for empty sublist */
         if (*token == SPFT_WORD && 
             sge_strnullcasecmp(spool_text, NONE_STR) == 0) {
FF_DEBUG("empty list");
            *token = spool_lex();

            if (instr->recursion_info.recursion_field == nm) {
               record_end = true;
            }
            /* check for field end - we have to skip it later */
            else if (is_delimiter(*token) &&
                     (*spool_text == instr->field_delimiter)) {
               field_end = true;
            }
         } else {
            /* parse sublist - do we have necessary info */
            const spool_flatfile_instr *sub_instr = (spool_flatfile_instr *)fields[field_index].clientdata;
            /* if no field specific instr exists, take default sub_instr */
            if (sub_instr == NULL) {
               sub_instr = instr->sub_instr;
            }
            
            if (sub_instr == NULL || 
               fields[field_index].sub_fields == NULL) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                       ANSWER_QUALITY_ERROR,
                                       MSG_DONTKNOWHOWTOHANDLELIST_S, 
                                       lNm2Str(nm));
               stop = true;
               continue;
            }

            /* get / check type of sublist */
            sub_descr = object_get_subtype(nm);
            if (sub_descr == NULL)  {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                       ANSWER_QUALITY_ERROR,
                                       MSG_UNKNOWNOBJECTTYPEFOR_SS, 
                                       lNm2Str(nm), SGE_FUNC);
               stop = true;
               continue;
            }
           
            /* read sublist */
            {
               char new_end_token[MAX_STRING_SIZE];

               get_end_token(new_end_token, MAX_STRING_SIZE, end_token,
                             instr->field_delimiter);
               /* We're passing in NULL for the fields_out parameter
                * because we don't care about the fields read from sub
                * lists. */
               list = _spool_flatfile_read_list(answer_list, sub_descr, 
                                                sub_instr, 
                                                fields[field_index].sub_fields, 
                                                NULL, token, 
                                                new_end_token, parse_values,
                                                fields[field_index].name);
            
               lSetPosList(*object, pos, list);
               
               if (instr->recursion_info.recursion_field == nm) {
                  lListElem *ep = NULL;
                  lListElem *rep = root;

                  if (rep == NULL) {
                     rep = *object;
                  }
            
                  if (instr->field_delimiter != '\0') {
                     if (!is_delimiter(*token) &&
                         *spool_text != instr->field_delimiter) {
                        answer_list_add_sprintf(answer_list, STATUS_ESYNTAX,
                                       ANSWER_QUALITY_ERROR,
                                       MSG_PARSINGOBJECTUNKNOWNTRAILER_DS,
                                       spool_line,
                                       token == 0 ? "<EOF>" : spool_text);
                        stop = true;
                        continue;
                     }
FF_DEBUG("skipping field delimiter");
                     *token = spool_lex();
                  }

                  /* Read in the full element for each element in the list */
                  for_each (ep, list) {
                     /* We're passing in NULL for the fields_out parameter
                      * because we don't care about the fields read from sub
                      * lists. */
                     _spool_flatfile_read_live_object(answer_list, &ep, descr,
                                                      rep, instr, fields, NULL,
                                                      token, end_token,
                                                      parse_values);
                  }
                  
                  record_end = true;
               }
            }
         }
      }
FF_DEBUG("after parsing value");

      /* check for eof */
      if (*token == 0) {
FF_DEBUG("eof detected");
         continue;
      }

      /* check for record_end while parsing value */
      if (record_end) {
         stop = true;
         continue;
      }

      /* check for record end */
      if (instr->record_end != '\0') {
         if (is_delimiter(*token) && *spool_text == instr->record_end) {
FF_DEBUG("detected record_end");
            stop = true;
            continue;
         }
      }

      /* check for record delimiter */
      if (instr->record_delimiter != '\0') {
         if (is_delimiter(*token) && *spool_text == instr->record_delimiter) {
FF_DEBUG("detected record_delimiter");
            stop = true;
            continue;
         }
      }

      /* if a field end has been detected while parsing a value, skip it 
       * else check for field end.
       */
      if (field_end) {
FF_DEBUG("skipping field delimiter");
         if (!field_has_name && !parse_values) {
            spool_finish_line = 1;
         }

         *token = spool_lex();
         
         spool_finish_line = 0;
      } else {
         if (instr->field_delimiter != '\0') {
            if (!is_delimiter(*token) &&
                *spool_text != instr->field_delimiter) {
               answer_list_add_sprintf(answer_list, STATUS_ESYNTAX,
                                       ANSWER_QUALITY_ERROR,
                                       MSG_PARSINGOBJECTUNKNOWNTRAILER_DS,
                                       spool_line,
                                       token == 0 ? "<EOF>" : spool_text);
               stop = true;
               continue;
            }
FF_DEBUG("skipping field delimiter");
            if (!field_has_name && !parse_values) {
               spool_finish_line = 1;
            }

            *token = spool_lex();

            spool_finish_line = 0;
         }
      }
   } /* while */

FF_DEBUG("after parsing object");

   /* cleanup */
   sge_dstring_free(&buffer);
   DRETURN_VOID;
}

/****** spool/flatfile/spool_flatfile_read_list() ***********************
*  NAME
*     spool_flatfile_read_list() -- read a list from file / stream
*
*  SYNOPSIS
*     lList * 
*     spool_flatfile_read_list(lList **answer_list, const lDescr *descr,
*                              const spooling_field *fields_in, 
*                              int fields_out[], 
*                              const spool_flatfile_instr *instr, 
*                              const spool_flatfile_format format, 
*                              FILE *file, const char *filepath) 
*
*  FUNCTION
*     Read a list of type <descr> from the stream <file> or a file described
*     by <filepath>.
*
*     <fields_in> names the fields that can be contained in the input.
*
*     The fields actually read are stored in <fields_out>.
*
*     <format> and <instr> describe the data format to expect.
*
*  INPUTS
*     lList **answer_list                - to report any errors
*     const lDescr *descr                - list type
*     const spooling_field *fields_in    - fields that can be contained in input
*     int fields_out[]                   - fields actually read
*     const spool_flatfile_instr *instr  - spooling instructions
*     const spool_flatfile_format format - data format
*     FILE *file                         - file to read or NULL if <filepath> 
*                                          shall be opened
*     const char *filepath               - file to open if <file> == NULL
*
*  RESULT
*     lList * - the list read on success, else NULL
*
*  SEE ALSO
*     spool/flatfile/spool_flatfile_write_list()
*     spool/flatfile/spool_flatfile_read_object()
*******************************************************************************/
lList * 
spool_flatfile_read_list(lList **answer_list, const lDescr *descr, 
                         const spooling_field *fields_in, int fields_out[],
                         bool parse_values, const spool_flatfile_instr *instr,
                         const spool_flatfile_format format,
                         FILE *file,
                         const char *filepath)
{
   bool file_opened = false;
   int token = 0;
   lList *list = NULL;
   const spooling_field *fields = fields_in;
   spooling_field *my_fields = NULL;
   const char *end_token = NULL;
   char new_end_token[MAX_STRING_SIZE];

   DENTER(FLATFILE_LAYER, "spool_flatfile_read_list");

   SGE_CHECK_POINTER_NULL(descr, answer_list);
   SGE_CHECK_POINTER_NULL(instr, answer_list);

   /* if no file handle is passed, try to open file for reading */
   if (file == NULL) {
      SGE_CHECK_POINTER_NULL(filepath, answer_list);

      if (sge_is_file(filepath) == 0) {
         answer_list_add_sprintf(answer_list, STATUS_EDISK,
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_ERROROPENINGFILEFORREADING_SS,
                                 filepath, strerror(errno));
         DRETURN(NULL);
      }

      file = fopen(filepath, "r");
      if (file == NULL) {
         answer_list_add_sprintf(answer_list, STATUS_EDISK,
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_ERROROPENINGFILEFORREADING_SS,
                                 filepath, strerror(errno));
         DEXIT;
         return NULL;
      }

      file_opened = true;
   }

   /* initialize scanner */
   token = spool_scanner_initialize(file);

   if (token == SPFT_ERROR_NO_MEMORY) {
      /* messages generated in spool_get_fields_to_spool */
      spool_scanner_shutdown();

      answer_list_add_sprintf(answer_list, STATUS_EDISK,
                              ANSWER_QUALITY_ERROR, 
                              "%s", MSG_SGETEXT_NOMEM);

      if (file_opened) {
         FCLOSE(file);
      }
      DRETURN(NULL);
   }

   /* if no fields are passed, retrieve them from instructions */
   if (fields == NULL) {
      my_fields = spool_get_fields_to_spool(answer_list, descr, 
                                            instr->spool_instr);
      if (my_fields == NULL) {
         /* messages generated in spool_get_fields_to_spool */
         spool_scanner_shutdown();
         if (file_opened) {
            FCLOSE(file);
         }
         DRETURN(NULL);
      }

      fields = my_fields;
   }

   get_end_token(new_end_token, MAX_STRING_SIZE, end_token,
                 instr->record_end);

   list = _spool_flatfile_read_list(answer_list, descr, instr, 
                                    fields, fields_out, &token, new_end_token,
                                    parse_values, NULL);
   spool_scanner_shutdown();

   /* if we opened the file, we also have to close it */
   if (file_opened) {
      FCLOSE(file);
   }

   /* if we created our own fields */
   if (my_fields != NULL) {
      my_fields = spool_free_spooling_fields(my_fields);
   }

   DRETURN(list);
FCLOSE_ERROR:
   lFreeList(&list);
   DRETURN(NULL);
}

static lList *
_spool_flatfile_read_list(lList **answer_list, const lDescr *descr, 
                          const spool_flatfile_instr *instr, 
                          const spooling_field *fields, int fields_out[], 
                          int *token, const char *end_token, bool parse_values, const char *list_name)
{
   bool stop = false;
   bool first_record = true;
   bool end_token_detected = false;
   char new_end_token[MAX_STRING_SIZE];
   lList *list;
   lListElem *object;

   DENTER(FLATFILE_LAYER, "_spool_flatfile_read_list");

   list = lCreateList("list", descr);
   if (list == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                              ANSWER_QUALITY_ERROR,
                              "%s", MSG_ERRORCREATINGLIST);
      DEXIT;
      return NULL;
   }

   if (instr->record_end == '\0') {
      get_end_token(new_end_token, MAX_STRING_SIZE, end_token,
                    instr->record_delimiter);
   } else {
      /* we need no end token, as record_end character is 
       * an explicit end criterium 
       */
      new_end_token[0] = instr->record_end; 
      new_end_token[1] = '\0';
   }

FF_DEBUG("read list");
   /* parse all objects in list */
   while (*token != 0 && !stop) {

      if (instr->ignore_list_name && list_name != NULL) {
         if (strcmp(list_name, spool_text) == 0) {
FF_DEBUG("ignored token");
               *token = spool_lex();
         }
      }
  
      /* check for list end condition */
      if (is_delimiter(*token) && check_end_token(end_token, *spool_text)) {
FF_DEBUG("detected end_token");
         stop = true;
         end_token_detected = true;
         continue;
      }

      /* for subsequent records check record_delimiter */
      if (!first_record) {
         if (instr->record_delimiter != '\0') {
            if (!is_delimiter(*token) || 
                *spool_text != instr->record_delimiter) {
               answer_list_add_sprintf(answer_list, STATUS_ESYNTAX,
                                       ANSWER_QUALITY_ERROR,
                                       MSG_PARSINGLISTBADRECORDSEP_DS,
                                       spool_line,
                                       output_delimiter(instr->record_delimiter));
               stop = true;
               continue;
            }
FF_DEBUG("detected record_delimiter");
            *token = spool_lex();
         }
      }

      /* check for record_start */
      if (instr->record_start != '\0') {
         if (!is_delimiter(*token) ||
            *spool_text != instr->record_start) {
            answer_list_add_sprintf(answer_list, STATUS_ESYNTAX,
                                    ANSWER_QUALITY_ERROR,
                                    MSG_PARSINGLISTBADRECORDSTART_DS,
                                    spool_line,
                                    output_delimiter(instr->record_start));
            stop = true;
            continue;
         }
FF_DEBUG("detected record_start");
         if (instr->record_start_end_newline) {
            *token = spool_lex();
         }
         *token = spool_lex();
      }

      /* read an object */
      object = _spool_flatfile_read_object(answer_list, descr, NULL, instr,
                                           fields, fields_out, token, 
                                           new_end_token, parse_values);

      /* store object */
      if (object != NULL) {
         lAppendElem(list, object);
      } else {
         /* if no object was read due to an error, a message has been
          * created in _spool_flatfile_read_object
          */
         stop = true;
         continue;
      }
      
      /* check for record_end */
      if (instr->record_end != '\0') {
         if (!is_delimiter(*token) ||
            *spool_text != instr->record_end) {
            answer_list_add_sprintf(answer_list, STATUS_ESYNTAX,
                                    ANSWER_QUALITY_ERROR,
                                    MSG_PARSINGLISTBADRECORDEND_DS,
                                    spool_line,
                                    output_delimiter(instr->record_end));
            stop = true;
            continue;
         }
FF_DEBUG("detected record_end");
         if (instr->record_start_end_newline == true) {
            *token = spool_lex();
         }
         *token = spool_lex();
      }

      first_record = false;
   } /* while */

   if (!end_token_detected) {
      *token = spool_lex();
   }

FF_DEBUG("after parsing list");

   /* if no objects could be read, we need no list */
   if (lGetNumberOfElem(list) == 0) {
      lFreeList(&list);
   }

   DEXIT;
   return list;
}

static spooling_field *get_recursion_field_list(const spool_flatfile_instr *instr)
{
   /* Only 2 entries in a recursion field list */
   spooling_field *fields = (spooling_field *)malloc(sizeof(spooling_field) * 2);
   memset(fields, 0, sizeof(spooling_field)*2);
   fields[0].nm = instr->recursion_info.id_field;
   fields[1].nm = NoName;
   
   return fields;
}

int spool_get_unprocessed_field(spooling_field in[], int out[], lList **alpp)
{
   int count = 0;
   
   while (in[count].nm != NoName) {
      int counter = 0;
      
      while (out[counter] != NoName) {
         if (out[counter] == in[count].nm) {
            break;
         }
         
         counter++;
      }
      
      if (out[counter] == NoName) {
         SGE_ADD_MSG_ID(sprintf(SGE_EVENT, 
                        MSG_FLATFILE_ATTRIBISMISSING_S,
                        (in[count].name == NULL) ?
                           lNm2Str(in[count].nm) :
                           in[count].name));
         answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, 
                         ANSWER_QUALITY_ERROR);
         return in[count].nm;
      }
      
      count++;
   }   
   
   return NoName;
}

int spool_get_number_of_fields(const spooling_field fields[])
{
   int count = 0;
   
   while (fields[count++].nm != NoName);
   
   return count;
}

/****** spool/flatfile/spool_flatfile_add_line_breaks() ************************
*  NAME
*     spool_flatfile_add_line_breaks() -- breaks up long lines by splitting on
*                                         commas and whitespace and inserting
*                                         backslashes at the ends of broken
*                                         lines
*
*  SYNOPSIS
*     static void spool_flatfile_add_line_breaks (dstring *buffer)
*
*  FUNCTION
*     Splits lines greater than MAX_LINE_LENGTH on commas and whitespace.  It
*     tries to choose the most convenient place to insert the break.  The break
*     consists of a space followed by a backslash followed by a new line
*     followed by enough spaces to indent the next line to the beginning of the
*     second word in the first line.  Lines are considered to be demarkated by
*     newlines.
*
*  INPUTS
*     dstring *buffer - The output to be split.  The dstring will be cleared,
*                       and the new output will be stored in it.
*
*******************************************************************************/
static void spool_flatfile_add_line_breaks(dstring *buffer)
{
   int index = 0;
   int word = 0;
   const char *tmp_orig = NULL;
   char *orig = NULL;
   char *strp = NULL;
   char str_buf[MAX_STRING_SIZE];
   char *indent_str = NULL;
   bool first_line = true;

   tmp_orig = sge_dstring_get_string(buffer);
   
   /* This happens when qconf -aconf is used. */
   if (tmp_orig == NULL) {
      return;
   }
   
   orig = strdup(tmp_orig);
   strp = orig;
   
   sge_dstring_clear(buffer);

   str_buf[0] = '\0';

   /* Loop as long as the string is longer than the max length. */
   while (strlen(strp) > MAX_LINE_LENGTH - word) {
      /* Account for newlines */
      char *newlp = strchr(strp, '\n');
      index = (newlp - strp) / sizeof (char);
      
      if ((newlp != NULL) && (index <= MAX_LINE_LENGTH - word)) {
         strncpy(str_buf, strp, index + 1);
         str_buf[index + 1] = '\0';
         sge_dstring_append(buffer, str_buf);

         strp = newlp + 1;

         /* Reset to the first line */
         first_line = true;
         
         if (indent_str != NULL) {
            sge_free(&indent_str);
            word = 0;
         }
         
         continue;
      }
      
      /* Build the indent string by finding the beginning of the second word in
       * the line. */
      if (indent_str == NULL) {
         /* Word should always be 0 when we enter this function, but I'll set
          * it again, just to be sure. */
         word = 0;
         
         /* Find the first whitespace or equals */
         while (!isspace(strp[word]) && (strp[word] != '=')) {
            word++;
         }
         
         /* Now find the next non-whitespace, non-equals to mark the beginning
          * of the second word.  I am assuming this will always work because
          * with the current file formats it does always work. */
         while (isspace(strp[word]) || (strp[word] == '=')) {
            word++;
         }
         
         indent_str = (char *)malloc((word + 4) * sizeof (char));
         indent_str[0] = ' ';
         indent_str[1] = '\\';
         indent_str[2] = '\n';
         memset(indent_str + 3, ' ', word);
         indent_str[word + 3] = '\0';
      }
      
      /* Remove any leading spaces */
      if (!first_line) {
         while (isspace(*strp)) {
            strp++;
         }
      }
      
      /* Break on the last available whitespace or comma */
      /* We have to account for the fact that on all lines after the first, the
       * line length is decreased by the indentation. */
      /* On the first line, we have to start looking at the end of the line and
       * stop looking before we reach the delimiter before the second word.  In
       * both cases, we start two charcters from the "end" so that there's
       * room for a space and a backslash. */
      if (first_line) {
         for (index = MAX_LINE_LENGTH - 2; index > word; index--) {
            if (isspace(strp[index]) || 
                ((index < MAX_LINE_LENGTH - 2) && (strp[index] == ','))) {
               break;
            }
         }
      }
      /* On later lines, we start looking at the end of the line, adjusted for
       * the amount of space we will indent it, and we stop looking at the
       * second character on the line, since having the first character as a
       * break point is useless. */
      else {
         for (index = MAX_LINE_LENGTH - word - 2; index >= 1; index--) {
            if (isspace(strp[index]) ||
                ((index < MAX_LINE_LENGTH - word - 2) &&
                 (strp[index] == ','))) {
               break;
            }
         }
      }
      
      if (isspace(strp[index])) {
         strncpy(str_buf, strp, index);
         str_buf[index] = '\0';
         sge_dstring_append(buffer, str_buf);
         sge_dstring_append(buffer, indent_str);

         strp += index + 1;
         
         first_line = false;
         continue;
      } else if (strp[index] == ',') {
         strncpy(str_buf, strp, index + 1);
         str_buf[index + 1] = '\0';
         sge_dstring_append(buffer, str_buf);
         sge_dstring_append(buffer, indent_str);

         strp += index + 1;
         
         first_line = false;
         continue;
      } else {
         /* Break on the first whitespace past the end of the line */
         /* Break on the last available whitespace or comma */
         /* We have to account for the fact that on all lines after the first, the
          * line length is decreased by the indentation. */
         /* On the first line, we start looking at the end of the line. */
         if (first_line) {
            index = MAX_LINE_LENGTH - 2;

            while ((index < MAX_STRING_SIZE - 1) && (strp[index] != '\0')) {
               if (isspace(strp[index]) || (strp[index] == ',') ||
                   (strp[index] == '\n')) {
                  break;
               }

               index++;
            }
         } else {
         /* On later lines, we start looking at the end of the line, adjusted for
          * the amount of space we will indent it. */
            index = MAX_LINE_LENGTH - word - 2;

            while ((index < MAX_STRING_SIZE - 1) && (strp[index] != '\0')) {
               if (isspace(strp[index]) || (strp[index] == ',') ||
                   (strp[index] == '\n')) {
                  break;
               }

               index++;
            }
         }

         if (strp[index] == '\n') {
            strncpy(str_buf, strp, index + 1);
            str_buf[index + 1] = '\0';
            sge_dstring_append(buffer, str_buf);

            strp += index + 1;
            
            /* Reset to first line */
            first_line = true;
            word = 0;

            /* Release the indent string because each line may have different
             * indentation */
            sge_free(&indent_str);
            continue;
         } else if (isspace (strp[index])) {
            strncpy(str_buf, strp, index);
            str_buf[index] = '\0';
            sge_dstring_append(buffer, str_buf);
            sge_dstring_append(buffer, indent_str);

            strp += index + 1;
            
            first_line = false;
            continue;
         } else if (strp[index] == ',') {
            strncpy(str_buf, strp, index + 1);
            str_buf[index + 1] = '\0';
            sge_dstring_append(buffer, str_buf);
            sge_dstring_append(buffer, indent_str);

            strp += index + 1;

            first_line = false;
            continue;
         } else if (index == MAX_STRING_SIZE - 1) {
            strncpy(str_buf, strp, MAX_STRING_SIZE - 1);
            str_buf[MAX_STRING_SIZE - 1] = '\0';
            sge_dstring_append(buffer, str_buf);
            
            strp += MAX_STRING_SIZE - 1;
            
            continue;
         } else {
            /* There's no spaces or commas, and hence nothing we can do. */
            break; /* while */
         }
      } /* else */
   } /* while */

   sge_dstring_append(buffer, strp);
   
   if (indent_str != NULL) {
      sge_free(&indent_str);
   }
   
   sge_free(&orig);
   
   return;
}
