#!/bin/sh
#
# make_obj_rules <output_file> <object names>...
# generates make rules for the given objects
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

current=""
output=""
allobjs=""


writeIDL() {
   echo ${current}.idl:: culltrans >> $output
   echo '	$(CULLTRANS) -idl $(CULLSRC) -oo ' $current >> $output
   echo >> $output

   echo ${current}.idl:: '$(CULLSRC)' >> $output
   echo '	$(CULLTRANS) -idl $(CULLSRC) -of $? -oo ' $current >> $output
   echo >> $output
}

writeIMPLEMENTH() {
   echo ${current}_implementation.h:: culltrans >> $output
   echo '	$(CULLTRANS) -hdr $(CULLSRC) -oo ' $current >> $output
   echo >> $output

   echo ${current}_implementation.h:: '$(CULLSRC)' >> $output
   echo '	if [ ! -s $@ ]; then \' >> $output
   echo '      $(CULLTRANS) -hdr $(CULLSRC) -of $? -oo ' $current '; \' >> $output
   echo '   fi' >> $output
   echo >> $output
}

writeIMPLEMENTC() {
   echo ${current}_implementation.cpp:: culltrans >> $output
   echo '	$(CULLTRANS) -impl $(CULLSRC) -oo ' $current >> $output
   echo >> $output

   echo ${current}_implementation.cpp:: '$(CULLSRC)' >> $output
   echo '	if [ ! -s $@ ]; then \' >> $output
   echo '      $(CULLTRANS) -impl $(CULLSRC) -of $? -oo ' $current '; \' >> $output
   echo '   fi' >> $output
   echo >> $output
}

writeOBJ() {
   echo ${current}.o: ${current}.cpp ${current}.h basic_types.h elem_codes.h >> $output
   echo '	$(CXX) $(CXXFLAGS) -c ' ${current}.cpp >> $output
   echo >> $output
}

writeSKELO() {
   echo ${current}_skel.o: ${current}_skel.cpp ${current}_skel.h basic_types_skel.h '\' >> $output
   echo '                     ' ${current}.h basic_types.h elem_codes.h >> $output
   echo '	$(CXX) $(CXXFLAGS) $(NOERR_CFLAG) -c ' ${current}_skel.cpp >> $output
   echo >> $output
}

writeIMPLEMENTO() {
   echo ${current}_implementation.o: ${current}_implementation.cpp ${current}_implementation.h ${current}_skel.h '\' >> $output
   echo '                     ' basic_types_skel.h ${current}.h basic_types.h elem_codes.h >> $output
   echo '	$(CXX) $(CXXFLAGS) $(NOERR_CFLAG) -c ' ${current}_implementation.cpp >> $output
   echo >> $output
}

writeIMPLO() {
   echo ${current}_impl.o: ${current}_impl.cpp ${current}_impl.h ${current}_implementation.h '\' >> $output
   echo '                     ' ${current}_skel.h basic_types_skel.h ${current}.h basic_types.h '\' >> $output
   echo '                     ' elem_codes.h Master_impl.h >> $output
   echo '	$(CXX) $(CXXFLAGS) $(NOERR_CFLAG) -c ' ${current}_impl.cpp >> $output
   echo >> $output
}

writeHDR() {
   #echo ${current}.h: ${current}.idl >> $output
   dep=""
   x=""
   for x in $allobjs; do
      dep="$dep ${x}.idl"
   done
   echo ${current}.h: basic_types.idl elem_codes.idl $dep >> $output
   echo '	$(IDL) $(IDLFLAGS) ' ${current}.idl >> $output
   echo ${current}.cpp ${current}_skel.cpp ${current}_skel.h: ${current}.h >> $output
   echo >> $output
   echo ${current}_impl.h: ${current}_implementation.h >> $output
   echo >> $output
}

#---------------------
# main()
#---------------------
if [ $# -lt 3 ]; then
   echo Usage: $0 output-file-name object-names...
   exit 1
fi

# store output file name and truncate it
output=$1
shift
rm -f $output
touch $output
allobjs=$*

# write out all the stuff
for i in $*; do
   current=$i
   writeIDL
   writeIMPLEMENTH
   writeIMPLEMENTC
   writeOBJ
   writeSKELO
   writeIMPLEMENTO
   writeIMPLO
   writeHDR
done
exit 0
