#ifndef __MSG_COMMLIB_H
#define __MSG_COMMLIB_H
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "basis_types.h"

#define MSG_CL_CRM_ERROR_MESSAGE1_SS                                          _MESSAGE(85000, _("server host resolves source host "SFQ" as "SFQ""))
#define MSG_CL_CRM_ERROR_MESSAGE2_SS                                          _MESSAGE(85001, _("server host resolves destination host "SFQ" as "SFQ""))
#define MSG_CL_CRM_ERROR_MESSAGE3_SS                                          _MESSAGE(85002, _("server host resolves rdata host "SFQ" as "SFQ""))
#define MSG_CL_CRM_ERROR_MESSAGE4_SS                                          _MESSAGE(85003, _("client IP resolved to host name "SFQ". This is not identical to clients host name "SFQ""))

/* cl_tcp_framework.c */

#define MSG_CL_TCP_FW_SOCKET_ERROR                                            _MESSAGE(85004, _("call to socket(AF_INET, SOCK_STREAM,0) returned value < 0"))
#define MSG_CL_TCP_FW_SETSOCKOPT_ERROR                                        _MESSAGE(85005, _("call to setsockopt() failed setting SO_REUSEADDR"))
#define MSG_CL_TCP_FW_FCNTL_ERROR                                             _MESSAGE(85006, _("call to fcntl() failed setting O_NONBLOCK"))
#define MSG_CL_TCP_FW_CANT_RESOLVE_HOST_S                                     _MESSAGE(85007, _("can't resolve hostname "SFQ""))
#define MSG_CL_TCP_FW_ADDR_NAME_RESOLVE_HOST_ERROR_SSSS                       _MESSAGE(85008, _("reverse mapping of IP "SFQ" returns "SFQ" while name resolving returns "SFQ" for host "SFQ""))
#define MSG_CL_TCP_FW_CONNECT_TIMEOUT                                         _MESSAGE(85013, _("connect timeout error"))
#define MSG_CL_TCP_FW_EMPTY_DESTINATION_HOST                                  _MESSAGE(85016, _("got empty destination host name from connected client"))
#define MSG_CL_TCP_FW_CANT_RESOLVE_DESTINATION_HOST_S                         _MESSAGE(85018, _("can't resolve destination hostname "SFQ""))
#define MSG_CL_TCP_FW_REMOTE_DESTINATION_HOSTNAME_X_NOT_Y_SS                  _MESSAGE(85019, _("remote destination host name "SFQ" is not equal to local resolved host name "SFQ""))
#define MSG_CL_TCP_FW_CANT_RESOLVE_RDATA_HOST_S                               _MESSAGE(85020, _("can't resolve rdata hostname "SFQ""))
#define MSG_CL_TCP_FW_EMPTY_RDATA_HOST                                        _MESSAGE(85021, _("got empty rdata host name from connected client" ))
#define MSG_CL_TCP_FW_REMOTE_RDATA_HOSTNAME_X_NOT_Y_SS                        _MESSAGE(85022, _("remote rdata host name "SFQ" is not equal to local resolved host name "SFQ""))
#define MSG_CL_TCP_FW_IP_ADDRESS_RESOLVING_X_NOT_Y_SS                         _MESSAGE(85023, _("IP based host name resolving "SFQ" doesn't match client host name from connect message "SFQ""))
#define MSG_CL_TCP_FW_CANT_RESOLVE_CLIENT_IP                                  _MESSAGE(85024, _("can't resolve client IP address"))
#define MSG_CL_TCP_FW_EMPTY_REMOTE_HOST                                       _MESSAGE(85025, _("got no remote host name from client"))
#define MSG_CL_TCP_FW_ENDPOINT_X_DOESNT_MATCH_Y_SSUSSU                        _MESSAGE(85026, _("local endpoint \""SFN"/"SFN"/"sge_U32CFormat"\" doesn't match requested endpoint \""SFN"/"SFN"/"sge_U32CFormat"\""))
#define MSG_CL_TCP_FW_ENDPOINT_X_ALREADY_CONNECTED_SSU                        _MESSAGE(85027, _("endpoint \""SFN"/"SFN"/"sge_U32CFormat"\" is already connected"))
#define MSG_CL_TCP_FW_ALLOWED_HOST_LIST_NOT_DEFINED                           _MESSAGE(85028, _("no allowed host list defined"))
#define MSG_CL_TCP_FW_HOST_X_NOT_IN_ALOWED_HOST_LIST_S                        _MESSAGE(85029, _("host "SFQ" is not in allowed host list"))
#define MSG_CL_TCP_FW_CONNECTION_STATUS_TEXT_OK                               _MESSAGE(85030, _("ok"))
#define MSG_CL_TCP_FW_CONNECTION_STATUS_TEXT_HOSTNAME_RESOLVING_ERROR         _MESSAGE(85031, _("hostname resolving error"))
#define MSG_CL_TCP_FW_CONNECTION_STATUS_TEXT_COMPONENT_NOT_FOUND              _MESSAGE(85032, _("requested component not found"))
#define MSG_CL_TCP_FW_CONNECTION_STATUS_TEXT_ENDPOINT_NOT_UNIQUE_ERROR        _MESSAGE(85033, _("already connected - endpoint not unique error"))
#define MSG_CL_TCP_FW_CONNECTION_STATUS_TEXT_CLIENT_NOT_IN_ALLOWED_HOST_LIST  _MESSAGE(85034, _("client is not in allowed host list"))
#define MSG_CL_TCP_FW_RESERVED_SOCKET_ERROR                                   _MESSAGE(85035, _("call to rresvport() returned value < 0"))
#define MSG_CL_TCP_FW_RESERVED_PORT_CONNECT_ERROR                             _MESSAGE(85036, _("debug client did not use a reserved port < 1024"))
#define MSG_CL_TCP_FW_ENDPOINT_X_NOT_FROM_RESERVED_PORT_SSU                   _MESSAGE(85037, _("debug client \""SFN"/"SFN"/"sge_U32CFormat"\" did not use a reserved port below 1024"))
#define MSG_CL_TCP_FW_ENDPOINT_X_NOT_FROM_LOCAL_HOST_SSUS                     _MESSAGE(85038, _("debug client \""SFN"/"SFN"/"sge_U32CFormat"\" is not running on host "SFQ""))
#define MSG_CL_TCP_FW_LOCAL_HOST_CONNECT_ERROR                                _MESSAGE(85039, _("debug client is not running on local host"))
#define MSG_CL_TCP_FW_STANDARD_ENDPOINT_X_NOT_FROM_RESERVED_PORT_SSU          _MESSAGE(85040, _("client \""SFN"/"SFN"/"sge_U32CFormat"\" did not use a reserved port below 1024"))
#define MSG_CL_TCP_FW_SSL_CONNECT_TIMEOUT                                     _MESSAGE(85041, _("connect timeout error"))
#define MSG_CL_SSL_FW_OPEN_SSL_CRYPTO_FAILED                                  _MESSAGE(85042, _("Unable to open the OpenSSL library.  Please make sure libssl is accessible from your shared library path."))
#define MSG_CL_SSL_FW_LOAD_CRYPTO_SYMBOL_FAILED                               _MESSAGE(85043, _("Unable to load symbol from libssl."))

