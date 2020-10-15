#ifndef __MSG_COMMLISTSLIB_H
#define __MSG_COMMLISTSLIB_H
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

#define MSG_CL_RETVAL_OK                          _MESSAGE(80000, _("no error happened"))
#define MSG_CL_RETVAL_MALLOC                      _MESSAGE(80001, _("can't allocate memory"))
#define MSG_CL_RETVAL_PARAMS                      _MESSAGE(80002, _("got unexpected parameters"))
#define MSG_CL_RETVAL_UNKNOWN                     _MESSAGE(80003, _("can't report a reason"))
#define MSG_CL_RETVAL_MUTEX_ERROR                 _MESSAGE(80004, _("got general mutex error"))
#define MSG_CL_RETVAL_MUTEX_CLEANUP_ERROR         _MESSAGE(80005, _("can't cleanup mutex"))
#define MSG_CL_RETVAL_MUTEX_LOCK_ERROR            _MESSAGE(80006, _("can't lock mutex"))
#define MSG_CL_RETVAL_MUTEX_UNLOCK_ERROR          _MESSAGE(80007, _("can't unlock mutex"))
#define MSG_CL_RETVAL_CONDITION_ERROR             _MESSAGE(80008, _("got general thread condition error"))
#define MSG_CL_RETVAL_CONDITION_CLEANUP_ERROR     _MESSAGE(80009, _("can't cleanup thread condition"))
#define MSG_CL_RETVAL_CONDITION_WAIT_TIMEOUT      _MESSAGE(80010, _("timeout while waiting for thread condition"))
#define MSG_CL_RETVAL_CONDITION_SIGNAL_ERROR      _MESSAGE(80011, _("received a signal while waiting for thread condition"))
#define MSG_CL_RETVAL_THREAD_CREATE_ERROR         _MESSAGE(80012, _("can't create thread"))
#define MSG_CL_RETVAL_THREAD_START_TIMEOUT        _MESSAGE(80013, _("timeout while waiting for thread start"))
#define MSG_CL_RETVAL_THREAD_NOT_FOUND            _MESSAGE(80014, _("can't find thread"))
#define MSG_CL_RETVAL_THREAD_JOIN_ERROR           _MESSAGE(80015, _("got thread join error"))
#define MSG_CL_RETVAL_THREAD_CANCELSTATE_ERROR    _MESSAGE(80016, _("got unexpected thread cancel state"))
#define MSG_CL_RETVAL_LOG_NO_LOGLIST              _MESSAGE(80017, _("no log list found"))
#define MSG_CL_RETVAL_CONNECTION_NOT_FOUND        _MESSAGE(80018, _("can't find connection"))
#define MSG_CL_RETVAL_HANDLE_NOT_FOUND            _MESSAGE(80019, _("can't find handle"))
#define MSG_CL_RETVAL_THREADS_ENABLED             _MESSAGE(80020, _("threads are enabled"))
#define MSG_CL_RETVAL_NO_MESSAGE                  _MESSAGE(80021, _("got no message"))
#define MSG_CL_RETVAL_CREATE_SOCKET               _MESSAGE(80022, _("can't create socket"))
#define MSG_CL_RETVAL_CONNECT_ERROR               _MESSAGE(80023, _("can't connect to service"))
#define MSG_CL_RETVAL_CONNECT_TIMEOUT             _MESSAGE(80024, _("got connect timeout"))
#define MSG_CL_RETVAL_NOT_OPEN                    _MESSAGE(80025, _("not open error"))
#define MSG_CL_RETVAL_SEND_ERROR                  _MESSAGE(80026, _("got send error"))
#define MSG_CL_RETVAL_BIND_SOCKET                 _MESSAGE(80027, _("can't bind socket"))
#define MSG_CL_RETVAL_SELECT_ERROR                _MESSAGE(80028, _("got select error"))
#define MSG_CL_RETVAL_PIPE_ERROR                  _MESSAGE(80030, _("got pipe error"))
#define MSG_CL_RETVAL_GETHOSTNAME_ERROR           _MESSAGE(80031, _("can't resolve host name"))
#define MSG_CL_RETVAL_GETHOSTADDR_ERROR           _MESSAGE(80032, _("can't resolve ip address"))
#define MSG_CL_RETVAL_SEND_TIMEOUT                _MESSAGE(80033, _("got send timeout"))
#define MSG_CL_RETVAL_READ_TIMEOUT                _MESSAGE(80034, _("got read timeout"))
#define MSG_CL_RETVAL_UNDEFINED_FRAMEWORK         _MESSAGE(80035, _("framework is not defined"))
#define MSG_CL_RETVAL_NOT_SERVICE_HANDLER         _MESSAGE(80036, _("handle is not defined as service handler"))
#define MSG_CL_RETVAL_NO_FRAMEWORK_INIT           _MESSAGE(80037, _("framework is not initialized"))
#define MSG_CL_RETVAL_SETSOCKOPT_ERROR            _MESSAGE(80038, _("can't set socket options"))
#define MSG_CL_RETVAL_FCNTL_ERROR                 _MESSAGE(80039, _("got fcntl error"))
#define MSG_CL_RETVAL_LISTEN_ERROR                _MESSAGE(80040, _("got listen error"))
#define MSG_CL_RETVAL_NEED_EMPTY_FRAMEWORK        _MESSAGE(80041, _("framework is not uninitialized"))
#define MSG_CL_RETVAL_LOCK_ERROR                  _MESSAGE(80042, _("can't lock error"))
#define MSG_CL_RETVAL_UNLOCK_ERROR                _MESSAGE(80043, _("can't unlock error"))
#define MSG_CL_RETVAL_WRONG_FRAMEWORK             _MESSAGE(80044, _("used wrong framework"))
#define MSG_CL_RETVAL_READ_ERROR                  _MESSAGE(80045, _("got read error"))
#define MSG_CL_RETVAL_MAX_READ_SIZE               _MESSAGE(80046, _("max read size reached"))
#define MSG_CL_RETVAL_CLIENT_WELCOME_ERROR        _MESSAGE(80047, _("got client welcome error"))
#define MSG_CL_RETVAL_UNKOWN_HOST_ERROR           _MESSAGE(80048, _("unknown host error"))
#define MSG_CL_RETVAL_LOCAL_HOSTNAME_ERROR        _MESSAGE(80049, _("local host name error"))
#define MSG_CL_RETVAL_UNKNOWN_ENDPOINT            _MESSAGE(80050, _("unknown endpoint error"))
#define MSG_CL_RETVAL_UNCOMPLETE_WRITE            _MESSAGE(80051, _("couldn't write all data"))
#define MSG_CL_RETVAL_UNCOMPLETE_READ             _MESSAGE(80052, _("couldn't read all data"))
#define MSG_CL_RETVAL_LIST_DATA_NOT_EMPTY         _MESSAGE(80053, _("list data is not empty"))
#define MSG_CL_RETVAL_LIST_NOT_EMPTY              _MESSAGE(80054, _("list is not empty"))
#define MSG_CL_RETVAL_LIST_DATA_IS_NULL           _MESSAGE(80055, _("list data is not initialized"))
#define MSG_CL_RETVAL_THREAD_SETSPECIFIC_ERROR    _MESSAGE(80056, _("got error setting thread specific data")) 
#define MSG_CL_RETVAL_NOT_THREAD_SPECIFIC_INIT    _MESSAGE(80057, _("could not initialize thread specific data"))
#define MSG_CL_RETVAL_ALLREADY_CONNECTED          _MESSAGE(80058, _("already connected error"))
#define MSG_CL_RETVAL_STREAM_BUFFER_OVERFLOW      _MESSAGE(80059, _("got stream buffer overflow"))
#define MSG_CL_RETVAL_GMSH_ERROR                  _MESSAGE(80060, _("can't read general message size header (GMSH)"))
#define MSG_CL_RETVAL_MESSAGE_ACK_ERROR           _MESSAGE(80061, _("got message acknowledge error"))
#define MSG_CL_RETVAL_MESSAGE_WAIT_FOR_ACK        _MESSAGE(80062, _("message is not acknowledged"))
#define MSG_CL_RETVAL_ENDPOINT_NOT_UNIQUE         _MESSAGE(80063, _("endpoint is not unique error"))
#define MSG_CL_RETVAL_SYNC_RECEIVE_TIMEOUT        _MESSAGE(80064, _("got syncron message receive timeout error"))
#define MSG_CL_RETVAL_MAX_MESSAGE_LENGTH_ERROR    _MESSAGE(80065, _("reached max message length"))
#define MSG_CL_RETVAL_RESOLVING_SETUP_ERROR       _MESSAGE(80066, _("resolve setup error"))
#define MSG_CL_RETVAL_MESSAGE_IN_BUFFER           _MESSAGE(80068, _("still messages in buffer"))
#define MSG_CL_RETVAL_CONNECTION_GOING_DOWN       _MESSAGE(80069, _("connection is going down"))
#define MSG_CL_RETVAL_CONNECTION_STATE_ERROR      _MESSAGE(80070, _("general connection state error")) 
#define MSG_CL_RETVAL_SELECT_TIMEOUT              _MESSAGE(80071, _("got select timeout"))
#define MSG_CL_RETVAL_SELECT_INTERRUPT            _MESSAGE(80072, _("select was interrupted"))
#define MSG_CL_RETVAL_NO_SELECT_DESCRIPTORS       _MESSAGE(80073, _("no file descriptors for select available"))
#define MSG_CL_RETVAL_ALIAS_EXISTS                _MESSAGE(80074, _("alias is already existing"))
#define MSG_CL_RETVAL_NO_ALIAS_FILE               _MESSAGE(80075, _("no alias file specified"))
#define MSG_CL_RETVAL_ALIAS_FILE_NOT_FOUND        _MESSAGE(80076, _("could not get alias file"))
#define MSG_CL_RETVAL_OPEN_ALIAS_FILE_FAILED      _MESSAGE(80077, _("could not open alias file"))
#define MSG_CL_RETVAL_ALIAS_VERSION_ERROR         _MESSAGE(80078, _("wrong alias file version"))
#define MSG_CL_RETVAL_SECURITY_ANNOUNCE_FAILED    _MESSAGE(80079, _("security announce failed"))
#define MSG_CL_RETVAL_SECURITY_SEND_FAILED        _MESSAGE(80080, _("security send failed")) 
#define MSG_CL_RETVAL_SECURITY_RECEIVE_FAILED     _MESSAGE(80081, _("security receive failed"))
#define MSG_CL_RETVAL_ACCESS_DENIED               _MESSAGE(80082, _("access denied"))
#define MSG_CL_RETVAL_MAX_CON_COUNT_REACHED       _MESSAGE(80083, _("max. connection count reached"))
#define MSG_CL_RETVAL_NO_PORT_ERROR               _MESSAGE(80084, _("no valid port number"))
#define MSG_CL_RETVAL_PROTOCOL_ERROR              _MESSAGE(80085, _("can't send response for this message id - protocol error"))
#define MSG_CL_RETVAL_LOCAL_ENDPOINT_NOT_UNIQUE   _MESSAGE(80086, _("local endpoint is not unique"))
#define MSG_CL_RETVAL_TO_LESS_FILEDESCRIPTORS     _MESSAGE(80087, _("operating system provides to less file descriptors"))
#define MSG_CL_RETVAL_DEBUG_CLIENTS_NOT_ENABLED   _MESSAGE(80088, _("debug client mode not active"))
#define MSG_CL_RETVAL_CREATE_RESERVED_PORT_SOCKET _MESSAGE(80089, _("can't create reserved port socket"))
#define MSG_CL_RETVAL_NO_RESERVED_PORT_CONNECTION _MESSAGE(80090, _("client did not use reserved port < 1024"))
#define MSG_CL_RETVAL_NO_LOCAL_HOST_CONNECTION    _MESSAGE(80091, _("client is not connected from local host"))
#define MSG_CL_RETVAL_UNEXPECTED_CHARACTERS       _MESSAGE(80092, _("got unexpected characters or values"))

