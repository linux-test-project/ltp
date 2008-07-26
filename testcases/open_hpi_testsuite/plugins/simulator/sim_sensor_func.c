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
 *	  Christina Hernandez <hernanc@us.ibm.com>
 *        W. David Ashley <dashley@us.ibm.com>
 */

#include <sim_init.h>

/************************************************************************/
/* Sensor functions                                                     */
/************************************************************************/
SaErrorT sim_get_sensor_reading(void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiSensorNumT num,
                                SaHpiSensorReadingT *data,
                                SaHpiEventStateT    *state) {
        struct SensorInfo *sinfo;
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;

        if (!hnd) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        /* Check if resource exists and has sensor capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, id);
        if (!rpt)
                return(SA_ERR_HPI_INVALID_RESOURCE);
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR))
                return(SA_ERR_HPI_CAPABILITY);

        /* Check if sensor exist and is enabled */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, id,
                                            SAHPI_SENSOR_RDR, num);
        if (rdr == NULL)
                return(SA_ERR_HPI_NOT_PRESENT);
        sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, id,
                                                     rdr->RecordId);
        if (sinfo == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return(SA_ERR_HPI_NOT_PRESENT);
        }

        /*If sensor is enabled, get sensor reading*/
        if (sinfo->sensor_enabled == SAHPI_FALSE) {
                return(SA_ERR_HPI_INVALID_REQUEST);
        } else {
        	if (data) {
        		*data = sinfo->reading;
        	}
        	
        	if (state) {
        		*state = sinfo->cur_state;
        	}		
        }

        return(SA_OK);
}


SaErrorT sim_get_sensor_eventstate(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiSensorNumT sid,
				   SaHpiSensorReadingT *reading,
				   SaHpiEventStateT *state) {
	struct SensorInfo *sinfo;
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;

	if (!hnd || !reading || !state) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Check if resource exists and has sensor capabilities */
	SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt)
                return(SA_ERR_HPI_INVALID_RESOURCE);
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR))
                return(SA_ERR_HPI_CAPABILITY);

	/* Check if sensor exist and is enabled */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, rid,
                                            SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL)
                return(SA_ERR_HPI_NOT_PRESENT);
	sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid,
                                                     rdr->RecordId);
 	if (sinfo == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return(SA_ERR_HPI_NOT_PRESENT);
	}

	/*If sensor is enabled, set event state to cur_state*/
	if (sinfo->sensor_enabled == SAHPI_FALSE){
		return(SA_ERR_HPI_INVALID_REQUEST);
	} else {
		*state = sinfo->cur_state;
	}

	return(SA_OK);
}


SaErrorT sim_get_sensor_thresholds(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiSensorNumT sid,
				   SaHpiSensorThresholdsT *thres) {
        struct SensorInfo *sinfo;
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;

        if (!hnd || !thres) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }
        
        if (!rid) return SA_ERR_HPI_INVALID_RESOURCE;
        if (!sid) return SA_ERR_HPI_NOT_PRESENT;

        /* Check if resource exists and has sensor capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt)
                return(SA_ERR_HPI_INVALID_RESOURCE);
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR))
                return(SA_ERR_HPI_CAPABILITY);

        /* Check if sensor exist and is enabled */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, rid,
                                            SAHPI_SENSOR_RDR, sid);
        if (rdr == NULL)
                return(SA_ERR_HPI_NOT_PRESENT);
        sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid,
                                                     rdr->RecordId);
        if (sinfo == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return(SA_ERR_HPI_NOT_PRESENT);
        }

        /* If sensor is enabled, set thresholds */
        if (sinfo->sensor_enabled == SAHPI_FALSE){
                return(SA_ERR_HPI_INVALID_REQUEST);
        } else {
        	*thres = sinfo->thres;
	}

	return(SA_OK);
}


