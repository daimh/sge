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

set logging_enabled "false"
set logfile "/tmp/jsv_[pid].log"

# Current state of the script
# Might be "initialized", "started" or "verifying"
set state "initialized"

set jsv_cli_params [list a ar A b ckpt cwd display dl e h hold_jid \
                         hold_jid_ad i j js m M masterq notify N \
                         o P p R r shell S tc w]

set jsv_mod_params [list ac l_hard l_soft q_hard q_soft pe_min pe_max pe_name \
                         binding_strategy binding_type binding_amount binding_socket \
                         binding_core binding_step binding_exp_n \
                         c_interval c_occasion t_min t_max t_step]

set jsv_add_params [list CLIENT CONTEXT GROUP VERSION JOB_ID CMDNAME CMDARGS USER]

set jsv_all_params [concat $jsv_cli_params $jsv_mod_params $jsv_add_params]

set jsv_all_envs {}

set quit "false"

proc jsv_clear_params {} {
   global jsv_all_params

   foreach suffix $jsv_all_params {
      set name "jsv_param_$suffix"
      set exists [llength [info globals $name]]
      
      if {$exists == 1} {
         global $name
         unset $name
      }
   }
}

proc jsv_clear_envs {} {
   global jsv_all_envs

   foreach suffix $jsv_all_envs {
      set name "jsv_env_$suffix"
      set exists [llength [info globals $name]]

      if {$exists == 1} {
         global $name
         unset $name
      }
   }
}

proc jsv_show_params {} {
   global jsv_all_params

   foreach suffix $jsv_all_params {
      set name "jsv_param_$suffix"
      set exists [llength [info globals $name]]
      
      if {$exists == 1} {
         global $name
         set value [set $name]
         puts "LOG INFO got param $name=$value"
      }
   }

}

proc jsv_show_envs {} {
   global jsv_all_envs

   foreach suffix $jsv_all_envs {
      set name "jsv_env_$suffix"
      set exists [llength [info globals $name]]
      
      if {$exists == 1} {
         global $name
         set value [set $name]
         puts "LOG INFO got env $name=$value"
      }
   }
} 

proc jsv_is_env {suffix} {
   set name "jsv_env_$suffix"
   set exists [llength [info globals $name]]
   set ret 0 

   if {$exists == 1} {
      global $name
      set ret 1 
   }
   return $ret
}

proc jsv_get_env {suffix} {
   set name "jsv_env_$suffix"
   set exists [llength [info globals $name]]
   set ret {}

   if {$exists == 1} {
      global $name
      eval "set ret \$$name"
   }
   return $ret
}

proc jsv_add_env {suffix value} {
   set name "jsv_env_$suffix"
   set exists [llength [info globals $name]]
   set ret {}

   global $name
   set $name $value
   _jsv_send_command "ENV ADD $suffix $value"
}

proc jsv_mod_env {suffix value} {
   set name "jsv_env_$suffix"
   set exists [llength [info globals $name]]
   set ret {}

   global $name
   set $name $value
   _jsv_send_command "ENV MOD $suffix $value"
}

proc jsv_del_env {suffix} {
   set name "jsv_env_$suffix"
   set exists [llength [info globals $name]]
   set ret {}

   if {$exists == 1} {
      global $name
      unset $name 
      _jsv_send_command "ENV DEL $suffix"
   }
}

proc jsv_is_param {suffix} {
   set name "jsv_param_$suffix"
   set exists [llength [info globals $name]]
   set ret 0 

   if {$exists == 1} {
      global $name
      set ret 1 
   }
   return $ret
}

proc jsv_get_param {suffix} {
   set name "jsv_param_$suffix"
   set exists [llength [info globals $name]]
   set ret {}

   if {$exists == 1} {
      global $name
      set ret [set $name]
   }
   return $ret
}

proc jsv_set_param {suffix value} {
   set name "jsv_param_$suffix"
   global $name
   set $name $value
   _jsv_send_command "PARAM $suffix $value"
}

proc jsv_del_param {suffix} {
   set name "jsv_param_$suffix"
   set exists [llength [info globals $name]]
   set ret {}

   if {$exists == 1} {
      global $name
      unset $name 
      _jsv_send_command "PARAM $suffix"
   }
}

proc jsv_sub_is_param {suffix sub_param} {
   set ret 0

   if {[jsv_is_param $suffix] == 1} {
      set token_list [split [jsv_get_param $suffix] ","]

      foreach token $token_list {
         set sub_token [split $token "="]

         if {[string compare $sub_param [lindex $sub_token 0]] == 0} {
            set ret 1
            break
         }
      }
   } 
   return $ret
}

