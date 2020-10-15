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

import javax.swing.event.TableModelEvent;

public class HostInstallTableModel extends SortedTableModel implements HostTableModel {
    private String[] headers = null;
    private Class[] types = null;

    private HostList hostList;

    public HostInstallTableModel(HostList hostList, String[] headers, Class[] types) {
        super(new Object[][]{}, headers);
        this.hostList = hostList;
        this.headers = headers;
        this.types = types;
    }

    @Override
    public int getColumnCount() {
        return (types.length);
    }


    @Override
    public int getRowCount() {
        return (hostList == null) ? 0 : hostList.size();
    }

    @Override
    public Object getValueAt(int row, int col) {
        row = getSortedRowIndex(row);
        if (row >= hostList.size()) {
            return null;
        }
        
        Host h = hostList.get(row);

        switch (col) {
            case 0: return h.getComponentString();
            case 1: return h.getDisplayName();
            case 2: return h.getIp();
            case 3: return h.getArchitecture();
            case 4: return h.getState();
            case 5: return h.getLogContent();
            default: throw new IndexOutOfBoundsException("Invalid index rowIndex="+row+" columnIndex="+col);
        }
    }

    @Override
    public void setValueAt(Object aValue, int row, int column) {
        // Necessary to avoid invalid recalls
        //super.setValueAt(aValue, row, column);
    }

    @Override
    public Class getColumnClass(int columnIndex) {
        return types[columnIndex];
    }

    @Override
    public boolean isCellEditable(int row, int col) {
        if (col==5) {
            row = getSortedRowIndex(row);

            Host host = hostList.get(row);
            
            return (host.getLogContent() != null && !host.getLogContent().equals(""));
        } else {
            return false;
        }
    }

    public boolean removeAll() {
        boolean result =  hostList.removeAll(hostList);

        sortedIndexes = null;

        fireTableChanged(new TableModelEvent(this));
        return result;
    }

    public void setHostState(Host h, Host.State state) {
        h.setState(state);

        int index = hostList.indexOf(h);
        if (index == -1) {
            return;
        }
        
        int row = getRowIndex(index);

        if (row > -1) {
            fireTableCellUpdated(row, 4);
        }
    }

    public Host addHost(Host h) {
        int row = hostList.size();
        boolean result = hostList.addUnchecked(h);
        fireTableRowsInserted(row, row);
        return (result ? h : null);
    }

    public void removeHost(Host h) {
        int row = getRowIndex(hostList.indexOf(h));
        if (row == -1 || row >= hostList.size()) {
            return;
        }

        hostList.removeUnchecked(h);

        if (row > -1) {
            reSort();

            fireTableRowsDeleted(row, row);
            fireTableChanged(new TableModelEvent(this));
        }
    }

    public void setHostLog(Host h, String log) {
        h.setLogContent(log);
        int row = getRowIndex(hostList.indexOf(h));
        
        if (row > -1) {
            fireTableRowsUpdated(row, row);
        }
    }

    public HostList getHostList() {
        return hostList;
    }
}
