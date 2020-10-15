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

#include "cull/pack.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

#if defined(INTERIX)
#include <arpa/inet.h>
#include "wingrid/wingrid.h"
#endif

#ifdef HPUX
#include <arpa/inet.h>
#endif

/* do not compile in monitoring code */
#ifndef NO_SGE_COMPILE_DEBUG
#define NO_SGE_COMPILE_DEBUG
#endif

#include "uti/sge_rmon.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_log.h"

#include "cull/cull_listP.h"
#include "cull/msg_cull.h"
#include "cull/cull_state.h"


#include "basis_types.h"

#if 0
#   undef PACK_LAYER
#   define PACK_LAYER BASIS_LAYER
#endif


/****** cull/pack/--CULL_Packing ***************************************
*
*  NAME
*     CULL_Packing -- platform independent exchange format
*
*  FUNCTION
*     The cull packing functions provide a framework for a 
*     platform independent data representation.
*
*     Data is written into a packbuffer. Individual data words
*     are written in network byte order.
*
*     Data in the packbuffer can be compressed.
*
*  NOTES
*     Other platform independent formats, like XML, should be 
*     implemented.
*
*  SEE ALSO
*     cull/pack/-Versioncontrol
*
*     cull/pack/packbitfield()
*     cull/pack/unpackbitfield()
****************************************************************************
*/

/* -------------------------------

   get chunk_size

 */
int pack_get_chunk(void)
{
   return CHUNK;
}

/****** cull/pack/init_packbuffer() *******************************************
*  NAME
*     init_packbuffer() -- initialize packing buffer
*
*  SYNOPSIS
*     int init_packbuffer(sge_pack_buffer *pb, int initial_size, 
*                         int just_count) 
*
*  FUNCTION
*     Initialize a packing buffer.
*     Allocates the necessary memory. If more memory is needed during the use
*     of the packbuffer, it will be reallocated increasing the size by 
*     chunk_size (see function pack_set_chunk).
*
*     Since version 6.0, version information is provided in the packbuffer and 
*     is included in sent messages.
*     For best possible backward interoperability with former versions, an 
*     integer with value 0 is padded before the version information as first 
*     word in the packbuffer. This triggeres error handling in former versions.
*
*     Functions using packing buffers in GDI or related code should use the 
*     function sge_gdi_packet_get_pb_size() to find the correct
*     "initial_size".  
*
*  INPUTS
*     sge_pack_buffer *pb - the packbuffer to initialize
*     int initial_size    - the amount of memory to be allocated at 
*                           initialization.
*                           If a value of 0 is given as initial_size, a size 
*                           of chunk_size (global variable, see function 
*                           pack_set_chunk) will be used.
*     int just_count      - if true, no memory will be allocated and the 
*                           "just_count" property of the packbuffer will 
*                           be set.
*
*  RESULT
*     int - PACK_SUCCESS on success
*           PACK_ENOMEM  if memory allocation fails
*           PACK_FORMAT  if no valid packbuffer is passed
*
*  NOTES
*     MT-NOTE: init_packbuffer() is MT safe (assumptions)
*  
*  SEE ALSO
*     cull/pack/-Packing-typedefs
*     cull/pack/pack_set_chunk()
*     gdi/request_internal/sge_gdi_packet_get_pb_size()
*******************************************************************************/
int 
init_packbuffer(sge_pack_buffer *pb, int initial_size, int just_count) 
{
   DENTER(PACK_LAYER, "init_packbuffer");

   if (pb == NULL) {
      ERROR((SGE_EVENT, MSG_CULL_ERRORININITPACKBUFFER_S, MSG_CULL_PACK_FORMAT));
      DEXIT;
      return PACK_FORMAT;
   }   

   if (!just_count) {
      if (initial_size == 0) {
         initial_size = CHUNK;
      } else {
         initial_size += 2 * INTSIZE;  /* space for version information */
      }
      
      memset(pb, 0, sizeof(sge_pack_buffer));
 
      pb->head_ptr = malloc(initial_size);
      if (pb->head_ptr == NULL) {
         ERROR((SGE_EVENT, MSG_CULL_NOTENOUGHMEMORY_D, initial_size));
         DRETURN(PACK_ENOMEM);
      }
      pb->cur_ptr = pb->head_ptr;
      pb->mem_size = initial_size;

      pb->bytes_used = 0;
      pb->just_count = 0;
      pb->version = CULL_VERSION;
      packint(pb, 0);              /* pad 0 byte -> error handing in former versions */
      packint(pb, pb->version);    /* version information is included in buffer      */
   } else {
      pb->head_ptr = pb->cur_ptr = NULL;
      pb->mem_size = 0;
      pb->bytes_used = 0;
      pb->just_count = 1;
   }

   DRETURN(PACK_SUCCESS);
}

