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

version = DRMAA.version
drm = DRMAA.drm_system
impl = DRMAA.drmaa_implementation
contact = DRMAA.contact
puts "DRMAA: #{drm} version #{version} impl #{impl} contact #{contact}"

session = DRMAA::Session.new

puts "supported DRMAA job template attributes:"
attrs = DRMAA.attributes.each { |n| puts "   #{n}" }
puts "supported DRMAA job template vector attributes:"
attrs = DRMAA.vector_attributes.each { |n| puts "   #{n}" }


exit 0
