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

#include <stdio.h>
#include <stdlib.h>     
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "uti/sge_rmon.h"
#include "uti/sge_io.h"
#include "uti/sge_stdio.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_unistd.h"
#include "uti/sge_log.h"

#include "uti/msg_utilib.h"  

#define BUFFER     4096
#define FILE_CHUNK (100*1024)  

/****** uti/io/sge_readnbytes() ***********************************************
*  NAME
*     sge_readnbytes() -- Read n bytes from file descriptor  
*
*  SYNOPSIS
*     int sge_readnbytes(int sfd, char *ptr, 
*                        int n) 
*
*  FUNCTION
*     Read n bytes from file descriptor. 
*
*  INPUTS
*     int sfd   - file descriptor 
*     char *ptr - pointer to buffer 
*     int n     - number of bytes 
*
*  RESULT
*     int - number of bytes read
*  
*  SEE ALSO
*     uti/io/sge_writenbytes()
*  
*  NOTES
*     MT-NOTE: sge_readnbytes() is MT safe
******************************************************************************/
int sge_readnbytes(int sfd, char *ptr, int n) 
{
   int i;                       /* number of bytes read */
   int nleft = n;               /* number of bytes still to read */

   DENTER(BASIS_LAYER, "sge_readnbytes");
   DPRINTF(("TOTAL BYTES TO BE READ %d\n", n));

   /* Read n bytes */
   while (nleft > 0) {
      i = read(sfd, ptr, nleft);
      DPRINTF(("read %d bytes on fd %d\n", i, sfd));

      if (i < 0) {
         DPRINTF(("sge_readnbytes: returning %d\n", i));
         DEXIT;
         return (i);
      } else {
         if (i == 0)
            break;
      } 
      nleft -= i;
      ptr += i;
   } 

   DPRINTF(("sge_readnbytes: returning %d\n", nleft));
   DEXIT;
   return (n - nleft);

}            

/****** uti/io/sge_writenbytes() **********************************************
*  NAME
*     sge_writenbytes() -- Write n bytes to file descriptor 
*
*  SYNOPSIS
*     int sge_writenbytes(int sfd, const char *ptr, 
*                         int n) 
*
*  FUNCTION
*     Write n bytes to file descriptor 
*
*  INPUTS
*     int sfd         - file descriptor 
*     const char *ptr - pointer to buffer 
*     int n           - number of bytes 
*
*  RESULT
*     int - number of bytes written
*
*  SEE ALSO
*     uti/io/sge_readnbytes()
*
*  NOTES
*     MT-NOTE: sge_writenbytes() is MT safe
******************************************************************************/
int sge_writenbytes(int sfd, const char *ptr, 
                    int n) 
{

   int i;                       /* number of bytes written */
   int nleft = n;               /* number of bytes still to write */

   DENTER(BASIS_LAYER, "sge_writenbytes");

   /* Write n bytes */
   while (nleft > 0) {
      DTRACE;
      i = write(sfd, ptr, nleft);
      if (i == -1) {
         DPRINTF(("write failed with error %d: %s\n", i, strerror(errno)));
      } else {
         DPRINTF(("wrote %d bytes on fd %d\n", i, sfd));
      }

      if (i <= 0) {
         DPRINTF(("sge_writenbytes: returning %d\n", i));
         DEXIT;
         return (i);
      }              
      nleft -= i;
      ptr += i;
   }                   

   DEXIT;
   return (n);
}

/****** uti/io/sge_filecmp() **************************************************
*  NAME
*     sge_filecmp() -- Compare two files
*
*  SYNOPSIS
*     int sge_filecmp(const char *name0, const char *name1)
*
*  FUNCTION
*     Compare two files. They are equal if:
*        - both of them have the same name
*        - if a stat() succeeds for both files and
*          i-node/device-id are equal
*
*     we are not sure
*     - if stat() failes for at least one of the files
*       (It could be that both pathes direct to the same
*       file not existing)
*
*  INPUTS
*     const char *name0 - 1st filename
*     const char *name1 - 2nd filename
*
*  RESULT
*     int - Identical?
*         0 - Yes.
*         1 - No they are not equivalent.
*
*  NOTES
*     MT-NOTE: sge_filecmp() is MT safe
******************************************************************************/
int sge_filecmp(const char *name0, const char *name1)
{
   SGE_STRUCT_STAT buf0, buf1;
 
   DENTER(TOP_LAYER, "filecmp");
 
   if (!strcmp(name0, name1)) {
      DEXIT;
      return 0;
   }
 
   if (SGE_STAT(name0, &buf0)<0) {
      DEXIT;
      return 1;
   }
 
   if (SGE_STAT(name1, &buf1)<0) {
      DEXIT;
      return 1;
   }
 
   if (buf0.st_ino == buf1.st_ino && buf0.st_dev == buf1.st_dev) {
      DEXIT;
      return 0;
   } else {
      DEXIT;
      return 1;
   }
}          

