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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* do not compile in monitoring code */
#ifndef NO_SGE_COMPILE_DEBUG
#define NO_SGE_COMPILE_DEBUG
#endif

#include "sge.h"
#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_dstring.h"
#include "uti/sge_varargs.h"
#include "symbols.h"

#define REALLOC_CHUNK   1024
#define BUFFER_SIZE 20000

#define DSTRING_LAYER BASIS_LAYER

/* JG: TODO: Introduction uti/dstring/--Dynamic_String is missing */

typedef const char* (*sge_dstring_copy_append_f)(dstring *sb, const char *a);

static const char*
sge_dstring_vsprintf_copy_append(dstring *sb,
                                 sge_dstring_copy_append_f function,
                                 const char *format,
                                 va_list ap)
{
   const char *ret = NULL;

   if (sb != NULL && format != NULL && function != NULL) {
      char static_buffer[BUFSIZ];
      int vsnprintf_ret;
      va_list ap_copy;

      va_copy(ap_copy, ap);
      vsnprintf_ret = vsnprintf(static_buffer, BUFSIZ, format, ap_copy);
      va_end(ap_copy);

      /*
       * We have to handle three cases here:
       *    1) If the function returns -1 then vsprintf does not follow 
       *       the C99 standard. We have to increase the buffer until
       *       all parameters fit into the buffer.
       *    2) The function returns a value >BUFSIZE. This indicates
       *       that the function follows the C99 standard. 
       *       vsnprintf_ret is the number of characters which would
       *       have been written to the buffer if it where large enough.
       *       We have to create a buffer of this size.
       *    3) If the return value is >0 and <BUFSIZ than vsprintf
       *       was successfull. We do not need a dyn_buffer.
       */
      if (vsnprintf_ret == -1) {
         size_t dyn_size = 2 * BUFSIZ;
         char *dyn_buffer = sge_malloc(dyn_size);

         while (vsnprintf_ret == -1 && dyn_buffer != NULL) {
            va_copy(ap_copy, ap);
            vsnprintf_ret = vsnprintf(dyn_buffer, dyn_size, format, ap_copy);
            va_end(ap_copy);

            if (vsnprintf_ret == -1) {
               dyn_size *= 2;
               dyn_buffer = sge_realloc(dyn_buffer, dyn_size, 0);
            }
         }
         if (dyn_buffer != NULL) {
            ret = function(sb, dyn_buffer);
            sge_free(&dyn_buffer);
         } else {
            /* error: no memory */
            ret = NULL;
         }
      } else if (vsnprintf_ret > BUFSIZ) {
         char *dyn_buffer = NULL;

         dyn_buffer = (char *)malloc((vsnprintf_ret + 1) * sizeof(char));
         if (dyn_buffer != NULL) {
            va_copy(ap_copy, ap);
            vsnprintf(dyn_buffer, vsnprintf_ret + 1, format, ap_copy);
            va_end(ap_copy);

            ret = function(sb, dyn_buffer);
            sge_free(&dyn_buffer);
         } else {
            /* error: no memory */
            ret = NULL;
         }
      } else {
         ret = function(sb, static_buffer);
      }
   }
   return ret;
}


static void
sge_dstring_allocate(dstring *sb, size_t request)
{  
   /* always request multiples of REALLOC_CHUNK */
   size_t chunks = request / REALLOC_CHUNK + 1;
   request = chunks * REALLOC_CHUNK;

   /* set new size */
   sb->size += request;

   /* allocate memory */
   if (sb->s != NULL) {
      sb->s = sge_realloc(sb->s, sb->size * sizeof(char), 1);
   } else {
      sb->s = malloc(sb->size * sizeof(char));
      sb->s[0] = '\0';
   }
}

