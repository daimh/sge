#ifndef __MSG_CULL_H
#define __MSG_CULL_H 
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

#include "basis_types.h"
/* 
** cull/src/cull_file.c
*/ 
#define MSG_CULL_NOPREFIXANDNOFILENAMEINWRITEELMTODISK   _MESSAGE(41000, _("no prefix and no filename in lWriteElemToDisk"))
#define MSG_CULL_NOPREFIXANDNOFILENAMEINREADELEMFROMDISK _MESSAGE(41001, _("no prefix and no filename in lReadElemFromDisk"))
#define MSG_CULL_NOTENOUGHMEMORYFORPACKINGXY_SS          _MESSAGE(41002, _("not enough memory for packing "SFN" "SFQ))
#define MSG_CULL_NOTENOUGHMEMORYFORUNPACKINGXY_SS        _MESSAGE(41003, _("not enough memory for unpacking "SFN" "SFQ))
#define MSG_CULL_FORMATERRORWHILEPACKINGXY_SS            _MESSAGE(41004, _("format error while packing "SFN" "SFQ))
#define MSG_CULL_UNEXPECTEDERRORWHILEPACKINGXY_SS        _MESSAGE(41005, _("unexpected error while packing "SFN" "SFQ))
#define MSG_CULL_UNEXPECTEDERRORWHILEUNPACKINGXY_SS      _MESSAGE(41006, _("unexpected error while unpacking "SFN" "SFQ))
#define MSG_CULL_FORMATERRORWHILEUNPACKINGXY_SS          _MESSAGE(41007, _("format error while unpacking "SFN" "SFQ))
#define MSG_CULL_CANTOPENXFORWRITINGOFYZ_SSS             _MESSAGE(41008, _("can't open "SFN" for writing of "SFN": "SFN))
#define MSG_CULL_CANTWRITEXTOFILEY_SS                    _MESSAGE(41009, _("can't write "SFN" to file "SFN))
#define MSG_CULL_CANTREADXFROMFILEY_SS                   _MESSAGE(41010, _("can't read "SFN" from file "SFN))
#define MSG_CULL_CANTGETFILESTATFORXFILEY_SS             _MESSAGE(41011, _("can't get file stat for "SFN" file "SFQ))
#define MSG_CULL_XFILEYHASZEROSIYE_SS                    _MESSAGE(41012, _(""SFN" file "SFQ" has zero size"))
#define MSG_CULL_ERRORREADINGXINFILEY_SS                 _MESSAGE(41013, _("error reading "SFN" in file "SFN))
#define MSG_CULL_BADARGUMENTWHILEUNPACKINGXY_SS          _MESSAGE(41014, _("bad argument error while unpacking "SFN" "SFQ))
#define MSG_CULL_ERRORININITPACKBUFFER_S                 _MESSAGE(41015, _("error in init_packbuffer: "SFN))
#define MSG_CULL_NOTENOUGHMEMORY_D                       _MESSAGE(41016, _("not enough memory to allocate %d bytes in init_packbuffer"))


