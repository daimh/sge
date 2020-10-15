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
require 'thread'

class Sleeper < DRMAA::JobTemplate
	def initialize
		super
		self.command = "/bin/sleep"
		self.arg     = ["1"]
		self.stdout  = ":/dev/null"
		self.join    = true
	end
end

# Submit a bunch of jobs from a number of threads
# and wait for them in main thread.
#
# Note: Ruby threads are not identical with operating
# system threads.

version = DRMAA.version
drm = DRMAA.drm_system
puts "DRMAA #{drm} v #{version}"

session = DRMAA::Session.new
t = Sleeper.new

i = 0
while i<4 do
	Thread.start do 
		j = 0
		while j<20 do
			puts "job: " + session.run(t)
			j += 1
		end
	end
	i += 1
end

session.wait_each{ |info|
	job = info.job
	if ! info.wifexited?
		puts "failed: " + info.job
	else	
		puts info.job + " returned with " + info.wexitstatus.to_s
	end
}

exit 0
