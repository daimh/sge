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

require 'dl/import'

module DRMAA
	class DRMAAException < StandardError ; end
	class DRMAAInternalError < DRMAAException ; end
	class DRMAACommunicationError < DRMAAException ; end
	class DRMAAAuthenticationError < DRMAAException ; end
	class DRMAAInvalidArgumentError < DRMAAException ; end
	class DRMAANoActiveSessionError < DRMAAException ; end
	class DRMAANoMemoryError < DRMAAException ; end
	class DRMAAInvalidContactError < DRMAAException ; end
	class DRMAADefaultContactError < DRMAAException ; end
	class DRMAASessionInitError < DRMAAException ; end
	class DRMAAAlreadyActiveSessionError < DRMAAException ; end
	class DRMAASessionExitError < DRMAAException ; end
	class DRMAAInvalidAttributeFormatError < DRMAAException ; end
	class DRMAAInvalidAttributeValueError < DRMAAException ; end
	class DRMAAConflictingAttributeValuesError < DRMAAException ; end
	class DRMAATryLater < DRMAAException ; end
	class DRMAADeniedError < DRMAAException ; end
	class DRMAAInvalidJobError < DRMAAException ; end
	class DRMAAResumeInconsistent < DRMAAException ; end
	class DRMAASuspendInconsistent < DRMAAException ; end
	class DRMAAHoldInconsistent < DRMAAException ; end
	class DRMAAReleaseInconsistent < DRMAAException ; end
	class DRMAATimeoutExit < DRMAAException ; end

	class DRMAANoDefaultContactSelected < DRMAAException ; end
	class DRMAANoMoreElements < DRMAAException ; end

	# drmaa_job_ps() constants
	STATE_UNDETERMINED          = 0x00
	STATE_QUEUED_ACTIVE         = 0x10
	STATE_SYSTEM_ON_HOLD        = 0x11
	STATE_USER_ON_HOLD          = 0x12
	STATE_USER_SYSTEM_ON_HOLD   = 0x13
	STATE_RUNNING               = 0x20
	STATE_SYSTEM_SUSPENDED      = 0x21
	STATE_USER_SUSPENDED        = 0x22
	STATE_USER_SYSTEM_SUSPENDED = 0x23
	STATE_DONE                  = 0x30
	STATE_FAILED                = 0x40

	# drmaa_control() constants
 	ACTION_SUSPEND   = 0
 	ACTION_RESUME    = 1
 	ACTION_HOLD      = 2
 	ACTION_RELEASE   = 3
 	ACTION_TERMINATE = 4

	# placeholders for job input/output/error path and working dir
	PLACEHOLDER_INCR = "$drmaa_incr_ph$"
	PLACEHOLDER_HD   = "$drmaa_hd_ph$"
	PLACEHOLDER_WD   = "$drmaa_wd_ph$"

