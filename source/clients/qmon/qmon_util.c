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
#include <stdlib.h>
#include <ctype.h>

#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/Scale.h>
#include <Xm/ToggleB.h>

#include <Xmt/Xmt.h>
#include <Xmt/Layout.h>

#include "qmon_util.h"
#include "qmon_cull.h"
#include "qmon_rmon.h"


#if XmVersion < 1002

/*-------------------------------------------------------------------------*/
void XmListDeletePositions(Widget list, int *pos_list, int pos_count)
{
   int i;

   for (i=0; i<pos_count; i++) {
      XmListDeleteItemsPos(list, 1, pos_list[i] - i);
   }
}

#endif

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
String* XmStringTableToStringTable(XmStringTable items, Cardinal itemCount, 
                                    XmStringCharSet tag)
{
   Cardinal i;
   String* str_table = NULL;

   DENTER(GUI_LAYER, "XmStringTableToStringTable");
   
   if (!itemCount || !items) {
      DPRINTF(("itemCount/items is 0/NULL\n"));
      DEXIT;
      return NULL;
   }
   
   str_table = (String*) XtMalloc(sizeof(String) * itemCount);

   if (!str_table) {
      DPRINTF(("malloc failure\n"));
      DEXIT;
      return NULL;
   }

   for (i=0; i<itemCount; i++) {
      if (!XmStringGetLtoR(items[i], tag ? tag : XmFONTLIST_DEFAULT_TAG, 
                     str_table + i) ) {
         DPRINTF(("XmStringGetLtoR failure\n"));
         StringTableFree(str_table, i);
         DEXIT;
         return NULL;
      }
   }

   DEXIT;
   return str_table;
}

/*-------------------------------------------------------------------------*/
XmString *StringTableToXmStringTable(String *items, Cardinal itemCount,
                                       XmStringCharSet tag)
{
   XmString *xstr_table;
   Cardinal i;
   
   DENTER(GUI_LAYER, "StringTableToXmStringTable");

   if (!itemCount || !items) {
      DPRINTF(("itemCount/items is 0/NULL\n"));
      DEXIT;
      return NULL;
   }
   
   xstr_table = (XmString*) XtMalloc(sizeof(XmString) * itemCount);

   if (!xstr_table) {
      DPRINTF(("malloc failure\n"));
      DEXIT;
      return NULL;
   }

   for (i=0; i<itemCount; i++) {
      xstr_table[i] = XmStringCreateLtoR( items[i], 
                         tag ? tag : XmFONTLIST_DEFAULT_TAG); 
   }

   DEXIT;
   return xstr_table;
}
   
