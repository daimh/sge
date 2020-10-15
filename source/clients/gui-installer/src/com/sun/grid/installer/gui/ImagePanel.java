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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.installer.gui;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Image;

import javax.swing.ImageIcon;
import javax.swing.JPanel;

import com.izforge.izpack.gui.IzPanelLayout;
import java.awt.Color;

public class ImagePanel extends JPanel {

	private Image image = null;
	
	public ImagePanel(String imagePath) {
		this(new ImageIcon(imagePath));
	}
	
	public ImagePanel(ImageIcon imageIcon) {
		super(new IzPanelLayout());
		this.image = imageIcon.getImage();
		
		Dimension size = new Dimension(image.getWidth(null), image.getHeight(null) + 1000);
        setMinimumSize(size);
		setPreferredSize(size);
        setMaximumSize(new Dimension(Integer.MAX_VALUE, Integer.MAX_VALUE));

        setBackground(Color.white);
	}
	
	@Override
	protected void paintComponent(Graphics g) {
        g.setColor(Color.white);
        g.fillRect(0, 0, image.getWidth(null),image.getHeight(null) + 1000);
		g.drawImage(image, 0, 0, null);
	}
}