SaErrorT sim_set_sensor_thresholds(void *hnd,
                                   SaHpiResourceIdT rid,
				   SaHpiSensorNumT sid,
				   const SaHpiSensorThresholdsT *thres) {
	struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
	struct SensorInfo *sinfo;

	if (!hnd || !thres) {
		err("Invalid parameter");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Check if resource exists and has sensor capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt)
                return(SA_ERR_HPI_INVALID_RESOURCE);
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR))
                return(SA_ERR_HPI_CAPABILITY);

        /* Check if sensor exist and is enabled */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, rid,
                                            SAHPI_SENSOR_RDR, sid);
        if (rdr == NULL)
                return(SA_ERR_HPI_NOT_PRESENT);
        sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid,
                                                     rdr->RecordId);
        if (sinfo == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return(SA_ERR_HPI_NOT_PRESENT);
        }

	if (rdr->RdrTypeUnion.SensorRec.Category != SAHPI_EC_THRESHOLD ||
	    rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible == SAHPI_FALSE ||
	    rdr->RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold == 0) {
	    	return(SA_ERR_HPI_INVALID_CMD);
	}

        if (sinfo->sensor_enabled == SAHPI_FALSE){
        	return(SA_ERR_HPI_INVALID_REQUEST);
        }
        else{
                memcpy(&sinfo->thres, thres, sizeof(SaHpiSensorThresholdsT));
	}

        return(SA_OK);
}
/* Do we want this functionality???
SaErrorT sim_set_threshold_reading(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSensorNumT sid,
				       const SaHpiSensorReadingT *reading)
*/


SaErrorT sim_get_sensor_enable(void *hnd,
                               SaHpiResourceIdT rid,
			       SaHpiSensorNumT sid,
			       SaHpiBoolT *enable) {
	struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
	struct SensorInfo *sinfo;

        if (!hnd || !rid || !sid) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        /* Check if resource exists and has sensor capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt)
                return(SA_ERR_HPI_INVALID_RESOURCE);
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR))
                return(SA_ERR_HPI_CAPABILITY);

        /* Check if sensor exist and is enabled */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, rid,
                                            SAHPI_SENSOR_RDR, sid);
        if (rdr == NULL)
                return(SA_ERR_HPI_NOT_PRESENT);
        sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid,
                                                     rdr->RecordId);
        if (sinfo == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return(SA_ERR_HPI_NOT_PRESENT);
        }

        /*If sensor is enabled, get sensor enable */
        if (sinfo->sensor_enabled == SAHPI_FALSE){
                return(SA_ERR_HPI_INVALID_REQUEST);
        } else {
                *enable = sinfo->sensor_enabled;
        }

        return(SA_OK);
}


SaErrorT sim_set_sensor_enable(void *hnd,
                               SaHpiResourceIdT rid,
			       SaHpiSensorNumT sid,
			       const SaHpiBoolT enable) {
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct SensorInfo *sinfo;

        if (!hnd || !rid || !sid || !enable) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        /* Check if resource exists and has sensor capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt)
                return(SA_ERR_HPI_INVALID_RESOURCE);
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR))
                return(SA_ERR_HPI_CAPABILITY);

        /* Check if sensor exist and is enabled */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, rid,
                                            SAHPI_SENSOR_RDR, sid);
        if (rdr == NULL)
                return(SA_ERR_HPI_NOT_PRESENT);
        sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid,
                                                     rdr->RecordId);
        if (sinfo == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return(SA_ERR_HPI_NOT_PRESENT);
        }

        /* set sensor flag */
        sinfo->sensor_enabled = enable;

        return(SA_OK);
}


SaErrorT sim_get_sensor_event_enable(void *hnd,
                                     SaHpiResourceIdT rid,
				     SaHpiSensorNumT sid,
				     SaHpiBoolT *enable) {
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct SensorInfo *sinfo;

        if (!hnd || !rid || !sid || !enable) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        /* Check if resource exists and has sensor capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt)
                return(SA_ERR_HPI_INVALID_RESOURCE);
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR))
                return(SA_ERR_HPI_CAPABILITY);

        /* Check if sensor exist and is enabled */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, rid,
                                            SAHPI_SENSOR_RDR, sid);
        if (rdr == NULL)
                return(SA_ERR_HPI_NOT_PRESENT);
        sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid,
                                                     rdr->RecordId);
        if (sinfo == NULL) {
                err("No sensor data. Sensor=%s", rdr->IdString.Data);
                return(SA_ERR_HPI_NOT_PRESENT);
        }

        /* ge sensor event enable flag */
        if (sinfo->sensor_enabled == SAHPI_FALSE){
                return(SA_ERR_HPI_INVALID_REQUEST);
        } else {
		*enable = sinfo->events_enabled;
	}
        return(SA_OK);
}