/*-------------------------------------------------------------------------*/
void StringTableFree( String *str_table, Cardinal str_table_size)
{
   int i;
   
   DENTER(GUI_LAYER, "StringTableFree");

   for (i=0; i<str_table_size; i++)
      XtFree((char *) str_table[i]);

   XtFree((char*)str_table);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void XmStringTableFree(XmString *xstr_table, Cardinal xstr_table_size)
{
   int i;
   
   DENTER(GUI_LAYER, "XmStringTableFree");

   for (i=0; i<xstr_table_size; i++)
      if (xstr_table[i])
         XmStringFree(xstr_table[i]);

   XtFree((char*)xstr_table);

   DEXIT;
}



/*-------------------------------------------------------------------------*/
Boolean AddStringToXmList(Widget list, String str, XmStringCharSet tag, 
                           int list_pos)
{
   XmString item;
   Boolean ret = False;
   
   DENTER(GUI_LAYER, "AddStringToXmList");

   item = XmStringCreateLtoR(str, tag);
   if ( ! XmListItemExists(list, item)) {
      XmListAddItem(list, item, list_pos);
      ret = True;
   }
   
   XmStringFree(item);
 
   DEXIT;
   return ret;
}
   
/*-------------------------------------------------------------------------*/
void UpdateXmList(Widget list, String *str_table, Cardinal size,
                     XmStringCharSet tag)
{
   XmString *items = NULL;
   
   DENTER(GUI_LAYER, "UpdateXmList");

   /*
   ** replace the old items
   */
   items = StringTableToXmStringTable(str_table, size, tag);

   XtVaSetValues( list,
                  XmNitems, items,
                  XmNitemCount, size,
                  NULL);

   XmStringTableFree(items, size);

#if 0
   XtVaGetValues( list,
                  XmNitems, &items,
                  XmNitemCount, &itemCount,
                  NULL);

   DPRINTF(("UpdateXmList: itemCount = %d\n", itemCount));

   for (i=0; i<size; i++) {
      /* strings are equal */
      if (i<itemCount) {
         DPRINTF(("in_str = '%s'\n", str_table[i]));
DTRACE;
         if (items[i] && XmStringGetLtoR(items[i], tag, &str1)) { 
            DPRINTF(("str1='%s'\n", str1));
DTRACE;
            if (!strcmp(str1, str_table[i])) {
DTRACE;
               XtFree((char *)str1);
               str1 = NULL;
DTRACE;
               continue;
            }
DTRACE;
            XtFree((char *)str1);
            str1 = NULL;
DTRACE;
         }
DTRACE;
         XmListDeletePos(list, i+1);   /* first element is 1 */
      }
DTRACE;
      new = XmStringCreateLtoR(str_table[i], tag);
DTRACE;
      XmListAddItem(list, new, i+1);
DTRACE;
      XmStringFree(new);
   }

   /* size < itemCount -> delete the rest */
DTRACE;
   if (i<itemCount)
      XmListDeleteItemsPos(list, itemCount-size, i+1);

DTRACE;
/*
   for (;i<itemCount;i++) {   
      XmListDeletePos(list, i+1);
      itemCount--;
   }
*/
#endif
   DEXIT;
}


/*-------------------------------------------------------------------------*/
Boolean UpdateXmListFromCull(Widget list, XmStringCharSet tag, lList *lp, 
                              int field)
{
   lListElem *ep;
   Cardinal itemCount;
   StringConst str;
   int pos;
   int dataType;
   const lDescr *listDescriptor = NULL;
   Widget lw;
   Boolean found = False;
   
   DENTER(GUI_LAYER, "UpdateXmListFromCull");
   XtVaGetValues(list, XmNitemCount, &itemCount, NULL);

   if (itemCount)
      XmListDeleteAllItems(list);

   if (lGetNumberOfElem(lp) <= 0) {
      DEXIT;
      return False;
   }

   for (lw = XtParent(list); lw; lw = XtParent(lw)) {
      if (XtIsSubclass(lw, xmtLayoutWidgetClass)) {
         found = True;
         break;
      }  
   }
   
   if (found && lw)
      XmtLayoutDisableLayout(lw);
   
   listDescriptor = lGetListDescr(lp);
   pos = lGetPosInDescr(listDescriptor, field); 
   dataType = lGetPosType(listDescriptor,pos);
   for (ep=lFirst(lp); ep; ep=lNext(ep)) {
      switch (dataType) {
         case lStringT:
              str = lGetPosString(ep, pos);
           break;
         case lHostT:
              str = lGetPosHost(ep, pos);
           break;
         default:
              str = "(null) UpdateXmListFromCull";
              DPRINTF(("UpdateXmListFromCull: unexpected data type\n"));
           break; 
      }
      /* FIX_CONST_GUI */
      XmListAddItemUniqueSorted(list, (String)str);
      DPRINTF(("UpdateXmListFromCull: str = '%s'\n", str));
   }   

   if (found && lw)
      XmtLayoutEnableLayout(lw);
      
   DEXIT;
   return True;
}

/*-------------------------------------------------------------------------*/
lList *XmStringToCull(Widget list, lDescr *dp, int nm, int selected)
{
   XmString *items;
   Cardinal itemCount;
   String *strings;
   lList *lp = NULL;
   int i;
   int dataType;
   
   DENTER(GUI_LAYER, "XmStringToCull");

   if (selected == ALL_ITEMS)
      XtVaGetValues( list,
                  XmNitems, &items,
                  XmNitemCount, &itemCount,
                  NULL);
   else
      XtVaGetValues( list,
                  XmNselectedItems, &items,
                  XmNselectedItemCount, &itemCount,
                  NULL);
   
   if (itemCount) {
      strings = XmStringTableToStringTable(items, itemCount, 
                                          XmFONTLIST_DEFAULT_TAG);
      if (strings) { 
         for (i=0; i<itemCount; i++) {
            dataType = lGetPosType(dp, lGetPosInDescr(dp, nm));
            switch (dataType) {
               case lStringT:
                  lAddElemStr(&lp, nm, strings[i], dp);
                  break;
               case lHostT:
                  lAddElemHost(&lp, nm, strings[i], dp);
                  break;
               default:
                  DPRINTF(("XmStringToCull - unexpected data type\n"));
                  break;
            }
         }
         StringTableFree(strings, itemCount);
      }
   }

   DEXIT;
   return lp;
}
   


   
/*-------------------------------------------------------------------------*/
Boolean DelStringFromXmList(Widget list, String str, XmStringCharSet tag,
                              int list_pos)
{
   XmString item;
   
   DENTER(GUI_LAYER, "DelStringFromXmList");
   
   if (!list_pos && !str) {
      DEXIT;
      return False;
   }

   if (list_pos) {
      XmListDeletePos(list, list_pos);
   }
   else {
      item = XmStringCreateLtoR(str, tag);
      XmListDeleteItem(list, item);
   }

   DEXIT;
   return True;
}
   

/*-------------------------------------------------------------------------*/
void XmListAddTextItem(Widget list_w, String item, XmStringCharSet tag,
                        int position)
{
   XmString str;

   str = XmStringCreateLtoR(item, tag);

   XmListAddItem(list_w, str, position);

   XmStringFree(str);
}

/*-------------------------------------------------------------------------*/
void XmListAddTextItems(Widget list_w, String *items, Cardinal itemCount,  
                        XmStringCharSet tag, int position)
{
   XmString *str;

   str = StringTableToXmStringTable(items, itemCount, tag);

   XmListAddItems(list_w, str, itemCount, position);

   XmStringTableFree(str, itemCount);
}

#ifdef _MOTIF_12_
/*-------------------------------------------------------------------------*/
void XmListAddTextItemUnselected(Widget list_w, String item, 
                                 XmStringCharSet tag, int position)
{
   XmString str;


   str = XmStringCreateLtoR(item, tag);

   XmListAddItemUnselected(list_w, str, position);

   XmStringFree(str);
}

/*-------------------------------------------------------------------------*/
void XmListAddTextItemsUnselected(Widget list_w, String *items, 
                     Cardinal itemCount, XmStringCharSet tag, int position)
{
   XmString *str;

   str = StringTableToXmStringTable(items, itemCount, tag);

   XmListAddItemsUnselected(list_w, str, itemCount, position);

   XmStringTableFree(str, itemCount);
}
#endif

/*-------------------------------------------------------------------------*/
void XmListSelectTextItem(Widget list_w, String item, XmStringCharSet tag, 
                           Boolean notify)
{
   XmString str;

   str = XmStringCreateLtoR(item, tag);
   
   XmListSelectItem(list_w, str, notify);

   XmStringFree(str);
}

/*-------------------------------------------------------------------------*/
void qmonPositionDialog(Widget dialog, XtPointer cld, XtPointer cad)
{
   static int x, y;
   int w, h, ws, hs;

   DENTER(GUI_LAYER, "qmonPositionDialog");

   XtVaGetValues(dialog, XmNwidth, &w, XmNheight, &h, NULL);

   ws = WidthOfScreen(XtScreen(dialog));
   hs = HeightOfScreen(XtScreen(dialog));
   if ((x + w) >= ws)
      x = 0;
   else
      x = (ws - w) / 2;

   if ((y + h) >= hs)
      y = 0;
   else
      y = (hs - h) / 2;

   XtVaSetValues(dialog, XmNx, x, XmNy, y, NULL);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void SetMinShellSize(Widget shell, XtPointer cld, XEvent *event, Boolean *ctd)
{
   XConfigureEvent *cevent = (XConfigureEvent*)event;

   DENTER(GUI_LAYER, "SetMinShellSize");

   if (cevent->type != ConfigureNotify) {
      DEXIT;
      return;
   }

   if (!cld) {
      XtVaSetValues( shell,
                  XmNminWidth, cevent->width,
                  XmNminHeight, cevent->height,
                  NULL);
   }
   else if ((long)cld == SHELL_WIDTH) {
            XtVaSetValues( shell,
                           XmNminWidth, cevent->width,
                           NULL);
   }
   else if ((long)cld == SHELL_HEIGHT) {
      XtVaSetValues( shell,
                  XmNminHeight, cevent->height,
                  NULL);
   } 

   XtRemoveEventHandler(shell, StructureNotifyMask, False, 
                           SetMinShellSize, NULL);
   
   DEXIT;
}
                  
/*-------------------------------------------------------------------------*/
void SetMaxShellSize(Widget shell, XtPointer cld, XEvent *event, Boolean *ctd)
{
   XConfigureEvent *cevent = (XConfigureEvent*)event;

   DENTER(GUI_LAYER, "SetMinShellSize");

   if (cevent->type != ConfigureNotify) {
      DEXIT;
      return;
   }

   if (!cld) {
      XtVaSetValues( shell,
                  XmNmaxWidth, cevent->width,
                  XmNmaxHeight, cevent->height,
                  NULL);
   }
   else if ((long)cld == SHELL_WIDTH) {
            XtVaSetValues( shell,
                           XmNmaxWidth, cevent->width,
                           NULL);
   }
   else if ((long)cld == SHELL_HEIGHT) {
      XtVaSetValues( shell,
                  XmNmaxHeight, cevent->height,
                  NULL);
   } 

   XtRemoveEventHandler(shell, StructureNotifyMask, False, 
                           SetMaxShellSize, NULL);
   
   DEXIT;
}
                  

/*-------------------------------------------------------------------------*/
int qmonForkEditor(String file)
{
   char *cp;
   char *editor = NULL;
   char command[BUFSIZ];
   static char default_editor[] = "vi";
   int status;
   
   DENTER(GUI_LAYER, "qmonForkEditor");

   if (!(editor = getenv("EDITOR")))
      editor = default_editor;

   cp = strrchr(editor, '/'); 
   if (cp && !strcmp(cp + 1, "jot"))
      strcpy(command, editor);
   else
      sprintf(command, "xterm -e %s", editor);

   if (file)
      snprintf(command + strlen(command), sizeof(command) - strlen(command),
               " %s &", file ? file : "");
   else
      strcat(command, "&");

   status = system(command);
   
   DEXIT;
   return status;
}


/*-------------------------------------------------------------------------*/
void qmonXmStringDrawImage(Display *dpy, Drawable da, GC gc, Position x, 
                           Position y, XmFontList fontlist, XmString str,
                           unsigned char layout, XRectangle *clip,
                           Dimension *width, Dimension *height)
{
   DENTER(GUI_LAYER, "qmonDrawRequestEntry");
   
   /* GET THE BOUNDING BOX */
   XmStringExtent(fontlist, str, width, height);

   /* DRAW RECTANGLE AND LABEL */
   XmStringDrawImage(dpy, da, fontlist, str, gc, x, y, *width,  
                  layout, XmSTRING_DIRECTION_L_TO_R, 
                  clip);
   DEXIT;
}


/*-------------------------------------------------------------------------*/
void qmonXmStringDraw(Display *dpy, Drawable da, GC gc, Position x, Position y,
                      XmFontList fontlist, XmString str, unsigned char layout,
                      XRectangle *clip, Dimension *width, Dimension *height)
{
   DENTER(GUI_LAYER, "qmonXmStringDraw");
   
   /* GET THE BOUNDING BOX */
   XmStringExtent(fontlist, str, width, height);

   /* DRAW RECTANGLE AND LABEL */
   XmStringDraw(dpy, da, fontlist, str, gc, x, y, *width,  
                  layout, XmSTRING_DIRECTION_L_TO_R, 
                  clip);
   DEXIT;
}



/*-------------------------------------------------------------------------*/
void setButtonLabel(Widget w, String label)
{
   XmString xlab;

   DENTER(GUI_LAYER, "setButtonLabel");

   xlab = XmtCreateLocalizedXmString(w, label);
   XtVaSetValues(w, XmNlabelString, xlab, NULL);
   XmStringFree(xlab);

   DEXIT;
}
   
   
/*-------------------------------------------------------------------------*/
void qmonScaleWeightToggle(Widget w, XtPointer cld, XtPointer cad)
{
   XmToggleButtonCallbackStruct *cbs = (XmToggleButtonCallbackStruct*)cad;
   tScaleWeight *weights = (tScaleWeight*) cld;
   int index;
   int *ip;

   DENTER(GUI_LAYER, "qmonScaleWeightToggle");
   
   XtVaGetValues(w, XmNuserData, &ip, NULL);
   index = *ip;
   DPRINTF(("index = %d\n", index));

   if (cbs->set) {
      weights[index].lock = 1;    
      XtSetSensitive(weights[index].scale, False);
   }
   else {
      weights[index].lock = 0;    
      XtSetSensitive(weights[index].scale, True);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void qmonScaleWeight(Widget w, XtPointer cld, XtPointer cad)
{
   XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct*)cad;
   tScaleWeight *weights = (tScaleWeight*) cld;
   int delta;
   int *ip;
   int index;
   int count = 0;
   int i, j;
   int max_value = 0;
   int delta_part;
   int sum = 0;

   DENTER(GUI_LAYER, "qmonScaleWeight");

   XtVaGetValues(w, XmNuserData, &ip, NULL);
   index = *ip;
   DPRINTF(("index = %d\n", index));

   if (cbs->value != weights[index].weight) {

      /*
      ** get the max_value and the number of non-locked scales 
      */
      for (i=0; i<weights[0].nr; i++) {
         if (!weights[i].lock) {
            max_value += weights[i].weight; 
            if (i!= index)
               count++;
         }
      }

      if (count) {
         /*
         ** check max value
         */
         if (cbs->value > max_value)
            weights[index].weight = max_value;
         else
            weights[index].weight = cbs->value;
         
         max_value -= weights[index].weight; 
          
         if (!max_value) {
            /*
            ** set the remaining non-locked scales to zero
            */
            for (i=0; i<weights[0].nr; i++) {
               if (!weights[i].lock && i != index) {
                  weights[i].weight = 0;
               }
            }
         }
         else {
            /*
            ** calculate the delta
            ** subtract the old weights
            ** so delta might be positive or negative
            */
            delta = max_value;
            for (i=0; i<weights[0].nr; i++) {
               if (!weights[i].lock && i != index) {
                  delta -= weights[i].weight;
               }
            }

            /*
            ** divide by number of unlocked scales to have a starting point
            */
            delta_part = delta/count;

            /*
            ** set the new values
            */
            for (i=0; i<weights[0].nr; i++) {
               if (!weights[i].lock && i != index) {
                  count--;
                  weights[i].weight += delta_part;
                  if (weights[i].weight < 0) {
                     weights[i].weight = 0;
                  }
                  else if (weights[i].weight > max_value) {
                     weights[i].weight = max_value;
                  }

                  max_value -= weights[i].weight;
                  delta = max_value;
                  if (max_value) {
                     for (j=i+1; j<weights[0].nr; j++) {
                        if (!weights[j].lock && j != index) {
                           delta -= weights[j].weight;
                        }
                     }
                     if (count)
                        delta_part = delta/count;
                     else
                        delta_part = delta;
                  }
                  else {
                     for (j=i+1; j<weights[0].nr; j++) {
                        if (!weights[j].lock && j != index) {
                           weights[j].weight = 0;
                        }
                     }
                     break;
                  }
               }
            }
         }


         /*
         ** the new values have been calculated, display them
         */
         for (i=0; i<weights[0].nr; i++) {
/* printf("weights[%d].weight: %d\n", i, weights[i].weight); */
            
            sum += weights[i].weight;
            XmScaleSetValue(weights[i].scale, 
                              weights[i].weight);
         }
/* printf("Sum: %d\n", sum); */
      }
      else {
         XmScaleSetValue(weights[index].scale,
                           weights[index].weight);
      }
      
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
Boolean is_empty_word(char *str)
{
   if (!str)
      return 1;

   while (*str && isspace(*str))
      str++;

   return (*str=='\0');
}


/*-------------------------------------------------------------------------*/
void xmui_manage(Widget w)
{
   Widget shell;

   DENTER(GUI_LAYER, "xmui_manage");

   if (XtIsTopLevelShell(w))
      shell = w;
   else
      shell = XtParent(w);

   if (XtIsTopLevelShell(shell))
      XtPopup(shell, XtGrabNone);
   else
      XtManageChild(w);
   
   if (XtIsRealized(shell))
      XMapWindow(XtDisplay(shell), XtWindow(shell));

   /*
   ** raise window
   */
   XRaiseWindow(XtDisplay(shell), XtWindow(shell));

   DEXIT;
}



/*-------------------------------------------------------------------------*/
void xmui_unmanage(Widget w)
{
   Widget shell;

   DENTER(GUI_LAYER, "xmui_unmanage");

   if (XtIsTopLevelShell(w))
      shell = w;
   else
      shell = XtParent(w);

   if (XtIsTopLevelShell(shell))
      XtPopdown(shell);
   else
      XtUnmanageChild(w);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
void DeleteItems(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount;
   Widget list = (Widget) cld;
   
   DENTER(GUI_LAYER, "DeleteItems");

   XtVaGetValues( list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount)
      XmListDeleteItems(list, selectedItems, selectedItemCount); 

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void XmListAddXmStringUniqueSorted(Widget list, XmString xmitem)
{
   XmString *items = NULL;
   int upperBound = 0, lowerBound = 0;
   String text = NULL, item2 = NULL;
   
   DENTER(GUI_LAYER, "AddItemUniqueSorted");

   if (xmitem) {
      if (XmListItemExists(list, xmitem)) {
         DEXIT;
         return;
      }
      if (!XmStringGetLtoR(xmitem, XmFONTLIST_DEFAULT_TAG, &item2)) {
         DEXIT;
         return;
      }   

      XtVaGetValues( list,
                  XmNitems, &items,
                  XmNitemCount, &upperBound,
                  NULL);
      /*
      ** binary search through the items in the list
      */
      upperBound--;
      while (upperBound >= lowerBound) {
         int i = lowerBound + (upperBound - lowerBound) / 2;
         /* convert XmString to String */
         if (!XmStringGetLtoR(items[i], XmFONTLIST_DEFAULT_TAG, &text))
            break;
         if (strcmp(text, item2) > 0)
            upperBound = i - 1; /* text comes after item2 */
         else
            lowerBound = i + 1; /* text comes before item2 */
         XtFree(text); 
      }
      XtFree(item2);
      XmListAddItemUnselected(list, xmitem, lowerBound + 1);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void XmListAddItemUniqueSorted(Widget list, String item)
{
   XmString xmitem = NULL;
   XmString *items = NULL;
   int upperBound = 0, lowerBound = 0;
   String text = NULL, item2 = NULL;
   
   DENTER(GUI_LAYER, "AddItemUniqueSorted");

   if (item && item[0] != '\0') {
      /*
      ** fix the strange behavior of @f* hostgroup names, this is caused
      ** by the Xmt font converter that interpretes @f as the beginning of
      ** a font change
      */
      char buf[10000];
      if (item[0] == '@' && item[1] != '{')
         snprintf(buf, sizeof(buf), "@%s", item);
      else
         snprintf(buf, sizeof(buf), "%s", item);
      xmitem = XmtCreateLocalizedXmString(list, buf);
      if (XmListItemExists(list, xmitem)) {
         XmStringFree(xmitem);
         DEXIT;
         return;
      }
      if (!XmStringGetLtoR(xmitem, XmFONTLIST_DEFAULT_TAG, &item2)) {
         XmStringFree(xmitem);
         DEXIT;
         return;
      }   

      XtVaGetValues( list,
                  XmNitems, &items,
                  XmNitemCount, &upperBound,
                  NULL);
      /*
      ** binary search through the items in the list
      */
      upperBound--;
      while (upperBound >= lowerBound) {
         int i = lowerBound + (upperBound - lowerBound) / 2;
         /* convert XmString to String */
         if (!XmStringGetLtoR(items[i], XmFONTLIST_DEFAULT_TAG, &text))
            break;
         if (strcmp(text, item2) > 0)
            upperBound = i - 1; /* text comes after item2 */
         else
            lowerBound = i + 1; /* text comes before item2 */
         XtFree(text); 
      }
      XtFree(item2);
      XmListAddItemUnselected(list, xmitem, lowerBound + 1);
      XmStringFree(xmitem);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void XmListMoveItemToPos(Widget list, String item, int pos)
{
   XmString xmitem = NULL;
   
   DENTER(GUI_LAYER, "XmListMoveItemToPos");

   if (item && item[0] != '\0') {
      xmitem = XmtCreateXmString(item);
      if (XmListItemExists(list, xmitem)) {
         XmListDeleteItem(list, xmitem);
         XmListAddItem(list, xmitem, pos);
      }
      XmStringFree(xmitem);
   }   
   DEXIT;
}


/*
** removes leading and trailing whitespace
** the string is changed
*/
char *qmon_trim(
char *s 
) {
   int len, i;

   if (!s)
      return NULL;

   /*
   ** remove leading whitespace
   */
   while (*s && isspace(*s))
      s++;
      
   len = strlen(s) - 1;

   for (i=len; i>0 && *s && isspace(s[i]); i--)
      s[i] = '\0';

   return s;
}   


void ForceUpdate(
Widget w 
) {
   Widget diashell, topshell;
   Window diawindow, topwindow;
   XtAppContext cxt = XtWidgetToApplicationContext(w);
   Display *dpy;
   XWindowAttributes xwa;
   XEvent event;

   for (diashell = w; !XtIsShell(diashell); diashell = XtParent(diashell))
      ;
   for (topshell = diashell; !XtIsTopLevelShell(topshell); topshell = XtParent(topshell))
      ;

   if (XtIsRealized(diashell) && XtIsRealized(topshell)) {
      dpy = XtDisplay(topshell);
      diawindow = XtWindow(diashell);
      topwindow = XtWindow(topshell);

      while (XGetWindowAttributes(dpy, diawindow, &xwa) && xwa.map_state != IsViewable) {
         if (XGetWindowAttributes(dpy, topwindow, &xwa) && xwa.map_state != IsViewable)
            break;
         XtAppNextEvent(cxt, &event);
         XtDispatchEvent(&event);
      }
   }
   XmUpdateDisplay(topshell);
}

   
   
