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
 * Base class for UI panels for entering application and resource
 * parameters.  Application-specific panels for entering Application
 * parameters should be added to topPanel by appropriate subclasses.
 *
 * Resource panels are likewise defined in appropriate subclasses,
 * and should be placed into bottomPanel.
 *
 * This class contains generic panels (implemented as inner classes) which
 * subclasses can utilize if they wish (or they can build their own UIs).
 *
 * @version 1.13, 09/22/00
 *
 * @author Eric Sharakan
 */
public class AppParamsPanel
  extends JPanel
  implements Serializable
{
  protected transient JPanel topPanel;
  protected transient JPanel bottomPanel;
  protected transient AppAgent aai;
  protected transient AppParamsInterface params;

  public AppParamsPanel()
  {
  }

  public AppParamsPanel(AppParamsInterface appParams,
			AppAgent agentRef,
			GridLayout grid)
  {
    super(new BorderLayout());

    topPanel = new JPanel(grid);
    bottomPanel = new JPanel(new BorderLayout());

    params = appParams;
    aai = agentRef;

    // Final assembly
    add(topPanel, BorderLayout.NORTH);
    add(bottomPanel, BorderLayout.SOUTH);
  }

  // Inner classes


  /**
   * Panel for accepting application or script pathname, and setting
   * the name field of AppParams.
   */
  class PathnamePanel
  extends JPanel
  {
    protected transient JTextField pathname;

    public PathnamePanel()
    {
      setBorder(BorderFactory.createTitledBorder("Command"));
      pathname = new JTextField(10);
      add(pathname);
      Button choose = new Button("Choose");
      add(choose);

      // Add listeners
      pathname.addFocusListener(new FocusAdapter() {
	public void focusLost(FocusEvent fe)
	  {
	    if (pathname.getText().equals(""))
	      params.setName(null);
	    else
	      params.setName(pathname.getText());
	  }
      });
      choose.addActionListener(new ActionListener() {
	public void actionPerformed(ActionEvent ae)
	  {
	    JFileChooser fileChooser = new JFileChooser();
	    fileChooser.setDialogType(JFileChooser.OPEN_DIALOG);
	    fileChooser.setApproveButtonText("Select");
	    fileChooser.setDialogTitle("Command chooser");
	    if (fileChooser.showOpenDialog((Component)ae.getSource()) ==
		JFileChooser.APPROVE_OPTION) {
	      File file = fileChooser.getSelectedFile();
	      if(file != null && file.isFile()) {
		pathname.setText(file.getPath());
		params.setName(file.getPath());
	      }
	    }
	  }
      });
    }
  }

  /**
   * Panel for accepting application arguments, and setting
   * the name args of AppParams.
   */
  class ArgsPanel
  extends JPanel
  {
    protected transient JTextField args;

    public ArgsPanel()
    {
      setBorder(BorderFactory.createTitledBorder("Parameters"));
      args = new JTextField(10);
      add(args);

      // Add listener
      args.addFocusListener(new FocusAdapter() {
	public void focusLost(FocusEvent ae)
	  {
	    if (args.getText().equals(""))
	      params.setArgs(null);
	    else
	      params.setArgs(new String[] { args.getText() } );
	  }
      });
    }
  }

  /**
   * Panel for accepting URL pathname, and setting
   * the files field of AppParams.
   */
  class URLPanel
  extends JPanel
  {
    protected transient JTextField fileURL;

    public URLPanel()
    {
      setBorder(BorderFactory.createTitledBorder("File URL"));
      fileURL = new JTextField(10);
      add(fileURL);

      // Add listener
      fileURL.addFocusListener(new FocusAdapter() {
	public void focusLost(FocusEvent ae)
	  {
	    if (fileURL.getText().equals(""))
	      params.setFiles(null);
	    else
	      try {
	      params.setFiles(new URL[] { new URL(fileURL.getText()) } );
	    } catch (MalformedURLException mue) {
	      mue.printStackTrace();
	    }
	  }
      });
    }
  }

  /**
   * Panel for accepting boolean options (via checkboxes), and setting
   * the apropriate fields of AppParams.  Currently, the only Boolean is
   * showOutput.
   */
  class BooleansPanel
  extends JPanel
  {
    public BooleansPanel()
    {
      super(new GridLayout(3, 1));
      setBorder(BorderFactory.createTitledBorder("Selections"));
      JCheckBox showOutput = new JCheckBox("Stdout to console window");
      add(showOutput);

      // Add listeners
      // Use individual listener (anonymous) classes here, since
      // there doesn't seem to be a way to tell which JCheckBox
      // was modified in an ItemListener.
      showOutput.addItemListener(new ItemListener() {
	public void itemStateChanged(ItemEvent ae)
	  {
	    if (ae.getStateChange() == ItemEvent.SELECTED)
	      params.setShowOutput(true);
	    else
	      params.setShowOutput(false);
	  }
      });
    }
  }
}
