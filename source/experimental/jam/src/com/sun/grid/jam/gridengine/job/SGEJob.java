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
 *   and/or Swiss Center for Scientific Computing
 * 
 *   Copyright: 2002 by Sun Microsystems, Inc.
 *   Copyright: 2002 by Swiss Center for Scientific Computing
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jam.gridengine.job;

import com.sun.grid.jam.gridengine.util.*;

import java.util.Date;

/**
 * This is a JAM representation of a SGE job.
 *
 * @version 1.6, 09/22/00
 *
 * @author Rafal Piotrowski
 */
public class SGEJob
{
  /**
   * Job name
   * set to INTERACTIVE if interactive job 
   * JB_job_name
   */
  private String name;
  
  /** Job ID */
  private int id;
  
  /**
   * Job's state & status
   * @see com.sun.grid.jam.gridengine.job.JobStat for more details.
   */
  private JobStat stat;
  
  /** Job type (i.e. Batch or Interactive) */
  private String type;
  
  /**
   * Job script i.e. JB_script_file
   * TODO: this could be an URL or something else ?
   */
  private String scriptFile;
  
  /**
   * Job arguments list. JB_job_args
   *
   * @see com.sun.grid.jam.gridengine.util.StringList
   */
  private StringList args;

  /**
   * Environment variables list
   * `DISPLAY=currentsubmitionhost:0.0' if interactive job
   * JB_env_list
   *
   * @see com.sun.grid.jam.gridengine.util.VariableList
   */
  private VariableList envVars;

  /**
   * Hard queue list JB_hard_queue_list
   *
   * @see com.sun.grid.jam.gridengine.util.StringList
   */
  private StringList hardQueueList;

  /** Current Working Directory. JB_cwd */
  private String cwd;

  /**
   * Shell list. JB_shell_list
   *
   * @see com.sun.grid.jam.gridengine.util.HostPathList
   */
  private HostPathList shells;

  /**
   * Merge stderr flag (false | true).
   * false = No (default)
   * true = Yes
   * JB_merge_stderr
   */
  private boolean mergeStderr;

  /**
   * stdout path list
   * JB_stdout_path_list
   *
   * @see com.sun.grid.jam.gridengine.util.HostPathList
   */
  private HostPathList stdoutPaths;

  /**
   * stderr path list
   * JB_stderr_path_list
   *
   * @see com.sun.grid.jam.gridengine.util.HostPathList
   */
  private HostPathList stderrPaths;

  /** Job priority. Default 0. JB_priority */
  private long priority;
  
  /**
   * Start job at some specified time. JB_execution_time
   *
   * @see java.util.Date
   */
  private Date time;

  /**
   * Execution time
   * set to 0 if job should be scheduled ASAP (default)
   * JB_execution_time
   */
  private long executionTime;
  
  /**
   * Restart job flag (false | true)
   * false = No
   * true = Yes
   * It will depend on queue.
   * JB_restart
   */
  private boolean restart;

  /**
   * Notify job flag (false | true)
   * false = No (default)
   * true = Yes
   * JB_notify
   */
  private boolean notify;

  /**
   * Hold job flag (false | true)
   * false = No (default)
   * true = Yes
   * JAT_hold
   */
  private boolean hold;

  
  //============= constructors ================

  /**
   * Default constructor. It sets all variables to default values.
   * It is used by other constructors.
   */  
  public SGEJob()
  {
    args = new StringList();
    envVars = new VariableList();
    stdoutPaths = new HostPathList();
    stderrPaths = new HostPathList();
    stat = new JobStat();

    name = null;
    id = -1;
    scriptFile = null;
    cwd = new String("/tmp");
        
    priority = 0;
    executionTime = 0;
    hold = true;
    notify = false;
    restart = false;
    mergeStderr = true;
  }

  /**
   * Usually this constructor is used when it's important to have
   * job's name and id only. Example of such use would be, when one
   * wants to delete a job.
   *
   * @param name - job's name
   * @param id - job's id
   */
  public SGEJob(String name, int id)
  {
    this();
    this.name = name;
    this.id = id;
  }

