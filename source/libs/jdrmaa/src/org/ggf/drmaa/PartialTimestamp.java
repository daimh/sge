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
 * DRMAACalendar.java
 *
 * Created on September 27, 2004, 12:40 PM
 */

package org.ggf.drmaa;

import java.util.*;

/**
 * The PartialTimestamp is a subclass of java.util.Calendar that allows for a
 * partially specified time that is resolved to the soonest matching time that
 * is in the future at the time of resolution.  That is to say that if one
 * creates a PartialTimestamp instance and sets the hour to 10am and the minute
 * to 0, and one resolves the time, i.e. PartialTimestamp.getTime(), at 11:01am
 * on November 24th, the PartialTimestamp will resolve to 10:00am on November
 * 25th, as that time is the soonest time which matches the specified fields and
 * is in the future.  If one later resolves that same PartialTimestamp instance
 * at 9:34am on December 1st, it will resolve to 10:00am on December 1st.
 *
 * <p>There are only two ways to resolve the time: getTime() and
 * getTimeInMillis().  All other means of accessing field values will return
 * unresolved values.  If the PartialTimestamp class is unable to resolve a
 * field because of other unset fields, it will simply leave the field
 * unresolved.  For example, if the DAY_OF_MONTH is set to 42, and the MONTH is
 * unset, the DAY_OF_MONTH will remain as 42.  If later the MONTH is set, the
 * DAY_OF_MONTH may be resolved to an in-range value.  ("<b>May</b>," because if
 * the MONTH is FEBRUARY, the YEAR and CENTURY are also needed to be able to
 * resolve the DAY_OF_MONTH.)</p>
 *
 * <p>Whereever possible and sensible, the PartialTimestamp class mimics the
 * functionality of the GregorianCalendar class.</p>
 *
 * <p>Since the PartialTimestamp only supports dates within the epoch, the ERA
 * field is always AD, and hence is not used.  The DRMAA specification, requires
 * that the year be handled as two separate fields, one for the first n-2 digits
 * (century) and one for the last two digits.  Because of the symantics of
 * extending the Calendar class, the CENTURY field is just an alias for the ERA
 * field, and the ERA contains the century data.  When using the
 * PartialTimestmap class, the ERA field should not be used.</p>
 *
 * <p>When rolling a date field, if the PartialTimestamp doesn't have enough
 * information to determine the field's valid value range, e.g. when rolling the
 * DAY_OF_MONTH while MONTH is unset, an assumption will be made.  The
 * assumptions a PartialTimestamp instance may make are: a year is 365 days or
 * 53 weeks, and a month is 31 days or 5 weeks.</p>
 *
 * <p>The PartialTimestamp class adds the idea of field modifiers to the
 * Calendar class.  Field modifiers are accessed via the setModifier() and
 * getModifier() methods.  A modifier represents a number that will be added to
 * the value of the field.  This is useful because in a PartialTimestamp
 * instance, it may necessary to add to an unset field, for example to indicate
 * the concept of "tomorrow."  If an add() method or out of range set() method
 * attempts to modify an unset field, the resulting modifier will be stored as
 * the modifier for that field.  For example, if the MONTH is set to DECEMBER,
 * and the YEAR is unset, and 1 is added to the MONTH, the result will be that
 * the MONTH is set to JANUARY, the year is still unset, and the YEAR
 * modifier is set to 1.  If later the YEAR is set to 97, the modifier will be
 * applied, resulting in a year of 98.  To avoid unwanted modifiers, use the
 * roll() method instead of the add() method.</p>
 *
 * <p>The roll() method in the PartialTimestamp class is very simple.  It does
 * not take into account Daylight Savings Time or other complications, as does
 * the roll() method of the GregorianCalendar.</p>
 *
 * <p>Example:</p>
 *
 * <pre>public static void main (String[] args) {
 *   SessionFactory factory = SessionFactory.getFactory ();
 *   Session session = factory.getSession ();
 *
 *   try {
 *      session.init (null);
 *      JobTemplate jt = session.createJobTemplate ();
 *      jt.setRemoteCommand ("sleeper.sh");
 *      jt.setArgs (new String[] {"5"});
 *
 *      PartialTimestamp date = new PartialTimestamp ();
 *
 *      // Run the job on the first of the month at 11:30
 *      date.set (PartialTimestamp.DATE, 1);
 *      date.set (PartialTimestamp.HOUR_OF_DAY, 11);
 *      date.set (PartialTimestamp.MINUTE, 30);
 *
 *      jt.setStartTime (date);
 *
 *      String id = session.runJob (jt);
 *
 *      session.deleteJobTemplate (jt);
 *      session.exit ();
 *   }
 *   catch (DrmaaException e) {
 *      System.out.println ("Error: " + e.getMessage ());
 *   }
 * }
 * </pre>
 * @author dan.templeton@sun.com
 * @since 0.5
 * @version 1.0
 */
public class PartialTimestamp extends Calendar {
    /**
     * CENTURY is a field required by the DRMAA timestamp.
     */
    /* The problem is that the Calendar class made isSet final.  That means I
     * can't introduce any new fields and have them checkable with isSet(); they
     * throw ArrayIndexOutOfBounds exceptions. The only way around this is to
     * recycle the value of ERA, which we weren't using anyway. */
    public static final int CENTURY = Calendar.ERA;
    /**
     * UNSET is the value assigned to fields which have not yet been set.
     */
    public static final int UNSET = Integer.MAX_VALUE;
    /**
     * 1 second in milliseconds
     */
    private static final long ONE_SECOND = 1000;
    /**
     * 1 minute in milliseconds
     */
    private static final long ONE_MINUTE = 60 * ONE_SECOND;
    /**
     * 1 hour in milliseconds
     */
    private static final long ONE_HOUR = 60 * ONE_MINUTE;
    /**
     * 1 day in milliseconds
     */
    private static final long ONE_DAY = 24 * ONE_HOUR;
    /**
     * 1 week in milliseconds
     */
    private static final long ONE_WEEK = 7 * ONE_DAY;
    /**
     * 1 non-leap year in milliseconds
     */
    private static final long ONE_YEAR = 365 * ONE_DAY;
    /**
     * 3 non-leap years in milliseconds
     */
    private static final long THREE_YEARS = 3 * ONE_YEAR;
    /**
     * 3 non-leap years and 1 leap year in milliseconds
     */
    private static final long FOUR_YEARS = ONE_YEAR * 4 + ONE_DAY;
    /**
     * 76 non-leap years and 24 leap years in milliseconds
     */
    private static final long ONE_HUNDRED_YEARS = FOUR_YEARS * 25 - ONE_DAY;
    /**
     * 303 non-leap years and 97 leap years in milliseconds
     */
    private static final long FOUR_HUNDRED_YEARS = ONE_HUNDRED_YEARS * 4 + ONE_DAY;
    /**
     * The fields required by the DRMAA specification
     */
    private static final int[] DRMAA_FIELDS = {CENTURY, YEAR, MONTH, DAY_OF_MONTH,
                                               HOUR_OF_DAY, MINUTE, SECOND};
    /**
     * The field modifiers
     */
    private int[] modifiers = null;
    /**
     * The last hour field set
     */
    private int lastHourSet = HOUR_OF_DAY;  // Required field
    /**
     * The next available time stamp
     */
    private int dayStamp = 0;
    /**
     * The MONTH time stamp
     */
    private int monthSet = UNSET;
    /**
     * The DAY_OF_WEEK time stamp
     */
    private int dayOfWeekSet = UNSET;
    /**
     * The DAY_OF_MONTH time stamp
     */
    private int dayOfMonthSet = UNSET;
    /**
     * The DAY_OF_WEEK_IN_MONTH time stamp
     */
    private int dayOfWeekInMonthSet = UNSET;
    /**
     * The DAY_OF_YEAR time stamp
     */
    private int dayOfYearSet = UNSET;
    /**
     * The WEEK_OF_MONTH time stamp
     */
    private int weekOfMonthSet = UNSET;
    /**
     * The WEEK_OF_YEAR time stamp
     */
    private int weekOfYearSet = UNSET;
    /**
     * Whether any fields have been modified since the last adjustFields() call
     */
    private boolean fieldsModified = true;

    /**
     * Constructs a default PartialTimestamp instance using the current time
     * in the default time zone with the default locale.
     */
    public PartialTimestamp() {
        this(TimeZone.getDefault(), Locale.getDefault());
    }

    /**
     * Constructs a PartialTimestamp instance based on the current time
     * in the given time zone with the default locale.
     * @param zone the given time zone.
     */
    public PartialTimestamp(TimeZone zone) {
        this(zone, Locale.getDefault());
    }

    /**
     * Constructs a PartialTimestamp instance based on the current time
     * in the default time zone with the given locale.
     * @param aLocale the given locale.
     */
    public PartialTimestamp(Locale aLocale) {
        this(TimeZone.getDefault(), aLocale);
    }

    /**
     * Constructs a PartialTimestamp instance based on the current time
     * in the given time zone with the given locale.
     * @param zone the given time zone.
     * @param aLocale the given locale.
     */
    public PartialTimestamp(TimeZone zone, Locale aLocale) {
        super(zone, aLocale);

        this.initializeModifiers();
        this.initializeFields();
    }

    /**
     * Sets all field modifiers to 0.
     */
    private void initializeModifiers() {
        modifiers = new int[] {0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0};
    }

    /**
     * Returns the value of the modifier for the given field.
     * @param field the field whose modifier will be returned
     * @return the value of the field's modifier
     */
    public int getModifier(int field) {
        return modifiers[field];
    }

    /**
     * Sets the value of the modifier for the given field.
     * @param field the field whose modifier will be set
     * @param value the value to which to set the field's modifier
     */
    public void setModifier(int field, int value) {
        modifiers[field] = value;
    }

    /**
     * Sets all field values to UNSET.
     */
    private void initializeFields() {
        for (int count = CENTURY; count < FIELD_COUNT; count++) {
            fields[count] = UNSET;
        }
    }

