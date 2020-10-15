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

import com.izforge.izpack.gui.IzPanelConstraints;
import com.izforge.izpack.gui.IzPanelLayout;
import com.izforge.izpack.gui.LayoutConstants;
import com.izforge.izpack.installer.GUIListener;
import com.izforge.izpack.installer.InstallData;
import com.izforge.izpack.installer.InstallerFrame;
import com.izforge.izpack.installer.IzPanel;
import com.izforge.izpack.util.VariableSubstitutor;
import com.sun.grid.installer.util.Config;
import javax.swing.JLabel;

/**
 * Abstarct panel class whitch offers the faacility to create view-less background
 * process panels.
 */
public abstract class ActionPanel extends IzPanel implements Config, GUIListener {
    /**
     * String suffix to store the execution time for the current panel in the
     * {@link InstallData}. Format <panelId>.<PROP_NUM_OF_EXECUTION>
     */
    private static final String PROP_NUM_OF_EXECUTION = "numOfExecution";

    /**
     * The id of the panel.
     */
    private String panelId = "";

    /**
     * Indicates the maximum time the undelying actions should be called.
     */
    private int numOfMaxExecution = 1;

    /**
     * Indicates whether this panel is activated or not.
     */
    private boolean panelActivated = false;

    /**
     * Constructor
     * @param parent The parent {@link InstallerFrame}
     * @param idata The {@link InstallData}
     */
    public ActionPanel(InstallerFrame parent, InstallData idata) {
        super(parent, idata, (parent == null ? null : new IzPanelLayout(LayoutConstants.FILL_OUT_COLUMN_WIDTH)));

        if (parent != null) {
            buildPanel();

            parent.addGuiListener(this);
        }
    }

    /**
     * Builds the panel
     */
    private void buildPanel() {
        VariableSubstitutor vs = new VariableSubstitutor(idata.getVariables());

        // Draw the "Please wait..." text
        JLabel textLabel = new JLabel(vs.substituteMultiple(parent.langpack.getString("title.please.wait"), null));
        textLabel.setHorizontalAlignment(JLabel.CENTER);

        // Center the text
        IzPanelConstraints ipc = new IzPanelConstraints();
        ipc.setXStretch(1.0);
        ipc.setYStretch(1.0);
        
        add(textLabel, ipc);

        getLayoutHelper().completeLayout();
    }

    /**
     * Executes the underlying actions. It's called only if the maximum number of
     * execution hasn't been reached.
     */
    abstract public void doAction();
    
    @Override
    public void panelActivate() {
        panelActivated = true;
    }

    @Override
    public void panelDeactivate() {
        panelActivated = false;
    }

    /**
     * Called from the parent {@link InstallerFrame} any time an action performed.
     * Never call it explicitly.
     *
     * @param what the id of the action
     * @param arg the argument
     */
    public void guiActionPerformed(int what, Object arg) {
        /**
         * We are interested only in panel switching events and only if the current
         * panel is visible
         */
        if (panelActivated && what == GUIListener.PANEL_SWITCHED) {
            // Spawn a new working thread in order to let the event thread finish properly
            new Thread(new Runnable() {
                public void run() {
                    parent.blockGUI();

                    panelId = idata.panels.get(idata.curPanelNumber).getMetadata().getPanelid();

                    // call the actions only if we haven't reached the maximum execution number
                    if (getNumOfExecution() < numOfMaxExecution) {
                        doAction();

                        incNumOfExecution();
                    }

                    parent.releaseGUI();

                    // finally jump to the next panel
                    getInstallerFrame().skipPanel();
                }
            }).start();
        }
    }

    /**
     * Returns the execution number of the current panel
     * @return the number of the execution
     *
     * @see ActionPanel#incNumOfExecution()
     * @see ActionPanel#getNumOfMaxExecution()
     * @see ActionPanel#setNumOfMaxExecution()
     */
    public int getNumOfExecution() {
        int num = 0;
        String numStr= null;
        
        if ((numStr = idata.getVariable(panelId + D + PROP_NUM_OF_EXECUTION)) != null) {
            try {
                num = Integer.parseInt(numStr);
            } catch (NumberFormatException e) {
                throw new IllegalStateException("Corrupted panel execution number: " + numStr);
            }
        }
        
        return num;
    }

    /**
     * Increments the number by one the current panel has gotten executed
     *
     * @see ActionPanel#getNumOfExecution()
     * @see ActionPanel#getNumOfMaxExecution()
     * @see ActionPanel#setNumOfMaxExecution()
     */
    public void incNumOfExecution() {
        int actNum = getNumOfExecution();
        actNum++;
        idata.setVariable(panelId + D + PROP_NUM_OF_EXECUTION, String.valueOf(actNum));
    }

    /**
     * Returns the id of the current panel.
     * @return the panel id
     */
    public String getPanelId() {
        return panelId;
    }

    /**
     * Returns the maximum number the current panel should be executed
     * @return the maximum execution number
     *
     * @see ActionPanel#getNumOfExecution()
     * @see ActionPanel#incNumOfExecution()
     * @see ActionPanel#setNumOfMaxExecution()
     */
    public int getNumOfMaxExecution() {
        return numOfMaxExecution;
    }

    /**
     * Sets the maximum number the current panel should be executed
     * @param numOfMaxExecution the maximum execution number
     *
     * @see ActionPanel#getNumOfExecution()
     * @see ActionPanel#incNumOfExecution()
     * @see ActionPanel#getNumOfMaxExecution()
     */
    public void setNumOfMaxExecution(int numOfMaxExecution) {
        this.numOfMaxExecution = numOfMaxExecution;
    }
    }
