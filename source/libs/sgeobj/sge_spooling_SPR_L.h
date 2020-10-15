#ifndef __SGE_SPOOL_SPR_L_H
#define __SGE_SPOOL_SPR_L_H
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

#include "cull/cull.h"
#include "sgeobj/sge_boundaries.h"

#ifdef  __cplusplus
extern "C" {
#endif

/****** spool/--SPR_Type ***************************************
*
*  NAME
*     SPR_Type -- Spooling rule
*
*  ELEMENTS
*     SGE_STRING(SPR_name, CULL_HASH | CULL_UNIQUE)
*        Unique name of the rule.
*
*     SGE_STRING(SPR_url, CULL_DEFAULT)
*        An url, e.g. a spool directory, a database url etc.
*
*     SGE_REF(SPR_option_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        function pointer to a function to set any database specific options, 
*        e.g. whether to do database recovery at startup or not.
*
*     SGE_REF(SPR_startup_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        function pointer to a startup function, 
*        e.g. establishing a connection to a database.        
*
*     SGE_REF(SPR_shutdown_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        function pointer to a shutdown function,
*        e.g. disconnecting from a database or closing file handles.
*
*     SGE_REF(SPR_maintenance_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        function pointer to a maintenance function for 
*           - creating the database tables / directories in case of 
*             filebased spooling
*           - switching between spooling with/without history
*           - backup
*           - cleaning up / compressing database
*           - etc.
*
*     SGE_REF(SPR_trigger_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        function pointer to a trigger function.
*        A trigger function is used to trigger regular actions, e.g.
*        checkpointing and cleaning the transaction log in case of the
*        Berkeley DB or vacuuming in case of PostgreSQL.
*
*     SGE_REF(SPR_transaction_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        function pointer to a function beginning and ending transactions.
*
*     SGE_REF(SPR_list_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        pointer to a function reading complete lists (master lists)
*        from the spooling data source.
*
*     SGE_REF(SPR_read_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        pointer to a function reading a single object from the
*        spooling data source.
*
*     SGE_REF(SPR_write_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        pointer to a function writing a single object.
*
*     SGE_REF(SPR_delete_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        pointer to a function deleting a single object.
*
*     SGE_REF(SPR_validate_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        pointer to a function validating a single object.
*
*     SGE_REF(SPR_validate_list_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        pointer to a function validating a list of objects.
*
*     SGE_REF(SPR_clientdata, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        clientdata; any pointer, can be used to store and
*        reference rule specific data, e.g. file or database handles.
*
*  FUNCTION
*     A spooling rule describes a certain way to store and retrieve
*     data from a defined storage facility.
*     
*     Spooling rules can implement spooling to files in a certain
*     directory or spooling into a database, to an LDAP repository
*     etc.
*  
*     A spooling context can contain multiple spooling rules.
*
*  SEE ALSO
*     spool/--Spooling
*     spool/--SPC_Type
****************************************************************************
*/

enum {
   SPR_name = SPR_LOWERBOUND,        /* name of the rule */
   SPR_url,                          /* an url, e.g. base dir, database url */
   SPR_option_func,                 /* function pointer: option, e.g. set db specific parameters */
   SPR_startup_func,                 /* function pointer: startup, e.g. connect to database */
   SPR_shutdown_func,                /* function pointer: shutdown, e.g. disconnect from db */
   SPR_maintenance_func,             /* function pointer: maintenance, e.g. backup of db */
   SPR_trigger_func,                 /* function pointer: trigger */
   SPR_transaction_func,             /* function pointer: transaction */
   SPR_list_func,                    /* function pointer: read master list */
   SPR_read_func,                    /* function pointer: read an object */
   SPR_write_func,                   /* function pointer: write an object */
   SPR_delete_func,                  /* function pointer: delete an object */
   SPR_validate_func,                /* function pointer: validate an object */
   SPR_validate_list_func,           /* function pointer: validate a list */
   SPR_clientdata                    /* any rule specific data */
};

LISTDEF(SPR_Type)
   SGE_STRING(SPR_name, CULL_HASH | CULL_UNIQUE)
   SGE_STRING(SPR_url, CULL_DEFAULT)
   SGE_REF(SPR_option_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(SPR_startup_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(SPR_shutdown_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(SPR_maintenance_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(SPR_trigger_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(SPR_transaction_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(SPR_list_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(SPR_read_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(SPR_write_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(SPR_delete_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(SPR_validate_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(SPR_validate_list_func, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(SPR_clientdata, CULL_ANY_SUBTYPE, CULL_DEFAULT)
LISTEND

NAMEDEF(SPRN)
   NAME("SPR_name")
   NAME("SPR_url")
   NAME("SPR_option_func")
   NAME("SPR_startup_func")
   NAME("SPR_shutdown_func")
   NAME("SPR_maintenance_func")
   NAME("SPR_trigger_func")
   NAME("SPR_transaction_func")
   NAME("SPR_list_func")
   NAME("SPR_read_func")
   NAME("SPR_write_func")
   NAME("SPR_delete_func")
   NAME("SPR_validate_func")
   NAME("SPR_validate_list_func")
   NAME("SPR_clientdata")
NAMEEND

#define SPRS sizeof(SPRN)/sizeof(char *)

#ifdef  __cplusplus
}
#endif

#endif /* __SGE_SPOOLL_H */
