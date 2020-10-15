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

import com.sun.grid.jam.queue.entry.QueueInfo;
import com.sun.grid.jam.queue.entry.QueueStatus;

/**
 * UI panel for entering queue parameters for matching against
 * queue services' attributes.
 *
 * @version 1.2, 09/22/00
 *
 * @author Eric Sharakan
 */ 
public class QueueInfoPanel
  extends JPanel
  implements Serializable
{
  private transient AppParamsInterface params;
  private transient AppAgent aai;
  private transient JTextField rmsText;

  public QueueInfoPanel()
  {
  }

  public QueueInfoPanel(AppParamsInterface api, AppAgent aa)
  {
    super(new GridLayout(4, 1));

    params = api;
    aai = aa;

    // QueueStatus
    JPanel queueInfoStatusPanel = new JPanel(new BorderLayout());
    queueInfoStatusPanel.setBorder(BorderFactory.createTitledBorder("Queue Status"));
    JRadioButton queueStatusOpen = new JRadioButton("OPEN");
    JRadioButton queueStatusClosed = new JRadioButton("CLOSED");
    JRadioButton queueStatusEnabled = new JRadioButton("ENABLED");
    JRadioButton queueStatusDisabled = new JRadioButton("DISABLED");
    JRadioButton queueStatusAny = new JRadioButton("ANY");
    ButtonGroup queueStatusGroup = new ButtonGroup();
    queueStatusGroup.add(queueStatusOpen);
    queueStatusGroup.add(queueStatusClosed);
    queueStatusGroup.add(queueStatusEnabled);
    queueStatusGroup.add(queueStatusDisabled);
    queueStatusGroup.add(queueStatusAny);

    JCheckBox needDedicatedQueue = new JCheckBox("Require Dedicated Queue");

    JPanel queueInfoStatusRadioPanel = new JPanel();
    queueInfoStatusRadioPanel.add(queueStatusOpen);
    queueInfoStatusRadioPanel.add(queueStatusClosed);
    queueInfoStatusRadioPanel.add(queueStatusEnabled);
    queueInfoStatusRadioPanel.add(queueStatusDisabled);
    queueInfoStatusRadioPanel.add(queueStatusAny);

    queueInfoStatusPanel.add(queueInfoStatusRadioPanel, BorderLayout.NORTH);
    queueInfoStatusPanel.add(needDedicatedQueue, BorderLayout.CENTER);

	  // Inner class to handle events
    class QueueStatusEvents implements ActionListener
    {
      public void actionPerformed(ActionEvent ae)
      {
	if (ae.getActionCommand().equals("OPEN")) {
	  params.getQueueInfo().condition = QueueStatus.QUEUE_OPEN;
	}
	if (ae.getActionCommand().equals("CLOSED")) {
	  params.getQueueInfo().condition = QueueStatus.QUEUE_CLOSE;
	}
	if (ae.getActionCommand().equals("ENABLED")) {
	  params.getQueueInfo().condition = QueueStatus.QUEUE_ENABLED;
	}
	if (ae.getActionCommand().equals("DISABLED")) {
	  params.getQueueInfo().condition = QueueStatus.QUEUE_DISABLED;
	}
	if (ae.getActionCommand().equals("ANY")) {
	  params.getQueueInfo().condition = null;
	}
	aai.updateQueueList();
      }
    }

    // Add listeners
    QueueStatusEvents statusListener = new QueueStatusEvents();
    queueStatusOpen.addActionListener(statusListener);
    queueStatusClosed.addActionListener(statusListener);
    queueStatusEnabled.addActionListener(statusListener);
    queueStatusDisabled.addActionListener(statusListener);
    queueStatusAny.addActionListener(statusListener);

      // Dedicated queue selector gets its own listener
    needDedicatedQueue.addItemListener(new ItemListener() {
      public void itemStateChanged(ItemEvent ae)
	{
	  if (ae.getStateChange() == ItemEvent.SELECTED)
	    params.getQueueInfo().dedicated = new Boolean(true);
	  else
	    params.getQueueInfo().dedicated = null;
	  aai.updateQueueList();
	}
    });
	 
	
    // Batch/Interactive
    JPanel queueInfoBatchIntPanel = new JPanel();
    queueInfoBatchIntPanel.setBorder(BorderFactory.createTitledBorder("Job Type"));
    JRadioButton batchJob;
    JRadioButton interactiveJob;
    batchJob = new JRadioButton("Batch");
    interactiveJob = new JRadioButton("Interactive");
    ButtonGroup jobTypeGroup = new ButtonGroup();
    jobTypeGroup.add(batchJob);
    jobTypeGroup.add(interactiveJob);
    queueInfoBatchIntPanel.add(batchJob);
    queueInfoBatchIntPanel.add(interactiveJob);

    // Inner class to handle events
    class JobTypeEvents implements ActionListener
    {
      public void actionPerformed(ActionEvent ae)
      {
	if (ae.getActionCommand().equals("Batch")) {
	  params.getQueueInfo().batch = new Boolean(true);
	  params.getQueueInfo().interactive = null;
	}
	if (ae.getActionCommand().equals("Interactive")) {
	  params.getQueueInfo().interactive = new Boolean(true);
	  params.getQueueInfo().batch = null;
	}
	aai.updateQueueList();
      }
    }

    // Add listeners
    JobTypeEvents jobTypeListener = new JobTypeEvents();
    batchJob.addActionListener(jobTypeListener);
    interactiveJob.addActionListener(jobTypeListener);
	 
    // Resource Manager
    JPanel rmsSpecifier = new JPanel();
    rmsSpecifier.setBorder(BorderFactory.createTitledBorder("Resource Manager"));
    rmsText = new JTextField(12);
    rmsSpecifier.add(rmsText);

	// Add listener
    rmsText.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ae)
	{
	  if (rmsText.getText().equals(""))
	    params.getQueueInfo().rms = null;
	  else
	    params.getQueueInfo().rms = rmsText.getText();
	  aai.updateQueueList();
	}
    });

    // Put all the Queue Info selection panels together
    add(queueInfoStatusPanel);
    add(queueInfoBatchIntPanel);
    add(rmsSpecifier);
  }
}
