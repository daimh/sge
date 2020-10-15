/*___INFO__MARK_BEGIN__*/ /*************************************************************************
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
import com.sun.grid.jgdi.monitoring.ClusterQueueSummary;
import com.sun.grid.jgdi.monitoring.ClusterQueueSummaryOptions;
import com.sun.grid.jgdi.monitoring.filter.QueueStateFilter;
import java.util.List;

/**
 *  Simple examples which demonstrates how to retrieve monitoring
 *  information from the Grid Engine like it obtained by the qstat -q c
 *  command.
 *
 */
public class ClusterQueueSummaryExample {
    
    /**
     *  @param args   args[0] can contain the jgdi connection url
     */
    public static void main(String[] args) {
        
        try {
            String url = "bootstrap:///opt/sge@default:1026";
            
            if (args.length == 1) {
                url = args[0];
            }
            
            JGDI jgdi = JGDIFactory.newInstance(url);
            
            try {
                ClusterQueueSummaryOptions options = new ClusterQueueSummaryOptions();
                QueueStateFilter queueStateFilter = new QueueStateFilter();
                queueStateFilter.setDisabled(true);
                options.setQueueStateFilter(queueStateFilter);
                List<ClusterQueueSummary> resultList = jgdi.getClusterQueueSummary(options);
                if (!resultList.isEmpty()) {
                    for (ClusterQueueSummary cqs : resultList) {
                        System.out.println(cqs.getName() + " is disabled");
                    }
                } else {
                    System.out.println("Found no disabled queue");
                }
            } finally {
                jgdi.close();
            }
        } catch (JGDIException e) {
            e.printStackTrace();
        }
    }
}
