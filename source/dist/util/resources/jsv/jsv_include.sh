#
#___INFO__MARK_BEGIN__
##########################################################################
#
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
#
##########################################################################
#___INFO__MARK_END__

########################################################################### 
#
# Be careful:  Job verification scripts are started with sgeadmin 
#              permissions if they are executed within the master process
#

# fixme: use arrays in bash

PATH=/bin:/usr/bin
ARCH=`"$SGE_ROOT/util/arch"`

# Is logging enabled?  Setting it to "true" produces a log in $__jsv_logfile
jsv_logging_enabled=${jsv_logging_enabled:-false}
# An old name for the above, never documented.  Use one or the other.
__jsv_logging_enabled="false"
# fixme: use mktemp
__jsv_logfile="/tmp/jsv_$$.log"  # logfile

# Current state of the script
# Might be "initialized", "started" or "verifying"
__jsv_state="initialized"       

# Following strings are switch names of command line clients (qsub, qrsh, ...) 
# and these strings will also be used as variable suffixes in this script
__jsv_cli_params="a ar A b ckpt cwd display\
                dl e h hold_jid hold_jid_ad i j js m M masterq notify\
                N o P p R r shell S tc w"

# These names are the suffixes of variable names which will contain
# the information of following submit client switches:
#     ac: summarized job context information (-sc -ac -dc)
#     l_hard: hard resource request (-hard -l ...)
#     l_soft: soft resource requests (-soft -l ...)
#     q_hard: hard queue resource request (-hard -q ...)
#     q_soft: soft queue resource request (-soft -q ...)
#     pe_min: minimum of pe range (-pe <pe_name> <pe_min>-<pe_max>)
#     pe_max: maximum of pe range (-pe <pe_name> <pe_min>-<pe_max>)
#     pe_name: pe name (-pe <pe_name> <pe_min>-<pe_max>)
#     binding_strategy: binding strategy (-binding [<type>] <strategy>:<amount>:<socket>:<core>)
#     binding_type: binding type name (-binding [<type>] <strategy>:<amount>:<socket>:<core>) 
#     binding_amount: binding amount (-binding [<type>] <strategy>:<amount>:<socket>:<core>)
#     binding_socket: binding socket (-binding [<type>] <strategy>:<amount>:<socket>:<core>)
#     binding_core: binding core (-binding [<type>] <strategy>:<amount>:<socket>:<core>)
#     binding_step: binding step (-binding [<type>] "striding":<amount>:<step>)
#     binding_exp_n: length of explicit list (-binding [<type>] "explicit":<socket0>,<core0>:...)
#     t_min: minimum of array job (-t) range
#     t_max: maximum of array job (-t) range
#     t_step: step for array job (-t) range
__jsv_mod_params="ac l_hard l_soft q_hard q_soft pe_min pe_max pe_name\
                  binding_strategy binding_type binding_amount binding_socket\
                  binding_core binding_step binding_exp_n\
                  c_interval c_occasion t_min t_max t_step"

# Here are the suffixes of variable names which do not directly appear
# as named switches in a client.
#     CLIENT: submit client which was used
#     CONTEXT: in which context is this script executed
#     ...
__jsv_add_params="CLIENT CONTEXT GROUP VERSION JOB_ID CMDNAME CMDARGS USER"

# Values specified with the list below will be available in this script 
# as variables with the name "jsv_param_<name>". If a corresponding value
# was specified during submission then this variable will have the
# same value. Find more information in jsv(5) and submit(1) man page.
__jsv_all_params="$__jsv_add_params $__jsv_cli_params $__jsv_mod_params"

# This is a space separated list of environment variable names which
# will be built up during runtime, when this script receives the job env.
__jsv_all_envs=""

__jsv_undef="variable_is_undefined"
__jsv_quit="false"
__jsv_saved_ifs="$IFS"

###### jsv/jsv_clear_params() ##################################################
#  NAME
#     jsv_clear_params() -- Clears all received job parameters.
#
#  SYNOPSIS
#     jsv_clear_params() 
#
#  FUNCTION
#     This function clears all received job parameters that were stored 
#     during the last job verification process.
#
#  INPUTS
#     none
#
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_clear_envs() 
#     jsv/jsv_show_params()
################################################################################
jsv_clear_params()
{
   for __jsv_i in $__jsv_all_params; do
      __jsv_name="jsv_param_${__jsv_i}"
      __jsv_command=`eval "echo \$\{$__jsv_name\:\-$__jsv_undef\}"`
      __jsv_isdef=`eval "echo $__jsv_command"`
      if [ "$__jsv_isdef" != "$__jsv_undef" ]; then
         unset $__jsv_name
      fi
      unset __jsv_name
      unset __jsv_command
      unset __jsv_isdef
   done
}

###### jsv/jsv_clear_envs() ##################################################
#  NAME
#     jsv_clear_envs() -- Clears all received job environment variables.
#
#  SYNOPSIS
#     jsv_clear_envs() 
#
#  FUNCTION
#     This function clears all received job environment variables that
#     were stored during the last job verification process.
#
#  INPUTS
#     none
#
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_clear_params() 
#     jsv/jsv_show_envs()
################################################################################
jsv_clear_envs()
{
   for __jsv_i in $__jsv_all_envs; do
      __jsv_name="jsv_env_$__jsv_i"
      __jsv_command=`eval "echo \$\{$__jsv_name\:\-$__jsv_undef\}"`
      __jsv_isdef=`eval "echo $__jsv_command"`
      if [ "$__jsv_isdef" != "$__jsv_undef" ]; then
         unset $__jsv_name
      fi
      unset __jsv_name
      unset __jsv_command
      unset __jsv_isdef
   done
   __jsv_all_envs=""
}

###### jsv/jsv_show_params() ##################################################
#  NAME
#     jsv_show_params() -- Prints all job parameters. 
#
#  SYNOPSIS
#     jsv_show_params() 
#
#  FUNCTION
#     A call of this function reports all known job parameters to the
#     counterpart of this script (client or master daemon thread). These
#     parameters will be reported as info messages and appear
#     either in the stdout stream of the client or in the message file of 
#     the master process.
#
#  INPUTS
#     none
#
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_clear_params() 
#     jsv/jsv_show_envs() 
################################################################################
jsv_show_params()
{
   for __jsv_i in $__jsv_all_params; do
      __jsv_name="jsv_param_$__jsv_i"
      __jsv_command=`eval "echo \$\{$__jsv_name\:\-$__jsv_undef\}"`
      __jsv_isdef=`eval "echo $__jsv_command"`
      if [ "$__jsv_isdef" != "$__jsv_undef" ]; then
         eval "echo LOG INFO got param $__jsv_i=\${$__jsv_name}"
      fi
      unset __jsv_name
      unset __jsv_command
      unset __jsv_isdef
   done
}