/****** uti/dstring/sge_dstring_append() **************************************
*  NAME
*     sge_dstring_append() -- strcat() for dstrings
*
*  SYNOPSIS
*     const char* sge_dstring_append(dstring *sb, const char *a) 
*
*  FUNCTION
*     Append 'a' after 'sb' 
*
*  INPUTS
*     dstring *sb   - dynamic string 
*     const char *a - string 
*
*  NOTES
*     MT-NOTE: sge_dstring_append() is MT safe
*
*  RESULT
*     const char* - result string
******************************************************************************/
const char* sge_dstring_append(dstring *sb, const char *a) 
{
   size_t len;  /* length of string a */

   DENTER(DSTRING_LAYER, "sge_dstring_append");

   if (sb == NULL || a == NULL) {
      DEXIT;
      return NULL;
   }

   len = strlen(a);
 
   if (sb->is_static) {
      if ((sb->length + len) > sb->size )
         len = sb->size - sb->length;

      strncat(sb->s + sb->length, a, len);
      sb->length += len;
   } else {
      size_t required;

      /* only allow to append a string with length 0
         for memory allocation */
      if (len == 0 && sb->s != NULL ) {
         DEXIT;
         return sb->s;
      }

      required = len + sb->length + 1;

      if (required > sb->size) {
         sge_dstring_allocate(sb, required - sb->size);
      }

      strcat(sb->s + sb->length, a);
      sb->length += len;
   }

   DEXIT;
   return sb->s;
}

const char* sge_dstring_append_char(dstring *sb, const char a)
{
   DENTER(DSTRING_LAYER, "sge_dstring_append_char");

   if (sb == NULL) {
      DRETURN(NULL);
   }

   if (a == '\0') {
      DRETURN(NULL);
   }
  
   if (sb->is_static) {
      if (sb->length < sb->size ) {
         sb->s[sb->length++] = a;
         sb->s[sb->length] = '\0';
      }
   } else {
      size_t required = sb->length + 1 + 1;

      if (required > sb->size) {
         sge_dstring_allocate(sb, required - sb->size);
      }

      sb->s[sb->length++] = a;
      sb->s[sb->length] = '\0';
   }

   DRETURN(sb->s);
}

const char* sge_dstring_append_time(dstring *buffer, time_t time, bool as_xml)
{
   struct tm *tm;
   struct tm tm_buffer;
          
   DENTER(DSTRING_LAYER, "sge_dstring_append_time");

   tm = localtime_r(&time, &tm_buffer);

   if (as_xml) {
      sge_dstring_sprintf_append(buffer, "%04d-%02d-%02dT%02d:%02d:%02d",
              1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
              tm->tm_hour, tm->tm_min, tm->tm_sec);
   } else {
      sge_dstring_sprintf_append(buffer, "%02d/%02d/%04d %02d:%02d:%02d",
              tm->tm_mon + 1, tm->tm_mday, 1900 + tm->tm_year,
              tm->tm_hour, tm->tm_min, tm->tm_sec);
   }

   DRETURN(buffer->s);
}

const char* sge_dstring_append_mailopt(dstring *sb, u_long32 mailopt)
{
   DENTER(DSTRING_LAYER, "sge_dstring_append_time");

   if ((MAIL_AT_ABORT | mailopt) == mailopt) {
      sge_dstring_append_char(sb, MAIL_AT_ABORT_SYM);
   }
   if ((MAIL_AT_BEGINNING | mailopt) == mailopt) {
      sge_dstring_append_char(sb, MAIL_AT_BEGINNING_SYM);
   }
   if ((MAIL_AT_EXIT | mailopt) == mailopt) {
      sge_dstring_append_char(sb, MAIL_AT_EXIT_SYM);
   }
   if ((NO_MAIL | mailopt) == mailopt) {
      sge_dstring_append_char(sb, NO_MAIL_SYM);
   }
   if ((MAIL_AT_SUSPENSION | mailopt) == mailopt) {
      sge_dstring_append_char(sb, MAIL_AT_SUSPENSION_SYM);
   }

   DRETURN(sb->s);
}

/****** uti/dstring/sge_dstring_append_dstring() ******************************
*  NAME
*     sge_dstring_append() -- strcat() for dstrings
*
*  SYNOPSIS
*     const char* sge_dstring_append(dstring *sb, const dstring *a) 
*
*  FUNCTION
*     Append 'a' after 'sb' 
*
*  INPUTS
*     dstring *sb      - dynamic string 
*     const dstring *a - string 
*
*  NOTES
*     MT-NOTE: sge_dstring_append_dstring() is MT safe
*
*  RESULT
*     const char* - result string
******************************************************************************/
const char* sge_dstring_append_dstring(dstring *sb, const dstring *a) 
{
   return sge_dstring_append(sb, sge_dstring_get_string(a));
}

