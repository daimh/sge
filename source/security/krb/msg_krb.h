#ifndef __MSG_KRB_H
#define __MSG_KRB_H
/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "basis_types.h"
/* 
** krb/src/krb_lib.c 
*/
#define MSG_KRB_KRB5INITCONTEXTFAILEDX_S                 _MESSAGE(53000, _("krb5_init_context failed - "SFN))
#define MSG_KRB_KRB5CCREGISTERFAILEDX_S                  _MESSAGE(53001, _("krb5_cc_register failed - "SFN))
#define MSG_KRB_COULDNOTGETREALMFORQMASTERHOSTXY_SS      _MESSAGE(53002, _("could not get realm for qmaster host "SFN" - "SFN))
#define MSG_KRB_COULDNOTFREEREALMLISTX_S                 _MESSAGE(53003, _("could not free realmlist - "SFN))
#define MSG_KRB_COULDNOTGETDEFAULTREALMX_S               _MESSAGE(53004, _("could not get default realm - "SFN))
#define MSG_KRB_COULDNOTDETERMINEHOSTSORREALFORQMASTER   _MESSAGE(53005, _("could not determine host or realm for qmaster"))
#define MSG_KRB_KRB5PARSENAMEFAILEDX_S                   _MESSAGE(53006, _("krb5_parse_name failed - "SFN))
#define MSG_KRB_KRB5PARSENAMEFAILED                      _MESSAGE(53007, _("krb5_parse_name failed"))
#define MSG_KRB_GETHOSTNAMEFAILED                        _MESSAGE(53008, _("gethostname failed"))
#define MSG_KRB_COULDNOTRESOLVEKEYTABX_S                 _MESSAGE(53009, _("could not resolve keytab - "SFN))
#define MSG_KRB_KRB5SNAMETOPRINCIPALFAILEDX_S            _MESSAGE(53010, _("krb5_sname_to_principal failed - "SFN))
#define MSG_KRB_KRB5SNAMETOPRINCIPAL                     _MESSAGE(53011, _("krb5_sname_to_principal"))
#define MSG_KRB_XCOULDNOTGETDAEMONKEYY_SS                _MESSAGE(53012, _(SFN" could not get daemon key - "SFN))
#define MSB_KRB_CONNECTIONLISTCOULDNOTBECREATED          _MESSAGE(53013, _("connection list could not be created"))
#define MSG_KRB_XCOULDNOTGETSGETICKETUSINGKEYTABY_SS     _MESSAGE(53014, _(SFN" could not get SGE ticket using keytab - "SFN))
#define MSG_KRB_KRBGETNEWAUTHCONFAILUREX_S               _MESSAGE(53015, _("krb_get_new_auth_con failure - "SFN))
#define MSG_KRB_KRBGETNEWAUTHCONFAILED                   _MESSAGE(53016, _("krb_get_new_auth_con failed"))
#define MSG_KRB_KRB5CCDEFAULTFAILEDX_S                   _MESSAGE(53017, _("krb5_cc_default failed - "SFN))
#define MSG_KRB_COULDNOTGETCLIENTCREDENTIALS             _MESSAGE(53018, _("Could not get client credentials"))
#define MSG_KRB_KRB5CCGETPRINCIPALFAILEDX_S              _MESSAGE(53019, _("krb5_cc_get_principal failed - "SFN))
#define MSG_KRB_COULDNOTGETCLIENTPRINCIPAL               _MESSAGE(53020, _("Could not get client principal"))
#define MSG_KRB_INITPACKBUFFERFAILED_S                   _MESSAGE(53021, _("init_packbuffer failed: "SFN))
#define MSG_KRB_CALLERDIDNOTCALLKRBINIT                  _MESSAGE(53022, _("caller did not call krb_init"))
#define MSG_KRB_KRB5TIMEOFDAYFAILEDX_S                   _MESSAGE(53023, _("krb5_timeofday failed - "SFN))
#define MSG_KRB_NOCLIENTENTRYFOR_SSI                     _MESSAGE(53024, _("No client entry for <"SFN","SFN",%d>"))
#define MSG_KRB_COULDNOTGETNEWAUTHCONTEXT                _MESSAGE(53026, _("could not get new auth context"))
#define MSG_KRB_FAILEDCREATINGAP_REQFORWXZY_SSIS         _MESSAGE(53027, _("failed creating AP_REQ for <"SFN","SFN",%d> - "SFN))
#define MSG_KRB_COULDNOTCREATEAUTHENTICATIONINFO         _MESSAGE(53028, _("could not create authentication info"))
#define MSG_KRB_KRB5AUTHCONSETADDRSFIALEDFORWXYZ_SSIS    _MESSAGE(53029, _("krb5_auth_con_setaddrs failed for <"SFN","SFN",%d> - "SFN))
#define MSG_KRB_COULDNOTSETPORTSINAUTHCONTEXT            _MESSAGE(53030, _("could not set ports in auth_context"))
#define MSG_KRB_COULDNOTSETADDRESSESINAUTHCONTEXT        _MESSAGE(53031, _("could not set addresses in auth_context"))
#define MSG_KRB_KRB5GENPORTADDRFAILEDFORWXYZ_SSIS        _MESSAGE(53032, _("krb5_gen_portaddr failed for <"SFN","SFN",%d> - "SFN))
#define MSG_KRB_COULDNOTGENPORTADDR                      _MESSAGE(53033, _("could not gen portaddr"))
#define MSG_KRB_KRB5GENREPLAYNAMEFAILEDFORWXYZ_SSIS      _MESSAGE(53034, _("krb5_gen_replay_name failed for <"SFN","SFN",%d> - "SFN))
#define MSG_KRB_COULDNOTGENREPLAYNAME                    _MESSAGE(53035, _("could not gen replay name"))
#define MSG_KRB_KRB5GETSERVERRCACHEFAILEDFORWXYZ_SSIS    _MESSAGE(53036, _("krb5_get_server_rcache failed for <"SFN","SFN",%d> - "SFN))
#define MSG_KRB_COULDNOTGETREPLAYCACHE                   _MESSAGE(53037, _("could not get replay cache"))
#define MSG_KRB_FAILEDENCRYPTINGMSGFORWXYZ_SSIS          _MESSAGE(53038, _("failed encrypting msg for <"SFN","SFN",%d> - "SFN))
#define MSG_KRB_FAILEDENCRYPTINGMESSAGE                  _MESSAGE(53039, _("failed encrypting message"))
#define MSG_KRB_COULDNOTGETFORWARDABLETGTFORWXYZ_SSIS    _MESSAGE(53041, _("could not get forwardable TGT for <"SFN","SFN",%d> - "SFN))
#define MSG_KRB_UNABLETOFORWARDTGT                       _MESSAGE(53042, _("unable to forward TGT"))
#define MSG_KRB_AUTHENTICATIONFAILURE                    _MESSAGE(53043, _("Authentication failure"))
#define MSG_KRB_DIDNOTCALLKRBINIT                        _MESSAGE(53044, _("did not call krb_init"))
#define MSG_KRB_INVALIDTAGAUTHFAILUREMSGRECEIVEDWXYZ_SSI _MESSAGE(53045, _("Invalid TAG_AUTH_FAILURE msg recvd from <"SFN","SFN",%d>"))
#define MSG_KRB_AUTHENTICATIONTOQMASTERFAILED            _MESSAGE(53046, _("Authentication to qmaster failed"))
#define MSG_KRB_AUTHENTICATIONFAILED                     _MESSAGE(53047, _("Authentication failed"))
#define MSG_KRB_INVALIDMESSAGEUNPACKFAILURE              _MESSAGE(53048, _("Invalid message - unpack failure"))
#define MSG_KRB_FAILEDSENDINGAUTH_FAILUREMESSAGEX_S      _MESSAGE(53049, _("failed sending AUTH_FAILURE message - "SFN))
#define MSG_KRB_INVALIDMESSAGEPACKINGERROR               _MESSAGE(53050, _("Invalid message - packing error"))
#define MSG_KRB_INVALIDMESSAGENOAP_REQ                   _MESSAGE(53051, _("Invalid message - no AP_REQ" ))
#define MSG_KRB_XCOULDNOTCREATEREPLAYCACHEY_SS           _MESSAGE(53052, _(SFN" could not create replay cache - "SFN))
#define MSG_KRB_FAILEDCREATEOFCLIENT                     _MESSAGE(53053, _("failed create of client"))
#define MSG_KRB_APPENDELEMFAILUREX_I                     _MESSAGE(53055, _("lAppendElem failure - %d"))
#define MSG_KRB_CLIENTWXYFAILEDAUTHENTICATIONZ_SSIS      _MESSAGE(53057, _("client <"SFN","SFN",%d> failed authentication - "SFN))
#define MSG_KRB_INVALIDMESSAGEHASANAP_REQ                _MESSAGE(53058, _("Invalid message has an AP_REQ"))
#define MSG_KRB_INVALIDMESSAGERECEIVED                   _MESSAGE(53059, _("Invalid message received"))
#define MSG_KRB_GETHOSTBYNAMEFAILED                      _MESSAGE(53060, _("gethostbyname failed"))
#define MSG_KRB_KRB5AUTHCONSETADDRSFAILEDFORWXYZ_SSIS    _MESSAGE(53061, _("krb5_auth_con_setaddrs failed for <"SFN","SFN",%d> - "SFN))
#define MSG_KRB_UNABLETODECRYPTFORWARDEDTGTFORCLIENTWXYZ_SSIS  _MESSAGE(53062, _("unable to decrypt forwarded TGT for client <"SFN","SFN",%d> - "SFN))
#define MSG_KRB_FAILEDADDINGTGTWTOCLIENTSTGTLISTFORXYZ_ISSI  _MESSAGE(53063, _("Failed adding TGT %d to client's TGT list for <"SFN","SFN",%d>"))
#define MSG_KRB_ILLOGICALFORWARDABLETGTRECEIVEDFROMXYZ_SSI  _MESSAGE(53064, _("illogical forwardable TGT received from <"SFN","SFN",%d>"))

