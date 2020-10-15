                            What is DRMAA?
                            --------------

Content
-------
1. Introduction
2. Getting Started with the C Binding
3. Getting Started with the Java Binding
4. Copyright

1. Introduction
---------------

   DRMAA is a specification developed by a working group in the Global Grid
   Forum (GGF).  The best way to describe DRMAA is to cite the DRMAA-WG
   Charter:

      "Develop an API specification for the submission and control of jobs
      to one or more Distributed Resource Management (DRM) systems. The
      scope of this specification is all the high level functionality which
      is necessary for an application to consign a job to a DRM system
      including common operations on jobs like termination or suspension. 
      The objective is to facilitate the direct interfacing of applications
      to today's DRM systems by application's builders, portal builders, and
      Independent Software Vendors (ISVs)."

   Simply put, DRMAA is an API for submitting and controlling jobs.  DRMAA
   has been implemented in several languages and for several DRMs.  The Grid
   Engine release includes a C implementation, or C binding, a
   Java[TM] language binding, and a Ruby binding.

   For more information about DRMAA and the various bindings that are
   available, visit the DRMAA website at:

      http://www.drmaa.org/

   There you will find links to the DRMAA specification and mailing list
   archives detailing the thought process that went into DRMAA.

   Information about grid computing standards in general can be found at the
   GGF website:

      http://www.gridforum.org/

   The Ruby DRMAA binding is found in util/resources/drmaa4ruby in the
   SGE installation.

   The Python binding is at https://code.google.com/p/drmaa-python/.

   The Perl language binding module for DRMAA can be found at:

      http://search.cpan.org/src/THARSCH/Schedule-DRMAAc-0.81/

   but this is under the GNU GPL licence, which is incompatible with
   the licence of libdrmaa, despite being produced explicitly for use
   with it.

2. Getting Started with the C Binding
-------------------------------------

   To develop applications that utilize the C binding, you will need two files.
   The first is the DRMAA header file, drmaa.h.  You will find this file under
   the $SGE_ROOT/include directory in the distribution.  You will need to
   include this file from any source files that are to use the DRMAA
   library.  You will also need the DRMAA shared library.  You will find
   this file under the $SGE_ROOT/lib/$ARCH directory in the distribution. 
   This file must be linked with your source files during compilation (make
   ... -ldrmaa ...) and must be accessible to the dynamic linker, e.g.
   via LD_LIBRARY_PATH, for your application to run.

   The first thing to do is to look through the header file.  This file
   lists the functions available to you as a DRMAA developer.  For more
   information on any function you can read the man page for that function. 
   For example:

      % man -M $SGE_ROOT/man drmaa_set_attribute

   The next step is to look at the example program included in the
   $SGE_ROOT/examples/drmaa directory of the distribution.  The example
   program demonstrates a simple usage of the DRMAA library to submit
   several bulk jobs and several single jobs, wait for the jobs to finish,
   and then output the results.

   Also in the $SGE_ROOT/examples/drmaa directory you will find the example
   programs from the online tutorial at:

      http://arc.liv.ac.uk/SGE/howto/drmaa.html

   Once you're familiar with the DRMAA API, you're ready to begin
   development of your application.  Every source file which will call
   functions from the DRMAA library will need to include the line:

      #include "drmaa.h"

   in order for the compiler to find the function definitions.  You may need
   to tell the compiler where to find the DRMAA header file, e.g. by passing
   the
   -I$SGE_ROOT/include to the compiler.

   When compiling your file, you will need to have $SGE_ROOT/lib/$ARCH
   on the linker search path, and indicate to the linker that you want
   to link in this library, e.g. by passing "-ldrmaa" to the
   compiler/linker.


3. Getting Started with the Java Binding
----------------------------------------

   To develop applications that utilize the Java language binding, you will
   need two files.  The first is the jar file, drmaa.jar.  This is needed
   for both compiling and running applications utilizing the Java language
   binding.  The second file is the DRMAA shared library.  This file will
   need to be accessible from the shared library path in order for your
   application to link properly and run.  If you built the binaries yourself
   (using "aimk -java" and "distinst -local") or installed the pre-built
   binaries, you will find the shared library in $SGE_ROOT/lib/$ARCH and the
   jar file in $SGE_ROOT/lib.  Note that the DRMAA shared library is the
   same one used by DRMAA C language binding programs.

   The first step is to look at the example program found at:

      http://arc.liv.ac.uk/repos/darcs/sge/source/libs/jdrmaa/src/DrmaaExample.java

   The example program demonstrates a simple usage of the DRMAA library to
   submit several bulk jobs and several single jobs, wait for the jobs to
   finish, and then output the results.

   In the:

      http://arc.liv.ac.uk/repos/darcs/sge/source/libs/jdrmaa/src/com/sun/grid/drmaa/howto

   directory you will find the example programs from the online tutorial at:

      http://arc.liv.ac.uk/SGE/howto/drmaa_java.html

   API documentation can be found at:

      $SGE_ROOT/doc

   Once you're familiar with DRMAA, you're ready to begin development of your
   Java application.  When compiling your file, you will need to have
   $SGE_ROOT/lib/drmaa.jar included in your CLASSPATH.


4. Copyright
------------
___INFO__MARK_BEGIN__
The Contents of this file are made available subject to the terms of the Sun
Industry Standards Source License Version 1.2

Sun Microsystems Inc., March, 2001

Sun Industry Standards Source License Version 1.2
=================================================

The contents of this file are subject to the Sun Industry Standards Source
License Version 1.2 (the "License"); You may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://gridengine.sunsource.net/Gridengine_SISSL_license.html

Software provided under this License is provided on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.

See the License for the specific provisions governing your rights and
obligations concerning the Software.

The Initial Developer of the Original Code is: Sun Microsystems, Inc.

Copyright: 2001 by Sun Microsystems, Inc.

All Rights Reserved.
___INFO__MARK_END__