/****** uti/dstring/sge_dstring_sprintf() *************************************
*  NAME
*     sge_dstring_sprintf() -- sprintf() for dstrings
*
*  SYNOPSIS
*     const char* sge_dstring_sprintf(dstring *sb, 
*                                     const char *format, ...) 
*
*  FUNCTION
*     see sprintf() 
*
*  INPUTS
*     dstring *sb        - dynamic string 
*     const char *format - format string 
*     ...                - additional parameters 
*
*  RESULT
*     const char* - result string 
*
*  NOTES
*     MT-NOTE: sge_dstring_sprintf() is MT safe
******************************************************************************/
const char* sge_dstring_sprintf(dstring *sb, const char *format, ...)
{
   const char *ret = NULL;

   if (sb != NULL) {
      if (format != NULL) {
         va_list ap;

         va_start(ap, format);
         ret = sge_dstring_vsprintf_copy_append(sb, sge_dstring_copy_string, format, ap);
         va_end(ap);
      } else {
         ret = sb->s;
      }
   }

   return ret;
}

/****** uti/dstring/sge_dstring_vsprintf() *************************************
*  NAME
*     sge_dstring_vsprintf() -- vsprintf() for dstrings
*
*  SYNOPSIS
*     const char* sge_dstring_vsprintf(dstring *sb, const char *format,va_list ap)
*
*  FUNCTION
*     see vsprintf() 
*
*  INPUTS
*     dstring *sb        - dynamic string 
*     const char *format - format string 
*     va_list ap         - argument list
*
*  RESULT
*     const char* - result string 
*
*  NOTES
*     MT-NOTE: sge_dstring_vsprintf() is MT safe
******************************************************************************/
const char* sge_dstring_vsprintf(dstring *sb, const char *format, va_list ap)
{
   const char *ret = NULL;

   if (sb != NULL) {
      if (format != NULL) {
         ret = sge_dstring_vsprintf_copy_append(sb, sge_dstring_copy_string,
                                               format, ap);
      } else {
         ret = sb->s;
      }
   }
   return ret;
}

/****** uti/dstring/sge_dstring_sprintf_append() ******************************
*  NAME
*     sge_dstring_sprintf_append() -- sprintf() and append for dstrings
*
*  SYNOPSIS
*     const char* sge_dstring_sprintf_append(dstring *sb, 
*                                            const char *format, ...) 
*
*  FUNCTION
*     See sprintf() 
*     The string created by sprintf is appended to the existing 
*     contents of the dstring.
*
*  INPUTS
*     dstring *sb        - dynamic string 
*     const char *format - format string 
*     ...                - additional parameters 
*
*  RESULT
*     const char* - result string 
*
*  NOTES
*     MT-NOTE: sge_dstring_sprintf_append() is MT safe
******************************************************************************/
const char* sge_dstring_sprintf_append(dstring *sb, const char *format, ...)
{
   const char *ret = NULL;

   if (sb != NULL) {
      if (format != NULL) {
         va_list ap;

         va_start(ap, format);
         ret = sge_dstring_vsprintf_copy_append(sb, sge_dstring_append, format, ap); 
         va_end(ap);
      } else {
         ret = sb->s;
      }
   }
   return ret;
}

/****** uti/dstring/sge_dstring_copy_string() *********************************
*  NAME
*     sge_dstring_copy_string() -- copy string into dstring 
*
*  SYNOPSIS
*     const char* sge_dstring_copy_string(dstring *sb, char* str) 
*
*  FUNCTION
*     Copy string into dstring 
*
*  INPUTS
*     dstring *sb - destination dstring 
*     char* str   - source string 
*
*  NOTES
*     MT-NOTE: sge_dstring_copy_string() is MT safe
*
*  RESULT
*     const char* - result string 
*******************************************************************************/
const char *sge_dstring_copy_string(dstring *sb, const char *str) 
{
   const char *ret = NULL;

   DENTER(DSTRING_LAYER, "sge_dstring_copy_string");

   if (sb != NULL) {
      sge_dstring_clear(sb);
      ret = sge_dstring_append(sb, str);
   }

   DEXIT;
   return ret;
}

/****** uti/dstring/sge_dstring_copy_dstring() ********************************
*  NAME
*     sge_dstring_copy_dstring() -- strcpy() for dstrings
*
*  SYNOPSIS
*     const char* sge_dstring_copy_dstring(dstring *sb1, 
*                                          const dstring *sb2) 
*
*  FUNCTION
*     strcpy() for dstrings
*
*  INPUTS
*     dstring *sb1 - destination dstring
*     const dstring *sb2 - source dstring 
*
*  NOTES
*     MT-NOTE: sge_dstring_copy_dstring() is MT safe
*
*  RESULT
*     const char* - result string buffer 
*******************************************************************************/
const char *sge_dstring_copy_dstring(dstring *sb1, const dstring *sb2) 
{
   const char *ret = NULL;

   DENTER(DSTRING_LAYER, "sge_dstring_copy_dstring");

   if (sb1 != NULL) {
      sge_dstring_clear(sb1);
      ret = sge_dstring_append(sb1, sge_dstring_get_string(sb2));
   }

   DEXIT;
   return ret;
}