/* 
** cull/src/cull_multitype.c
*/ 
#define MSG_CULL_GETINT_WRONGTYPEFORFIELDXY_SS           _MESSAGE(41025, _("lGetInt: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_GETULONG_WRONGTYPEFORFIELDXY_SS         _MESSAGE(41029, _("lGetUlong: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_GETSTRING_WRONGTYPEFORFILEDXY_SS        _MESSAGE(41030, _("lGetString: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_GETHOST_WRONGTYPEFORFILEDXY_SS          _MESSAGE(41031, _("lGetHost: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_GETLIST_WRONGTYPEFORFIELDXY_SS          _MESSAGE(41032, _("lGetList: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_GETFLOAT_WRONGTYPEFORFIELDXY_SS         _MESSAGE(41033, _("lGetFloat: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_GETDOUBLE_WRONGTYPEFORFIELDXY_SS        _MESSAGE(41034, _("lGetDouble: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_GETLONG_WRONGTYPEFORFIELDXY_SS          _MESSAGE(41035, _("lGetLong: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_GETCHAR_WRONGTYPEFORFIELDXY_SS          _MESSAGE(41036, _("lGetChar: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_GETREF_WRONGTYPEFORFIELDXY_SS           _MESSAGE(41037, _("lGetRef: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_SETINT_WRONGTYPEFORFIELDXY_SS           _MESSAGE(41038, _("lSetInt: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_SETULONG_WRONGTYPEFORFIELDXY_SS         _MESSAGE(41039, _("lSetUlong: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_SETSTRING_NOSUCHNAMEXYINDESCRIPTOR_IS   _MESSAGE(41040, _("lSetString: no such name (%d, "SFN") in descriptor"))
#define MSG_CULL_SETHOST_NOSUCHNAMEXYINDESCRIPTOR_IS     _MESSAGE(41041, _("lSetHost: no such name (%d, "SFN") in descriptor"))
#define MSG_CULL_SETSTRING_WRONGTYPEFORFIELDXY_SS        _MESSAGE(41042, _("lSetString: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_SETHOST_WRONGTYPEFORFIELDXY_SS          _MESSAGE(41043, _("lSetHost: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_XCHGLIST_WRONGTYPEFORFIELDXY_SS         _MESSAGE(41044, _("lXchgList: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_SETLIST_WRONGTYPEFORFIELDXY_SS          _MESSAGE(41045, _("lSetList: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_SETFLOAT_WRONGTYPEFORFIELDXY_SS         _MESSAGE(41046, _("lSetFloat: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_SETDOUBLE_WRONGTYPEFORFIELDXY_SS        _MESSAGE(41047, _("lSetDouble: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_SETLONG_WRONGTYPEFORFIELDXY_SS          _MESSAGE(41048, _("lSetLong: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_SETCHAR_WRONGTYPEFORFIELDXY_SS          _MESSAGE(41049, _("lSetChar: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_SETREF_WRONGTYPEFORFIELDXY_SS           _MESSAGE(41050, _("lSetRef: wrong type for field "SFN" ("SFN")" ))
#define MSG_CULL_ADDSUBSTRERRORXRUNTIMETYPE_S            _MESSAGE(41051, _("error: lAddSubStr("SFN"): run time type error"))
#define MSG_CULL_ADDSUBHOSTERRORXRUNTIMETYPE_S            _MESSAGE(41052, _("error: lAddSubHost("SFN"): run time type error"))
#define MSG_CULL_ADDELEMSTRERRORXRUNTIMETYPE_S           _MESSAGE(41053, _("error: lAddElemStr("SFN"): run time type error"))
#define MSG_CULL_ADDELEMHOSTERRORXRUNTIMETYPE_S           _MESSAGE(41054, _("error: lAddElemHost("SFN"): run time type error"))
#define MSG_CULL_DELELEMSTRERRORXRUNTIMETYPEERROR_S      _MESSAGE(41057, _("error: lDelElemStr("SFN"): run time type error"))
#define MSG_CULL_GETELEMSTRERRORXRUNTIMETYPE_S           _MESSAGE(41060, _("error: lGetElemStr("SFN"): run time type error"))
#define MSG_CULL_ADDSUBULONGERRORXRUNTIMETYPE_S          _MESSAGE(41062, _("error: lAddSubUlong("SFN"): run time type error"))
#define MSG_CULL_ADDELEMULONGERRORXRUNTIMETYPE_S         _MESSAGE(41063, _("error: lAddElemUlong("SFN"): run time type error"))
#define MSG_CULL_DELELEMULONGERRORXRUNTIMETYPE_S         _MESSAGE(41065, _("error: lDelElemUlong("SFN"): run time type error"))
#define MSG_CULL_GETELEMULONGERRORXRUNTIMETYPE_S         _MESSAGE(41067, _("error: lGetElemUlong("SFN"): run time type error"))
#define MSG_CULL_DELELEMCASESTRERRORXRUNTIMETYPE_S       _MESSAGE(41071, _("error: lDelElemCaseStr("SFN"): run time type error"))
#define MSG_CULL_DELELEMHOSTERRORXRUNTIMETYPE_S          _MESSAGE(41072, _("error: lDelElemHost("SFN"): run time type error"))
#define MSG_CULL_GETELEMCASESTRERRORXRUNTIMETYPE_S       _MESSAGE(41074, _("error: lGetElemCaseStr("SFN"): run time type error"))
#define MSG_CULL_GETELEMHOSTERRORXRUNTIMETYPE_S          _MESSAGE(41076, _("error: lGetElemHost("SFN"): run time type error"))
#define MSG_CULL_GETPOSULONG_GOTINVALIDPOSITION          _MESSAGE(41077, _("!!!!!!!!!! lGetPosUlong() got an invalid pos !!!!!!!!!!!!"))
#define MSG_CULL_GETPOSLIST_GOTANINVALIDPOS              _MESSAGE(41078, _("!!!!!!!!!!!11 lGetPosList() got an invalid pos !!!!!!!!"))