private
	ANY_JOB  = "DRMAA_JOB_IDS_SESSION_ANY"
	ALL_JOBS = "DRMAA_JOB_IDS_SESSION_ALL"

	# need errno mapping due to errno's changed from DRMAA 0.95 to 1.0 ... sigh!
	ERRNO_MAP_095 = [ [ "DRMAA_ERRNO_SUCCESS",                        0 ],
   			 		   [ "DRMAA_ERRNO_INTERNAL_ERROR",                 1 ],
   					   [ "DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE",      2 ],
   					   [ "DRMAA_ERRNO_AUTH_FAILURE",                   3 ],
   					   [ "DRMAA_ERRNO_INVALID_ARGUMENT",               4 ],
   					   [ "DRMAA_ERRNO_NO_ACTIVE_SESSION",              5 ],
   					   [ "DRMAA_ERRNO_NO_MEMORY",                      6 ],

							[ "DRMAA_ERRNO_INVALID_CONTACT_STRING",         7 ],
							[ "DRMAA_ERRNO_DEFAULT_CONTACT_STRING_ERROR" ,  8 ],
						   [ "DRMAA_ERRNO_DRMS_INIT_FAILED",               9 ],
						   [ "DRMAA_ERRNO_ALREADY_ACTIVE_SESSION",         10 ],
						   [ "DRMAA_ERRNO_DRMS_EXIT_ERROR",                11 ],

						   [ "DRMAA_ERRNO_INVALID_ATTRIBUTE_FORMAT",       12 ],
						   [ "DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE",        13 ],
						   [ "DRMAA_ERRNO_CONFLICTING_ATTRIBUTE_VALUES",   14 ],

						   [ "DRMAA_ERRNO_TRY_LATER",                      15 ],
						   [ "DRMAA_ERRNO_DENIED_BY_DRM",                  16 ],

						   [ "DRMAA_ERRNO_INVALID_JOB",                    17 ],
						   [ "DRMAA_ERRNO_RESUME_INCONSISTENT_STATE",      18 ],
						   [ "DRMAA_ERRNO_SUSPEND_INCONSISTENT_STATE",     19 ],
						   [ "DRMAA_ERRNO_HOLD_INCONSISTENT_STATE",        20 ],
						   [ "DRMAA_ERRNO_RELEASE_INCONSISTENT_STATE",     21 ],
						   [ "DRMAA_ERRNO_EXIT_TIMEOUT",                   22 ],
						   [ "DRMAA_ERRNO_NO_RUSAGE",                      23 ] ]

	ERRNO_MAP_100 = [ [ "DRMAA_ERRNO_SUCCESS",                            0 ],
   			 		   [ "DRMAA_ERRNO_INTERNAL_ERROR",                     1 ],
   					   [ "DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE",          2 ],
   					   [ "DRMAA_ERRNO_AUTH_FAILURE",                       3 ],
   					   [ "DRMAA_ERRNO_INVALID_ARGUMENT",                   4 ],
   					   [ "DRMAA_ERRNO_NO_ACTIVE_SESSION",                  5 ],
   					   [ "DRMAA_ERRNO_NO_MEMORY",                          6 ],

							[ "DRMAA_ERRNO_INVALID_CONTACT_STRING",             7 ],
							[ "DRMAA_ERRNO_DEFAULT_CONTACT_STRING_ERROR",       8 ],
   					   [ "DRMAA_ERRNO_NO_DEFAULT_CONTACT_STRING_SELECTED", 9 ],
						   [ "DRMAA_ERRNO_DRMS_INIT_FAILED",                   10 ],
						   [ "DRMAA_ERRNO_ALREADY_ACTIVE_SESSION",             11 ],
						   [ "DRMAA_ERRNO_DRMS_EXIT_ERROR",                    12 ],

						   [ "DRMAA_ERRNO_INVALID_ATTRIBUTE_FORMAT",           13 ],
						   [ "DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE",            14 ],
						   [ "DRMAA_ERRNO_CONFLICTING_ATTRIBUTE_VALUES",       15 ],

						   [ "DRMAA_ERRNO_TRY_LATER",                          16 ],
						   [ "DRMAA_ERRNO_DENIED_BY_DRM",                      17 ],

						   [ "DRMAA_ERRNO_INVALID_JOB",                        18 ],
						   [ "DRMAA_ERRNO_RESUME_INCONSISTENT_STATE",          19 ],
						   [ "DRMAA_ERRNO_SUSPEND_INCONSISTENT_STATE",         20 ],
						   [ "DRMAA_ERRNO_HOLD_INCONSISTENT_STATE",            21 ],
						   [ "DRMAA_ERRNO_RELEASE_INCONSISTENT_STATE",         22 ],
						   [ "DRMAA_ERRNO_EXIT_TIMEOUT",                       23 ],
						   [ "DRMAA_ERRNO_NO_RUSAGE",                          24 ],
						   [ "DRMAA_ERRNO_NO_MORE_ELEMENTS",                   25 ]]


	def DRMAA.errno2str(drmaa_errno)
		if @version < 1.0
			s = ERRNO_MAP_095.find{ |pair| pair[1] == drmaa_errno }[0]
		else
			s = ERRNO_MAP_100.find{ |pair| pair[1] == drmaa_errno }[0]
		end
		s = "DRMAA_ERRNO_INTERNAL_ERROR" if s.nil?
		# puts "errno2str(" + drmaa_errno.to_s + ") = " + s
		return s
	end

	def DRMAA.str2errno(str)
		if @version < 1.0
			errno = ERRNO_MAP_095.find{ |pair| pair[0] == str }[1]
		else
			errno = ERRNO_MAP_100.find{ |pair| pair[0] == str }[1]
		end
		errno = 1 if errno.nil? # internal error
		# puts "str2errno(" + str + ") = " + errno.to_s
		return errno
	end

	# 161 character buffer constant (length is arbitrary)
	JC = EC = "\0\0\0\0\0\0\0\0\0\0" + "\0\0\0\0\0\0\0\0\0\0" + "\0\0\0\0\0\0\0\0\0\0" + "\0\0\0\0\0\0\0\0\0\0" +
		       "\0\0\0\0\0\0\0\0\0\0" + "\0\0\0\0\0\0\0\0\0\0" + "\0\0\0\0\0\0\0\0\0\0" + "\0\0\0\0\0\0\0\0\0\0" +
		       "\0\0\0\0\0\0\0\0\0\0" + "\0\0\0\0\0\0\0\0\0\0" + "\0\0\0\0\0\0\0\0\0\0" + "\0\0\0\0\0\0\0\0\0\0" +
		       "\0\0\0\0\0\0\0\0\0\0" + "\0\0\0\0\0\0\0\0\0\0" + "\0\0\0\0\0\0\0\0\0\0" + "\0\0\0\0\0\0\0\0\0\0" + "\0" 

	# do dlopen() and initialize symbols
	def DRMAA.dopen
		if @libdrmaa.nil?
			osname = `uname`.strip!
			case osname
			when "Darwin"
				libext = ".dylib"
                        # fixme: cygwin etc.
			else
				libext = ".so"
			end
			@libdrmaa = DL.dlopen('libdrmaa' + libext)

			@drmaa_version = @libdrmaa['drmaa_version', 'IiisI']
			@drmaa_get_drm_system = @libdrmaa['drmaa_get_DRM_system', 'IsIsI']
			@drmaa_get_contact = @libdrmaa['drmaa_get_contact', 'IsIsI']
			@drmaa_get_drmaa_implementation = @libdrmaa['drmaa_get_DRMAA_implementation', 'IsIsI']

			@drmaa_init = @libdrmaa['drmaa_init', 'ISsI']
			@drmaa_exit = @libdrmaa['drmaa_exit', 'IsI']

			@drmaa_allocate_job_template = @libdrmaa['drmaa_allocate_job_template', 'IpsI']
			@drmaa_delete_job_template   = @libdrmaa['drmaa_delete_job_template', 'IPsI']

			@drmaa_set_attribute = @libdrmaa['drmaa_set_attribute', 'IPSSsI']
			@drmaa_set_vector_attribute = @libdrmaa['drmaa_set_vector_attribute', 'IPSPsI']
			@drmaa_get_attribute = @libdrmaa['drmaa_get_attribute', 'IPSsIsI']
			@drmaa_get_vector_attribute = @libdrmaa['drmaa_get_vector_attribute', 'IPSpsI']

			@drmaa_get_attribute_names = @libdrmaa['drmaa_get_attribute_names', 'IpsI']
			@drmaa_get_vector_attribute_names = @libdrmaa['drmaa_get_vector_attribute_names', 'IpsI']

			@drmaa_run_job = @libdrmaa['drmaa_run_job', 'IsIPsI']
			@drmaa_run_bulk_jobs = @libdrmaa['drmaa_run_bulk_jobs', 'IpPIIIsI']

			@drmaa_get_next_job_id = @libdrmaa['drmaa_get_next_job_id', 'IPsI']
			@drmaa_release_job_ids = @libdrmaa['drmaa_release_job_ids', '0P']
			@drmaa_get_next_attr_name = @libdrmaa['drmaa_get_next_attr_name', 'IPsI']
			@drmaa_release_attr_names = @libdrmaa['drmaa_release_attr_names', '0P']
			@drmaa_get_next_attr_value = @libdrmaa['drmaa_get_next_attr_value', 'IPsI']
			@drmaa_release_attr_values = @libdrmaa['drmaa_release_attr_values', '0P']

			@drmaa_job_ps = @libdrmaa['drmaa_job_ps', 'ISisI']
			@drmaa_control = @libdrmaa['drmaa_control', 'ISIsI']
			@drmaa_synchronize = @libdrmaa['drmaa_synchronize', 'IPLIsI']

			@drmaa_wait = @libdrmaa['drmaa_wait', 'ISsIiIpsI']
			@drmaa_wifexited = @libdrmaa['drmaa_wifexited', 'IiIsI']
			@drmaa_wifsignaled = @libdrmaa['drmaa_wifsignaled', 'IiIsI']
			@drmaa_wifaborted = @libdrmaa['drmaa_wifaborted', 'IiIsI']
			@drmaa_wcoredump = @libdrmaa['drmaa_wcoredump', 'IiIsI']
			@drmaa_wexitstatus = @libdrmaa['drmaa_wexitstatus', 'IiIsI']
			@drmaa_wtermsig = @libdrmaa['drmaa_wtermsig', 'IsIIsI']
			@drmaa_strerror = @libdrmaa['drmaa_strerror', 'SI']
	
			@version = 1.0
			@version = DRMAA.version

			if @version >= 1.0
				# Here we could initialize symbols for 1.0 specific
				#    int drmaa_get_num_attr_names(drmaa_attr_names_t* values, int *size);
				#    int drmaa_get_num_attr_values(drmaa_attr_values_t* values, int *size);
				#    int drmaa_get_num_job_ids(drmaa_job_ids_t* values, int *size);
				# actually we don't need these, but a failure with loading them can't be igored
				@drmaa_get_num_attr_names = @libdrmaa['drmaa_get_num_attr_names', 'IPi']
				@drmaa_get_num_attr_values = @libdrmaa['drmaa_get_num_attr_values', 'IPi']
				@drmaa_get_num_job_ids = @libdrmaa['drmaa_get_num_job_ids', 'IPi']
			end
		end
	end

