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
# TODO: 
# - provide means to restart entire flows with failed flowjobs be rerun only
# - support bulk jobs
# - allow DRMAA user hold be used despite user hold be used by flow itself
#########################################################################

require 'drmaa'


# ------------------------------------------------------------------------------------------
# Exceptions thrown during parsing stage

class ParsingFunction < ArgumentError ; end
class ParsingFormat < ArgumentError ; end


# ------------------------------------------------------------------------------------------
# The FlowFunction classes represent the entities found in the flowfile.

class FlowFunction
end
class JobsInParallel < FlowFunction
	attr_accessor :par
	def make(key, vars, depend, depth, select)
		do_it = select_func?(key, vars, select)
	
		all_jobs = Array.new
		@par.each { |sub| 
			name = sub[0]
			if do_it
				flowprint(depth, "PARALLEL: " + name)
			end
			new_vars = sub[1]
			sub_vars = vars.dup
			if ! new_vars.nil?
				new_vars.each_pair { |var,val| sub_vars[var] = val }
			end
			j = $flowfunction[name]
			if j.nil?
				raise ParsingFunction.new("#{key}(): flow function \"#{name}\" does not exit")
			end
			if do_it
				jobs = j.make(name, sub_vars, depend, depth+1, nil)
			else
				jobs = j.make(name, sub_vars, depend, depth+1, select)
			end
			if ! jobs.nil?
				all_jobs += jobs
			end
		}
		if all_jobs.size != 0
			return all_jobs
		else
			return nil
		end
	end
end

class JobsInSequence  < FlowFunction
	attr_accessor :seq
	def make(key, vars, depend, depth, select)
		do_it = select_func?(key, vars, select)
		first = true
		@seq.each { |sub| 
			name = sub[0]
			flowprint(depth, "SEQUENTIAL: " + name) if do_it
			new_vars = sub[1]
			sub_vars = vars.dup
			if ! new_vars.nil?
				new_vars.each_pair { |var,val| sub_vars[var] = val }
			end
			j = $flowfunction[name]
			if j.nil?
				raise ParsingFunction.new("#{key}: flow function \"#{name}\" does not exit")
			end
			if do_it
				depend = j.make(name, sub_vars, depend, depth+1, nil)
			else
				depend = j.make(name, sub_vars, depend, depth+1, select)
			end
		}
		return depend
	end
end

class RunnableJob < FlowFunction
	attr_accessor :attrs, :njobs
	def initialize
		@njobs = 0
	end

	def make(key, vars, depend, depth, select)
		@njobs += 1
		job_key = key + "#" + @njobs.to_s

		do_it = select_func?(key, vars, select)

		fj_attrs = Array.new
		@attrs.each_pair { |name,t|
			value = substitute(t, vars)
			fj_attrs.push([ name, value ])
		}
		if depend.nil?
			f = FlowJob.new(nil, fj_attrs)
			flowprint(depth, job_key + "(" + comma_vars(vars) + ")") if do_it
		else 
			f = FlowJob.new(depend.dup, fj_attrs)
			flowprint(depth, job_key + "(" + comma_vars(vars) + ") waiting for " + comma_jobs(f.depend, ", ")) if do_it
		end
		fj_attrs.each { |a| flowprint(depth+1, a[0] + "=\"" + a[1] + "\"") } if do_it
		f.presubproc(job_key)
		f.verify(job_key)

		if ! do_it
			$not_selected += 1
			return [ ]
		end

		$flowjob[job_key] = f
		return [ job_key ]
	end
end

def flowprint(depth, s)
   return if ! $parse_only
   (depth*3).times { putc " " } ; puts s
end

def comma_vars(vars)
s = ""
first = true
vars.each_pair { |var,val|
	if first == false
		s += ", " 
	else
		first = false
	end
	s += var + "=" + val
}
return s
end

def comma_jobs(jobs, sep = ",")
s = ""
first = true
jobs.each { |job|
	if first == false
		s += sep
	else
		first = false
	end
	s += job
}
return s
end

def substitute(s, vars)
vars.each_pair { |var,val|
	s = s.sub(var, val)
}
return s
end

# parses name1=value1,... into a Hash
# used both for params and attrs
def var_list(str)
	vars = Hash.new
	if ! str.nil?
		str.strip.scan(/[^,][^,]*/) { |vardef|
			n = vardef.strip.scan(/[^=][^=]*/)
			vars[n[0].strip] = n[1].strip
		}
	end
	return vars
