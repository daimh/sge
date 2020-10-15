#ifndef _QMON_APPRES_H_
#define _QMON_APPRES_H_
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

typedef struct _tApplicationResources {
   int qmon_version;
   int display_message_duration;
   int fetch_time;
   int multi_click_time;
   int msg_how;
   String htmlHelpFile;
   Boolean showHostTab;
   Boolean automaticUpdateHostTab;
/*
   int queue_grid_xoffset;
   int queue_grid_yoffset;
   int queue_grid_width;
   int queue_grid_height;
   int queue_max_horiz;
   int queue_max_vert;
*/
} tApplicationResources;

extern tApplicationResources     QmonApplicationResources;


#define QMON_TIMER_UNIT                   1000        /* 1000 ms */

#define DISPLAY_MESSAGE_DURATION  \
   QmonApplicationResources.display_message_duration * QMON_TIMER_UNIT
#define FETCH_TIME \
   QmonApplicationResources.fetch_time * QMON_TIMER_UNIT

#define MULTI_CLICK_TIME \
   QmonApplicationResources.multi_click_time

#define QMON_VERSION                   QmonApplicationResources.qmon_version
#define MSG_HOW                        QmonApplicationResources.msg_how
#define HTMLHELPFILE                   QmonApplicationResources.htmlHelpFile
#define SHOW_HOST_TAB                  QmonApplicationResources.showHostTab
#define AUTOMATIC_UPDATE_HOST_TAB      QmonApplicationResources.automaticUpdateHostTab

void qmonGetApplicationResources(Widget top, ArgList args, Cardinal num_args);


#endif /* _QMON_APPRES_H_ */