###### jsv/jsv_show_envs() ##################################################
#  NAME
#     jsv_show_envs() -- Prints all job environment variables.
#
#  SYNOPSIS
#     jsv_show_envs() 
#
#  FUNCTION
#     A call of this function reports all known job environment variables
#     to the counterpart of this script (client or master daemon thread). 
#     They will be reported as info messages and appear in the stdout 
#     stream of the client or in the message file of the master process. 
#
#  INPUTS
#     none
#
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_clear_params() 
#     jsv/jsv_clear_envs() 
################################################################################
jsv_show_envs()
{
   for __jsv_i in $__jsv_all_envs; do
      __jsv_name="jsv_env_$__jsv_i"
      __jsv_command=`eval "echo \$\{$__jsv_name\:\-$__jsv_undef\}"`
      __jsv_isdef=`eval "echo $__jsv_command"`
      if [ "$__jsv_isdef" != "$__jsv_undef" ]; then
         eval "echo LOG INFO got env $__jsv_i=\${$__jsv_name}"
      fi
      unset __jsv_name
      unset __jsv_command
      unset __jsv_isdef
   done
}

###### jsv/jsv_is_env() ######################################################
#  NAME
#     jsv_is_env() -- Returns whether or not a specific job environment 
#                     variable exists in the job currently being verified.
#
#  SYNOPSIS
#     jsv_is_env(variable_name) 
#
#  FUNCTION
#     If the function returns "true", then the job environment variable with 
#     the name "variable_name" exists in the job currently being verified and
#     jsv_get_env() can be used to retrieve the value of that variable.
#
#     If the function returns "false", then the job environment variable with 
#     the name "variable_name" does not exist.
#
#  INPUTS
#     variable_name - e.g "HOME", "LD_LIBRARY_PATH" ...
#
#  RESULT
#     "true" or "false" as string 
#
#  SEE ALSO
#     jsv/jsv_clear_params() 
#     jsv/jsv_show_envs() 
#     jsv/jsv_get_env()
################################################################################
jsv_is_env() 
{
   __jsv_name="jsv_env_$1"
   __jsv_command=`eval "echo \$\{$__jsv_name\:\-$__jsv_undef\}"`
   __jsv_isdef=`eval "echo $__jsv_command"`
   if [ "$__jsv_isdef" != "$__jsv_undef" ]; then
      echo true 
   else
      echo false
   fi
   unset __jsv_name
   unset __jsv_command
   unset __jsv_isdef
}

###### jsv/jsv_get_env() #####################################################
#  NAME
#     jsv_get_env() -- Returns the value of a job environment variable.
#
#  SYNOPSIS
#     jsv_get_env(variable_name) 
#
#  FUNCTION
#     This function returns the value of a job environment variable
#     ("variable_name"). 
#
#     This variable has to be passed with the command line switch
#     -v or -V and it has to be enabled that environment variable data is 
#     passed to JSV scripts. Environment variable data is passed when the 
#     function jsv_send_env() is called in the callback function 
#     jsv_on_start().
# 
#     If the variable does not exist or if environment variable 
#     information is not available then an empty string will be returned. 
#    
#  INPUTS
#     variable_name - e.g "HOME", "LD_LIBRARY_PATH" ...
#
#  RESULT
#     value of the variable as string
#
#  SEE ALSO
#     jsv/jsv_show_envs() 
#     jsv/jsv_is_env()
#     jsv/jsv_add_env()
#     jsv/jsv_mod_env()
#     jsv/jsv_del_env()
#     jsv/jsv_on_start()
################################################################################
jsv_get_env() 
{
   __jsv_name="jsv_env_$1"
   __jsv_command=`eval "echo \$\{$__jsv_name\:\-$__jsv_undef\}"`
   __jsv_isdef=`eval "echo $__jsv_command"`
   if [ "$__jsv_isdef" != "$__jsv_undef" ]; then
      eval "echo \${$__jsv_name}" 
   fi
   unset __jsv_name
   unset __jsv_command
   unset __jsv_isdef
}

###### jsv/jsv_add_env() #####################################################
#  NAME
#     jsv_add_env() -- Adds a job environment variable.
#
#  SYNOPSIS
#     jsv_add_env(variable_name, variable_value) 
#
#  FUNCTION
#     This function adds an additional environment variable to the set 
#     of variables that will exported to the job, when it is started.
#     As a result the "variable_name" and "variable_value" become 
#     available, as if the -v or -V was specified during job submission.
#
#     "variable_value" is optional. If there is an empty string passed 
#     then the variable is defined without value.
#
#     If "variable_name" already exists in the set of job environment 
#     variables, then the corresponding value will be replaced by
#     "variable_value", as if the function jsv_mod_env() was used. 
#     If an empty string is passed then the old value will be deleted.
#
#     To delete a environment variable the function jsv_del_env()
#     has to be used.
#
#  INPUTS
#     variable_name  - e.g "HOME", "LD_LIBRARY_PATH" ...
#     variable_value - e.g "/home/user", "", ...
#
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_show_envs() 
#     jsv/jsv_is_env()
#     jsv/jsv_get_env()
#     jsv/jsv_mod_env()
#     jsv/jsv_del_env()
#     jsv/jsv_on_start()
################################################################################
jsv_add_env() 
{
   __jsv_name="jsv_env_$1"
   eval "$__jsv_name=$2"
   _jsv_send_command ENV ADD "$1" "$2"
   unset __jsv_name
}

###### jsv/jsv_mod_env() #####################################################
#  NAME
#     jsv_mod_env() -- Modifies the value of a environment variable.
#
#  SYNOPSIS
#     jsv_mod_env(variable_name, variable_value) 
#
#  FUNCTION
#     This function modifies an existing environment variable that is 
#     in the set of variables which will exported to the job, when it 
#     is started.
#     As a result, the "variable_name" and "variable_value" will be
#     available as if the -v or -V was specified during job submission.
#
#     "variable_value" is optional. If there is an empty string passed 
#     then the variable is defined without a value.
#
#     If "variable_name" does not already exist in the set of job 
#     environment variables, then the corresponding name and value will 
#     be added as if the function jsv_add_env() was used. 
#
#     To delete a environment variable, use the function jsv_del_env().
#
#  INPUTS
#     variable_name  - e.g "HOME", "LD_LIBRARY_PATH" ...
#     variable_value - e.g. "/home/user", "" ...
#
#  RESULT
#     none 
#
#  SEE ALSO
#     jsv/jsv_show_envs() 
#     jsv/jsv_is_env()
#     jsv/jsv_get_env()
#     jsv/jsv_add_env()
#     jsv/jsv_get_env()
#     jsv/jsv_del_env()
#     jsv/jsv_on_start()
################################################################################
jsv_mod_env() 
{
   __jsv_name="jsv_env_$1"
   eval "$__jsv_name=$2"
   _jsv_send_command ENV MOD "$1" "$2"
   unset __jsv_name
}