public
	# returns string specifying the DRM system
	# int drmaa_get_drm_system(char *, size_t , char *, size_t)
	def DRMAA.drm_system
		DRMAA.dopen
		err = drm = EC
		r,r1 = @drmaa_get_drm_system.call(drm, 160, err, 160)
		DRMAA.throw(r, r1[2])
		return r1[0]
	end

	# returns string specifying contact information
	# int drmaa_get_contact(char *, size_t, char *, size_t)
	def DRMAA.contact
		DRMAA.dopen
		err = contact = EC
		r,r1 = @drmaa_get_contact.call(contact, 160, err, 160)
		DRMAA.throw(r, r1[2])
		return r1[0]
	end

	# returns string specifying DRMAA implementation
	# int drmaa_get_DRMAA_implementation(char *, size_t , char *, size_t)
	def DRMAA.drmaa_implementation
		DRMAA.dopen
		err = implementation = EC
		r,r1 = @drmaa_get_drmaa_implementation.call(implementation, 160, err, 160)
		DRMAA.throw(r, r1[2])
		return r1[0]
	end

	# returns DRMAA version (e.g. 1.0 or 0.95)
	# int drmaa_version(unsigned int *, unsigned int *, char *, size_t )
	def DRMAA.version
		DRMAA.dopen
		err = EC
		major = minor = 0
		r,r1 = @drmaa_version.call(major, minor, err, 160)
		DRMAA.throw(r, r1[2])
		return r1[0] + (Float(r1[1])/100)
	end

