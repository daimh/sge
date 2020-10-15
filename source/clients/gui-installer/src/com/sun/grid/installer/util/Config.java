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

/**
 * Config
 */
public interface Config {
    public static final String CONFIG_VAR_PREFIX    = "cfg";
    public static final String VAR_CONF_FILE_DIR    = "templates/";
    public static final String VAR_CONF_FILE_NAME   = "inst_template.conf";

    public static final String CONST_DEFAULT_WINDOWS_SPOOL_DIR = "/var/spool/";
    public static final String CONST_MODE_WINDOWS = "windows";

    public static final String VAR_INSTALL_QMASTER = "install.qmaster";
    public static final String VAR_INSTALL_EXECD = "install.execd";
    public static final String VAR_INSTALL_SHADOW = "install.shadowd";
    public static final String VAR_INSTALL_BDB = "install.bdb";
    public static final String VAR_INSTALL_MODE = "install.mode";
    public static final String VAR_INSTALL_MODE_EXPRESS = "install.mode.express";
    public static final String VAR_INSTALL_MODE_CUSTOM = "install.mode.custom";

    public static final String VAR_SGE_ROOT    = "cfg.sge.root";
    public static final String VAR_SGE_CELL_NAME = "cfg.cell.name";
    public static final String VAR_ADMIN_USER  = "cfg.admin.user";
    public static final String VAR_SGE_QMASTER_PORT  = "cfg.sge.qmaster.port";
    public static final String VAR_SGE_EXECD_PORT  = "cfg.sge.execd.port";
    public static final String VAR_EXEC_HOST_LIST = "cfg.exec.host.list";
    public static final String VAR_EXEC_HOST_LIST_RM = "cfg.exec.host.list.rm";
    public static final String VAR_ADMIN_HOST_LIST = "cfg.admin.host.list";
    public static final String VAR_SUBMIT_HOST_LIST = "cfg.submit.host.list";
    public static final String VAR_SHADOW_HOST_LIST = "cfg.shadow.host";
    public static final String VAR_QMASTER_SPOOL_DIR = "cfg.qmaster.spool.dir";
    public static final String VAR_EXECD_SPOOL_DIR = "cfg.execd.spool.dir";
    public static final String VAR_EXECD_SPOOL_DIR_LOCAL = "cfg.exec.spool.dir.local";
    public static final String VAR_SHELL_NAME = "cfg.shell.name";
    public static final String VAR_COPY_COMMAND = "cfg.copy.command";
    public static final String VAR_DB_SPOOLING_SERVER = "cfg.db.spooling.server";
    public static final String VAR_SPOOLING_METHOD = "cfg.spooling.method";
    public static final String VAR_WINDOWS_SUPPORT = "cfg.windows.support";
    public static final String VAR_WIN_DOMAIN_ACCESS = "cfg.win.domain.access";
    public static final String VAR_WIN_ADMIN_NAME = "cfg.win.admin.name";
    public static final String VAR_HOSTNAME_RESOLVING = "cfg.hostname.resolving";
    public static final String VAR_JVM_LIB_PATH = "cfg.sge.jvm.lib.path";
    public static final String VAR_ADDITIONAL_JVM_ARGS = "cfg.sge.additional.jvm.args";
    public static final String VAR_DB_SPOOLING_DIR = "cfg.db.spooling.dir";
    public static final String VAR_SGE_JMX_PORT = "cfg.sge.jmx.port";
    public static final String VAR_ADD_TO_RC = "cfg.add.to.rc";
    public static final String VAR_SGE_ENABLE_SMF= "cfg.sge.enable.smf";
    public static final String VAR_REMOVE_RC= "cfg.remove.rc";
    public static final String VAR_ALL_HOSTS = "gui_allhosts";
    public static final String VAR_ALL_ADMIN_HOSTS = "gui_alladminhosts";
    public static final String VAR_ALL_SUBMIT_HOSTS = "gui_allsubmithosts";
    public static final String VAR_ALL_CSPHOSTS = "gui_csphosts";
    public static final String VAR_ALL_COPYUSERS = "gui_csphosts_connectusers";
    public static final String VAR_QMASTER_CONNECT_USER = "gui_qmaster_connect_user";
    public static final String VAR_GUI_SHELL_OPTIONS = "gui_shell_options";
    public static final String VAR_FIRST_TASK = "first_install_task";
    public static final String VAR_LAST_TASK = "last_install_task";
    public static final String VAR_GID_RANGE = "cfg.gid.range";
    public static final String VAR_ADMIN_MAIL = "cfg.admin.mail";
    public static final String VAR_SGE_CLUSTER_NAME = "cfg.sge.cluster.name";

