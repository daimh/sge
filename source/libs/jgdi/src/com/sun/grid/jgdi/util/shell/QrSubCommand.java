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

import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.configuration.ARA;
import com.sun.grid.jgdi.configuration.ARAImpl;
import com.sun.grid.jgdi.configuration.AdvanceReservation;
import com.sun.grid.jgdi.configuration.AdvanceReservationImpl;
import com.sun.grid.jgdi.configuration.ComplexEntry;
import com.sun.grid.jgdi.configuration.ComplexEntryImpl;
import com.sun.grid.jgdi.configuration.JGDIAnswer;
import com.sun.grid.jgdi.configuration.MailReceiver;
import com.sun.grid.jgdi.configuration.MailReceiverImpl;
import com.sun.grid.jgdi.configuration.Range;
import com.sun.grid.jgdi.configuration.RangeImpl;
import com.sun.grid.jgdi.monitoring.filter.ResourceFilter;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import static com.sun.grid.jgdi.util.JGDIShell.getResourceString;
import static com.sun.grid.jgdi.util.shell.Util.*;

/**
 *
 */
@CommandAnnotation(value = "qrsub")
public class QrSubCommand extends AnnotatedCommand {
    AdvanceReservation ar = null;
    
    public void run(String[] args) throws Exception {
        
        // new ar object
        ar = new AdvanceReservationImpl(false);
        // parse arguments and fill the ar object
        parseAndInvokeOptions(args);
        
        // Send the ar object to qmaster
        List<JGDIAnswer> answers = new ArrayList<JGDIAnswer>();
        jgdi.addAdvanceReservationWithAnswer(ar, answers);
        printAnswers(answers);
    }
    
    //[-a date_time]                           request a start time
    @OptionAnnotation("-a")
    public void setStartTime(final OptionInfo oi) throws JGDIException {
        ar.setStartTime(getDateTimeAsInt(oi.getFirstArg()));
    }
    
    //[-A account_string]                      account string in accounting record
    @OptionAnnotation("-A")
    public void setAaccountString(final OptionInfo oi) throws JGDIException {
        ar.setAccount(oi.getFirstArg());
    }
    
    //[-ckpt ckpt-name]                        request checkpoint method
    @OptionAnnotation("-ckpt")
    public void setCheckPoint(final OptionInfo oi) throws JGDIException {
        ar.setCheckpointName(oi.getFirstArg());
    }
    //[-d time]                                duration of time window
    @OptionAnnotation("-d")
    public void setDuration(final OptionInfo oi) throws JGDIException {
        ar.setDuration(getTimeAsInt(oi.getFirstArg()));
        oi.optionDone();
    }
    //[-e date_time]                           request an end time
    @OptionAnnotation("-e")
    public void setEndTime(final OptionInfo oi) throws JGDIException {
        ar.setEndTime(getDateTimeAsInt(oi.getFirstArg()));
    }
    //[-he  y[es]|n[o]]                        enable/disable hard error handling
    @OptionAnnotation("-he")
    public void setHardError(final OptionInfo oi) throws JGDIException {
        ar.setErrorHandling(getYesNoAsInt(oi.getFirstArg()));
    }
    //[-help]                                  print this help
    @OptionAnnotation(value = "-help", min = 0)
    public void printUsage(final OptionInfo oi) throws JGDIException {
        out.println(getUsage());
        // To avoid the continue of the command
        throw new AbortException();
    }
    //[-l resource_list]                       request the given resources
    @OptionAnnotation(value="-l",extra=OptionAnnotation.MAX_ARG_VALUE)
    public void setResourceList(final OptionInfo oi) throws JGDIException {
        ResourceFilter resourceList =  ResourceFilter.parse(oi.getArgsAsString());
        @SuppressWarnings("unchecked")
        Set<String> names = resourceList.getResourceNames();
        for(String name: names){
            ComplexEntry ce = new ComplexEntryImpl(name);
            ce.setStringval(resourceList.getResource(name));
            ar.addResource(ce);
        }
        oi.optionDone();
    }
    //[-m mail_options]                        define mail notification events
    @OptionAnnotation(value="-m",extra=OptionAnnotation.MAX_ARG_VALUE)
    public void setMailOption(final OptionInfo oi) throws JGDIException {
        ar.setMailOptions(getMailOptionsAsInt(oi.getFirstArg()));
    }
    //[-masterq wc_queue_list]                 bind master task to queue(s)
    @OptionAnnotation(value="-masterq",extra=OptionAnnotation.MAX_ARG_VALUE)
    public void setMasterQueue(final OptionInfo oi) throws JGDIException {
        for(String amasterQueue: oi.getArgs()){
            ar.addMasterQueue(amasterQueue);
        }
        oi.optionDone();
    }
    //[-now y[es]|n[o]]                        start job immediately or not at all
    @OptionAnnotation("-now")
    public void setNow(final OptionInfo oi) throws JGDIException {
        ar.setType(getYesNoAsInt(oi.getFirstArg()));
    }
    //[-M mail_list]                           notify these e-mail addresses
    @OptionAnnotation(value="-M",extra=OptionAnnotation.MAX_ARG_VALUE)
    public void setMailList(final OptionInfo oi) throws JGDIException {
        MailReceiver mr = new MailReceiverImpl(oi.getFirstArg());
        ar.addMail(mr);
    }
    //[-N name]                                specify job name
    @OptionAnnotation("-N")
    public void setJobName(final OptionInfo oi) throws JGDIException {
        ar.setName(oi.getFirstArg());
    }
    //[-pe pe-name slot_range]                 request slot range for parallel jobs
    @OptionAnnotation(value="-pe",min=2)
    public void setPE(final OptionInfo oi) throws JGDIException {
        ar.setPe(oi.getFirstArg());
        String range[] = oi.getFirstArg().split("[-]");
        Range apeRange = new RangeImpl();
        if(range[0] != null && range[0].length()>0){
            apeRange.setMin(Integer.parseInt(range[0]));
        }
        if(range.length>1 && range[1] != null && range[1].length()>0){
            apeRange.setMax(Integer.parseInt(range[1]));
        }
        ar.addPeRange(apeRange);
    }
    //[-q wc_queue_list]                       print information on given queue
    @OptionAnnotation(value="-q",extra=OptionAnnotation.MAX_ARG_VALUE)
    public void setQueueList(final OptionInfo oi) throws JGDIException {
        for(String queue: oi.getArgs()){
            ar.addQueue(queue);
        }
        oi.optionDone();
    }
    
    //[-u user_list]
    @OptionAnnotation(value = "-u", extra = OptionAnnotation.MAX_ARG_VALUE)
    public void setUserList(final OptionInfo oi) throws JGDIException {
        for (String user : oi.getArgs()) {
            if (user.startsWith("!")) {
                ARA xacl = new ARAImpl(user.substring(1));
                ar.addXacl(xacl);
            } else {
                ARA acl = new ARAImpl(user);
                ar.addAcl(acl);
            }
        }
        oi.optionDone();
    }
    
}