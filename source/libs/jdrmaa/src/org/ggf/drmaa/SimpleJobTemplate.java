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
package org.ggf.drmaa;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.Set;

/**
 * This class is a trivial implementation of the JobTemplate interface.  This
 * class can either be used as the base class for a implementation-specific
 * job template class, or for implementations which do not need more than the
 * required properties, it can be used as the job template implementation
 * directly.
 *
 * <p>All non-primitive properties default to <code>null</code>.  All boolean
 * properties default to <code>false</code>.  All other primitive properties
 * default to zero.</p>
 *
 * <p>The SimpleJobTemplate class is not thread safe.  No attempt is made to
 * prevent setters and getters from interfering with each other or the
 * toString() method.</p>
 *
 * @author dan.templeton@sun.com
 * @see JobTemplate
 * @since 1.0
 */
public class SimpleJobTemplate implements JobTemplate, Serializable {
    private String toString = null;
    private Set allPropertyNames = null;
    private boolean modified = true;
    /**
     * Remote command to execute
     * @see JobTemplate#setRemoteCommand(String)
     */
    protected String remoteCommand = null;
    /**
     * Input parameters passed as arguments to the job
     * @see JobTemplate#setArgs(List)
     */
    protected List args = null;
    /**
     * Job state at submission, either HOLD or ACTIVE
     * @see JobTemplate#setJobSubmissionState(int)
     */
    protected int jobSubmissionState = ACTIVE_STATE;
    /**
     * The environment values that define the job's remote environment
     * @see JobTemplate#setJobEnvironment(Map)
     */
    protected Map jobEnvironment = null;
    /**
     * The directory where the job is executed.
     * @see JobTemplate#setWorkingDirectory(String)
     */
    protected String workingDirectory = null;
    /**
     * An implementation-defined string specifying how to resolve site-specific
     * resources and/or policies
     * @see JobTemplate#setJobCategory(String)
     */
    protected String jobCategory = null;
    /**
     * An implementation-defined string that is passed by the end user to DRMAA
     * to specify site-specific resources and/or policies
     * @see JobTemplate#setNativeSpecification(String)
     */
    protected String nativeSpecification = null;
    /**
     * E-mail addresses used to report the job completion and status
     * @see JobTemplate#setEmail(Set)
     */
    protected Set email = null;
    /**
     * Blocks sending e-mail by default, regardless of the DRMS setting
     * @see JobTemplate#setBlockEmail(boolean)
     */
    protected boolean blockEmail = false;
    /**
     * The earliest time when the job may be eligible to be run
     * @see JobTemplate#setStartTime(PartialTimestamp)
     */
    protected PartialTimestamp startTime = null;
    /**
     * Job name
     * @see JobTemplate#setJobName(String)
     */
    protected String jobName = null;
    /**
     * The job's standard input stream
     * @see JobTemplate#setInputPath(String)
     */
    protected String inputPath = null;
    /**
     * The job's standard output stream
     * @see JobTemplate#setOutputPath(String)
     */
    protected String outputPath = null;
    /**
     * The job's standard error stream
     * @see JobTemplate#setErrorPath(String)
     */
    protected String errorPath = null;
    /**
     * Whether the error stream should be intermixed with the output stream
     * @see JobTemplate#setJoinFiles(boolean)
     */
    protected boolean joinFiles = false;

    /**
     * Create a new instance of a JobTemplate.
     */
    public SimpleJobTemplate() {
    }

    public void setRemoteCommand(String remoteCommand) throws DrmaaException {
        this.remoteCommand = remoteCommand;
        modified = true;
    }

    public String getRemoteCommand() throws DrmaaException {
        return remoteCommand;
    }

    public void setArgs(List args) throws DrmaaException {
        if (args != null) {
            this.args = new ArrayList(args);
        } else {
            this.args = null;
        }

        modified = true;
    }

    public List getArgs() throws DrmaaException {
        List returnValue = null;

        if (args != null) {
            returnValue = Collections.unmodifiableList(args);
        }

        return returnValue;
    }

    public void setJobSubmissionState(int state) throws DrmaaException {
        if ((state != ACTIVE_STATE) && (state != HOLD_STATE)) {
            throw new IllegalArgumentException("Invalid state");
        }

        this.jobSubmissionState = state;
        modified = true;
    }

