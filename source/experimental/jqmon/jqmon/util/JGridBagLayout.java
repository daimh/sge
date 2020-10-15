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

import java.awt.*;

public class JGridBagLayout extends GridBagLayout {

	public JGridBagLayout() {
		super();
	}

   // 
   // Constraint-Funktionen
   //
   // Sie dienen nur dazu, den Umgang mit dem doch recht komplexen
   // GridBagLayout zu erleichtern.
   //
   public void constrain(Container container, Component component,
                  int grid_x, int grid_y, int grid_width, int grid_height,
                  int fill, int anchor, double weight_x, double weight_y,
                  int top, int left, int bottom, int right)
   {
      GridBagConstraints c = new GridBagConstraints();
      c.gridx = grid_x;
      c.gridy = grid_y;
      c.gridwidth = grid_width;
      c.gridheight = grid_height;
      c.fill = fill;
      c.anchor = anchor;
      c.weightx = weight_x;
      c.weighty = weight_y;
      if (top + bottom + left + right > 0) {
         c.insets = new Insets(top, left, bottom, right);
      }

      ((GridBagLayout)container.getLayout()).setConstraints(component, c);
      container.add(component);
   }

   public void constrain(Container container, Component component,
                  int grid_x, int grid_y, int grid_width, int grid_height)
   {
      constrain(container, component, grid_x, grid_y, grid_width, grid_height,
            GridBagConstraints.NONE, GridBagConstraints.NORTHWEST,
            0.0, 0.0, 0, 0, 0, 0);
   }

   public void constrain(Container container, Component component,
                  int grid_x, int grid_y, int grid_width, int grid_height,
                  int top, int left, int bottom, int right)
   {
      constrain(container, component, grid_x, grid_y, grid_width, grid_height,
            GridBagConstraints.NONE, GridBagConstraints.NORTHWEST,
            0.0, 0.0, top, left, bottom, right);
   }
}