/**************************************************************
 initialize packing buffer out of a normal character buffer
 set read/write pointer to the beginning

 NOTES
    MT-NOTE: init_packbuffer_from_buffer() is MT safe (assumptions)
 **************************************************************/
int 
init_packbuffer_from_buffer(sge_pack_buffer *pb, char *buf, u_long32 buflen) 
{
   DENTER(PACK_LAYER, "init_packbuffer_from_buffer");

   if (!pb || !buf) {
      DRETURN(PACK_FORMAT);
   }   

   memset(pb, 0, sizeof(sge_pack_buffer));

   pb->head_ptr = buf;
   pb->cur_ptr = buf;
   pb->mem_size = buflen;
   pb->bytes_used = 0;

   /* check cull version (only if buffer contains any data) */
   if (buflen > 0) {
      int ret;
      u_long32 pad, version;

      if((ret = unpackint(pb, &pad)) != PACK_SUCCESS) {
         DEXIT;
         return ret;
      } 

      if((ret = unpackint(pb, &version)) != PACK_SUCCESS) {
         DEXIT;
         return ret;
      }

      if(pad != 0 || version != CULL_VERSION) {
         ERROR((SGE_EVENT, MSG_CULL_PACK_WRONG_VERSION_XX, (unsigned int) version, CULL_VERSION));
         DEXIT;
         return PACK_VERSION;
      }

      pb->version = version;
   } else {
      pb->version = CULL_VERSION;
   }

   DEXIT;
   return PACK_SUCCESS;
}

/**************************************************************/
/* MT-NOTE: clear_packbuffer() is MT safe */
void clear_packbuffer(sge_pack_buffer *pb) {
   if (pb != NULL) {
      sge_free(&(pb->head_ptr));
   }
   return;
}

/*************************************************************
 look whether pb is filled
 *************************************************************/
int pb_filled(
sge_pack_buffer *pb 
) {
   /* do we have more bytes used than the version information? */
   return (pb_used(pb) > (2 * INTSIZE));
}

/*************************************************************
 look for the number of bytes that are unused
 i.e. that are not yet consumed
 *************************************************************/
int pb_unused(
sge_pack_buffer *pb 
) {
   return (pb->mem_size - pb->bytes_used);
}

/*************************************************************
 look for the number of bytes that are used
 *************************************************************/
int pb_used(
sge_pack_buffer *pb 
) {
   return pb->bytes_used;
}

/* --------------------------------------------------------- */
/*                                                           */
/*            low level functions for packing                */
/*                                                           */
/* --------------------------------------------------------- */

/*
   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT
 */
int packint(sge_pack_buffer *pb, u_long32 i) 
{
   u_long32 J=0;

   DENTER(PACK_LAYER, "packint");

   if (!pb->just_count) {
      if (pb->bytes_used + INTSIZE > pb->mem_size) {
         DPRINTF(("realloc(%d + %d)\n", pb->mem_size, CHUNK));
         pb->mem_size += CHUNK;
         pb->head_ptr = sge_realloc(pb->head_ptr, pb->mem_size, 0);
         if (!pb->head_ptr) {
            DEXIT;
            return PACK_ENOMEM;
         }
         pb->cur_ptr = &(pb->head_ptr[pb->bytes_used]);
      }

      /* copy in packing buffer */
      J = htonl(i);
      memcpy(pb->cur_ptr, (((char *) &J) + INTOFF), INTSIZE);
      pb->cur_ptr = &(pb->cur_ptr[INTSIZE]);
   }
   pb->bytes_used += INTSIZE;

   DEXIT;
   return PACK_SUCCESS;
}

