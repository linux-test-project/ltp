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

#include <snmp_bc_plugin.h>

/**
 * snmp_bc_get_sensor_reading:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @data: Location to store sensor's value (may be NULL).
 * @state: Location to store sensor's state (may be NULL).
 *
 * Retrieves a sensor's value and/or state. Both @data and @state
 * may be NULL, in which case this function can be used to test for
 * sensor presence.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 * SA_ERR_HPI_INVALID_REQUEST - Sensor is disabled.
 * SA_ERR_HPI_NOT_PRESENT - Sensor doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT snmp_bc_get_sensor_reading(void *hnd,
				    SaHpiResourceIdT rid,
				    SaHpiSensorNumT sid,
				    SaHpiSensorReadingT *reading,
				    SaHpiEventStateT *state)
{
	SaErrorT err;
	SaHpiSensorReadingT working_reading;
	SaHpiEventStateT working_state;
	struct SensorInfo *sinfo;
        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;
        SaHpiRdrT *rdr;

	if (!hnd) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has sensor capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {	
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Check if sensor exists and is enabled */
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
	sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
 	if (sinfo == NULL) {
		snmp_bc_unlock_handler(custom_handle);
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       
	
	if (sinfo->sensor_enabled == SAHPI_FALSE) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_REQUEST);
	}

	memset(&working_reading, 0, sizeof(SaHpiSensorReadingT));
	working_state = SAHPI_ES_UNSPECIFIED;

	dbg("Sensor Reading: Resource=%s; RDR=%s", rpt->ResourceTag.Data, rdr->IdString.Data);
	
	/************************************************************
	 * Get sensor's reading.
         * Need to go through this logic, since user may request just
         * the event state for a readable sensor. Need to translate
         * sensor reading to event in this case.
         ************************************************************/
	if (rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported == SAHPI_TRUE) {
		err = SA_OK;
		if ((sid == BLADECENTER_SENSOR_NUM_MGMNT_ACTIVE) || 
				(sid ==  BLADECENTER_SENSOR_NUM_MGMNT_STANDBY))
		{ 
			err = snmp_bc_get_logical_sensors(hnd, rid, sid, &working_reading);
		
		}
		else if (sid == BLADECENTER_SENSOR_NUM_SLOT_STATE)
		{
			err = snmp_bc_get_slot_state_sensor(hnd, rid, sid, &working_reading);
		}
		else if ( (sid == BLADECENTER_SENSOR_NUM_MAX_POWER) ||
				(sid == BLADECENTER_SENSOR_NUM_ASSIGNED_POWER) ||
				(sid == BLADECENTER_SENSOR_NUM_MIN_POWER) )
		{
			err = snmp_bc_get_slot_power_sensor(hnd, rid, sid, &working_reading);
		}
		else                /* Normal sensors */
		{ 
			err = snmp_bc_get_sensor_oid_reading(hnd, rid, sid, sinfo->mib.oid, &working_reading);
		}
	
		if (err) {
			err("Cannot determine sensor's reading. Error=%s", oh_lookup_error(err));
			snmp_bc_unlock_handler(custom_handle);
			return(err);
		}
	}
	else {
		working_reading.IsSupported = SAHPI_FALSE;
	}

	/******************************************************************
	 * Get sensor's event state.
         * Always get the event state, to reset the sensor's current state,
         * whether caller wants to know event state or not.
	 ******************************************************************/
	err = snmp_bc_get_sensor_eventstate(hnd, rid, sid, &working_reading, &working_state);
	if (err) {
		err("Cannot determine sensor's event state. Error=%s", oh_lookup_error(err));
		snmp_bc_unlock_handler(custom_handle);
		return(err);
	}

#if 0
	{       /* Debug section */
		SaHpiTextBufferT buffer;

		dbg("Sensor=%s", rdr->IdString.Data);
		oh_decode_sensorreading(working_reading, rdr->RdrTypeUnion.SensorRec.DataFormat, &buffer);
		dbg("  Reading: %s.", buffer.Data);
		
		oh_decode_eventstate(working_state, rdr->RdrTypeUnion.SensorRec.Category, &buffer);
		dbg("  Event State: %s\n", buffer.Data);
	}
#endif

	/* sinfo->cur_state = working_state; */
	if (reading) memcpy(reading, &working_reading, sizeof(SaHpiSensorReadingT));
	if (state) memcpy(state, &working_state, sizeof(SaHpiEventStateT));

	snmp_bc_unlock_handler(custom_handle);
        return(SA_OK);
}

/**
 * snmp_bc_get_sensor_eventstate:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @reading: Location of sensor's reading
 * @state: Location to store sensor's state.
 *
 * Translates and sensor's reading into an event state(s).
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 * SA_ERR_HPI_INVALID_REQUEST - Sensor is disabled.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 * SA_ERR_HPI_NOT_PRESENT - Sensor doesn't exist.
 **/
SaErrorT snmp_bc_get_sensor_eventstate(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       SaHpiSensorReadingT *reading,
				       SaHpiEventStateT *state)
{	
	int i;
	struct SensorInfo *sinfo;
        struct oh_handler_state *handle;
	SaHpiRptEntryT *rpt;
        SaHpiRdrT *rdr;
	SaErrorT err;

	if (!hnd || !reading || !state) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	/* Check if resource exists and has sensor capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) return(SA_ERR_HPI_INVALID_RESOURCE);
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) return(SA_ERR_HPI_CAPABILITY);
	
	/* Check if sensor exist and is enabled */
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL) return(SA_ERR_HPI_NOT_PRESENT);
	sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
 	if (sinfo == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       
	if (sinfo->sensor_enabled == SAHPI_FALSE) return(SA_ERR_HPI_INVALID_REQUEST);

	/* If sensor is not readable, return current event state */
	if (rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported == SAHPI_FALSE) {
		*state = sinfo->cur_state;
		return(SA_OK);
	}
	
	/***************************************************************************
	 * Translate reading into event state. Algorithm is:
	 * - If sensor is a threshold and has readable thresholds.
         *   - If so, check from most severe to least
	 * - If not found or (not a threshold value && not present sensor), search reading2event array.
	 *   - Check for Ranges supported; return after first match.
	 *   - Nominal only - reading must match nominal value
	 *   - Max && Min - min value <= reading <= max value
	 *   - Max only - reading > max value 
	 *   - Min only - reading < min value
	 *   - else SAHPI_ES_UNSPECIFIED
	 ***************************************************************************/
	if (rdr->RdrTypeUnion.SensorRec.Category == SAHPI_EC_THRESHOLD &&
	    rdr->RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold != 0) {
		SaHpiSensorThresholdsT thres;		
		memset(&thres, 0, sizeof(SaHpiSensorThresholdsT));

		err = snmp_bc_get_sensor_thresholds(hnd, rid, sid, &thres);
		if (err) {
			err("Cannot get sensor thresholds for Sensor=%s. Error=%s", 
			    rdr->IdString.Data, oh_lookup_error(err));
			return(err);
		}
		if (thres.LowCritical.IsSupported == SAHPI_TRUE) {
			if (oh_compare_sensorreading(reading->Type, reading, &thres.LowCritical) <= 0) {
				*state = *state | SAHPI_ES_LOWER_CRIT;
				return(SA_OK);
			}
		}
		if (thres.LowMajor.IsSupported == SAHPI_TRUE) {
			if (oh_compare_sensorreading(reading->Type, reading, &thres.LowMajor) <= 0) {
				*state = *state | SAHPI_ES_LOWER_MAJOR;
				return(SA_OK);
			}
		}
		if (thres.LowMinor.IsSupported == SAHPI_TRUE) {
			if (oh_compare_sensorreading(reading->Type, reading, &thres.LowMinor) <= 0) {
				*state = *state | SAHPI_ES_LOWER_MINOR;
				return(SA_OK);
			}
		}
		if (thres.UpCritical.IsSupported == SAHPI_TRUE) {
			if (oh_compare_sensorreading(reading->Type, reading, &thres.UpCritical) >= 0) {
				*state = *state | SAHPI_ES_UPPER_CRIT;
				return(SA_OK);
			}
		}
		if (thres.UpMajor.IsSupported == SAHPI_TRUE) {
			if (oh_compare_sensorreading(reading->Type, reading, &thres.UpMajor) >= 0) {
				*state = *state | SAHPI_ES_UPPER_MAJOR;
				return(SA_OK);
			}
		}
		if (thres.UpMinor.IsSupported == SAHPI_TRUE) {		
			if (oh_compare_sensorreading(reading->Type, reading, &thres.UpMinor) >= 0) {
				*state = *state | SAHPI_ES_UPPER_MINOR;
				return(SA_OK);
			}
		}
		
	} else if (rdr->RdrTypeUnion.SensorRec.Category == SAHPI_EC_PRESENCE) {
		if ((sid == BLADECENTER_SENSOR_NUM_SLOT_STATE) ||
			(sid == BLADECENTER_SENSOR_NUM_MGMNT_STANDBY))
			*state = sinfo->cur_state;
		else
			*state = SAHPI_ES_PRESENT;
		
	} else {		
	
		/* Check reading2event array */
		for (i=0; i < SNMP_BC_MAX_SENSOR_READING_MAP_ARRAY_SIZE &&
				     sinfo->reading2event[i].num != 0; i++) {

			/* reading == nominal */
			if (sinfo->reading2event[i].rangemap.Flags & SAHPI_SRF_NOMINAL) {
				if (oh_compare_sensorreading(reading->Type, reading, 
						     &sinfo->reading2event[i].rangemap.Nominal) == 0) {
					*state = sinfo->reading2event[i].state;
					return(SA_OK);
				}
			}
			/* min <= reading <= max */
			if (sinfo->reading2event[i].rangemap.Flags & SAHPI_SRF_MAX &&
		    			sinfo->reading2event[i].rangemap.Flags & SAHPI_SRF_MIN) {
				if (oh_compare_sensorreading(reading->Type, reading, 
						     &sinfo->reading2event[i].rangemap.Min) >= 0 &&
			    				oh_compare_sensorreading(reading->Type, reading, 
						     	&sinfo->reading2event[i].rangemap.Max) <= 0) {
					*state = sinfo->reading2event[i].state;
					return(SA_OK);
				}
			}
			/* reading > max */
			if (sinfo->reading2event[i].rangemap.Flags & SAHPI_SRF_MAX &&
		    			!(sinfo->reading2event[i].rangemap.Flags & SAHPI_SRF_MIN)) {
				if (oh_compare_sensorreading(reading->Type, reading, 
						     &sinfo->reading2event[i].rangemap.Max) > 0) {
					*state = sinfo->reading2event[i].state;
					return(SA_OK);
				}
			}
			/* reading < min */
			if (!(sinfo->reading2event[i].rangemap.Flags & SAHPI_SRF_MAX) &&
		    			sinfo->reading2event[i].rangemap.Flags & SAHPI_SRF_MIN) {
				if (oh_compare_sensorreading(reading->Type, reading, 
						     &sinfo->reading2event[i].rangemap.Min) < 0) {
					*state = sinfo->reading2event[i].state;
					return(SA_OK);
				}
			}
		}

        	/* Unfortunately for thresholds, this also means normal */
		*state = SAHPI_ES_UNSPECIFIED; 
	}
	
	return(SA_OK);
}

