/*
 * IzPack - Copyright 2001-2008 Julien Ponge, All Rights Reserved.
 *
 * http://izpack.org/
 * http://izpack.codehaus.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.izforge.izpack.installer;

import com.izforge.izpack.Info;
import com.izforge.izpack.LocaleDatabase;
import com.izforge.izpack.Pack;
import net.n3.nanoxml.XMLElement;

import java.util.*;
import java.util.zip.ZipOutputStream;

/**
 * Encloses information about the install process. This implementation is not thread safe.
 *
 * @author Julien Ponge <julien@izforge.com>
 * @author Johannes Lehtinen <johannes.lehtinen@iki.fi>
 */
public class AutomatedInstallData {

    public static final String MODIFY_INSTALLATION = "modify.izpack.install";
    public static final String INSTALLATION_INFORMATION = ".installationinformation";

    static final String[] CUSTOM_ACTION_TYPES = new String[]{"/installerListeners",
            "/uninstallerListeners", "/uninstallerLibs", "/uninstallerJars"};

    public static final int INSTALLER_LISTENER_INDEX = 0;

    public static final int UNINSTALLER_LISTENER_INDEX = 1;

    public static final int UNINSTALLER_LIBS_INDEX = 2;

    public static final int UNINSTALLER_JARS_INDEX = 3;
    public String localeISO3;
    public Locale locale;
    public LocaleDatabase langpack;
    public ZipOutputStream uninstallOutJar;
    public Info info;
    public List<Pack> allPacks;
    public List availablePacks;
    public List selectedPacks;
    public List<IzPanel> panels;
    public List panelsOrder;
    public int curPanelNumber;
    public boolean canClose = false;
    public boolean installSuccess = true;
    public XMLElement xmlData;
    public Map<String, List> customData;
    protected Properties variables;
    protected Map<String, Object> attributes;
    private static AutomatedInstallData self = null;

    public static AutomatedInstallData getInstance() {
        return (self);
    }

    public AutomatedInstallData() {
        availablePacks = new ArrayList();
        selectedPacks = new ArrayList();
        panels = new ArrayList<IzPanel>();
        panelsOrder = new ArrayList();
        xmlData = new XMLElement("AutomatedInstallation");
        variables = new Properties();
        attributes = new HashMap<String, Object>();
        customData = new HashMap<String, List>();
        self = this;
    }

    public Properties getVariables() {
        return variables;
    }

    public void setVariable(String var, String val) {
        variables.setProperty(var, val);
    }

    public String getVariable(String var) {
        return variables.getProperty(var);
    }

    public void setInstallPath(String path) {
        setVariable(ScriptParser.INSTALL_PATH, path);
    }

    public String getInstallPath() {
        return getVariable(ScriptParser.INSTALL_PATH);
    }

    public Object getAttribute(String attr) {
        return attributes.get(attr);
    }

    public void setAttribute(String attr, Object val) {
        if (val == null) {
            attributes.remove(attr);
        } else {
            attributes.put(attr, val);
        }

    }
}