    public int getJobSubmissionState() throws DrmaaException {
        return jobSubmissionState;
    }

    public void setJobEnvironment(Map env) throws DrmaaException {
        if (env != null) {
            this.jobEnvironment = new HashMap(env);
        } else {
            this.jobEnvironment = null;
        }

        modified = true;
    }

    public Map getJobEnvironment() throws DrmaaException {
        Map returnValue = null;

        if (jobEnvironment != null) {
            returnValue = Collections.unmodifiableMap(jobEnvironment);
        }

        return returnValue;
    }

    public void setWorkingDirectory(String wd) throws DrmaaException {
        if (wd.indexOf(HOME_DIRECTORY) > 0) {
            throw new InvalidAttributeFormatException(HOME_DIRECTORY +
                    " may only appear at the beginning of the path.");
        } else if (wd.indexOf(WORKING_DIRECTORY) >= 0) {
            throw new InvalidAttributeFormatException(WORKING_DIRECTORY +
                    " may not be used in the workingDirectory path.");
        }

        this.workingDirectory = wd;
        modified = true;
    }

    public String getWorkingDirectory() throws DrmaaException {
        return workingDirectory;
    }

    public void setJobCategory(String category) throws DrmaaException {
        this.jobCategory = category;
        modified = true;
    }

    public String getJobCategory() throws DrmaaException {
        return jobCategory;
    }

    public void setNativeSpecification(String spec) throws DrmaaException {
        this.nativeSpecification = spec;
        modified = true;
    }

    public String getNativeSpecification() throws DrmaaException {
        return nativeSpecification;
    }

    public void setEmail(Set email) throws DrmaaException {
        if (email != null) {
            this.email = new HashSet(email);
        } else {
            this.email = null;
        }

        modified = true;
    }

    public Set getEmail() throws DrmaaException {
        Set returnValue = null;

        if (email != null) {
            returnValue = Collections.unmodifiableSet(email);
        }

        return returnValue;
    }

    public void setBlockEmail(boolean blockEmail) throws DrmaaException {
        this.blockEmail = blockEmail;
        modified = true;
    }

    public boolean getBlockEmail() throws DrmaaException {
        return blockEmail;
    }

    public void setStartTime(PartialTimestamp startTime) throws DrmaaException {
        if (startTime != null) {
            if (startTime.getTimeInMillis() < System.currentTimeMillis()) {
                throw new IllegalArgumentException("Start time is in the past.");
            }

            this.startTime = startTime;
        } else {
            startTime = null;
        }

        modified = true;
    }

    public PartialTimestamp getStartTime() throws DrmaaException {
        if (startTime != null) {
            return (PartialTimestamp)startTime.clone();
        } else {
            return null;
        }
    }

    public void setJobName(String name) throws DrmaaException {
        this.jobName = name;
        modified = true;
    }

    public String getJobName() throws DrmaaException {
        return jobName;
    }

    public void setInputPath(String inputPath) throws DrmaaException {
        this.checkPath(inputPath);
        this.inputPath = inputPath;
        modified = true;
    }

    public String getInputPath() throws DrmaaException {
        return inputPath;
    }

    public void setOutputPath(String outputPath) throws DrmaaException {
        this.checkPath(outputPath);
        this.outputPath = outputPath;
        modified = true;
    }

    public String getOutputPath() throws DrmaaException {
        return outputPath;
    }

    public void setErrorPath(String errorPath) throws DrmaaException {
        this.checkPath(errorPath);
        this.errorPath = errorPath;
        modified = true;
    }

    public String getErrorPath() throws DrmaaException {
        return errorPath;
    }

    public void setJoinFiles(boolean join) throws DrmaaException {
        this.joinFiles = join;
        modified = true;
    }

    public boolean getJoinFiles() throws DrmaaException {
        return joinFiles;
    }

    public void setTransferFiles(FileTransferMode mode) throws DrmaaException {
        throw new UnsupportedAttributeException("The transferFiles attribute " +
                                                "is not supported.");
    }

    public FileTransferMode getTransferFiles() throws DrmaaException {
        throw new UnsupportedAttributeException("The transferFiles attribute " +
                                                "is not supported.");
    }