int repackint(sge_pack_buffer *pb, u_long32 i) 
{
   u_long32 J=0;

   DENTER(PACK_LAYER, "repackint");

   if (!pb->just_count) {
      J = htonl(i);
      memcpy(pb->cur_ptr, (((char *) &J) + INTOFF), INTSIZE);
      pb->cur_ptr = &(pb->cur_ptr[INTSIZE]);
   }

   DEXIT;
   return PACK_SUCCESS;
}

int packint64(sge_pack_buffer *pb, u_long64 i) 
{
   u_long64 J=0;

   DENTER(PACK_LAYER, "packint64");

   if (!pb->just_count) {
      if (pb->bytes_used + (INTSIZE * 2) > pb->mem_size) {
         DPRINTF(("realloc(%d + %d)\n", pb->mem_size, CHUNK));
         pb->mem_size += CHUNK;
         pb->head_ptr = sge_realloc(pb->head_ptr, pb->mem_size, 0);
         if (!pb->head_ptr) {
            DEXIT;
            return PACK_ENOMEM;
         }
         pb->cur_ptr = &(pb->head_ptr[pb->bytes_used]);
      }

      /* copy in packing buffer */
      J = htonl(i);
      memcpy(pb->cur_ptr, (((char *) &J) + INTOFF), (INTSIZE * 2));
      pb->cur_ptr = &(pb->cur_ptr[(INTSIZE * 2)]);
   }
   pb->bytes_used += (INTSIZE * 2);

   DEXIT;
   return PACK_SUCCESS;
}
#define DOUBLESIZE 8

/*
   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT
 */
int packdouble(sge_pack_buffer *pb, double d) {
   char buf[32];
#if !(defined(WIN32))                                         /* var not needed */
   XDR xdrs;
#endif

   DENTER(PACK_LAYER, "packdouble");

   if (!pb->just_count) {
      if (pb->bytes_used + DOUBLESIZE > pb->mem_size) {
         DPRINTF(("realloc(%d + %d)\n", pb->mem_size, CHUNK));
         pb->mem_size += CHUNK;
         pb->head_ptr = sge_realloc(pb->head_ptr, pb->mem_size, 0);
         if (!pb->head_ptr) {
            DEXIT;
            return PACK_ENOMEM;
         }
         pb->cur_ptr = &(pb->head_ptr[pb->bytes_used]);
      }

      /* copy in packing buffer */
#if !(defined(WIN32) || defined(INTERIX))                      /* XDR not called */
      xdrmem_create(&xdrs, (caddr_t)buf, sizeof(buf), XDR_ENCODE);

      if (!(xdr_double(&xdrs, &d))) {
         DPRINTF(("error - XDR of double failed\n"));
         xdr_destroy(&xdrs);
         DEXIT;
         return PACK_FORMAT;
      }

      if (xdr_getpos(&xdrs) != DOUBLESIZE) {
         DPRINTF(("error - size of XDRed double is %d\n", xdr_getpos(&xdrs)));
         xdr_destroy(&xdrs);
         DEXIT;
         return PACK_FORMAT;
      }
#endif/* WIN32 || INTERIX */
#if defined(INTERIX)
      wl_xdrmem_create(&xdrs, (caddr_t)buf, sizeof(buf), XDR_ENCODE);

      if (!(wl_xdr_double(&xdrs, &d))) {
         DPRINTF(("error - XDR of double failed\n"));
         wl_xdr_destroy(&xdrs);
         DEXIT;
         return PACK_FORMAT;
      }

      if (wl_xdr_getpos(&xdrs) != DOUBLESIZE) {
         DPRINTF(("error - size of XDRed double is %d\n", wl_xdr_getpos(&xdrs)));
         wl_xdr_destroy(&xdrs);
         DEXIT;
         return PACK_FORMAT;
      }
#endif

      memcpy(pb->cur_ptr, buf, DOUBLESIZE);
      /* we have to increment the buffer even through WIN32 will not use it */
      pb->cur_ptr = &(pb->cur_ptr[DOUBLESIZE]);

#if !(defined(WIN32) || defined(INTERIX)) /* XDR not called */
      xdr_destroy(&xdrs);
#endif
#if defined(INTERIX)
      wl_xdr_destroy(&xdrs);
#endif
   }
   pb->bytes_used += DOUBLESIZE;

   DEXIT;
   return PACK_SUCCESS;
}

