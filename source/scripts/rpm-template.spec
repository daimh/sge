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
#  Copyright: 2001 by Sun Microsystems, Inc.
#
#  All Rights Reserved.
#
##########################################################################
#___INFO__MARK_END__

%define        sge_root #vdir#
%define        debug_package %{nil}

%ifarch i386
%define        sge_arch lx-x86
%else
%define        sge_arch lx-amd64
%endif

%define        _rpmdir		   #basedir#
%define        _sourcedir		#basedir#

Name:          #product#
Requires:      openmotif
Version:       #version#
Release:       0
License: (BSD and LGPLv2+ and MIT and SISSL) and GPLv2+ and BSD with advertising
URL:           https://arc.liv.ac.uk/trac/SGE
Group:         Applications/System
Vendor:        #vendor#
Packager:      #packager#
Provides:      #provides# 
Summary:       #summary#
Prefix:        %{sge_root}
BuildArchitectures: #arch#

%description


%package common
Summary:       #summary# - Architecture independent files
Group:         Applications/System
%description common
#description#

%package doc 
Summary:       #summary# - Documentation
Group:         Applications/System
%description doc
#description#

%package bin
Summary:       #summary# - Binary files
Group:         Applications/System
Requires:      #product#-common = #version#
%description bin
#description#

%post bin
export SGE_ROOT=$RPM_INSTALL_PREFIX
$SGE_ROOT/util/setfileperm.sh -auto $SGE_ROOT

