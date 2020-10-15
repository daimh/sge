# Python JSV library for SGE

#  The Contents of this file are made available subject to the terms of
#  the Sun Industry Standards Source License Version 1.2
#
#  Sun Microsystems Inc., March, 2001
#
#
#  Sun Industry Standards Source License Version 1.2
#  =================================================
#  The contents of this file are subject to the Sun Industry Standards
#  Source License Version 1.2 (the "License"); You may not use this file
#  except in compliance with the License. You may obtain a copy of the
#  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
#
#  Software provided under this License is provided on an "AS IS" basis,
#  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#  See the License for the specific provisions governing your rights and
#  obligations concerning the Software.
#
#  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#  Copyright: 2008 by Sun Microsystems, Inc.
#
#  All Rights Reserved.
# Copyright (C) 2013  Dave Love, Liverpool University

# This is a fairly direct translation of the Bourne shell library (and
# so inherits the licence to be on the safe side).

# It doesn't define a class with
# methods to be overridden, so usage is like:
#
#   #!/usr/bin/python
#   from JSV import *
#   def jsv_on_start ():
#       ...
#   def jsv_on_verify ():
#       ...

__all__ = ["jsv_is_param", "jsv_get_param",
           "jsv_del_param", "jsv_sub_is_param", "jsv_sub_get_param",
           "jsv_sub_add_param", "jsv_sub_del_param", "jsv_is_env",
           "jsv_get_env", "jsv_add_env", "jsv_mod_env", "jsv_del_env",
           "jsv_accept", "jsv_correct", "jsv_reject", "jsv_reject_wait",
           "jsv_show_params", "jsv_show_envs", "jsv_log_info",
           "jsv_log_warning", "jsv_log_error", "jsv_main",
           "jsv_logging_enabled"]

import os, re, sys, __main__
from datetime import datetime

if os.getenv("jsv_logging_enabled"):
    jsv_logging_enabled = True
else:
    jsv_logging_enabled = False
__jsv_logging_enabled=False
__jsv_logfile="/tmp/jsv_{0}.log".format(os.getpid())  # logfile

__jsv_state="initialized"

__jsv_cli_params = ["a", "ar", "A", "b", "ckpt", "cwd", "display", "dl",
                    "e", "h", "hold_jid", "hold_jid_ad", "i", "j", "js",
                    "m", "M", "masterq", "notify", "N", "o", "P", "p", "R",
                    "r", "shell", "S", "tc", "w"]

__jsv_mod_params = ["ac", "l_hard", "l_soft", "q_hard", "q_soft", "pe_min",
                    "pe_max", "pe_name", "binding_strategy", "binding_type",
                    "binding_amount", "binding_socket", "binding_core",
                    "binding_step", "binding_exp_n", "c_interval",
                    "c_occasion", "t_min", "t_max", "t_step"]

__jsv_add_params= ["CLIENT", "CONTEXT", "GROUP", "VERSION", "JOB_ID",
                   "CMDNAME", "CMDARGS", "USER"]

__jsv_param_list = __jsv_add_params + __jsv_cli_params + __jsv_mod_params

__jsv_all_params = {}

__jsv_all_envs={}

__jsv_quit=False

def jsv_clear_params ():
    global __jsv_all_params
    __jsv_all_params = {}

def jsv_clear_envs ():
   global __jsv_all_envs
   __jsv_all_envs = {}

def jsv_show_params ():
    for (n, v) in __jsv_all_params.items():
        print ("LOG INFO got param " + n + "=" + _jsv_format_param_val (v))
        sys.stdout.flush ()

def jsv_show_envs ():
    for n in __jsv_all_envs:
        print ("LOG INFO got env " + n + "=" + __jsv_all_params[n])
        sys.stdout.flush ()

def jsv_is_env (name):
    return name in __jsv_all_envs

def jsv_get_env (name):
    if name in __jsv_all_envs:
        return __jsv_all_envs[name]
    return None

def jsv_add_env (name, value=""):
    __jsv_all_envs[name] = value
    _jsv_send_command ("ENV ADD " + name + " " + value)

def jsv_mod_env (name, value=""):
    __jsv_all_envs[name] = value
    _jsv_send_command ("ENV MOD " + name + " " + value)

def jsv_del_env (name):
    if name in __jsv_all_envs:
        del __jsv_all_envs[name]
        _jsv_send_command ("ENV DEL " + name)

def jsv_is_param (name):
    return name in __jsv_all_params

def jsv_get_param (name):
    if name in __jsv_all_params:
        return __jsv_all_params[name]
    return None

def jsv_set_param (name, value):
    if name in __jsv_param_list:
        __jsv_all_params[name] = value
        _jsv_send_command ("PARAM " + name + " " + value)
    # fixme: else log error

def jsv_del_param (name):
    if name in __jsv_all_params:
        del __jsv_all_params[name]
        _jsv_send_command ("PARAM " + name)

def jsv_sub_is_param (param, var):
    if not jsv_is_param (param):
        return False
    val = jsv_get_param (param)
    return type (val) == dict and var in val

def jsv_sub_del_param (param, var):
    if jsv_sub_is_param (param, var):
        del __jsv_all_params[param][var]
        subs = __jsv_all_params[param]
        elems = []
        for i in subs:
            if subs[i]:
                elems.append (var + "=" + subs[i])
            else:
                elems.append (var)
        _jsv_send_command ("PARAM" + param + " " + _jsv_format_param_val(elems))

