#ifndef _QMON_QUEUE_H_
#define _QMON_QUEUE_H_
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

#include "qmon_proto.h"
#include "qmon_cull.h"

/*-------------------------------------------------------------------------*/
/* Sizes in Pixels, these values should be resources */
#define QUEUE_GRID_XOFFSET  5
#define QUEUE_GRID_YOFFSET  5

#define QUEUE_GRID_WIDTH  80
#define QUEUE_GRID_HEIGHT 80

#define QUEUE_MAX_HORIZ   7    /* So we have QUEUE_MAX_HORIZ * QUEUE_MAX_VERT */
#define QUEUE_MAX_VERT    394  /*  PB's (queues)                              */     

#if 0
#define QUEUE_DRAWINGAREA_HEIGHT QUEUE_MAX_VERT * \
                                 QUEUE_GRID_HEIGHT + \
                                 2 * QUEUE_GRID_YOFFSET
#define QUEUE_DRAWINGAREA_WIDTH  QUEUE_MAX_HORIZ * \
                                 QUEUE_GRID_WIDTH + \
                                 2 * QUEUE_GRID_XOFFSET
#endif

#define QUEUE_BUTTON_WIDTH       70 
#define QUEUE_BUTTON_HEIGHT      70


/*-------------------------------------------------------------------------*/
#define QUEUE_SELECT_COLOR    50

typedef struct _tQueueIcon {
   long quark;      /* unique quark */
   int type;            /* QMON_SIMPLE, QMON_VM, QMON_USERDEFINED */
   Boolean selected;    /* is the widget selected */
   Boolean deleted;     /* the widget is marked deleted */
   int grid_x;          /* grid position -> position in button grid */
   int grid_y;
   Pixmap pixmap;       /* pixmap to show */
   int x,y;             /* pixmap position */
   unsigned int   icon_width, 
                  icon_height; /* pixmap width, height  */
   String arch;         /* architecture of queue */
   lListElem *qp;       /* queue list ptr */       
} tQueueIcon;


typedef struct _tQueueButton {
   unsigned long run;   /* every redisplay increments run   */
   Widget   id;         /* button id                        */
   Widget   bgid;       /* drawing area parent to show select  */
   tQueueIcon *qI;      /* connected queue icon, NULL if the   */
                        /* button is not mapped                */
   Boolean  selected;
   int      state;      
} tQueueButton;

void qmonQueuePopup(Widget w, XtPointer cld, XtPointer cad);
void updateQueueList(void);
void updateQueueListCB(Widget w, XtPointer cld, XtPointer cad);


#endif /* _QMON_QUEUE_H_ */