proc jsv_sub_get_param {suffix sub_param} {
   set ret {}

   if {[jsv_is_param $suffix] == 1} {
      set token_list [split [jsv_get_param $suffix] ","]

      foreach token $token_list {
         set sub_token [split $token "="]

         if {[string compare $sub_param [lindex $sub_token 0]] == 0} {
            set sub_token [lrange $sub_token 1 [llength $sub_token]]
            set value [join $sub_token "="]

            set ret $value
            break
         }
      }
   }
   return $ret
}

proc jsv_sub_add_param {suffix sub_param {sub_value ""}} {
   set name "jsv_param_$suffix"

   global $name
   set exists [llength [info globals $name]]
   if {$exists == 1} {
      set token_list [split [set $name] ","]
   } else {
      set token_list {} 
   }
   set new_token_list {}
   set new_sub_value {}
   set found 0

   # check all subelements and replace the value if element already exists
   foreach token $token_list {
      set sub_token [split $token "="]
      set sub_var [lindex $sub_token 0]

      if {[string compare $sub_param $sub_var] == 0} {
         set found 1
         set new_sub_value $sub_value
      } else {
         set new_sub_value [join [lrange $sub_token 1 [llength $sub_token]] "="]
      }
      if {[string compare "" $new_token_list] == 0} {
         if {[string compare "" $new_sub_value] == 0} {
            set new_token_list "${sub_var}"
         } else {
            set new_token_list "${sub_var}=$new_sub_value"
         }
      } else {
         if {[string compare "" $new_sub_value] == 0} {
            set new_token_list "${new_token_list},${sub_var}"
         } else {
            set new_token_list "${new_token_list},${sub_var}=$new_sub_value"
         }
      }
   }

   # if element was not found and replaced then we can add it here
   if {$found == 0} {
      set sub_var $sub_param
      set new_sub_value $sub_value
      if {[string compare "" $new_token_list] == 0} {
         if {[string compare "" $new_sub_value] == 0} {
            set new_token_list "${sub_var}"
         } else {
            set new_token_list "${sub_var}=$new_sub_value"
         }
      } else {
         if {[string compare "" $new_sub_value] == 0} {
            set new_token_list "${new_token_list},${sub_var}"
         } else {
            set new_token_list "${new_token_list},${sub_var}=$new_sub_value"
         }
      }
       
   } 
   
   # set the new value
   set $name $new_token_list
   _jsv_send_command "PARAM $suffix $new_token_list"
}

proc jsv_sub_del_param {suffix sub_param} {
   set name "jsv_param_$suffix"

   global $name
   set exists [llength [info globals $name]]
   if {$exists == 1} {
      set token_list [split [set $name] ","]
   } else {
      set token_list {} 
   }
   set new_token_list {}
   set new_sub_value {}

   # check all subelements and replace the value if element already exists
   foreach token $token_list {
      set sub_token [split $token "="]
      set sub_var [lindex $sub_token 0]
      set found 0

      if {[string compare $sub_param $sub_var] == 0} {
         set found 1
      } else {
         set new_sub_value [join [lrange $sub_token 1 [llength $sub_token]] "="]
      }
      if {$found == 0} {
         if {[string compare "" $new_token_list] == 0} {
            if {[string compare "" $new_sub_value] == 0} {
               set new_token_list "${sub_var}"
            } else {
               set new_token_list "${sub_var}=$new_sub_value"
            }
         } else {
            if {[string compare "" $new_sub_value] == 0} {
               set new_token_list "${new_token_list},${sub_var}"
            } else {
               set new_token_list "${new_token_list},${sub_var}=$new_sub_value"
            }
         }
      }
   }
   # set the new value
   set $name $new_token_list
   _jsv_send_command "PARAM $suffix $new_token_list"
}

proc _jsv_handle_start_command {} {
   global state

   if {[string compare "initialized" $state] == 0} {
      jsv_on_start
      _jsv_send_command "STARTED"
      set state "started"
   } else {
      _jsv_send_command "ERROR JSV script got START command but is in state $state"
   }
}

proc _jsv_handle_begin_command {} {
   global state

   if {[string compare "started" $state] == 0} {
      set state "verifying"
      jsv_on_verify
      jsv_clear_params
      jsv_clear_envs
   } else {
      _jsv_send_command "ERROR JSV script got BEGIN command but is in state $state"
   }
}

proc _jsv_handle_param_command {param value} {
   global state

   if {[string compare "started" $state] == 0} {
      set variable jsv_param_$param

      global $variable 
      set $variable $value
   } else {
      _jsv_send_command "ERROR JSV script got PARAM command but is in state $state"
   }
}