/****** uti/io/sge_copy_append() **********************************************
*  NAME
*     sge_copy_append() -- Copy/append one file to another 
*
*  SYNOPSIS
*     int sge_copy_append(char *src, const char *dst, 
*                         sge_mode_t mode) 
*
*  FUNCTION
*     Copy/append content from 'src' to 'dst' 
*
*  INPUTS
*     char *src       - source filename 
*     const char *dst - destination filename 
*     sge_mode_t mode - mode 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
*
*  SEE ALSO
*     uti/io/sge_mode_t
*
*  NOTES
*     MT-NOTE: sge_copy_append() is MT safe
******************************************************************************/
int sge_copy_append(char *src, const char *dst, sge_mode_t mode)
{
#define CPBUF 1024

   char buf[CPBUF];
   int fdsrc, fddst, modus, rs, ws;
   bool error;

   DENTER(TOP_LAYER, "sge_copy_append");
  
   if (src == NULL || dst == NULL || strlen(src) == 0 || strlen(dst) == 0 ||
      !(mode == SGE_MODE_APPEND || mode == SGE_MODE_COPY)) {
      DEXIT;
      return -1;   
   }
   if (!strcmp(src, dst)) {
      DEXIT;
      return -1;
   }   
 
   /* Return if source file doesn't exist */
   if ((fdsrc = SGE_OPEN2(src, O_RDONLY)) == -1) {
      DEXIT;
      return -1;
   }   
     
   if (mode == SGE_MODE_APPEND)
      modus = O_WRONLY | O_APPEND | O_CREAT;
   else
      modus = O_WRONLY | O_CREAT;      
    
   if ((fddst = SGE_OPEN3(dst, modus, 0666)) == -1) {
      DEXIT;
      return -1;
   }    
    
   error = false;
   while (!error) {
      rs = read(fdsrc, buf, 512);
      if (rs == -1 && errno == EINTR)
         continue;
      else if (rs == -1)
         error = true;
    
      if (!error && rs > 0) {      
         while (!error) {   
            ws = write(fddst, buf, rs);
            if (ws == -1 && errno == EINTR)   
               continue;
            else if (ws == -1) {
               error = true;
               break;
            } 
            else
               break;
         }
      }
      if (error)
         break;
      if (rs == 0)
         break;   
   }           
    
   close(fdsrc);
   close(fddst);
 
   DEXIT;
   return (error ? -1: 0);
}

/****** uti/io/sge_bin2string() ***********************************************
*  NAME
*     sge_bin2string() -- Put binary stream into a string 
*
*  SYNOPSIS
*     char* sge_bin2string(FILE *fp, int size) 
*
*  FUNCTION
*     Read a binary steam from given file descriptor 'fp' and
*     write it into (dynamically) malloced buffer as "ASCII" format.
*  
*     "ASCII" format means:
*           '\0' is written as '\\' '\0' 
*           '\\' is written as '\\' '\\'
*           End of buffer is written as '\0'
*
*  INPUTS
*     FILE *fp - file descriptor 
*     int size - size of the buffer used within this function 
*
*  RESULT
*     char* - malloced buffer
*
*  SEE ALSO
*     uti/io/sge_string2bin()
*
*  NOTES
*     MT-NOTE: sge_bin2string() is MT safe
******************************************************************************/
char *sge_bin2string(FILE *fp, int size) 
{
   int i, fd;
   char inbuf[BUFFER], outbuf[2*BUFFER];
   char *inp, *outp;
   char *dstbuf;
   int len,             /* length of current tmp buffer */
       dstbuflen,       /* total length of destination buffer */
       chunksize,       /* chunks for realloc */
       lastpos,         /* last position in destination buffer */
       error;
   
   if ((fd = fileno(fp)) == -1)
      return NULL;

   chunksize = 20480;
   
   if (size <= 0)       /* no idea about buffer, malloc in chunks */
      size = chunksize;

   dstbuf = (char *) malloc(size+1);
   dstbuflen = size;
   lastpos = 0;

   error = false;

   while (!error) {
      i = read(fd, inbuf, BUFFER);
      if (i > 0) {
         inp = inbuf;
         outp = outbuf;
         while (inp < &inbuf[i]) {
            if (*inp == '\\') {
               *outp++ = '\\';
               *outp++ = '\\';
            }
            else if (*inp == '\0') {
               *outp++ = '\\';
               *outp++ = '0';
            }
            else
               *outp++ = *inp;
            inp++;
         }


         len = outp - outbuf;

         if (lastpos + len > dstbuflen) {
            if ((dstbuf = sge_realloc(dstbuf, lastpos + len + chunksize, 0)) == NULL) {
               error = true;
               break;
            }   
            dstbuflen = lastpos + len + chunksize;

         }
         
         memcpy(&dstbuf[lastpos], outbuf, len);
         lastpos += len;

      }
      else if (i == 0) {
         break;
      }
      else {
         if (errno != EINTR) {
            error=true;
            break;
         }
      }
   }

   if (error) {
      sge_free(&dstbuf);
      return NULL;
   }
   else {
      if ((dstbuf = sge_realloc(dstbuf, lastpos + 1, 0)) == NULL) {
         return NULL;
      }
      dstbuf[lastpos] = '\0';
      return dstbuf;
   }
}

