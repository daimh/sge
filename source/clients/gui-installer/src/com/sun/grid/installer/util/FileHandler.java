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
package com.sun.grid.installer.util;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Vector;

import com.izforge.izpack.util.Debug;
import com.sun.grid.installer.util.cmd.SimpleLocalCommand;

/**
 * General file handling methods
 */
public class FileHandler {
    public static int POS_PERMISSION_FIELD = 0;
    public static int POS_NUMOFLINKS_FIELD = 1;
    public static int POS_OWNER_FIELD      = 2;
    public static int POS_GROUP_FIELD      = 3;
    
    public static String SEPARATOR = File.separator;

    /**
     * Generates file from content
     * @param lines The content to generate file for
     * @param filePath The result file path
     * 
     * @throws java.io.IOException
     */
    public static void generateFile(ArrayList<String> lines, String filePath) throws IOException {
    	FileWriter fileWriter = null;
    	BufferedWriter bufferedWriter = null;
    	
    	try {
    		fileWriter = new FileWriter(filePath, false);
    		bufferedWriter = new BufferedWriter(fileWriter);

    		for (int idx = 0; idx < lines.size(); idx++) {
    			String elem = lines.get(idx);
    			bufferedWriter.write(elem);
    			bufferedWriter.newLine();
    		}
    	} finally {
    		if (bufferedWriter != null) {
    			bufferedWriter.close();
    		}
    	}
    }

    /**
     * Reads file content
     * @param filePath The path to the file to be read
     * @param keepLineFeed indicates whether the line feed ('\n') should be kept in the result.
     * @return Array containing the lines have been red from the file.
     * 
     * @throws java.io.FileNotFoundException
     * @throws java.io.IOException
     */
    public static ArrayList<String> readFileContent(String filePath, boolean keepLineFeed) throws FileNotFoundException, IOException {
        ArrayList<String> result = new ArrayList<String>();
        BufferedReader bufferedReader = null;

    	try {
    		bufferedReader = new BufferedReader(new FileReader(filePath));

    		String line = null;
    		while ((line = bufferedReader.readLine()) != null) {
                if (keepLineFeed) {
                    line += "\n";
                }
    			result.add(line);
    		}
    	} finally {
    		if (bufferedReader != null) {
    			bufferedReader.close();
    		}
    	}

        return result;
    }

    /**
     * Returns with the file properies supplied by the 'ls -la' command
     * @param filePath Path to the file whose properies should return
     * @return The properties of the file. Order:
     * POS_PERMISSION_FIELD, POS_NUMOFLINKS_FIELD, POS_OWNER_FIELD, POS_GROUP_FIELD
     */
    protected static String[] getFileProps(String filePath)  {
        String[] props = null;

        // check file
        if (filePath == null) {
            return null;
        }

        File file = new File(filePath);
        if (!file.exists()) {
            return null;
        }

        // call 'ls -la' command...
        SimpleLocalCommand cmd = new SimpleLocalCommand("ls -la " + filePath);
        cmd.execute();

        // in case of failure try /usr/bin/ls -la...
        if (cmd.getExitValue() != Config.EXIT_VAL_SUCCESS) {
            cmd = new SimpleLocalCommand("/usr/bin/ls -la " + filePath);
            cmd.execute();
        }

        if (cmd.getExitValue() != Config.EXIT_VAL_SUCCESS) {
            Debug.trace("Failed to execute 'ls -la " + filePath + "'. Out:"
                    + cmd.getOutput().toString() + " Err:" + cmd.getError().toString());
            return null;
        }

        // process output
        Vector<String> result = cmd.getOutput();
        String line = "";
        if (file.isFile() && result.size() != 0) {
            line = result.firstElement();
        } else if (result.size() >= 2) {
            line = result.get(1);
        }

        // cleanup output
        props = line.split(" ");
        props = cleanUpFileProps(props);

        if (props.length < 4) {
            Debug.trace("Unexpected empty output from ls -la " + filePath);
            return null;
        }

        return props;
    }

    /**
     * Normalizes the output of the 'ls -la' command
     * @param pureFileProps The output of 'ls -la' command
     * @return The normalized output
     */
    private static String[] cleanUpFileProps(String[] pureFileProps) {
        Vector<String> cleanedFileProps = new Vector<String>(pureFileProps.length);

        // remove empty values
        for (int i = 0; i < pureFileProps.length; i++) {
            if (!pureFileProps[i].equals("")) {
                cleanedFileProps.add(pureFileProps[i]);
            }
        }

        // TODO collect and put into one field the modification date value

        cleanedFileProps.trimToSize();
        String[] result = new String[cleanedFileProps.size()];
        return cleanedFileProps.toArray(result);
    }
}