private
	# const char *drmaa_strerror(int drmaa_errno)
	def DRMAA.strerror(errno)
		r =  @drmaa_strerror.call(drmaa_errno)
		return r.to_s
	end

	# int drmaa_job_ps( const char *, int *, char *, size_t )
	def DRMAA.job_ps(job)
		err = EC
		state = 0
		r,r1 = @drmaa_job_ps.call(job, state, err, 160)
		DRMAA.throw(r, r1[2])
		return r1[1]
	end

	# int drmaa_control(const char *, int , char *, size_t )
	def DRMAA.control(job, action)
		err = EC
		r,r1 = @drmaa_control.call(job, action, err, 160)
		DRMAA.throw(r, r1[2])
	end


	# int drmaa_init(const char *, char *, size_t)
	def DRMAA.init(contact)
		DRMAA.dopen
		err = EC
		r,r1 = @drmaa_init.call(contact, err, 160-1)
		DRMAA.throw(r, r1[1])
	end


	# int drmaa_exit(char *, size_t)
	def DRMAA.exit
		err = EC
		r,r1 = @drmaa_exit.call(err, 160)
		DRMAA.throw(r, r1[0])
	end

	# int drmaa_allocate_job_template(drmaa_job_template_t **, char *, size_t)
	def DRMAA.allocate_job_template
		err = EC
		jt = DL.malloc(DL.sizeof('p'))
		r,r1 = @drmaa_allocate_job_template.call(jt, err, 160)
		DRMAA.throw(r, r1[1])
		return jt
	end

	# int drmaa_delete_job_template(drmaa_job_template_t *, char *, size_t)
	def DRMAA.delete_job_template(jt)
		err = EC
		r,r1 = @drmaa_delete_job_template.call(jt.ptr, err, 160)
		DRMAA.throw(r, r1[1])
	end

	# int drmaa_get_vector_attribute_names(drmaa_attr_names_t **, char *, size_t)
	def DRMAA.vector_attributes()
		err = EC
		names = DL.malloc(DL.sizeof('p'))
		r,r1 = @drmaa_get_vector_attribute_names.call(names, err, 160)
		DRMAA.throw(r, r1[1])
		return DRMAA.get_attr_names(names)
	end

	# int drmaa_get_attribute_names(drmaa_attr_names_t **, char *, size_t)
	def DRMAA.attributes()
		err = EC
		names = DL.malloc(DL.sizeof('p'))
		r,r1 = @drmaa_get_attribute_names.call(names, err, 160)
		DRMAA.throw(r, r1[1])
		return DRMAA.get_attr_names(names)
	end

	def DRMAA.get_all(ids, nxt, rls)
		if @version < 1.0
			errno_expect = DRMAA.str2errno("DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE")
		else
			errno_expect = DRMAA.str2errno("DRMAA_ERRNO_NO_MORE_ELEMENTS")
		end
		# STDERR.puts "get_all(1)"
		values = Array.new
		ret = 0
		while  ret != errno_expect do
			# STDERR.puts "get_all(2) " + DRMAA.errno2str(ret)
			err = EC
			jobid = EC
			r,r1 = nxt.call(ids.ptr, jobid, 160)
			if r != errno_expect
				DRMAA.throw(r, "unexpected error")
				values.push(r1[1])
				# puts "get_all(3) " + DRMAA.errno2str(r)
			end
			ret = r
		end
		# puts "get_all(4)"
		rls.call(ids.ptr)

		return values
	end

	# int drmaa_get_next_job_id(drmaa_job_ids_t*, char *, size_t )
	# void drmaa_release_job_ids(drmaa_job_ids_t*)
	def DRMAA.get_job_ids(ids)
		return DRMAA.get_all(ids, @drmaa_get_next_job_id, @drmaa_release_job_ids)
	end


	# int drmaa_get_next_attr_name(drmaa_attr_names_t*, char *, size_t )
	# void drmaa_release_attr_names(drmaa_attr_names_t*)
	def DRMAA.get_attr_names(names)
		return DRMAA.get_all(names, @drmaa_get_next_attr_name, @drmaa_release_attr_names)
	end

	# int drmaa_get_next_attr_value(drmaa_attr_values_t*, char *, size_t )
	# void drmaa_release_attr_values(drmaa_attr_values_t*)
	def DRMAA.get_attr_values(ids)
		return DRMAA.get_all(ids, @drmaa_get_next_attr_value, @drmaa_release_attr_values)
	end

	# int drmaa_wifexited(int *, int, char *, size_t)
	def DRMAA.wifexited(stat)
		return DRMAA.wif(stat, @drmaa_wifexited)
	end

	# int drmaa_wifsignaled(int *, int, char *, size_t)
	def DRMAA.wifsignaled(stat)
		return DRMAA.wif(stat, @drmaa_wifsignaled)
	end

	# int drmaa_wifaborted(int *, int , char *, size_t)
	def DRMAA.wifaborted(stat)
		return DRMAA.wif(stat, @drmaa_wifaborted)
	end

	# int drmaa_wcoredump(int *, int , char *, size_t)
	def DRMAA.wcoredump(stat)
		return DRMAA.wif(stat, @drmaa_wcoredump)
	end

	def DRMAA.wif(stat, method)
		err = EC
		boo = 0
		r,r1 = method.call(boo, stat, err, 160)
		DRMAA.throw(r, r1[2])
		boo = r1[0]
		if boo == 0
			return false
		else
			return true
		end
	end

	# int drmaa_wexitstatus(int *, int, char *, size_t)
	def DRMAA.wexitstatus(stat)
		err = EC
		ret = 0
		r,r1 = @drmaa_wexitstatus.call(ret, stat, err, 160)
		DRMAA.throw(r, r1[2]) 
		return r1[0]
	end

	# int drmaa_wtermsig(char *signal, size_t signal_len, int stat, char *error_diagnosis, size_t error_diag_len);
	def DRMAA.wtermsig(stat)
		err = signal = EC
		r,r1 = @drmaa_wtermsig.call(signal, 160, stat, err, 160)
		DRMAA.throw(r, r1[3]) 
		return r1[0]
	end

	# int drmaa_wait(const char *, char *, size_t , int *, signed long , 
	#               drmaa_attr_values_t **, char *, size_t );
	def DRMAA.wait(jobid, timeout)
		errno_timeout = DRMAA.str2errno("DRMAA_ERRNO_EXIT_TIMEOUT")
		errno_no_rusage = DRMAA.str2errno("DRMAA_ERRNO_NO_RUSAGE")
		err = EC
		waited = EC
		stat = 0
		usage = DL.malloc(DL.sizeof('p'))
		r,r1 = @drmaa_wait.call(jobid, waited, 160, stat, timeout, usage, err, 160)
		return nil if r == errno_timeout
		if r != errno_no_rusage
			DRMAA.throw(r, r1[6])
			return JobInfo.new(r1[1], r1[3], usage)
		else
			return JobInfo.new(r1[1], r1[3])
		end
	end

	# int drmaa_run_bulk_jobs(drmaa_job_ids_t **, const drmaa_job_template_t *jt, 
	#                         int, int, int, char *, size_t)
	def DRMAA.run_bulk_jobs(jt, first, last, incr)
		err = EC
		ids = DL.malloc(DL.sizeof('p'))
		r,r1 = @drmaa_run_bulk_jobs.call(ids, jt.ptr, first, last, incr, err, 160)
		DRMAA.throw(r, r1[5])
		return DRMAA.get_job_ids(ids)
	end

	# int drmaa_run_job(char *, size_t, const drmaa_job_template_t *, char *, size_t)
	def DRMAA.run_job(jt)
		err = EC
		jobid = JC
		r,r1 = @drmaa_run_job.call(jobid, 25, jt.ptr, err, 160)
		DRMAA.throw(r, r1[3])
		return r1[0]
	end

	# int drmaa_set_attribute(drmaa_job_template_t *, const char *, const char *, char *, size_t)
	def DRMAA.set_attribute(jt, name, value)
		err = EC
		r,r1 = @drmaa_set_attribute.call(jt.ptr, name, value, err, 160)
		DRMAA.throw(r, r1[3])
	end

	# int drmaa_set_vector_attribute(drmaa_job_template_t *, const char *, 
	#                               const char *value[], char *, size_t)
	def DRMAA.set_vector_attribute(jt, name, ary)
		err = EC
		ids = ary.map{|s| s.to_ptr}
		ids << DL::PtrData.new(0)
		r,r1 = @drmaa_set_vector_attribute.call(jt.ptr, name, ids, err, 160)
		DRMAA.throw(r, r1[3])
	end

	# int drmaa_get_attribute(drmaa_job_template_t *, const char *, char *, 
	#  							size_t , char *, size_t)
	def DRMAA.get_attribute(jt, name)
		err = EC
		value = EC
		r,r1 = @drmaa_get_attribute.call(jt.ptr, name, value, 160, err, 160)
		DRMAA.throw(r, r1[3])
		return r1[2]
	end

	# int drmaa_get_vector_attribute(drmaa_job_template_t *, const char *, 
	#                   drmaa_attr_values_t **, char *, size_t )
	def DRMAA.get_vector_attribute(jt, name)
		attr = DL.malloc(DL.sizeof('p'))
		r,r1 = @drmaa_get_vector_attribute.call(jt.ptr, name, attr, err, 160)
		DRMAA.throw(r, r1[3])
		return drmaa_get_vector_attribute(r1[2])
	end

	# int drmaa_synchronize(const char *job_ids[], signed long timeout, int dispose, char *, size_t)
	def DRMAA.synchronize(jobs, timeout, dispose)
		err = EC 
		if dispose == false
			disp = 0
		else
			disp = 1
		end
		errno_timeout = DRMAA.str2errno("DRMAA_ERRNO_EXIT_TIMEOUT")
		ids = jobs.map{|s| s.to_ptr}
		ids << DL::PtrData.new(0)
		r,r1 = @drmaa_synchronize.call(ids, timeout, disp, err, 160)
		if r == errno_timeout
			return false
		else
			DRMAA.throw(r, r1[3]) 
			return true
		end
	end

	def DRMAA.throw(r, diag)
		return if r == 0
		s_errno = DRMAA.errno2str(r)
		case s_errno
		when "DRMAA_ERRNO_INTERNAL_ERROR"
			raise DRMAAInternalError, diag
		when "DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE"
			raise DRMAACommunicationError, diag
		when "DRMAA_ERRNO_AUTH_FAILURE"
			raise DRMAAAuthenticationError, diag
		when "DRMAA_ERRNO_INVALID_ARGUMENT"
			raise DRMAAInvalidArgumentError, diag
		when "DRMAA_ERRNO_NO_ACTIVE_SESSION"
			raise DRMAANoActiveSessionError, diag
		when "DRMAA_ERRNO_NO_MEMORY"
			raise DRMAANoMemoryError, diag
		when "DRMAA_ERRNO_INVALID_CONTACT_STRING"
			raise DRMAAInvalidContactError, diag
		when "DRMAA_ERRNO_DEFAULT_CONTACT_STRING_ERROR"
			raise DRMAADefaultContactError, diag
		when "DRMAA_ERRNO_NO_DEFAULT_CONTACT_STRING_SELECTED"
			raise DRMAANoDefaultContactSelected, diag
		when "DRMAA_ERRNO_DRMS_INIT_FAILED"
			raise DRMAASessionInitError, diag
		when "DRMAA_ERRNO_ALREADY_ACTIVE_SESSION"
			raise DRMAAAlreadyActiveSessionError, diag
		when "DRMAA_ERRNO_DRMS_EXIT_ERROR"
			raise DRMAASessionExitError, diag
		when "DRMAA_ERRNO_INVALID_ATTRIBUTE_FORMAT"
			raise DRMAAInvalidAttributeFormatError, diag
		when "DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE"
			raise DRMAAInvalidAttributeValueError, diag
		when "DRMAA_ERRNO_CONFLICTING_ATTRIBUTE_VALUES"
			raise DRMAAConflictingAttributeValuesError, diag
		when "DRMAA_ERRNO_TRY_LATER"
			raise DRMAATryLater, diag
		when "DRMAA_ERRNO_DENIED_BY_DRM"
			raise DRMAADeniedError, diag
		when "DRMAA_ERRNO_INVALID_JOB"
			raise DRMAAInvalidJobError, diag
		when "DRMAA_ERRNO_RESUME_INCONSISTENT_STATE"
			raise DRMAAResumeInconsistent, diag
		when "DRMAA_ERRNO_SUSPEND_INCONSISTENT_STATE"
			raise DRMAASuspendInconsistent, diag
		when "DRMAA_ERRNO_HOLD_INCONSISTENT_STATE"
			raise DRMAAHoldInconsistent, diag
		when "DRMAA_ERRNO_RELEASE_INCONSISTENT_STATE"
			raise DRMAAReleaseInconsistent, diag
		when "DRMAA_ERRNO_EXIT_TIMEOUT"
			raise DRMAATimeoutExit, diag
		when "DRMAA_ERRNO_NO_RUSAGE"
			raise DRMAANoRusage, diag
		when "DRMAA_ERRNO_NO_MORE_ELEMENTS"
			raise DRMAANoMoreElements, diag
		end
	end

