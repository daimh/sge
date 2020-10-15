#ifndef __SGE_STDIO_H
#define __SGE_STDIO_H
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
 *   Copyright (C) 2012 Mark Dixon, University of Leeds
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>  
#include <unistd.h>

#include "basis_types.h"

/* On some systems, FOPEN is already defined as value -1 */
#undef FOPEN

#define FOPEN(var,fname,fmode) \
   if((var = fopen(fname,fmode)) == NULL) { \
      goto FOPEN_ERROR; \
   }

/****** uti/stdio/FPRINTF() ***************************************************
*  NAME
*     FPRINTF() -- fprintf() macro 
*
*  SYNOPSIS
*     #define FPRINTF(arguments)
*     void fprintf(FILE *stream, const char *format, ...)
*
*  FUNCTION
*     This FPRINTF macro has to be used similar to the fprintf 
*     function. It is not necessary to check the return value. 
*     In case of an error the macro will jump to a defined label.
*     The label name is 'FPRINTF_ERROR'.
*
*  INPUTS
*     FILE *stream       - output stream
*     const char *format - format string
*     ...
*
*  NOTES
*     Don't forget to define the 'FPRINTF_ERROR'-label
******************************************************************************/
#define FPRINTF(x) \
   if (fprintf x < 0) { \
      goto FPRINTF_ERROR; \
   }

/****** uti/stdio/FPRINTF_ASSIGN() *******************************************
*  NAME
*     FPRINTF_ASSIGN() -- fprintf() macro with return value assignment 
*
*  SYNOPSIS
*     #define FPRINTF_ASSIGN(var, arguments)
*     void fprintf(FILE *stream, const char *format, ...)
*
*  FUNCTION
*     This FPRINTF macro has to be used similar to the fprintf 
*     function. It is not necessary to check the return value. 
*     In case of an error the macro will jump to a defined label.
*     The label name is 'FPRINTF_ERROR'. This is a variarion of 
*     FPRINTF() that allows assigning the fprintf() return value to
*     the variable passed as first makro argument.
*
*  INPUTS
*     FILE *stream       - output stream
*     const char *format - format string
*     ...
*
*  NOTES
*     Don't forget to define the 'FPRINTF_ERROR'-label
******************************************************************************/
#define FPRINTF_ASSIGN(var, x) \
   if ((var = fprintf x) < 0) { \
      goto FPRINTF_ERROR; \
   }

/****** uti/stdio/FCLOSE() ****************************************************
*  NAME
*     FCLOSE() -- fclose() macro 
*
*  SYNOPSIS
*     #define FCLOSE(argument)
*     int fclose(FILE *stream)
*
*  FUNCTION
*     This FCLOSE macro has to be used similar to the fclose 
*     function. It is not necessary to check the return value. 
*     In case of an error the macro will jump to a defined label.
*     The label name is 'FCLOSE_ERROR'.
*
*  INPUTS
*     FILE *stream       - output stream
*
*  NOTES
*     Don't forget to define the 'FCLOSE_ERROR'-label
******************************************************************************/
#if defined(IRIX)
#define FCLOSE(x) \
   if (x != NULL) { \
      fsync(fileno(x)); \
      if (fclose(x) != 0) { \
         goto FCLOSE_ERROR; \
      } \
   }
#else
#define FCLOSE(x) \
   if (x != NULL) { \
      if (fclose(x) != 0) { \
         goto FCLOSE_ERROR; \
      } \
   }
#endif

#define FCLOSE_IGNORE_ERROR(x) fclose(x)

/****** uti/stdio/CLOSE() ****************************************************
*  NAME
*     CLOSE() -- close() macro
*
*  SYNOPSIS
*     #define CLOSE(argument)
*     int close(int fd)
*
*  FUNCTION
*     This CLOSE macro has to be used similar to the close
*     function. It is not necessary to check the return value.
*     In case of an error the macro will jump to a defined label.
*     The label name is 'CLOSE_ERROR'.
*
*  INPUTS
*     int fd       - file descriptor
*
*  NOTES
*     Don't forget to define the 'CLOSE_ERROR'-label
******************************************************************************/
/* The check for x>0 is because in various places the descriptor is
   initialized to -1 and a close is attempted on it even if it hasn't
   been given a proper value.  */
#define CLOSE(x)                    \
   if ((x) >= 0 && close(x) != 0) { \
      goto CLOSE_ERROR;             \
   }

#define CLOSE_IGNORE_ERROR(x) (void) close(x)

pid_t sge_peopen(const char *shell, int login_shell, const char *command, 
                 const char *user, char **env, FILE **fp_in, FILE **fp_out, 
                 FILE **fp_err, bool null_stderr);
 
int sge_peclose(pid_t pid, FILE *fp_in, FILE *fp_out, FILE *fp_err, 
                struct timeval *timeout); 

void print_option_syntax(FILE *fp, const char *option, const char *meaning);

bool sge_check_stdout_stream(FILE *file, int fd);

pid_t sge_peopen_r(const char *shell, int login_shell, const char *command,
                 const char *user, char **env,  FILE **fp_in, FILE **fp_out,
                 FILE **fp_err, bool null_stderr);

#if defined(SOLARIS)
#define SGE_DEFAULT_PATH "/usr/local/bin:/bin:/usr/bin:/usr/ucb"
#elif defined(IRIX) 
#define SGE_DEFAULT_PATH "/usr/local/bin:/bin:/usr/bin:/usr/bsd"
#else
#define SGE_DEFAULT_PATH "/usr/local/bin:/bin:/usr/bin"
#endif


#endif /* __SGE_STDIO_H */
