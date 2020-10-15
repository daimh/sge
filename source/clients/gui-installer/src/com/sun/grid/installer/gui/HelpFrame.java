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
import com.izforge.izpack.installer.InstallData;
import com.izforge.izpack.installer.InstallerFrame;
import com.izforge.izpack.installer.IzPanel;
import com.izforge.izpack.util.Debug;
import com.izforge.izpack.util.VariableSubstitutor;
import com.sun.grid.installer.util.Config;
import com.sun.grid.installer.util.FileHandler;

import com.sun.grid.installer.util.Util;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.net.MalformedURLException;
import java.net.URL;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JEditorPane;
import javax.swing.JFrame;
import javax.swing.JList;
import javax.swing.JSeparator;
import javax.swing.JTextField;
import javax.swing.ListCellRenderer;
import javax.swing.border.EmptyBorder;
import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultHighlighter;
import javax.swing.text.Document;
import javax.swing.text.Highlighter;
import javax.swing.text.html.HTMLEditorKit;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Properties;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

public class HelpFrame extends JFrame implements ActionListener, Config {

    private static final long serialVersionUID = -852797126101349127L;

    private JComboBox stepsComboBox = null;
    private JEditorPane editorPane = null;
    private JTextField searchField = null;
    private JButton searchButton = null;

    private InstallerFrame parent = null;
    private InstallData idata = null;

    private static HelpFrame helpFrame = null;

    public static URL errorPage = null;
    public static URL emptyPage = null;

    private static int DEF_WIDTH = 600;
    private static int DEF_HEIGH = 400;
    private static int MIN_WIDTH = 560;
    private static int MIN_HEIGH = 200;

    private String workDir = null;
    
    private String searchText = "";
    private static String SEARCH_RESULT_PAGE = "";
    private static String SEARCH_NO_RESULT_PAGE = "";

    /**
     * Constructor
     * @param parent The installer frame
     * @param idata The install data
     */
    private HelpFrame(InstallerFrame parent, InstallData idata) {
        super(parent.langpack.getString("installer.help"));
        setIconImage(parent.icons.getImageIcon("help").getImage());
        addWindowListener(new WindowAdapter() {

            @Override
            public void windowClosing(WindowEvent e) {
                super.windowClosing(e);
                helpFrame = null;
            }
        });

        this.parent = parent;
        this.idata = idata;

        SEARCH_RESULT_PAGE = parent.langpack.getString("search.result.page");
        SEARCH_NO_RESULT_PAGE = parent.langpack.getString("search.noresult.page");

        workDir = idata.getVariable(VAR_WORK_DIR);
        if (!workDir.endsWith(FileHandler.SEPARATOR)) {
            workDir += FileHandler.SEPARATOR;
        }

        errorPage = getURL(parent.langpack.getString(LANGID_HELP_ERROR_PAGE), null);
        emptyPage = getURL(parent.langpack.getString(LANGID_HELP_EMPTY_PAGE), errorPage);

        buidGUI();
        setSteps();
        loadSelectedPage();
    }

    /**
     * Builds the entire Help window
     */
    private void buidGUI() {
        editorPane = new JEditorPane();
        editorPane.setEditable(false);
        editorPane.setContentType("text/html");
        editorPane.setBackground(Color.white);
        editorPane.addHyperlinkListener(new HyperlinkListener() {
            public void hyperlinkUpdate(HyperlinkEvent e) {
                if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
                    if (e.getURL().getProtocol().equals("http")) {
                        if (Util.openBrowser(e.getURL().toString())) {
                            return;
                        }
                    }

                    loadURL(e.getURL());

                    for (int i = 0; i < stepsComboBox.getItemCount(); i++) {
                        if (stepsComboBox.getItemAt(i) instanceof UrlItem) {
                            UrlItem urlItem = (UrlItem) stepsComboBox.getItemAt(i);
                            if (urlItem.url.equals(e.getURL())) {
                                stepsComboBox.removeActionListener(HelpFrame.this);
                                stepsComboBox.setSelectedIndex(i);
                                stepsComboBox.addActionListener(HelpFrame.this);
                            }
                        }
                    }
                }
            }
        });
        
