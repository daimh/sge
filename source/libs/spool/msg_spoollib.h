#ifndef __MSG_SPOOLLIB_H
#define __MSG_SPOOLLIB_H
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

#include "basis_types.h"


/* 
 * libs/spool/sge_spooling.c
 */
#define MSG_SPOOL_UNKNOWNOBJECTTYPEINCONTEXT_SS     _MESSAGE(59000, _("object type "SFQ" is not handled in spooling context "SFQ)) 
#define MSG_SPOOL_NODEFAULTRULEFORTYPEINCONTEXT_SS  _MESSAGE(59001, _("no default rule for object type "SFQ" in spooling context "SFQ))
#define MSG_SPOOL_NORULESFORTYPEINCONTEXT_SS        _MESSAGE(59002, _("no rules for object type "SFQ" in spooling context "SFQ))
#define MSG_SPOOL_CORRUPTRULEINCONTEXT_SSS          _MESSAGE(59003, _("corrupt rule "SFQ" in spooling context "SFQ": "SFQ" is missing"))
#define MSG_SPOOL_RULEINCONTEXTFAILEDWRITING_SS     _MESSAGE(59005, _("rule "SFQ" in spooling context "SFQ" failed writing an object"))
#define MSG_SPOOL_NOVALIDCONTEXT_S                  _MESSAGE(59009, _("no valid spooling context passed to "SFQ))
#define MSG_SPOOL_CONTEXTNEEDSNAME                  _MESSAGE(59010, _("spooling context has to have a name"))
#define MSG_SPOOL_NOVALIDSPOOLTYPE_S                _MESSAGE(59011, _("no valid spool type passed to "SFQ))
#define MSG_SPOOL_NOVALIDRULE_S                     _MESSAGE(59012, _("no valid spooling rule passed to "SFQ))
#define MSG_SPOOL_CONTEXTCONTAINSNOTYPES_S          _MESSAGE(59013, _("spooling context "SFQ" does not contain any object type descriptions"))
#define MSG_SPOOL_CONTEXTCONTAINSNORULES_S          _MESSAGE(59014, _("spooling context "SFQ" does not contain any spooling"))
#define MSG_SPOOL_TYPECONTAINSNORULES_SS            _MESSAGE(59015, _("type definition "SFQ" in spooling context "SFQ" contains no references to rules"))
#define MSG_SPOOL_TYPEHASNODEFAULTRULE_SS           _MESSAGE(59016, _("type definition "SFQ" in spooling context "SFQ" has no default rule"))
#define MSG_SPOOL_TYPEHASMORETHANONEDEFAULTRULE_SS  _MESSAGE(59017, _("type definition "SFQ" in spooling context "SFQ" has more than one default rule"))
#define MSG_SPOOL_RULEALREADYEXISTS_SS              _MESSAGE(59018, _("a rule named "SFQ" already exists in spooling context "SFQ))
#define MSG_SPOOL_TYPEALREADYHASDEFAULTRULE_S       _MESSAGE(59019, _("spooling type "SFQ" already has a default rule, cannot add a second one"))
#define MSG_SPOOL_STARTUPOFRULEFAILED_SS  _MESSAGE(59020, _("startup of rule "SFQ" in context "SFQ" failed"))
#define MSG_SPOOL_SHUTDOWNOFRULEFAILED_SS  _MESSAGE(59021,_("shutdown of rule "SFQ" in context "SFQ" failed"))
#define MSG_SPOOL_MAINTENANCEOFRULEFAILED_SS  _MESSAGE(59022,_("maintenance function of rule "SFQ" in context "SFQ" failed"))
#define MSG_SPOOL_TRIGGEROFRULEFAILED_SS  _MESSAGE(59023,_("trigger function of rule "SFQ" in context "SFQ" failed"))
#define MSG_SPOOL_TRANSACTIONOFRULEFAILED_SS  _MESSAGE(59024,_("transaction function of rule "SFQ" in context "SFQ" failed"))
#define MSG_SPOOL_SETOPTIONOFRULEFAILED_SS  _MESSAGE(59025,_("set_option function of rule "SFQ" in context "SFQ" failed"))
/* 
 * libs/spool/sge_spooling_utilities.c
 */
#define MSG_DONTKNOWHOWTOSPOOLSUBLIST_SS _MESSAGE(59100, _("don't know how to spool list attribute "SFQ" in function "SFQ""))
#define MSG_UNKNOWNOBJECTTYPEFOR_SS _MESSAGE(59101, _("unknown object type for list attribute "SFQ" in function "SFQ))
#define MSG_NONAMEFORATTRIBUTE_D _MESSAGE(59103, _("no or invalid name for attribute %d"))
#define MSG_SPOOL_CANTRESOLVEHOSTNAME_SS _MESSAGE(59104, _("can't resolve host name "SFQ": "SFN))

/*
 * libs/spool/<method>/<code>
 * messages common to different spooling methods
 */
#define MSG_SPOOL_SPOOLINGOFXNOTSUPPORTED_S  _MESSAGE(59900, _("(un)spooling objects of type "SFQ" not supported"))
#define MSG_ATTRIBUTENOTINOBJECT_S _MESSAGE(59901, _("attribute "SFQ" is not part of current object"))
/* unused:
#define MSG_CANTREADSUBLISTASPRIMARYKEYVALUEUNKNOWN_S _MESSAGE(59902, _("can't unspool sublist "SFQ" as primary key value is not (yet) known"))
#define MSG_CANTOPENTRANSACTIONALREADYOPEN _MESSAGE(59903, _("can't open a transaction - it is already open"))
#define MSG_CANTCLOSETRANSACTIONNONEOPEN _MESSAGE(59904, _("can't close a transaction - we have no transaction open"))
#define MSG_OBJECTWITHPRIMARYKEYUNKNOWN_S _MESSAGE(59905, _("object with primary key "SFQ" unknown to database spooling"))
#define MSG_PARENTKEYORIDNULL _MESSAGE(59906, _("required primary key or database id of the parent object is NULL"))
#define MSG_UNKNOWNTABLENAMEFORSUBLIST_S  _MESSAGE(59907, _("table name unknown for sublist "SFN))
#define MSG_NOTABLENAMEPASSEDTO_S _MESSAGE(59908, _("no table_name passed to function "SFN))
#define MSG_UNKNOWNPREFIXORKEYNMFORTABLE_S _MESSAGE(59909, _("prefix or primary key unknown for table "SFN))
#define MSG_SPOOL_WRONGVERSION_SS      _MESSAGE(59910, _("wrong version: database schema was created for version "SFQ", we run version "SFQ))
*/
#endif /* __MSG_SPOOLLIB_H */
