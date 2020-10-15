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

import com.sun.grid.jgdi.configuration.JGDIAnswer;
import static com.sun.grid.jgdi.util.JGDIShell.getResourceString;
import java.util.LinkedList;
import java.util.List;
import java.util.logging.Level;

/**
 *
 */
@CommandAnnotation(value = "qmod")
public class QModCommand extends AbstractCommand {

    public void run(String[] args) throws Exception {

        if (args.length == 0) {
            out.println(getUsage());
        }

        if (jgdi == null) {
            throw new IllegalStateException("Not connected");
        }

        boolean force = false;

        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("-help")) {
                out.println(getUsage());
                return;
            } else if (args[i].equals("-cj")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("job_wc_queue_list is missing");
                }
                String[] jobs = parseJobWCQueueList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.clearJobsWithAnswer(jobs, force, answers);
                printAnswers(answers);
                break;
            } else if (args[i].equals("-c")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("job_wc_queue_list is missing");
                }
                String[] queues = parseJobWCQueueList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.clearQueuesWithAnswer(queues, force, answers);
                printAnswers(answers);
            } else if (args[i].equals("-s")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("job_wc_queue_list is missing");
                }
                String[] queues = parseJobWCQueueList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.suspendWithAnswer(queues, force, answers);
                printAnswers(answers);
            } else if (args[i].equals("-us")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("job_wc_queue_list is missing");
                }
                String[] queues = parseJobWCQueueList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.unsuspendWithAnswer(queues, force, answers);
                printAnswers(answers);
            } else if (args[i].equals("-sj")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("job_list is missing");
                }
                String[] jobs = parseJobList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.suspendJobsWithAnswer(jobs, force, answers);
                printAnswers(answers);
            } else if (args[i].equals("-usj")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("job_list is missing");
                }
                String[] jobs = parseJobList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.unsuspendJobsWithAnswer(jobs, force, answers);
                printAnswers(answers);
            } else if (args[i].equals("-sq")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("job_wc_queue_list is missing");
                }
                String[] queues = parseWCQueueList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.suspendQueuesWithAnswer(queues, force, answers);
                printAnswers(answers);
            } else if (args[i].equals("-uq")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("job_wc_queue_list is missing");
                }
                String[] queues = parseWCQueueList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.unsuspendQueuesWithAnswer(queues, force, answers);
                printAnswers(answers);
            } else if (args[i].equals("-r")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("job_wc_queue_list is missing");
                }
                String[] jobs = parseJobWCQueueList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.rescheduleWithAnswer(jobs, force, answers);
                printAnswers(answers);
            } else if (args[i].equals("-rj")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("job_wc_queue_list is missing");
                }
                String[] jobs = parseJobList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.rescheduleJobsWithAnswer(jobs, force, answers);
                printAnswers(answers);
            } else if (args[i].equals("-cq")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("wc_queue_list is missing");
                }
                String[] queues = parseWCQueueList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.clearQueuesWithAnswer(queues, force, answers);
                printAnswers(answers);
                break;
            } else if (args[i].equals("-rq")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("wc_queue_list is missing");
                }
                String[] queues = parseWCQueueList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.rescheduleQueuesWithAnswer(queues, force, answers);
                printAnswers(answers);
                break;
            } else if (args[i].equals("-d")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("wc_queue_list is missing");
                }
                String[] queues = parseWCQueueList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.disableQueuesWithAnswer(queues, force, answers);
                printAnswers(answers);
                break;
            } else if (args[i].equals("-e")) {
                i++;
                if (i >= args.length) {
                    throw new IllegalArgumentException("wc_queue_list is missing");
                }
                String[] queues = parseWCQueueList(args[i]);
                List<JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
                jgdi.enableQueuesWithAnswer(queues, force, answers);
                printAnswers(answers);
                break;
            } else if (args[i].equals("-f")) {
                force = true;
            } else {
                out.println(getUsage());
                throw new IllegalArgumentException("ERROR! invalid option argument \"" + args[i] + "\"");
            }
        }
    }

    public String[] parseWCQueueList(String arg) {
        String[] ret = arg.split(",");
        if (getShell().getLogger().isLoggable(Level.FINE)) {
            StringBuilder buf = new StringBuilder();
            buf.append("wc_queue_list [");
            for (int i = 0; i < ret.length; i++) {
                if (i > 0) {
                    buf.append(", ");
                }
                buf.append(ret[i]);
            }
            buf.append("]");
            getShell().getLogger().fine(buf.toString());
        }
        return ret;
    }

    public String[] parseJobWCQueueList(String arg) {
        String[] ret = arg.split(",");
        if (getShell().getLogger().isLoggable(Level.FINE)) {
            StringBuilder buf = new StringBuilder();
            buf.append("job_wc_queue_list [");
            for (int i = 0; i < ret.length; i++) {
                if (i > 0) {
                    buf.append(", ");
                }
                buf.append(ret[i]);
            }
            buf.append("]");
            getShell().getLogger().fine(buf.toString());
        }
        return ret;
    }

    public String[] parseJobList(String arg) {
        String[] ret = arg.split(",");
        if (getShell().getLogger().isLoggable(Level.FINE)) {
            StringBuilder buf = new StringBuilder();
            buf.append("job_list [");
            for (int i = 0; i < ret.length; i++) {
                if (i > 0) {
                    buf.append(", ");
                }
                buf.append(ret[i]);
            }
            buf.append("]");
            getShell().getLogger().fine(buf.toString());
        }
        return ret;
    }
}
