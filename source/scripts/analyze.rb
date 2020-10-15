#!/usr/bin/env ruby

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
#
# This script is a developer utility for accounting(8) file ananlysis.
# It has been developed with ruby 1.8.1.
# Do analyze.rb -help for usage.
#########################################################################

$debug = true
def debug s
	if $debug
		STDERR.puts "... " + s
		STDERR.flush
	end
end

# ----- abstract classes -----


class Debitable
	attr_reader :jobs, :wallclock, :cpu, :maxvmem, :maxrss, :pending
	def initialize()
		@jobs = @wallclock = @cpu = @pending = @maxvmem = @maxrss = 0
	end
	def debit_record(r)
		@jobs      += 1
      @wallclock += r.wallclock
      @cpu       += r.cpu
      @maxvmem   += r.maxvmem
      @maxrss    += r.maxrss
      if r.rd == 0
              @pending   += r.sd - r.qd
      else
              @pending   += r.sd - r.rd
      end
	end
	def print_debitable(key, first, size, name)
		if first
			puts "###### Table with #{size} #{name}s ######"
			puts sprintf("|%-10.10s %7.7s | sum %9.9s %9.9s %9.9s %9.9s %9.9s | avg %9.9s %9.9s %9.9s %9.9s %9.9s | %11.11s |",
				name, "njobs", "pend", "runtime", "cpu", "maxvmem", "maxrss", "pend", "runtime", 
               "cpu", "maxvmem", "maxrss", "runtime/cpu") 
		end
		pavg = pending/jobs
		wavg = wallclock/jobs
		cavg = cpu/jobs
		vavg = maxvmem/jobs
		ravg = maxrss/jobs
		if cavg != 0
			cpu_util = wavg/cavg
		else
			cpu_util = 0
		end
		puts sprintf("|%-10.10s %7.7s |     %9.9s %9.9s %9.9s %9.9s %9.9s |     %9.9s %9.9s %9.9s %9.9s %9.9s | %11.11s |", 
				key, jobs.to_s, pending.to_s, wallclock.to_s, cpu.to_s, maxvmem.to_s, maxrss.to_s, 
						pavg.to_s, wavg.to_s, cavg.to_s, vavg.to_s, ravg.to_s, cpu_util.to_s)
	end
end

class PrintableHash < Hash
	def print_all(name)
		first = true
		self.keys.sort.each do |debitable|
			self[debitable].print_debitable(debitable, first, self.size, name)
			if first == true
				first = false
			end
		end
	end
end


# ----- concrete classes -----


class Record
	attr_reader :user, :wallclock, :qd, :sd, :ed, :rd, :cat, :project, :parallel, :cpu, :queue, :host, :maxrss, :maxvmem
	def pending_time ; @sd - @qd ; end
	def initialize(user, wallclock, qd, sd, ed, cat, project, cpu, queue, host, parallel, requeued, maxrss, maxvmem)
		@qd = qd
		@sd = sd
		@ed = ed
		@rd = requeued
		@user = user
		@project = project
		@queue = queue
		@parallel = parallel
		@host = host
		@cat = cat
		@cpu = cpu
		@wallclock = wallclock
		@maxrss = maxrss
		@maxvmem = maxvmem
	end
end