#define get_threshold(thdmask, thdname) \
do { \
        if (rdr->RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold & thdmask) { \
		if (sinfo->mib.threshold_oids.thdname == NULL || \
		    sinfo->mib.threshold_oids.thdname[0] == '\0') { \
			err("No OID defined for readable threshold. Sensor=%s", rdr->IdString.Data); \
			snmp_bc_unlock_handler(custom_handle); \
			return(SA_ERR_HPI_INTERNAL_ERROR); \
		} \
		err = snmp_bc_get_sensor_oid_reading(hnd, rid, sid, \
							      sinfo->mib.threshold_oids.thdname, \
							      &(working.thdname)); \
		if (err) { \
			snmp_bc_unlock_handler(custom_handle); \
			return(err); \
		} \
		if (working.thdname.Type == SAHPI_SENSOR_READING_TYPE_BUFFER) { \
			err("Sensor type SAHPI_SENSOR_READING_TYPE_BUFFER cannot have thresholds. Sensor=%s", \
			    rdr->IdString.Data); \
			snmp_bc_unlock_handler(custom_handle); \
			return(SA_ERR_HPI_INTERNAL_ERROR); \
		} \
		found_thresholds = found_thresholds | thdmask; \
	} \
	else { \
		working.thdname.IsSupported = SAHPI_FALSE; \
	} \
} while(0)


/**
 * snmp_bc_get_virtual_MM_sensor:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @reading: Location of sensor's reading
 *
 * Re-direct and read sensors from real physical MM for Virtual MM..
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 * SA_ERR_HPI_INVALID_REQUEST - Sensor is disabled.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 * SA_ERR_HPI_NOT_PRESENT - Sensor doesn't exist.
 **/
SaErrorT snmp_bc_get_logical_sensors(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       SaHpiSensorReadingT *reading)
				     
{	
        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	struct snmp_value  mm_install_mask, active_mm_id;
        SaHpiEntityPathT   ep_root, res_ep;
	char * root_tuple;
	int    mm_id;
	SaErrorT err;
	


	if (!hnd) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	err = SA_OK;
	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	/* Fetch MMs installed vector  */
	get_installed_mask(SNMP_BC_MM_INSTALLED, mm_install_mask);
	 
	/* Fetch Active MM ID */
	err = snmp_bc_snmp_get(custom_handle, SNMP_BC_MGMNT_ACTIVE, &active_mm_id, SAHPI_TRUE);
        if (err || active_mm_id.type != ASN_INTEGER) {
		err("Cannot get SNMP_BC_MGMNT_ACTIVE=%s; Received Type=%d; Error=%s.",
		      SNMP_BC_MGMNT_ACTIVE, active_mm_id.type, oh_lookup_error(err));
		if (err) { return(err); }
		else { return(SA_ERR_HPI_INTERNAL_ERROR); }
        }
	
	mm_id = SNMP_BC_NOT_VALID;  /* Init it to someting invalid, so we can visually catch error */
	switch (sid) {
		case BLADECENTER_SENSOR_NUM_MGMNT_ACTIVE:
			mm_id = active_mm_id.integer;
			break;
		case BLADECENTER_SENSOR_NUM_MGMNT_STANDBY:
			if ( atoi(mm_install_mask.string) > 10) {
				switch(active_mm_id.integer)
				{
					case 1:
						mm_id = 2;
						break;
					case 2:
						mm_id = 1;
						break;
					default:
						err("Internal Error.");
						break;
				}
			}
			break; 
		default:
			err("Should not be here. sid is not one of the special sensors.");
			break;
	}

	/* Complete Sensor Read record */
	reading->IsSupported = SAHPI_TRUE;
	reading->Type  = SAHPI_SENSOR_READING_TYPE_UINT64;
		
	if (mm_id != SNMP_BC_NOT_VALID) {
		root_tuple = (char *)g_hash_table_lookup(handle->config, "entity_root");
        	if (root_tuple == NULL) {
                	err("Cannot find configuration parameter.");
                	snmp_bc_unlock_handler(custom_handle);
                	return(SA_ERR_HPI_INTERNAL_ERROR);
        	}
        	err = oh_encode_entitypath(root_tuple, &ep_root);
			
		res_ep = snmp_bc_rpt_array[BC_RPT_ENTRY_MGMNT_MODULE].rpt.ResourceEntity;
		oh_concat_ep(&res_ep, &ep_root);
		oh_set_ep_location(&res_ep,  BLADECENTER_SYS_MGMNT_MODULE_SLOT, mm_id);			
		oh_set_ep_location(&res_ep,  SAHPI_ENT_SYS_MGMNT_MODULE, mm_id);	
		reading->Value.SensorUint64 = (SaHpiUint64T) oh_uid_from_entity_path(&res_ep);
		
	} else {
		reading->Value.SensorUint64 = SAHPI_UNSPECIFIED_RESOURCE_ID;
	}        
	
	
	return(err);
}

/**
 * snmp_bc_get_sensor_thresholds:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @thres: Location to store sensor's threshold values.
 *
 * Retreives sensor's threshold values, if defined.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 * SA_ERR_HPI_INVALID_CMD - Sensor is not threshold type, has accessible or readable thresholds.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 * SA_ERR_HPI_NOT_PRESENT - Sensor doesn't exist.
 **/
