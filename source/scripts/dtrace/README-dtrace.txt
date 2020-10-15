                Monitoring Grid Engine Masters with dtrace
                ------------------------------------------

Content
-------
1. Introduction
2. Master bottleneck analyis with dtrace
3. Copyright

1. Introduction
---------------

   Dtrace is a comprehensive framework for tracing dynamic events in
   Solaris 10. Please see under

      http://www.sun.com/bigadmin/content/dtrace/

   for more detailed information about dtrace.

2. Master bottleneck analyis with dtrace
----------------------------------------

   Understanding the bottlenecks of distributed systems is crucial for 
   performance tuning. The script $SGE_ROOT/util/dtrace/monitor.sh allows 
   a Grid Engine master be monitored, if Solaris 10 dtrace(1) can be used.

   Monitor.sh measures throughput-relevant data of your running Grid Engine 
   master and compiles this data into few indices that are printed in a 
   single-line view per interval with columns below.

      Spooling:
        #wrt 
           Number of qmaster write operations via spool_write_object() and 
           spool_delete_object(). Almost any significant write operation goes
           through this function both in bdb/classic spooling.

        wrt/ms
           Total time all threads spend in spool_write_object() in micro
           seconds.

      Message processing:
        #rep
           Number of reports qmaster processed through sge_c_report().
           Most data sent by execd's to qmaster comes as such a report
           (job/load/config report).

        #gdi 
           Number of GDI requests qmaster processed through do_gdi_request().
           Almost anything sent from client commands arrives qmaster as a
           GDI request, but also execd's and scheduler use GDI requests.

        #ack
           Number of ACK messages qmaster processed through do_c_ack().
           High numbers of ACK messages can be an indication of job
           signalling, but they are used also for other purposes.
           
      Scheduling:
         #dsp
           Number of calls to dispatch_jobs() in schedd. Each call
           to dispatch_jobs() can seen as a scheduling run.

         dsp/ms
           Total time scheduler spent in all calls to dispatch_jobs().

         #sad
           Number of calls to select_assign_debit(). Each call to
           select_assign_debit() can be seen as a try of the scheduler
           to find an assignement or a reservation for a job.

      Qmaster/Schedd synchronization:
         #snd
           Number of event packages sent by qmaster to schedd. If that
           number goes down to zero over longer time there is something
           wrong and qmaster/schedd get out of sync.

         #rcv
           Number of event packages received by schedd from qmaster.
           If that number goes down to zero over longer time there is
           something wrong and qmaster/schedd get out of sync.

      Qmaster communication:
         #in++   
           Number of messages added into qmaster received messages 
           buffer.

         #in--
           Number of messages removed from qmaster received messages 
           buffer. If more messages are added than removed during an 
           interval, the total of messages not yet processed is about 
           to grow.

         #out++  
           Number of messages added into qmaster send messages 
           buffer.

         #out--
           Number of messages removed from qmaster send messages 
           buffer. If more messages are added than removed during an 
           interval, the total of not yet messages not yet delivered 
           is about to grow.

      Qmaster locks:
         #lck0/#ulck0
           Number of calls to sge_lock()/sge_unlock() for qmasters
           "global" lock. This lock must always be obtained, when
           qmaster-internal lists (job list, queue list, etc.) are
           accessed.

         #lck1/#ulck1
           Number of calls to sge_lock()/sge_unlock() for qmasters
           "master_config" lock. This lock is a secondary lock, but
           also plays it's role.

   note, currently the following options are supported:

      -interval <time> 
       
          For use of statistics intervals other than "15sec"

      -spooling
     
          Shows qmaster spooling probes besides statistics. This
          option allows diving into a presumed spooling bottleneck.

      -requests

          Shows incoming qmaster request probes. This option allows 
          diving into cases where you presume there must be someone
          flooding your qmaster.

      -verify

          Just verify probes are functioning and exit(0) then.

   besides, for ease of use any critical/error/warning logging appears
   in monitor.sh output.

3. Copyright
------------
___INFO__MARK_BEGIN__
The Contents of this file are made available subject to the terms of the Sun
Industry Standards Source License Version 1.2

Sun Microsystems Inc., March, 2001

Sun Industry Standards Source License Version 1.2
=================================================

The contents of this file are subject to the Sun Industry Standards Source
License Version 1.2 (the "License"); You may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://gridengine.sunsource.net/Gridengine_SISSL_license.html

Software provided under this License is provided on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.

See the License for the specific provisions governing your rights and
obligations concerning the Software.

The Initial Developer of the Original Code is: Sun Microsystems, Inc.

Copyright: 2001 by Sun Microsystems, Inc.

All Rights Reserved.
___INFO__MARK_END__
