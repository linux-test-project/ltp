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
 */

#include "ipmi.h"

/* 
 * Use for getting sensor data functions
 */
struct ohoi_sensor_reading {
	SaHpiSensorReadingT	*sensor_reading;
	int			done;
};

/*
 * Use for getting/setting sensor threadholds
 */
struct ohoi_sensor_thresholds {
	SaHpiSensorThresholdsT	*sensor_thres;
	int			thres_done;
        int                     hyster_done;
};

/*
 * Use for enabling/disabling sensor
 */
struct ohoi_sensor_enables {
	SaHpiSensorEvtEnablesT	*sensor_enables;
	int			done;	
};

static int ignore_sensor(ipmi_sensor_t *sensor)
{
        ipmi_entity_t *ent;

        if ( ipmi_sensor_get_ignore_if_no_entity(sensor) )
                return 0;

        ent = ipmi_sensor_get_entity(sensor);

        if (ent && ipmi_entity_is_present(ent))
                return 0;

        return 1;
}

static void sensor_read_states(ipmi_sensor_t *sensor,
                               int           err,
                               ipmi_states_t *states,
                               void          *cb_data)
{
	struct ohoi_sensor_reading *p = cb_data;

        p->sensor_reading->ValuesPresent = SAHPI_SRF_EVENT_STATE;

	if (ipmi_is_event_messages_enabled(states))
		p->sensor_reading->EventStatus.SensorStatus |=
			SAHPI_SENSTAT_EVENTS_ENABLED;

	if (ipmi_is_sensor_scanning_enabled(states))
		p->sensor_reading->EventStatus.SensorStatus |= 
			SAHPI_SENSTAT_EVENTS_ENABLED;

	if (ipmi_is_initial_update_in_progress(states))
		p->sensor_reading->EventStatus.SensorStatus |=
			SAHPI_SENSTAT_BUSY;

	p->sensor_reading->EventStatus.EventStatus = states->__states;
	
	p->done = 1;
}

static void sensor_read(ipmi_sensor_t			*sensor,
			int 				err,
			enum ipmi_value_present_e	value_present,
			unsigned int 			raw_val,
			double 				val,
			ipmi_states_t 			*states,
			void 				*cb_data)
{
	struct ohoi_sensor_reading *p = cb_data;

	if (err) {
		dbg("sensor reading error");
		p->done = 1;
		return;
	}	
	
	if (value_present == IPMI_BOTH_VALUES_PRESENT) {
		p->sensor_reading->ValuesPresent = SAHPI_SRF_RAW |
						   SAHPI_SRF_INTERPRETED;
		p->sensor_reading->Raw = (SaHpiUint32T)raw_val;
		p->sensor_reading->Interpreted.Type =
			SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32;
		p->sensor_reading->Interpreted.Value.SensorFloat32 = 
			(SaHpiFloat32T)val;
	}
	else if (value_present == IPMI_RAW_VALUE_PRESENT) {
		p->sensor_reading->ValuesPresent = SAHPI_SRF_RAW;
		p->sensor_reading->Raw = (SaHpiUint32T)raw_val;
	}
	else
		p->sensor_reading->ValuesPresent = 0;
	
	p->done = 1;
}

static void get_sensor_data(ipmi_sensor_t *sensor, void *cb_data)
{
	struct ohoi_sensor_reading *reading_data;	
	int rv;	

        reading_data = cb_data;
        
	if (ignore_sensor(sensor)) {
		dbg("Sensor is not present, ignored");
		return;
	}	

	if (ipmi_sensor_get_event_reading_type(sensor) ==
			IPMI_EVENT_READING_TYPE_THRESHOLD) {
		rv = ipmi_reading_get(sensor, sensor_read, reading_data);
		if (rv) {
			dbg("Unable to get sensor reading: %s\n",
                            strerror( rv ) );
			return;
		}
        } else {
                rv = ipmi_states_get(sensor, sensor_read_states, reading_data);
		if (rv) {
			dbg("Unable to get sensor reading states: %s\n",
                            strerror( rv ) );
			return;
                }
        }

}