/* 
** cull/src/cull_where.c
*/ 
#define MSG_CULL_WHERE_SHOULDBEINTT                      _MESSAGE(41079, _("lWhere (should be a lIntT)"))
#define MSG_CULL_WHERE_SHOULDBESTRINGT                   _MESSAGE(41080, _("lWhere (should be a lStringT or lHostT)"))
#define MSG_CULL_WHERE_SHOULDBEULONGT                    _MESSAGE(41081, _("lWhere (should be a lUlongT)"))
#define MSG_CULL_WHERE_SHOULDBEFLOATT                    _MESSAGE(41082, _("lWhere (should be a lFloatT)"))
#define MSG_CULL_WHERE_SHOULDBEDOUBLET                   _MESSAGE(41083, _("lWhere (should be a lDoubleT)"))
#define MSG_CULL_WHERE_SHOULDBELONGT                     _MESSAGE(41084, _("lWhere (should be a lLongT)"))
#define MSG_CULL_WHERE_SHOULDBECHART                     _MESSAGE(41085, _("lWhere (should be a lCharT)"))
#define MSG_CULL_WHERE_SHOULDBEREFT                      _MESSAGE(41086, _("lWhere (should be a lRefT)"))
#define MSG_CULL_WHERE_OPERANDHITNOTOPERATORERROR        _MESSAGE(41087, _("operand does not match to operator"))

