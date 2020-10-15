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
package com.sun.grid.jgdi.util.shell;

import java.text.DateFormat;
import java.text.MessageFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.InputMismatchException;
import java.util.Locale;
import java.util.Scanner;
import java.util.TimeZone;

public class Util {
    public static final String PATTERN_SEPARATOR = "\\s+";
    public static final String PATTERN_COMMA_SEPARATOR = ",\\s*";
    public static final String PATTERN_COMMA_SPACE_SEPARATOR = PATTERN_COMMA_SEPARATOR + "|" + PATTERN_SEPARATOR;
    
    public static final String PATTERN_SINGLE_RANGE = "\\d+(-\\d+(:\\d+)?)?";
    public static final String PATTERN_RANGE = PATTERN_SINGLE_RANGE+"("+PATTERN_SEPARATOR+PATTERN_SINGLE_RANGE+")*";
    
    public static final String PATTERN_HOST = "[^@]*";
    public static final String PATTERN_HOSTGROUP = "@*";
    
    public static final String PATTERN_PARALLEL_ENV = "(\\d+(-\\d*)?)|(-\\d+)";
    
    public static final String PATTERN_DATE_TIME = "(\\d\\d){4,6}([.]\\d\\d)?";
    public static final String PATTERN_TIME = "\\d*:?\\d*:?\\d*";
    
    public static final String PATTERN_YES_NO = "y(es)?|no?";
    
    
    public static String getDateAndTimeAsString(int time) {
        if (time == -1) {
            return "N/A";
        }
        int offset = TimeZone.getDefault().getOffset(0) / 1000;
        String ret = String.format("%1$tm/%1$td/%1$tY %1$tT", new Date((time-offset)*1000L));
        return ret;
    }
    
    public static int getDateTimeAsInt(String ts) {
        if (!isValueValid(ts, PATTERN_DATE_TIME)) {
            throw new IllegalArgumentException(ts+" is not a date_time type");
        }
        String [] elems = ts.split("[.]");
        String temp;
        int secs = 0;
        //=Seconds
        if (elems.length > 1) {
            secs = Integer.parseInt(elems[1]);
        }
        String dateStr = elems[0];
        SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHHmm");
        switch (dateStr.length()) {
            case 8:  //MMddHHmm
                dateStr = GregorianCalendar.getInstance().get(Calendar.YEAR) + dateStr;
                break;
            case 10: //yyMMddHHmm
                dateStr = String.valueOf(GregorianCalendar.getInstance().get(Calendar.YEAR)).substring(0, 2) + dateStr;
        }
        if (dateStr.length() != 12) {
            throw new IllegalArgumentException(ts+" not a date_time type") ;
        }
        Date d;
        try {
            d = df.parse(dateStr);
            int time = (int) (d.getTime()/1000);
            int offset = TimeZone.getDefault().getOffset(0) / 1000;
            return time + secs + offset;
        } catch (ParseException ex) {
            ex.printStackTrace();
        }
        return -1;
    }
    
    public static String getTimeAsString(int secs) {
        if (secs == -1) {
            return "N/A";
        }
        int minutes;
        int hours;
        hours = secs / (60 * 60);
        secs -= hours * (60 * 60);
        minutes = secs / 60;
        secs -= minutes * 60;
        return MessageFormat.format("{0,number,00}:{1,number,00}:{2,number,00}", hours, minutes, secs);
    }
    
    public static int getTimeAsInt(String duration) {
        if (!isValueValid(duration, PATTERN_TIME) || duration.length() == 0) {
            throw new IllegalArgumentException(duration+" is not a time type");
        }
        if (duration.endsWith(":")) {
            duration += " ";
        }
        Scanner s = new Scanner(duration);
        int res = 0;
        s.useLocale(Locale.US);//TODO LP: What to do with locale formatted numbers?
        s.useDelimiter(":");
        String val;
        while (s.hasNext()) {
            res = res * 60;
            if (s.hasNextInt()) {
                res += s.nextInt();
            } else {
                s.next();
            }
        }
        s.close();
        return res;
    }
    
    public static int getYesNoAsInt(String str) {
        str = str.toLowerCase();
        if (!isValueValid(str, PATTERN_YES_NO)) {
            throw new IllegalArgumentException(str+" is not a yes_no type");
        }
        if (str.startsWith("y")) {
            return 1;
        } else if (str.startsWith("n")) {
            return 0;
        } else {
            throw new IllegalArgumentException("YesNoAsInt coversion error for input: "+str);
        }
    }
    
