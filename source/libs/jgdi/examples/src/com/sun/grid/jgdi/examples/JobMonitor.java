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
package com.sun.grid.jgdi.examples;

import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.monitoring.JobSummary;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummary;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryOptions;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryResult;
import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.prefs.BackingStoreException;
import java.util.prefs.Preferences;
import javax.swing.AbstractAction;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.Timer;
import javax.swing.event.TableModelEvent;
import javax.swing.table.AbstractTableModel;

/**
 *
 */
public class JobMonitor extends JFrame {
    
    private JobTableModel jobTableModel = new JobTableModel();
    private JTable jobTable = new MyTable(jobTableModel);
    private JMenu recentConnectionMenu = new JMenu("Connect to Recent");
    private Thread updateThread;
    private StatusPanel statusPanel = new StatusPanel();
    private ConnectionHistory connectionHistory;
    static private String myurl = null;
    
    public JobMonitor() {
        super("Sun Grid Engine Job Monitor");
        this.addWindowListener(new ExitHandler());
        
        JScrollPane sc = new JScrollPane(jobTable);
        
        JPanel mainPanel = new JPanel(new BorderLayout());
        mainPanel.add(sc, BorderLayout.CENTER);
        mainPanel.add(statusPanel, BorderLayout.SOUTH);
        
        getContentPane().add(mainPanel);
        initMenuBar();
        jobTableModel.addJobTableModelListener(statusPanel);
        
        connectionHistory = new ConnectionHistory(recentConnectionMenu);
        jobTableModel.addJobTableModelListener(connectionHistory);
    }
    
    private void initMenuBar() {
        JMenuBar mb = new JMenuBar();
        JMenu fileMenu = new JMenu("File");
        fileMenu.add(new ConnectAction());
        fileMenu.add(recentConnectionMenu);
        fileMenu.addSeparator();
        fileMenu.add(new ExitAction());
        mb.add(fileMenu);
        setJMenuBar(mb);
    }
    
    private void doExit() {
        updateThread.interrupt();
        connectionHistory.save();
        try {
            updateThread.join();
        } catch (InterruptedException ire) {
            // Ignore
        }
        System.exit(0);
    }
    
    public static void main(String[] args) {
        
        JobMonitor.myurl = args[0];
        
        JobMonitor jm = new JobMonitor();
        
        jm.setSize(600, 400);
        jm.start();
        jm.setVisible(true);
        
        try {
            jm.waitForEnd();
        } catch (InterruptedException ire) {
            // Ignore
        }
    }
    
