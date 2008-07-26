/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Sean Dague <http://dague.net/sean>
 */

#include <snmp_bc_plugin.h>

/* 
 * Enums and Constants
 */ 

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
        { 2,  0, SECOND_WEEK,  SUNDAY,    MARCH,
          2,  0, FIRST_WEEK,   SUNDAY,   NOVEMBER   },
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
 * Determines if the given year is a leap year
 */ 
gboolean is_leap_year(guchar year)
{
	/*
	 * Leap years occur in years exactly divisible by 4,
	 * except that years ending in 00 are leap years ONLY if
	 * they are divisible by 400
	 */
	if ((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0))
		return(TRUE);
	else
		return(FALSE);
} // End is_leap_year

/*                                                                            
 * Calculates day of month given month/year and weekday/week
 *
 * Note: This routine does not do any error checking on the inputs.
 *       weekday assumed to be in DST_WEEKDAY
 *       week assumed to be in DST_WEEK
 *       month assumed to be in DST_MONTH
 *       year assumed to be 0-99
 */ 
guchar get_day_of_month(guchar weekday, guchar week, guchar month, guchar year)
{
	guchar	month_adj;
	guchar	index;
	guchar	day;

	/* 
	 * Calculate month adjustment
	 */
	month_adj = 0;
	for (index = 0; (index < (month - 1)); index++)
			month_adj += 35 - days_in_month[index];
	if ((month > 2) && (is_leap_year(year) == TRUE))
		month_adj--;  // Allow for this year's leap day

	/*
	 * Calculate day
	 */
	day = (weekday+14);             /* Initialize (+14 to avoid going neg below) */
	
	/*
	 * Century adjustment. 90-99 is 1990-1999.
	 * 00-89 is 2000-2089.  Each century moves
         * day calculation 1 place.
	 */
	if (year >= 2) day--; 

	day += month_adj;               /* Month adjustment */
	day -= (year%7);                /* Year adjustment (every year moves day 1 place) */
	day -= (((year+3)/4) % 7);      /* Add effects of leap year */
	day %= 7;                       /* Convert to weekday (0-6) */
	day += ((week-1) * 7);          /* Add in whole weeks       */
	day++;                          /* Adjust weekday (1-7)     */

	/*
	 * May go past end if using LAST_WEEK, adjust if needed
	 */
	if (day > days_in_month[month-1])
					day -= 7;

	return(day);
} // End get_day_of_month


gboolean is_dst_in_effect(struct tm *time, gchar **zone_token) {

	guchar 	year;
	guchar	start_hour, end_hour;
	guchar	start_day, end_day;
	guchar	start_week, end_week;
	guchar	start_wkday, end_wkday;
	guchar	start_month, end_month;
	gboolean rc;
	guchar zone_index;

	rc = FALSE;
	year = time->tm_year;
	
	if (zone_token[2] == NULL) {
		zone_index = 1;
	} else {
		zone_index = atoi(zone_token[2]);
	}
	if (zone_index != 0) zone_index--;
	
	start_hour  = DST_TABLE[zone_index].start_hour;
	start_day   = DST_TABLE[zone_index].start_day;
	start_week  = DST_TABLE[zone_index].start_week;
	start_wkday = DST_TABLE[zone_index].start_weekday;
	start_month = DST_TABLE[zone_index].start_month;
	end_hour    = DST_TABLE[zone_index].end_hour;
	end_day     = DST_TABLE[zone_index].end_day;
	end_week    = DST_TABLE[zone_index].end_week;
	end_wkday   = DST_TABLE[zone_index].end_weekday;
	end_month   = DST_TABLE[zone_index].end_month;

	/*
	 * If start_day not provided, use information from start_week and
	 * start_weekday to calculate start_day
	 */
	if (start_day == 0)
		start_day = get_day_of_month(start_wkday, start_week, start_month, year);

	/*
	 * If end_day not provided, use information from end_week and
	 * end_weekday to calculate end_day
	 */
	if (end_day == 0)
		end_day = get_day_of_month(end_wkday, end_week, end_month, year);

	// It is daylight saving time if:
	//    the month consists entirely of daylight saving days
	//    it is the starting month but after the begin date
	//    it is the begin date but past the starting hour
	//    it is the ending month but before the end date
	//    it is the end date but before the ending hour
	if (((end_month > start_month) && // Northern hemisphere check
		(time->tm_mon > start_month) && (time->tm_mon < end_month)) ||
		((start_month > end_month) && // Southern hemisphere check
		((time->tm_mon > start_month) || (time->tm_mon < end_month))) ||
		((time->tm_mon == start_month) && ((time->tm_mday > start_day) ||
		((time->tm_mday == start_day) && (time->tm_hour >= start_hour)))) ||
		((time->tm_mon == end_month) && ((time->tm_mday < end_day) ||
		((time->tm_mday == end_day) && (time->tm_hour < (end_hour-1))))))
	{
		rc = TRUE;
	}

	return(rc);
}