int ohoi_get_sensor_data(ipmi_sensor_id_t sensor_id, 
                         SaHpiSensorReadingT *data)
{
	struct ohoi_sensor_reading reading_data;	
        int rv;
        
        memset(data, 0, sizeof(*data));
        reading_data.sensor_reading     = data;

        rv = ipmi_sensor_pointer_cb(sensor_id, 
                                          get_sensor_data,
                                          &reading_data);
        if (rv) {
                dbg("Unable to convert sensor_id to pointer");
                return SA_ERR_HPI_INVALID;
        }
        
        return ohoi_loop(&reading_data.done);
}

static void thres_get(ipmi_sensor_t		*sensor,
		      ipmi_thresholds_t		*th,
		      unsigned int		event,
		      SaHpiSensorReadingT	*thres)
{
	int val;
	
	ipmi_sensor_threshold_readable(sensor, event, &val);
	if (val) {
		thres->ValuesPresent = SAHPI_SRF_INTERPRETED;
		thres->Interpreted.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32;
		thres->Interpreted.Value.SensorFloat32 = (SaHpiFloat32T)
			th->vals[event].val;
	}
}

static void thresholds_read(ipmi_sensor_t	*sensor,
			    int			err,
			    ipmi_thresholds_t	*th,
			    void		*cb_data)
{
	struct ohoi_sensor_thresholds *p = cb_data;

	if (err) {
		dbg("sensor thresholds reading error");
		p->thres_done = 1;
		return;
	}
	
	thres_get(sensor, th, IPMI_LOWER_NON_CRITICAL,
		  &p->sensor_thres->LowMinor);
	thres_get(sensor, th, IPMI_LOWER_CRITICAL, &p->sensor_thres->LowMajor);
	thres_get(sensor, th, IPMI_LOWER_NON_RECOVERABLE,
		  &p->sensor_thres->LowCritical);
	thres_get(sensor, th, IPMI_UPPER_NON_CRITICAL,
		  &p->sensor_thres->UpMinor);
	thres_get(sensor, th, IPMI_UPPER_CRITICAL, &p->sensor_thres->UpMajor);
	thres_get(sensor, th, IPMI_UPPER_NON_RECOVERABLE,
		  &p->sensor_thres->UpCritical);

	p->thres_done = 1;
}

static SaErrorT get_thresholds(ipmi_sensor_t				*sensor,
			  struct ohoi_sensor_thresholds	*thres_data)
{
	int		rv;
		
	rv = ipmi_thresholds_get(sensor, thresholds_read, thres_data);
	if (rv) 
		dbg("Unable to get sensor thresholds: 0x%x\n", rv);

	return (rv? SA_ERR_HPI_INVALID : SA_OK);
}

static void hysteresis_read(ipmi_sensor_t	*sensor,
			    int			err,
			    unsigned int	positive_hysteresis,
			    unsigned int 	negative_hysteresis,
			    void 		*cb_data)
{
	struct ohoi_sensor_thresholds *p = cb_data;
	
	if (err) {
		dbg("sensor hysteresis reading error");
		p->hyster_done = 1;
		return;		
	}
	
	p->sensor_thres->PosThdHysteresis.ValuesPresent =
		SAHPI_SRF_RAW;	
	p->sensor_thres->PosThdHysteresis.Raw = positive_hysteresis;
	p->sensor_thres->NegThdHysteresis.ValuesPresent =
		SAHPI_SRF_RAW;
	p->sensor_thres->PosThdHysteresis.Raw = negative_hysteresis;
	p->hyster_done = 1;
}

static SaErrorT get_hysteresis(ipmi_sensor_t			*sensor,
			  struct ohoi_sensor_thresholds	*thres_data)
{
	int		rv;
	
