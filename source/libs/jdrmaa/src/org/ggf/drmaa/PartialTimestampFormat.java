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
 * PartialTimestampFormat.java
 *
 * Created on November 13, 2004, 7:03 PM
 */

package org.ggf.drmaa;

import java.text.*;
import java.util.*;
import java.util.regex.*;

/**
 * This class coverts a PartialTimestamp to and from the DRMAA specified time
 * string.  In order to represent a PartialTimestamp object as a string, the
 * PartialTimestamp object cannot have an unset field that is less significant
 * than the most significant set field.  That is to say that if
 * <code>CENTURY</code> is set, <code>YEAR</code>, <code>MONTH</code>, and
 * <code>DAY_OF_MONTH</code> must also be set.  <code>SECONDS</code> and
 * <code>ZONE_OFFSET</code> are always optional.
 *
 * <p>Example:</p>
 *
 * <pre>   PartialTimestamp date = new PartialTimestamp();
 *   PartialTimestampFormat ptf = new PartialTimestampFormat();
 *
 *   date.set(PartialTimestamp.HOUR_OF_DAY, 11);
 *   date.set(PartialTimestamp.MINUTE, 30);
 *
 *   String timeString = ptf.format(date);
 *
 *   PartialTimestamp date2 = ptf.parse(timeString);
 *
 *   assert(date.equals(date2));
 * </pre>
 * @author dan.templeton@sun.com
 * @see PartialTimestamp
 * @since 0.5
 * @version 1.0
 */
public class PartialTimestampFormat extends Format {
    /**
     * 1 minute in seconds
     */
    private static final int ONE_MINUTE = 60 * 1000;
    /**
     * 1 hour in seconds
     */
    private static final int ONE_HOUR = 60 * ONE_MINUTE;
    /**
     * A formter for printing fields
     */
    private NumberFormat nf = null;

    /**
     * Creates a new instance of PartialTimestampFormat
     */
    public PartialTimestampFormat() {
        nf = NumberFormat.getIntegerInstance();
        nf.setMinimumIntegerDigits(2);
    }

    /**
     * Creates a copy of this object.
     * @return a copy of this object
     */
    public Object clone() {
        return super.clone();
    }

    /**
     * Translates the PartialTimestamp into a DRMAA specified time string and
     * appends the string to the given StringBuffer.  Since the
     * PartialTimestampFormat class doesn't use fields, the fieldPosition
     * parameter is ignored.  This method is equivalent to
     * <code>stringBuffer.append (format (obj))</code>.
     *
     * <p>In order to represent a PartialTimestamp object as a string, the
     * PartialTimestamp object cannot have an unset field that is less
     * significant than the most significant set field.  TThat is to say that if
     * <code>CENTURY</code> is set, <code>YEAR</code>, <code>MONTH</code>, and
     * <code>DAY_OF_MONTH</code> must also be set.  <code>SECONDS</code> and
     * <code>ZONE_OFFSET</code> are always optional.</p>
     * @param obj the object to format
     * @param stringBuffer the StringBuffer to which to append the results
     * @param fieldPosition ignored
     * @return the stringBuffer parameter
     */
    public StringBuffer format(Object obj, StringBuffer stringBuffer, FieldPosition fieldPosition) {
        if (!(obj instanceof PartialTimestamp)) {
            throw new IllegalArgumentException("Cannot parse " + obj.getClass().getName());
        }

        return this.format((PartialTimestamp)obj, stringBuffer, fieldPosition);
    }

