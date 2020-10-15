#ifndef __SGE_PACKET_TYPE_H
#define __SGE_PACKET_TYPE_H
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

#include "basis_types.h"

#include "cull/cull.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define PACKET_USIZE 128
#define PACKET_UFMT "%127s"

typedef struct _sge_gdi_task_class_t sge_gdi_task_class_t;

typedef struct _sge_gdi_packet_class_t sge_gdi_packet_class_t;

struct _sge_gdi_task_class_t {
   /*
    * id identifying the GDI packet uniquely within the
    * context of a GDI client
    */
   u_long32 id;

   /*
    * common parts of a GDI request
    */
   u_long32 command;
   u_long32 target;
   lList *data_list;
   lList *answer_list;
   lCondition *condition;
   lEnumeration *enumeration;

   /*
    * This flag is used in qmaster to identify if a special 
    * optimization can be done. This optimization can only be
    * done for GDI GET requests where the client is
    * an external GDI client (no thread using GDI). 
    *
    * In that case it is possible that the lSelectHashPack()
    * function is called with a packbuffer so that the function
    * directly packs into this packbuffer.
    * 
    * This avoids a copy operation 
    */
   bool do_select_pack_simultaneous;

   /*
    * pointer to the next task in a multi GDI request
    */
   sge_gdi_task_class_t *next;
};

struct _sge_gdi_packet_class_t {
   /* 
    * mutex to gard the "is_handled" flag of this structure 
    */
   pthread_mutex_t mutex;

   /*
    * condition used for synchronisation of multiple threads
    * which want to access this structure
    */
   pthread_cond_t cond;

   /*
    * true if the worker thread does not need to access this
    * structure anymore. Guarded with "mutex" part of this 
    * structure
    */
   bool is_handled;

   /*
    * true if this structure was created by a qmaster
    * internal thread (scheduler, JVM...)
    */
   bool is_intern_request;

   /*
    * true if the packet contains a GDI request otherwise
    * is containes a report request
    */
   bool is_gdi_request;

   /*
    * unique id identifying this packet uniquely in the context
    * of the creating process/thread
    */
   u_long32 id;

   /*
    * set in qmaster to identify the source for this GDI packet.
    * qmaster will use that information to send a response
    * to the correct external communication partner using the
    * commlib infrastructure
    */
   char *host;
   char *commproc;
   u_short commproc_id;
   u_long32 response_id;

   /*
    * GDI version of this structure
    */
   u_long32 version;

   /*
    * pointers to the first and last task part of a multi
    * GDI request. This list contains at least one element
    */
   sge_gdi_task_class_t *first_task;
   sge_gdi_task_class_t *last_task;  

   /*
    * Possibly-encrypted authentication information. This information will
    * be decrypted to the fields "uid", "gid", "user" and "group"
    * part of this structure 
    */
   char *auth_info;  

   /*
    * User/group information of that user which used GDI functionality.
    * Used in qmasters GDI handling code to identify if that
    * user has the allowance to do the requested GDI activities. 
    */
   uid_t uid;
   gid_t gid;
   char user[PACKET_USIZE];
   char group[PACKET_USIZE];

   /*
    * Packbuffer used for GDI GET requests to directly store the 
    * result of lSelectHashPack()
    *
    * EB: TODO: Cleanup: eleminate "pb" from sge_gdi_packet_class_t
    *    
    *    We might eleminate this member as soon as pure GDI GET
    *    requests are handled by some kind of read only thread.
    *    in qmaster. Write requests don't need the packbuffer.
    *    Due to that fact we could create and release the packbuffer
    *    in the the listener thread and use cull lists (part
    *    of the task sublist) to transfer GDI result information
    *    from the worker to the listener then we are able to
    *    remove pb. 
    */
   sge_pack_buffer pb;

   /* 
    * if this packet is part of a packet queue then this
    * pointer might point to the next packet in the queue
    */
   sge_gdi_packet_class_t *next;
};

#ifdef  __cplusplus
}
#endif

#endif 