SaErrorT snmp_bc_get_sensor_thresholds(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       SaHpiSensorThresholdsT *thres)
{	
	int upper_thresholds, lower_thresholds;
	SaHpiSensorThdMaskT  found_thresholds;
	SaHpiSensorThresholdsT working;
        struct SensorInfo *sinfo;
        struct oh_handler_state *handle;	
	struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;
        SaHpiRdrT *rdr;
	SaErrorT err;

	if (!hnd || !thres) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	handle = (struct oh_handler_state *)hnd;	
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has sensor capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {	
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Check if sensor exits and has readable thresholds */
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL){
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
        sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
 	if (sinfo == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}
	
	if (rdr->RdrTypeUnion.SensorRec.Category != SAHPI_EC_THRESHOLD ||
	    rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible == SAHPI_FALSE ||
	    rdr->RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold == 0) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_CMD);
	}

        memset(&working, 0, sizeof(SaHpiSensorThresholdsT));
	found_thresholds = lower_thresholds = upper_thresholds = 0;
	
	get_threshold(SAHPI_STM_LOW_MINOR, LowMinor);
	if (found_thresholds & SAHPI_STM_LOW_MINOR) lower_thresholds++;
	get_threshold(SAHPI_STM_LOW_MAJOR, LowMajor);
	if (found_thresholds & SAHPI_STM_LOW_MAJOR) lower_thresholds++;
	get_threshold(SAHPI_STM_LOW_CRIT, LowCritical);
	if (found_thresholds & SAHPI_STM_LOW_CRIT) lower_thresholds++;
	get_threshold(SAHPI_STM_UP_MINOR, UpMinor);
	if (found_thresholds & SAHPI_STM_UP_MINOR) upper_thresholds++;
	get_threshold(SAHPI_STM_UP_MAJOR, UpMajor);
	if (found_thresholds & SAHPI_STM_UP_MAJOR) upper_thresholds++;
	get_threshold(SAHPI_STM_UP_CRIT, UpCritical);
	if (found_thresholds & SAHPI_STM_UP_CRIT) upper_thresholds++;

	/************************************************************************
	 * Find Hysteresis Values
	 *
         * Hardware can define hysteresis values two ways:
         * 
         * - As delta values as defined in the spec. In this case, 
         *   PosThdHysteresis and/or NegThdHysteresis values are defined
         *   and this routine just returns those values.
         *
         * - Total values - as in threshold is 80 degrees; positive hysteresis is
         *   defined to be 78 degrees. In this case, TotalPosThdHysteresis and/or
         *   TotalNegThdHysteresis are defined and this routine needs to make 
         *   the required calculations to return to the user a delta value. Total
         *   values can only be used if:
         *   1) if there is more thanone upper/lower threshold defined, the least
         *      critical threshold is used as the base for calculating delta values.
         *   2) Total values cannot be of type SAHPI_SENSOR_READING_TYPE_UINT64 or 
         *      SAHPI_SENSOR_READING_TYPE_BUFFER.
         *
         *   Code can support a delta value for one set of thresholds (upper or 
         *   lower) and a total value for the opposite set.
         *************************************************************************/

	if (sinfo->mib.threshold_oids.NegThdHysteresis) {
		get_threshold(SAHPI_STM_LOW_HYSTERESIS, NegThdHysteresis);
	}
	if (sinfo->mib.threshold_oids.PosThdHysteresis) {
		get_threshold(SAHPI_STM_UP_HYSTERESIS, PosThdHysteresis);
	}

	/* Negative Total Hysteresis - applies to lower thresholds */
	if (sinfo->mib.threshold_oids.TotalNegThdHysteresis) {
		SaHpiSensorReadingT reading;

		if (found_thresholds & SAHPI_STM_LOW_HYSTERESIS) {
			err("Cannot define both delta and total negative hysteresis. Sensor=%s",
			    rdr->IdString.Data);
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
		if (lower_thresholds == 0) {
			err("No lower thresholds are defined for total negative hysteresis. Sensor=%s",
			    rdr->IdString.Data);
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}

		/* Get negative hysteresis value */
		err = snmp_bc_get_sensor_oid_reading(hnd, rid, sid,
							      sinfo->mib.threshold_oids.TotalNegThdHysteresis,
							      &reading);
		if (err) {
			snmp_bc_unlock_handler(custom_handle);
			return(err);
		}
		
		switch (rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType) {
		case SAHPI_SENSOR_READING_TYPE_INT64:
		{
			SaHpiInt64T delta;
			if (found_thresholds & SAHPI_STM_LOW_MINOR) {
				delta = reading.Value.SensorInt64 - working.LowMinor.Value.SensorInt64;
			}
			else {
				if (found_thresholds & SAHPI_STM_LOW_MAJOR) {
					delta = reading.Value.SensorInt64 - working.LowMajor.Value.SensorInt64;
				}
				else {
					delta = reading.Value.SensorInt64 - working.LowCritical.Value.SensorInt64;
				}
			}

			if (delta < 0) {
				err("Negative hysteresis delta is less than 0");
				working.NegThdHysteresis.IsSupported = SAHPI_FALSE;
			}
			else {
				working.NegThdHysteresis.IsSupported = SAHPI_TRUE;
				working.NegThdHysteresis.Type = SAHPI_SENSOR_READING_TYPE_INT64;
				working.NegThdHysteresis.Value.SensorInt64 = delta;
			}
			break;
		}
		case SAHPI_SENSOR_READING_TYPE_FLOAT64:
		{
			SaHpiFloat64T delta;
			if (found_thresholds & SAHPI_STM_LOW_MINOR) {
				delta = reading.Value.SensorFloat64 - working.LowMinor.Value.SensorFloat64;
			}
			else {
				if (found_thresholds & SAHPI_STM_LOW_MAJOR) {
					delta = reading.Value.SensorFloat64 - working.LowMajor.Value.SensorFloat64;
				}
				else {
					delta = reading.Value.SensorFloat64 - working.LowCritical.Value.SensorFloat64;
				}
			}

			if (delta < 0) {
				err("Negative hysteresis delta is less than 0");
				working.NegThdHysteresis.IsSupported = SAHPI_FALSE;
			}
			else {
				working.NegThdHysteresis.IsSupported = SAHPI_TRUE;
				working.NegThdHysteresis.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
				working.NegThdHysteresis.Value.SensorFloat64 = delta;
			}
			break;
		}
		case SAHPI_SENSOR_READING_TYPE_UINT64:
		case SAHPI_SENSOR_READING_TYPE_BUFFER:
		default:
			err("Invalid reading type for threshold. Sensor=%s", rdr->IdString.Data);
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
	}
	
	/* Positive Total Hysteresis - applies to upper thresholds */
	if (sinfo->mib.threshold_oids.TotalPosThdHysteresis) {
		SaHpiSensorReadingT reading;

		if (found_thresholds & SAHPI_STM_UP_HYSTERESIS) {
			err("Cannot define both delta and total positive hysteresis. Sensor=%s",
			    rdr->IdString.Data);			    
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
		if (upper_thresholds == 0) {
			err("No upper thresholds are defined for total positive hysteresis. Sensor=%s",
			    rdr->IdString.Data);
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}

		/* Get positive hysteresis value */
		err = snmp_bc_get_sensor_oid_reading(hnd, rid, sid,
							      sinfo->mib.threshold_oids.TotalPosThdHysteresis,
							      &reading);
		if (err) {
			snmp_bc_unlock_handler(custom_handle);
			return(err);
		}
		
		switch (rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType) {
		case SAHPI_SENSOR_READING_TYPE_INT64:
		{
			SaHpiInt64T delta;
			if (found_thresholds & SAHPI_STM_UP_MINOR) {
				delta = working.UpMinor.Value.SensorInt64 - reading.Value.SensorInt64;
			}
			else {
				if (found_thresholds & SAHPI_STM_UP_MAJOR) {
					delta = working.UpMajor.Value.SensorInt64 - reading.Value.SensorInt64;
				}
				else {
					delta = working.UpCritical.Value.SensorInt64 - reading.Value.SensorInt64;
				}
			}

			if (delta < 0) {
				err("Positive hysteresis delta is less than 0");
				working.PosThdHysteresis.IsSupported = SAHPI_FALSE;
			}
			else {
				working.PosThdHysteresis.IsSupported = SAHPI_TRUE;
				working.PosThdHysteresis.Type = SAHPI_SENSOR_READING_TYPE_INT64;
				working.PosThdHysteresis.Value.SensorInt64 = delta;
			}
			break;
		}
		case SAHPI_SENSOR_READING_TYPE_FLOAT64:
		{
			SaHpiFloat64T delta;
			if (found_thresholds & SAHPI_STM_UP_MINOR) {
				delta = working.UpMinor.Value.SensorFloat64 - reading.Value.SensorFloat64;
			}
			else {
				if (found_thresholds & SAHPI_STM_UP_MAJOR) {
					delta = working.UpMajor.Value.SensorFloat64 - reading.Value.SensorFloat64;
				}
				else {
					delta = working.UpCritical.Value.SensorFloat64 - reading.Value.SensorFloat64;
				}
			}

			if (delta < 0) {
				err("Positive hysteresis delta is less than 0");
				working.PosThdHysteresis.IsSupported = SAHPI_FALSE;
			}
			else {
				working.PosThdHysteresis.IsSupported = SAHPI_TRUE;
				working.PosThdHysteresis.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
				working.PosThdHysteresis.Value.SensorFloat64 = delta;
			}

			break;
		}
		case SAHPI_SENSOR_READING_TYPE_UINT64:
		case SAHPI_SENSOR_READING_TYPE_BUFFER:
		default:
			err("Invalid reading type for threshold. Sensor=%s", rdr->IdString.Data);
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
	}

	if (found_thresholds == 0) {
		err("No readable thresholds found. Sensor=%s", rdr->IdString.Data);
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	memcpy(thres, &working, sizeof(SaHpiSensorThresholdsT));
	snmp_bc_unlock_handler(custom_handle);
	return(SA_OK);
}

#define merge_threshold(thdname) \
do { \
        if (thres->thdname.IsSupported) { \
                working.thdname.IsSupported = SAHPI_TRUE; \
                working.thdname.Type = thres->thdname.Type; \
        	switch(thres->thdname.Type) { \
        	case SAHPI_SENSOR_READING_TYPE_INT64: \
        		working.thdname.Value.SensorInt64 = thres->thdname.Value.SensorInt64; \
        		break; \
        	case SAHPI_SENSOR_READING_TYPE_FLOAT64: \
        		working.thdname.Value.SensorFloat64 = thres->thdname.Value.SensorFloat64; \
        		break; \
        	case SAHPI_SENSOR_READING_TYPE_UINT64: \
        		working.thdname.Value.SensorUint64 = thres->thdname.Value.SensorUint64; \
		        break; \
        	case SAHPI_SENSOR_READING_TYPE_BUFFER: \
        	default: \
        		err("Invalid threshold reading type."); \
			snmp_bc_unlock_handler(custom_handle); \
        		return(SA_ERR_HPI_INVALID_CMD); \
        	} \
        } \
} while(0)

#define write_valid_threshold(thdname) \
do { \
	if (thres->thdname.IsSupported) { \
		if (sinfo->mib.threshold_write_oids.thdname == NULL || \
		    sinfo->mib.threshold_oids.thdname[0] == '\0') { \
			err("No writable threshold OID defined for thdname."); \
			snmp_bc_unlock_handler(custom_handle); \
			return(SA_ERR_HPI_INTERNAL_ERROR); \
		} \
		err = snmp_bc_set_threshold_reading(hnd, rid, sid, \
					            sinfo->mib.threshold_write_oids.thdname, \
						    &(working.thdname)); \
		if (err) { \
			snmp_bc_unlock_handler(custom_handle); \
			return(err); \
		} \
	} \
} while(0)

/**
 * snmp_bc_set_sensor_thresholds:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @thres: Location of sensor's settable threshold values.
 *
 * Sets sensor's threshold values.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 * SA_ERR_HPI_INVALID_CMD - Non-writable thresholds or invalid thresholds.
 * SA_ERR_HPI_INVALID_DATA - Threshold values out of order; negative hysteresis
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 * SA_ERR_HPI_NOT_PRESENT - Sensor doesn't exist.
 **/
SaErrorT snmp_bc_set_sensor_thresholds(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       const SaHpiSensorThresholdsT *thres)
{	
	SaErrorT err;
	SaHpiSensorThresholdsT working;
	struct oh_handler_state *handle;
	struct SensorInfo *sinfo;
	struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;
        SaHpiRdrT *rdr;

	if (!hnd || !thres) {
		err("Invalid parameter");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has sensor capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Check if sensor exists and has writable thresholds */
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
	sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
 	if (sinfo == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       
	if (rdr->RdrTypeUnion.SensorRec.Category != SAHPI_EC_THRESHOLD ||
	    rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible == SAHPI_FALSE ||
	    rdr->RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold == 0) {
	    	snmp_bc_unlock_handler(custom_handle);
	    	return(SA_ERR_HPI_INVALID_CMD);
	}
  
	/* Overlay proposed thresholds on top of existing ones and validate */
	err = snmp_bc_get_sensor_thresholds(hnd, rid, sid, &working);
	if (err) {
		snmp_bc_unlock_handler(custom_handle);
		return(err);
	}
	
	merge_threshold(LowCritical);
	merge_threshold(LowMajor);
	merge_threshold(LowMinor);
	merge_threshold(UpCritical);
	merge_threshold(UpMajor);
	merge_threshold(UpMinor);
	merge_threshold(PosThdHysteresis);
	merge_threshold(NegThdHysteresis);
	
	err = oh_valid_thresholds(&working, rdr);
	if (err) {
		snmp_bc_unlock_handler(custom_handle);
		return(err);
	}
	
	/************************ 
	 * Write valid thresholds
         ************************/
	write_valid_threshold(UpCritical);
	write_valid_threshold(UpMajor);
	write_valid_threshold(UpMinor);
	write_valid_threshold(LowCritical);
	write_valid_threshold(LowMajor);
	write_valid_threshold(LowMinor);

	/* We don't support writing total value hysteresis only deltas */
	write_valid_threshold(NegThdHysteresis);
	write_valid_threshold(PosThdHysteresis);

	snmp_bc_unlock_handler(custom_handle);
	return(SA_OK);
}

SaErrorT snmp_bc_get_sensor_oid_reading(void *hnd,
					SaHpiResourceIdT rid,
					SaHpiSensorNumT sid,
					const char *raw_oid,
					SaHpiSensorReadingT *reading)
{
	SaHpiSensorReadingT working;
	SaErrorT err;
	struct SensorInfo *sinfo;
	struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	struct snmp_value get_value;
	SaHpiEntityPathT valEntity;
	SaHpiRdrT *rdr;

	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL) return(SA_ERR_HPI_NOT_PRESENT);
	sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
 	if (sinfo == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Normalize and read sensor's raw SNMP OID */
	err = snmp_bc_validate_ep(&(rdr->Entity), &valEntity);
	err = snmp_bc_oid_snmp_get(custom_handle, &valEntity, sinfo->mib.loc_offset,
				   raw_oid, &get_value, SAHPI_TRUE);
	if (err) {
		err("SNMP cannot read sensor OID=%s. Type=%d", raw_oid, get_value.type);
		return(err);
	}
		
	/* Convert SNMP value to HPI reading value */
	working.IsSupported = SAHPI_TRUE;
	if (get_value.type == ASN_INTEGER) {
		working.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		working.Value.SensorInt64 = (SaHpiInt64T)get_value.integer;
	} 
	else {
		SaHpiTextBufferT buffer;
		oh_init_textbuffer(&buffer);
		oh_append_textbuffer(&buffer, get_value.string);
		
		err = oh_encode_sensorreading(&buffer,
					      rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType,
					      &working);
		if (err) {
			err("Cannot convert sensor OID=%s value=%s. Error=%s",
			    sinfo->mib.oid, buffer.Data, oh_lookup_error(err));
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
	}
	
	memcpy(reading, &working, sizeof(SaHpiSensorReadingT));

	return(SA_OK);
}

SaErrorT snmp_bc_set_threshold_reading(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       const char *raw_oid,
				       const SaHpiSensorReadingT *reading)
{	
	SaErrorT err;
	SaHpiTextBufferT buffer;
	SaHpiFloat64T tmp_num;
	struct SensorInfo *sinfo;
	struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	struct snmp_value set_value;
	SaHpiEntityPathT valEntity;
	SaHpiRdrT *rdr;

	if (!hnd || !reading || !raw_oid) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL) return(SA_ERR_HPI_NOT_PRESENT);
	sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
 	if (sinfo == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Convert reading into SNMP string structure */
	err = oh_init_textbuffer(&buffer);
	if (err) return(err);

	switch (reading->Type) {
	case SAHPI_SENSOR_READING_TYPE_INT64:
		tmp_num = (SaHpiFloat64T)reading->Value.SensorInt64;
		break;
	case SAHPI_SENSOR_READING_TYPE_FLOAT64:
		tmp_num = reading->Value.SensorFloat64;
		break;
	case SAHPI_SENSOR_READING_TYPE_UINT64:
		tmp_num = (SaHpiFloat64T)reading->Value.SensorUint64;
		break;
	case SAHPI_SENSOR_READING_TYPE_BUFFER:
		default:
			err("Invalid type for threshold. Sensor=%s", rdr->IdString.Data);
			return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	/*************************************************************
	 * NOTE! Assuming max format for writable thresholds is ddd.dd
         *************************************************************/
 	snprintf((char *)buffer.Data, SAHPI_MAX_TEXT_BUFFER_LENGTH, "%'+3.2f", tmp_num);

	/* Copy string to SNMP structure */
	set_value.type = ASN_OCTET_STR;
	g_ascii_strncasecmp(set_value.string, (char *)buffer.Data, buffer.DataLength);

	/* Normalize and read sensor's raw SNMP OID */
	err = snmp_bc_validate_ep(&(rdr->Entity), &valEntity);
	err = snmp_bc_oid_snmp_set(custom_handle, &valEntity, sinfo->mib.loc_offset,
				   raw_oid, set_value);
	if (err) {
		err("SNMP cannot set sensor OID=%s.", raw_oid);
		return(err);
	}
		
	return(SA_OK);
}

/**
 * snmp_bc_get_sensor_enable:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @enable: Location to store sensor's enablement boolean.
 *
 * Retrieves a sensor's boolean enablement status.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - @enable is NULL.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 * SA_ERR_HPI_NOT_PRESENT - Sensor doesn't exist.
 **/
SaErrorT snmp_bc_get_sensor_enable(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiSensorNumT sid,
				   SaHpiBoolT *enable)
{	
	struct oh_handler_state *handle;
	struct SensorInfo *sinfo;
	struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;
        SaHpiRdrT *rdr;

	if (!hnd || !enable) {
		err("Invalid parameter");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has sensor capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Check if sensor exists and return enablement status */
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
	sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
 	if (sinfo == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       
	
	*enable = sinfo->sensor_enabled;

	snmp_bc_unlock_handler(custom_handle);
	return(SA_OK);
}

/**
 * snmp_bc_set_sensor_enable:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @enable: Enable/disable sensor.
 *
 * Sets a sensor's boolean enablement status.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 * SA_ERR_HPI_NOT_PRESENT - Sensor doesn't exist.
 **/
SaErrorT snmp_bc_set_sensor_enable(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiSensorNumT sid,
				   const SaHpiBoolT enable)
{
	struct oh_handler_state *handle;
	struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;
        SaHpiRdrT *rdr;
	struct SensorInfo *sinfo;

	if (!hnd ) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has sensor capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Check if sensor exists and if it supports setting of sensor enablement */
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
	if (rdr->RdrTypeUnion.SensorRec.EnableCtrl == SAHPI_TRUE) {
		err("BladeCenter/RSA do not support snmp_bc_set_sensor_enable");
		sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
		if (sinfo == NULL) {
			err("No sensor data. Sensor=%s", rdr->IdString.Data);
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}

		if (sinfo->sensor_enabled != enable) {
			/* Probably need to drive an OID, if hardware supported it */
			sinfo->sensor_enabled = enable;
			/* FIXME:: Add SAHPI_ET_SENSOR_ENABLE_CHANGE event on IF event Q */
		}
	}
	else {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_READ_ONLY);
	}

	snmp_bc_unlock_handler(custom_handle);
	return(SA_OK);
}

/**
 * snmp_bc_get_sensor_event_enable:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @enable: Location to store sensor event enablement boolean.
 *
 * Retrieves a sensor's boolean event enablement status.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 * SA_ERR_HPI_NOT_PRESENT - Sensor doesn't exist.
 **/
SaErrorT snmp_bc_get_sensor_event_enable(void *hnd,
					 SaHpiResourceIdT rid,
					 SaHpiSensorNumT sid,
					 SaHpiBoolT *enable)
{	
	struct oh_handler_state *handle;
	struct SensorInfo *sinfo;
	struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;
        SaHpiRdrT *rdr;

	if (!hnd || !enable) {
		err("Invalid parameter");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has sensor capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Check if sensor exists and return enablement status */
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
	sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
 	if (sinfo == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       
	
	*enable = sinfo->events_enabled;

	snmp_bc_unlock_handler(custom_handle);
        return(SA_OK);
}

/**
 * snmp_bc_set_sensor_event_enable:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @enable: Enable/disable sensor.
 *
 * Sets a sensor's boolean event enablement status.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 * SA_ERR_HPI_NOT_PRESENT - Sensor doesn't exist.
 **/
SaErrorT snmp_bc_set_sensor_event_enable(void *hnd,
					 SaHpiResourceIdT rid,
					 SaHpiSensorNumT sid,
					 const SaHpiBoolT enable)
{
	struct oh_handler_state *handle;
	struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;
        SaHpiRdrT *rdr;
	struct SensorInfo *sinfo;

	if (!hnd ) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	snmp_bc_lock_handler(custom_handle);

	/* Check if resource exists and has sensor capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Check if sensor exists and if it supports setting of sensor event enablement */
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
	if (rdr->RdrTypeUnion.SensorRec.EventCtrl == SAHPI_SEC_PER_EVENT ||
	    rdr->RdrTypeUnion.SensorRec.EventCtrl == SAHPI_SEC_READ_ONLY_MASKS) {
		err("BladeCenter/RSA do not support snmp_bc_set_sensor_event_enable.");    
		sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
		if (sinfo == NULL) {
			err("No sensor data. Sensor=%s", rdr->IdString.Data);
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
		
		if (sinfo->events_enabled != enable) {
			/* Probably need to drive an OID, if hardware supported it */
			sinfo->events_enabled = enable;
			/* FIXME:: Add SAHPI_ET_SENSOR_ENABLE_CHANGE event on IF event Q */
		}
	}
	else {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_READ_ONLY);
	}

	snmp_bc_unlock_handler(custom_handle);
	return(SA_OK);
}

/**
 * snmp_bc_get_sensor_event_masks:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @AssertEventMask: Location to store sensor's assert event mask.
 * @DeassertEventMask: Location to store sensor's deassert event mask.
 *
 * Retrieves a sensor's assert and deassert event masks.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 * SA_ERR_HPI_NOT_PRESENT - Sensor doesn't exist.
 **/
SaErrorT snmp_bc_get_sensor_event_masks(void *hnd,
					SaHpiResourceIdT rid,
					SaHpiSensorNumT sid,
					SaHpiEventStateT *AssertEventMask,
					SaHpiEventStateT *DeassertEventMask)
{	
	struct oh_handler_state *handle;
	struct SensorInfo *sinfo;
	struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;
        SaHpiRdrT *rdr;

	if (!hnd ) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has sensor capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}
	
	/* Check if sensor exists and return enablement status */
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_NOT_PRESENT);
	} 
	
	sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
 	if (sinfo == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       

	if (AssertEventMask) *AssertEventMask = sinfo->assert_mask;
	
	if (DeassertEventMask) {
	        if (rpt->ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS) {
		        *DeassertEventMask = sinfo->assert_mask;
	        } else {
		        *DeassertEventMask = sinfo->deassert_mask;	
	        }
	}

	snmp_bc_unlock_handler(custom_handle);
        return(SA_OK);
}

/**
 * snmp_bc_set_sensor_event_masks:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @act: Add/Remove action to perform on event masks.
 * @AssertEventMask: Sensor's assert event mask.
 * @DeassertEventMask: sensor's deassert event mask.
 *
 * Sets a sensor's assert and deassert event masks.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 * SA_ERR_HPI_INVALID_DATA - @act not valid or @AssertEventMask/@DeassertEventMask
 *                           contain events not supported by sensor. 
 * SA_ERR_HPI_NOT_PRESENT - Sensor doesn't exist.
 **/
SaErrorT snmp_bc_set_sensor_event_masks(void *hnd,
					SaHpiResourceIdT rid,
					SaHpiSensorNumT sid,
					SaHpiSensorEventMaskActionT act,
					const SaHpiEventStateT AssertEventMask,
					const SaHpiEventStateT DeassertEventMask)
{
	
	struct oh_handler_state *handle;
	struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;
        SaHpiRdrT *rdr;
	struct SensorInfo *sinfo;
	SaHpiEventStateT orig_assert_mask;
	SaHpiEventStateT orig_deassert_mask;

	if (!hnd ) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if (oh_lookup_sensoreventmaskaction(act) == NULL) {
		return(SA_ERR_HPI_INVALID_DATA);
	}

	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has sensor capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	} 
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Check if sensor exists and if it supports setting of sensor event masks */
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
	if (rdr->RdrTypeUnion.SensorRec.EventCtrl == SAHPI_SEC_PER_EVENT) {
		err("BladeCenter/RSA do not support snmp_bc_set_sensor_event_masks");
                /* Probably need to drive an OID, if hardware supported it */
		sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
		if (sinfo == NULL) {
			err("No sensor data. Sensor=%s", rdr->IdString.Data);
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}

		orig_assert_mask = sinfo->assert_mask;
		orig_deassert_mask = sinfo->deassert_mask;

		/* Check for invalid data in user masks */
		if ( (AssertEventMask != SAHPI_ALL_EVENT_STATES) &&
		     (AssertEventMask & ~(rdr->RdrTypeUnion.SensorRec.Events)) ) { 
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INVALID_DATA);
		}
		if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS)) {
			if  ( (DeassertEventMask != SAHPI_ALL_EVENT_STATES) &&
				(DeassertEventMask & ~(rdr->RdrTypeUnion.SensorRec.Events)) ) {
				snmp_bc_unlock_handler(custom_handle);
				return(SA_ERR_HPI_INVALID_DATA);
			}
		}

		/* Add to event masks */
		if (act == SAHPI_SENS_ADD_EVENTS_TO_MASKS) {
			if (AssertEventMask == SAHPI_ALL_EVENT_STATES) {
				sinfo->assert_mask = rdr->RdrTypeUnion.SensorRec.Events;
			}
			else {
				sinfo->assert_mask = sinfo->assert_mask | AssertEventMask;
			}
			if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS)) {
				if (DeassertEventMask == SAHPI_ALL_EVENT_STATES) {
					sinfo->deassert_mask = rdr->RdrTypeUnion.SensorRec.Events;
				}
				else {
					sinfo->deassert_mask = sinfo->deassert_mask | DeassertEventMask;
				}
			}
		}
		else { /* Remove from event masks */
			if (AssertEventMask == SAHPI_ALL_EVENT_STATES) {
				sinfo->assert_mask = 0;
			}
			else {
				sinfo->assert_mask = sinfo->assert_mask & ~AssertEventMask;
			}
			if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS)) {
				if (DeassertEventMask == SAHPI_ALL_EVENT_STATES) {
					sinfo->deassert_mask = 0;
				}
				else {
					sinfo->deassert_mask = sinfo->deassert_mask & ~DeassertEventMask;
				}
			}
		}
		
		/* Generate event, if needed */
		if (sinfo->assert_mask != orig_assert_mask) {
			/* FIXME:: Add SAHPI_ET_SENSOR_ENABLE_CHANGE event on IF event Q */
		}
		else {
			if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS) &&
			    sinfo->deassert_mask != orig_deassert_mask) {
				/* FIXME:: Add SAHPI_ET_SENSOR_ENABLE_CHANGE event on IF event Q */
			}
		}
	}
	else {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_READ_ONLY);
	}

	snmp_bc_unlock_handler(custom_handle);
	return(SA_OK);
}


