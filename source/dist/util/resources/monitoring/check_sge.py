#!/usr/bin/python

# check_sge - plugin for nagios to check the status of a host and its queues wtih SGE
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# Copyright 2004 Duke University
# Written by Sean Dilda <sean@duke.edu>

# Updates for SGE 8 and monitoring load independent of alarm -- see
# repo history.
version = '0.6'
#
#
# Version History:
#
# 0.5 - Fix bug where the timeout option wasn't honored
#       Fix bug where plugin tracebacked when a timeout was reached
# 0.4 - Handle receiving a 'global' line from 'qhost -q -h' - needed for SGE6
#       Change default SGE_ROOT and qhostPath to match my cluster
# 0.3 - Allow user to specify which queue states are warning and critical
#       Change command line arguments around
# 0.2 - Fix bug with checking the return value of qhost
# 0.1 - Initial version
#
#
# IMPORTANT NOTE: please check the full path to 'qhost' and the setting for
# SGE_ROOT below and make sure they are correct for your setup.

import os
import sys
import string
import getopt
import signal
import warnings
# fixme: replace popen2
warnings.simplefilter("ignore", DeprecationWarning)
import popen2

# fixme: should be configured externally
qhostPath = '/opt/sge/bin/lx-amd64/qhost'
os.environ['SGE_ROOT'] = '/opt/sge'
os.environ['SGE_CELL'] = 'default'
# These might be required for CSP
#os.environ['SGE_QMASTER_PORT'] = '6444'
#os.environ['SGE_CLUSTER_NAME'] = 'p6444'

nagiosStateOk = 0
nagiosStateWarning = 1
nagiosStateCritical = 2
nagiosStateUnknown = 3

hostName = ''
warningStates = ''
criticalStates = ''
childPid = 0
timeout = 15

def printUsage():
    print """\
Usage: check_sge -H <hostname> [-w <warning states>] [-c <critical states>]
    [--load-warning=<level>] [--load-critical=<level>] [-t <timeout>]"""

def printHelp():
    printUsage()
    print"""
Options:
-H, --hostname=HOST
   The hostname (as recognized by SGE) of the node you want to check
-w, --warning=STR
   The queue states that will cause a warning.  (ie -w ds)
-c, --critical=STR
   The queue states that will cause a critical error.  (ie -c au)
-t, --timeout=TIMEOUT
   Plugin timeout in seconds.  (default: 15)
--load-warning=LEVEL
   Load level that will cause a warning.
   If LEVEL has a leading "+", warn if load is that much higher than the
   number of CPUs.  If it has a trailing "%", warn if load is that
   percentage higher than the number of CPUs.
--load-critical=LEVEL
   Load level that will cause a critical error, similarly to --load-warning."""
# --vm-warning=LEVEL
#    Virtual memory value (in megabytes) that will cause a warning.
#    If LEVEL has a leading "+", warn if VM usage is that much higher than
#    the node's physical memory.  If it has a trailing "%", warn if VM
#    usage is that percentage higher than physical memory.
# --vm-critical=LEVEL
#    Virtual memory value that will cause a critical error, similarly to
#    --vm-warning.
# """
    sys.exit(nagiosStateUnknown)

try:
    optlist, args = getopt.getopt(sys.argv[1:], 'VhH:w:c:t:v?', ['version', 'help', 'hostname=', 'warning=', 'critical=', 'verbose', 'timeout=', "load-warning=", "load-critical=", "vm-warning=", "vm-critical="])
except getopt.GetoptError, errorStr:
    print errorStr
    printUsage()
    sys.exit(nagiosStateUnknown)

if len(args) != 0:
    printUsage()
    sys.exit(nagiosStateUnknown)

def check_load_or_vm_arg (name, arg):
    kind = None; val = 0
    try:
        if arg[0] == "+":
            kind = "+"
            val = float (arg[1:])
        elif arg[-1:] == "%":
            kind = "%"
            val = float (arg[:-1])
        else:
            val = float (arg)
    except:
        print "Invalid argument for %s: %s" % (name, arg)
    return (kind, val)

swarn = scrit = lwarn = lcrit = None

for opt, arg in optlist:
    if opt in ('-V', '--version'):
        print 'check_sge %s' % (version)
        sys.exit(nagiosStateUnknown)
    elif opt in ('-h', '--help'):
        printHelp()
        sys.exit(nagiosStateUnknown)
    elif opt in ('-H', '--hostname'):
        hostName = arg
    elif opt in ('-w', '--warning'):
        warningStates = arg
    elif opt in ('-c', '--critical'):
        criticalStates = arg
    elif opt in ('-v', '--verbose'):
        # Plugin guidelines require this, but we don't have anything extra to
        # report
        pass
    elif opt in ('-t', '--timeout'):
        try:
            timeout = int(arg)
        except ValueError:
            print 'Invalid argument for %s: %s' % (opt, arg)
            sys.exit(nagiosStateUnknown)
    elif opt == "--load-critical":
        lcrit = check_load_or_vm_arg (opt, arg)
    elif opt == "--load-warning":
        lwarn = check_load_or_vm_arg (opt, arg)
