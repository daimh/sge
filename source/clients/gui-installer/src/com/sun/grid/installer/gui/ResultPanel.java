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

import com.izforge.izpack.gui.ButtonFactory;
import com.izforge.izpack.gui.FlowLayout;
import com.izforge.izpack.gui.IzPanelLayout;
import com.izforge.izpack.gui.LabelFactory;
import com.izforge.izpack.gui.LayoutConstants;
import com.izforge.izpack.installer.InstallData;
import com.izforge.izpack.installer.InstallerFrame;
import com.izforge.izpack.installer.IzPanel;
import com.izforge.izpack.util.VariableSubstitutor;
import com.izforge.izpack.util.Debug;
import com.sun.grid.installer.util.Config;
import com.sun.grid.installer.util.FileHandler;
import com.sun.grid.installer.util.Util;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.print.PageFormat;
import java.awt.print.Printable;
import java.awt.print.PrinterException;
import java.awt.print.PrinterJob;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Vector;
import javax.swing.*;
import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;
import net.n3.nanoxml.XMLElement;

public class ResultPanel extends IzPanel implements Printable, Config {

    /**
     * The info string.
     */
    private JEditorPane editorPane = null;
    private String readmeTemplatePath = "";
    private String readmePath = "";
    private ActionListener[] nextButtonActionListeners = null;

    /**
     * The constructor.
     *
     * @param parent The parent window.
     * @param idata  The installation data.
     */
    public ResultPanel(InstallerFrame parent, InstallData idata) {
        super(parent, idata, new IzPanelLayout());

        // The info label.
        add(LabelFactory.create(parent.langpack.getString("result.info.label"),
                parent.icons.getImageIcon("edit"), SwingConstants.LEADING), LayoutConstants.NEXT_LINE);
        // The text area which shows the info.
        editorPane = new JEditorPane();
        editorPane.setCaretPosition(0);
        editorPane.setContentType("text/html");
        editorPane.setEditable(false);
        editorPane.setBackground(Color.white);
        editorPane.addHyperlinkListener(new HyperlinkListener() {
            public void hyperlinkUpdate(HyperlinkEvent e) {
                if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
                    if (!Util.openBrowser(e.getURL().toString())) {
                        emitError(ResultPanel.this.parent.langpack.getString("error.can.not.open.browser.title"),
                                ResultPanel.this.parent.langpack.getString("error.can.not.open.browser.message"));
                    }
                }
            }
        });
        JScrollPane scroller = new JScrollPane(editorPane);