/**
 * snmp_bc_set_slot_state_sensor:
 * @hnd: Handler data pointer.
 * @e: Pointer to struct oh_event of the resource
 * @slot_ep: Pointer to Slot Entity Path of the resource.
 *
 * Sets Slot State Sensor values. 
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 **/
SaErrorT snmp_bc_set_slot_state_sensor(void *hnd, 
					struct oh_event *e, 
					SaHpiEntityPathT *slot_ep)
{

	SaErrorT err;
	SaHpiRptEntryT  *res;
	SaHpiRdrT *rdr;	
	struct SensorInfo *sinfo;
	struct oh_handler_state *handle;
	struct snmp_bc_hnd *custom_handle;

	
	if (!e || !hnd || !slot_ep ) 
		return(SA_ERR_HPI_INVALID_PARAMS);
		
	handle = (struct oh_handler_state *) hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
					
	res = oh_get_resource_by_ep(handle->rptcache, slot_ep);
	
        if (!res) {
		err("No valid resource or rdr at hand. Could not process new rdr.");
                return(SA_ERR_HPI_INVALID_DATA);
	}

	rdr = oh_get_rdr_next(handle->rptcache, res->ResourceId, SAHPI_FIRST_ENTRY);
		
	while (rdr) {
		if ((rdr->RdrType == SAHPI_SENSOR_RDR) && 
		               (rdr->RdrTypeUnion.SensorRec.Num == BLADECENTER_SENSOR_NUM_SLOT_STATE))
		{
			sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, 
								res->ResourceId, rdr->RecordId);
			sinfo->cur_state = SAHPI_ES_PRESENT;
			sinfo->cur_child_rid = 	e->resource.ResourceId;
			
			err = oh_add_rdr(handle->rptcache,
					 res->ResourceId,
					 rdr,
				 	 sinfo, 0);				
		
			break;
		}
		rdr = oh_get_rdr_next(handle->rptcache, res->ResourceId, rdr->RecordId);
	}

			
	return(SA_OK);
}


