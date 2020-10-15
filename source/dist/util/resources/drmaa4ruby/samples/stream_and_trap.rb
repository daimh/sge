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


$terminate_session = $terminate_program = false
def handle_signal
	if ! $terminate_session 
		$terminate_session = true
	elsif ! $terminate_program 
		$terminate_program = true
	end
end
trap("INT") { handle_signal }
trap("TERM") { handle_signal }

session = DRMAA::Session.new

# causes DRMAA::Session:run() and DRMAA::Session:run_bulk()
# to sleep and retry in case of DRMAA::DRMAATryAgain. 
# That way we implement job streaming.
session.retry = 5

t = Sleeper.new

jobs = Array.new
for i in 1 .. 20 do
	job = session.run(t)
	puts "job: " + job
	jobs << job
	break if $terminate_session
end
for i in 1 .. 10
	bulk = session.run_bulk(t, 1, 2)
	bulk.each { |job| puts "job: " + job }
	jobs += bulk
	break if $terminate_session
end

while jobs.size > 0 do
	info = session.wait_any(1)
	if ! info.nil?
		if ! info.wifexited?
			puts "failed: " + info.job
		else	
			puts info.job + " returned with " + info.wexitstatus.to_s
		end
		jobs.delete(info.job)
	else
		if $terminate_program
			jobs.clear
		elsif $terminate_session
			session.terminate
		end
	end
end

exit 0