    public void setDeadlineTime(PartialTimestamp deadline)
            throws DrmaaException {
        throw new UnsupportedAttributeException("The deadlineTime attribute " +
                                                "is not supported.");
    }

    public PartialTimestamp getDeadlineTime() throws DrmaaException {
        throw new UnsupportedAttributeException("The deadlineTime attribute " +
                                                "is not supported.");
    }

    public void setHardWallclockTimeLimit(long hardWallclockLimit)
             throws DrmaaException {
        throw new UnsupportedAttributeException("The hardWallclockTimeLimit " +
                                                "attribute is not supported.");
    }

    public long getHardWallclockTimeLimit() throws DrmaaException {
        throw new UnsupportedAttributeException("The hardWallclockTimeLimit " +
                                                "attribute is not supported.");
    }

    public void setSoftWallclockTimeLimit(long softWallclockLimit)
             throws DrmaaException {
        throw new UnsupportedAttributeException("The softWallclockTimeLimit " +
                                                "attribute is not supported.");
    }

    public long getSoftWallclockTimeLimit() throws DrmaaException {
        throw new UnsupportedAttributeException("The softWallclockTimeLimit " +
                                                "attribute is not supported.");
    }

    public void setHardRunDurationLimit(long hardRunLimit)
            throws DrmaaException {
        throw new UnsupportedAttributeException("The hardRunDurationLimit " +
                                                "attribute is not supported.");
    }

    public long getHardRunDurationLimit() throws DrmaaException {
        throw new UnsupportedAttributeException("The hardRunDurationLimit " +
                                                "attribute is not supported.");
    }

    public void setSoftRunDurationLimit(long softRunLimit)
            throws DrmaaException {
        throw new UnsupportedAttributeException("The softRunDurationLimit " +
                                                "attribute is not supported.");
    }

    public long getSoftRunDurationLimit() throws DrmaaException {
        throw new UnsupportedAttributeException("The softRunDurationLimit " +
                                                "attribute is not supported.");
    }

    public Set getAttributeNames() throws DrmaaException {
        if (allPropertyNames == null) {
            allPropertyNames = new HashSet();
            addRequiredNames(allPropertyNames);
            allPropertyNames.addAll(getOptionalAttributeNames());
        }

        return allPropertyNames;
    }

    /** This method returns the names of all optional and implementation-specific
     * properties supported by this DRMAA implementation.  Unless overridden by the
     * DRMAA implementation, this method returns an empty list.
     * This method is used by the getAttributeNames() method to construct the full list
     * of implementation-supported property names.
     * @return The names of all optional and implementation-specific
     * properties supported by this DRMAA implementation
     * @see #getAttributeNames
     */
    protected Set getOptionalAttributeNames() {
        return Collections.EMPTY_SET;
    }

    private static final void addRequiredNames(Set names) {
        names.add("args");
        names.add("blockEmail");
        names.add("email");
        names.add("errorPath");
        names.add("inputPath");
        names.add("jobCategory");
        names.add("jobEnvironment");
        names.add("jobName");
        names.add("jobSubmissionState");
        names.add("joinFiles");
        names.add("nativeSpecification");
        names.add("outputPath");
        names.add("remoteCommand");
        names.add("startTime");
        names.add("workingDirectory");
    }

    /**
     * Checks for a valid path.  Throws an InvalidArgumentException is the path
     * is not valid.
     * @param path The path to validate
     */
    private void checkPath(String path) throws IllegalArgumentException {
        /* On a null path, we just return because null paths are OK. */
        if (path == null) {
            return;
        }

        if (path.indexOf(HOME_DIRECTORY) > 0) {
            throw new IllegalArgumentException(HOME_DIRECTORY +
                    " may only appear at the beginning of the path.");
        }

        if (path.indexOf(WORKING_DIRECTORY) > 0) {
            throw new IllegalArgumentException(WORKING_DIRECTORY +
                    " may only appear at the beginning of the path.");
        }
    }

    /**
     * Calling this method indicates that the job template's properties have
     * been* modified since the last call to toString().  All setters should
     * call this method before returning.
     */
    protected void modified() {
        modified = true;
    }

