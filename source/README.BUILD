//-*- asciidoc -*-
= Building and Installing Grid Engine
:toc:

== Overview

This document gives you a brief overview of how to compile and
install Grid Engine.

Unfortunately, the build system is home-grown and awkward to use.
See `source/README.aimk` for information on this.

On a Debian- or Red Hat-derived GNU/Linux system, it is recommended
to xref:packages[build dpkg or rpm packages] if suitable binary ones
aren't already available to install, or you don't want to trust them.

The result of a normal build is a tar file which can be used for
installation on a cluster of machines of the same type.

There is a short summary of the process (or recipe for direct local
installation) in the <<quickstart,Quickstart section>>.

== Platform Support

Please report portability problems, preferably with fixes, or success
on platforms not listed here.  This is a summary of current knowledge
of working platforms.

=== GNU/Linux

<<packages,Source packaging>> is available for Debian (and derivatives) and RHEL
but probably needs small adjustment for SuSE.  The included dpkg
packaging is a much better bet than the current official Debian
packaging (which is being worked on); it doesn't currently provide GUI
installer or Hadoop support, unlike the RPM packaging.

[horizontal]
* x86_64/amd64::  Widely used in production.  Should work at least on
  any currently-supported distribution (e.g. Debian stable, RHEL 5 or
  later).  Binary packages are available for RHEL 5 and 6, and Debian
  Wheezy.
* Other GNU/Linux architectures::  Debian packages are available for
  armel; x86 is known to work; other architectures should work.  Build
  problems are most likely to be due to naming conventions for
  architectures (`uname -m`) and Java locations.

Some support libraries are available in the
http://arc.liv.ac.uk/downloads/SGE/support/[distribution area] in case
they aren't available from your OS.

=== Solaris

Recent versions have built on Solaris 10 with GCC or the system
compiler and should do so on Solaris 11.  Requires add-on library
support for openssl, Berkeley DB, and hwloc, e.g. from
http://www.opencsw.org/[OpenCSW].  You need `/usr/ccs/bin` on your path.  You
need to use GCC (`aimk -gcc`) to build against the CSW hwloc package.

=== MS Windows

==== Interix/SUA/SFU

The <<windowsbuild,Windows build>> is supposed to work with Server
2003 Release 2, Server 2008, Vista Enterprise, and Vista Ultimate
using Subsystem for UNIX-based Applications; also for Server 2003, XP
Professional with at least Service Pack 1, Windows 2000 Server with at
least Service Pack 3, or Windows 2000 Professional with at least
Service Pack 3 using Services for UNIX.  `qmaster`, `qmon`, DRMAA, and
`qsh` are not supported.

==== Cygwin

32-bit builds and clients work, but there may be problems with daemons
(tested on Windows 7), apparently related to threading.  The native
components as in the Interix build are currently not supported, and
neither are.  A 64-bit version builds.

=== FreeBSD, NetBSD, DragonFly, OpenBSD

Recent NetBSD is known to work.  Support is present for FreeBSD and
OpenBSD, with all known available patches imported.  Binding/topology
support via hwloc may not work.

=== Darwin (Apple Mac OS X)

Support is present for recent versions (10.4+?) on x86 and Power.

=== AIX

Support is present for version 5.1 and above.

=== HP-UX

Support is present for version 11, 32- or 64-bit.

=== Tru64

Support is present for Tru64 5.  It is not known to be tested, and
will probably be removed with final HP support for Tru64 ceased.

=== Irix

Support is present for Irix 6.5.  It is not known to be tested, and
may be removed with final SGI support for Irix finishing, although
the system has some interesting management features for which the
support code may be kept as an example.

=== Others

