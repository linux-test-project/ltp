/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <snmp_bc_plugin.h>

static SaErrorT snmp_bc_parse_threshold_str(gchar *str,
					    gchar *root_str,
					    SaHpiTextBufferT *read_value_str,
					    SaHpiTextBufferT *trigger_value_str);

static SaErrorT snmp_bc_logsrc2rid(struct oh_handler_state *handle,
				   gchar *src,
				   LogSource2ResourceT *logsrc2res,
				   unsigned long long ovr_flags);

static SaErrorT snmp_bc_set_cur_prev_event_states(struct oh_handler_state *handle,
						  EventMapInfoT *eventmap_info,
						  SaHpiEventT *event,
						  int recovery_event);

static SaErrorT snmp_bc_set_event_severity(struct oh_handler_state *handle,
					   EventMapInfoT *eventmap_info,
					   SaHpiEventT *event,
					   SaHpiSeverityT *event_severity);

static SaErrorT snmp_bc_map2oem(SaHpiEventT *event,
				sel_entry *sel_entry,
				OEMReasonCodeT reason);

static ErrLog2EventInfoT *snmp_bc_findevent4dupstr(gchar *search_str,
						     ErrLog2EventInfoT *dupstrhash_data,
						     LogSource2ResourceT *logsrc2res);
/**
 * event2hpi_hash_init:
 * @handle: Pointer to handler's data.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) or event2hpi_hash_ptr are NULL.
 * SA_ERR_HPI_OUT_OF_MEMORY   - Can not malloc space
 **/
