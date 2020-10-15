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
package com.sun.grid.drmaa.howto;

import java.util.Collections;
import org.ggf.drmaa.DrmaaException;
import org.ggf.drmaa.JobTemplate;
import org.ggf.drmaa.Session;
import org.ggf.drmaa.SessionFactory;

public class Howto5 {
   public static void main(String[] args) {
      SessionFactory factory = SessionFactory.getFactory();
      Session session = factory.getSession();
      
      try {
         session.init("");
         JobTemplate jt = session.createJobTemplate();
         jt.setRemoteCommand("sleeper.sh");
         jt.setArgs(Collections.singletonList("5"));
         
         String id = session.runJob(jt);
         
         System.out.println("Your job has been submitted with id " + id);
         
         try {
            Thread.sleep(20 * 1000);
         } catch (InterruptedException e) {
            // Don't care
         }
         
         int status = session.getJobProgramStatus(id);
         
         switch (status) {
            case Session.UNDETERMINED:
               System.out.println("Job status cannot be determined\n");
               break;
            case Session.QUEUED_ACTIVE:
               System.out.println("Job is queued and active\n");
               break;
            case Session.SYSTEM_ON_HOLD:
               System.out.println("Job is queued and in system hold\n");
               break;
            case Session.USER_ON_HOLD:
               System.out.println("Job is queued and in user hold\n");
               break;
            case Session.USER_SYSTEM_ON_HOLD:
               System.out.println("Job is queued and in user and system hold\n");
               break;
            case Session.RUNNING:
               System.out.println("Job is running\n");
               break;
            case Session.SYSTEM_SUSPENDED:
               System.out.println("Job is system suspended\n");
               break;
            case Session.USER_SUSPENDED:
               System.out.println("Job is user suspended\n");
               break;
            case Session.USER_SYSTEM_SUSPENDED:
               System.out.println("Job is user and system suspended\n");
               break;
            case Session.DONE:
               System.out.println("Job finished normally\n");
               break;
            case Session.FAILED:
               System.out.println("Job finished, but failed\n");
               break;
         } /* switch */
         
         session.deleteJobTemplate(jt);
         session.exit();
      } catch (DrmaaException e) {
         System.out.println("Error: " + e.getMessage());
      }
   }
}
