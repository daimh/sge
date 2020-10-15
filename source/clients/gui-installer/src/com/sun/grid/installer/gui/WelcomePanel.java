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

import java.io.IOException;

import javax.swing.JLabel;
import javax.swing.JPanel;

import com.izforge.izpack.gui.IzPanelLayout;
import com.izforge.izpack.gui.LayoutConstants;
import com.izforge.izpack.installer.InstallData;
import com.izforge.izpack.installer.InstallerFrame;
import com.izforge.izpack.installer.IzPanel;
import com.izforge.izpack.installer.ResourceManager;
import com.izforge.izpack.util.Debug;
import com.izforge.izpack.util.VariableSubstitutor;
import com.sun.grid.installer.util.Config;
import javax.swing.border.EmptyBorder;

/**
 * Welcome panel
 */
public class WelcomePanel extends IzPanel implements Config {
    
    /**
     * Constructor
     * @param parent The InstallerFrame
     * @param idata The InstallData
     */
    public WelcomePanel(InstallerFrame parent, InstallData idata) {
        super(parent, idata, new IzPanelLayout(IzPanelLayout.FILL_OUT_COLUMN_SIZE));
        
        buildPanel();
    }

    /**
     * Builds the panel
     */
    private void buildPanel() {
    	ImagePanel imagePanel = null;
    	ResourceManager rm = ResourceManager.getInstance();

        // get image
    	try {
			imagePanel = new ImagePanel(rm.getImageIconResource(WELCOME_IMAGE_RESOURCE));
		} catch (Exception e) {
			Debug.error(e);
		}
		
		JPanel textPanel = new JPanel(new IzPanelLayout());
        textPanel.setBorder(new EmptyBorder(5, 5, 5, 5));

		VariableSubstitutor vs = new VariableSubstitutor(idata.getVariables());
		JLabel textLabel = new JLabel(vs.substituteMultiple(parent.langpack.getString(WELCOME_TEXT_RESOURCE), null));
		textPanel.add(textLabel);
		
		if (imagePanel != null) {
			add(imagePanel, LayoutConstants.NEXT_COLUMN);
		}
		add(textPanel, LayoutConstants.NEXT_COLUMN);
		
		getLayoutHelper().completeLayout();
    }
}