/**
 * snmp_bc_reset_slot_state_sensor:
 * @hnd: Handler data pointer.
 * @slot_ep: Pointer to Slot Entity Path of the resource.
 *
 * REsets Slot State Sensor values. 
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 **/
SaErrorT snmp_bc_reset_slot_state_sensor(void *hnd, SaHpiEntityPathT *slot_ep)
{

	SaErrorT err;
	SaHpiRptEntryT  *res;
	SaHpiRdrT *rdr;	
	struct SensorInfo *sinfo;
	struct oh_handler_state *handle;
	struct snmp_bc_hnd *custom_handle;

	
	if (!hnd || !slot_ep ) 
		return(SA_ERR_HPI_INVALID_PARAMS);
		
	handle = (struct oh_handler_state *) hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
					
	res = oh_get_resource_by_ep(handle->rptcache, slot_ep);

        if (!res) {
		err("No valid resource or rdr at hand. Could not process new rdr.");
                return(SA_ERR_HPI_INVALID_DATA);
	}

	rdr = oh_get_rdr_next(handle->rptcache, res->ResourceId, SAHPI_FIRST_ENTRY);
	while (rdr) {
		if ((rdr->RdrType == SAHPI_SENSOR_RDR) && 
		               (rdr->RdrTypeUnion.SensorRec.Num == BLADECENTER_SENSOR_NUM_SLOT_STATE))
		{
			sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, res->ResourceId, rdr->RecordId);
			sinfo->cur_state = SAHPI_ES_ABSENT;
			sinfo->cur_child_rid = 	SAHPI_UNSPECIFIED_RESOURCE_ID;
			
			err = oh_add_rdr(handle->rptcache,
					 res->ResourceId,
					 rdr,
				 	 sinfo, 0);			
		
			break;
		}
		rdr = oh_get_rdr_next(handle->rptcache, res->ResourceId, rdr->RecordId);
	}

			
	return(SA_OK);
}