    /**
     * Constructs a PartialTimestamp instance with the given date
     * and time set for the default time zone with the default locale.
     * @param year the value used to set the YEAR time field in the calendar.
     * @param month the value used to set the MONTH time field in the calendar.
     * Month value is 0-based. e.g., 0 for January.
     * @param date the value used to set the DATE time field in the calendar.
     * @param hour the value used to set the HOUR_OF_DAY time field
     * in the calendar.
     * @param minute the value used to set the MINUTE time field
     * in the calendar.
     */
    public PartialTimestamp(int year, int month, int date, int hour, int minute) {
        this(TimeZone.getDefault(), Locale.getDefault());
        this.set(YEAR, year);
        this.set(MONTH, month);
        this.set(DATE, date);
        this.set(HOUR_OF_DAY, hour);
        this.set(MINUTE, minute);
    }

    /**
     * Constructs a PartialTimestamp instance with the given date
     * and time set for the default time zone with the default locale.
     * @param hour the value used to set the HOUR_OF_DAY time field
     * in the calendar.
     * @param minute the value used to set the MINUTE time field
     * in the calendar.
     * @param second the value used to set the SECOND time field
     * in the calendar.
     */
    public PartialTimestamp(int hour, int minute, int second) {
        this(TimeZone.getDefault(), Locale.getDefault());
        this.set(HOUR_OF_DAY, hour);
        this.set(MINUTE, minute);
        this.set(SECOND, second);
    }

    /**
     * Constructs a PartialTimestamp instance with the given date
     * and time set for the default time zone with the default locale.
     * @param year the value used to set the YEAR time field in the calendar.
     * @param month the value used to set the MONTH time field in the calendar.
     * Month value is 0-based. e.g., 0 for January.
     * @param date the value used to set the DATE time field in the calendar.
     * @param hour the value used to set the HOUR_OF_DAY time field
     * in the calendar.
     * @param minute the value used to set the MINUTE time field
     * in the calendar.
     * @param second the value used to set the SECOND time field
     * in the calendar.
     */
    public PartialTimestamp(int year, int month, int date, int hour, int minute, int second) {
        this(TimeZone.getDefault(), Locale.getDefault());
        this.set(YEAR, year);
        this.set(MONTH, month);
        this.set(DATE, date);
        this.set(HOUR_OF_DAY, hour);
        this.set(MINUTE, minute);
        this.set(SECOND, second);
    }

    // This needs to be completey rewritten to account for modifiers on unset fields
    /**
     * Adds the given value to the given <i>field</i>.  If the field is unset, the
     * <i>amount</i> will
     * be added to the field's modifier instead.  If the field is set, but the addition
     * causes changes to another field, which is unset, the unset field will receive
     * the change as a modifier.
     * @param field the field to which to add the amount
     * @param amount the amount to add
     */
    public void add(int field, int amount) {
        if (amount == 0) {
            return;
        }

        /* We can only add to valid fields. */
        if ((field < ERA) || (field >= FIELD_COUNT)) {
            throw new IllegalArgumentException("Invalid field");
        }

        if (!isSet(field)) {
            modifiers[field] += amount;
        } else {
            adjustFields();

            if ((field == MONTH) && isSet(MONTH) && isSet(DAY_OF_MONTH)) {
                int month = internalGet(MONTH);
                int dayOfMonth = internalGet(DAY_OF_MONTH);
                int newMonth = (month + amount) % 12;

                modifiers[YEAR] += (month + amount) / 12;

                while (newMonth < JANUARY) {
                    newMonth += 12;
                    modifiers[YEAR]--;
                }

                /* If there's no February involed, we don't care what the year
                 * is. */
                if ((month != FEBRUARY) && (newMonth != FEBRUARY) &&
                        (dayOfMonth > getLengthOfMonth(newMonth, false))) {
                    myInternalSet(DAY_OF_MONTH, getLengthOfMonth(newMonth, false));
                }
                /* If there is a February involed and we know the year, do the
                 * math. */
                else if (isSet(YEAR) && isSet(CENTURY)) {
                    int fullYear = this.getYear();
                    int newMonthLength = getLengthOfMonth(newMonth,
                            isLeapYear(fullYear + modifiers[YEAR]));

                    if (dayOfMonth > newMonthLength) {
                        myInternalSet(DAY_OF_MONTH, newMonthLength);
                    }

                    /* Set the year */
                    fullYear += modifiers[YEAR];
                    myInternalSet(YEAR, fullYear % 100);
                    myInternalSet(CENTURY, fullYear / 100);
                    modifiers[YEAR] = 0;
                }
                /* If there's a February involed, and we don't know the year, we
                 * can't do the math.  There's also no reasonable way to save the
                 * math until later because for us to learn enough to be able to do
                 * the math, we'd have to make changes that would render the math
                 * pointless.  Therefore, we just ignore the pinning. */

                /* Set the new month. */
                set(MONTH, newMonth);
            }
            /* If it's the year, and we know the year, pin the day if needed */
            else if (isSet(DAY_OF_YEAR) && isSet(YEAR) && isSet(CENTURY) &&
                    ((field == YEAR) || (field == CENTURY))) {
                int fullYear = this.getYear();
                int daysInYear = this.getLengthOfYear(fullYear);
                int dayOfYear = internalGet(DAY_OF_YEAR);
                boolean pinDay = (dayOfYear == daysInYear);

                // Add in the amount
                if (field == YEAR) {
                    fullYear += amount;
                } else {
                    fullYear += amount * 100;
                }

                // Figure out how many days in this year
                daysInYear = this.getLengthOfYear(fullYear);

                // If we ended up in the next year, set it back to the end of this
                // year
                if (pinDay && (dayOfYear > daysInYear)) {
                   /* No need to do math here because we know that if the current day
                    * of the year was valid, and is now invalid, that's bacause it is
                    * 366 and was a leap year. */
                    myInternalSet(DAY_OF_YEAR, 365);
                }

                /* Internal or not doesn't matter since I don't keep timestamps for
                 * these fields. */
                set(YEAR, fullYear % 100);
                set(CENTURY, fullYear / 100);
            }
            // Otherwise, store it in the parent.
            else {
                super.set(field, internalGet(field) + amount);
            }
        }

        fieldsModified = true;
    }

    // Called by setTimeInMillis() and complete().  Always called after the time
    // has been explicitly set.
    /**
     * This method is called directly after the internal time in milliseconds is set.
     * It computes from this time all the resulting field values.  After
     * computeFields() is called, all fields are considered set.
     */
    protected void computeFields() {
        TimeZone tz = getTimeZone();
        long localTime = time;
        int year = 1970;
        int numDays = 0;

        // First set timezone fields
        if (tz.inDaylightTime(new Date(localTime))) {
            set(DST_OFFSET, tz.getDSTSavings());
        } else {
            set(DST_OFFSET, 0);
        }

        set(ZONE_OFFSET, tz.getOffset(localTime) - internalGet(DST_OFFSET));

        localTime += internalGet(DST_OFFSET) + internalGet(ZONE_OFFSET);

        // Next set the date fields
        /* To make the leap year math work out more easily, we skip the first
         * three years of the epoch if we can, so that we start on the year after
         * a leap year. */
        if (localTime >= (THREE_YEARS + ONE_DAY)) {
            year = 1973;
            localTime -= THREE_YEARS + ONE_DAY;

            int num400 = (int)(localTime / FOUR_HUNDRED_YEARS);
            localTime %= FOUR_HUNDRED_YEARS;
            int num100 = (int)(localTime / ONE_HUNDRED_YEARS);
            localTime %= ONE_HUNDRED_YEARS;
            int num4 = (int)(localTime / FOUR_YEARS);
            localTime %= FOUR_YEARS;
            int num1 = (int)(localTime / ONE_YEAR);
            localTime %= ONE_YEAR;
            year += num400 * 400 + num100 * 100 + num4 * 4 + num1;
            numDays = (int)(localTime / ONE_DAY); // Zero-based
            localTime %= ONE_DAY;

            /* If this is the end of a 4- or 400-year period, and we're short one day
             * of the full leap period, then we know that it's actually Dec 31st of
             * the previous year. */
            if ((num1 == 4) || (num100 == 4)) {
                numDays = 365; // Zero-based
                year--;
            }
        }
        /* If it's exactly 1096 days after the beginning of the epoch, we know
         * that it's Dec 31st, 1972, not Jan 1, 1973, because 1972 is a leap
         * year. */
        else if (localTime >= THREE_YEARS) {
            year = 1972;
            numDays = 365; // Zero-based
        }
        /* If the date is less than Dec 31st, 1972, we know that there are no leap
         * years involved in the math. */
        else {
            int num1 = (int)(localTime / ONE_YEAR);

            numDays = (int)(localTime % ONE_YEAR);
            year = 1970 + num1;
        }

        numDays++; // No longer zero-based

        /* Sets all date fields, including month, year and day of week */
        this.setDateFields(numDays, year);

        /* Because the setDateFields uses myInternalSet(), we have to go back
         * through with set() to make sure that all the fields are recognized as
         * set. */
        set(CENTURY, internalGet(CENTURY));
        set(YEAR, internalGet(YEAR));
        set(MONTH, internalGet(MONTH));
        set(DAY_OF_MONTH, internalGet(DAY_OF_MONTH));
        set(DAY_OF_WEEK, internalGet(DAY_OF_WEEK));
        set(DAY_OF_YEAR, internalGet(DAY_OF_YEAR));
        set(DAY_OF_WEEK_IN_MONTH, internalGet(DAY_OF_WEEK_IN_MONTH));
        set(WEEK_OF_MONTH, internalGet(WEEK_OF_MONTH));
        set(WEEK_OF_YEAR, internalGet(WEEK_OF_YEAR));

        // Finally, set time fields
        set(HOUR_OF_DAY, (int)(localTime / ONE_HOUR));
        set(HOUR, internalGet(HOUR_OF_DAY) % 12);
        set(AM_PM, internalGet(HOUR_OF_DAY) / 12);
        localTime %= ONE_HOUR;
        set(MINUTE, (int)(localTime / ONE_MINUTE));
        localTime %= ONE_MINUTE;
        set(SECOND, (int)(localTime / ONE_SECOND));
        localTime %= ONE_SECOND;
        set(MILLISECOND, (int)localTime);

        fieldsModified = false;
    }