    private void showError(final Throwable th) {
        
        if (!SwingUtilities.isEventDispatchThread()) {
            try {
                SwingUtilities.invokeAndWait(new Runnable() {
                    
                    public void run() {
                        showError(th);
                    }
                });
            } catch (InvocationTargetException ex) {
                // Ignore
            } catch (InterruptedException ex1) {
                // Ignore
            }
        } else {
            JOptionPane.showMessageDialog(this, th.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
        }
    }
    
    public void start() {
        updateThread = new Thread(jobTableModel);
        updateThread.start();
    }
    
    public void waitForEnd() throws InterruptedException {
        updateThread.join();
    }
    public static final String[] COLUMNS = {"Name", "State", "User"};
    
    class JobTableModel extends AbstractTableModel implements Runnable {
        
        private List<JobSummary> jobList = new ArrayList<JobSummary>();
        private JGDI jgdi;
        private long sleepTime = 3000;
        private String url = null;
        private Object syncObject = new Object();
        private List<JobTableModelListener> listenerList = new ArrayList<JobTableModelListener>();
        
        public int getRowCount() {
            return jobList.size();
        }
        
        public int getColumnCount() {
            return COLUMNS.length;
        }
        
        public String getColumnName(int column) {
            return COLUMNS[column];
        }
        
        public void addJobTableModelListener(JobTableModelListener listener) {
            this.listenerList.add(listener);
        }
        
        public void removeJobTableModelListener(JobTableModelListener listener) {
            this.listenerList.remove(listener);
        }
        
        public JobSummary getJob(int rowIndex) {
            return jobList.get(rowIndex);
        }
        
        public Object getValueAt(int rowIndex, int columnIndex) {
            synchronized (jobList) {
                JobSummary js = getJob(rowIndex);
                switch (columnIndex) {
                    case 0:
                        return js.getName();
                    case 1:
                        return js.getState();
                    case 2:
                        return js.getUser();
                    default:
                        return "Error";
                }
            }
        }
        
        public void setUrl(String url) {
            synchronized (syncObject) {
                if (isConnected()) {
                    disconnect();
                }
                this.url = url;
                if (url != null) {
                    connect();
                }
            }
        }
        
        private void connect() {
            synchronized (syncObject) {
                if (!isConnected()) {
                    try {
                        jgdi = JGDIFactory.newInstance(url);
                        syncObject.notify();
                    } catch (JGDIException ex) {
                        showError(ex);
                        return;
                    }
                }
            }
            fireConnected(url);
        }
        
        public void disconnect() {
            
            synchronized (syncObject) {
                if (jgdi != null) {
                    try {
                        jgdi.close();
                    } catch (JGDIException jgde) {
                        // Ignore
                    } finally {
                        jgdi = null;
                    }
                }
                if (!jobList.isEmpty()) {
                    jobList.clear();
                }
            }
            fireDisconnected();
        }
        
        public boolean isConnected() {
            return jgdi != null;
        }
        
        public void run() {
            
            try {
                while (!Thread.currentThread().isInterrupted()) {
                    
                    if (isConnected()) {
                        
                        try {
                            fireUpdateStarted();
                            QueueInstanceSummaryOptions options = new QueueInstanceSummaryOptions();
                            
                            options.setShowFullOutput(true);
                            
                            QueueInstanceSummaryResult result = jgdi.getQueueInstanceSummary(options);
                            
                            updateJobs(result);
                        } catch (JGDIException jgdie) {
                            showError(jgdie);
                            disconnect();
                        } finally {
                            fireUpdateFinished(getRowCount());
                        }
                    }
                    synchronized (syncObject) {
                        syncObject.wait();
                    }
                }
            } catch (InterruptedException ire) {
            } finally {
                disconnect();
            }
        }
        
        public void runUpdate() {
            synchronized (syncObject) {
                syncObject.notify();
            }
        }
        
        private void updateJobs(QueueInstanceSummaryResult result) {
            
            synchronized (syncObject) {
                jobList.clear();
                for (QueueInstanceSummary qis : result.getQueueInstanceSummary()) {
                    jobList.addAll(qis.getJobList());
                }
                jobList.addAll(result.getPendingJobs());
                jobList.addAll(result.getErrorJobs());
                jobList.addAll(result.getFinishedJobs());
                jobList.addAll(result.getZombieJobs());
            }
            
            if (SwingUtilities.isEventDispatchThread()) {
                fireTableDataChanged();
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        fireTableDataChanged();
                    }
                });
            }
        }
        
        private void fireConnected(String url) {
            Object[] lis = listenerList.toArray();
            for (int i = 0; i < lis.length; i++) {
                ((JobTableModelListener) lis[i]).connected(url);
            }
        }
        
        public void fireDisconnected() {
            Object[] lis = listenerList.toArray();
            for (int i = 0; i < lis.length; i++) {
                ((JobTableModelListener) lis[i]).disconnected();
            }
        }
        
        public void fireUpdateStarted() {
            Object[] lis = listenerList.toArray();
            for (int i = 0; i < lis.length; i++) {
                ((JobTableModelListener) lis[i]).updateStarted();
            }
        }
        
        public void fireUpdateFinished(int jobCount) {
            Object[] lis = listenerList.toArray();
            for (int i = 0; i < lis.length; i++) {
                ((JobTableModelListener) lis[i]).updateFinished(jobCount);
            }
        }
    }
    
    static interface JobTableModelListener {
        
        public void connected(String url);
        
        public void disconnected();
        
        public void updateStarted();
        
        public void updateFinished(int jobCount);
        
        public void nextUpdate(int seconds);
    }
    
    class ConnectionHistory implements JobTableModelListener {
        
        private JMenu recentMenu = null;
        private Preferences prefs;
        private LinkedList<String> connections = new LinkedList<String>();
        private Map menuMap = new HashMap();
        
        ConnectionHistory(JMenu recentMenu) {
            this.recentMenu = recentMenu;
            
            prefs = Preferences.userNodeForPackage(JobMonitor.class).node("jobmon");
            String[] keys;
            try {
                keys = prefs.keys();
                for (int i = 0; i < keys.length; i++) {
                    
                    String con = prefs.get(keys[i], null);
                    if (con != null && !connections.contains(con)) {
                        menuMap.put(con, recentMenu.add(new ConnectAction(con)));
                        connections.add(con);
                    }
                }
            } catch (BackingStoreException ex) {
                ex.printStackTrace();
            }
        }
        
        public void save() {
            int i = 0;
            for (String con : connections) {
                prefs.put("url" + i, con);
                i++;
            }
            while (i < 10) {
                prefs.remove("url" + i);
                i++;
            }
        }
        
        public void connected(final String url) {
            if (SwingUtilities.isEventDispatchThread()) {
                if (connections.contains(url)) {
                    connections.remove(url);
                    connections.add(url);
                    Component comp = (Component) menuMap.get(url);
                    recentMenu.remove(comp);
                    recentMenu.add(comp);
                } else {
                    while (connections.size() > 10) {
                        String con = connections.removeLast();
                        Component comp = (Component) menuMap.remove(con);
                        recentMenu.remove(comp);
                    }
                    
                    connections.add(url);
                    menuMap.put(url, recentMenu.add(new ConnectAction(url)));
                }
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        connected(url);
                    }
                });
            }
        }
        
        public void disconnected() {
        }
        
        public void updateStarted() {
        }
        
        public void updateFinished(int jobCount) {
        }
        
        public void nextUpdate(int seconds) {
        }
    }
    
    class StatusPanel extends JPanel implements JobTableModelListener {
        
        private JLabel jobCountLabel = new JLabel("JobMon");
        private JTextField jobCountTextField = new JTextField(5);
        private JButton updateButton = new JButton("Update");
        private UpdateAction updateAction = new UpdateAction();
        private Timer timer = null;
        
        public StatusPanel() {
            
            BoxLayout bl = new BoxLayout(this, BoxLayout.X_AXIS);
            
            setLayout(bl);
            
            add(jobCountLabel);
            add(jobCountTextField);
            jobCountTextField.setOpaque(false);
            jobCountTextField.setBorder(null);
            add(Box.createHorizontalGlue());
            
            add(updateButton);
            updateButton.addActionListener(updateAction);
            
            timer = new Timer(1000, updateAction);
        }
        
        public void tableChanged(TableModelEvent e) {
            jobCountTextField.setText(Integer.toString(jobTableModel.getRowCount()));
        }
        
        public void connected(final String url) {
            if (SwingUtilities.isEventDispatchThread()) {
                setTitle("JobMon: " + url);
                timer.start();
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        connected(url);
                    }
                });
            }
        }
        
        public void disconnected() {
            if (SwingUtilities.isEventDispatchThread()) {
                setTitle("Connect");
                jobCountTextField.setText("");
                timer.stop();
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        disconnected();
                    }
                });
            }
        }
        
        public void updateStarted() {
            if (SwingUtilities.isEventDispatchThread()) {
                updateButton.setText("Update");
                updateButton.setEnabled(false);
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        updateStarted();
                    }
                });
            }
        }
        
        public void updateFinished(final int jobCount) {
            if (SwingUtilities.isEventDispatchThread()) {
                jobCountTextField.setText(Integer.toString(jobCount));
                updateButton.setEnabled(true);
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        updateFinished(jobCount);
                    }
                });
            }
        }
        
        public void nextUpdate(final int seconds) {
            if (SwingUtilities.isEventDispatchThread()) {
                updateButton.setText("Update (" + seconds + ")");
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        nextUpdate(seconds);
                    }
                });
            }
        }
        
        class UpdateAction implements ActionListener {
            
            int seconds = 20;
            
            public void actionPerformed(ActionEvent e) {
                
                if (e.getSource() == timer) {
                    seconds--;
                    if (seconds <= 0) {
                        timer.stop();
                        jobTableModel.runUpdate();
                        seconds = 20;
                        timer.restart();
                    }
                    nextUpdate(seconds);
                } else {
                    timer.stop();
                    jobTableModel.runUpdate();
                    seconds = 20;
                    timer.restart();
                    nextUpdate(seconds);
                }
            }
        }
    }
    
    class ConnectAction extends AbstractAction {
        
        private String url;
        
        public ConnectAction() {
            super("Connect");
            url = null;
        }
        
        public ConnectAction(String url) {
            super(url);
            this.url = url;
        }
        
        public void actionPerformed(ActionEvent e) {
            
            if (url == null) {
                String url = JOptionPane.showInputDialog(JobMonitor.this, "Enter Grid Engine connection url:", (myurl == null) ? "bootstrap://<SGE_ROOT>@<SGE_CELL>:<SGE_QMASTER_PORT>": myurl);
                if (url != null) {
                    jobTableModel.setUrl(url);
                }
            } else {
                jobTableModel.setUrl(url);
            }
        }
    }
    
    class ExitHandler extends WindowAdapter {
        
        public void windowClosing(WindowEvent e) {
            doExit();
        }
    }
    
    class ExitAction extends AbstractAction {
        
        public ExitAction() {
            super("Exit");
        }
        
        public void actionPerformed(ActionEvent e) {
            doExit();
        }
    }
    
    class MyTable extends JTable {
        
        public MyTable(JobTableModel model) {
            super(model);
        }
        
        public String getToolTipText(MouseEvent e) {
            
            
            String tip = null;
            java.awt.Point p = e.getPoint();
            int rowIndex = rowAtPoint(p);
            
            
            JobSummary job = jobTableModel.getJob(rowIndex);
            
            StringWriter sw = new StringWriter();
            PrintWriter pw = new PrintWriter(sw);
            
            pw.print("Job ");
            pw.print(job.getId());
            pw.print(" (");
            pw.print(job.getName());
            pw.println(")");
            
            pw.println("job_number:                 " + job.getId() + "<br>");
            pw.println("name:                       " + job.getName() + "<br>");
            pw.println("user:                       " + job.getUser() + "<br>");
            pw.println("CheckpointEnv:              " + job.getCheckpointEnv() + "<br>");
            
            pw.flush();
            String ret = sw.getBuffer().toString();
            System.out.println(ret);
            return ret;
        }
    }
}