	rv = ipmi_sensor_get_hysteresis(sensor, hysteresis_read, thres_data);
        if (rv)
                dbg("Unable to get sensor hysteresis: 0x%x\n", rv);
        
	return (rv? SA_ERR_HPI_INVALID : SA_OK);
}

static void get_sensor_thresholds(ipmi_sensor_t *sensor, 
                                  void          *cb_data)
{
	struct ohoi_sensor_thresholds *thres_data;
	int rv;
	
        thres_data = cb_data;
	if (ignore_sensor(sensor)) {
                dbg("ENTITY_NOT_PRESENT");
		return;
	}	
	
	if (ipmi_sensor_get_event_reading_type(sensor) ==
			IPMI_EVENT_READING_TYPE_THRESHOLD) {
		if (ipmi_sensor_get_threshold_access(sensor) ==
				IPMI_EVENT_SUPPORT_NONE)
			dbg("sensor doesn't support threshold read");
		else {
			rv = get_thresholds(sensor, thres_data);
			if (rv < 0) {
                                dbg("Unable to get sensor thresholds");
				return;
                        }
		}
					
		rv = ipmi_sensor_get_hysteresis_support(sensor);
		if (rv == IPMI_HYSTERESIS_SUPPORT_NONE) {
			/* I'm zeroing them so we return but invalid data FIXME? */
			thres_data->sensor_thres->PosThdHysteresis.ValuesPresent = 0;
			thres_data->sensor_thres->NegThdHysteresis.ValuesPresent = 0;
                        thres_data->hyster_done = 1; /* read no more */
			return;
		} else {
			if (rv == IPMI_HYSTERESIS_SUPPORT_READABLE ||
					rv == IPMI_HYSTERESIS_SUPPORT_SETTABLE) { 
				rv = get_hysteresis(sensor, thres_data);
				if (rv < 0)
					dbg("failed to get hysteresis");
			}
		}
		
	} else {
		dbg("Not threshold sensor!");
        }
	
	return;
}

static int is_get_sensor_thresholds_done(const void *cb_data)
{
        const struct ohoi_sensor_thresholds *thres_data;
        
        thres_data = cb_data;
        return (thres_data->thres_done && thres_data->hyster_done);
}

int ohoi_get_sensor_thresholds(ipmi_sensor_id_t sensor_id, SaHpiSensorThresholdsT *thres)
{
	struct ohoi_sensor_thresholds	thres_data;
        int rv;
		
        memset(thres, 0, sizeof(*thres));
        
        thres_data.sensor_thres = thres;
        thres_data.thres_done   = 0;
        thres_data.hyster_done  = 0;
                
        rv = ipmi_sensor_pointer_cb(sensor_id,
                                          get_sensor_thresholds,
                                          &thres_data);
        if (rv) {
                dbg("Unable to convert sensor id into pointer");
                return SA_ERR_HPI_INVALID;
        }

        return ohoi_loop_until(is_get_sensor_thresholds_done, 
                               &thres_data, 5);
}

static void set_data(ipmi_sensor_t *sensor, int err, void *cb_data)
{
	int *done = cb_data;
	
	if (err)
		dbg("Something wrong in setting thresholds");
	*done = 1;
}

static int thres_cpy(ipmi_sensor_t			*sensor, 
		      const SaHpiSensorReadingT		reading,
		      unsigned int			event,
		      ipmi_thresholds_t			*info) 
{
	double	tmp;
	int	val;
	int	rv;

	ipmi_sensor_threshold_settable(sensor, event, &val);
	if (!val)
	       return 0;
	
	if (reading.ValuesPresent & SAHPI_SRF_RAW) {
		rv = ipmi_sensor_convert_from_raw(sensor, reading.Raw, &tmp);
		if (rv < 0) {
			dbg("Invalid raw value");
			return -1;
		}
		info->vals[event].status = 1;
		info->vals[event].val = tmp;
	}
	else if (reading.ValuesPresent & SAHPI_SRF_INTERPRETED) {
		if (reading.Interpreted.Type ==
		    SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32) {
		info->vals[event].status = 1;
		info->vals[event].val = reading.Interpreted.Value.SensorFloat32;
		}
		else {
			dbg("Invalid input thresholds");
			return -1;
		}
	}

	return 0;
}