/* ---------------------------------------------------------

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
int packstr(sge_pack_buffer *pb, const char *str) 
{
   DENTER(PACK_LAYER, "packstr");

   /* determine string length */
   if (str == NULL) {
      if (!pb->just_count) {
         /* is realloc necessary */
         if ((pb->bytes_used + 1) > pb->mem_size) {

            /* realloc */
            DPRINTF(("realloc(%d + %d)\n", pb->mem_size, CHUNK));
            pb->mem_size += CHUNK;
            pb->head_ptr = sge_realloc(pb->head_ptr, pb->mem_size, 0);
            if (!pb->head_ptr) {
               DEXIT;
               return PACK_ENOMEM;
            }

            /* update cur_ptr */
            pb->cur_ptr = &(pb->head_ptr[pb->bytes_used]);
         }
         pb->cur_ptr[0] = '\0';
         /* update cur_ptr & bytes_packed */
         pb->cur_ptr = &(pb->cur_ptr[1]);
      }
      pb->bytes_used++;
   } else {
      size_t n = strlen(str) + 1;

      if (!pb->just_count) {
         /* is realloc necessary */
         if ((pb->bytes_used + n) > pb->mem_size) {
            /* realloc */
            DPRINTF(("realloc(%d + %d)\n", pb->mem_size, CHUNK));
            while ((pb->bytes_used + n) > pb->mem_size)
               pb->mem_size += CHUNK;
            pb->head_ptr = sge_realloc(pb->head_ptr, pb->mem_size, 0);
            if (!pb->head_ptr) {
               DEXIT;
               return PACK_ENOMEM;
            }

            /* update cur_ptr */
            pb->cur_ptr = &(pb->head_ptr[pb->bytes_used]);
         }
         memcpy(pb->cur_ptr, str, n);
         /* update cur_ptr & bytes_packed */
         pb->cur_ptr = &(pb->cur_ptr[n]);
      }
      pb->bytes_used += n;
   }

   DEXIT;
   return PACK_SUCCESS;
}


/****** cull/pack/packbitfield() ****************************************************
*  NAME
*     packbitfield() -- pack a bitfield 
*
*  SYNOPSIS
*     int packbitfield(sge_pack_buffer *pb, const bitfield *bitfield) 
*
*  FUNCTION
*     Writes the bitfield into the given packbuffer.
*     The following information will be written:
*        - the size of the bitfield in bits
*        - the bitfield itself as binary buffer
*
*  INPUTS
*     sge_pack_buffer *pb - the target packbuffer
*     const bitfield *bitfield   - the bitfield to pack
*
*  RESULT
*     int - PACK_SUCCESS on success,
*           else PACK_* error codes
*
*  SEE ALSO
*     uti/bitfield/--Bitfield
*     cull/pack/unpackbitfield()
*******************************************************************************/
int packbitfield(sge_pack_buffer *pb, const bitfield *bitfield)
{
   int ret;
   u_long32 size;
   u_long32 char_size;

   DENTER(PACK_LAYER, "packbitfield");

   size = sge_bitfield_get_size(bitfield);
   char_size = sge_bitfield_get_size_bytes(size);

   if((ret = packint(pb, size)) != PACK_SUCCESS) {
      DEXIT;
      return ret;
   }
   
   if((ret = packbuf(pb, sge_bitfield_get_buffer(bitfield),
                     char_size)) != PACK_SUCCESS) {
      DEXIT;
      return ret;
   }

   DEXIT;
   return PACK_SUCCESS;
}

/* ---------------------------------------------------------

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT

 */
