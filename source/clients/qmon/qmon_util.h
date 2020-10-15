#ifndef QMON_UTIL_H
#define QMON_UTIL_H
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

#include <Xm/Xm.h>
#include "qmon_proto.h"
#include "qmon_cull.h"

typedef struct _tScaleWeight {
   int weight;
   Widget scale;
   Widget toggle;
   int lock;
   int nr;
} tScaleWeight;

enum _selectedItem {
   ALL_ITEMS,
   SELECTED_ITEMS
};

enum _shellDimensionMode {
   SHELL_WIDTH = 1,
   SHELL_HEIGHT
};

#if XmVersion < 1002
void XmListDeletePositions(Widget list, int *pos_list, int pos_count);
#endif

String* XmStringTableToStringTable(XmStringTable items, Cardinal itemCount, XmStringCharSet tag);
XmString* StringTableToXmStringTable(String *items, Cardinal itemCount, XmStringCharSet tag);
void XmStringTableFree(XmStringTable items, Cardinal itemCount);
void StringTableFree(String *items, Cardinal itemCount);
Boolean AddStringToXmList(Widget list, String str, XmStringCharSet tag, int list_pos);
Boolean DelStringFromXmList(Widget list, String str, XmStringCharSet tag, int list_pos);
void UpdateXmList(Widget list, String *str_table, Cardinal size, XmStringCharSet tag);
Boolean UpdateXmListFromCull(Widget list, XmStringCharSet tag, lList *lp, int field);
lList *XmStringToCull(Widget list, lDescr *dp, int nm, int selected);
void qmonPositionDialog(Widget w, XtPointer cld, XtPointer cad);
void SetMinShellSize(Widget shell, XtPointer cld, XEvent *event, Boolean *ctd);
void SetMaxShellSize(Widget shell, XtPointer cld, XEvent *event, Boolean *ctd);
void XmListAddTextItem(Widget list_w, String item, XmStringCharSet tag, int position);
void XmListAddTextItems(Widget list_w, String *items, Cardinal itemCount, XmStringCharSet tag, int position);

#ifdef _MOTIF_12_
void XmListAddTextItemUnselected(Widget list_w, String item, XmStringCharSet tag, int position);
void XmListAddTextItemsUnselected(Widget list_w, String *items, Cardinal itemCount, XmStringCharSet tag, int position);
#endif

void XmListSelectTextItem(Widget list_w, String item, XmStringCharSet tag, Boolean notify);

int qmonForkEditor(String file);

void qmonXmStringDrawImage(Display *dpy, Drawable da, GC gc, Position x, Position y, XmFontList fontlist, XmString str, unsigned char layout, XRectangle *clip, Dimension *width, Dimension *height);

void qmonXmStringDraw(Display *dpy, Drawable da, GC gc, Position x, Position y, XmFontList fontlist, XmString str, unsigned char layout, XRectangle *clip, Dimension *width, Dimension *height);

void setButtonLabel(Widget w, String label);

void qmonScaleWeight(Widget w, XtPointer cld, XtPointer cad);
void qmonScaleWeightToggle(Widget w, XtPointer cld, XtPointer cad);

void DeleteItems(Widget w, XtPointer cld, XtPointer cad);
void XmListAddItemUniqueSorted(Widget list, String item); 
void XmListAddXmStringUniqueSorted(Widget list, XmString item); 
void XmListMoveItemToPos(Widget list, String item, int pos); 

Boolean is_empty_word(char *str);

void xmui_manage(Widget w);
void xmui_unmanage(Widget w);

void ForceUpdate(Widget w);

char *qmon_trim(char *s);

#endif /* QMON_UTIL_H */

