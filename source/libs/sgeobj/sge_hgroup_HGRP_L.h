#ifndef __SGE_HROUPL_H
#define __SGE_HROUPL_H

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

#include "sgeobj/sge_boundaries.h"
#include "cull/cull.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */

/****** sgeobj/hgroup/--HGRP_Type *********************************************
*  NAME
*     HGRP_Type - CULL hostgroup element
*
*  ELEMENTS
*     SGE_HOST(HGRP_name)
*        Name of the hostgroup
*
*     SGE_LIST(HGRP_host_list)
*        List of hostnames and/or hostgroup names
*     
*  FUNCTION 
*
*     HGRP_Type elements are used to define groups of hosts. Each group
*     will be identified by a unique name.
*     Each hostgroup might refer to none, one or multiple hosts and/or
*     hostgroups. This object makes it possible to define a network of
*     hostgroups and hosts.
*
*     
*                       --------------                                 
*                       |            |
*                       V            | 0:x
*                 ------------- -----| 
*                 | HGRP_Type |               ------------
*                 ------------- ------------> | hostname |
*                                    0:x      ------------
*     
*     Example
*
*        Following diagram shows a network of 9 hostgroups (A; B; C; 
*        D; E; G; H; I). Each of those groups references one host 
*        (A -> a; B -> b; C -> c; ...). Additionally some of those 
*        hostgroups refer to one (A -> C; B -> C; C -> E; ...) or two 
*        hostgroups (E -> F,G; F -> H,I) The connections are all 
*        uni-directional, you have to read the diagram from the left
*        to the right.
*
*                 -----                           -----
*                 | A | -- a                      | H | -- h
*                 ----- \                       / -----
*                         -----           -----
*                         | C | -- c      | F | -- f
*                         -----         / -----
*                 ----- /       \ -----         \ -----
*                 | B | -- b      | E | -- e      | I | -- i
*                 -----           -----           -----
*                         ----- /       \ -----
*                         | D | -- d      | G | -- g
*                         -----           -----
*
*     Several functions exist to create such networks and to find
*     certain sets of hosts and hostgroups within such a network:
*
*     hgroup_find_references("E", &answer, master_list, &hosts, &groups)
*        hosts -> e 
*        groups -> F, G
* 
*     hgroup_find_all_references("E", &answer, master_list, &hosts, &groups)
*        hosts -> e, f, g, h, i
*        groups -> F, G, H, I
*
*     hgroup_find_referencees("E", &answer, master_list, &groups)
*        groups -> C, D
*
*     hgroup_find_all_referencees("E", &answer, master_list, &groups)
*        groups -> A, B, C, D
*
*  SEE ALSO
*     sgeobj/hgroup/hgroup_list_get_master_list()
*     sgeobj/hgroup/hgroup_list_locate()
*     sgeobj/hgroup/hgroup_create()
*     sgeobj/hgroup/hgroup_add_references()
*     sgeobj/hgroup/hgroup_find_all_references()
*     sgeobj/hgroup/hgroup_find_references()
*     sgeobj/hgroup/hgroup_find_all_referencees()
*     sgeobj/hgroup/hgroup_find_referencees()
*     sgeobj/hgroup/hgroup_list_exists()
*******************************************************************************/
enum {
   HGRP_name = HGRP_LOWERBOUND,
   HGRP_host_list,
   HGRP_cqueue_list
};

LISTDEF(HGRP_Type)
   JGDI_ROOT_OBJ(Hostgroup, SGE_HGRP_LIST, ADD | MODIFY | DELETE | GET | GET_LIST)
   JGDI_EVENT_OBJ(ADD(sgeE_HGROUP_ADD) | MODIFY(sgeE_HGROUP_MOD) | DELETE(sgeE_HGROUP_DEL) | GET_LIST(sgeE_HGROUP_LIST))
   SGE_HOST_D(HGRP_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL | CULL_JGDI_CONF, "@template")
   SGE_LIST(HGRP_host_list, HR_Type, CULL_SPOOL | CULL_JGDI_CONF) 
   SGE_LIST(HGRP_cqueue_list, CQ_Type, CULL_DEFAULT | CULL_JGDI_HIDDEN) 
LISTEND

NAMEDEF(HGRPN)
   NAME("HGRP_name")
   NAME("HGRP_host_list")
   NAME("HGRP_cqueue_list")
NAMEEND

#define HGRPS sizeof(HGRPN)/sizeof(char*)

/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif

#endif /* __SGE_HGROUPL_H */