    /**
     * Calculates whether the given year is a leap year.
     * @param year the year to test
     * @return whether the year is a leap year
     */
    static boolean isLeapYear(int year) {
        return ((year % 400) == 0) || (((year % 100) != 0) && ((year % 4) == 0));
    }

    /**
     * Returns the total number of days in the year up to and including the given
     * month.  If the <i>month</i> is less than JANUARY, 0 is returned.  For example, if
     * <i>month</i>
     * is <code>JANUARY</code>, and <i>isLeapYear</i> is <CODE>false</CODE>, this method would return 31.
     * If <i>month</i> is <code>FEBRUARY</code> and <i>isLeapYear</i> is <CODE>true</CODE>, this method would
     * return 60.
     * @param month the month of interest
     * @param isLeapYear whether the current year is a leap year
     * @return the number of ays in the year up to and including the given
     * month
     */
    static int getTotalDays(int month, boolean isLeapYear) {
        int adjustment = (isLeapYear ? 1 : 0);
        switch (month) {
            case JANUARY:
                return 31;
            case FEBRUARY:
                return (59 + adjustment);
            case MARCH:
                return (90 + adjustment);
            case APRIL:
                return (120 + adjustment);
            case MAY:
                return (151 + adjustment);
            case JUNE:
                return (181 + adjustment);
            case JULY:
                return (212 + adjustment);
            case AUGUST:
                return (243 + adjustment);
            case SEPTEMBER:
                return (273 + adjustment);
            case OCTOBER:
                return (304 + adjustment);
            case NOVEMBER:
                return (334 + adjustment);
            case DECEMBER:
                return (365 + adjustment);
            default:
                return 0;
        }
    }

    /**
     * Calculates the current month from the day of the year.
     * @param dayOfYear the day of the year, starting with 1
     * @param isLeapYear whether the current year is a leap year
     * @return the current month
     */
    static int calculateMonth(int dayOfYear, boolean isLeapYear) {
        for (int month = JANUARY; month <= DECEMBER; month++) {
            if (dayOfYear <= getTotalDays(month, isLeapYear)) {
                return month;
            }
        }

        throw new IllegalArgumentException();
    }

    // Have to override this so I can implement my own time stamps
    /**
     * Sets the value of the given field.  After a call to set, the field is considered
     * set.  There is no way to unset a field that has been previously set.  If the
     * value is out of range for the field, it will be adjusted to be within range, if
     * possible.  If, because of other unset fields, there is not enough information to
     * adjust the field, the field will remain as set.
     * @param field the field to set
     * @param value the value to which to set the field
     */
    public void set(int field, int value) {
        /* We can only set valid fields. */
        if ((field < ERA) || (field >= FIELD_COUNT)) {
            throw new IllegalArgumentException("Invalid field");
        } else {
            switch (field) {
                case HOUR:
                case HOUR_OF_DAY:
                    lastHourSet = field;
                    break;
                case DAY_OF_YEAR:
                    dayOfYearSet = getNextStamp();
                    break;
                case WEEK_OF_YEAR:
                    weekOfYearSet = getNextStamp();
                    break;
                case WEEK_OF_MONTH:
                    weekOfMonthSet = getNextStamp();
                    break;
                case DAY_OF_MONTH:
                    dayOfMonthSet = getNextStamp();
                    break;
                case DAY_OF_WEEK_IN_MONTH:
                    dayOfWeekInMonthSet = getNextStamp();
                    break;
                case MONTH:
                    monthSet = getNextStamp();
                    break;
                case DAY_OF_WEEK:
                    dayOfWeekSet = getNextStamp();
                    break;
            }
        }

        super.set(field, value);

        fieldsModified = true;
    }

    // Have to override because our fields are not based on a time in millis
    /**
     * Get the value of the given field.  If the field is unset, this method will
     * return UNSET.
     * @param field which field's value to get
     * @return The value of the field.
     */
    public int get(int field) {
        this.adjustFields();
        return internalGet(field);
    }

