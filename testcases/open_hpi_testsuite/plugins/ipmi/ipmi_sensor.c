/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Louis Zhuang <louis.zhuang@linux.intel.com>
 *     Kevin Gao <kevin.gao@linux.intel.com>
 *     Rusty Lynch <rusty.lynch@linux.intel.com>
 *     Racing Guo <racing.guo@intel.com>
 */

#include "ipmi.h"

#define OHOI_TIMEOUT  10    /* 10 seconds, was 5 seconds */
struct ipmi_event_state_s
{
    unsigned int status;
    /* Pay no attention to the implementation. */
    unsigned int __assertion_events;
    unsigned int __deassertion_events;
};


/* 
 * Use for getting sensor reading
 */
struct ohoi_sensor_reading {
	SaHpiSensorReadingT	reading;
        SaHpiEventStateT        ev_state;
	int			done;
	int			rvalue;
};

/*
 * Use for getting/setting sensor threadholds
 */


struct ohoi_sensor_thresholds {
	SaHpiSensorThresholdsT	sensor_thres;
	ipmi_thresholds_t	*thrhlds;
	int			thres_done;
        int                     hyster_done;
	int			rvalue;
};


/*
 * Use for sensor event enable and sensor event masks
 */

struct ohoi_sensor_event_enable_masks {
	SaHpiBoolT   enable;
	SaHpiEventStateT  assert;
	SaHpiEventStateT  deassert;
	unsigned int	  a_support;
	unsigned int	  d_support;
	ipmi_event_state_t	*states;
	int done;
	int rvalue;
};

static int ignore_sensor(ipmi_sensor_t *sensor)
{
        ipmi_entity_t *ent;

        if (ipmi_sensor_get_ignore_if_no_entity(sensor)) {
		err("ignore if no entity");
                return 0;
	}
        ent = ipmi_sensor_get_entity(sensor);
	if (ent == NULL) {
		err("ipmi_sensor_get_entity = NULL");
		return 1;
	}
        if (!ipmi_entity_is_present(ent)) {
		err("!ipmi_entity_is_present. (%d,%d,%d,%d) %s",
		ipmi_entity_get_entity_id(ent), 
		ipmi_entity_get_entity_instance(ent),
		ipmi_entity_get_device_channel(ent),
		ipmi_entity_get_device_address(ent),
		ipmi_entity_get_entity_id_string(ent));
                return 0;
	}
        return 0;
}




		/*      GET SENSOR READING  */


static SaHpiEventStateT retrieve_states(ipmi_states_t *states)
{
	SaHpiEventStateT st = 0;
	int i;

	// fortunatly as discrete as threshold sensors IPMI states
	// is mapped 1:1 to HPI states
	for (i = 0; i < 15; i++) {
		if (ipmi_is_state_set(states, i)) {
			st |= (1 << i);
		}
	}
	return st;
}
	
static void sensor_read_states(ipmi_sensor_t *sensor,
			       int	   err,
			       ipmi_states_t *states,
			       void *cb_data)
{
	struct ohoi_sensor_reading *p = cb_data;

	p->done = 1;
	if (err) {
		err("sensor reading state error");
		p->rvalue = SA_ERR_HPI_INTERNAL_ERROR;
		return;
	}
	p->reading.IsSupported = SAHPI_FALSE;
	p->ev_state = retrieve_states(states);
}


static void sensor_reading(ipmi_sensor_t		*sensor,
		  	   int 				err,
			   enum ipmi_value_present_e	value_present,
			   unsigned int 		raw_val,
			   double 			val,
			   ipmi_states_t 		*states,
			   void 			*cb_data)
{
	struct ohoi_sensor_reading *p = cb_data;
 
	p->done = 1;

	if (err) {
		OHOI_MAP_ERROR(p->rvalue, err);
		err("sensor reading error");
		p->rvalue = SA_ERR_HPI_INTERNAL_ERROR;
		return;
	}

	p->reading.IsSupported = SAHPI_FALSE;

	if (value_present == IPMI_BOTH_VALUES_PRESENT) {
		p->reading.IsSupported = SAHPI_TRUE;
		p->reading.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		p->reading.Value.SensorFloat64 = val;
	} else if(value_present == IPMI_RAW_VALUE_PRESENT) {
		p->reading.IsSupported = SAHPI_TRUE;
		p->reading.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		p->reading.Value.SensorFloat64 = raw_val;
	} else {
		err("value present = 0x%x", value_present);
	}
	// always returns 1 in 7th bit. Ignore extra 1
	p->ev_state = retrieve_states(states) &
		(SAHPI_ES_LOWER_MINOR | SAHPI_ES_LOWER_MAJOR |
		 SAHPI_ES_LOWER_CRIT | SAHPI_ES_UPPER_MINOR |
		 SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT);
}

static void get_sensor_reading(ipmi_sensor_t *sensor, void *cb_data)
{
	struct ohoi_sensor_reading *reading_data;	
	int rv;	

        reading_data = cb_data;
        
	if (ignore_sensor(sensor)) {
		reading_data->done = 1;
		reading_data->rvalue = SA_ERR_HPI_NOT_PRESENT;
		err("Sensor is not present, ignored");
		return;
	}

	if (ipmi_sensor_get_event_reading_type(sensor) ==
			 IPMI_EVENT_READING_TYPE_THRESHOLD) {
		rv = ipmi_sensor_get_reading(sensor, sensor_reading,
			reading_data);
		if (rv) {
			reading_data->done = 1;
			reading_data->rvalue = SA_ERR_HPI_INVALID_REQUEST;
			err("Unable to get sensor reading: 0x%x", rv);
				
		}
		return;
	}
	rv = ipmi_sensor_get_states(sensor, sensor_read_states, reading_data);
	if (rv) {
		reading_data->done = 1;
		reading_data->rvalue = SA_ERR_HPI_INVALID_REQUEST;
		err("Unable to get sensor reading states: 0x%x", rv);
	}

}


