/*
 * IzPack - Copyright 2001-2008 Julien Ponge, All Rights Reserved.
 *
 * http://izpack.org/
 * http://izpack.codehaus.org/
 *
 * Copyright 2003 Jonathan Halliday
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

import com.izforge.izpack.util.Debug;
import com.izforge.izpack.util.StringTool;

import java.util.Date;
import java.util.Properties;

/**
 * The program entry point. Selects between GUI and text install modes.
 *
 * @author Jonathan Halliday
 */
public class Installer
{
    private static String ARG_KEY_AUTO_CONF_FILE = "auto";
    private static String ARG_PREFIX = "-";
    private static String ARG_SEPARATOR = "=";

    /**
     * The main method (program entry point).
     *
     * @param args The arguments passed on the command-line.
     */
    public static void main(String[] args)
    {
        Debug.log(" - Logger initialized at '" + new Date(System.currentTimeMillis()) + "'.");

        Debug.log(" - commandline args: " + StringTool.stringArrayToSpaceSeparatedString(args));

        // OS X tweakings
        if (System.getProperty("mrj.version") != null)
        {
            System.setProperty("com.apple.mrj.application.apple.menu.about.name", "IzPack");
            System.setProperty("com.apple.mrj.application.growbox.intrudes", "false");
            System.setProperty("com.apple.mrj.application.live-resize", "true");
        }
        
        try
        {
            String inputFileName = "";
            Properties argValues = new Properties();
            for (int i = 0; i < args.length; i++) {
                Debug.trace("Found argument: " + args[i]);
                String[] keyValue = getKeyValuePair(args[i]);
                if (keyValue[0].equals(ARG_KEY_AUTO_CONF_FILE)) {
                    inputFileName = keyValue[1].trim();
                } else {
                    argValues.put(keyValue[0].trim(), keyValue[1].trim());

                }
            }

            if (inputFileName.equals(""))
            {
                // can't load the GUIInstaller class on headless machines,
                // so we use Class.forName to force lazy loading.
                GUIInstaller guiInstaller = (GUIInstaller)Class.forName("com.izforge.izpack.installer.GUIInstaller").newInstance();

                try {
                    // Check whether it's a demo version
                    Class.forName("com.sun.grid.installer.util.cmd.TestBedManager");

                    Thread.sleep(2000);

                    String appTitle = GUIInstaller.getInstallerFrame().getTitle();
                    appTitle += " - DEMO version";
                    GUIInstaller.getInstallerFrame().setTitle(appTitle);
                } catch (ClassNotFoundException ex) {
                    // Normal version
                } catch (Exception e) {
                    // can not set the title
                }

                guiInstaller.addExtraVariables(argValues);
            }
            else
            {
                AutomatedInstaller ai = new AutomatedInstaller(inputFileName);
                ai.addExtraVariables(argValues);
                // this method will also exit!
                ai.doInstall();
            }
        }
        catch (Exception e)
        {
            System.err.println("- ERROR -");
            System.err.println(e.toString());
            e.printStackTrace();
            System.exit(1);
        }
    }

    private static String[] getKeyValuePair(String arg) {
        if (arg.startsWith(ARG_PREFIX)) {
            arg = arg.substring(ARG_PREFIX.length());
        }

        if (arg.indexOf(ARG_SEPARATOR) == -1) {
            return new String[]{arg, ""};
        } else {
            return arg.split(ARG_SEPARATOR);
        }
    }
}
