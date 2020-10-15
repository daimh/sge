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
#include <string.h>
#include <stdlib.h>

#include "comm/lists/cl_util.h"
#include "comm/lists/cl_errors.h"

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_util_get_number_length()"
int cl_util_get_ulong_number_length(unsigned long id) {
   char help[512];
#if defined(_WIN32)
   _snprintf(help,512,"%lu",id);
#else
   snprintf(help,512,"%lu",id);
#endif
   return (int)strlen(help);
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_util_get_int_number_length()"
int cl_util_get_int_number_length(int id) {
   char help[512];
#if defined(_WIN32)
   _snprintf(help,512,"%d",id);
#else
   snprintf(help,512,"%d",id);
#endif
   return (int)strlen(help);
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_util_get_double_number_length()"
int cl_util_get_double_number_length(double id) {
   char help[512];
#if defined(_WIN32)
   _snprintf(help,512,"%f",id);
#else
   snprintf(help,512,"%f",id);
#endif
   return (int)strlen(help);
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_util_get_ulong_value()"
unsigned long cl_util_get_ulong_value(const char* text) {
   unsigned long value = 0;
   if (text != NULL) {
      sscanf(text, "%lu" , &value);
   }
   return value;
}



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_util_get_ascii_hex_buffer()"
int cl_util_get_ascii_hex_buffer(unsigned char* buffer, unsigned long buf_len, char** ascii_buffer, char* separator) {
   char*         asc_buffer       = NULL;
   unsigned long asc_buffer_size  = 0;
   unsigned long asc_buffer_index = 0;
   unsigned long buffer_index     = 0;
   int           sep_length       = 0;

   if (buffer == NULL || ascii_buffer == NULL) {
      return CL_RETVAL_PARAMS;
   }
 
   if (*ascii_buffer != NULL) {
      return CL_RETVAL_PARAMS;
   }

   if (separator == NULL) {
      sep_length = 0;
   } else {
      sep_length = strlen(separator);
   }
   asc_buffer_size = buf_len * (2 + sep_length) + 1;

   asc_buffer = (char*) malloc(sizeof(char)*asc_buffer_size);
   if (asc_buffer == NULL) {
      return CL_RETVAL_MALLOC;
   }

   asc_buffer_index = 0;
   for(buffer_index = 0; buffer_index < buf_len; buffer_index++) {
      asc_buffer[asc_buffer_index++] = cl_util_get_ascii_hex_char( (buffer[buffer_index] & 0xf0) >> 4 );
      asc_buffer[asc_buffer_index++] = cl_util_get_ascii_hex_char( (buffer[buffer_index] & 0x0f) );
      if (separator != NULL && (buffer_index + 1) < buf_len) {
         strncpy(&asc_buffer[asc_buffer_index], separator, sep_length);
         asc_buffer_index += sep_length;
      }
   }
   asc_buffer[asc_buffer_index] = '\0';
   
   *ascii_buffer = asc_buffer;

   return CL_RETVAL_OK;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_util_get_binary_buffer()"
int cl_util_get_binary_buffer(char* hex_buffer, unsigned char** buffer, unsigned long* buffer_lenght) {
   unsigned char* bin_buffer = NULL;
   unsigned long  bin_buffer_len = 0;
   unsigned long  bin_buffer_index = 0;
   unsigned long  hex_buffer_index = 0;
   unsigned long  hex_buffer_len = 0;
   int            hi=0;
   int            lo=0;

   if (hex_buffer == NULL || buffer == NULL || buffer_lenght == NULL) {
      return CL_RETVAL_PARAMS;
   }
   if (*buffer != NULL) {
      return CL_RETVAL_PARAMS;
   }

   hex_buffer_len = strlen(hex_buffer);

   if (hex_buffer_len % 2 != 0) {
      return CL_RETVAL_PARAMS;
   }
   bin_buffer_len = hex_buffer_len / 2;
   bin_buffer = (unsigned char*) malloc(sizeof(char) * bin_buffer_len);
   if (bin_buffer == NULL) {
      return CL_RETVAL_MALLOC;
   }

   while(bin_buffer_index < bin_buffer_len) {
      hi = cl_util_get_hex_value(hex_buffer[hex_buffer_index++]);
      lo = cl_util_get_hex_value(hex_buffer[hex_buffer_index++]);
      if (hi != -1 && lo != -1) {
         bin_buffer[bin_buffer_index++] = (hi << 4) + lo; 
      } else {
         free(bin_buffer);
         return CL_RETVAL_UNEXPECTED_CHARACTERS;
      }
   }

   *buffer_lenght = bin_buffer_len;
   *buffer = bin_buffer;

   return CL_RETVAL_OK;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_util_get_hex_value()"
int  cl_util_get_hex_value(char hex_char) {
   int ret_val;
   switch(hex_char) {
      case 'f':
      case 'F':
         ret_val = 15;
         break;
      case 'e':
      case 'E':
         ret_val = 14;
         break;
      case 'd':
      case 'D':
         ret_val = 13;
         break;
      case 'c':
      case 'C':
         ret_val = 12;
         break;
      case 'b':
      case 'B':
         ret_val = 11;
         break;
      case 'a':
      case 'A':
         ret_val = 10;
         break;
      case '9':
         ret_val = 9;
         break;
      case '8':
         ret_val = 8;
         break;
      case '7':
         ret_val = 7;
         break;
      case '6':
         ret_val = 6;
         break;
      case '5':
         ret_val = 5;
         break;
      case '4':
         ret_val = 4;
         break;
      case '3':
         ret_val = 3;
         break;
      case '2':
         ret_val = 2;
         break;
      case '1':
         ret_val = 1;
         break;
      case '0':
         ret_val = 0;
         break;
      default:
         ret_val = -1;
   }
   return ret_val;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_util_get_ascii_hex_char()"
char cl_util_get_ascii_hex_char(unsigned char value) {
   char ret_val;
   switch(value) {
      case 15:
         ret_val = 'f';
         break;
      case 14:
         ret_val = 'e';
         break;
      case 13:
         ret_val = 'd';
         break;
      case 12:
         ret_val = 'c';
         break;
      case 11:
         ret_val = 'b';
         break;
      case 10:
         ret_val = 'a';
         break;
      case 9:
         ret_val = '9';
         break;
      case 8:
         ret_val = '8';
         break;
      case 7:
         ret_val = '7';
         break;
      case 6:
         ret_val = '6';
         break;
      case 5:
         ret_val = '5';
         break;
      case 4:
         ret_val = '4';
         break;
      case 3:
         ret_val = '3';
         break;
      case 2:
         ret_val = '2';
         break;
      case 1:
         ret_val = '1';
         break;
      case 0:
         ret_val = '0';
         break;
      default:
         ret_val = '?';
   }
   return ret_val;
}
