/*      -*- linux-c -*-
 *
 *
 * (C) Copyright IBM Corp. 2003
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 *
 * Authors:
 *      peter d phan   <pdphan@users.sf.net>
 *      
 * Logs:
 *	 Duplicate enums from IBM Blade Center
 */

#ifndef SNMP_BC_TIME_H
#define SNMP_BC_TIME_H

/*                                                                 
 * Set timezone constants
 */                                                               
typedef enum {
	DST_NONE = 0,
	DST_USA,
	DST_ESA,
	DST_MID,
	DST_EEC,
	DST_EEU,
	DST_EGT,
	DST_FLE,
	DST_IRN,
	DST_AUS,
	DST_TAS,
	DST_NWZ,
	DST_AUTOMATIC              // Must be last in list, used to validate entry
} DST_STANDARDS;
    
typedef enum {
	FIRST_WEEK = 1,
	SECOND_WEEK,
	THIRD_WEEK,
	FOURTH_WEEK,
	LAST_WEEK
} DST_WEEK;
    
typedef enum {
	SUNDAY = 1,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY
} DST_WEEKDAY;
    
typedef enum {
	JANUARY = 1,
	FEBRUARY,
	MARCH,
	APRIL,
	MAY,
	JUNE,
	JULY,
	AUGUST,
	SEPTEMBER,
	OCTOBER,
	NOVEMBER,
	DECEMBER
} DST_MONTH;

/*                                                               
 * Daylight saving time standards table entry
 *
 * This structure contains the definition of how daylight saving
 * time is observed for the supported timezones.
 */
typedef struct tag_DST_ENTRY {
	unsigned char start_hour;           // Hour daylight saving begins
	unsigned char start_day;            // Specific day daylight saving begins
	unsigned char start_week;           // Week number daylight saving begins
	unsigned char start_weekday;        // Day of week daylight saving begins
	unsigned char start_month;          // Month daylight saving begins
	unsigned char end_hour;             // Hour daylight saving ends
	unsigned char end_day;              // Specific day daylight saving ends
	unsigned char end_week;             // Week number daylight saving ends
	unsigned char end_weekday;          // Day of week daylight saving ends
	unsigned char end_month;            // Month daylight saving ends
} DST_ENTRY;

/*
 * Table of days in each month.
 */
const unsigned short days_in_month[12] = /* Table of days in each month. */
{
	31,     /* January              */
	28,     /* February             */
	31,     /* March                */
	30,     /* April                */
	31,     /* May                  */
	30,     /* June                 */
	31,     /* July                 */
	31,     /* August               */
	30,     /* September            */
	31,     /* October              */
	30,     /* November             */
	31      /* December             */
};

/*       
 * Daylight saving time standards table
 *
 * This structure contains the definition of how daylight saving
 * time is observed for the supported timezones.
 *
 * If you add or remove any entries from this table you must also
 * change DST_STANDARDS (contains the indices for this table).
 */                                                              
const DST_ENTRY DST_TABLE[] =
{
	/*                                                               
	* DST_USA:
	*   Alaskan, Pacific, Mountain, Central, Eastern,
	*   Atlantic, Newfoundland
	*/                                                          
	{ 2,  0, FIRST_WEEK,  SUNDAY,    APRIL,
	  2,  0, LAST_WEEK,   SUNDAY,    OCTOBER   },
	/*                                                           
	 * DST_ESA:
	 *   E. South America
	 */
	{ 2,  0, THIRD_WEEK,  SUNDAY,    OCTOBER,
	  2,  0, SECOND_WEEK, SUNDAY,    FEBRUARY  },
	/*                                                            
	 * DST_MID:
	 *   Mid-Atlantic
	 */                                                            
	{ 2,  0, LAST_WEEK,   SUNDAY,    MARCH,
	  2,  0, LAST_WEEK,   SUNDAY,    SEPTEMBER },
	/* 
	 * DST_EEC:
	 *   Azores, GMT, Romance, Central European, GTB,
	 *   W. Europe, Arab, Russian, Ekateinburg, Yakutsk
	 */                                                            
	{ 2,  0, LAST_WEEK,   SUNDAY,    MARCH,
	  3,  0, LAST_WEEK,   SUNDAY,    OCTOBER   },
	/*                                                            
	 * DST_EEU:
	 *   E. Europe
	 */                                                           
	{ 0,  0, LAST_WEEK,   SUNDAY,    MARCH,
	  1,  0, LAST_WEEK,   SUNDAY,    SEPTEMBER },
	/*                                                           
	 * DST_EGT:
	 *   Egypt
	 */                                                             
	{ 2,  0, FIRST_WEEK,  FRIDAY,    MAY,
	  2,  0, LAST_WEEK,   WEDNESDAY, SEPTEMBER },
	/*                                                           
	 * DST_FLE:
	 *   FLE
	 */                                                            
	{ 3,  0, LAST_WEEK,   SUNDAY,    MARCH,
	  4,  0, LAST_WEEK,   SUNDAY,    OCTOBER   },
	/*                                                           
	 * DST_IRN:
	 *   Iran
	 */                                                             
	{ 2,  0, FIRST_WEEK,  SUNDAY,    MARCH,
	  2,  0, LAST_WEEK,   TUESDAY,   SEPTEMBER },
	/*                                                            
	 * DST_AUS:
	 *   Cen. Australia, AUS Eastern
	 */                                                          
	{ 2,  0, LAST_WEEK,   SUNDAY,    OCTOBER,
	  2,  0, LAST_WEEK,   SUNDAY,    MARCH     },
	/*                                                           
	 * DST_TAS:
	 *   Tasmania
	 */                                                            
	{ 2,  0, FIRST_WEEK,  SUNDAY,    OCTOBER,
	  2,  0, LAST_WEEK,   SUNDAY,    MARCH     },
	/*                                                            
	 * DST_NWZ:
	 *   New Zealand
	 */                                                              
	{ 2,  0, LAST_WEEK,   SUNDAY,    MARCH,
	  3,  0, LAST_WEEK,   SUNDAY,    OCTOBER   }
};

/* 
 * Function Prototyping
 */
 
int set_bc_dst(struct snmp_session *, struct tm *);
gboolean is_dst_in_effect(struct tm *, gchar **);
gboolean is_leap_year(guchar );
guchar   get_day_of_month(guchar, guchar, guchar, guchar);

#endif
