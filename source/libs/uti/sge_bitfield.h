#ifndef __SGE_BITFIELD_H
#define __SGE_BITFIELD_H
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

#include "sge.h"
#include "basis_types.h"

/****** uti/bitfield/BIT_MANIPULATION_MAKROS() ********************************
*  NAME
*     ISSET(),VALID(),SETBIT(),CLEARBIT() - Bit manipulation makros 
*
*  SYNOPSIS
*     #define ISSET(a,b)      ((a&b)==b)
*     #define VALID(a,b)      ((a|b)==b)
*     #define SETBIT(a,b)     (b=(a)|b);
*     #define CLEARBIT(a,b)   (b &= (~(a)));
*
*  FUNCTION
*     Makros to get/set/clear bits in native variables. 
*
*  INPUTS
*     int,long,u_long32... a - Bitmask
*     int,long,u_long32... b - Variable 
*
*  RESULT
*     b will be modified
*
*  NOTE
*     These Makros can't be used in combination with the bitfield type.
*******************************************************************************/
#define ISSET(a,b)      ((a&b)==b)
#define VALID(a,b)      ((a|b)==b)
#define SETBIT(a,b)     (b=(a)|b);
#define CLEARBIT(a,b)   (b &= (~(a)));

typedef struct {
   unsigned int size;
   union {
      char fix[sizeof(char *)];  /* fixed size buffer for small bitfields */
      char *dyn;                 /* dynamic size buffer for large bitfields */
   } bf;
} bitfield;

/* fixme:  should any of this be inline?  */
bitfield *
sge_bitfield_new(unsigned int size);

bitfield *
sge_bitfield_free(bitfield *bf);

bool 
sge_bitfield_init(bitfield *bf, unsigned int size);

bool 
sge_bitfield_free_data(bitfield *bf);

bool 
sge_bitfield_copy(const bitfield *source, bitfield *target);

bool 
sge_bitfield_bitwise_copy(const bitfield *source, bitfield *target);

bool 
sge_bitfield_set(bitfield *bf, unsigned int bit);

bool 
sge_bitfield_get(const bitfield *bf, unsigned int bit) __attribute__ ((__pure__));

bool 
sge_bitfield_clear(bitfield *bf, unsigned int bit);

bool 
sge_bitfield_reset(bitfield *source);

bool 
sge_bitfield_changed(const bitfield *source) __attribute__ ((__pure__));

void 
sge_bitfield_print(const bitfield *bf, FILE *fd); 

#define fixed_bits (sizeof(char *) * 8)
#define sge_bitfield_get_size(bf) ((bf)->size)
#define sge_bitfield_get_size_bytes(size) ((size) / 8 + (((size) % 8) > 0 ? 1 : 0))
#define sge_bitfield_get_buffer(source) ((source)->size <= fixed_bits) ? (source)->bf.fix : (source)->bf.dyn

#endif /* __SGE_BITFIELD_H */
