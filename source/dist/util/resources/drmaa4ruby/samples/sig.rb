#!/usr/bin/ruby

#########################################################################
#
#  The Contents of this file are made available subject to the terms of
#  the Sun Industry Standards Source License Version 1.2
#
#  Sun Microsystems Inc., March, 2006
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
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2006 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#########################################################################

require 'drmaa'

if ARGV.length < 1
	puts "usage: sig.rb <job-path> <args>"
	exit 1
end

s = DRMAA::Session.new

t = DRMAA::JobTemplate.new
t.command = ARGV[0]
ARGV.shift
t.arg = ARGV
t.stdout = ":/dev/null"
t.join = true

job = s.run(t)

puts "job: " + job

info = s.wait(job)

if info.wifexited?
	puts info.job + " returned with " + info.wexitstatus.to_s
elsif info.wifsignaled?
	if info.wcoredump?
		puts info.job + " returned died from " + info.wtermsig.to_s + " (core dumped)"
	else
		puts info.job + " returned died from " + info.wtermsig.to_s 
	end
elsif info.wifaborted?
	puts "aborted: " + info.job
end
info.rusage.each_pair { |name,value| puts "usage " + name + " " + value }