/****** uti/io/sge_string2bin() ***********************************************
*  NAME
*     sge_string2bin() -- Write 'binary' string into file 
*
*  SYNOPSIS
*     int sge_string2bin(FILE *fp, const char *buf) 
*
*  FUNCTION
*     Write 'binary' string into file 
*
*  INPUTS
*     FILE *fp        - file descriptor 
*     const char *buf - "ASCII" string (see sge_bin2string())
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
*
*  SEE ALSO
*     uti/io/sge_bin2string()
*
*  NOTES
*     MT-NOTE: sge_string2bin() is MT safe
******************************************************************************/
int sge_string2bin(FILE *fp, const char *buf) 
{
   char outbuf[BUFFER];
   char *outp;
   int fd;
      
   if ((fd = fileno(fp)) == -1)
      return -1;
   
   if (!buf)
      return -1;
         
   while (*buf) {
      outp = outbuf;
      while (*buf && (outp - outbuf < BUFFER)) {
         if (*buf == '\\') {
            if (*(buf+1) == '\\')
               *outp++ = '\\';
            else
               *outp++ = '\0';
            buf+=2;    
         }   
         else
            *outp++ = *buf++;
      }
      
      if (write(fd, outbuf, outp - outbuf) != outp - outbuf)
         return -1;
   }
   fflush(fp);
   return 0;
}

/****** uti/io/sge_file2string() **********************************************
*  NAME
*     sge_file2string() -- Load file into string
*
*  SYNOPSIS
*     char* sge_file2string(const char *fname, int *len)
*
*  FUNCTION
*     Load file into string. Returns a pointer to a string buffer containing
*     the file contents and the size of the buffer (= number of bytes read)
*     in the variable len.
*     If the file cannot be read (doesn't exist, permissions etc.), NULL is
*     returned as buffer and len is set to 0.
*
*  INPUTS
*     const char *fname - filename
*     int *len          - number of bytes read
*
*  RESULT
*     char* - malloced string buffer
*
*  SEE ALSO
*     uti/io/sge_string2file()
*     uti/io/sge_stream2string()
*
*  NOTES
*     MT-NOTE: sge_file2string() is MT safe
******************************************************************************/
char *sge_file2string(const char *fname, int *len)
{
   FILE *fp;
   SGE_STRUCT_STAT statbuf;
   int size, i;
   char *str;
 
   DENTER(CULL_LAYER, "sge_file2string");

   /* initialize len - in case of errors we want to return 0 
    * JG: TODO: it would be better to return -1. Check if calling
    * functions would handle this situation.
    */
   if (len != NULL) {
      *len = 0;
   }

   /* try file access, read file info */
   if (SGE_STAT(fname, &statbuf)) {
      DEXIT;
      return NULL;
   }
 
   size = statbuf.st_size;
 
   if ((fp = fopen(fname, "r")) == NULL) {
      ERROR((SGE_EVENT, MSG_FILE_FOPENFAILED_SS, fname, strerror(errno)));
      DEXIT;
      return NULL;
   }
 
   if ((str = malloc(size+1)) == NULL) {
      FCLOSE(fp);
      DEXIT;
      return NULL;
   }

   str[0] = '\0';

   /*
   ** With fread(..., size, 1, ...),
   ** Windows cannot read <size> bytes here, because in
   ** text mode the trailing ^Z is ignored.
   ** CRLF -> LF conversion reduces size even further.
   ** Therefore, the file has less than size-1 bytes if read
   ** in text (ascii) mode.
   ** Correctly, fread returns 0, because 0 elements of
   ** size <size> were read.
   */
   if (size > 0) {
#ifdef WIN32 /* fread call and evaluation of return value is different */
      i = fread(str, 1, size, fp);
      if (i == 0) {
         sge_free(&str);
         FCLOSE(fp);
         DEXIT;
         return NULL;
      }
      str[i] = '\0';    /* delimit this string */   
      if (len != NULL) {
         *len = i;
      }
#else
      i = fread(str, size, 1, fp);
      if (i != 1) {
         ERROR((SGE_EVENT, MSG_FILE_FREADFAILED_SS, fname, strerror(errno)));
         sge_free(&str);
         FCLOSE(fp);
         DEXIT;
         return NULL;
      }
      str[size] = '\0';    /* delimit this string */
      if (len != NULL) {
         *len = size;
      }
#endif
   } 
 
   FCLOSE(fp);

   DEXIT;
   return str;
FCLOSE_ERROR:
   sge_free(&str);
   DEXIT;
   return NULL;
}
 