#define MSG_KRB_FAILEDDECRYPTINGMSGFORWXYZ_SSIS             _MESSAGE(53065, _("failed decrypting msg for <"SFN","SFN",%d> - "SFN))
#define MSG_KRB_FAILEDSENDINGAUTH_FAILUREMESSAGE_S       _MESSAGE(53066, _("failed sending AUTH_FAILURE message - "SFN))
#define MSG_KRB_KRB5AUTHCONGETAUTHENTICATORFAILEDFORWXYZ_SSIS  _MESSAGE(53067, _("krb5_auth_con_getauthenticator failed for <"SFN","SFN",%d> - "SFN))
#define MSG_KRB_TGTCREDSHOSTORCOMPROCISNULL              _MESSAGE(53068, _("tgt_creds, host, or comproc is NULL"))
#define MSG_KRB_NOCLIENTENTRYFORXYZ_SSI                  _MESSAGE(53069, _("No client entry for <"SFN","SFN",%d>"))
#define MSG_KRB_NOTGTFOUNDFORWXYWITHIDZ_SSID            _MESSAGE(53070, _("No TGT found for <"SFN","SFN",%d> with ID %ld"))
#define MSG_KRB_NOTCLIENTENTRYFORXYZUNABLETOSTORETGT_SSI _MESSAGE(53071, _("No client entry for <"SFN","SFN",%d> - unable to store TGT"))
#define MSG_KRB_FAILEDSTORINGFORWARDEDTGTFORUIDXJOBYZ_IIS  _MESSAGE(53072, _("failed storing forwarded TGT for uid %d, job %d - "SFN))
#define MSG_KRB_FAILEDDELETINGTGTFORJOBXY_IS             _MESSAGE(53073, _("failed deleting TGT for job %d - "SFN))




/* 
** krb/src/krb_util.c
*/
#define MSG_KRB_COULDNOTDECRYPTTGTFORJOBXY_DS   _MESSAGE(53074, _("could not decrypt TGT for job "sge_U32CFormat" - "SFN))
#define MSG_KRB_COULDNOTRENEWTGTFORJOBXY_DS     _MESSAGE(53075, _("could not renew TGT for job "sge_U32CFormat" - "SFN))
#define MSG_KRB_COULDNOTECRYPTTGTFORJOBXY_DS    _MESSAGE(53076, _("could not encrypt TGT for job "sge_U32CFormat" - "SFN))
#define MSG_KRB_COULDNOTSTORERENEWEDTGTFORXJOBY_SD _MESSAGE(53077, _("could not store renewed TGT for "SFN" - job "sge_U32CFormat))
#define MSG_KRB_COULDNOTGETUSERIDFORXY_SD       _MESSAGE(53078, _("could not get user ID for "SFN" - job "sge_U32CFormat))


#endif /* __MSG_KRB_H  */ 