###### jsv/jsv_del_env() #####################################################
#  NAME
#     jsv_del_env() -- Deletes a job environment variable.
#
#  SYNOPSIS
#     jsv_del_env(variable_name) 
#
#  FUNCTION
#     This function removes a job environment variable ("variable_name")
#     from the set of variables that will be exported
#     to the job, when it is started.
#
#     If "variable_name" does not already exist in the set of job
#     environment variables then the command is ignored.
#
#     To change the value of a variable use the function jsv_mod_env()
#     to add a new value, call the function jsv_add_env().
#
#  INPUTS
#     variable_name - e.g "HOME", "LD_LIBRARY_PATH" ...
#
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_show_envs() 
#     jsv/jsv_is_env()
#     jsv/jsv_get_env()
#     jsv/jsv_add_env()
#     jsv/jsv_mod_env()
#     jsv/jsv_on_start()
################################################################################
jsv_del_env() 
{
   __jsv_name="jsv_env_$1"
   __jsv_command=`eval "echo \$\{$__jsv_name\:\-$__jsv_undef\}"`
   __jsv_isdef=`eval "echo $__jsv_command"`
   if [ "$__jsv_isdef" != "$__jsv_undef" ]; then
      eval "unset $__jsv_name" 
      _jsv_send_command ENV DEL "$1" 
   fi
   unset __jsv_name
   unset __jsv_command
   unset __jsv_isdef
}

###### jsv/jsv_is_param() #####################################################
#  NAME
#     jsv_is_param() -- Returns whether or not a specific job parameters
#                       exists in the job currently being verified.
#
#  SYNOPSIS
#     jsv_is_param(param_name) 
#
#  FUNCTION
#     This function returns whether or not a specific job parameters is 
#     available for the job which is currently being verified. Either the 
#     string "true" or "false" will be returned. The availability/absence 
#     of a job parameter does not mean that the corresponding command line
#     switch was used/not used. 
#
#  INPUTS
#     param_name - The following values are allowed. Corresponding
#                  qsub/qrsh/qsh/... switches are mentioned in parenthesis 
#                  next to the parameter name if they are different. 
#                  Find additional information in qsub(1) man page where
#                  additional notes can be found describing the availability 
#                  and value format.
#                  Multi-letter job parameters written in capital letters
#                  are pseudo parameters. These parameters describe
#                  either the context where this JSV is executed or
#                  additional values describing the job submission
#                  context.
#
#        name           switch/description
#        -------------- -------------------------------------------
#        a              
#        ac             (combination of -ac, -sc, -dc)
#        ar             
#        A             
#        b
#        binding_strategy (from -binding)
#        binding_type
#        binding_amount
#        binding_socket
#        binding_core
#        binding_step
#        binding_exp_n
#        c_interval     (from -c)
#        c_occasion
#        ckpt
#        cwd
#        display 
#        dl
#        e
#        h
#        hold_jid
#        hold_jid_ad
#        i
#        l_hard         (-hard followed by -l)   
#        l_soft         (-soft followed by -l)
#        j
#        js
#        m
#        M
#        masterq
#        N
#        notify
#        o
#        ot
#        P
#        pe_min         (from -pe)
#        pe_mex         (from -pe)
#        pe_name        (from -pe)
#        q_hard         (-hard followed by -q)
#        q_soft         (-soft followed by -q)
#        R
#        r
#        shell
#        S
#        tc
#        t_min          (from -t)
#        t_max
#        t_step
#        w
#        CLIENT         name of the submit client (e.g qsub, qmon, ...)
#        CONTEXT        "client" or "server"
#        GROUP          unix group of the job submitter
#        VERSION        1.0
#        JOB_ID         0 in client context or job id of the job in 
#                       server context
#        CMDNAME        binary/script path
#        CMDARGS        number of additional job arguments
#        CMDARG<i>      parameter <i>
#        USER           unix user of the person which submitted the job
#
#  RESULT
#     "true" or "false"
#
#  SEE ALSO
#     jsv/jsv_show_params() 
#     jsv/jsv_is_param()
#     jsv/jsv_get_param()
#     jsv/jsv_on_start()
################################################################################
jsv_is_param() 
{
   __jsv_name="jsv_param_$1"
   __jsv_command=`eval "echo \$\{$__jsv_name\:\-$__jsv_undef\}"`
   __jsv_isdef=`eval "echo $__jsv_command"`
   if [ "$__jsv_isdef" != "$__jsv_undef" ]; then
      echo true 
   else
      echo false
   fi
   unset __jsv_name
   unset __jsv_command
   unset __jsv_isdef
}

###### jsv/jsv_get_param() #####################################################
#  NAME
#     jsv_get_param() -- Returns the value of a job parameter.
#
#  SYNOPSIS
#     jsv_get_param(param_name) 
#
#  FUNCTION
#     This function returns the value of a specific job parameter 
#     ("param_name"). 
#
#     This value is only available if the function jsv_is_param()
#     returns "true". Otherwise an empty string is returned.
# 
#  INPUTS
#     param_name - Find a list of allowed parameter names in the
#                  documentation section for the function jsv_is_param().
#
#  RESULT
#     value of the variable as string
#
#  SEE ALSO
#     jsv/jsv_show_param() 
#     jsv/jsv_is_param()
#     jsv/jsv_set_param()
#     jsv/jsv_del_param()
################################################################################
jsv_get_param() 
{
   __jsv_name="jsv_param_$1"
   __jsv_command=`eval "echo \"\$\{$__jsv_name\:\-$__jsv_undef\}\""`
   __jsv_isdef=`eval "echo \"$__jsv_command\""`
   if [ "$__jsv_isdef" != "$__jsv_undef" ]; then
      eval "echo \${$__jsv_name}" 
   fi
   unset __jsv_name
   unset __jsv_command
   unset __jsv_isdef
}

###### jsv/jsv_set_param() #####################################################
#  NAME
#     jsv_set_param() -- Changes the value of a job environment variable. 
#
#  SYNOPSIS
#     jsv_set_param(param_name, param_value) 
#
#  FUNCTION
#     This function changes the job parameter with the name "param_name"
#     to the value "param_value".
#
#     If "param_name" is an empty string then the corresponding
#     job parameter will be deleted. 
# 
#  INPUTS
#     param_name - Find a list of allowed parameter names in the
#                  documentation section for the function jsv_is_param(). 
#
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_show_param() 
#     jsv/jsv_is_param()
#     jsv/jsv_set_param()
#     jsv/jsv_del_param()
################################################################################
jsv_set_param()
{
   __jsv_name="jsv_param_$1"
   eval "$__jsv_name=$2"
   _jsv_send_command PARAM "$1" "$2"
   unset __jsv_name
}

