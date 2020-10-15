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
 *   Copyright: 2006 by Sun Microsystems, Inc
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jgdi.examples.jmxeventmonitor;

import com.sun.grid.jgdi.event.Event;
import com.sun.grid.jgdi.event.EventListener;
import com.sun.grid.jgdi.event.EventTypeEnum;
import com.sun.grid.jgdi.management.JGDIProxy;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.io.File;
import java.io.FileInputStream;
import java.security.KeyStore;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.EventObject;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import javax.swing.AbstractAction;
import javax.swing.BorderFactory;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JToolBar;
import javax.swing.SwingUtilities;
import javax.swing.event.CellEditorListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;

/**
 *
 */
public class MainFrame extends JFrame {
    
    private final static long serialVersionUID = -2007121301L;
    
    //private final DefaultListModel subscriptionListModel = new DefaultListModel();
    //private final JList  subscriptionList = new JList(subscriptionListModel);
    private final ConnectionController controller = new ConnectionController();
    
    private final SubscriptionTableModel subscriptionTableModel = new SubscriptionTableModel(controller);
    private final JTable subscriptionTable = new JTable(subscriptionTableModel);
    
    private final EventTableModel eventTableModel = new EventTableModel();

    private final JCheckBox autoCommit = new JCheckBox("auto commit");
    
    private final JTable eventTable = new JTable(eventTableModel);
    private final ConnectAction connectAction = new ConnectAction();
    private final DisConnectAction disconnectAction = new DisConnectAction();
    private final ClearAction clearAction = new ClearAction();
    private final SubscribeAllAction subscribeAllAction = new SubscribeAllAction();
    private final UnsubscribeAllAction unsubscribeAllAction = new UnsubscribeAllAction();
    private final ShowDetailsAction showDetailsAction = new ShowDetailsAction();
    private final CommitAction commitAction = new CommitAction();
    
    
    private JLabel statusLabel = new JLabel();
    
    /** Creates a new instance of MainFrame */
    public MainFrame() {
        
        setTitle("JGDI Event Monitor: Not connected");
        JPanel panel = new JPanel(new BorderLayout());
        getContentPane().add(panel);
        
        int SUB_COL_WIDTH = 40;
        int TYPE_WIDTH = 140;
        subscriptionTable.getColumnModel().getColumn(0).setMaxWidth(SUB_COL_WIDTH);
        
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        
        JPanel subscriptionPanel = new JPanel(new BorderLayout());
        
        JScrollPane sp = new JScrollPane(subscriptionTable, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
        sp.setMaximumSize(new Dimension(SUB_COL_WIDTH + TYPE_WIDTH + 20, Integer.MAX_VALUE));
        sp.setPreferredSize(new Dimension(SUB_COL_WIDTH + TYPE_WIDTH + 20, 300));
        sp.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createTitledBorder("Subscription:"), BorderFactory.createLoweredBevelBorder()));
        
        subscriptionPanel.add(sp, BorderLayout.CENTER);
        
        JToolBar toolBar = new JToolBar();
        toolBar.add(subscribeAllAction);
        toolBar.add(unsubscribeAllAction);        
        toolBar.add(commitAction);
        toolBar.add(autoCommit);
        
        subscriptionPanel.add(toolBar, BorderLayout.SOUTH);
        
        panel.add(subscriptionPanel, BorderLayout.EAST);
        
