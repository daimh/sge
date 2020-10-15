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
#include <string.h>

#include "uti/sge_bitfield.h"
#include "uti/sge_stdlib.h"

/****** uti/bitfield/--Bitfield ****************************************
*  NAME
*     Bitfield -- A variable size bitfield implementation
*
*  SYNOPSIS
*     bitfield sge_bitfield_new(unsigned int size)
*
*  FUNCTION
*     This module provides variable size bitfields.
*     The size of a bitfield can be defined when the bitfield is created.
*     Individual bits can be set, read and cleared.
*     The contents of a bitfield can be printed to stdout or any
*     file handle.
*
*  EXAMPLE
*     See main program (module test) in libs/uti/sge_bitfield.c
*
*  NOTES
*     MT-NOTE: this module is MT safe
*
*  SEE ALSO
*     uti/bitfield/sge_bitfield_new()
*     uti/bitfield/sge_bitfield_free()
*     uti/bitfield/sge_bitfield_set()
*     uti/bitfield/sge_bitfield_get()
*     uti/bitfield/sge_bitfield_clear()
*     uti/bitfield/sge_bitfield_reset()
*     uti/bitfield/sge_bitfield_copy()
*     uti/bitfield/sge_bitfield_bitwise_copy()
*     uti/bitfield/sge_bitfield_print()
*******************************************************************************/

/****** uti/bitfield/-Bitfield_Typedefs ****************************************
*  NAME
*     Bitfield_Typedefs -- type definitions for the Bitfield module
*
*  SYNOPSIS
*     typedef struct {
*        unsigned int size;
*        union {
*           char fix[sizeof(char *)];
*           char *dyn;
*        } bf;
*     } bitfield;
*     
*     typedef _bitfield *bitfield;
*
*  FUNCTION
*     The _bitfield structure is the internal representation of a bitfield.
*     All operations on bitfields use the bitfield type.
*
*     For small bitfields, no memory is allocated, but the space available in
*     bf.fix is used (depending on the architecture, this suffices for 32 or 64
*     bit bitfields).
*     This saves a considerable amount of memory and above all processing time.
*******************************************************************************/

/****** uti/bitfield/sge_bitfield_new() ****************************************
*  NAME
*     sge_bitfield_new() -- create a new bitfield
*
*  SYNOPSIS
*     bitfield *
*     sge_bitfield_new(unsigned int size) 
*
*  FUNCTION
*     Allocates and initializes the necessary memory.
*     It is in the responsibility of the caller to free the bitfield
*     once it is no longer needed.
*
*  INPUTS
*     unsigned int size - size in bits
*
*  RESULT
*     bitfield - a new bitfield or NULL, if the creation of the bitfield 
*                failed
*
*  NOTES
*     MT-NOTE: sge_bitfield_new() is MT safe
*
*  SEE ALSO
*     uti/bitfield/sge_bitfield_free()
*******************************************************************************/
bitfield *
sge_bitfield_new(unsigned int size)
{
   bitfield *bf;

   /* allocate storage for bitfield object */
   bf = (bitfield *) malloc(sizeof(bitfield));
   if (bf != NULL) {
      /* initialize bitfield, on errors, free bitfield */
      if (!sge_bitfield_init(bf, size)) {
         sge_free(&bf);
      }
   }

   return bf;
}