###### jsv/jsv_del_param() #####################################################
#  NAME
#     jsv_del_param() -- Deletes a job parameter. 
#
#  SYNOPSIS
#     jsv_del_param(param_name) 
#
#  FUNCTION
#     This function deletes the job parameter with the name "param_name".
# 
#  INPUTS
#     param_name - Find a list of allowed parameter names in the
#                  documentation section for the function jsv_is_param().
#
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_show_param() 
#     jsv/jsv_is_param()
#     jsv/jsv_set_param()
#     jsv/jsv_del_param()
################################################################################
jsv_del_param() 
{
   __jsv_name="jsv_param_$1"
   __jsv_command=`eval "echo \$\{$__jsv_name\:\-$__jsv_undef\}"`
   __jsv_isdef=`eval "echo $__jsv_command"`
   if [ "$__jsv_isdef" != "$__jsv_undef" ]; then
      eval "unset \${$__jsv_name}" 
      _jsv_send_command PARAM "$1" ""
   fi
   unset __jsv_name
   unset __jsv_command
   unset __jsv_isdef
}

###### jsv/jsv_sub_is_param() ##################################################
#  NAME
#     jsv_sub_is_param() -- Returns whether or not a job parameter list 
#                           contains a variable. 
#
#  SYNOPSIS
#     jsv_sub_is_param(param_name, variable_name) 
#
#  FUNCTION
#     Some job parameters are lists that can contain multiple variables with
#     an optional value.
#
#     This function returns "true" if a job parameters list contains a 
#     variable and "false" otherwise. "false" might also indicate that
#     the parameter list itself is not available. 
#
#  INPUTS
#     param_name - The following parameters are list parameters
#
#        name           switch/description
#        -------------- -------------------------------------------
#        ac             (combination of -ac, -sc, -dc)
#        hold_jid
#        hold_jid_ad
#        l_hard         (-hard followed by -l)   
#        l_soft         (-soft followed by -l)
#        M
#        masterq
#        q_hard         (-hard followed by -q)
#        q_soft         (-soft followed by -q)
#
#     variable_name - String value
#        
#        The value depends on the "param_name". For "ac" the 
#        "variable_name" has to be a variable name. "l_hard", "l_soft" 
#        require a resource name (e.g "h_vmem"). "masterq", "q_hard",
#        "q_soft" require a cluster queue name or a queue instance
#        name.
#
#  RESULT
#     "true" or "false"
#
#  EXAMPLE
#
#     Assume that the following job is submitted:
#
#        qsub -ac a=1,b,d=7 -l h_vmem=5G,lic=1 -M user@domain script.sh 
#
#     For this job, the following function calls will return the 
#     following values:
#  
#        function call                             value
#        ----------------------------------------- ---------
#        jsv_sub_is_param("ac", "a")               true
#        jsv_sub_is_param("ac", "c")               false 
#        jsv_sub_is_param("l_hard", "h_vmem")      true 
#        jsv_sub_is_param("l_soft", "h_vmem")      false 
#        jsv_sub_is_param("M", "user@domain")      true 
#        jsv_sub_is_param("M", "unknown@domain")   false
#
#  SEE ALSO
#     jsv/jsv_show_params() 
#     jsv/jsv_is_param()
#     jsv/jsv_get_param()
#     jsv/jsv_on_start()
################################################################################
jsv_sub_is_param() {
   __jsv_ret="false"
   __jsv_name="jsv_param_$1"
   __jsv_sub_name="$2" 
   __jsv_command=`eval "echo \$\{$__jsv_name\:\-$__jsv_undef\}"`
   __jsv_list=`eval "echo $__jsv_command"`
   if [ "$__jsv_list" != "$__jsv_undef" ]; then
      IFS=","
      for __jsv_i in $__jsv_list; do
         IFS="="
         for __jsv_j in $__jsv_i; do
            IFS="$__jsv_saved_ifs"
            if [ "$__jsv_j" = "$__jsv_sub_name" ]; then
               __jsv_ret="true"
               break
            else
               break
            fi
            IFS="="
         done
         IFS="$__jsv_saved_ifs"
         if [ "$__jsv_ret" = "true" ]; then
            break
         fi
         IFS=","
      done
      IFS="$__jsv_saved_ifs"
   fi
   unset __jsv_name
   unset __jsv_sub_name
   unset __jsv_command
   unset __jsv_list
   echo $__jsv_ret
   return
}

###### jsv/jsv_sub_del_param() ################################################
#  NAME
#     jsv_sub_del_param() -- Deletes a variable from a sublist.
#
#  SYNOPSIS
#     jsv_sub_del_param(param_name, variable_name) 
#
#  FUNCTION
#     Some job parameters are lists which can contain multiple variables with
#     an optional value. 
#
#     This function deletes a variable with the name "variable_name" and 
#     if available the corresponding value. If "variable_name" is not
#     available then the command will be ignored.
#
#  INPUTS
#     param_name - Find a list of list parameter names in the
#                  documentation for the function jsv_sub_is_param().
#
#     variable_name - Find more information in the documentation
#                     section for jsv_sub_is_param().
#        
#  RESULT
#     none
#
#  EXAMPLE
#
#     Assume that the following job is submitted:
#
#        qsub -ac a=1,b,d=7 -l h_vmem=5G,lic=1 -M user@domain script.sh 
#
#     Assume the following commands in jsv_on_verify()
#
#        jsv_sub_del_param("ac", "b");
#        jsv_sub_del_param("ac", "d");
#        jsv_sub_del_param("l_hard", "lic");
#        jsv_sub_del_param("M", "lic");
#        jsv_correct();
#
#     In that case the job will be modified as if the following qsub 
#     command was used:
#
#        qsub -ac a=1 -l h_vmem=5G script.sh
#
#  SEE ALSO
#     jsv/jsv_show_params() 
#     jsv/jsv_sub_is_param()
#     jsv/jsv_sub_get_param()
#     jsv/jsv_sub_add_param()
#     jsv/jsv_on_verify()
################################################################################
jsv_sub_del_param() 
{
   __jsv_param_name="$1"
   __jsv_name="jsv_param_$__jsv_param_name"
   __jsv_sub_name="$2" 
   __jsv_command=`eval "echo \$\{$__jsv_name\:\-$__jsv_undef\}"`
   __jsv_list=`eval "echo $__jsv_command"`
   __jsv_new_param=""
   if [ "$__jsv_list" != "$__jsv_undef" ]; then

      # split token between ',' character
      IFS=","
      for __jsv_i in $__jsv_list; do
         __jsv_found="false"

         # split the first string before '=' character
         # This is the variable name and if it is the
         # one which should be deleted then set "found" 
         # to "true" 
         IFS="="
         for __jsv_j in $__jsv_i; do
            IFS="$__jsv_saved_ifs"
            if [ "$__jsv_j" = "$__jsv_sub_name" ]; then
               __jsv_found="true"
               break
            else
               break
            fi
            IFS="="
         done
         IFS="$__jsv_saved_ifs"
         # append all entries to new_param
         # skip only the entry which should be deleted
         if [ "$__jsv_found" != "true" ]; then
            if [ "$__jsv_new_param" = "" ]; then  
               __jsv_new_param="$__jsv_i"
            else
               __jsv_new_param="${__jsv_new_param},$__jsv_i"
            fi
         fi
         IFS=","
      done
      IFS="$__jsv_saved_ifs"

      # set local variable and send modification to client/master
      eval "$__jsv_name=\"\$__jsv_new_param\""
      _jsv_send_command PARAM $__jsv_param_name "$__jsv_new_param" 
   fi
   unset __jsv_new_param
   unset __jsv_list
   unset __jsv_command
   unset __jsv_name
   unset __jsv_sub_name
   unset __jsv_param_name
   return
}

