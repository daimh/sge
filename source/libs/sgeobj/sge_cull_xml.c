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
 * Portions of this code are Copyright 2011 Univa Corporation.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

#include "uti/sge_rmon.h"
#include "uti/sge_dstring.h"
#include "uti/sge_log.h"

#include "cull/cull_lerrnoP.h"
#include "cull/cull_list.h"
#include "cull/msg_cull.h"

#include "sgeobj/sge_cull_xml.h"

static void lWriteElemXML_(const lListElem *ep, int nesting_level, FILE *fp, int ignore_cull_name); 
static void lWriteListXML_(const lList *lp, int nesting_level, FILE *fp, int ignore_cull_name); 
static bool lAttributesToString_(const lList *attr_list, dstring *attr);
static void lWriteXMLHead_(const lListElem *ep, int nesting_level, FILE *fp, int ignore_cull_name);
static lListElem *append_Attr_S(lList *attributeList, const char *name, const char *value);

lListElem* xml_getHead(const char *name, lList *list, lList *attributes) {
   lListElem *xml_head = NULL;

   xml_head = lCreateElem(XMLH_Type);

   if (xml_head) {
      
      lSetString(xml_head, XMLH_Version, "<?xml version='1.0'?>");
      lSetString(xml_head, XMLH_Name, name);
      lSetList(xml_head, XMLH_Attribute, attributes);
      lSetList(xml_head, XMLH_Element, list);
      if (strcmp(name, "detailed_job_info") == 0)
         xml_addAttribute(xml_head, "xmlns:xsd", "http://arc.liv.ac.uk/repos/darcs/sge/source/dist/util/resources/schemas/qstat/detailed_job_info.xsd");
      else if (strcmp(name, "job_info") == 0)
         xml_addAttribute(xml_head, "xmlns:xsd", "http://arc.liv.ac.uk/repos/darcs/sge/source/dist/util/resources/schemas/qstat/qstat.xsd");
      else if (strcmp(name, "message") == 0)
         xml_addAttribute(xml_head, "xmlns:xsd", "http://arc.liv.ac.uk/repos/darcs/sge/source/dist/util/resources/schemas/qstat/message.xsd");
      /* fixme:  also called with "communication_error" and "unknown_jobs"  */

/* we do not support stylesheets yet */
/*    xml_addStylesheet(xml_head, "xmlns:xsl", "http://www.w3.org/1999/XSL/Transform", "1.0");*/
   }
   
   return xml_head;
}

void xml_addStylesheet(lListElem *xml_head, const char* name, const char *url, const char *version) {
   lListElem *stylesheet_elem = lCreateElem(XMLS_Type);
   lList *stylesheet_list = NULL;
   
   if (stylesheet_elem) {
      lSetString(stylesheet_elem, XMLS_Name, name);
      lSetString(stylesheet_elem, XMLS_Value, url);
      lSetString(stylesheet_elem, XMLS_Version, version);
      stylesheet_list = lGetList(xml_head, XMLH_Stylesheet);
      if (!stylesheet_list)
         lSetList(xml_head, XMLH_Stylesheet, (stylesheet_list = lCreateList("Stylesheet", XMLS_Type)));
      
      lAppendElem(stylesheet_list, stylesheet_elem);
   }   
}

void xml_addAttributeD(lListElem *xml_elem, const char *name, double value){
   char buffer[20]="";
   dstring string;

   sge_dstring_init(&string, buffer, 20);
   xml_addAttribute(xml_elem, name, sge_dstring_sprintf(&string, "%f", value));
}

