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
package com.sun.grid.jam.job.ui;

import javax.swing.*;
import javax.swing.border.EtchedBorder;
import java.awt.*;
import java.awt.event.*;
import java.util.Date;
import java.util.StringTokenizer;
import java.io.BufferedReader;
import java.io.IOException;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import net.jini.core.event.RemoteEventListener;
import net.jini.core.event.RemoteEvent;
import net.jini.core.event.EventRegistration;
import net.jini.core.lease.Lease;
import com.sun.grid.jam.job.JobProxy;
import com.sun.grid.jam.job.JobStatus;
import com.sun.grid.jam.job.JobAction;
import com.sun.grid.jam.job.entry.JobInfo;
import com.sun.grid.jam.job.entry.JobParams;
import com.sun.grid.jam.job.entry.JobControlEntry;
import com.sun.grid.jam.util.RemoteInputStreamInterface;

/**
 * Defines a panel for controlling and 
 * monitoring a job
 * 
 *
 * @version 1.9, 12/04/00
 *
 * @author Nello Nellari
 * @author Rafal Piotrowski
 *
 */
public class JobMonitorJPanel
  extends JPanel
  implements ActionListener
{
  private JobProxy jobService;

  private final JPanel actionMasterPanel = new JPanel(new FlowLayout());
  private final JPanel actionPanel = new JPanel(new GridLayout(1, 0, 10, 10));

  private final JButton exitButton = new JButton("Exit");
  private final JButton startButton = new JButton("Start");
  private final JButton suspendButton = new JButton("Suspend");
  private final JButton resumeButton = new JButton("Resume");
  private final JButton stopButton = new JButton("Stop");
  private final JButton restartButton = new JButton("Resubmit");
  private final JButton killButton = new JButton("Kill");
  private final JButton removeButton = new JButton("Remove");
  
  private JTextField name;
  private JTextField queueName;
  private JTextField type;
  private JTextField jobID; 
  private JTextField userID;
  private JTextField groupID;

  private JTextField subName;
  private JTextField subID;
  
  private JTextField status;
  private JTextField action;
  
  private JTextField time;
  private JTextArea outArea;
  private JTextArea errArea;
  private RestartDialog restartDialog;
  private ChangeListener changeListener;
  private Lease lease;
  private RemoteInputStreamInterface out;
  private RemoteInputStreamInterface err;
  private UpdateThread updater;
  
  /**
   * Defines a remote listener that
   * update the job info when the related
   * job description changes in the JAM job
   * repository.
   *
   * @see RemoteEventListener
   */
  class ChangeListener
    extends UnicastRemoteObject
    implements RemoteEventListener
  {

    public ChangeListener()
      throws RemoteException
    {
      super();
    }

    public void notify(RemoteEvent e)
    {
//       System.out.println("\n--ChangeListener--: notify: updating info");
      updateInfo();
    }
  }
  class UpdateThread
    extends Thread
  {
    boolean run;
    
    public UpdateThread()
    {
      super();
      run = true;
    }
    
    public void run()
    {
      while(run) {
        try{
          sleep(3000);
        } catch(InterruptedException ie) {
          ie.printStackTrace();
        }
        updateInfo();
      }
    }
    
    public void terminate()
    {
      run = false;
    }
  }

  /**
   * Builds a panel for monitoring a submitted job.
   *
   * @param JobProxy jobService - the job proxy
   *
   */
  public JobMonitorJPanel(JobProxy jobService)
  {
    super(new BorderLayout());
    this.jobService = jobService;
    outArea = new JTextArea(10, 20);
    errArea = new JTextArea(3, 20);
    //Job Info Textfields 
    name = new JTextField();
    type = new JTextField();
    userID = new JTextField();
    groupID = new JTextField();
    queueName = new JTextField();
    jobID = new JTextField();
    //SGE Textfields
    subName = new JTextField();
    subID = new JTextField();
    //State Textfields
    status = new JTextField();
    action = new JTextField();
    //
    time = new JTextField();

    //Buttons
    exitButton.addActionListener(this);
    startButton.addActionListener(this);
    suspendButton.addActionListener(this);
    resumeButton.addActionListener(this);
    stopButton.addActionListener(this);
    restartButton.addActionListener(this);
    killButton.addActionListener(this);
    removeButton.addActionListener(this);

    exitButton.setActionCommand("0");
    startButton.setActionCommand("1");
    suspendButton.setActionCommand("2");
    resumeButton.setActionCommand("3");
    stopButton.setActionCommand("4");
    restartButton.setActionCommand("5");
    killButton.setActionCommand("6");
    removeButton.setActionCommand("7");
    
    
//     suspendResume = new JButton("Suspend");
//     stopRestart = new JButton("Stop");
//     kill = new JButton("Kill");
//     suspendResume.addActionListener(this);
//     stopRestart.addActionListener(this);
//     kill.addActionListener(this);

    //Info Panel
    JPanel infoPanel = new JPanel(new GridLayout(1,0));
    infoPanel.setBorder(BorderFactory.createTitledBorder("Job Info"));
    // First column
    JPanel jobInfoPanel = new JPanel(new BorderLayout());
    JPanel labInfo = new JPanel(new GridLayout(3, 0));
    labInfo.add(new JLabel("Name   : "));
    labInfo.add(new JLabel("UserID : "));
    labInfo.add(new JLabel("Queue  : "));
    JPanel txtInfo = new JPanel(new GridLayout(3, 0));
    txtInfo.add(name);
    txtInfo.add(userID);
    txtInfo.add(queueName);
    jobInfoPanel.add(labInfo, BorderLayout.WEST);
    jobInfoPanel.add(txtInfo, BorderLayout.CENTER);
    //Second column
    JPanel jobDataPanel = new JPanel(new BorderLayout());
    JPanel labData = new JPanel(new GridLayout(3, 0));
    labData.add(new JLabel("Type    : "));
    labData.add(new JLabel("GroupID : "));
    labData.add(new JLabel("JobID   : "));
    JPanel txtData = new JPanel(new GridLayout(3, 0));
    txtData.add(type);
    txtData.add(groupID);
    txtData.add(jobID);
    jobDataPanel.add(labData, BorderLayout.WEST);
    jobDataPanel.add(txtData, BorderLayout.CENTER);
    // Add columns
    infoPanel.add(jobInfoPanel);
    infoPanel.add(jobDataPanel);

    // RMS Job Params Panel
    JPanel rmsPanel = new JPanel(new GridLayout(1,0));
    rmsPanel.setBorder(BorderFactory.createTitledBorder("RMS Params"));
    // First column
    JPanel rmsOne = new JPanel(new BorderLayout());
    JPanel rmsLabOne = new JPanel(new GridLayout(1, 0));
    rmsLabOne.add(new JLabel("Submission : "));
    JPanel rmsTxtOne = new JPanel(new GridLayout(1, 0));
    rmsTxtOne.add(subName);
    rmsOne.add(rmsLabOne, BorderLayout.WEST);
    rmsOne.add(rmsTxtOne, BorderLayout.CENTER);
    //Second column
    JPanel rmsTwo = new JPanel(new BorderLayout());
    JPanel rmsLabTwo = new JPanel(new GridLayout(1, 0));
    rmsLabTwo.add(new JLabel("ID : "));
    JPanel rmsTxtTwo = new JPanel(new GridLayout(1, 0));
    rmsTxtTwo.add(subID);
    rmsTwo.add(rmsLabTwo, BorderLayout.WEST);
    rmsTwo.add(rmsTxtTwo, BorderLayout.CENTER);
    // Add columns
    rmsPanel.add(rmsOne);
    rmsPanel.add(rmsTwo);

    // Status Reporting Panel
    JPanel stPanel = new JPanel(new GridLayout(1,0));
    stPanel.setBorder(BorderFactory.createTitledBorder("Status Report"));
    // First column
    JPanel stOne = new JPanel(new BorderLayout());
    JPanel stLabOne = new JPanel(new GridLayout(1, 0));
    stLabOne.add(new JLabel("Status : "));
    JPanel stTxtOne = new JPanel(new GridLayout(1, 0));
    stTxtOne.add(status);
    stOne.add(stLabOne, BorderLayout.WEST);
    stOne.add(stTxtOne, BorderLayout.CENTER);
    //Second column
    JPanel stTwo = new JPanel(new BorderLayout());
    JPanel stLabTwo = new JPanel(new GridLayout(1, 0));
    stLabTwo.add(new JLabel("Action : "));
    JPanel stTxtTwo = new JPanel(new GridLayout(1, 0));
    stTxtTwo.add(action);
    stTwo.add(stLabTwo, BorderLayout.WEST);
    stTwo.add(stTxtTwo, BorderLayout.CENTER);
    // Add columns
    stPanel.add(stOne);
    stPanel.add(stTwo);

    // Text Area Panel
    //    JPanel textAreaPanel = new JPanel(new BorderLayout());
    JScrollPane scroll1 = new JScrollPane(outArea);
    scroll1.setBorder(BorderFactory.createTitledBorder("Output"));
    JScrollPane scroll2 = new JScrollPane(errArea);
    scroll2.setBorder(BorderFactory.createTitledBorder("Error messages"));
    //    textAreaPanel.add(scroll1, BorderLayout.CENTER);
    //    textAreaPanel.add(scroll2, BorderLayout.SOUTH);
    JSplitPane splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT,
                                          scroll1, scroll2);
    splitPane.setOneTouchExpandable(true);
    
    // Action Panel
    JButton[] butts1 = { exitButton, startButton, suspendButton,
                        resumeButton, stopButton, restartButton,
                        killButton, removeButton };
    JButton[] butts2 = { startButton, stopButton, killButton };
    replaceButtonsInActionPanelWith(butts1);
    replaceButtonsInActionPanelWith(butts2);
    actionMasterPanel.setBorder(BorderFactory.createTitledBorder("ActionPanel"));
    actionMasterPanel.add(actionPanel);
    
    // Add the three main panel
    JPanel tmp = new JPanel(new BorderLayout());
    tmp.add(infoPanel, BorderLayout.NORTH);
    tmp.add(rmsPanel, BorderLayout.CENTER);
    tmp.add(stPanel, BorderLayout.SOUTH);
    add(tmp, BorderLayout.NORTH);
    //    add(textAreaPanel, BorderLayout.CENTER);
    add(splitPane, BorderLayout.CENTER);
    add(actionMasterPanel, BorderLayout.SOUTH);
    try {
      changeListener = new ChangeListener();
      lease = jobService.notifyJobStatus(changeListener).getLease();
    } catch(RemoteException re) {
      errArea.append(new
                     StringBuffer(re.toString()).append("\n").toString());
      re.printStackTrace();
    }
    displayJobInfo();
  }

  private void displayJobInfo()
  {
    JobInfo ji = jobService.getJobInfo();
    name.setText(ji.name);
    type.setText(ji.type.toString());
    userID.setText(ji.userID);
    groupID.setText(ji.groupID);
    queueName.setText(ji.queueName);
  }

  private void displayJobParams(JobParams jp)
  {
    if(jp != null) {
      jobID.setText(jp.jobID.toString());
      subName.setText(jp.submissionName);
      subID.setText(jp.submissionID.toString());
      //TObe added: check if the URL is unchanged
      //if it is changed than close the RemoteInputStream and
      //open a new one with the new URL
      //read from the stream
      if(out == null) {
        try {
          out = jobService.getReader(jp.output);
        } catch(RemoteException re) {
          re.printStackTrace();
        }
      }
      if(err == null) {
        try {
          err = jobService.getReader(jp.error);
        } catch(RemoteException re) {
          re.printStackTrace();
        }
      }
    }
    if(out != null)
      outArea.append(getTextFromStream(out));
    if(err != null)
      errArea.append(getTextFromStream(err));
  }

  private void displayJobControlEntry(JobStatus st, JobAction ac)
  {
    status.setText(st.toString());
    action.setText(ac.toString());
  }
  
  public void updateInfo()
  {
    JobControlEntry jce = jobService.getJobControlEntry();
    if(jce != null) {
      JobStatus st = jce.jobStatus;
      JobAction ac = jce.jobAction;
      displayJobControlEntry(st, ac);
      setMonitorState(st, ac);
    }
    displayJobParams(jobService.getJobParams());
  }

  private final void disableAllButtons()
  {
    suspendButton.setEnabled(false);
    resumeButton.setEnabled(false);
    startButton.setEnabled(false);
    stopButton.setEnabled(false);
    restartButton.setEnabled(false);
    killButton.setEnabled(false);
    removeButton.setEnabled(false);
  }

  private final void disableButtons(JButton[] butts)
  {
    for(int i = 0; i <= butts.length; i++){
      butts[i].setEnabled(false);
    }
  }

  private final void enableAllButtons()
  {
    suspendButton.setEnabled(true);
    resumeButton.setEnabled(true);
    startButton.setEnabled(true);
    stopButton.setEnabled(true);
    restartButton.setEnabled(true);
    killButton.setEnabled(true);
    removeButton.setEnabled(true);
  }

  private final void enableButtons(JButton[] butts)
  {
    for(int i = 0; i < butts.length; i++){
      butts[i].setEnabled(true);
    }
  }

  private final void replaceButtonsInActionPanelWith(JButton[] butts)
  {
    actionPanel.removeAll();
    for(int i = 0; i < butts.length; i++){
      actionPanel.add(butts[i]);
    }
    // it validates it's components but not itself !!!
    actionMasterPanel.validate();
  }

  private final void setMonitorState(JobStatus st, JobAction ac)
  {
    if(ac == JobAction.BLOCK) {
//       System.out.println("--JobMonitorJPanel--: JobAction.BLOCK: disabling buttons");
      disableAllButtons();
      return;
    }

    switch(st.intValue()) {
      
    case JobStatus._ERROR:
//       System.out.println("--JobMonitorJPanel--: JobStatus._ERROR"); 
      JButton[] butts1 = { suspendButton, stopButton, killButton };
      enableButtons(butts1);
      replaceButtonsInActionPanelWith(butts1);
      break;

      // when it is JAMQUEUD we want to SUBMIT it to RMS
    case JobStatus._JAMQUEUED: // this is NOT good !!!
//       System.out.println("--JobMonitorJPanel--: JobStatus._JAMQUEUED");
      JButton[] butts2 = { startButton, stopButton, killButton };
      enableButtons(butts2);
      replaceButtonsInActionPanelWith(butts2);
      break;

    case JobStatus._IDLE:
//       System.out.println("--JobMonitorJPanel--: JobStatus._IDLE");
      JButton[] butts3 = { suspendButton, stopButton, killButton };
      enableButtons(butts3);
      replaceButtonsInActionPanelWith(butts3);
      break;

    case JobStatus._RUNNING:
//       System.out.println("--JobMonitorJPanel--: JobStatus._RUNNING");
      JButton[] butts4 = { suspendButton, stopButton, killButton };
      enableButtons(butts4);
      replaceButtonsInActionPanelWith(butts4);
      break;

    case JobStatus._SUSPENDED:
//       System.out.println("--JobMonitorJPanel--: JobStatus._SUSPENDED");
      JButton[] butts5 = { resumeButton, stopButton, killButton };
      enableButtons(butts5);
      replaceButtonsInActionPanelWith(butts5);
      break;

    case JobStatus._HELD:
      // NOTE: we should have diff. button for this action
//       System.out.println("--JobMonitorJPanel--: JobStatus._HELD");
      JButton[] butts6 = { startButton, stopButton, killButton };
      enableButtons(butts6);
      replaceButtonsInActionPanelWith(butts6);
      break;
      
    case JobStatus._STOPPED:
//       System.out.println("--JobMonitorJPanel--: JobStatus._STOPPED");
      JButton[] butts7 = { restartButton, killButton };
      enableButtons(butts7);
      replaceButtonsInActionPanelWith(butts7);
      stopUpdateThread();
      break;

    case JobStatus._COMPLETED:
//       System.out.println("--JobMonitorJPanel--: JobStatus._COMPLETED");
      JButton[] butts8 = { restartButton, removeButton };
      enableButtons(butts8);
      replaceButtonsInActionPanelWith(butts8);
      stopUpdateThread();
      break;

    case JobStatus._KILLED:
//       System.out.println("--JobMonitorJPanel--: JobStatus._KILLED");
      JButton[] butts9 = { exitButton };
      enableButtons(butts9);
      replaceButtonsInActionPanelWith(butts9);
      stopUpdateThread();
      break;

    default:
//       System.out.println("--JobMonitorJPanel--: default case: disable everything");
      disableAllButtons();
      break;
    }
  }

  public void actionPerformed(ActionEvent e)
  {
    try {
      switch(e.getActionCommand().charAt(0)) {

      case '0': // exit
//         System.out.println("\n--JobMonitorJPanel--: actionPerformed: exit");
        try {
          jobService.kill();
        } catch(RemoteException rex) {}
        try {
          com.sun.jini.admin.DestroyAdmin admin =
            (com.sun.jini.admin.DestroyAdmin)jobService.getAdmin();
          admin.destroy();
        } catch(RemoteException re) {
          System.out.println("Exception in destroying the service");
          re.printStackTrace();
        }
        break;

      case '1': // start
//         System.out.println("\n--JobMonitorJPanel--: actionPerformed: start");
        // Start for the first time
        // The job has been submitted to RMS, but it is not running
        // we want user to make a specific request to make it running
        startButton.setEnabled(false);
        jobService.start();
        startUpdateThread();
        break;
        
      case '2': // suspend
//         System.out.println("\n--JobMonitorJPanel--: actionPerformed: suspend");
        suspendButton.setEnabled(false);
        jobService.suspend();
        break;        
        
      case '3': // resume
//         System.out.println("\n--JobMonitorJPanel--: actionPerformed: resume");
        resumeButton.setEnabled(false);
        jobService.resume();
        break;        

      case '4': // stop
//         System.out.println("\n--JobMonitorJPanel--: actionPerformed: stop");
        stopButton.setEnabled(false);
        startButton.setEnabled(false);
        suspendButton.setEnabled(false);
        resumeButton.setEnabled(false);
        stopUpdateThread();
        jobService.stop();
        break;
        
      case '5': // restart
//         System.out.println("\n--JobMonitorJPanel--: actionPerformed: restart");
        //restartButton.setEnabled(false);
        if(restartDialog == null) {
          restartDialog = new
            RestartDialog((Frame)this.getTopLevelAncestor(),
                          "Job Parameters", true);
        }
        restartDialog.setJobParams(jobService.getJobParams());
        restartDialog.pack();
        restartDialog.show();
        if(restartDialog.option == RestartDialog.OK_OPTION) {
          if(restartDialog.isValueChanged())
            jobService.restart(restartDialog.getJobParams());
          else
            jobService.restart();
          out.close();
          err.close();
          out = null;
          err = null;
          outArea.setText("");
        }
        break;
        
      case '6': // kill
//         System.out.println("\n--JobMonitorJPanel--: actionPerformed: kill & remove");
        JButton[] butts1 = { exitButton };
        disableAllButtons();
        enableButtons(butts1);
        replaceButtonsInActionPanelWith(butts1);
        stopUpdateThread();
        jobService.kill();
        break;

      case '7': // remove
//         System.out.println("\n--JobMonitorJPanel--: actionPerformed: kill & remove");
        JButton[] butts2 = { exitButton };
        disableAllButtons();
        enableButtons(butts2);
        replaceButtonsInActionPanelWith(butts2);
        jobService.kill();
        break;        

      case '8': // refresh panel
//         System.out.println("\n--JobMonitorJPanel--: actionPerformed: refresh");
        updateInfo();
        break;
        
      default:
//         System.out.println("\n--JobMonitorJPanel--: actionPerformed: default");
        break;
      }
    } catch(Exception ex) {
      errArea.append(ex.toString() + "\n");
//       System.out.println("--JobMonitorPanel--: Exception in action: "+
//                          e.getActionCommand().charAt(0));
      ex.printStackTrace();
    }
  }
  
  private String getTextFromStream(RemoteInputStreamInterface stream)
  {
    StringBuffer buf = new StringBuffer();
    try {
      byte[] b;
      while(stream.available() > 0) {
        b = stream.read(1024);
        if(b != null && b.length > 0) {
          buf.append(new String(b));
        }
      }
    } catch(RemoteException re) {
      re.printStackTrace();
    }
    return buf.toString();
  }

  private void startUpdateThread()
  {
    if(updater != null)
      updater.terminate();
    updater = new UpdateThread();
    updater.start();
  }

  private void stopUpdateThread()
  {
    if(updater != null) {
      updater.terminate();
      updater = null;
    }
  }
  
  public void registerClosingListener(ActionListener listener)
  {
    exitButton.addActionListener(listener);
  }
  
  /**
   * Cancels the lease that belongs to
   * the change listener object. Deletes
   * all the streams opened.
   *
   */
  public void terminate()
  {
    if(out != null) {
      try {
        out.close();
      } catch(RemoteException re) {
      }
    }
    if(err != null) {
      try {
        err.close();
      } catch(RemoteException re) {
      }
    }
    if(lease != null) {
      try {
        lease.cancel();
      } catch(Exception e) {
      }
    }
  }

  protected void finalize()
    throws Throwable
  {
    terminate();
    super.finalize();
  }
}