SaErrorT orig_get_sensor_reading(struct oh_handler_state *handler,
				 struct ohoi_sensor_info *sensor_info,
				 SaHpiSensorReadingT *reading,
				 SaHpiEventStateT *ev_state)
{
	struct ohoi_handler	*ipmi_handler =
					(struct ohoi_handler *)(handler->data);
	struct ohoi_sensor_reading	reading_data;	
        int				rv;
	ipmi_sensor_id_t		sensor_id;

	sensor_id = sensor_info->info.orig_sensor_info.sensor_id;

	memset(&reading_data, 0, sizeof(reading_data));

        rv = ipmi_sensor_pointer_cb(sensor_id, 
				    get_sensor_reading,
				    &reading_data);
	if (rv) {
		err("Unable to convert sensor_id to pointer");
		return SA_ERR_HPI_INVALID_CMD;
	}

	rv = ohoi_loop(&reading_data.done, ipmi_handler);

	if (rv)
		return rv;
	if (reading_data.rvalue)
		return reading_data.rvalue;

	*reading = reading_data.reading;
	*ev_state = reading_data.ev_state & 0x7fff;

	return SA_OK;
}

SaErrorT ohoi_get_sensor_reading(void *hnd,
				 struct ohoi_sensor_info *sensor_info,
				 SaHpiSensorReadingT *reading,
				 SaHpiEventStateT *ev_state)
{
	struct oh_handler_state *handler = (struct oh_handler_state *)hnd;

	if (sensor_info->ohoii.get_sensor_reading == NULL) {
		return SA_ERR_HPI_INVALID_CMD;
	}

	return sensor_info->ohoii.get_sensor_reading(handler,
					sensor_info, reading, ev_state);
}



		/*     GET SENSOR THRESHOLDS     */


static void thres_get(ipmi_sensor_t		*sensor,
		      ipmi_thresholds_t		*th,
		      unsigned int		event,
		      SaHpiSensorReadingT	*thres)
{
	int val;
	
	ipmi_sensor_threshold_readable(sensor, event, &val);
	if (!val) {
		thres->IsSupported = SAHPI_FALSE;
		return;
	}	
	if (0 == ipmi_threshold_get(th, event,
				&thres->Value.SensorFloat64)) {
		thres->IsSupported = SAHPI_TRUE;
		thres->Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
	}else {
		thres->IsSupported = SAHPI_FALSE;
	}
}

static void thresholds_read(ipmi_sensor_t	*sensor,
			    int			err,
			    ipmi_thresholds_t	*th,
			    void		*cb_data)
{
	struct ohoi_sensor_thresholds *p = cb_data;


	if (err) {
		OHOI_MAP_ERROR(p->rvalue, err);
		p->thres_done = 1;
		err("sensor thresholds reading error");
		return;
	}
	
	thres_get(sensor, th, IPMI_LOWER_NON_CRITICAL,
		  &p->sensor_thres.LowMinor);
	thres_get(sensor, th, IPMI_LOWER_CRITICAL,
		  &p->sensor_thres.LowMajor);
	thres_get(sensor, th, IPMI_LOWER_NON_RECOVERABLE,
		  &p->sensor_thres.LowCritical);
	thres_get(sensor, th, IPMI_UPPER_NON_CRITICAL,
		  &p->sensor_thres.UpMinor);
	thres_get(sensor, th, IPMI_UPPER_CRITICAL,
		  &p->sensor_thres.UpMajor);
	thres_get(sensor, th, IPMI_UPPER_NON_RECOVERABLE,
		  &p->sensor_thres.UpCritical);
	p->thres_done = 1;
}

static SaErrorT get_thresholds(ipmi_sensor_t	*sensor,
			       struct ohoi_sensor_thresholds	*thres_data)
{
	int		rv;

	rv = ipmi_sensor_get_thresholds(sensor, thresholds_read, thres_data);
	if (rv) 
		err("Unable to get sensor thresholds: 0x%x\n", rv);
	return (rv? SA_ERR_HPI_INVALID_CMD : SA_OK);
}

static void hysteresis_read(ipmi_sensor_t	*sensor,
			    int			err,
			    unsigned int	positive_hysteresis,
			    unsigned int 	negative_hysteresis,
			    void 		*cb_data)
{
	struct ohoi_sensor_thresholds *p = cb_data;

	if (err) {
		p->rvalue = SA_ERR_HPI_INTERNAL_ERROR;
		p->hyster_done = 1;
		err("sensor hysteresis reading error");
		return;		
	}

	p->sensor_thres.PosThdHysteresis.IsSupported = SAHPI_TRUE;
	p->sensor_thres.PosThdHysteresis.Type =
		SAHPI_SENSOR_READING_TYPE_FLOAT64;
	p->sensor_thres.PosThdHysteresis.Value.SensorFloat64 =
		positive_hysteresis;

	p->sensor_thres.NegThdHysteresis.IsSupported = SAHPI_TRUE;
        p->sensor_thres.NegThdHysteresis.Type =
		SAHPI_SENSOR_READING_TYPE_FLOAT64;
        p->sensor_thres.NegThdHysteresis.Value.SensorFloat64 =
		negative_hysteresis;
	p->hyster_done = 1;
}

static SaErrorT get_hysteresis(ipmi_sensor_t			*sensor,
			  struct ohoi_sensor_thresholds	*thres_data)
{
	int		rv;
	
	rv = ipmi_sensor_get_hysteresis(sensor, hysteresis_read, thres_data);
        if (rv)
                err("Unable to get sensor hysteresis: 0x%x\n", rv);
        
	return (rv? SA_ERR_HPI_INVALID_CMD : SA_OK);

}