/****** uti/bitfield/sge_bitfield_init() ***************************************
*  NAME
*     sge_bitfield_init() -- initialize a bitfield object
*
*  SYNOPSIS
*     bool 
*     sge_bitfield_init(bitfield *bf, unsigned int size) 
*
*  FUNCTION
*     Initializes a bitfield object.
*     Storage for the bits is allocated if necessary (size of the bitfield is 
*     bigger than the preallocated storage) and the size is stored.
*
*  INPUTS
*     bitfield *bf      - the bitfield to initialize
*     unsigned int size - the targeted size of the bitfield
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     MT-NOTE: sge_bitfield_init() is MT safe 
*******************************************************************************/
bool
sge_bitfield_init(bitfield *bf, unsigned int size)
{
   bool ret = true;

   if (bf == NULL) {
      ret = false;
   } else {
      unsigned int char_size = sge_bitfield_get_size_bytes(size);

      /* malloc bitfield buffer only if char * has less bits than required */
      if (size <= fixed_bits) {
         /* clear buffer */
         bf->bf.dyn = (char *)0;
      } else {
         bf->bf.dyn = (char *)malloc(char_size);
         if (bf->bf.dyn == NULL) {
            ret = false;
         } else {
            /* clear buffer */
            memset(bf->bf.dyn, 0, char_size);
         }
      }

      bf->size = size;
   }
   
   return ret;
}

/****** uti/bitfield/sge_bitfield_copy() ***************************************
*  NAME
*     sge_bitfield_copy() -- copies a bitfield into another one. 
*
*  SYNOPSIS
*     bool
*     sge_bitfield_copy(const bitfield *source, bitfield *target) 
*
*  FUNCTION
*     The memory has to be allocated before, and source and target has to have
*     the same size. Otherwise it will return false and does not copy anything.
*
*  INPUTS
*     const bitfield *source  - source bitfield
*     bitfield *target        - target bitfield
*
*  RESULT
*     bool - false, if one of the bitfields is NULL or 
*                   the bitfield sizes are different
*
*  NOTES
*     MT-NOTE: sge_bitfield_copy() is MT safe 
*
*******************************************************************************/
bool 
sge_bitfield_copy(const bitfield *source, bitfield *target)
{
   bool ret = true;

   if (source == NULL || target == NULL) {
      ret = false;
   }

   if (ret && source->size != target->size) {
      ret = false;
   }
   if (ret) {
      unsigned int char_size = sge_bitfield_get_size_bytes(source->size);
      if (source->size <= fixed_bits) {
         target->bf.dyn = source->bf.dyn;
      } else {
         memcpy(target->bf.dyn, source->bf.dyn, char_size);
      }
   }
   
   return ret;
}


/****** uti/bitfield/sge_bitfield_bitwise_copy() *******************************
*  NAME
*     sge_bitfield_copy() -- copies a bitfield into another one. 
*
*  SYNOPSIS
*     bool 
*     sge_bitfield_bitwise_copy(const bitfield *source, bitfield *target) 
*
*  FUNCTION
*     The memory has to be allocated before, but the bitfields can have
*     different sizes.  If the source is longer than the target, only the bits
*     up to target's length are copied.
*
*  INPUTS
*     const bitfield *source  - source bitfield
*     bitfield *target        - target bitfield
*
*  RESULT
*     bool - false, if one of the bitfields is NULL
*
*  NOTES
*     MT-NOTE: sge_bitfield_bitwise_copy() is MT safe 
*
*******************************************************************************/
bool 
sge_bitfield_bitwise_copy(const bitfield *source, bitfield *target)
{
   bool ret = true;

   if (source == NULL || target == NULL) {
      ret = false;
   }

   if (ret) {
      unsigned int char_size = 0;
      const char *source_buffer = sge_bitfield_get_buffer(source);
      char *target_buffer = sge_bitfield_get_buffer(target);
     
      if (source->size > target->size) {
         /* This may result in the target getting a few more bits than it wants
          * (if target->size isn't a multiple of 8), but that shouldn't matter
          * because sge_bitfield_get() guards against accessing those extra
          * bits. */
         char_size = sge_bitfield_get_size_bytes(target->size);
      } else {
         char_size = sge_bitfield_get_size_bytes(source->size);
      }
      
      memcpy(target_buffer, source_buffer, char_size);
   }
   
   return ret;
}

