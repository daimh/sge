#ifndef __SGE_BINDING_HLP_H
#define __SGE_BINDING_HLP_H

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
 *   Portions of this code are Copyright 2011 Univa Inc.
 *
 ************************************************************************/

/*___INFO__MARK_END__*/

#include "uti/sge_dstring.h"
#include "uti/sge_binding_parse.h"

#if HAVE_HWLOC
#  define NOGDI /* else ERROR defined by wingdi.h pulled in on Cygwin */
#  include <hwloc.h>
extern
hwloc_topology_t sge_hwloc_topology;
#  if HWLOC_API_VERSION < 0x00010100
#     error "need hwloc version >= 1.1"
#  endif
#endif

/* functions related for parsing command line (see parse_qsub.c) */
/* shepherd also needs them */
bool parse_binding_parameter_string(const char* parameter, binding_type_t* type, 
      dstring* strategy, int* amount, int* stepsize, int* firstsocket, 
      int* firstcore, dstring* socketcorelist, dstring* error);

binding_type_t binding_parse_type(const char* parameter);

int binding_linear_parse_number(const char* parameter);
int binding_linear_parse_core_offset(const char* parameter);
int binding_linear_parse_socket_offset(const char* parameter);

int binding_striding_parse_number(const char* parameter);
int binding_striding_parse_first_core(const char* parameter);
int binding_striding_parse_first_socket(const char* parameter);
int binding_striding_parse_step_size(const char* parameter);

bool binding_explicit_has_correct_syntax(const char* parameter, dstring* error);
int get_explicit_number(const char* expl, const bool with_explicit_prefix);
bool check_explicit_binding_string(const char* expl, const int amount, const bool with_explicit_prefix);

const char* binding_get_topology_for_job(const char *binding_result);

bool topology_string_to_socket_core_lists(const char* topology, int** sockets,
                                     int** cores, int* amount);
/* Used as an "infinite" value.  The number of cores it's used for is
   defined as u_long32, but is treated as signed in the code.  */
#define BIND_INFINITY 0x7fffffff
#endif /* __SGE_BINDING_HLP_H */
