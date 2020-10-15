#ifndef __SGE_LOADMEM_H
#define __SGE_LOADMEM_H
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

#if defined(SOLARIS) || defined(HPUX) || defined(ALPHA) || defined(LINUX) || defined(IRIX) || defined(DARWIN) || defined(FREEBSD) || defined(NETBSD) || defined(HAS_AIX5_PERFLIB)

#define SGE_LOADMEM

/****** uti/os/sge_mem_info_t *************************************************
*  NAME
*     sge_mem_info_t -- Structure to store memory information
*
*  SYNOPSIS
*     typedef struct {
*        double mem_total; 
*        double mem_free;  
*        double swap_total; 
*        double swap_free;   
*     #ifdef IRIX
*        double swap_rsvd;   
*     #endif
*     } sge_mem_info_t;
*
*  FUNCTION
*     mem_total  - total amount of memory in MB
*     mem_free   - amount of free memory in MB
*     swap_total - total amount of swap space in MB
*     swap_free  - amount of free swap space in MB
*     swap_rsvd  - amount of reserved swap space in MB
*
*  SEE ALSO
*     uti/os/sge_loadmem()
******************************************************************************/
typedef struct {
   double mem_total;  
   double mem_free;    
   double swap_total;  
   double swap_free; 
#ifdef IRIX
   double swap_rsvd;  
#endif
} sge_mem_info_t;

int sge_loadmem(sge_mem_info_t *mem_info);
#endif /* SGE_LOADMEM */

#endif /* __SGE_LOADMEM_H */