static int init_thresholeds_info(ipmi_sensor_t			*sensor, 
				  const SaHpiSensorThresholdsT	*thres,
				  ipmi_thresholds_t		*info)
{
	int rv;
	
	rv = thres_cpy(sensor, thres->LowMinor, IPMI_LOWER_NON_CRITICAL, info);
	if (rv < 0)
		return -1;
	
	rv = thres_cpy(sensor, thres->LowMajor, IPMI_LOWER_CRITICAL, info);
	if (rv < 0)
		return -1;
	
	rv = thres_cpy(sensor, thres->LowCritical, IPMI_LOWER_NON_RECOVERABLE,
		       info);
	if (rv < 0)
		return -1;
	
	rv = thres_cpy(sensor, thres->UpMinor, IPMI_UPPER_NON_CRITICAL, info);
	if (rv < 0)
		return -1;
	
	rv = thres_cpy(sensor, thres->UpMajor, IPMI_UPPER_CRITICAL, info);
	if (rv < 0)
		return -1;
	
	rv = thres_cpy(sensor, thres->UpCritical, IPMI_UPPER_NON_RECOVERABLE,
		       info);
	if (rv < 0)
		return -1;

	return 0;
}

static int set_thresholds(ipmi_sensor_t                 *sensor, 
			  struct ohoi_sensor_thresholds *thres_data)
{
	ipmi_thresholds_t	info;
	int			rv;	
	
	memset(&info, 0, sizeof(info));
	rv = init_thresholeds_info(sensor, thres_data->sensor_thres, &info);
	if (rv < 0)
		return -1;
	
	rv = ipmi_thresholds_set(sensor, &info, set_data, 
                                 &thres_data->thres_done);
	if (rv) {
		dbg("Unable to set sensor thresholds: 0x%x\n", rv);
		return -1;
	}

	return rv;
}

static int set_hysteresis(ipmi_sensor_t	                *sensor,
			  struct ohoi_sensor_thresholds	*thres_data)
{
	int			rv;	
	double			tmp;
	unsigned int		pos, neg;
	SaHpiSensorReadingT	pos_reading 
                = thres_data->sensor_thres->PosThdHysteresis;
	SaHpiSensorReadingT	neg_reading 
                = thres_data->sensor_thres->NegThdHysteresis;	
	
	if (pos_reading.ValuesPresent & SAHPI_SRF_RAW)
		pos = pos_reading.Raw;
	else if (pos_reading.ValuesPresent & SAHPI_SRF_INTERPRETED) {
		if (pos_reading.Interpreted.Type !=
		    SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32) {
			tmp = pos_reading.Interpreted.Value.SensorFloat32;
			ipmi_sensor_convert_to_raw(sensor, ROUND_NORMAL, 
						   tmp, &pos);	
		}
		else {
			dbg("Invalid input thresholds");
			return -1;
		}
	}
	
	if (neg_reading.ValuesPresent & SAHPI_SRF_RAW) 
		neg = neg_reading.Raw;
	else if (neg_reading.ValuesPresent & SAHPI_SRF_INTERPRETED) {
		if (neg_reading.Interpreted.Type !=
		    SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32) {
			tmp = neg_reading.Interpreted.Value.SensorFloat32;
			ipmi_sensor_convert_to_raw(sensor, ROUND_NORMAL,
						   tmp, &neg);
		}
		else {
			dbg("Invalid input thresholds");
			return -1;
		}
	}
	
	rv = ipmi_sensor_set_hysteresis(sensor, pos, neg, set_data, 
                                        &thres_data->hyster_done);
	if (rv) {
		dbg("Unable to set sensor thresholds: 0x%x\n", rv);
		return -1;
	}