#define MSG_CL_COMMLIB_CLOSING_SSU                                            _MESSAGE(85044, _("closing \""SFN"/"SFN"/"sge_U32CFormat"\""))
#define MSG_CL_COMMLIB_COMPILE_SOURCE_WITH_LARGER_FD_SETSIZE                  _MESSAGE(85045, _("compile source code with larger FD_SETSIZE value"))
#define MSG_CL_COMMLIB_CHECK_SSL_CERTIFICATE                                  _MESSAGE(85046, _("please check certificate validity"))
#define MSG_CL_COMMLIB_HOSTNAME_EXEEDS_MAX_HOSTNAME_LENGTH_SU                 _MESSAGE(85047, _("hostname "SFQ" exceeds MAXHOSTNAMELEN(="sge_U32CFormat")"))
#define MSG_CL_COMMLIB_SSL_ERROR_USS                                          _MESSAGE(85048, _("SSL error(nr.:"sge_U32CFormat") in "SFN": "SFN))


#define MSG_CL_COMMLIB_SSL_CLIENT_CERTIFICATE_ERROR   _MESSAGE(85049, _("client certificate doesn't verify"))
#define MSG_CL_COMMLIB_SSL_PEER_CERT_GET_ERROR        _MESSAGE(85051, _("cannot get peer certificate"))
#define MSG_CL_COMMLIB_SSL_USER_ID_VERIFY_ERROR       _MESSAGE(85052, _("user id doesn't verify"))
#define MSG_CL_COMMLIB_SSL_USER_ID_GET_ERROR          _MESSAGE(85053, _("cannot get user id"))
#define MSG_CL_COMMLIB_SSL_CLIENT_CERT_NOT_SENT_ERROR _MESSAGE(85054, _("client did not send peer certificate"))
#define MSG_CL_COMMLIB_SSL_HANDSHAKE_ERROR            _MESSAGE(85055, _("SSL handshake error"))
#define MSG_CL_COMMLIB_SSL_VERIFY_CALLBACK_FUNC_ERROR _MESSAGE(85056, _("commlib ssl verify callback function failed"))
#define MSG_CL_COMMLIB_SSL_SERVER_CERT_NOT_SENT_ERROR _MESSAGE(85058, _("service did not send peer certificate"))
#define MSG_CL_COMMLIB_SSL_MESSAGE_SIZE_EXEED_ERROR   _MESSAGE(85059, _("message size exceeds integer size on this architecture"))
#define MSG_CL_COMMLIB_SSL_ACCEPT_TIMEOUT_ERROR_S     _MESSAGE(85060, _("ssl accept timeout for client "SFQ""))
#define MSG_CL_COMMLIB_SSL_ACCEPT_TIMEOUT_ERROR       _MESSAGE(85061, _("ssl accept timeout for unresolvable client"))
#define MSG_CL_COMMLIB_SSL_ACCEPT_ERROR_S             _MESSAGE(85062, _("ssl accept error for client "SFQ""))
#define MSG_CL_COMMLIB_SSL_ACCEPT_ERROR               _MESSAGE(85063, _("ssl accept error for unresolvable client"))