SaErrorT sim_set_sensor_event_enable(void *hnd,
                                     SaHpiResourceIdT rid,
				     SaHpiSensorNumT sid,
				     const SaHpiBoolT enable) {
	struct oh_handler_state *handle = (struct oh_handler_state *)hnd;

	if (!hnd ) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Check if resource exists and has sensor capabilities */
	SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt)
		return(SA_ERR_HPI_INVALID_RESOURCE);
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR))
		return(SA_ERR_HPI_CAPABILITY);

	/* Check if sensor exists and if it supports setting of sensor event enablement */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, rid,
                                            SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL)
		return(SA_ERR_HPI_NOT_PRESENT);

	if (rdr->RdrTypeUnion.SensorRec.EventCtrl == SAHPI_SEC_PER_EVENT ||
	    rdr->RdrTypeUnion.SensorRec.EventCtrl == SAHPI_SEC_READ_ONLY_MASKS) {
		err("BladeCenter/RSA do not support sim_set_sensor_event_enable\n");
		struct SensorInfo *sinfo;
		sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache,
                                                             rid, rdr->RecordId);
		if (sinfo == NULL) {
			err("No sensor data. Sensor=%s", rdr->IdString.Data);
                        return(SA_ERR_HPI_NOT_PRESENT);
		}
		sinfo->events_enabled = enable;
	} else {
		return(SA_ERR_HPI_READ_ONLY);
	}

	return(SA_OK);
}


SaErrorT sim_get_sensor_event_masks(void *hnd,
                                    SaHpiResourceIdT rid,
				    SaHpiSensorNumT sid,
				    SaHpiEventStateT *AssertEventMask,
				    SaHpiEventStateT *DeassertEventMask) {
	struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
	struct SensorInfo *sinfo;

	if (!hnd ) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Check if resource exists and has sensor capabilities */
	SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt)
		return(SA_ERR_HPI_INVALID_RESOURCE);
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR))
		return(SA_ERR_HPI_CAPABILITY);

	/* Check if sensor exists and return enablement status */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL)
		return(SA_ERR_HPI_NOT_PRESENT);

	if (!AssertEventMask && !DeassertEventMask) {
		return SA_OK;
	}
	
	sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, rid,
                                                     rdr->RecordId);
 	if (sinfo == NULL) {
		err("No sensor data. Sensor=%s", rdr->IdString.Data);
		return(SA_ERR_HPI_NOT_PRESENT);
	}

	if (AssertEventMask) *AssertEventMask = sinfo->assert_mask;
	
	if (DeassertEventMask) {
		if (rpt->ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS) {
			*DeassertEventMask = sinfo->assert_mask;
		}
		else {
			*DeassertEventMask = sinfo->deassert_mask;
		}
	}

        return(SA_OK);
}


