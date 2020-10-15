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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#include "comm/cl_commlib.h"
#include "comm/lists/cl_lists.h"
#include "comm/lists/cl_errors.h"
#include "comm/lists/cl_util.h"
#include "comm/cl_xml_parsing.h"

#include "basis_types.h"

#define CL_DO_XML_DEBUG 0


typedef struct cl_xml_sequence_type {
   char  character;
   char* sequence;
   int   sequence_length;
} cl_xml_sequence_t;

/* 
 * sequence array for XML escape sequences (used by cl_com_transformXML2String()
 * and cl_com_transformString2XML(). The first character has to be an "&" for sequence.
 * This is because cl_com_transformXML2String() is only checking for "&" characters when
 * parsing XML string in order to find sequences.
 */
#define CL_XML_SEQUENCE_ARRAY_SIZE 8 
static const cl_xml_sequence_t cl_com_sequence_array[CL_XML_SEQUENCE_ARRAY_SIZE] = {
                                                   {'\n', "&#x0D;" , 6}, /* carriage return */
                                                   {'\r', "&#x0A;" , 6}, /* linefeed */
                                                   {'\t', "&#x09;" , 6}, /* tab */
                                                   {'&',  "&amp;"  , 5}, /* amp */
                                                   {'<',  "&lt;"   , 4}, /* lower than */
                                                   {'>',  "&gt;"   , 4}, /* greater than */
                                                   {'"',  "&quot;" , 6}, /* quote */
                                                   {'\'', "&apos;" , 6}  /* apostrophe */
};




static bool cl_xml_parse_is_version(const char* buffer, unsigned long start, unsigned long buffer_length);

static char *cl_xml_parse_version(char *charptr, unsigned long buffer_length)
{
   char *ret = NULL;
   char *sign = NULL;

   charptr[buffer_length-1] = '\0';
   if ((sign = strchr(charptr, '"'))) {
      int size = sign - charptr;
      ret = malloc(sizeof(char) * (size + 1));
      if (ret != NULL) {
         ret = strncpy(ret , charptr, size);
         ret [size] = '\0';
      }
   }

   return ret;
}

/****** commlib/cl_xml_parsing/cl_com_transformXML2String() ****************************
*  NAME
*     cl_com_transformXML2String() -- convert XML escape sequences to string
*
*  SYNOPSIS
*     int cl_com_transformXML2String(const char* input, char** output) 
*
*  FUNCTION
*     Converts an XML string into standard string without XML escape sequences.
*
*
*     Character      XML escape sequence        name
*
*       '\n'         "&#x0D;"                   carriage return
*       '\r'         "&#x0A;"                   linefeed
*       '\t'         "&#x09;"                   tab
*       '&'          "&amp;"                    amp
*       '<'          "&lt;"                     lower than
*       '>'          "&gt;"                     greater than
*       '"'          "&quot;"                   quote
*       '\''         "&apos;"                   apostrophe
*
*  INPUTS
*     const char* input - xml sequence string
*     char** output     - pointer to empty string pointer. The function will
*                         malloc() memory for the output string and return
*                         the input string with xml escape sequences converted to
*                         standard string characters.
*
*  RESULT
*     int - CL_RETVAL_OK     - no errors
*           CL_RETVAL_PARAMS - input or output are not correctly initialized
*           CL_RETVAL_MALLOC - can't malloc() memory for output string
*
*  NOTES
*     MT-NOTE: cl_com_transformXML2String() is MT safe 
*
*  SEE ALSO
*     commlib/cl_xml_parsing/cl_com_transformString2XML()
*******************************************************************************/
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_transformXML2String()"
int cl_com_transformXML2String(const char* input, char** output) {
   int i,pos,s;
   int input_length = 0;
   int output_length = 0;
   int seq_count;
   int matched;

   if (input == NULL || output == NULL || *output != NULL) {
      return CL_RETVAL_PARAMS;
   }

   input_length = strlen(input);
   
   /* since output is shorter than input we don't calculate the output length */
   output_length = input_length;
   *output = (char*)malloc((sizeof(char) * (1 + output_length)));
   if (*output == NULL) {
      return CL_RETVAL_MALLOC;
   }
   
   pos = 0;
   seq_count = 0;
   for (i=0;i<input_length;i++) {
      if ( input[i] == '&' ) {
         /* found possible escape sequence */
         for (s=0;s<CL_XML_SEQUENCE_ARRAY_SIZE;s++) {
            matched = 0;
            for (seq_count=0; i + seq_count < input_length && seq_count < cl_com_sequence_array[s].sequence_length ; seq_count++) {
               if (input[i+seq_count] != cl_com_sequence_array[s].sequence[seq_count]) {
                  break;
               }
               if ( seq_count + 1 == cl_com_sequence_array[s].sequence_length ) {
                  /* match */
                  (*output)[pos++] = cl_com_sequence_array[s].character;
                  i += cl_com_sequence_array[s].sequence_length - 1;
                  matched = 1;
                  break;
               }
            }
            if (matched == 1) {
               break;
            }
         }
         continue;
      }
      (*output)[pos++] = input[i];
   }
   (*output)[pos] = '\0';
   return CL_RETVAL_OK;
}