SaErrorT snmp_bc_set_dst(struct oh_handler_state *handle, struct tm *time) {

	gchar **zone_token;
	struct snmp_bc_hnd *custom_handle = handle->data;


	zone_token = g_strsplit(custom_handle->handler_timezone, ",", 3);
	if (zone_token[1] == NULL) {
			/* Daylight Saving Time info is not provided */
		time->tm_isdst = -1;		
	} else {
		if (g_ascii_strncasecmp(zone_token[1], "yes", sizeof("yes")) == 0) {
			/* Does the timestamp of this particular log */
			/* fall within the observes DST period for this timezone? */
			if (is_dst_in_effect(time, zone_token) == TRUE) 
				time->tm_isdst = 1;
			else 
				time->tm_isdst = 0;
		} else { 
			/* Daylight Saving Time info is not observed */
			/* Assuming "Not Observed" == "Not In DST"   */
			time->tm_isdst = 0;		
		}
	}
	g_strfreev(zone_token);
	return(SA_OK); 
}

/**
 * snmp_bc_get_sp_time:
 * @handle: Pointer to handler's state.
 * @time: Pointer to tm struct to store data.
 *
 * Returns SaHpiEventLogInfoT information about Event Log.
 * 
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Any pointer parameter is NULL.
 * SA_ERR_HPI_INTERNAL_ERROR - If cannot parse date and time returned from bc
 **/
SaErrorT snmp_bc_get_sp_time(struct oh_handler_state *handle, struct tm *time)
{
	
        struct snmp_value get_value;
        struct tm tmptime;
	SaErrorT rv = SA_OK;
        struct snmp_bc_hnd *custom_handle;

	if (!handle || !time)
		return(SA_ERR_HPI_INVALID_PARAMS);

	        
	rv = SA_OK;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	if (custom_handle->platform == SNMP_BC_PLATFORM_RSA) {
		rv = snmp_bc_snmp_get(custom_handle, SNMP_BC_DATETIME_OID_RSA, &get_value, SAHPI_TRUE);
	}
	else {
		rv = snmp_bc_snmp_get(custom_handle, SNMP_BC_DATETIME_OID, &get_value, SAHPI_TRUE);
	}

        if ( (rv == SA_OK) && (get_value.type == ASN_OCTET_STR) ) {
                if(sscanf(get_value.string,"%2d/%2d/%4d,%2d:%2d:%2d",
                          &tmptime.tm_mon, &tmptime.tm_mday, &tmptime.tm_year, 
                          &tmptime.tm_hour, &tmptime.tm_min, &tmptime.tm_sec)) {
			snmp_bc_set_dst(handle, &tmptime);
                        tmptime.tm_mon--;
                        tmptime.tm_year -= 1900;
                } else {
                        err("Couldn't parse Date/Time from Blade Center SP");
                        return(SA_ERR_HPI_INTERNAL_ERROR);
                }
        } else {
                err("Couldn't fetch Blade Center SP Date/Time Entry");
		if (rv == SA_OK) rv = SA_ERR_HPI_INTERNAL_ERROR;
                return(rv);
        }
        *time = tmptime;
        return(SA_OK);
}

/**
 * snmp_bc_set_sp_time:
 * @ss: Pointer to session info.
 * @time: Pointer to tm struct to store data.
 *
 * 
 * 
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Any pointer parameter is NULL.
 * SA_ERR_HPI_INTERNAL_ERROR - If cannot parse date and time returned from bc
 * Returncode from snmp_set()
 **/
SaErrorT snmp_bc_set_sp_time(struct snmp_bc_hnd *custom_handle, struct tm *time) 
{
        struct snmp_value set_value;
        SaErrorT returncode;

	if (!custom_handle || !time)
		return(SA_ERR_HPI_INVALID_PARAMS);
		
	returncode = SA_OK;
        set_value.type = ASN_OCTET_STR;
	
        strftime(set_value.string, sizeof(set_value.string), "%m/%d/%Y,%H:%M:%S", time);
	set_value.str_len = 19;
	
	if (custom_handle->platform == SNMP_BC_PLATFORM_RSA) {
		returncode = snmp_bc_snmp_set(custom_handle, SNMP_BC_DATETIME_OID_RSA, set_value);
	}
	else {
		returncode = snmp_bc_snmp_set(custom_handle, SNMP_BC_DATETIME_OID, set_value);
	}

        if (returncode != SA_OK)
                err("snmp_set is NOT successful\n");

       return returncode;
}
