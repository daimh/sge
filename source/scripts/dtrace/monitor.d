/*************************************************************************
 * ___INFO__MARK_BEGIN__
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
 * ___INFO__MARK_END__
 ************************************************************************/

/*  Parameters:
      $1 = qmaster_pid
      $2 = interval
      $3 = show qmaster spooling probes
      $4 = show incoming qmaster request probes
      $5 = do probe verification only
*/

BEGIN
{
   printf("%20s |%7s %7s |%4s %4s %4s|%7s %7s %7s|%7s %7s|%7s %7s %7s %7s|%7s %7s %7s %7s", 
         "Time", "#wrt", "wrt/ms", "#rep", "#gdi", "#ack", "#dsp", "dsp/ms", "#sad", 
         "#snd", "#rcv", "#in++", "#in--", "#out++", "#out--", 
         "#lck0", "#ulck0", "#lck1", "#ulck1");
   snd = 0;
   rcv = 0;
   rep = 0;
   ack = 0;
   gdi = 0;
   wrt = 0;
   wrt_total = 0;
   dsp = 0;
   sad = 0;
   dsp_total = 0;
   add_in = 0;
   add_out = 0;
   remove_in = 0;
   remove_out = 0;
   lck0 = 0;
   lck1 = 0;
   ulck0 = 0;
   ulck1 = 0;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*                                qmaster/scheduler logging                             */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* errors, warnings and criticals only */
pid$1::sge_log:entry
/arg0 == 2 || arg0 == 3 || arg0 == 4/
{
   printf("%20Y | %s(%d, %s)", walltimestamp, probefunc, arg0, copyinstr(arg1));
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*                                    statistics                                        */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

profile:::tick-$2
/$5 == 1/
{
   exit(0);
}

profile:::tick-$2
/$5 == 0/
{
   printf("%20Y | %7d %7d|%4d %4d %4d|%7d %7d %7d|%7d %7d|%7d %7d %7d %7d|%7d %7d %7d %7d",
        walltimestamp, wrt, wrt_total/1000000, rep, gdi, ack, dsp, dsp_total, sad, 
        snd, rcv, add_in, remove_in, add_out, remove_out, lck0, ulck0, lck1, ulck1);
   snd = 0;
   rcv = 0;
   rep = 0;
   ack = 0;
   gdi = 0;
   wrt = 0;
   wrt_total = 0;
   dsp = 0;
   dsp_total = 0;
   add_in = 0;
   add_out = 0;
   remove_in = 0;
   remove_out = 0;
   sad = 0;
   lck0 = 0;
   ulck0 = 0;
   lck1 = 0;
   ulck1 = 0;

}

/* -------------------------------------- [spooling] ---------------------------------- */

pid$1::spool_write_object:entry, pid$1::spool_delete_object:entry, pid$1::spool_read_object:entry,
pid$1::spool_write_script:entry, pid$1::spool_delete_script:entry, pid$1::spool_read_script:entry
{
   self->spool_start = timestamp;
}

pid$1::spool_write_object:return, pid$1::spool_delete_object:return, pid$1::spool_read_object:return,
pid$1::spool_write_script:return, pid$1::spool_delete_script:return, pid$1::spool_read_script:return
{ 
   wrt_total += timestamp - self->spool_start;
   @q[ probefunc ] = quantize((timestamp - self->spool_start)/1000);
   wrt++;
}

/* -------------------------------------- [requests] ---------------------------------- */

pid$1::sge_c_report:return
{ 
   rep++;
}

pid$1::do_c_ack:return
{ 
   ack++;
}

pid$1::do_gdi_packet:return
{ 
   gdi++;
}

/* ------------------------------------- [scheduling] --------------------------------- */

pid$1::dispatch_jobs:entry
{
   self->dispatch_start = timestamp;
}
pid$1::dispatch_jobs:return
{ 
   dsp_total += (timestamp - self->dispatch_start)/1000000;
   @q[ probefunc ] = quantize((timestamp - self->dispatch_start)/1000000);
   dsp++;
}

pid$1::select_assign_debit:return
{
   sad++;
}

/* ---------------------------------- [synchronization] ------------------------------- */

pid$1::event_update_func:return
{ 
   snd++;
}

pid$1::sge_mirror_process_event_list:return
{ 
   rcv++;
}

/* --------------------------------------- [locks] ------------------------------------ */

pid$1::sge_lock:entry
{
   self->lnum = arg0
/*    printf("\t%s(%d, %s) tid %d %d", probefunc, arg0, copyinstr(arg2), tid, timestamp); */
}
pid$1::sge_lock:return
/self->lnum == 0/
{ 
   lck0++;
}
pid$1::sge_lock:return
/self->lnum == 1/
{ 
   lck1++;
}

pid$1::sge_unlock:entry
{
   self->lnum = arg0
}
pid$1::sge_unlock:return
/self->lnum == 0/
{ 
   ulck0++;
}
pid$1::sge_unlock:return
/self->lnum == 1/
{ 
   ulck1++;
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*                            showing probes in detail                                  */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* -------------------------------------- [spooling] ---------------------------------- */

pid$1::spool_read_script:entry,pid$1::spool_write_script:entry,pid$1::spool_delete_script:entry
/$3 == 1/
{
   printf("\t%s(%d) tid %d", probefunc, arg1, tid);
}

pid$1::spool_read_object:entry
/$3 == 1/
{
   printf("\t%s(%d, %s) tid %d", probefunc, arg2, copyinstr(arg3), tid);
}
pid$1::spool_write_object:entry
/$3 == 1/
{
   printf("\t%s(%d, %s) tid %d", probefunc, arg4, copyinstr(arg3), tid);
}
pid$1::spool_delete_object:entry
/$3 == 1/
{
   printf("\t%s(%d, %s) tid %d", probefunc, arg2, copyinstr(arg3), tid);
}

/* -------------------------------------- [requests] ---------------------------------- */
pid$1::sge_c_report:entry
/$4 == 1/
{
   printf("\t%s(%s, %s) tid %d", probefunc, copyinstr(arg1), copyinstr(arg2), tid);
}

pid$1::do_c_ack:entry
/$4 == 1/
{
   printf("\t%s() tid %d", probefunc, tid);
}

pid$1::sge_c_gdi_permcheck:entry 
/$4 == 1/
{
   printf("\t%s(%s) tid %d", probefunc, copyinstr(arg0), tid);
}

pid$1::sge_c_gdi_trigger:entry, pid$1::sge_c_gdi_get:entry, pid$1::sge_c_gdi_del:entry
/$4 == 1/
{
   printf("\t%s(%s) tid %d", probefunc, copyinstr(arg1), tid);
}

pid$1::sge_c_gdi_mod:entry, pid$1::sge_c_gdi_add:entry, pid$1::sge_c_gdi_copy:entry 
/$4 == 1/
{
   printf("\t%s(%s) tid %d", probefunc, copyinstr(arg2), tid);
}



/* ------------------------------------- [scheduling] --------------------------------- */
/* ---------------------------------- [synchronization] ------------------------------- */
pid$1::sge_mirror_process_events:entry
/$4 == 1/
{ 
   printf("\t%s() tid %d", probefunc, tid);
}
/* -------------------------------------- [commlib] ----------------------------------- */

pid$1::cl_message_list_remove_receive:return
{
   remove_in++;
}
pid$1::cl_message_list_append_receive:return
{
   add_in++;
}
pid$1::cl_message_list_remove_send:return
{
   remove_out++;
}
pid$1::cl_message_list_append_send:return
{
   add_out++;
}

/* --------------------------------------- [locks] ------------------------------------ */