SaErrorT sim_set_sensor_event_masks(void *hnd,
                                    SaHpiResourceIdT rid,
				    SaHpiSensorNumT sid,
				    SaHpiSensorEventMaskActionT act,
				    const SaHpiEventStateT AssertEventMask,
				    const SaHpiEventStateT DeassertEventMask) {
	struct oh_handler_state *handle = (struct oh_handler_state *)hnd;

	if (!hnd ) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	if (oh_lookup_sensoreventmaskaction(act) == NULL)
		return(SA_ERR_HPI_INVALID_DATA);

	/* Check if resource exists and has sensor capabilities */
	SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt)
		return(SA_ERR_HPI_INVALID_RESOURCE);
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR))
		return(SA_ERR_HPI_CAPABILITY);

	/* Check if sensor exists and if it supports setting of sensor event masks */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, rid,
                                            SAHPI_SENSOR_RDR, sid);
	if (rdr == NULL)
		return(SA_ERR_HPI_NOT_PRESENT);

	if (rdr->RdrTypeUnion.SensorRec.EventCtrl == SAHPI_SEC_PER_EVENT) {
		err("BladeCenter/RSA do not support sim_set_sensor_event_masks");
                /* Probably need to drive an OID, if hardware supported it */
		struct SensorInfo *sinfo;
		sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache,
                                                             rid, rdr->RecordId);
		if (sinfo == NULL) {
			err("No sensor data. Sensor=%s", rdr->IdString.Data);
                        return(SA_ERR_HPI_NOT_PRESENT);
		}

		SaHpiEventStateT orig_assert_mask = sinfo->assert_mask;
		SaHpiEventStateT orig_deassert_mask = sinfo->deassert_mask;

		/* Check for invalid data in user masks */
		if ( (AssertEventMask != SAHPI_ALL_EVENT_STATES) &&
		     (AssertEventMask & ~(rdr->RdrTypeUnion.SensorRec.Events)) ) {
			return(SA_ERR_HPI_INVALID_DATA);
		}
		if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS)) {
			if  ( (DeassertEventMask != SAHPI_ALL_EVENT_STATES) &&
				(DeassertEventMask & ~(rdr->RdrTypeUnion.SensorRec.Events)) ) {
				return(SA_ERR_HPI_INVALID_DATA);
			}
		}

		/* Add to event masks */
		if (act == SAHPI_SENS_ADD_EVENTS_TO_MASKS) {
			if (AssertEventMask == SAHPI_ALL_EVENT_STATES) {
				sinfo->assert_mask = rdr->RdrTypeUnion.SensorRec.Events;
			} else {
				sinfo->assert_mask = sinfo->assert_mask | AssertEventMask;
			}
			if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS)) {
				if (DeassertEventMask == SAHPI_ALL_EVENT_STATES) {
					sinfo->deassert_mask = rdr->RdrTypeUnion.SensorRec.Events;
				} else {
					sinfo->deassert_mask = sinfo->deassert_mask | DeassertEventMask;
				}
			}
		}
		else { /* Remove from event masks */
			if (AssertEventMask == SAHPI_ALL_EVENT_STATES) {
				sinfo->assert_mask = 0;
			} else {
				sinfo->assert_mask = sinfo->assert_mask & ~AssertEventMask;
			}
			if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS)) {
				if (DeassertEventMask == SAHPI_ALL_EVENT_STATES) {
					sinfo->deassert_mask = 0;
				} else {
					sinfo->deassert_mask = sinfo->deassert_mask & ~DeassertEventMask;
				}
			}
		}

		if (sinfo->assert_mask != orig_assert_mask) {
                        // nop?
		} else {
			if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS) &&
			    sinfo->deassert_mask != orig_deassert_mask) {
			}
		}
	} else {
		return(SA_ERR_HPI_READ_ONLY);
	}

	return(SA_OK);
}


/*
 * Simulator plugin interface
 *
 */

void * oh_get_sensor_reading (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiSensorReadingT *,
                            SaHpiEventStateT    *)
                __attribute__ ((weak, alias("sim_get_sensor_reading")));

void * oh_get_sensor_thresholds (void *, SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 SaHpiSensorThresholdsT *)
                __attribute__ ((weak, alias("sim_get_sensor_thresholds")));

void * oh_set_sensor_thresholds (void *, SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 const SaHpiSensorThresholdsT *)
                __attribute__ ((weak, alias("sim_set_sensor_thresholds")));

void * oh_get_sensor_enable (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiBoolT *)
                __attribute__ ((weak, alias("sim_get_sensor_enable")));

void * oh_set_sensor_enable (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiBoolT)
                __attribute__ ((weak, alias("sim_set_sensor_enable")));

void * oh_get_sensor_event_enables (void *, SaHpiResourceIdT,
                                    SaHpiSensorNumT,
                                    SaHpiBoolT *)
                __attribute__ ((weak, alias("sim_get_sensor_event_enable")));

void * oh_set_sensor_event_enables (void *, SaHpiResourceIdT id, SaHpiSensorNumT,
                                    SaHpiBoolT *)
                __attribute__ ((weak, alias("sim_set_sensor_event_enable")));

void * oh_get_sensor_event_masks (void *, SaHpiResourceIdT, SaHpiSensorNumT,
                                  SaHpiEventStateT *, SaHpiEventStateT *)
                __attribute__ ((weak, alias("sim_get_sensor_event_masks")));

void * oh_set_sensor_event_masks (void *, SaHpiResourceIdT, SaHpiSensorNumT,
                                  SaHpiSensorEventMaskActionT,
                                  SaHpiEventStateT,
                                  SaHpiEventStateT)
                __attribute__ ((weak, alias("sim_set_sensor_event_masks")));


