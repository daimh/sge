# This spec file is intended to build gridengine rpms on RedHat and
# derivatives (with the EPEL repository enabled) or Fedora, and also
# OpenSuSE.  It installs into /opt/sge, intended to support shared
# installations like the original Sun distribution.  It doesn't deal
# with the configuration of the installed binaries (use the vanilla
# instructions) or with init scripts, because it's difficult with
# anything other than a default cell.

# This was originally derived from the Fedora version, but
# doesn't bear much resemblance to it now.

# Fixmes:
# * Check on/port to other relevant distributions.
# * Support shared installs "--with sharedinstall"
# * Clarify the licence on this file to the extent it's derived from the
#   Fedora one.
# * Patch or hook for installation scripts to default appropriately for
#   packaged installation.
# * Build GSS modules?

# Use "rpmbuild --without java" to omit all Java bits
%ifarch ppc64
%bcond_with java
%else
%bcond_without java
%endif

# Use "rpmbuild --with hadoop" to build Hadoop support (the herd library)
# against Cloudera RPMs.
# Perhaps this should be herd, not hadoop?
%bcond_with hadoop

%define sge_home /opt/sge
%define sge_lib %{sge_home}/lib
# Binaries actually go in arch-dependent subdir of this
%define sge_bin %{sge_home}/bin
%define sge_mandir %{sge_home}/man
%define sge_libexecdir %{sge_home}/utilbin
%define sge_javadir %sge_lib
%define sge_include %{sge_home}/include
%define sge_docdir %{sge_home}/doc
%define _docdir %{sge_docdir}
# admin user maybe to create
%define username sgeadmin

%global _hardened_build 1

# swing-layout is now in supported Fedora and EPEL6
%if 0%{?fedora} > 18 || 0%{?el6} || 0%{?suse_version}
%global have_layout 1
%else
%global have_layout 0
%endif

%global __requires_exclude \(libspool.*\|/usr/bin/\(tclsh\|python\|ruby\|ksh\)\)
%global __provides_exclude_from %{sge_lib}/[^/]+/libspool.*\.so
%{?filter_setup:
%filter_from_provides /libspool.*\.so/d
%filter_from_requires /\/usr\/bin\/\(tclsh\|python\|ruby\|ksh\)\|libspool.*\.so/d
%filter_setup
}

Name:    gridengine
Version: 8.1.9

%if 0%{?fedora}
Epoch:   1
%endif
Release: 1%{?dist}
Summary: Grid Engine - Distributed Resource Manager

Group:   Applications/System
# per 3rd_party_licscopyrights
License: (SISSL and BSD and LGPLv3+ and MIT) and GPLv3+ and GFDL and others
URL:     https://arc.liv.ac.uk/trac/SGE
Source:  https://arc.liv.ac.uk/downloads/SGE/releases/%{version}/sge-%{version}.tar.gz
Source1: IzPack-4.1.1-mod.tar.gz
Source2: swing-layout-1.0.3.tar.gz

Prefix: %{sge_home}

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

# OpenSuSE support; SLES changes welcome
# Fixme: maybe just depend on specific file names
%if 0%{?suse_version}
%global sslpkg libopenssl
# Only in Factory as of OS 13.1
%global hwlocpkg libhwloc
%global xmupkg xorg-x11-libXmu
%global with_jemalloc %nil
%global with_munge %nil
%else
%global sslpkg openssl
%global hwlocpkg hwloc
%global xmupkg libXmu
%global with_jemalloc -with-jemalloc
%global with_munge -with-munge
BuildRequires: jemalloc-devel munge-devel
%endif