static void get_sensor_thresholds(ipmi_sensor_t *sensor, 
                                  void          *cb_data)
{
	struct ohoi_sensor_thresholds *thres_data;
	int rv;
	
        thres_data = cb_data;

	if (ignore_sensor(sensor)) {
		thres_data->hyster_done = 1;
		thres_data->thres_done = 1;
		thres_data->rvalue = SA_ERR_HPI_NOT_PRESENT;
                err("ENTITY_NOT_PRESENT");
		return;
	}	

	if (ipmi_sensor_get_event_reading_type(sensor) !=
			IPMI_EVENT_READING_TYPE_THRESHOLD) {
		err("Not threshold sensor!");
		thres_data->hyster_done = 1;
		thres_data->thres_done = 1;
		thres_data->rvalue = SA_ERR_HPI_INVALID_CMD;
		return;
	}
	if (ipmi_sensor_get_threshold_access(sensor) ==
				IPMI_THRESHOLD_ACCESS_SUPPORT_NONE) {
		err("sensor doesn't support threshold read");
			err("Unable to get sensor thresholds");
			thres_data->hyster_done = 1;
			thres_data->thres_done = 1;
			thres_data->rvalue = SA_ERR_HPI_INVALID_CMD;
			return;
	}
	rv = get_thresholds(sensor, thres_data);
	if (rv != SA_OK) {
		err("Unable to get sensor thresholds");
		thres_data->hyster_done = 1;
		thres_data->thres_done = 1;
		thres_data->rvalue = rv;
		return;
	}

	rv = ipmi_sensor_get_hysteresis_support(sensor);

	if (rv != IPMI_HYSTERESIS_SUPPORT_READABLE &&
			rv != IPMI_HYSTERESIS_SUPPORT_SETTABLE) {
//		thres_data->thres_done = 1;
		thres_data->hyster_done = 1;
		thres_data->sensor_thres.PosThdHysteresis.IsSupported =
				SAHPI_FALSE;
		thres_data->sensor_thres.NegThdHysteresis.IsSupported =
				SAHPI_FALSE;
		return;
	} 
	rv = get_hysteresis(sensor, thres_data);
	if (rv != SA_OK) {
		err("failed to get hysteresis");
		thres_data->hyster_done = 1;
//		thres_data->thres_done = 1;
		thres_data->rvalue = SA_ERR_HPI_INTERNAL_ERROR;
		return;
	}

	return;
}

static int is_get_sensor_thresholds_done(const void *cb_data)
{
        const struct ohoi_sensor_thresholds *thres_data;
        
        thres_data = cb_data;
	/* Can we check the validity of this pointer here to avoid SegFault? */
        return (thres_data->thres_done && thres_data->hyster_done);
}

SaErrorT orig_get_sensor_thresholds(struct oh_handler_state *handler,
				    struct ohoi_sensor_info *sensor_info,
				    SaHpiSensorThresholdsT *thres)
{
	struct ohoi_handler	*ipmi_handler =
					(struct ohoi_handler *)(handler->data);
	struct ohoi_sensor_thresholds	thres_data;
        int				rv;
		ipmi_sensor_id_t		sensor_id;

	sensor_id = sensor_info->info.orig_sensor_info.sensor_id;

        memset(&thres_data, 0, sizeof(thres_data));
        
        rv = ipmi_sensor_pointer_cb(sensor_id,
                                    get_sensor_thresholds,
                                    &thres_data);
        if (rv) {
                err("Unable to convert sensor id into pointer");
		return SA_ERR_HPI_INVALID_CMD;
        }

        rv = ohoi_loop_until(is_get_sensor_thresholds_done, 
                               &thres_data, OHOI_TIMEOUT, ipmi_handler);

	if (rv) 
		return rv;
	
	if (thres_data.rvalue)
		return thres_data.rvalue;

	if (thres)
		*thres = thres_data.sensor_thres;
	return SA_OK;
}

SaErrorT ohoi_get_sensor_thresholds(void *hnd,
				    struct ohoi_sensor_info *sensor_info,
				    SaHpiSensorThresholdsT *thres)
{
	struct oh_handler_state *handler = (struct oh_handler_state *)hnd;

	if (sensor_info->ohoii.get_sensor_thresholds == NULL) {
		return SA_ERR_HPI_INVALID_CMD;
	}

	return sensor_info->ohoii.get_sensor_thresholds(handler,
							sensor_info, thres);
}




		/*      SET SENSOR THRESHOLDS  */


static void thres_set_data(ipmi_sensor_t *sensor, int err, void *cb_data)
{
	struct ohoi_sensor_thresholds *info = cb_data;
	if (err) {
		err("err = 0x%x", err);
		if (info->rvalue == SA_OK) {
			OHOI_MAP_ERROR(info->rvalue, err);
		}
	}
	info->thres_done = 1;
}


static void hys_set_data(ipmi_sensor_t *sensor, int err, void *cb_data)
{
	struct ohoi_sensor_thresholds *info = cb_data;
	if (err) {
		err("err = 0x%x", err);
		if (info->rvalue == SA_OK) {
			OHOI_MAP_ERROR(info->rvalue, err);
		}
	}
	info->hyster_done = 1;
}



static SaErrorT thres_cpy(ipmi_sensor_t			*sensor, 
		      const SaHpiSensorReadingT		reading,
		      unsigned int			event,
		      ipmi_thresholds_t			*info) 
{
	int	val;
	int	rv;

	if (!reading.IsSupported) {
		return SA_OK;
	}
	if ((rv = ipmi_sensor_threshold_settable(sensor, event, &val))) {
		err("ipmi_sensor_threshold_settable error = %d", rv);
		return SA_ERR_HPI_INVALID_CMD;
	}
	if (!val) {
		err("ipmi threshold 0x%x isn't settable", event);
		return SA_ERR_HPI_INVALID_DATA;
	}
	
	switch (reading.Type) {
		/*Fix Me* case...*/
		case  SAHPI_SENSOR_READING_TYPE_INT64:
		case  SAHPI_SENSOR_READING_TYPE_UINT64:
		case  SAHPI_SENSOR_READING_TYPE_BUFFER:
			break;
		case SAHPI_SENSOR_READING_TYPE_FLOAT64:
			if(ipmi_threshold_set(info, sensor, event, 
				reading.Value.SensorFloat64)) {
				return SA_OK;
			}
			break;
	}
	return SA_OK;
}

static SaErrorT init_thresholeds_info(
				ipmi_sensor_t			*sensor, 
				const SaHpiSensorThresholdsT	*thres,
				ipmi_thresholds_t		*info)
{
	SaErrorT rv;
	if (ipmi_thresholds_init(info)) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	rv = thres_cpy(sensor, thres->LowMinor, IPMI_LOWER_NON_CRITICAL, info);
	if (rv != SA_OK)
		return rv;
	
	rv = thres_cpy(sensor, thres->LowMajor, IPMI_LOWER_CRITICAL, info);
	if (rv != SA_OK)
		return rv;
	
	rv = thres_cpy(sensor, thres->LowCritical, IPMI_LOWER_NON_RECOVERABLE,
		       info);
	if (rv != SA_OK)
		return rv;
	
	rv = thres_cpy(sensor, thres->UpMinor, IPMI_UPPER_NON_CRITICAL, info);
	if (rv != SA_OK)
		return rv;
	
	rv = thres_cpy(sensor, thres->UpMajor, IPMI_UPPER_CRITICAL, info);
	if (rv != SA_OK)
		return rv;
	
	rv = thres_cpy(sensor, thres->UpCritical, IPMI_UPPER_NON_RECOVERABLE,
		       info);
	if (rv != SA_OK)
		return rv;

	return SA_OK;
}

