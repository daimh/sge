#ifndef __HOSTL_H
#define __HOSTL_H
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

#include "cull/boundaries.h"
#include "cull/cull.h"

enum {
   H_hostname = H_LOWERBOUND,    /* THIS IS VERY IMPORTANT */
        H_arch,
        H_os,
        H_memsize,
        H_queuelist
};


LISTDEF( HostT )
   SGE_HOST     ( H_hostname, CULL_DEFAULT )
   SGE_STRING   ( H_arch, CULL_DEFAULT )
   SGE_STRING   ( H_os, CULL_DEFAULT )
   SGE_ULONG    ( H_memsize, CULL_DEFAULT )
   SGE_LIST  ( H_queuelist, QueueT, CULL_DEFAULT)
LISTEND

NAMEDEF( HostN )
   NAME   ( "H_hostname" )
   NAME   ( "H_arch" )
   NAME   ( "H_os" )
   NAME   ( "H_memsize" )
   NAME   ( "H_queuelist" )
NAMEEND

#define HostS sizeof(HostN)/sizeof(char*)

#endif /* __HOSTL_H */