BuildRequires: /bin/csh, %{sslpkg}-devel, ncurses-devel, pam-devel
BuildRequires: net-tools, %xmupkg-devel, %hwlocpkg-devel >= 1.1
# The relevant package might be db4-devel, libdb-devel, or
# libdb-4_8-devel etc., so simplify by requiring the header.  "zypper
# install /usr/include/db.h" doesn't work on SuSE, so you have to
# install the packages explicitly.
BuildRequires: /usr/include/db.h
# This could be in lesstif-devel, motif-devel, or openmotif-devel
BuildRequires: /usr/include/Xm/Xm.h
%if %{with java}
BuildRequires: java-devel >= 1.6.0, javacc, ant-junit
%if ! 0%{?fedora} && 0%{?rhel} < 7
BuildRequires: ant-nodeps
%endif
%if %have_layout
BuildRequires: swing-layout
%endif
%if %{with hadoop}
BuildRequires: hadoop-0.20 >= 0.20.2+923.197
%endif
%endif
# hostname was in net-tools, but is in its own package in Fedora 19;
# requiring /bin/hostname doesn't work generally as it's in /usr/bin in
# Fedora > 20.
%if 0%{?fedora} >= 19
BuildRequires: hostname
Requires: hostname
%else
BuildRequires: /bin/hostname
Requires: /bin/hostname
%endif
Requires: binutils, ncurses, shadow-utils, /bin/awk, which, openssl
%if 0%{?fedora} || 0%{?rhel} > 6
Requires: man-db
%else
# for makewhatis, or SuSE mandb
Requires: man
%endif
# There's an implicit dependency on perl(XML::Simple), which is in
# package perl-XML-Simple but only in the optional-rpms repo on RH6.

%if 0%{?epoch}
%global epch %{epoch}:
%else
%global epch %{nil}
%endif

%description
Grid Engine (often known as SGE) is a distributed resource manager,
typically deployed to manage batch jobs on computational clusters (like
Torque/Maui), but also capable of managing interactive jobs and looser
collections of resources, such as desktop PCs (like Condor).

The computational resources may be heterogeneous (including different
operating systems) with specified properties.  Jobs are matched to
available resources according to the properties they request.

These are the files shared by both the qmaster and execd daemons,
required to run either the server or clients.

https://arc.liv.ac.uk/trac/SGE

%package devel
Summary: Gridengine development files
Group: Development/Libraries
License: SISSL
Requires: %{name} = %{epch}%{version}-%{release}
%if 0%{?rhel} >= 6 || 0%{?fedora}
BuildArch: noarch
%endif

%description devel
Grid Engine development headers and libraries.

%package qmon
Summary: Gridengine qmon monitor
Group: Applications/System
License: BSD and LGPLv3+ and MIT and SISSL and others
Requires: %{name} = %{epch}%{version}-%{release}
Requires: xorg-x11-fonts-ISO8859-1-100dpi xorg-x11-fonts-ISO8859-1-75dpi

%description qmon
The qmon graphical graphical interface to Grid Engine.

%package execd
Summary: Gridengine execd program
Group: Applications/System
License: BSD and LGPLv3+ and MIT and SISSL and others
Requires: %{name} = %{epch}%{version}-%{release}
Requires(postun): %{name} = %{epch}%{version}-%{release}
Requires(preun): %{name} = %{epch}%{version}-%{release}
Requires: /bin/ps xterm /bin/mail

%description execd
Programs needed to run a Grid Engine execution host.

%package qmaster
Summary: Gridengine qmaster programs
Group: Applications/System
License: BSD and LGPLv3+ and MIT and SISSL and others
Requires: %{name} = %{epch}%{version}-%{release}
Requires: db4-utils
Requires(postun): %{name} = %{epch}%{version}-%{release}
Requires(preun): %{name} = %{epch}%{version}-%{release}
Requires: /bin/ps

%description qmaster
Programs needed to run a Grid Engine master host.

%package drmaa4ruby
Summary: Ruby binding for DRMAA library
Group: Development/Libraries
License: SISSL
Requires: /usr/bin/ruby
%if 0%{?rhel} >= 6 || 0%{?fedora}
BuildArch: noarch
%endif

%description drmaa4ruby
This binding is presumably not Grid Engine-specific.

%if %{with java}
%if %{with hadoop}
%package hadoop
Summary: Grid Engine Hadoop integration
Group: Applications/System
License: SISSL
%if 0%{?rhel} >= 6 || 0%{?fedora}
BuildArch: noarch
%endif

%description hadoop
Support for Grid Engine Hadoop integration.
%endif

%package guiinst
Summary: Grid Engine GUI installer
Group: Applications/System
License: SISSL and LGPLv3+ and Apache License and others
Requires: java >= 1.6.0
%if 0%{?rhel} >= 6 || 0%{?fedora}
BuildArch: noarch
%endif

%description guiinst
Optional Java-based GUI installer for Grid Engine.
%endif

%prep