    /**
     * This method adjusts all fields so that they are within their valid value ranges.
     * Adjustments are only made if enough information is available; no assumptions are
     * made.
     */
    private void adjustFields() {
        if (!fieldsModified) {
            return;
        }

        int century = internalGet(CENTURY);
        int year = internalGet(YEAR);
        int month = internalGet(MONTH);
        int dayOfWeek = internalGet(DAY_OF_WEEK);
        int origDayOfWeek = dayOfWeek;
        int dayOfMonth = internalGet(DAY_OF_MONTH);
        int weekOfMonth = internalGet(WEEK_OF_MONTH);
        int dayOfWeekInMonth = internalGet(DAY_OF_WEEK_IN_MONTH);
        int dayOfYear = internalGet(DAY_OF_YEAR);
        int weekOfYear = internalGet(WEEK_OF_YEAR);
        int hour = internalGet(HOUR);
        int hourOfDay = internalGet(HOUR_OF_DAY);
        int ampm = internalGet(AM_PM);
        int minute = internalGet(MINUTE);
        int second = internalGet(SECOND);
        int millisecond = internalGet(MILLISECOND);
        int offset = internalGet(ZONE_OFFSET);
        boolean setHours = false;
        boolean setDays = false;
        int date = 0;
        int fullYear = 0;
        boolean isLeapYear = false;
        boolean dayFieldChanged = false;

        int lastDayFieldSet = getLastDayFieldSet();
        /* This is the field with units of days that corresponds to the last day
         * field set. */
        int lastTrueDayFieldSet = getLastTrueDayFieldSet(lastDayFieldSet);
        /* This is the field with units of weeks that corresponds to the last day
         * field set, or 0 if there isn't one. */
        int lastWeekFieldSet = getLastWeekFieldSet(lastDayFieldSet);
        /* Whether the last day field set needs the month to calculate the
         * date. */
        boolean lastDayFieldNeedsMonth = lastDayFieldNeedsMonth(lastDayFieldSet);

        /* If being lenient, and a field overflows add the excess to the next
         * field up.  If a field is negative, borrow a unit from the next field
         * up. */

        /* First we do the time calculations.  These are pretty easy because
         * smaller fields don't depend on larger fields. */
        if (isSet(MILLISECOND)) {
            millisecond += modifiers[MILLISECOND];
            modifiers[MILLISECOND] = 0;

            if ((millisecond > 999) || (millisecond < 0)) {
                if (isLenient()) {
                    modifiers[SECOND] += millisecond / 1000;
                    millisecond %= 1000;

                    if (millisecond < 0) {
                        millisecond += 1000;
                        modifiers[SECOND]--;
                    }
                } else {
                    throw new IllegalArgumentException("Invalid MILLISECOND field");
                }
            }
        }

        if (isSet(SECOND)) {
            second += modifiers[SECOND];
            modifiers[SECOND] = 0;

            if ((second > 59) || (second < 0)) {
                if (isLenient()) {
                    modifiers[MINUTE] += second / 60;
                    second %= 60;

                    if (second < 0) {
                        second += 60;
                        modifiers[MINUTE]--;
                    }
                } else {
                    throw new IllegalArgumentException("Invalid SECOND field");
                }
            }
        }

        /* Required field, but we can't throw an exception because it might just
         * not be set yet. */
        if (isSet(MINUTE)) {
            minute += modifiers[MINUTE];
            modifiers[MINUTE] = 0;

            if ((minute > 59) || (minute < 0)) {
                if (isLenient()) {
                    modifiers[lastHourSet] += minute / 60;
                    minute %= 60;

                    if (minute < 0) {
                        minute += 60;
                        modifiers[lastHourSet]--;
                    }
                } else {
                    throw new IllegalArgumentException("Invalid MINUTE field");
                }
            }
        }

        /* Required field, but we can't throw an exception because it might just
         * not be set yet. */
        /* lastHourSet is HOUR_OF_DAY if neither hour field has been set yet. */
        if ((lastHourSet == HOUR_OF_DAY) && isSet(HOUR_OF_DAY)) {
            hourOfDay += modifiers[HOUR_OF_DAY];
            modifiers[HOUR_OF_DAY] = 0;

            if ((hourOfDay > 23) || (hourOfDay < 0)) {
                if (isLenient()) {
                    modifiers[lastTrueDayFieldSet] += hourOfDay / 24;
                    hourOfDay %= 24;

                    if (hourOfDay < 0) {
                        hourOfDay += 24;
                        modifiers[lastTrueDayFieldSet]--;
                    }

                    /* Set other hour fields. */
                    if (isSet(HOUR)) {
                        hour = hourOfDay % 12;

                        if (hour == 0) {
                            hour = 12;
                        }
                    }

                    if (isSet(AM_PM)) {
                        ampm = hourOfDay / 12;
                        /* We have to clear the modifiers here because AM_PM gets
                         * checked next, and since we have overridden its value, we
                         * don't want it to do anything on its own. */
                        modifiers[AM_PM] = 0;
                    }
                } else {
                    throw new IllegalArgumentException("Invalid HOUR_OF_DAY field");
                }
            }
        } else if (lastHourSet == HOUR) {
            int rawHour = hour;

            hour += modifiers[HOUR];
            modifiers[HOUR] = 0;

            /* We enter here if the hour is greater than 12, or if the hour has
             * been made equal to 12 by modifiers. */
            if ((hour > 12) || ((hour == 12) && (rawHour < 12)) || (hour < 1)) {
                if (isLenient()) {
                    modifiers[AM_PM] += hour / 12;
                    hour = hour % 12;

                    if (hour < 0) {
                        hour += 12;
                        modifiers[AM_PM]--;
                    }

                    /* Restore the normal hour. */
                    if (hour == 0) {
                        hour = 12;
                    }

                    /* Set other hour field when we handle AM_PM. */
                } else {
                    throw new IllegalArgumentException("Invalid HOUR field");
                }
            }
        }

        /* This has to come after the hour calculation so that it can catch any
         * overflow. */
        if (isSet(AM_PM)) {
            ampm += modifiers[AM_PM];
            modifiers[AM_PM] = 0;

            if ((ampm > PM) || (ampm < AM)) {
                if (isLenient()) {
                    /* We treat AM_PM overflow as one day. */
                    modifiers[lastTrueDayFieldSet] += ampm / 2;
                    ampm %= 2;

                    if (ampm < 0) {
                        ampm += 2;
                        modifiers[lastTrueDayFieldSet]--;
                    }

                    /* Set the hour fo the day if we can. */
                    if (isSet(HOUR) && isSet(HOUR_OF_DAY)) {
                        hourOfDay = hour;

                        if (ampm == PM) {
                            hourOfDay += 12;
                        }

                        /* Readjust for misnight. */
                        if (hourOfDay == 24) {
                            hourOfDay = 0;
                        }
                    }
                } else {
                    throw new IllegalArgumentException("Invalid AM_PM field");
                }
            }
        }

        /* Now do the date calculations.  These get interesting because there are
         * dependencies in both directions, e.g. month depends on day depends on
         * month.  In order to deal with this, we first run through the day of
         * week, month, and year math once here.  This way the day math knows the
         * day of week, month, and year.  The result of the day math will be a
         * year and day of year, from which we can derive all the other date
         * fields. */

        if (isSet(MONTH)) {
            month += modifiers[MONTH];
            modifiers[MONTH] = 0;

            if ((month < JANUARY) || (month > DECEMBER)) {
                if (isLenient()) {
                    modifiers[YEAR] += (month - JANUARY) / 12;
                    month = ((month - JANUARY) % 12) + JANUARY;

                    if (month < JANUARY) {
                        month += 12;
                        modifiers[YEAR]--;
                    }
                } else {
                    throw new IllegalArgumentException("Invalid MONTH field");
                }
            }
        }

        if (isSet(YEAR)) {
            year += modifiers[YEAR];
            modifiers[YEAR] = 0;

            if ((year < 0) || (year > 99)) {
                if (isLenient()) {
                    modifiers[CENTURY] += year / 100;
                    year %= 100;

                    if (year < 0) {
                        year += 100;
                        modifiers[CENTURY]--;
                    }
                } else {
                    throw new IllegalArgumentException("Invalid YEAR field");
                }
            }
        }

        if (isSet(CENTURY)) {
            century += modifiers[CENTURY];
            modifiers[CENTURY] = 0;

            if ((century < 0) || (century > Integer.MAX_VALUE - 1)) {
                if (!isLenient()) {
                    throw new IllegalArgumentException("Invalid CENTURY field");
                }
            }
        }

        /* Calculate useful values. */
        if (isSet(YEAR) && isSet(CENTURY)) {
            fullYear = year + century * 100;
            isLeapYear = isLeapYear(fullYear);
        }

        /* We only need day of week for specific date sets.  It has to come after
         * the year and month, because we have to be able to calculate first day
         * of the month. */
        if (isSet(DAY_OF_WEEK)) {
            dayOfWeek += modifiers[DAY_OF_WEEK];
            modifiers[DAY_OF_WEEK] = 0;

            if ((dayOfWeek < SUNDAY) || (dayOfWeek > SATURDAY)) {
                /* Since the day of week in month isn't incremented on Saturday,
                 * as all the other week fields are, we delay handling the day
                 * of the week until we handle the day of week in month. */
                if (isLenient() && (lastWeekFieldSet != 0) &&
                        (lastWeekFieldSet != DAY_OF_WEEK_IN_MONTH)) {
                    modifiers[lastWeekFieldSet] += (dayOfWeek - SUNDAY) / 7;
                    dayOfWeek = ((dayOfWeek - SUNDAY) % 7) + SUNDAY;

                    if (dayOfWeek < SUNDAY) {
                        dayOfWeek += SATURDAY;
                        modifiers[lastWeekFieldSet]--;
                    }
                }
                /* We will deal with day of week in month later.  We don't deal with
                 * non-week day fields at all. */
                else if (!isLenient() && (lastWeekFieldSet != 0) &&
                        (lastWeekFieldSet != DAY_OF_WEEK_IN_MONTH)) {
                    throw new IllegalArgumentException("Invalid DAY_OF_WEEK field");
                }
            }
        }

        /* Note that at this point if month, year, century, and/or day of week are
         * set, there are no modifiers on those set fields.  This will be
         * important when doing the say math, particularly for the year. */

        /* Now comes the really tricky stuff.  First off, we can't resolve a date
         * past the bounds of our knowledge.  For example, we have to know the
         * month to resolve a day overflow.  The tables below show
         * what we can resolve and when.  Without the year, we really can't
         * resolve anything.
         *
         * Resolution without knowing the month:
         * DoM - none - we need the month to do anything
         * DoW/WoM - none - we need the month to do anything
         * DoW/DoWiM - none - we need the month to do anything
         * DoY - fully
         * DoW/WoY - fully
         *
         * Resolution without knowing the day of the week
         * DoM/Month - fully
         * WoM/Month - none - we need the day of the week to do anything
         * DoWiM/Month - none - we need the day of the week to do anything
         * DoY - fully
         * WoY - none - we need the day of the week to do anything
         */

        if (lastDayFieldSet == DAY_OF_MONTH) {
            dayOfMonth += modifiers[DAY_OF_MONTH];
            modifiers[DAY_OF_MONTH] = 0;

            if ((dayOfMonth < 1) ||
                    (dayOfMonth > this.getLengthOfMonth(month, false))) {
                if (isLenient()) {
                    if (isSet(MONTH) && isSet(YEAR) && isSet(CENTURY)) {
                        date = dayOfMonth + getTotalDays(month - 1, isLeapYear);
                        dayFieldChanged = true;

                        if (date < 1) {
                            do {
                                modifiers[YEAR]--;
                                date += getLengthOfYear(fullYear + modifiers[YEAR]);
                            } while (date < 1);
                        } else {
                            int length = getLengthOfYear(fullYear);

                            while (date > length) {
                                date -= length;
                                modifiers[YEAR]++;
                                length = getLengthOfYear(fullYear + modifiers[YEAR]);
                            }
                        }
                    } /* if MONTH && YEAR && CENTURY */
                    /* If we don't know the year, we can't set the other date fields,
                     * but we can do some resolution of day of month. */
                    else if (isSet(MONTH) && (dayOfMonth <= 0)) {
                        int days = getTotalDays(month - 1, false) + dayOfMonth;

                        if (month <= FEBRUARY) {
                            /* If we're in last year, but after February, decrement
                             * the year and do the math normally, assuming no leap
                             * years. */
                            if ((days > -306) && (days < 1)) {
                                days += 365;
                                modifiers[YEAR]--;
                                month = calculateMonth(days, false);
                                dayOfMonth = days - getTotalDays(month - 1, false);
                            }
                            /* If we're in last year before March, decrement the year,
                             * set the month to March and add the span from March to
                             * December to the day of month.  We won't resolve any
                             * further. */
                            else if (days < 1) {
                                dayOfMonth = 306 + days;
                                modifiers[YEAR]--;
                                month = MARCH;
                            }
                            /* If we're still in this year, we have to be in January,
                             * and the date is already correct. */
                            else { // days > 0
                                month = JANUARY;
                                dayOfMonth = days;
                            }
                        } else if (month > MARCH) {
                            /* If we're in this year after February, do the math
                             * normally, assuming no leap years. */
                            if (days > 59) {
                                month = calculateMonth(days, false);
                                dayOfMonth = days - getTotalDays(month - 1, false);
                            }
                            /* Otherwise, set the month to March and figure the days
                             * from there.  We won't resolve this any further. */
                            else {
                                dayOfMonth = days - 59;
                                month = MARCH;
                            }
                        }

                        // Recalculate year if there were any changes. */
                        if (isSet(YEAR) && (modifiers[YEAR] != 0)) {
                            fullYear += modifiers[YEAR];
                            modifiers[YEAR] = 0;

                            year = fullYear % 100;
                            century = fullYear / 100;
                        }
                    } else if (isSet(MONTH)) {
                        if (month == JANUARY) {
                            dayOfMonth -= getLengthOfMonth(JANUARY, false);
                            month++;
                        } else if (month > FEBRUARY) {
                            /* This is all ok because we assume no leap year in all
                             * parts. */
                            /* If we're still in this year, do all the normal math, but
                             * assume no leap years. */
                            if (dayOfMonth < (365 - getTotalDays(month - 1, false))) {
                                month = calculateMonth(dayOfMonth + getTotalDays(month - 1, false), false);
                                dayOfMonth -= getTotalDays(month - 1, false);
                            }
                            /* If we're in January of next year, subtract out the rest
                             * of this year, and set the month to January. */
                            else if (dayOfMonth < (396 - getTotalDays(month - 1, false))) {
                                dayOfMonth -= 365 - getTotalDays(month - 1, false);
                                month = JANUARY;
                            }
                            /* If we're past January of next year, subtract out the
                             * rest of this year plus January, and set the month to
                             * February.  We won't resolve any further overflow. */
                            else {
                                dayOfMonth -= 396 - getTotalDays(month - 1, false);
                                month = FEBRUARY;
                            }
                        }
                    }
                } else {
                    throw new IllegalArgumentException("Invalid DAY_OF_MONTH field");
                }
            }
        } else if (lastDayFieldSet == WEEK_OF_MONTH) {
            weekOfMonth += modifiers[WEEK_OF_MONTH];
            modifiers[WEEK_OF_MONTH] = 0;

            /* We use 4 here it's easier to just do the math for all cases that
             * aren't guaranteed to be safe than to figure out which ones are
             * valid. */
            if ((weekOfMonth < 1) || (weekOfMonth > 4)) {
                if (isLenient()) {
                    if (isSet(MONTH) && isSet(DAY_OF_WEEK) &&
                            isSet(YEAR) && isSet(CENTURY)) {
                        date += getTotalDays(month - 1, isLeapYear);
                        dayFieldChanged = true;

                        if (weekOfMonth < 1) {
                            int lastDayOfMonth = calculateDayOfWeek(getLengthOfMonth(month, isLeapYear),
                                    month, fullYear);
                            /* Add in the number of weeks times 7 plus the number of
                             * days in the first week and the number of days in the
                             * last week. */
                            date += ((weekOfMonth - 1) * 7) +
                                    (dayOfWeek - lastDayOfMonth);

                            int length = getLengthOfYear(fullYear);

                            while (date < 1) {
                                date += length;
                                modifiers[YEAR]--;
                                length = getLengthOfYear(fullYear + modifiers[YEAR]);
                            }
                        } else {
                            int firstDayOfMonth = calculateDayOfWeek(1, month,
                                    fullYear);

                            /* Add in the number of weeks times 7 plus the number of
                             * days in the first week and the number of days in the
                             * last week. */
                            date += ((weekOfMonth - 1) * 7) +
                                    (dayOfWeek - firstDayOfMonth) + 1;

                            int length = getLengthOfYear(fullYear);

                            while (date > length) {
                                date -= length;
                                modifiers[YEAR]++;
                                length = getLengthOfYear(fullYear + modifiers[YEAR]);
                            }
                        }
                    }
                } else {
                    throw new IllegalArgumentException("Invalid WEEK_OF_MONTH field");
                }
            }
        } else if (lastDayFieldSet == DAY_OF_WEEK_IN_MONTH) {
            dayOfWeekInMonth += modifiers[DAY_OF_WEEK_IN_MONTH];
            modifiers[DAY_OF_WEEK_IN_MONTH] = 0;

            /* We use 4  and -4 here it's easier to just do the math for all cases
             * that aren't guaranteed to be safe than to figure out which ones are
             * valid.  We have to check for 0 because the valid range is [-4,-1],
             * [1,4].  We also have to check the day of week here because we
             * skipped it earlier. */
            if ((dayOfWeekInMonth < -4) || (dayOfWeekInMonth > 4) ||
                    (dayOfWeekInMonth == 0) || (dayOfWeek < SUNDAY) ||
                    (dayOfWeek > SATURDAY)) {
                if (isLenient()) {
                    if (isSet(MONTH) && isSet(DAY_OF_WEEK) &&
                            isSet(YEAR) && isSet(CENTURY)) {
                        date = getTotalDays(month - 1, isLeapYear);
                        dayFieldChanged = true;

                        /* If the day of week in month is negative, we treat it as
                         * coming from the end of the month, e.g. -1 means the last
                         * day of its kind in the month. */
                        if (dayOfWeekInMonth < 0) {
                            int lastDayOfMonth = calculateDayOfWeek(getLengthOfMonth(month, isLeapYear),
                                    month, fullYear);
                            /* We want to normalize to the day after the last day of
                             * the month.  That's the day on which the "week"
                             * begins. */
                            int diff = lastDayOfMonth + 1;

                            if (diff > SATURDAY) {
                                diff -= 7;
                            }

                            diff = dayOfWeek - diff;

                            if (diff < SUNDAY) {
                                diff += 7;
                            }

                            /* Find the day after the day that is
                             * (day of week in month * 7) days before the end of the
                             * month, and add the normalized day of week. */
                            date += (dayOfWeekInMonth * 7) + 1 + diff +
                                    getLengthOfMonth(month, isLeapYear);

                            /* Resolve any yearly slippage. */
                            int length = getLengthOfYear(fullYear);

                            while (date < 1) {
                                date += length;
                                modifiers[YEAR]--;
                                length = getLengthOfYear(fullYear + modifiers[YEAR]);
                            }
                        } else {
                            int firstDayOfMonth = calculateDayOfWeek(1, month,
                                    fullYear);
                            /* We want to normalize to the first day of the month. */
                            int diff = dayOfWeek - firstDayOfMonth;

                            if (diff < SUNDAY) {
                                diff += 7;
                            }

                            /* Find the day that is (day of week in month * 7) days
                             * after the first day of the month, and add the normalized
                             * day of week. */
                            date += ((dayOfWeekInMonth - 1) * 7) + diff + 1;

                            /* Resolve any yearly slippage that may have happened. */
                            int length = getLengthOfYear(fullYear);

                            /* If the day of week in month was 0, we could have slipped
                             * slightly into the previous year. */
                            if (date < 1) {
                                modifiers[YEAR]--;
                                date += getLengthOfYear  (fullYear + modifiers[YEAR]);
                            }
                            /* Otherwise, we can only have gone forward. */
                            else {
                                while (date > length) {
                                    date -= length;
                                    modifiers[YEAR]++;
                                    length = getLengthOfYear(fullYear + modifiers[YEAR]);
                                }
                            }
                        }

                        /* Deal with day of week now that we have the day of week in
                         * month set.  Since we already have done all the important
                         * math, all we need to do here is get the value in range. */
                        if (dayOfWeek < SUNDAY) {
                            do {
                                dayOfWeek += 7;
                            } while (dayOfWeek < SUNDAY);
                        } else {
                            while (dayOfWeek > SATURDAY) {
                                dayOfWeek -= 7;
                            }
                        }
                    } /* if MONTH && DAY_OF_WEEK && YEAR */
                } else {
                    throw new IllegalArgumentException("Invalid DAY_OF_WEEK_IN_MONTH field");
                }
            }
        } else if (lastDayFieldSet == DAY_OF_YEAR) {
            dayOfYear += modifiers[DAY_OF_YEAR];
            modifiers[DAY_OF_YEAR] = 0;

            if ((dayOfYear < 1) || (dayOfYear > 365)) {
                if (isLenient()) {
                    if (isSet(YEAR) && isSet(CENTURY)) {
                        date = dayOfYear;
                        dayFieldChanged = true;

                        if (date < 1) {
                            do {
                                modifiers[YEAR]--;
                                date += getLengthOfYear(fullYear + modifiers[YEAR]);
                            } while (date < 1);
                        } else {
                            int length = getLengthOfYear(fullYear);

                            while (date > length) {
                                date -= length;
                                modifiers[YEAR]++;
                                length = getLengthOfYear(fullYear + modifiers[YEAR]);
                            }
                        }
                    }
                } else {
                    throw new IllegalArgumentException("Invalid DAY_OF_YEAR field");
                }
            }
        } else if (lastDayFieldSet == WEEK_OF_YEAR) {
            weekOfYear += modifiers[WEEK_OF_YEAR];
            modifiers[WEEK_OF_YEAR] = 0;

            if ((weekOfYear < 1) || (weekOfYear > 52)) {
                if (isLenient()) {
                    if (isSet(DAY_OF_WEEK) && isSet(YEAR) && isSet(CENTURY)) {
                        int firstDay = calculateDayOfWeek(1, JANUARY, fullYear);
                        /* The day of the year equals 7 times the number of weeks,
                         * minus the days in the previous year and minus the days
                         * missing from the last week. */
                        date = (weekOfYear * 7) - (SATURDAY - dayOfWeek) -
                                (firstDay - SUNDAY);
                        dayFieldChanged = true;

                        if (date < 1) {
                            do {
                                modifiers[YEAR]--;
                                date += getLengthOfYear(fullYear + modifiers[YEAR]);
                            } while (date < 1);
                        } else {
                            int length = getLengthOfYear(fullYear);

                            while (date > length) {
                                date -= length;
                                modifiers[YEAR]++;
                                length = getLengthOfYear(fullYear + modifiers[YEAR]);
                            }
                        }
                    } /* if YEAR */
                } else {
                    throw new IllegalArgumentException("Invalid WEEK_OF_YEAR field");
                }
            }
        }

        /* Now we have to do the last bits of cleanup.  In the course of the day
         * math, we may have generated modifiers on the year.  We now have to
         * apply those modifiers to the final year.  We know that since we only do
         * the date math when the year and century are set that if there are
         * modifiers, the year and century are set. */
        if (dayFieldChanged) {
            if (modifiers[YEAR] != 0) {
                fullYear += modifiers[YEAR];
                modifiers[YEAR] = 0;
            }

            /* Now we have to use the date we generated to set all the other date
             * fields.  This sets the century, year, month, and all the day and week
             * fields. */
            setDateFields(date, fullYear);
        }
        /* Set all the day fields in case they changed. */
        else {
            myInternalSet(CENTURY, century);
            myInternalSet(YEAR, year);
            myInternalSet(MONTH, month);
            myInternalSet(DAY_OF_WEEK, dayOfWeek);
            myInternalSet(DAY_OF_MONTH, dayOfMonth);
            myInternalSet(DAY_OF_YEAR, dayOfYear);
            myInternalSet(DAY_OF_WEEK_IN_MONTH, dayOfWeekInMonth);
            myInternalSet(WEEK_OF_MONTH, weekOfMonth);
            myInternalSet(WEEK_OF_YEAR, weekOfYear);
        }

        /* Next thing is to set the time fields.  Simple. */
        myInternalSet(MILLISECOND, millisecond);
        myInternalSet(SECOND, second);
        myInternalSet(MINUTE, minute);
        myInternalSet(HOUR, hour);
        myInternalSet(AM_PM, ampm);
        myInternalSet(HOUR_OF_DAY, hourOfDay);

        fieldsModified = false;

        /* All set fields are now consistent, and all unset fields remain unset.*/
    }