int packbuf(
sge_pack_buffer *pb,
const char *buf_ptr,
u_long32 buf_size 
) {

   DENTER(PACK_LAYER, "packbuf");

   if (!pb->just_count) {
      /* is realloc necessary */
#if !(defined(WIN32)) /* cast */
      if (buf_size + (u_long32) pb->bytes_used > (u_long32) pb->mem_size) {
#else
      if (buf_size + pb->bytes_used > pb->mem_size) {
#endif /* WIN32 */

         /* realloc */
         DPRINTF(("realloc(%d + %d)\n", pb->mem_size, CHUNK));
         pb->mem_size += CHUNK;
         pb->head_ptr = sge_realloc(pb->head_ptr, pb->mem_size, 0);
         if (!(pb->head_ptr)) {
            DEXIT;
            return PACK_ENOMEM;
         }

         /* update cur_ptr */
         pb->cur_ptr = &(pb->head_ptr[pb->bytes_used]);
      }

      /* copy in packing buffer */
      memcpy(pb->cur_ptr, buf_ptr, buf_size);
      /* update cur_ptr & bytes_packed */
      pb->cur_ptr = &(pb->cur_ptr[buf_size]);
   }
   pb->bytes_used += buf_size;

   DEXIT;
   return PACK_SUCCESS;
}


/* --------------------------------------------------------- */
/*                                                           */
/*            low level functions for unpacking              */
/*                                                           */
/* --------------------------------------------------------- */

/* ---------------------------------------------------------

   return values:
   PACK_SUCCESS
   (PACK_ENOMEM)
   PACK_FORMAT

 */
int unpackint(sge_pack_buffer *pb, u_long32 *ip) 
{
   DENTER(PACK_LAYER, "unpackint");

   /* are there enough bytes ? */
   if (pb->bytes_used + INTSIZE > pb->mem_size) {
      *ip = 0;
      DEXIT;
      return PACK_FORMAT;
   }

   /* copy integer */
   memset(ip, 0, sizeof(u_long32));
   memcpy(((char *) ip) + INTOFF, pb->cur_ptr, INTSIZE);
   *ip = ntohl(*ip);

   /* update cur_ptr & bytes_unpacked */
   pb->cur_ptr = &(pb->cur_ptr[INTSIZE]);
   pb->bytes_used += INTSIZE;

   DEXIT;
   return PACK_SUCCESS;
}

/* ---------------------------------------------------------

   return values:
   PACK_SUCCESS
   (PACK_ENOMEM)
   PACK_FORMAT

 */
int unpackint64(sge_pack_buffer *pb, u_long64 *ip) 
{
   DENTER(PACK_LAYER, "unpackint64");

   /* are there enough bytes ? */
   if (pb->bytes_used + (INTSIZE *2) > pb->mem_size) {
      *ip = 0;
      DEXIT;
      return PACK_FORMAT;
   }

   /* copy integer */
   memset(ip, 0, sizeof(u_long64));
   memcpy(((char *) ip) + INTOFF, pb->cur_ptr, (INTSIZE * 2));
   *ip = ntohl(*ip);

   /* update cur_ptr & bytes_unpacked */
   pb->cur_ptr = &(pb->cur_ptr[(INTSIZE * 2)]);
   pb->bytes_used += (INTSIZE * 2);

   DEXIT;
   return PACK_SUCCESS;
}

/* ---------------------------------------------------------

   return values:
   PACK_SUCCESS
   (PACK_ENOMEM)
   PACK_FORMAT

 */
int unpackdouble(sge_pack_buffer *pb, double *dp) 
{

#if !(defined(WIN32))                                       /* var not needed */
   XDR xdrs;
   char buf[32];
#endif

   DENTER(PACK_LAYER, "unpackdouble");

   /* are there enough bytes ? */
   if (pb->bytes_used + DOUBLESIZE > pb->mem_size) {
      *dp = 0;
      DEXIT;
      return PACK_FORMAT;
   }

   /* copy double */

#if !(defined(WIN32) || defined(INTERIX))                   /* XDR not called */
   memcpy(buf, pb->cur_ptr, DOUBLESIZE);
   xdrmem_create(&xdrs, buf, DOUBLESIZE, XDR_DECODE);
   if (!(xdr_double(&xdrs, dp))) {
      *dp = 0;
      DPRINTF(("error unpacking XDRed double\n"));
      xdr_destroy(&xdrs);
      DEXIT;
      return PACK_FORMAT;
   }
#endif /* WIN32 || INTERIX */
#if defined(INTERIX)
   memcpy(buf, pb->cur_ptr, DOUBLESIZE);
   wl_xdrmem_create(&xdrs, buf, DOUBLESIZE, XDR_DECODE);
   if (!(wl_xdr_double(&xdrs, dp))) {
      *dp = 0;
      DPRINTF(("error unpacking XDRed double\n"));
      wl_xdr_destroy(&xdrs);
      DEXIT;
      return PACK_FORMAT;
   }
#endif

   /* update cur_ptr & bytes_unpacked */
   pb->cur_ptr = &(pb->cur_ptr[DOUBLESIZE]);
   pb->bytes_used += DOUBLESIZE;

#if !(defined(WIN32) || defined(INTERIX))                   /* XDR not called */
   xdr_destroy(&xdrs);
#endif /* WIN32 || INTERIX */
#if defined(INTIERX)
   wl_xdr_destroy(&xdrs);
#endif

   DEXIT;
   return PACK_SUCCESS;
}

/* ---------------------------------------------------------

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT
 */
int unpackstr(sge_pack_buffer *pb, char **str) 
{
   u_long32 n;

   DENTER(PACK_LAYER, "unpackstr");

   /* determine string length */
   if (!pb->cur_ptr[0]) {

      *str = NULL;

      /* update cur_ptr & bytes_unpacked */
      pb->cur_ptr = &(pb->cur_ptr[1]);
      pb->bytes_used++;

      /* are there enough bytes ? */
      if (pb->bytes_used > pb->mem_size) {
         DEXIT;
         return PACK_FORMAT;
      }

      DEXIT;
      return PACK_SUCCESS;
   } else {
      n = strlen(pb->cur_ptr) + 1;

      /* are there enough bytes ? */
      if (n + pb->bytes_used > pb->mem_size) {
         DEXIT;
         return PACK_FORMAT;
      }
      *str = strdup(pb->cur_ptr);
      if (!*str) {
         DEXIT;
         return PACK_ENOMEM;
      }
      /* update cur_ptr & bytes_unpacked */
      pb->bytes_used += n;
      pb->cur_ptr = &(pb->cur_ptr[n]);
   }

   DEXIT;
   return PACK_SUCCESS;
}

/* ---------------------------------------------------------

   return values:
   PACK_SUCCESS
   PACK_ENOMEM
   PACK_FORMAT
 */
int unpackbuf(sge_pack_buffer *pb, char **buf_ptr, int buf_size) 
{

   DENTER(PACK_LAYER, "unpackbuf");

   if (buf_size == 0) {
      DEXIT;
      return PACK_SUCCESS;
   }

   /* are there enough bytes ? */
   if ((buf_size + pb->bytes_used) > pb->mem_size) {
      DEXIT;
      return PACK_FORMAT;
   }

   /* copy buffer */
   *buf_ptr = malloc(buf_size);
   if (!*buf_ptr) {
      DEXIT;
      return PACK_ENOMEM;
   }
   memcpy(*buf_ptr, pb->cur_ptr, buf_size);
   /* update cur_ptr & bytes_unpacked */
   pb->cur_ptr = &(pb->cur_ptr[buf_size]);
   pb->bytes_used += buf_size;

   DEXIT;
   return PACK_SUCCESS;
}

/****** cull/pack/unpackbitfield() ********************************************
*  NAME
*     unpackbitfield() -- unpack a bitfield 
*
*  SYNOPSIS
*     int unpackbitfield(sge_pack_buffer *pb, bitfield *bitfield) 
*
*  FUNCTION
*     Unpacks a bitfield from a packbuffer.
*
*     If the size of the descriptor doesn't match the size of the unpacked
*     bitfield, create a new bitfield.
*
*  INPUTS
*     sge_pack_buffer *pb - the source packbuffer
*     bitfield *bitfield  - used to return the unpacked bitfield
*     int descr_size      - size of the corresponding descriptor
*
*  RESULT
*     int - PACK_SUCCESS on success,
*           else PACK_* error codes
*
*  SEE ALSO
*     uti/bitfield/--Bitfield
*     cull/pack/packbitfield()
*******************************************************************************/
int unpackbitfield(sge_pack_buffer *pb, bitfield *bitfield, int descr_size)
{
   int ret;
   u_long32 size, char_size;
   char *buffer = NULL;

   DENTER(PACK_LAYER, "unpackbitfield");

   /* create new bitfield */
   if (!sge_bitfield_init(bitfield, descr_size)) {
      DEXIT;
      return PACK_ENOMEM;
   }

   /* unpack the size in bits */
   if((ret = unpackint(pb, &size)) != PACK_SUCCESS) {
      DEXIT;
      return ret;
   }

   /* size may not be bigger than bitfield initialized from descr information */
   if (size > descr_size) {
      DEXIT;
      return PACK_ENOMEM;
   }

   /* unpack contents of the bitfield */
   char_size = sge_bitfield_get_size_bytes(size);
   if((ret = unpackbuf(pb, &buffer, char_size)) != PACK_SUCCESS) {
      sge_bitfield_free_data(bitfield);
      DEXIT;
      return ret;
   }
   
   memcpy(sge_bitfield_get_buffer(bitfield), buffer, char_size);

   /* free unpacked bitfield buffer */
   sge_free(&buffer);

   DEXIT;
   return PACK_SUCCESS;
}

const char *cull_pack_strerror(int errnum)
{
   switch(errnum) {
      case PACK_SUCCESS:
         return MSG_CULL_PACK_SUCCESS;
      case PACK_ENOMEM:
         return MSG_CULL_PACK_ENOMEM;
      case PACK_FORMAT:
         return MSG_CULL_PACK_FORMAT;
      case PACK_BADARG:
         return MSG_CULL_PACK_BADARG;
      case PACK_VERSION:
         return MSG_CULL_PACK_VERSION;
      default:
         /* JG: TODO: we should have some global error message for all strerror functions */
         return "";
   }
}

/****** cull/pack/pb_are_equivalent() *****************************************
*  NAME
*     pb_are_equivalent() -- check if both buffers are equivalent 
*
*  SYNOPSIS
*     bool pb_are_equivalent(sge_pack_buffer *pb1, sge_pack_buffer *pb2) 
*
*  FUNCTION
*     Check if size and content of both packbuffers is equivalent 
*
*  INPUTS
*     sge_pack_buffer *pb1 - packbuffer 
*     sge_pack_buffer *pb2 - packbuffer 
*
*  RESULT
*     bool - equivalent?
*        true  - yes
*        false - no
*******************************************************************************/
bool
pb_are_equivalent(sge_pack_buffer *pb1, sge_pack_buffer *pb2)
{
   bool ret = true;

   if (pb1 != NULL && pb2 != NULL) {
      ret &= (pb1->bytes_used == pb2->bytes_used);
      ret &= (memcmp(pb1->head_ptr, pb2->head_ptr, pb1->bytes_used) == 0);
   }
   return ret;
}

/****** cull/pack/pb_print_to() ***********************************************
*  NAME
*     pb_print_to() -- Print content of packbuffer 
*
*  SYNOPSIS
*     void pb_print_to(sge_pack_buffer *pb, FILE* file) 
*
*  FUNCTION
*     Print content of packbuffer into file 
*
*  INPUTS
*     sge_pack_buffer *pb - packbuffer pointer 
*     bool only_header    - show only summary information 
*     FILE* file          - file stream (e.g. stderr) 
*
*  RESULT
*     void - NONE
*******************************************************************************/
void
pb_print_to(sge_pack_buffer *pb, bool only_header, FILE* file)
{
   int i;

   fprintf(file, "head_ptr: %p\n", pb->head_ptr);
   fprintf(file, "cur_ptr: %p\n", pb->cur_ptr);
   fprintf(file, "mem_size: %d\n", (int)pb->mem_size);
   fprintf(file, "bytes_used: %d\n", (int)pb->bytes_used);
   fprintf(file, "buffer:\n");
   if (!only_header) {
      for (i = 0; i < pb->bytes_used; i++) {
         fprintf(file, "%3d ", pb->head_ptr[i]);
         if ((i + 1) % 15 == 0) {
            fprintf(file, "\n");
         }
      }
      fprintf(file, "\n");
   }
}

