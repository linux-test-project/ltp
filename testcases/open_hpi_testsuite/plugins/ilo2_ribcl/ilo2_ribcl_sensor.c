/*
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
 *                     All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the Hewlett-Packard Corporation, nor the names
 * of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author(s)
 *     Shuah Khan <shuah.khan@hp.com>
 *     Richard White <richard.white@hp.com>
 */

/***************
 * This source file contains Sensor HPI ABI routines iLO2 RIBCL plug-in
 * implements. Other source files provide support functionality for
 * these ABIs.
***************/
#include <ilo2_ribcl.h>
#include <ilo2_ribcl_sensor.h>

/************************************
	Forward declarations for static functions in this file
************************************/
static SaErrorT ilo2_ribcl_get_sensor_allinfo(
				struct oh_handler_state *,
				SaHpiResourceIdT ,
				SaHpiSensorNumT , 
				struct ilo2_ribcl_sens_allinfo *);
static SaErrorT ilo2_ribcl_sensor_send_event( struct oh_handler_state *, 
				struct ilo2_ribcl_sens_allinfo *,
				SaHpiEventTypeT,
				SaHpiSeverityT,
				SaHpiBoolT);
static void ilo2_ribcl_process_severitysensor( struct oh_handler_state *,
				struct ilo2_ribcl_sens_allinfo *,
				I2R_SensorDataT *);

/**
 * ilo2_ribcl_get_sensor_reading:
 * @hdn:	Pointer to the handler for this instance.
 * @rid:	Resource ID for resource containing this sensor. 
 * @s_num:	Number of this sensor in the resource.
 * @s_reading:	Pointer used to return sensor reading.
 * @s_state:	Pointer used to return sensor state.
 *
 * Description:
 * Implements the plugin specific part of the saHpiSensorReadingGet() API.
 * 
 * We make a call to ilo2_ribcl_get_sensor_allinfo() to obtain the sensor RDR,
 * the rpt entry for the resource containing the sensor, and the struct 
 * ilo2_ribcl_sensinfo that contains all our sensor data.
 *
 * Since the sensors that we support all return a 64bit integer reading,
 * this routine assumes a 64bit integer value. If we support additional
 * sensor types in the future, this routine may need to change.
 *
 * Since the communication latency with iLo2 is so large (or the order of 10
 * seconds), we don't read the actual sensor values from iLo2 in 
 * this routine. Instead, we peridically poll for the sensor values and store
 * them in the struct ilo2_ribcl_sensinfo structures that are associated with
 * the sensor RDRs (done in ilo2_ribcl_process_sensors()). This routine returns
 * those cached sensor values. Currently, the sensor values are read during a
 * discovery operation - either via a client's call to saHpiDiscover() or via
 * the daemon's periodic discovery done in oh_discovery_thread_loop().
 *
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_CAPABILITY - Resource does not support sensors.
 * SA_ERR_HPI_INVALID_REQUEST - Sensor is curerntly disabled.
 * SA_ERR_HPI_NOT_PRESENT - The requested sensor is not present.
 * SA_ERR_HPI_INVALID_RESOURCE - The resource does not exist.
 * SA_ERR_HPI_INTERNAL_ERROR - There is no private data for this sensor. 
 **/
