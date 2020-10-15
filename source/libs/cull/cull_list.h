#ifndef __CULL_LIST_H
#define __CULL_LIST_H

#ifndef JGDI_GENERATE

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

#include <stdio.h> 

#include "basis_types.h"
#include "cull/cull_hashP.h"
#include "cull/pack.h"
#include "uti/sge_dstring.h"

#ifdef  __cplusplus
extern "C" {
#endif


#define NoName -1

typedef struct _lDescr lDescr;
typedef struct _lNameSpace lNameSpace;
typedef struct _lList lList;
typedef struct _lListElem lListElem;
typedef struct _lCondition lCondition;
typedef struct _lEnumeration lEnumeration;
typedef union _lMultiType lMultiType;
typedef struct _lSortOrder lSortOrder;
typedef struct _WhereArg WhereArg, *WhereArgList;

typedef float lFloat;
typedef double lDouble;
typedef u_long32 lUlong;
typedef u_long64 lUlong64;
typedef long lLong;
typedef char lChar;
typedef char lBool;
typedef int lInt;
typedef char *lString;
typedef char *lHost;
typedef lListElem *lObject;
typedef void*  lRef;

/* IF YOU CHANGE THIS ENUM, CHANGE cull_multitype.c/multitypes[] */
enum _enum_lMultiType {
   lEndT,                       /* This is the end of the descriptor */
   lFloatT,
   lDoubleT,
   lUlongT,
   lLongT,
   lCharT,
   lBoolT,
   lIntT,
   lStringT,
   lListT,
   lObjectT,
   lRefT,
   lHostT,
   lUlong64T
};

/* flags for the field definition 
 * reserve 8 bit for data types (currently only 4 bit in use)
 * see doc header cull/list/-Field_Attributes in cull_list.c for details
 */
#define CULL_DEFAULT       0x00000000
#define CULL_PRIMARY_KEY   0x00000100
#define CULL_HASH          0x00000200
#define CULL_UNIQUE        0x00000400
#define CULL_JGDI_HIDDEN   0x00000800
#define CULL_CONFIGURE     0x00001000
#define CULL_SPOOL         0x00002000
#define CULL_SUBLIST       0x00010000
#define CULL_SPOOL_PROJECT 0x00020000
#define CULL_SPOOL_USER    0x00040000
#define CULL_JGDI_RO       0x00080000
#define CULL_JGDI_CONF     0x00100000
#define CULL_IS_REDUCED    0x00200000

/*
** JGDI specific defines
*/
#define JGDI_ROOT_OBJ( idlname, listname, operators )
#define JGDI_EVENT_OBJ(eventtypes)
#define JGDI_OBJ(idlname)
#define JGDI_MAPPED_OBJ(impl_class_name)
#define JGDI_PRIMITIVE_OBJ(primitive_attribute)
#define JGDI_PRIMITIVE_ROOT_OBJ(idlname, primitive_attribute, listname, operators)
#define JGDI_INHERITED_ROOT_OBJ(idlname, listname, operators)
#define JGDI_MAP_OBJ(key_field, value_field)


#define BASIC_UNIT 50         /* Don't touch */
#define MAX_DESCR_SIZE  4*BASIC_UNIT

#ifdef __SGE_GDI_LIBRARY_HOME_OBJECT_FILE__

#define LISTDEF( name ) lDescr name[] = {
#define LISTEND {NoName, lEndT, NULL}};

#define SGE_INT(name,flags)         { name, lIntT    | flags, NULL }, /* don't use it, not implemented on gdi level */
#define SGE_HOST(name,flags)        { name, lHostT   | flags, NULL },
#define SGE_STRING(name,flags)      { name, lStringT | flags, NULL },
#define SGE_FLOAT(name,flags)       { name, lFloatT  | flags, NULL },
#define SGE_DOUBLE(name,flags)      { name, lDoubleT | flags, NULL },
#define SGE_CHAR(name,flags)        { name, lCharT   | flags, NULL },
#define SGE_LONG(name,flags)        { name, lLongT   | flags, NULL },
#define SGE_ULONG(name,flags)       { name, lUlongT  | flags, NULL },
#define SGE_ULONG64(name,flags)     { name, lUlong64T  | flags, NULL },
#define SGE_BOOL(name,flags)        { name, lBoolT   | flags, NULL },
#define SGE_LIST(name,type,flags)   { name, lListT   | flags, NULL },
#define SGE_MAP(name,type,flags)   { name, lListT   | flags, NULL },
#define SGE_MAPLIST(name,type,flags)   { name, lListT   | flags, NULL },
#define SGE_OBJECT(name,type,flags) { name, lObjectT | flags, NULL },
#define SGE_REF(name,type,flags)    { name, lRefT    | flags, NULL },

#define DERIVED_LISTDEF(name,parent) lDescr *name = parent
#define DERIVED_LISTEND ; 

#define SGE_INT_D(name,flags,def)         { name, lIntT    | flags, NULL },
#define SGE_HOST_D(name,flags,def)        { name, lHostT   | flags, NULL },
#define SGE_STRING_D(name,flags,def)      { name, lStringT | flags, NULL },
#define SGE_FLOAT_D(name,flags,def)       { name, lFloatT  | flags, NULL },
#define SGE_DOUBLE_D(name,flags,def)      { name, lDoubleT | flags, NULL },
#define SGE_CHAR_D(name,flags,def)        { name, lCharT   | flags, NULL },
#define SGE_LONG_D(name,flags,def)        { name, lLongT   | flags, NULL },
#define SGE_ULONG_D(name,flags,def)       { name, lUlongT  | flags, NULL },
#define SGE_ULONG64_D(name,flags,def)     { name, lUlong64T  | flags, NULL },
#define SGE_BOOL_D(name,flags,def)        { name, lBoolT   | flags, NULL },
#define SGE_LIST_D(name,type,flags,def)   { name, lListT   | flags, NULL },
#define SGE_MAP_D(name,type,flags,defkey,keyvalue,jgdi_keyname,jgdi_valuename)   { name, lListT   | flags, NULL},
#define SGE_MAPLIST_D(name,type,flags,defkey,defvalue,jgdi_keyname,jgdi_valuename)   { name, lListT   | flags, NULL},
#define SGE_OBJECT_D(name,type,flags,def) { name, lObjectT | flags, NULL },
#define SGE_REF_D(name,type,flags,def)    { name, lRefT    | flags, NULL },

/* 
 * For lists, objects and references the type of the subordinate object(s) 
 * must be specified.
 * If multiple types are thinkable or non cull data types are referenced,
 * use the following define CULL_ANY_SUBTYPE as type
 */
#define CULL_ANY_SUBTYPE 0

#define NAMEDEF( name ) char *name[] = {
#define NAME( name ) name ,
#define NAMEEND    };

#else

#ifdef __SGE_GDI_LIBRARY_SUBLIST_FILE__

#define LISTDEF( name )
#define LISTEND

#define DERIVED_LISTDEF( name, parent )
#define DERIVED_LISTEND


#define SGE_INT(name,flags)
#define SGE_HOST(name,flags)
#define SGE_STRING(name,flags)
#define SGE_FLOAT(name,flags)
#define SGE_DOUBLE(name,flags)
#define SGE_CHAR(name,flags)
#define SGE_LONG(name,flags)
#define SGE_ULONG(name,flags)
#define SGE_ULONG64(name,flags)
#define SGE_BOOL(name,flags)
#define SGE_LIST(name,type,flags) __SUBTYPE_MAPPING__ name type
#define SGE_MAP(name,type,flags) __SUBTYPE_MAPPING__ name type
#define SGE_MAPLIST(name,type,flags) __SUBTYPE_MAPPING__ name type
#define SGE_OBJECT(name,type,flags)
#define SGE_REF(name,type,flags)

#define SGE_INT_D(name,flags,def)
#define SGE_HOST_D(name,flags,def)
#define SGE_STRING_D(name,flags,def)
#define SGE_FLOAT_D(name,flags,def)
#define SGE_DOUBLE_D(name,flags,def)
#define SGE_CHAR_D(name,flags,def)
#define SGE_LONG_D(name,flags,def)
#define SGE_LONG64_D(name,flags,def)
#define SGE_ULONG_D(name,flags,def)
#define SGE_BOOL_D(name,flags,def)
#define SGE_LIST_D(name,type,flags,def)
#define SGE_MAP_D(name,type,flags,defkey,keyvalue,jgdi_keyname,jgdi_valuename)
#define SGE_MAPLIST_D(name,type,flags,defkey,defvalue,jgdi_keyname,jgdi_valuename)
#define SGE_OBJECT_D(name,type,flags,def)
#define SGE_REF_D(name,type,flags,def)

#define NAMEDEF( name ) 
#define NAME( name )
#define NAMEEND

#else

#define LISTDEF( name ) extern lDescr name[];
#define LISTEND

#define DERIVED_LISTDEF(name,parent) extern lDescr *name
#define DERIVED_LISTEND ; 

#define SGE_INT(name,flags)
#define SGE_HOST(name,flags)
#define SGE_STRING(name,flags)
#define SGE_FLOAT(name,flags)
#define SGE_DOUBLE(name,flags)
#define SGE_CHAR(name,flags)
#define SGE_LONG(name,flags)
#define SGE_ULONG(name,flags)
#define SGE_ULONG64(name,flags)
#define SGE_BOOL(name,flags)
#define SGE_LIST(name,type,flags)
#define SGE_MAP(name,type,flags)
#define SGE_MAPLIST(name,type,flags)
#define SGE_OBJECT(name,type,flags)
#define SGE_REF(name,type,flags)

#define SGE_INT_D(name,flags,def)
#define SGE_HOST_D(name,flags,def)
#define SGE_STRING_D(name,flags,def)
#define SGE_FLOAT_D(name,flags,def)
#define SGE_DOUBLE_D(name,flags,def)
#define SGE_CHAR_D(name,flags,def)
#define SGE_LONG_D(name,flags,def)
#define SGE_ULONG_D(name,flags,def)
#define SGE_ULONG64_D(name,flags,def)
#define SGE_BOOL_D(name,flags,def)
#define SGE_LIST_D(name,type,flags,def)
#define SGE_MAP_D(name,type,flags,defkey,keyvalue,jgdi_keyname,jgdi_valuename)
#define SGE_MAPLIST_D(name,type,flags,defkey,defvalue,jgdi_keyname,jgdi_valuename)
#define SGE_OBJECT_D(name,type,flags,def)
#define SGE_REF_D(name,type,flags,def)

#define NAMEDEF( name ) extern char *name[];
#define NAME( name )
#define NAMEEND

#endif
#endif

struct _lNameSpace {
   int lower;
   int size;
   char **namev;
};

struct _lDescr {
   int nm;                             /* name */
   int mt;                             /* multitype information */
   cull_htable ht;
};

/* LIST SPECIFIC FUNCTIONS */
/* fixme:  inline some of this? */
/* Note most of these can't be declared pure because they do i/o
   (DENTER); perhaps they shouldn't.  */
const char *lGetListName(const lList *lp);
const lDescr *lGetListDescr(const lList *lp);
int lGetNumberOfElem(const lList *lp);
int lGetNumberOfRemainingElem(const lListElem *ep);
int lGetElemIndex(const lListElem *ep, const lList *lp);

const lDescr *lGetElemDescr(const lListElem *ep);
void lWriteElem(const lListElem *ep);
void lWriteElemTo(const lListElem *ep, FILE *fp);
void lWriteElemToStr(const lListElem *ep, dstring *buffer);

void lWriteList(const lList *lp);
void lWriteListTo(const lList *lp, FILE *fp);
void lWriteListToStr(const lList* lp, dstring* buffer);

lListElem *lCreateElem(const lDescr *dp);
lList *lCreateList(const char *listname, const lDescr *descr);
lList *lCreateListHash(const char *listname, const lDescr *descr, bool hash);
lList *lCreateElemList(const char *listname, const lDescr *descr, int nr_elem);

void lFreeElem(lListElem **ep);
void lFreeList(lList **ilp);

int lAddList(lList *lp0, lList **lp1);
int lAppendList(lList *lp0, lList *lp1);
int lOverrideStrList(lList *lp0, lList *lp1, int nm, const char *str);
lList *lAddSubList(lListElem *ep, int nm, lList *to_add);
int lCompListDescr(const lDescr *dp0, const lDescr *dp1);
lList *lCopyList(const char *name, const lList *src);
lList *lCopyListHash(const char *name, const lList *src, bool hash);
lListElem *lCopyElem(const lListElem *src);
lListElem *lCopyElemHash(const lListElem *src, bool isHash);

int lModifyWhat(lListElem *dst, const lListElem *src, const lEnumeration *enp);

int 
lCopyElemPartialPack(lListElem *dst, int *jp, const lListElem *src, 
                     const lEnumeration *ep, bool isHash, sge_pack_buffer *pb);

int 
lCopySwitchPack(const lListElem *sep, lListElem *dep, int src_idx, int dst_idx,
                bool isHash, lEnumeration *ep, sge_pack_buffer *pb);

int lAppendElem(lList *lp, lListElem *ep);
lListElem *lDechainElem(lList *lp, lListElem *ep);
void lDechainList(lList *source, lList **target, lListElem *ep);
lListElem *lDechainObject(lListElem *parent, int name);
int lRemoveElem(lList *lp, lListElem **ep);
int lInsertElem(lList *lp, lListElem *ep, lListElem *new_elem);

int lPSortList(lList *lp, const char *fmt, ...);
int lSortList(lList *lp, const lSortOrder *sp);
int lUniqStr(lList *lp, int keyfield);
int lUniqHost(lList *lp, int keyfield);

lListElem *lFirst(const lList *lp);
lListElem *lLast(const lList *lp);
lListElem *lNext(const lListElem *ep);
lListElem *lPrev(const lListElem *ep);

lListElem *lFindNext(const lListElem *ep, const lCondition *cp);
lListElem *lFindPrev(const lListElem *ep, const lCondition *cp);
lListElem *lFindFirst(const lList *lp, const lCondition *cp);
lListElem *lFindLast(const lList *lp, const lCondition *cp);

#define mt_get_type(mt) ((mt) & 0x000000FF)
#define mt_do_hashing(mt) (((mt) & CULL_HASH) ? true : false)
#define mt_is_unique(mt) (((mt) & CULL_UNIQUE) ? true : false)

bool lListElem_is_pos_changed(const lListElem *ep, int pos) __attribute__ ((__pure__));
bool lListElem_is_changed(const lListElem *ep) __attribute__ ((__pure__));
bool lList_clear_changed_info(lList *lp);
bool lListElem_clear_changed_info(lListElem *lp);

/* #define for_each(ep,lp) for (ep=lFirst(lp);ep;ep=lNext(ep)) */
/* #define for_each_rev(ep,lp) for (ep=lLast(lp);ep;ep=lPrev(ep)) */

#define for_each(ep,lp) for (ep = ((lp) ? (lp)->first : (lListElem *) NULL); ep; ep = ep->next)
#define for_each_rev(ep,lp) for (ep = ((lp) ? (lp)->last : (lListElem *) NULL); ep; ep = ep->prev)

#define for_each_where(ep,lp,cp) \
   for (ep=lFindFirst(lp,cp);ep;ep=lFindNext(ep,cp))
#define for_each_where_rev(ep,lp,cp) \
   for (ep=lFindLast(lp,cp);ep;ep=lFindPrev(ep,cp))

#ifdef  __cplusplus
}
#endif

#endif /* JGDI_GENERATE */

#endif /* #ifndef __CULL_LIST_H */

