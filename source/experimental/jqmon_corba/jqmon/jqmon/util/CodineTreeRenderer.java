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
package jqmon.util;


import javax.swing.*;
import javax.swing.tree.*;
import jcodine.*;


public class CodineTreeRenderer extends DefaultTreeCellRenderer {
   
   static ImageIcon defaultIcon;
   static ImageIcon calendarIcon;
   static ImageIcon checkpointIcon;
   static ImageIcon complexIcon;
   static ImageIcon hostIcon;
   static ImageIcon queueIcon;
   static ImageIcon selCalendarIcon;
   static ImageIcon selCheckpointIcon;
   static ImageIcon selComplexIcon;
   static ImageIcon selHostIcon;
   static ImageIcon selQueueIcon;
   static ImageIcon calendarsIcon;
   static ImageIcon checkpointsIcon;
   static ImageIcon complexesIcon;
   static ImageIcon hostsIcon;
   static ImageIcon queuesIcon;
   

   public CodineTreeRenderer() {
      defaultIcon    = new ImageIcon("../images/host.gif");
      
      calendarIcon   = new ImageIcon("../images/host.gif");
      checkpointIcon = new ImageIcon("../images/host.gif");
      complexIcon    = new ImageIcon("../images/host.gif");
      hostIcon       = new ImageIcon("../images/host.gif");
      queueIcon      = new ImageIcon("../images/queue.gif");
      selCalendarIcon   = new ImageIcon("../images/hostSel.gif");
      selCheckpointIcon = new ImageIcon("../images/hostSel.gif");
      selComplexIcon    = new ImageIcon("../images/hostSel.gif");
      selHostIcon       = new ImageIcon("../images/hostSel.gif");
      selQueueIcon      = new ImageIcon("../images/queueSel.gif");
      calendarsIcon   = new ImageIcon("../images/host.gif");
      checkpointsIcon = new ImageIcon("../images/host.gif");
      complexesIcon   = new ImageIcon("../images/host.gif");
      hostsIcon       = new ImageIcon("../images/host.gif");
      queuesIcon      = new ImageIcon("../images/queue.gif");
   }

   public java.awt.Component getTreeCellRendererComponent(JTree tree, Object value, boolean sel, boolean expanded, boolean leaf, int row, boolean hasFocus) {

      super.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
      
      setCodineIcon(value, sel, expanded, leaf);

      return this;
   }
   
   protected void setCodineIcon(Object value, boolean sel, boolean expanded, boolean leaf) {
      JCalendar   calendar    = new JCalendar(null);
      JCheckpoint checkpoint  = new JCheckpoint(null);
      JComplex    complex     = new JComplex(null);
      JHost       host        = new JHost(null);
      JQueue      queue       = new JQueue(null);
      
      DefaultMutableTreeNode node = ((DefaultMutableTreeNode)value);
      Object object = (Object)node.getUserObject();
      Class obclass = object.getClass();
      
      if ((object.toString()).equals("Calendars")) {
         setIcon(calendarsIcon);
      }
      else if ((object.toString()).equals("Checkpoints")) {
         setIcon(checkpointsIcon);
      }
      else if ((object.toString()).equals("Complexes")) {
         setIcon(complexesIcon);
      }
      else if ((object.toString()).equals("Hosts")) {
         setIcon(hostsIcon);
      }
      else if ((object.toString()).equals("Queues")) {
         setIcon(queuesIcon);
      }

      if (leaf) {
         if (obclass != null) {
            if (obclass.equals(calendar.getClass())) {
               setIcon((sel)? selCalendarIcon : calendarIcon);  
            }  
            else if (obclass.equals(checkpoint.getClass())) {
               setIcon((sel)? selCheckpointIcon : checkpointIcon);
            }
            else if (obclass.equals(complex.getClass())) {
               setIcon((sel)? selComplexIcon : complexIcon);
            }
            else if (obclass.equals(host.getClass())) {
               setIcon((sel)? selHostIcon : hostIcon);
            }
            else if (obclass.equals(queue.getClass())) {
               setIcon((sel)? selQueueIcon : queueIcon);
            }
            else {
               setIcon(defaultIcon);
            }
         }
      }
   }
}

