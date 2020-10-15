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
package com.sun.grid.cull.ant;

import java.util.logging.Handler;
import java.util.logging.Level;
import org.apache.tools.ant.Project;

/**
 *
 */
public class AntLoggingHandler extends Handler {
   
   private Project project;
   
   public AntLoggingHandler(Project project) {
      this.project = project;
   }
   
   public void close() throws SecurityException {
   }
   
   public void flush() {
   }
   
   public void publish(java.util.logging.LogRecord lr) {
      
      int level = lr.getLevel().intValue();
      int antLevel = Project.MSG_INFO;
      if( level < Level.FINE.intValue() ) {
         antLevel = Project.MSG_DEBUG;
      } else if ( level >= Level.SEVERE.intValue() ) {
         antLevel = Project.MSG_ERR;
      } else if ( level == Level.FINE.intValue() ) {
         antLevel = Project.MSG_VERBOSE;
      } else if ( level == Level.WARNING.intValue() ) {
         antLevel = Project.MSG_WARN;
      }
      project.log(lr.getMessage(), antLevel);
   }
   
}
