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
import Codine.*;
import Codine.Codine.*;
import org.omg.CosNaming.*;
import org.omg.CosEventComm.*;
import org.omg.CORBA.*;

import java.lang.*;
import java.util.*;

//import jqmon.*;
//import jqmon.util.*;
//import jqmon.debug.*;
//import jqmon.events.*;
//import jcodine.*;


public class QueueClientEx 
{
   public static void main(String args[])
   {
      try {
         // create and initialize the ORB
         ORB orb = ORB.init(args, new java.util.Properties());
 
			String ref = null;
			try {
            String refFile = "/cod_home/hartmut/grd12/default/common/master.ior";
				java.io.BufferedReader in = new java.io.BufferedReader(new java.io.FileReader(refFile));
				ref = in.readLine();
			}
			catch(java.io.IOException ex)
			{
            System.out.println("Can't read from '" + ex.getMessage() + "`");
				System.exit(1);
			}

         // get the root naming context
         org.omg.CORBA.Object objRef = orb.string_to_object(ref);
			Master master = MasterHelper.narrow(objRef);
				
			//NamingContext ncRef = NamingContextHelper.narrow(objRef);
			//NameComponent nc = new NameComponent("Master", "");
         //NameComponent path[] = {nc};
         //Master masterRef = MasterHelper.narrow(ncRef.resolve(path));					

			System.out.println("We have an object masterRef");				

			//initialize context
			Context ctx = orb.get_default_context();
			Any any = orb.create_any();
			any.insert_string("268:268:100:100:frodo");
			ctx.set_one_value("cod_auth", any);

			try {
            System.out.println("Print the Calendars");
            CalendarSeqHolder cs = new CalendarSeqHolder(master.getCalendars(ctx));
               
				for(int i = 0; i<cs.value.length; i++) {
               System.out.println("Calendar " + i);
					System.out.println("Name: " + cs.value[i].get_name(ctx) );
					System.out.println("Year_Calendar: " + cs.value[i].get_year_calendar(ctx) );
					System.out.println("Week_Calendar: " + cs.value[i].get_week_calendar(ctx) );
            }
               
				// print all the queues with some of their information
				System.out.println("Print the queues");
            QueueSeqHolder qs = new QueueSeqHolder(master.getQueues(ctx));
               
				for(LongHolder i = new LongHolder(0); i.value < qs.value.length; i.value++) {
               System.out.println("Queue " + i.value );
					String foo;
					foo = qs.value[(int)i.value].get_qname(ctx);
					System.out.println("Name: " + foo );
					foo = qs.value[(int)i.value].get_qhostname(ctx);
					System.out.println("host: " + foo );
					System.out.println("prio: " + qs.value[(int)i.value].get_priority(ctx) );
					System.out.println("type: " + qs.value[(int)i.value].get_qtype(ctx) );
					System.out.println("slot: " + qs.value[(int)i.value].get_job_slots(ctx) );
					System.out.println("load thresholds: ");
					ComplexEntrySeqHolder lts = new ComplexEntrySeqHolder(qs.value[(int)i.value].get_load_thresholds(ctx));  
					for(int x = 0; x < lts.value.length; x++) {
						System.out.print(" ( " + lts.value[x].name + "," + lts.value[x].shortcut);
						System.out.print("," + lts.value[x].valtype);
						System.out.print("," + lts.value[x].stringval);
						System.out.print("," + lts.value[x].relop);
                  System.out.print("," + lts.value[x].request);
						System.out.print("," + lts.value[x].consumable);
                  System.out.print("," + lts.value[x].forced);
						System.out.println(")");
					}
					System.out.println("");	
				};

				//create a queue
				//...
			}
			catch(Exception e /*Codine.Error x*/) {
				System.out.println("Codine Error");
			}

/*
            NamingContext ncRef = NamingContextHelper.narrow(objRef);

            // resolve the Object Reference in Naming
            NameComponent nc = new NameComponent("Hello", "");
            NameComponent path[] = {nc};
            Hello helloRef = HelloHelper.narrow(ncRef.resolve(path));
 
            // call the Hello server object and print results
            String hello = helloRef.sayHello();
            System.out.println(hello);
 */
      } 
      catch (Exception e) {
         System.out.println("ERROR : " + e) ;
         e.printStackTrace(System.out);
      }
   }
}