    /**
     * Translates the PartialTimestamp into a DRMAA specified time string and
     * appends the string to the given StringBuffer.  Since the
     * PartialTimestampFormat class doesn't use fields, the fieldPosition
     * parameter is ignored.  This method is equivalent to
     * <code>stringBuffer.append (format (obj))</code>.
     *
     * <p>In order to represent a PartialTimestamp object as a string, the
     * PartialTimestamp object cannot have an unset field that is less
     * significant than the most significant set field.  TThat is to say that if
     * <code>CENTURY</code> is set, <code>YEAR</code>, <code>MONTH</code>, and
     * <code>DAY_OF_MONTH</code> must also be set.  <code>SECONDS</code> and
     * <code>ZONE_OFFSET</code> are always optional.</p>
     * @param obj the object to format
     * @param stringBuffer the StringBuffer to which to append the results
     * @param fieldPosition ignored
     * @return the stringBuffer parameter
     */
    public StringBuffer format(PartialTimestamp obj, StringBuffer stringBuffer, FieldPosition fieldPosition) {
        boolean fieldSet = false;

        if (stringBuffer == null) {
            throw new NullPointerException("stringBuffer parameter is null");
        } else if (fieldPosition == null) {
            throw new NullPointerException("fieldPosition parameter is null");
        }

        /* We ignore the fieldPosition since we don't use fields. */

        if (obj.isSet(obj.CENTURY)) {
            stringBuffer.append(nf.format(obj.get(obj.CENTURY)));
            fieldSet = true;
        }

        if (obj.isSet(obj.YEAR)) {
            stringBuffer.append(nf.format(obj.get(obj.YEAR)));
            fieldSet = true;
        } else if (fieldSet) {
            throw new IllegalArgumentException("In PartialTimestamp object, CENTURY is set but YEAR is not");
        }

        if (obj.isSet(obj.MONTH)) {
            if (fieldSet) {
                stringBuffer.append('/');
            }

            stringBuffer.append(nf.format(obj.get(obj.MONTH) + 1));
            fieldSet = true;
        } else if (fieldSet) {
            throw new IllegalArgumentException("In PartialTimestamp object, YEAR is set but MONTH is not");
        }

        if (obj.isSet(obj.DAY_OF_MONTH)) {
            if (fieldSet) {
                stringBuffer.append('/');
            }

            stringBuffer.append(nf.format(obj.get(obj.DAY_OF_MONTH)));
            fieldSet = true;
        } else if (fieldSet) {
            throw new IllegalArgumentException("In PartialTimestamp object, MONTH is set but DAY_OF_MONTH is not");
        }

        if (obj.isSet(obj.HOUR_OF_DAY)) {
            if (fieldSet) {
                stringBuffer.append(' ');
            }

            stringBuffer.append(nf.format(obj.get(obj.HOUR_OF_DAY)));
        } else {
            throw new IllegalArgumentException("In PartialTimestamp object, HOUR_OF_DAY is not set");
        }

        if (obj.isSet(obj.MINUTE)) {
            stringBuffer.append(':');
            stringBuffer.append(nf.format(obj.get(obj.MINUTE)));
        } else {
            throw new IllegalArgumentException("In PartialTimestamp object, MINUTE is not set");
        }

        if (obj.isSet(obj.SECOND)) {
            stringBuffer.append(':');
            stringBuffer.append(nf.format(obj.get(obj.SECOND)));
        }

        if (obj.isSet(obj.ZONE_OFFSET)) {
            int offset = obj.get(obj.ZONE_OFFSET);

            stringBuffer.append(' ');

            if (offset < 0) {
                stringBuffer.append('-');
                offset *= -1;
            } else { // offset >= 0
                stringBuffer.append('+');
            }

            int hours = offset / ONE_HOUR;

            offset %= ONE_HOUR;
            stringBuffer.append(nf.format(hours));
            stringBuffer.append(':');

            int minutes = offset / ONE_MINUTE;

            /* We don't care what happens to the rest of the timezone offset. */
            stringBuffer.append(nf.format(minutes));
        }

        return stringBuffer;
    }

    /**
     * Translates the PartialTimestamp into a DRMAA specified time string.
     * This method is equivalent to <code>format(obj, new StringBuffer (), new
     * FieldPosition(0)).toString()</code>.
     *
     * <p>In order to represent a PartialTimestamp object as a string, the
     * PartialTimestamp object cannot have an unset field that is less
     * significant than the most significant set field.  TThat is to say that if
     * <code>CENTURY</code> is set, <code>YEAR</code>, <code>MONTH</code>, and
     * <code>DAY_OF_MONTH</code> must also be set.  <code>SECONDS</code> and
     * <code>ZONE_OFFSET</code> are always optional.</p>
     * @param obj the object to format
     * @return the DRMAA specified time string
     */
    public String format(PartialTimestamp obj) {
        StringBuffer buffer = this.format(obj, new StringBuffer(),
                new FieldPosition(0));

        return buffer.toString();
    }

    /**
     * Translates a DRMAA specified time string into a PartialTimestamp object.
     * This method will parse as far as possible, but after successfully parsing
     * the <code>HOUR_OF_DAY</code> and <code>MINUTE</code> fields, if it
     * encounters unparsable text, it will stop and will <b>not</b> throw a
     * java.text.ParseException.
     * @param str a DRMAA specified time string
     * @throws ParseException thrown if the string is not parsable.
     * @return a PartialTimestamp object
     */
    public PartialTimestamp parse(String str) throws ParseException {
        ParsePosition pp = new ParsePosition(0);
        PartialTimestamp pt = this.parse(str, pp);

        if (pp.getErrorIndex() >= 0) {
            throw new ParseException("Unable to parse DRMAA date string: \"" +
                    str + "\"", pp.getErrorIndex());
        }

        return pt;
    }