     public static int getYesNoJobAsInt(String str) {
        str = str.toLowerCase();

        if (str.startsWith("n")) {
            return 0;
        } else if (str.startsWith("y")) {
            return 1;
        } else if (str.startsWith("j")) {
            return 2;
        } else {
            throw new IllegalArgumentException("YesNoJobAsInt coversion error for input: "+str);
        }
    }
    
    public static boolean isYes(String str) {
        return getYesNoAsInt(str)==1 ? true : false;
    }
    
    public static String getYesNoAsString(int val) {
        switch (val) {
            case 0: return "NO";
            case 1: return "YES";
            default: throw new IllegalArgumentException("Unknown value "+val+" for YesNo type conversion");
        }
    }
    
    public static String getYesNoJobAsString(int val) {
        switch (val) {
            case 0: return "NO";
            case 1: return "YES";
            case 2: return "JOB";
            default: throw new IllegalArgumentException("Unknown value "+val+" for YesNoJob type conversion");
        }
    }
    
    public static String getMailOptionsAsString(int val) {
        String ret = "";
        if ((val | 0x00040000) == val) {
            ret += "a" ;
        } else if ((val | 0x00080000) == val) {
            ret += "b" ;
        } else if ((val | 0x00100000) == val) {
            ret += "e" ;
        } else if ((val | 0x00200000) == val) {
            ret += "n" ;
        }
        return ret.length() > 1 ? ret : "?";
    }
    
    public static int getMailOptionsAsInt(String val) {
        int ret = 0;
        
        if (val.contains("a")) {
            val.replaceAll("a", "");
            ret |= 0x00040000;
        } else if (val.contains("b")) {
            val.replaceAll("b", "");
            ret |= 0x00080000;
        } else if (val.contains("e")) {
            val.replaceAll("e", "");
            ret |= 0x00100000;
        } else if (val.contains("n")) {
            val.replaceAll("n", "");
            ret |= 0x00200000;
        }
        if (val.length() > 0) {
            throw new IllegalArgumentException("Unknown mail option type(s): '"+val+"'");
        }
        return ret;
    }
    
    static int getJobVerifyModeAsInt(String firstArg) {
        throw new UnsupportedOperationException("Not yet implemented");
    }
    
    private static boolean isValueValid(String value, String pattern) {
        Scanner s = new Scanner(value);
        try {
            s.next("^("+pattern+")$");
            return true;
        } catch (InputMismatchException ex) {
            return false;
        }
    }
    
    public static void main(String[] args) {
        int ts;
        
        System.out.println(Util.getDateAndTimeAsString(1181012640));
        System.out.println(Util.getDateAndTimeAsString(0));
        
        ts = getDateTimeAsInt("202307340504.23");
        System.out.println(ts);
        System.out.println(getDateAndTimeAsString(ts));
        
        ts = getDateTimeAsInt("03340504.23");
        System.out.println(ts);
        System.out.println(getDateAndTimeAsString(ts));
        
        ts = getDateTimeAsInt("06050504");
        System.out.println(ts);
        System.out.println(getDateAndTimeAsString(ts));
        
        ts = getDateTimeAsInt("197001010000.00");
        System.out.println(ts);
        System.out.println(getDateAndTimeAsString(ts));
        
        ts = getTimeAsInt("25");
        System.out.println(ts);
        System.out.println(getTimeAsString(ts));
        ts = getTimeAsInt("12:30");
        System.out.println(ts);
        System.out.println(getTimeAsString(ts));
        ts = getTimeAsInt("12::");
        System.out.println(ts);
        System.out.println(getTimeAsString(ts));
        ts = getTimeAsInt("25:8:");
        System.out.println(ts);
        System.out.println(getTimeAsString(ts));
        ts = getTimeAsInt("25::59");
        System.out.println(ts);
        System.out.println(getTimeAsString(ts));
        ts = getTimeAsInt("99999::9");
        System.out.println(ts);
        System.out.println(getTimeAsString(ts));
        ts = getTimeAsInt("25:02:34");
        System.out.println(ts);
        System.out.println(getTimeAsString(ts));
        ts = getTimeAsInt("7806");
        System.out.println(ts);
        System.out.println(getTimeAsString(ts));
        ts = getTimeAsInt("10:34000:242400");
        System.out.println(ts);
        System.out.println(getTimeAsString(ts));
    }
}