#     elif opt == "--vm-critical":
#         scrit = check_load_or_vm_arg (opt, arg)
#     elif opt == "--vm-warning":
#         swarn = check_load_or_vm_arg (opt, arg)
    elif opt == '-?':
        printUsage()
        sys.exit(nagiosStateUnknown)

if hostName == '':
    print 'No hostname specified.'
    printUsage()
    sys.exit(nagiosStateUnknown)

def handleAlarm(signum, frame):
    try:
        if childPid != 0:
            os.kill(childPid, signal.SIGKILL)
    except OSError:
        pass
    print 'Execution timeout exceeded'
    sys.exit(nagiosStateUnknown)

signal.signal(signal.SIGALRM, handleAlarm)
signal.alarm(timeout)

if os.access(qhostPath, os.X_OK) == 0:
    print 'Cannot execute %s' % (qhostPath)
    sys.exit(nagiosStateUnknown)

qhostPipe = popen2.Popen4('%s -q -h %s' % (qhostPath, hostName))
childPid = qhostPipe.pid
exitStatus = qhostPipe.wait()
childPid = 0
lines = qhostPipe.fromchild.readlines()
if not os.WIFEXITED(exitStatus) or os.WEXITSTATUS(exitStatus) != 0 or len(lines) < 3:
    # qhost didn't exit cleanly
    # Check if qhost printed something out
    if len(lines) >= 1:
        # Print first line of output.  Use [:-1] so that we don't get an extra
        # carriage return
        print 'Error with qhost %s: %s' % ('%s -q -h %s' % (qhostPath, hostName), lines[0][:-1])
    else:
        print 'Error with qhost'
    sys.exit(nagiosStateUnknown)

hostData = string.split(lines[2])
lines = lines[3:]
# If 'qhost -h' includes global (this happens in SGE6), then skip it
if hostData[0] == 'global':
    hostData = string.split(lines[0])
    lines = lines[1:]
queueLines = lines

def check_load_or_vm (param, warn, crit, base, value):
    if crit:
        if crit[0] == "+":
            threshold = crit[1] + base
        elif crit[0] == "%":
            threshold = base + (base * crit[1]/100.0)
        else:
            threshold = crit[1]
        if value >= threshold:
            print "CRITICAL: %s: %s >= %s" % (param, value, threshold)
            sys.exit (nagiosStateCritical)
    if warn:
        if warn[0] == "+":
            threshold = warn[1] + base
        elif warn[0] == "%":
            threshold = base + (base * warn[1]/100.0)
        else:
            threshold = warn[1]
        if value >= threshold:
            print "WARNING: %s: %s >= %s" % (param, value, threshold)
            sys.exit (nagiosStateWarning)

if hostData[6] == '-':
    print "CRITICAL: execd not communicating"
    sys.exit(nagiosStateCritical)

def memory (val):
    """Return number in MB, given value with G, M, or K suffix."""
    if val[-1:] == "T": return float (val[:-1]) * 1e6
    elif val[-1:] == "G": return float (val[:-1]) * 1000.0
    elif val[-1:] == "M": return float (val[:-1])
    # qhost prints `K', not `k'
    elif val[-1:] == "K": return float (val[:-1]) / 1000.0
    else: return float (val)

# Fixme: get the hostData indices from the header line, in case the format
# changes, like it did with v8.0.0
ncpu = int (hostData[2])
load = float (hostData[6])
mem = memory (hostData[7])
memused = memory (hostData[8])
vmused = memory (hostData[9]) + memused

check_load_or_vm ("load", None, lcrit, ncpu, load)
if memused + 100 > mem:         # 100 is arbitrary fiddle-factor
#    check_load_or_vm ("virtual memory", None, scrit, mem, vmused)
    print "WARNING: real memory (%sGB) consumed; real+swap=%s" % \
        (mem/1000.0, vmused/1000.0)
    sys.exit (nagiosStateWarning)

warning = []
critical = []
for line in queueLines:
    data = string.split(line)
    if len(data) >= 4:
        isWarning = 0
        isCritical = 0
        for state in data[3]:
            if state in warningStates:
                isWarning = 1
            elif state in criticalStates:
                isCritical = 1
        if isCritical == 1:
            critical.append('%s(%s)' % (data[0], data[3]))
        elif isWarning == 1:
            warning.append('%s(%s)' % (data[0], data[3]))


outputMsg = ''
exitCode = nagiosStateOk
if len(critical) >= 1:
    outputMsg = 'SGE CRITICAL: Queues in critical state: %s' % (string.join(critical, ', '))
    exitCode = nagiosStateCritical

if len(warning) >= 1:
    if len(outputMsg) > 0:
        outputMsg = outputMsg + ' '
    else:
        outputMsg = 'SGE WARNING: '
    outputMsg = outputMsg + 'Queues in warning state: %s' % (string.join(warning, ', '))
    exitCode = max(exitCode, nagiosStateWarning)

if len(outputMsg) > 0:
    print outputMsg
else:
    check_load_or_vm ("load", lwarn, lwarn, ncpu, load)
    if memused + 100 > mem:
        check_load_or_vm ("virtual memory", swarn, swarn, mem, vmused)
    print 'SGE OK Host and Queues'

sys.exit(exitCode)