/****** uti/dstring/sge_dstring_free() ****************************************
*  NAME
*     sge_dstring_free() -- sge_free() for dstrings
*
*  SYNOPSIS
*     void sge_dstring_free(dstring *sb) 
*
*  FUNCTION
*     Frees a dynamically allocated string 
*
*  NOTES
*     MT-NOTE: sge_dstring_free() is MT safe
*
*  INPUTS
*     dstring *sb - dynamic string 
******************************************************************************/
void sge_dstring_free(dstring *sb) 
{
   if (sb != NULL && !sb->is_static && sb->s != NULL) {
      sge_free(&(sb->s));
      sb->size = 0;
      sb->length = 0;
   }
}   

/****** uti/dstring/sge_dstring_clear() ****************************************
*  NAME
*     sge_dstring_clear() -- empty a dstring
*
*  SYNOPSIS
*     void sge_dstring_clear(dstring *sb) 
*
*  FUNCTION
*     Set a dstring to an empty string.
*
*  NOTES
*     MT-NOTE: sge_dstring_clear() is MT safe
*
*  INPUTS
*     dstring *sb - dynamic string 
******************************************************************************/
void sge_dstring_clear(dstring *sb) 
{
   if (sb == NULL)
      return;

   if (sb->s != NULL) {
      sb->s[0] = '\0';
   }

   sb->length = 0;
}   

/****** uti/dstring/sge_dstring_get_string() **********************************
*  NAME
*     sge_dstring_get_string() -- Returns string buffer 
*
*  SYNOPSIS
*     const char* sge_dstring_get_string(const dstring *string) 
*
*  FUNCTION
*     Returns a pointer to the buffer where the string is stored.
*     The pointer is not valid until doomsday. The next
*     sge_dstring_* call may make it invalid.
*
*  INPUTS
*     const dstring *string - pointer to dynamic string 
*
*  NOTES
*     MT-NOTE: sge_dstring_get_string() is MT safe
*
*  RESULT
*     const char* - pointer to string buffer
*******************************************************************************/
const char *sge_dstring_get_string(const dstring *sb)
{
   return (sb != NULL) ? sb->s : NULL;
}


/****** uti/dstring/sge_dstring_strlen() **************************************
*  NAME
*     sge_dstring_strlen() -- strlen() for dstrings
*
*  SYNOPSIS
*     size_t sge_dstring_strlen(const dstring *string) 
*
*  FUNCTION
*     strlen() for dstrings
*
*  INPUTS
*     const dstring *string - pointer to dynamic string 
*
*  NOTES
*     MT-NOTE: sge_dstring_strlen() is MT safe
*
*  RESULT
*     size_t - string length
*******************************************************************************/
size_t sge_dstring_strlen(const dstring *sb)
{
   size_t ret = 0;

   if (sb != NULL) {
      ret = sb->length;
   }

   return ret;
}

/****** uti/dstring/sge_dstring_remaining() **************************************
*  NAME
*     sge_dstring_remaining() -- remaining chars in dstring
*
*  SYNOPSIS
*     size_t sge_dstring_remaining(const dstring *string) 
*
*  FUNCTION
*     Returns number of chars remaining in dstrings.
*
*  INPUTS
*     const dstring *string - pointer to dynamic string 
*
*  NOTES
*     MT-NOTE: sge_dstring_remaining() is MT safe
*
*  RESULT
*     size_t - remaining chars
*******************************************************************************/
size_t sge_dstring_remaining(const dstring *sb)
{
   size_t ret = 0;

   if (sb != NULL) {
      if (sb->is_static) {
         ret = sb->size - sb->length;
      } else {
         ret = U_LONG32_MAX;
      }
   }

   return ret;
}