void xml_addAttribute(lListElem *xml_elem, const char *name, const char *value){
   lListElem *attr_elem = lCreateElem(XMLA_Type);
   lList *attr_list = NULL;
   dstring mod_value = DSTRING_INIT;
   bool is_mod_value; 
   DENTER(CULL_LAYER, "xml_addAttribute");

   is_mod_value = escape_string(value, &mod_value); 
   
   if (attr_elem) {
      lSetString(attr_elem, XMLA_Name, name);
      lSetString(attr_elem, XMLA_Value, (is_mod_value?sge_dstring_get_string(&mod_value):""));
      if (lGetPosViaElem(xml_elem, XMLH_Attribute, SGE_NO_ABORT) != -1) {   
         attr_list = lGetList(xml_elem, XMLH_Attribute);
         if (!attr_list)
            lSetList(xml_elem, XMLH_Attribute, (attr_list = lCreateList("Attributes", XMLA_Type)));
      }
      else if (lGetPosViaElem(xml_elem, XMLE_Attribute, SGE_NO_ABORT) != -1) {
         attr_list = lGetList(xml_elem, XMLE_Attribute);
         if (!attr_list)
            lSetList(xml_elem, XMLE_Attribute, (attr_list = lCreateList("Attributes", XMLA_Type)));
      }
      else {
         sge_dstring_free(&mod_value);
         CRITICAL((SGE_EVENT, "xml_addAttribute() called on wrong cull structure"));
         
         DEXIT;   
         abort();
      }
      lAppendElem(attr_list, attr_elem);
   }
   sge_dstring_free(&mod_value);
   DEXIT;
   return;
}

/****** cull/list/lWriteListXMLTo() **********************************************
*  NAME
*     lWriteListXMLTo() -- Write a list to a file stream 
*
*  SYNOPSIS
*     void lWriteListXMLTo(const lList *lp, FILE *fp) 
*
*  FUNCTION
*     Write a list to a file stream in XML format
*
*  INPUTS
*     const lList *lp   - list 
*     int nesting_level - current nesting level
*     FILE *fp          - file stream 
*
*  NOTE:
*    MT-NOTE: is thread save, works only on the objects which are passed in 
*
*******************************************************************************/
static void lWriteListXML_(const lList *lp, int nesting_level, FILE *fp, int ignore_cull_name) 
{
   lListElem *ep;
   char indent[128];
   int i;
   bool is_XML_elem = false;
   dstring attr = DSTRING_INIT;
   bool is_attr = false;

   DENTER(CULL_LAYER, "lWriteListXML_");
   
   if (!lp) {
      LERROR(LELISTNULL);
      DEXIT;
      return;
   }

   {
      int max = nesting_level * 2;
      if (max > 128)
      max = 128;
      for (i = 0; i < max; i++)
         indent[i] = ' ';
      indent[i] = '\0';
   }

   for_each(ep, lp) {
      is_XML_elem = false;
      is_attr = false;

      if (lGetPosViaElem(ep, XMLE_Attribute, SGE_NO_ABORT) != -1) {
         sge_dstring_clear(&attr);
         is_attr = lAttributesToString_(lGetList(ep, XMLE_Attribute), &attr);  
         is_XML_elem = true;
      }
      
      if (is_XML_elem && (lGetBool(ep, XMLE_Print)))  {
         lListElem *elem = lGetObject(ep, XMLE_Element);
	 const char *tag = lGetString(elem, XMLA_Name);
	 bool tag_ok = tag && strlen(tag);
         if (!fp){
            if (lGetString(elem, XMLA_Value) != NULL){
	       if (tag_ok) {
		  DPRINTF(("%s<%s%s>", indent, tag, (is_attr?sge_dstring_get_string(&attr):"")));
		  DPRINTF(("%s", lGetString(elem, XMLA_Value)));
	       }
               lWriteListXML_(lGetList(ep, XMLE_List), nesting_level+1, fp, ignore_cull_name);
	       if (tag_ok)
		 DPRINTF(("</%s>\n", tag));
            }
            else{
	       if (tag_ok)
		  DPRINTF(("%s<%s%s>\n", indent, tag, (is_attr?sge_dstring_get_string(&attr):"")));
               lWriteListXML_(lGetList(ep, XMLE_List), nesting_level+1, fp, ignore_cull_name);
	       if (tag_ok)
		  DPRINTF(("%s</%s>\n", indent, tag));
            }
         }
         else {
            if (lGetString(elem, XMLA_Value) != NULL){
	       if (tag_ok) {
		  fprintf(fp, "%s<%s%s>", indent, tag, (is_attr?sge_dstring_get_string(&attr):""));
		  fprintf(fp, "%s", lGetString(elem, XMLA_Value));
	       }
               lWriteListXML_(lGetList(ep, XMLE_List), nesting_level+1, fp, ignore_cull_name);
	       if (tag_ok)
		  fprintf(fp, "</%s>\n", tag);
            }
            else{
	       if (tag_ok)
		  fprintf(fp, "%s<%s%s>\n", indent, tag, (is_attr?sge_dstring_get_string(&attr):""));
               lWriteListXML_(lGetList(ep, XMLE_List), nesting_level+1, fp, ignore_cull_name);
	       if (tag_ok)
		 fprintf(fp, "%s</%s>\n", indent, tag);
            }
         }
      }
      else {
         const char* listName = lGetListName(lp);
	 bool tag_ok = listName && strlen(listName);
         if (strcmp (listName, "No list name specified") == 0){
            listName = "element";
         }
         if (!fp){
            if (tag_ok)
               DPRINTF(("%s<%s%s>\n", indent, listName, ((is_attr)?sge_dstring_get_string(&attr):"")));
            
            lWriteElemXML_(ep, nesting_level+1, NULL, ignore_cull_name);
            
            if (tag_ok)
               DPRINTF(("%s</%s>\n", indent, listName));
         }
         else {
            if (tag_ok)
               fprintf(fp, "%s<%s%s>\n", indent, listName, ((is_attr)?sge_dstring_get_string(&attr):""));
            
            lWriteElemXML_(ep, nesting_level+1, fp, ignore_cull_name);
            if (tag_ok)
               fprintf(fp, "%s</%s>\n", indent, listName);
         }
      }
   }
   sge_dstring_free(&attr);
   DEXIT;
}