        editorPane.addPropertyChangeListener("page", new PropertyChangeListener() {

            public void propertyChange(PropertyChangeEvent evt) {
                higlightText(searchText);
            }
        });
        JScrollPane scrollPane = new JScrollPane(editorPane);

        JLabel stepsLabel = new JLabel(parent.langpack.getString("step.label"));
        //stepsLabel.setFont(stepsLabel.getFont().deriveFont(Font.BOLD));
        stepsLabel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 0));

        stepsComboBox = new JComboBox();
//        stepsComboBox.setRenderer(new SeparatorComboBoxRenderer());
        stepsComboBox.addActionListener(this);
        stepsComboBox.setPreferredSize(new Dimension(230, 25));

        JPanel stepsPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 0));
        stepsPanel.add(stepsLabel);
        stepsPanel.add(stepsComboBox);

        JPanel searchPanel = new JPanel(new FlowLayout(FlowLayout.RIGHT, 10, 0));
        searchField = new JTextField();
        searchField.setPreferredSize(new Dimension(150, 25));
        searchField.addKeyListener(new KeyListener() {

            public void keyPressed(KeyEvent e) {
                if (e.getKeyCode() == KeyEvent.VK_ENTER) {
                    search(searchField.getText());
                }
            }

            public void keyReleased(KeyEvent e) {
            }

            public void keyTyped(KeyEvent e) {
            }
        });
        searchButton = ButtonFactory.createButton(parent.langpack.getString("button.search.label"), parent.icons.getImageIcon("search"), idata.buttonsHColor);
        searchButton.setToolTipText(parent.langpack.getString("button.search.tooltip"));
        searchButton.addActionListener(this);
        searchPanel.add(searchField);
        searchPanel.add(searchButton);

        JPanel headPanel = new JPanel(new BorderLayout());
        headPanel.add(stepsPanel, BorderLayout.WEST);
        headPanel.add(searchPanel, BorderLayout.EAST);

        getContentPane().add(headPanel, BorderLayout.NORTH);
        getContentPane().add(scrollPane);

