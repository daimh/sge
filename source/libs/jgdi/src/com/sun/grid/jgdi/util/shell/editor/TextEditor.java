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

package com.sun.grid.jgdi.util.shell.editor;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Font;

/**
 *
 */
public class TextEditor extends javax.swing.JFrame {
    private boolean isDone=false;
    private String text;
    /** Creates new form TextEditor */
    public TextEditor() {
        initComponents();
    }
    
    public TextEditor(String str) {
        initComponents();
        textArea.setText(str);
        this.setVisible(true);
    }
    
    private void initComponents() {
        jScrollPane1 = new javax.swing.JScrollPane();
        textArea = new javax.swing.JTextArea();
        
        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosed(java.awt.event.WindowEvent evt) {
                formWindowClosed(evt);
            }
        });
        
        textArea.setFont(new Font("Courier",Font.PLAIN,14));
        //textArea.putClientProperty(com.sun.java.swing.SwingUtilities2.AA_TEXT_PROPERTY_KEY, Boolean.TRUE);
        jScrollPane1.setViewportView(textArea);
        
        getContentPane().setLayout(new BorderLayout());
        getContentPane().add(jScrollPane1);
        
        pack();
        this.setSize(500,600);
    }
    
    private void formWindowClosed(java.awt.event.WindowEvent evt) {
        text = textArea.getText();
        isDone=true;
    }
    
    public String getText() {
        return text;
    }
    
    /** Provides an easy way of knowing if the editing was completed
     */
    public boolean isDone() {
        return isDone;
    }
    
    // Variables declaration - do not modify
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JTextArea textArea;
    // End of variables declaration
    
}