/****** uti/bitfield/sge_bitfield_changed() ************************************
*  NAME
*     sge_bitfield_changed() -- figures out if something was changed.
*
*  SYNOPSIS
*     bool 
*     sge_bitfield_changed(const bitfield *source) 
*
*  FUNCTION
*
*  INPUTS
*     bitfield *source - bitfield to analyze
*
*  RESULT
*     bool - true, if the bitfield has a changed bit set.
*
*  NOTES
*     MT-NOTE: sge_bitfield_copy() is MT safe 
*
*******************************************************************************/
bool 
sge_bitfield_changed(const bitfield *bf) 
{
   bool ret = false;

   if (bf != NULL) {
      const char *buf = sge_bitfield_get_buffer(bf);
      unsigned int char_size = sge_bitfield_get_size_bytes(bf->size);
      unsigned int i;

      for (i = 0; i < char_size; i++) {
         if (buf[i] != 0) {
            ret = true;
            break;
         }
      }
   }
   
   return ret;
}

/****** uti/bitfield/sge_bitfield_reset() ***************************************
*  NAME
*     sge_bitfield_reset() -- clears a bitfield
*
*  SYNOPSIS
*     bool 
*     sge_bitfield_reset(bitfield *bf) 
*
*  FUNCTION
*
*  INPUTS
*     bitfield *bf - bitfield to reset
*
*  RESULT
*     bool - false, if bf is NULL
*
*  NOTES
*     MT-NOTE: sge_bitfield_copy() is MT safe 
*
*******************************************************************************/
bool 
sge_bitfield_reset(bitfield *bf)
{
   if (bf != NULL) {
      if (bf->size > fixed_bits) {
         unsigned int char_size = sge_bitfield_get_size_bytes(bf->size);
         memset(bf->bf.dyn, 0, char_size);
      } else {
         bf->bf.dyn = (char *)0;
      }

      return true;    
   }
   
   return false;
}


/****** uti/bitfield/sge_bitfield_free() ***************************************
*  NAME
*     sge_bitfield_free() -- destroy a bitfield
*
*  SYNOPSIS
*     bitfield sge_bitfield_free(bitfield *bf) 
*
*  FUNCTION
*     Destroys a bitfield. Frees all memory allocated by the bitfield.
*
*  INPUTS
*     bitfield *bf - the bitfield to destroy
*
*  NOTES
*     MT-NOTE: sge_bitfield_free() is MT safe
*
*  RESULT
*     bitfield * - NULL
*******************************************************************************/
bitfield *sge_bitfield_free(bitfield *bf)
{
   if (bf != NULL) {
      if (bf->size > fixed_bits) {
         if (bf->bf.dyn != NULL) {
            sge_free(&(bf->bf.dyn));
         }
      }
      sge_free(&bf);
   }

   return NULL;
}

/****** uti/bitfield/sge_bitfield_free_data() **********************************
*  NAME
*     sge_bitfield_free_data() -- free the bitfield data
*
*  SYNOPSIS
*     bool 
*     sge_bitfield_free_data(bitfield *bf) 
*
*  FUNCTION
*     Frees the data part of a bitfield.
*     The bitfield itself is not freed.
*
*  INPUTS
*     bitfield *bf - the bitfield to work on
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     MT-NOTE: sge_bitfield_free_data() is MT safe 
*******************************************************************************/
bool
sge_bitfield_free_data(bitfield *bf)
{
   bool ret = true;

   if (bf == NULL) {
      ret = false;
   } else {
      if (bf->size > fixed_bits) {
         if (bf->bf.dyn != NULL) {
            sge_free(&(bf->bf.dyn));
         }
      }
   }

   return ret;
}

