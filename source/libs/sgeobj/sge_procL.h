#ifndef __PROCL_H
#define __PROCL_H
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
/* Portions of this code are Copyright (c) 2011 Univa Corporation. */
/*___INFO__MARK_END__*/

#include "cull/cull.h"

#include "sgeobj/sge_boundaries.h"

#ifdef __cplusplus
extern "C" {
#endif
/****** sgeobj/job/--PRO_Type **************************************************
*  NAME
*     PRO_Type - CULL process element 
*
*  ELEMENTS
*     SGE_ULONG (PRO_pid)
*        Process id of the element
*     SGE_ULONG (PRO_utime)
*        Number of jiffies that this process has been scheduled in user mode.
*     SGE_ULONG (PRO_stime)
*        Number of jiffies that this process has been scheduled in kernel mode.
*     SGE_ULONG64 (PRO_vsize)
*        Virtual memory size in bytes.
*     SGE_LIST (PRO_groups)
*        GR_Type list with all groups associated with this process.
*     SGE_BOOL (PRO_rel)
*        Flag if this process belongs to a GE job.
*     SGE_BOOL (PRO_run)
*        Flag if this process is still running.
*     SGE_ULONG (PRO_io)
*        IO statistic for the running process.
******************************************************************************/
enum {
   PRO_pid = PRO_LOWERBOUND,
   PRO_utime,
   PRO_stime,
   PRO_vsize,
   PRO_groups,
   PRO_rel,
   PRO_run,
   PRO_io
};

LISTDEF( PRO_Type )
   SGE_ULONG     ( PRO_pid, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE )
   SGE_ULONG     ( PRO_utime, CULL_DEFAULT )
   SGE_ULONG     ( PRO_stime, CULL_DEFAULT )
   SGE_ULONG64   ( PRO_vsize, CULL_DEFAULT )
   SGE_LIST      ( PRO_groups, GR_Type, CULL_DEFAULT )
   SGE_BOOL      ( PRO_rel, CULL_DEFAULT )
   SGE_BOOL      ( PRO_run, CULL_DEFAULT )
   SGE_ULONG     ( PRO_io, CULL_DEFAULT )
LISTEND

NAMEDEF( PRON )
   NAME   ( "PRO_pid" )
   NAME   ( "PRO_utime" )
   NAME   ( "PRO_stime" )
   NAME   ( "PRO_vsize" )
   NAME   ( "PRO_groups" )
   NAME   ( "PRO_rel" )
   NAME   ( "PRO_run" )
   NAME   ( "PRO_io" )
NAMEEND

#define PROS sizeof(PRON)/sizeof(char*)

/****** sgeobj/job/--GR_Type **************************************************
*  NAME
*     GR_Type - CULL group element 
*
*  ELEMENTS
*     SGE_ULONG (GR_group)
*       Group IDs of a process. 
******************************************************************************/
enum {
   GR_group = GR_LOWERBOUND
};


LISTDEF( GR_Type )
   SGE_ULONG     ( GR_group, CULL_DEFAULT | CULL_HASH )
LISTEND

NAMEDEF( GRN )
   NAME   ( "GR_group" )
NAMEEND

#define GRS sizeof(GRN)/sizeof(char*)

#ifdef __cplusplus
}
#endif
#endif /* __PROCL_H */
