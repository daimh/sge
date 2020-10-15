#ifndef __BOUNDARIES_H
#define __BOUNDARIES_H
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
   THIS IS AN EXAMPLE FOR THE BOUNDARIES HEADER FILE

   There are different 'name intervals' reserved for special internal
   structs (lists).
   The LOWERBOUND & UPPERBOUND values restrict the namespaces to a defined 
   number of maximal fields.
   The user is offered 'user ?? areas'. One restriction to be maintained is
   that the maximum UPPERBOUND is smaller than the MAXINT value.
   Basic unit is only for easier extension used. If it is changed one has
   to recompile all clients. The compatibility is no longer kept.
 */

#define EXAMPLE_BASIC_UNIT 1000         /* Don't touch */

enum NameSpaceBoundaries {
   Q_LOWERBOUND = 1,
   Q_UPPERBOUND = Q_LOWERBOUND + 1 * EXAMPLE_BASIC_UNIT - 1,

   N_LOWERBOUND = 1 * EXAMPLE_BASIC_UNIT + 1,
   N_UPPERBOUND = N_LOWERBOUND + 2 * EXAMPLE_BASIC_UNIT - 1,

   C_LOWERBOUND = 2 * EXAMPLE_BASIC_UNIT + 1,
   C_UPPERBOUND = C_LOWERBOUND + 3 * EXAMPLE_BASIC_UNIT - 1,

   A_LOWERBOUND = 3 * EXAMPLE_BASIC_UNIT + 1,
   A_UPPERBOUND = A_LOWERBOUND + 4 * EXAMPLE_BASIC_UNIT - 1,

   J_LOWERBOUND = 4 * EXAMPLE_BASIC_UNIT + 1,
   J_UPPERBOUND = J_LOWERBOUND + 5 * EXAMPLE_BASIC_UNIT - 1,

   R_LOWERBOUND = 5 * EXAMPLE_BASIC_UNIT + 1,
   R_UPPERBOUND = R_LOWERBOUND + 6 * EXAMPLE_BASIC_UNIT - 1,

   O_LOWERBOUND = 6 * EXAMPLE_BASIC_UNIT + 1,
   O_UPPERBOUND = O_LOWERBOUND + 7 * EXAMPLE_BASIC_UNIT - 1,

   H_LOWERBOUND = 7 * EXAMPLE_BASIC_UNIT + 1,
   H_UPPERBOUND = H_LOWERBOUND + 8 * EXAMPLE_BASIC_UNIT - 1,

/* USER RESERVED AREAS */

   U1_LOWERBOUND = 20 * EXAMPLE_BASIC_UNIT + 1,
   U1_UPPERBOUND = U1_LOWERBOUND + 19 * EXAMPLE_BASIC_UNIT - 1,

   U2_LOWERBOUND = 21 * EXAMPLE_BASIC_UNIT + 1,
   U2_UPPERBOUND = U2_LOWERBOUND + 20 * EXAMPLE_BASIC_UNIT - 1,

   U3_LOWERBOUND = 22 * EXAMPLE_BASIC_UNIT + 1,
   U3_UPPERBOUND = U3_LOWERBOUND + 21 * EXAMPLE_BASIC_UNIT - 1
};

#endif /* __BOUNDARIES_H */
