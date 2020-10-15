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
package jqmon.debug;

import java.awt.*;
import java.awt.event.*;

/* Swing 1.0.x 
import com.sun.java.swing.*;
import com.sun.java.swing.event.*;
import com.sun.java.swing.border.*;
*/

/* Swing 1.1 */
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;


/**
  This class represents the internal JDebugWindow.

  @author    Michael Roehrl
  @version   0,01
*/

public class JDebugWindow extends JInternalFrame implements ChangeListener {

   /** The View */
   protected JDebugView view;

   /** The Model */
   protected JDPrintStringModel model;
   
	/** Die ScrollPane */
	protected JScrollPane sc = null;

	
   /** The default Constructor */
   public JDebugWindow(JDPrintStringModel m) {
      this(m,"Debug Window");
   }

   
   /** Give it a name */
   public JDebugWindow(JDPrintStringModel m, String title) {
      super(title,true,true,true,true);
      model = m;
      view = new JDebugView();
      initGUI();
      model.addChangeListener(this);
   }

   protected void finalize() {
      System.out.println("Destruktor");
      model.removeChangeListener(this);
   }


   /** Init the GUI */
   public void initGUI() {
      
      sc = new JScrollPane();
      sc.setPreferredSize(new Dimension(600,400));
      sc.setMinimumSize(new Dimension(100,50));
      sc.getViewport().add(view);
      sc.setBorder(new BevelBorder(BevelBorder.LOWERED));
      sc.setVisible(true);
      view.setEditable(false);
		view.setFont(new Font("Courier",Font.PLAIN,12));
      getContentPane().add("Center", sc);
      show();
		setMaximizable(true);
   }


   public void stateChanged(ChangeEvent e) {
      view.changed(model.getJDPrintString());
		sc.paintImmediately(sc.getX(), sc.getY(), sc.getWidth(), sc.getHeight());
		JScrollBar sb = sc.getVerticalScrollBar();
		sb.paintImmediately(sb.getX(), sb.getY(), sb.getWidth(), sb.getHeight());
   }
}
   