###### jsv/jsv_sub_get_param() ################################################
#  NAME
#     jsv_sub_get_param() -- Returns the value of the variable 
#                            ("variable_name") in a sublist.
#
#  SYNOPSIS
#     jsv_sub_get_param(param_name, variable_name) 
#
#  FUNCTION
#     Some job parameters are lists that can contain multiple variables 
#     with an optional value. 
#
#     This function returns the value of the variable ("variable_name").
#     For sub list elements that have no value an empty string will be 
#     returned.
#
#  INPUTS
#     param_name - Find a list of list parameter names in the
#                  documentation for the function jsv_sub_is_param().
#
#     variable_name - Find more information in the documentation
#                     section for jsv_sub_is_param().
#        
#  RESULT
#     String representation of the value or an empty string if there
#     is no value available.
#
#  EXAMPLE
#
#     Assume that the following job is submitted:
#
#        qsub -ac a=1,b,d=7 -l h_vmem=5G,lic=1 -M user@domain script.sh 
#
#     For the above job, the following function calls will return following values:
#  
#        function call                              value
#        -----------------------------------------  ---------
#        jsv_sub_get_param("ac", "a")               "1"
#        jsv_sub_get_param("ac", "b")               "" 
#        jsv_sub_get_param("l_hard", "h_vmem")      "5G" 
#        jsv_sub_get_param("l_soft", "h_vmem")      "" 
#        jsv_sub_get_param("M", "user@domain")      "" 
#        jsv_sub_get_param("M", "unknown@domain")   ""
#
#  SEE ALSO
#     jsv/jsv_show_params() 
#     jsv/jsv_sub_is_param()
#     jsv/jsv_sub_get_param()
#     jsv/jsv_sub_add_param()
#     jsv/jsv_on_verify()
################################################################################
jsv_sub_get_param() 
{
   __jsv_param_name="$1"
   __jsv_name="jsv_param_$__jsv_param_name"
   __jsv_sub_name="$2" 
   __jsv_command=`eval "echo \$\{$__jsv_name\:\-$__jsv_undef\}"`
   __jsv_list=`eval "echo $__jsv_command"`
   if [ "$__jsv_list" != "$__jsv_undef" ]; then

      # split token between ',' character
      IFS=","
      for __jsv_i in $__jsv_list; do
         __jsv_found="false"

         # split the first string before '=' character
         # This is the variable name and if it is the
         # one which should be deleted then set "found" 
         # to "true" 
         IFS="="
         for __jsv_j in $__jsv_i; do
            IFS="$__jsv_saved_ifs"
            if [ "$__jsv_found" = "true" ]; then
               echo "$__jsv_j"
            else
               if [ "$__jsv_j" = "$__jsv_sub_name" ]; then
                  __jsv_found="true"
               fi
            fi
            IFS="="
         done
         IFS="$__jsv_saved_ifs"
         if [ "$__jsv_found" = "true" ]; then
            break;
         fi
         IFS=","
      done
      IFS="$__jsv_saved_ifs"
   fi
   unset __jsv_list
   unset __jsv_command
   unset __jsv_name
   unset __jsv_sub_name
   unset __jsv_param_name
   return
}

