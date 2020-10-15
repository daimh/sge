#ifndef __CULLTRANS_H
#define __CULLTRANS_H
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

#include <map>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "culltrans_repository.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "cull.h"

#ifdef __cplusplus
}
#endif

#ifdef HAVE_STD
using namespace std;
#endif

#if 0
// the following must be bigger than sizeof(_enum_multitype)
#define lBoolT                         9
#endif

// global stuff from culltrans.cpp
extern map<string, List>               lists;
extern map<string, List>::iterator     active;
extern map<int, string>                constants;
extern int                             last_qidl_only;
extern FILE*                           disthdr;
extern const char*                     yyin_name;
extern const char*                     multiType2sgeType[];
extern const char*                     multiType2idlType[];
extern const char*                     multiType2getType[];

// functions from culltrans_parse.cpp
void newList(const char* name, const char* type, const char* sge_list_type, const bool interface = true);
void endList();
void newElem(const int type, const char* name, const bool readonly = false, const bool idlonly = false, const bool key = false);
void newXElem(const char* type, const char* name);
void newListElem(const char* name, const char* type, const bool readonly = false, const bool idlonly = false);
void newObjElem(const char* name, const char* type, const bool readonly = false, const bool idlonly = false);
void newXList(const char* name, const char* type = NULL, const bool object = false);
void newIDL(const char* line);
void newWord(const char* word);
void newOther(const char other);

// actual output functions
bool writeIDL(map<string, List>::iterator& elem);
bool writeHdr(map<string, List>::iterator& elem);
bool writeImplementation(map<string, List>::iterator& elem);
bool writeConsts();

#endif