        return rv;
}

static void set_sensor_thresholds(ipmi_sensor_t *sensor, 
                                  void          *cb_data)
{
        struct ohoi_sensor_thresholds *thres_data;
	int rv;	

        thres_data = cb_data;
	if (ignore_sensor(sensor)) {
		dbg("sensor is ignored");
		return;
	}	
	
	if (ipmi_sensor_get_event_reading_type(sensor) ==
			IPMI_EVENT_READING_TYPE_THRESHOLD) {
		if (ipmi_sensor_get_threshold_access(sensor) ==
		    IPMI_THRESHOLD_ACCESS_SUPPORT_SETTABLE) {
			rv = set_thresholds(sensor, thres_data);
			if (rv < 0) {
                                dbg("Unable to set thresholds");
				return;
                        }
		} else
			dbg("sensor doesn't support threshold set");	

		rv = ipmi_sensor_get_hysteresis_support(sensor);
		if (rv == IPMI_HYSTERESIS_SUPPORT_SETTABLE) { 
			rv = set_hysteresis(sensor, thres_data);
			if (rv < 0) {
                                dbg("Unable to set hysteresis");
				return;
                        }
		} else
			dbg("sensor doesn't support hysteresis set");
	} else
		dbg("Not threshold sensor!");
}

int ohoi_set_sensor_thresholds(ipmi_sensor_id_t		        sensor_id, 
			       const SaHpiSensorThresholdsT     *thres)
{
        struct ohoi_sensor_thresholds thres_data;
        SaHpiSensorThresholdsT tmp_thres; 
        int rv;
        
        tmp_thres = *thres;

        thres_data.sensor_thres = &tmp_thres;
        thres_data.thres_done   = 0;
        thres_data.hyster_done  = 0;
        
        rv = ipmi_sensor_pointer_cb(sensor_id,
                                          set_sensor_thresholds,
                                          &thres_data);
        if (rv) {
                dbg("Unable to convert sensor_id to pointer");
                return SA_ERR_HPI_INVALID;
        }

        return ohoi_loop_until(is_get_sensor_thresholds_done, 
                               &thres_data, 5);
}

static void enables_read(ipmi_sensor_t		*sensor,
			 int 			err,
			 ipmi_event_state_t	*state,
			 void			*cb_data)
{
	struct ohoi_sensor_enables 	*p = cb_data;
	int 				rv;

	if (err) {
		dbg("Sensor event enables reading error");
		p->done = 1;
		return;
	}
	
	rv = ipmi_event_state_get_events_enabled(state);
	if (rv)
		p->sensor_enables->SensorStatus |= SAHPI_SENSTAT_EVENTS_ENABLED;

	rv = ipmi_event_state_get_scanning_enabled(state);
	if (rv)
		p->sensor_enables->SensorStatus |= SAHPI_SENSTAT_SCAN_ENABLED;

	rv = ipmi_event_state_get_busy(state);
	if (rv)
		p->sensor_enables->SensorStatus |= SAHPI_SENSTAT_BUSY;

	p->sensor_enables->AssertEvents = (SaHpiEventStateT)
		state->__assertion_events;
	p->sensor_enables->DeassertEvents = (SaHpiEventStateT)
		state->__deassertion_events;
	
	p->done = 1;
}

static void get_sensor_event_enables(ipmi_sensor_t	*sensor, 
                                     void               *cb_data)
{
	struct ohoi_sensor_enables *enables_data;
	int rv;
	
        enables_data = cb_data;
        
	if (ignore_sensor(sensor)) {
		dbg("sensor is ignored");
                enables_data->done = 1;
		return;
	}	

	if (ipmi_sensor_get_event_support(sensor) != IPMI_EVENT_SUPPORT_NONE) {
		rv = ipmi_sensor_events_enable_get(sensor, enables_read,
					           enables_data);
		if (rv) {
			dbg("Unable to sensor event enables: 0x%x\n", rv);
			return;
		}
	} else {
                dbg("Sensor do not support event");
                enables_data->done = 1;
        }
}