###### jsv/jsv_sub_add_param() ################################################
#  NAME
#     jsv_sub_add_param() -- Adds a new variable with a new value or 
#                            it modifies an existing variable that is 
#                            already in the sublist.
#
#  SYNOPSIS
#     jsv_sub_add_param(param_name, variable_name, variable_value) 
#
#  FUNCTION
#     Some job parameters are list that can contain multiple variables 
#     with an optional value. 
#
#     This function either adds a new variable with a new value or it
#     modifies the value if the variable is already in the list parameter.
#     "variable_value" is optional. In that case, the variable has no value.
#
#  INPUTS
#     param_name - Find a list of list parameter names in the
#                  documentation for the function jsv_sub_is_param().
#
#     variable_name - Find more information in the documentation
#                     section for jsv_sub_is_param().
#        
#  RESULT
#     none
#
#  EXAMPLE
#
#     Assume that the following job is submitted:
#
#        qsub -ac a=1,d=7 -l h_vmem=5G,lic=1 -M user@domain script.sh 
#
#     and the following commands in jsv_on_verify():
#
#        jsv_sub_add_param("ac", "b", "");
#        jsv_sub_add_param("ac", "c", "5");
#        jsv_sub_add_param("ac", "d", "99");
#        jsv_sub_add_param("l_hard", "lic", "2");
#        jsv_sub_add_param("M", "user2@domain", "");
#        jsv_correct();
#
#     In the above case, the job will be modified as if the following qsub 
#     command was used.
#
#        qsub -ac a=1,d=99,b,c=5 -l h_vmem=5G,lic=2 -M user@domain,user2@domain script.sh
#
#  SEE ALSO
#     jsv/jsv_show_params() 
#     jsv/jsv_sub_is_param()
#     jsv/jsv_sub_get_param()
#     jsv/jsv_sub_add_param()
#     jsv/jsv_on_verify()
################################################################################
jsv_sub_add_param() 
{
   __jsv_param_name="$1"
   __jsv_name="jsv_param_$__jsv_param_name"
   __jsv_sub_name="$2" 
   __jsv_value="$3"
   __jsv_command=`eval "echo \$\{$__jsv_name\:\-$__jsv_undef\}"`
   __jsv_list=`eval "echo $__jsv_command"`
   __jsv_new_param=""
   if [ "$__jsv_list" = "$__jsv_undef" ]; then
      __jsv_list=""
   fi
   __jsv_found="false"

   # split token between ',' character
   IFS=","
   for __jsv_i in $__jsv_list; do
      # split the first string before '=' character
      # This is the variable name and if it is the
      # one which should be deleted then set "found" 
      # to "true" 
      IFS="="
      for __jsv_j in $__jsv_i; do
         IFS="$__jsv_saved_ifs"
         if [ "$__jsv_j" = "$__jsv_sub_name" ]; then
            __jsv_found="true"
            if [ "$__jsv_value" = "" ]; then
               if [ "$__jsv_new_param" = "" ]; then
                  __jsv_new_param="$__jsv_j"
               else
                  __jsv_new_param="${__jsv_new_param},$__jsv_j"
               fi
            else
               if [ "$__jsv_new_param" = "" ]; then  
                  __jsv_new_param="${__jsv_j}=${__jsv_value}"
               else
                  __jsv_new_param="${__jsv_new_param},${__jsv_j}=${__jsv_value}"
               fi
            fi
         else
            if [ "$__jsv_new_param" = "" ]; then  
               __jsv_new_param="$__jsv_i"
            else
               __jsv_new_param="${__jsv_new_param},$__jsv_i"
            fi
         fi
         IFS="="
         break;
      done
      IFS="$__jsv_saved_ifs"
   done
   if [ "$__jsv_found" = "false" ]; then
      if [ "$__jsv_value" = "" ]; then
         if [ "$__jsv_new_param" = "" ]; then  
            __jsv_new_param="$__jsv_sub_name"
         else
            __jsv_new_param="${__jsv_new_param},$__jsv_sub_name"
         fi
      else 
         if [ "$__jsv_new_param" = "" ]; then  
            __jsv_new_param="${__jsv_sub_name}=$__jsv_value"
         else
            __jsv_new_param="${__jsv_new_param},${__jsv_sub_name}=$__jsv_value"
         fi
      fi
   fi
   IFS="$__jsv_saved_ifs"

   # set local variable and send modification to client/master
   eval "$__jsv_name=\"\$__jsv_new_param\""
   _jsv_send_command PARAM "$__jsv_param_name" "$__jsv_new_param"
   unset __jsv_new_param
   unset __jsv_list
   unset __jsv_command
   unset __jsv_name
   unset __jsv_sub_name
   unset __jsv_param_name
   return
}

###### jsv/jsv_send_env() #####################################################
#  NAME
#     jsv_send_env() -- Requests the job environment for the next job 
#
#  SYNOPSIS
#     jsv_send_env()
#
#  FUNCTION
#     This function can only be used in jsv_on_start(). If it is used
#     there, then the job environment information will be available 
#     in jsv_on_verify() for the next job that is scheduled to be 
#     verified.
#
#     This function must be called for the functions jsv_show_envs(), 
#     jsv_is_env(), jsv_get_env(), jsv_add_env() and jsv_mod_env() to
#     behave correctly. 
#
#     Job environments might become very big (10K and more). This
#     will slow down the executing component (submit client or
#     master daemon thread). For this reason, job environment information 
#     is not passed to JSV scripts by default.
#
#     Please note also that the data in the job environment can't be
#     verified by Grid Engine and might therefore contain data which
#     could be misinterpreted in the script environment
#     and cause security issues. 
#     
#  INPUTS
#     none
#        
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_on_start()
#     jsv/jsv_on_verify()
################################################################################
jsv_send_env()
{
   _jsv_send_command "SEND ENV"
}

###### jsv/jsv_accept() ########################################################
#  NAME
#     jsv_accept() -- Accepts a job as it was initially provided.
#
#  SYNOPSIS
#     jsv_accept(message)
#
#  FUNCTION
#     This function can only be used in jsv_on_verify(). After it has been
#     called, the function jsv_on_verify() has to return immediately. 
#
#     A call to this function indicates that the job that is 
#     currently being verified should be accepted as it was initially 
#     provided. All job  modifications that might have been applied 
#     in jsv_on_verify() before this function was called, are then ignored.
#
#     Instead of calling jsv_accept() in jsv_on_verify() also the
#     functions jsv_correct(), jsv_reject() or jsv_reject_wait() can
#     be called, but only one of these functions can be used at a time.
#     
#  INPUTS
#     message - (optional) if it is provided it will be ignored.
#        
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_on_verify()
#     jsv/jsv_correct()
#     jsv/jsv_reject()
#     jsv/jsv_reject_wait()
################################################################################
jsv_accept()
{
   if [ "$__jsv_state" = "verifying" ]; then
      _jsv_send_command "RESULT STATE ACCEPT $*"
      __jsv_state="initialized"
   else
      _jsv_send_command "ERROR JSV script will send RESULT command but is in state $__jsv_state"
   fi
}

###### jsv/jsv_correct() #######################################################
#  NAME
#     jsv_correct() -- Accepts a job after applying all modifications.
#
#  SYNOPSIS
#     jsv_correct(message)
#
#  FUNCTION
#     This function can only be used in jsv_on_verify(). After it has been
#     called, the function jsv_on_verify() has to return immediately. 
#
#     A call to this function indicates that the job that is currently being 
#     verified has to be modified before it can be accepted. All job parameter 
#     modifications that were previously applied will be committed
#     and the job will be accepted. "Accept" in that case means that
#     the job will either be passed to the next JSV instance for
#     modification or that it is passed to that component in the master 
#     daemon that adds it to the master data store when the
#     last JSV instance has verified the job.
#     
#     Instead of calling jsv_correct() in jsv_on_verify(), the
#     functions jsv_accept(), jsv_reject() or jsv_reject_wait() can
#     be called, but only one of these functions can be used.
#     
#  INPUTS
#     message - (optional) if it is provided it will be ignored.
#        
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_on_verify()
#     jsv/jsv_accept()
#     jsv/jsv_correct()
#     jsv/jsv_reject()
#     jsv/jsv_reject_wait()
################################################################################
jsv_correct()
{
   if [ "$__jsv_state" = "verifying" ]; then
      _jsv_send_command "RESULT STATE CORRECT $*"
      __jsv_state="initialized"
   else
      _jsv_send_command "ERROR JSV script will send RESULT command but is in state $__jsv_state"
   fi
}