    /**
     * Translates a DRMAA specified time string into a PartialTimestamp object.
     * This method will parse as far as possible.  Upon completion, the parse
     * position object will contain the index of the last character parsed.
     * @param str a DRMAA specified time string
     * @param parsePosition the parse position object
     * @return a PartialTimestamp object
     */
    public PartialTimestamp parse(String str, ParsePosition parsePosition) {
        return (PartialTimestamp)this.parseObject(str, parsePosition);
    }

    /**
     * Translates a DRMAA specified time string into a PartialTimestamp object.
     * This method will parse as far as possible.  Upon completion, the parse
     * position object will contain the index of the last character parsed.
     * @param str a DRMAA specified time string
     * @param parsePosition the parse position object
     * @return a PartialTimestamp object
     */
    public Object parseObject(String str, ParsePosition parsePosition) {
        if (parsePosition == null) {
            throw new NullPointerException("parsePosition parameter is null");
        }

        char[] chars = str.toCharArray();
        PartialTimestamp pt = new PartialTimestamp();
        char[][] fields = new char[][] {new char[4], new char[2], new char[2],
        new char[2], new char[2], new char[2],
        new char[1], new char[2], new char[2]};
        int field = 0;
        int count = 0;
        boolean inWhitespace = false;
        int whitespaceIndex = 0;
        int minuteIndex = 0;
        int tzIndex = 0;

        // Initialize field arrays
        for (int field_count = 0; field_count < fields.length; field_count++) {
            for (int char_count = 0; char_count < fields[field_count].length; char_count++) {
                fields[field_count][char_count] = ' ';
            }
        }

        for (int index = parsePosition.getIndex(); index < chars.length; index++) {
            /* If the character is a digit, and we're in a numeric field, and we
             * haven't reached the end of the field, append the character. */
            if ((field != 6) &&
                    (((field == 0) && (count < 4)) || ((field > 0) && (count < 2))) &&
                    Character.isDigit(chars[index])) {
                /* If we're at the beginning of the minute, note where it
                 * started. */
                if ((field == 4) && (count == 0)) {
                    minuteIndex = index;
                }

                fields[field][count] = chars[index];
                count++;
                inWhitespace = false;
            }
            /* If the character is a colon, and we're not in the date, second, or
             * offset sign fields, and we're at the end of the field, advance to
             * the next field. */
            else if ((field != 1) && (field != 2) && (field != 5) &&
                    (field != 6) && (count == 2) && (chars[index] == ':')) {
                /* If we found the hour instead of the year, skip to the
                 * minute. */
                if (field == 0) {
                    field = 4;
                } else {
                    field++;
                }

                count = 0;
                inWhitespace = false;
            }
            /* If the character is a slash, and we're in the year or month, and
             * we at the end of a field, advance to the next field. */
            else if ((field < 2) && ((count == 2) || (count == 4)) &&
                    (chars[index] == '/')) {
                field++;
                count = 0;
                inWhitespace = false;
            }
            /* If the character is a space, and we're not in the hour, offset sign,
             * or offset hour, and we're at the end of the field, advance to the
             * next field. */
            else if ((field != 3) && (field != 6) && (field != 7) &&
                    (chars[index] == ' ') && ((count == 0) || (count == 2))) {
                /* Ignore consequetive space characters. */
                if (!inWhitespace) {
                    /* If we found the hour instead of the month, skip to the hour. */
                    if ((field == 0) && (count == 2)) {
                        field = 3;
                    }
                    /* If we found the hour instead of the day, skip to the hour. */
                    else if ((field == 1) && (count == 2)) {
                        field = 3;
                    }
                    /* If we found the offset instead of the second, skip to the
                     * offset. */
                    else if ((field == 4) && (count == 2)) {
                        field = 6;
                    }
                    /* The first field is allowed to have leading whitespace. */
                    else if (count == 2) {
                        field++;
                    }

                    count = 0;
                    inWhitespace = true;
                    /* Note where the whitespace started. */
                    whitespaceIndex = index;
                }
            }
            /* If the character is a minus or plus and we're in the offset field,
             * set the offset field. */
            else if ((field == 6) &&
                    ((chars[index] == '-') || (chars[index] == '+'))) {
                /* Note where the offset started. */
                tzIndex = index;
                fields[field][0] = chars[index];
                field++;
                count = 0;
                inWhitespace = false;
            }
            /* If we're not in a patch of whitespace, we're in the minute, second,
             * or offset minute, and we're at the end of the field, and we can't
             * parse the next character, we're done. */
            else if (!inWhitespace && (count == 2) &&
                    ((field == 4) || (field == 5) || (field == 8))) {
                parsePosition.setIndex(index);

                break;
            }
            /* If we're in a patch of whitespace, and we're in the second or offset
             * sign, and we can't parse the next character, we're done, but we have
             * to note the end of the string as being the beginning of the
             * whitespace. */
            else if (inWhitespace && (count == 0) &&
                    ((field == 5) || (field == 6))) {
                parsePosition.setIndex(whitespaceIndex);

                break;
            }
            /* If we can't parse the next character, we have a problem. */
            else {
                parsePosition.setErrorIndex(index);

                return null;
            }
        }

        /* Check for ending in the middle of something. */
        if ((count != 0) && (count != 2) && (count != 4)) {
            parsePosition.setErrorIndex(chars.length);

            return null;
        }

        /* If the hour field was set, parse it. */
        if (Character.isDigit(fields[3][0])) {
            pt.set(pt.HOUR_OF_DAY, Integer.parseInt(new String(fields[3])));

            /* If the day field was set, parse the day, month, and year. */
            if (Character.isDigit(fields[2][0])) {
                pt.set(pt.DAY_OF_MONTH, Integer.parseInt(new String(fields[2])));
                pt.set(pt.MONTH, Integer.parseInt(new String(fields[1])) - 1);

                String year = new String(fields[0]).trim();

                /* If the year has four digits, parse the year and century. */
                if (year.length() == 4) {
                    pt.set(pt.YEAR, Integer.parseInt(year.substring(2, 4)));
                    pt.set(pt.CENTURY, Integer.parseInt(year.substring(0, 2)));
                }
                /* If the year has two digits, parse the year. */
                else {
                    pt.set(pt.YEAR, Integer.parseInt(year));
                }
            }
         /* If the day field wasn't set, but the month was, parse the day and
          * month from the month and year fields. */
            else if (Character.isDigit(fields[1][0])) {
                pt.set(pt.DAY_OF_MONTH, Integer.parseInt(new String(fields[1])));

                String month = new String(fields[0]).trim();

                /* The month should only have two digits. */
                if (month.length() != 2) {
                    parsePosition.setErrorIndex(7);

                    return null;
                } else {
                    pt.set(pt.MONTH, Integer.parseInt(month) - 1);
                }
            }
         /* If the day and month weren't set, but the year was, parse the day
          * from the year field. */
            else if (Character.isDigit(fields[0][0])) {
                String day = new String(fields[0]).trim();

                /* The day should only have 2 digits. */
                if (day.length() != 2) {
                    parsePosition.setErrorIndex(4);

                    return null;
                } else {
                    pt.set(pt.DAY_OF_MONTH, Integer.parseInt(day));
                }
            }
        }
      /* If the hour field wasn't set, but the year was, parse the hour from the
       * year field. */
        else if (Character.isDigit(fields[0][0])) {
            String hour = new String(fields[0]).trim();

            if (hour.length() != 2) {
                parsePosition.setErrorIndex(4);

                return null;
            } else {
                pt.set(pt.HOUR_OF_DAY, Integer.parseInt(hour));
            }
        }
        /* If there is no hour, we have a problem. */
        else {
            parsePosition.setErrorIndex(0);

            return null;
        }

        /* If the minute field is set, parse it. */
        if (Character.isDigit(fields[4][0])) {
            pt.set(pt.MINUTE, Integer.parseInt(new String(fields[4])));
        }
        /* If the minute field isn't set, we have a problem. */
        else {
            parsePosition.setErrorIndex(minuteIndex);

            return null;
        }

        /* If the second field is set, parse it. */
        if (Character.isDigit(fields[5][0])) {
            pt.set(pt.SECOND, Integer.parseInt(new String(fields[5])));
        }

        /* if the offset is set, parse it. */
        if ((fields[6][0] != ' ') && Character.isDigit(fields[7][0]) &&
                Character.isDigit(fields[8][0])) {
            int offset = Integer.parseInt(new String(fields[7])) * ONE_HOUR;

            offset += Integer.parseInt(new String(fields[8])) * ONE_MINUTE;

            if (fields[6][0] == '-') {
                offset *= -1;
            }

            pt.set(pt.ZONE_OFFSET, offset);
        }
        /* If the offset is partially set, we have a problem. */
        else if (!((fields[6][0] == ' ') && !Character.isDigit(fields[7][0]) &&
                !Character.isDigit(fields[8][0]))) {
            /* Figure out just where the problem is. */
            if (fields[6][0] != ' ') {
                if (Character.isDigit(fields[7][0])) {
                    if ((chars.length > tzIndex + 3) &&
                            (chars[tzIndex + 3] == ':')) {
                        tzIndex += 4;
                    } else {
                        tzIndex += 3;
                    }
                } else {
                    tzIndex ++;
                }
            }

            parsePosition.setErrorIndex(tzIndex);

            return null;
        }

        return pt;
    }
}
