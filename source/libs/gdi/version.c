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

#include <stdio.h>

#include "gdi/version.h"

const char GDI_VERSION[] = "8.1.9";

const gdi_ulong32 GRM_GDI_VERSION = 0x10003001;

/* fixme:  this isn't currently getting an entry for each release --
   should it?  */
vdict_t GRM_GDI_VERSION_ARRAY[] = 
{
      { 0x10000000, "5.0"  },
      { 0x10000001, "5.1"  },
      { 0x10000002, "5.2"  },
      { 0x10000003, "5.2.3"  },
      { 0x100000F0, "5.3alpha1" },
      { 0x100000F1, "5.3beta1 without hashing" },
      { 0x100000F2, "5.3beta1" },
      { 0x100000F3, "5.3beta2" },
      { 0x100000F4, "5.3" },
      { 0x10000FFF, "6.0"   },
      { 0x10001000, "6.0u3" },
      { 0x10001001, "6.0u4" },
      { 0x10001002, "6.0u8_2" },
      { 0x10002000, "6.1" },
      { 0x100020F0, "6.1AR_snapshot1" },
      { 0x10002001, "6.1u7" },
      { 0x100020F1, "6.2" },
      { 0x100020F2, "6.2u3" },
      { 0x100020F3, "6.2u4" },
      { 0x100020F4, "6.2u5alpha1" },
      { 0x100020F5, "6.2u5alpha2" },
      { 0x100020F6, "6.2u5beta1" },
      { 0x100020F7, "6.2u5beta2" },
      { 0x100020F8, "6.2u5beta2" },
      { 0x100020F8, "6.2u5" },
      { 0x10010000, "6.2u5.1" },
      { 0x10003000, "8.0.0a" },
      { 0, NULL }
};

const char GE_LONGNAME[] = "Son of Grid Engine";
const char GE_SHORTNAME[] = "SGE";

const char SFLN_ELN[] = "\n\
   Grid Engine is based on code donated by Sun Microsystems.\n\
   The copyright is owned by Sun Microsystems and other contributors.\n\
   It has been made available to the open source community under the SISSL license.\n\
   For further information and the latest news visit: @fBhttp://arc.liv.ac.uk/trac/SGE\n\n";

const char DQS_ACK[] = "\n\
We would like to acknowledge and thank the efforts of the\n\
Florida State University in creating the DQS program.\n";

const char SISSL[] = "\n\
The Contents of this file are made available subject to the terms of\n\
the Sun Industry Standards Source License Version 1.2\n\
\n\
Sun Microsystems Inc., March, 2001\n\
\n\
\n\
Sun Industry Standards Source License Version 1.2\n\
=================================================\n\
The contents of this file are subject to the Sun Industry Standards\n\
Source License Version 1.2 (the \"License\"); You may not use this file\n\
except in compliance with the License. You may obtain a copy of the\n\
License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html\n\
\n\
Software provided under this License is provided on an \"AS IS\" basis,\n\
WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,\n\
WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,\n\
MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.\n\
See the License for the specific provisions governing your rights and\n\
obligations concerning the Software.\n\
\n\
The Initial Developer of the Original Code is: Sun Microsystems, Inc.\n\
\n\
Copyright: 2001 by Sun Microsystems, Inc.\n\
\n\
All Rights Reserved.\n"; 
