#/bin/csh
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
set argument1=$1

if ( ! $?SGE_ROOT ) then
   echo "please set SGE_ROOT"
   exit 1
endif

if ( ! $?SGE_QMASTER_PORT ) then
   echo "please set SGE_QMASTER_PORT"
   exit 1
endif

if ( ! $?USER  ) then
   echo "please set SGE_QMASTER_PORT"
   exit 1
endif


if ( $argument1 == "client" ) then
   echo "setting client environment"
   setenv SSL_CA_CERT_FILE "$SGE_ROOT/default/common/sgeCA/cacert.pem"
   setenv SSL_CERT_FILE "/var/lib/sgeCA/port$SGE_QMASTER_PORT/default/userkeys/$USER/cert.pem"
   setenv SSL_KEY_FILE "/var/lib/sgeCA/port$SGE_QMASTER_PORT/default/userkeys/$USER/key.pem"
   setenv SSL_RAND_FILE "/var/lib/sgeCA/port$SGE_QMASTER_PORT/default/userkeys/$USER/rand.seed"
   exit 0
endif

if ( $argument1 == "service" ) then
   echo "setting service environment"
   setenv SSL_CA_CERT_FILE "$SGE_ROOT/default/common/sgeCA/cacert.pem"
   setenv SSL_CERT_FILE "$SGE_ROOT/default/common/sgeCA/certs/cert.pem"
   setenv SSL_KEY_FILE "/var/lib/sgeCA/port$SGE_QMASTER_PORT/default/private/key.pem"
   setenv SSL_RAND_FILE "/var/lib/sgeCA/port$SGE_QMASTER_PORT/default/private/rand.seed"
   setenv SSL_CRL_FILE "$SGE_ROOT/default/common/sgeCA/ca-crl.pem"
   exit 0
endif

echo "usage: certificate.sh client | service"

# $SGE_ROOT/utilbin/$ARCH/openssl s_client -port $SGE_QMASTER_PORT -cert $SSL_CERT_FILE -key $SSL_KEY_FILE
# $SGE_ROOT/utilbin/$ARCH/openssl s_server -port $CL_PORT -cert $SSL_CERT_FILE -key $SSL_KEY_FILE -state -nbio -verify true -CAfile $SSL_CA_CERT_FILE