  /**
   * 
   *
   * @param name - job's name
   * @param scriptFile - job's script file name
   * @param cwd - common working directory
   */
  public SGEJob(String name, String scriptFile, String cwd)
  {
    this();
    this.name = name;
    this.cwd = cwd;
    if(scriptFile != null)
      this.scriptFile = scriptFile;
    else 
      this.scriptFile = new StringBuffer().append(cwd).append("/").append(name).toString();
  }
  
  //============== adders ================

  /**
   * Add job's argument.
   *
   * @param arg - job's argument
   */
  public void addArg(String arg)
  {
    args.add(arg);
  }

  /**
   * Add job's envirable variable that need to be set before this job
   * can start executing.
   *
   * @param envVar - environment variable
   *
   * @see com.sun.grid.jam.gridengine.util.Variable
   */
  public void addEnvVar(Variable envVar)
  {
    envVars.add(envVar);
  }

  /**
   * Add job's standard output path that should be used as standard
   * output path.
   *
   * @param stdoutPath - standard output path
   *
   * @see com.sun.grid.jam.gridengine.util.HostPath
   */
  public void addStdoutPath(HostPath stdoutPath)
  {
    stdoutPaths.add(stdoutPath);
  }

  /**
   * Add job's standard error path that should be used as standard
   * error path.
   *
   * @param stderrPath - standard error path
   *
   * @see com.sun.grid.jam.gridengine.util.HostPath
   */
  public void addStderrPath(HostPath stderrPath)
  {
    stderrPaths.add(stderrPath);
  }

  //============== getters =============

  /**
   * Gets job's name.
   *
   * @return job's name
   */
  public String getName()
  {
    return name;
  }

  /**
   * Gets job's id.
   *
   * @return job's id
   */
  public int getID()
  {
    return id;
  }

  /**
   * Gets job's script file name.
   *
   * @return job's script file name
   */
  public String getScriptFile()
  {
    return scriptFile;
  }

  /**
   * Gets job's common working directory.
   *
   * @return job's common working directory
   */
  public String getCWD()
  {
    return cwd;
  }

  /**
   * Gets list of job's arguments.
   *
   * @return string list of job's arguments
   *
   * @see com.sun.grid.jam.gridengine.util.StringList
   */
  public StringList getArgs()
  {
    return args;
  }

  /**
   * Gets list of job's environment variables.
   *
   * @return variable list of job's environment variables
   *
   * @see com.sun.grid.jam.gridengine.util.VariableList
   */
  public VariableList getEnvVars()
  {
    return envVars;
  }

  /**
   * Gets list of job's standard output paths.
   *
   * @return host & path list of job's standard output paths
   *
   * @see com.sun.grid.jam.gridengine.util.HostPathList
   */
  public HostPathList getStdoutPaths() 
  {
    return stdoutPaths;
  }

  /**
   * Gets list of job's standard error paths.
   *
   * @return host & path list of job's standard error paths
   *
   * @see com.sun.grid.jam.gridengine.util.HostPathList
   */
  public HostPathList getStderrPaths() 
  {
    return stderrPaths;
  }

  /**
   * Gets job's stat that contains status & state.
   *
   * @return job's state @ status
   *
   * @see com.sun.grid.jam.gridengine.job.JobStat
   */
  public synchronized JobStat getStat()
  {
    return stat;
  }

  public boolean getHold()
  {
    return hold;
  }
  
  //============== setters ==============

  /**
   * Sets job's id.
   *
   * @param id - job's id
   */
  public void setID(int id)
  {
    this.id = id;
  }

  /**
   * Sets job's state & status
   *
   * @param stat - JobStat object that contains job's state & status
   *
   * @see com.sun.grid.jam.gridengine.job.JobStat
   */
  public synchronized void setStat(JobStat stat)
  {
    this.stat = stat;
  }

  public void setHold()
  {
    hold = true;
  }

  public void unSetHold()
  {
    hold = false;
  }
  
}