proc _jsv_handle_env_command {action param value_list} {
   global jsv_all_envs
   global state

   if {[string compare "started" $state] == 0} {
      if {[string compare "ADD" $action] == 0} {
         set variable jsv_env_$param

         if {[lsearch -exact $jsv_all_envs $param] == -1} {
            lappend jsv_all_envs $param
         }
         global $variable
         set $variable [join $value_list " "]
      } 
   } else {
      _jsv_send_command "ERROR JSV script got ENV command but is in state $state"
   }
}

proc _jsv_send_command {args} {
   set command [lindex $args 0]

   puts $command
   flush stdout
   _jsv_script_log "<<< $command"
}

proc jsv_send_env {args} {
   _jsv_send_command "SEND ENV"
}

proc jsv_accept {args} {
   global state

   if {[string compare "verifying" $state] == 0} {
      set message [lindex $args 0]

      _jsv_send_command "RESULT STATE ACCEPT $message"
      set state "initialized"
   } else {
      _jsv_send_command "ERROR JSV script will send RESULT command but is in state $state"
   }
}

proc jsv_correct {args} {
   global state

   if {[string compare "verifying" $state] == 0} {
      set message [lindex $args 0]

      _jsv_send_command "RESULT STATE CORRECT $message"
      set state "initialized"
   } else {
      _jsv_send_command "ERROR JSV script will send RESULT command but is in state $state"
   }
}

proc jsv_reject {args} {
   global state

   if {[string compare "verifying" $state] == 0} {
      set message [lindex $args 0]

      _jsv_send_command "RESULT STATE REJECT $message"
      set state "initialized"
   } else {
      _jsv_send_command "ERROR JSV script will send RESULT command but is in state $state"
   }
}

proc jsv_reject_wait {args} {
   global state

   if {[string compare "verifying" $state] == 0} {
      set message [lindex $args 0]

      _jsv_send_command "RESULT STATE REJECT_WAIT $message"
      set state "initialized"
   } else {
      _jsv_send_command "ERROR JSV script will send RESULT command but is in state $state"
   }
}

proc _jsv_script_log {args} {
   global logging_enabled
   global logfile

   if {[string compare "true" $logging_enabled] == 0} {
      set fd [open $logfile "a+"] 

      puts $fd [lindex $args 0]
      close $fd
   }
}

proc jsv_log_info {args} {
   _jsv_send_command "LOG INFO $args"
}

proc jsv_log_warning {args} {
   _jsv_send_command "LOG WARNING $args"
}

proc jsv_log_error {args} {
   _jsv_send_command "LOG ERROR $args"
}

proc jsv_main {} {
   global quit
   global sge_arch
   global argv0

   set date_time [clock format [clock seconds]]
   _jsv_script_log "$argv0 started on $date_time"
   _jsv_script_log ""
   _jsv_script_log "This file contains logging output from an SGE JSV script. Lines beginning"
   _jsv_script_log "with >>> contain the data which was send by a command line client or"
   _jsv_script_log "sge_qmaster to the JSV script. Lines beginning with <<< contain data"
   _jsv_script_log "which is send from this JSV script to the client or sge_qmaster"
   _jsv_script_log ""


   # do as long as pipe is not closed and we get no quit command 
   while {[eof stdin] == 0 && [string compare "false" $quit] == 0} {
      gets stdin input

      if {[string compare "" $input] == 0} {
         # empty lines will be ignored
      } else {
         set arguments [split $input " "]
         set first [lindex $arguments 0] 
         set second [lindex $arguments 1] 
         set remaining [lrange $arguments 2 [llength $arguments]]
   
         _jsv_script_log ">>> $input"

         if {[string compare "QUIT" "$first"] == 0} {
            set quit "true"
         } elseif {[string compare "PARAM" $first] == 0} {
            _jsv_handle_param_command "$second" "$remaining"
         } elseif {[string compare "ENV" $first] == 0} {
            set third [lindex $remaining 0]
            set remaining [lrange $remaining 1 [llength $remaining]]
            _jsv_handle_env_command "$second" "$third" "$remaining"
         } elseif {[string compare "START" $first] == 0} {
            _jsv_handle_start_command
         } elseif {[string compare "BEGIN" $first] == 0} {
            _jsv_handle_begin_command
         } elseif {[string compare "SHOW" $first] == 0} {
            jsv_show_params
            jsv_show_envs
         } else {
            _jsv_send_command "ERROR JSV script got unknown command \"$first\""
         }
      }
   }

   set date_time [clock format [clock seconds]]
   _jsv_script_log "$argv0 is terminating on $date_time"
}