#define MSG_CL_RETVAL_SSL_COULD_NOT_SET_METHOD         _MESSAGE(80093, _("can't set ssl method"))
#define MSG_CL_RETVAL_SSL_COULD_NOT_CREATE_CONTEXT     _MESSAGE(80094, _("can't create ssl context"))
#define MSG_CL_RETVAL_SSL_COULD_NOT_SET_CA_CHAIN_FILE  _MESSAGE(80095, _("can't set CA chain file"))
#define MSG_CL_RETVAL_SSL_CANT_SET_CA_KEY_PEM_FILE     _MESSAGE(80096, _("can't set private key pem file"))
#define MSG_CL_RETVAL_SSL_CANT_READ_CA_LIST            _MESSAGE(80097, _("can't read trusted CA certificates file(s)"))
#define MSG_CL_RETVAL_SSL_NO_SYMBOL_TABLE              _MESSAGE(80098, _("no symbol table declared"))
#define MSG_CL_RETVAL_SSL_SYMBOL_TABLE_ALREADY_LOADED  _MESSAGE(80099, _("symbol table already loaded"))
#define MSG_CL_RETVAL_SSL_DLOPEN_SSL_LIB_FAILED        _MESSAGE(80100, _("can't open ssl library"))
#define MSG_CL_RETVAL_SSL_CANT_LOAD_ALL_FUNCTIONS      _MESSAGE(80101, _("can't load ssl library function"))
#define MSG_CL_RETVAL_SSL_SHUTDOWN_ERROR               _MESSAGE(80102, _("ssl shutdown error"))
#define MSG_CL_RETVAL_SSL_CANT_CREATE_SSL_OBJECT       _MESSAGE(80103, _("can't create ssl object"))
#define MSG_CL_RETVAL_SSL_CANT_CREATE_BIO_SOCKET       _MESSAGE(80104, _("can't create bio socket"))
#define MSG_CL_RETVAL_SSL_ACCEPT_HANDSHAKE_TIMEOUT     _MESSAGE(80105, _("ssl accept handshake timeout"))
#define MSG_CL_RETVAL_SSL_ACCEPT_ERROR                 _MESSAGE(80106, _("ssl accept error"))
#define MSG_CL_RETVAL_SSL_CONNECT_HANDSHAKE_TIMEOUT    _MESSAGE(80107, _("ssl connect handshake timeout"))
#define MSG_CL_RETVAL_SSL_CONNECT_ERROR                _MESSAGE(80108, _("ssl connect error"))
#define MSG_CL_RETVAL_SSL_CERTIFICATE_ERROR            _MESSAGE(80109, _("ssl certificate error"))
#define MSG_CL_RETVAL_SSL_PEER_CERTIFICATE_ERROR       _MESSAGE(80110, _("ssl peer certificate error"))
#define MSG_CL_RETVAL_SSL_GET_SSL_ERROR                _MESSAGE(80111, _("ssl error"))
#define MSG_CL_RETVAL_SSL_NO_SERVICE_PEER_NAME         _MESSAGE(80112, _("got no expected peer name for service certificate check"))
#define MSG_CL_RETVAL_SSL_RAND_SEED_FAILURE            _MESSAGE(80113, _("PRNG hasn't been seeded with enough data"))
#define MSG_CL_RETVAL_SSL_NOT_SUPPORTED                _MESSAGE(80114, _("SSL module not compiled with -DSECURE (aimk -secure) option"))
#define MSG_CL_RETVAL_ERROR_SETTING_CIPHER_LIST        _MESSAGE(80115, _("error setting cipher list"))
#define MSG_CL_RETVAL_REACHED_FILEDESCRIPTOR_LIMIT     _MESSAGE(80116, _("file descriptor exceeds FD_SETSIZE of this system"))
#define MSG_CL_RETVAL_HOSTNAME_LENGTH_ERROR            _MESSAGE(80117, _("hostname exceeds hostname length(MAXHOSTNAMELEN) on this system"))
#define MSG_CL_RETVAL_HANDLE_SHUTDOWN_IN_PROGRESS      _MESSAGE(80118, _("handle shutdown in progress"))
#define MSG_CL_RETVAL_COMMLIB_SETUP_ALREADY_CALLED     _MESSAGE(80119, _("cl_com_setup_commlib() processed twice"))
#define MSG_CL_RETVAL_DO_IGNORE                        _MESSAGE(80120, _("value is ignored"))
#define MSG_CL_RETVAL_CLOSE_ALIAS_FILE_FAILED          _MESSAGE(80121, _("could not close alias file"))
#define MSG_CL_RETVAL_SSL_CANT_SET_CERT_PEM_BYTE       _MESSAGE(80122, _("can't set certificate bytes"))
#define MSG_CL_RETVAL_SSL_SET_CERT_PEM_BYTE_IS_NULL    _MESSAGE(80123, _("certificate bytes are NULL"))
#define MSG_CL_RETVAL_SSL_CANT_SET_KEY_PEM_BYTE        _MESSAGE(80124, _("can't set key bytes"))
#define MSG_CL_RETVAL_UNKNOWN_PARAMETER                _MESSAGE(80125, _("parameter not found"))

#endif /* __MSG_COMMLISTSLIB_H */
