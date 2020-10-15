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
<%
   com.sun.grid.cull.JavaHelper jh = (com.sun.grid.cull.JavaHelper)params.get("javaHelper");
   com.sun.grid.cull.CullDefinition cullDef = (com.sun.grid.cull.CullDefinition)params.get("cullDef");

   if( cullDef == null ) {
     throw new IllegalStateException("param cullDef not found");
   }
%>
#include <ctype.h>
#include <string.h>
#include <jni.h>
#include "basis_types.h"
#include "cull.h"
#include "commlib.h"
#include "sge_rmon.h"
#include "sge_all_listsL.h"
#include "sge_answer.h"
#include "sge_prog.h"
#include "sge_bootstrap.h"
#include "sge_gdi.h"
#include "sge_gdi_ctx.h"
#include "jgdi_common.h"



lDescr* get_descr(const char* name) {
<%
   for (String name : cullDef.getObjectNames()) {
     com.sun.grid.cull.CullObject cullObj = cullDef.getCullObject(name);
%>
   if( strcmp(name, "<%=name%>") == 0 ) return <%=name%>;
<%   
  } // end of for
%>
   return NULL;
}

lDescr* get_descr_for_classname(const char* classname ) {

<%
   for (String name : cullDef.getObjectNames()) {
     com.sun.grid.cull.CullObject cullObj = cullDef.getCullObject(name);
     String classname = jh.getFullClassName(cullObj);
     if (cullObj.getParentName() != null) {
        name = cullObj.getParentName();
     }
%>
   if (strcmp(classname, "<%=classname%>") == 0) return <%=name%>;
<%   
  } // end of for
%>
   return NULL;
}


<%
   for (String name : cullDef.getEnumNames()) {
      com.sun.grid.cull.CullEnum aEnum = cullDef.getEnum(name);
%>
int get_<%=name%>(const char* a_<%=name%>_name) {
<%
    for (String elemName : aEnum.getElems()) {
%>
    if (strcmp("<%=elemName%>", a_<%=name%>_name) == 0) return <%=elemName%>;
<%
    } // end of inner for
  } // end of for
%>

const char* get_classname_for_descr(const lDescr *descr) {
<%
   for (String name : cullDef.getObjectNames()) {
     com.sun.grid.cull.CullObject cullObj = cullDef.getCullObject(name);
     String classname = jh.getFullClassName(cullObj);
     classname = classname.replace('.', '/');
     if(cullObj.getParentName() != null) {
        name = cullObj.getParentName();
     }
%>
   if (descr == <%=name%>) {
      return "<%=classname%>";
   }
<%   
  } // end of for
%>
   return NULL;
}

int get_master_list_for_classname(const char* classname) {
<%
   for (String name : cullDef.getObjectNames()) {  
     com.sun.grid.cull.CullObject cullObj = cullDef.getCullObject(name);
     String classname = jh.getFullClassName(cullObj);
%>
   if( strcmp(classname, "<%=classname%>") == 0 ) {
<%
     if(cullObj.isRootObject()) {
%>
     return <%=cullObj.getListName()%>;
<%   } else { %>     
     return -1;
<%   } %>      
   }
<%   
  } // end of for
%>
   return -1;
}