/****** uti/dstring/sge_dstring_init() **************************************
*  NAME
*     sge_dstring_init() -- init static dstrings
*
*  SYNOPSIS
*     size_t sge_dstring_init(dstring *string, char *s, size_t size) 
*
*  FUNCTION
*     Initialize dstring with a static buffer.
*
*  INPUTS
*     const dstring *string - pointer to dynamic string 
*
*  NOTES
*     MT-NOTE: sge_dstring_init() is MT safe
*
*  RESULT
*     size_t - remaining chars
*******************************************************************************/
void sge_dstring_init(dstring *sb, char *s, size_t size)
{
   if (sb != NULL && s != NULL) {
      sb->is_static = true;
      sb->length = 0;
      sb->size = size - 1;   /* leave space for trailing 0 */
      sb->s = s;
      sb->s[0] = '\0';
   }
}

/****** uti/dstring/sge_dstring_ulong_to_binstring() **************************
*  NAME
*     sge_dstring_ulong_to_binstring() -- convert ulong into bin-string 
*
*  SYNOPSIS
*     const char* 
*     sge_dstring_ulong_to_binstring(dstring *sb, u_long32 number) 
*
*  FUNCTION
*     Convert ulong into bin-string 
*
*  INPUTS
*     dstring *sb     - dstring 
*     u_long32 number - 32 bit ulong value 
*
*  RESULT
*     const char* - pointer to dstrings internal buffer
*******************************************************************************/
const char *sge_dstring_ulong_to_binstring(dstring *sb, u_long32 number)
{
   char buffer[33] = "                              ";
   int i = 31;

   while (number > 0) {
      if ((number % 2) > 0) {
         buffer[i] = '1';
      } else {
         buffer[i] = '0';
      }
      i--;
      number /= 2;
   }  
   sge_strip_blanks(buffer);
   sge_dstring_sprintf(sb, "%s", buffer);
   return sge_dstring_get_string(sb);
}

/****** uti/dstring/sge_dstring_split() ****************************************
*  NAME
*     sge_dstring_split() -- splits a string into two parts 
*
*  SYNOPSIS
*     bool 
*     sge_dstring_split(dstring *string, char character, 
*                       dstring *before, dstring *after)
*
*  FUNCTION
*     This functions tries to find the first occurrence of "character"
*     in "string". The characters before will be copied into "before"
*     and the characters behind into "after" dstring.
*
*  INPUTS
*     dstring *sb     - dstring 
*     char character  - character
*     dstring *before - characters before
*     dstring *after  - characters after
*
*  RESULT
*     error state
*        true  - success
*        false - error 
*******************************************************************************/
bool
sge_dstring_split(dstring *string, char character, dstring *before, dstring *after)
{
   bool ret = true;

   DENTER(DSTRING_LAYER, "sge_dstring_split");
   if (string != NULL && before != NULL && after != NULL) {
      const char *s = sge_dstring_get_string(string);
      const char *end = strchr(s, character);

      while (end != NULL && s != end) {
         sge_dstring_append_char(before, *(s++));
      }
      if (*s == character) {
         s++;
      }
      sge_dstring_append(after, s);
   }
   DRETURN(ret);
}

/****** uti/dstring/sge_dstring_strip_white_space_at_eol() *********************
*  NAME
*     sge_dstring_strip_white_space_at_eol() -- as it says 
*
*  SYNOPSIS
*     void sge_dstring_strip_white_space_at_eol(dstring *string)
*
*  FUNCTION
*     removes whitespace at the end of the given "string".
*
*  INPUTS
*     dstring *string - dstring 
*******************************************************************************/
void sge_dstring_strip_white_space_at_eol(dstring *string)
{
   DENTER(DSTRING_LAYER, "sge_strip_white_space_at_eol");
   if (string != NULL) {
      char *s = (string != NULL) ? string->s : NULL;

      if (s != NULL) {
         sge_strip_white_space_at_eol(s);
      }
   }  
   DRETURN_VOID;
}      

#if 0 /* EB: DEBUG: */
int main(void)
{
   char *s;
   dstring sb = DSTRING_INIT;    /* initialize */

   /*
    * change content
    */
   s = sge_dstring_append(&sb, "Trala");
   s = sge_dstring_append(&sb, " trolo");
   s = sge_dstring_append(&sb, " troet");
   s = sge_dstring_sprintf(&sb, "%d, %s, %f\n", 5, "rabarber ", 5.6);

   /*
    * use string
    */
   printf("%s\n", s);
   printf("%s\n", sge_dstring_get_string(&sb));

   /*
    * free the string when no longer needed
    */
   sge_dstring_free(&sb);
   return 0;
}
#endif
