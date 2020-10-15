#ifndef __SGE_IO_H
#define __SGE_IO_H
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

#include "sge_tmpnam.h"

/****** uti/io/sge_mode_t *****************************************************
*  NAME
*     sge_mode_t -- Type of operation for sge_copy_append()
*
*  SYNOPSIS
*
*     typedef enum {
*        SGE_MODE_APPEND = 1,
*        SGE_MODE_COPY = 2
*     } sge_mode_t;
*
*  FUNCTION
*     Defines the operation which should be performed by 
*     sge_copy_append():
*
*        SGE_MODE_APPEND - append the file
*        SGE_MODE_COPY   - copy the file
*
*  SEE ALSO
*     uti/io/sge_copy_append();
******************************************************************************/
typedef enum {
   SGE_MODE_APPEND = 1,
   SGE_MODE_COPY = 2
} sge_mode_t;

int sge_readnbytes(register int sfd, register char *ptr, register int n);

int sge_writenbytes(register int sfd, register const char *ptr, 
                    register int n);

int sge_filecmp(const char *file0, const char *file1); 

int sge_copy_append(char *dst, const char *src, sge_mode_t mode);
 
char *sge_bin2string(FILE *fp, int size);
 
int sge_string2bin(FILE *fp, const char *buf);  

char *sge_file2string(const char *fname, int *len);
 
char *sge_stream2string(FILE *fp, int *len);
 
int sge_string2file(const char *str, int len, const char *fname); 
 
#endif /* __SGE_IO_H */

