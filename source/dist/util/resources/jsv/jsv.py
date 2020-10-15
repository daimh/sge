#!/usr/bin/python
# JSV example for Python, translated from jsv.sh (so inherits the licence)

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

from os import getenv
import sys

jsvpath = "{0}/{1}/util/resources/jsv".format(getenv ("SGE_ROOT"),
                                              (getenv ("SGE_CELL", "default")))
sys.path.append (jsvpath)

from JSV import *

def jsv_on_start ():
    pass

def jsv_on_verify ():
    do_correct = False
    do_wait = False
    if jsv_get_param ("b") == "y":
        jsv_reject ("Binary job is rejected.")
        return
    if jsv_get_param ("pe_name"):
        slots = jsv_get_param ("pe_min")
        i = slots % 16
        if i > 0:
            jsv_reject ("Parallel job does not request a multiple of 16 slots")
            return
    l_hard = jsv_get_param ("l_hard")
    if l_hard:
        client = jsv_get_param ("CONTEXT") == "client"
        has_h_vmem = jsv_sub_is_param ("l_hard", "h_vmem")
        has_h_data = jsv_sub_is_param ("l_hard", "h_data")
        if has_h_vmem:
            jsv_sub_del_param ("l_hard", "h_vmem")
            do_wait = True
            if client:
                jsv_log_info ("h_vmem as hard resource requirement has been deleted")
        if has_h_data:
            jsv_sub_del_param ("l_hard", "h_data")
            do_correct = True
            if client:
                jsv_log_info ("h_data as hard resource requirement has been deleted")
    ac = jsv_get_param ("ac")
    if ac:
        has_ac_a = jsv_sub_is_param ("ac", "a")
        has_ac_b = jsv_sub_is_param ("ac", "b")
        if has_ac_a:
            ac_a_value = jsv_sub_get_param ("ac", "a")
            jsv_sub_add_param ("ac", "a", str (int (ac_a_value) + 1))
        else:
            jsv_sub_add_param ("ac", "a", str (1))
        if has_ac_b:
            jsv_sub_del_param ("ac", "b")
        jsv_sub_add_param ("ac", "c")
    if do_wait:
        jsv_reject_wait ("Job is rejected. It might be submitted later.")
    elif do_correct:
        jsv_correct ("Job was modified before it was accepted")
    else:
        jsv_accept ("Job is accepted")
    return

jsv_main ()
