/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Sean Dague <http://dague.net/sean>
 */

#include <glib.h>
#include <time.h>
#include <SaHpi.h>

#include <openhpi.h>
#include <snmp_util.h>
#include <bc_resources.h>
#include <snmp_bc.h>
#include <snmp_bc_sel.h>
#include <snmp_bc_time.h>

static int get_bc_sel_size(struct snmp_session *ss)
{
        struct snmp_value run_value;
        char oid[50];
        int i = 1;
        /*
          yes there clearly must be a much less dumb way of doing this.
          wait for Konrad next week to figure this out 
        */
        do {
                sprintf(oid, "%s.%d", BC_SEL_INDEX_OID, i);
                i++;
        } while(snmp_get(ss,oid,&run_value) == 0);
        
        /* think about it, and it makes sense */
        i -= 2;
        return i;
}

int snmp_bc_get_sel_info(void *hnd, SaHpiResourceIdT id, SaHpiSelInfoT *info) 
{
        struct snmp_value first_value;
        struct oh_handler_state *handle = hnd;
        struct snmp_bc_hnd *custom_handle = (struct snmp_bc_hnd *)handle->data;
        bc_sel_entry sel_entry;
        struct tm curtime;
        char oid[50];
        int i = 1;
        
        SaHpiSelInfoT sel = {
                .Size = 512, /* this is clearly a guess but looks about right 
                              * from the 75% full errors I've seen */
                .Enabled = SAHPI_TRUE,
                .OverflowFlag = SAHPI_FALSE,
                .OverflowAction = SAHPI_SEL_OVERFLOW_DROP,
                .DeleteEntrySupported = SAHPI_FALSE
        };
        
        sprintf(oid, "%s.%d", BC_SEL_ENTRY_OID, i);
        /* we need first value to figure out what our update time is */
        snmp_get(custom_handle->ss,oid,&first_value);
        
        if(first_value.type == ASN_OCTET_STR) {
                if(snmp_bc_parse_sel_entry(custom_handle->ss,first_value.string, &sel_entry) < 0) {
                        dbg("Couldn't get first date");
                } else {
                        sel.UpdateTimestamp = 
                                (SaHpiTimeT) mktime(&sel_entry.time) * 1000000000;
                }
        }
        
        if(get_bc_sp_time(custom_handle->ss,&curtime) == 0) {
                sel.CurrentTime = 
                        (SaHpiTimeT) mktime(&curtime) * 1000000000;
        }
        
        sel.Entries = get_bc_sel_size(custom_handle->ss);
        *info = sel;
        
        return 0;
}

/**
 * snmp_bc_get_sel_entry:
 * @hnd: 
 * @id: 
 * @current: 
 * @prev: 
 * @next: 
 * @entry: 
 * 
 * See saHpiEventLogEntryGet for params
 * 
 * Return value: 0 on success, < 0 on error
 **/
