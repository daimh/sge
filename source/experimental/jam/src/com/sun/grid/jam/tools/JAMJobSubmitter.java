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
package com.sun.grid.jam.tools;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.rmi.*;
import java.io.*;
import java.util.Properties;
import java.util.Vector;
import java.util.Date;
import java.util.StringTokenizer;
import java.net.MalformedURLException;
import net.jini.core.entry.Entry;
import net.jini.core.lease.Lease;
import net.jini.core.lookup.ServiceMatches;
import net.jini.core.lookup.ServiceRegistrar;
import net.jini.core.lookup.ServiceTemplate;
import net.jini.core.lookup.ServiceID;
import net.jini.core.lookup.ServiceItem;
import net.jini.lookup.ServiceDiscoveryManager;
import net.jini.lookup.LookupCache;
import net.jini.lookup.ServiceItemFilter;
import net.jini.lookup.ServiceDiscoveryListener;
import net.jini.lookup.ServiceDiscoveryEvent;
import net.jini.discovery.LookupDiscovery;
import net.jini.lease.LeaseRenewalManager;
import net.jini.lookup.entry.Name;
import net.jini.lookup.entry.NameBean;

import com.sun.grid.jam.app.AppParamsInterface;
import com.sun.grid.jam.app.AppParams;
import com.sun.grid.jam.job.JobInterface;
import com.sun.grid.jam.job.entry.JobUserKey;
import com.sun.grid.jam.rms.ComputeInfoParams;
import com.sun.grid.jam.admin.UserProperties;
import com.sun.grid.jam.queue.QueueInterface;

/**
 * Submits a job in a JAM queue service
 *
 * @version 1.7, 09/22/00
 *
 * @author Nello Nellari
 */
