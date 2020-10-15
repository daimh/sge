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
import java.awt.event.*;
import javax.swing.*;
import com.sun.grid.jam.queue.entry.ComputeInfoEntry;

/**
 * UI panel for entering compute resource parameters for matching against
 * queue services' underlying compute resources.
 *
 * @version 1.8, 09/22/00
 *
 * @author Eric Sharakan
 */ 
public class ComputeInfoPanel
  extends JPanel
  implements Serializable
{
  private transient ComputeInfoEntry attrComputeInfo;
  private transient JTextField mfgrText;
  private transient JTextField modelText;
  private transient JTextField archText;
  private transient JTextField osText;
  private transient JTextField osRevText;
  private transient JTextField osArchText;
  private transient JTextField procCountText;
  private transient JTextField tilingText;

  private transient AppAgent agent;

  public ComputeInfoPanel()
  {
  }

  public ComputeInfoPanel(ComputeInfoEntry computeAttr, AppAgent aai)
  {
    super(new GridLayout(4, 2));

    attrComputeInfo = computeAttr;
    agent = aai;

    // Display Compute Info for selection.  This should really be implemented
    // as a property editor of ComputeInfo. XXX

    // Manufacturer
    JPanel mfgrSpecifier = new JPanel();
    mfgrSpecifier.setBorder(BorderFactory.createTitledBorder("Manufacturer"));
    mfgrText = new JTextField(6);
    mfgrSpecifier.add(mfgrText);

	// Add listener
    mfgrText.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ae)
	{
	  if (mfgrText.getText().equals(""))
	    attrComputeInfo.manufacturer = null;
	  else
	    attrComputeInfo.manufacturer = mfgrText.getText();
	  agent.updateQueueList();
	}
    });

    // Model
    JPanel modelSpecifier = new JPanel();
    modelSpecifier.setBorder(BorderFactory.createTitledBorder("Model"));
    modelText = new JTextField(6);
    modelSpecifier.add(modelText);

	// Add listener
    modelText.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ae)
	{
	  if (modelText.getText().equals(""))
	    attrComputeInfo.model = null;
	  else
	    attrComputeInfo.model = modelText.getText();
	  agent.updateQueueList();
	}
    });

    // Architecture
    JPanel archSpecifier = new JPanel();
    archSpecifier.setBorder(BorderFactory.createTitledBorder("Architecture"));
    archText = new JTextField(6);
    archSpecifier.add(archText);

	// Add listener
    archText.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ae)
	{
	  if (archText.getText().equals(""))
	    attrComputeInfo.architecture = null;
	  else
	    attrComputeInfo.architecture = archText.getText();
	  agent.updateQueueList();
	}
    });

    // OS
    JPanel osSpecifier = new JPanel();
    osSpecifier.setBorder(BorderFactory.createTitledBorder("OS"));
    osText = new JTextField(6);
    osSpecifier.add(osText);

	// Add listener
    osText.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ae)
	{
	  if (osText.getText().equals(""))
	    attrComputeInfo.osName = null;
	  else
	    attrComputeInfo.osName = osText.getText();
	  agent.updateQueueList();
	}
    });

    // OS Rev
    JPanel osRevSpecifier = new JPanel();
    osRevSpecifier.setBorder(BorderFactory.createTitledBorder("OS Revision"));
    osRevText = new JTextField(6);
    osRevSpecifier.add(osRevText);

	// Add listener
    osRevText.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ae)
	{
	  if (osRevText.getText().equals(""))
	    attrComputeInfo.osRev = null;
	  else
	    attrComputeInfo.osRev = osRevText.getText();
	  agent.updateQueueList();
	}
    });

    // OS Architecture
    JPanel osArchSpecifier = new JPanel();
    osArchSpecifier.setBorder(BorderFactory.createTitledBorder("OS Architecture"));
    osArchText = new JTextField(6);
    osArchSpecifier.add(osArchText);

	// Add listener
    osArchText.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ae)
	{
	  if (osArchText.getText().equals(""))
	    attrComputeInfo.osArch = null;
	  else
	    attrComputeInfo.osArch = osArchText.getText();
	  agent.updateQueueList();
	}
    });

	// Process count
	JPanel procCountSpecifier = new JPanel();
	procCountSpecifier.setBorder(BorderFactory.createTitledBorder("Process count"));
	procCountText = new JTextField(6);
	procCountSpecifier.add(procCountText);

	// Add listener
	procCountText.addActionListener(new ActionListener() {
	  public void actionPerformed(ActionEvent ae)
	    {
	      try {
		attrComputeInfo.processCount = new Integer(procCountText.getText());
	      } catch (NumberFormatException nfe) {
		attrComputeInfo.processCount = null;
	      }
	      agent.updateQueueList();
	    }
	});

	// Tiling factor
	JPanel tilingSpecifier = new JPanel();
	tilingSpecifier.setBorder(BorderFactory.createTitledBorder("Tiling Factor"));
	tilingText = new JTextField(6);
	tilingSpecifier.add(tilingText);

	// Add listener
	tilingText.addActionListener(new ActionListener() {
	  public void actionPerformed(ActionEvent ae)
	    {
	      try {
		attrComputeInfo.tilingFactor = new Integer(tilingText.getText());
	      } catch (NumberFormatException nfe) {
		attrComputeInfo.tilingFactor = null;
	      }
	      agent.updateQueueList();
	    }
	});

    // Put all the Compute Info selection panels together
    add(mfgrSpecifier);
    add(modelSpecifier);
    add(archSpecifier);
    add(osSpecifier);
    add(osRevSpecifier);
    add(osArchSpecifier);
    add(procCountSpecifier);
    add(tilingSpecifier);
  }
}
