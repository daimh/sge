/*
 * Settings.java
 *
 * Created on September 7, 2005, 4:01 PM
 */

package com.sun.grid;

import java.util.HashMap;
import java.util.Map;

/**
 *
 * @author  dan.templeton@sun.com
 */
public class Settings {
    private static Map settings = new HashMap();
    public static final String USER = "user";
    public static final String TEST_PATH = "test_path";
    public static final String SGE_ROOT = "sge_root";
    public static final String ARCH = "arch";
    public static final String HOSTNAME = "hostname";
    public static final String GETS_ROOT = "gets_root";
    public static final String CWD = "cwd";
    public static final String SCRIPTS_DIR = "scripts_dir";
    public static final String CHECKTREE_ROOT = "checktree_root";
    public static final String PID = "pid";
    public static final String HOME_DIR = "home_dir";
    public static final String VERSION = "version";
    
   /* The first time this class is used, it will initialize all of the values
    * automatically. */
    static {
        try {
            settings.put(USER, System.getProperty("gets.user", "/"));
            settings.put(TEST_PATH, System.getProperty("gets.test_path", "/"));
            settings.put(SGE_ROOT, System.getProperty("gets.sge_root", "/"));
            settings.put(ARCH, System.getProperty("gets.arch", ""));
            settings.put(HOSTNAME, System.getProperty("gets.hostname", "unknown"));
            settings.put(GETS_ROOT, System.getProperty("gets.gets_root", "/"));
            settings.put(CWD, System.getProperty("gets.cwd", "/"));
            settings.put(SCRIPTS_DIR, System.getProperty("gets.scripts_dir", "/"));
            settings.put(CHECKTREE_ROOT, System.getProperty("gets.checktree_root", "/"));
            settings.put(PID, System.getProperty("gets.pid", "-1"));
            settings.put(HOME_DIR, System.getProperty("gets.home_dir", "/"));
            settings.put(VERSION, System.getProperty("gets.version", "maintrunk"));
        } catch (SecurityException e) {
            System.err.println("Not allowed to access system properties");
            System.exit(1);
        }
    }
    
    public static String get(String setting) {
        return (String)settings.get(setting);
    }
    
    public static void put(String setting, String value) {
        settings.put(setting, value);
    }
    
    public String toString() {
        return settings.toString();
    }
}
