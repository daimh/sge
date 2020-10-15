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

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_event.h"
#include "sgeobj/sge_answer.h"

#include "comm/commlib.h"

#include "mir/sge_mirror.h"

#include "gdi/sge_gdiP.h"

#include "usage.h"
#include "sig_handlers.h"
#include "msg_clients_common.h"

int events;
u_long32 events_size;

static sge_callback_result
print_event(sge_evc_class_t *evc, object_description *object_base, sge_object_type type, 
            sge_event_action action, lListElem *event, void *clientdata)
{
   char buffer[1024];
   sge_pack_buffer pb;
   dstring buffer_wrapper;

   DENTER(TOP_LAYER, "print_event");

   events++;
   init_packbuffer(&pb, 0, 1);
   cull_pack_elem(&pb, event);
   events_size += pb.bytes_used;

   sge_dstring_init(&buffer_wrapper, buffer, sizeof(buffer));

   printf("%s\n", event_text(event, &buffer_wrapper));

   /* create a callback error to test error handling */
   if (type == SGE_TYPE_GLOBAL_CONFIG) {
      DRETURN(SGE_EMA_FAILURE);
   }
   
   DRETURN(SGE_EMA_OK);
}

int main(int argc, char *argv[])
{
   int cl_err = 0;
   lList *alp = NULL;
   sge_gdi_ctx_class_t *ctx = NULL; 
   sge_evc_class_t *evc = NULL;

   DENTER_MAIN(TOP_LAYER, "test_sge_mirror");

   sge_setup_sig_handlers(QEVENT);

   /* setup event client */
   cl_err = sge_gdi2_setup(&ctx, QEVENT, MAIN_THREAD, &alp);
   if (cl_err != AE_OK) {
      answer_list_output(&alp);
      SGE_EXIT((void**)&ctx, 1);
   }

   if (false == sge_gdi2_evc_setup(&evc, ctx, EV_ID_ANY, &alp, NULL)) {
      answer_list_output(&alp);
      SGE_EXIT((void**)&ctx, 1);
   }

   sge_mirror_initialize(evc, EV_ID_ANY, "test_sge_mirror", true, 
                         NULL, NULL, NULL, NULL, NULL);
   sge_mirror_subscribe(evc, SGE_TYPE_ALL, print_event, NULL, NULL, NULL, NULL);
   
   while(!shut_me_down) {
      events=events_size=0;
      printf("---------------------\n");
      sge_mirror_process_events(evc);
      printf("received "sge_u32" kbytes in with %d events\n", events_size/1024, events);
   }

   sge_mirror_shutdown(evc);

   DRETURN(EXIT_SUCCESS);
}
