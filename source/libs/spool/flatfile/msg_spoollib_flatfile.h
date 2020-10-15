#ifndef __MSG_SPOOLLIB_FLATFILE_H
#define __MSG_SPOOLLIB_FLATFILE_H
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
 * libs/spool/sge_spooling_flatfile.c
 */

#define MSG_FLATFILE_NODATATOSPOOL           _MESSAGE(62000, _("no data available for spooling"))
#define MSG_FILE_ERRORCLOSEINGX_S            _MESSAGE(62001, _("error closing "SFN))
#define MSG_FILE_ERROROPENINGX_S             _MESSAGE(62002, _("error opening "SFN))
#define MSG_UNKNOWNATTRIBUTENAME_S           _MESSAGE(62003, _("unknown attribute name "SFQ))
#define MSG_DONTKNOWHOWTOHANDLELIST_S        _MESSAGE(62004, _("don't know how to handle sublist "SFQ))
#define MSG_FLATFILE_ATTRIBISMISSING_S       _MESSAGE(62005, _("required attribute "SFQ" is missing"))
#define MSG_FLATFILE_DUPLICATEATTRIB_S       _MESSAGE(62006, _("attribute "SFQ" appears more than once"))
#define MSG_PARSINGOBJECTNOATTRIBUTE_D       _MESSAGE(62007, _("line %d should begin with an attribute name"))
#define MSG_PARSINGOBJECTNAMEVALUESEP_SD     _MESSAGE(62008, _(SFQ" is the only character allowed between the attribute name and the value in line %d"))
#define MSG_PARSINGOBJECTUNKNOWNTRAILER_DS   _MESSAGE(62009, _("unrecognized characters after the attribute values in line %d: "SFQ))
#define MSG_PARSINGLISTBADRECORDSTART_DS     _MESSAGE(62010, _("each value in the attribute value list in line %d should begin with "SFQ))
#define MSG_PARSINGLISTBADRECORDEND_DS       _MESSAGE(62011, _("each value in the attribute value list in line %d should end with "SFQ))
#define MSG_PARSINGLISTBADRECORDSEP_DS       _MESSAGE(62012, _("values in the attribute value list in line %d should be separated by "SFQ))
#define MSG_FLATFILE_ERROR_READINGFILE_S     _MESSAGE(62013, _("error reading file: "SFQ))
#define MSG_SPOOL_INCORRECTPATHSFORCOMMONANDSPOOLDIR _MESSAGE(62014, _("incorrect paths given for common and/or spool directory"))
#define MSG_SPOOL_COMMONDIRDOESNOTEXIST_S    _MESSAGE(62015, _("common directory "SFQ" does not exist"))
#define MSG_SPOOL_SPOOLDIRDOESNOTEXIST_S     _MESSAGE(62016, _("spool directory "SFQ" does not exist"))
#define MSG_SPOOL_GLOBALCONFIGNOTDELETED     _MESSAGE(62017, _("the global configuration must not be deleted"))
#define MSG_SPOOL_SCHEDDCONFIGNOTDELETED     _MESSAGE(62018, _("the scheduler configuration must not be deleted"))
#define MSG_MUST_BE_POSITIVE_VALUE_S         _MESSAGE(62019, _("parameter "SFQ" must be a positive number"))

#endif /* __MSG_SPOOLLIB_FLATFILE_H */