    /**
     * Given the last day field set, this method returns the corresponding field whose
     * units are days.
     * @param lastDayFieldSet the last day field set
     * @return the corresponding field whose
     * units are days
     */
    private static int getLastTrueDayFieldSet(int lastDayFieldSet) {
        int lastTrueDayFieldSet = 0;

        /* The order of precedence is handled in the getLastDayFieldSet()
         * method. */
        switch (lastDayFieldSet) {
            case WEEK_OF_MONTH:
            case DAY_OF_WEEK_IN_MONTH:
            case WEEK_OF_YEAR:
                lastTrueDayFieldSet = DAY_OF_WEEK;
                break;
            case DAY_OF_YEAR:
                lastTrueDayFieldSet = DAY_OF_YEAR;
                break;
            case DAY_OF_MONTH:
            default:
                lastTrueDayFieldSet = DAY_OF_MONTH;
                break;
        }

        return lastTrueDayFieldSet;
    }

    /**
     * Given the last day field set, this method returns the corresponding field whose
     * units are weeks.
     * @param lastDayFieldSet the last day field set
     * @return the corresponding field whose
     * units are weeks
     */
    private static int getLastWeekFieldSet(int lastDayFieldSet) {
        int lastWeekFieldSet = 0;

        switch (lastDayFieldSet) {
            case WEEK_OF_MONTH:
                lastWeekFieldSet = WEEK_OF_MONTH;
                break;
            case WEEK_OF_YEAR:
                lastWeekFieldSet = WEEK_OF_YEAR;
                break;
            case DAY_OF_WEEK_IN_MONTH:
                lastWeekFieldSet = DAY_OF_WEEK_IN_MONTH;
                break;
            default:
                lastWeekFieldSet = 0;
                break;
        }

        return lastWeekFieldSet;
    }