SaErrorT event2hpi_hash_init(struct oh_handler_state *handle)
{
	struct snmp_bc_hnd *custom_handle;

	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	custom_handle->event2hpi_hash_ptr = g_hash_table_new(g_str_hash, g_str_equal);
	if (custom_handle->event2hpi_hash_ptr == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	return(SA_OK);
}

/**
 * free_hash_data:
 * @key
 * @value
 * @user_data
 *
 * Return values: none
 **/
static void free_hash_data(gpointer key, gpointer value, gpointer user_data)
{
        g_free(key); /* Memory was created for these during normalization process */
        g_free(value);
}

/**
 * event2hpi_hash_free:
 * @handle: Pointer to handler's data.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) or event2hpi_hash_ptr are NULL.
 **/
SaErrorT event2hpi_hash_free(struct oh_handler_state *handle)
{
	struct snmp_bc_hnd *custom_handle;
	
	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if (custom_handle->event2hpi_hash_ptr != NULL) {
		g_hash_table_foreach(custom_handle->event2hpi_hash_ptr, free_hash_data, NULL);
		g_hash_table_destroy(custom_handle->event2hpi_hash_ptr);
	}

	return(SA_OK);
}

/**
 * snmp_bc_discover_res_events: 
 * @handle: Pointer to handler's data.
 * @ep: Pointer to resource's entity path.
 * @resinfo: Pointer to a resource's event mapping information.
 *
 * Discovers a resource's events and records the static mapping
 * information needed to translate BladeCenter event log messages
 * into HPI event types. It is assumed that all resource events are
 * hot swap events.
 * 
 * Mapping information is stored in the hash table - event2hpi_hash.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) or event2hpi_hash_ptr are NULL.
 **/
SaErrorT snmp_bc_discover_res_events(struct oh_handler_state *handle,
				     SaHpiEntityPathT *ep,
				     const struct ResourceInfo *resinfo)
{
	int i;
	int max;
	char *normalized_str;
	char *hash_existing_key, *hash_value;
	EventMapInfoT *eventmap_info;
	SaHpiResourceIdT rid;
	struct snmp_bc_hnd *custom_handle;

	if (!handle || !ep || !resinfo) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle || !custom_handle->event2hpi_hash_ptr) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	rid = oh_uid_lookup(ep);
	if (rid == 0) {
		err("No RID.");
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	max = SNMP_BC_MAX_RESOURCE_EVENT_ARRAY_SIZE;
	for (i=0; resinfo->event_array[i].event != NULL && i < max; i++) {
		/* Normalized and convert event string */
		normalized_str = oh_derive_string(ep, 0, 16, resinfo->event_array[i].event);
		if (normalized_str == NULL) {
			err("Cannot derive %s.", resinfo->event_array[i].event);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		} 

		/*  Add to hash; Set HPI values */
		if (!g_hash_table_lookup_extended(custom_handle->event2hpi_hash_ptr,
						  normalized_str,
						  (gpointer)&hash_existing_key,
						  (gpointer)&hash_value)) {

			eventmap_info = g_malloc0(sizeof(EventMapInfoT));
			if (!eventmap_info) {
				err("Out of memory.");
				g_free(normalized_str);
				return(SA_ERR_HPI_OUT_OF_MEMORY);
			}

			eventmap_info->hpievent.Source = rid;
			eventmap_info->ep = *ep;
			eventmap_info->hpievent.EventType = SAHPI_ET_HOTSWAP;

			eventmap_info->hpievent.EventDataUnion.HotSwapEvent.HotSwapState = resinfo->event_array[i].event_state;
			eventmap_info->hs_event_auto_state = resinfo->event_array[i].event_auto_state;
			eventmap_info->hs_recovery_state = resinfo->event_array[i].recovery_state;
			eventmap_info->hs_recovery_auto_state = resinfo->event_array[i].recovery_auto_state;
			eventmap_info->event_res_failure = resinfo->event_array[i].event_res_failure;
			eventmap_info->event_res_failure_unexpected = resinfo->event_array[i].event_res_failure_unexpected;

			dbg("Discovered resource event=%s.", normalized_str);

			g_hash_table_insert(custom_handle->event2hpi_hash_ptr, normalized_str, eventmap_info);
			/* normalized_str space is recovered when hash is freed */
		}
		else {
			/* Event already exists (e.g. same event for multiple blades) */
			dbg("Event already exists=%s.", normalized_str);
			g_free(normalized_str);
		}
	}

	return(SA_OK);
}

/**
 * snmp_bc_discover_sensor_events: 
 * @handle: Pointer to handler's data.
 * @ep: Pointer to parent resource's entity path.
 * @sid: Sensor ID.
 * @sinfo: Pointer to a sensor's event mapping information.
 *
 * Discovers a sensor's events and records the static mapping
 * information needed to translate BladeCenter event log messages
 * into HPI event types.
 * 
 * Mapping information is stored in the hash table - event2hpi_hash.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL; sid <= 0.
 **/
SaErrorT snmp_bc_discover_sensor_events(struct oh_handler_state *handle,
					SaHpiEntityPathT *ep,
					SaHpiSensorNumT sid,
					const struct snmp_bc_sensor *sinfo)
{

	int i;
	int max = SNMP_BC_MAX_SENSOR_EVENT_ARRAY_SIZE;
	char *normalized_str;
	char *hash_existing_key, *hash_value;
	EventMapInfoT *eventmap_info;
	SaHpiResourceIdT rid;
	struct snmp_bc_hnd *custom_handle;

	if (!handle || !ep || !sinfo || sid <= 0) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	max = SNMP_BC_MAX_SENSOR_EVENT_ARRAY_SIZE;

	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle || !custom_handle->event2hpi_hash_ptr) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	rid = oh_uid_lookup(ep);
	if (rid == 0) {
		err("No RID.");
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}
		
	for (i=0; sinfo->sensor_info.event_array[i].event != NULL && i < max; i++) {
		/* Normalized and convert event string */
		normalized_str = oh_derive_string(ep, 0, 16, sinfo->sensor_info.event_array[i].event);
		if (normalized_str == NULL) {
			err("Cannot derive %s.", sinfo->sensor_info.event_array[i].event);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
		  
		/*  Add to hash; Set HPI values */
		if (!g_hash_table_lookup_extended(custom_handle->event2hpi_hash_ptr,
						  normalized_str,
						  (gpointer)&hash_existing_key, 
						  (gpointer)&hash_value)) {

			eventmap_info = g_malloc0(sizeof(EventMapInfoT));
			if (!eventmap_info) {
				err("Out of memory.");
				g_free(normalized_str);
				return(SA_ERR_HPI_OUT_OF_MEMORY);
			}

			/* Set default values */
			eventmap_info->hpievent.Source = rid;
			eventmap_info->hpievent.EventType = SAHPI_ET_SENSOR;
			eventmap_info->hpievent.EventDataUnion.SensorEvent.SensorNum = sid;
			eventmap_info->hpievent.EventDataUnion.SensorEvent.SensorType = sinfo->sensor.Type;
			eventmap_info->hpievent.EventDataUnion.SensorEvent.EventCategory = sinfo->sensor.Category;
			eventmap_info->hpievent.EventDataUnion.SensorEvent.Assertion = 
				sinfo->sensor_info.event_array[i].event_assertion;
			eventmap_info->hpievent.EventDataUnion.SensorEvent.EventState =
			eventmap_info->hpievent.EventDataUnion.SensorEvent.CurrentState =
				sinfo->sensor_info.event_array[i].event_state;

			eventmap_info->sensor_recovery_state = sinfo->sensor_info.event_array[i].recovery_state;
			eventmap_info->event_res_failure = sinfo->sensor_info.event_array[i].event_res_failure;
			eventmap_info->event_res_failure_unexpected = sinfo->sensor_info.event_array[i].event_res_failure_unexpected;
			
			/* Setup static trigger info for threshold sensors - some may be event-only */
			if (sinfo->sensor.Category == SAHPI_EC_THRESHOLD) {
				eventmap_info->hpievent.EventDataUnion.SensorEvent.TriggerReading.IsSupported =
				eventmap_info->hpievent.EventDataUnion.SensorEvent.TriggerThreshold.IsSupported =
					SAHPI_TRUE;

				eventmap_info->hpievent.EventDataUnion.SensorEvent.TriggerReading.Type =
				eventmap_info->hpievent.EventDataUnion.SensorEvent.TriggerThreshold.Type =
					sinfo->sensor.DataFormat.ReadingType;
			}
			
			dbg("Discovered sensor event=%s.", normalized_str);

			g_hash_table_insert(custom_handle->event2hpi_hash_ptr, normalized_str, eventmap_info);
			/* normalized_str space is recovered when hash is freed */
		}
		else {
			/* Event already exists (e.g. same event for multiple blades) */
			dbg("Event already exists=%s.", normalized_str);
			g_free(normalized_str);
		}
	}

	return(SA_OK);
}

/**
 * snmp_bc_log2event: 
 * @handle: Pointer to handler's data.
 * @logstr: BladeCenter event log string to be mapped to an HPI event.
 * @event: Location to store mapped HPI event.
 * @isdst: Is Daylight Savings Time on or off.
 *
 * Maps BladeCenter event log messages to HPI events.
 * 
 * @isdst ("is DayLight Savings Time") parameter is a performance hack.
 * Design assumes the event's timestamp is the time local to the platform itself.
 * So instead of forcing platform accesses for each log entry to determine if
 * DST is in effect, the isdst parameter allows the caller to query the
 * hardware DST info once then make multiple translation calls.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_log2event(struct oh_handler_state *handle,
			   gchar *logstr,
			   SaHpiEventT *event,
			   int isdst,
			   LogSource2ResourceT *ret_logsrc2res)
{
	sel_entry           log_entry;
	gchar               *recovery_str, *login_str, *post_str;
	gchar               root_str[SNMP_BC_MAX_SEL_ENTRY_LENGTH];
	gchar               search_str[SNMP_BC_MAX_SEL_ENTRY_LENGTH];
	EventMapInfoT       *eventmap_info;
	LogSource2ResourceT logsrc2res;
	SaErrorT            err;
	SaHpiBoolT          is_recovery_event, is_threshold_event;
	SaHpiEventT         working;
	SaHpiResourceIdT    event_rid;
	SaHpiSeverityT      event_severity;
	SaHpiTextBufferT    thresh_read_value, thresh_trigger_value;
	SaHpiTimeT          event_time;
	ErrLog2EventInfoT   *strhash_data;
        struct snmp_bc_hnd *custom_handle;
	int dupovrovr;
	struct oh_event *e;
	SaHpiHsStateT sav_cur_state;

	if (!handle || !logstr || !event || !ret_logsrc2res) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	memset(&working, 0, sizeof(SaHpiEventT));
	is_recovery_event = is_threshold_event = SAHPI_FALSE;

	dbg("Original event string = %s", logstr);

        /* Parse hardware log entry into its various components */
	err = snmp_bc_parse_sel_entry(handle, logstr, &log_entry);
	if (err) {
		err("Cannot parse log entry=%s. Error=%s.", logstr, oh_lookup_error(err));
		return(err);
	}

	/**********************************************************************
	 * For some types of BladeCenter events (e.g. thresholds), dynamic
	 * data is appended to a root event log string. Need to find this root 
         * string, since its what mapped in the event hash table.
         **********************************************************************/

	/* Set default search string */
	strncpy(search_str, log_entry.text, SNMP_BC_MAX_SEL_ENTRY_LENGTH);

	/* Discover "recovery" event strings */
	recovery_str = strstr(search_str, EVT_RECOVERY);
	if (recovery_str && (recovery_str == search_str)) {
		is_recovery_event = SAHPI_TRUE;
		memset(search_str, 0, SNMP_BC_MAX_SEL_ENTRY_LENGTH);
		strncpy(search_str, (log_entry.text + strlen(EVT_RECOVERY)),
			SNMP_BC_MAX_SEL_ENTRY_LENGTH - strlen(EVT_RECOVERY));
	}

	/* Adjust "login" event strings - strip username */
	login_str = strstr(log_entry.text, LOG_LOGIN_STRING);
	if (login_str) {
		gchar *id_str = strstr(log_entry.text, LOG_LOGIN_CHAR);
		if (id_str != NULL) {
			memset(search_str, 0, SNMP_BC_MAX_SEL_ENTRY_LENGTH);
			strncpy(search_str, log_entry.text, (id_str - log_entry.text));
			search_str[(id_str - log_entry.text)] = '\0';
		}
	}

	/* Adjust "POST" event strings - strip post results */
	post_str = strstr(log_entry.text, LOG_POST_STRING);
	if (post_str) {
		memset(search_str, 0, SNMP_BC_MAX_SEL_ENTRY_LENGTH);
		strncpy(search_str, log_entry.text, (post_str - log_entry.text));
		search_str[(post_str - log_entry.text - 1)] = '\0';
	}

        /* Replace internal double blanks with a single blank */
	{
		gchar *double_blanks;
		double_blanks = strstr(log_entry.text, "  ");
		if (double_blanks) {
			gchar *tmp_str;
			int len;
			tmp_str = log_entry.text;
			memset(search_str, 0, SNMP_BC_MAX_SEL_ENTRY_LENGTH);
			do {
				strncat(search_str, tmp_str, (double_blanks - tmp_str));
				tmp_str = double_blanks + 1;
				len = strlen(tmp_str);
				double_blanks = strstr(tmp_str, "  ");
			} while (double_blanks);
			strncat(search_str, tmp_str, len);
		}
	}

	/* Adjust "threshold" event strings */
	if (strstr(log_entry.text, LOG_THRESHOLD_VALUE_STRING) ||
	    strstr(log_entry.text, LOG_THRESHOLD_STRING)) {
		is_threshold_event = SAHPI_TRUE;
		oh_init_textbuffer(&thresh_read_value);
		oh_init_textbuffer(&thresh_trigger_value);
		err = snmp_bc_parse_threshold_str(search_str, root_str,
						  &thresh_read_value, &thresh_trigger_value);
		if (err) {
			err("Cannot parse threshold string=%s.", search_str);
		}
		else {
			memset(search_str, 0, SNMP_BC_MAX_SEL_ENTRY_LENGTH);
			strncpy(search_str, root_str, SNMP_BC_MAX_SEL_ENTRY_LENGTH);
		}
	}

	/* Strip any leading/trailing blanks */
	{
		gchar *tmp_str;
		tmp_str = g_strstrip(g_strdup(search_str));
		strncpy(search_str, tmp_str, SNMP_BC_MAX_SEL_ENTRY_LENGTH);
		g_free(tmp_str);
		if ((search_str == NULL || search_str[0] == '\0')) {
			err("Search string is NULL for log string=%s", log_entry.text);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
	}

	/* Strip any trailing period */
	if (search_str[strlen(search_str) - 1] == '.')
		search_str[strlen(search_str) - 1] = '\0';

	dbg("Event search string=%s", search_str);

	/* Set dynamic event fields with default values from the log string.
	   These may be overwritten in the code below */
	event_severity = log_entry.sev; 
	event_time = (SaHpiTimeT)mktime(&log_entry.time) * 1000000000;

	/* Find default RID from Error Log's "Source" field - need if NOT_ALERTABLE OEM event */
	err = snmp_bc_logsrc2rid(handle, log_entry.source, &logsrc2res, 0);
	if (err) {
		err("Cannot translate %s to RID. Error=%s", log_entry.source, oh_lookup_error(err));
		return(err);
	}
	event_rid = logsrc2res.rid;

	/***********************************************************
	 * See if adjusted root string is in errlog2event_hash table
         ***********************************************************/
	strhash_data = (ErrLog2EventInfoT *)g_hash_table_lookup(errlog2event_hash, search_str);
	if (!strhash_data) {
		if (snmp_bc_map2oem(&working, &log_entry, EVENT_NOT_ALERTABLE)) {
			err("Cannot map to OEM Event %s.", log_entry.text);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
		goto DONE;
	}
	/* See if need to override default RID; These are hardcoded exceptions caused by the
           fact that we have to handle duplicates event strings for resources that the
           BladeCenter's event log Source field doesn't define. These options must not be
           used with the OVR_RID option. Note: OVR_EXP messages always assumed to
           have a SOURCE field = BLADE_0x */
	if (strhash_data->event_ovr & OVR_EXP ||
	    strhash_data->event_ovr & OVR_VMM ||
	    strhash_data->event_ovr & OVR_MM1 ||
	    strhash_data->event_ovr & OVR_MM2 ||
	    strhash_data->event_ovr & OVR_MM_PRIME ||
	    strhash_data->event_ovr & OVR_MM_STBY) {

		err = snmp_bc_logsrc2rid(handle, log_entry.source, &logsrc2res,
					 strhash_data->event_ovr);
		if (err) {
			err("Cannot translate %s to RID. Error=%s.",
			    log_entry.source, oh_lookup_error(err));
			return(err);
		}
		
		event_rid = logsrc2res.rid;
	}

	/* Handle duplicate strings that have different event numbers */
	dupovrovr = 0;
	if (strhash_data->event_dup) {
		strhash_data = snmp_bc_findevent4dupstr(search_str, strhash_data, &logsrc2res);
		if (strhash_data == NULL) {
			err("Cannot find valid event for duplicate string=%s and RID=%d.", 
			    search_str, logsrc2res.rid);
			
			if (snmp_bc_map2oem(&working, &log_entry,  EVENT_NOT_ALERTABLE)) {
				err("Cannot map to OEM Event %s.", log_entry.text);
				return(SA_ERR_HPI_INTERNAL_ERROR);
			}
			goto DONE;
		}
		
		if (strhash_data->event_ovr & OVR_RID) {
			err("Cannot have RID override on duplicate strin;g=%s.", search_str);
			dupovrovr = 1;
		}
	}
	
	/* If OVR_SEV, use BCT-level severity calculated in off-line scripts */
	if (strhash_data->event_ovr & OVR_SEV) {
		event_severity = strhash_data->event_sev;
	}
	
	/**************************************************
	 * Find event string's mapped HPI event information
         **************************************************/
	eventmap_info = (EventMapInfoT *)g_hash_table_lookup(custom_handle->event2hpi_hash_ptr, 
							     strhash_data->event);
	if (!eventmap_info) {
		
		if (snmp_bc_map2oem(&working, &log_entry, EVENT_NOT_MAPPED)) {
			err("Cannot map to OEM Event %s.", log_entry.text);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
		goto DONE;
	}

	/* Set static event data defined during resource discovery */	
	working = eventmap_info->hpievent;
	logsrc2res.ep = eventmap_info->ep;
	logsrc2res.rid = eventmap_info->hpievent.Source;

	/* Handle OVR_RID - only for non-duplicate event strings */
	if ((strhash_data->event_ovr & OVR_RID) && !dupovrovr) {
		event_rid = eventmap_info->hpievent.Source;
	}
	
	/* Set RID in structure - used in later calls */
	working.Source = event_rid;

	/* Handle sensor events */
	if (working.EventType == SAHPI_ET_SENSOR) {
		if (is_recovery_event == SAHPI_TRUE) {
			working.EventDataUnion.SensorEvent.Assertion = SAHPI_FALSE;
		}
		
		/* Determine severity of event */
		err = snmp_bc_set_event_severity(handle, eventmap_info, &working, &event_severity);
		
		/* Set optional event current and previous states, if possible */
		err = snmp_bc_set_cur_prev_event_states(handle, eventmap_info, 
							&working, is_recovery_event);
		
		/* Set optional event threshold values */
		if (is_threshold_event == SAHPI_TRUE) {
			if (oh_encode_sensorreading(&thresh_read_value,
						    working.EventDataUnion.SensorEvent.TriggerReading.Type,
						    &working.EventDataUnion.SensorEvent.TriggerReading)) {
				err("Cannot convert trigger reading=%s; text=%s.",
				    thresh_read_value.Data, log_entry.text);
				return(SA_ERR_HPI_INTERNAL_ERROR);
			}
			working.EventDataUnion.SensorEvent.OptionalDataPresent =
				working.EventDataUnion.SensorEvent.OptionalDataPresent |
				SAHPI_SOD_TRIGGER_READING;

			if (oh_encode_sensorreading(&thresh_trigger_value,
						    working.EventDataUnion.SensorEvent.TriggerThreshold.Type,
						    &working.EventDataUnion.SensorEvent.TriggerThreshold)) {
				err("Cannot convert trigger threshold=%s; text=%s.",
				    thresh_trigger_value.Data, log_entry.text);
				return(SA_ERR_HPI_INTERNAL_ERROR);
			}
			working.EventDataUnion.SensorEvent.OptionalDataPresent =
				working.EventDataUnion.SensorEvent.OptionalDataPresent |
				SAHPI_SOD_TRIGGER_THRESHOLD;
		}
	}
	
	/* Handle hot swap events */
	else if (working.EventType == SAHPI_ET_HOTSWAP) {
		SaHpiHsStateT  hs_event_state, hs_event_auto_state;
		SaHpiRptEntryT *rpt;	
		struct ResourceInfo *resinfo2;

		rpt = oh_get_resource_by_id(handle->rptcache, event_rid);
		resinfo2 = (struct ResourceInfo *)oh_get_resource_data(handle->rptcache, event_rid);

		if ( (custom_handle->isFirstDiscovery == SAHPI_TRUE)||!rpt || !resinfo2) {
			if (is_recovery_event != SAHPI_TRUE) {
				snmp_bc_set_event_severity(handle, eventmap_info, &working, &event_severity);
			}
			snmp_bc_set_cur_prev_event_states(handle, eventmap_info,
							  &working, is_recovery_event);
							  
			if (custom_handle->isFirstDiscovery == SAHPI_FALSE) {
				/* Call rediscover() here to discover newly installed resource */
				err = snmp_bc_rediscover(handle, &working, &logsrc2res);
			}						  
			
					
			goto RESUME_TO_EXIT;

		}

		/* Find hot swap state and any defined hot swap events to generate */
		if (is_recovery_event) {
			hs_event_state = eventmap_info->hs_recovery_state;
			hs_event_auto_state = eventmap_info->hs_recovery_auto_state;
		}
		else {
			hs_event_state = eventmap_info->hpievent.EventDataUnion.HotSwapEvent.HotSwapState;
			hs_event_auto_state = eventmap_info->hs_event_auto_state;
		}

		/********************************************************************
                 * Check for reannouncement of current state
                 *  - don't auto-generate any events
		 *  - reannoucement has same current/previous state as original event
  		 ********************************************************************/
		if (hs_event_state == resinfo2->cur_state) {
			working.EventDataUnion.HotSwapEvent.HotSwapState = resinfo2->cur_state;
			working.EventDataUnion.HotSwapEvent.PreviousHotSwapState = resinfo2->cur_state;
			event_severity = SAHPI_INFORMATIONAL;
		}
		else {
			/* Generate any automatic hot swap state event */
			if (hs_event_auto_state) {
				if (rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) {
					SaErrorT err;
					SaHpiEventT autoevent;
					
					/* Build event */
					memcpy((void *)&autoevent, (void *)&working, sizeof(SaHpiEventT));
					autoevent.Timestamp = event_time; 
					autoevent.Severity = SAHPI_INFORMATIONAL;
					
					autoevent.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
						resinfo2->prev_state = resinfo2->cur_state;
					autoevent.EventDataUnion.HotSwapEvent.HotSwapState =
						resinfo2->cur_state = hs_event_auto_state;
										
					if (custom_handle->isFirstDiscovery == SAHPI_FALSE) {
						err = snmp_bc_add_to_eventq(handle, &autoevent, SAHPI_TRUE);
						if (err) {
							err("Cannot add entry to eventq. Error=%s.", oh_lookup_error(err));
							return(err);
						}
					}
				}
				else {
					err("Invalid Hot Swap State for %s", rpt->ResourceTag.Data);
					return(SA_ERR_HPI_INTERNAL_ERROR);
				}
			}

			/* Normal hot swap event transitions */

			sav_cur_state = resinfo2->cur_state;
			if (is_recovery_event)
				working.EventDataUnion.HotSwapEvent.HotSwapState =
				resinfo2->cur_state = 
				eventmap_info->hs_recovery_state;
			else
				working.EventDataUnion.HotSwapEvent.HotSwapState =
				resinfo2->cur_state = 
				eventmap_info->hpievent.EventDataUnion.HotSwapEvent.HotSwapState;
			
			if ( (rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) && 
							(resinfo2->cur_state == SAHPI_HS_STATE_NOT_PRESENT)){
				working.EventDataUnion.HotSwapEvent.PreviousHotSwapState = sav_cur_state;
				resinfo2->prev_state = SAHPI_HS_STATE_EXTRACTION_PENDING;			
			} else {
				working.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
							resinfo2->prev_state = sav_cur_state;
						
			}
			
			  
			/* Determine severity */
			if (resinfo2->cur_state == SAHPI_HS_STATE_NOT_PRESENT &&
			    resinfo2->prev_state != SAHPI_HS_STATE_INACTIVE) {
				event_severity = rpt->ResourceSeverity;
			}
			else {
				event_severity = SAHPI_INFORMATIONAL;
			}
			
			/* Call rediscover() here to do cleanup for hotswap-remove event */
			/* cleanup = removing rpt and rdrs from plugin handle->rptcache  */
			/* rpt and rdrs will be remove in snmp_bc_add_to_eventq() becasue*/
			/* the new oh_event requires rpt present, else infrastructure    */
			/* drops the event.  We have to wait to the very last min to     */
			/* remove rpt from rptcache.                                     */
  			/* err = snmp_bc_rediscover(handle, &working, &logsrc2res);      */	
		}
	}
	else {
		err("Platform doesn't support events of type=%s.", oh_lookup_eventtype(working.EventType));
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}
	
	/************************************************** 
	 * Check to see if need to mark resource as failed.
	 **************************************************/

RESUME_TO_EXIT:
	if (eventmap_info->event_res_failure) {
		SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, event->Source);
		if (rpt) {
			/* Only notify if change in status */
			if (rpt->ResourceFailed == SAHPI_FALSE) {
				rpt->ResourceFailed = SAHPI_TRUE;
				/* Add changed resource to event queue */
				e = snmp_bc_alloc_oh_event();
				if (e == NULL) {
					err("Out of memory.");
					return(SA_ERR_HPI_OUT_OF_MEMORY);
				}
				
				e->resource = *rpt;
				e->event.Severity = e->resource.ResourceSeverity;
				e->event.Source =   e->resource.ResourceId;
				e->event.EventType = SAHPI_ET_RESOURCE;
				e->event.EventDataUnion.ResourceEvent.ResourceEventType 
									= SAHPI_RESE_RESOURCE_FAILURE;
				if (oh_gettimeofday(&e->event.Timestamp) != SA_OK)
		                		    e->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
				custom_handle->eventq = g_slist_append(custom_handle->eventq, e);
			}
		}
	}

 DONE:
	working.Source = event_rid;
	working.Timestamp = event_time;
	working.Severity = event_severity;
	memcpy((void *)event, (void *)&working, sizeof(SaHpiEventT));
	memcpy(ret_logsrc2res, &logsrc2res, sizeof(LogSource2ResourceT));	

	return(SA_OK);
}

/**
 * snmp_bc_findevent4dupstr:
 * @search_str: Error Log string.
 * @strhash_data: Pointer to string to event ID mapping information.
 * @resinfo: Pointer to resource mapping information
 *
 * Returns a pointer to the error log event number to HPI event mapping 
 * information. A NULL is returned if the information cannot be found.
 * 
 * There are several identical Error Log messages strings that are shared by
 * multiple resources. The scripts that populate errlog2event_hash
 * create unique entries for these duplicate strings by tacking on
 * an unique string (HPIDUP_duplicate_number) to the error log message.
 * This is then stored in errlog2event_hash. So there is a unique mapping
 * for each resource with a duplicate string.
 * 
 * This routine goes finds the unique mapping for all the duplicate strings.
 * Then searches through all the resource's events and all the resource's
 * sensor events to find a match. It does this for each duplicate string
 * mapping until it finds a match or runs out of strings to test (in which
 * case, it returns NULL).
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Parameter pointer(s) are NULL.
 **/
static ErrLog2EventInfoT *snmp_bc_findevent4dupstr(gchar *search_str,
						   ErrLog2EventInfoT *strhash_data,
						   LogSource2ResourceT *logsrc2res)
{	
	gchar dupstr[SNMP_BC_MAX_SEL_ENTRY_LENGTH];
	ErrLog2EventInfoT *dupstr_hash_data;
	short strnum;

	if (!search_str || !strhash_data || !logsrc2res) {
		err("Invalid parameter.");
		return(NULL);
	}

	strncpy(dupstr, search_str, SNMP_BC_MAX_SEL_ENTRY_LENGTH);
	dupstr_hash_data = strhash_data;
	strnum = strhash_data->event_dup + 1; /* Original string plus dups */
       
	while (strnum && (dupstr_hash_data != NULL)) {
		int i,j;
		gchar *normalized_event; 

		/* Search entire sensor array for the duplicate string's event */
		for (i=0; (logsrc2res->sensor_array_ptr + i)->sensor.Num != 0; i++) {
			for (j=0; (logsrc2res->sensor_array_ptr + i)->sensor_info.event_array[j].event != NULL; j++) {
				normalized_event = oh_derive_string(&(logsrc2res->ep), 0, 16,
						   (logsrc2res->sensor_array_ptr + i)->sensor_info.event_array[j].event);
				
				if (!g_ascii_strcasecmp(dupstr_hash_data->event, normalized_event)) {
					g_free(normalized_event);
					return(dupstr_hash_data);
				}
				g_free(normalized_event);
			}
		}
		
		/* Search resource array for the duplicate string's event */
		for (i=0; snmp_bc_rpt_array[logsrc2res->rpt].res_info.event_array[i].event != NULL; i++) {
			normalized_event = oh_derive_string(&(logsrc2res->ep), 0, 16,
					   snmp_bc_rpt_array[logsrc2res->rpt].res_info.event_array[i].event);
			if (!g_ascii_strcasecmp(dupstr_hash_data->event, normalized_event)) {
				g_free(normalized_event);
				return(dupstr_hash_data);
			}
			g_free(normalized_event);
		}

		/* Find next duplicate string */
		strnum--;
		if (strnum) {
			gchar strnum_str[OH_MAX_LOCATION_DIGITS];
			gchar *tmpstr;

			snprintf(strnum_str, OH_MAX_LOCATION_DIGITS, "%d", strnum);
			tmpstr = g_strconcat(search_str, HPIDUP_STRING, strnum_str, NULL);
			strncpy(dupstr, tmpstr, SNMP_BC_MAX_SEL_ENTRY_LENGTH);
			g_free(tmpstr);

			dupstr_hash_data = (ErrLog2EventInfoT *)g_hash_table_lookup(errlog2event_hash, dupstr);
			if (dupstr_hash_data == NULL) {
				err("Cannot find duplicate string=%s.", dupstr);
			}
		}
	}

	return(NULL);
}

/**
 * snmp_bc_parse_threshold_str:
 * @str: Input Error Log threshold string.
 * @root_str: Location to store threshold's root string.
 * @read_value_str: Location to store threshold's read value string.
 * @trigger_value_str: Location to store threshold's trigger value string.
 *
 * Parses a Error Log threshold string into its root string, read, 
 * and trigger value strings.
 * 
 * Format is a root string (in the errlog2event_hash table) followed by a
 * read threshold value string, followed by a trigger threshold value string.
 * Unfortunately cannot convert directly to sensor values yet because 
 * don't yet know if event is in the event2hpi_hash table or if it is, 
 * what the sensor's threshold data type is.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Parameter pointer(s) are NULL.
 **/
static SaErrorT snmp_bc_parse_threshold_str(gchar *str,
					    gchar *root_str,
					    SaHpiTextBufferT *read_value_str,
					    SaHpiTextBufferT *trigger_value_str)
{
	gchar  **event_substrs;
	gchar  **thresh_substrs;
	SaErrorT err;

	if (!str || !root_str || !read_value_str || !trigger_value_str) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	event_substrs = NULL;
	thresh_substrs = NULL;
	err = SA_OK;
	
	/* Handle BladeCenter's two basic threshold event formats */
	if (strstr(str, LOG_READ_VALUE_STRING)) {
		event_substrs = g_strsplit(str, LOG_READ_VALUE_STRING, -1);
		thresh_substrs = g_strsplit(event_substrs[1], LOG_THRESHOLD_VALUE_STRING, -1);
	}
	else {
		event_substrs = g_strsplit(str, LOG_READ_STRING, -1);
		thresh_substrs = g_strsplit(event_substrs[1], LOG_THRESHOLD_STRING, -1);
	}
   
	if (thresh_substrs == NULL ||
	    (thresh_substrs[0] == NULL || thresh_substrs[0][0] == '\0') ||
	    (thresh_substrs[1] == NULL || thresh_substrs[1][0] == '\0') ||
	    (thresh_substrs[2] != NULL)) {
		err("Cannot split threshold string=%s.", str);
		err = SA_ERR_HPI_INTERNAL_ERROR;
		goto CLEANUP;
	}
	
	/* Strip any leading/trailing blanks */
	event_substrs[0]= g_strstrip(event_substrs[0]);
	thresh_substrs[0] = g_strstrip(thresh_substrs[0]);
	thresh_substrs[1] = g_strstrip(thresh_substrs[1]);
	if ((event_substrs[0] == NULL || event_substrs[0] == '\0') ||
	    (thresh_substrs[0] == NULL || thresh_substrs[0][0] == '\0') ||
	    (thresh_substrs[1] == NULL || thresh_substrs[1][0] == '\0')) {
		err("NULL base string or threshold values=%s.", str);
		err = SA_ERR_HPI_INTERNAL_ERROR;
		goto CLEANUP;
	}

	/* Change any leading period to a blank.
           This put here because MM code added a period after the 
           Threadhold Value string and before the threshold value 
           (e.g. "Threshold value. 23.0") */
	if (thresh_substrs[0][0] == '.') thresh_substrs[0][0] = ' ';
	if (thresh_substrs[1][0] == '.') thresh_substrs[1][0] = ' ';

	/* Strip any leading/trailing blanks - in case of leading period */
	thresh_substrs[0] = g_strstrip(thresh_substrs[0]);
	thresh_substrs[1] = g_strstrip(thresh_substrs[1]);
	if ((event_substrs[0] == NULL || event_substrs[0] == '\0') ||
	    (thresh_substrs[0] == NULL || thresh_substrs[0][0] == '\0') ||
	    (thresh_substrs[1] == NULL || thresh_substrs[1][0] == '\0')) {
		err("NULL base string or threshold values=%s.", str);
		err = SA_ERR_HPI_INTERNAL_ERROR;
		goto CLEANUP;
	}

	/* Strip any ending periods, commas, colons, or semicolons */
	if ((thresh_substrs[0][strlen(thresh_substrs[0]) - 1] == '.') ||
	    (thresh_substrs[0][strlen(thresh_substrs[0]) - 1] == ',') ||
	    (thresh_substrs[0][strlen(thresh_substrs[0]) - 1] == ':') ||
	    (thresh_substrs[0][strlen(thresh_substrs[0]) - 1] == ';'))
		thresh_substrs[0][strlen(thresh_substrs[0]) - 1] = '\0';
	if ((thresh_substrs[1][strlen(thresh_substrs[1]) - 1]  == '.') ||
	    (thresh_substrs[1][strlen(thresh_substrs[1]) - 1]  == ',') ||
	    (thresh_substrs[1][strlen(thresh_substrs[1]) - 1]  == ':') ||
	    (thresh_substrs[1][strlen(thresh_substrs[1]) - 1]  == ';'))
		thresh_substrs[1][strlen(thresh_substrs[1]) - 1] = '\0';

	/* Check for valid length */
	if ((strlen(thresh_substrs[0]) > SAHPI_MAX_TEXT_BUFFER_LENGTH) ||
	    (strlen(thresh_substrs[1]) > SAHPI_MAX_TEXT_BUFFER_LENGTH)) {
		err("Threshold value string(s) exceed max size for %s.", str);
		err = SA_ERR_HPI_INTERNAL_ERROR;
		goto CLEANUP;
	}

	dbg("Threshold strings: %s and %s", thresh_substrs[0], thresh_substrs[1]);
	
	strcpy(root_str, event_substrs[0]);
	oh_append_textbuffer(read_value_str, thresh_substrs[0]);
	oh_append_textbuffer(trigger_value_str, thresh_substrs[1]);

	if (root_str == NULL) {
		err("Cannot parse threshold string=%s.", str);
		err = SA_ERR_HPI_INTERNAL_ERROR;
	}

 CLEANUP:
	g_strfreev(event_substrs);
	g_strfreev(thresh_substrs);

	return(err);
}

/**
 * snmp_bc_set_event_severity:
 * @handle: Handler data pointer.
 * @eventmap_info: Event state and recovery information pointer.
 * @event: Event pointer.
 * @event_severity: Location to store severity.
 *
 * Overwrites normal severity, if sensor is of type SAHPI_EC_THRESHOLD or
 * SAHPI_EC_SEVERITY. If event is not one of these types, this routine 
 * checks to see if it's an unexpected resource failure event.
 * If it is, the resource's severity (user writable) is used.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Parameter pointer(s) are NULL.
 **/
static SaErrorT snmp_bc_set_event_severity(struct oh_handler_state *handle,
					   EventMapInfoT *eventmap_info,
					   SaHpiEventT *event,
					   SaHpiSeverityT *event_severity)
{
	int sensor_severity_override;
	sensor_severity_override = 0;


	if (!handle || !eventmap_info || !event || !event_severity) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
						
	if (event->EventType == SAHPI_ET_SENSOR) {

		/* Force HPI Threshold and Severity category severities */
		if (event->EventDataUnion.SensorEvent.EventCategory == SAHPI_EC_THRESHOLD) {
			sensor_severity_override = 1;
			if (event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_LOWER_CRIT ||
			    event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_UPPER_CRIT) {
				*event_severity = SAHPI_CRITICAL;
			}
			else {
				if (event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_LOWER_MAJOR ||
				    event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_UPPER_MAJOR) {
					*event_severity = SAHPI_MAJOR;
				}
				else {
					if (event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_LOWER_MINOR ||
					    event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_UPPER_MINOR) {
						*event_severity = SAHPI_MINOR;
					}
				}
			}
		}
		else {
			if (event->EventDataUnion.SensorEvent.EventCategory == SAHPI_EC_SEVERITY) {
				sensor_severity_override = 1;
				if (event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_OK)
					*event_severity = SAHPI_OK;
				if (event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_MINOR_FROM_OK)
					*event_severity = SAHPI_MINOR;
				if (event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_MAJOR_FROM_LESS)
					*event_severity = SAHPI_MAJOR;
				if (event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_CRITICAL_FROM_LESS)
					*event_severity = SAHPI_CRITICAL;
				if (event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_MINOR_FROM_MORE)
					*event_severity = SAHPI_MINOR;
				if (event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_MAJOR_FROM_CRITICAL)
					*event_severity = SAHPI_MAJOR;
				if (event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_CRITICAL)
					*event_severity = SAHPI_CRITICAL;
				if (event->EventDataUnion.SensorEvent.EventState & SAHPI_ES_INFORMATIONAL)
					*event_severity = SAHPI_INFORMATIONAL;
			}
		}
	}

      /* Use resource's severity, if unexpected failure */
	if (!sensor_severity_override && eventmap_info->event_res_failure_unexpected) {

		SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, event->Source);
		if (!rpt) return(SA_ERR_HPI_INVALID_RESOURCE);

		*event_severity	= rpt->ResourceSeverity;
	}

	return(SA_OK);
}

/**
 * snmp_bc_set_cur_prev_event_states:
 * @handle: Handler data pointer.
 * @eventmap_info: Event state and recovery information pointer.
 * @event: Location to store current/previous event infomation.
 * @recovery_event: Is the event a recovery event or not.
 *
 * Attempts to set the optional current and previous state information
 * in an event.
 *
 * NOTES:
 * A sensor's previous state depends on if it is readable or not. If 
 * the sensor is not readable, then the previous state is just the state
 * of the sensor's last processed event. If the sensor is readable,
 * the previous state is set to the sensor's current state when the
 * last event was processed. That is when an event is processed,
 * the sensor is read and the info stored to be placed in the previous
 * state field for the next time an event for that sensor is processed.
 * 
 * For non-readable sensors, current state is simply the processed 
 * event's state.
 *
 * This routine is optional for sensors; but NOT hot swap.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT snmp_bc_set_cur_prev_event_states(struct oh_handler_state *handle,
						  EventMapInfoT *eventmap_info,
						  SaHpiEventT *event,
						  int recovery_event)
{
	SaErrorT err;
	SaHpiRdrT *rdr;

	if (!handle || !eventmap_info || !event) {
		err("Invalid parameters.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if ( (((struct snmp_bc_hnd *)handle->data)->isFirstDiscovery == SAHPI_TRUE) && 
	     (event->EventType == SAHPI_ET_HOTSWAP) )
	{
		/* This is HPI time zero processing */
		/* Discovery routines set proper current state for each resource */
		/* Do not override state setting with (stale) data from Event Log processing */
		return(SA_OK);
	}
	
	switch (event->EventType) {
	case SAHPI_ET_SENSOR:
	{
		SaHpiSensorReadingT cur_reading;
		SaHpiEventStateT cur_state, prev_state;
		struct SensorInfo *sinfo;

		/* Initialize previous and current state event info */
		event->EventDataUnion.SensorEvent.PreviousState = SAHPI_ES_UNSPECIFIED;
		event->EventDataUnion.SensorEvent.CurrentState = SAHPI_ES_UNSPECIFIED;

		/* Set previous state to sensor's current state */
		rdr = oh_get_rdr_by_type(handle->rptcache, event->Source, SAHPI_SENSOR_RDR,
						    event->EventDataUnion.SensorEvent.SensorNum);
		if (rdr == NULL) {
			return(SA_ERR_HPI_NOT_PRESENT);
		}
		sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, event->Source, rdr->RecordId);
		if (sinfo == NULL) {
			err("No sensor data. Sensor=%s.", rdr->IdString.Data);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}

		/* Record previous state info */
		prev_state = sinfo->cur_state;

		/* Try to read sensor to get and record the current state */
		err = snmp_bc_get_sensor_reading((void *)handle,
						 event->Source,
						 event->EventDataUnion.SensorEvent.SensorNum,
						 &cur_reading,
						 &cur_state);
		
		/* If can't read sensor (error, sensor disabled, or event-only), 
		   use static event state definitions */
		if (err || cur_reading.IsSupported == SAHPI_FALSE) {
			if (recovery_event) {
				cur_state = sinfo->cur_state = eventmap_info->sensor_recovery_state;
			}
			else {
				cur_state = sinfo->cur_state = event->EventDataUnion.SensorEvent.EventState;
			}
		}
		else {
			sinfo->cur_state = cur_state;
		}

#if 0
		{       /* Debug section */
			SaHpiTextBufferT buffer;

			err("Event for Sensor=%s", rdr->IdString.Data);
			oh_decode_eventstate(prev_state, event->EventDataUnion.SensorEvent.EventCategory, &buffer);
			err("  Previous Event State: %s.", buffer.Data);
			oh_decode_eventstate(cur_state, event->EventDataUnion.SensorEvent.EventCategory, &buffer);
			err("  Current Event State: %s", buffer.Data);
		}
#endif
		
		/* Set previous and current state event info */
		event->EventDataUnion.SensorEvent.PreviousState = prev_state;
		event->EventDataUnion.SensorEvent.CurrentState = cur_state;
		event->EventDataUnion.SensorEvent.OptionalDataPresent =
			event->EventDataUnion.SensorEvent.OptionalDataPresent |
			SAHPI_SOD_PREVIOUS_STATE |
			SAHPI_SOD_CURRENT_STATE;
		break;
	}

	case SAHPI_ET_HOTSWAP:
	{
		struct ResourceInfo *resinfo;

		resinfo = (struct ResourceInfo *)oh_get_resource_data(handle->rptcache, event->Source);
		if (resinfo == NULL) {
			/* If there is no resource data in rptcache, ie. no rpt,   */
			/* this resource either did not exist at HPI time 0, or    */
			/* has previously been removed from system.                */
			/* It is safe to assume that it is being Hotswap-Installed */
			dbg("No resource data. RID=%x", event->Source);
			event->EventDataUnion.HotSwapEvent.PreviousHotSwapState	= SAHPI_HS_STATE_NOT_PRESENT;
			event->EventDataUnion.HotSwapEvent.HotSwapState =  SAHPI_HS_STATE_INACTIVE;			
			return(SA_OK);
		}

		event->EventDataUnion.HotSwapEvent.PreviousHotSwapState	= resinfo->cur_state;
		
		if (recovery_event) {
			event->EventDataUnion.HotSwapEvent.HotSwapState =
				resinfo->cur_state =
				eventmap_info->hs_recovery_state;
		}
		else {
			event->EventDataUnion.HotSwapEvent.HotSwapState =
				resinfo->cur_state =
				eventmap_info->hpievent.EventDataUnion.HotSwapEvent.HotSwapState;
		}
		break;
	}
	default:
		err("Unrecognized Event Type=%s.", oh_lookup_eventtype(event->EventType));
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}
	
	return(SA_OK);
}

