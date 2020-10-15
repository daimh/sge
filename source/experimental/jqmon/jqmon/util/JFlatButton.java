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


/* Swing 1.0.x 
import com.sun.java.swing.*;
*/

/* Swing 1.1 */
import javax.swing.*;

import java.awt.*;
import java.awt.event.*;

public class JFlatButton extends JButton {
	
	/** Creates a button with no set text or icon */
	public JFlatButton() {
		super();
		init();
	}

	/** Creates a button with an icon */
	public JFlatButton(Icon icon) {
		super(icon);
		init();
	}

	/** Creates a button with text */
	public JFlatButton(String text) {
		super(text);
		init();
	}

	/** Creates a button with text and icon */
	public JFlatButton(String text, Icon icon) {
		super(text,icon);
		init();
	}

	/** Inits the JFLatButton */
	private void init() {
		MouseListener mouse = new MouseAdapter() {
			public void mouseEntered(MouseEvent e) {
				if (isEnabled()) {
					setBorderPainted(true);
				}
			}

			public void mouseExited(MouseEvent e) {
				setBorderPainted(false);
			}
		};

		addMouseListener(mouse);
		setBorderPainted(false);
		setFocusPainted(false);
	}
}
		
