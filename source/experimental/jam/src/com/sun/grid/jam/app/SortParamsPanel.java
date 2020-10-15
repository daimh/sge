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
 *   and/or Swiss Center for Scientific Computing
 * 
 *   Copyright: 2002 by Sun Microsystems, Inc.
 *   Copyright: 2002 by Swiss Center for Scientific Computing
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jam.app;

import java.io.*;
import java.awt.*;
import java.net.*;
import java.awt.event.*;
import javax.swing.*;

/**
 * UI panel for "sort" application.  Includes both
 * compute-level and queue-level resource parameeters.
 *
 * @version 1.4, 09/22/00
 *
 * @author Eric Sharakan
 */ 
public class SortParamsPanel
  extends AppParamsPanel
{
  // App-specific UI elements
  private transient JTabbedPane agentPane;
  private transient JPanel queueInfoPanel;
  private transient JPanel computeInfoPanel;
  private transient JPanel sortInputPanel;
  private transient JTextField sortFile;

  public SortParamsPanel()
  {
  }

  public SortParamsPanel(AppParamsInterface appParams,
			AppAgent agentRef)
  {
    super(appParams, agentRef, new GridLayout(1, 1));

    // Pre-defined parameters
    params.setName("SortApp");

    // Input file panel
    sortInputPanel = new JPanel();
    sortInputPanel.setBorder(BorderFactory.createTitledBorder("Unsorted File"));
    sortFile = new JTextField(32);
    params.setArgs(new String[] { sortFile.getText() } );
    sortInputPanel.add(sortFile);

    // Add listener
    sortFile.addFocusListener(new FocusAdapter() {
      public void focusLost(FocusEvent fe)
	{
	  if (sortFile.getText().equals(""))
	    params.setArgs(null);
	  else
	    params.setArgs(new String[] { sortFile.getText() } );
	}
    });

    // Now get QueueInfo; ComputeInfo panels
    agentPane = new JTabbedPane();
    computeInfoPanel = new ComputeInfoPanel(params.getComputeParams().getComputeInfoEntry(), aai);
    queueInfoPanel = new QueueInfoPanel(params, aai);

    agentPane.addTab("Queue", null, queueInfoPanel, "Queue parameter selection");
    agentPane.addTab("Compute", null, computeInfoPanel, "Compute parameter selection");

    // Now put it all together
    topPanel.add(sortInputPanel);
    bottomPanel.add(agentPane);
  }
}
