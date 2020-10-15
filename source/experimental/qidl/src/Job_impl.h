#ifndef __JOB_IMPL_H
#define __JOB_IMPL_H
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
// Job_impl.h
// header file for job implementation


#include "Job_implementation.h"

class Sge_Job_impl : public virtual Sge_Job_implementation {
   public:
      Sge_Job_impl(Sge_sge_ulong key, CORBA_ORB_var o);
      Sge_Job_impl(Sge_sge_ulong key, const time_t& tm, CORBA_ORB_var o);
      virtual ~Sge_Job_impl();

      // idl
      virtual Sge_sge_ulong   get_job_number(CORBA_Context* ctx);

      virtual Sge_ParallelEnvironment*   get_pe_object(CORBA_Context* ctx);
      virtual Sge_sge_ulong              set_pe_object(Sge_ParallelEnvironment* val, CORBA_Context* ctx); 
      
      virtual Sge_Checkpoint* get_checkpoint_object(CORBA_Context* ctx);
      virtual Sge_sge_ulong   set_checkpoint_object(Sge_Checkpoint* val, CORBA_Context* ctx); 
      
      virtual Sge_QueueSeq*   get_hard_queue_list(CORBA_Context* ctx);
      virtual Sge_sge_ulong   set_hard_queue_list(const Sge_QueueSeq& val, CORBA_Context* ctx); 
      
      virtual Sge_QueueSeq*   get_soft_queue_list(CORBA_Context* ctx);
      virtual Sge_sge_ulong   set_soft_queue_list(const Sge_QueueSeq& val, CORBA_Context* ctx); 

      virtual Sge_JobSeq*     get_jid_predecessor_list(CORBA_Context* ctx);
      virtual Sge_sge_ulong   set_jid_predecessor_list(const Sge_JobSeq& val, CORBA_Context* ctx); 

      virtual Sge_UserProject* get_project(CORBA_Context* ctx);
      virtual Sge_sge_ulong    set_project(Sge_UserProject* val, CORBA_Context* ctx); 

      virtual Sge_UserSet*     get_department(CORBA_Context* ctx);

      virtual void               destroy(CORBA_Context* ctx);
      virtual void               add(CORBA_Context* ctx);
      virtual void               submit(CORBA_Context* ctx);
      virtual void               submit_array(Sge_sge_ulong min,
                                              Sge_sge_ulong max,
                                              Sge_sge_ulong step,
                                              CORBA_Context* ctx);

      virtual void               hold(CORBA_Context* ctx);
      virtual void               hold_task(Sge_sge_ulong task_id,
                                           CORBA_Context* ctx);
      virtual void               hold_range(Sge_sge_ulong start,
                                            Sge_sge_ulong end,
                                            Sge_sge_ulong step,
                                            CORBA_Context* ctx);

      virtual void               release(CORBA_Context* ctx);
      virtual void               release_task(Sge_sge_ulong task_id,
                                              CORBA_Context* ctx);
      virtual void               release_range(Sge_sge_ulong start,
                                               Sge_sge_ulong end,
                                               Sge_sge_ulong step,
                                               CORBA_Context* ctx);

      virtual void               suspend(CORBA_Boolean force,
                                         CORBA_Context* ctx);
      virtual void               suspend_task(Sge_sge_ulong task_id,
                                              CORBA_Boolean force,
                                              CORBA_Context* ctx);
      virtual void               suspend_range(Sge_sge_ulong start,
                                               Sge_sge_ulong end,
                                               Sge_sge_ulong range,
                                               CORBA_Boolean force,
                                               CORBA_Context* ctx);

      virtual void               unsuspend(CORBA_Boolean force,
                                           CORBA_Context* ctx);
      virtual void               unsuspend_task(Sge_sge_ulong task_id,
                                                CORBA_Boolean force,
                                                CORBA_Context* ctx);
      virtual void               unsuspend_range(Sge_sge_ulong start,
                                                 Sge_sge_ulong end,
                                                 Sge_sge_ulong range,
                                                 CORBA_Boolean force,
                                                 CORBA_Context* ctx);

      // non-idl
      virtual void               changed(lListElem* _newSelf);

   private:
      virtual lListElem* getSelf();
      virtual void               hold_impl(CORBA_Context* ctx,
                                      lUlong start=0,
                                      lUlong end=0,
                                      lUlong step=0);
      virtual void               release_impl(CORBA_Context* ctx,
                                         lUlong start=0,
                                         lUlong end=0,
                                         lUlong step=0);
      virtual void               suspend_impl(CORBA_Context* ctx,
                                              CORBA_Boolean force,
                                              lUlong start=0,
                                              lUlong end=0,
                                              lUlong step=0);
      virtual void               unsuspend_impl(CORBA_Context* ctx,
                                                CORBA_Boolean force,
                                                lUlong start=0,
                                                lUlong end=0,
                                                lUlong step=0);

      // data member that stores status of hold flag for a non-public
      // job, i.e. one that has not yet been submitted. this status
      // is then used to build up the ja_tasks list upon submission
      Sge_sge_ulong           hold_new_job;

      friend class Sge_Master_impl;
};

#endif /* __JOB_IMPL_H */