    /**
     * Converts this JobTemplate into a String which contains all property
     * settings.  The generated string is then cached and reused until one of
     * the property settings is modified.
     * @return a string containing all property settings
     */
    public String toString() {
        if (modified) {
            boolean error = false;
            boolean firstProperty = true;
            StringBuffer out = new StringBuffer();

            List args = null;

            try {
                args = getArgs();
            } catch (DrmaaException e) {
                out.append("{args = <ERROR>}");
                firstProperty = false;
                error = true;
            }

            if ((args != null) && (args.size() > 0)) {
                Iterator i = args.iterator();
                boolean firstArg = true;

                out.append("{args = ");

                while (i.hasNext()) {
                    if (firstArg) {
                        firstArg = false;
                    } else {
                        out.append(", ");
                    }

                    out.append("\"");
                    out.append(i.next());
                    out.append("\"");
                }

                out.append("}");
                firstProperty = false;
            }

            if (!firstProperty) {
                out.append(" ");
            }

            out.append("{blockEmail = ");

            try {
                out.append(Boolean.toString(getBlockEmail()));
            } catch (DrmaaException e) {
                out.append("<ERROR>");
                error = true;
            }
            out.append("}");

            try {
                if (getDeadlineTime() != null) {
                    PartialTimestampFormat ptf = new PartialTimestampFormat();

                    out.append(" {deadlineTime = ");
                    out.append(ptf.format(getDeadlineTime()));
                    out.append("}");
                }
            } catch (UnsupportedAttributeException e) {
                /* Skip it. */
            } catch (DrmaaException e) {
                out.append(" {deadlineTime = <ERROR>}");
                error = true;
            }

            Set email = null;

            try {
                email = getEmail();
            } catch (DrmaaException e) {
                out.append(" {email = <ERROR>}");
                error = true;
            }

            if ((email != null) && (email.size() > 0)) {
                Iterator i = email.iterator();
                boolean firstEmail = true;

                out.append(" {email = ");

                while (i.hasNext()) {
                    if (firstEmail) {
                        firstEmail = false;
                    } else {
                        out.append(", ");
                    }

                    out.append("\"");
                    out.append(i.next());
                    out.append("\"");
                }

                out.append("}");
            }

            try {
                if (getErrorPath() != null) {
                    out.append(" {errorPath = ");
                    out.append(getErrorPath());
                    out.append("}");
                }
            } catch (DrmaaException e) {
                out.append(" {errorPath = <ERROR>}");
                error = true;
            }

            try {
                if (getHardRunDurationLimit() != 0L) {
                    out.append(" {hardRunDurationLimit = ");
                    out.append(getHardRunDurationLimit());
                    out.append("ms}");
                }
            } catch (UnsupportedAttributeException e) {
                /* Skip it. */
            } catch (DrmaaException e) {
                out.append(" {hardRunDurationLimit = <ERROR>}");
                error = true;
            }

            try {
                if (getHardWallclockTimeLimit() != 0L) {
                    out.append(" {hardWallclockTimeLimit = ");
                    out.append(getHardWallclockTimeLimit());
                    out.append("ms}");
                }
            } catch (UnsupportedAttributeException e) {
                /* Skip it. */
            } catch (DrmaaException e) {
                out.append(" {hardWallclockTimeLimit = <ERROR>}");
                error = true;
            }

            try {
                if (getInputPath() != null) {
                    out.append(" {inputPath = ");
                    out.append(getInputPath());
                    out.append("}");
                }
            } catch (DrmaaException e) {
                out.append(" {inputPath = <ERROR>}");
                error = true;
            }

            try {
                if (getJobCategory() != null) {
                    out.append(" {jobCategory = ");
                    out.append(getJobCategory());
                    out.append("}");
                }
            } catch (DrmaaException e) {
                out.append(" {jobCategory = <ERROR>}");
                error = true;
            }

            Map env = null;

            try {
                env = getJobEnvironment();
            } catch (DrmaaException e) {
                out.append(" {jobEnvironment = <ERROR>}");
                error = true;
            }

            if ((env != null) && (env.size() > 0)) {
                Iterator i = env.keySet().iterator();
                boolean firstEnv = true;

                out.append(" {jobEnvironment = ");

                while (i.hasNext()) {
                    String entry = (String)i.next();
                    if (firstEnv) {
                        firstEnv = false;
                    } else {
                        out.append(", ");
                    }

                    out.append("[\"");
                    out.append(entry);
                    out.append("\" = \"");
                    out.append(env.get(entry));
                    out.append("\"]");
                }

                out.append("}");
            }

            try {
                if (getJobName() != null) {
                    out.append(" {jobName = ");
                    out.append(getJobName());
                    out.append("}");
                }
            } catch (DrmaaException e) {
                out.append(" {jobName = <ERROR>}");
                error = true;
            }

            out.append(" {jobSubmissionState = ");

            try {
                if (getJobSubmissionState() == HOLD_STATE) {
                    out.append("HOLD_STATE}");
                } else {
                    out.append("ACTIVE_STATE}");
                }
            } catch (DrmaaException e) {
                out.append(" {jobSubmissionState = <ERROR>}");
                error = true;
            }

            out.append(" {joinFiles = ");

            try {
                out.append(Boolean.toString(getJoinFiles()));
            } catch (DrmaaException e) {
                out.append(" {joinFiles = <ERROR>}");
                error = true;
            }

            out.append("}");

            try {
                if (getNativeSpecification() != null) {
                    out.append(" {nativeSpecification = \"");
                    out.append(getNativeSpecification());
                    out.append("\"}");
                }
            } catch (DrmaaException e) {
                out.append(" {nativeSpecification = <ERROR>}");
                error = true;
            }

            try {
                if (getOutputPath() != null) {
                    out.append(" {outputPath = ");
                    out.append(getOutputPath());
                    out.append("}");
                }
            } catch (DrmaaException e) {
                out.append(" {outputPath = <ERROR>}");
                error = true;
            }

            try {
                if (getRemoteCommand() != null) {
                    out.append(" {remoteCommand = ");
                    out.append(getRemoteCommand());
                    out.append("}");
                }
            } catch (DrmaaException e) {
                out.append(" {remoteCommand = <ERROR>}");
                error = true;
            }

            try {
                if (getSoftRunDurationLimit() != 0L) {
                    out.append(" {softRunDurationLimit = ");
                    out.append(getSoftRunDurationLimit());
                    out.append("ms}");
                }
            } catch (UnsupportedAttributeException e) {
                /* Skip it. */
            } catch (DrmaaException e) {
                out.append(" {softRunDurationLimit = <ERROR>}");
                error = true;
            }

            try {
                if (getSoftWallclockTimeLimit() != 0L) {
                    out.append(" {softWallclockTimeLimit = ");
                    out.append(getSoftWallclockTimeLimit());
                    out.append("ms}");
                }
            } catch (UnsupportedAttributeException e) {
                /* Skip it. */
            } catch (DrmaaException e) {
                out.append(" {softWallclockTimeLimit = <ERROR>}");
                error = true;
            }

            try {
                if (getStartTime() != null) {
                    PartialTimestampFormat ptf = new PartialTimestampFormat();

                    out.append(" {startTime = \"");
                    out.append(ptf.format(getStartTime()));
                    out.append("\"}");
                }
            } catch (DrmaaException e) {
                out.append(" {startTime = <ERROR>}");
                error = true;
            }

            try {
                if (getTransferFiles() != null) {
                    out.append(" {transferFiles = \"");
                    out.append(getTransferFiles().toString());
                    out.append("\"}");
                }
            } catch (UnsupportedAttributeException e) {
                /* Skip it. */
            } catch (DrmaaException e) {
                out.append(" {transferFiles = <ERROR>}");
                error = true;
            }

            try {
                if (getWorkingDirectory() != null) {
                    out.append(" {workingDirectory = ");
                    out.append(getWorkingDirectory());
                    out.append("}");
                }
            } catch (DrmaaException e) {
                out.append(" {workingDirectory = <ERROR>}");
                error = true;
            }

            toString = out.toString();

            /* If there was an error while getting a property value, we can't
             * unset the modified flag.  Because the errored property getter
             * might work during a later attempt, we have to leave the modified
             * flag as true so that we try again on the next call to
             * toString(). */
            modified = error;
        }

        return toString;
    }
}
