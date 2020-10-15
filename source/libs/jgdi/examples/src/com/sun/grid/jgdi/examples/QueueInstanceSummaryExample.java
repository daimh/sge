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
package com.sun.grid.jgdi.examples;

import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummary;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryOptions;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryOptions;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryResult;
import com.sun.grid.jgdi.monitoring.filter.ResourceAttributeFilter;
import com.sun.grid.jgdi.monitoring.JobSummary;
import java.util.Iterator;

/**
 *
 */
public class QueueInstanceSummaryExample {
    
    public static void main(String [] args)  {
        
        try {
            String url = "bootstrap:///opt/sge@default:1026";
            
            if(args.length == 1 ) {
                url = args[0];
            }
            
            JGDI jgdi = JGDIFactory.newInstance(url);
            
            try {
                System.out.println("Successfully connected to " + url);
                QueueInstanceSummaryOptions options = new QueueInstanceSummaryOptions();
//            ResourceAttributeFilter raf = new ResourceAttributeFilter();
//            raf.addValueName("arch");
//            options.setResourceAttributeFilter(raf);
                QueueInstanceSummaryResult res = jgdi.getQueueInstanceSummary(options);
                for (QueueInstanceSummary cis : res.getQueueInstanceSummary()) {
                    cis.getJobList();
                    System.out.println(cis.getName() + " " + cis.getLoadAvg());
                }
                
                for (JobSummary js : res.getPendingJobs()) {
                    System.out.println(js.getId() + " (" + js.getName() + ") is pending");
                }
            } finally {
                jgdi.close();
            }
        } catch (JGDIException e) {
            e.printStackTrace();
        }
        
    }
    
}