end

# decide if a paricular flow call was selected as target
def select_func?(k1, vrs1, select)
	return true if select.nil?

	k2 = select[0]
	vrs2 = select[1]
	if k1 != k2 or vrs1.size < vrs2.size
		return false
	end

	vrs2.each_pair { |k,v|
		if ! vrs1.has_key?(k) or vrs1[k] != v
			return false
		end
	}

	return true
end

# return name of first function
def parse_flow(file)
	all = nil
	begin
		IO::foreach(file) { |line|
			case line
			when /^#/
				next
			else
				# crack line
				function = line.sub(/[ ]*=.*$/, "").strip
				val = line.sub(/^[^=]*=/, "").strip
				if all.nil?
					all = function
				end

				# runnable job
				if ! val.index("{").nil?
					r = RunnableJob.new
					jobdef = val.scan(/[^{}][^{}]*/)[0].strip
					r.attrs = var_list(jobdef)
					$flowfunction[function] = r

				# jobs in parallel
				elsif ! val.index("&").nil?
					p = JobsInParallel.new
					p.par = Array.new
					val.scan(/[^&][^&]*/) { |sub| p.par << parse_flowcall(sub) }
					$flowfunction[function] = p

				# jobs in sequence
				elsif ! val.index("|").nil?
					s = JobsInSequence.new
					s.seq = Array.new
					val.scan(/[^|][^|]*/) { |sub| s.seq << parse_flowcall(sub) }
					$flowfunction[function] = s

				else
					# parsing code possibly is not yet good enoug -- sorryh
					raise ParsingFormat.new("flow file may not have empty lines")
				end
			end
		}
	end
	return all
end

def parse_flowcall(s)
	jobdef = s.strip.scan(/[^()][^()]*/)
	key = jobdef[0].strip
	vars = var_list(jobdef[1])
	return [ key, vars ]
end


# ------------------------------------------------------------------------------------------
# At end of parsing stage there is one FlowJob for each job to be run.
# The FlowJob also keeps state information, dependency information and 
# job finish information.

