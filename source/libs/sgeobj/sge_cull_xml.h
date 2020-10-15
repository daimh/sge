#ifndef __CULL_XML_H
#define __CULL_XML_H
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

#include <stdio.h> 

#include "uti/sge_htable.h"

#include "sgeobj/sge_cull_xml_XMLA_L.h"
#include "sgeobj/sge_cull_xml_XMLS_L.h"
#include "sgeobj/sge_cull_xml_XMLH_L.h"
#include "sgeobj/sge_cull_xml_XMLE_L.h"

#include "basis_types.h"

#ifdef  __cplusplus
extern "C" {
#endif

void lWriteElemXMLTo(const lListElem *ep, FILE *fp, int ingnore_name);

lListElem* xml_getHead(const char *name, lList *liste, lList *attributs); 

void xml_addAttributeD(lListElem *xml_elem, const char *name, double value);
void xml_addAttribute(lListElem *xml_elem, const char *name, const char *value);

void xml_addStylesheet(lListElem *xml_head, const char* name, const char *url, const char *version);

lListElem *xml_append_Attr_D(lList *attributeList, const char *name, double value);
lListElem *xml_append_Attr_D8(lList *attributeList, const char *name, double value);
lListElem *xml_append_Attr_S(lList *attributeList, const char *name, const char *value);
lListElem *xml_append_Attr_I(lList *attributeList, const char *name, int value);
bool escape_string(const char *string, dstring *target);

#ifdef  __cplusplus
}
#endif
#endif /* #ifndef __CULL_XML_H */