/****** uti/io/sge_stream2string() ********************************************
*  NAME
*     sge_stream2string() -- Read string from stream
*
*  SYNOPSIS
*     char* sge_stream2string(FILE *fp, int *len)
*
*  FUNCTION
*     Read string from stream
*
*  INPUTS
*     FILE *fp - file descriptor
*     int *len - number of bytes read
*
*  RESULT
*     char* - pointer to malloced string buffer
*
*  SEE ALSO
*     uti/io/sge_file2string()
*     uti/io/sge_string2file()
*
*  NOTES
*     MT-NOTE: sge_stream2string() is MT safe
******************************************************************************/ 
char *sge_stream2string(FILE *fp, int *len)
{
   char *str;
   int filled = 0;
   int malloced_len, i;
 
   DENTER(TOP_LAYER, "sge_stream2string");
 
   if (!(str = malloc(FILE_CHUNK))) {
      DEXIT;
      return NULL;
   }
   malloced_len = FILE_CHUNK;
 
   /* malloced_len-filled-1 cause we reserve space for \0 termination */
   while ((i = fread(&str[filled], 1, malloced_len-filled-1, fp)) > 0) {
      filled += i;
      if (malloced_len == filled+1) {
         str = sge_realloc(str, malloced_len + FILE_CHUNK, 0);
         if (str == NULL) {
            DEXIT;
            return NULL;
         }
         malloced_len += FILE_CHUNK;
      }
 
      if (feof(fp)) {
         DPRINTF(("got EOF\n"));
         break;
      }
   }
   str[filled] = '\0';  /* NULL termination */
   *len = filled;
 
   DEXIT;
   return str;
}
 
/****** uti/io/sge_string2file() **********************************************
*  NAME
*     sge_string2file() -- Write string into file
*
*  SYNOPSIS
*     int sge_string2file(const char *str, int len, const char *fname)
*
*  FUNCTION
*     Write string into file 
*
*  INPUTS
*     const char *str   - pointer to buffer
*     int len           - number of bytes which should be written
*     const char *fname - filename
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
*
*  SEE ALSO
*     uti/io/sge_file2string()  
*     uti/io/sge_stream2string()
*
*  NOTES
*     MT-NOTE: sge_string2file() is MT safe
******************************************************************************/ 

int sge_string2file(const char *str, int len, const char *fname)
{
   int fp = -1;
 
   DENTER(TOP_LAYER, "sge_string2file");
 
   if (!(fp = open(fname, O_WRONLY | O_CREAT, 0666)))
   {
      ERROR((SGE_EVENT, MSG_FILE_OPENFAILED_S , fname));
      DEXIT;
      return -1;
   }
   if (!len) {
      len = strlen(str);
   }
 
   if (write(fp, str, len) != len)
   {
      int old_errno = errno;
      ERROR((SGE_EVENT, MSG_FILE_WRITEBYTESFAILED_ISS, len, fname,
             strerror(errno)));
      if (close(fp) != 0) { 
         goto FCLOSE_ERROR; 
      }
      unlink(fname);
      errno = old_errno;
      DEXIT;
      return -1;
   }
 
   if (close(fp) != 0) { 
      goto FCLOSE_ERROR; 
   }
   DEXIT;
   return 0;
FCLOSE_ERROR:
   ERROR((SGE_EVENT, MSG_FILE_FCLOSEFAILED_SS, fname, strerror(errno)));
   DEXIT;
   return -1;
}          