static SaErrorT set_thresholds(ipmi_sensor_t                 *sensor, 
			  struct ohoi_sensor_thresholds *thres_data)
{
	ipmi_thresholds_t	*info = thres_data->thrhlds;
	int			rv;	
	

	rv = init_thresholeds_info(sensor, &thres_data->sensor_thres, info);
	if (rv != SA_OK) {
		err("Unable to init sensor thresholds: 0x%x\n", rv);
		return rv;
	}

	rv = ipmi_sensor_set_thresholds(sensor, info, thres_set_data, 
                                 thres_data);
	if (rv) {
		err("Unable to set sensor thresholds: 0x%x\n", rv);
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	return SA_OK;
}

static SaErrorT set_hysteresis(ipmi_sensor_t	                *sensor,
			  struct ohoi_sensor_thresholds	*thres_data)
{
	int			rv;	
	unsigned int		pos = 0, neg = 0;
	SaHpiSensorReadingT	pos_reading 
                = thres_data->sensor_thres.PosThdHysteresis;
	SaHpiSensorReadingT	neg_reading 
                = thres_data->sensor_thres.NegThdHysteresis;
	
        switch (pos_reading.Type) {
                case SAHPI_SENSOR_READING_TYPE_INT64:
                        pos = pos_reading.Value.SensorInt64;
                        break;
                case SAHPI_SENSOR_READING_TYPE_UINT64:
                        pos = pos_reading.Value.SensorUint64;
                        break;
                case SAHPI_SENSOR_READING_TYPE_FLOAT64:
                        pos = pos_reading.Value.SensorFloat64;
                        break;
                case SAHPI_SENSOR_READING_TYPE_BUFFER:
                        err("ipmi sensor doesn't support this type of reading");
                        return SA_ERR_HPI_INVALID_DATA;   
        }
	
        switch (neg_reading.Type) {
                case SAHPI_SENSOR_READING_TYPE_INT64:
                        neg = neg_reading.Value.SensorInt64;
                        break;
                case SAHPI_SENSOR_READING_TYPE_UINT64:
                        neg = neg_reading.Value.SensorUint64;
                        break;
                case SAHPI_SENSOR_READING_TYPE_FLOAT64:
                        neg = neg_reading.Value.SensorFloat64;
                        break;
                case SAHPI_SENSOR_READING_TYPE_BUFFER:
                        err("ipmi sensor doesn't support this type of reading");
                        return SA_ERR_HPI_INVALID_DATA;
        }


	rv = ipmi_sensor_set_hysteresis(sensor, pos, neg, hys_set_data, 
                                        thres_data);
	if (rv) {
		err("Unable to set sensor hysteresis: 0x%x\n", rv);
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

        return SA_OK;
}

static void set_sensor_thresholds(ipmi_sensor_t *sensor, 
                                  void          *cb_data)
{
	struct ohoi_sensor_thresholds *thres_data;
	SaErrorT rv;	

	thres_data = cb_data;
	if (ignore_sensor(sensor)) {
		err("sensor is ignored");
		thres_data->hyster_done = 1;
		thres_data->thres_done = 1;
		thres_data->rvalue = SA_ERR_HPI_NOT_PRESENT;
		return;
	}

	if (ipmi_sensor_get_event_reading_type(sensor) !=
			IPMI_EVENT_READING_TYPE_THRESHOLD) {
		err("Not threshold sensor!");
		thres_data->hyster_done = 1;
		thres_data->thres_done = 1;
		thres_data->rvalue = SA_ERR_HPI_INVALID_CMD;
		return;
	}
				
	if ((ipmi_sensor_get_threshold_access(sensor) !=
				IPMI_THRESHOLD_ACCESS_SUPPORT_SETTABLE) ||
			(ipmi_sensor_get_hysteresis_support(sensor) !=
					IPMI_HYSTERESIS_SUPPORT_SETTABLE)) {
		err("sensor doesn't support threshold or histeresis set");
		thres_data->hyster_done = 1;
		thres_data->thres_done = 1;
		thres_data->rvalue = SA_ERR_HPI_INVALID_CMD;
		return;
	}
	rv = set_thresholds(sensor, thres_data);
	if (rv != SA_OK) {
		err("Unable to set thresholds");
		thres_data->hyster_done = 1;
		thres_data->thres_done = 1;
		thres_data->rvalue = rv;
		return;
	}	
 
	rv = set_hysteresis(sensor, thres_data);
	if (rv != SA_OK) {
		thres_data->hyster_done = 1;
		thres_data->thres_done = 1;
		thres_data->rvalue = rv;
		err("Unable to set hysteresis");
		return;
	}
	return;
}

SaErrorT orig_set_sensor_thresholds(struct oh_handler_state *handler,
				    struct ohoi_sensor_info *sensor_info,
				    const SaHpiSensorThresholdsT *thres)
{
	struct ohoi_handler	*ipmi_handler =
					(struct ohoi_handler *)(handler->data);
        struct ohoi_sensor_thresholds	thres_data;
        int				rv;
		ipmi_sensor_id_t		sensor_id;

	sensor_id = sensor_info->info.orig_sensor_info.sensor_id;

	memset(&thres_data, 0, sizeof(thres_data));
	thres_data.thrhlds = malloc(ipmi_thresholds_size()); 
	if (thres_data.thrhlds == NULL) {
		err("could not alloc memory");
		return SA_ERR_HPI_OUT_OF_MEMORY;
	}
       
	thres_data.sensor_thres = *thres;
        
        rv = ipmi_sensor_pointer_cb(sensor_id,
				    set_sensor_thresholds,
				    &thres_data);
		
        if (rv) {
		err("Unable to convert sensor_id to pointer");
		free(thres_data.thrhlds);
                return SA_ERR_HPI_INVALID_CMD;
	}

	rv = ohoi_loop_until(is_get_sensor_thresholds_done, 
                               &thres_data, OHOI_TIMEOUT, ipmi_handler);
	free(thres_data.thrhlds);
	if (rv != SA_OK) {
		return rv;
	}
	if (thres_data.rvalue) {
		return thres_data.rvalue;
	}
	return SA_OK;
}

SaErrorT ohoi_set_sensor_thresholds(void *hnd,
				    struct ohoi_sensor_info *sensor_info,
				    const SaHpiSensorThresholdsT *thres)
{
	struct oh_handler_state *handler = (struct oh_handler_state *)hnd;

	if (sensor_info->ohoii.set_sensor_thresholds == NULL) {
		return SA_ERR_HPI_INVALID_CMD;
	}

	return sensor_info->ohoii.set_sensor_thresholds(handler,
						sensor_info, thres);
}