    /**
     * Given the last day field set, this method determines if that field needs the
     * <CODE>MONTH</CODE> field to be set in order to adjust the field value.
     * @param lastDayFieldSet the last day field set
     * @return whether the given field needs the MONTH field to be set in order to adjust the field value
     */
    private static boolean lastDayFieldNeedsMonth(int lastDayFieldSet) {
        boolean lastDayFieldNeedsMonth = false;

        if ((lastDayFieldSet == DAY_OF_MONTH) ||
                (lastDayFieldSet == WEEK_OF_MONTH) ||
                (lastDayFieldSet == DAY_OF_WEEK_IN_MONTH)) {
            lastDayFieldNeedsMonth = true;
        }

        return lastDayFieldNeedsMonth;
    }

    /**
     * <p>Sets the value of all day fields.  Even though all day fields are assigned a
     * value, previously unset fields will still be considered unset.</p>
     * <p>The day fields are:
     * <ul>
     * <li><code>DAY_OF_MONTH</code> or <code>DATE</code></li>
     * <li><code>WEEK_OF_MONTH</code></li>
     * <li><code>DAY_OF_WEEK_IN_MONTH</code></li>
     * <li><code>DAY_OF_YEAR</code></li>
     * <li><code>WEEK_OF_YEAR</code></li>
     * </ul>
     * </p>
     * @param dayOfYear the day of the year, starting with 1
     * @param year the current year
     */
    private void setDateFields(int dayOfYear, int year) {
        int month = calculateMonth(dayOfYear, isLeapYear(year));
        int date = dayOfYear - getTotalDays(month - 1, isLeapYear(year));
        int dayOfWeek = calculateDayOfWeek(date, month, year);

        myInternalSet(DAY_OF_MONTH, date);
        myInternalSet(DAY_OF_WEEK, dayOfWeek);
        setWeekOfMonth(dayOfWeek, date);
        setDayOfWeekInMonth(date);
        myInternalSet(DAY_OF_YEAR, dayOfYear);
        setWeekOfYear(dayOfYear, calculateDayOfWeek(1, JANUARY, year));
        myInternalSet(MONTH, month);
        myInternalSet(YEAR, year % 100);
        myInternalSet(CENTURY, year / 100);
    }

    /**
     * Sets the <code>WEEK_OF_YEAR</code> field.
     * @param dayOfYear the day of the year, starting with 1
     * @param firstDay the day of the week of the first day of the year
     */
    private void setWeekOfYear(int dayOfYear, int firstDay) {
        /* Subtract the number of days in the previous year. */
        int leading = (SATURDAY + 1 - firstDay) % 7;
        dayOfYear -= leading;
        /* Get the number of days missing from the last week.  We don't have to
         * sutract it out because the division will truncate it because by
         * subtracting out the days in the previous year, we guarantee that the
         * partial week is less than 7 days. */
        int trailing = dayOfYear % 7;
        int weekOfYear = dayOfYear / 7 + ((leading > 0) ? 1 : 0) +
                ((trailing > 0) ? 1 : 0);

        myInternalSet(WEEK_OF_YEAR, weekOfYear);
    }

    /**
     * Sets the D<code>AY_OF_WEEK_IN_MONTH</code> field.
     * @param date the day of the year
     */
    private void setDayOfWeekInMonth(int date) {
        /* Calculate how many weeks into the month we are.  The day
         * of the week is actually irrelevant. */
        int dayOfWeekInMonth = date / 7;

        if (date % 7 != 0) {
            dayOfWeekInMonth++;
        }

        myInternalSet(DAY_OF_WEEK_IN_MONTH, dayOfWeekInMonth);
    }

    /**
     * Sets the <code>WEEK_IN_MONTH</code> field.
     * @param dayOfWeek the day of the week
     * @param date the day of the year
     */
    private void setWeekOfMonth(int dayOfWeek, int date) {
        /* Figure out what day the 1st of the month is */
        int firstDayOfMonth = dayOfWeek - ((date - 1) % 7);

        if (firstDayOfMonth < SUNDAY) {
            firstDayOfMonth += SATURDAY;
        }

        /* Subtract days in first week in previous month. */
        int leading = (SATURDAY + 1 - firstDayOfMonth) % 7;
        int newDays = date - leading;
        /* Get days in last week.  We don't have to sutract it out because the
         * division will truncate it because by subtracting out the days in the
         * previous month, we guarantee that the partial week is less than 7
         * days. */
        int trailing = newDays % 7;
        /* Count the weeks, including partial weeks on both ends */
        int weekOfMonth = newDays / 7 + ((trailing > 0) ? 1 : 0) +
                ((leading > 0) ? 1 : 0);

        myInternalSet(WEEK_OF_MONTH, weekOfMonth);
    }

    /**
     * This method calculates the day of the week for the given date.
     * @param date the day of the month
     * @param month the month
     * @param year the year
     * @return the day of the week
     */
    static int calculateDayOfWeek(int date, int month, int year) {
        int dayOfWeek = WEDNESDAY; // Jan 1, 1969
        int days = getTotalDays(month - 1, isLeapYear(year));

        /* Each year advances the DoW by 1, except leap years which advance by 2.
         * Remember to account for double and triple leap years! */
        if (year > 2000) {
            dayOfWeek += (((year - 1969) / 4) * 5) - ((year - 2001) / 100) +
                    ((year - 2001) / 400) + ((year - 1969) % 4);
        }
        /* No double or triple leap years between 1970 and 1999. */
        else {
            dayOfWeek += (((year - 1969) / 4) * 5) + ((year - 1969) % 4);
        }

        /* Add in the number of days past Jan 1 in the current year. */
        dayOfWeek += days + date - 1;
        /* Mod by the number of days in a week. */
        dayOfWeek = ((dayOfWeek - SUNDAY) % 7) + SUNDAY;

        return dayOfWeek;
    }

    // Called by updateTime(), which is called from writeObject(), complete(),
    // and getTimeInMillis()
    /**
     * This method uses the set fields and the current time to calculate the soonest
     * time in milliseconds for this PartialTimestamp object which is in the future.
     * If no time in the future can be found, the resulting time will be the same as
     * creating a new GregorianCalendar() and setting all the set fields of the
     * PartialTimestamp object in it.  In any case, the resulting is stored internally.
     */
    protected void computeTime() {
        int firstSet = -1;

        if (!isSet(HOUR_OF_DAY) && !isSet(HOUR) && !isSet(AM_PM)) {
            throw new IllegalArgumentException("HOUR_OF_DAY is a required field.");
        }

        if (!isSet(MINUTE)) {
            throw new IllegalArgumentException("MINUTE is a required field.");
        }

        /* Make sure all the fields are valid. */
        adjustFields();

        /* Get a test calendar */
        Calendar then = Calendar.getInstance(this.getTimeZone());

        // Fill in set field in then - handle year separately
        for (int count = 2; count < DRMAA_FIELDS.length; count++) {
            if (isSet(DRMAA_FIELDS[count])) {
                then.set(DRMAA_FIELDS[count],
                        internalGet(DRMAA_FIELDS[count]));

                if (firstSet == -1) {
                    firstSet = DRMAA_FIELDS[count];
                }
            }
            /* SECOND & MILLISECOND are just too fine-grained to worry about. */
            else if (DRMAA_FIELDS[count] == SECOND) {
                then.set(DRMAA_FIELDS[count], 0);
            }
        }

        if (isSet(YEAR) && isSet(CENTURY)) {
            then.set(YEAR, internalGet(YEAR) + internalGet(CENTURY) * 100);
            firstSet = CENTURY;
        } else if (isSet(YEAR)) {
            then.set(YEAR, internalGet(YEAR) +
                    (then.get(YEAR) - (then.get(YEAR) % 100)));
            firstSet = YEAR;
        } else if (isSet(CENTURY)) {
            then.set(YEAR, (then.get(YEAR) % 100) +
                    internalGet(CENTURY) * 100);
            firstSet = CENTURY;
        }

        then.set(MILLISECOND, 0);

        // If then is less than now,
        if (then.getTimeInMillis() < System.currentTimeMillis()) {
            // Add 1 to the field above the highest order set field
            /* There's nothing we can do about a badly set century. */
            if (firstSet == YEAR) {
                then.add(getNextField(firstSet), 100);
            } else if (firstSet != CENTURY) {
                then.add(getNextField(firstSet), 1);
            }
        }

        // Set time to then.getTimeInMillis()
        time = then.getTimeInMillis();
    }

    /**
     * Gets the next field up from the given field.  For example, <code>YEAR</code> is the next
     * field up from <code>MONTH</code>, <code>MONTH</code> is the next field up from <code>DAY_OF_MONTH</code>, and
     * <code>DAY_OF_YEAR</code> is the next field up from <code>HOUR_OF_DAY</code>.  This method is used by
     * computeTime() to increment the lowest unset field in an attempt to find a time
     * in the future.
     * @param field the field of interest
     * @return the next field up
     */
    private static int getNextField(int field) {
        int nextField = -1;

        switch (field) {
            case YEAR:
            case MONTH:
            case WEEK_OF_YEAR:
                nextField = YEAR;
                break;
            case DAY_OF_MONTH:
            case DAY_OF_WEEK:
            case DAY_OF_YEAR:
            case DAY_OF_WEEK_IN_MONTH:
            case WEEK_OF_MONTH:
                nextField = MONTH;
                break;
            case HOUR:
            case HOUR_OF_DAY:
            case AM_PM:
                nextField = DAY_OF_YEAR;
                break;
        }

        return nextField;
    }

    /**
     * Gets the greatest minimum value that the field may ever have.
     * @param field the field of interest
     * @return the greatest minimum value that the field may ever have
     */
    public int getGreatestMinimum(int field) {
        switch (field) {
            case YEAR:
                return 70; // Jan 1, 1970
            case DAY_OF_WEEK:
                return THURSDAY; // Jan 1, 1970
            case DAY_OF_WEEK_IN_MONTH:
                return -4;
            default:
                return this.getMinimum(field);
        }
    }

