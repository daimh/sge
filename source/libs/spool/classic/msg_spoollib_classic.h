#ifndef __MSG_SPOOLLIB_CLASSIC_H
#define __MSG_SPOOLLIB_CLASSIC_H
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
 * libs/spool/read_write_job.c
 */
#define MSG_CONFIG_READINGIN_S                        _MESSAGE(61000, _("Reading in "SFN"."))
#define MSG_CONFIG_FAILEDREMOVINGSCRIPT_SS            _MESSAGE(61001, _("failed removing script of bad jobfile (reason: "SFN"): please delete "SFQ" manually"))
#define MSG_CONFIG_REMOVEDSCRIPTOFBADJOBFILEX_S       _MESSAGE(61002, _("removed script of bad jobfile "SFQ))
#define MSG_CONFIG_NODIRECTORY_S                      _MESSAGE(61003, _(SFQ" is not a directory - skipping the entry"))
#define MSG_CONFIG_CANTFINDSCRIPTFILE_U               _MESSAGE(61004, _("can't find script file for job " sge_U32CFormat " - deleting"))
#define MSG_CONFIG_JOBFILEXHASWRONGFILENAMEDELETING_U _MESSAGE(61005, _("job file \""sge_U32CFormat"\" has wrong file name - deleting"))
#define MSG_CONFIG_JOBSPOOLINGLONGDELAY_UUI           _MESSAGE(61006, _("spooling job "sge_U32CFormat"."sge_U32CFormat" took %d seconds")) 

#endif /* __MSG_SPOOLLIB_CLASSIC_H */