class RestartDialog
  extends JDialog
  implements ActionListener
{

  final static int OK_OPTION = 0;
  final static int CANCEL_OPTION = -1;
  int option = CANCEL_OPTION;
  boolean changed = false;
  String scriptText;
  String argsText;
  JTextField args;
  JTextArea script;
  JobParams value;
 
  public RestartDialog(Frame ow, String name, boolean md)
  {
    super(ow, name, md);
    args = new JTextField();
    JPanel north = new JPanel(new BorderLayout());
    north.setBorder(new EtchedBorder());
    north.add(new JLabel("Parameters:"), BorderLayout.WEST);
    north.add(args, BorderLayout.CENTER);

    script = new JTextArea();
    JPanel center = new JPanel(new BorderLayout());
    center.setBorder(new EtchedBorder());
    center.add(new JLabel("Script:"), BorderLayout.WEST);
    center.add(new JScrollPane(script), BorderLayout.CENTER);
    
    JButton ok = new JButton("Ok");
    JButton cancel = new JButton("Cancel");
    JPanel south = new JPanel(new FlowLayout());
    JPanel in = new JPanel(new GridLayout(1, 0, 5, 5));
    in.add(ok);
    in.add(cancel);
    
    south.setBorder(new EtchedBorder());
    south.add(in);
    ok.addActionListener(this);
    cancel.addActionListener(this);
    getContentPane().add(north, BorderLayout.NORTH);
    getContentPane().add(center, BorderLayout.CENTER);
    getContentPane().add(south, BorderLayout.SOUTH);
    setLocationRelativeTo(ow);
    addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent w) {
        setVisible(false);
      }
    });
  }

  public RestartDialog(Frame ow, String name, boolean md, JobParams
                       value)
  {
    this(ow, name, md);
    setJobParams(value);
  }

  public void setJobParams(JobParams jp)
  {
    value = jp;
    option = CANCEL_OPTION;
    changed = false;
    String allArgs = new String();
    if(value.args != null) {
      for(int i = 0; i < value.args.length; i ++) {
        allArgs = allArgs + value.args[i] + " ";
      }
    }
    argsText = allArgs;
    scriptText = value.script;
    args.setText(allArgs);
    script.setText(value.script);
  }
  
  public JobParams getJobParams()
  {
    return value;
  }

  public boolean isValueChanged()
  {
    return changed;
  }

  public void actionPerformed(ActionEvent e)
  {
    if(e.getActionCommand().equals("Ok")) {
      String argsT = args.getText();
      String scriptT = script.getText();
      if(scriptText != null && argsText != null &&
         (!argsText.equals(argsT) || !scriptText.equals(scriptT)))
        changed = true;
      StringTokenizer tokenizer = new StringTokenizer(argsT);
      String[] argList = new String[tokenizer.countTokens()];
      for(int i = 0; i < argList.length; i ++)
        argList[i] = tokenizer.nextToken();
      value.args = argList;
      value.script = scriptT;
      option = OK_OPTION;
    }
    setVisible(false);
  }
}

