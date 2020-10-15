#ifndef __SYMBOLS_H
#define __SYMBOLS_H
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

/* Checkpoint/Restart Constants */
#define CHECKPOINT_AT_MINIMUM_INTERVAL_SYM 'm'
#define CHECKPOINT_AT_MINIMUM_INTERVAL     0x00000001
#define CHECKPOINT_AT_SHUTDOWN_SYM         's'
#define CHECKPOINT_AT_SHUTDOWN             0x00000002
#define CHECKPOINT_SUSPEND_SYM             'x'
#define CHECKPOINT_SUSPEND                 0x00000004
#define NO_CHECKPOINT_SYM                  'n'
#define NO_CHECKPOINT                      0x00000008
#define CHECKPOINT_AT_AUTO_RES_SYM         'r'
#define CHECKPOINT_AT_AUTO_RES             0x00000010


/* Hold Type Constants */
#define NO_HOLD_SYM                        'n'
#define NO_HOLD                            0x00000010
#define OTHER_SYM                          'o'
#define OTHER                              0x00000020
#define SYSTEM_SYM                         's'
#define SYSTEM                             0x00000040
#define USER_SYM                           'u'
#define USER                               0x00000080

#define ALARM_SYM                          'a'
#define SUSPEND_ALARM_SYM                  'A'
#define DISABLED_SYM                       'd'
#define ENABLED_SYM                        'e'
#define HELD_SYM                           'h'
#define QUEUED_SYM                         'q'
#define RESTARTING_SYM                     'R'  /* NOT PART OF P1003.15D12! */
#define RUNNING_SYM                        'r'
#define SUSPENDED_SYM                      's'  /* NOT PART OF P1003.15D12! */
#define SUSPENDED_ON_SUBORDINATE_SYM       'S'
#define SUSPENDED_ON_THRESHOLD_SYM         'T' /* NOT PART OF P1003.15D12! */
#define TRANSITING_SYM                     't'
#define WAITING_SYM                        'w'
#define EXITING_SYM                        'x'  /* NOT P1003.15D12 compliant! 'e' */
#define ERROR_SYM                          'E'

/* Mail Option Constants */
#define MAIL_AT_ABORT_SYM                  'a'
#define MAIL_AT_ABORT                      0x00040000
#define MAIL_AT_BEGINNING_SYM              'b'
#define MAIL_AT_BEGINNING                  0x00080000
#define MAIL_AT_EXIT_SYM                   'e'
#define MAIL_AT_EXIT                       0x00100000
#define NO_MAIL_SYM                        'n'
#define NO_MAIL                            0x00200000
#define MAIL_AT_SUSPENSION_SYM             's'  /* NOT PART OF P1003.15D12! */
#define MAIL_AT_SUSPENSION                 0x00400000

#endif /* __SYMBOLS_H */