###### jsv/jsv_reject() #######################################################
#  NAME
#     jsv_reject() -- Rejects a job. 
#
#  SYNOPSIS
#     jsv_reject(message)
#
#  FUNCTION
#     This function can only be used in jsv_on_verify(). After it has been
#     called the function jsv_on_verify() has to return immediately. 
#
#     The job that is currently being verified will be rejected. "message"
#     will be passed to the client application that tried to submit
#     the job. Command line clients like qsub will print that message
#     to stdout to inform the user that the submission has failed.
#
#     jsv_reject_wait() should be called if the user may try to submit
#     the job again. jsv_reject_wait() indicates the verification process
#     might be successful in the future.
#
#     Instead of calling jsv_reject() in jsv_on_verify() also the
#     functions jsv_accept(), jsv_correct() or jsv_reject_wait() can
#     be also called, but only one of these functions can be used.
#     
#  INPUTS
#     message - error message passed to the submitter of the job.
#        
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_on_verify()
#     jsv/jsv_accept()
#     jsv/jsv_correct()
#     jsv/jsv_reject_wait()
################################################################################
jsv_reject()
{
   if [ "$__jsv_state" = "verifying" ]; then
      _jsv_send_command "RESULT STATE REJECT $*"
      __jsv_state="initialized"
   else
      _jsv_send_command "ERROR JSV script will send RESULT command but is in state $__jsv_state"
   fi
}

###### jsv/jsv_reject_wait() ##################################################
#  NAME
#     jsv_reject_wait() -- Reject a job. The job might be submitted later.
#
#  SYNOPSIS
#     jsv_reject_wait(message)
#
#  FUNCTION
#     This function can only be used in jsv_on_verify(). After it has been
#     called the function jsv_on_verify() has to return immediately. 
#
#     The job which is currently verified will be rejected. "message"
#     will be passed to the client application, that tries to submit
#     the job. Command line clients like qsub will print that message
#     to stdout to inform the user that the submission has failed.
#
#     This function should be called if the user who tries to submit the 
#     job might have a chance to submit the job later. jsv_reject()
#     indicates that the verified job will also be rejected in future.
#
#     Instead of calling jsv_reject() in jsv_on_verify() the
#     functions jsv_accept(), jsv_correct() or jsv_reject() can 
#     be also called, but only one of these functions can be used.
#     
#  INPUTS
#     message - error message passed to the submitter of the job.
#        
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_on_verify()
#     jsv/jsv_accept()
#     jsv/jsv_correct()
#     jsv/jsv_reject()
################################################################################
jsv_reject_wait()
{
   if [ "$__jsv_state" = "verifying" ]; then
      _jsv_send_command "RESULT STATE REJECT_WAIT $*"
      __jsv_state="initialized"
   else
      _jsv_send_command "ERROR JSV script will send RESULT command but is in state $__jsv_state"
   fi
}

###### jsv/jsv_log_info() ####################################################
#  NAME
#     jsv_log_info() -- Sends an info message to client.
#
#  SYNOPSIS
#     jsv_log_info(message)
#
#  FUNCTION
#     This function sends an info message to the client or
#     master daemon instance that started the JSV script.
#
#     For client JSVs, this means that the command line client will get
#     the information and print it to the stdout stream. Server JSVs
#     will print that message as an info message to the master daemon
#     message file.
#     
#  INPUTS
#     message - string message 
#        
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_log_waring()
#     jsv/jsv_log_error()
################################################################################
jsv_log_info() 
{
   _jsv_send_command "LOG INFO $*"
}

###### jsv/jsv_log_warning() ###################################################
#  NAME
#     jsv_log_warning() -- Sends a warning message to the client.
#
#  SYNOPSIS
#     jsv_log_warning(message)
#
#  FUNCTION
#     This function sends a warning message to the client or
#     master daemon instance that started the JSV script.
#
#     For client JSVs, this means that the command line client will get
#     the information and print it to the stdout stream. Server JSVs
#     will print that message as a warning message to the master daemon
#     message file.
#     
#  INPUTS
#     message - string message 
#        
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_log_info()
#     jsv/jsv_log_error()
################################################################################
jsv_log_warning() 
{
   _jsv_send_command "LOG WARNING $*"
}

###### jsv/jsv_log_error() #####################################################
#  NAME
#     jsv_log_error() -- Sends error message to the client 
#
#  SYNOPSIS
#     jsv_log_error(message)
#
#  FUNCTION
#     This function sends an error message to the client or
#     master daemon instance that started the JSV script.
#
#     For client JSVs, this means that the command line client will get
#     the error message and print it to the stdout stream. Server JSVs
#     will print that message as an error message to the master daemon
#     message file.
#     
#  INPUTS
#     message - string message 
#        
#  RESULT
#     none
#
#  SEE ALSO
#     jsv/jsv_log_info()
#     jsv/jsv_log_waring()
################################################################################
jsv_log_error() 
{
   _jsv_send_command "LOG ERROR $*"
}


# private function 
_jsv_handle_start_command()
{
   if [ "$__jsv_state" = "initialized" ]; then
      jsv_on_start
      _jsv_send_command "STARTED"
      __jsv_state="started"
   else
      _jsv_send_command "ERROR JSV script got START command but is in state $__jsv_state"
   fi
}

# private function 
_jsv_handle_begin_command()
{
   if [ "$__jsv_state" = "started" ]; then
      __jsv_state="verifying"
      jsv_on_verify
      jsv_clear_params
      jsv_clear_envs
   else
      _jsv_send_command "ERROR JSV script got BEGIN command but is in state $__jsv_state"
   fi
}

# private function 
_jsv_handle_param_command()
{
   if [ "$__jsv_state" = "started" ]; then
      __jsv_param="$1"
      __jsv_value="$2"

      eval "jsv_param_$__jsv_param=\"\${__jsv_value}\""
      unset __jsv_param
      unset __jsv_value
   else
      _jsv_send_command "ERROR JSV script got PARAM command but is in state $__jsv_state"
   fi
}

# private function 
_jsv_handle_env_command()
{
   if [ "$__jsv_state" = "started" ]; then
      __jsv_action="$1"
      __jsv_name="$2"
      __jsv_data="$3"
      if [ "$__jsv_action" = "ADD" ]; then
         __jsv_all_envs="$__jsv_all_envs $__jsv_name"
         eval "jsv_env_${__jsv_name}=\"\${__jsv_data}\""
      fi
      unset __jsv_action
      unset __jsv_name
      unset __jsv_data
   else
      _jsv_send_command "ERROR JSV script got ENV command but is in state $__jsv_state"
   fi
}

# private function 
_jsv_send_command()
{
   echo "$@"

   _jsv_script_log "<<< $@"
}

