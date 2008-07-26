/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *        Daniel de Araujo <ddearauj@us.ibm.com>
 *        Renier Morales <renier@openhpi.org>
 *
 */
 

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <rtas_sensor.h>
#include <rtas_utils.h>


/**
 * rtas_get_sensor_reading:
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
 * SA_ERR_HPI_CAPABILITY      - Resource doesn't have SAHPI_CAPABILITY_SENSOR.
 * SA_ERR_HPI_INVALID_REQUEST - Sensor is disabled.
 * SA_ERR_HPI_NOT_PRESENT     - Sensor doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS  - Pointer parameter(s) are NULL.
 **/
 
SaErrorT rtas_get_sensor_reading(void                *handler,
				 SaHpiResourceIdT     resourceid,
				 SaHpiSensorNumT      sensornum,
				 SaHpiSensorReadingT *reading,
				 SaHpiEventStateT    *e_state)
{
	SaHpiSensorReadingT sensor_reading;
	SaHpiEventStateT sensor_state;
	struct SensorInfo *sensor_info;
	SaHpiInt32T state = 0, val;
	char err_buf[150];
	size_t sizebuf = sizeof(err_buf);
	
	if (!handler) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

        struct oh_handler_state *handler_state = (struct oh_handler_state *)handler;

	//do some locking here......
	
	/* Check if resource exists and has sensor capabilities */
	SaHpiRptEntryT *rpt = oh_get_resource_by_id(handler_state->rptcache, resourceid);
        
	if (!rpt) {	
		//unlock it
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
		//unlock it
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Check if sensor exists and is enabled */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handler_state->rptcache, resourceid, SAHPI_SENSOR_RDR, sensornum);
	
	if (rdr == NULL) {
		//unlock it
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
	sensor_info = (struct SensorInfo *)oh_get_rdr_data(handler_state->rptcache, resourceid, rdr->RecordId);
 	
	if (sensor_info == NULL) {
		//unlock it
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       
	
	memset(&sensor_reading, 0, sizeof(SaHpiSensorReadingT));
	sensor_state = SAHPI_ES_UNSPECIFIED;

	dbg("Sensor Reading: Resource=%s; RDR=%s", rpt->ResourceTag.Data, rdr->IdString.Data);
	
	/************************************************************
	 * Get sensor's reading.
         * Need to go through this logic, since user may request just
         * the event state for a readable sensor. Need to translate
         * sensor reading to event in this case.
         ************************************************************/
	if (rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported == SAHPI_TRUE) {
		
		state = rtas_get_sensor(sensor_info->token, sensor_info->index, &val);
		
		if (state < 0) {
			
			decode_rtas_error(state, err_buf, sizebuf, sensor_info->token, sensor_info->index);
					
			err("Cannot determine sensor's reading. Error=%s", err_buf);
			//unlock it
			
			/* NEED TO MAP RTAS ERRORS TO HPI ERRORS */
			return(state);
		}
		
		else {
			
			/* if the return code is 990x. we must sleep for 10^x milliseconds before
			 * trying to query the rtas sensor again
			 */
			if ((state & TOKEN_MASK) == 9900) {
				sleep((exp10(state & TIME_MASK)));
				
				state = rtas_get_sensor(sensor_info->token, sensor_info->index, &val);
		
				if (state < 0) {
					decode_rtas_error(state, err_buf, sizebuf, sensor_info->token, sensor_info->index);
					
					err("Cannot determine sensor's reading. Error=%s", err_buf);
					//unlock it
					/* NEED TO MAP RTAS ERRORS TO HPI ERRORS */
					return(state);
				}
			}
				
			sensor_reading.IsSupported = SAHPI_TRUE;
			sensor_reading.Type = rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType;
				
			switch (sensor_reading.Type) {
					
				case SAHPI_SENSOR_READING_TYPE_INT64:
				
					sensor_reading.Value.SensorInt64 = val;
					break;
				
				case SAHPI_SENSOR_READING_TYPE_UINT64:
					
					sensor_reading.Value.SensorUint64 = (SaHpiUint32T)val;
					break;
			
				case SAHPI_SENSOR_READING_TYPE_FLOAT64:
						
					sensor_reading.Value.SensorFloat64 = (SaHpiFloat64T)val;
					break;
				
				case SAHPI_SENSOR_READING_TYPE_BUFFER:
					
					memcpy(sensor_reading.Value.SensorBuffer, &val, sizeof(val)); 
					break;
						
				default:
					
					break;	
			}
				
				sensor_info->current_val = val;
						
		}					
		
	}
	else {
		sensor_reading.IsSupported = SAHPI_FALSE;
	}

	/******************************************************************
	 * We already grabbed the the sensor's state in rtas_get_sensor. 
	 * We now need to translate it into an event state and write it
	 * to the stored location.
	 ******************************************************************/		
	
	/* If we couldn't get read the data, just assign the state to the one found during discovery */
	if (sensor_reading.IsSupported == SAHPI_FALSE) {
		*e_state = sensor_info->current_state;
	}
	else {
		switch ((rtasSensorState)state) {
			case SENSOR_OK: 
				sensor_state = SAHPI_ES_UNSPECIFIED; 
				sensor_info->current_state = SAHPI_ES_UNSPECIFIED; 
				break; 
				
			case SENSOR_CRITICAL_LOW:
				sensor_state = SAHPI_ES_LOWER_CRIT; 
				sensor_info->current_state = SAHPI_ES_LOWER_CRIT;
				break; 	
			
			case SENSOR_WARNING_LOW:
				sensor_state = SAHPI_ES_LOWER_MINOR; 
				sensor_info->current_state = SAHPI_ES_LOWER_MINOR;
				break; 
				
			case SENSOR_NORMAL:
				sensor_state = SAHPI_ES_OK;
				sensor_info->current_state = SAHPI_ES_OK; 
				break; 
				
			case SENSOR_WARNING_HIGH:
				sensor_state = SAHPI_ES_UPPER_MINOR; 
				sensor_info->current_state = SAHPI_ES_UPPER_MINOR; 
				break; 
				
			case SENSOR_CRITICAL_HIGH:	
				sensor_state = SAHPI_ES_UPPER_CRIT; 
				sensor_info->current_state = SAHPI_ES_UPPER_CRIT;
				break; 
				
			default:
				sensor_state = SAHPI_ES_UNSPECIFIED; 
				sensor_info->current_state = SAHPI_ES_UNSPECIFIED; 
				break; 
		}		
		
	}
	
	
	/* sinfo->cur_state = working_state; */
	if (reading) memcpy(reading, &sensor_reading, sizeof(SaHpiSensorReadingT));
	if (e_state) memcpy(e_state, &sensor_state, sizeof(SaHpiEventStateT));

	//unlock it
        
	return(SA_OK);
}

/**
 * snmp_bc_get_sensor_thresholds:
 * @handler: Handler data pointer.
 * @resourceid: Resource ID.
 * @sensornum: Sensor Number.
 * @threshold: Location to store sensor's threshold values.
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
 
SaErrorT rtas_get_sensor_thresholds(void *handler,
				    SaHpiResourceIdT resourceid,
				    SaHpiSensorNumT  sensornum,
				    SaHpiSensorThresholdsT *thresholds)
{
        
	struct SensorInfo *sinfo;
	struct oh_handler_state *handler_state = (struct oh_handler_state *)handler;
	
	if (!handler || !thresholds) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	//do locking stuff here
	/* Check if resource exists and has sensor capabilities */
	SaHpiRptEntryT *rpt = oh_get_resource_by_id(handler_state->rptcache, resourceid);
        
	if (!rpt) {
		//unlock it
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {	
		//unlock it
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Check if sensor exits and has readable thresholds */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handler_state->rptcache, resourceid, SAHPI_SENSOR_RDR, sensornum);
	
	if (rdr == NULL){
		//unlock it
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
        sinfo = (struct SensorInfo *)oh_get_rdr_data(handler_state->rptcache, resourceid, rdr->RecordId);
 	if (sinfo == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		//unlock it
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}
	
	/* Since the RTAS library doesn't support reading of thresholds, return the 
	 * appropriate return code 
	 */
	 
	//unlock it
	return SA_ERR_HPI_INVALID_CMD;
					    
}				    


/**
 * snmp_bc_set_sensor_thresholds:
 * @handler: Handler data pointer.
 * @resourceid: Resource ID.
 * @sensornum: Sensor ID.
 * @thresholds: Location of sensor's settable threshold values.
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
 
SaErrorT rtas_set_sensor_thresholds(void *handler,
				       SaHpiResourceIdT resourceid,
				       SaHpiSensorNumT sensornum,
				       const SaHpiSensorThresholdsT *thresholds)
{
	struct oh_handler_state *handler_state = (struct oh_handler_state *)handler;
	struct SensorInfo *sinfo;

	if (!handler || !thresholds) {
		err("Invalid parameter");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	//do locking stuff here
	/* Check if resource exists and has sensor capabilities */
	SaHpiRptEntryT *rpt = oh_get_resource_by_id(handler_state->rptcache, resourceid);
        if (!rpt) {
		//unlock it
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)) {
		//unlock it
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Check if sensor exists and has writable thresholds */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handler_state->rptcache, resourceid, SAHPI_SENSOR_RDR, sensornum);
	
	if (rdr == NULL) {
		//unlock it
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
	sinfo = (struct SensorInfo *)oh_get_rdr_data(handler_state->rptcache, resourceid, rdr->RecordId);
 	
	if (sinfo == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		//unlock it
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       

	/* Since the RTAS library doesn't support setting of thresholds, return the 
	 * appropriate return code 
	 */
	
	return(SA_ERR_HPI_INVALID_CMD);
	
}



SaErrorT rtas_get_sensor_enable(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiSensorNumT num,
                                   SaHpiBoolT *enable)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_set_sensor_enable(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiSensorNumT num,
                                   SaHpiBoolT enable)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_get_sensor_event_enables(void *hnd,
                                          SaHpiResourceIdT id,
                                          SaHpiSensorNumT num,
                                          SaHpiBoolT *enables)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_set_sensor_event_enables(void *hnd,
                                          SaHpiResourceIdT id,
                                          SaHpiSensorNumT num,
                                          const SaHpiBoolT enables)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_get_sensor_event_masks(void *hnd,
                                        SaHpiResourceIdT id,
                                        SaHpiSensorNumT  num,
                                        SaHpiEventStateT *AssertEventMask,
                                        SaHpiEventStateT *DeassertEventMask)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_set_sensor_event_masks(void *hnd,
                                        SaHpiResourceIdT id,
                                        SaHpiSensorNumT num,
                                        SaHpiSensorEventMaskActionT act,
                                        SaHpiEventStateT AssertEventMask,
                                        SaHpiEventStateT DeassertEventMask)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_get_sensor_event_enabled(void *hnd, 
                                          SaHpiResourceIdT id,
                                          SaHpiSensorNumT sensornum,
                                          SaHpiBoolT *enable)
{
	return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_set_sensor_event_enabled(void *hnd, 
                                          SaHpiResourceIdT id,
                                          SaHpiSensorNumT sensornum,
                                          SaHpiBoolT *enable)
{
	return SA_ERR_HPI_INTERNAL_ERROR;
}


/**
 * rtas_get_sensor_location_code
 * 
 * @token  - the sensor type.
 * @index  - index of the sensor.
 * @buffer - buffer to store the location code of the sensor.
 *
 * @return - TBD
 */

int rtas_get_sensor_location_code(int token, int index, char *buffer)
{
	int filehandle, i, len;
	char filename[45], temp[4096], *pos;

	if (buffer == NULL)
		return 0;

	buffer[0] = '\0';
	snprintf(filename, MAX_SENSOR_LOCATION_STRING_SIZE, 
	                          "%s%04d", RTAS_SENSOR_LOCATION, token);

	filehandle = open(filename, O_RDONLY);
	if (filehandle < 0)
		return 0;

	if ((len = read(filehandle, temp, 4096)) < 0)
		return 0;

	pos = temp;

	for (i=0; i<index; i++) {
		pos += strlen(pos)+1;
		if (pos >= temp + len) {
			close(filehandle);
			return 0;
		}
	}

	strncpy(buffer, pos, MAX_SENSOR_LOCATION_STRING_SIZE);
	close(filehandle);

	return 1;
}

void * oh_get_sensor_reading (void *, 
			      SaHpiResourceIdT,
                              SaHpiSensorNumT,
                              SaHpiSensorReadingT *,
                              SaHpiEventStateT    *)
                __attribute__ ((weak, alias("rtas_get_sensor_reading")));
		
void * oh_get_sensor_thresholds (void *, 
                                 SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 SaHpiSensorThresholdsT *)
                __attribute__ ((weak, alias("rtas_get_sensor_thresholds")));		

void * oh_set_sensor_thresholds (void *, 
				 SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 const SaHpiSensorThresholdsT *)
                __attribute__ ((weak, alias("rtas_set_sensor_thresholds")));

void * oh_get_sensor_enable (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiBoolT *)
        __attribute__ ((weak, alias("rtas_get_sensor_enable")));
void * oh_set_sensor_enable (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiBoolT)
        __attribute__ ((weak, alias("rtas_set_sensor_enable")));
void * oh_get_sensor_event_enables (void *, SaHpiResourceIdT,
                                    SaHpiSensorNumT,
                                    SaHpiBoolT *)
        __attribute__ ((weak, alias("rtas_get_sensor_event_enabled")));
void * oh_set_sensor_event_enables (void *, SaHpiResourceIdT id, SaHpiSensorNumT,
                                    SaHpiBoolT *)
        __attribute__ ((weak, alias("rtas_set_sensor_event_enabled")));
void * oh_get_sensor_event_masks (void *, SaHpiResourceIdT, SaHpiSensorNumT,
                                  SaHpiEventStateT *, SaHpiEventStateT *)
        __attribute__ ((weak, alias("rtas_get_sensor_event_masks")));
void * oh_set_sensor_event_masks (void *, SaHpiResourceIdT, SaHpiSensorNumT,
                                  SaHpiSensorEventMaskActionT,
                                  SaHpiEventStateT,
                                  SaHpiEventStateT)
        __attribute__ ((weak, alias("rtas_set_sensor_event_masks")));
