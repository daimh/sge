#ifndef __CULL_LERRNO_H
#define __CULL_LERRNO_H
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


#ifdef  __cplusplus
extern "C" {
#endif

#define  LEMALLOC       1       /* malloc failure                         */
#define  LEINCTYPE      2       /* incompatible type                      */
#define  LEUNKTYPE      3       /* unknown type                           */
#define  LEELEMNULL     4       /* element is NULL                        */
#define  LENAMENOT      5       /* name not in descriptor contained       */
#define  LENAMEOUT      6       /* name out of namespaces                 */
#define  LEDESCRNULL    7       /* descriptor is NULL, empty describtor   */
#define  LENEGPOS       8       /* negative position is not allowed       */
#define  LESTRDUP       9       /* strdup failure                         */
#define  LEFILENULL     10      /* file pointer is NULL                   */
#define  LEFGETBRA      11      /* fGetBra failed                         */
#define  LEFGETKET      12      /* fGetKet failed                         */
#define  LEFGETINT      13      /* fGetInt failed                         */
#define  LEFGETDESCR    14      /* fGetDescr failed                       */
#define  LELISTNULL     15      /* list is NULL                           */
#define  LECREATEELEM   16      /* lCreateElem failure                    */
#define  LECOUNTDESCR   17      /* lCountDescr failure                    */
#define  LEFIELDREAD    18      /* reading field failure                  */
#define  LEFGETSTRING   19      /* fGetString failure                     */
#define  LECREATELIST   20      /* lCreateList failure                    */
#define  LEUNDUMPELEM   21      /* lUndumpElem failure                    */

#define  LESSCANF       22      /* sscanf failure                         */
#define  LESYNTAX       23      /* syntax error                           */
#define  LEFGETLINE     24      /* fGetLine failure                       */
#define  LEFGETS        25      /* fgets failure                          */
#define  LESPACECOMMENT 26      /* space_comment failure                  */
#define  LEUNDUMPLIST   27      /* lUndumpList failure                    */
#define  LECOPYSWITCH   28      /* lCopySwitch failure                    */
#define  LEENUMNULL     29      /* lEnumeration is NULL                   */
#define  LECONDNULL     30      /* lCondition is NULL                     */
#define  LENOLISTNAME   31      /* no listname specified                  */
#define  LEDIFFDESCR    32      /* different list descriptors             */
#define  LEDECHAINELEM  33      /* lDechainElem failure                   */
#define  LEAPPENDELEM   34      /* lAppendElem failure                    */
#define  LENOFORMATSTR  35      /* format string is missing               */
#define  LEPARSESORTORD 36      /* lParseSortOrder failure                */
#define  LEGETNROFELEM  37      /* lgetNumberOfElem failure               */
#define  LESORTORDNULL  38      /* lSortOrder is NULL                     */
#define  LESUM          39      /* sum in where.c failure                 */
#define  LEOPUNKNOWN    40      /* operator of lCondition struct unknown  */
#define  LECOPYELEMPART 41      /* lCopyElemPartialPack failure           */
#define  LENULLARGS     42      /* function argument is NULL              */
#define  LEFALSEFIELD   43      /* field is not allowed here              */
#define  LEJOINDESCR    44      /* lJoinDescr failure                     */
#define  LEJOIN         45      /* lJoin failure                          */
#define  LEJOINCOPYELEM 46      /* lJoinCopyElem failure                  */
#define  LEADDLIST      47      /* lAddList failure                       */
#define  LECOUNTWHAT    48      /* lCountWhat failure                     */
#define  LEPARTIALDESCR 49      /* lPartialDescr failure                  */
#define  LEENUMDESCR    50      /* enumeration no subset of descriptor    */
#define  LEENUMBOTHNONE 51      /* at least one enumeration required      */
#define  LENULLSTRING   52      /* string NULL not allowed                */
#define  LEPARSECOND    53      /* parsing condition failed               */
#define  LEFORMAT       54      /* wrong packing format                   */
#define  LEOPEN         55      /* error opening file                     */
#define  LECLOSE        56      /* error closeing file                    */
#define  LEBOUNDELEM    57      /* lListElem is already bound             */
/* prototypes */

int lerror(void);

#ifdef  __cplusplus
}
#endif

#endif /* __CULL_LERRNO_H */