/* 
** cull/src/cull_lerrno.c
*/ 
#define MSG_CULL_LEMALLOC                _MESSAGE(41088, _("malloc failure"))
#define MSG_CULL_LEINCTYPE               _MESSAGE(41089, _("incompatible type"))
#define MSG_CULL_LEUNKTYPE               _MESSAGE(41090, _("unknown type"))
#define MSG_CULL_LEELEMNULL              _MESSAGE(41091, _("element is NULL"))
#define MSG_CULL_LENAMENOT               _MESSAGE(41092, _("name not in descriptor contained"))
#define MSG_CULL_LENAMEOUT               _MESSAGE(41093, _("name out of namespaces"))
#define MSG_CULL_LEDESCRNULL             _MESSAGE(41094, _("descriptor is NULL, empty descriptor"))
#define MSG_CULL_LENEGPOS                _MESSAGE(41095, _("negative position is not allowed"))
#define MSG_CULL_LESTRDUP                _MESSAGE(41096, _("strdup failure"))
#define MSG_CULL_LEFILENULL              _MESSAGE(41097, _("file pointer is NULL"))
#define MSG_CULL_LEFGETBRA               _MESSAGE(41098, _("fGetBra failed"))
#define MSG_CULL_LEFGETKET               _MESSAGE(41099, _("fGetKet failed"))
#define MSG_CULL_LEFGETINT               _MESSAGE(41100, _("fGetInt failed"))
#define MSG_CULL_LEFGETDESCR             _MESSAGE(41101, _("fGetDescr failed"))
#define MSG_CULL_LELISTNULL              _MESSAGE(41102, _("list is NULL"))
#define MSG_CULL_LECREATEELEM            _MESSAGE(41103, _("lCreateElem failure"))
#define MSG_CULL_LECOUNTDESCR            _MESSAGE(41104, _("lCountDescr failure"))
#define MSG_CULL_LEFIELDREAD             _MESSAGE(41105, _("reading field failure"))
#define MSG_CULL_LEFGETSTRING            _MESSAGE(41106, _("fGetString failure"))
#define MSG_CULL_LECREATELIST            _MESSAGE(41107, _("lCreateList failure"))
#define MSG_CULL_LEUNDUMPELEM            _MESSAGE(41108, _("lUndumpElem failure"))
#define MSG_CULL_LESSCANF                _MESSAGE(41109, _("sscanf failure"))
#define MSG_CULL_LESYNTAX                _MESSAGE(41110, _("syntax error"))
#define MSG_CULL_LEFGETLINE              _MESSAGE(41111, _("fGetLine failure"))
#define MSG_CULL_LEFGETS                 _MESSAGE(41112, _("fgets failure"))
#define MSG_CULL_LESPACECOMMENT          _MESSAGE(41113, _("space_comment failure"))
#define MSG_CULL_LEUNDUMPLIST            _MESSAGE(41114, _("lUndumpList failure"))
#define MSG_CULL_LECOPYSWITCH            _MESSAGE(41115, _("lCopySwitch failure"))
#define MSG_CULL_LEENUMNULL              _MESSAGE(41116, _("lEnumeration is NULL"))
#define MSG_CULL_LECONDNULL              _MESSAGE(41117, _("lCondition is NULL"))
#define MSG_CULL_LENOLISTNAME            _MESSAGE(41118, _("no listname specified"))
#define MSG_CULL_LEDIFFDESCR             _MESSAGE(41119, _("different list descriptors"))
#define MSG_CULL_LEDECHAINELEM           _MESSAGE(41120, _("lDechainElem failure"))
#define MSG_CULL_LEAPPENDELEM            _MESSAGE(41121, _("lAppendElem failure"))
#define MSG_CULL_LENOFORMATSTR           _MESSAGE(41122, _("format string is missing"))
#define MSG_CULL_LEPARSESORTORD          _MESSAGE(41123, _("lParseSortOrder failure"))
#define MSG_CULL_LEGETNROFELEM           _MESSAGE(41124, _("lgetNumberOfElem failure"))
#define MSG_CULL_LESORTORDNULL           _MESSAGE(41125, _("lSortOrder is NULL"))
#define MSG_CULL_LESUM                   _MESSAGE(41126, _("sum in where.c failure"))
#define MSG_CULL_LEOPUNKNOWN             _MESSAGE(41127, _("operator of lCondition struct unknown"))
#define MSG_CULL_LECOPYELEMPART          _MESSAGE(41128, _("lCopyElemPartialPack failure"))
#define MSG_CULL_LENULLARGS              _MESSAGE(41129, _("function argument is NULL"))
#define MSG_CULL_LEFALSEFIELD            _MESSAGE(41130, _("field is not allowed here"))
#define MSG_CULL_LEJOINDESCR             _MESSAGE(41131, _("lJoinDescr failure"))
#define MSG_CULL_LEJOIN                  _MESSAGE(41132, _("lJoin failure"))
#define MSG_CULL_LEJOINCOPYELEM          _MESSAGE(41133, _("lJoinCopyElem failure"))
#define MSG_CULL_LEADDLIST               _MESSAGE(41134, _("lAddList failure"))
#define MSG_CULL_LECOUNTWHAT             _MESSAGE(41135, _("lCountWhat failure"))
#define MSG_CULL_LEPARTIALDESCR          _MESSAGE(41136, _("lPartialDescr failure"))
#define MSG_CULL_LEENUMDESCR             _MESSAGE(41137, _("enumeration no subset of descriptor"))
#define MSG_CULL_LEENUMBOTHNONE          _MESSAGE(41138, _("at least one enumeration required"))
#define MSG_CULL_LENULLSTRING            _MESSAGE(41139, _("string NULL not allowed"))
#define MSG_CULL_LEPARSECOND             _MESSAGE(41140, _("parsing condition failed"))
#define MSG_CULL_LEFORMAT                _MESSAGE(41141, _("wrong packing format"))
#define MSG_CULL_LEOPEN                  _MESSAGE(41142, _("could not open file"))