//        Color backgroundColor = null;
//        if (idata.guiPrefs.modifier.containsKey("headingBackgroundColor")) {
//            try {
//				backgroundColor = Color.decode(idata.guiPrefs.modifier.get("headingBackgroundColor"));
//				setBackground(backgroundColor);
//
//                headPanel.setBackground(backgroundColor);
//                headPanel.setOpaque(true);
//
//                searchPanel.setBackground(backgroundColor);
//                stepsPanel.setBackground(backgroundColor);
//                stepsPanel.setOpaque(true);
//
//			} catch (NumberFormatException e1) {
//				backgroundColor = null;
//			}
//        }
    }

    /**
     * Sets the content of the steps combo box with the panel names which have available help page.
     */
    public void setSteps() {
        stepsComboBox.removeActionListener(this);

        int selectedStep = stepsComboBox.getSelectedIndex();
        stepsComboBox.removeAllItems();

        ArrayList<UrlItem> helpUrls = getHelpUrls();
        for (int i = 0; i < helpUrls.size(); i++) {
            stepsComboBox.addItem(helpUrls.get(i));
        }

        if (selectedStep < stepsComboBox.getItemCount() && selectedStep != -1) {
            stepsComboBox.setSelectedIndex(selectedStep);
        } else {
            stepsComboBox.setSelectedIndex(0);
        }

        stepsComboBox.addActionListener(this);
    }

    /**
     * Loads the page into the editor pane selected in the combo box
     */
    private void loadSelectedPage() {
        Object selectedItem = stepsComboBox.getSelectedItem();
        if (stepsComboBox.getSelectedIndex() != -1 && selectedItem instanceof UrlItem) {
            UrlItem urlItem = (UrlItem) selectedItem;
            if (urlItem.url != null) {
                loadURL(urlItem.url);
            } else {
                loadText(urlItem.getContent());
            }
        }
    }

    /**
     * Opens the help frame
     * @param panelNumber The initial panel whose help page will be shown
     */
    public void openFrame(int panelNumber) {
        setHelpPage(panelNumber);

        if (!isVisible()) {
            setVisible(true);
        } else if (getState() == JFrame.ICONIFIED) {
            setState(JFrame.NORMAL);
        }
    }

    /**
     * Returns all of the available help pages specified for IzPanels
     * @return the pages have been found
     */
    private ArrayList<UrlItem> getHelpUrls() {
        List<IzPanel> panels = idata.panels;
        ArrayList<UrlItem> helpUrls = new ArrayList<UrlItem>(panels.size());
        for (int i = 0; i < panels.size(); i++) {
            IzPanel izPanel = panels.get(i);
            if (!izPanel.isHidden()) {
                UrlItem item = getHelpUrl(izPanel);

                if (item != null) {
                    helpUrls.add(item);
                }
            }
        }

        return helpUrls;
    }

    /**
     * Returns with the UrlItem belonging to the given izPanel.
     * @param izPanel The IzPanel
     * @return The UrlItem of IzPanel if there any otherwise null.
     */
    private UrlItem getHelpUrl(IzPanel izPanel) {
        VariableSubstitutor vs = new VariableSubstitutor(idata.getVariables());

        String name = izPanel.getI18nStringForClass("headline");
        String url = vs.substituteMultiple(izPanel.getI18nStringForClass("helppage"), null);

        if (name != null && !name.equals("") && url != null && !url.equals("")) {
            Debug.log("Found help page: " + name + " " + url);
            return new UrlItem(name, url);
        } else {
            return null;
        }
    }

    /**
     * Sets the help page for the given panel shown on the editor pane.
     * @param panelNumber The ordinal numer of the IzPanel.
     */
    public void setHelpPage(int panelNumber) {
        IzPanel panel = idata.panels.get(panelNumber);
        UrlItem item = getHelpUrl(panel);

        if (item != null) {
            stepsComboBox.setSelectedItem(item);
        }
    }

    /**
     * Returns with the single instance of the Help window positioned relative to the parent frame.
     * The orientation depends on the available space around the parent frame.
     * @param parent The InstallerFrame
     * @param idata The installation data
     * @return the singelton HelpFrame
     *
     * @see InstallerFrame
     * @see InstallData
     */
    public static HelpFrame getHelpFrame(InstallerFrame parent, InstallData idata) {
        if (helpFrame == null) {
            helpFrame = new HelpFrame(parent, idata);
            helpFrame.setLocationRelativeTo(parent);
            helpFrame.setSize(parent.getWidth() - 80, parent.getHeight() - 80);

            // imitate docked window
            Rectangle screenRect = new Rectangle(Toolkit.getDefaultToolkit().getScreenSize());
            Rectangle[] orientations = getFrameOrientations(parent.getBounds());
            for (int i = 0; i < orientations.length; i++) {
                if (screenRect.contains(orientations[i])) {
                    helpFrame.setBounds(orientations[i]);
                    break;
                }
            }

            //helpFrame.setMinimumSize(new Dimension(MIN_WIDTH, MIN_HEIGH));
        }

        return helpFrame;
    }

    /**
     * Counts the possible orientations realtive to the parent's frame rectangle.
     * @param parent The parent's frame rectangle
     * @return orientations counted from the rectangle. Order: left, rigth, up, down
     */
    private static Rectangle[] getFrameOrientations(Rectangle parent) {
        return new Rectangle[]{
                    new Rectangle(parent.x + parent.width, parent.y, MIN_WIDTH, parent.height),
                    new Rectangle(parent.x - MIN_WIDTH, parent.y, MIN_WIDTH, parent.height),
                    new Rectangle(parent.x, parent.y + parent.height, parent.width, MIN_HEIGH),
                    new Rectangle(parent.x, parent.y - MIN_HEIGH, parent.width, MIN_HEIGH)};
    }

    /**
     * Loads a single text value into the editor pane.
     * @param content The text value has to be loaded
     */
    private void loadText(String content) {
        editorPane.setText(content);
    }

    /**
     * Sets a page for editor pane to be displayed
     * @param url The URL of the page
     */
    private void loadURL(URL url) {
        try {
            editorPane.setPage(url);
        } catch (Exception e) { //ex.: IOException or FileNotFoundException
            Debug.error("Could not load page! Reason: " + e.getLocalizedMessage());
        }
    }

    /**
     * Highlights a text in the content being displayed on the editor pane.
     * @param textToHighlight The text to be highlighted
     */
    private void higlightText(String textToHighlight) {
        if (textToHighlight.equals("")) {
            return;
        }

        // remove previous higlights
        Highlighter highlighter = editorPane.getHighlighter();
        highlighter.removeAllHighlights();

        // get the text desplayed on the pane
        String text = "";
        Document doc = editorPane.getDocument();
        try {
            text = doc.getText(0, doc.getLength()).toLowerCase();
        } catch (BadLocationException e1) {
            Debug.error(e1);
            return;
        }

        // find the indexes of the given text in the document
        ArrayList<Integer> result = indexesOf(text, textToHighlight);

        // set the highlights depending on the indexies has been found
        for (Iterator iter = result.iterator(); iter.hasNext();) {
            Integer element = (Integer) iter.next();
            try {
                highlighter.addHighlight(element.intValue(), element.intValue() + textToHighlight.length(), new DefaultHighlighter.DefaultHighlightPainter(Color.yellow));
            } catch (BadLocationException e) {
                Debug.error(e);
            }
        }
    }

    public void actionPerformed(ActionEvent e) {
        if (e.getSource().equals(stepsComboBox)) {
            loadSelectedPage();
        } else if (e.getSource().equals(searchButton)) {
            search(searchField.getText());
        }
    }

    /**
     * Searches for the given text between the help pages
     * @param text
     */
    private void search(String text) {
        if (text.equals("")) {
            return;
        }

        searchText = text;

        // keep only those help pages which hold the search text
        ArrayList<UrlItem> helpUrls = getHelpUrls();
        for (int i = helpUrls.size() - 1; i >= 0; i--) {
            if (!searchPage(helpUrls.get(i).url, text)) {
                helpUrls.remove(i);
            }
        }

        // create the search result page
        UrlItem searchUrl = new UrlItem(parent.langpack.getString("button.search.label") + "...");
        searchUrl.setContent(generateSearchResultPage(helpUrls));

        // refress the gui
        setSteps();
//    	stepsComboBox.removeActionListener(this);
//    	stepsComboBox.addItem(new JSeparator(JSeparator.HORIZONTAL));
//    	stepsComboBox.addItem(searchUrl);
//    	stepsComboBox.addActionListener(this);
//    	stepsComboBox.setSelectedItem(searchUrl);
        loadText(searchUrl.getContent());
    }

    /**
     * Searches for a text in the given page
     * @param page The URL of the page to search in
     * @param searchText The text value to search for.
     * @return true if the page coantins the given text value, false otherwise.
     */
    private boolean searchPage(URL page, String searchText) {
        try {
            // create a document instance to search only in the view
            // and to exlude the formatting texts like HTML tags
            HTMLEditorKit editorKit = new HTMLEditorKit();
            Document doc = editorKit.createDefaultDocument();
            doc.putProperty("IgnoreCharsetDirective", new Boolean(true));
            editorKit.read(page.openStream(), doc, 0);

            String content = doc.getText(0, doc.getLength());

            return content.toLowerCase().indexOf(searchText.toLowerCase()) > -1;
        } catch (Exception e) {
            Debug.error(e);
        }

        return false;
    }

    /**
     * Generates a search result page.
     * @param urls The links of the pages to be shown on the search result page
     * @return The HTML formatted search result pane if urls >=1, otherwise the "No matches found" page.
     */
    private String generateSearchResultPage(ArrayList<UrlItem> urls) {
        String srp = "";

        if (urls != null && urls.size() > 0) {
            // generate the HTML links
            StringBuffer sr = new StringBuffer();
            for (int i = 0; i < urls.size(); i++) {
                sr.append("<a href=\"" + urls.get(i).url + "\">" + urls.get(i).name + "</a>");
                sr.append("<br>");
            }

            // build a Properties instance for substitution
            Properties prop = new Properties();
            prop.put("search.result", sr.toString());

            // substitute the '${search.result}' value with the HTML links
            VariableSubstitutor vs = new VariableSubstitutor(prop);
            srp = vs.substitute(SEARCH_RESULT_PAGE, null);

        } else {
            srp = SEARCH_NO_RESULT_PAGE;
        }

        return srp;
    }

    /**
     * Creates an URL instance
     * @param url A string represented url
     * @param defaultUrl The default URL instance
     * @return the new URL instance has been created. If an exception occurs during the instantiation returns with the default URL
     */
    public static URL getURL(String url, URL defaultUrl) {
        try {
            return new URL(url);
        } catch (MalformedURLException e) {
            Debug.error("Malformed URL: " + url);

            return defaultUrl;
        }
    }

    /**
     * Returns all of the indexes within the string of the specified substring
     * @param string The string to search in
     * @param textToFind The text to find
     * @return all of the found occurences of the given text.
     */
    private static ArrayList<Integer> indexesOf(String string, String textToFind) {
        ArrayList<Integer> indexes = new ArrayList<Integer>();
        int from = 0;
        while (from < string.length()) {
            int index = string.indexOf(textToFind, from);
            if (index != -1) {
                indexes.add(new Integer(index));
                from = index + 1;
            } else {
                break;
            }
        }

        return indexes;
    }

    /**
     * Class to encapsulate an URL a name and a content value.
     */
    private class UrlItem {

        private String name = "";
        private URL url = null;
        private String content = "";

        public UrlItem(String name) {
            this.name = name;
        }

        public UrlItem(String name, String url) {
            this(name, HelpFrame.getURL(url, HelpFrame.errorPage));
        }

        public UrlItem(String name, URL url) {
            this.name = name;
            this.url = url;
        }

        public String getContent() {
            return content;
        }

        public void setContent(String content) {
            this.content = content;
        }

        @Override
        public String toString() {
            return name;
        }

        @Override
        public boolean equals(Object obj) {
            if (obj == this) {
                return true;
            } else if (obj instanceof UrlItem) {
                UrlItem o = (UrlItem) obj;

                boolean result = true;
                if (url != null && o.url != null) {
                    result = result && url.equals(o.url);
                }

                return result && name.equals(o.name) && content.equals(o.content);
            } else {
                return false;
            }
        }

        @Override
        public int hashCode() {
            int hash = 7;
            hash = 29 * hash + (this.name != null ? this.name.hashCode() : 0);
            hash = 29 * hash + (this.url != null ? this.url.hashCode() : 0);
            return hash;
        }
    }

    /**
     * Class to render separator like combo box element
     */
    private class SeparatorComboBoxRenderer implements ListCellRenderer {

        private JLabel label = null;

        public SeparatorComboBoxRenderer() {
            label = new JLabel();
            label.setOpaque(true);
            label.setBorder(new EmptyBorder(2, 2, 2, 2));
        }

        public Component getListCellRendererComponent(JList list, Object value, int index, boolean isSelected, boolean cellHasFocus) {
            String text = "";

            if (value instanceof UrlItem) {
                text = ((UrlItem) value).name;
            } else if (value instanceof JSeparator) {
                return (JSeparator) value;
            }

            label.setFont(list.getFont());
            label.setText(text);

            if (isSelected) {
                label.setBackground(list.getSelectionBackground());
                label.setForeground(list.getSelectionForeground());
            } else {
                label.setBackground(list.getBackground());
                label.setForeground(list.getForeground());
            }

            return label;
        }
    }
}
