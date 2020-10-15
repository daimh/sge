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
#include <Xmt/Xmt.h>
#include <Xmt/AppRes.h>

#include "qmon_rmon.h"
#include "qmon_appres.h"



static XtResource resources[] = {
   { "qmon_version", "qmon_version", XtRInt,
      sizeof(int), XtOffsetOf(tApplicationResources, qmon_version),
      XtRImmediate, (XtPointer) 4000 },
      
   { "display_message_duration", "display_message_duration", XtRInt,
      sizeof(int), XtOffsetOf(tApplicationResources, display_message_duration),
      XtRImmediate, (XtPointer) 10 },

   { "fetch_time", "fetch_time", XtRInt,
      sizeof(int), XtOffsetOf(tApplicationResources, fetch_time),
      XtRImmediate, (XtPointer) 20 },
   
   { "multi_click_time", "multi_click_time", XtRInt,
      sizeof(int), XtOffsetOf(tApplicationResources, multi_click_time),
      XtRImmediate, (XtPointer) 350 },
   
   { "msg_how", "msg_how", XtRInt,
      sizeof(int), XtOffsetOf(tApplicationResources, msg_how),
      XtRImmediate, (XtPointer) 0 },

   { "htmlHelpFile", "htmlHelpFile", XtRString,
      sizeof(String), XtOffsetOf(tApplicationResources, htmlHelpFile),
      XtRImmediate, (XtPointer) NULL },

   { "showHostTab", "showHostTab", XtRBoolean,
      sizeof(int), XtOffsetOf(tApplicationResources, showHostTab),
      XtRImmediate, (XtPointer)False },

   { "automaticUpdateHostTab", "automaticUpdateHostTab", XtRBoolean,
      sizeof(int), XtOffsetOf(tApplicationResources, automaticUpdateHostTab),
      XtRImmediate, (XtPointer)False }

};


tApplicationResources QmonApplicationResources;

/*-------------------------------------------------------------------------*/
void qmonGetApplicationResources(
Widget top,
ArgList args,
Cardinal num_args 
) {
   DENTER(TOP_LAYER, "qmonGetApplicationResources");

   XtGetApplicationResources( top, 
                              &QmonApplicationResources,
                              resources, XtNumber(resources),
                              args, num_args );
   DPRINTF(("qmon_version: %d\n", QMON_VERSION));
   DPRINTF(("display_message_duration: %d\n", DISPLAY_MESSAGE_DURATION));
   DPRINTF(("fetch_time: %d\n", FETCH_TIME));
   DPRINTF(("multi_click_time: %d\n", MULTI_CLICK_TIME));
   DPRINTF(("htmlHelpFile: %s\n", HTMLHELPFILE ? HTMLHELPFILE : "NA"));
   DPRINTF(("showHostTab: %s\n", SHOW_HOST_TAB ? "true" : "false"));
   DPRINTF(("automaticUpdateHostTab: %s\n", AUTOMATIC_UPDATE_HOST_TAB ? "true" : "false"));

   DEXIT;

}