    public static final String VAR_AUTO_CONF_TEMP_FILE = "auto.conf.temp.file";
    public static final String VAR_SILENT_INSTALL_FILE = "silent.install.file";
    public static final String VAR_AUTO_CONF_FILE = "auto.conf.file";
    public static final String VAR_INSTALL_SCRIPT_FILE_NAME = "install.script.file.name";
    public static final String VAR_INSTALL_SCRIPT_FILE_DIR  = "install.script.file.dir";
    public static final String VAR_AUTO_INSTALL_COMPONENT_TEMP_FILE = "auto.install.component.temp.file";
    public static final String VAR_AUTO_INSTALL_COMPONENT_FILE  = "auto.install.component.file";
    public static final String VAR_CHECK_HOST_TEMP_FILE  = "check.host.temp.file";
    public static final String VAR_CHECK_HOST_FILE  = "check.host.file";
    public static final String VAR_WORK_DIR  = "work.dir";
    public static final String VAR_TEMPLATES_DIR  = "templates.dir";
    public static final String VAR_README_TEMP_FILE  = "readme.temp.file";
    public static final String VAR_README_FILE_NAME_1  = "readme.file.name.1";
    public static final String VAR_README_FILE_NAME_2  = "readme.file.name.2";
    public static final String VAR_PROGRESS_TYPE = "mode";
    public static final String VAR_USER_NAME = "user.name";
    public static final String VAR_ROOT_USER = "root.user";
    public static final String VAR_CONNECT_USER = "connect_user";
    //public static final String VAR_USER_GROUP = "user.group";
    public static final String VAR_RESULT_INFO = "result.info";
    public static final String VAR_LOCALHOST_ARCH = "localhost.arch";

    public static final String VAR_QMASTER_HOST = "add.qmaster.host";
    public static final String VAR_QMASTER_HOST_ARCH = "add.qmaster.host.arch";
    public static final String VAR_SPOOLING_METHOD_BERKELEYDBSERVER = "add.spooling.method.berkeleydbserver";
    public static final String VAR_SPOOLING_METHOD_BERKELEYDB = "add.spooling.method.berkeleydb";
    public static final String VAR_SGE_JMX = "add.sge.jmx";
    public static final String VAR_DB_SPOOLING_DIR_BDB = "add.db.spooling.dir.bdb";
    public static final String VAR_DB_SPOOLING_DIR_BDB_DEF = "add.db.spooling.dir.bdb.def";
    public static final String VAR_JMX_SSL = "cfg.sge.jmx.ssl";
    public static final String VAR_JMX_SSL_CLIENT = "cfg.sge.jmx.ssl.client";
    public static final String VAR_JMX_SSL_KEYSTORE = "cfg.sge.jmx.ssl.keystore";
    public static final String VAR_JMX_SSL_KEYSTORE_DEF = "add.sge.jmx.ssl.keystore.def";
    public static final String VAR_JMX_SSL_KEYSTORE_PWD = "cfg.sge.jmx.ssl.keystore.pw";
    public static final String VAR_PRODUCT_MODE = "add.product.mode";

    public static final String VAR_QMASTER_HOST_FAILED = "add.qmaster.host.failed";
    public static final String VAR_EXEC_HOST_LIST_FAILED = "add.exec.host.list.failed";
    public static final String VAR_SHADOW_HOST_LIST_FAILED = "add.shadow.host.failed";
    public static final String VAR_DB_SPOOLING_SERVER_FAILED = "add.db.spooling.server.failed";
    public static final String VAR_ADMIN_HOST_LIST_FAILED = "add.admin.host.list.failed";
    public static final String VAR_SUBMIT_HOST_LIST_FAILED = "add.submit.host.list.failed";
    
    public static final String PARAMETER_1 = "param1";
    public static final String PARAMETER_2 = "param2";