/**
 * snmp_bc_set_resource_slot_state_sensor:
 * @hnd: Handler data pointer.
 * @e: Pointer to struct oh_event of the resource
 * @resourcewidth: Number of physical slot this resource occupies
 *
 * Setting Slot State Sensor values for all slots occupied by this resource. 
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 **/

SaErrorT snmp_bc_set_resource_slot_state_sensor(void *hnd, struct oh_event *e, guint resourcewidth)
{
	guint i, j;
	SaErrorT err;
	SaHpiEntityPathT slot_ep;
	struct oh_handler_state *handle;
	struct snmp_bc_hnd *custom_handle;
		
	if (!e) 
		return(SA_ERR_HPI_INVALID_PARAMS);

	handle = (struct oh_handler_state *) hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;				
	err = snmp_bc_extract_slot_ep( &(e->resource.ResourceEntity), &slot_ep);

	j = slot_ep.Entry[0].EntityLocation;
	if ( (custom_handle->platform == SNMP_BC_PLATFORM_BC) || 
			(custom_handle->platform == SNMP_BC_PLATFORM_BCH))
	{
		for (i = 0; i < resourcewidth; i++) {
			oh_set_ep_location(&slot_ep,
				   slot_ep.Entry[0].EntityType, j+i);
	
			err = snmp_bc_set_slot_state_sensor(handle, e, &slot_ep);
		}
	} else if ( (custom_handle->platform == SNMP_BC_PLATFORM_BCT) || 
			(custom_handle->platform == SNMP_BC_PLATFORM_BCHT) ){
		for (i = 0; i < resourcewidth; i++) {
			oh_set_ep_location(&slot_ep,
				   slot_ep.Entry[0].EntityType, j - i);
	
			err = snmp_bc_set_slot_state_sensor(handle, e, &slot_ep);
		}
	}
	return(SA_OK);	
}

/**
 * snmp_bc_get_slot_state_sensor:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @reading: Location of sensor's reading
 *
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 **/
SaErrorT snmp_bc_get_slot_state_sensor(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       SaHpiSensorReadingT *reading)
{
	SaHpiRdrT *rdr;
	struct SensorInfo *sinfo;	
	struct oh_handler_state *handle;
	struct snmp_bc_hnd *custom_handle;
			
	if (!hnd || !reading) 
		return(SA_ERR_HPI_INVALID_PARAMS);

	handle = (struct oh_handler_state *) hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;	
		
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL) return(SA_ERR_HPI_NOT_PRESENT);
	sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
 	if (sinfo == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}
					
	/* Set SensorReading structure using data stored in rptcache  */																		
      	reading->IsSupported = rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported;
      	reading->Type = rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType;
      	reading->Value.SensorUint64 = (SaHpiUint64T) sinfo->cur_child_rid;
						
	return(SA_OK);
}				    				       

/**
 * snmp_bc_clear_resource_slot_state_sensor:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @reading: Location of sensor's reading
 *
 *
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 **/
SaErrorT snmp_bc_reset_resource_slot_state_sensor(void *hnd, SaHpiRptEntryT *res)
{
	guint i, j;
	SaErrorT err;
	guint resourcewidth;
	SaHpiEntityPathT slot_ep;

	struct oh_handler_state *handler;
	struct snmp_bc_hnd *custom_handler;
	struct ResourceInfo *res_info_ptr;
 		
	if (!hnd || !res ) return(SA_ERR_HPI_INVALID_PARAMS);

	handler = (struct oh_handler_state *) hnd;
	custom_handler = (struct snmp_bc_hnd *)handler->data;				
	err = snmp_bc_extract_slot_ep( &(res->ResourceEntity), &slot_ep);
	res_info_ptr =  (struct ResourceInfo *)oh_get_resource_data(handler->rptcache, res->ResourceId);
	
	resourcewidth = res_info_ptr->resourcewidth;
	res_info_ptr->resourcewidth = 1;
	
	j = slot_ep.Entry[0].EntityLocation;
	if ( (custom_handler->platform == SNMP_BC_PLATFORM_BC) || 
		(custom_handler->platform == SNMP_BC_PLATFORM_BCH))
	{
		for (i = 0; i < resourcewidth; i++) {
			oh_set_ep_location(&slot_ep,
				   slot_ep.Entry[0].EntityType, j+i);
	
			err = snmp_bc_reset_slot_state_sensor(handler, &slot_ep);
		}
	} else if ( (custom_handler->platform == SNMP_BC_PLATFORM_BCT) || 
			(custom_handler->platform == SNMP_BC_PLATFORM_BCHT) ) {
		for (i = 0; i < resourcewidth; i++) {
			oh_set_ep_location(&slot_ep,
				   slot_ep.Entry[0].EntityType, j - i);
	
			err = snmp_bc_reset_slot_state_sensor(handler, &slot_ep);
		}
	}	
	
	return(SA_OK);

}


#define usepowerdomain1 \
do { \
	switch(sid) { \
		case BLADECENTER_SENSOR_NUM_MAX_POWER: \
			thisOID = SNMP_BC_PD1POWERMAX; \
			break; \
		case BLADECENTER_SENSOR_NUM_ASSIGNED_POWER: \
			thisOID = SNMP_BC_PD1POWERCURRENT; \
			break; \
		case BLADECENTER_SENSOR_NUM_MIN_POWER: \
			thisOID = SNMP_BC_PD1POWERMIN; \
			break; \
		default: \
			err("Not one of the Slot Power Sensors."); \
			return(SA_ERR_HPI_INTERNAL_ERROR); \
			break; \
	} \
} while(0)


#define usepowerdomain2 \
do { \
	switch(sid) { \
		case BLADECENTER_SENSOR_NUM_MAX_POWER: \
			thisOID = SNMP_BC_PD2POWERMAX; \
			break; \
		case BLADECENTER_SENSOR_NUM_ASSIGNED_POWER: \
			thisOID = SNMP_BC_PD2POWERCURRENT; \
			break; \
		case BLADECENTER_SENSOR_NUM_MIN_POWER: \
			thisOID = SNMP_BC_PD2POWERMIN; \
			break; \
		default: \
			err("Not one of the Slot Power Sensors."); \
			return(SA_ERR_HPI_INTERNAL_ERROR); \
			break; \
	} \
} while(0)

/**
 * snmp_bc_get_slot_power_sensor:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @sid: Sensor ID.
 * @reading: Location of sensor's reading
 *
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS.
 **/