class RecordHash < PrintableHash

   def inscope?(submit, start, ende, first, last)
      if first != "first"
         if submit < first.to_i && start < first.to_i && ende < first.to_i
            return false
         end
      end
      if last != "last"
         if submit > last.to_i && start > last.to_i && ende > last.to_i
            return false
         end
      end
      return true
   end

	# ----- init data structures -----
	# jobid user wallclock submit start end cat project cpu  queue host taskid ru_maxrss  maxvmem
	#  5     3    13        8      9     10  39    31    36     0  1    35     16         42
   # should also use taskid 35 to deal with array jobs
	def initialize(path, first, last)
		f = open(path)
		begin
			f.each_line do |line|
				if ! line.index("#")
					s = line.split(':')
					job     = s[5].to_i
					task    = s[35].to_i
				   queue   = s[0]
				   host    = s[1]
					user    = s[3]
					submit  = s[8].to_i
					start   = s[9].to_i
				   ende    = s[10].to_i
					wc      = s[13].to_i
				   project = s[31]
				   cpu     = s[36].to_i
				   cat     = s[39]
				   maxrss  = s[16].to_i
				   maxvmem = s[42].to_i
				   parallel = s[33]
               # puts "PE = #{parallel}"

					# if submit == 0 || start == 0 || ende == 0
					if start == 0 || ende == 0
						STDERR.puts "invalid record " + job.to_s + " submitted " + submit.to_s + " started " + start.to_s + " ended " + ende.to_s + " wallclock " + wc.to_s
               elsif ! inscope?(submit, start, ende, first, last)
						STDERR.puts "out of scope record " + job.to_s + " submitted " + submit.to_s + " started " + start.to_s + " ended " + ende.to_s + " wallclock " + wc.to_s
               else
                  rd = 0
                  index = 1
                  while self.has_key?([job, task, index]) do
                     rd = self[[job, task, index]].ed
                     index += 1
                     # puts "requeued job " + job.to_s + " task #{task} pending #{start - rd} requeued #{rd} #{s[5]}"
						end
						self[[job, task, index]] = Record.new(user, wc, submit, start, ende, cat.strip, project, cpu, queue, host, parallel, rd, maxrss, maxvmem)
					end
				end
			end
	   ensure
			f.close
		end
		debug "read " + self.size.to_s + " records"
	end
	def print_all
		first = true
		self.keys.sort.each do |key|
			r = self[key]
			job = key[0]
			task = key[1]
			run = key[2]
			if first
				puts "###### Table with #{self.size} job runs ######"
				puts sprintf("%-8.8s %-5.5s %-2.2s %-7.7s %7.7s %9.9s %9.9s %9.9s %9.9s %-12.12s %-12.12s %-12.12s %s", 
					"job", "task", "n", "user", "pending", "wallclock", "cpu", "maxvmem", "maxrss", "submit", "start", "ended", "category")
				first = false
			end
			puts sprintf("%-8.8s %-5.5s %-2.2s %-7.7s %7.7s %9.9s %9.9s %9.9s %9.9s %-12.12s %-12.12s %-12.12s %s", 
					job.to_s, task.to_s, run.to_s, r.user, r.pending_time.to_s, r.wallclock.to_s, 
						r.cpu.to_s, r.maxvmem.to_s, r.maxrss.to_s,  r.qd.to_s, r.sd.to_s, r.ed.to_s,
					"\"" + r.cat + "\"")
		end
	end
end

# ----------

class Queue < Debitable 
end

class QueueHash < PrintableHash
	def initialize(record)
		record.each_value do |r|
			if ! self.has_key?(r.queue)
				self[r.queue] = Queue.new()
			end
			self[r.queue].debit_record(r)
		end
		debug "debug did queues"
	end
	def print_all
		super("queue")
	end
end

# ----------

class User < Debitable 
end

class UserHash < PrintableHash
	def initialize(record)
		record.each_value do |r|
			if ! self.has_key?(r.user)
				self[r.user] = User.new()
			end
			self[r.user].debit_record(r)
		end
		debug "debug did users"
	end
	def print_all
		super("user")
	end
end

# ----------

class Project < Debitable 
end

class ProjectHash < PrintableHash
	def initialize(record)
		record.each_value do |r|
			if ! self.has_key?(r.project)
				self[r.project] = Project.new()
			end
			self[r.project].debit_record(r)
		end
		debug "debug did project"
	end
	def print_all
		super("project")
	end
end

# ----------

class Host < Debitable 
end

class HostHash < PrintableHash
	def initialize(record)
		record.each_value do |r|
			if ! self.has_key?(r.host)
				self[r.host] = Host.new()
			end
			self[r.host].debit_record(r)
		end
		debug "debug did hosts"
	end
	def print_all
		super("host")
	end
end

# ----------

class Parallel < Debitable 
end