Support for obsolete NEC SX, (`classic') Cray, and vestigial bits for
other systems has been removed, but could conceivably be revived from
the repository.  (SX had possibly-interesting resource management
features like Irix.)

New ports require at least support in the       `arch`, `compilearch`,
and `aimk` scripts, but may need other substantial changes, depending
on how similar they are to an existing port.

See `source/dist/README.arch` for how the architecture is determined
and used in the build and installation processes.

== Prerequisites

For up-to-date source, see <https://arc.liv.ac.uk/trac/SGE>, and for
more information, see <http://arc.liv.ac.uk/SGE>.

The following are requirements for building from source and installing
the result.  If you are on a Debian- or Red Hat-ish system and aren't
building packages for some reason, check +gridengine.spec+ and
+debian/control+ in the sources for the names of packages to satisfy
build dependencies mentioned below.

* A version of `csh` (e.g. http://www.tcsh.org/Welcome[tcsh]) and the
  normal packages for building C source on your system;
* GNU autoconf if you are building from the repository rather than
  a distribution;
* It is highly recommended to use CSP security or MUNGE authentication
  <http://arc.liv.ac.uk/SGE/howto/sge-security.html>.  For CSP you
  need the http://www.openssl.org/[openSSL library], preferably your OS's
  packaged version; without it, build with `aimk -no-secure`.
  To provide http://munge.googlecode.com/[MUNGE authentication],
  install the library and daemon from OS packages or the MUNGE web
  site.  It needs to   be version 5.9+ to get a compatible licence.
  Use `aimk --with-munge`;
* The http://www.canonware.com/jemalloc/[jemalloc library] can be
  linked using `aimk -with-jemalloc` to improve qmaster performance if
  that is a concern.
* For BDB spooling in qmaster, the
  http://www.oracle.com/technetwork/database/berkeleydb/overview/[Berkeleydb
  library]; otherwise build with `aimk
  -spool-classic`.  Note that the BDB server mode is no longer supported;
* Possibly development packages of `ncurses` and (on
  GNU/Linux) `pam`;
* For the `qmon` GUI,
  development packages of `lesstif` or `openmotif` (now free
  software, and appearing in distributions), `libXmu`, and `libXpm`;
* On AIX, `perfstat` (the `bos.perf.libperfstat` and
  `bos.perf.perfstat` filesets);
* For core binding support,
  http://www.open-mpi.org/projects/hwloc/[hwloc], at least version
  1.1, and possibly later for specific architecture support; otherwise
  build with `aimk -no-hwloc`;
* For the basic Java targets, as well as a Java 1.6 or 1.7 JDK
  (e.g. OpenJDK) you will need `ant`, `ant-nodeps` (Red Hat) or
  `ant-optional` (Debian), `javacc`, and `junit` packages.  The GUI
  installer requires http://izpack.org[IzPack] and
  http://swing-layout.dev.java.net[swing-layout].  Izpack 4.1.1 is
  known to work, later versions may not; swing-layout 1.0.3 is known
  to work.  Copies are available from
  <http://arc.liv.ac.uk/downloads/SGE/support/> and are included in
  the source RPM.
+
The `herd` library
requires a basic Hadoop distribution (hadoop-0.20 GNU/Linux
packages) of a suitable version
(e.g. <http://archive.cloudera.com/cdh/3/>).  The cdh3u3
(0.20.2+923.197) and cdh3u4 (0.20.2+923.256) versions are known to
work and cdh3u0 is known not to with this version of
SGE.  (There was an incompatible change in the Hadoop distribution,
and support for earlier versions can be found in the repository for
sge-8.0.0e and earlier.)
+
Properties for ant are set in the
top-level `build.properties`, and may be overridden in
`build_private.properties`.  Other properties you might need to set
are typically found in `nbproject/project.properties` in each Java
component directory;

* For requirements for the Interix MS Windows build, see <<Windows
  build,below>>;

* For the Cygwin MS Windows build, the packages required over the base
  system are at least: tcsh, make, gcc-core, openssl-devel,
  libdb*-devel (where * may be a version number or null), crypt (or
  libcrypt-devel, depending on version), libncurses-devel,
  libXm-devel, libXext-devel, libXmu-devel (the last three for qmon);
  also build and install hwloc from
  <http://www.open-mpi.org/software/hwloc/> or use `aimk -hwloc` (see
  below) to avoid core binding support.

If in doubt for other systems, maybe consult and adapt the Red Hat recipe
given by the `%build` section of `gridengine.spec` at the top level of the
source directory.

== Building

The following commands assume that your current working directory is
++sge-++__version__++/source++ from unpacking the source distribution
tarball, and that you are using a Bourne-like shell.

A summary of the build procedure is:

* Build the dependency tool and create dependencies with `aimk`;

* Compile with `aimk`;

* Build a repository of the distribution with `distinst`;

* Build a distribution tarball from the repository with `mk_dist`.

=== Setup and make Dependencies

Run

  $ sh scripts/bootstrap.sh

to fix file permissions, build the `sge_depend` tool for finding
header dependencies for `make` (`aimk -only-depend`), create initial
dependency files (`scripts/zerodepend`) run `sge_depend` (`aimk
depend`), and run the autoconf `configure` scripts.

See source/3rdparty/sge_depend/sge_depend.html for more information
about `sge_depend`.
Running it may give ignorable warnings.

[NOTE]
Dependencies are not recreated automatically if source files
change, and are shared between targets in the build area.

=== Compilation

To compile, run

  $ ./aimk

This tries to build all the normal targets, some of which might be
problematic for various reasons (e.g. Java).  Various `aimk` switches
provide selective compilation; use `./aimk -help` for the options, not
all of which may actually work, especially in combination.

   Useful aimk options:

[horizontal]
`-no-qmon`:: Don't build `qmon`;
`-no-qmake`:: don't build `qmake`;
`-no-qtcsh`:: don't build `qtcsh`;
`-no-java -no-jni`:: avoid all Java-related stuff;
`-no-remote`:: don't build `rsh` etc.
(obsoleted by use of `ssh` and the SGE PAM module).

For the core system (daemons, command line clients, but not `qmon`) use

  $ ./aimk -only-core

Building creates a sub-directory named with an uppercase architecture
string (e.g. `LINUXAMD64`), into which binaries are written.  Similar
directories are created in the `3rdparty` directory.  Java components
are built into the `CLASSES` sub-directory.

At least on the supported and tested platforms, problems with compilation
most likely will be related to the required compiler (often the operating
system compiler is used and other compilers like gcc are not supported or
not well tested) or compiler version, partially to the default memory
model on your machine (32 or 64bit). Usually these problems can be solved
by tuning aimk.

=== Man pages and qmon help file

To create man pages run

  $ ./aimk -man

=== Build Staging

Use the script `scripts/distinst` to stage the binaries to a
distribution directory from which a tarball or package may be created
or (with the `-local` flag) install the binaries directly under
`$SGE_ROOT`.  See `scripts/README.distinst` for details.

==== Direct Installation

It may be convenient to install directly from the build area.  To do
so, create a directory for the installation (e.g. `/opt/sge`), set the
environment variable `SGE_ROOT` to its name and export the variable.
Then run
....
  $ echo y | scripts/distinst -local -noexit -allall <arch1> <arch2> ...
....
This probably needs to be done as root, at least if the setuid
binaries are required.      `-noexit` avoids exiting if any part of
the distribution hasn't been built, and `-allall` copies everything,
including documentation and binaries for the built architectures
`<arch1>` etc., to `$SGE_ROOT`.

==== Distribution Staging

To create a distribution tarball, the first step is to install the
files into a `staging' directory with +distinst+, e.g.
....
  $ echo y | scripts/distinst -allall -basedir `pwd`/dist -vdir 8.1.4 -noexit -noopenssl -nobdb
....

=== Creating Distribution Tarball

The script `scripts/mk_dist` can be used to create distribution
tarballs from the staging area.  See its `-help` option and
scripts/README.mk_dist` for more information.  E.g.

....
$ scripts/mk_dist -basedir `pwd`/dist -vdir 8.1.4 -version 8.1.4 -bin
$ scripts/mk_dist -basedir `pwd`/dist -vdir 8.1.4 -version 8.1.4 -common
....

== Set Up

After installing the distribution into place you need to run the
installation script `inst_sge` (or a wrapper) on your master host, and
on all execution hosts which don't share the installation via a
distributed file system.  This will configure and start up the
daemons.  If the installation is shared with the execution hosts,
using a shared, writable spool directory, it isn't necessary to do an
explicit install on them.  Use `inst_sge` to install the execd on the
master host and copy to or share with the hosts the `sgeexecd` init
script.  Then start this after the qmaster has been configured with
the relevant host names (with `qconf -ah`).  You may or may not want
to keep the execd on the master host for maintaining global load
values, for instance, but you probably want to ensure it has zero
slots, so as not to be able to run jobs.

== Quickstart Installation[[quickstart]]

With the build dependencies in place, the following procedure will
build and install on the local system on a supported platform under
a Bourne-like shell:
....
sh scripts/bootstrap.sh
./aimk -system-libs # maybe add -pam for PAM, and -no-herd to avoid Hadoop
./aimk -man
SGE_ROOT=/opt/sge
export SGE_ROOT
scripts/distinst -local -allall -noexit # asks for confirmation
cd $SGE_ROOT
./inst_sge -m -x -csp  # or run ./start_gui_installer
....
If you haven't built all the binaries (e.g. used `aimk -only-core`),
add the option `-nobincheck` to `inst_sge` to avoid a check for all
the possible binaries.

== Building on MS Windows with SUA/SFU[[windowsbuild]]

These are notes from building 8.1.3+ on 64-bit MS ``Windows 7
Enterprise'' in the ``normal'' way with the ``SUA'' system.  The
procedure will be different for installing the basic environment with
the earlier ``SFU'' system in MS Windows XP.  +aimk+ may well need
hacking for other versions, as well as +aimk.site+.

The initial SUA installation is via:
====
Control panel -> Programs and Features -> Turn Windows features on
and off -> Subsystem for UNIX-based applications
====

There is a parallel item to enable the NFS client, which you may want
on execution hosts.

Then go to ``Download utilities ...'' from the SUA item in the ``All
Programs'' menu and install it.  You need the GNU utilities
and SDK components.    To build CSP support, you need openssl.  There isn't
currently an official distribution for Interix, so either build it
yourself, or use the somewhat old version from
<http://arc.liv.ac.uk/downloads/SGE/support/>.  `aimk.site` expects it
to be unpacked into `/usr/local/ssl`.

You also need the native compiler from Microsoft Visual C++.  (It may
be possible to use the free MinGW compiler, but that has not been
tried.)  You can
use the gratis ``Express Edition'', which appears to be available from
<https://www.microsoft.com/visualstudio/eng/products/visual-studio-express-products>.  The tested procedure here was with ``Visual Studio 9.0'' ``2008
Express Edition''.  There's a crucial, obscure step necessary to be
able to run the compiler;  without it, `cl.exe` won't run (or the
Interix `/bin/ar`), and you won't see why not (a missing dll) unless you
run the binary explicitly, i.e. not via the path.  Assuming the
default location for the VC installation of that version, then the
PATH environment variable must have the _two_ components:

  PATH="/dev/fs/C/Program Files (x86)/Microsoft Visual Studio 9.0/VC/bin:/dev/fs/C/Program Files (x86)/Microsoft Visual Studio 9.0/Common7/IDE:$PATH"

The latter is where mspdb80.dll (in this version) resides.  These are
set by default in aimk.site.  On a 32-bit system, remove ` (x86)` from
the paths.

It's similarly non-obvious that `/bin/ar` won't work (at least in this
64-bit version for 32-bit objects) without that path or by supplying
option `-m x86`.

The 32-bit OpenSSL library should be in `/usr/local/ssl/lib/x86`.  It
is set up to link statically in the `aimk` files.  The hwloc library
isn't currently available on Interix, so core binding and topology
information isn't available.

It isn't currently clear whether a 64-bit Interix version can be built.

== Building Debian or Red Hat packages[[packages]]

Packaging is available for building binary packages for Debian and
derived distributions (like Ubuntu), and for Red Hat-derived
distributions (RHEL, Fedora, CentOS, SL).  SuSE and other RPM-based
distributions aren't currently support, but patches for the RPM spec
file would be welcome.

The binary packages will install into `/opt/sge`, intended for shared
installations, although they will also work with a local installation.

=== dpkg

See <http://wiki.debian.org/BuildingAPackage> for general help on
building Debian packages.  It is intended that the
http://wiki.debian.org/Hardening#hardening-wrapper[hardening-wrapper]
package is used, but it isn't required by the build dependencies.

* Ensure the `dpkg-dev` package is installed

* Run
+
....
  $ tar fx sge-<version>.tar.gz; cd sge-<version>
....

* Run
+
....
  $ dpkg-buildpackage -b
....
* If necessary, satisfy build dependencies:
+
....
  $ sudo apt-get install <missing packages>
  $ dpkg-buildpackage -b
....
+
If you run lintian, e.g. via `debuild` instead of `dpkg-buildpackage`,
recent versions will complain bitterly about use of `/opt`,
unfortunately.

=== RPM

* Ensure the `rpm-build` package is installed.  You may (RHEL5,
  typically) or may not (RHEL6) need to be root to run `rpmbuild`.

* Run
+
....
  $ rpmbuild --rebuild gridengine-<version>.src.rpm
....
* If necessary, satisfy build dependencies:
+
....
  $ sudo yum install <missing packages>
  $ rpmbuild --rebuild gridengine-<version>.src.rpm
....
To build the Hadoop support after (installing the `hadoop-0.20`
package from the http://archive.cloudera.com/redhat/cdh/3/[Cloudera
CDH3 repository] add `--with hadoop` to the `rpmbuild` command, or to
avoid the Java components, add `--without java`.

[float]
{nbsp}
------
Copyright (C) 2013, Dave Love, University of Liverpool

Licence http://arc.liv.ac.uk/repos/darcs/sge/LICENCES/GFDL-1.3[GFDL].