    /**
     * Gets the least maximum value that the field may ever have.
     * @param field the field of interest
     * @return the least maximum value that the field may ever have
     */
    public int getLeastMaximum(int field) {
        switch (field) {
            case WEEK_OF_YEAR:
                return 52;
            case WEEK_OF_MONTH:
                return 4;
            case DATE:
                return 28;
            case DAY_OF_YEAR:
                return 365;
            case DAY_OF_WEEK_IN_MONTH:
                return 4;
            default:
                return this.getMaximum(field);
        }
    }

    /**
     * Gets the greatest maximum value that the field may ever have.
     * @param field the field of interest
     * @return the greatest maximum value that the field may ever have
     */
    public int getMaximum(int field) {
        switch (field) {
            case YEAR:
                return 99;
            case MONTH:
                return 11;
            case WEEK_OF_YEAR:
                return 54;  /* Should this maybe be 53? Depends on how this field is defined. */
            case WEEK_OF_MONTH:
                return 6;
            case DATE:
                return 31;
            case DAY_OF_YEAR:
                return 366;
            case DAY_OF_WEEK:
                return SATURDAY;
            case DAY_OF_WEEK_IN_MONTH:
                return 5;
            case AM_PM:
                return PM;
            case HOUR:
                return 11;
            case HOUR_OF_DAY:
                return 23;
            case MINUTE:
                return 59;
            case SECOND:
                return 59;
            case MILLISECOND:
                return 999;
            case ZONE_OFFSET:
                return 12;
            case DST_OFFSET:
                return 1;
            case CENTURY:
                return Integer.MAX_VALUE;
            default:
                throw new IllegalArgumentException();
        }
    }

    /**
     * Gets the least minimum value that the field may ever have.
     * @param field the field of interest
     * @return the least minimum value that the field may ever have
     */
    public int getMinimum(int field) {
        switch (field) {
            case YEAR:
                return 0;
            case MONTH:
                return 0;
            case WEEK_OF_YEAR:
                return 1;
            case WEEK_OF_MONTH:
                return 1;
            case DATE:
                return 1;
            case DAY_OF_YEAR:
                return 1;
            case DAY_OF_WEEK:
                return SUNDAY;
            case DAY_OF_WEEK_IN_MONTH:
                return 0;
            case AM_PM:
                return AM;
            case HOUR:
                return 0;
            case HOUR_OF_DAY:
                return 0;
            case MINUTE:
                return 0;
            case SECOND:
                return 0;
            case MILLISECOND:
                return 0;
            case ZONE_OFFSET:
                return -12;
            case DST_OFFSET:
                return 0;
            case CENTURY:
                return 19;
            default:
                throw new IllegalArgumentException();
        }
    }

    /**
     * This method naively rolls the value of the given field by 1, either up or down.
     * If the resulting value is out of range for the field, the value will roll over
     * without affecting other fields.
     * @param field the field to roll
     * @param up whether to roll up
     */
    public void roll(int field, boolean up) {
        roll(field, up ? +1 : -1);
    }

    /**
     * This method naively rolls the value of the given field up by the given amount.
     * To roll down, use a negative amount.  If the resulting value is out of range for
     * the field, the value will roll over without affecting other fields.
     * @param field the field to roll
     * @param amount the amount to roll up
     */
    public void roll(int field, int amount) {
        if (amount == 0) {
            return; // Nothing to do
        }

        /* I am implementing this in a very naive way.  Except for day pinning, I
         * am simply rolling through the range of possible values for the fields.
         * The GregorianCalendar does a whole lot more.  I'm not exactly certain
         * I even understand why it's doing all that it's doing.  However,
         * assuming that the GregorianCalander.roll() method is "correct," this
         * implementation is not.  Given the likely uses of this implementation,
         * however, I doubt that is a problem, and if it becomes a problem, I can
         * do a better job in a later release. */

        int min = 0;
        int max = 0;
        int gap = 0;
        boolean dayAtEnd = false;

        /* We can only roll valid fields that have been set. */
        if ((field >= ERA) && (field < FIELD_COUNT) && isSet(field)) {
            adjustFields();
            min = getMinimum(field);
            max = getMaximum(field);
        } else if (!isSet(field)) {
            throw new IllegalArgumentException("Invalid field");
        } else {
            throw new IllegalArgumentException("Cannot roll unset fields");
        }

        switch (field) {
            case MONTH: {
                boolean isLeapYear = isLeapYear(this.getYear());
                int length = getLengthOfMonth(internalGet(MONTH), isLeapYear);

                dayAtEnd = isSet(DAY_OF_MONTH) &&
                        (internalGet(DAY_OF_MONTH) == length);
                break;
            }
            case YEAR: {
                int length = getLengthOfYear(this.getYear());

                dayAtEnd = isSet(DAY_OF_YEAR) &&
                        (internalGet(DAY_OF_YEAR) == length);
                break;
            }
            case WEEK_OF_YEAR:
                if (isSet(YEAR) && isSet(CENTURY)) {
                    max = getActualMaximum(field);
                }
                /* If we don't know the year, we assume 53 weeks, which is almost
                 * always the case.  (The exception is leap years that start on
                 * Saturdays.) */
                else {
                    max = 53;
                }

                min = getActualMinimum(field);

                break;
            case WEEK_OF_MONTH:
                if (isSet(YEAR) && isSet(CENTURY)) {
                    max = getActualMaximum(field);
                }
                /* If we don't know the year, we assume 5 weeks, which is usually
                 * the case.  (The exceptions are Februarys in non-leap years which
                 * begin on Sunday, and months with 31 days which begin on
                 * Saturdays.) */
                else {
                    max = 5;
                }

                min = getActualMinimum(field);

                break;
            case DAY_OF_WEEK_IN_MONTH:
                /* This on is a little interesting since we have to treat it as it
                 * is, without turning it into a date, doing the roll, and turning
                 * it back. */
                int dayOfWeekInMonth = internalGet(field);

                if (dayOfWeekInMonth < 0) {
                    max = getActualMinimum(field);
                    max = -max;

                    if (isSet(YEAR) && isSet(CENTURY)) {
                        min = getActualMaximum(field);
                        min = -min;
                    } else {
                        min = -5;
                    }
                } else {
                    if (isSet(YEAR) && isSet(CENTURY)) {
                        max = getActualMaximum(field);
                    }
                    /* If we don't know the year, we assume 5 weeks, which is usually
                     * the case.  (The exceptions are Februarys in non-leap years which
                     * begin on Sunday, and months with 31 days which begin on
                     * Saturdays.) */
                    else {
                        max = 5;
                    }

                    min = getActualMinimum(field);
                }

                break;
            case CENTURY:
            case ZONE_OFFSET:
            case DST_OFFSET:
                // These fields cannot be rolled
                throw new IllegalArgumentException("This field cannot be rolled");
            default:
                max = getActualMaximum(field);
                min = getActualMinimum(field);
                break;
        }

        // These are the standard roll instructions.  These work for all
        // simple cases, that is, cases in which the limits are fixed, such
        // as the hour, the month, and the era.
        int value = internalGet(field) + amount;

        gap = max - min + 1;
        value = (value - min) % gap;

        if (value < 0) {
            value += gap;
        }

        value += min;

        set(field, value);

        // Pin the day if needed.
        if (dayAtEnd) {
            if (field == MONTH) {
                boolean isLeapYear = isLeapYear(this.getYear());
                int length = getLengthOfMonth(internalGet(MONTH), isLeapYear);

                myInternalSet(DAY_OF_MONTH, length);
            } else { // if (field == YEAR)
                int length = getLengthOfYear(this.getYear());

                myInternalSet(DAY_OF_YEAR, length);
            }
        }

        fieldsModified = true;
    }

    /**
     * Returns the next available time stamp and increments the next available time
     * stamp.
     * @return the next available time stamp
     */
    private synchronized int getNextStamp() {
        /* If we've run out of stamps, normalize to 0. */
        if (dayStamp == UNSET) {
            /* First find the minimum stamp. */
            int min = Math.min(monthSet, dayOfWeekSet);
            min = Math.min(min, dayOfMonthSet);
            min = Math.min(min, dayOfWeekInMonthSet);
            min = Math.min(min, dayOfYearSet);
            min = Math.min(min, weekOfMonthSet);
            min = Math.min(min, weekOfYearSet);

            /* Now use that to normalize all the stamps to 0. */
            monthSet -= min;
            dayOfWeekSet -= min;
            dayOfMonthSet -= min;
            dayOfWeekInMonthSet -= min;
            dayOfYearSet -= min;
            weekOfMonthSet -= min;
            weekOfYearSet -= min;

            /* Now find the maximum stamp value. */
            int max = Math.max(monthSet, dayOfWeekSet);
            max = Math.max(max, dayOfMonthSet);
            max = Math.max(max, dayOfWeekInMonthSet);
            max = Math.max(max, dayOfYearSet);
            max = Math.max(max, weekOfMonthSet);
            max = Math.max(max, weekOfYearSet);

            /* The next stamp will start one about the current max. */
            dayStamp = max + 1;
        }

        return dayStamp++;
    }

    /**
     * Returns the last day field set on this object. The day fields are:
     * <ul>
     * <li>DAY_OF_MONTH or DATE</li>
     * <li>WEEK_OF_MONTH</li>
     * <li>DAY_OF_WEEK_IN_MONTH</li>
     * <li>DAY_OF_YEAR</li>
     * <li>WEEK_OF_YEAR</li>
     * </ul>
     * @return the last day field set
     */
    private int getLastDayFieldSet() {
        int[] best = null;
        int[] thisBest = null;
        int bestField = -1;

        /* compareStamps() gives preference to completely set groups. */
        if (isSet(DAY_OF_MONTH)) {
            thisBest = new int[] {monthSet, dayOfMonthSet};

            if ((best == null) || (compareStamps(thisBest, best) == 1)) {
                best = thisBest;
                bestField = DAY_OF_MONTH;
            }
        }

        if (isSet(WEEK_OF_MONTH)) {
            thisBest = new int[] {monthSet, weekOfMonthSet, dayOfWeekSet};

            if ((best == null) || (compareStamps(thisBest, best) == 1)) {
                best = thisBest;
                bestField = WEEK_OF_MONTH;
            }
        }

        if (isSet(DAY_OF_WEEK_IN_MONTH)) {
            thisBest = new int[] {monthSet, dayOfWeekInMonthSet, dayOfWeekSet};

            if ((best == null) || (compareStamps(thisBest, best) == 1)) {
                best = thisBest;
                bestField = DAY_OF_WEEK_IN_MONTH;
            }
        }

        if (isSet(DAY_OF_YEAR)) {
            thisBest = new int[] {dayOfYearSet};

            if ((best == null) || (compareStamps(thisBest, best) == 1)) {
                best = thisBest;
                bestField = DAY_OF_YEAR;
            }
        }

        if (isSet(WEEK_OF_YEAR)) {
            thisBest = new int[] {weekOfYearSet, dayOfWeekSet};

            if ((best == null) || (compareStamps(thisBest, best) == 1)) {
                /* No need to store best since we're done comparing. */
                bestField = WEEK_OF_YEAR;
            }
        }

        return bestField;
    }

