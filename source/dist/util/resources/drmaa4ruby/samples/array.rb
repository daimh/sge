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

class Sleeper < DRMAA::JobTemplate
	def initialize
		super
		self.command = "/bin/sleep"
		self.arg     = ["1"]
		self.stdout  = ":/dev/null"
		self.join    = true
	end
end

# Two array jobs are submitted. The second array task remains in
# hold until the predecessor task was run. Demonstrate possible 
# use of hold/release.

NTASKS = 30
session = DRMAA::Session.new

jt = Sleeper.new
pre = session.run_bulk(jt, 1, NTASKS, 1)
suc = session.run_bulk(jt, 1, NTASKS, 1)

h = Hash.new
for i in 0 .. NTASKS-1 do
	h[pre[i]] = suc[i] 
end

session.wait_each{ |info|
	job = info.job
	if h.has_key?(job) 
		session.release(h[job])
	end

	if info.wifexited?
		puts job + " returned with " + info.wexitstatus.to_s
		# info.rusage.each { |u| puts "usage " + u }
	elsif info.wifaborted?
		puts job + " aborted"
	elsif info.wifsignaled?
		puts job + " died from " + info.wtermsig
	end
}

exit 0
