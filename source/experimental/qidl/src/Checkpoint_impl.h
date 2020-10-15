#ifndef __CHECKPOINT_IMPL_H
#define __CHECKPOINT_IMPL_H
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
// Checkpoint_impl.h
// header for for Checkpoint implementation


#include "Checkpoint_implementation.h"

class Sge_Checkpoint_impl : public virtual Sge_Checkpoint_implementation {
   public:
      Sge_Checkpoint_impl(const char* _name, CORBA_ORB_var o);
      Sge_Checkpoint_impl(const char* _name, const time_t& tm, CORBA_ORB_var o);
      virtual ~Sge_Checkpoint_impl();

      // idl
      virtual Sge_sge_string  get_name(CORBA_Context* ctx);
      virtual void               add(CORBA_Context* ctx);
      virtual void               destroy(CORBA_Context* ctx);
      
      virtual Sge_QueueSeq*   get_queue_list(CORBA_Context* ctx);
      virtual Sge_sge_ulong   set_queue_list(const Sge_QueueSeq& val, CORBA_Context* ctx);

      // non-idl
      virtual void      changed(lListElem* _newSelf);

   private:
      virtual lListElem* getSelf();

      friend class Sge_Master_impl;
};



#endif /* __CHECKPOINT_IMPL_H */