    /**
     * Compares to array of time stamps for equality.  This is down by comparing the
     * highest stamp in each array.  If they are equal, the next highest is compared,
     * etc.  A tie goes to the shorter array.  If the first array is "higher", -1 is
     * returned.  If both arrays are identical, 0 is returned.  If the second array is
     * "higher", 1 is returned.
     * @param stamp1 the first time stamp array to compare
     * @param stamp2 the second time stamp array to compare
     * @return -1 if the first array is "higher," 1 is the second array is "higher," and 0 if
     * they are equal
     */
    private static int compareStamps(int[] stamp1, int[] stamp2) {
        Arrays.sort(stamp1);
        Arrays.sort(stamp2);

        int comparison = 0;
        int count = 1;
        int length1 = stamp1.length;
        int length2 = stamp2.length;
        boolean unset1 = false;
        boolean unset2 = false;

        while ((count <= length1) && (stamp1[length1 - count] == UNSET)) {
            count++;
        }

        unset1 = (count > 1);
        length1 = length1 - count;
        count = 1;

        while ((count <= length2) && (stamp2[length2 - count] == UNSET)) {
            count++;
        }

        unset2 = (count > 1);
        length2 = length2 - count;

        // If 1 is complete and 2 is not
        if (!unset1 && unset2) {
            comparison = 1;
        }
        // If 2 is complete and 1 is not
        else if (unset1 && !unset2) {
            comparison = -1;
        }
        // If both are completely unset
        else if ((stamp1[length1] == UNSET) && (stamp2[length2] == UNSET)) {
            comparison = 0;
        }
        // If 1 is completely unset and 2 is not
        else if ((stamp1[length1] == UNSET) && (stamp2[length2] != UNSET)) {
            comparison = -1;
        }
        // If 2 is completely unset and 1 is not
        else if ((stamp1[length1] != UNSET) && (stamp2[length2] == UNSET)) {
            comparison = -1;
        }
        // If neither is completely set or unset
        else {
            count = 0;

            while (((length1 - count) >= 0) && ((length2 - count) >= 0)) {
                if (stamp1[length1 - count] > stamp2[length2 - count]) {
                    comparison = 1;
                    break;
                } else if (stamp1[length1 - count] < stamp2[length2 - count]) {
                    comparison = -1;
                    break;
                }

                count++;
            }
        }

        return comparison;
    }

    /**
     * Returns the length of the given month in days.
     * @param month the month of interest
     * @param isLeapYear whether the current year is a leap year
     * @return the length of the given month in days.
     */
    static int getLengthOfMonth(int month, boolean isLeapYear) {
        if ((month == JANUARY) || (month == MARCH) || (month == MAY) ||
                (month == JULY) || (month == AUGUST) || (month == OCTOBER) ||
                (month == DECEMBER)) {
            return 31;
        } else if ((month == APRIL) || (month == JUNE) || (month == SEPTEMBER) ||
                (month == NOVEMBER)) {
            return 30;
        } else if (isLeapYear) {
            return 29;
        } else {
            return 28;
        }
    }

    /**
     * Returns the length of the given year in days.
     * @param year the year of interest
     * @return the length of the given year in days
     */
    static int getLengthOfYear(int year) {
        if (isLeapYear(year)) {
            return 366;
        } else {
            return 365;
        }
    }

    /**
     * Sets the value of the given field.  If the field is unset before the call,
     * it will still be &quotunset&quot; after the call finishes.
     * @param field the field to set
     * @param value the value to which to set the field
     */
    protected void myInternalSet(int field, int value) {
        fields[field] = value;
    }

    /**
     * Returns the actual minimum value for the field, based on the current field
     * values.
     * @param field the field of interest
     * @return the actual minimum value for the field
     */
    public int getActualMinimum(int field) {
        switch (field) {
            case YEAR:
                if (isSet(CENTURY) && (internalGet(CENTURY) == 19)) {
                    return 70; // Jan 1, 1970
                }

                break;
            case DAY_OF_WEEK:
                if (isSet(CENTURY) && (internalGet(CENTURY) == 19) &&
                        isSet(YEAR) && (internalGet(YEAR) == 70) &&
                        isSet(MONTH) && (internalGet(MONTH) == JANUARY) &&
                        ((isSet(DAY_OF_MONTH) &&
                        (internalGet(DAY_OF_MONTH) <= 3)) ||
                        (isSet(WEEK_OF_MONTH) &&
                        (internalGet(WEEK_OF_MONTH) == 1)) ||
                        (isSet(DAY_OF_WEEK_IN_MONTH) &&
                        ((internalGet(DAY_OF_WEEK_IN_MONTH) == 1) ||
                        (internalGet(DAY_OF_WEEK_IN_MONTH) == -5))) ||
                        (isSet(DAY_OF_YEAR) &&
                        (internalGet(DAY_OF_YEAR) <= 3)) ||
                        (isSet(WEEK_OF_YEAR) &&
                        (internalGet(WEEK_OF_YEAR) == 1)))) {
                    return THURSDAY; // Jan 1, 1970
                }

                break;
        }

        return this.getMinimum(field);
    }

    /**
     * Returns the actual maximum value for the field, based on the current field
     * values.
     * @param field the field of interest
     * @return the actual maximum value for the field
     */
    public int getActualMaximum(int field) {
        switch (field) {
            case WEEK_OF_YEAR:
                if (isSet(YEAR) && isSet(CENTURY) && isSet(MONTH) &&
                        isLeapYear(this.getYear()) &&
                        (calculateDayOfWeek(1, JANUARY,
                        internalGet(YEAR)) == SUNDAY)) {
                    return 54;
                }

                return 53;
            case WEEK_OF_MONTH:
            case DAY_OF_WEEK_IN_MONTH:
                if (isSet(MONTH) && isSet(YEAR) && isSet(CENTURY)) {
                    int numWeeks = 4;
                    int month = internalGet(MONTH);
                    int year = this.getYear();

                    // If the month doesn't start on a SUNDAY, there's a partial
                    // week at the beginning.
                    if (this.calculateDayOfWeek(1, month, year) != SUNDAY) {
                        numWeeks++;
                    }

                    // If the month doesn't end on a SATURDAY, there's a partial
                    // week at the end.
                    if (this.calculateDayOfWeek(getLengthOfMonth(month, isLeapYear(year)),
                            month, year) != SATURDAY) {
                        numWeeks++;
                    }

                    return numWeeks;
                } else if (isSet(MONTH) && (internalGet(MONTH) == FEBRUARY)) {
                    return 5;
                }
                /* If we don't know the year, there's always a chance that the month
                 * could have 6 weeks. */
                else {
                    return 6;
                }
            case DAY_OF_MONTH:
                if (isSet(MONTH) && isSet(YEAR) && isSet(CENTURY)) {
                    return getLengthOfMonth(internalGet(MONTH), isLeapYear(this.getYear()));
                } else if (isSet(MONTH)) {
                    return getLengthOfMonth(internalGet(MONTH), false);
                } else {
                    return 31;
                }
            case DAY_OF_YEAR:
                if (isSet(YEAR) && isSet(CENTURY) &&
                        isLeapYear(this.getYear())) {
                    return 366;
                }

                return 365;
        }

        return this.getMaximum(field);
    }

    /**
     * Gets the current year from the YEAR and CENTURY fields as YEAR + CENTURY * 100.
     * @return the current year
     */
    private int getYear() {
        return internalGet(YEAR) + internalGet(CENTURY) * 100;
    }

    /**
     * Compares two PartialTimestamp objects.  They are equal if they both have all of
     * the same field values and field modifier values.
     * @param obj the object against which to compare
     * @return whether the given object equals this object
     */
    public boolean equals(Object obj) {
        if (!(obj instanceof PartialTimestamp)) {
            return false;
        } else {
            PartialTimestamp pt = (PartialTimestamp)obj;

            for (int field = CENTURY; field < FIELD_COUNT; field++) {
                if (this.isSet(field) && pt.isSet(field) &&
                        (this.get(field) != pt.get(field))) {
                    return false;
                } else if (this.isSet(field) != pt.isSet(field)) {
                    return false;
                }

                if (this.getModifier(field) != pt.getModifier(field)) {
                    return false;
                }
            }
            return ((this.lastHourSet == pt.lastHourSet) &&
                    (this.getLastDayFieldSet() == pt.getLastDayFieldSet()));
        }
    }

    /**
     * Makes a complete copy of this object.
     * @return a complete copy of this object
     */
    public Object clone() {
        PartialTimestamp pt = (PartialTimestamp)super.clone();

        pt.modifiers = new int[FIELD_COUNT];

        System.arraycopy(this.modifiers, 0, pt.modifiers, 0, FIELD_COUNT);

        pt.lastHourSet = this.lastHourSet;
        pt.fieldsModified = this.fieldsModified;
        pt.dayOfMonthSet = this.dayOfMonthSet;
        pt.dayOfWeekInMonthSet = this.dayOfWeekInMonthSet;
        pt.dayOfYearSet = this.dayOfYearSet;
        pt.weekOfMonthSet = this.weekOfMonthSet;
        pt.weekOfYearSet = this.weekOfYearSet;
        pt.monthSet = this.monthSet;
        pt.dayStamp = this.dayStamp;

        return pt;
    }
}