class ParallelHash < PrintableHash
	def initialize(record)
		record.each_value do |r|
			if ! self.has_key?(r.parallel)
				self[r.parallel] = Parallel.new()
			end
			self[r.parallel].debit_record(r)
		end
		debug "debug did parallel environments"
	end
	def print_all
		super("parallel environment")
	end
end

# ----------

class Category < Debitable 
end

class CategoryHash < PrintableHash
	def initialize(record)
		record.each_value do |r|
			if ! self.has_key?(r.cat)
				self[r.cat] = Category.new()
			end
			self[r.cat].debit_record(r)
		end
	end
	def print_all
		first = true
		self.keys.sort.each do |cat|
			c = self[cat]
			if first
				puts "###### Table with #{self.size} categories ######"
				puts sprintf("%-7.7s %9.9s %9.9s %9.9s %9.9s %s", "njobs", 
               "pending", "wallclock", "p/njobs", "w/njobs", "category")
				first = false
			end
			puts sprintf("%-7.7s %9.9s %9.9s %9.9s %9.9s %s", c.jobs.to_s, c.pending.to_s, 
               c.wallclock.to_s, (c.pending/c.jobs).to_s, (c.wallclock/c.jobs).to_s, cat)
		end
	end
end

# ---------- Timestep analyzer


class Timestep
	attr_reader :queued, :requeued, :started, :ended, :pcats, :rcats
	def initialize
		@queued = JobsList.new
		@requeued = JobsList.new
		@started = JobsList.new
		@ended = JobsList.new
		@pcats = Hash.new
		@rcats = Hash.new
	end
	def job_events
		need_space = false
		s = ""
		if ! @queued.empty?
			s = s + "submitted " + @queued.to_s 
			need_space=true
		end
		if ! @requeued.empty?
			if need_space
				s += " "
			end
			s = s + "requeued " + @requeued.to_s 
			need_space = true
		end
		if ! @started.empty?
			if need_space
				s += " "
			end
			s = s + "started " + @started.to_s 
			need_space = true
		end
		if ! @ended.empty?
			if need_space
				s += " "
			end
			s = s + "ended " + @ended.to_s 
			need_space = true
		end
		return s
	end

	def print_cats_per_ts(time)
		c_first = true

		self.rcats.each_pair do |cat,jobs|
			if c_first
				puts "###### Categories for timestep #{time} ######"
				puts sprintf("%-8.8s %-18.18s %s", "number", "status", "category")
				c_first = false
			end
			puts sprintf("%-8.8s %-18.18s %s", jobs.size.to_s, "running", "\"" + cat + "\"")
		end

		self.pcats.each_pair do |cat,jobs|
			if c_first
				puts "###### Categories for timestep #{time} ######"
				puts sprintf("%-8.8s %-18.18s %s", "number", "status", "category")
				c_first = false
			end
			puts sprintf("%-8.8s %-18.18s %s", jobs.size.to_s, "pending", "\"" + cat + "\"")
		end
		STDOUT.flush
	end

	def print_jobs_per_ts(time, rjobs, pjobs, record)
		j_first = true

		rjobs.sort.each do |job|
			if j_first
				puts "###### Jobs at timestep #{time} ######"
				puts sprintf("%-10.10s %-7.7s %-10.10s %7.7s %s", "job", "status", "user", "pending", "category")
				j_first = false
			end
			r = record[job]
			puts sprintf("%-10.10s %-7.7s %-10.10s %7.7s %s", key_to_s(job),
					"running", r.user, r.pending_time.to_s, "\"" + r.cat + "\"")
		end

		pjobs.sort.each do |job|
			if j_first
				puts "###### Jobs at timestep #{time} ######"
				puts sprintf("%-10.10s %-7.7s %-10.10s %7.7s %s", "job", "status", "user", "pending", "category")
				j_first = false
			end
			r = record[job]
			puts sprintf("%-10.10s %-7.7s %-10.10s %7.7s %s", key_to_s(job),
					"pending", r.user, r.pending_time.to_s, "\"" + r.cat + "\"")
		end
		STDOUT.flush
	end
end

