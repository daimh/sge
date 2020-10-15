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
package com.sun.grid.drmaa.howto;

import org.ggf.drmaa.PartialTimestamp;
import org.ggf.drmaa.PartialTimestampFormat;

public class Howto7 {
   public static void main(String[] args) {
      try {
         PartialTimestamp date = new PartialTimestamp();
         PartialTimestampFormat format = new PartialTimestampFormat();
         
         date.set(PartialTimestamp.HOUR_OF_DAY, 13);
         date.set(PartialTimestamp.MINUTE, 0);
         
         System.out.println(format.format(date));
         
         date.setModifier(PartialTimestamp.DAY_OF_MONTH, 1);
         
         System.out.println(format.format(date));
         System.out.println(date.getTime().toString());
         
         date.setTimeInMillis(System.currentTimeMillis());
         
         System.out.println(format.format(date));
         
         date = format.parse("01/05 14:07:29");
         
         System.out.println(format.format(date));
      } catch (java.text.ParseException e) {
         System.out.println("Error: " + e.getMessage());
      }
   }
}