int ohoi_get_sensor_event_enables(ipmi_sensor_id_t      sensor_id, 
			     SaHpiSensorEvtEnablesT	*enables)
{
	struct ohoi_sensor_enables enables_data;
        int rv;
        
        enables_data.sensor_enables     = enables;
        enables_data.done               = 0;
        
        rv = ipmi_sensor_pointer_cb(sensor_id,
                                          get_sensor_event_enables,
                                          &enables_data);
        if (rv) {
                dbg("Unable to convert sensor_id to pointer");
                return SA_ERR_HPI_INVALID;
        }
        
        return ohoi_loop(&enables_data.done);
}

static void set_sensor_event_enables(ipmi_sensor_t      *sensor,
                                     void               *cb_data)
{
        struct ohoi_sensor_enables *enables_data;
	int			rv;
	ipmi_event_state_t	info;
        int i;

        enables_data = cb_data;

	if (ignore_sensor(sensor)) {
		dbg("sensor is ignored");
                enables_data->done = 1;
		return;
	}	
        
	if (ipmi_sensor_get_event_support(sensor) != IPMI_EVENT_SUPPORT_NONE) {
		info.status = enables_data->sensor_enables->SensorStatus;

                if ( enables_data->sensor_enables->AssertEvents == 0xffff ) {
                        /* enable all assertion events */
                        info.__assertion_events = 0;

                        for( i = 0; i < 12; i++ ) {
                                int val = 0;

                                if ( ipmi_sensor_get_event_reading_type( sensor ) == IPMI_EVENT_READING_TYPE_THRESHOLD )
                                        ipmi_sensor_threshold_assertion_event_supported( sensor, 0, i, &val );
                                else
                                        ipmi_sensor_discrete_assertion_event_supported( sensor, i, &val );

                                if ( val )
                                        info.__assertion_events |= (1 << i);
                        }
                } else
                        info.__assertion_events = enables_data->sensor_enables->AssertEvents;

                if ( enables_data->sensor_enables->DeassertEvents == 0xffff ) {
                        /* enable all deassertion events */

                        info.__deassertion_events = 0;

                        for( i = 0; i < 12; i++ ) {
                                int val = 0;

                                if ( ipmi_sensor_get_event_reading_type( sensor ) == IPMI_EVENT_READING_TYPE_THRESHOLD )
                                        ipmi_sensor_threshold_deassertion_event_supported( sensor, 0, i, &val );
                                else
                                        ipmi_sensor_discrete_deassertion_event_supported( sensor, i, &val );

                                if ( val )
                                        info.__deassertion_events |= (1 << i);
                        }
                } else
                        info.__deassertion_events = enables_data->sensor_enables->DeassertEvents;

		rv = ipmi_sensor_events_enable_set(sensor, &info, set_data,
						   &enables_data->done);
		if (rv) {
                        dbg("Unable to sensor event enables: 0x%x\n", rv);
                        enables_data->done = 1;
		}
	} else {
		dbg("%#x", ipmi_sensor_get_event_support(sensor));	
                enables_data->done = 1;
        }
}

int ohoi_set_sensor_event_enables(ipmi_sensor_id_t		sensor_id,
                                  const SaHpiSensorEvtEnablesT  *enables)

{
        SaHpiSensorEvtEnablesT tmp_enables;
        struct ohoi_sensor_enables enables_data;
        int rv;
        
        tmp_enables = *enables;
        
        enables_data.sensor_enables    = &tmp_enables;
        enables_data.done              = 0;
        
        rv = ipmi_sensor_pointer_cb(sensor_id,
                                          set_sensor_event_enables,
                                          &enables_data);
        if (rv) {
                dbg("Unable to convert sensor_id to pointer");
                return SA_ERR_HPI_INVALID;
        }
        
        return ohoi_loop(&enables_data.done);
}