/****** uti/bitfield/sge_bitfield_set() ****************************************
*  NAME
*     sge_bitfield_set() -- set a bit
*
*  SYNOPSIS
*     bool
*     sge_bitfield_set(bitfield *bf, unsigned int bit) 
*
*  FUNCTION
*     Sets a certain bit in a bitfield to 1.
*
*  INPUTS
*     bitfield *bf - the bitfield to manipulate
*     unsigned int bit     - the bit to set
*
*  NOTES
*     MT-NOTE: sge_bitfield_set() is MT safe
*
*  RESULT
*     bool - true on success, 
*           false on error
*******************************************************************************/
bool
sge_bitfield_set(bitfield *bf, unsigned int bit)
{
   bool ret = true;

   if(bf == NULL || bit >= bf->size) {
      ret = false;
   }

   if (ret) {
      char *buf = sge_bitfield_get_buffer(bf);
      unsigned int byte_offset = bit / 8;
      unsigned int bit_offset  = bit % 8;

      buf[byte_offset] |= 1 << bit_offset;
   }

   return ret;
}

/****** uti/bitfield/sge_bitfield_get() ****************************************
*  NAME
*     sge_bitfield_get() -- read a bit 
*
*  SYNOPSIS
*     bool
*     sge_bitfield_get(const bitfield *bf, unsigned int bit) 
*
*  FUNCTION
*     Reads a certain bit of a bitfield and returns it's contents.
*
*  INPUTS
*     bitfield *bf - the bitfield to read from
*     unsigned int bit     - the bit to read
*
*  NOTES
*     MT-NOTE: sge_bitfield_get() is MT safe
*
*  RESULT
*     bool - false, if bit is not set (or input params invalid),
*            true, if bit is set
*******************************************************************************/
bool
sge_bitfield_get(const bitfield *bf, unsigned int bit)
{
   bool ret = false;

   if (bf != NULL && bit < bf->size) {
      const char *buf = sge_bitfield_get_buffer(bf);
      unsigned int byte_offset = bit / 8;
      unsigned int bit_offset  = bit % 8;

      if ((buf[byte_offset] & (1 << bit_offset)) > 0) {
         ret = true;
      }
   }

   return ret;
}

/****** uti/bitfield/sge_bitfield_clear() **************************************
*  NAME
*     sge_bitfield_clear() -- clear a bit
*
*  SYNOPSIS
*     bool
*     sge_bitfield_clear(bitfield *bf, unsigned int bit) 
*
*  FUNCTION
*     Clears a certain bit in a bitfield (sets its content to 0).
*
*  INPUTS
*     bitfield *bf - the bitfield to manipulate
*     unsigned int bit     - the bit to clear
*
*  NOTES
*     MT-NOTE: sge_bitfield_clear() is MT safe
*
*  RESULT
*     bool - true on success,
*            false on error
*******************************************************************************/
bool
sge_bitfield_clear(bitfield *bf, unsigned int bit)
{
   bool ret = true;

   if(bf == NULL || bit >= bf->size) {
      ret = false;
   }

   if (ret) {
      char *buf = sge_bitfield_get_buffer(bf);
      unsigned int byte_offset = bit / 8;
      unsigned int bit_offset  = bit % 8;

      buf[byte_offset] &= 0xff ^ (1 << bit_offset);
   }

   return ret;
}

/****** uti/bitfield/sge_bitfield_print() **************************************
*  NAME
*     sge_bitfield_print() -- print contents of a bitfield
*
*  SYNOPSIS
*     void sge_bitfield_print(bitfield *bf, FILE *fd) 
*
*  FUNCTION
*     Prints the contents of a bitfield.
*     For each bit one digit (0/1) is printed.
*     If NULL is passed as file descriptor, output is sent to stdout.
*
*  NOTES
*     MT-NOTE: sge_bitfield_print() is MT safe
*
*  INPUTS
*     bitfield  bf - the bitfield to output
*     FILE *fd     - filehandle or NULL
*******************************************************************************/
void sge_bitfield_print(const bitfield  *bf, FILE *fd)
{
   unsigned int i;

   if (bf == NULL) {
      return;
   }

   if (fd == NULL) {
      fd = stdout;
   }

   for (i = 0; i < bf->size; i++) {
      int value = sge_bitfield_get(bf, i) ? 1 : 0;
      fprintf(fd, "%d ", value);
   }
}