/****** commlib/cl_xml_parsing/cl_com_transformString2XML() ****************************
*  NAME
*     cl_com_transformString2XML() -- convert special chars to escape sequences
*
*  SYNOPSIS
*     int cl_com_transformString2XML(const char* input, char** output) 
*
*  FUNCTION
*     This function will parse the input char string and replace characters
*     by escape sequences in the output string where necessary. The user has
*     to sge_free() the output string.
*
*  INPUTS
*     const char* input - input string without xml escape sequences
*     char** output     - pointer to empty string pointer. The function will
*                         malloc() memory for the output string and return
*                         the input string with xml escape sequences.
*
*     Character      XML escape sequence        name
*
*       '\n'         "&#x0D;"                   carriage return
*       '\r'         "&#x0A;"                   linefeed
*       '\t'         "&#x09;"                   tab
*       '&'          "&amp;"                    amp
*       '<'          "&lt;"                     lower than
*       '>'          "&gt;"                     greater than
*       '"'          "&quot;"                   quote
*       '\''         "&apos;"                   apostrophe
*
*  RESULT
*     int - CL_RETVAL_OK     - no errors
*           CL_RETVAL_PARAMS - input or output are not correctly initialized
*           CL_RETVAL_MALLOC - can't malloc() memory for output string
*
*  NOTES
*     MT-NOTE: cl_com_transformString2XML() is MT safe 
*
*  SEE ALSO
*     commlib/cl_xml_parsing/cl_com_transformXML2String()
*******************************************************************************/
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_transformString2XML()"
int cl_com_transformString2XML(const char* input, char** output) {
   int input_length, i, s;
   int used = 0;
   int malloced_size;

   if (input == NULL || output == NULL || *output != NULL) {
      return CL_RETVAL_PARAMS;
   }

   input_length = strlen(input);

   /* we malloc the double size of the original string and hope that we don't need to realloc */
   malloced_size = input_length * 2;
   *output = sge_malloc((sizeof(char) * (malloced_size + 1)));
   for (i=0; i < input_length; i++) {
      bool found = false;
      int add_length = 1;
      for (s=0 ; s < CL_XML_SEQUENCE_ARRAY_SIZE; s++) {
         if (input[i] == cl_com_sequence_array[s].character) {
            found = true;
            add_length = cl_com_sequence_array[s].sequence_length;
            break;
         }
      }

      if (used + add_length >= malloced_size) {
         /* worst case, need to realloc */
         malloced_size += malloced_size;
         *output = sge_realloc(*output, malloced_size + 1, 1);
      }

      if (found == false) {
         (*output)[used] = input[i];
      } else {
         strncpy((char *)&(*output)[used], cl_com_sequence_array[s].sequence,
                 add_length);
      }
      used += add_length;
   }
   (*output)[used] = '\0';
   return CL_RETVAL_OK;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_get_mih_mat_string()"
const char* cl_com_get_mih_mat_string(cl_xml_ack_type_t mat) {
   switch (mat) {
      case CL_MIH_MAT_NAK:
         return CL_MIH_MESSAGE_ACK_TYPE_NAK;
      case CL_MIH_MAT_ACK:
         return CL_MIH_MESSAGE_ACK_TYPE_ACK;
      case CL_MIH_MAT_SYNC:
         return CL_MIH_MESSAGE_ACK_TYPE_SYNC;
      case CL_MIH_MAT_UNDEFINED:
         return "undefined";
   }
   return "undefined";
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_get_mih_df_string()"
const char* cl_com_get_mih_df_string(cl_xml_mih_data_format_t df) {
   switch (df) {
      case CL_MIH_DF_BIN:
         return CL_MIH_MESSAGE_DATA_FORMAT_BIN;
      case CL_MIH_DF_XML:
         return CL_MIH_MESSAGE_DATA_FORMAT_XML;
      case CL_MIH_DF_AM:
         return CL_MIH_MESSAGE_DATA_FORMAT_AM;
      case CL_MIH_DF_SIM:
         return CL_MIH_MESSAGE_DATA_FORMAT_SIM;
      case CL_MIH_DF_SIRM:
         return CL_MIH_MESSAGE_DATA_FORMAT_SIRM;
      case CL_MIH_DF_CCM:
         return CL_MIH_MESSAGE_DATA_FORMAT_CCM;
      case CL_MIH_DF_CCRM:
         return CL_MIH_MESSAGE_DATA_FORMAT_CCRM;
      case CL_MIH_DF_CM:
         return CL_MIH_MESSAGE_DATA_FORMAT_CM;
      case CL_MIH_DF_CRM:
         return CL_MIH_MESSAGE_DATA_FORMAT_CRM;
      case CL_MIH_DF_UNDEFINED:
         return "undefined";
   }
   return "undefined";
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_free_gmsh_header()"
int cl_com_free_gmsh_header(cl_com_GMSH_t** header) {
   if (header == NULL || *header == NULL) {
      return CL_RETVAL_PARAMS;
   }
   sge_free(header);
   return CL_RETVAL_OK;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_free_cm_message()"
int cl_com_free_cm_message(cl_com_CM_t** message) {   /* CR check */
   if (message == NULL || *message == NULL) {
      return CL_RETVAL_PARAMS;
   }
   if ((*message)->version != NULL) {
      sge_free(&((*message)->version));
   }
   cl_com_free_endpoint(&((*message)->rdata));
   cl_com_free_endpoint(&((*message)->dst));

   sge_free(message);
   return CL_RETVAL_OK;
}

int cl_com_free_mih_message(cl_com_MIH_t** message) {   /* CR check */
   if (message == NULL || *message == NULL) {
      return CL_RETVAL_PARAMS;
   }
   sge_free(&((*message)->version));
   sge_free(message);
   return CL_RETVAL_OK;
}

int cl_com_free_am_message(cl_com_AM_t** message) {   /* CR check */
   if (message == NULL || *message == NULL) {
      return CL_RETVAL_PARAMS;
   }
   sge_free(&((*message)->version));
   sge_free(message);
   return CL_RETVAL_OK;
}

int cl_com_free_sim_message(cl_com_SIM_t** message) {
   if (message == NULL || *message == NULL) {
      return CL_RETVAL_PARAMS;
   }
   sge_free(&((*message)->version));
   sge_free(message);
   return CL_RETVAL_OK;
}

int cl_com_free_sirm_message(cl_com_SIRM_t** message) {
   if (message == NULL || *message == NULL) {
      return CL_RETVAL_PARAMS;
   }
   sge_free(&((*message)->version));
   sge_free(&((*message)->info));
   sge_free(message);
   return CL_RETVAL_OK;
}

int cl_com_free_ccm_message(cl_com_CCM_t** message) {   /* CR check */
   if (message == NULL || *message == NULL) {
      return CL_RETVAL_PARAMS;
   }
   sge_free(&((*message)->version));
   sge_free(message);
   return CL_RETVAL_OK;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_free_ccrm_message()"
int cl_com_free_ccrm_message(cl_com_CCRM_t** message) {   /* CR check */
   if (message == NULL || *message == NULL) {
      return CL_RETVAL_PARAMS;
   }
   sge_free(&((*message)->version));
   sge_free(message);
   return CL_RETVAL_OK;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_free_crm_message()"
int cl_com_free_crm_message(cl_com_CRM_t** message) {   /* CR check */
   if (message == NULL || *message == NULL) {
      return CL_RETVAL_PARAMS;
   }
   sge_free(&((*message)->version));
   sge_free(&((*message)->cs_text));
   sge_free(&((*message)->formats));
   cl_com_free_endpoint(&((*message)->rdata));
   sge_free(&((*message)->params));
   sge_free(message);
   return CL_RETVAL_OK;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_xml_parse_GMSH()"
int cl_xml_parse_GMSH(unsigned char* buffer, unsigned long buffer_length, cl_com_GMSH_t* header , unsigned long *used_buffer_length) {

   unsigned long buf_pointer = 0;
   unsigned long tag_begin = 0;
   unsigned long tag_end = 0;
   unsigned long dl_begin = 0;
   unsigned long dl_end = 0;
   bool closing_tag;

   if (header == NULL || buffer == NULL ) {
      return CL_RETVAL_PARAMS;
   }

   header->dl = 0;
   *used_buffer_length = 0;

   while(buf_pointer <= buffer_length) {
      switch(buffer[buf_pointer]) {
         case '<':
            tag_begin = buf_pointer + 1;
            break; 
         case '>':
            tag_end = buf_pointer - 1;
            if (tag_begin < tag_end && tag_begin > 0) {
               const char *charptr = (char*)&(buffer[tag_begin]);
               
               if (charptr[0] == '/') {
                  closing_tag = true;
                  charptr++;
               } else {
                  closing_tag = false;
               }

               buffer[buf_pointer] = '\0';
               if (closing_tag == true && strcmp(charptr, "gmsh") == 0) {
                  if (*used_buffer_length == 0) {
                     *used_buffer_length = buf_pointer+1;
                  }
                  buf_pointer++;
                  break;
               } else if (strcmp(charptr, "dl") == 0) {
                  if (closing_tag == false) {
                     dl_begin = buf_pointer + 1;
                  } else {
                     dl_end = buf_pointer - 1;
                  }
               }
            }
            break;
      }
      buf_pointer++;
   }

   if (dl_begin > 0 && dl_end >= dl_begin) {
      char *charptr = (char *)&(buffer[dl_begin]);
      buffer[dl_end] = '\0';
      header->dl = cl_util_get_ulong_value(charptr);
   }
   return CL_RETVAL_OK;
}




#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_xml_parse_CM()"
int cl_xml_parse_CM(unsigned char* buffer, unsigned long buffer_length, cl_com_CM_t** message) {

   unsigned long i;
   char help_buf[256];
   unsigned long help_buf_pointer = 0;
   unsigned long buf_pointer = 0;
   int in_tag = 0;
   unsigned long tag_begin = 0;
   unsigned long tag_end = 0;
   unsigned long version_begin = 0;
   unsigned long df_begin = 0;
   unsigned long df_end = 0;
   unsigned long ct_begin = 0;
   unsigned long ct_end = 0;
   unsigned long dst_begin = 0;
   unsigned long dst_end = 0;
   unsigned long rdata_begin = 0;
   unsigned long rdata_end = 0;
   unsigned long port_begin = 0;
   unsigned long port_end = 0;
   unsigned long autoclose_begin = 0;
   unsigned long autoclose_end = 0;
   bool closing_tag;
  
   if (message == NULL || buffer == NULL || *message != NULL) {
      return CL_RETVAL_PARAMS;
   }

   *message = (cl_com_CM_t*)malloc(sizeof(cl_com_CM_t));
   if (*message == NULL) {
      return CL_RETVAL_MALLOC;
   }

   (*message)->version = NULL;
   (*message)->df = CL_CM_DF_UNDEFINED;
   (*message)->ct = CL_CM_CT_UNDEFINED;
   (*message)->ac = CL_CM_AC_UNDEFINED;
   (*message)->port = 0;
   (*message)->rdata = NULL;
   (*message)->dst = NULL;

   while(buf_pointer < buffer_length) {
      switch(buffer[buf_pointer]) {
         case '=':
            if (in_tag == 1) {
               if (version_begin == 0 && cl_xml_parse_is_version((const char *)buffer, tag_begin, buffer_length) == true) {
                  version_begin = buf_pointer + 2;
               }
            }
         break;   
         case '<':
            in_tag = 1;
            tag_begin = buf_pointer + 1;
         break; 
         case '>':
            in_tag = 0;
            tag_end = buf_pointer - 1;
            if (tag_begin < tag_end && tag_begin > 0) {
               const char *charptr = (char*)&(buffer[tag_begin]);

               if (charptr[0] == '/') {
                  closing_tag = true;
                  charptr++;
               } else {
                  closing_tag = false;
               }
               buffer[buf_pointer] = '\0';

               if (strncmp(charptr, "df", 2) == 0) {
                  if (closing_tag == false) {
                     df_begin = buf_pointer + 1;
                  } else {
                     df_end = buf_pointer - 4;
                  }
               } else if (strncmp(charptr, "ct", 2) == 0) {
                  if (closing_tag == false) {
                     ct_begin = buf_pointer + 1;
                  } else {
                     ct_end = buf_pointer - 4;
                  }
               } else if (strncmp(charptr, "port", 4) == 0) {
                  if (closing_tag == false) {
                     port_begin = buf_pointer + 1;
                  } else {
                     port_end = buf_pointer - 6;
                  }
               } else if (strncmp(charptr, "ac", 2) == 0) {
                  if (closing_tag == false) {
                     autoclose_begin = buf_pointer + 1;
                  } else {
                     autoclose_end = buf_pointer - 4;
                  }
               } else if (strncmp(charptr, "rdata", 5) == 0) {
                  if (closing_tag == false) {
                     if (rdata_begin == 0) {
                        rdata_begin = tag_begin;
                     }
                  } else {
                     rdata_end = buf_pointer - 7;
                  }
               } else if (strncmp(charptr, "dst", 3) == 0) {
                  if (closing_tag == false) {
                     if (dst_begin == 0) {
                        dst_begin = tag_begin;
                     }
                  } else {
                     dst_end = buf_pointer - 5;
                  }
               } 
            }
            break;
      }
      buf_pointer++;
   }

   if (df_begin > 0 && df_end >= df_begin) {
      char *charptr = (char *)&(buffer[df_begin]);
      buffer[df_end] = '\0';
      if (strcmp("bin", charptr) == 0) {
         (*message)->df = CL_CM_DF_BIN;
      } else if (strcmp("xml", charptr) == 0) {
         (*message)->df = CL_CM_DF_XML;
      }
   }

   if (ct_begin > 0 && ct_end >= ct_begin) {
      char *charptr = (char *)&(buffer[ct_begin]);
      buffer[ct_end] = '\0';
      if (strcmp(CL_CONNECT_MESSAGE_DATA_FLOW_STREAM, charptr) == 0) {
         (*message)->ct = CL_CM_CT_STREAM;
      } else if (strcmp(CL_CONNECT_MESSAGE_DATA_FLOW_MESSAGE, charptr) == 0) {
         (*message)->ct = CL_CM_CT_MESSAGE;
      }
   }

   if (port_begin > 0 && port_end >= port_begin) {
      char *charptr = (char *)&(buffer[port_begin]);
      buffer[port_end] = '\0';
      (*message)->port = cl_util_get_ulong_value(charptr);
   } else {
      CL_LOG(CL_LOG_ERROR,"port information undefined");
         return CL_RETVAL_UNKNOWN;
   }

   if (autoclose_begin > 0 && autoclose_end >= autoclose_begin) {
      char *charptr = (char *)&(buffer[autoclose_begin]);
      buffer[autoclose_end] = '\0';
      if (strcmp(CL_CONNECT_MESSAGE_AUTOCLOSE_ENABLED, charptr) == 0) {
         (*message)->ac = CL_CM_AC_ENABLED;
      } else if (strcmp(CL_CONNECT_MESSAGE_AUTOCLOSE_DISABLED, charptr) == 0) {
         (*message)->ac = CL_CM_AC_DISABLED;
      }
   }

   /* get version */
   if (version_begin > 0) {
      (*message)->version = cl_xml_parse_version((char*)&(buffer[version_begin]), buffer_length-version_begin);
      if ((*message)->version == NULL) {
         CL_LOG(CL_LOG_ERROR,"version information undefined");
         return CL_RETVAL_UNKNOWN;
      }
   } else {
      CL_LOG(CL_LOG_ERROR,"no version information found");
      return CL_RETVAL_UNKNOWN;
   }

   if ((*message)->df == CL_CM_DF_UNDEFINED) {
      CL_LOG(CL_LOG_ERROR,"data format undefined");
      return CL_RETVAL_UNKNOWN;
   }

   if ((*message)->ct == CL_CM_CT_UNDEFINED) {
      CL_LOG(CL_LOG_ERROR,"connection type undefined");
      return CL_RETVAL_UNKNOWN;
   }

   if ((*message)->ac == CL_CM_AC_UNDEFINED) {
      CL_LOG(CL_LOG_ERROR,"autoclose option undefined");
      return CL_RETVAL_UNKNOWN;
   }

   /* get rdata */
   if (rdata_begin > 0 && rdata_end >= rdata_begin) {
      (*message)->rdata = (cl_com_endpoint_t*)malloc(sizeof(cl_com_endpoint_t));
      if ((*message)->rdata == NULL) {
         return CL_RETVAL_MALLOC;
      }
      (*message)->rdata->comp_host = NULL;
      (*message)->rdata->comp_name = NULL;
      (*message)->rdata->hash_id = NULL;
      i = rdata_begin;

      tag_begin = 0;
      help_buf_pointer = 0;
      while(i<=rdata_end && help_buf_pointer < 254) {
         if (buffer[i] == '\"') {
            if (tag_begin == 1) {
               i++;
               break;             
            }
            tag_begin = 1;
            i++;
            continue;
         }
         if (tag_begin == 1) {
            help_buf[help_buf_pointer++] = buffer[i];
         }
         i++;
      }
      help_buf[help_buf_pointer] = '\0';
      (*message)->rdata->comp_host = strdup(help_buf);
      if ((*message)->rdata->comp_host == NULL) {
         return CL_RETVAL_MALLOC;
      }
      tag_begin = 0;
      help_buf_pointer = 0;
      while(i<=rdata_end && help_buf_pointer < 254) {
        
         if (buffer[i] == '\"') {
            if (tag_begin == 1) {
               i++;
               break;             
            }
            tag_begin = 1;
            i++;
            continue;
         }
         if (tag_begin == 1) {
            help_buf[help_buf_pointer++] = buffer[i];
         }
         i++;
      }
      help_buf[help_buf_pointer] = 0;
      (*message)->rdata->comp_name = strdup(help_buf);
      if ((*message)->rdata->comp_name == NULL) {
         return CL_RETVAL_MALLOC;
      }
      tag_begin = 0;
      help_buf_pointer = 0;
      while(i<=rdata_end && help_buf_pointer < 254) {
        
         if (buffer[i] == '\"') {
            if (tag_begin == 1) {
               i++;
               break;             
            }
            tag_begin = 1;
            i++;
            continue;
         }
         if (tag_begin == 1) {
            help_buf[help_buf_pointer++] = buffer[i];
         }
         i++;
      }
      help_buf[help_buf_pointer] = 0;
      (*message)->rdata->comp_id = cl_util_get_ulong_value(help_buf);
      (*message)->rdata->hash_id = NULL;
   }

   /* get dst data */
   if (dst_begin > 0 && dst_end >= dst_begin) {
      (*message)->dst = (cl_com_endpoint_t*)malloc(sizeof(cl_com_endpoint_t));
      if ((*message)->dst == NULL) {
         return CL_RETVAL_MALLOC;
      }
      (*message)->dst->comp_host = NULL;
      (*message)->dst->comp_name = NULL;
      (*message)->dst->hash_id = NULL;

      i = dst_begin;

      tag_begin = 0;
      help_buf_pointer = 0;
      while(i<=dst_end && help_buf_pointer < 254) {
         if (buffer[i] == '\"') {
            if (tag_begin == 1) {
               i++;
               break;             
            }
            tag_begin = 1;
            i++;
            continue;
         }
         if (tag_begin == 1) {
            help_buf[help_buf_pointer++] = buffer[i];
         }
         i++;
      }
      help_buf[help_buf_pointer] = '\0';
      (*message)->dst->comp_host = strdup(help_buf);
      if ((*message)->dst->comp_host == NULL) {
         return CL_RETVAL_MALLOC;
      }
      tag_begin = 0;
      help_buf_pointer = 0;
      while(i<=dst_end && help_buf_pointer < 254) {
        
         if (buffer[i] == '\"') {
            if (tag_begin == 1) {
               i++;
               break;             
            }
            tag_begin = 1;
            i++;
            continue;
         }
         if (tag_begin == 1) {
            help_buf[help_buf_pointer++] = buffer[i];
         }
         i++;
      }
      help_buf[help_buf_pointer] = 0;
      (*message)->dst->comp_name = strdup(help_buf);
      if ((*message)->dst->comp_name == NULL) {
         return CL_RETVAL_MALLOC;
      }
      tag_begin = 0;
      help_buf_pointer = 0;
      while(i<=dst_end && help_buf_pointer < 254) {
        
         if (buffer[i] == '\"') {
            if (tag_begin == 1) {
               i++;
               break;             
            }
            tag_begin = 1;
            i++;
            continue;
         }
         if (tag_begin == 1) {
            help_buf[help_buf_pointer++] = buffer[i];
         }
         i++;
      }
      help_buf[help_buf_pointer] = 0;
      (*message)->dst->comp_id = cl_util_get_ulong_value(help_buf);
      (*message)->dst->hash_id = NULL;
   }

   /* check complete return structure of dst and rdata */
   if ((*message)->dst == NULL) {
      CL_LOG(CL_LOG_ERROR,"dst information not set");
      return CL_RETVAL_UNKNOWN;
   } else {
      if ((*message)->dst->comp_host == NULL) {
         CL_LOG(CL_LOG_ERROR,"dst comp host information not set");
         return CL_RETVAL_UNKNOWN;
      }
      if ((*message)->dst->comp_name == NULL) {
         CL_LOG(CL_LOG_ERROR,"dst comp name information not set");
         return CL_RETVAL_UNKNOWN;
      }
   }
   if ((*message)->rdata == NULL) {
      CL_LOG(CL_LOG_ERROR,"rdata information not set");
      return CL_RETVAL_UNKNOWN;
   } else {
      if ((*message)->rdata->comp_host == NULL) {
         CL_LOG(CL_LOG_ERROR,"rdata comp host information not set");
         return CL_RETVAL_UNKNOWN;
      }
      if ((*message)->rdata->comp_name == NULL) {
         CL_LOG(CL_LOG_ERROR,"rdata comp name information not set");
         return CL_RETVAL_UNKNOWN;
      }
   }
   return CL_RETVAL_OK;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_xml_parse_CRM()"
int cl_xml_parse_CRM(unsigned char* buffer, unsigned long buffer_length, cl_com_CRM_t** message ) {

   unsigned long i;
   char help_buf[256];
   unsigned long help_buf_pointer = 0;
   unsigned long buf_pointer = 0;
   int in_tag = 0;
   unsigned long tag_begin = 0;
   unsigned long tag_end = 0;
   unsigned long version_begin = 0;
   unsigned long cs_begin = 0;
   unsigned long cs_end = 0;

   unsigned long rdata_begin = 0;
   unsigned long rdata_end = 0;
   bool closing_tag;

   unsigned long params_begin = 0;
   unsigned long params_end = 0;

   if (message == NULL || buffer == NULL || *message != NULL) {
      return CL_RETVAL_PARAMS;
   }

   *message = (cl_com_CRM_t*)malloc(sizeof(cl_com_CRM_t));
   if (*message == NULL) {
      return CL_RETVAL_MALLOC;
   }
   memset((char *) (*message), 0, sizeof(cl_com_CRM_t));

   (*message)->cs_condition = CL_CRM_CS_UNDEFINED;
   
   while(buf_pointer < buffer_length) {
      switch( buffer[buf_pointer] ) {
         case '=':
            if (in_tag == 1) {
               if (version_begin == 0 && cl_xml_parse_is_version((const char *)buffer, tag_begin, buffer_length) == true) {
                  version_begin = buf_pointer + 2;
               }
            }
         break;   
         case '<':
            in_tag = 1;
            tag_begin = buf_pointer + 1;
         break; 
         case '>':
            in_tag = 0;
            tag_end = buf_pointer - 1;
            if (tag_begin < tag_end && tag_begin > 0) {
               const char *charptr = (char*)&(buffer[tag_begin]);

               if (charptr[0] == '/') {
                  closing_tag = true;
                  charptr++;
               } else {
                  closing_tag = false;
               }

               /* set string end to ">" position of tag */
               buffer[buf_pointer] = '\0';

               if (strncmp(charptr,"cs", 2) == 0) {
                  if (closing_tag == false) {
                     if (cs_begin == 0) {
                        cs_begin = tag_begin + 3;
                     }
                  } else {
                     cs_end = buf_pointer - 5;
                  }
               } else if (strncmp(charptr, "rdata", 5) == 0) {
                  if (closing_tag == false) {
                     if (rdata_begin == 0) {
                        rdata_begin = tag_begin + 6;
                     }
                  } else {
                     rdata_end = buf_pointer - 8;
                  }
               } else if (strncmp(charptr, "params", 6) == 0) {
                  if (closing_tag == false) {
                     if (params_begin == 0) {
                        params_begin = tag_begin + 7;
                     }
                  } else {
                     params_end = buf_pointer - 9;
                  }
               }
            }
      }
      buf_pointer++;
   }

   /* get version */
   if (version_begin > 0) {
      (*message)->version = cl_xml_parse_version((char*)&(buffer[version_begin]), buffer_length-version_begin);
   }

   /* get cs_condition */
   if (cs_begin > 0 && cs_end >= cs_begin) {
      i = cs_begin;
      tag_begin = 0;
      help_buf_pointer = 0;
      while(i<=cs_end && help_buf_pointer < 254) {
         if (buffer[i] == '\"') {
            if (tag_begin == 1) {
               i++;
               break;             
            }
            tag_begin = 1;
            i++;
            continue;
         }
         if (tag_begin == 1) {
            help_buf[help_buf_pointer++] = buffer[i];
         }
         i++;
      }
      help_buf[help_buf_pointer] = '\0';

      if (strcmp(CL_CONNECT_RESPONSE_MESSAGE_CONNECTION_STATUS_OK,help_buf) == 0) {
         (*message)->cs_condition = CL_CRM_CS_CONNECTED;
      } else if (strcmp(CL_CONNECT_RESPONSE_MESSAGE_CONNECTION_STATUS_DENIED, help_buf) == 0) {
         (*message)->cs_condition = CL_CRM_CS_DENIED;
      } else if (strcmp(CL_CONNECT_RESPONSE_MESSAGE_CONNECTION_UNSUP_DATA_FORMAT, help_buf) == 0) {
         (*message)->cs_condition = CL_CRM_CS_UNSUPPORTED;
      } else if (strcmp(CL_CONNECT_RESPONSE_MESSAGE_CONNECTION_STATUS_NOT_UNIQUE, help_buf) == 0) {
         (*message)->cs_condition = CL_CRM_CS_ENDPOINT_NOT_UNIQUE;
      }
      i++;
      help_buf_pointer = '\0';
      while(i<=cs_end && help_buf_pointer < 254) {
         help_buf[help_buf_pointer++] = buffer[i];
         i++;
      }
      help_buf[help_buf_pointer] = 0;
      (*message)->cs_text = strdup(help_buf);
   }

   /* get rdata */
   if (rdata_begin > 0 && rdata_end >= rdata_begin) {
      (*message)->rdata = (cl_com_endpoint_t*)malloc(sizeof(cl_com_endpoint_t));
      if ((*message)->rdata == NULL) {
         cl_com_free_crm_message(message);
         return CL_RETVAL_MALLOC;
      }
      i = rdata_begin;

      tag_begin = 0;
      help_buf_pointer = 0;
      while(i<=rdata_end && help_buf_pointer < 254) {
         if (buffer[i] == '\"') {
            if (tag_begin == 1) {
               i++;
               break;             
            }
            tag_begin = 1;
            i++;
            continue;
         }
         if (tag_begin == 1) {
            help_buf[help_buf_pointer++] = buffer[i];
         }
         i++;
      }
      help_buf[help_buf_pointer] = '\0';
      (*message)->rdata->comp_host = strdup(help_buf);

      tag_begin = 0;
      help_buf_pointer = 0;
      while(i<=rdata_end && help_buf_pointer < 254) {
        
         if (buffer[i] == '\"') {
            if (tag_begin == 1) {
               i++;
               break;             
            }
            tag_begin = 1;
            i++;
            continue;
         }
         if (tag_begin == 1) {
            help_buf[help_buf_pointer++] = buffer[i];
         }
         i++;
      }
      help_buf[help_buf_pointer] = '\0';
      (*message)->rdata->comp_name = strdup(help_buf);

      tag_begin = 0;
      help_buf_pointer = 0;
      while(i<=rdata_end && help_buf_pointer < 254) {
        
         if (buffer[i] == '\"') {
            if (tag_begin == 1) {
               i++;
               break;             
            }
            tag_begin = 1;
            i++;
            continue;
         }
         if (tag_begin == 1) {
            help_buf[help_buf_pointer++] = buffer[i];
         }
         i++;
      }
      help_buf[help_buf_pointer] = '\0';
      (*message)->rdata->comp_id = cl_util_get_ulong_value(help_buf);
      (*message)->rdata->hash_id = NULL;
   }

   /* get env params */
   if (params_begin > 0 && params_end >= params_begin) {
      i = params_begin;
      help_buf_pointer = 0;
      while(i<=params_end && help_buf_pointer < 254) {
         help_buf[help_buf_pointer++] = buffer[i];
         i++;
      }
      help_buf[help_buf_pointer] = '\0';
      (*message)->params = strdup(help_buf);
   }

   (*message)->formats = strdup("not supported");
#if CL_DO_XML_DEBUG
   CL_LOG_STR(CL_LOG_DEBUG,"version:     ", (*message)->version);
   CL_LOG_INT(CL_LOG_DEBUG,"cs_condition:", (int)(*message)->cs_condition);
   CL_LOG_STR(CL_LOG_DEBUG,"cs_text:     ", (*message)->cs_text);
   CL_LOG_STR(CL_LOG_DEBUG,"formats:     ", (*message)->formats);
   CL_LOG_STR(CL_LOG_DEBUG,"rdata->host: ", (*message)->rdata->comp_host);
   CL_LOG_STR(CL_LOG_DEBUG,"rdata->comp: ", (*message)->rdata->comp_name);
   CL_LOG_INT(CL_LOG_DEBUG,"rdata->id:   ", (int)(*message)->rdata->comp_id);
   CL_LOG_STR(CL_LOG_DEBUG,"params:      ", (*message)->params);
#endif

   return CL_RETVAL_OK;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_xml_parse_is_version()"
static bool cl_xml_parse_is_version(const char* buffer, unsigned long start, unsigned long buffer_length) {
   unsigned long i = 0;
   bool found = false;

   if (buffer != NULL) {
      for (i=start; ( i < buffer_length) && (buffer[i] != '>'); i++) {
         /* check for version string in tag */
         if (strncmp(&buffer[i],"version", 7) == 0) {
            found = true;
            break;
         }   
      }
   }
   return found;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_xml_parse_MIH()"
int cl_xml_parse_MIH(unsigned char* buffer, unsigned long buffer_length, cl_com_MIH_t** message ) {
   unsigned long buf_pointer = 0;
   int in_tag = 0;
   unsigned long tag_begin = 0;
   unsigned long tag_end = 0;
   unsigned long version_begin = 0;
 
   unsigned long mid_begin = 0;
   unsigned long mid_end = 0;
   unsigned long dl_begin = 0;
   unsigned long dl_end = 0;
   unsigned long df_begin = 0;
   unsigned long df_end = 0;
   unsigned long mat_begin = 0;
   unsigned long mat_end = 0;  
   unsigned long mtag_begin = 0;
   unsigned long mtag_end = 0;  
   unsigned long rid_begin = 0;
   unsigned long rid_end = 0;
   bool closing_tag;

   if (message == NULL || buffer == NULL || *message != NULL) {
      return CL_RETVAL_PARAMS;
   }

   *message = (cl_com_MIH_t*)malloc(sizeof(cl_com_MIH_t));
   if (*message == NULL) {
      return CL_RETVAL_MALLOC;
   }
   memset((char *) (*message), 0, sizeof(cl_com_MIH_t));
   
   (*message)->df  = CL_MIH_DF_UNDEFINED;
   (*message)->mat = CL_MIH_MAT_UNDEFINED;
   
   while(buf_pointer < buffer_length) {
      switch( buffer[buf_pointer] ) {
         case '=':
            if (in_tag == 1) {
               if (version_begin == 0 && cl_xml_parse_is_version((const char*)buffer, tag_begin, buffer_length) == true) {
                  version_begin = buf_pointer + 2;
               }
            }
         break;   
         case '<':
            in_tag = 1;
            tag_begin = buf_pointer + 1;
         break; 
         case '>':
            in_tag = 0;
            tag_end = buf_pointer - 1;
            if (tag_begin < tag_end && tag_begin > 0) {
               const char *charptr = (char*)&(buffer[tag_begin]);
               
               if (charptr[0] == '/') {
                  closing_tag = true;
                  charptr++;
               } else {
                  closing_tag = false;
               }

               buffer[buf_pointer] = '\0';

               if (strcmp(charptr,"mid") == 0 ) {
                  if (closing_tag == false) {
                     mid_begin = tag_end + 2;
                  } else {
                     mid_end = tag_begin - 1;
                  }
               } else if (strcmp(charptr,"dl") == 0 ) {
                  if (closing_tag == false) {
                     dl_begin = tag_end + 2;
                  } else {
                     dl_end = tag_begin - 1;
                  }
               } else if (strcmp(charptr,"df") == 0 ) {
                  if (closing_tag == false) {
                     df_begin = tag_end + 2;
                  } else {
                     df_end = tag_begin - 1;
                  }
               } else if (strcmp(charptr,"mat") == 0 ) {
                  if (closing_tag == false) {
                     mat_begin = tag_end + 2;
                  } else {
                     mat_end = tag_begin - 1;
                  }
               } else if (strcmp(charptr,"tag") == 0 ) {
                  if (closing_tag == false) {
                     mtag_begin = tag_end + 2;
                  } else {
                     mtag_end = tag_begin - 1;
                  }
               } else if (strcmp(charptr,"rid") == 0 ) {
                  if (closing_tag == false) {
                     rid_begin = tag_end + 2;
                  } else {
                     rid_end = tag_begin - 1;
                  }
               }
            }
            break;
      }
      buf_pointer++;
   }


   /* get version */
   if (version_begin > 0) {
      (*message)->version = cl_xml_parse_version((char*)&(buffer[version_begin]), buffer_length-version_begin);
   }


   /* get mid */
   if (mid_begin > 0 && mid_end >= mid_begin) {
      char *charptr = (char *)&(buffer[mid_begin]);
      buffer[mid_end] = '\0';
      (*message)->mid = cl_util_get_ulong_value(charptr);
   }

   /* get tag */
   if (mtag_begin > 0 && mtag_end >= mtag_begin) {
      char *charptr = (char *)&(buffer[mtag_begin]);
      buffer[mtag_end] = '\0';
      (*message)->tag = cl_util_get_ulong_value(charptr);
   }

   /* get rid */
   if (rid_begin > 0&& rid_end >= rid_begin) {
      char *charptr = (char *)&(buffer[rid_begin]);
      buffer[rid_end] = '\0';
      (*message)->rid = cl_util_get_ulong_value(charptr);
   }

   /* get dl */
   if (dl_begin > 0 && dl_end >= dl_begin) {
      char *charptr = (char *)&(buffer[dl_begin]);
      buffer[dl_end] = '\0';
      (*message)->dl = cl_util_get_ulong_value(charptr);
   }

   /* get df */
   if (df_begin > 0 && df_end >= df_begin) {
      char *charptr = (char *)&(buffer[df_begin]);

      buffer[df_end] = '\0';
      if (strcmp(CL_MIH_MESSAGE_DATA_FORMAT_BIN, charptr) == 0) {
         (*message)->df = CL_MIH_DF_BIN;
      } else if (strcmp(CL_MIH_MESSAGE_DATA_FORMAT_AM, charptr) == 0) {
         (*message)->df = CL_MIH_DF_AM;
      } else if (strcmp(CL_MIH_MESSAGE_DATA_FORMAT_CCM, charptr) == 0) {
         (*message)->df = CL_MIH_DF_CCM;
      } else if (strcmp(CL_MIH_MESSAGE_DATA_FORMAT_CCRM, charptr) == 0) {
         (*message)->df = CL_MIH_DF_CCRM;
      } else if (strcmp(CL_MIH_MESSAGE_DATA_FORMAT_XML, charptr) == 0) {
         (*message)->df = CL_MIH_DF_XML;
      } else if (strcmp(CL_MIH_MESSAGE_DATA_FORMAT_SIM, charptr) == 0) {
         (*message)->df = CL_MIH_DF_SIM;
      } else if (strcmp(CL_MIH_MESSAGE_DATA_FORMAT_SIRM, charptr) == 0) {
         (*message)->df = CL_MIH_DF_SIRM;
      }
   }
   /* get mat */
   if (mat_begin > 0 && mat_end >= mat_begin) {
      char *charptr = (char *)&(buffer[mat_begin]);

      buffer[mat_end] = '\0';
      if (strcmp(CL_MIH_MESSAGE_ACK_TYPE_NAK, charptr) == 0) {
         (*message)->mat = CL_MIH_MAT_NAK;
      } else if (strcmp(CL_MIH_MESSAGE_ACK_TYPE_ACK, charptr) == 0) {
         (*message)->mat = CL_MIH_MAT_ACK;
      } else if (strcmp(CL_MIH_MESSAGE_ACK_TYPE_SYNC, charptr) == 0) {
         (*message)->mat = CL_MIH_MAT_SYNC;
      }
   }

#if CL_DO_XML_DEBUG
   CL_LOG_STR(CL_LOG_DEBUG,"version: ", (*message)->version);
   CL_LOG_INT(CL_LOG_DEBUG,"mid:     ", (int)(*message)->mid);
   CL_LOG_INT(CL_LOG_DEBUG,"dl:      ", (int)(*message)->dl);
   CL_LOG_STR(CL_LOG_DEBUG,"df:      ", cl_com_get_mih_df_string((*message)->df));
   CL_LOG_STR(CL_LOG_DEBUG,"mat:     ", cl_com_get_mih_mat_string((*message)->mat));
   CL_LOG_INT(CL_LOG_DEBUG,"tag:     ", (int)(*message)->tag);
   CL_LOG_INT(CL_LOG_DEBUG,"rid:     ", (int)(*message)->rid);
#endif

   if ((*message)->dl > CL_DEFINE_MAX_MESSAGE_LENGTH) {
      return CL_RETVAL_MAX_MESSAGE_LENGTH_ERROR; 
   }

   return CL_RETVAL_OK;
}
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_xml_parse_SIRM()"
int cl_xml_parse_SIRM(unsigned char* buffer, unsigned long buffer_length, cl_com_SIRM_t** message ) {
   unsigned long buf_pointer = 0;
   int in_tag = 0;
   unsigned long tag_begin = 0;
   unsigned long tag_end = 0;
   unsigned long version_begin = 0;
 
   unsigned long mid_begin = 0;
   unsigned long mid_end = 0;

   unsigned long starttime_begin = 0;
   unsigned long starttime_end = 0;

   unsigned long runtime_begin = 0;
   unsigned long runtime_end = 0;

   unsigned long application_messages_brm_begin = 0;
   unsigned long application_messages_brm_end = 0;

   unsigned long application_messages_bwm_begin = 0;
   unsigned long application_messages_bwm_end = 0;

   unsigned long application_connections_noc_begin = 0;
   unsigned long application_connections_noc_end = 0;

   unsigned long application_status_begin = 0;
   unsigned long application_status_end = 0;

   unsigned long info_begin = 0;
   unsigned long info_end = 0;
   bool closing_tag;

   if (message == NULL || buffer == NULL || *message != NULL) {
      return CL_RETVAL_PARAMS;
   }

   *message = (cl_com_SIRM_t*)malloc(sizeof(cl_com_SIRM_t));
   if (*message == NULL) {
      return CL_RETVAL_MALLOC;
   }
   memset((char *) (*message), 0, sizeof(cl_com_SIRM_t));
  
   while(buf_pointer < buffer_length) {
      switch( buffer[buf_pointer] ) {
         case '=':
            if (in_tag == 1) {
               if (version_begin == 0 && cl_xml_parse_is_version((const char *)buffer, tag_begin, buffer_length) == true) {
                  version_begin = buf_pointer + 2;
               }
            }
         break;   
         case '<':
            in_tag = 1;
            tag_begin = buf_pointer + 1;
         break; 
         case '>':
            in_tag = 0;
            tag_end = buf_pointer - 1;
            if (tag_begin < tag_end && tag_begin > 0) {
               const char *charptr = (char*)&(buffer[tag_begin]);

               if (charptr[0] == '/') {
                  closing_tag = true;
                  charptr++;
               } else {
                  closing_tag = false;
               }
              
               buffer[buf_pointer] = '\0';
               
               if (strcmp(charptr,"mid") == 0 ) {
                  if (closing_tag == false) {
                     mid_begin = tag_end + 2;
                  } else {
                     mid_end = tag_begin - 1;
                  }
               } else if (strcmp(charptr,"starttime") == 0 ) {
                  if (closing_tag == false) {
                     starttime_begin = tag_end + 2;
                  } else {
                     starttime_end = tag_begin - 1;
                  }
               } else if (strcmp(charptr,"runtime") == 0 ) {
                  if (closing_tag == false) {
                     runtime_begin = tag_end + 2;
                  } else {
                     runtime_end = tag_begin - 1;
                  }
               } else if (strcmp(charptr,"brm") == 0 ) {
                  if (closing_tag == false) {
                     application_messages_brm_begin = tag_end + 2;
                  } else {
                     application_messages_brm_end = tag_begin - 1;
                  }
               } else if (strcmp(charptr,"bwm") == 0 ) {
                  if (closing_tag == false) {
                     application_messages_bwm_begin = tag_end + 2;
                  } else {
                     application_messages_bwm_end = tag_begin - 1;
                  }
               } else if (strcmp(charptr,"noc") == 0 ) {
                  if (closing_tag == false) {
                     application_connections_noc_begin = tag_end + 2;
                  } else {
                     application_connections_noc_end = tag_begin - 1;
                  }
               } else if (strcmp(charptr,"status") == 0 ) {
                  if (closing_tag == false) {
                     application_status_begin = tag_end + 2;
                  } else {
                     application_status_end = tag_begin - 1;
                  }
               } else if (strcmp(charptr,"info") == 0 ) {
                  if (closing_tag == false) {
                     info_begin = tag_end + 2;
                  } else {
                     info_end = tag_begin - 1;
                  }
               }
            }
            break;
      }
      buf_pointer++;
   }

   /* get version */
   if (version_begin > 0) {
      (*message)->version = cl_xml_parse_version((char*)&(buffer[version_begin]), buffer_length-version_begin);
   }

   /* get info */
   if (info_begin > 0 && info_end >= info_begin) {
      buffer[info_end] = '\0';
      cl_com_transformXML2String( (char*)&(buffer[info_begin]), &((*message)->info) );
   }

   /* get mid */
   if (mid_begin > 0 && mid_end >= mid_begin) {
      buffer[mid_end] = '\0';
      (*message)->mid = cl_util_get_ulong_value((char*)&(buffer[mid_begin]));
   }

   /* get starttime */
   if (starttime_begin > 0 && starttime_end >= starttime_begin) {
      buffer[starttime_end] = '\0';
      (*message)->starttime = cl_util_get_ulong_value((char*)&(buffer[starttime_begin]));
   }

   /* get runtime */
   if (runtime_begin > 0 && runtime_end >= runtime_begin) {
      buffer[runtime_end] = '\0';
      (*message)->runtime = cl_util_get_ulong_value((char*)&(buffer[runtime_begin]));
   }

   /* get application_messages_brm */
   if (application_messages_brm_begin > 0 && application_messages_brm_end >= application_messages_brm_begin) {
      buffer[application_messages_brm_end] = '\0';
      (*message)->application_messages_brm= cl_util_get_ulong_value((char*)&(buffer[application_messages_brm_begin]));
   }

   /* get application_messages_bwm */
   if (application_messages_bwm_begin > 0 && application_messages_bwm_end >= application_messages_bwm_begin) {
      buffer[application_messages_bwm_end] = '\0';
      (*message)->application_messages_bwm= cl_util_get_ulong_value((char*)&(buffer[application_messages_bwm_begin]));
   }

   /* get application_connections_noc */
   if (application_connections_noc_begin > 0 && application_connections_noc_end >= application_connections_noc_begin) {
      buffer[application_connections_noc_end] = '\0';
      (*message)->application_connections_noc= cl_util_get_ulong_value((char*)&(buffer[application_connections_noc_begin]));
   }

   /* get application_connections_noc */
   if (application_status_begin > 0 && application_status_end >= application_status_begin) {
      buffer[application_status_end] = '\0';
      (*message)->application_status = cl_util_get_ulong_value((char*)&(buffer[application_status_begin]));
   }

#if CL_DO_XML_DEBUG
   CL_LOG_STR(CL_LOG_DEBUG,"version:   ", (*message)->version);
   CL_LOG_INT(CL_LOG_DEBUG,"mid:       ", (int)(*message)->mid);
   CL_LOG_INT(CL_LOG_DEBUG,"starttime: ", (int)(*message)->starttime);
   CL_LOG_INT(CL_LOG_DEBUG,"runtime:   ", (int)(*message)->runtime);
   CL_LOG_INT(CL_LOG_DEBUG,"brm:       ", (int)(*message)->application_messages_brm);
   CL_LOG_INT(CL_LOG_DEBUG,"bwm:       ", (int)(*message)->application_messages_bwm);
   CL_LOG_INT(CL_LOG_DEBUG,"noc:       ", (int)(*message)->application_connections_noc);
   CL_LOG_INT(CL_LOG_DEBUG,"status:    ", (int)(*message)->application_status);
   CL_LOG_STR(CL_LOG_DEBUG,"info:      ", (*message)->info);
#endif

   return CL_RETVAL_OK;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_xml_parse_AM()"
int cl_xml_parse_AM(unsigned char* buffer, unsigned long buffer_length, cl_com_AM_t** message ) {
   unsigned long buf_pointer = 0;
   int in_tag = 0;
   unsigned long tag_begin = 0;
   unsigned long tag_end = 0;
   unsigned long version_begin = 0;
 
   unsigned long mid_begin = 0;
   unsigned long mid_end = 0;
   bool closing_tag;

   if (message == NULL || buffer == NULL || *message != NULL) {
      return CL_RETVAL_PARAMS;
   }

   *message = (cl_com_AM_t*)malloc(sizeof(cl_com_AM_t));
   if (*message == NULL) {
      return CL_RETVAL_MALLOC;
   }
   
   while(buf_pointer < buffer_length) {
      switch( buffer[buf_pointer] ) {
         case '=':
            if (in_tag == 1) {
               if (version_begin == 0 && cl_xml_parse_is_version((const char *)buffer, tag_begin, buffer_length) == true) {
                  version_begin = buf_pointer + 2;
               }
            }
         break;   
         case '<':
            in_tag = 1;
            tag_begin = buf_pointer + 1;
         break; 
         case '>':
            in_tag = 0;
            tag_end = buf_pointer - 1;
            if (tag_begin < tag_end && tag_begin > 0) {
               const char *charptr = (char*)&(buffer[tag_begin]);
               
               if (charptr[0] == '/') {
                  closing_tag = true;
                  charptr++;
               } else {
                  closing_tag = false;
               }

               buffer[buf_pointer] = '\0';

               if (strcmp(charptr,"mid") == 0 ) {
                  if (closing_tag == false) {
                     mid_begin = tag_end + 2;
                  } else {
                     mid_end = tag_begin - 2;
                  }
               }
            }
            break;
      }
      buf_pointer++;
   }


   /* get version */
   if (version_begin > 0) {
      (*message)->version = cl_xml_parse_version((char*)&(buffer[version_begin]), buffer_length-version_begin);
   } else {
      (*message)->version = NULL;
   }


   /* get mid */
   if (mid_begin > 0 && mid_end >= mid_begin) {
      buffer[mid_end] = '\0';
      (*message)->mid = cl_util_get_ulong_value((char*)&(buffer[mid_begin]));
   } else {
      (*message)->mid = 0;
   }

#if CL_DO_XML_DEBUG
   CL_LOG_STR(CL_LOG_DEBUG,"version: ", (*message)->version);
   CL_LOG_INT(CL_LOG_DEBUG,"mid:     ", (int)(*message)->mid);
#endif

   return CL_RETVAL_OK;

}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_xml_parse_CCM()"
int cl_xml_parse_CCM(unsigned char* buffer, unsigned long buffer_length, cl_com_CCM_t** message ) {
   unsigned long buf_pointer = 0;
   int in_tag = 0;
   unsigned long tag_begin = 0;
   unsigned long tag_end = 0;
   unsigned long version_begin = 0;
 
   if (message == NULL || buffer == NULL || *message != NULL) {
      return CL_RETVAL_PARAMS;
   }

   *message = (cl_com_CCM_t*)malloc(sizeof(cl_com_CCM_t));
   if (*message == NULL) {
      return CL_RETVAL_MALLOC;
   }
   
   while(buf_pointer < buffer_length) {
      switch( buffer[buf_pointer] ) {
         case '=':
            if (in_tag == 1) {
               if (version_begin == 0 && cl_xml_parse_is_version((const char *)buffer, tag_begin, buffer_length) == true) {
                  version_begin = buf_pointer + 2;
               }
            }
         break;   
         case '<':
            in_tag = 1;
            tag_begin = buf_pointer + 1;
         break; 
         case '>':
            in_tag = 0;
            tag_end = buf_pointer - 1;
            if (tag_begin < tag_end && tag_begin > 0) {
               const char *charptr = (char*)&(buffer[tag_begin]);
               
               buffer[buf_pointer] = '\0';
               
               if (strcmp(charptr,"/ccm") == 0) {
                  buf_pointer++;
                  continue;
               }
            }
         break;
      }
      buf_pointer++;
   }

   /* get version */
   if (version_begin > 0) {
      (*message)->version = cl_xml_parse_version((char*)&(buffer[version_begin]), buffer_length-version_begin);
   } else {
      (*message)->version = NULL;
   }

#if CL_DO_XML_DEBUG
   CL_LOG_STR(CL_LOG_DEBUG,"version: ", (*message)->version);
#endif
   return CL_RETVAL_OK;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_xml_parse_CCRM()"
int cl_xml_parse_CCRM(unsigned char* buffer, unsigned long buffer_length, cl_com_CCRM_t** message ) {
   unsigned long buf_pointer = 0;
   int in_tag = 0;
   unsigned long tag_begin = 0;
   unsigned long tag_end = 0;
   unsigned long version_begin = 0;
 

   if (message == NULL || buffer == NULL || *message != NULL) {
      return CL_RETVAL_PARAMS;
   }

   *message = (cl_com_CCRM_t*)malloc(sizeof(cl_com_CCRM_t));
   if (*message == NULL) {
      return CL_RETVAL_MALLOC;
   }
   
   while(buf_pointer < buffer_length) {
      switch( buffer[buf_pointer] ) {
         case '=':
            if (in_tag == 1) {
               if (version_begin == 0 && cl_xml_parse_is_version((const char *)buffer, tag_begin, buffer_length) == true) {
                  version_begin = buf_pointer + 2;
               }
            }
         break;   
         case '<':
            in_tag = 1;
            tag_begin = buf_pointer + 1;
         break; 
         case '>':
            in_tag = 0;
            tag_end = buf_pointer - 1;
            if (tag_begin < tag_end && tag_begin > 0) {
               const char *charptr = (char*)&(buffer[tag_begin]);
               
               buffer[buf_pointer] = '\0';
               
               if (strcmp(charptr,"/ccrm") == 0) {
                  buf_pointer++;
                  continue;
               }
            }
         break;
      }
      buf_pointer++;
   }


   /* get version */
   if (version_begin > 0) {
      (*message)->version = cl_xml_parse_version((char*)&(buffer[version_begin]), buffer_length-version_begin);
   } else {
      (*message)->version = NULL;
   }

#if CL_DO_XML_DEBUG
   CL_LOG_STR(CL_LOG_DEBUG,"version: ", (*message)->version);
#endif

   return CL_RETVAL_OK;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_xml_parse_SIM()"
int cl_xml_parse_SIM(unsigned char* buffer, unsigned long buffer_length, cl_com_SIM_t** message ) {
   unsigned long buf_pointer = 0;
   int in_tag = 0;
   unsigned long tag_begin = 0;
   unsigned long tag_end = 0;
   unsigned long version_begin = 0;
 
   if (message == NULL || buffer == NULL || *message != NULL) {
      return CL_RETVAL_PARAMS;
   }

   *message = (cl_com_SIM_t*)malloc(sizeof(cl_com_SIM_t));
   if (*message == NULL) {
      return CL_RETVAL_MALLOC;
   }
   
   while(buf_pointer < buffer_length) {
      switch( buffer[buf_pointer] ) {
         case '=':
            if (in_tag == 1) {
               if (version_begin == 0 && cl_xml_parse_is_version((const char *)buffer, tag_begin, buffer_length) == true) {
                  version_begin = buf_pointer + 2;
               }
            }
         break;   
         case '<':
            in_tag = 1;
            tag_begin = buf_pointer + 1;
         break; 
         case '>':
            in_tag = 0;
            tag_end = buf_pointer - 1;
            if (tag_begin < tag_end && tag_begin > 0) {
               const char *charptr = (char*)&(buffer[tag_begin]);
               
               buffer[buf_pointer] = '\0';
               
               if (strcmp(charptr,"/sim") == 0) {
                  buf_pointer++;
               }
            }
         break;
      }
      buf_pointer++;
   }


   /* get version */
   if (version_begin > 0) {
      (*message)->version = cl_xml_parse_version((char*)&(buffer[version_begin]), buffer_length-version_begin);
   } else {
      (*message)->version = NULL;
   }

#if CL_DO_XML_DEBUG
   CL_LOG_STR(CL_LOG_DEBUG,"version: ", (*message)->version);
#endif
   return CL_RETVAL_OK;
}



/* free cl_com_endpoint_t  structure 
  
   params: 
   cl_com_endpoint_t** endpoint -> address of an pointer to cl_com_endpoint_t

   return:
      - *endpoint is set to NULL
      - int - CL_RETVAL_XXXX error number
*/
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_free_endpoint()"
int cl_com_free_endpoint(cl_com_endpoint_t** endpoint) { /* CR check */
   if (endpoint == NULL || *endpoint == NULL) {
      return CL_RETVAL_PARAMS;
   }
   if ((*endpoint)->comp_host != NULL) {
      sge_free(&((*endpoint)->comp_host));
   }
   if ((*endpoint)->comp_name != NULL) {
      sge_free(&((*endpoint)->comp_name));
   }
   if ((*endpoint)->hash_id != NULL) {
      sge_free(&((*endpoint)->hash_id));
   }
   sge_free(endpoint);
   return CL_RETVAL_OK;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_dup_endpoint()"
cl_com_endpoint_t* cl_com_dup_endpoint(cl_com_endpoint_t* endpoint) {
    if (endpoint == NULL || endpoint->comp_host == NULL || endpoint->comp_name == NULL) {
       return NULL;
    }
    return cl_com_create_endpoint(endpoint->comp_host, endpoint->comp_name, endpoint->comp_id, &endpoint->addr);
}



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_create_endpoint()"
cl_com_endpoint_t* cl_com_create_endpoint(const char* host, const char* name,
                                          unsigned long id, const struct in_addr *in_addr)
{
   cl_com_endpoint_t* endpoint = NULL;

   if (host == NULL || name == NULL ) {
      CL_LOG(CL_LOG_ERROR,"parameter errors");
      return NULL;
   }
   if (strlen(name) > 256) {
      CL_LOG(CL_LOG_ERROR,"max supported component name length is 256");
      return NULL;
   }

   endpoint = (cl_com_endpoint_t*)malloc(sizeof(cl_com_endpoint_t));
   if (endpoint == NULL) {
      CL_LOG(CL_LOG_ERROR,"malloc error");
      return NULL;
   }
   endpoint->comp_host = strdup(host);
   endpoint->comp_name = strdup(name);
   endpoint->comp_id   = id;
   endpoint->addr.s_addr = in_addr->s_addr;
   endpoint->hash_id = cl_create_endpoint_string(endpoint);

   if (endpoint->comp_host == NULL || endpoint->comp_name == NULL || endpoint->hash_id == NULL) {
      cl_com_free_endpoint(&endpoint);
      CL_LOG(CL_LOG_ERROR,"malloc error");
      return NULL;
   }
   return endpoint;
}