public
	# const char *drmaa_strerror(int drmaa_errno)
	# DRMAA job info as returned by drmaa_wait()
	class JobInfo
		attr_reader :job
		def initialize(job, stat, rusage = nil)
			@job = job
			@stat = stat
			@rusage = Hash.new
			if ! rusage.nil?
				DRMAA.get_attr_values(rusage).each { |u|
					nv = u.scan(/[^=][^=]*/)
					@rusage[nv[0]] = nv[1]
				}
			end
		end
		def wifaborted?  
			DRMAA.wifaborted(@stat) 
		end
		# true if job finished and exit status available
		def wifexited?  
			DRMAA.wifexited(@stat) 
		end
		# true if job was signaled and termination signal available
		def wifsignaled?  
			DRMAA.wifsignaled(@stat) 
		end
		# true if job core dumped
		def wcoredump?  
			DRMAA.wcoredump(@stat) 
		end
		# returns job exit status
		def wexitstatus 
			DRMAA.wexitstatus(@stat) 
		end
		# returns termination signal as string
		def wtermsig 
			DRMAA.wtermsig(@stat) 
		end
		# returns resource utilization as string array ('name=value')
		def rusage
			return @rusage
		end
	end

	# DRMAA Session
	class Session
		attr_accessor :retry

		# initialize DRMAA session
		def initialize(contact = "")
			DRMAA.init(contact)
			ObjectSpace.define_finalizer(self, self.method(:finalize).to_proc)
			@retry = 0
		end

		# close DRMAA session
		def finalize(id)
			# STDERR.puts "... exiting DRMAA"
			DRMAA.exit
		end

		# non-zero retry interval causes DRMAA::DRMAATryLater be handled transparently  
		def retry_until
			if @retry == 0
				job = yield
			else
				begin
					job = yield
				rescue DRMAA::DRMAATryLater
					STDERR.puts "... sleeping"
					sleep @retry
					retry
				end
			end
			return job
		end

		# submits job described by JobTemplate 't' and returns job id as string
		def run(t)
			retry_until { DRMAA.run_job(t.ptr) }
		end

		# submits bulk job described by JobTemplate 't' 
		# and returns an array of job id strings
		def run_bulk(t, first, last, incr = 1)
			retry_until { DRMAA.run_bulk_jobs(t.ptr, first, last, incr) }
		end

		# wait for any job of this session and return JobInfo 
		def wait_any(timeout = -1)
			DRMAA.wait(ANY_JOB, timeout)
		end

		# wait for job and return JobInfo 
		def wait(job, timeout = -1)
			DRMAA.wait(job, timeout)
		end

		# run block with JobInfo to finish for each waited session job
		# or return JobInfo array if no block was passed
		def wait_each(timeout = -1)
			if ! block_given?
				ary = Array.new
			end
			while true
				begin
					info = DRMAA.wait(ANY_JOB, timeout)
				rescue DRMAAInvalidJobError
					break
				end
				if block_given?
					yield info
				else
					ary << info
				end
			end
			if ! block_given?
				return ary
			end
		end

		# synchronize with all session jobs and dispose any job finish information
		# returns false in case of a timeout
		def sync_all!(timeout = -1)
			DRMAA.synchronize([ ALL_JOBS ], timeout, true)
		end

		# synchronize with all session jobs
		# returns false in case of a timeout
		def sync_all(timeout = -1, dispose = false)
			DRMAA.synchronize([ ALL_JOBS ], timeout, dispose)
		end

		# synchronize with specified session jobs and dispose any job finish information
		# returns false in case of a timeout
		def sync!(jobs, timeout = -1)
			DRMAA.synchronize(jobs, timeout, true)
		end

		# synchronize with specified session jobs
		# returns false in case of a timeout
		def sync(jobs, timeout = -1)
			DRMAA.synchronize(jobs, timeout, false)
		end

		# suspend specified job or all session jobs
		def suspend(job = ALL_JOBS)
			DRMAA.control(job, DRMAA::ACTION_SUSPEND)
		end

		# resume specified job or all session jobs
		def resume(job = ALL_JOBS)
			DRMAA.control(job, DRMAA::ACTION_RESUME)
		end

		# put specified job or all session jobs in hold state
		def hold(job = ALL_JOBS)
			DRMAA.control(job, DRMAA::ACTION_HOLD)
		end

		# release hold state for specified job or all session jobs
		def release(job = ALL_JOBS)
			DRMAA.control(job, DRMAA::ACTION_RELEASE)
		end

		# terminate specified job or all session jobs
		def terminate(job = ALL_JOBS)
			DRMAA.control(job, DRMAA::ACTION_TERMINATE)
		end
	
		# get job state
		def job_ps(job)
		   DRMAA.job_ps(job)
		end
	end

	# DRMAA job template as required by drmaa_run_job() and drmaa_run_bulk_jobs()
	class JobTemplate
		attr_reader :ptr
		def initialize
			@ptr = DRMAA.allocate_job_template
			ObjectSpace.define_finalizer(self, self.method(:finalize).to_proc)
		end
		def finalize(id)
			# STDERR.puts "... releasing job template"
			DRMAA.delete_job_template(@ptr)
		end
		def set(name, value)
			DRMAA.set_attribute(@ptr, name, value)
		end
		def get(name)
			DRMAA.get_attribute(@ptr, name)
		end
		def vset(name, values)
			DRMAA.set_vector_attribute(@ptr, name, values)
		end
		def vget(name)
			DRMAA.get_vector_attribute(@ptr, name)
		end

		# path of the command to be started as a job
		def command=(cmd)
			set("drmaa_remote_command", cmd) 
		end
		def command()
			return get("drmaa_remote_command") 
		end

		# DRMAA job category
		def category=(cat)
			set("drmaa_job_category", cat) 
		end
		def category()
			return set("drmaa_job_category") 
		end

		# an opaque string that is interpreted by the DRM
		# refer to DRM documentation for what can be specified here
		def native=(nat)
			set("drmaa_native_specification", nat) 
		end
		def native()
			return get("drmaa_native_specification") 
		end

		# jobs stdin path (format "[<hostname>]:<file_path>")
		def stdin=(host_path)
			set("drmaa_input_path", host_path) 
		end
		def stdin()
			get("drmaa_input_path") 
		end

		# jobs stdout path (format "[<hostname>]:<file_path>")
		def stdout=(host_path)
			set("drmaa_output_path", host_path) 
		end
		def stdout()
			return get("drmaa_output_path") 
		end

		# jobs stderr path (format "[<hostname>]:<file_path>")
		def stderr=(host_path)
			set("drmaa_error_path", host_path) 
		end
		def stderr()
			return get("drmaa_error_path") 
		end

	   # specifies which files need to be transfered	
		def transfer=(transfer)
			set("drmaa_transfer_files", transfer) 
		end

		# job name
		def name=(name)
			set("drmaa_job_name", name) 
		end
		def name
			return get("drmaa_job_name") 
		end

		# jobs working directory
		def wd=(path)
			set("drmaa_wd", path) 
		end
		def wd
			return get("drmaa_wd") 
		end

		# set jobs start time (format ""[[[[CC]YY/]MM/]DD] hh:mm[:ss] [{-|+}UU:uu])")
		def start_time=(time)
			set("drmaa_start_time", time) 
		end

		# jobs can be submitted in hold state and released later-on
		def hold=(hold)
			if hold
				set("drmaa_js_state", "drmaa_hold") 
			else
				set("drmaa_js_state", "drmaa_active") 
			end
		end
		def hold?
			if get("drmaa_js_state") == "drmaa_hold" 
		   true else false end
		end

		def block_mail=(block)
			if block
				set("drmaa_block_email", "1") 
			else
				set("drmaa_block_email", "0") 
			end
		end

		# join jobs stdout/stderr 
		def join=(join) 
			if join
				set("drmaa_join_files", "y") 
			else
				set("drmaa_join_files", "n") 
			end
		end
		def join?() 
		   if	get("drmaa_join_files") == "y" 
			   return true
			else
			   return false
			end
		end

		# job arguments
		def arg=(argv) 
			vset("drmaa_v_argv", argv) 
		end
		def arg() 
			return vget("drmaa_v_argv") 
		end

		# job environment
		def env=(env) 
			vset("drmaa_v_env", env) 
		end

		# mail receipants
		def mail=(mail) 
			vset("drmaa_v_email", mail) 
		end
	end
end