SaErrorT snmp_bc_get_slot_power_sensor(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       SaHpiSensorReadingT *reading)
{

#define totalPower 0xEE
	guint slotnum;
	guint oidIndex;
	SaErrorT err;
	char *thisOID;
	SaHpiRdrT *rdr;	
	gchar  **power_substrs;
	char oid[SNMP_BC_MAX_OID_LENGTH];
	SaHpiRptEntryT  *res;
	struct snmp_value get_value;
	struct snmp_value pm3_state, pm4_state;
	struct oh_handler_state *handle;
	struct snmp_bc_hnd *custom_handle;
 		
	if (!hnd || !reading ) 
		return(SA_ERR_HPI_INVALID_PARAMS);

	handle = (struct oh_handler_state *) hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;				

	res = oh_get_resource_by_id(handle->rptcache, rid);
	slotnum = res->ResourceEntity.Entry[0].EntityLocation;	
	
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL) return(SA_ERR_HPI_NOT_PRESENT);
	
	oidIndex = SNMP_BC_NOT_VALID;
	thisOID = NULL;
		
	switch (res->ResourceEntity.Entry[0].EntityType)
	{
		case BLADECENTER_PERIPHERAL_BAY_SLOT:
			oidIndex = 2;
			usepowerdomain1;			
			break;
			
		case BLADECENTER_SWITCH_SLOT:
			if (custom_handle->platform == SNMP_BC_PLATFORM_BCT)  
			{
				 switch (slotnum) {
				 	case 1:
						oidIndex = 9;
						break;
					case 2:
						oidIndex = 10;
						break;					
					case 3:
						oidIndex = 11;
						break;					

					case 4:
						oidIndex = 12;
						break;					
					default:
						err("Not one of the valid BC-T Swich Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				
				usepowerdomain1;

			} else if (custom_handle->platform == SNMP_BC_PLATFORM_BC) {
				 switch (slotnum) {
				 	case 1:
						oidIndex = 7;
						break;
					case 2:
						oidIndex = 8;
						break;					
					case 3:
						oidIndex = 9;
						break;					

					case 4:
						oidIndex = 10;
						break;					
					default:
						err("Not one of the valid BC Switch Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				
				usepowerdomain1;
			} else if (custom_handle->platform == SNMP_BC_PLATFORM_BCH) {
				 switch (slotnum) {
				 	case 1:
						oidIndex = 9;
						break;
					case 2:
						oidIndex = 10;
						break;					
					case 3:
						oidIndex = 11;
						break;					

					case 4:
						oidIndex = 12;
						break;					
					case 5:
						oidIndex = 11;
						break;					
					case 6:
						oidIndex = 12;
						break;					
					case 7:
						oidIndex = 13;
						break;
					case 8:
						oidIndex = 14;
						break;
					case 9:
						oidIndex = 15;
						break;
					case 10:
						oidIndex = 16;
						break;
					default:
						err("Not one of the valid BC H Switch Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				if ( (slotnum == 5) || (slotnum == 6)){
					usepowerdomain2;
				} else {
					usepowerdomain1;
				}
			} else if (custom_handle->platform == SNMP_BC_PLATFORM_BCHT) {
				 switch (slotnum) {
				 	case 1:
						oidIndex = 19;
						break;
					case 2:
						oidIndex = 20;
						break;					
					case 3:
						oidIndex = 21;
						break;					
					case 4:
						oidIndex = 22;
						break;
					case 5:
					case 6:
						/* Reading is not supported on BC-HT*/
						oidIndex = SNMP_BC_NOT_VALID;
						thisOID = NULL;
						break;										
					case 7:
						oidIndex = 9;
						break;
					case 8:
						oidIndex = 10;
						break;
					case 9:
						oidIndex = 11;
						break;
					case 10:
						oidIndex = 12;
						break;
					default:
						err("Not one of the valid BC HT Switch Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				if (slotnum > 6){
					usepowerdomain2;
				} else if (slotnum < 5) {
					usepowerdomain1;
				}														
			} else { 
				err("Not one of the supported platform.");
				return(SA_ERR_HPI_INTERNAL_ERROR);		
			}
			break;

		case BLADECENTER_SYS_MGMNT_MODULE_SLOT:
			if (custom_handle->platform == SNMP_BC_PLATFORM_BCT) {
				 switch (slotnum) {
				 	case 1:
						oidIndex = 7;
						break;
					case 2:
						oidIndex = 8;
						break;					
					default:
						err("Not one of the valid BC-T MM Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				
				usepowerdomain1;
				
			} else if (custom_handle->platform == SNMP_BC_PLATFORM_BC) {
				 switch (slotnum) {
				 	case 1:
						oidIndex = 5;
						break;
					case 2:
						oidIndex = 6;
						break;					
					default:
						err("Not one of the valid BC MM Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				
				usepowerdomain1;
				
			} else if (custom_handle->platform == SNMP_BC_PLATFORM_BCH) {
				 switch (slotnum) {
				 	case 1:
						oidIndex = 7;
						break;
					case 2:
						oidIndex = 8;
						break;					
					default:
						err("Not one of the valid BC H MM Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				
				usepowerdomain1;
			} else if (custom_handle->platform == SNMP_BC_PLATFORM_BCHT) {
				 switch (slotnum) {
				 	case 1:
						oidIndex = 17;
						break;
					case 2:
						oidIndex = 18;
						break;					
					default:
						err("Not one of the valid BC HT MM Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				
				usepowerdomain1;
			
			} else { 
				err("Not one of the supported platform.");
				return(SA_ERR_HPI_INTERNAL_ERROR);		
			}
			break;
		
		case BLADECENTER_BLOWER_SLOT:
			if (custom_handle->platform == SNMP_BC_PLATFORM_BCT) {
			
				if ( (slotnum == 3) || (slotnum == 4)) {
					snprintf(oid, SNMP_BC_MAX_OID_LENGTH, "%s.%d", SNMP_BC_PMSTATE, 3);
					get_integer_object(oid, pm3_state);
					// getsnmpvalue(pm3_state);
					snprintf(oid, SNMP_BC_MAX_OID_LENGTH, "%s.%d", SNMP_BC_PMSTATE, 4);
					get_integer_object(oid, pm4_state);
					// getsnmpvalue(pm4_state);
				}
				
				switch (slotnum) {
				 	case 1:
						oidIndex = 3;
						break;
					case 2:
						oidIndex = 4;
						break;					
					case 3:
						if ((pm3_state.integer == 3) && 
								(pm4_state.integer == 3))
							oidIndex = 5;
						else 	
							oidIndex = 1;
						break;					

					case 4:
						if ((pm3_state.integer == 3) && 
								(pm4_state.integer == 3))
							oidIndex = 6;
						else 
							oidIndex = 2;
						break;					
					default:
						err("Not one of the valid BC-T Fan Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				
				if (slotnum < 3) {
					usepowerdomain1;
				} else {
					if ((pm3_state.integer == 3) && 
							(pm4_state.integer == 3))
					     usepowerdomain1;
					else usepowerdomain2;
				}			
				
			} else if (custom_handle->platform == SNMP_BC_PLATFORM_BC) {
				 switch (slotnum) {
				 	case 1:
						oidIndex = 3;
						break;
					case 2:
						oidIndex = 4;
						break;							
					default:
						err("Not one of the valid BC Fan Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				
				usepowerdomain1;
				
			} else if (custom_handle->platform == SNMP_BC_PLATFORM_BCH) {
			
				/* Reading is not supported on BC-H */
				oidIndex = SNMP_BC_NOT_VALID;
				thisOID = NULL;
			} else if (custom_handle->platform == SNMP_BC_PLATFORM_BCHT) {
				/* pdp - FIX ME - Reading is not supported on BC-H */
				oidIndex = SNMP_BC_NOT_VALID;
				thisOID = NULL;			
			} else { 
				err("Not one of the supported platform.");
				return(SA_ERR_HPI_INTERNAL_ERROR);		
			}
			break;		
		case SAHPI_ENT_PHYSICAL_SLOT:
			if (custom_handle->platform == SNMP_BC_PLATFORM_BCT) {
				 switch (slotnum) {
				 	case 1:
						oidIndex = 13;
						break;
					case 2:
						oidIndex = 14;
						break;					
					case 3:
						oidIndex = 15;
						break;					

					case 4:
						oidIndex = 16;
						break;					
					case 5:
						oidIndex = 3;
						break;					
					case 6:
						oidIndex = 4;
						break;					
					case 7:
						oidIndex = 5;
						break;
					case 8:
						oidIndex = 6;
						break;
					default:
						err("Not one of the valid BC-T Blade Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				
				if (slotnum < 5) {
					usepowerdomain1;
				} else {
					usepowerdomain2;
				}			
				
			} else if (custom_handle->platform == SNMP_BC_PLATFORM_BC) {
				 switch (slotnum) {
				 	case 1:
						oidIndex = 11;
						break;
					case 2:
						oidIndex = 12;
						break;					
					case 3:
						oidIndex = 13;
						break;					

					case 4:
						oidIndex = 14;
						break;					
					case 5:
						oidIndex = 15;
						break;					
					case 6:
						oidIndex = 16;
						break;					
					case 7:
						oidIndex = 1;
						break;
					case 8:
						oidIndex = 2;
						break;
					case 9:
						oidIndex = 3;
						break;
					case 10:
						oidIndex = 4;
						break;
					case 11:
						oidIndex = 5;
						break;
					case 12:
						oidIndex = 6;
						break;
					case 13:
						oidIndex = 7;
						break;
					case 14:
						oidIndex = 8;
						break;
					default:
						err("Not one of the valid BC Blade Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				
				
				if (slotnum < 7) {
					usepowerdomain1;
				} else {
					usepowerdomain2;
				}			
			} else if (custom_handle->platform == SNMP_BC_PLATFORM_BCH) {
				 switch (slotnum) {
				 	case 1:
					case 8:
						oidIndex = 17;
						break;
					case 2:
					case 9:
						oidIndex = 18;
						break;					
					case 3:
					case 10:
						oidIndex = 19;
						break;					

					case 4:
					case 11:
						oidIndex = 20;
						break;					
					case 5:
					case 12:
						oidIndex = 21;
						break;					
					case 6:
					case 13:
						oidIndex = 22;
						break;					
					case 7:
					case 14:
						oidIndex = 23;
						break;
					default:
						err("Not one of the valid BC H Switch Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				if ( slotnum > 7){
				
					usepowerdomain2;
				} else {
					usepowerdomain1;
				}
			} else if (custom_handle->platform == SNMP_BC_PLATFORM_BCHT) {
				 switch (slotnum) {
				 	case 1:
						oidIndex = 23;
						break;
					case 2:
						oidIndex = 24;
						break;					
					case 3:
						oidIndex = 25;
						break;					

					case 4:
						oidIndex = 26;
						break;					
					case 5:
						oidIndex = 27;
						break;					
					case 6:
						oidIndex = 28;
						break;					
					case 7:
						oidIndex = 13;
						break;
					case 8:
						oidIndex = 14;
						break;
					case 9:
						oidIndex = 15;
						break;
					case 10:
						oidIndex = 16;
						break;
					case 11:
						oidIndex = 17;
						break;
					case 12:
						oidIndex = 18;
						break;												
					default:
						err("Not one of the valid BC H Switch Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				if ( slotnum > 6){		
					usepowerdomain2;
				} else {
					usepowerdomain1;
				}			
			} else { 
				err("Not one of the supported platform.");
				return(SA_ERR_HPI_INTERNAL_ERROR);		
			}
			break;		
		case BLADECENTER_POWER_SUPPLY_SLOT:
			if ((custom_handle->platform == SNMP_BC_PLATFORM_BCT) || 
					 (custom_handle->platform == SNMP_BC_PLATFORM_BC) ||
					 (custom_handle->platform == SNMP_BC_PLATFORM_BCHT)) {
				/* Reading is not supported on BC, BC-T and BC-HT*/
				oidIndex = SNMP_BC_NOT_VALID;
				thisOID = NULL;
							
			} else if (custom_handle->platform == SNMP_BC_PLATFORM_BCH) {
				 switch (slotnum) {
				 	case 1:
						oidIndex = 3;
						break;
					case 2:
						oidIndex = 4;
						break;					
					case 3:
						oidIndex = 5;
						break;					

					case 4:
						oidIndex = 6;
						break;					
					default:
						err("Not one of the valid BC H Power Module Slots.");
						return(SA_ERR_HPI_INTERNAL_ERROR);
						break;
				}
				usepowerdomain1;
			} else { 
				err("Not one of the supported platform.");
				return(SA_ERR_HPI_INTERNAL_ERROR);		
			}
			break;
		case SAHPI_ENT_SYS_MGMNT_MODULE:
			/* Assign Midplane Power reading to Virtual management module */
			switch (slotnum) {
				case 0:
					oidIndex = 1;
					break;
				default:
					err("Not one of the valid resources.");
					return(SA_ERR_HPI_INTERNAL_ERROR);
					break;
				}
				usepowerdomain1;		
			break;
		case SAHPI_ENT_SYSTEM_CHASSIS:
			usepowerdomain1;
			oidIndex = totalPower;
			break;
		default:
			thisOID = NULL;
			oidIndex = SNMP_BC_NOT_VALID;
			break;
		
	}
 
	if ( (thisOID == NULL) || (oidIndex == SNMP_BC_NOT_VALID)) {
		reading->IsSupported = SAHPI_FALSE;
		return(SA_OK);
	} else if (oidIndex == totalPower) {
		if (custom_handle->platform == SNMP_BC_PLATFORM_BCT) {
			usepowerdomain1;
			snprintf(oid, SNMP_BC_MAX_OID_LENGTH, "%s.%d", thisOID, 17);
			get_string_object(oid, pm3_state);
		
			usepowerdomain2;
			snprintf(oid, SNMP_BC_MAX_OID_LENGTH, "%s.%d", thisOID, 7);
			get_string_object(oid, pm4_state);
			
		} else if (custom_handle->platform == SNMP_BC_PLATFORM_BC) {
			usepowerdomain1;
			snprintf(oid, SNMP_BC_MAX_OID_LENGTH, "%s.%d", thisOID, 17);
			get_string_object(oid, pm3_state);
		
			usepowerdomain2;
			snprintf(oid, SNMP_BC_MAX_OID_LENGTH, "%s.%d", thisOID, 9);
			get_string_object(oid, pm4_state);
										
		} else if (custom_handle->platform == SNMP_BC_PLATFORM_BCH) {
			usepowerdomain1;
			snprintf(oid, SNMP_BC_MAX_OID_LENGTH, "%s.%d", thisOID, 24);
			get_string_object(oid, pm3_state);
		
			usepowerdomain2;
			snprintf(oid, SNMP_BC_MAX_OID_LENGTH, "%s.%d", thisOID, 24);
			get_string_object(oid, pm4_state);
		} else if (custom_handle->platform == SNMP_BC_PLATFORM_BCHT) {
			usepowerdomain1;
			snprintf(oid, SNMP_BC_MAX_OID_LENGTH, "%s.%d", thisOID, 29);
			get_string_object(oid, pm3_state);
		
			usepowerdomain2;
			snprintf(oid, SNMP_BC_MAX_OID_LENGTH, "%s.%d", thisOID, 19);
			get_string_object(oid, pm4_state);		
		} else { 
			err("Not one of the supported platform.\n");
			return(SA_ERR_HPI_INTERNAL_ERROR);		
		}
		power_substrs = g_strsplit(pm3_state.string, " ", -1);
		if (power_substrs[0] == NULL)
			return(SA_ERR_HPI_INTERNAL_ERROR);
			
		reading->Value.SensorUint64 = g_strtod(power_substrs[0], NULL);
				
		power_substrs = g_strsplit(pm4_state.string, " ", -1);
		if (power_substrs[0] == NULL)
			return(SA_ERR_HPI_INTERNAL_ERROR);
			
		reading->Value.SensorUint64 = reading->Value.SensorUint64 + g_strtod(power_substrs[0], NULL);
		
		/* Set SensorReading structure  */
      		reading->IsSupported = rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported;
      		reading->Type = rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType;
		return(SA_OK);		
	}
	
	snprintf(oid, SNMP_BC_MAX_OID_LENGTH, "%s.%d", thisOID, oidIndex);
	get_string_object(oid, get_value);			 			 

	/* Set SensorReading structure  */	
	if (g_ascii_strncasecmp(get_value.string, "N/A", sizeof("N/A")) == 0) {
      		reading->Value.SensorUint64 = 0;
	} else {
		power_substrs = g_strsplit(get_value.string, " ", -1);
		if (power_substrs[0] == NULL)
			return(SA_ERR_HPI_INTERNAL_ERROR);
			
		reading->Value.SensorUint64 = g_strtod(power_substrs[0], NULL);
	}
	
      	reading->IsSupported = rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported;
      	reading->Type = rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType;
	
	return(SA_OK);
}				    				       

/** 
 *
 * Intrastructure to Plugin APIs
 *
 **/
void * oh_get_sensor_reading (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiSensorReadingT *,
                             SaHpiEventStateT    *)
                __attribute__ ((weak, alias("snmp_bc_get_sensor_reading")));

void * oh_get_sensor_thresholds (void *, SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 SaHpiSensorThresholdsT *)
                __attribute__ ((weak, alias("snmp_bc_get_sensor_thresholds")));

void * oh_set_sensor_thresholds (void *, SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 const SaHpiSensorThresholdsT *)
                __attribute__ ((weak, alias("snmp_bc_set_sensor_thresholds")));

void * oh_get_sensor_enable (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiBoolT *)
                __attribute__ ((weak, alias("snmp_bc_get_sensor_enable")));

void * oh_set_sensor_enable (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiBoolT)
                __attribute__ ((weak, alias("snmp_bc_set_sensor_enable")));

void * oh_get_sensor_event_enables (void *, SaHpiResourceIdT,
                                    SaHpiSensorNumT,
                                    SaHpiBoolT *)
                __attribute__ ((weak, alias("snmp_bc_get_sensor_event_enable")));

void * oh_set_sensor_event_enables (void *, SaHpiResourceIdT id, SaHpiSensorNumT,
                                    SaHpiBoolT *)
                __attribute__ ((weak, alias("snmp_bc_set_sensor_event_enable")));

void * oh_get_sensor_event_masks (void *, SaHpiResourceIdT, SaHpiSensorNumT,
                                  SaHpiEventStateT *, SaHpiEventStateT *)
                __attribute__ ((weak, alias("snmp_bc_get_sensor_event_masks")));

void * oh_set_sensor_event_masks (void *, SaHpiResourceIdT, SaHpiSensorNumT,
                                  SaHpiSensorEventMaskActionT,
                                  SaHpiEventStateT,
                                  SaHpiEventStateT)
                __attribute__ ((weak, alias("snmp_bc_set_sensor_event_masks")));