        sp = new JScrollPane(eventTable);
        sp.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createTitledBorder("Events:"), BorderFactory.createLoweredBevelBorder()));
        panel.add(sp, BorderLayout.CENTER);
        
        JPanel statusLabelPanel = new JPanel(new BorderLayout());
        
        statusLabel.setText("OK");
        statusLabelPanel.add(statusLabel, BorderLayout.CENTER);
        statusLabelPanel.setBorder(BorderFactory.createLoweredBevelBorder());
        panel.add(statusLabelPanel, BorderLayout.SOUTH);
        
        toolBar = new JToolBar();
        toolBar.add(connectAction);
        toolBar.add(disconnectAction);
        toolBar.addSeparator();
        toolBar.add(clearAction);
        toolBar.add(showDetailsAction);
        getContentPane().add(toolBar, BorderLayout.NORTH);
        
        JMenuBar mb = new JMenuBar();
        
        JMenu menu = new JMenu("File");
        mb.add(menu);
        
        menu.add(connectAction);
        menu.add(disconnectAction);
        menu.addSeparator();
        menu.add(clearAction);
        menu.add(showDetailsAction);
        menu.addSeparator();
        menu.add(new ExitAction());
        setJMenuBar(mb);
        
        disconnectAction.setEnabled(false);
        subscriptionTable.setEnabled(false);
        subscribeAllAction.setEnabled(false);
        unsubscribeAllAction.setEnabled(false);
        //showDetailsAction.setEnabled(false);
        commitAction.setEnabled(false);
        
        controller.addEventListener(new MyNotificationListener());
        controller.addListener(new ControllerListener());
        eventTable.getSelectionModel().addListSelectionListener(showDetailsAction);
        
        MyCellRenderer boolanHandler = new MyCellRenderer(
                subscriptionTable.getDefaultRenderer(Boolean.class),
                subscriptionTable.getDefaultEditor(Boolean.class));
               
        MyCellRenderer stringHandler = new MyCellRenderer(
                subscriptionTable.getDefaultRenderer(String.class),
                subscriptionTable.getDefaultEditor(String.class));
        
        subscriptionTable.setDefaultRenderer(Boolean.class, boolanHandler);
        subscriptionTable.setDefaultRenderer(String.class, stringHandler);
        subscriptionTable.setDefaultEditor(Boolean.class, boolanHandler);
        subscriptionTable.setDefaultEditor(String.class, stringHandler);
    }
    
    private class ControllerListener implements ConnectionController.Listener {
        
        public void connected(final String host, final int port, final Set<EventTypeEnum> subscription) {
            if (SwingUtilities.isEventDispatchThread()) {
                setTitle("JGDI Event Monitor: " + host + ":" + port);
                connectAction.setEnabled(false);
                disconnectAction.setEnabled(true);
                subscriptionTable.setEnabled(true);
                subscribeAllAction.setEnabled(true);
                unsubscribeAllAction.setEnabled(true);
                commitAction.setEnabled(false);
                subscriptionTableModel.setSubscription(subscription);
                statusLabel.setText("Connected to " + host + ":" + port);
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        connected(host, port, subscription);
                    }
                });
            }
        }
        
        public void disconnected() {
            if (SwingUtilities.isEventDispatchThread()) {
                setTitle("JGDI Event Monitor: Not connected");
                connectAction.setEnabled(true);
                disconnectAction.setEnabled(false);
                subscriptionTable.setEnabled(false);
                subscribeAllAction.setEnabled(false);
                unsubscribeAllAction.setEnabled(false);
                commitAction.setEnabled(false);
                subscriptionTableModel.clearSubscription();
                statusLabel.setText("disconnected");
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        disconnected();
                    }
                });
            }
        }
        
        public void subscribed(Set<EventTypeEnum> types) {
            if (SwingUtilities.isEventDispatchThread()) {
                subscriptionTableModel.subscribed(types);
                statusLabel.setText(types + " subscribed");
            } else {
                final Set<EventTypeEnum> tmpTypes = new HashSet<EventTypeEnum>(types);
                SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        subscribed(tmpTypes);
                    }
                });
            }
        }
        
        public void unsubscribed(final Set<EventTypeEnum> types) {
            if (SwingUtilities.isEventDispatchThread()) {
                subscriptionTableModel.unsubscribed(types);
                statusLabel.setText(types + " unsubscribed");
            } else {
                final Set<EventTypeEnum> tmpTypes = new HashSet<EventTypeEnum>(types);
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        unsubscribed(tmpTypes);
                    }
                });
            }
        }
        
        public void errorOccured(final Throwable ex) {
            if (SwingUtilities.isEventDispatchThread()) {
                ErrorDialog.showErrorDialog(MainFrame.this, ex);
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        errorOccured(ex);
                    }
                });
            }
        }

        public void clearSubscription() {
            if (SwingUtilities.isEventDispatchThread()) {
                statusLabel.setText("event client has been stopped");
                MainFrame.this.subscriptionTableModel.clearSubscription();
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        clearSubscription();
                    }
                });
            }
            
        }

        public void subscriptionSet(Set<EventTypeEnum> types) {
            if (SwingUtilities.isEventDispatchThread()) {
                statusLabel.setText("subscription changed");
                subscriptionTableModel.setSubscription(types);
            } else {
                final Set<EventTypeEnum> tmpTypes = new HashSet<EventTypeEnum>(types);
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        subscriptionSet(tmpTypes);
                    }
                });
            }
        }
    }
    
    private class ShowDetailsAction extends AbstractAction implements ListSelectionListener  {
        private final static long serialVersionUID = -2007121301L;
        private final EventDetailsDialog eventDetailsDialog = new EventDetailsDialog(MainFrame.this);
        
        public ShowDetailsAction() {
            super("details");
        }
        
        public void actionPerformed(ActionEvent e) {
            eventDetailsDialog.setVisible(true);
        }
        

        public void valueChanged(ListSelectionEvent e) {
            if(!e.getValueIsAdjusting()) {
                int row = eventTable.getSelectedRow();
                if(row >= 0 && row < eventTableModel.getRowCount()) {
                    //showDetailsAction.setEnabled(true);
                    eventDetailsDialog.setEvent(eventTableModel.get(row));
                } else {
                    //showDetailsAction.setEnabled(false);
                }
            }
        }
    }
    
    private class ClearAction extends AbstractAction {
        private final static long serialVersionUID = -2007121301L;
        public ClearAction() {
            super("clear");
        }
        
        public void actionPerformed(ActionEvent e) {
            eventTableModel.clear();
        }        
    }
    
    private class CommitAction extends AbstractAction {
        
        private final static long serialVersionUID = -2008010901L;
        
        public CommitAction() {
            super("commit");
        }
        public void actionPerformed(ActionEvent e) {
            controller.setSubscription(subscriptionTableModel.getSubscription());
            setEnabled(false);
        }
    }
    
    private class SubscribeAllAction extends AbstractAction {
        private final static long serialVersionUID = -2008010901L;
        public SubscribeAllAction() {
            super("all");
        }
        
        public void actionPerformed(ActionEvent e) {
            if(autoCommit.isSelected()) {
                controller.subscribeAll();   
            } else {
                subscriptionTableModel.subscribeAll();
            }
        }        
    }
    
    private class UnsubscribeAllAction extends AbstractAction {
        private final static long serialVersionUID = -2008010901L;
        public UnsubscribeAllAction() {
            super("none");
        }
        
        public void actionPerformed(ActionEvent e) {
            if(autoCommit.isSelected()) {
                controller.unsubscribeAll();   
            } else {
                subscriptionTableModel.unsubscribeAll();
            }
        }        
    }
    
    
    private class ConnectAction extends AbstractAction {
        private final static long serialVersionUID = -2007121301L;
        
        ConnectDialog dlg = new ConnectDialog(MainFrame.this);
        
        public ConnectAction() {
            super("connect");
        }
        
        public void actionPerformed(ActionEvent e) {
            
            statusLabel.setText("Connecting");
            
            dlg.setLocationRelativeTo(MainFrame.this);
            while(true) {
                
                dlg.setVisible(true);
                
                if(dlg.isCanceled()) {
                    statusLabel.setText("Connecting canceled");
                    break;
                }
                
                try {
                    statusLabel.setText("Connecting to " + dlg.getHost() + ":" + dlg.getPort());
                    
                    Object credential = null;
                    if (dlg.useSSL()) {
                        KeyStore ks = createKeyStore(dlg.getKeystore(), dlg.getUsername(), dlg.getKeyStorePassword());
                        credential = JGDIProxy.createCredentialsFromKeyStore(ks, dlg.getUsername(), dlg.getKeyStorePassword());
                    } else {
                        credential = new String []  { dlg.getUsername(), new String(dlg.getPassword()) };
                    }
                    controller.connect(dlg.getHost(), dlg.getPort(), credential, dlg.getCaTop(), dlg.getKeystore(), dlg.getKeyStorePassword());
                    dlg.saveInPrefs();
                    break;
                } catch (Exception ex) {
                    ErrorDialog.showErrorDialog(MainFrame.this, ex);
                    break;
                }
            }
        }
        
        private KeyStore createKeyStore(String path, String username, char[] pw) throws Exception {

            File keyStoreFile = new File(path);
            FileInputStream fin = new FileInputStream(keyStoreFile);

            try {
                KeyStore ret = KeyStore.getInstance(KeyStore.getDefaultType());
                ret.load(fin, pw);

                if (!ret.isKeyEntry(username)) {
                    throw new Exception(MessageFormat.format("Keystore {0} does not contain credentials for user {1}", path, username));
                }
                return ret;
            } finally {
                fin.close();
            }
        }        
    }
    
    private class ExitAction extends AbstractAction {
        private final static long serialVersionUID = -2007121301L;
        
        public ExitAction() {
            super("Exit");
        }
        
        public void actionPerformed(ActionEvent e) {
            System.exit(0);
        }
    }
    
    private class DisConnectAction extends AbstractAction {
        private final static long serialVersionUID = -2007121301L;
        
        public DisConnectAction() {
            super("disconnect");
        }
        
        public void actionPerformed(ActionEvent e) {
            statusLabel.setText("Disconnecting");
            controller.disconnect();
        }
    }
    
    public class MyNotificationListener implements EventListener {
        
        
        public void eventOccured(final Event evt) {
            if(SwingUtilities.isEventDispatchThread()) {
                eventTableModel.add(evt);
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        eventOccured(evt);
                    }
                });
            }
        }
    }
    
    private class EventTableModel extends AbstractTableModel {
        private final static long serialVersionUID = -2008010901L;
        
        private final LinkedList<Event> rows = new LinkedList<Event>();
        
        public int getRowCount() {
            return rows.size();
        }

        public int getColumnCount() {
            return 1;
        }

        public Object getValueAt(int rowIndex, int columnIndex) {
            return rows.get(rowIndex).toString();
        }

        @Override
        public Class<?> getColumnClass(int columnIndex) {
            return String.class;
        }

        @Override
        public String getColumnName(int column) {
            return "Event";
        }

        @Override
        public boolean isCellEditable(int rowIndex, int columnIndex) {
            return false;
        }
        
        public Event get(int row) {
            return rows.get(row);
        }
        
        public void add(Event event) {
            int index = rows.size();
            rows.add(event);
            fireTableRowsInserted(index, index);
            if(index > 1000) {
                rows.removeFirst();
                fireTableRowsDeleted(0, 0);
            }
        }
        
        public void clear() {
            int count = rows.size();
            if (count > 0) {
                rows.clear();
                fireTableRowsDeleted(0, count -1);
            }
        }
        
        
    }
    
    private static final int TYPE_COL = 1;
    private static final int SUB_COL = 0;
    
    private class SubscriptionTableModel extends AbstractTableModel {
        private final static long serialVersionUID = -2007121301L;
        
        private final Map<EventTypeEnum, SubscriptionElem> subscription = new HashMap<EventTypeEnum, SubscriptionElem>();
        private final List<EventTypeEnum> subscriptionList;
        
        private class SubscriptionElem {
            boolean subscribed;
            boolean dirty;
            public SubscriptionElem() {
                subscribed = false;
                dirty = true;
            }
            
            public void clear() {
                subscribed = false;
                dirty = false;
            }
        }
        private final ConnectionController controller;
        
        public SubscriptionTableModel(ConnectionController controller) {
            
            this.controller = controller;
            subscriptionList = new ArrayList<EventTypeEnum>(Arrays.asList(EventTypeEnum.values()));
            
            Collections.sort(subscriptionList, new Comparator<EventTypeEnum>() {
                
                public int compare(EventTypeEnum o1, EventTypeEnum o2) {
                    return o1.toString().compareTo(o2.toString());
                }
            });
            
            for (EventTypeEnum type : subscriptionList) {
                subscription.put(type, new SubscriptionElem());
            }
        }
        
        public int getRowCount() {
            return subscriptionList.size();
        }
        
        public int getColumnCount() {
            return 2;
        }
        
        public Object getValueAt(int rowIndex, int columnIndex) {
            switch (columnIndex) {
                case TYPE_COL:
                    return subscriptionList.get(rowIndex);
                case SUB_COL:
                {
                    EventTypeEnum t = subscriptionList.get(rowIndex);
                    switch (t) {
                        case Shutdown:        return true;
                        case QmasterGoesDown: return true;
                        default:
                            return subscription.get(t).subscribed;
                    }
                }
                default:
                    return "Error";
            }
        }
        
        public boolean isDirty(int row) {
            EventTypeEnum type = subscriptionList.get(row);
            return subscription.get(type).dirty;
        } 
        
        @Override
        public boolean isCellEditable(int rowIndex, int columnIndex) {
            EventTypeEnum t = subscriptionList.get(rowIndex);
            switch(t) {
                case Shutdown: return false;
                case QmasterGoesDown: return false;
                default:
                     return columnIndex == SUB_COL;
            }
        }
        
        @Override
        public void setValueAt(Object aValue, final int rowIndex, int columnIndex) {
            if (columnIndex == SUB_COL) {
                EventTypeEnum type = subscriptionList.get(rowIndex);
                boolean subscribe = (Boolean) aValue;
                SubscriptionElem elem = subscription.get(type);
                elem.dirty = true;
                elem.subscribed = subscribe;
                if(autoCommit.isSelected()) {
                    if (subscribe) {
                        statusLabel.setText("subscribing " + type);
                        controller.subscribe(Collections.singleton(type));
                    } else {
                        statusLabel.setText("unsubscribing " + type);
                        controller.unsubscribe(Collections.singleton(type));
                    }
                } else {
                    commitAction.setEnabled(true);
                }
                SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        fireTableRowsUpdated(rowIndex, rowIndex);
                    }
                });
            }
        }
        
        
        public void clearSubscription() {
            for (Map.Entry<EventTypeEnum, SubscriptionElem> entry : subscription.entrySet()) {
                entry.getValue().clear();
            }
            fireTableDataChanged();
        }
        
        public EventTypeEnum getType(int row) {
            return subscriptionList.get(row);
        }
        
        public void setSubscription(final EventTypeEnum type, final boolean subscribed) {
            if(SwingUtilities.isEventDispatchThread()) {
                int row = subscriptionList.indexOf(type);
                if(row >= 0) {
                    SubscriptionElem elem = subscription.get(type);
                    
                    if (subscribed != elem.subscribed || elem.dirty ) {
                        elem.subscribed = subscribed;
                        elem.dirty = false;
                        fireTableCellUpdated(row, 0);
                    }
                }
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        setSubscription(type, subscribed);
                    }
                    
                });
            }
        }
        
        public void setSubscription(Set<EventTypeEnum> types) {
            if(SwingUtilities.isEventDispatchThread()) {
                for (Map.Entry<EventTypeEnum,SubscriptionElem> entry : subscription.entrySet()) {
                    entry.getValue().clear();
                }
                for(EventTypeEnum type: types) {
                    SubscriptionElem elem = subscription.get(type);
                    elem.subscribed = true;
                }
                fireTableDataChanged();
            } else {
                final Set<EventTypeEnum> tmpTypes = new HashSet<EventTypeEnum>(types);
                SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        setSubscription(tmpTypes);
                    }
                });
            }
        }
        
        public Set<EventTypeEnum> getSubscription() {
            Set<EventTypeEnum> ret = new HashSet<EventTypeEnum>();
            for(Map.Entry<EventTypeEnum,SubscriptionElem> entry: subscription.entrySet()) {
                if( entry.getValue().subscribed ) {
                    ret.add(entry.getKey());
                }
            }
            return ret;
        }
        
        
        public boolean isSubscribed(EventTypeEnum type) {
            return subscription.get(type).subscribed;
        }
        
        
        @Override
        public String getColumnName(int column) {
            switch (column) {
                case TYPE_COL:
                    return "Type";
                case SUB_COL:
                    return "";
                default:
                    return "Error";
            }
        }
        
        @Override
        public Class<?> getColumnClass(int columnIndex) {
            switch (columnIndex) {
                case TYPE_COL:
                    return String.class;
                case SUB_COL:
                    return Boolean.class;
                default:
                    return null;
            }
        }
        
        public void subscribeAll() {
            if (SwingUtilities.isEventDispatchThread()) {
                boolean changed = false;
                for(Map.Entry<EventTypeEnum,SubscriptionElem> elem: subscription.entrySet()) {
                    switch(elem.getKey()) {
                        case QmasterGoesDown:
                        break;
                        case Shutdown:
                        break;
                        default:
                            if (elem.getValue().subscribed == false) {
                                elem.getValue().subscribed = true;
                                elem.getValue().dirty = true;
                                changed = true;
                            }
                    }
                }
                if(changed) {
                    fireTableDataChanged();
                    commitAction.setEnabled(true);
                }
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        subscribeAll();
                    }
                });
            }
        }
        
        public void unsubscribeAll() {
            if (SwingUtilities.isEventDispatchThread()) {
                boolean changed = false;
                for(Map.Entry<EventTypeEnum,SubscriptionElem> elem: subscription.entrySet()) {
                    switch(elem.getKey()) {
                        case QmasterGoesDown:
                        break;
                        case Shutdown:
                        break;
                        default:
                        if(elem.getValue().subscribed == true) {
                            elem.getValue().subscribed = false;
                            elem.getValue().dirty = true;
                            changed  = true;
                        }
                    }
                }
                if(changed) {
                    fireTableDataChanged();
                    commitAction.setEnabled(true);
                }
            } else {
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        unsubscribeAll();
                    }
                });
            }
        }
        
        public void subscribed(Set<EventTypeEnum> types) {
            if (SwingUtilities.isEventDispatchThread()) {
                for (EventTypeEnum type : types) {
                    SubscriptionElem elem = subscription.get(type);
                    elem.subscribed = true;
                    elem.dirty = false;
                }
                fireTableDataChanged();
            } else {
                final Set<EventTypeEnum> tmpTypes = new HashSet<EventTypeEnum>(types);
                SwingUtilities.invokeLater(new Runnable() {
                    
                    public void run() {
                        subscribed(tmpTypes);
                    }
                });
            }
        }
        
        public void unsubscribed(Set<EventTypeEnum> types) {
            if (SwingUtilities.isEventDispatchThread()) {
                for (EventTypeEnum type : types) {
                    SubscriptionElem elem = subscription.get(type);
                    elem.subscribed = false;
                    elem.dirty = false;
                }
                fireTableDataChanged();
            } else {
                final Set<EventTypeEnum> tmpTypes = new HashSet<EventTypeEnum>(types);
                SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        unsubscribed(tmpTypes);
                    }
                });
            }
        }
    }
    
    private class MyCellRenderer implements TableCellRenderer, TableCellEditor {
        private final static long serialVersionUID = -2008011401L;
        private final TableCellRenderer rendererDelegate;
        private final TableCellEditor editorDelegate;
        
        public MyCellRenderer(TableCellRenderer delegate, TableCellEditor editorDelegate) {
            this.rendererDelegate = delegate;
            this.editorDelegate = editorDelegate;
        }
        public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {
            
            Component ret = rendererDelegate.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, column);
            
            if(subscriptionTable.isEnabled() && subscriptionTableModel.isDirty(row)) {
                ret.setForeground(Color.RED);
            } else {
                ret.setForeground(Color.BLACK);
            }
            return ret;
        }

        
        public Component getTableCellEditorComponent(JTable table, Object value, boolean isSelected, int row, int column) {
            Component ret = editorDelegate.getTableCellEditorComponent(table, value, isSelected, row, column);
            if(subscriptionTable.isEnabled() && subscriptionTableModel.isDirty(row)) {
                ret.setForeground(Color.RED);
            } else {
                ret.setForeground(Color.BLACK);
            }
            return ret;
        }

        public Object getCellEditorValue() {
            return editorDelegate.getCellEditorValue();
        }

        public boolean isCellEditable(EventObject anEvent) {
            return editorDelegate.isCellEditable(anEvent);
        }

        public boolean shouldSelectCell(EventObject anEvent) {
            return editorDelegate.shouldSelectCell(anEvent);
        }

        public boolean stopCellEditing() {
            return editorDelegate.stopCellEditing();
        }

        public void cancelCellEditing() {
            editorDelegate.cancelCellEditing();
        }

        public void addCellEditorListener(CellEditorListener l) {
            editorDelegate.addCellEditorListener(l);
        }

        public void removeCellEditorListener(CellEditorListener l) {
            editorDelegate.removeCellEditorListener(l);
        }
        
    }
    
}