%setup -q -n sge-%{version}
%if %{with java}
tar zfx %SOURCE1
%if ! %have_layout
tar zfx %SOURCE2
%endif
%endif

%build
%if %{with java}
# swing-layout is now in EPEL6 and supported Fedora
%if ! %have_layout
cd swing-layout-1.0.3
ant
cd ..
%endif
%endif
cd source
> aimk.private
cat > build_private.properties <<\EOF
javacc.home=%{_javadir}
libs.junit_4.classpath=%{_javadir}/junit.jar
hadoop.dir=/usr/lib/hadoop-0.20
hadoop.version=0.20.2-cdh3u3
file.reference.hadoop-0.20.2-core.jar=${hadoop.dir}/hadoop-core.jar
file.reference.hadoop-0.20.2-tools.jar=${hadoop.dir}/hadoop-tools.jar
izpack.home=${sge.srcdir}/../IzPack-4.1.1
%if ! %have_layout
libs.swing-layout.classpath=${sge.srcdir}/../swing-layout-1.0.3/dist/swing-layout.jar
%endif
libs.ant.classpath=%{_javadir}/ant.jar
EOF

# -O2/-O3 gives warnings about type puns.  It's not clear whether
# they're serious, but -fno-strict-aliasing just in case.
export SGE_INPUT_CFLAGS="$RPM_OPT_FLAGS -fno-strict-aliasing"
export SGE_INPUT_LDFLAGS="$LDFLAGS"
[ -n "$RPM_BUILD_NCPUS" ] && parallel_flags="-parallel $RPM_BUILD_NCPUS"
%if %{without java}
JAVA_BUILD_OPTIONS="-no-java -no-jni"
%else
%if %{without hadoop}
JAVA_BUILD_OPTIONS="-no-herd"
%endif
%endif
sh scripts/bootstrap.sh $JAVA_BUILD_OPTIONS
# -no-remote because we have ssh and PAM instead
./aimk -pam %with_jemalloc -no-remote %with_munge $parallel_flags $JAVA_BUILD_OPTIONS
./aimk -man $JAVA_BUILD_OPTIONS
%if %{with java}
# "-no-gui-inst -no-herd -javadoc" still produces all the javadocs
ant drmaa.javadoc jjsv.javadoc
# Fixme:  These have symbol-not-found and other errors, which cause the
# build to fail with recent javadoc.
ant juti.javadoc jgdi.javadoc gui_inst.javadoc || true
%if %{with hadoop}
ant herd.javadoc
%endif
%endif

%install 
rm -rf $RPM_BUILD_ROOT
SGE_ROOT=$RPM_BUILD_ROOT%{sge_home}
export SGE_ROOT
mkdir -p $SGE_ROOT
cd source
echo instremote=false >> distinst.private
gearch=`dist/util/arch`
echo 'y'| scripts/distinst -local -allall ${gearch}
( cd $RPM_BUILD_ROOT/%{sge_home}
  rm -rf dtrace catman
%if %{without hadoop}
  rm -r hadoop
%endif
  rm man/man8/SGE_Helper_Service.exe.8
  rm -r util/sgeSMF
  rm doc/arc_depend_*
  rm util/resources/loadsensors/interix-loadsensor.sh # uses ksh
  for l in lib/*/libdrmaa.so.1.0; do
    ( cd $(dirname $l); ln -sf libdrmaa.so.1.0 libdrmaa.so )
  done
# This won't work on RH5 or 6 unless we fiddle with the previously-linked
# pages, so don't bother.
#   find man -type l | xargs rm -f
#   gzip man/man*/*
)
cat ../README - > $RPM_BUILD_ROOT/%{sge_home}/doc/README <<+

Note that, unlike the Fedora rpm, this version doesn't try to configure
the system, or provide its own init scripts, and installs into /opt/sge,
which is appropriate for a cluster shared installation, consistent with
the old Sun binaries.
+

%clean
rm -rf $RPM_BUILD_ROOT

%pre
%{_sbindir}/useradd -d / -s /sbin/nologin \
   -M -r -c 'Grid Engine admin' %username 2>/dev/null || :

%post
if [ -f /usr/sbin/mandb -o  -f /usr/bin/mandb ]; then
  # Later Fedora
  mandb %{sge_home}/man
else
  makewhatis -u %{sge_home}/man
fi