public class JAMJobSubmitter
  extends JFrame
  implements MouseListener, ActionListener,
             ServiceDiscoveryListener
{
  JTextField scriptPath;
  JTextField param;
  JList queueList;
  JScrollPane scroll;
  JButton browse;
  JButton submit;
  JButton reset;
  JButton exit;
  String appName;
  String userID;
  ServiceDiscoveryManager lookupManager;
  LookupCache lookupCache;
  ServiceTemplate template;

  public JAMJobSubmitter(String title, ServiceTemplate template,
                         String[] groups)
    throws IOException
  {
    super(title);
    this.template = template;
    lookupManager = new ServiceDiscoveryManager(new
                                            LookupDiscovery(groups),
                                            null);
    lookupCache = lookupManager.createLookupCache(template, null,
                                                 this);
    browse = new JButton("Browse");
    submit = new JButton("Submit");
    reset = new JButton("Reset");
    exit  = new JButton("Exit");
    scriptPath = new JTextField();
    param = new JTextField();
    queueList = new JList(new ServiceListModel());
    queueList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
    queueList.setVisibleRowCount(7);
    queueList.addMouseListener(this);
    browse.setActionCommand("1");
    submit.setActionCommand("2");
    reset.setActionCommand("3");
    exit.setActionCommand("4");
    scriptPath.setText("");
    //    submit.setEnabled(false);
    browse.addActionListener(this);
    submit.addActionListener(this);
    reset.addActionListener(this);
    exit.addActionListener(this);
    getContentPane().setLayout(new BorderLayout());
    JPanel north = new JPanel(new BorderLayout());
    north.setBorder(BorderFactory.createTitledBorder("User Defined Command"));
    JPanel northLab = new JPanel(new GridLayout(2, 0));
    northLab.add(browse);
    northLab.add(new JLabel("Parameters: "));
    JPanel northText = new JPanel(new GridLayout(2, 0));
    northText.add(scriptPath);
    northText.add(param);
    north.add(northLab, BorderLayout.WEST);
    north.add(northText, BorderLayout.CENTER);

    scroll = new JScrollPane(queueList);
    scroll.setBorder(BorderFactory.createTitledBorder("Queue Browser"));
    scroll.setPreferredSize(new Dimension(300, 150));
    getContentPane().add(north, BorderLayout.NORTH);
    getContentPane().add(scroll, BorderLayout.CENTER);
    JPanel south = new JPanel(new FlowLayout());
    south.setBorder(BorderFactory.createTitledBorder("Action panel"));
    JPanel action = new JPanel(new GridLayout(1, 0, 10, 10));
    action.add(submit);
    action.add(reset);
    action.add(exit);
    south.add(action);
    getContentPane().add(south, BorderLayout.SOUTH);
    addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent w) {
        terminate();
        w.getWindow().setVisible(false);
        w.getWindow().dispose();
        System.exit(0);
      }
    });
  }
  
  public JAMJobSubmitter(String title, ServiceTemplate template,
                         String[] groups, String userID)
    throws IOException
  {
    this(title, template, groups);
    this.userID = userID;
  }
  
  public void setUserID(String userID)
  {
    this.userID = userID;
  }
  
  /**
   * Implementing ServiceDiscoveryListener interface
   *
   *
   */
  public void serviceAdded(ServiceDiscoveryEvent e)
  {
    ServiceItem si = e.getPostEventServiceItem();
    ServiceListModel model = (ServiceListModel)queueList.getModel();
    model.addElement(si);
  }

  /**
   * Implementing ServiceDiscoveryListener interface
   *
   *
   */
  public void serviceRemoved(ServiceDiscoveryEvent e)
  {
    ServiceItem si = e.getPreEventServiceItem();
    ServiceListModel model = (ServiceListModel)queueList.getModel();
    model.removeElement(si);
  }

  /**
   * Implementing ServiceDiscoveryListener interface
   *
   *
   */
  public void serviceChanged(ServiceDiscoveryEvent e)
  {
    ServiceItem si = e.getPreEventServiceItem();
    ServiceListModel model = (ServiceListModel)queueList.getModel();
    model.update(si, e.getPostEventServiceItem());
  }

  /**
   * Implementing ActionListener interface
   *
   *
   */
  public void actionPerformed(ActionEvent e)
  {
    switch(e.getActionCommand().charAt(0)) {

    case '1':
      //Browse
      JFileChooser fileChooser = new JFileChooser();
      fileChooser.setDialogType(JFileChooser.OPEN_DIALOG);
      fileChooser.setApproveButtonText("Select");
      fileChooser.setDialogTitle("Script chooser");
      if(fileChooser.showOpenDialog(this) ==
         JFileChooser.APPROVE_OPTION) {
        File file = fileChooser.getSelectedFile();
        if(file != null && file.isFile()) {
          scriptPath.setText(file.getPath());
          appName = file.getName();
        }
      }
      break;

    case '2':
      String scriptFile = scriptPath.getText();
      if(scriptFile !="" && !queueList.isSelectionEmpty()) {
        submitJob(scriptFile);
      }
      break;

    case '3':
      //Reset
      queueList.clearSelection();
      scriptPath.setText("");
      param.setText("");
      break;
      
    case '4':
      //Exit
      terminate();
      System.exit(0);
      break;  

    default :
      break;
    }
  }

  private void submitJob(String scriptFile)
  {
    try {
      //Get the service from lookup cache
      QueueInterface q = getQueueService(queueList.getSelectedIndex());
      //Prepare an application Object
      StringTokenizer pList = new StringTokenizer(param.getText());
      String[] p = new String[pList.countTokens()];
      for(int i = 0; i < p.length; i ++)
        p[i] = pList.nextToken();
      String script = getScriptContent(scriptFile);
      UserProperties user = new UserProperties("Real Name",
                                               userID);
      Entry entry [] = { new JobUserKey(appName,
                                        user,
                                        "localhost",
                                        new Date()),
                         new Name(appName),
                         new Name(userID) };
      q.submit(new AppParams(appName, null, script, p, true), entry,
               user);
      //Start a service listener waiting for the Job service
      JAMServiceUILauncher launcher = new
        JAMServiceUILauncher(lookupManager, entry,
                             "com.sun.grid.jam.job.JobInterface");
    } catch(Exception ex) {
      ex.printStackTrace();
    }
  }
  
  private String getScriptContent(String file)
    throws IOException
  {
    StringBuffer script = new StringBuffer();
    BufferedReader reader = new BufferedReader(new FileReader(file));
    String s;
    while((s = reader.readLine()) != null)
      script.append(s + "\n");
    return script.toString();
  }

  private QueueInterface getQueueService(int index)
    throws IOException
  {
    final ServiceID id =
      ((ServiceListModel)queueList.getModel()).getServiceID(index);
    ServiceItem si = lookupCache.lookup(new ServiceItemFilter() {

      public boolean check(ServiceItem item) {
        if(item.serviceID.equals(id))
          return true;
        return false;
      }
    });
    return (QueueInterface)si.service;
  }
    
  public void mouseEntered(MouseEvent e)
  {
  }

  /**
   *
   *
   */
  public void mouseExited(MouseEvent e)
  {
  }

  /**
   *
   *
   */
  public void mousePressed(MouseEvent e)
  {
  }
  
  /**
   *
   *
   */
  public void mouseReleased(MouseEvent e)
  {
  }

  /**
   * Launch application
   *
   */
  public void mouseClicked(MouseEvent e)
  {

//     if(e.getClickCount() > 1) {
//       int index = queueList.getSelectedIndex();
//       try {
//         ServiceID id = (ServiceID)serviceIDList.get(index);
//         ServiceRegistrar[] lks = discovery.getRegistrars();
//         ServiceMatches sm = null;
      //         for(int i = 0 ; i < lks.length && sm == null; i++) {
//           sm = lks[i].lookup(new ServiceTemplate(id, null,
//                                                  null), 1);
//         }
//         if(sm != null) {
// //           QueueInfoJFrame fr = new QueueInfoJFrame(sm.items[0].attributeSets);
// //           fr.pack();
// //           fr.setResizable(false);
// //           fr.setVisible(true);
//         }
//       } catch(Exception re) {
//         re.printStackTrace();
//       }
//     }
  }

  private void terminate()
  {
    lookupCache.removeListener(this);
    lookupCache.terminate();
    lookupManager.terminate();
  }

  public static void main(String args[])
    throws ClassNotFoundException
  {
    try {
      System.setSecurityManager(new SecurityManager());
      Class [] cls =
      { Class.forName("com.sun.grid.jam.queue.QueueInterface") };
      ServiceTemplate tmpl = new ServiceTemplate(null, cls, null);
      String[] groups = { System.getProperty("user.name")};
      JAMJobSubmitter submitter = new JAMJobSubmitter("JAM Job Submission",
                                                      tmpl, groups);
      submitter.pack();
      submitter.setVisible(true);
      LoginDialog login = new LoginDialog(submitter, "JAM login",
                                          true);
      login.pack();
      login.show();
      if(login.getResult() == LoginDialog.APPROVED) {
        String s = login.getLoginName();
        submitter.setUserID(s);
      } else {
        submitter.setVisible(false);
        System.exit(0);
      }
    } catch(IOException ioe) {
      System.out.println("JAM Submitter could not start: " +
                         ioe.getMessage());
      System.exit(0);
    }
  }
}