/**
 * snmp_bc_map2oem:
 * @event: Pointer to handler's data.
 * @sel_entry: Error Log's "Source" field string.
 * @reason: Location to store HPI mapping data for resource.
 *
 * Any event not explicitly recognized is mapped into an HPI 
 * OEM event. This routine performs the mapping. 
 *
 * NOTE:
 * A reason code is passed, if in the future we record in some temp file, 
 * non-mapped events. Reason records why the event wasn't mapped.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT snmp_bc_map2oem(SaHpiEventT *event,
				sel_entry *sel_entry,
				OEMReasonCodeT reason)
{
	if (!event || !sel_entry) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	dbg("OEM Event Reason Code=%s\n", reason ? "NOT_ALERTABLE" : "NOT MAPPED");

	event->EventType = SAHPI_ET_OEM;
	event->EventDataUnion.OemEvent.MId = IBM_MANUFACTURING_ID;

	/* Language set to ENGLISH, default  */
	oh_init_textbuffer(&(event->EventDataUnion.OemEvent.OemEventData));
	strncpy((char *)(event->EventDataUnion.OemEvent.OemEventData.Data),
		sel_entry->text, SAHPI_MAX_TEXT_BUFFER_LENGTH - 1);
	event->EventDataUnion.OemEvent.OemEventData.Data[SAHPI_MAX_TEXT_BUFFER_LENGTH - 1] = '\0';
	event->EventDataUnion.OemEvent.OemEventData.DataLength = strlen(sel_entry->text);

	return(SA_OK);
}