    public static final String COND_INSTALL_QMASTER = "cond.install.qmaster";
    public static final String COND_INSTALL_EXECD = "cond.install.execd";
    public static final String COND_INSTALL_SHADOWD = "cond.install.shadowd";
    public static final String COND_INSTALL_BDB = "cond.install.bdb";
    public static final String COND_EXPRESS_INSTALL = "cond.express.install";
    public static final String COND_SPOOLING_CLASSIC = "cond.spooling.classic";
    public static final String COND_SPOOLING_BDB = "cond.spooling.bdb";
    public static final String COND_SPOOLING_BDBSERVER = "cond.spooling.bdbserver";
    public static final String COND_JMX = "cond.sge.jmx";
    public static final String COND_JMX_SSL = "cond.sge.jmx.ssl";
    public static final String COND_USER_ROOT = "cond.user.root";
    public static final String COND_NO_CONNECT_USER = "cond.no.connect.user";
    
    public static final String LANGID_HELP_ERROR_PAGE = "installer.help.errorpage";
    public static final String LANGID_HELP_EMPTY_PAGE = "installer.help.emptypage";
    public static final String LANGID_PREFIX_STATE = "state";
    
    public static final String TOOLTIP = "tooltip";

    public static final String WELCOME_IMAGE_RESOURCE = "Welcome.image";
    public static final String WELCOME_TEXT_RESOURCE = "WelcomePanel.welcome";

    public static final String WARNING_USER_NOT_ROOT = "warning.user.not.root.message";
    public static final String ERROR_USER_INVALID = "error.user.invalid.message";

    public static final String ARG_RESOLVE_THREAD_POOL_SIZE = "resolve_pool";
    public static final String ARG_INSTALL_THREAD_POOL_SIZE = "install_pool";

    public static final String ARG_RESOLVE_TIMEOUT = "resolve_timeout";
    public static final String ARG_INSTALL_TIMEOUT = "install_timeout";
    
    public static final String ARG_CONNECT_USER = "connect_user";
    public static final String ARG_CONNECT_MODE = "connect_mode";

    // Overall exit value
    public static final int EXIT_VAL_SUCCESS = 0;

    // Check_host exit values
    public static final int EXIT_VAL_QMASTER_SPOOL_DIR_PERM_DENIED = 20;
    public static final int EXIT_VAL_EXECD_SPOOL_DIR_PERM_DENIED = 21;
    public static final int EXIT_VAL_JMX_KEYSTORE_PERM_DENIED = 22;
    public static final int EXIT_VAL_JVM_LIB_DOES_NOT_EXIST_QMASTER = 23;
    public static final int EXIT_VAL_JVM_LIB_INVALID_QMASTER = 24;
    public static final int EXIT_VAL_BDB_SPOOL_DIR_EXISTS = 25;
    public static final int EXIT_VAL_BDB_SPOOL_WRONG_FSTYPE = 26;
    public static final int EXIT_VAL_BDB_SPOOL_DIR_PERM_DENIED = 27;
    public static final int EXIT_VAL_JVM_LIB_DOES_NOT_EXIST_SHADOWD = 30;
    public static final int EXIT_VAL_JVM_LIB_INVALID_SHADOWD = 31;
    public static final int EXIT_VAL_EXECD_SPOOL_DIR_LOCAL_PERM_DENIED = 40;
    public static final int EXIT_VAL_BDB_SERVER_SPOOL_DIR_EXISTS = 50;
    public static final int EXIT_VAL_BDB_SERVER_SPOOL_DIR_PERM_DENIED = 51;
    public static final int EXIT_VAL_ADMIN_USER_NOT_KNOWN = 60;

    // Get architecture task exit values
    public static final int EXIT_VAL_UNKNOWN_HOST = 70;

    // Install task exit values
    public static final int EXIT_VAL_FAILED_ALREADY_INSTALLED_COMPONENT = 10;

    // Command executor exit values
    public static final int EXIT_VAL_CMDEXEC_INITIAL       = -1;
    public static final int EXIT_VAL_CMDEXEC_OTHER         = -2;
    public static final int EXIT_VAL_CMDEXEC_INTERRUPTED   = -3;
    public static final int EXIT_VAL_CMDEXEC_TERMINATED    = -4;
    public static final int EXIT_VAL_CMDEXEC_MISSING_FILE  = 15;
}