class LoginDialog
  extends JDialog
  implements ActionListener
{
  public final static int APPROVED = 1;
  JTextField loginField;
  JPasswordField passwdField;
  JButton ok;
  JButton cancel;
  JButton exit;
  int result = 3;
  
  public LoginDialog(Frame ow, String name, boolean md)
  {
    super(ow, name, md);
    loginField = new JTextField();
    passwdField = new JPasswordField();
    loginField.setActionCommand("1");
    passwdField.setActionCommand("1");
    ok = new JButton("Login");
    ok.setActionCommand("1");
    cancel = new JButton("Cancel");
    cancel.setActionCommand("2");
    exit = new JButton("Exit");
    exit.setActionCommand("3");
    JPanel loginPanel = new JPanel(new BorderLayout());
    JPanel labels = new JPanel(new GridLayout(2, 0));
    labels.add(new JLabel("Login Name"));
    labels.add(new JLabel("Password"));
    JPanel texts = new JPanel(new GridLayout(2, 0));
    texts.add(loginField);
    texts.add(passwdField);
    loginPanel.setBorder(BorderFactory.createTitledBorder(""));
    loginPanel.add(labels, BorderLayout.WEST);
    loginPanel.add(texts, BorderLayout.CENTER);
    JPanel actionPanel = new JPanel(new FlowLayout());
    JPanel buttons = new JPanel(new GridLayout(1, 0, 10, 10));
    buttons.add(ok);
    buttons.add(cancel);
    buttons.add(exit);
    actionPanel.add(buttons);
    getContentPane().add(loginPanel, BorderLayout.NORTH);
    getContentPane().add(actionPanel, BorderLayout.SOUTH);
    ok.addActionListener(this);
    cancel.addActionListener(this);
    exit.addActionListener(this);
    loginField.addActionListener(this);
    passwdField.addActionListener(this);
    setLocationRelativeTo(ow);
    addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent w) {
        setVisible(false);
      }
    });
  }
  
  public String getLoginName()
  {
    return loginField.getText();
  }

  public int getResult()
  {
    return result;
  }
  
  public void actionPerformed(ActionEvent e)
  {
    switch(e.getActionCommand().charAt(0)) {

    case '1':
      String s = loginField.getText();
      if( s == null || !s.equals(System.getProperty("user.name"))) {
        Toolkit.getDefaultToolkit().beep();
        break;
      }
      result = 1;
      setVisible(false);
      dispose();
      break;

    case '2':
      loginField.setText("");
      passwdField.setText("");
      break;

    case '3':
      result = 3;
      setVisible(false);
      dispose();
      break;
    }
  }
}

class ServiceListModel
  extends DefaultListModel 
{
  Vector idList;
  
  public ServiceListModel()
  {
    super();
    idList = new Vector();
  }

  public void addElement(Object obj)
  {
    if(obj instanceof ServiceItem) {
      idList.add(((ServiceItem)obj).serviceID);
      super.addElement(getServiceName((ServiceItem) obj));
      return;
    }
    super.addElement(obj);
  }

  public Object set(int index, Object obj)
  {
    if(obj instanceof ServiceItem) {
      idList.set(index, ((ServiceItem)obj).serviceID);
      return super.set(index, getServiceName((ServiceItem) obj));
    }
    return super.set(index, obj);
  }

  public boolean removeElement(Object obj)
  {
    if(obj instanceof ServiceItem) {
      ServiceID id = ((ServiceItem)obj).serviceID;
      super.remove(idList.indexOf(id));
      idList.removeElement(id);
      return true;
    }
    return super.removeElement(obj);
  }

  public Object update(ServiceItem pre, ServiceItem post)
  {
    int index = idList.indexOf(pre.serviceID);
    return super.set(index, getServiceName(post));
  }
  
  public ServiceID getServiceID(int index)
  {
    return (ServiceID)idList.get(index);
  }
  
  private String getServiceName(ServiceItem item)
  {
    for(int i = 0; i < item.attributeSets.length; i++) {
      if(item.attributeSets[i] instanceof Name) {
        NameBean bean = new NameBean();
        bean.makeLink(item.attributeSets[i]);
        return bean.getName();
      }
    }
    return item.service.getClass().getName();
  }
}