/****** cull/list/lWriteElemXML() ************************************************
*  NAME
*     lWriteElemXML() -- Write a element to monitoring level CULL_LAYER 
*
*  SYNOPSIS
*     void lWriteElemXML(const lListElem *ep) 
*
*  FUNCTION
*     Write a element to monitoring level CULL_LAYER a info message 
*
*  INPUTS
*     const lListElem *ep - element 
******************************************************************************/
void lWriteElemXML(const lListElem *ep) 
{
   DENTER(CULL_LAYER, "lWriteElem");

   lWriteElemXML_(ep, 0, NULL, -1);

   DEXIT;
}

/****** cull/list/lWriteElemXMLTo() **********************************************
*  NAME
*     lWriteElemXMLTo() -- Write a element to file stream 
*
*  SYNOPSIS
*     void lWriteElemXMLTo(const lListElem *ep, FILE *fp) 
*
*  FUNCTION
*     Write a element to file stream in XML format 
*
*  INPUTS
*     const lListElem *ep  - element 
*     FILE *fp             - file stream 
*     int ignore_cull_name - ignore the specified cull name if != -1 
*   
*  NOTE:
*    MT-NOTE: is thread save, works only on the objects which are passed in 
******************************************************************************/
void lWriteElemXMLTo(const lListElem *ep, FILE *fp, int ignore_cull_name) 
{
   DENTER(CULL_LAYER, "lWriteElemTo");

   lWriteElemXML_(ep, 0, fp, ignore_cull_name);

   DEXIT;
}