# private function
# Whether or not echo does backslash-processing is implementation-defined.
# bash's built-in and GNU echo support -E to suppress escapes and -e
# to interpret them.
if [ "`echo -E '\n'`" = '\n' -a "`echo -e '\n'`" = '' ]; then
_jsv_echo_raw() {
    # Special case to avoid problems if passed -n as first arg
    if [ "x$1" = x-n ]; then
        "$SGE_ROOT/utilbin/$ARCH/echo_raw" "$@"
    else
	# If $1 is -n, rely on echo -E -e == echo -e
        echo -E "$@"
    fi
}
else
    # Can't rely on echo
_jsv_echo_raw() {
    "$SGE_ROOT/utilbin/$ARCH/echo_raw" "$@"
}
fi

# private function

# Simple read can't be used generally because it does backslash
# escaping, and read -r (raw mode) may not be available.  Therefore we
# may have to use our own implementation.
if [ -n "$BASH_VERSION" -o "$ARCH" = "darwin-x86" -o "$ARCH" = "darwin-x64" ]; then
__jsv_use_read=true
_jsv_read_raw() {
    read -r __jsv_first __jsv_second __jsv_remaining
    __jsv_input="$__jsv_first $__jsv_second $__jsv_remaining"
}
else
_jsv_read_raw() {
    __jsv_input=`"$SGE_ROOT/utilbin/$ARCH/read_raw"`
}
fi
if [ -n "$BASH_VERSION" ]; then
_jsv_echo_raw() {
    if [ "x$1" = x-n ]; then
        "$SGE_ROOT/utilbin/$ARCH/echo_raw" "$@"
    else
        echo -E "$@"
    fi
}
else
_jsv_echo_raw() {
    "$SGE_ROOT/utilbin/$ARCH/echo_raw" "$@"
}
fi

# private function
_jsv_script_log()
{
   if [ $__jsv_logging_enabled = true -o $jsv_logging_enabled = true ]; then
      echo "$@" >>$__jsv_logfile
   fi
}

###### jsv/jsv_main() #########################################################
#  NAME
#     jsv_main() -- JSV main function.
#
#  SYNOPSIS
#     jsv_main()
#
#  FUNCTION
#     This function has to be called in JSV scripts. It implements
#     the JSV protocol and performs the communication with client and server
#     components which might start JSV scripts.
#     
#     This function does not return immediately. It returns only when
#     the "QUIT" command is sent by the client or server component.
#
#     During the communication with client and server components, this
#     function triggers two callback functions for each job that 
#     should be verified. First jsv_on_start() and later on jsv_on_verify().
#
#     jsv_on_start() can be used to initialize certain things that might 
#     be needed for the verification process. jsv_on_verify() does the
#     verification process itself.
#
#     The function jsv_send_env() can be called in jsv_on_start() so that
#     the job environment is available in jsv_on_verify(). 
#
#     The following function can only be used in jsv_on_verify().
#     Simple job parameters can be accessed/modified with:
#
#        jsv_is_param()
#        jsv_get_param()
#        jsv_del_param()
#
#     List based job parameters can be accessed with:
#
#        jsv_sub_is_param()
#        jsv_sub_get_param()
#        jsv_sub_add_param()
#        jsv_sub_del_param()
#
#     If the environment was requested with jsv_send_env() in jsv_on_start() 
#     then the environment can be accessed/modified with the following
#     commands:
#
#        jsv_is_env()
#        jsv_get_env()
#        jsv_add_env()
#        jsv_mod_env()
#        jsv_del_env()
#
#     Jobs can be accepted/rejected with the following:
#
#        jsv_accept()
#        jsv_correct()
#        jsv_reject()
#        jsv_reject_wait()
#
#     The following functions send messages to the calling component of a JSV
#     that will either appear on the stdout stream of the client or in the
#     master message file. This is especially useful when new JSV scripts 
#     should be tested
#
#        jsv_show_params() 
#        jsv_show_envs() 
#        jsv_log_info()
#        jsv_log_warning()
#        jsv_log_error()
#     
#  INPUTS
#     none
#        
#  RESULT
#     none
#
#  SEE ALSO
################################################################################
jsv_main()
{
   _jsv_script_log "$0 started on `date`"
   _jsv_script_log ""
   _jsv_script_log "This file contains logging output from an SGE JSV script. Lines beginning"
   _jsv_script_log "with >>> contain the data which was send by a command line client or"
   _jsv_script_log "sge_qmaster to the JSV script. Lines beginning with <<< contain data"
   _jsv_script_log "which is sent for this JSV script to the client or sge_qmaster."
   _jsv_script_log ""

   while [ $__jsv_quit = false ]; do
      _jsv_read_raw
      __jsv_result=$?

      if [ $__jsv_result != 0 ]; then
         __jsv_quit=true
      else
         if [ "$__jsv_input" != "" ]; then
            _jsv_script_log ">>> $__jsv_input"
            if [ -n "$__jsv_use_read" ]; then
                __jsv_first=`_jsv_echo_raw $__jsv_input | cut -d' ' -f 1`
                __jsv_second=`_jsv_echo_raw $__jsv_input | cut -d' ' -f 2`
                __jsv_remaining=`_jsv_echo_raw $__jsv_input | cut -d' ' -f 3-`
            fi
            if [ "$__jsv_first" = "QUIT" ]; then
               __jsv_quit=true
            elif [ "$__jsv_first" = "PARAM" ]; then
               _jsv_handle_param_command "$__jsv_second" "$__jsv_remaining"
            elif [ "$__jsv_first" = "ENV" ]; then
               if [ -n "$BASH_VERSION" ]; then
                  read -r __jsv_third __jsv_data <<EOF
$__jsv_remaining
EOF
               else
                   __jsv_third=`_jsv_echo_raw $__jsv_input | cut -d' ' -f 3`
                   __jsv_len=`_jsv_echo_raw "$__jsv_first $__jsv_second $__jsv_third " | wc -c`
                   __jsv_data=`_jsv_echo_raw $__jsv_input | cut -c ${__jsv_len}-`
               fi
               _jsv_handle_env_command "$__jsv_second" "$__jsv_third" "$__jsv_data"
            elif [ "$__jsv_first" = "START" ]; then
               _jsv_handle_start_command
            elif [ "$__jsv_first" = "BEGIN" ]; then
               _jsv_handle_begin_command
            elif [ "$__jsv_first" = "SHOW" ]; then
               jsv_show_params
               jsv_show_envs
            else
               _jsv_send_command "ERROR JSV script got unknown command \"$__jsv_first\""
            fi
         fi
      fi
      unset __jsv_result
   done
   _jsv_script_log "$0 is terminating on `date`"
}