class TimestepHash < PrintableHash
	attr_reader :first, :last

	class Event
		attr_reader :job, :time
		def initialize(job, time)
			@job = job
			@time = time
		end
	end

	def initialize(record)
		times = Array.new
		queued = Array.new
		requeued = Array.new
		started = Array.new
		ended = Array.new

		record.each_pair do |key, r|
			if key[2] == 1
				times.push(r.qd)
				queued.push(Event.new(key, r.qd))
			else
				times.push(r.rd)
				requeued.push(Event.new(key, r.rd))
			end
			times.push(r.sd)
			started.push(Event.new(key, r.sd))

			times.push(r.ed)
			ended.push(Event.new(key, r.ed))
		end

		times.compact!
		times.sort!
		times.uniq!

		@first = times[0]
		@last = times[times.size - 1]
		debug "first " + @first.to_s + " last " + @last.to_s

		i = 0
		nxt = 500
		times.each do |t|
			self[t] = Timestep.new

			i += 1
			if i == nxt
				debug "did #{i} of #{times.size} timesteps"
				nxt += 500
			end
		end

		queued.each do |q|
			self[q.time].queued.push(q.job)
		end
		debug "did #{queued.size} job queued events"
		requeued.each do |r|
			self[r.time].requeued.push(r.job)
		end
		debug "did #{requeued.size} job requeued events"
		started.each do |s|
			self[s.time].started.push(s.job)
		end
		debug "did #{started.size} job started events"
		ended.each do |e|
			self[e.time].ended.push(e.job)
		end
		debug "did #{ended.size} job ended events"

		debug "did timesteps"
	end
end

# -------------------

def key_to_s(key)
	if key[2] == 1
		run = ""
	else
		run = "#" + key[2].to_s
	end
	if key[1] == 0
		key[0].to_s + run
	else
		key[0].to_s + "." + key[1].to_s + run
	end
end
class JobsList < Array
	def to_s
		first = true
		s=""
		self.each do |key|
			if first
				s = key_to_s(key)
				first = false
			else
				s += ", " +  key_to_s(key)
			end
		end
		return s
	end
end

