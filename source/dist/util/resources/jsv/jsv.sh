#!/bin/bash
# Use bash rather than sh because it can largely avoid external commands,
# and so is faster.
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
# example for a job verification script 
#
# Be careful:  Job verification scripts are started with sgeadmin 
#              permissions if they are executed within the master 
#              process (server JSV) otherwise they will be started
#              as user which submitted the job.
#

PATH=/bin:/usr/bin

####### JSV/jsv_on_start() ###################################################
#  NAME
#     jsv_on_start() -- initiates a job verification for one job
#
#  SYNOPSIS
#     jsv_on_start()
#
#  FUNCTION
#     This callback function is triggered when a new job should be verified
#     by this script. Here it is possible to request additional information
#     which will then be available during the job verification process
#     itself. (see jsv_send_env)
#
#  INPUTS
#     None
#
#  RESULT
#     None 
#
#  NOTES
#     By using jsv_send_env call the JSV scripts requests the 
#     job environment for a job. The data contained in this
#     environment is not verified by Grid Engine and might 
#     therefore contain data which could cause issues in JSV 
#     scripts. Be careful when you interpret or otherwise use
#     that information.
# 
#     In general it is recommended NOT to request the job
#     environment in server JSVs due to performance reason. 
#
#  SEE ALSO
#     JSV/jsv_send_env
#     JSV/jsv_on_verify
##############################################################################
jsv_on_start()
{
#   jsv_send_env
   return
}

####### JSV/jsv_on_verify() ##################################################
#  NAME
#     jsv_on_verify() -- initiates a job verification for one job
#
#  SYNOPSIS
#     jsv_on_verify()
#
#  FUNCTION
#     Callback function which is triggered when a job should be verified.
#     Job specification and optionally job environment are available by
#     calling special functions:
#
#        jsv_is_param
#        jsv_get_param
#        jsv_set_param
#        jsv_del_param
#
#        jsv_sub_is_param
#        jsv_sub_add_param
#        jsv_sub_del_param
#        jsv_sub_get_param
#
#        jsv_is_env
#        jsv_add_env
#        jsv_del_env
#        jsv_get_env
#        jsv_mod_env
#
#     The evaluation process has to be terminated by a call of one of these
#     functions which will then either accept, correct or reject a job:
#
#        jsv_correct <Message> 
#        jsv_accept <Message>
#        jsv_reject_wait <Message>
#        jsv_reject <Message>
#
#     During the verification process following functions can be used to
#     add logging output to the master message file or stdout of a submit
#     client:
#
#        jsv_log_info <Message> 
#        jsv_log_warning <Message>
#        jsv_log_error <Message> 
#        jsv_show_params
#        jsv_show_envs
#
#  INPUTS
#     None
#
#  RESULT
#     None 
#
#  NOTES
#
#  SEE ALSO
#     JSV/jsv_send_env
#     JSV/jsv_on_start
##############################################################################
jsv_on_verify()
{
   # EXAMPLE:
   #     - all binary jobs will be rejected
   #     - pe jobs should use a multiple of 16 slots otherwise they 
   #       are rejected
   #     - "h_vmem" is not allowed in hard resource requests.
   #       It will be removed but later on the job will be rejected with
   #       reject_wait.
   #     - "h_data" is not allowed in hard resource requests.
   #       It will be removed but job will be accepted with the correction. 
   #     - add/modify a=1 to the job context of each job
   #     - print the result to stdout of the client but not into the
   #       message file of the master if this script is a master side 
   #       script
   do_correct="false" 
   do_wait="false"
   if [ "`jsv_get_param b`" = "y" ]; then
      jsv_reject "Binary job is rejected."
      return
   fi 

   if [ "`jsv_get_param pe_name`" != "" ]; then
      slots=`jsv_get_param pe_min`
      i=`echo "$slots % 16" | bc`

      if [ $i -gt 0 ]; then
         jsv_reject "Parallel job does not request a multiple of 16 slots"
         return
      fi
   fi

   l_hard=`jsv_get_param l_hard`
   if [ "$l_hard" != "" ]; then
      context=`jsv_get_param CONTEXT` 
      has_h_vmem=`jsv_sub_is_param l_hard h_vmem`
      has_h_data=`jsv_sub_is_param l_hard h_data`

   
      if [ "$has_h_vmem" = "true" ]; then
         jsv_sub_del_param l_hard h_vmem
         do_wait="true"
         if [ "$context" = "client" ]; then
            jsv_log_info "h_vmem as hard resource requirement has been deleted"
         fi 
      fi
      if [ "$has_h_data" = "true" ]; then
         jsv_sub_del_param l_hard h_data
         do_correct="true"
         if [ "$context" = "client" ]; then
            jsv_log_info "h_data as hard resource requirement has been deleted"
         fi 
      fi
   fi

   ac=`jsv_get_param ac`
   if [ "$ac" != "" ]; then
      has_ac_a=`jsv_sub_is_param ac a`
      has_ac_b=`jsv_sub_is_param ac b`

      if [ "$has_ac_a" = "true" ]; then
         ac_a_value=`jsv_sub_get_param ac a`
         new_value=`echo "$ac_a_value + 1" | bc`

         jsv_sub_add_param ac a $new_value
      else
         jsv_sub_add_param ac a 1
      fi
      if [ "$has_ac_b" = "true" ]; then
         jsv_sub_del_param ac b 
      fi
      jsv_sub_add_param ac c
   fi

   if [ "$do_wait" = "true" ]; then
      jsv_reject_wait "Job is rejected. It might be submitted later."
   elif [ "$do_correct" = "true" ]; then
      jsv_correct "Job was modified before it was accepted"
   else
      jsv_accept "Job is accepted"
   fi
   return
}

. ${SGE_ROOT}/util/resources/jsv/jsv_include.sh

# main routine handling the protocol between client/master and JSV script
jsv_main