		/*  SENSOR ENABLE  */


static void set_sensor_enable(ipmi_sensor_t *sensor,
			      void          *cb_data)
{
//	SaHpiBoolT *enable = cb_data;
//	ipmi_sensor_set_ignore_if_no_entity(sensor, *enable);
	return;

}

int ohoi_set_sensor_enable(ipmi_sensor_id_t sensor_id,
			   SaHpiBoolT   enable,
			   void *cb_data)
{
	SaErrorT rv;
        rv = ipmi_sensor_pointer_cb(sensor_id,
				    set_sensor_enable,
		  		    &enable);
	if (rv) {
		err("Unable to convert sensor_id to pointer");
                return SA_ERR_HPI_INVALID_CMD;
        }
	return SA_OK;
}



			/*    SENSOR EVENT ENABLE MASK    */
			

static void convert_to_ohoi_event_states(ipmi_sensor_t	*sensor,
			ipmi_event_state_t	*state,
			SaHpiEventStateT *assert,
			SaHpiEventStateT *deassert)
{
	int i;

	*assert = 0;
	*deassert = 0;
	if(ipmi_sensor_get_event_reading_type(sensor) !=
			IPMI_EVENT_READING_TYPE_THRESHOLD) {
		for (i = 0; i < 15; i++) {
			if (ipmi_is_discrete_event_set(state, i,
					IPMI_ASSERTION)) {
				*assert |= (1 << i);
			}
			if (ipmi_is_discrete_event_set(state, i,
					IPMI_DEASSERTION)) {
				*deassert |= (1 << i);
			}

		}
		return;
	}
	
	// threshold sensor
	if (ipmi_is_threshold_event_set(state,
			IPMI_LOWER_NON_CRITICAL, IPMI_GOING_LOW, 
			IPMI_ASSERTION)) {
		*assert |= SAHPI_ES_LOWER_MINOR;
	}
	if (ipmi_is_threshold_event_set(state,
			IPMI_LOWER_NON_CRITICAL, IPMI_GOING_HIGH, 
			IPMI_ASSERTION)) {
		*deassert |= SAHPI_ES_LOWER_MINOR;
	}
	if (ipmi_is_threshold_event_set(state,
			IPMI_LOWER_NON_CRITICAL, IPMI_GOING_LOW, 
			IPMI_DEASSERTION)) {
		*deassert |= SAHPI_ES_LOWER_MINOR;
	}	
	if (ipmi_is_threshold_event_set(state,
			IPMI_LOWER_NON_CRITICAL, IPMI_GOING_HIGH, 
			IPMI_DEASSERTION)) {
		*assert |= SAHPI_ES_LOWER_MINOR;
	}

	if (ipmi_is_threshold_event_set(state,
			IPMI_LOWER_CRITICAL, IPMI_GOING_LOW, 
			IPMI_ASSERTION)) {
		*assert |= SAHPI_ES_LOWER_MAJOR;
	}
	if (ipmi_is_threshold_event_set(state,
			IPMI_LOWER_CRITICAL, IPMI_GOING_HIGH, 
			IPMI_ASSERTION)) {
		*deassert |= SAHPI_ES_LOWER_MAJOR;
	}
	if (ipmi_is_threshold_event_set(state,
			IPMI_LOWER_CRITICAL, IPMI_GOING_LOW, 
			IPMI_DEASSERTION)) {
		*deassert |= SAHPI_ES_LOWER_MAJOR;
	}	
	if (ipmi_is_threshold_event_set(state,
			IPMI_LOWER_CRITICAL, IPMI_GOING_HIGH, 
			IPMI_DEASSERTION)) {
		*assert |= SAHPI_ES_LOWER_MAJOR;
	}

	if (ipmi_is_threshold_event_set(state,
			IPMI_LOWER_NON_RECOVERABLE, IPMI_GOING_LOW, 
			IPMI_ASSERTION)) {
		*assert |= SAHPI_ES_LOWER_CRIT;
	}
	if (ipmi_is_threshold_event_set(state,
			IPMI_LOWER_NON_RECOVERABLE, IPMI_GOING_HIGH, 
			IPMI_ASSERTION)) {
		*deassert |= SAHPI_ES_LOWER_CRIT;
	}
	if (ipmi_is_threshold_event_set(state,
			IPMI_LOWER_NON_RECOVERABLE, IPMI_GOING_LOW, 
			IPMI_DEASSERTION)) {
		*deassert |= SAHPI_ES_LOWER_CRIT;
	}	
	if (ipmi_is_threshold_event_set(state,
			IPMI_LOWER_NON_RECOVERABLE, IPMI_GOING_HIGH, 
			IPMI_DEASSERTION)) {
		*assert |= SAHPI_ES_LOWER_CRIT;
	}	


	if (ipmi_is_threshold_event_set(state,
			IPMI_UPPER_NON_CRITICAL, IPMI_GOING_LOW, 
			IPMI_ASSERTION)) {
		*deassert |= SAHPI_ES_UPPER_MINOR;
	}
	if (ipmi_is_threshold_event_set(state,
			IPMI_UPPER_NON_CRITICAL, IPMI_GOING_HIGH, 
			IPMI_ASSERTION)) {
		*assert |= SAHPI_ES_UPPER_MINOR;
	}
	if (ipmi_is_threshold_event_set(state,
			IPMI_UPPER_NON_CRITICAL, IPMI_GOING_LOW, 
			IPMI_DEASSERTION)) {
		*assert |= SAHPI_ES_UPPER_MINOR;
	}	
	if (ipmi_is_threshold_event_set(state,
			IPMI_UPPER_NON_CRITICAL, IPMI_GOING_HIGH, 
			IPMI_DEASSERTION)) {
		*deassert |= SAHPI_ES_UPPER_MINOR;
	}

	if (ipmi_is_threshold_event_set(state,
			IPMI_UPPER_CRITICAL, IPMI_GOING_LOW, 
			IPMI_ASSERTION)) {
		*deassert |= SAHPI_ES_UPPER_MAJOR;
	}
	if (ipmi_is_threshold_event_set(state,
			IPMI_UPPER_CRITICAL, IPMI_GOING_HIGH, 
			IPMI_ASSERTION)) {
		*assert |= SAHPI_ES_UPPER_MAJOR;
	}
	if (ipmi_is_threshold_event_set(state,
			IPMI_UPPER_CRITICAL, IPMI_GOING_LOW, 
			IPMI_DEASSERTION)) {
		*assert |= SAHPI_ES_UPPER_MAJOR;
	}	
	if (ipmi_is_threshold_event_set(state,
			IPMI_UPPER_CRITICAL, IPMI_GOING_HIGH, 
			IPMI_DEASSERTION)) {
		*deassert |= SAHPI_ES_UPPER_MAJOR;
	}

	if (ipmi_is_threshold_event_set(state,
			IPMI_UPPER_NON_RECOVERABLE, IPMI_GOING_LOW, 
			IPMI_ASSERTION)) {
		*deassert |= SAHPI_ES_UPPER_CRIT;
	}
	if (ipmi_is_threshold_event_set(state,
			IPMI_UPPER_NON_RECOVERABLE, IPMI_GOING_HIGH, 
			IPMI_ASSERTION)) {
		*assert |= SAHPI_ES_UPPER_CRIT;
	}
	if (ipmi_is_threshold_event_set(state,
			IPMI_UPPER_NON_RECOVERABLE, IPMI_GOING_LOW, 
			IPMI_DEASSERTION)) {
		*assert |= SAHPI_ES_UPPER_CRIT;
	}	
	if (ipmi_is_threshold_event_set(state,
			IPMI_UPPER_NON_RECOVERABLE, IPMI_GOING_HIGH, 
			IPMI_DEASSERTION)) {
		*deassert |= SAHPI_ES_UPPER_CRIT;
	}	

}

static void event_enable_masks_read(ipmi_sensor_t	*sensor,
			           int 			err,
			           ipmi_event_state_t	*state,
			           void			*cb_data)
{
	struct ohoi_sensor_event_enable_masks *p = cb_data;
	int 				rv;

	p->done = 1;

	if (err) {
		err("Sensor event enable reading error 0x%x", err);
			OHOI_MAP_ERROR(p->rvalue, err);
		return;
	}

	p->enable = SAHPI_FALSE;

	rv = ipmi_event_state_get_events_enabled(state);
        if (rv)
		p->enable = SAHPI_TRUE;

	convert_to_ohoi_event_states(sensor, state,
		&p->assert, &p->deassert);
}

static void get_sensor_event_enable_masks(ipmi_sensor_t *sensor, 
                                          void          *cb_data)
{
	struct ohoi_sensor_event_enable_masks *enable_data;
	int rv;
	
        enable_data = cb_data;

	if (ignore_sensor(sensor)) {
		err("sensor is ignored");
		enable_data->done = 1;
		enable_data->rvalue = SA_ERR_HPI_NOT_PRESENT;
		return;
	}	

	if ((ipmi_sensor_get_event_support(sensor) ==
			IPMI_EVENT_SUPPORT_PER_STATE)||
	   (ipmi_sensor_get_event_support(sensor) ==
				 IPMI_EVENT_SUPPORT_ENTIRE_SENSOR)){
		rv = ipmi_sensor_get_event_enables(sensor,
				event_enable_masks_read, enable_data);
		if (rv) {
			err("Unable to sensor event enable: 0x%x\n", rv);
			enable_data->rvalue = SA_ERR_HPI_INTERNAL_ERROR;
			return;
		}
	} else {
                err("Sensor do not support event");
		enable_data->assert = 0;
		enable_data->deassert = 0;
		enable_data->enable = SAHPI_FALSE;
		enable_data->rvalue = SA_OK;
		enable_data->done = 1;       
        }
}

static int insert_events_to_ipmi_event_state(
		ipmi_sensor_t		*sensor,
		ipmi_event_state_t	*state,
		SaHpiEventStateT	a_mask,
		SaHpiEventStateT	d_mask,
		unsigned int		a_sup,
		unsigned int		d_sup)
{
	int i;  

	if (ipmi_sensor_get_event_support(sensor) !=
			IPMI_EVENT_SUPPORT_PER_STATE) {
		return 0;
	}

	if (ipmi_sensor_get_event_reading_type(sensor) !=
			IPMI_EVENT_READING_TYPE_THRESHOLD) {
		// discrete sensor. map states 1:1
		if ((a_mask &~a_sup) || (d_mask & ~d_sup)) {
			return 1;
		}
		for (i = 0; i < 15; i++) {
			if (a_mask & (1 << i)) {
				ipmi_discrete_event_set(state, i,
					IPMI_ASSERTION);
			}
			if (d_mask & (1 << i)) {
				ipmi_discrete_event_set(state, i,
					IPMI_DEASSERTION);
			}
		}
		return 0;
	}

	// threhold sensor;
		// set assertion mask
	if (a_mask & SAHPI_ES_LOWER_MINOR) {
		if (a_sup & OHOI_THS_LMINL) {
			ipmi_threshold_event_set(state, IPMI_LOWER_NON_CRITICAL,
				IPMI_GOING_LOW, IPMI_ASSERTION);
		} else if (d_sup & OHOI_THS_LMINH) {
			ipmi_threshold_event_set(state, IPMI_LOWER_NON_CRITICAL,
				IPMI_GOING_HIGH, IPMI_DEASSERTION);
		} else {
			return 1;
		}
	}

	if (a_mask & SAHPI_ES_LOWER_MAJOR) {
		if (a_sup & OHOI_THS_LMAJL) {
			ipmi_threshold_event_set(state, IPMI_LOWER_CRITICAL,
				IPMI_GOING_LOW, IPMI_ASSERTION);
		} else if (d_sup & OHOI_THS_LMAJH) {
			ipmi_threshold_event_set(state, IPMI_LOWER_CRITICAL,
				IPMI_GOING_HIGH, IPMI_DEASSERTION);
		} else {
			return 1;
		}
	}

	if (a_mask & SAHPI_ES_LOWER_CRIT) {
		if (a_sup & OHOI_THS_LCRTL) {
			ipmi_threshold_event_set(state,
				IPMI_LOWER_NON_RECOVERABLE, IPMI_GOING_LOW,
				IPMI_ASSERTION);
		} else if (d_sup & OHOI_THS_LCRTH) {
			ipmi_threshold_event_set(state,
				IPMI_LOWER_NON_RECOVERABLE, IPMI_GOING_HIGH,
				IPMI_DEASSERTION);
		} else {
			return 1;
		}
	}


	if (a_mask & SAHPI_ES_UPPER_MINOR) {
		if (a_sup & OHOI_THS_UMINH) {
			ipmi_threshold_event_set(state,
				IPMI_UPPER_NON_CRITICAL, IPMI_GOING_HIGH,
				IPMI_ASSERTION);
		} else if (d_sup & OHOI_THS_UMINL) {
			ipmi_threshold_event_set(state,
				IPMI_UPPER_NON_CRITICAL, IPMI_GOING_LOW,
				IPMI_DEASSERTION);
		} else {
			return 1;
		}
	}

	if (a_mask & SAHPI_ES_UPPER_MAJOR) {
		if (a_sup & OHOI_THS_UMAJH) {
			ipmi_threshold_event_set(state, IPMI_UPPER_CRITICAL,
				IPMI_GOING_HIGH, IPMI_ASSERTION);
		} else if (d_sup & OHOI_THS_UMAJL) {
			ipmi_threshold_event_set(state, IPMI_UPPER_CRITICAL,
				IPMI_GOING_LOW, IPMI_DEASSERTION);
		} else {
			return 1;
		}
	}

	if (a_mask & SAHPI_ES_UPPER_CRIT) {
		if (a_sup & OHOI_THS_UCRTH) {
			ipmi_threshold_event_set(state,
				IPMI_UPPER_NON_RECOVERABLE, IPMI_GOING_HIGH,
				IPMI_ASSERTION);
		} else if (d_sup & OHOI_THS_UCRTL) {
			ipmi_threshold_event_set(state, IPMI_GOING_LOW,
				IPMI_UPPER_NON_RECOVERABLE, IPMI_DEASSERTION);
		} else {
			return 1;
		}
	}

		// set deassertion mask
	if (d_mask & SAHPI_ES_LOWER_MINOR) {
		if (d_sup & OHOI_THS_LMINL) {
			ipmi_threshold_event_set(state,
				IPMI_LOWER_NON_CRITICAL, IPMI_GOING_LOW,
				IPMI_DEASSERTION);
		} else if (a_sup & OHOI_THS_LMINH) {
			ipmi_threshold_event_set(state,
				IPMI_LOWER_NON_CRITICAL, IPMI_GOING_HIGH,
				IPMI_ASSERTION);
		} else {
			return 1;
		}
	}

	if (d_mask & SAHPI_ES_LOWER_MAJOR) {
		if (d_sup & OHOI_THS_LMAJL) {
			ipmi_threshold_event_set(state,
				IPMI_LOWER_CRITICAL, IPMI_GOING_LOW,
				IPMI_DEASSERTION);
		} else if (a_sup & OHOI_THS_LMAJH) {
			ipmi_threshold_event_set(state, IPMI_GOING_HIGH,
				IPMI_LOWER_CRITICAL, IPMI_ASSERTION);
		} else {
			return 1;
		}
	}

	if (d_mask & SAHPI_ES_LOWER_CRIT) {
		if (d_sup & OHOI_THS_LCRTL) {
			ipmi_threshold_event_set(state,
				IPMI_LOWER_NON_RECOVERABLE, IPMI_GOING_LOW,
				IPMI_DEASSERTION);
		} else if (a_sup & OHOI_THS_LCRTH) {
			ipmi_threshold_event_set(state,
				IPMI_LOWER_NON_RECOVERABLE, IPMI_GOING_HIGH,
				IPMI_ASSERTION);
		} else {
			return 1;
		}
	}


	if (d_mask & SAHPI_ES_UPPER_MINOR) {
		if (d_sup & OHOI_THS_UMINH) {
			ipmi_threshold_event_set(state,
				IPMI_UPPER_NON_CRITICAL, IPMI_GOING_HIGH,
				IPMI_DEASSERTION);
		} else if (a_sup & OHOI_THS_UMINL) {
			ipmi_threshold_event_set(state,
				IPMI_UPPER_NON_CRITICAL, IPMI_GOING_LOW,
				IPMI_ASSERTION);
		} else {
			return 1;
		}
	}

	if (d_mask & SAHPI_ES_UPPER_MAJOR) {
		if (d_sup & OHOI_THS_UMAJH) {
			ipmi_threshold_event_set(state,
				IPMI_UPPER_CRITICAL, IPMI_GOING_HIGH,
				IPMI_DEASSERTION);
		} else if (a_sup & OHOI_THS_UMAJL) {
			ipmi_threshold_event_set(state, IPMI_GOING_LOW,
				IPMI_UPPER_CRITICAL, IPMI_ASSERTION);
		} else {
			return 1;
		}
	}

	if (d_mask & SAHPI_ES_UPPER_CRIT) {
		if (d_sup & OHOI_THS_UCRTH) {
			ipmi_threshold_event_set(state,
				IPMI_UPPER_NON_RECOVERABLE, IPMI_GOING_HIGH,
				IPMI_DEASSERTION);
		} else if (a_sup & OHOI_THS_UCRTL) {
			ipmi_threshold_event_set(state,
				IPMI_UPPER_NON_RECOVERABLE, IPMI_GOING_LOW,
				IPMI_ASSERTION);
		} else {
			return 1;
		}
	}

	return 0;
}


static void mask_set_data(ipmi_sensor_t *sensor, int err, void *cb_data)
{
	struct ohoi_sensor_event_enable_masks *info = cb_data;
	if (err) {
		err("err = 0x%x", err);
		info->rvalue = SA_ERR_HPI_INVALID_CMD;
	}
	info->done = 1;
}
				
static void set_sensor_event_enable_masks(ipmi_sensor_t      *sensor,
                                    void               *cb_data)
{
	struct ohoi_sensor_event_enable_masks *enable_data = cb_data;
	int			rv;
	ipmi_event_state_t	*info = enable_data->states;

	if (ignore_sensor(sensor)) {
		err("sensor is ignored");
		enable_data->done = 1;
		enable_data->rvalue = SA_ERR_HPI_NOT_PRESENT;
		return;
	}

	ipmi_event_state_init(info);
	ipmi_event_state_set_events_enabled(info,
		(enable_data->enable == SAHPI_TRUE) ? 1 : 0);
	if (insert_events_to_ipmi_event_state(sensor, info,
			enable_data->assert, enable_data->deassert,
			enable_data->a_support, enable_data->d_support)) {
		err("Attempt to set not supported event 0x%x/0x%x",
			enable_data->assert, enable_data->deassert);
		enable_data->done = 1;
		enable_data->rvalue = SA_ERR_HPI_INVALID_DATA;
		return;
	}


	rv = ipmi_sensor_set_event_enables(sensor, info, mask_set_data,
					   enable_data);
	if (rv) {
		err("Unable to sensor event enable = %d", rv);
		enable_data->done = 1;
		if (rv == EINVAL) {
			// invalid event in mask for this sensor */
			enable_data->rvalue = SA_ERR_HPI_INVALID_DATA;
		} else {	
			enable_data->rvalue = SA_ERR_HPI_INTERNAL_ERROR;
		}
	}
}

SaErrorT orig_get_sensor_event_enable(struct oh_handler_state *handler,
				      struct ohoi_sensor_info *sensor_info,
				      SaHpiBoolT   *enable,
				      SaHpiEventStateT  *assert,
				      SaHpiEventStateT  *deassert)
{
        SaErrorT		rv;
	struct ohoi_handler	*ipmi_handler =
					(struct ohoi_handler *)(handler->data);
	struct ohoi_sensor_event_enable_masks	enable_data;
		ipmi_sensor_id_t		sensor_id;

	sensor_id = sensor_info->info.orig_sensor_info.sensor_id;
        
	memset(&enable_data, 0, sizeof(enable_data));
        
        rv = ipmi_sensor_pointer_cb(sensor_id,
				    get_sensor_event_enable_masks,
				    &enable_data);
        if (rv) {
		err("Unable to convert sensor_id to pointer");
                return SA_ERR_HPI_INVALID_CMD;
        }
        
        rv = ohoi_loop(&enable_data.done, ipmi_handler);

	if (rv)
		return rv;
	if (enable_data.rvalue)
		return enable_data.rvalue;

	*enable = enable_data.enable;
	*assert = enable_data.assert & 0x7fff;
	*deassert = enable_data.deassert & 0x7fff;

	return SA_OK;

}


SaErrorT ohoi_get_sensor_event_enable(void *hnd,
				      struct ohoi_sensor_info *sensor_info,
				      SaHpiBoolT   *enable,
				      SaHpiEventStateT  *assert,
				      SaHpiEventStateT  *deassert)
{
	struct oh_handler_state *handler = (struct oh_handler_state *)hnd;

	if (sensor_info->ohoii.get_sensor_event_enable == NULL) {
		return SA_ERR_HPI_INVALID_CMD;
	}

	return sensor_info->ohoii.get_sensor_event_enable(handler,
					sensor_info, enable,
					assert, deassert);
}

SaErrorT orig_set_sensor_event_enable(struct oh_handler_state *handler,
				      struct ohoi_sensor_info *sensor_info,
				      SaHpiBoolT enable,
				      SaHpiEventStateT assert,
				      SaHpiEventStateT deassert,
				      unsigned int a_supported,
				      unsigned int d_supported)
{
        int			rv;
	struct ohoi_handler	*ipmi_handler =
					(struct ohoi_handler *)(handler->data);
	struct ohoi_sensor_event_enable_masks	enable_data;
		ipmi_sensor_id_t		sensor_id;

	sensor_id = sensor_info->info.orig_sensor_info.sensor_id;

	memset(&enable_data, 0, sizeof(enable_data));
	enable_data.states = malloc(ipmi_event_state_size());
	if (enable_data.states == NULL) {
		err("out of memory");
		return SA_ERR_HPI_OUT_OF_MEMORY;
	}
	enable_data.enable = enable;
	enable_data.assert = assert;
	enable_data.deassert = deassert;
	enable_data.a_support = a_supported;
	enable_data.d_support = d_supported;
	rv = ipmi_sensor_pointer_cb(sensor_id,
				    set_sensor_event_enable_masks,
		  		    &enable_data);
	if (rv) {
		err("Unable to convert sensor_id to pointer");
		free(enable_data.states);
		return SA_ERR_HPI_INVALID_CMD;
	}       
	rv = ohoi_loop(&enable_data.done, ipmi_handler);
	free(enable_data.states);      
	if (rv) {
		return rv;
	}
	if (enable_data.rvalue)
		return enable_data.rvalue;

	return SA_OK;
}


SaErrorT ohoi_set_sensor_event_enable(void *hnd,
				      struct ohoi_sensor_info *sensor_info,
				      SaHpiBoolT enable,
				      SaHpiEventStateT assert,
				      SaHpiEventStateT deassert,
				      unsigned int a_supported,
				      unsigned int d_supported)
{
	struct oh_handler_state *handler = (struct oh_handler_state *)hnd;

	if (sensor_info->ohoii.set_sensor_event_enable == NULL) {
		return SA_ERR_HPI_INVALID_CMD;
	}

	return sensor_info->ohoii.set_sensor_event_enable(handler,
					sensor_info, enable,
					assert, deassert,
					a_supported, d_supported);
}



