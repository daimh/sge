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
package com.sun.grid.jgdi.util.shell;

import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.configuration.AdvanceReservation;
import com.sun.grid.jgdi.configuration.AdvanceReservationImpl;
import com.sun.grid.jgdi.configuration.xml.XMLUtil;
import java.util.ArrayList;
import java.util.List;

import static com.sun.grid.jgdi.util.JGDIShell.getResourceString;
import static com.sun.grid.jgdi.util.shell.Util.*;

@CommandAnnotation("qrstat")
public class QrStatCommand extends AnnotatedCommand {
    List<String> userList = null;
    List<Integer> arList = null;
    
    boolean xml=false;
    boolean explain=false;
    
    public void run(String[] args) throws Exception {
        clear();
        
        //parse the option
        parseAndInvokeOptions(args);
        
        boolean arlist = !arList.isEmpty();
        //Let's take ar_list and look for candidates to delete
        @SuppressWarnings(value = "unchecked")
        List<AdvanceReservation> ars = (List<AdvanceReservation>) jgdi.getAdvanceReservationList();
        //Filter out just the ars in the arList
        if (ars.size() > 0) {
            if (arList.isEmpty()) {
                out.println("ar-id   name       owner        state start at             end at               duration");
                out.println("----------------------------------------------------------------------------------------");
            }
            for (AdvanceReservation ar : ars) {
                if (!arlist) {
                    if (userList.isEmpty() || userList.contains(ar.getOwner())) {
                        if (xml) {
                            XMLUtil.write(ar, out);
                        } else {
                            out.printf("%1$7d %2$-10.10s %3$-12.12s %4$-5.5s %5$-20.20s %6$-20.20s %7$s\n", ar.getId(), ar.getName() == null ? "" : ar.getName(), ar.getOwner(), ar.getStateAsString(), getDateAndTimeAsString(ar.getStartTime()), getDateAndTimeAsString(ar.getEndTime()), getTimeAsString(ar.getDuration()));
                        }
                    }
                } else {
                    if (arList.remove((Object) ar.getId()) && (userList.isEmpty() || userList.contains(ar.getOwner()))) {
                        AdvanceReservationImpl ari = (AdvanceReservationImpl) ar;
                        if (xml) {
                            XMLUtil.write(ari, out);
                        } else {
                            out.println(ari.dump());
                        }
                    }
                }
            }
        }
        
        if (!arList.isEmpty()) {
            out.println("Following advance reservations do not exist:");
            out.println(arList);
        }
    }
    
    //[-ar ar_id_list]                         show advance reservation information
    @OptionAnnotation(value="-ar",extra=OptionAnnotation.MAX_ARG_VALUE)
    public void setAdvanceReservationList(final OptionInfo oi) throws JGDIException {
        try {
            arList.add(Integer.parseInt(oi.getFirstArg()));
        } catch (NumberFormatException ex) {
            throw new IllegalArgumentException("error: ERROR! invalid id, must be an unsigned integer");
        }
    }
    //[-help]                                  print this help
    @OptionAnnotation(value="-help",min=0)
    public void printUsage(final OptionInfo oi) throws JGDIException {
        out.println(getUsage());
        // To avoid the continue of the command
        throw new AbortException();
    }
    //[-u user_list]                           all advance reservations of users specified in list
    @OptionAnnotation(value="-u",extra=OptionAnnotation.MAX_ARG_VALUE)
    public void setUserList(final OptionInfo oi) throws JGDIException {
        userList.add(oi.getFirstArg());
    }
    
    //[-xml]                                   display the information in XML-Format
    @OptionAnnotation(value="-xml",min=0)
    public void setXml(final OptionInfo oi) throws JGDIException {
        xml=true;
    }
    //[-explain]                               show reason for error state
    @OptionAnnotation(value="-explain",min=0)
    public void setExplain(final OptionInfo oi) throws JGDIException {
        explain=true;
        throw new UnsupportedOperationException("Option -explain is not implemented");
    }
    
    private void clear() {
        userList = new ArrayList<String>();
        arList = new ArrayList<Integer>();
    }
}