int snmp_bc_get_sel_entry(void *hnd, SaHpiResourceIdT id, SaHpiSelEntryIdT current,
                          SaHpiSelEntryIdT *prev, SaHpiSelEntryIdT *next,
                          SaHpiSelEntryT *entry)
{
        struct snmp_value get_value;
        struct oh_handler_state *handle = hnd;
        struct snmp_bc_hnd *custom_handle = handle->data;
        bc_sel_entry sel_entry;
        SaHpiSelEntryT tmpentry;
        char oid[50];
        int oidinst = 1;
        
        if(current == SAHPI_OLDEST_ENTRY) {
                oidinst = get_bc_sel_size(custom_handle->ss);
                *next = oidinst - 1;
                *prev = SAHPI_NO_MORE_ENTRIES;
        } else if(current == SAHPI_NEWEST_ENTRY) {
                oidinst = 1;
                *next = SAHPI_NO_MORE_ENTRIES;
                *prev = oidinst + 1;
        } else {
                int final = get_bc_sel_size(custom_handle->ss);
                if(current > final) {
                        dbg("Gone past the end of the log");
                        return -1;
                }
                oidinst = current;
                if(current < final) {
                        *prev = current + 1;
                } else {
                        *prev = SAHPI_NO_MORE_ENTRIES;
                }
                
                if(current > 1) {
                        *next = current - 1;
                } else {
                        *next = SAHPI_NO_MORE_ENTRIES;
                }
        }

        sprintf(oid, "%s.%d", BC_SEL_ENTRY_OID, oidinst);
        /* we need first value to figure out what our update time is */
        snmp_get(custom_handle->ss,oid,&get_value);
        
        if(get_value.type == ASN_OCTET_STR) {
                if(snmp_bc_parse_sel_entry(custom_handle->ss,get_value.string, &sel_entry) < 0) {
                        dbg("Couldn't parse SEL Entry");
                        return -1;
                }
        } else {
                dbg("Couldn't fetch SEL Entry");
                return -1;
        }

        tmpentry.EntryId = current;
        tmpentry.Timestamp = (SaHpiTimeT) mktime(&sel_entry.time) * 1000000000;
        tmpentry.Event.Severity = sel_entry.sev;
        tmpentry.Event.Timestamp = (SaHpiTimeT) mktime(&sel_entry.time) * 1000000000;
        /* TODO: do this for real, sel_entry.source will give this sort of */
        tmpentry.Event.Source = id;
        tmpentry.Event.EventType = SAHPI_ET_OEM;
        tmpentry.Event.EventDataUnion.OemEvent.MId = 2;
        /* TODO: break down events better.  It would be nice if there was
           a TEXT type event for informational messages */
        strncpy(tmpentry.Event.EventDataUnion.OemEvent.OemEventData,
                sel_entry.text, SAHPI_OEM_EVENT_DATA_SIZE - 1);
        tmpentry.Event.EventDataUnion.OemEvent.OemEventData
                [SAHPI_OEM_EVENT_DATA_SIZE - 1] = '\0';
        *entry = tmpentry;

        return 0;
}


/**
 * snmp_bc_set_sel_time:
 * @hnd: 
 * @id: 
 * @time: 
 * 
 * 
 * 
 * Return value: 
 **/
int snmp_bc_set_sel_time(void *hnd, SaHpiResourceIdT id, SaHpiTimeT time)
{
        struct oh_handler_state *handle = hnd;
        struct snmp_bc_hnd *custom_handle = (struct snmp_bc_hnd *)handle->data;
        struct tm tv;
        time_t tt;
        SaErrorT returncode;

        tt = time / 1000000000;
        
        localtime_r(&tt, &tv);

        if (set_bc_sp_time(custom_handle->ss,&tv) == 0)
                returncode = SA_OK;
        else
                returncode = SA_ERR_HPI_ERROR;
                 
        return returncode;
}

/**
 * snmp_bc_add_sel_entry:
 * @hnd: 
 * @id: 
 * @Event: 
 * 
 * Add is not supported with tihs hardware, so -1 is always returned
 * 
 * Return value: -1
 **/
int snmp_bc_add_sel_entry(void *hnd, SaHpiResourceIdT id, const SaHpiSelEntryT *Event)
{
        return SA_ERR_HPI_INVALID_CMD;
}

/**
 * snmp_bc_del_sel_entry:
 * @hnd: 
 * @id: 
 * @sid: 
 * 
 * Delete is not supported with this hardware, so -1 is always returned
 * 
 * Return value: -1
 **/
int snmp_bc_del_sel_entry(void *hnd, SaHpiResourceIdT id, SaHpiSelEntryIdT sid)
{
        return SA_ERR_HPI_INVALID_CMD;
}


/**
 * snmp_bc_parse_sel_entry:
 * @text: text as returned by snmpget call for an event log entry
 * @sel: blade center system event log
 * 
 * This call is used to create a blade center sel entry from the returned
 * snmp string.  Another transform will have to happen to turn this into 
 * an SAHPI sel entry. 
 * 
 * Return value: 0 for success, -1 for format error, -2 for premature data termination
 **/
