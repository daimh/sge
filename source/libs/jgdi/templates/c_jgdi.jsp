<%
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
%>

<%
   com.sun.grid.cull.JavaHelper jh = (com.sun.grid.cull.JavaHelper)params.get("javaHelper");
   com.sun.grid.cull.CullDefinition cullDef = (com.sun.grid.cull.CullDefinition)params.get("cullDef");
   com.sun.grid.cull.CullObject cullObj = (com.sun.grid.cull.CullObject)params.get("cullObj");
   
   class CGDIGenerator {
       
       String fullClassname;
       String classname;
       String listname;
       String cullname;
       
       public CGDIGenerator(String fullClassname, String classname, String listname, String cullname) {
           this.fullClassname = fullClassname.replace('.','/');
           this.classname = classname;
           this.listname = listname;
           this.cullname = cullname;
       } 
       
       
       public void genListMethod() {
           
           String methodName = "Java_com_sun_grid_jgdi_jni_JGDIImpl_fill" + classname + "List";
%>           
/*
 * Class:     com_sun_grid_jgdi_jni_JGDIImpl
 * Method:    get<%=classname%>List
 */
JNIEXPORT void JNICALL <%=methodName%>WithAnswer(JNIEnv *env, jobject jgdi, jobject list, jobject filter, jobject answers) {
   
   DENTER(TOP_LAYER, "<%=methodName%>");
   jgdi_fill(env, jgdi, list, filter, "<%=fullClassname%>", <%=listname%>, <%=cullname%>, answers);
   DEXIT;
}
<%           
       } // end of genListMethod
       
       
       public void genAddMethod() {
         String methodName = "Java_com_sun_grid_jgdi_jni_JGDIImpl_add" + classname;
%>
/* -------------- ADD ------------------------------------------------------- */
/*
 * Class:     com_sun_grid_jgdi_jni_JGDIImpl
 * Method:    add<%=classname%>
 * Signature: (L<%=fullClassname%>;)V
 */
JNIEXPORT void JNICALL <%=methodName%>(JNIEnv *env, jobject jgdi, jobject jobj)
{
   DENTER(TOP_LAYER, "<%=methodName%>");
   jgdi_add(env, jgdi, jobj, "<%=fullClassname%>", <%=listname%>, <%=cullname%>, NULL);
   DEXIT;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIImpl
 * Method:    add<%=classname%>WithAnswer
 * Signature: (L<%=fullClassname%>;L/java/util/List;)V
 */
JNIEXPORT void JNICALL <%=methodName%>WithAnswer(JNIEnv *env, jobject jgdi, jobject jobj, jobject answers)
{
   DENTER(TOP_LAYER, "<%=methodName%>WithAnswer");
   jgdi_add(env, jgdi, jobj, "<%=fullClassname%>", <%=listname%>, <%=cullname%>, answers);
   DEXIT;
}
<%
       } // end of genAddMethod     

       public void genDeleteMethod() {
         String methodName = "Java_com_sun_grid_jgdi_jni_JGDIImpl_delete" + classname;
%>
/* -------------- Delete ------------------------------------------------------- */           
/*
 * Class:     com_sun_grid_jgdi_jni_JGDIImpl
 * Method:    delete<%=classname%>
 * Signature: (L<%=fullClassname%>;)V
 */
JNIEXPORT void JNICALL <%=methodName%>(JNIEnv *env, jobject jgdi, jobject jobj)
{
   DENTER(TOP_LAYER, "<%=methodName%>");
   jgdi_delete(env, jgdi, jobj, "<%=fullClassname%>", <%=listname%>, <%=cullname%>, false, NULL);
   DEXIT;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIImpl
 * Method:    delete<%=classname%>WithAnswer
 * Signature: (L<%=fullClassname%>;L/java/util/List;)V
 */
JNIEXPORT void JNICALL <%=methodName%>WithAnswer(JNIEnv *env, jobject jgdi, jobject jobj, jobject answers)
{
   DENTER(TOP_LAYER, "<%=methodName%>WithAnswer");
   jgdi_delete(env, jgdi, jobj, "<%=fullClassname%>", <%=listname%>, <%=cullname%>, false, answers);
   DEXIT;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIImpl
 * Method:    delete<%=classname%>sWithAnswer
 * Signature: ([Ljava/lang/Object;ZL/java/util/List;)V
 */
JNIEXPORT void JNICALL <%=methodName%>sWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray jobj_array, jboolean forced, jobject options, jobject answers)
{
   DENTER(TOP_LAYER, "<%=methodName%>sWithAnswer");
   jgdi_delete_array(env, jgdi, jobj_array, "<%=fullClassname%>", <%=listname%>, <%=cullname%>, forced, options, answers);
   DEXIT;
}

<%                      
       } // end of genDeleteMethod
       
       public void genUpdateMethod() {
           
          String methodName = "Java_com_sun_grid_jgdi_jni_JGDIImpl_update" + classname;
%>
/* -------------- Update ------------------------------------------------------- */
/*
 * Class:     com_sun_grid_jgdi_jni_JGDIImpl
 * Method:    update<%=classname%>
 * Signature: (L<%=fullClassname%>;)V
 */
JNIEXPORT void JNICALL <%=methodName%>(JNIEnv *env, jobject jgdi, jobject jobj)
{
   DENTER(TOP_LAYER, "<%=methodName%>");
   jgdi_update(env, jgdi, jobj, "<%=fullClassname%>", <%=listname%>, <%=cullname%>, NULL);
   DEXIT;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIImpl
 * Method:    update<%=classname%>WithAnswer
 * Signature: (L<%=fullClassname%>;, L/java/util/List;)V
 */
JNIEXPORT void JNICALL <%=methodName%>WithAnswer(JNIEnv *env, jobject jgdi, jobject jobj, jobject answers)
{
   DENTER(TOP_LAYER, "<%=methodName%>");
   jgdi_update(env, jgdi, jobj, "<%=fullClassname%>", <%=listname%>, <%=cullname%>, answers);
   DEXIT;
}
<%           
       } // end of genUpdateMethod
   } // end of class CGDIGenerator
   
   // -------------------------------------------------------------------------
   // Build the generators 
   // -------------------------------------------------------------------------

   java.util.List<CGDIGenerator> generators = new java.util.ArrayList<CGDIGenerator>();
   
   if (cullObj == null ) {
     throw new IllegalStateException("param cullObj not found");
   }
   if (cullObj.getIdlName() == null ) {
     throw new IllegalStateException("cullObj " + cullObj.getName() + " is has no idl name");
   }
   if (cullDef == null ) {
     throw new IllegalStateException("param cullDef not found");
   }
   if (!cullObj.isRootObject()) {
      return;
   }
   String listname = cullObj.getListName();

   if (listname == null) {
     // we not a ILISTDEF, return
     return;
   }
   {
   CGDIGenerator gen = null;
   
   if (cullObj.getParentName() != null) {      
      gen = new CGDIGenerator(jh.getFullClassName(cullObj), cullObj.getIdlName(), 
                              listname, cullObj.getParentName());
   } else {
      gen = new CGDIGenerator(jh.getFullClassName(cullObj), cullObj.getIdlName(), 
                              listname, cullObj.getName());
   }
   generators.add(gen);
   }
%>
<%
   boolean first = true;
   for (CGDIGenerator gen : generators) {
      if (first) {
         first = false;
%>
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
#include "sge_gdi_ctx.h"
#include "cl_errors.h"
#include "sge_log.h"
#include "sge_error_class.h"
#include "jgdi_common.h"
#include "jgdi.h"

#define JGDI_DEBUG

<%}
      gen.genListMethod();
      gen.genAddMethod();
      gen.genDeleteMethod();
      gen.genUpdateMethod();
   }
   
%>


