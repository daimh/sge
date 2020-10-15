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
#include <stdlib.h>
#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/Label.h>

#include <Xmt/Xmt.h>
#include <Xmt/WidgetType.h>
#include <Xmt/Layout.h>
#include <Xmt/InputField.h>

#include "Spinbox.h"
#include "IconList.h"
#include "ListTree.h"
#include "Tab.h"

#include "uti/sge_time.h"
#include "uti/sge_string.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_string.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_mailrec.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/cull_parse_util.h"
#include "sgeobj/sge_var.h"

#include "qmon_quarks.h"
#include "qmon_widgets.h"
#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_util.h"
#include "qmon_submit.h"
#include "qmon_message.h"
#include "qmon_init.h"

#include "parse_qsub.h"

#if 0
#include "Outline.h"
#include "Handle.h"
#endif

static void RegisterDerivedConstructor(String parent_name, String name, XmtWidgetConstructor constructor);


static Widget CreateInputField(Widget parent, String name, ArgList arglist, Cardinal argcount);
static void qmonShowCursor(Widget w, XtPointer cld, XtPointer cad);
static void qmonHideCursor(Widget w, XtPointer cld, XtPointer cad);

static Widget CreateListTree(Widget parent, String name, ArgList arglist, Cardinal argcount);


static void set_DurationInput(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void get_DurationInput(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void set_TimeInput(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void get_TimeInput(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void set_StringList(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void get_StringList(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void set_Spinbox(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void get_Spinbox(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void set_StrListField(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void get_StrListField(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void set_Ulong32Field(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void get_Ulong32Field(Widget w, XtPointer address, XrmQuark type, Cardinal size);
                           
static void set_label(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void set_LayoutString(Widget w, XtPointer address, XrmQuark type, Cardinal size);


static XmtWidgetType widgets[] = {
#if 0
   {
      "Outline",
      NULL,
      XmCreateOutline,
      NULL,
      NULL,
      False
   },
   {
      "Handle",
      NULL,
      XmCreateHandle,
      NULL,
      NULL,
      False
   },
#endif
   {
      "StringList",
      NULL,
      (XmtWidgetConstructor) XmCreateScrolledList,
      set_StringList,
      get_StringList,
      False
   },
   {
      "Spinbox",
      NULL,
      XmpCreateSpinbox,
      set_Spinbox,
      get_Spinbox,
      False
   },
   {
      "StrListField",
      NULL,
      (XmtWidgetConstructor) CreateInputField,
      set_StrListField,
      get_StrListField,
      False
   },
   {
      "Ulong32Field",
      NULL,
      (XmtWidgetConstructor) CreateInputField,
      set_Ulong32Field,
      get_Ulong32Field,
      False
   },
   {
      "Iconlist",
      NULL,
      (XmtWidgetConstructor) XmCreateScrolledIconList,
      NULL,
      NULL,
      False
   },
   {
      "ListTree",
      NULL,
      (XmtWidgetConstructor) CreateListTree,
      NULL,
      NULL,
      False
   },
   {
      "Folder",
      NULL,
      (XmtWidgetConstructor) XmCreateTabWidget,
      NULL,
      NULL,
      False
   },
   {
      "XmtLayoutString",
      NULL,
      (XmtWidgetConstructor) XmtCreateLayoutString,
      set_LayoutString,
      NULL,
      False
   },
   {
      "Label",
      NULL,
      XmCreateLabel,
      set_label,
      NULL,
      False
   },
   {
      "TimeInput",
      NULL,
      (XmtWidgetConstructor) CreateInputField,
      set_TimeInput,
      get_TimeInput,
      False
   },
   {
      "DurationInput",
      NULL,
      (XmtWidgetConstructor) CreateInputField,
      set_DurationInput,
      get_DurationInput,
      False
   }
};



/*-------------------------------------------------------------------------*/
void QmonRegisterWidgets(void)
{
   QmonInitQuarks();   /* initialize the quarks */
   RegisterDerivedConstructor("XmtInputField", "InputField",
                                 (XmtWidgetConstructor)CreateInputField);
   XmtRegisterWidgetTypes(widgets, XtNumber(widgets));
}
   
/*-------------------------------------------------------------------------*/
static void RegisterDerivedConstructor(
String parent_name, 
String name,
XmtWidgetConstructor constructor 
) {
   XmtWidgetType *parent_type;
   XmtWidgetType *type;

   parent_type = XmtLookupWidgetType(parent_name);

   if (!parent_type)
      return;

   type = XtNew(XmtWidgetType);  /* never free'd */
   type->name = name;
   type->wclass = NULL;
   type->constructor = constructor;
   type->set_value_proc = parent_type->set_value_proc;
   type->get_value_proc = parent_type->get_value_proc;
   type->popup = parent_type->popup;

   XmtRegisterWidgetTypes(type, 1);

}

/*-------------------------------------------------------------------------*/
static Widget CreateInputField(
Widget parent,
String name,
ArgList arglist,
Cardinal argcount 
) {
   Widget inputfield;
   Cardinal ac;
   Arg args[10];
   
   ac = 0;
   XtSetArg(args[ac], XmNcursorPositionVisible, False); ac++;
   
   inputfield = XmtCreateInputField(parent, name, args, ac);

   XtAddCallback(inputfield, XmNlosingFocusCallback,
                  qmonHideCursor, NULL);
   XtAddCallback(inputfield, XmNfocusCallback, 
                     qmonShowCursor, NULL);
   return inputfield;
}

/*-------------------------------------------------------------------------*/
static void qmonHideCursor(Widget w, XtPointer cld, XtPointer cad)
{
   XtVaSetValues(w, XmNcursorPositionVisible, False, NULL);
}

/*-------------------------------------------------------------------------*/
static void qmonShowCursor(Widget w, XtPointer cld, XtPointer cad)
{
   XtVaSetValues(w, XmNcursorPositionVisible, True, NULL);
}


/*-------------------------------------------------------------------------*/
static Widget CreateListTree(
Widget parent,
String name,
ArgList arglist,
Cardinal argcount 
) {
   Widget lw;
   Pixmap branch;
   Pixmap leaf;
   Pixmap branch_open;
   Pixmap leaf_open;
   Arg args[10]; 
   ArgList arg_list;
   int ac;

   branch = qmonGetIcon("branch");
   branch_open = qmonGetIcon("branch_open");
   leaf = qmonGetIcon("leaf");
   leaf_open = qmonGetIcon("leaf_open");

   ac = 0;
   XtSetArg(args[ac], XtNbranchPixmap, branch); ac++;
   XtSetArg(args[ac], XtNbranchOpenPixmap, branch_open); ac++;
   XtSetArg(args[ac], XtNleafPixmap, leaf); ac++;
   XtSetArg(args[ac], XtNleafOpenPixmap, leaf_open); ac++;
   XtSetArg(args[ac], XtNdoIncrementalHighlightCallback, True); ac++;
   XtSetArg(args[ac], XtNhighlightPath, False); ac++;
   XtSetArg(args[ac], XtNindent, 5); ac++;

   arg_list = XtMergeArgLists(arglist, argcount, args, ac);
   argcount += ac;
   
   lw = XmCreateScrolledListTree(parent, name, arg_list, argcount);

   return lw;
}

/*-------------------------------------------------------------------------*/
static void set_DurationInput(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   Cardinal time_val = 0;
/*    Cardinal day_val = 0; */
   Cardinal hour_val = 0;
   Cardinal minute_val = 0;
   Cardinal second_val = 0;
   char buffer[256];

   if (type != QmonQCardinal )  {
      XmtWarningMsg("XmtDialogSetDialogValues", "DurationInput",
         "Type Mismatch: Widget '%s':\n\tCan't set widget values"
         " from a resource of type '%s'",
          XtName(w), XrmQuarkToString(type));

      return;
   }

   if (size == sizeof(Cardinal))
       time_val = *(Cardinal*) address;
   else
      return;

   if (time_val == 0) {
      sprintf(buffer, "%02d:%02d:%02d", 0, 0, 0);
   } else {
/*       day_val = time_val/(24*3600); */
/*       time_val %= 24*3600; */
      hour_val = time_val/3600;
      time_val %= 3600;
      minute_val = time_val/60;
      second_val = time_val % 60;
      sprintf(buffer, "%02d:%02d:%02d", hour_val, minute_val, second_val);
   }
   
   XmtInputFieldSetString(w, buffer);
}

/*-------------------------------------------------------------------------*/
static void get_DurationInput(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   Cardinal value = 0;
   String str = NULL;
   
   if (type != QmonQCardinal ) { 
      XmtWarningMsg("XmtDialogGetDialogValues", "DurationInput",
         "Type Mismatch: Widget '%s':\n\tCan't get widget values"
         " from a resource of type '%s'",
          XtName(w), XrmQuarkToString(type));

      return;
   }
   str = XmtInputFieldGetString(w);

   if (str && str[0] != '\0') {
      u_long32 tmp_date_time;

      parse_ulong_val(NULL, &tmp_date_time, TYPE_TIM, str, NULL, 0);
      value = (Cardinal)tmp_date_time;
   }

   *(Cardinal*)address = value;
}

/*-------------------------------------------------------------------------*/
static void set_TimeInput(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   int value = 0;
   String str = NULL;
   dstring ds;
   char buffer[128];

   sge_dstring_init(&ds, buffer, sizeof(buffer));

   if (type != QmonQCardinal )  {
      XmtWarningMsg("XmtDialogSetDialogValues", "TimeInput",
         "Type Mismatch: Widget '%s':\n\tCan't set widget values"
         " from a resource of type '%s'",
          XtName(w), XrmQuarkToString(type));

      return;
   }

   if (size == sizeof(Cardinal))
       value = *(Cardinal*) address;
   else
      return;

   if (value != 0 && sge_at_time(value, &ds)!=NULL) 
      str = buffer;

   if (str)
      XmtInputFieldSetString(w, str);
   else
      XmtInputFieldSetString(w, "");
      
}

/*-------------------------------------------------------------------------*/
static void get_TimeInput(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   Cardinal value = 0;
   String str = NULL;
   
   if (type != QmonQCardinal ) { 
      XmtWarningMsg("XmtDialogGetDialogValues", "TimeInput",
         "Type Mismatch: Widget '%s':\n\tCan't get widget values"
         " from a resource of type '%s'",
          XtName(w), XrmQuarkToString(type));

      return;
   }
   str = XmtInputFieldGetString(w);

   if (str && str[0] != '\0') {
      u_long32 tmp_date_time;

      ulong_parse_date_time_from_string(&tmp_date_time, NULL, str);
      value = (Cardinal)tmp_date_time;
   }

   *(Cardinal*)address = value;
}

/*-------------------------------------------------------------------------*/
static void set_StringList(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   lList *lp = NULL;
   Widget parent;

   /*
    * Here special Qmon Quarks are used. Does it work ?
    */

   if (type != QmonQUS_Type && type != QmonQMR_Type && 
       type != QmonQPR_Type && type != QmonQSTR_Type && 
       type != QmonQSTU_Type && type != QmonQHR_Type) {
      XmtWarningMsg("XmtDialogSetDialogValues", "XbaeMatrix",
         "Type Mismatch: Widget '%s':\n\tCan't set widget values"
         " from a resource of type '%s'",
          XtName(w), XrmQuarkToString(type));

      return;
   }

   if (size == sizeof(lList*))
      lp = *(lList**) address;
   else
      return;


   /* if our parent is a layout widget disable/reenable it */
   parent = XtParent(w);
   while (parent && !XtIsSubclass(parent, xmtLayoutWidgetClass))
      parent = XtParent(parent);

   XmtLayoutDisableLayout(parent);
   if (type == QmonQUS_Type) {
      UpdateXmListFromCull(w, XmFONTLIST_DEFAULT_TAG, lp, US_name);
   }
   
   if (type == QmonQPR_Type) {
      UpdateXmListFromCull(w, XmFONTLIST_DEFAULT_TAG, lp, PR_name);
   }
   
   if (type == QmonQHR_Type) {
      UpdateXmListFromCull(w, XmFONTLIST_DEFAULT_TAG, lp, HR_name);
   }
   
   if (type == QmonQMR_Type) {
      String *str_table = NULL;
      Cardinal itemCount;
      StringConst str1, str2;
      int size, i;
      char buf[BUFSIZ];
      lListElem *ep = NULL;
      
      XtVaGetValues(w, XmNitemCount, &itemCount, NULL);

      if (!lp) {
         if (itemCount)
            XmListDeleteAllItems(w);
         return;
      }

      if ((size = lGetNumberOfElem(lp)) <=0) {
         return;
      }
      
      if (!(str_table = (String *)XtMalloc(sizeof(String)*size))) {
         return;
      }

      for (ep=lFirst(lp), i=0; ep; ep=lNext(ep), i++) {
         str1 = (StringConst)lGetString(ep, MR_user);
         str2 = (StringConst)lGetHost(ep, MR_host);
         if (str1) {
            if (!str2)
               sge_strlcpy(buf, str1, BUFSIZ);
            else
               snprintf(buf, BUFSIZ, "%s@%s", str1, str2);
            str_table[i] = XtNewString(buf);
         }
         else
            str_table[i] = NULL;
      }
      UpdateXmList(w, str_table, size, XmFONTLIST_DEFAULT_TAG);
      StringTableFree(str_table, size);
   }   
   
   if (type == QmonQSTR_Type) {
      UpdateXmListFromCull(w, XmFONTLIST_DEFAULT_TAG, lp, ST_name);
   }

   if (type == QmonQSTU_Type) {
      UpdateXmListFromCull(w, XmFONTLIST_DEFAULT_TAG, lp, STU_name);
   }
      
   XmtLayoutEnableLayout(parent);

}

/*-------------------------------------------------------------------------*/
static void get_StringList(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   lList *lp = NULL;
   
   /*
    * Here special Qmon Quarks are used. Does it work ?
    */

   if (type != QmonQUS_Type && type != QmonQMR_Type && 
       type != QmonQPR_Type && type != QmonQSTR_Type && 
       type != QmonQSTU_Type && type != QmonQHR_Type) {
      XmtWarningMsg("XmtDialogSetDialogValues", "XbaeMatrix",
         "Type Mismatch: Widget '%s':\n\tCan't get widget values"
         " from a resource of type '%s'",
          XtName(w), XrmQuarkToString(type));
      return;
   }

   if (type == QmonQUS_Type) {
      lp = XmStringToCull(w, US_Type, US_name, ALL_ITEMS);
   }
         
   if (type == QmonQPR_Type) {
      lp = XmStringToCull(w, PR_Type, PR_name, ALL_ITEMS);
   }
         
   if (type == QmonQMR_Type) {
      lp = XmStringToCull(w, MR_Type, MR_user, ALL_ITEMS);
   }

   if (type == QmonQSTR_Type) {
      lp = XmStringToCull(w, ST_Type, ST_name, ALL_ITEMS);
   }

   if (type == QmonQSTU_Type) {
      lp = XmStringToCull(w, STU_Type, STU_name, ALL_ITEMS);
   }

   if (type == QmonQHR_Type) {
      lp = XmStringToCull(w, HR_Type, HR_name, ALL_ITEMS);
   }

   *(lList**)address = lp;
}

/*-------------------------------------------------------------------------*/
static void set_Spinbox(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   int value;
   
   /*
    * Here special Qmon Quarks are used. Does it work ?
    */

   if (type != QmonQInt) {
      XmtWarningMsg("XmtDialogSetDialogValues", "Spinbox",
         "Type Mismatch: Widget '%s':\n\tCan't set widget values"
         " from a resource of type '%s'; QmonQInt expected.",
          XtName(w), XrmQuarkToString(type));

      return;
   }

   if (size == sizeof(int))
      value = *(int *) address;
   else
      return;

   XmpSpinboxSetValue(w, (long)value, True);

}

/*-------------------------------------------------------------------------*/
static void get_Spinbox(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   int value;   

   if (type != QmonQInt) {
      XmtWarningMsg("XmtDialogSetDialogValues", "Spinbox",
         "Type Mismatch: Widget '%s':\n\tCan't get widget values"
         " from a resource of type '%s'; QmonQInt expected.",
          XtName(w), XrmQuarkToString(type));
      return;
   }

   value = XmpSpinboxGetValue(w);

   *(int*)address = value;
}

/*-------------------------------------------------------------------------*/
static void get_Ulong32Field(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   static u_long32 uv = 0;
   unsigned long lv = 0;
   String s = NULL;
    
   if (type != QmonQUlong32) {
      XmtWarningMsg("XmtDialogGetDialogValues", "Ulong32Field",
         "Type Mismatch: Widget '%s':\n\tCan't get widget values"
         " from a resource of type '%s'.",
          XtName(w), XrmQuarkToString(type));

   }

   s = XmtInputFieldGetString(w);

   if (s && s[0] != '\0') {
      lv = strtoul(s, NULL, 10);
   } else {
     lv = 0;
   }  

   if (lv > U_LONG32_MAX) {
      uv = U_LONG32_MAX;
   } else {
      uv = (u_long32)lv;
   }   

   if (type == QmonQUlong32) {
      *(u_long32 *)address = uv;
   }
}

/*-------------------------------------------------------------------------*/
static void set_Ulong32Field(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   static char buf[BUFSIZ];

   if (type != QmonQUlong32) {
      XmtWarningMsg("XmtDialogSetDialogValues", "Ulong32Field",
         "Type Mismatch: Widget '%s':\n\tCan't set widget values"
         " from a resource of type '%s'.",
          XtName(w), XrmQuarkToString(type));

   }

	(void)sprintf(buf, "%u", *(unsigned int *)address);
	XmtInputFieldSetString(w, buf);
}

/*-------------------------------------------------------------------------*/
static void set_StrListField(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   lList *list = NULL;
   String str = NULL;
   static char buf[100 * BUFSIZ];
   lListElem *ep = NULL;

   /*
    * Here special Qmon Quarks are used. Does it work ?
    */

   if (type != QmonQENV_Type && type != QmonQST_Type &&
       type != QmonQRN_Type && type != QmonQTRN_Type && type != QmonQPN_Type &&
       type != QmonQMR_Type && type != QmonQQR_Type &&
       type != QmonQJRE_Type && type != QmonQCTX_Type &&
       type != QmonQPE_Type && type != QmonQARA_Type) {
      XmtWarningMsg("XmtDialogSetDialogValues", "StrListField",
         "Type Mismatch: Widget '%s':\n\tCan't set widget values"
         " from a resource of type '%s'.",
          XtName(w), XrmQuarkToString(type));

      return;
   }

   buf[0] = '\0';

   if (size == sizeof(lList*) || size == sizeof(String)) {
      if (type == QmonQPE_Type)
         str = *(String*) address;
      else
         list = *(lList**) address;
   }
   else
      return;

   if (type == QmonQENV_Type || type == QmonQCTX_Type) {
      str = write_pair_list(list, VARIABLE_TYPE);
   } 
   if (type == QmonQST_Type) {
      int first_time = 1;
      strcpy(buf, "");
      for_each(ep, list) {
         if (first_time) {
            strcpy(buf, lGetString(ep, ST_name));
            first_time = 0;
         }
         else
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " %s",
                     lGetString(ep, ST_name));
      }
      str = buf;
   }
   if (type == QmonQRN_Type) {
      dstring range_string = DSTRING_INIT;

      range_list_print_to_string(list, &range_string, true, false, false);
      strcpy(buf, sge_dstring_get_string(&range_string));
      sge_dstring_free(&range_string);
      str = buf;
   }
   if (type == QmonQTRN_Type) {
      dstring range_string = DSTRING_INIT;

      range_list_print_to_string(list, &range_string, false, false, false);
      strcpy(buf, sge_dstring_get_string(&range_string));
      sge_dstring_free(&range_string);
      str = buf;
   }
   if (type == QmonQPN_Type) {
      str = write_pair_list(list, PATH_TYPE);
   }
   if (type == QmonQMR_Type) {
      str = write_pair_list(list, MAIL_TYPE);
   }
   if (type == QmonQQR_Type) {
      int first_time = 1;
      strcpy(buf, "");
      for_each(ep, list) {
         if (first_time) {
            first_time = 0;
            strcpy(buf, lGetString(ep, QR_name));
         }
         else
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), ",%s",
                     lGetString(ep, QR_name));
      }
      str = buf;
   }
   if (type == QmonQJRE_Type) {
      int first_time = 1;
      strcpy(buf, "");
      for_each(ep, list) {
         if (first_time) {
            first_time = 0;
            if (lGetString(ep, JRE_job_name))
               sge_strlcpy(buf, lGetString(ep, JRE_job_name), sizeof(buf));
            else
               sprintf(buf, sge_u32, lGetUlong(ep, JRE_job_number));
         } else {
            if (lGetString(ep, JRE_job_name))
               snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " %s",
                        lGetString(ep, JRE_job_name));
            else  
               snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " "sge_u32,
                        lGetUlong(ep, JRE_job_number));
         }   
      }
      str = buf;
   }
   if (type == QmonQARA_Type) {
      int first_time = 1;
      strcpy(buf, "");
      for_each(ep, list) {
         if (first_time) {
            first_time = 0;
            sge_strlcpy(buf, lGetString(ep, ARA_name), sizeof(buf));
         }
         else {
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " %s",
                     lGetString(ep, ARA_name));
         }   
      }
      str = buf;
   }

   XmtInputFieldSetString(w, str ? str: "");
   
}

/*-------------------------------------------------------------------------*/
static void get_StrListField(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   lList *ret_list = NULL;  /* NULL important ! */
   static String str = NULL;
   lList *alp = NULL;

   if (type != QmonQENV_Type && type != QmonQST_Type &&
       type != QmonQRN_Type && type != QmonQTRN_Type && type != QmonQPN_Type &&
       type != QmonQMR_Type && type != QmonQQR_Type &&
       type != QmonQJRE_Type && type != QmonQCTX_Type &&
       type != QmonQPE_Type && type != QmonQARA_Type) {
      XmtWarningMsg("XmtDialogSetDialogValues", "StrListField",
         "Type Mismatch: Widget '%s':\n\tCan't get widget values"
         " from a resource of type '%s'.", XtName(w), XrmQuarkToString(type));
      return;
   }

   str = XmtInputFieldGetString(w);

   if (str && str[0] != '\0') {
      if (type == QmonQENV_Type || type == QmonQCTX_Type) {
         var_list_parse_from_string(&ret_list, str, 0); 
      }
      if (type == QmonQST_Type) {
         str_list_parse_from_string(&ret_list, str, " ");
      }
      if (type == QmonQRN_Type) {
         range_list_parse_from_string(&ret_list, &alp, str,
                                      0, 0, INF_ALLOWED);
         if (alp) {
            qmonMessageShow(w, True, "%s",
                            (StringConst)lGetString(lFirst(alp), AN_text));
            lFreeList(&alp);
         }
      }
      if (type == QmonQTRN_Type) {
         range_list_parse_from_string(&ret_list, &alp, str,
                                      0, 1, INF_NOT_ALLOWED);
         if (alp) {
            qmonMessageShow(w, True, "%s",
                            (StringConst)lGetString(lFirst(alp), AN_text));
            lFreeList(&alp);
         }
      }
      if (type == QmonQPN_Type) {
         cull_parse_path_list(&ret_list, str);
      }
      if (type == QmonQMR_Type) {
         mailrec_parse(&ret_list, str);
      }
      if (type == QmonQQR_Type) {
         lString2List(str, &ret_list, QR_Type, QR_name, ",");
      }
      if (type == QmonQJRE_Type) {
         lList *sl = NULL;
         lListElem *ep = NULL;
         cull_parse_jid_hold_list(&sl, str); 
         for_each (ep, sl) {
            lAddElemStr(&ret_list, JRE_job_name, lGetString(ep, ST_name), JRE_Type);
         }
         lFreeList(&sl);
      }
      if (type == QmonQARA_Type) {
         get_ara_list(str, &ret_list);
      }   
   }
   
   if (type == QmonQPE_Type) {
      *(String*)address = str;
   } else {
      if (*(lList**)address != NULL) {
         lFreeList((lList**)address);
      }
      *(lList**)address = ret_list;
   }   
}

/*-------------------------------------------------------------------------*/
String write_pair_list(
lList *lp,
int type 
) {
   lListElem *ep;
   static char pair_string[ 10 * BUFSIZ];
   int nm1 = 0, nm2 = 0;
   StringConst field1, field2;
   char delimitor[10];
   char comma[10];
   int set_comma = 0;
   
   DENTER(GUI_LAYER, "write_pair_list");

   switch (type) {
      case MAIL_TYPE:
         nm1 = MR_user;
         nm2 = MR_host;
         strcpy(delimitor, "@"); 
         strcpy(comma, ","); 
         break;

      case PATH_TYPE:
         nm1 = PN_host;
         nm2 = PN_path;
         strcpy(delimitor, ":"); 
         strcpy(comma, ","); 
         break;

      case VARIABLE_TYPE:
         nm1 = VA_variable;
         nm2 = VA_value;
         strcpy(delimitor, "="); 
         strcpy(comma, ","); 
         break;

      default:
         DEXIT;
         return NULL;
   }

   strcpy(pair_string, "");

   for_each(ep, lp) {
      if (!set_comma)
         set_comma = 1;
      else
         strcat(pair_string, comma);

      if (nm1 == PN_host ) {
         field1 = (StringConst)lGetHost(ep, nm1);
      } else {
         field1 = (StringConst)lGetString(ep, nm1);
      }

      if (nm2 == MR_host) {
         field2 = (StringConst)lGetHost(ep, nm2);
      } else {
         field2 = (StringConst)lGetString(ep, nm2);
      }
      
      if (field1 && field1[0] != '\0') {
         strcat(pair_string, field1);
         if (field2 && field2[0] != '\0')
            strcat(pair_string, delimitor);
      }
      if (field2) {
         strcat(pair_string, field2);
      }
   }

   DEXIT;
   return pair_string;
}

/*-------------------------------------------------------------------------*/
static void set_LayoutString(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   char buf[128];

   DENTER(GUI_LAYER, "set_LayoutString");

   if (type != XmtQString && type != XmtQBuffer && type != XmtQShort && 
       type != XmtQInt && type != XmtQCardinal && type != XmtQFloat && 
       type != XmtQDouble) {
      XmtWarningMsg("XmtDialogSetDialogValues", "XmtLayoutString",
         "Type Mismatch: Widget '%s':\n\tCan't get widget values"
         " from a resource of type '%s'",
          XtName(w), XrmQuarkToString(type));
      return;
   }

   if (type == XmtQString) {
      XtVaSetValues(w, XmtNlabel, *(String*)address, NULL);
   }
   if (type == XmtQBuffer) {
      XtVaSetValues(w, XmtNlabel, (char *)address, NULL);
   }
   if (type == XmtQShort) {
      sprintf(buf, "%d", *(short *)address);
      XtVaSetValues(w, XmtNlabel, buf, NULL);
   }
   if (type == XmtQInt) {
      sprintf(buf, "%d", *(int *)address);
      XtVaSetValues(w, XmtNlabel, buf, NULL);
   }
   if (type == XmtQCardinal) {
      sprintf(buf, "%u", *(unsigned int *)address);
      XtVaSetValues(w, XmtNlabel, buf, NULL);
   }
   if (type == XmtQFloat) {
      sprintf(buf, "%g", *(float *)address);
      XtVaSetValues(w, XmtNlabel, buf, NULL);
   }
   if (type == XmtQDouble) {
      sprintf(buf, "%g", *(double *)address);
      XtVaSetValues(w, XmtNlabel, buf, NULL);
   }
         
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void set_label(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   char buf[128];
   XmString str = NULL;

   DENTER(GUI_LAYER, "set_label");

   if (type != XmtQString && type != XmtQBuffer && type != XmtQShort && 
       type != XmtQInt && type != XmtQCardinal && type != XmtQFloat && 
       type != XmtQDouble && type != QmonQUlong32) {
      XmtWarningMsg("XmtDialogSetDialogValues", "Label",
         "Type Mismatch: Widget '%s':\n\tCan't get widget values"
         " from a resource of type '%s'",
          XtName(w), XrmQuarkToString(type));
      return;
   }

   if (type == XmtQString) {
      str = XmtCreateXmString(*(String*)address);
   }
   if (type == XmtQBuffer) {
      str = XmtCreateXmString((char*)address);
   }
   if (type == XmtQShort) {
      sprintf(buf, "%d", *(short *)address);
      str = XmtCreateXmString(buf);
   }
   if (type == XmtQInt) {
      sprintf(buf, "%d", *(int *)address);
      str = XmtCreateXmString(buf);
   }
   if (type == XmtQCardinal || type == QmonQUlong32) {
      sprintf(buf, "%u", *(unsigned int *)address);
      str = XmtCreateXmString(buf);
   }
   if (type == XmtQFloat) {
      sprintf(buf, "%g", *(float *)address);
      str = XmtCreateXmString(buf);
   }
   if (type == XmtQDouble) {
      sprintf(buf, "%g", *(double *)address);
      str = XmtCreateXmString(buf);
   }

   XtVaSetValues(w, XmNlabelString, str, NULL);
   XmStringFree(str);
         
   DEXIT;
}

void get_ara_list(const char *str, lList **ret_list) {
   if (str == NULL || str[0] == '\0') {
      lFreeList(ret_list);
   } else {   
      int rule[] = {ARA_name, 0};
      char *tmp = sge_strdup(NULL, str);
      char **dest = string_list(tmp, ",", NULL);
      lFreeList(ret_list);
      cull_parse_string_list(dest, "user_list", ARA_Type, rule, ret_list); 
      sge_free(&tmp);
      sge_free(&dest);
   }
}
