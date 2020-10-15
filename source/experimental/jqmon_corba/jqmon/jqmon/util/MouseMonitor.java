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
import javax.swing.border.*;

import java.awt.event.*;
import java.awt.*;


// MouseMonitor implements a MouseListener.
// it recognizes, when the mouse moves over a JComponent
// and then redraws the frame of that component
//
// @author Michael Roehrl
// @version 1.0

public class MouseMonitor extends MouseAdapter {
	
	// the frame that should be displayed when the mouse enters the component
	protected Border bMouseInComponent;
	
	// the frame that should be displayed when the mouse leaves the component
	protected Border bMouseNotInComponent;

	// contructs a Mousemonitor with a default frame
   // mouseInComponent: BevelBorder(BevelBorder.LOWERED)
	// mouseNotInComponent: BevelBorder(BevelBorder.RAISED);
	public MouseMonitor() {
		this(new BevelBorder(BevelBorder.LOWERED));
	}

	// contructs a Mousemonitor with the specified frame 
   // this frame is displayed when the mouse enters the component
   // and the default frame is displayed when the mouse leaves the component
	public MouseMonitor(Border mouseIsInComponent) {
		this(mouseIsInComponent, new BevelBorder(BevelBorder.RAISED));
	}

	// contructs a Mousemonitor with the specified frame 
	public MouseMonitor(Border mouseIsInComponent, 
			 			Border mouseIsNotInComponent) {
		bMouseInComponent    = mouseIsInComponent;
		bMouseNotInComponent = mouseIsNotInComponent;
	}

	// is invoked, when the mouse enters the component
	public void mouseEntered(MouseEvent e) {
		JComponent c = (JComponent)e.getSource();
		c.setBorder(bMouseInComponent);
	}

	// is invoked, when the mouse leaves the component
	public void mouseExited(MouseEvent e) {
		JComponent c = (JComponent)e.getSource();
		c.setBorder(bMouseNotInComponent);
	}

	//**************
	//* Properties *
	//**************

	public void setBorderEntered(Border b) {
		bMouseInComponent = b;
	}

	public Border getBorderEntered() {
		return bMouseInComponent;
	}

	public void setBorderExited(Border b) {
		bMouseNotInComponent = b;
	}

	public Border getBorderExited() {
		return bMouseNotInComponent;
	}
	
}