static void lWriteElemXML_(const lListElem *ep, int nesting_level, FILE *fp, int ignore_cull_name) 
{
   int i;
   char space[128];
   lList *tlp;
   lListElem *tep;
   const char *str;
   const char *attr_name;
   int max = nesting_level *2;

   DENTER(CULL_LAYER, "lWriteElemXML_");

   if (!ep) {
      LERROR(LEELEMNULL);
      DRETURN_VOID;
   }

   if (max > 128) {
      max = 128;
   }

   for (i = 0; i < max; i++) {
      space[i] = ' ';
   }
   space[i] = '\0';

   if (lGetPosViaElem(ep, XMLH_Version, SGE_NO_ABORT) != -1) {   
      lWriteXMLHead_(ep, nesting_level, fp, ignore_cull_name);        
   } else if (lGetPosViaElem(ep, XMLE_Attribute, SGE_NO_ABORT) !=-1 ) {
      if (lGetBool(ep, XMLE_Print)) {
         dstring attr = DSTRING_INIT;
         bool is_attr; 
         lListElem *elem = lGetObject(ep, XMLE_Element);
         is_attr = lAttributesToString_(lGetList(ep, XMLE_Attribute), &attr); 
         if (!fp){
            DPRINTF(("%s<%s%s>", space, lGetString(elem, XMLA_Name), (is_attr?sge_dstring_get_string(&attr):"")));
            DPRINTF(("%s", lGetString(elem, XMLA_Value))); 
            lWriteListXML_(lGetList(ep, XMLE_List), nesting_level+1, fp, ignore_cull_name);
            DPRINTF(("</%s>\n", lGetString(elem, XMLA_Name)));
         }
         else {
            fprintf(fp, "%s<%s%s>", space, lGetString(elem, XMLA_Name), (is_attr?sge_dstring_get_string(&attr):""));
            fprintf(fp, "%s", lGetString(elem, XMLA_Value));
            lWriteListXML_(lGetList(ep, XMLE_List), nesting_level+1, fp, ignore_cull_name);
            fprintf(fp, "</%s>\n", lGetString(elem, XMLA_Name));
         }
         sge_dstring_free(&attr);
      }
      else{
         lWriteElemXML_(lGetObject(ep, XMLE_Element), nesting_level, fp, ignore_cull_name);
         lWriteListXML_(lGetList(ep, XMLE_List), nesting_level, fp, ignore_cull_name);
      }
   } else {  
      for (i = 0; mt_get_type(ep->descr[i].mt) != lEndT; i++) {
         if (ep->descr[i].nm != ignore_cull_name || ignore_cull_name == -1) { 
            /* ignore empty lists */
            switch (mt_get_type(ep->descr[i].mt)) {
            case lListT :
               tlp = lGetPosList(ep, i);
               if (lGetNumberOfElem(tlp) == 0)
                  continue;
               break;   
            case lStringT:
               if (lGetPosString(ep, i) == NULL)
                  continue;
               break;
            case lRefT:
               /* connot print a ref */
               continue;
            }   
            
            attr_name = lNm2Str(ep->descr[i].nm);
            if (!fp) {
               DPRINTF(("%s<%s>", space, attr_name));
            }
            else {
               fprintf(fp, "%s<%s>", space, attr_name);
            }
            switch (mt_get_type(ep->descr[i].mt)) {
            case lIntT:
               if (!fp) {
                  DPRINTF(("%d", lGetPosInt(ep, i)));
               } else {
                  fprintf(fp, "%d", lGetPosInt(ep, i));
               }
               break;
            case lUlongT:
               if (!fp) {
                  DPRINTF(( sge_u32, lGetPosUlong(ep, i)));
               } else {
                  fprintf(fp, sge_u32, lGetPosUlong(ep, i));
               }
               break;
            case lStringT:
               {
                  dstring string = DSTRING_INIT;
                  str = lGetPosString(ep, i);
                  if (escape_string(str, &string)) {
                     if (!fp) {
                        DPRINTF(("%s", sge_dstring_get_string(&string) ));
                     } else {
                        fprintf(fp, "%s", sge_dstring_get_string(&string) );
                     }
                     
                     sge_dstring_free(&string);
                  }
               }
               break;

            case lHostT:
               {
                  dstring string = DSTRING_INIT;
                  str = lGetPosHost(ep, i);
                  if (escape_string(str, &string)) {
                     if (!fp) {
                        DPRINTF(("%s", sge_dstring_get_string(&string) ));
                     } else {
                        fprintf(fp, "%s", sge_dstring_get_string(&string) );
                     }
                     sge_dstring_free(&string);
                  }
               }
               break;

            case lListT:
               tlp = lGetPosList(ep, i);
               if (tlp) {
                  if (!fp) {
                     DPRINTF(("\n"));
                  }   
                  else {
                     fprintf(fp, "\n");
                  }
                  lWriteListXML_(tlp, nesting_level + 1, fp, ignore_cull_name);

                  if (!fp){ 
                     DPRINTF(("%s", space));
                  }
                  else { 
                     fprintf(fp, "%s", space);
                  }
               }
               break;

            case lObjectT:
               tep = lGetPosObject(ep, i);
               if (tep) {
                  if (!fp) {
                     DPRINTF(("\n"));
                  }
                  else {
                     fprintf(fp, "\n");
                  }
                  lWriteElemXML_(tep, nesting_level, fp, ignore_cull_name);
                  if (!fp) {
                     DPRINTF(("%s", space));
                  }
                  else { 
                     fprintf(fp, "%s", space);
                  }
               }
               break;
            case lFloatT:
               if (!fp) {
                  DPRINTF(("%f", lGetPosFloat(ep, i)));
               } else {
                  fprintf(fp, "%f", lGetPosFloat(ep, i));
               }
               break;
            case lDoubleT:
               if (!fp) {
                  DPRINTF(("%f", lGetPosDouble(ep, i)));
               } else {
                  fprintf(fp, "%f", lGetPosDouble(ep, i));
               }
               break;
            case lLongT:
               if (!fp) {
                  DPRINTF(("%ld", lGetPosLong(ep, i)));
               } else {
                  fprintf(fp, "%ld", lGetPosLong(ep, i));
               }
               break;
            case lBoolT:
               if (!fp) {
                  DPRINTF(("%s", lGetPosBool(ep, i) ? "true" : "false"));
               } else {
                  fprintf(fp, "%s", lGetPosBool(ep, i) ? "true" : "false");
               }
               break;
            case lCharT:
               if (!fp) {
                  DPRINTF(("%c", lGetPosChar(ep, i)));
               } else {
                  fprintf(fp, "%c", lGetPosChar(ep, i));
               }
               break;
            case lRefT:
               /* cannot be printed */
               break;
            default:
               unknownType("lWriteElem");
            }
            if (!fp) {
               DPRINTF(("%s</%s>\n", space, attr_name));
            }
            else {
               fprintf(fp, "</%s>\n", attr_name);
            }
         }
      }
   }
   DEXIT;
   return;
}