class FlowJob 
	# configuration
	attr_accessor :attrs, :depend
	# state information
	attr_accessor :jobid, :info 
	def initialize(depend, attrs)
		@depend = depend
		@attrs = attrs
	end
	# -- verification
	def verify(key)
		cmd = false
		@attrs.each { |a|
			name = a[0]
			value = a[1]
			if value.index('$')
				raise ParsingFunction.new("#{key}: #{name}=#{value} contains \"$\"")
			end
			case name
			when "cmd"
				if value.index('/') == 0
					if ! File.executable?(value)
						raise ParsingFunction.new("#{key}: cmd=#{value} must be executable")
					end
				else
					if executable_cmd(value).nil?
						raise ParsingFunction.new("#{key}: could't find cmd=#{value} in CMDPATH")
					end
				end
				cmd = true
			when "join", "nomail"	
				true_or_false?(key, name, value)
			when "args", "name", "nat", "cat", "wd", "in", "out", "err", "join", "trans", "mail"
			else
				# bug: must use DRMAA.get_attribute_names() to detect use of invalid attributes
				raise ParsingFunction.new("#{key}: unknown attribute \"#{name}\"")
			end
		}
		if !cmd
			raise ParsingFunction.new("#{key}: missing mandatory attribute \"cmd\"")
		end
	end
	def presubproc(job_key)
		if defined? FlowRC.presubmit_proc
			FlowRC.presubmit_proc(job_key, @attrs)
		end
	end
	def executable_cmd(cmd)
		path = nil
		$CMDPATH.each { |p|
			if File.executable?(p + "/" + cmd)
				path = p + "/" + cmd
				break
			end
		}
		return path
	end
	def true_or_false?(key, name, value)
		case value
		when "0", "false", "no", "n"
			return false
		when "1", "true", "yes", "y"
			return true
		else
			raise ParsingFunction.new("#{key}: \"#{name}=#{value}\" is neither \"true\" nor \"false\"")
		end
	end

	def submit(key, predecessors)
		if $MAX_JOBS != 0 and $jobs_in_drm == $MAX_JOBS 
			return false
		end
		jt = DRMAA::JobTemplate.new

      # job defaults
		jt.name = key # flow job name
		if $flowdir.nil?
			jt.wd = $wd
			jt.stdout = ":/dev/null"
			jt.join = true
		else
			jt.wd = $flowdir
			jt.stdout = ":#{$flowdir}/#{key}.o"
			jt.stderr = ":#{$flowdir}/#{key}.e"
			jt.join = false
		end

		native = nil

		attrs.each { |a|
			name = a[0]
			value = a[1]
			case name
			when "cmd"
				if value.index("/") == 0 
					jt.command = value
				else
					jt.command = executable_cmd(value)
				end
			when "args"
				jt.arg = value.split(" ")
			when "env"
				jt.env = value.split(",")
			when "name"
				jt.name = value
			when "nat"
				native = value
			when "cat"	
				jt.category = value
			when "hold"	
				# careful! hold is used by flow itself
				# jt.hold = true_or_false?(key, name, value)
			when "wd"
				jt.wd = value
			when "in"	
				jt.stdin = value
			when "out"	
				jt.stdout = value
			when "err"	
				jt.stderr = value
			when "join"	
				jt.join = true_or_false?(key, name, value)
			when "trans"	
				jt.transfer = value

			when "mail"
				jt.mail = value.split(",")
			when "nomail"	
				jt.block_mail = true_or_false?(key, name, value)
			end
		}

		if ! predecessors.nil?
			if $drm_depend 
				if native.nil?
					jt.native = "-hold_jid " + predecessors
				else
					jt.native = native + " -hold_jid " + predecessors
				end
			else
				jt.hold = true
				jt.native = native unless native.nil?
			end
		else
			jt.native = native unless native.nil?
		end

		begin
			jobid = $session.run(jt)
			$already_submitted += 1
			$last_submission = Time.now
			@jobid = jobid
			if ! predecessors.nil?
				puts "#{key} " + jobid + " submitted depending on " + predecessors
			else
				puts "#{key} " + jobid + " submitted"
			end
		rescue DRMAA::DRMAATryLater
			STDERR.puts "... try later (#{key})"
			return false
		end
		$jobs_in_drm += 1
		return true
	end

	# true, if all predecessors done 
	def is_due? 
		return true if @depend.nil?

		self.depend.each { |key|
			info = $flowjob[key].info
			if info.nil? 
				return false # not yet finished
			end
			if ! info.wifexited? or info.wexitstatus != 0
				return false # failed
			end
		}

		return true
	end

	def can_submit 
		# now   --> [0, jobids]
		# later --> [1, nil]
		# never --> [2, nil]
		r = 0
		jobids = nil
		self.depend.each { |key|
			node = $flowjob[key]

			info = node.info
			if ! info.nil?
				if !info.wifexited? or info.wexitstatus != 0
					return [ 2, nil] # failed
				else
					next # done
				end
			end

			jobid = node.jobid
			if jobid.nil?
				r = 1 # predecessor not yet submitted
			else
				# collect jobids
				if jobids.nil?
					jobids = jobid
				else
					jobids += "," + jobid
				end
			end
		}
		if r == 1
			return [1,nil]
		else
			return [0,jobids]
		end
	end
end


# ------------------------------------------------------------------------------------------
# The functions below are used by main to run the workflow  and cause 
# successor jobs be submitted/released once they are due.

# Workflow optimization requires job be submitted in order
# pass (1): jobs without predecessors or with all predecessors run
# pass (2): jobs whose predecessors are submitted
# aims is as broad as possible flow submission.
def submit_jobs(flush)

	if $flowjob.size == $already_submitted or $terminate_session
		# STDERR.puts "all jobs submitted"
		return true # all submitted
	end

	if ! flush 
		if $last_submission != 0 and (Time.now - $last_submission) < $STREAMING_RETRY
			# puts "... retry not yet reached"
			return false # retry not yet reached
		end
	end

	# STDERR.puts "1st pass"
	$flowjob.each_pair { |key,fj|
		next if ! fj.jobid.nil? # already submitted
		next if ! fj.info.nil? # already finished

		# all predecessors done 
		next if ! fj.is_due? 

		if ! fj.submit(key, nil)
			return false # try again
		end

		if $terminate_program
			exit 1
		elsif $terminate_session
			terminate()
			return true
		end
	}

	begin
		# STDERR.puts "2nd pass"
		all_submitted = true

		$flowjob.each_pair { |key,fj|
			next if ! fj.jobid.nil? # already submitted
			next if ! fj.info.nil? # already finished

			# analyze predecessors
			status = fj.can_submit()
			if status[0] != 0 
				all_submitted = false if status[0] == 1 
				next
			end
			predecessors = status[1]

			if ! fj.submit(key, predecessors)
				return false # try again
			end

			if $terminate_program
				exit 1
			elsif $terminate_session
				terminate()
				return true
			end
		}
	end until all_submitted

	return true # all submitted
