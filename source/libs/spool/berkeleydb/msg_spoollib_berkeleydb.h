#ifndef __MSG_SPOOLLIB_BERKELEYDB_H
#define __MSG_SPOOLLIB_BERKELEYDB_H
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

/* 
 * libs/spool/sge_spooling_berkeleydb.c
 */

#define MSG_BERKELEY_COULDNTCREATEDBHANDLE_IS _MESSAGE(70000, _("couldn't create berkeley db database handle %d: "SFN))
#define MSG_BERKELEY_COULDNTOPENDB_SIS _MESSAGE(70001, _("couldn't open berkeley database "SFQ": (%d) "SFN))
#define MSG_BERKELEY_COULDNTCLOSEDB_SIS _MESSAGE(70002, _("couldn't close berkeley database "SFQ": (%d) "SFN))
#define MSG_BERKELEY_NOCONNECTIONOPEN_S _MESSAGE(70003, _("no connection open to berkeley database "SFQ))
#define MSG_BERKELEY_CLOSEDDB_S _MESSAGE(70004, _("closed berkeley database "SFQ))
#define MSG_BERKELEY_COULDNTCREATEDB_SIS _MESSAGE(70005, _("couldn't open berkeley database "SFQ": (%d) "SFN))
#define MSG_BERKELEY_PUTERROR_SIS _MESSAGE(70006, _("error writing object with key "SFQ" into berkeley database: (%d) "SFN))
#define MSG_BERKELEY_QUERYERROR_SIS _MESSAGE(70007, _("error retrieving data ("SFQ") from berkeley database: (%d) "SFN))
#define MSG_BERKELEY_DELETEERROR_SIS _MESSAGE(70008, _("error deleting record with key "SFQ" from berkeley database: (%d) "SFN))
#define MSG_BERKELEY_NULLVALUEASKEY_S _MESSAGE(70010, _("null value given as object primary key: "SFN))
#define MSG_BERKELEY_TXNALREADYOPEN _MESSAGE(70017, _("cannot open new transaction: There is already one open"))
#define MSG_BERKELEY_ERRORSTARTINGTRANSACTION_IS _MESSAGE(70018, _("error starting a transaction: (%d) "SFN))
#define MSG_BERKELEY_TXNNOTOPEN _MESSAGE(70019, _("cannot close transaction: There is no open transaction"))
#define MSG_BERKELEY_ABORTINGTRANSACTION _MESSAGE(70020, _("aborting transaction (rollback)"))
#define MSG_BERKELEY_ERRORENDINGTRANSACTION_IS _MESSAGE(70021, _("error ending a transaction: (%d) "SFN))
#define MSG_BERKELEY_CANNOTRETRIEVELOGARCHIVE_IS _MESSAGE(70022, _("error retrieving berkeley db log archive: (%d) "SFN))
#define MSG_BERKELEY_CANNOTCHECKPOINT_IS _MESSAGE(70023, _("error checkpointing berkeley db: (%d) "SFN))
#define MSG_BERKELEY_TRANSACTIONEINVAL _MESSAGE(70024, _("invalid transaction command"))
#define MSG_BERKELEY_USINGBDBVERSION_S _MESSAGE(70025, _("using BerkeleyDB version "SFN))
#define MSG_BERKELEY_WRONGBDBVERSIONEXPECTING_SDD  _MESSAGE(70026, _("wrong BerkeleyDB version: Using "SFN", but expect major version = %d, minor version >= %d"))
#define MSG_BERKELEY_PACKERROR_SS _MESSAGE(70027, _("error packing object with key "SFQ": "SFN))
#define MSG_BERKELEY_PACKINITERROR_SS _MESSAGE(70028, _("error initializing packing buffer for object with key "SFQ": "SFN))
#define MSG_BERKELEY_UNPACKERROR_SS _MESSAGE(70029, _("error unpacking object with key "SFQ": "SFN))
#define MSG_BERKELEY_UNPACKINITERROR_SS _MESSAGE(70030, _("error initializing packing buffer while unpacking object with key "SFQ": "SFN))
#define MSG_BERKELEY_SETOPTIONTO_SS _MESSAGE(70031, _("setting spooling option "SFQ" to "SFQ))
#define MSG_BERKELEY_NORPC_S _MESSAGE(70032, _("BerkeleyDB RPC server not supported: "SFQ))

/*
 * sge_bdb.c
 */
#define MSG_BERKELEY_DATABASEDIRDOESNTEXIST_S _MESSAGE(70101, _("database directory "SFN" doesn't exist"))
#define MSG_BERKELEY_COULDNTCREATEENVIRONMENT_IS _MESSAGE(70102, _("couldn't create database environment: (%d) "SFN))
#define MSG_BERKELEY_COULDNTOPENENVIRONMENT_SSIS _MESSAGE(70103, _("couldn't open database environment for server "SFQ", directory "SFQ": (%d) "SFN))
#define MSG_BERKELEY_COULDNTCLOSEENVIRONMENT_SIS _MESSAGE(70104, _("couldn't close database environment: "SFN": (%d) "SFN))
#define MSG_BERKELEY_COULDNTESETUPLOCKDETECTION_IS _MESSAGE(70105, _("couldn't setup deadlock detection: (%d) "SFN))
#define MSG_BERKELEY_COULDNTESETRPCSERVER_IS _MESSAGE(70106, _("couldn't set rpc server in database environment: (%d) "SFN))
#define MSG_BERKELEY_CONNECTION_LOST_SS _MESSAGE(70107, _("connection to rpc server "SFQ", database "SFQ" lost. Freeing all resources to prepare for a reconnect."))
#define MSG_BERKELEY_RUNRECOVERY _MESSAGE(70108, _("Corrupted database detected. Freeing all resources to prepare for a reconnect with recovery."))
#define MSG_BERKELEY_COULDNTSETLOCKERS_IS _MESSAGE(70109, _("couldn't set maximum number of lockers: (%d) "SFN))
#define MSG_BERKELEY_COULDNTSETOBJECTS_IS _MESSAGE(70110, _("couldn't set maximum number of locked objects: (%d) "SFN))
#define MSG_BERKELEY_COULDNTSETLOCKS_IS _MESSAGE(70111, _("couldn't set maximum number of locks: (%d) "SFN))
#define MSG_BERKELEY_CANNOTCREATECURSOR_IS _MESSAGE(70112, _("cannot create database cursor: (%d) "SFN))
#define MSG_BERKELEY_CANTSETENVFLAGS_IS _MESSAGE(70113, _("cannot set environment flags: (%d) "SFN))
#define MSG_BERKELEY_CANTSETENVCACHE_IS _MESSAGE(70114, _("cannot set environment cache size: (%d) "SFN))
#define MSG_BERKELEY_RPCSERVERLOSTHOME_SS _MESSAGE(70115, _("rpc server "SFQ" reported lost databasedirectory "SFQ". Freeing all resources to prepare for a reconnect."))

/*
 * sge_bdb_types.c
 */
#define MSG_BERKELEY_DBNOTINITIALIZED _MESSAGE(70200, _("database not initialized"))

#endif /* __MSG_SPOOLLIB_BERKELEYDB_H */