/*
** cull_hash.c
*/
#define MSG_CULL_HASHTABLEALREADYEXISTS_S    _MESSAGE(41151, _("hash table already exists for field "SFQ))

/*
** cull_pack.c
*/
#define MSG_CULL_PACK_WRONG_VERSION_XX    _MESSAGE(41152, _("wrong cull version, read 0x%08x, but expected actual version 0x%08x"))

/*
** cull_pack.c, macros for cull_pack_strerror()
*/
#define MSG_CULL_PACK_SUCCESS             _MESSAGE(41153, _("packing successful"))
#define MSG_CULL_PACK_ENOMEM              _MESSAGE(41154, _("can't allocate memory"))
#define MSG_CULL_PACK_FORMAT              _MESSAGE(41155, _("invalid input parameter"))
#define MSG_CULL_PACK_BADARG              _MESSAGE(41156, _("bad argument"))
#define MSG_CULL_PACK_VERSION             _MESSAGE(41157, _("wrong cull version"))


#define MSG_CULL_GETPOSOBJECT_GOTANINVALIDPOS              _MESSAGE(41158, _("!!!!!!!!!!!11 lGetPosObject() got an invalid pos !!!!!!!!"))
#define MSG_CULL_GETOBJECT_WRONGTYPEFORFIELDXY_SS          _MESSAGE(41161, _("lGetObject: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_DECHAINOBJECT_WRONGTYPEFORFIELDXY_S       _MESSAGE(41163, _("lDechainObject: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_GETBOOL_WRONGTYPEFORFIELDXY_SS            _MESSAGE(41165, _("lGetBool: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_SETBOOL_WRONGTYPEFORFIELDXY_SS            _MESSAGE(41166, _("lSetBool: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_WHERE_SHOULDBEBOOL                        _MESSAGE(41167, _("lWhere (should be a lBoolT)"))
#define MSG_CULL_XNOTFOUNDINELEMENT_S                      _MESSAGE(41068, _("!!!!!!!!!! "SFN" not found in element !!!!!!!!!!"))
#define MSG_CULL_POINTER_NULLELEMENTFORX_S                 _MESSAGE(41069, _("!!!!!!!!!! got NULL element for "SFN" !!!!!!!!!!"))

/*
** Messages for lUlong64
*/
#endif /* __MSG_CULL_H */ 
#define MSG_CULL_GETULONG64_WRONGTYPEFORFIELDXY_SS         _MESSAGE(41170, _("lGetUlong64: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_SETULONG64_WRONGTYPEFORFIELDXY_SS         _MESSAGE(41171, _("lSetUlong64: wrong type for field "SFN" ("SFN")"))
#define MSG_CULL_ADDSUBULONG64ERRORXRUNTIMETYPE_S          _MESSAGE(41172, _("error: lAddSubUlong64("SFN"): run time type error"))
#define MSG_CULL_ADDELEMULONG64ERRORXRUNTIMETYPE_S         _MESSAGE(41173, _("error: lAddElemUlong64("SFN"): run time type error"))
#define MSG_CULL_DELELEMULONG64ERRORXRUNTIMETYPE_S         _MESSAGE(41174, _("error: lDelElemUlong64("SFN"): run time type error"))
#define MSG_CULL_GETELEMULONG64ERRORXRUNTIMETYPE_S         _MESSAGE(41175, _("error: lGetElemUlong64("SFN"): run time type error"))
#define MSG_CULL_GETPOSULONG64_GOTINVALIDPOSITION          _MESSAGE(41176, _("!!!!!!!!!! lGetPosUlong64() got an invalid pos !!!!!!!!!!!!"))
#define MSG_CULL_WHERE_SHOULDBEULONG64T                    _MESSAGE(41177, _("lWhere (should be a lUlong64T)"))