/**
 * snmp_bc_logsrc2rid:
 * @handle: Pointer to handler's data.
 * @src: Error Log's "Source" field string.
 * @resinfo: Location to store HPI resource mapping information.
 * @ovr_flags: Override flags
 *
 * Translates platform error log's "Source" field into an HPI resource ID
 * and stores HPI mapping info needed by other routines in
 * @resinfo. Assume "Source" field text is in the following format:
 *
 *   "BLADE_0x" - map to blade x RID
 *   "SWITCH_x" - map to switch x RID
 *
 * All other "Source" field text strings are mapped to the 
 * Chassis's resource ID.
 *
 * @ovr_flags is used to indicate exception cases. The two case
 * supported are:
 *   - OVR_EXP      - indicates resource is an expansion card.
 *   - OVR_VMM      - indicates resource is Virtual MM
 *   - OVR_MM1      - indicates resource is the MM 1 card.
 *   - OVR_MM2      - indicates resource is the MM 2 card.
 *   - OVR_MM_PRIME - indicates resource is the physical MM of primary MM.
 *   - OVR_MM_STBY  - indicates resource is the physical MM of the standby MM.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT snmp_bc_logsrc2rid(struct oh_handler_state *handle,
				   gchar *src,
				   LogSource2ResourceT *logsrc2res,
				   unsigned long long ovr_flags)
{
	int rpt_index;
	guint src_loc;
	gchar **src_parts, *endptr, *root_tuple;
	SaErrorT err;
	SaHpiBoolT isblade, isexpansioncard, isswitch, ismm;
	SaHpiEntityPathT ep, ep_root;
	SaHpiEntityTypeT entity_type;
	struct snmp_bc_sensor *array_ptr;
	struct snmp_bc_hnd *custom_handle;

	if (!handle || !src || !logsrc2res) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	src_parts = NULL;
	endptr = NULL;
	
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Find top-level chassis entity path */
        oh_init_ep(&ep);
	oh_init_ep(&ep_root);
	root_tuple = (gchar *)g_hash_table_lookup(handle->config, "entity_root");
        oh_encode_entitypath(root_tuple, &ep_root);
        
        /* Assume chassis location/type unless another resource type is discovered */
	src_loc = ep_root.Entry[0].EntityLocation;
	entity_type = ep_root.Entry[0].EntityType;

	/* Break down "Source" text string to find source's RPT index and location */
	src_parts = g_strsplit(src, "_", -1);
	if (src_parts == NULL) {
		err("Cannot split Source text string.");
		g_strfreev(src_parts);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* See if resource is something other than the chassis */
	isblade = isexpansioncard = isswitch = ismm = SAHPI_FALSE;
	if (!g_ascii_strncasecmp(src_parts[0], "BLADE", sizeof("BLADE"))) { 
		/* All expansion card events are reported as blade events in the Error Log */
		if (ovr_flags & OVR_EXP) { isexpansioncard = SAHPI_TRUE; }
		else { isblade = SAHPI_TRUE; }
	}
	else {
		if (!g_ascii_strncasecmp(src_parts[0], "SWITCH", sizeof("SWITCH"))) { isswitch = SAHPI_TRUE; }
	}

	/* Find the location value from last part of log's source string */
	if (isexpansioncard == SAHPI_TRUE || isblade == SAHPI_TRUE || isswitch == SAHPI_TRUE) {
		if (src_parts[1]) src_loc = strtoul(src_parts[1], &endptr, 10);
		if (isexpansioncard == SAHPI_TRUE) {
			rpt_index = BC_RPT_ENTRY_BLADE_EXPANSION_CARD;
			array_ptr = &snmp_bc_bem_sensors[0];	
		}
		else {
			if (isblade == SAHPI_TRUE) { 
				rpt_index = BC_RPT_ENTRY_BLADE; 
				array_ptr = &snmp_bc_blade_sensors[0];
			}
			else { 
				rpt_index = BC_RPT_ENTRY_SWITCH_MODULE;
				array_ptr = &snmp_bc_switch_sensors[0];
			}
		}
		entity_type = snmp_bc_rpt_array[rpt_index].rpt.ResourceEntity.Entry[0].EntityType;
	}
	else {
		/* Check for OVR_MMx overrides, if cannot find explict resource from error
                   logs "Source" field */
		if (ovr_flags & OVR_VMM || 
		    ovr_flags & OVR_MM1 || ovr_flags & OVR_MM2 || 
		    ovr_flags & OVR_MM_PRIME || ovr_flags & OVR_MM_STBY) {
			if (ovr_flags & OVR_VMM) {
				rpt_index = BC_RPT_ENTRY_VIRTUAL_MGMNT_MODULE; 
				src_loc = 0;
				array_ptr = &snmp_bc_virtual_mgmnt_sensors[0];
			}
			else {
				ismm = SAHPI_TRUE;
				rpt_index = BC_RPT_ENTRY_MGMNT_MODULE;
				array_ptr = &snmp_bc_mgmnt_sensors[0];
				
				if (ovr_flags & OVR_MM1 || ovr_flags & OVR_MM2) {
					if (ovr_flags & OVR_MM1) { src_loc = 1; }
					else { src_loc = 2; }
				}
				else { 	/* Assign primary/standby events to physical MM */
					struct snmp_value value;
					int primary, standby;

					err = snmp_bc_snmp_get(custom_handle, SNMP_BC_MGMNT_ACTIVE, &value, SAHPI_TRUE);
					if (err) {
						err("Cannot get OID=%s.",  SNMP_BC_MGMNT_ACTIVE);
						return(SA_ERR_HPI_INTERNAL_ERROR);
					}

					primary = value.integer;
					if (value.integer == 1) { standby = 2; }
					else { standby = 1; }
					
					if (ovr_flags & OVR_MM_PRIME) { src_loc = primary; }
					else { src_loc = standby; } /* Standby override */
				}
			}
			entity_type = snmp_bc_rpt_array[rpt_index].rpt.ResourceEntity.Entry[0].EntityType;
		}
		else {
			rpt_index = BC_RPT_ENTRY_CHASSIS;
			array_ptr = &snmp_bc_chassis_sensors[0];
		}
	}
	g_strfreev(src_parts);

	/* Find rest of Entity Path and calculate the RID */
	err = oh_concat_ep(&ep, &snmp_bc_rpt_array[rpt_index].rpt.ResourceEntity);
	if (err) {
		err("Cannot concat Entity Path. Error=%s.", oh_lookup_error(err));
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}
	err = oh_concat_ep(&ep, &ep_root);
	if (err) {
		err("Cannot concat Entity Path. Error=%s.", oh_lookup_error(err));
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}
	/* FIXME:: Need to rewrite this section when discover multiple BEMs/blade */
	/* Also switches aren't currently supported with the Source field - can remove this logic */
	if ((isexpansioncard == SAHPI_TRUE)) {
		err = oh_set_ep_location(&ep, entity_type, 1);
	}
	else {
		err = oh_set_ep_location(&ep, entity_type, src_loc);
	}

	if (err) {
		err("Cannot set location. Type=%s; Location=%d; Error=%s.",
		    oh_lookup_entitytype(entity_type), src_loc, oh_lookup_error(err));
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	if ((isblade == SAHPI_TRUE) || (isexpansioncard == SAHPI_TRUE)) {
		err = oh_set_ep_location(&ep, SAHPI_ENT_PHYSICAL_SLOT, src_loc);	
	} else if (ismm == SAHPI_TRUE) {
		err = oh_set_ep_location(&ep, BLADECENTER_SYS_MGMNT_MODULE_SLOT, src_loc);	
	}
	
	/* Special case - if Expansion Card set location of parent blade as well */
	if (isexpansioncard == SAHPI_TRUE) {
		err = oh_set_ep_location(&ep, SAHPI_ENT_SBC_BLADE, src_loc);
		if (err) {
			err("Cannot set location. Type=%s; Location=%d; Error=%s.",
			    oh_lookup_entitytype(SAHPI_ENT_SBC_BLADE), src_loc, oh_lookup_error(err));  
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
	}

	/* Fill in RID and RPT table info about "Source" */
	logsrc2res->rpt = rpt_index;
	logsrc2res->sensor_array_ptr = array_ptr;
	logsrc2res->ep = ep;
	logsrc2res->rid = oh_uid_lookup(&ep);

	/**********************************************************************************
	 * Generate RID, if necessary.
         * 
	 * RID may be zero if resource hasn't ever been discovered since HPI initialization.
	 * In this case, generate a valid RID. Infra-structure always assigns same RID 
         * number to an unique entity path. User app needs to worry about if resource 
         * is actually in the chassis - just as they do for hot swapped resources.
         **********************************************************************************/
	if (logsrc2res->rid == 0) {
		logsrc2res->rid = oh_uid_from_entity_path(&ep);
		if (logsrc2res->rid == 0) {
			err("No RID.");
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
	}
	
	return(SA_OK);
}

 /**
 * snmp_bc_add_to_eventq
 * @handle: Pointer to handler's data.
 * @thisEvent: Location to store event.
 *
 * Add event to Infrastructure's event queue.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_add_to_eventq(struct oh_handler_state *handle, SaHpiEventT *thisEvent, SaHpiBoolT prepend)
{
	SaHpiEntryIdT rdrid;
        struct oh_event *e = NULL;
	SaHpiRptEntryT *thisRpt;
	SaHpiRdrT      *thisRdr;
	LogSource2ResourceT logsrc2res;
	SaErrorT err;
        struct snmp_bc_hnd *custom_handle = (struct snmp_bc_hnd *)handle->data;

        /* Insert entry to eventq for processing */
        e = snmp_bc_alloc_oh_event();
        if (!e) {
                err("Out of memory.");
                return(SA_ERR_HPI_OUT_OF_MEMORY);
        }
				
	thisRpt = oh_get_resource_by_id(handle->rptcache, thisEvent->Source);
        if (thisRpt) e->resource = *thisRpt;
        memcpy(&e->event, thisEvent, sizeof(SaHpiEventT));

	/* Setting RDR ID to event struct */	
	switch (thisEvent->EventType) {
	case SAHPI_ET_HOTSWAP:
		if (snmp_bc_isrediscover(thisEvent) == SNMP_BC_RESOURCE_INSTALLED) {
    			for (thisRdr = oh_get_rdr_by_id(handle->rptcache, thisEvent->Source, SAHPI_FIRST_ENTRY);
                     	     thisRdr != NULL;
                             thisRdr = oh_get_rdr_next(handle->rptcache, thisEvent->Source, thisRdr->RecordId)) 
			{
				e->rdrs = g_slist_append(e->rdrs, g_memdup(thisRdr, sizeof(SaHpiRdrT)));

                	}
		} else if (snmp_bc_isrediscover(thisEvent) == SNMP_BC_RESOURCE_REMOVED) {
			/* Call rediscovery to remove rpt and rdrs from rptcache */
			if (thisRpt) logsrc2res.ep = thisRpt->ResourceEntity;
			err = snmp_bc_rediscover(handle, thisEvent, &logsrc2res);
		}
		break;
	case SAHPI_ET_OEM:
	case SAHPI_ET_USER:
		/* There is no RDR associated to OEM event */
		e->rdrs = NULL;
		/* Set RDR Type to SAHPI_NO_RECORD, spec B-01.01 */
		/* It is redundant because SAHPI_NO_RECORD == 0, Put code here for clarity */
		break;			           
	case SAHPI_ET_SENSOR:
		rdrid = oh_get_rdr_uid(SAHPI_SENSOR_RDR,
				    thisEvent->EventDataUnion.SensorEvent.SensorNum);
		thisRdr =  oh_get_rdr_by_id(handle->rptcache, thisEvent->Source, rdrid);
		if (thisRdr) 
			e->rdrs = g_slist_append(e->rdrs, g_memdup(thisRdr, sizeof(SaHpiRdrT)));
		else 
			err("Rdr not found for rid %d, rdrid %d\n",thisEvent->Source, rdrid);
		break;
	case SAHPI_ET_WATCHDOG:
		rdrid = oh_get_rdr_uid(SAHPI_WATCHDOG_RDR,
				    thisEvent->EventDataUnion.WatchdogEvent.WatchdogNum);
		thisRdr =  oh_get_rdr_by_id(handle->rptcache, thisEvent->Source, rdrid);
		if (thisRdr) 
			e->rdrs = g_slist_append(e->rdrs, g_memdup(thisRdr, sizeof(SaHpiRdrT)));
		else 
			err("Rdr not found for rid %d, rdrid %d\n",thisEvent->Source, rdrid);

		break;
	case SAHPI_ET_RESOURCE:
	case SAHPI_ET_DOMAIN:
	case SAHPI_ET_SENSOR_ENABLE_CHANGE:
	case SAHPI_ET_HPI_SW:
	default:
		err("Unsupported Event Type=%s.", oh_lookup_eventtype(thisEvent->EventType));
		return(SA_ERR_HPI_INTERNAL_ERROR);
		break;
	} 
	

	if (prepend == SAHPI_TRUE) { 
       		custom_handle->eventq = g_slist_prepend(custom_handle->eventq, e);
	} else {
		custom_handle->eventq = g_slist_append(custom_handle->eventq, e);
	}
	
        return(SA_OK);
}