        JButton saveButton = ButtonFactory.createButton(parent.langpack.getString("button.save.label"), parent.icons.getImageIcon("save"), idata.buttonsHColor);
        saveButton.setToolTipText(parent.langpack.getString("button.save.tooltip"));
        saveButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                final JFileChooser fc = new JFileChooser();
                fc.setDialogType(JFileChooser.SAVE_DIALOG);
                fc.setMultiSelectionEnabled(false);
                int ret = fc.showSaveDialog(ResultPanel.this.getParent());
                if (ret == JFileChooser.APPROVE_OPTION) {
                    try {
                        File f = fc.getSelectedFile();
                        ArrayList<String> content = FileHandler.readFileContent(readmePath, true);

                        FileHandler.generateFile(content, f.getAbsolutePath());
                    } catch (FileNotFoundException ex) {
                        Debug.error(ex);
                    } catch (IOException ex) {
                        Debug.error(ex);
                    }

                }
            }
        });

        JButton printButton = ButtonFactory.createButton(parent.langpack.getString("button.print.label"), parent.icons.getImageIcon("print"), idata.buttonsHColor);
        printButton.setToolTipText(parent.langpack.getString("button.print.tooltip"));
        printButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                PrinterJob job = PrinterJob.getPrinterJob();
                job.setPrintable(ResultPanel.this);
                if (job.printDialog()) {
                    try {
                        job.print();
                    } catch (PrinterException pe) {
                        Debug.error(pe);
                    }
                }
            }
        });

        JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 0));
        buttonPanel.add(saveButton);
        buttonPanel.add(printButton);

        add(scroller, LayoutConstants.NEXT_LINE);
        add(buttonPanel, LayoutConstants.NEXT_LINE);

        // At end of layouting we should call the completeLayout method also they do nothing.
        getLayoutHelper().completeLayout();
    }

    /**
     * Loads the info text.
     */
    private void loadResult() {
        try {
            VariableSubstitutor vs = new VariableSubstitutor(idata.getVariables());
            readmeTemplatePath = vs.substituteMultiple(idata.getVariable(VAR_README_TEMP_FILE), null);

            readmePath = vs.substituteMultiple(idata.getVariable(VAR_README_FILE_NAME_1), null);
            readmePath += "_" + Util.generateTimeStamp() + ".html";
            //TODO: Detect whole cluster settings
            // Features - bootstrap (CSP, AFS, JMX)
            // spooling - bootstrap (BDB server, bdb, classic)
            // qmaster - act_qmaster
            // shadowds - shadow_masters
            // execd - qconf -sel
            // submit hosts - qconf -ss
            // admin hosts - qconf -sh
            Util.fillUpTemplate(readmeTemplatePath, readmePath, idata.getVariables(), new String[]{"<!--", "/*", "*", "-->"}, true);
            Debug.trace("Generating readme.html file: '" + readmePath + "'.");

            editorPane.setPage("file://" + readmePath);

            String  readmePath2 = vs.substituteMultiple(idata.getVariable(VAR_README_FILE_NAME_2), null);
            readmePath2 += "_" + Util.generateTimeStamp() + ".html";

            Util.fillUpTemplate(readmeTemplatePath, readmePath2, idata.getVariables(), new String[]{"<!--", "/*", "*"}, true);
        } catch (Exception e) {
            Debug.error("Can not generate readme file! " + e);
        }
    }

    @Override
    public void panelActivate() {
        parent.setQuitButtonText(parent.langpack.getString("FinishPanel.done"));
        parent.setQuitButtonIcon("done");

        JButton nextButton = parent.getNextButton();
        nextButton.setText(parent.langpack.getString("button.startover.label"));
        nextButton.setIcon(parent.icons.getImageIcon("refresh"));
        nextButton.setVisible(true);
        nextButton.setToolTipText(parent.langpack.getString("button.startover.tooltip"));
        nextButtonActionListeners = Util.removeListeners(nextButton);
        parent.unlockNextButton();
        nextButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    Debug.trace("@@@@@@@@@@@@@ Continue @@@@@@@@@@@@@");
                    resetInstaller();

                    parent.navigate(idata.curPanelNumber, 3);
                } catch (Exception ex) {
                    Debug.error("Can not continue the installation! " + ex);
                }
            }
        });

        loadResult();

        // TODO Find a proper place for the output
        // Generate auto installation file
