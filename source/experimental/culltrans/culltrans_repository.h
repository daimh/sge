#ifndef __CULLTRANS_REPOSITORY_H
#define __CULLTRANS_REPOSITORY_H
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

// culltrans_repository.h
// data structures to store cull list information
// when parsing through sge_*L.h files
// relies heavily on STL

#include <vector>
#include <set>
#include <string>

#ifdef HAVE_STD
using namespace std;
#endif

extern "C"
{
#include "cull.h"
}

// struct Elem
// this is a single list element of a cull list
// (e.g. STRING, ULONG, ...)
//
// type:       the type of the element
// name:       the name of the element
// listType:   the type of list a LIST element references.
//             only used when type==lListT
// readonly:   element will be a readonly attribute in interfaces
// object:     references exactly one other list
//             (e.g. Job references ONE Queue it runs on)
// key:        this element is the unique identifier of the list (eg QU_qname)
// idlonly:    this element is idl-only
struct Elem {
   int                                       type;
   string                                    name;
   string                                    listType;
   bool                                      readonly;
   bool                                      object;
   bool                                      key;
   bool                                      idlonly;

   Elem(const int t, const char* n, const bool r, const bool k=false, const bool io=false)
      : type(t), name(n), readonly(r), object(false), key(k), idlonly(io) {}
   Elem(const char* n, const char* lType, const bool ro = false, const bool obj = false, const bool io=false) 
      : type(lListT), name(n), listType(lType), readonly(ro), object(obj), key(false), idlonly(io) {}
};

// struct List
// this is a complete cull list. can be translated
// to an IDL struct or interface. contains struct Elems
// file:       the file where this list is defined
//             (e.g. "../gdilib/sge_queueL.h")
// name:       the name of the list (e.g. Calendar)
// type:       the type of the list (e.g. CAL_Type)
// elems:      the list's elements
// interf:     flag indicating ILISTDEF or SLISTDEF
//             ILISTDEF will be translated to an IDL interface
//             SLISTDEF will be translated to an IDL struct
// idl:        code to be inlined into to the .idl file
//             must be between /*IDL and XIDL*/ in the cull
//             header file
struct List {
   string                                    file;
   string                                    name;
   string                                    type;
   string                                    sge_list_type;
   vector<Elem>                              elems;
   bool                                      interf;
   string                                    idl;
   
   List(const char* t, const char* n, const char* f, const char* clt, const bool i = true) 
               : file(f?f:""), name(n?n:""), type(t?t:""), sge_list_type(clt?clt:""), interf(i) {}
};

#endif // __CULLTRANS_REPOSITORY_H