%files
# Ensure we can make sgeadmin-owned cell directory
%attr(775,root,%{username}) %dir %{sge_home}
%exclude %{sge_bin}/*/qmon
%exclude %{sge_bin}/*/sge_coshepherd
%exclude %{sge_bin}/*/sge_execd
%exclude %{sge_bin}/*/sge_qmaster
%exclude %{sge_bin}/*/sge_shadowd
%exclude %{sge_bin}/*/sge_shepherd
%if %{with java}
%exclude %{sge_docdir}/javadocs
%exclude %{sge_home}/util/gui-installer
%endif
%exclude %{sge_home}/start_gui_installer
%exclude %{sge_home}/examples/drmaa
%exclude %{sge_mandir}/man1/qmon.1
%exclude %{sge_mandir}/man8/sge_qmaster.8
%exclude %{sge_mandir}/man8/sge_execd.8
%exclude %{sge_mandir}/man8/sge_*shepherd.8
%exclude %{sge_mandir}/man8/sge_shadowd.8
%exclude %{sge_mandir}/man8/pam*8
%exclude %{sge_mandir}/man3
%exclude %{sge_lib}/*/pam*
%if %{with hadoop}
%exclude %{sge_home}/hadoop
%exclude %{sge_lib}/herd.jar
%endif
%exclude %{sge_home}/pvm/src
%exclude %{sge_bin}/process-scheduler-log
%exclude %{sge_home}/util/resources/drmaa4ruby
%{sge_bin}
%{sge_lib}
%doc %{sge_docdir}
%{sge_home}/inst_sge
%{sge_home}/mpi
%{sge_home}/pvm
%{sge_home}/util
%config(noreplace) %{sge_home}/util/sgeCA/*cnf
%config(noreplace) %{sge_home}/util/install_modules/inst_template.conf
%{sge_home}/utilbin
%if 0
%attr(4755,root,root) %{sge_home}/utilbin/*/testsuidroot
%attr(4755,root,root) %{sge_home}/utilbin/*/authuser
# Avoid this for safety, assuming no MS Windows hosts
%attr(4755,root,root) %{sge_home}/utilbin/*/sgepasswd
%endif
%{sge_mandir}/man1/*.1
%{sge_mandir}/man5/*.5
%{sge_mandir}/man8/*.8
%{sge_home}/examples

%files devel
%{sge_include}
%{sge_home}/pvm/src
%{sge_mandir}/man3/*.3
%if %{with java}
%doc %{sge_docdir}/javadocs
%endif
%{sge_home}/examples/drmaa

%files qmon
%{sge_bin}/*/qmon
%{sge_home}/qmon
%{sge_mandir}/man1/qmon.1

%files execd
%{sge_bin}/*/sge_execd
%{sge_bin}/*/sge_shepherd
%{sge_bin}/*/sge_coshepherd
%{sge_home}/install_execd
%{sge_mandir}/man8/sge_execd.8
%{sge_mandir}/man8/sge_*shepherd.8
%{sge_mandir}/man8/pam*8
%{sge_lib}/*/pam*

%files qmaster
%{sge_bin}/*/sge_qmaster
%{sge_bin}/*/sge_shadowd
%{sge_bin}/process-scheduler-log
%{sge_home}/install_qmaster
%{sge_mandir}/man8/sge_qmaster.8
%{sge_mandir}/man8/sge_shadowd.8

%files drmaa4ruby
%{sge_home}/util/resources/drmaa4ruby

%if %{with hadoop}
%files hadoop
%{sge_home}/hadoop
%{sge_home}/lib/herd.jar
%endif

%if %{with java}
%files guiinst
%{sge_home}/util/gui-installer
%{sge_home}/start_gui_installer
%endif


%changelog
* Sun Feb 28 2016 Dave Love <d.love@liverpool.ac.uk> 8.1.9
- Fix OpenSuSE build
- Don't install testsuidroot suid

* Tue Nov 24 2015 Dave Love <d.love@liverpool.ac.uk> 8.1.9
- Bump java version required on el6
- MUNGE support
- Fixes to build on Fedora 23

* Thu Feb  5 2015 Dave Love <d.love@liverpool.ac.uk> 8.1.9pre
- Move spool objects back to main package

