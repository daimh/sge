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
   com.sun.grid.cull.CullObject cullObj = (com.sun.grid.cull.CullObject)params.get("cullObj");

   if( cullObj == null ) {
     throw new IllegalStateException("param cullObj not found");
   }

   if( cullObj.getIdlName() == null ) {
     throw new IllegalStateException("cullObj " + cullObj.getName() + " is has no a idl name");
   }


   if( cullDef == null ) {
     throw new IllegalStateException("param cullDef not found");
   }

   String name = jh.getClassName(cullObj);

   String fullClassName = jh.getFullClassName(cullObj);
   
   fullClassName = fullClassName.replace('.', '/');

   String methodName = "Java_com_sun_grid_jgdi_jni_JGDI_fill" + name + "List";

   String listName = cullObj.getListName();

   if (listName == null) {
     // we not a ILISTDEF, return
     return;
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
#include "sge_gdi_ctx.h"
#include "cl_errors.h"
#include "sge_log.h"
#include "sge_error_class.h"
#include "jgdi_common.h"
#include "jgdi.h"
/*
   ILISTDEF(<%=cullObj.getName()%>, <%=cullObj.getIdlName()%>, <%=cullObj.getListName()%>, <%
    for(int i = 0; i < cullObj.getParamCount(); i++ ) { 
       if ( i > 0 ) { %> , <% } %> <%=cullObj.getParam(i)%> <% } %> )
*/
/* ------------------ GET --------------------------------------------------- */
/*
 * Class:     com_sun_grid_jgdi_jni_JGDI
 * Method:    get<%=jh.getClassName(cullObj)%>List
 */
JNIEXPORT void JNICALL <%=methodName%>(JNIEnv *env, jobject jgdi, jobject list) {

   jclass cls;    
   jclass obj_class;
    
   /* receive Cull Object */
   lList *lp = NULL;
   const lDescr *descr = NULL;
   lList *alp = NULL;
   lCondition *where = NULL;
   static lEnumeration *what  = NULL;
   jmethodID add_mid = NULL;
   lListElem *ep = NULL;
   jobject obj;
   char classname[] = "<%=fullClassName%>";
   sge_gdi_ctx_class_t *ctx = NULL; 
      
   sge_error_class_t *eh = NULL;
   
   
   DENTER(TOP_LAYER, "<%=methodName%>");
   
   /* create error handler */
   eh = sge_error_class_create();
   if (!eh) {
      THROW_ERROR((env, "sge_error_class_create failed"));
      DEXIT;
      return;
   }
   
   /* create what and where */
   if (!what) {
      what = lWhat("%T(ALL)", <%=cullObj.getName()%>);
   }   

   /* get context */
   ctx = getGDIContext(env, jgdi, &alp);

   alp = ctx->gdi(ctx, <%=listName%>, SGE_GDI_GET, &lp, where, what, NULL);
   /* if error throw exception */
   if(answer_list_has_error(&alp)) {
      dstring ds = DSTRING_INIT;
      answer_list_to_dstring(alp, &ds);
      THROW_ERROR((env, "sge_gdi() reported error: %s", sge_dstring_get_string(&ds)));
      sge_dstring_free(&ds);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   
   cls = (*env)->GetObjectClass(env, list);
   obj_class = (*env)->FindClass(env, classname);
   
   if (!obj_class) {
      DEXIT;
      return;
   }
 
   add_mid = get_methodid(env, cls, "add", "(Ljava/lang/Object;)Z", &alp);
   if (add_mid == 0) {
      dstring ds = DSTRING_INIT;
      answer_list_to_dstring(alp, &ds);
      THROW_ERROR((env, "get_methodid failed: %s", sge_dstring_get_string(&ds)));
      sge_dstring_free(&ds);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   
   descr = lGetListDescr(lp);
   for_each (ep, lp) {
      // convert to Java representation
      obj = listelem_to_obj(env, ep, descr, obj_class, &alp);
      if (!obj) {
         DEXIT;
         return;
      }   
      (*env)->CallVoidMethod( env, list, add_mid, obj );
   }

   // if error throw exception
   if(answer_list_has_error(&alp)) {
      dstring ds = DSTRING_INIT;
      answer_list_to_dstring(alp, &ds);
      THROW_ERROR((env, "listelem_to_obj failed: %s", sge_dstring_get_string(&ds)));
      sge_dstring_free(&ds);
   }

   lFreeList(&alp);
   DEXIT;
}

<%
     methodName = "Java_com_sun_grid_jgdi_jni_JGDI_add" + name;
%>
/* -------------- ADD ------------------------------------------------------- */
/*
 * Class:     com_sun_grid_jgdi_jni_JGDI
 * Method:    add<%=jh.getClassName(cullObj)%>
 * Signature: (L<%=fullClassName%>;)V
 */
JNIEXPORT void JNICALL <%=methodName%>(JNIEnv *env, jobject jgdi, jobject jobj)
{
   lList *lp = NULL;
   lList *alp = NULL;
   lCondition *where = NULL;
   static lEnumeration *what  = NULL;
   lListElem *ep = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   sge_error_class_t *eh = NULL;
   
   DENTER( TOP_LAYER, "<%=methodName%>" );

   eh = sge_error_class_create();
   if (!eh) {
      THROW_ERROR((env, "sge_error_class_create failed"));
      DEXIT;
      return;
   }   
   
   /* create what and where */
   if (!what) {
      what = lWhat("%T(ALL)", <%=cullObj.getName()%>);
   }   

   ep = obj_to_listelem(env, jobj, <%=cullObj.getName()%>, &alp);

   if (answer_list_has_error(&alp)) {
      dstring ds = DSTRING_INIT;
      answer_list_to_dstring(alp, &ds);
      THROW_ERROR((env, "obj_to_listelem: %s", sge_dstring_get_string(&ds)));
      sge_dstring_free(&ds);
      DEXIT;
      return; 
   }
 
   lp = lCreateList("<%=listName%> list to add", <%=cullObj.getName()%>);
   lAppendElem(lp, ep);

   /* get context */
   ctx = getGDIContext(env, jgdi, &alp);

   /* add to <%=cullObj.getName()%> list */

   alp = ctx->gdi(ctx, <%=listName%>, SGE_GDI_ADD, &lp, where, what, NULL);
   lFreeList(&lp);

   /* if error throw exception */
   if (answer_list_has_error(&alp)) {
      dstring ds = DSTRING_INIT;
      answer_list_to_dstring(alp, &ds);
      THROW_ERROR((env, "sge_gdi() reported error: %s", sge_dstring_get_string(&ds)));
      sge_dstring_free(&ds);
   }
 
   lFreeList(&alp);

   DEXIT;
}


<%
     methodName = "Java_com_sun_grid_jgdi_jni_JGDI_delete" + name;
%>

/*
 * Class:     com_sun_grid_jgdi_jni_JGDI
 * Method:    delete<%=name%>
 * Signature: (L<%=fullClassName%>;)V
 */
JNIEXPORT void JNICALL <%=methodName%>(JNIEnv *env, jobject jgdi, jobject jobj)
{
   lList *lp = NULL;
   lList *alp = NULL;
   lCondition *where = NULL;
   static lEnumeration *what  = NULL;
   lListElem *ep = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;

   DENTER( TOP_LAYER, "<%=methodName%>" );
   
   /* create what and where */
   if (!what) {
      what = lWhat("%T(ALL)", <%=cullObj.getName()%>);
   }   

   ep = obj_to_listelem(env, jobj, <%=cullObj.getName()%>, &alp);

   if (answer_list_has_error(&alp)) {
      dstring ds = DSTRING_INIT;
      answer_list_to_dstring(alp, &ds);
      THROW_ERROR((env, "obj_to_listelem: %s", sge_dstring_get_string(&ds)));
      sge_dstring_free(&ds);
      DEXIT;
      return; 
   }
 
   lp = lCreateList("<%=listName%> list to delete", <%=cullObj.getName()%>);
   lAppendElem(lp, ep);

   /* get context */
   ctx = getGDIContext(env,jgdi, &alp);

   /* delete from <%=cullObj.getName()%> list */
   alp = ctx->gdi(ctx, <%=listName%>, SGE_GDI_DEL, &lp, where, what, NULL);
   lFreeList(&lp);

   /* if error throw exception */
   if (answer_list_has_error(&alp)) {
      dstring ds = DSTRING_INIT;
      answer_list_to_dstring(alp, &ds);
      THROW_ERROR((env, "sge_gdi() reported error: %s", sge_dstring_get_string(&ds)));
      sge_dstring_free(&ds);
   }
 
   lFreeList(&alp);

   DEXIT;
}