int snmp_bc_parse_sel_entry(struct snmp_session *ss, char * text, bc_sel_entry * sel) 
{
        bc_sel_entry ent;
        char level[8];
        char * start = text;
        
        /* Severity first */
        if(sscanf(start,"Severity:%7s",level)) {
                if(strcmp(level,"INFO") == 0) {
                        ent.sev = SAHPI_INFORMATIONAL;
                } else if(strcmp(level,"WARN") == 0) {
                        ent.sev = SAHPI_MINOR;
                } else if(strcmp(level,"ERR") == 0) {
                        ent.sev = SAHPI_CRITICAL;
                } else {
                        ent.sev = SAHPI_DEBUG;
                }
        } else {
                dbg("Couldn't parse Severity from Blade Center Log Entry");
                return -1;
        }
                
        while(start && (strncmp(start,"Source:",7) != 0)) { start++; }
        if(!start) { return -2; }

        if(!sscanf(start,"Source:%19s",ent.source)) {
                dbg("Couldn't parse Source from Blade Center Log Entry");
                return -1;
        }

        while(start && (strncmp(start,"Name:",5) != 0)) { start++; }
        if(!start) { return -2; }

        if(!sscanf(start,"Name:%19s",ent.sname)) {
                dbg("Couldn't parse Name from Blade Center Log Entry");
                return -1;
        }
        
        while(start && (strncmp(start,"Date:",5) != 0)) { start++; }
        if(!start) { return -2; }
        
        if(sscanf(start,"Date:%2d/%2d/%2d  Time:%2d:%2d:%2d",
                  &ent.time.tm_mon, &ent.time.tm_mday, &ent.time.tm_year, 
                  &ent.time.tm_hour, &ent.time.tm_min, &ent.time.tm_sec)) {
                /* TODO: real test for DST */
                /* ent.time.tm_isdst = 0; */
		set_bc_dst(ss, &ent.time);
                /* that word, I do not think it means what you think it means */
                ent.time.tm_mon--;
                ent.time.tm_year += 100;
        } else {
                dbg("Couldn't parse Date/Time from Blade Center Log Entry");
                return -1;
        }
        
        while(start && (strncmp(start,"Text:",5) != 0)) { start++; }
        if(!start) { return -2; }
        
        /* advance to data */
        start += 5;
        strncpy(ent.text,start,BC_SEL_ENTRY_STRING - 1);
        ent.text[BC_SEL_ENTRY_STRING - 1] = '\0';
        
        *sel = ent;
        return 0;
}

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
	gboolean rc = FALSE;
	guchar zone_index;

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
int set_bc_dst(struct snmp_session *ss, struct tm *time) {

        struct snmp_value get_value;
	int rc = 0;
	gchar **zone_token;

        snmp_get(ss,SNMP_BC_TIME_DST,&get_value);

	if (get_value.type == ASN_OCTET_STR)  {
		zone_token = g_strsplit(get_value.string, ",", 3);
		if (zone_token[1] == NULL) {
			/* Daylight Saving Time info is not provided */
			time->tm_isdst = -1;		
		} else {
			if (strcmp(zone_token[1],"yes") == 0) {
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
		rc = 0;
	} else {
                dbg("Couldn't fetch Date/Time from Blade Center SP");
                rc = -1;		
	}
	return rc;
}

int get_bc_sp_time(struct snmp_session *ss, struct tm *time) {
        struct snmp_value get_value;
        struct tm tmptime;

        snmp_get(ss,BC_DATETIME_OID,&get_value);
        if(get_value.type == ASN_OCTET_STR) {
                if(sscanf(get_value.string,"%2d/%2d/%4d,%2d:%2d:%2d",
                          &tmptime.tm_mon, &tmptime.tm_mday, &tmptime.tm_year, 
                          &tmptime.tm_hour, &tmptime.tm_min, &tmptime.tm_sec)) {
                        /* TODO: real test for DST */
                        /* tmptime.tm_isdst = 0;   */
			set_bc_dst(ss, &tmptime);

                        /* that word, I do not think it means what you think it means */
                        tmptime.tm_mon--;
                        tmptime.tm_year -= 1900;
                } else {
                        dbg("Couldn't parse Date/Time from Blade Center SP");
                        return -1;
                }
        } else {
                dbg("Couldn't fetch Blade Center SP Date/Time Entry");
                return -1;
        }
        *time = tmptime;
        return 0;
}

int set_bc_sp_time(struct snmp_session *ss, struct tm *time) {

        struct snmp_value set_value;
        int returncode = 0;

        set_value.type = ASN_OCTET_STR;
        strftime(set_value.string, sizeof(set_value.string), "%m/%d/%Y,%H:%M:%S", time);

        if (snmp_set(ss,BC_DATETIME_OID,set_value) == 0)
        {
                returncode = 0;
        } else {
                dbg("snmp_set is NOT successful\n");
                returncode = -1;
        }
        return returncode;

}