//        FileOutputStream out = null;
//        BufferedOutputStream outBuff = null;
//        try {
//            VariableSubstitutor vs = new VariableSubstitutor(idata.getVariables());
//            String silentInstallationFile = vs.substituteMultiple(idata.getVariable(VAR_SILENT_INSTALL_FILE), null);
//            silentInstallationFile += "_" + Util.generateTimeStamp() + ".xml";
//
//            Debug.trace("Generating auto installation file: '" + silentInstallationFile + "'.");
//
//            // Find the cfg.sge.jmx.ssl.keystore.pw entry and replace it with "changeit"
//            Vector<XMLElement> userInputPanelElements = idata.xmlData.getChildrenNamed("com.izforge.izpack.panels.UserInputPanel");
//            searchEnd:
//            for (XMLElement userInputPanelElement : userInputPanelElements) {
//                Vector<XMLElement> userInputElements = userInputPanelElement.getFirstChildNamed("userInput").getChildrenNamed("entry");
//                for (XMLElement userInputElement : userInputElements) {
//                    if (userInputElement.getAttribute("key").equals(VAR_JMX_SSL_KEYSTORE_PWD)) {
//                        userInputElement.setAttribute("value", "changeit");
//                        break searchEnd;
//                    }
//                }
//            }
//
//            out = new FileOutputStream(silentInstallationFile);
//            outBuff = new BufferedOutputStream(out, 5120);
//            parent.writeXMLTree(idata.xmlData, outBuff);
//            outBuff.flush();
//
//        } catch (Exception e) {
//            Debug.error("Can not write auto installation file! " + e);
//        } finally {
//            try {
//                out.close();
//                outBuff.close();
//            } catch (Exception ex) {}
//        }
    }

    /**
     * Indicates wether the panel has been validated or not.
     *
     * @return Always true.
     */
    @Override
    public boolean isValidated() {
        return true;
    }

    public int print(Graphics graphics, PageFormat pageFormat, int pageIndex) throws PrinterException {
        if (pageIndex != 0) {
            return Printable.NO_SUCH_PAGE;
        } else {
            Graphics2D g2d = (Graphics2D) graphics;
            g2d.translate(pageFormat.getImageableX(),
                    pageFormat.getImageableY());
            int cw = editorPane.getWidth();
            int ch = editorPane.getHeight();
            double pw = pageFormat.getImageableWidth();
            double ph = pageFormat.getImageableHeight();
            if (cw > pw || ch > ph) {
                double scaleX = pw / cw;
                double scaleY = ph / ch;
                double scale = scaleX < scaleY ? scaleX : scaleY;
                g2d.scale(scale, scale);
            }

            editorPane.printAll((Graphics) g2d);
            return Printable.PAGE_EXISTS;
        }
    }

    /**
     * Reset the vales/components for starting over the installation
     * @throws java.lang.Exception
     */
    private void resetInstaller() throws Exception {
        /**
         * Reset install data
         */
        // Select qmaster component to be installed in the next round if it failed at last.
        if (idata.getVariable(VAR_QMASTER_HOST).equals("") && !idata.getVariable(VAR_QMASTER_HOST_FAILED).equals("")) {
            idata.setVariable(VAR_INSTALL_QMASTER, "true");
            idata.setVariable(VAR_QMASTER_HOST, idata.getVariable(VAR_QMASTER_HOST_FAILED));
        } else {
            idata.setVariable(VAR_INSTALL_QMASTER, "false");
            idata.setVariable(VAR_QMASTER_HOST, Host.localHostName);

            idata.setVariable(VAR_QMASTER_SPOOL_DIR, "${cfg.sge.root}/${cfg.cell.name}/spool/qmaster");
            idata.setVariable(VAR_EXECD_SPOOL_DIR, "${cfg.sge.root}/${cfg.cell.name}/spool");
            idata.setVariable(VAR_JMX_SSL_KEYSTORE, "/var/lib/sgeCA/port${cfg.sge.qmaster.port}/${cfg.cell.name}/private/keystore");
            idata.setVariable(VAR_DB_SPOOLING_DIR_BDB, "${cfg.sge.root}/${cfg.cell.name}/spool/spooldb");
        }
        
        idata.setVariable(VAR_INSTALL_EXECD, "true");
        idata.setVariable(VAR_INSTALL_SHADOW, "false");

        // Select bdbserver component to be installed in the next round if it failed at last.
        idata.setVariable(VAR_SPOOLING_METHOD_BERKELEYDBSERVER, "none");
        if (idata.getVariable(VAR_DB_SPOOLING_SERVER).equals("") && !idata.getVariable(VAR_DB_SPOOLING_SERVER_FAILED).equals("")) {
            idata.setVariable(VAR_INSTALL_BDB, "true");
            idata.setVariable(VAR_SPOOLING_METHOD, idata.getVariable(VAR_SPOOLING_METHOD_BERKELEYDBSERVER));
            idata.setVariable(VAR_DB_SPOOLING_SERVER, idata.getVariable(VAR_DB_SPOOLING_SERVER_FAILED));
        } else {
            idata.setVariable(VAR_INSTALL_BDB, "false");
            idata.setVariable(VAR_SPOOLING_METHOD, idata.getVariable(VAR_SPOOLING_METHOD_BERKELEYDB));
            idata.setVariable(VAR_DB_SPOOLING_SERVER, Host.localHostName);
        }

        //idata.setVariable(VAR_INSTALL_MODE, ""); leave the mode the same

        idata.setVariable(VAR_EXEC_HOST_LIST, "");
        idata.setVariable(VAR_SHADOW_HOST_LIST, "");       
        idata.setVariable(VAR_ADMIN_HOST_LIST, "");
        idata.setVariable(VAR_SUBMIT_HOST_LIST, "");

        /**
         * Reset buttons
         */
        JButton nextButton = parent.getNextButton();
        Util.removeListeners(nextButton);
        for (ActionListener actionListener : nextButtonActionListeners) {
            nextButton.addActionListener(actionListener);
        }
        nextButton.setText(parent.langpack.getString("installer.next"));
        nextButton.setIcon(parent.icons.getImageIcon("stepforward"));
        nextButton.setToolTipText(null);

        parent.setQuitButtonText(parent.langpack.getString("installer.quit"));
        parent.setQuitButtonIcon("stop");

        /**
         * Reset panels
         */
        HostPanel hostPanel = (HostPanel)idata.panels.get(idata.curPanelNumber - 2);
        Method initMethod = HostPanel.class.getDeclaredMethod("init", (Class[])null);
        initMethod.setAccessible(true);
        initMethod.invoke(hostPanel, (Object[])null);
    }
}