#define MSG_CL_COMMLIB_NO_ADDITIONAL_INFO             _MESSAGE(85064, _("no additional information available"))
#define MSG_CL_COMMLIB_CANT_SWITCH_THREAD_MODE_WITH_EXISTING_HANDLES _MESSAGE(85065, _("can't switch commlib thread mode while communication handles are defined"))
#define MSG_CL_COMMLIB_SSL_ERROR_NR_AND_TEXT_USS      _MESSAGE(85066, _("[ID="sge_X32CFormat"] in module "SFQ": "SFQ))
#define MSG_CL_COMMLIB_CANNOT_DUP_SOCKET_FD           _MESSAGE(85067, _("cannot dup socket fd to be larger or equal 3"))

#define MSG_CL_COMMLIB_SSL_ERROR_151441508            _MESSAGE(85500, _("certificate file contains a bad certificate"))
#define MSG_CL_COMMLIB_SSL_ERROR_33558541             _MESSAGE(85501, _("please check the permissions of the pem certificate file"))
#define MSG_CL_COMMLIB_SSL_ERROR_336151573            _MESSAGE(85502, _("the used certificate is expired"))
#define MSG_CL_COMMLIB_SSL_ERROR_336105650            _MESSAGE(85503, _("the used certificate is expired or invalid"))












#endif /* __MSG_COMMLIB_H */