* Thu Aug 14 2014 Dave Love <d.love@liverpool.ac.uk> 8.1.8
- Add rpm Epoch on Fedora
- Mark sgeCA/*cnf files as config
- Sanitize requires/provides somewhat
- Require db.h and Xm.h, not packages
- Conditions on ant-nodeps and db devel BRs
- Move spool objects to qmaster package
- Maybe use packaged swing-layout
- Require xterm for execd package (for qsh)

* Wed Jan 22 2014 Dave Love <d.love@liverpool.ac.uk> 8.1.7-1
- Support RHEL7 beta
- Remove -system-libs
- Port to SuSE
- Require xterm for execd (for qsh)

* Fri Aug 16 2013 Dave Love <d.love@liverpool.ac.uk> 8.1.4
- Require /bin/ps for execd, qmaster

* Thu Jul 11 2013 Dave Love <d.love@liverpool.ac.uk> 8.1.4
- Don't use "BuildArch: noarch" on RHEL5 -- fails with un-packaged files errors

* Thu May 30 2013 Dave Love <d.love@liverpool.ac.uk> 8.1.4
- Pass $LDFLAGS to aimk (e.g. hardened build)
- Remove fedora-usermgmt-devel BuildRequires

* Fri May 17 2013 Dave Love <d.love@liverpool.ac.uk> 8.1.4pre-1
- Drop libXpm-devel dependency
- Use bootstrap.sh
- Don't build-require elfutils-libelf-devel
- Fix License and Group headers
- Mark inst_template.conf as %%conf(noreplace)
- Use noarch appropriately
- Don't require java for qmaster.  (Should fail obviously if jvm thread
  configured.)
- Make separate guiinst package.

* Mon Dec 10 2012 Dave Love <d.love@liverpool.ac.uk> - 8.1.3pre-1
- Depend on net-tools (for hostname, at least)
- Define _hardened_build

* Wed Jul 25 2012 Dave Love <d.love@liverpool.ac.uk> - 8.1.2-1
- Make inst_template.conf a config file (from Florian La Roche)
- Don't exclude sge_share_mon

* Mon Jul  2 2012 Dave Love <d.love@liverpool.ac.uk> - 8.1.1-1
- Build-require gcc etc. and not ant-nodeps.
- Move qacct, jobstats.
- Revert last (temporary) change and update version
- Don't compress man pages

* Thu Jun 14 2012 Dave Love <d.love@liverpool.ac.uk> - 8.1.0-2
- Don't require rshd etc. in install script.

* Mon May 14 2012 Dave Love <d.love@liverpool.ac.uk> - 8.1.0
- Add IzPack, swing-layout and build GUI installer.
- Support optional Hadoop build.
- Update License tags.
- Use -no-remote, assuming ssh.

* Thu Apr 12 2012 Dave Love <d.love@liverpool.ac.uk> - 8.0.0e
- Modify for changes in licence info and distinst.
- Consider setuid binaries.

* Wed Feb  1 2012 Dave Love <d.love@liverpool.ac.uk> - 8.0.0e
- Fix --without java

* Sat Oct  8 2011 Dave Love <d.love@liverpool.ac.uk> - 8.0.0pre_c-1
- Depend on hwloc.

* Mon Oct  3 2011 Dave Love <d.love@liverpool.ac.uk> - 8.0.0pre_c-1
- Update version; don't use Packager; fix Source.
- Require man.
- New package drmaa4ruby.
- Move qsched to qmaster package.

* Thu Aug 18 2011 Dave Love <d.love@liverpool.ac.uk> 8.0.0pre_b
- Add Requires: xorg-x11-fonts-ISO8859-1-100dpi xorg-x11-fonts-ISO8859-1-75dpi
  to qmon package [from the EPEL 6 package (Orion Poplawski)].
- Fix libdrmaa symlink [from Florian La Roche].

* Sat Jun 18 2011 Dave Love <d.love@liverpool.ac.uk> 8.0.0a
- Add --without java (adapted from Jesse Becker <hawson@gmail.com>).
- Use csh -f.  Add Prefix.  Use -fno-strict-aliasing.

* Wed May 25 2011 Dave Love <d.love@liverpool.ac.uk> - 8.0.0a-1
- Heavily re-written from Orion Poplawski's Fedora original for shared
  installation under /opt and not doing any configuration.  Different
  enough that it's probably not worh presevring the changelog.
