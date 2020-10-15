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
package com.sun.grid.jgdi.util.shell;

import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.monitoring.ResourceQuotaRuleInfo;
import com.sun.grid.jgdi.monitoring.QQuotaOptions;
import com.sun.grid.jgdi.monitoring.QQuotaResult;
import com.sun.grid.jgdi.monitoring.ResourceQuota;
import com.sun.grid.jgdi.monitoring.filter.HostFilter;
import com.sun.grid.jgdi.monitoring.filter.ParallelEnvironmentFilter;
import com.sun.grid.jgdi.monitoring.filter.ProjectFilter;
import com.sun.grid.jgdi.monitoring.filter.QueueFilter;
import com.sun.grid.jgdi.monitoring.filter.ResourceAttributeFilter;
import com.sun.grid.jgdi.monitoring.filter.ResourceFilter;
import com.sun.grid.jgdi.monitoring.filter.UserFilter;
import java.util.LinkedList;
import static com.sun.grid.jgdi.util.JGDIShell.getResourceString;

/**
 *
 */
@CommandAnnotation(value = "qquota")
public class QQuotaCommand extends AbstractCommand {
    
    public void run(String[] args) throws Exception {
        QQuotaOptions options = parse(args);
        if (options == null) {
            return;
        }
        
        QQuotaResult res = jgdi.getQQuota(options);
        
        if (!res.getResourceQuotaRules().isEmpty()) {
            out.println("resource quota rule    limit                filter");
            out.println("--------------------------------------------------------------------------------");
            for (ResourceQuotaRuleInfo info : res.getResourceQuotaRules()) {
                // need a Formatter here
                out.print(info.getResouceQuotaRuleName());
                for (ResourceQuota relim : info.getLimits()) {
                    out.print(" " + relim.getName() + "=" + relim.getUsageValue() + "/" + relim.getLimitValue());
                }
                if (!info.getUsers().isEmpty()) {
                    out.print(" users" + info.getUsers());
                }
                if (!info.getProjects().isEmpty()) {
                    out.print(" projects" + info.getProjects());
                }
                if (!info.getPes().isEmpty()) {
                    out.print(" pes" + info.getPes());
                }
                if (!info.getQueues().isEmpty()) {
                    out.print(" queues" + info.getQueues());
                }
                if (!info.getHosts().isEmpty()) {
                    out.print(" hosts" + info.getHosts());
                }
                out.println();
            }
        }
    }
    
    private QQuotaOptions parse(String[] args) throws Exception {
        ResourceAttributeFilter resourceAttributeFilter = null;
        ResourceFilter resourceFilter = null;
        UserFilter userFilter = null;
        HostFilter hostFilter = null;
        ProjectFilter projectFilter = null;
        ParallelEnvironmentFilter peFilter = null;
        QueueFilter queueFilter = null;
        
        LinkedList<String> argList = new LinkedList<String>();
        for (int i = 0; i < args.length; i++) {
            argList.add(args[i]);
        }
        
        while (!argList.isEmpty()) {
            String arg = (String) argList.removeFirst();
            
            if (arg.equals("-help")) {
                out.println(getUsage());
                return null;
            } else if (arg.equals("-h")) {
                if (argList.isEmpty()) {
                    throw new IllegalArgumentException("missing host_list");
                }
                arg = (String) argList.removeFirst();
                hostFilter = HostFilter.parse(arg);
            } else if (arg.equals("-l")) {
                if (argList.isEmpty()) {
                    throw new IllegalArgumentException("missing resource_list");
                }
                resourceFilter = new ResourceFilter();
                arg = (String) argList.removeFirst();
                resourceFilter = ResourceFilter.parse(arg);
            } else if (arg.equals("-u")) {
                if (argList.isEmpty()) {
                    throw new IllegalArgumentException("missing user_list");
                }
                arg = (String) argList.removeFirst();
                userFilter = UserFilter.parse(arg);
            } else if (arg.equals("-pe")) {
                if (argList.isEmpty()) {
                    throw new IllegalArgumentException("missing pe_list");
                }
                arg = (String) argList.removeFirst();
                peFilter = ParallelEnvironmentFilter.parse(arg);
            } else if (arg.equals("-P")) {
                if (argList.isEmpty()) {
                    throw new IllegalArgumentException("missing project_list");
                }
                arg = (String) argList.removeFirst();
                projectFilter = ProjectFilter.parse(arg);
            } else if (arg.equals("-q")) {
                if (argList.isEmpty()) {
                    throw new IllegalArgumentException("missing wc_queue_list");
                }
                arg = (String) argList.removeFirst();
                queueFilter = QueueFilter.parse(arg);
            } else {
                throw new IllegalStateException("Unknown argument: " + arg);
            }
        }
        
        QQuotaOptions options = new QQuotaOptions();
        
        if (hostFilter != null) {
            options.setHostFilter(hostFilter);
        }
        if (resourceFilter != null) {
            options.setResourceFilter(resourceFilter);
        }
        if (userFilter != null) {
            options.setUserFilter(userFilter);
        }
        if (peFilter != null) {
            options.setPeFilter(peFilter);
        }
        if (projectFilter != null) {
            options.setProjectFilter(projectFilter);
        }
        if (queueFilter != null) {
            options.setQueueFilter(queueFilter);
        }
        
        return options;
    }
}