SaErrorT ilo2_ribcl_get_sensor_reading(void *hnd,
                                       SaHpiResourceIdT rid,
                                       SaHpiSensorNumT s_num,
                                       SaHpiSensorReadingT *s_reading,
                                       SaHpiEventStateT *s_state)
{
	SaErrorT ret;
	struct oh_handler_state *oh_handler;
	struct ilo2_ribcl_sens_allinfo sens_allinfo;
	struct ilo2_ribcl_sensinfo *sensinfo;

	if( !hnd){
		err(" ilo2_ribcl_get_sensor_reading: invalid handle.");
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	oh_handler = (struct oh_handler_state *)hnd;

	/* Look up our sensor RDR, and it's associated data */
	ret = ilo2_ribcl_get_sensor_allinfo( oh_handler, rid, s_num,
					      &sens_allinfo); 

	if( ret != SA_OK){
		return( ret);
	}

	/* If this sensor is currently disabled, return an error */

	sensinfo = sens_allinfo.sens_dat;
	if( !sensinfo->sens_enabled){
		return( SA_ERR_HPI_INVALID_REQUEST);
	}

	if( s_reading != NULL){
		s_reading->IsSupported = SAHPI_TRUE;
		s_reading->Type = 
		sens_allinfo.rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType;
		s_reading->Value.SensorInt64 = sensinfo->sens_value;
	}

	if( s_state != NULL){
		*s_state = sensinfo->sens_ev_state; 
	}

	return( SA_OK);

} /* end ilo2_ribcl_get_sensor_reading() */



/**
 * ilo2_ribcl_get_sensor_enable:
 * @hdn:	Pointer to the handler for this instance.
 * @rid:	Resource ID for resource containing this sensor. 
 * @s_num:	Number of this sensor in the resource.
 * @s_enable:	pointer to return sensor enable status.
 *
 * Description:
 * Implements the plugin specific part of the saHpiSensorEnableGet() API.
 * 
 * We make a call to ilo2_ribcl_get_sensor_allinfo() to obtain the sensor RDR,
 * the rpt entry for the resource containing the sensor, and the struct 
 * ilo2_ribcl_sensinfo that contains all our sensor data.
 *
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_CAPABILITY - Resource does not support sensors.
 * SA_ERR_HPI_INVALID_PARAMS - s_enable pointer or handler is NULL.
 * SA_ERR_HPI_NOT_PRESENT - The requested sensor is not present.
 * SA_ERR_HPI_INVALID_RESOURCE - The resource does not exist.
 * SA_ERR_HPI_INTERNAL_ERROR - There is no private data for this sensor. 
 **/
SaErrorT ilo2_ribcl_get_sensor_enable(void *hnd,
                                      SaHpiResourceIdT rid,
                                      SaHpiSensorNumT s_num,
                                      SaHpiBoolT *s_enable)
{
	SaErrorT ret;
	struct oh_handler_state *oh_handler;
	struct ilo2_ribcl_sens_allinfo sens_allinfo;

	if( !hnd){
		err(" ilo2_ribcl_get_sensor_enable: invalid handle.");
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	if( !s_enable){
		err(" ilo2_ribcl_get_sensor_enable: invalid enable pointer.");
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	oh_handler = (struct oh_handler_state *)hnd;

	/* Look up our sensor RDR, and it's associated data */
	ret = ilo2_ribcl_get_sensor_allinfo( oh_handler, rid, s_num,
					      &sens_allinfo); 

	if( ret != SA_OK){
		return( ret);
	}

	/* Now that we have our sensor data handy, return the enable value */

	*s_enable = sens_allinfo.sens_dat->sens_enabled;
	return( SA_OK);

} /* end ilo2_ribcl_get_sensor_enable() */



/**
 * ilo2_ribcl_set_sensor_enable:
 * @hdn:	Pointer to the handler for this instance.
 * @rid:	Resource ID for resource containing this sensor. 
 * @s_num:	Number of this sensor in the resource.
 * @s_enable:	The new sensor enable status.
 *
 * Description:
 * Implements the plugin specific part of the saHpiSensorEnableSet() API.
 * 
 * We make a call to ilo2_ribcl_get_sensor_allinfo() to obtain the sensor RDR,
 * the rpt entry for the resource containing the sensor, and the struct 
 * ilo2_ribcl_sensinfo that contains all our sensor data.
 *
 * If the new sensor enable suppiled by the user differs from the existing
 * value in our sensor data structure, we call ilo2_ribcl_sensor_send_event()
 * to send a sensor enable change event. 
 *
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_CAPABILITY - Resource does not support sensors.
 * SA_ERR_HPI_NOT_PRESENT - The requested sensor is not present.
 * SA_ERR_HPI_INVALID_RESOURCE - The resource does not exist.
 * SA_ERR_HPI_READ_ONLY	- The sensor does not support changing enable status.
 * SA_ERR_HPI_INTERNAL_ERROR - There is no private data for this sensor. 
 **/
SaErrorT ilo2_ribcl_set_sensor_enable(void *hnd,
                                      SaHpiResourceIdT rid,
                                      SaHpiSensorNumT s_num,
                                      SaHpiBoolT s_enable)
{
	SaErrorT ret = SA_OK;
	struct oh_handler_state *oh_handler;
	struct ilo2_ribcl_sens_allinfo sens_allinfo;

	if( !hnd){
		err(" ilo2_ribcl_set_sensor_enable: invalid handle.");
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	oh_handler = (struct oh_handler_state *)hnd;

	/* Look up our sensor RDR, and it's associated data */
	ret = ilo2_ribcl_get_sensor_allinfo( oh_handler, rid, s_num,
					      &sens_allinfo); 

	if( ret != SA_OK){
		return( ret);
	}

	/* Check that this sensor supports changing the enable status. */

	if( sens_allinfo.rdr->RdrTypeUnion.SensorRec.EnableCtrl != SAHPI_TRUE){
		return( SA_ERR_HPI_READ_ONLY);
	}

	/* We want to send an enable change event only if the enable
	 * value has actually been changed. */

	if( s_enable != sens_allinfo.sens_dat->sens_enabled){
		sens_allinfo.sens_dat->sens_enabled = s_enable; 
		ret = ilo2_ribcl_sensor_send_event( oh_handler, &sens_allinfo,
					SAHPI_ET_SENSOR_ENABLE_CHANGE,
					SAHPI_INFORMATIONAL, SAHPI_TRUE); 
	} 
	return( ret);

} /* end ilo2_ribcl_set_sensor_enable() */



/**
 * ilo2_ribcl_get_sensor_event_enable:
 * @hdn:	Pointer to the handler for this instance.
 * @rid:	Resource ID for resource containing this sensor. 
 * @s_num:	Number of this sensor in the resource.
 * @e_enables:	Pointer to return the sensor event enables.
 *
 * Description:
 * Implements the plugin specific part of the saHpiSensorEventEnableGet() API.
 * 
 * We make a call to ilo2_ribcl_get_sensor_allinfo() to obtain the sensor RDR,
 * the rpt entry for the resource containing the sensor, and the struct 
 * ilo2_ribcl_sensinfo that contains all our sensor data.
 *
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_CAPABILITY - Resource does not support sensors.
 * SA_ER_HPI_INVALID_PARAMS - e_enables parameter is NULL.
 * SA_ERR_HPI_NOT_PRESENT - The requested sensor is not present.
 * SA_ERR_HPI_INVALID_RESOURCE - The resource does not exist.
 * SA_ERR_HPI_INTERNAL_ERROR - There is no private data for this sensor. 
 **/
SaErrorT ilo2_ribcl_get_sensor_event_enable(void *hnd,
                                             SaHpiResourceIdT rid,
                                             SaHpiSensorNumT s_num,
                                             SaHpiBoolT *e_enable)
{
	SaErrorT ret;
	struct oh_handler_state *oh_handler;
	struct ilo2_ribcl_sens_allinfo sens_allinfo;

	if( !hnd){
		err(" ilo2_ribcl_get_sensor_event_enable: invalid handle.");
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	if( !e_enable){
		err(" ilo2_ribcl_get_sensor_event_enable: invalid enable pointer.");
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	oh_handler = (struct oh_handler_state *)hnd;

	/* Look up our sensor RDR, and it's associated data */
	ret = ilo2_ribcl_get_sensor_allinfo( oh_handler, rid, s_num,
					      &sens_allinfo); 

	if( ret != SA_OK){
		return( ret);
	}

	/* Now that we have our sensor data handy, return the event enable
	 * status value */

	*e_enable = sens_allinfo.sens_dat->sens_ev_enabled;
	return( SA_OK);

} /* end ilo2_ribcl_get_sensor_event_enable() */



/**
 * ilo2_ribcl_set_sensor_event_enable:
 * @hdn:	Pointer to the handler for this instance.
 * @rid:	Resource ID for resource containing this sensor. 
 * @s_num:	Number of this sensor in the resource.
 * @e_enables:	The new sensor event enables.
 *
 * Description:
 * Implements the plugin specific part of the saHpiSensorEventEnableSet() API.
 * 
 * We make a call to ilo2_ribcl_get_sensor_allinfo() to obtain the sensor RDR,
 * the rpt entry for the resource containing the sensor, and the struct 
 * ilo2_ribcl_sensinfo that contains all our sensor data.
 *
 * If the new event enable suppiled by the user differs from the existing
 * value in our sensor data structure, we call ilo2_ribcl_sensor_send_event()
 * to send a sensor enable change event. 
 *
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_CAPABILITY - Resource does not support sensors.
 * SA_ERR_HPI_NOT_PRESENT - The requested sensor is not present.
 * SA_ERR_HPI_INVALID_RESOURCE - The resource does not exist.
 * SA_ERR_HPI_READ_ONLY	- The sensor does not support changing enable status.
 * SA_ERR_HPI_INTERNAL_ERROR - There is no private data for this sensor. 
 * SA_ERR_HPI_OUT_OF_MEMORY - Allocation failed.
 **/
SaErrorT ilo2_ribcl_set_sensor_event_enable(void *hnd,
                                             SaHpiResourceIdT rid,
                                             SaHpiSensorNumT s_num,
                                             SaHpiBoolT e_enable)
{
	SaErrorT ret = SA_OK;
	struct oh_handler_state *oh_handler;
	struct ilo2_ribcl_sens_allinfo sens_allinfo;

	if( !hnd){
		err(" ilo2_ribcl_set_sensor_event_enable: invalid handle.");
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	oh_handler = (struct oh_handler_state *)hnd;

	/* Look up our sensor RDR, and it's associated data */
	ret = ilo2_ribcl_get_sensor_allinfo( oh_handler, rid, s_num,
					      &sens_allinfo); 

	if( ret != SA_OK){
		return( ret);
	}

	/* Check that this sensor supports changing the enable status. */

	if( sens_allinfo.rdr->RdrTypeUnion.SensorRec.EnableCtrl == 
							SAHPI_SEC_READ_ONLY){
		return( SA_ERR_HPI_READ_ONLY);
	}

	/* We want to send an enable change event only if the enable
	 * value has actually been changed. */

	if( e_enable != sens_allinfo.sens_dat->sens_ev_enabled){
		sens_allinfo.sens_dat->sens_ev_enabled = e_enable; 
		ret = ilo2_ribcl_sensor_send_event( oh_handler, &sens_allinfo,
					SAHPI_ET_SENSOR_ENABLE_CHANGE,
					SAHPI_INFORMATIONAL, SAHPI_TRUE); 
	} 

	return( ret);
} /* end ilo2_ribcl_set_sensor_event_enable() */



/**
 * ilo2_ribcl_get_sensor_event_masks:
 * @hdn:	Pointer to the handler for this instance.
 * @rid:	Resource ID for resource containing this sensor. 
 * @s_num:	Number of this sensor in the resource.
 * @s_assertmask:   Pointer to return the sensor event assert mask.
 * @s_deassertmask: Pointer to return the sensor event deassert mask.
 *
 * Description:
 * Implements the plugin specific part of the saHpiSensorEventMasksGet() API.
 * 
 * We make a call to ilo2_ribcl_get_sensor_allinfo() to obtain the sensor RDR,
 * the rpt entry for the resource containing the sensor, and the struct 
 * ilo2_ribcl_sensinfo that contains all our sensor data.
 *
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_CAPABILITY - Resource does not support sensors.
 * SA_ERR_HPI_NOT_PRESENT - The requested sensor is not present.
 * SA_ERR_HPI_INVALID_RESOURCE - The resource does not exist.
 * SA_ERR_HPI_INTERNAL_ERROR - There is no private data for this sensor. 
 **/
SaErrorT ilo2_ribcl_get_sensor_event_masks(void *hnd,
                                           SaHpiResourceIdT rid,
                                           SaHpiSensorNumT s_num,
                                           SaHpiEventStateT *s_assertmask,
                                           SaHpiEventStateT *s_deassertmask)
{
	SaErrorT ret;
	struct oh_handler_state *oh_handler;
	struct ilo2_ribcl_sens_allinfo sens_allinfo;

	if( !hnd){
		err(" ilo2_ribcl_get_sensor_event_masks: invalid handle.");
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	oh_handler = (struct oh_handler_state *)hnd;

	/* Look up our sensor RDR, and it's associated data */
	ret = ilo2_ribcl_get_sensor_allinfo( oh_handler, rid, s_num,
					      &sens_allinfo); 

	if( ret != SA_OK){
		return( ret);
	}

	if( s_assertmask != NULL){
		*s_assertmask = sens_allinfo.sens_dat->sens_assertmask;
	}

	if( s_deassertmask != NULL){
		*s_deassertmask = sens_allinfo.sens_dat->sens_deassertmask;
	}

	return( SA_OK);

} /* end ilo2_ribcl_get_sensor_event_masks() */



/**
 * ilo2_ribcl_set_sensor_event_masks:
 * @hdn:	Pointer to the handler for this instance.
 * @rid:	Resource ID for resource containing this sensor. 
 * @s_num:	Number of this sensor in the resource.
 * @act:	Specified the masking action to perform
 * @s_assertmask:   Pointer to return the sensor event assert mask.
 * @s_deassertmask: Pointer to return the sensor event deassert mask.
 *
 * Description:
 * Implements the plugin specific part of the saHpiSensorEventMasksSet() API.
 * 
 * We make a call to ilo2_ribcl_get_sensor_allinfo() to obtain the sensor RDR,
 * the rpt entry for the resource containing the sensor, and the struct 
 * ilo2_ribcl_sensinfo that contains all our sensor data.
 *
 * If the application of the new event mask(s) suppiled by the user results
 * in a change to  the mask(s) in our sensor data structure, we call
 * ilo2_ribcl_sensor_send_event() to send a sensor enable change event. 
 * 
 * Even though none of our sensors currently set SAHPI_CAPABILITY_EVT_DEASSERTS,
 * this routine will set the deassert mask equal to the assert mask if
 * SAHPI_CAPABILITY_EVT_DEASSERTS is ever set.
 * 
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_CAPABILITY - Resource does not support sensors.
 * SA_ERR_HPI_NOT_PRESENT - The requested sensor is not present.
 * SA_ERR_HPI_INVALID_RESOURCE - The resource does not exist.
 * SA_ERR_HPI_INVALID_PARAMS- The act parameter is out of range.
 * SA_ERR_HPI_INVALID_DATA - 
 * SA_ERR_HPI_READ_ONLY - The sensor does not support updating the masks.
 * SA_ERR_HPI_INTERNAL_ERROR - There is no private data for this sensor. 
 **/
SaErrorT ilo2_ribcl_set_sensor_event_masks(void *hnd,
                                           SaHpiResourceIdT rid,
                                           SaHpiSensorNumT s_num,
                                           SaHpiSensorEventMaskActionT act,
                                           SaHpiEventStateT s_assertmask,
                                           SaHpiEventStateT s_deassertmask)
{
	SaErrorT ret = SA_OK;
	struct oh_handler_state *oh_handler;
	struct ilo2_ribcl_sens_allinfo sens_allinfo;
	SaHpiSensorEventCtrlT event_ctl;
	struct ilo2_ribcl_sensinfo *sensinfo;
	SaHpiEventStateT supported_events;
	SaHpiEventStateT new_assertmask;
	SaHpiEventStateT new_deassertmask;

	if( !hnd){
		err(" ilo2_ribcl_set_sensor_event_masks: invalid handle.");
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	oh_handler = (struct oh_handler_state *)hnd;

	/* Look up our sensor RDR, and it's associated data */
	ret = ilo2_ribcl_get_sensor_allinfo( oh_handler, rid, s_num,
					      &sens_allinfo); 

	if( ret != SA_OK){
		return( ret);
	}

	sensinfo = sens_allinfo.sens_dat;
	supported_events = sens_allinfo.rdr->RdrTypeUnion.SensorRec.Events;

	/* Check if the event masks can be written */
	event_ctl = sens_allinfo.rdr->RdrTypeUnion.SensorRec.EventCtrl;
	if( (event_ctl == SAHPI_SEC_READ_ONLY_MASKS) ||
					 (event_ctl == SAHPI_SEC_READ_ONLY) ){
		return( SA_ERR_HPI_READ_ONLY);
	}

	/* If SAHPI_CAPABILITY_EVT_DEASSERTS has been set in the RPT entry,
	 * then the de-assertion mask must track the assertion mask. The
	 * SAHPI spec does not specify an error if the user provides masks
	 * that don't match. */

	if( sens_allinfo.rpt->ResourceCapabilities &
					 SAHPI_CAPABILITY_EVT_DEASSERTS){
		s_deassertmask = s_assertmask;
	}

	/* If either of the masks is SAHPI_ALL_EVENT_STATES, then set that
 	 * mask to all the event states that this sensor supports */

	supported_events = sens_allinfo.rdr->RdrTypeUnion.SensorRec.Events;
	if( s_assertmask == SAHPI_ALL_EVENT_STATES){
		s_assertmask = supported_events;
	}
	if( s_deassertmask == SAHPI_ALL_EVENT_STATES){
		s_deassertmask = supported_events;
	}

	/* Check that the masks passed as parameters don't enclude event
	 * states that this sensor does not support */

	if( act == SAHPI_SENS_ADD_EVENTS_TO_MASKS){
		if( (s_assertmask | supported_events) != supported_events){
			return( SA_ERR_HPI_INVALID_DATA);
		}

		if( (s_deassertmask | supported_events) != supported_events){
			return( SA_ERR_HPI_INVALID_DATA);
		}
	}

	if( act == SAHPI_SENS_ADD_EVENTS_TO_MASKS){
		new_assertmask = sensinfo->sens_assertmask | s_assertmask;
		new_deassertmask = sensinfo->sens_deassertmask | s_deassertmask;
	} else if(  act == SAHPI_SENS_REMOVE_EVENTS_FROM_MASKS){
		new_assertmask = sensinfo->sens_assertmask & ~s_assertmask;
		new_deassertmask = sensinfo->sens_deassertmask & ~s_deassertmask;
	} else {
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	/* We only want to send an event if either of the masks will actually
	 * change after all of this. */

	if( (sensinfo->sens_assertmask == new_assertmask) &&
	    (sensinfo->sens_deassertmask == new_deassertmask) ){
		return( SA_OK);
	}

	sensinfo->sens_assertmask = new_assertmask;
	sensinfo->sens_deassertmask = new_deassertmask;

	ret = ilo2_ribcl_sensor_send_event( oh_handler, &sens_allinfo,
				SAHPI_ET_SENSOR_ENABLE_CHANGE,
				SAHPI_INFORMATIONAL, SAHPI_TRUE); 

	return( ret);

} /* end ilo2_ribcl_set_sensor_event_masks() */



/**
 * ilo2_ribcl_get_sensor_allinfo:
 * @oh_handler:	Pointer to the handler for this instance.
 * @rid:	Resource ID for resource containing this sensor. 
 * @s_num:	Number of this sensor in the resource.
 * @sens_allinfo: Pointer to structure used to return rdr and data pointers.
 *
 * This is a support routine used within our plugin. It returns a pointer to
 * the sensor RDR, a pointer to the rpt entry for the resource containing the
 * sensor, and a pointer to the struct ilo2_ribcl_sensinfo that is associated
 * with the sensor RDR. These pointers are returned via the sens_allinfo
 * parameter, which should point to a struct ilo2_ribcl_sens_allinfo that has
 * been allocated by our caller. 
 *
 * First, we get the rpt entry with a call to oh_get_resource_by_id(). Then
 * we get the sensor RDR with a call to oh_get_rdr_by_type(). Finally, we get
 * the associated sensor data with a call to oh_get_rdr_data(). 
 *  
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_INVALID_RESOURCE - The resource does not exist.
 * SA_ERR_HPI_CAPABILITY - Resource does not support sensors.
 * SA_ERR_HPI_NOT_PRESENT - The requested sensor is not present.
 * SA_ERR_HPI_INTERNAL_ERROR - There is no private data for this sensor. 
 **/
SaErrorT ilo2_ribcl_get_sensor_allinfo( struct oh_handler_state *oh_handler,
				SaHpiResourceIdT rid,
				SaHpiSensorNumT s_num, 
				struct ilo2_ribcl_sens_allinfo *sens_allinfo)
{
	sens_allinfo->rpt = NULL;
	sens_allinfo->rdr = NULL;
	sens_allinfo->sens_dat = NULL;

	/* Check if the resource exists, and that it has sensor capability */

	sens_allinfo->rpt = oh_get_resource_by_id(oh_handler->rptcache, rid);
	if( !sens_allinfo->rpt){
		err("ilo2_ribcl_get_sensor_allinfo: no rpt entry for resource id %d.", rid);
		return( SA_ERR_HPI_INVALID_RESOURCE);
	}

	if( !(sens_allinfo->rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)){
		err("ilo2_ribcl_get_sensor_allinfo: no sensor capability for resource id %d.", rid);
		return( SA_ERR_HPI_CAPABILITY);
	}

	/* look up the RDR for this sensor */

	sens_allinfo->rdr = oh_get_rdr_by_type(oh_handler->rptcache, rid,
			SAHPI_SENSOR_RDR, s_num);

	if( sens_allinfo->rdr == NULL){
		err("ilo2_ribcl_get_sensor_allinfo: no sensor RDR for resource id %d, sennsor %d.", rid, s_num);
		return( SA_ERR_HPI_NOT_PRESENT);
	}

	/* Finally, get the assoicated private data for this sensor */

	sens_allinfo->sens_dat = (struct ilo2_ribcl_sensinfo *)oh_get_rdr_data(
			oh_handler->rptcache, rid, sens_allinfo->rdr->RecordId);

	if( sens_allinfo->sens_dat == NULL){
		err("ilo2_ribcl_get_sensor_allinfo: no private sensor data for resource id %d, sensor %d, label: %s.",
				 rid, s_num, sens_allinfo->rdr->IdString.Data);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	return( SA_OK);

} /* end ilo2_ribcl_get_sensor_allinfo( ) */



/**
 * ilo2_ribcl_sensor_send_event:
 * @oh_handler:	    Pointer to the handler for this instance.
 * @sens_allinfo:   Pointer to structure used to return rdr and data pointers.
 * @event_type:     Type of sensor event. 
 * @event_severity: Severity of event.
 * @is_assertion:   True if this is an assertion event, false if not.
 *
 * This routine is used by our plugin to send both sensor change and sensor
 * enable change events. 
 *
 * Sensor change events:
 * 	The single sensor event to be asserted or de-asserted is given by
 * 	the event_sens_ev_state element of struct ilo2_ribcl_sensinfo that
 * 	has been provided by the sens_allinfo parameter.
 *
 *	The optional data SAHPI_SOD_CURRENT_STATE and SAHPI_SOD_PREVIOUS_STATE
 *	are provided in our sensor events.
 *
 * Sensor enable change events:
 *	The optional data SAHPI_SEOD_CURRENT_STATE is provided in our sensor
 *	enable change events.
 * 
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_OUT_OF_MEMORY - Allocation failed.
 * SA_ERR_HPI_INTERNAL_ERROR - The event type was not SAHPI_ET_SENSOR or
 *		SAHPI_ET_SENSOR_ENABLE_CHANGE. 
 **/
static SaErrorT ilo2_ribcl_sensor_send_event(
				struct oh_handler_state *oh_handler, 
				struct ilo2_ribcl_sens_allinfo *sens_allinfo,
				SaHpiEventTypeT event_type,
				SaHpiSeverityT sens_severity,
				SaHpiBoolT is_assertion)
{
	struct oh_event *ev;
	SaHpiRdrT *rdr;
	struct ilo2_ribcl_sensinfo *sensinfo;
	SaHpiSensorEnableChangeEventT *sen_evch;
	SaHpiSensorEventT *sen_ev;

	if( (event_type != SAHPI_ET_SENSOR) && 
	    ( event_type != SAHPI_ET_SENSOR_ENABLE_CHANGE) ){
		err("ilo2_ribcl_sensor_send_event: invalid event type.");
		return( SA_ERR_HPI_INTERNAL_ERROR); 
	}

	rdr = sens_allinfo->rdr;
	sensinfo = sens_allinfo->sens_dat;

	ev = oh_new_event();
	if( ev == NULL){
		err("ilo2_ribcl_undiscovered_fru(): event allocation failed.");
		return( SA_ERR_HPI_OUT_OF_MEMORY);
	}

	ev->resource = *(sens_allinfo->rpt);
	ev->hid = oh_handler->hid;
	ev->event.EventType = event_type;
	ev->event.Severity = sens_severity; 
	ev->event.Source = ev->resource.ResourceId; 
	if ( oh_gettimeofday(&(ev->event.Timestamp)) != SA_OK){
		ev->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
	}
	ev->rdrs = g_slist_append(ev->rdrs,
		 g_memdup(sens_allinfo->rdr, sizeof(SaHpiRdrT)));

	/* Now, fill out the sensor specific part */

	if( event_type == SAHPI_ET_SENSOR_ENABLE_CHANGE){

		sen_evch = &(ev->event.EventDataUnion.SensorEnableChangeEvent);
		sen_evch->SensorNum = sensinfo->sens_num;
		sen_evch->SensorType = rdr->RdrTypeUnion.SensorRec.Type;
		sen_evch->EventCategory = rdr->RdrTypeUnion.SensorRec.Category;
		sen_evch->SensorEnable = sensinfo->sens_enabled; 
		sen_evch->SensorEventEnable = sensinfo->sens_ev_enabled;
		sen_evch->AssertEventMask = sensinfo->sens_assertmask;
		sen_evch->DeassertEventMask = sensinfo->sens_deassertmask;
		/* Optional data follows */
		sen_evch->OptionalDataPresent |=  SAHPI_SEOD_CURRENT_STATE;
		sen_evch->CurrentState = sensinfo->sens_ev_state;

	} else { /* Otherwise, it's a regular sensor event */

		sen_ev = &(ev->event.EventDataUnion.SensorEvent);
		sen_ev->SensorNum = sensinfo->sens_num; 
		sen_ev->SensorType = rdr->RdrTypeUnion.SensorRec.Type; 
		sen_ev->EventCategory = rdr->RdrTypeUnion.SensorRec.Category; 
		sen_ev->Assertion = is_assertion;
		sen_ev->EventState = sensinfo->event_sens_ev_state; 
		/* Optional data follows */
		sen_ev->OptionalDataPresent = SAHPI_SOD_CURRENT_STATE |
					      SAHPI_SOD_PREVIOUS_STATE;
		sen_ev->PreviousState = sensinfo->prev_sens_ev_state;
		sen_ev->CurrentState =  sensinfo->sens_ev_state;

	}

	oh_evt_queue_push( oh_handler->eventq, ev);
	return( SA_OK);

} /* end ilo2_ribcl_sensor_send_event() */



/**
 * ilo2_ribcl_process_sensors:
 * @oh_handler:	    Pointer to the handler for this instance.
 *
 * This routine is called to examine the values of the sensors retrieved
 * and stored in the handler's DiscoveryData during the last discovery
 * operation, update the sensor values in the sensor data structures
 * associated with the sensor RDRs, and send the appropriate events if
 * the sensor values have changed.
 *
 * Since the communication latency with iLo2 is so large (or the order of 10
 * seconds), we don't read the actual sensor values from iLo2 in 
 * ilo2_ribcl_get_sensor_reading(). Instead, we peridically poll for the 
 * sensor values and store them in the struct ilo2_ribcl_sensinfo structures
 * that are associated with the sensor RDRs.
 *
 * Currently, the sensor values are read during a discovery operation - either
 * via a client's call to saHpiDiscover() or via the daemon's periodic
 * discovery done in oh_discovery_thread_loop(). This routine is then called
 * at the end of ilo2_ribcl_do_discovery().
 *
 * Chassis sensors:
 * For all chassis sensor readings stored in the handler's DiscoveryData,
 * get the sensor's associated HPI structures, and call 
 * ilo2_ribcl_process_severitysensor(). 
 * 
 * Return values:
 * None
 **/
void ilo2_ribcl_process_sensors( struct oh_handler_state *oh_handler) 
{
	SaErrorT ret;
	SaHpiSensorNumT sens_num;
	ilo2_ribcl_handler_t *ir_handler = NULL;
	I2R_SensorDataT *ir_sens_dat;
	struct ilo2_ribcl_sens_allinfo sens_allinfo;

	ir_handler = (ilo2_ribcl_handler_t *) oh_handler->data;

	/* Handle the chassis sensors */
	for( sens_num = 1; sens_num < I2R_NUM_CHASSIS_SENSORS; sens_num++){
		
		ir_sens_dat =
			&(ir_handler->DiscoveryData.chassis_sensors[sens_num]);

		/* If this sensor was not found during discovery, skip it */
		if( ir_sens_dat->state == I2R_NO_EXIST){
			continue;
		}

		/* Get the rpt of the sensor resource, the RDR of the sensor,
		 * and the sensor data associated with the RDR. */

		ret = ilo2_ribcl_get_sensor_allinfo( oh_handler,
				     ir_sens_dat->rid, sens_num, &sens_allinfo);
		if( ret != SA_OK){
			err("ilo2_ribcl_process_sensors: could not locate HPI data for chassis sensor number %d.", sens_num);
			continue;
		}

		/* All of our chassis sensors are the severity type */
		ilo2_ribcl_process_severitysensor( oh_handler, &sens_allinfo,
						   ir_sens_dat);
	}

	/* Processing of any future sensors will be inserted here */
	
} /* end ilo2_ribcl_process_sensors() */



/* This array maps the event states of ilo2_ribcl sensor severity states to
 * HPI event severity.
 */
static SaHpiSeverityT ir_state2crit[] = {
	/* I2R_INITIAL */		SAHPI_OK,
	/* I2R_OK */			SAHPI_OK,
	/* I2R_DEGRADED_FROM_OK */	SAHPI_MAJOR,
	/* I2R_DEGRADED_FROM_FAIL */	SAHPI_MAJOR,
	/* I2R_FAILED */		SAHPI_CRITICAL
};

/* This array maps the event states of ilo2_ribcl sensor severity states to
 * the corresponding HPI sensor SAHPI_EC_SEVERITY event state value.
 */
static 	SaHpiEventStateT ir_state2ev_state[] = {
	/* I2R_INITIAL */		SAHPI_ES_OK,
	/* I2R_OK */			SAHPI_ES_OK,
	/* I2R_DEGRADED_FROM_OK */	SAHPI_ES_MAJOR_FROM_LESS,
	/* I2R_DEGRADED_FROM_FAIL */	SAHPI_ES_MAJOR_FROM_CRITICAL,
	/* I2R_FAILED */		SAHPI_ES_CRITICAL
};

/**
 * ilo2_ribcl_process_severity_sensor:
 * @oh_handler:	   Pointer to the handler for this instance.
 * @sens_allinfo:  Pointer to structure giving all HPI info for this sensor.
 * @ir_sens_dat:   Pointer to iLo2 RIBCL data for this sensor. 
 *
 * This routine is given the sensor data obtained from iLo2 via parameter
 * ir_sens_dat and converts that data into an HPI sensor value, an HPI
 * event state, and a HPI event severity. It also sends the appropriate sensor
 * de-assertion and assertion events, if required. The new sensor value is
 * then cached in the sensor data associated with the sensor RDR.
 *
 * The sensor viewed from iLo2 can be in one of the following states (given
 * in ir_sens_dat->state):
 *
 *	I2R_NO_EXIST - The sensor has never been detected during discovery.
 *	I2R_INITIAL  - The sensor has never been processed.
 *	I2R_OK	     - iLo2 reports the sensor status as "Ok".
 *	I2R_DEGRADED_FROM_OK - iLo2 previously reported the sensor as "Ok",
 *			but now reports the sensor as "Degraded".
 *	I2R_DEGRADED_FROM_FAIL - iLo2 previously reported the sensor as
 *			"Failed", but now reports the sensor as "Degraded".
 *	I2R_FAILED   - iLo2 reports the sensor as "Failed". This does not mean
 *			the sensor itself has failed. It means a failure staus
 *			of the health the sensor was monitoring.
 *
 * The iLo2 sensor state is  mapped into the following HPI defined
 * SAHPI_EC_SEVERITY event states:
 *
 *	SAHPI_ES_OK
 * 	SAHPI_ES_MAJOR_FROM_LESS
 *	SAHPI_ES_MAJOR_FROM_CRITICAL
 *	SAHPI_ES_CRITICAL
 *
 * A high level description of the algorithm used is: 
 *
 * 	if( sensor is disabled){
 *		return;
 * 	}
 * 	if( sensor reading has not changed){
 *		return;
 * 	}
 *
 *	Update the stored sensor value with the current sensor reading.
 *
 *	Determine the new ribcl sensor state based on the new sensor reading
 *	and the and the current iLo2 RIBCL sensor state.
 *
 *	Determine the new HPI severity sensor SAHPI_EC_SEVERITY event state
 *	from the iLo2 RIBCL sensor state.
 *
 *	if( ( enables ok) && ( previously not in the initial state) ){
 *		Send de-assert event for the previous state.
 *	}
 *
 *	if( enables ok){
 *		Send assert event for the new state.
 *	}
 *
 * Return values:
 * None
 **/
static void ilo2_ribcl_process_severitysensor(
			struct oh_handler_state *oh_handler,
			struct ilo2_ribcl_sens_allinfo *sens_allinfo,
			I2R_SensorDataT *ir_sens_dat)
{
	struct ilo2_ribcl_sensinfo *sensinfo;
	I2R_SensorStateT old_ribcl_state;

	sensinfo = sens_allinfo->sens_dat;

	/* If the sensor is not enabled, we should not do anything */
	if( sensinfo->sens_enabled != SAHPI_TRUE){
		return;
	}

	/* If the sensor reading has not changed since we last examined it,
	 * we have nothing further to do. However if we are just starting
	 * up, we may want to send an event for our current value. 
	 */
	if( (ir_sens_dat->reading.intval == sensinfo->sens_value) &&
	    (ir_sens_dat->state != I2R_INITIAL)){
		return ;
	}

	old_ribcl_state = ir_sens_dat->state;

	/* Update our stored HPI sensor value with the current iLo2 RIBCL
	 * sensor reading.
	 */
	sensinfo->sens_value = ir_sens_dat->reading.intval;

	/* Now, we determine the new iLo2 RIBCL sensor state based upon the
	 * new sensor reading and the current iLo2 RIBCL sensor state.
	 */
	switch( ir_sens_dat->reading.intval){

		case I2R_SEN_VAL_OK:
			ir_sens_dat->state = I2R_OK;
			break;

		case I2R_SEN_VAL_DEGRADED:
			if( ir_sens_dat->state == I2R_FAILED){
				ir_sens_dat->state = I2R_DEGRADED_FROM_FAIL;
			} else {
				ir_sens_dat->state = I2R_DEGRADED_FROM_OK;
			}
			break;

		case I2R_SEN_VAL_FAILED:
			ir_sens_dat->state = I2R_FAILED;
			break;

		default:
			/* This should not be possible */
			err("ilo2_ribcl_process_severitysensor: invalid value %d for sensor number %d.",
			      ir_sens_dat->reading.intval,  sensinfo->sens_num);
			break;

	} /* end switch(ir_sens_dat->reading.intval) */

	/* Determine the new HPI sensor severity SAHPI_EC_SEVERITY event 
	 * state from our new iLo2 RIBCL sensor state.
	 */
	sensinfo->prev_sens_ev_state = sensinfo->sens_ev_state;
	sensinfo->sens_ev_state =  ir_state2ev_state[ir_sens_dat->state];

	/* Since the value has changed, we might have to send an event to
	 * deassert the previous event state. If our previous ribcl state was
	 * I2R_INITIAL, then there was no previous event state. Below,
	 * sensinfo->event_sens_ev_state is the single state we will use
	 * for the event.
	 */
	sensinfo->event_sens_ev_state = sensinfo->prev_sens_ev_state;
	if( (old_ribcl_state != I2R_INITIAL) && (sensinfo->sens_ev_enabled) &&
	    ( sensinfo->event_sens_ev_state & sensinfo->sens_deassertmask)){

		ilo2_ribcl_sensor_send_event( oh_handler, sens_allinfo,
			SAHPI_ET_SENSOR, ir_state2crit[old_ribcl_state],
			SAHPI_FALSE);
	}

	/* Finally, we may have to send an event to assert the new event
	 * state.
	 */
	if( (sensinfo->sens_ev_enabled) &&
	     ( sensinfo->sens_ev_state & sensinfo->sens_assertmask) ){

		sensinfo->event_sens_ev_state = sensinfo->sens_ev_state;
		ilo2_ribcl_sensor_send_event( oh_handler, sens_allinfo,
			SAHPI_ET_SENSOR, ir_state2crit[ir_sens_dat->state],
			SAHPI_TRUE);
	}

} /* end ilo2_ribcl_process_severitysensor() */



/**
 * ilo2_ribcl_init_sensor_data:
 * @ir_handler:	    Pointer to the private handler for this instance.
 *
 * Designed to be called at handler instance open time, this routine
 * initializes the sensor data structures in the instance's DiscoveryData.
 *
 * Chassis sensors:
 * For all sensors in the chassis_sensors[] array within this handler's
 * DiscoveryData structure, set the state field to I2R_NO_EXIST, and the
 * reading.intval element to I2R_SEN_VAL_UNINITIALIZED.
 *
 * Return values:
 * None
 **/
	
void ilo2_ribcl_init_sensor_data( ilo2_ribcl_handler_t *ir_handler)
{
	int iter;

        for( iter = 0; iter < I2R_NUM_CHASSIS_SENSORS; iter++){
		ir_handler->DiscoveryData.chassis_sensors[iter].state =
								  I2R_NO_EXIST;
                ir_handler->DiscoveryData.chassis_sensors[iter].reading.intval =
						     I2R_SEN_VAL_UNINITIALIZED;
        }

} /* end ilo2_ribcl_init_sensor_data() */



/*****************************
	OpenHPI plug-in to iLO2 RIBCL plug-in ABI function mapping
*****************************/
void * oh_get_sensor_reading (void *, SaHpiResourceIdT, SaHpiSensorNumT,
			      SaHpiSensorReadingT *, SaHpiEventStateT *)
	__attribute__ ((weak, alias("ilo2_ribcl_get_sensor_reading")));

void * oh_get_sensor_enable (void *, SaHpiResourceIdT, SaHpiSensorNumT,
			      SaHpiBoolT *)
	__attribute__ ((weak, alias("ilo2_ribcl_get_sensor_enable")));

void * oh_set_sensor_enable (void *, SaHpiResourceIdT, SaHpiSensorNumT,
			      SaHpiBoolT )
	__attribute__ ((weak, alias("ilo2_ribcl_set_sensor_enable")));

void * oh_get_sensor_event_enables (void *, SaHpiResourceIdT, SaHpiSensorNumT,
				    SaHpiBoolT *)
	__attribute__ ((weak, alias("ilo2_ribcl_get_sensor_event_enable")));

void * oh_set_sensor_event_enables (void *, SaHpiResourceIdT, SaHpiSensorNumT,
				    SaHpiBoolT )
	__attribute__ ((weak, alias("ilo2_ribcl_set_sensor_event_enable")));

void * oh_get_sensor_event_masks (void *, SaHpiResourceIdT, SaHpiSensorNumT,
				  SaHpiEventStateT *, SaHpiEventStateT *)
	__attribute__ ((weak, alias("ilo2_ribcl_get_sensor_event_masks")));

void * oh_set_sensor_event_masks (void *, SaHpiResourceIdT, SaHpiSensorNumT,
				  SaHpiSensorEventMaskActionT,
				  SaHpiEventStateT , SaHpiEventStateT )
	__attribute__ ((weak, alias("ilo2_ribcl_set_sensor_event_masks")));