def jsv_sub_get_param (param, var):
    if jsv_sub_is_param (param, var):
        return jsv_get_param(param)[var]
    return None

def _jsv_format_param_val (value):
    if type (value) != dict:
        return value
    else:
        l = []
        for (n, v) in value.items():
            if v:
                l.append (n + "=" + v)
            else:
                l.append (n)
        return ",".join(l)

def jsv_sub_add_param (param, var, val=""):
    if type (jsv_get_param (param)) == dict:
        __jsv_all_params[param][var] = val
    else:
        print 'non dict', jsv_get_param (param)
        __jsv_all_params[param] = {var: val}
    subs = __jsv_all_params[param]
    _jsv_send_command ("PARAM" + param + " " + _jsv_format_param_val(subs))

def jsv_send_env ():
   _jsv_send_command ("SEND ENV")

def _jsv_accept_etc (result, message):
    global __jsv_state
    if  __jsv_state == "verifying":
        _jsv_send_command ("RESULT " + state + " ACCEPT " + message)
        __jsv_state = "initialized"
    else:
        _jsv_send_command ("ERROR JSV script will send RESULT command but is in state "\
                               + __jsv_state)
    
def jsv_accept (message=""):
    _jsv_accept_etc ("ACCEPT", message)

def jsv_correct (message=""):
    _jsv_accept_etc ("CORRECT", message)

def jsv_reject (message=""):
    _jsv_accept_etc ("REJECT", message)

def jsv_reject_wait (message=""):
    _jsv_accept_etc ("REJECT_WAIT", message)

def jsv_log_info (message):
    _jsv_send_command ("LOG INFO " + message)

def jsv_log_warning (message):
    _jsv_send_command ("LOG WARNING " + message)

def jsv_log_error (message):
    _jsv_send_command ("LOG ERROR " + message)

def _jsv_handle_start_command ():
    global __jsv_state
    if __jsv_state == "initialized":
        __main__.jsv_on_start ()
        _jsv_send_command ("STARTED")
        __jsv_state = "started"
    else:
        _jsv_send_command ("ERROR JSV script got START command but is in state "\
                               + __jsv_state)

def _jsv_handle_begin_command ():
    global __jsv_state
    if __jsv_state == "started":
        __jsv_state = "verifying"
        __main__.jsv_on_verify ()
        jsv_clear_params ()
        jsv_clear_envs ()
    else:
        _jsv_send_command ("ERROR JSV script got BEGIN command but is in state "\
                               + __jsv_state)

def _jsv_handle_param_command (param, value):
    global __jsv_state
    if __jsv_state == "started":
        __jsv_all_params[param] = {}
        for item in value.split(","):
            kv = item.split("=")
            if len (kv) == 2:
                __jsv_all_params[param][kv[0]] = kv[1]
            else:
                __jsv_all_params[param][kv[0]] = None
    else:
        _jsv_send_command ("ERROR JSV script got PARAM command but is in state "\
                               + __jsv_state)

def _jsv_handle_env_command (action, name, data):
    global __jsv_state
    if __jsv_state == "started":
        if action == "ADD":
            __jsv_all_envs[name] = data
    else:
        _jsv_send_command ("ERROR JSV script got ENV command but is in state "\
                               + __jsv_state)

def _jsv_send_command (string):
    print (string)
    sys.stdout.flush ()
    _jsv_script_log ("<<< " + string)

def _jsv_script_log (string):
    if __jsv_logging_enabled or jsv_logging_enabled:
        __jsv_logfile.write(string)

def jsv_main ():
    global __jsv_quit
    _jsv_script_log (os.path.basename(__file__) + " started on " +
                     datetime.ctime(datetime.now()) +
"""
This file contains logging output from an SGE JSV script. Lines beginning
with >>> contain the data which was send by a command line client or
sge_qmaster to the JSV script. Lines beginning with <<< contain data
which is sent for this JSV script to the client or sge_qmaster.
""")
    __jsv_quit = False
    try:
        while not __jsv_quit:
            ip = sys.stdin.readline().strip('\n')
            if len (ip) == 0:
                continue
            _jsv_script_log (">>> " + ip)
            if ip.startswith ("QUIT"):
                __jsv_quit = True
            elif ip.startswith ("PARAM"):
                [cmd, name, value] = re.split("\s+", ip, maxsplit=3)
                _jsv_handle_param_command (name, value)
            elif ip.startswith ("ENV"):
                [cmd, modifier, name, value] = re.split("\s+", ip, maxsplit=4)
                _jsv_handle_env_command (modifier, name, value)
            elif ip.startswith ("START"):
                _jsv_handle_start_command ()
            elif ip.startswith ("BEGIN"):
                _jsv_handle_begin_command ()
            elif ip.startswith ("SHOW"):
                jsv_show_params ()
                jsv_show_envs ()
            else:
                args = re.split ("\s+", ip, maxsplit=2)
                _jsv_send_command ("ERROR JSV script got unknown command " + args[0])
    except (EOFError):
        pass

    _jsv_script_log (os.path.basename(__file__) + " is terminating on " +
                     datetime.ctime(datetime.now()))