end

def reap_jobs

	$session.wait_each(1) { |info|

		# delete workflow upon user interrupt
		if $terminate_program
			exit 1
		elsif $terminate_session
			terminate()
		end

		# nothing happend
		if info.nil?
			submit_jobs(false)
			next
		end
		$jobs_in_drm -= 1

		# interpret job finish information
		if info.wifaborted? 
			failed = true
			happend = "aborted"
			caused = "terminated"
		elsif info.wifsignaled? 
			failed = true
			happend = "died from " + info.wtermsig 
			happend += " (core dump)" if info.wcoredump?  
			caused = "terminated"
		elsif info.wifexited?
			exit_status = info.wexitstatus
			if  exit_status != 0
				failed = true
				happend = "exited with " + exit_status.to_s
				caused = "terminated"
			else
				failed = false
				happend = "done"
				caused = "released"
			end
		end

		# search flow job
		job_key = nil
		fjob = nil
		$flowjob.each_pair { |k,v|
			if v.jobid.nil?
				next 
			end
			if v.jobid == info.job
				job_key = k
				fjob = v
				break
			end
		}
		if fjob.nil?
			puts "missing flow job for finished job " + info.job
			exit 1
		end

		# mark flow job as done
		fjob.info = info
		fjob.jobid = nil

		trigger = Array.new
		if ! $terminate_session
			# drive conclusions
			$flowjob.each_pair { |k,v|
				# finished and non-blocked ones: skip 
				next if ! v.info.nil? or v.depend.nil? or v.jobid.nil?
				# dependend to others: skip 
				next if ! v.depend.include?(job_key)
				
				if failed
					begin
						$session.terminate(v.jobid)
					rescue DRMAA::DRMAAInvalidJobError
					end
					trigger << v.jobid
				else
					do_rls = true
					v.depend.each { |k|
						do_rls = false if $flowjob[k].info.nil?
					}
					if do_rls and ! $drm_depend 
						$session.release(v.jobid)
						trigger << v.jobid
					end
				end
			}
		end

		# report what happend
		if trigger.size == 0
			puts "#{job_key} #{info.job} " + happend
		else
			puts "#{job_key} #{info.job} " + happend + " " + caused + " " + comma_jobs(trigger, ", ")
		end

		submit_jobs(false)
	}
end

# show final statistics
def final_report
	nfailed = 0
	nrun = 0
	nnotrun = 0

	rusage = Hash.new
	$flowjob.each_pair { |k,v|
		if v.info.nil?
			nnotrun += 1
			next 
		end
		if ! v.info.wifexited? or v.info.wexitstatus != 0
			nfailed += 1
		else
			nrun += 1
		end
		usage = v.info.rusage
		next if usage.nil?
		usage.each_pair { |name,value| 
			if $USAGE_REPORT.include?(name)
				if ! rusage.has_key?(name)
					rusage[name] = value.to_f
				else
					rusage[name] += value.to_f
				end
			end
		}
	}
	puts "# ---- final report"
	rusage.each_pair { |name,value|
		printf("usage: #{name} = %-7.2f\n", value)
	}
	puts "run: #{nrun} failed: #{nfailed} notrun: #{nnotrun}"
end

def terminate
	if ! $did_terminate
		STDERR.puts "Terminate!"
		$session.terminate
		$did_terminate = true
	end
end

def handle_signal
	if ! $terminate_session 
		$terminate_session = true
	elsif ! $terminate_program 
		$terminate_program = true
	end
end

def usage(ret)
	if ret == 0
		out =  STDOUT
	else
		out = STDERR
	end
	out.puts "usage: flow.rb [options] workflow.ff [start]"
	out.puts "  options: -verify         only parse and verify the flow"
	out.puts "           -dd             use DRM dependencies"
	out.puts "           -flowdir <path> flowdir is used as defaults"
	out.puts "  start:   <flowcall> -->  TEST or TEST($arch=solaris)"
	exit ret
end

# ------------------------------------------------------------------------------------------
#    main

# use defaults 
# (1) from ./.flowrc.rb 
# (2) from $HOME/.flowrc.rb 
# (3) or built-in ones