lListElem *xml_append_Attr_D(lList *attributeList, const char *name, double value) {
   char buffer[20];
   /* fixme: worry about locale */
   snprintf(buffer, sizeof(buffer),"%.5f",value);
   return append_Attr_S(attributeList, name, buffer);
}

lListElem *xml_append_Attr_D8(lList *attributeList, const char *name, double value) {
   char buffer[20];
   if (value > 99999999)
      snprintf(buffer, sizeof(buffer), "%.3g", value);
   else
      sprintf(buffer,"%.0f", value);
   return append_Attr_S(attributeList, name, buffer);
}

lListElem *xml_append_Attr_S(lList *attributeList, const char *name, const char *value){
   dstring string = DSTRING_INIT;
   lListElem *xml_Elem = NULL;
   
   if (escape_string(value, &string)) {
      xml_Elem = append_Attr_S(attributeList, name, sge_dstring_get_string(&string));
      sge_dstring_free(&string);
   }
   else
      xml_Elem = append_Attr_S(attributeList, name, "");
         
   return xml_Elem;         
}

lListElem *xml_append_Attr_I(lList *attributeList, const char *name, int value) {
   char buffer[20];
   sprintf(buffer,"%d", value);
   return append_Attr_S(attributeList, name, buffer);
}

static bool lAttributesToString_(const lList *attr_list, dstring *attr){
   const lListElem *attr_elem = NULL;
   
   if(attr == NULL)
      return false;

   if (lGetNumberOfElem(attr_list) == 0){
      return false;
   }
   else {
      for_each(attr_elem, attr_list) {
         const char *name = lGetString(attr_elem, XMLA_Name);
         const char *value = lGetString(attr_elem, XMLA_Value);

         sge_dstring_sprintf_append(attr, " %s=\"%s\"", name, value);
      }
   }
   return true;
}