class Analyzer
	def initialize
		@do_records    = false
		@do_users      = false
		@do_hosts      = false
		@do_queues     = false
		@do_projects   = false
		@do_categories = false
		@do_timesteps  = false
		@do_cat_per_ts = false
		@do_job_per_ts = false
		@first_ts      = "first"
		@last_ts       = "last"
	end

	def read_records(path)
		@record = RecordHash.new(path, @first_ts, @last_ts)
	end

	# ----- option parsing and usage-----


	def parse_options
		if ARGV[0] == "-help"
			usage 0
		end
		while ARGV.length >= 2 do 
			opt = ARGV.shift
			case opt
			when "-r"
				@do_records = true
			when "-c"
				@do_categories = true
			when "-u"
				@do_users = true
			when "-p"
				@do_projects = true
			when "-par"
				@do_parallel = true
			when "-q"
				@do_queues = true
			when "-h"
				@do_hosts = true
			when "-ts"
				@do_timesteps = true
			when "-ts_c"
				@do_cat_per_ts = true
			when "-ts_j"
				@do_job_per_ts = true
			when "-t"
				usage(1) unless ARGV.length >= 2
				@first_ts = ARGV.shift
				@last_ts = ARGV.shift
			end
		end
		usage(1) unless ARGV.length == 1
	end

	def usage(ret)
		puts 'usage: analyze.rb <options> accounting_file'
		puts '        -help'
		puts '        -r                                records table'
		puts '        -u                                users table'
		puts '        -h                                hosts table'
		puts '        -par                              parallel environment table'
		puts '        -q                                queues table'
		puts '        -p                                projects table'
		puts '        -c                                categories table'
		puts '        -ts                               timesteps table'
		puts '        -ts_c                             categories per timestep'
		puts '        -ts_j                             jobs per timestep'
		puts '        -t "first"|<first> "last"|<last>  full analysis, but print these timesteps only'
		exit ret
	end

	def init_ts_categories
		pjobs = JobsList.new
		rjobs = JobsList.new

		i = 0
		nxt = 500
		@timestep.keys.sort.each do |t|
			e = @timestep[t]

			pjobs = pjobs + e.queued + e.requeued - e.started

			# no need to do all pending categories 
			if t >= @first && t <= @last then 
				pjobs.each do |job|
					cat = @record[job].cat
					if ! e.pcats.has_key?(cat) 
						e.pcats[cat] = Array.new
					end
					e.pcats[cat].push(job)
				end
			end

			rjobs = rjobs + e.started - e.requeued - e.ended
			rjobs.each do |job|
				cat = @record[job].cat
				if ! e.rcats.has_key?(cat) 
					e.rcats[cat] = Array.new
				end
				e.rcats[cat].push(job)
			end

			i += 1
			if i == nxt
				debug "did #{i} categories for #{@timestep.size} timesteps"
				nxt += 500
			end
		end
		debug "did categories per timestep"
	end

	# ----- report printing -----

	def print_timesteps(dotime, docat, dojob)
		t_first = true
		pjobs = JobsList.new
		rjobs = JobsList.new
		duration = @last - @first
		prev = 0
		@timestep.keys.sort.each do |time|
			step = @timestep[time]
			pjobs = pjobs + step.queued + step.requeued - step.started
			rjobs = rjobs + step.started - step.requeued - step.ended

			if time >= @first && time <= @last then
				# table per timestep
				if dotime
					if t_first
						puts "###### Table with #{@timestep.size} timesteps over #{duration} seconds) ######"
						puts sprintf("%-12.12s %6.6s %4.4s %6.6s %4.4s %5.5s %s", 
								"time", "npend", "pcat", "nrunn", "rcat", "delta", "what happend?")
						t_first = false
						delta = 0
					else
						delta = time - prev
					end
					puts sprintf("%-12.12s %6.6s %-4.4s %-6.6s %-4.4s %5.5s %s", 
							time.to_s, pjobs.size.to_s, step.pcats.size.to_s, 
							rjobs.size.to_s, step.rcats.size.to_s, delta.to_s, step.job_events())
					prev = time
				end

				# categories table per timestep
				if docat && @do_cat_per_ts
					step.print_cats_per_ts(time)
					if ! dotime
						puts
					end
				end

				# job table per timestep
				if dojob && @do_job_per_ts
					step.print_jobs_per_ts(time, rjobs, pjobs, @record)
					if ! dotime
						puts
					end
				end
			end
		end
	end

	def init_others
		@user = UserHash.new(@record) if @do_users
		@host = HostHash.new(@record) if @do_hosts
		@parallel = ParallelHash.new(@record) if @do_parallel
		@queue = QueueHash.new(@record) if @do_queues
		@project = ProjectHash.new(@record) if @do_projects
		@category = CategoryHash.new(@record) if @do_categories
			
		if @do_timesteps || @do_cat_per_ts || @do_job_per_ts
			@timestep = TimestepHash.new(@record) 
			if @first_ts == "first"
				@first = @timestep.first
			else
				@first = @first_ts.to_i
			end
			if @last_ts == "last"
				@last =  @timestep.last
			else
				@last = @last_ts.to_i
			end
		end
		init_ts_categories if @do_timesteps || @do_cat_per_ts
	end

	def report
		if @do_records # -r
			@record.print_all 
			STDOUT.flush
		end
		if @do_queues # -q
			@queue.print_all 
			STDOUT.flush
		end
		if @do_projects # -p
			@project.print_all
			STDOUT.flush
		end
		if @do_hosts # -h
			@host.print_all
			STDOUT.flush
		end
		if @do_parallel # -par
			@parallel.print_all
			STDOUT.flush
		end
		if @do_users # -u
			@user.print_all
			STDOUT.flush
		end
		if @do_categories # -c
			@category.print_all
			STDOUT.flush
		end
		if @do_timesteps  # -ts
			print_timesteps(true, false, false)
			STDOUT.flush
		end
		if @do_cat_per_ts || @do_job_per_ts  # -ts_j -ts-c
			print_timesteps(false, true, true)
			STDOUT.flush
		end
	end
end

# ----------- main ---------

a = Analyzer.new
a.parse_options
a.read_records(ARGV[0])
a.init_others
a.report

exit 0