read_rc_file = false
if FileTest.exist?('.flowrc.rb')
	require '.flowrc'
	read_rc_file = true
elsif FileTest.exist?(ENV["HOME"] + "/.flowrc.rb")
	require ENV["HOME"] + "/.flowrc.rb"
	read_rc_file = true
end

if ! read_rc_file
	$CMDPATH = Dir::getwd()
	$STREAMING_RETRY = 5
	$USAGE_REPORT = [ ]
	$MAX_JOBS = 0
else
	$CMDPATH = FlowRC::CMDPATH
	$STREAMING_RETRY = FlowRC::STREAMING_RETRY
	$USAGE_REPORT = FlowRC::USAGE_REPORT
	$MAX_JOBS = FlowRC::MAX_JOBS
end

# The flowdir is used in a number of cases to have reasonable 
# defaults. Thus it makes some difference if flowdir was 
# specified or not:
# 
# wd (drmaa_wd)
#    The flowdir is used as jobs' default working directory.
#    Without flowdir the current working directory is simply 
#    used. Though each jobs' working directory can also be
#    specified within the flowfile, but if they have to that
#    would make them harder to read by humans.  
#
# out/err/join (drmaa- stdout_path/stderr_path/join)
#    Without flowdir "/dev/null" is used as default for 'out'
#    and 'join' is true. Reason is there were no better 
#    default to store job output/error files than the
#    current working directory, but if that were used
#    it might incidentally happen that masses of job 
#    output files are dumped in some directory. If flowdir
#    was specified at command line it is used as default
#    for storing job output and error separately in 
#    $flowdir/<flowjobname>.o and $flowdir/<flowjobname>.o.
#
# cmd (drmaa_remote_command)
# args (drmaa_argv)
# env (drmaa_env)


$parse_only = false
$drm_depend = false
$flowdir = nil

# command line parsing
while ARGV.length >= 2 do 
	case ARGV[0]
	when "-verify"
		$parse_only = true
		ARGV.shift
	when "-dd"
		$drm_depend = true
		ARGV.shift
	when "-flowdir"
		ARGV.shift
		usage(1) if $flowdir or ARGV.length < 2
		$flowdir = ARGV[0]
		ARGV.shift
	when "-h", "-help"
		usage 0
	else
		break
	end
end
if ARGV.length >= 1
	flowfile=ARGV.shift
	if ! FileTest.readable?(flowfile)
	 	STDERR.puts flowfile + " does not exit"
	 	exit 1
	 end
else 
	usage(1)
end
if ARGV.length == 1
	target = parse_flowcall(ARGV.shift)
end
usage(1) unless ARGV.length == 0

# flow parsing and verification
begin
	$wd = Dir::getwd

	$flowfunction = Hash.new
	all = parse_flow(flowfile)
	j = $flowfunction[all]

	$flowjob = Hash.new
	$not_selected = 0
	target = parse_flowcall(all) if target.nil?
	j.make(all, vars = Hash.new, nil, 0, target)
	if $flowjob.size == 0
		raise ParsingFormat.new("flow start \"#{target[0]}\" does not exist in #{flowfile}")
	end
	puts "---+ doing #{$flowjob.size} of #{$flowjob.size+$not_selected} jobs with #{target[0]} as flow target"

	STDOUT.flush
	exit 0 if $parse_only
rescue ParsingFunction => msg
	STDERR.puts "Error in " + msg
	exit 1
rescue ParsingFormat => msg
	STDERR.puts "Format error: " + msg
	exit 1
end

# run the workflow
t1 = Time.now
begin
	$terminate_session = $terminate_program = false
	trap("INT") { handle_signal }
	trap("TERM") { handle_signal }

	$session = DRMAA::Session.new
	# puts "# ----- submitting jobs" 
	$already_submitted = $last_submission = 0
	$jobs_in_drm = 0

	# May not stop reaping before all jobs 
	# are submitted in case of streaming.
	first = true
	begin
		all_reaped = false
		all_submitted = submit_jobs(true)
		if first
			# puts "# ----- reaping jobs" 
			first = false
		else
			if all_submitted
				all_reaped = true 
			else
				sleep $STREAMING_RETRY
			end
		end
		reap_jobs()
	end until all_reaped

rescue DRMAA::DRMAAException => msg
	puts msg
	exit 1
end

final_report()

t2 = Time.now
printf("total: %7.1f seconds\n", t2-t1)
exit 0