static void lWriteXMLHead_(const lListElem *ep, int nesting_level, FILE *fp, int ignore_cull_name) {
   const lListElem *elem = NULL;
   const char *name = NULL;
   dstring attr = DSTRING_INIT;
   bool is_attr = false;
   
   DENTER(CULL_LAYER, "lWriteXMLHead_");
   
   if (!ep){
      DEXIT;
      return;
   }

   name = lGetString(ep, XMLH_Name);
   is_attr = lAttributesToString_(lGetList(ep, XMLH_Attribute), &attr);

   if (!fp) {
      DPRINTF(("%s\n", lGetString(ep, XMLH_Version)));
      
      for_each(elem, lGetList(ep, XMLH_Stylesheet)) {
         DPRINTF(("<xsl:stylesheet %s=\"%s\" version=\"%s\">\n", 
                  lGetString(elem, XMLS_Name),
                  lGetString(elem, XMLS_Value),
                  lGetString(elem, XMLS_Version)));
      }
      DPRINTF(("<%s %s>\n", name, (is_attr?sge_dstring_get_string(&attr):"")));
      lWriteListXML_(lGetList(ep, XMLH_Element), nesting_level +1, fp, ignore_cull_name); 
      DPRINTF(("</%s>\n", name));
   }
   else {
      fprintf(fp, "%s\n", lGetString(ep, XMLH_Version));
      for_each(elem, lGetList(ep, XMLH_Stylesheet)) {
         fprintf(fp, "<xsl:stylesheet %s=\"%s\" version=\"%s\">\n", 
                  lGetString(elem, XMLS_Name),
                  lGetString(elem, XMLS_Value),
                  lGetString(elem, XMLS_Version));
      }
      fprintf(fp, "<%s %s>\n", name, (is_attr?sge_dstring_get_string(&attr):""));
      lWriteListXML_(lGetList(ep, XMLH_Element), nesting_level +1, fp, ignore_cull_name); 
      fprintf(fp, "</%s>\n", name);
   }
   sge_dstring_free(&attr);
   DEXIT;
}

static lListElem *append_Attr_S(lList *attributeList, const char *name, const char *value) {
   lListElem *elem = NULL;
   lListElem *parent = NULL; 
   if (!value)
      return parent;
   parent = lCreateElem(XMLE_Type);
   if (parent) {
      elem = lCreateElem(XMLA_Type);
      if (elem){
         lSetString(elem, XMLA_Name, name);
         lSetString(elem, XMLA_Value, value);
         lSetObject(parent, XMLE_Element, elem);
      }
      lSetBool(parent, XMLE_Print, true);
      lAppendElem(attributeList, parent);
   }
   return parent;
}

bool escape_string(const char *string, dstring *target){
   int size;
   int i;
 
   DENTER(CULL_LAYER, "escape_string");
   
   if (target == NULL) {
      DPRINTF(("no target string in escape_string()\n"));
      DEXIT;
      abort();
   } 
 
   if (string == NULL){
      DEXIT;
      return false;
   }
     
   size = strlen(string);

   for(i = 0; i<size; i++){
      switch(string[i]){
         case '<' : sge_dstring_append(target, "%lt;");
            break;
         case '>' : sge_dstring_append(target, "&gt;");
            break;
         case '&' : sge_dstring_append(target, "&amp;");
            break;
         case '\'' : sge_dstring_append(target, "&apos;");
            break;
         case '\"' : sge_dstring_append(target, "&quot;");
            break;
         default :
            sge_dstring_append_char(target, string[i]); 
      }
   }
   DEXIT;
   return true;
}

