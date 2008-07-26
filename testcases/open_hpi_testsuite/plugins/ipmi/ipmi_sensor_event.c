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
 */

#include "ipmi.h"
#include <oh_utils.h>
#include <string.h>

enum ohoi_event_type {
	EVENT_DATA_0 = 0,
	EVENT_DATA_1,
	EVENT_DATA_2,
	EVENT_DATA_3
};

/* IPMI does not define the decrete sensor event state */
enum ohoi_discrete_e {
	IPMI_TRANS_IDLE = 0,
	IPMI_TRANS_ACTIVE,
	IPMI_TRANS_BUSY
};




static void set_discrete_sensor_misc_event(ipmi_event_t		*event,
					   SaHpiSensorEventT	*e)
{
	enum ohoi_event_type  type;
	char	data[IPMI_EVENT_DATA_MAX_LEN];
	int	dt_len;
	SaHpiSensorOptionalDataT od = 0;

	dt_len = ipmi_event_get_data(event, (unsigned char *)data, 0, IPMI_EVENT_DATA_MAX_LEN);
	if (dt_len != 13) {
		err("Wrong size of ipmi event data = %i", dt_len);
		return;
	}


	e->EventState = (1 << (data[10] & 0x0f));
	type = data[10] >> 6;
	if (type == EVENT_DATA_1) {
		if ((data[11] & 0x0f) != 0x0f) {
			od |= SAHPI_SOD_PREVIOUS_STATE;
			e->PreviousState = (1 << (data[11] & 0x0f));
		}
	} else if (type == EVENT_DATA_2) {
		od |= SAHPI_SOD_OEM;
	} else if (type == EVENT_DATA_3) {
		od |= SAHPI_SOD_SENSOR_SPECIFIC;
	}

	type = (data[10] & 0x30) >> 4;
	if (type == EVENT_DATA_2) {
		od |= SAHPI_SOD_OEM;
	} else if (type == EVENT_DATA_3) {
		od |= SAHPI_SOD_SENSOR_SPECIFIC;
	}
	if (e->SensorType == IPMI_SENSOR_TYPE_OS_CRITICAL_STOP) {
		// implementation feature
		od |= SAHPI_SOD_SENSOR_SPECIFIC;
		e->SensorSpecific = (data[12] << 16) |
			(data[11] << 8) | data[9];
	} else {
		e->SensorSpecific = (data[12] << 16) |
			(data[11] << 8) | data[10];
	}
	e->Oem = (data[12] << 16) | (data[11] << 8) | data[10];
	e->OptionalDataPresent = od;
}




static void set_event_sensor_num(ipmi_sensor_t    *sensor,
				 struct oh_event *e,
				 struct oh_handler_state *handler)
{
	ipmi_entity_id_t	entity_id;
        ipmi_sensor_id_t        sensor_id;
        SaHpiRptEntryT          *rpt_entry;
        SaHpiRdrT               *rdr;

	entity_id  = ipmi_entity_convert_to_id(ipmi_sensor_get_entity(sensor));
	sensor_id = ipmi_sensor_convert_to_id(sensor);
	rpt_entry  = ohoi_get_resource_by_entityid(handler->rptcache,
	                                 &entity_id);

	if (!rpt_entry) {
                dump_entity_id("Sensor without RPT Entry?!", entity_id);
		return;
	}
	e->event.Source = rpt_entry->ResourceId;
        rdr  = ohoi_get_rdr_by_data(handler->rptcache,
		                    rpt_entry->ResourceId,
	                            SAHPI_SENSOR_RDR, &sensor_id);
        if (!rdr) {
                err("No rdr for sensor %d in resource:%d\n",
			sensor_id.sensor_num, rpt_entry->ResourceId);
		return;
	}

	e->event.EventDataUnion.SensorEvent.SensorNum =
		rdr->RdrTypeUnion.SensorRec.Num;
}


/*
 * sensor_discrete_map_event
 * @dir: assertion or disassertion
 * @offset: not used now
 * @severity: severity of event
 * @prev_severity: previous state of sensor event
 * @event: ipmi event
 *
 * Helper function to map ipmi event from discrete sensor to oh_event.
 * It is used to implement saHpiEventLogEntryGet() and saHpiEventGet().
 *
 * Returns: oh_event to which ipmi event is mapped
 */
static struct oh_event *sensor_discrete_map_event(
				  struct ohoi_handler *ipmi_handler,
				  enum ipmi_event_dir_e	dir,
				  int	offset,
				  int	severity,
				  int	prev_severity,
				  ipmi_event_t	*event)
{
	struct oh_event         *e;
        unsigned char           data[IPMI_EVENT_DATA_MAX_LEN];
	unsigned int		dt_len;

        dt_len = ipmi_event_get_data(event, data, 0, IPMI_EVENT_DATA_MAX_LEN);
	if (dt_len != 13) {
		err("Wrong size of ipmi event data = %i", dt_len);
		return NULL;
	}
        e = malloc(sizeof(*e));
	if (!e) {
	err("Out of space");
		return NULL;
	}
	memset(e, 0, sizeof(*e));
	e->event.Source = 0;
	/* Do not find EventType in IPMI */
	e->event.EventType = SAHPI_ET_SENSOR;
	e->event.Timestamp = ipmi_event_get_timestamp(event);

	e->event.Severity = (SaHpiSeverityT)severity;

	e->event.EventDataUnion.SensorEvent.SensorNum = 0;
	e->event.EventDataUnion.SensorEvent.SensorType = data[7];
	if ((data[9] & 0x7f) == 0x6f) {
		/* sensor specific event */
		e->event.EventDataUnion.SensorEvent.EventCategory =
			SAHPI_EC_SENSOR_SPECIFIC;
	} else if ((data[9] & 0x7f) >= 0x70) {
		e->event.EventDataUnion.SensorEvent.EventCategory =
			SAHPI_EC_GENERIC;
	} else {
		e->event.EventDataUnion.SensorEvent.EventCategory
                	= data[9] & 0x7f;
	}
	if (data[7] >= 0xc0) {
		e->event.EventDataUnion.SensorEvent.SensorType =
				SAHPI_OEM_SENSOR;
	}


	e->event.EventDataUnion.SensorEvent.Assertion
                = !(dir);

	set_discrete_sensor_misc_event(event,
		&e->event.EventDataUnion.SensorEvent);


	if (!IS_ATCA(ipmi_handler->d_type) || ( data[7] != 0xf1)) {
		return e;
	}
	// ATCA IPMB-0 link sensor. Special mapping
	e->event.EventDataUnion.SensorEvent.EventCategory =
							SAHPI_EC_REDUNDANCY;
	switch (data[10] & 0x0f) { // IPMI event state
	case 0x00: // IPMB_A disable, IPMB-B disable
		e->event.EventDataUnion.SensorEvent.EventState =
				SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES;
		e->event.Severity = SAHPI_CRITICAL;
		break;
	case 0x01: // IPMB_A enable, IPMB-B disable
	case 0x02: // IPMB_A disable, IPMB-B enable
		e->event.EventDataUnion.SensorEvent.EventState =
				SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES;
		e->event.Severity = SAHPI_MAJOR;
		break;
	case 0x03: // IPMB_A enable, IPMB-B enable
		e->event.EventDataUnion.SensorEvent.EventState =
					SAHPI_ES_FULLY_REDUNDANT;
		e->event.Severity = SAHPI_OK;
		break;
	default:
		err("wrong IPMIB-0 Status Change Event State = 0x%x",
				data[10] & 0x0f);
		break;
	}
	if (!(e->event.EventDataUnion.SensorEvent.OptionalDataPresent
			 & SAHPI_SOD_PREVIOUS_STATE)) {
		// no previous state
		return e;
	}
	switch (data[11] & 0x0f) { // previous IPMI event state
	case 0x00: // IPMB_A disable, IPMB-B disable
		e->event.EventDataUnion.SensorEvent.PreviousState =
				SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES;
		break;
	case 0x01: // IPMB_A enable, IPMB-B disable
	case 0x02: // IPMB_A disable, IPMB-B enable
		e->event.EventDataUnion.SensorEvent.PreviousState =
				SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES;
		break;
	case 0x03: // IPMB_A enable, IPMB-B enable
		e->event.EventDataUnion.SensorEvent.PreviousState =
					SAHPI_ES_FULLY_REDUNDANT;
		break;
	default:
		err("wrong IPMIB-0 Status Change Previous Event State = 0x%x",
				data[11] & 0x0f);
		break;
	}

	return e;

}

static int sensor_discrete_event(ipmi_sensor_t	*sensor,
				  enum ipmi_event_dir_e		dir,
				  int				offset,
				  int				severity,
				  int				prev_severity,
				  void			*	cb_data,
				  ipmi_event_t			*event)
{
	struct oh_event		*e;
	struct oh_handler_state *handler = cb_data;
	struct ohoi_handler *ipmi_handler = handler->data;
	ipmi_sensor_id_t sid = ipmi_sensor_convert_to_id(sensor);

	trace_ipmi_sensors("EVENT", sid);
	if ((ipmi_handler->d_type == IPMI_DOMAIN_TYPE_ATCA) &&
			(ipmi_sensor_get_sensor_type(sensor) == 0xF0)) {
		// hot swap sensor. We don't report its event.
		// hotswap event will be reported via entity hotswap
		// handeler instead of.
		return IPMI_EVENT_NOT_HANDLED;
	}
	e = sensor_discrete_map_event(ipmi_handler, dir, offset, severity,
					prev_severity, event);
	if (e == NULL) {
		return IPMI_EVENT_HANDLED;
	}
	set_event_sensor_num(sensor, e, handler);
	e->hid = handler->hid;
        oh_evt_queue_push(handler->eventq, e);

	return IPMI_EVENT_HANDLED;
}


static void
set_thresholed_sensor_event_state(enum ipmi_thresh_e		threshold,
				  enum ipmi_event_dir_e		dir,
				  enum ipmi_event_value_dir_e	high_low,
				  SaHpiSensorEventT		*event,
				  SaHpiSeverityT		*severity)
{
	if ((dir == IPMI_ASSERTION && high_low == IPMI_GOING_HIGH) ||
	    (dir == IPMI_DEASSERTION && high_low == IPMI_GOING_LOW))
		event->Assertion = SAHPI_TRUE;
	else if ((dir == IPMI_ASSERTION &&  high_low == IPMI_GOING_LOW) ||
		 (dir == IPMI_DEASSERTION && high_low == IPMI_GOING_HIGH))
		event->Assertion = SAHPI_FALSE;

	switch (threshold) {
		case IPMI_LOWER_NON_CRITICAL:
			event->EventState = SAHPI_ES_LOWER_MINOR;
			*severity = SAHPI_MINOR;
			break;

		case IPMI_LOWER_CRITICAL:
			event->EventState = SAHPI_ES_LOWER_MAJOR;
			*severity = SAHPI_MAJOR;
			break;

		case IPMI_LOWER_NON_RECOVERABLE:
			event->EventState = SAHPI_ES_LOWER_CRIT;
			*severity = SAHPI_CRITICAL;
			break;

		case IPMI_UPPER_NON_CRITICAL:
			event->EventState = SAHPI_ES_UPPER_MINOR;
			*severity = SAHPI_MINOR;
			break;

		case IPMI_UPPER_CRITICAL:
			event->EventState = SAHPI_ES_UPPER_MAJOR;
			*severity = SAHPI_MAJOR;
			break;

		case IPMI_UPPER_NON_RECOVERABLE:
			event->EventState = SAHPI_ES_UPPER_CRIT;
			*severity = SAHPI_CRITICAL;
			break;

		default:
			err("Invalid threshold giving");
			event->EventState = SAHPI_ES_UNSPECIFIED;
	}
}

static void set_thresholds_sensor_misc_event(ipmi_event_t	*event,
					      SaHpiSensorEventT	*e)
{
	unsigned int type;
	SaHpiSensorOptionalDataT od = 0;
	unsigned char	data[IPMI_EVENT_DATA_MAX_LEN];
	unsigned int	dt_len;

	dt_len = ipmi_event_get_data(event, data, 0, IPMI_EVENT_DATA_MAX_LEN);
	if (dt_len != 13) {
		err("Wrong size of ipmi event data = %i", dt_len);
		return;
	}
	type = data[10] >> 6;
	if (type == EVENT_DATA_1) {
		od |= SAHPI_SOD_TRIGGER_READING;
		e->TriggerReading.IsSupported = SAHPI_TRUE;
		e->TriggerReading.Type = SAHPI_SENSOR_READING_TYPE_UINT64;
		e->TriggerReading.Value.SensorUint64 = data[11];
	} else if (type == EVENT_DATA_2) {
		od |= SAHPI_SOD_OEM;
	} else if (type == EVENT_DATA_3) {
		od |= SAHPI_SOD_SENSOR_SPECIFIC;
	}

	type = (data[10] & 0x30) >> 4;
	if (type == EVENT_DATA_1) {
		od |= SAHPI_SOD_TRIGGER_THRESHOLD;
		e->TriggerThreshold.IsSupported = SAHPI_TRUE;
                e->TriggerThreshold.Type = SAHPI_SENSOR_READING_TYPE_UINT64;
                e->TriggerThreshold.Value.SensorUint64 = data[12];
	} else if (type == EVENT_DATA_2) {
		od |= SAHPI_SOD_OEM;
	} else if (type == EVENT_DATA_3) {
		od |= SAHPI_SOD_SENSOR_SPECIFIC;
	}
	if (e->SensorType == IPMI_SENSOR_TYPE_OS_CRITICAL_STOP) {
		od |= SAHPI_SOD_SENSOR_SPECIFIC;
		e->SensorSpecific = (data[12] << 16) |
			(data[11] << 8) | data[9];
	} else {
		e->SensorSpecific = (data[12] << 16) |
			(data[11] << 8) | data[10];
	}
	e->Oem = (data[12] << 16) | (data[11] << 8) | data[10];
	e->OptionalDataPresent = od;
}

/*
 * sensor_threshold_map_event
 * @dir: assertion or disassertion - pass to set_thresholed_sensor_event_state
 * @threshhold: to pass to set_thresholed_sensor_event_state
 * @high_low: to pass to set_thresholed_sensor_event_state
 * @value_present: not used now
 * @raw_value: not used now
 * value: not used now
 * @event: ipmi event
 *
 * Helper function to map ipmi event from threshold sensor to oh_event.
 * It is used to implement saHpiEventLogEntryGet() and saHpiEventGet().
 *
 * Returns: oh_event to which ipmi event is mapped
 */
static struct oh_event *sensor_threshold_map_event(
				   enum ipmi_event_dir_e	dir,
				   enum ipmi_thresh_e		threshold,
				   enum ipmi_event_value_dir_e	high_low,
				   enum ipmi_value_present_e	value_present,
				   unsigned int			raw_value,
				   double			value,
				   ipmi_event_t			*event)
{
	struct oh_event		*e;
	SaHpiSeverityT		severity = SAHPI_CRITICAL;
	unsigned char	data[IPMI_EVENT_DATA_MAX_LEN];
	unsigned int	dt_len;

	dt_len = ipmi_event_get_data(event, data, 0, IPMI_EVENT_DATA_MAX_LEN);
	if (dt_len != 13) {
		err("Wrong size of ipmi event data = %i", dt_len);
		return NULL;
	}

	e = malloc(sizeof(*e));
	if (!e) {
		err("Out of space");
		return NULL;
	}

	memset(e, 0, sizeof(*e));
	e->event.Source = 0;
	e->event.EventType = SAHPI_ET_SENSOR;
	e->event.Timestamp = ipmi_event_get_timestamp(event);

       // sensor num should be assigned later in calling functions
       e->event.EventDataUnion.SensorEvent.SensorNum = 0;

	if (data[7] >= 0xc0) {
		e->event.EventDataUnion.SensorEvent.SensorType =
			SAHPI_OEM_SENSOR;
	} else {
		e->event.EventDataUnion.SensorEvent.SensorType =
                	data[7];
	}
	e->event.EventDataUnion.SensorEvent.EventCategory
                = data[9] & 0x7f;

	set_thresholed_sensor_event_state(threshold, dir, high_low,
			&e->event.EventDataUnion.SensorEvent,
			&severity);
	e->event.Severity = severity;

	set_thresholds_sensor_misc_event
		(event, &e->event.EventDataUnion.SensorEvent);
        return e;

}


static int sensor_threshold_event(ipmi_sensor_t		*sensor,
				   enum ipmi_event_dir_e	dir,
				   enum ipmi_thresh_e		threshold,
				   enum ipmi_event_value_dir_e	high_low,
				   enum ipmi_value_present_e	value_present,
				   unsigned int			raw_value,
				   double			value,
				   void				*cb_data,
				   ipmi_event_t			*event)
{
	struct oh_event		*e = NULL;
	struct oh_handler_state *handler = cb_data;

	e = sensor_threshold_map_event(dir, threshold, high_low,
			value_present, raw_value, value, event);
	if (e == NULL) {
		return IPMI_EVENT_HANDLED;
	}
	set_event_sensor_num(sensor, e, handler);
	e->hid = handler->hid;
        oh_evt_queue_push(handler->eventq, e);

	return IPMI_EVENT_HANDLED;
}


static void add_sensor_event_thresholds(ipmi_sensor_t	*sensor,
					SaHpiSensorRecT	*rec)
{
	int 			val;
	SaHpiSensorThdMaskT	temp;
	unsigned int		access;

	if (rec->Category != SAHPI_EC_THRESHOLD) {
		rec->ThresholdDefn.IsAccessible = SAHPI_FALSE;
		return;
	}

	access = ipmi_sensor_get_threshold_access(sensor);
	if (access == IPMI_THRESHOLD_ACCESS_SUPPORT_NONE) {
		rec->ThresholdDefn.IsAccessible = SAHPI_FALSE;
		return;
	}

	if (access >= IPMI_THRESHOLD_ACCESS_SUPPORT_READABLE) {
		rec->ThresholdDefn.IsAccessible = SAHPI_TRUE;

		temp = 0;
		ipmi_sensor_threshold_readable(sensor,
				IPMI_LOWER_NON_CRITICAL, &val);
		if (val)
			temp |= SAHPI_STM_LOW_MINOR;

		ipmi_sensor_threshold_readable(sensor, IPMI_LOWER_CRITICAL,
					       &val);
		if (val)
			temp |= SAHPI_STM_LOW_MAJOR;

		ipmi_sensor_threshold_readable(sensor,
				               IPMI_LOWER_NON_RECOVERABLE,
					       &val);
		if (val)
			temp |= SAHPI_STM_LOW_CRIT;

		ipmi_sensor_threshold_readable(sensor, IPMI_UPPER_NON_CRITICAL,
					       &val);
		if (val)
			temp |= SAHPI_STM_UP_MINOR;

		ipmi_sensor_threshold_readable(sensor, IPMI_UPPER_CRITICAL,
					       &val);
		if (val)
			temp |= SAHPI_STM_UP_MAJOR;

		ipmi_sensor_threshold_readable(sensor,
					       IPMI_UPPER_NON_RECOVERABLE,
					       &val);
		if (val)
			temp |= SAHPI_STM_UP_CRIT;

		val = ipmi_sensor_get_hysteresis_support(sensor);
		if (val == IPMI_HYSTERESIS_SUPPORT_READABLE ||
		    val == IPMI_HYSTERESIS_SUPPORT_SETTABLE)
			temp |= SAHPI_STM_UP_HYSTERESIS |
				SAHPI_STM_LOW_HYSTERESIS;

		rec->ThresholdDefn.ReadThold = temp;
	}

	if (access == IPMI_THRESHOLD_ACCESS_SUPPORT_SETTABLE) {
		temp = 0;
		ipmi_sensor_threshold_settable(sensor, IPMI_LOWER_NON_CRITICAL,
					       &val);
		if (val)
			temp |= SAHPI_STM_LOW_MINOR;

		ipmi_sensor_threshold_settable(sensor, IPMI_LOWER_CRITICAL,
					       &val);
		if (val)
			temp |= SAHPI_STM_LOW_MAJOR;

		ipmi_sensor_threshold_settable(sensor,
					       IPMI_LOWER_NON_RECOVERABLE,
					       &val);
		if (val)
			temp |= SAHPI_STM_LOW_CRIT;

		ipmi_sensor_threshold_settable(sensor, IPMI_UPPER_NON_CRITICAL,
					       &val);
		if (val)
			temp |= SAHPI_STM_UP_MINOR;

		ipmi_sensor_threshold_settable(sensor, IPMI_UPPER_CRITICAL,
					       &val);
		if (val)
			temp |= SAHPI_STM_UP_MAJOR;

		ipmi_sensor_threshold_settable(sensor,
					       IPMI_UPPER_NON_RECOVERABLE,
					       &val);
		if (val)
			temp |= SAHPI_STM_UP_CRIT;

		val = ipmi_sensor_get_hysteresis_support(sensor);
		if (val == IPMI_HYSTERESIS_SUPPORT_SETTABLE)
			temp |= SAHPI_STM_UP_HYSTERESIS |
				SAHPI_STM_LOW_HYSTERESIS;

		rec->ThresholdDefn.WriteThold = temp;
	}
}

static void add_sensor_event_data_format(ipmi_sensor_t		*sensor,
					 SaHpiSensorRecT	*rec)
{
#define FILL_READING(reading, val)                                         \
	do {                                                               \
		(reading).IsSupported = SAHPI_TRUE;                        \
		(reading).Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;        \
		(reading).Value.SensorFloat64 = (SaHpiFloat64T)val;        \
	} while (0)
	int rv;
	double accur = 0;
	double fval = 0;


	if (ipmi_sensor_get_event_reading_type(sensor) !=
				IPMI_EVENT_READING_TYPE_THRESHOLD) {
		rec->DataFormat.IsSupported = SAHPI_FALSE;
		return;
	}
	rec->DataFormat.IsSupported = SAHPI_TRUE;

	rec->DataFormat.ReadingType = SAHPI_SENSOR_READING_TYPE_FLOAT64;

	rec->DataFormat.BaseUnits = (SaHpiSensorUnitsT)
		ipmi_sensor_get_base_unit(sensor);
	rec->DataFormat.ModifierUnits = (SaHpiSensorUnitsT)
		ipmi_sensor_get_modifier_unit(sensor);
	rec->DataFormat.ModifierUse = (SaHpiSensorModUnitUseT)
		ipmi_sensor_get_modifier_unit_use(sensor);

	ipmi_sensor_get_accuracy(sensor, 0, &accur);
	rec->DataFormat.AccuracyFactor = (SaHpiFloat64T)accur;

	rec->DataFormat.Percentage = (SaHpiBoolT)
		ipmi_sensor_get_percentage(sensor);

	rec->DataFormat.Range.Flags = 0;

	rv = ipmi_sensor_get_sensor_max(sensor, &fval);
	if (!rv) {
		FILL_READING(rec->DataFormat.Range.Max, fval);
		rec->DataFormat.Range.Flags |= SAHPI_SRF_MAX;
	}

	rv = ipmi_sensor_get_sensor_min(sensor, &fval);
	if (!rv) {
		FILL_READING(rec->DataFormat.Range.Min, fval);
		rec->DataFormat.Range.Flags |= SAHPI_SRF_MIN;
	}

	if (ipmi_sensor_get_nominal_reading_specified(sensor)) {
		rv = ipmi_sensor_get_nominal_reading(sensor, &fval);
		if (!rv) {
			FILL_READING(rec->DataFormat.Range.Nominal, fval);
			rec->DataFormat.Range.Flags |= SAHPI_SRF_NOMINAL;
		}
	}
	if (ipmi_sensor_get_normal_max_specified(sensor)) {
		rv = ipmi_sensor_get_normal_max(sensor, &fval);
		if (!rv) {
			FILL_READING(rec->DataFormat.Range.NormalMax, fval);
			rec->DataFormat.Range.Flags |= SAHPI_SRF_NORMAL_MAX;
		}
	}

	if (ipmi_sensor_get_normal_min_specified(sensor)) {
		rv = ipmi_sensor_get_normal_min(sensor, &fval);
		if (!rv) {
			FILL_READING(rec->DataFormat.Range.NormalMin, fval);
			rec->DataFormat.Range.Flags |= SAHPI_SRF_NORMAL_MIN;
		}
	}
}

static SaHpiEventCategoryT ohoi_sensor_get_event_reading_type(ipmi_sensor_t   *sensor)
{
	SaHpiEventCategoryT 	hpi_category;
	unsigned int 		ipmi_category;

	ipmi_category = ipmi_sensor_get_event_reading_type(sensor);
	switch (ipmi_category) {
		case IPMI_EVENT_READING_TYPE_DISCRETE_ACPI_POWER:
		case IPMI_EVENT_READING_TYPE_SENSOR_SPECIFIC:
			hpi_category = SAHPI_EC_GENERIC;
			break;
		default:
			hpi_category = ipmi_category;
			break;
	}
	return hpi_category;
}

static void add_sensor_states(ipmi_sensor_t	*sensor,
			struct ohoi_sensor_info *sensor_info)
{
	int i;
	int val;
	int rv;

	sensor_info->support_assert = 0;
	sensor_info->support_deassert = 0;
	if(ipmi_sensor_get_event_reading_type(sensor) !=
			IPMI_EVENT_READING_TYPE_THRESHOLD) {
		for (i = 0; i < 15; i++) {
			rv = ipmi_sensor_discrete_event_supported(sensor, i,
					IPMI_ASSERTION, &val);
			if ((rv == 0) && val) {
				sensor_info->support_assert |= (1 << i);
			}
			rv = ipmi_sensor_discrete_event_supported(sensor, i,
					IPMI_DEASSERTION, &val);
			if ((rv == 0) && val) {
				sensor_info->support_deassert |= (1 << i);
			}

		}

		return;
	}

	// threshold sensor
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_LOWER_NON_CRITICAL, IPMI_GOING_LOW,
		IPMI_ASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_assert |= OHOI_THS_LMINL;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_LOWER_NON_CRITICAL, IPMI_GOING_HIGH,
		IPMI_ASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_assert |= OHOI_THS_LMINH;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_LOWER_NON_CRITICAL, IPMI_GOING_LOW,
		IPMI_DEASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_deassert |= OHOI_THS_LMINL;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_LOWER_NON_CRITICAL, IPMI_GOING_HIGH,
		IPMI_DEASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_deassert |= OHOI_THS_LMINH;
	}

	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_LOWER_CRITICAL, IPMI_GOING_LOW,
		IPMI_ASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_assert |= OHOI_THS_LMAJL;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_LOWER_CRITICAL, IPMI_GOING_HIGH,
		IPMI_ASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_assert |= OHOI_THS_LMAJH;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_LOWER_CRITICAL, IPMI_GOING_LOW,
		IPMI_DEASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_deassert |= OHOI_THS_LMAJL;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_LOWER_CRITICAL, IPMI_GOING_HIGH,
		IPMI_DEASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_deassert |= OHOI_THS_LMAJH;
	}

	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_LOWER_NON_RECOVERABLE, IPMI_GOING_LOW,
		IPMI_ASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_assert |= OHOI_THS_LCRTL;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_LOWER_NON_RECOVERABLE, IPMI_GOING_HIGH,
		IPMI_ASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_assert |= OHOI_THS_LCRTH;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_LOWER_NON_RECOVERABLE, IPMI_GOING_LOW,
		IPMI_DEASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_deassert |= OHOI_THS_LCRTL;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_LOWER_NON_RECOVERABLE, IPMI_GOING_HIGH,
		IPMI_DEASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_deassert |= OHOI_THS_LCRTH;
	}


	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_UPPER_NON_CRITICAL, IPMI_GOING_LOW,
		IPMI_ASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_assert |= OHOI_THS_UMINL;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_UPPER_NON_CRITICAL, IPMI_GOING_HIGH,
		IPMI_ASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_assert |= OHOI_THS_UMINH;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_UPPER_NON_CRITICAL, IPMI_GOING_LOW,
		IPMI_DEASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_deassert |= OHOI_THS_UMINL;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_UPPER_NON_CRITICAL, IPMI_GOING_HIGH,
		IPMI_DEASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_deassert |= OHOI_THS_UMINH;
	}

	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_UPPER_CRITICAL, IPMI_GOING_LOW,
		IPMI_ASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_assert |= OHOI_THS_UMAJL;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_UPPER_CRITICAL, IPMI_GOING_HIGH,
		IPMI_ASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_assert |= OHOI_THS_UMAJH;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_UPPER_CRITICAL, IPMI_GOING_LOW,
		IPMI_DEASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_deassert |= OHOI_THS_UMAJL;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_UPPER_CRITICAL, IPMI_GOING_HIGH,
		IPMI_DEASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_deassert |= OHOI_THS_UMAJH;
	}

	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_UPPER_NON_RECOVERABLE, IPMI_GOING_LOW,
		IPMI_ASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_assert |= OHOI_THS_UCRTL;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_UPPER_NON_RECOVERABLE, IPMI_GOING_HIGH,
		IPMI_ASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_assert |= OHOI_THS_UCRTH;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_UPPER_NON_RECOVERABLE, IPMI_GOING_LOW,
		IPMI_DEASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_deassert |= OHOI_THS_UCRTL;
	}
	rv = ipmi_sensor_threshold_event_supported(sensor,
		IPMI_UPPER_NON_RECOVERABLE, IPMI_GOING_HIGH,
		IPMI_DEASSERTION, &val);
	if ((rv == 0) && (val != 0)) {
		sensor_info->support_deassert |= OHOI_THS_UCRTH;
	}
}

static SaHpiEventStateT convert_to_hpi_event_state(
					ipmi_sensor_t	*sensor,
					struct ohoi_sensor_info *sensor_info)
{
	unsigned int ast = sensor_info->support_assert;
	unsigned int dst = sensor_info->support_deassert;
	SaHpiEventStateT hst = 0;

	if(ipmi_sensor_get_event_reading_type(sensor) !=
			IPMI_EVENT_READING_TYPE_THRESHOLD) {
		return (SaHpiEventStateT)((ast | dst) & 0x7fff);
	}
	if ((ast | dst) & (OHOI_THS_LMINL | OHOI_THS_LMINH)) {
		hst |= SAHPI_ES_LOWER_MINOR;
	}
	if ((ast | dst) & (OHOI_THS_LMAJL | OHOI_THS_LMAJH)) {
		hst |= SAHPI_ES_LOWER_MAJOR;
	}
	if ((ast | dst) & (OHOI_THS_LCRTL | OHOI_THS_LCRTH)) {
		hst |= SAHPI_ES_LOWER_CRIT;
	}
	if ((ast | dst) & (OHOI_THS_UMINL | OHOI_THS_UMINH)) {
		hst |= SAHPI_ES_UPPER_MINOR;
	}
	if ((ast | dst) & (OHOI_THS_UMAJL | OHOI_THS_UMAJH)) {
		hst |= SAHPI_ES_UPPER_MAJOR;
	}
	if ((ast | dst) & (OHOI_THS_UCRTL | OHOI_THS_UCRTH)) {
		hst |= SAHPI_ES_UPPER_CRIT;
	}
	return hst;
}


static void add_sensor_event_sensor_rec(struct oh_handler_state *handler,
					ipmi_sensor_t	*sensor,
					SaHpiSensorRecT	*rec)
{

	rec->Type = (SaHpiSensorTypeT)ipmi_sensor_get_sensor_type(sensor);
	if (rec->Type >= 0xc0) {
		rec->Type = SAHPI_OEM_SENSOR;
	}
	rec->Category = ohoi_sensor_get_event_reading_type(sensor);
	rec->EventCtrl =
		(SaHpiSensorEventCtrlT)ipmi_sensor_get_event_support(sensor);
	add_sensor_event_data_format(sensor, rec);
	add_sensor_event_thresholds(sensor, rec);
	rec->Oem = ipmi_sensor_get_oem1(sensor);
}

static void add_sensor_event_rdr(struct oh_handler_state *handler,
				 ipmi_sensor_t		*sensor,
				 SaHpiRdrT		*rdr,
				 SaHpiEntityPathT	parent_ep,
				 SaHpiResourceIdT	res_id)
{
	char		name[SAHPI_MAX_TEXT_BUFFER_LENGTH];
	SaHpiTextTypeT	data_type;
	int		name_len;

	memset(name, '\0', SAHPI_MAX_TEXT_BUFFER_LENGTH);
	rdr->RecordId = 0;
	rdr->RdrType = SAHPI_SENSOR_RDR;
	rdr->Entity = parent_ep;

	add_sensor_event_sensor_rec(handler, sensor,
				&rdr->RdrTypeUnion.SensorRec);

	ipmi_sensor_get_id(sensor, name, SAHPI_MAX_TEXT_BUFFER_LENGTH);
	data_type = convert_to_hpi_data_type(ipmi_sensor_get_id_type(sensor));
	name_len = ipmi_sensor_get_id_length(sensor);
	if (name_len >= SAHPI_MAX_TEXT_BUFFER_LENGTH)
		name_len = SAHPI_MAX_TEXT_BUFFER_LENGTH - 1;
	rdr->IdString.DataType = data_type;
	rdr->IdString.Language = SAHPI_LANG_ENGLISH;
	rdr->IdString.DataLength = name_len;

	switch ( ipmi_sensor_get_event_support(sensor) ) {
		case IPMI_EVENT_SUPPORT_PER_STATE:
			rdr->RdrTypeUnion.SensorRec.EventCtrl =
							SAHPI_SEC_PER_EVENT;
			rdr->RdrTypeUnion.SensorRec.EnableCtrl = SAHPI_TRUE;
			break;
		case IPMI_EVENT_SUPPORT_ENTIRE_SENSOR:
			rdr->RdrTypeUnion.SensorRec.EventCtrl =
						SAHPI_SEC_READ_ONLY_MASKS;
			rdr->RdrTypeUnion.SensorRec.EnableCtrl = SAHPI_TRUE;
			break;

                case IPMI_EVENT_SUPPORT_GLOBAL_ENABLE:
		case IPMI_EVENT_SUPPORT_NONE:
                        rdr->RdrTypeUnion.SensorRec.EventCtrl =
						SAHPI_SEC_READ_ONLY;
			break;
	}

	memset(rdr->IdString.Data, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
	memcpy(rdr->IdString.Data, name, name_len);
}



static void add_sensor_event(ipmi_entity_t	*ent,
			     ipmi_sensor_t	*sensor,
			     struct oh_handler_state *handler,
			     SaHpiRptEntryT	*rpt)
{
	struct ohoi_sensor_info *sensor_info;
	SaHpiRdrT		rdr;
	int lun, num;
	int rv;

	sensor_info = malloc(sizeof(*sensor_info));

	if (!sensor_info) {
	      	err("Out of memory for sensor info");
		return;
	}

	sensor_info->type = OHOI_SENSOR_ORIGINAL;
	sensor_info->ohoii.get_sensor_event_enable =
					orig_get_sensor_event_enable;
	sensor_info->ohoii.set_sensor_event_enable =
					orig_set_sensor_event_enable;
	sensor_info->ohoii.get_sensor_reading =
					orig_get_sensor_reading;
	sensor_info->ohoii.get_sensor_thresholds =
					orig_get_sensor_thresholds;
	sensor_info->ohoii.set_sensor_thresholds =
					orig_set_sensor_thresholds;

	sensor_info->info.orig_sensor_info.sensor_id  =
				ipmi_sensor_convert_to_id(sensor);
	sensor_info->sen_enabled = SAHPI_TRUE;
        sensor_info->enable = SAHPI_TRUE;
	add_sensor_states(sensor, sensor_info);

	memset(&rdr, 0, sizeof(rdr));
	rdr.RdrTypeUnion.SensorRec.Events =
		convert_to_hpi_event_state(sensor, sensor_info);

	rv = ipmi_sensor_get_num(sensor, &lun, &num);
	if(rv) {
		err("Error getting sensor number");
        	rdr.RdrTypeUnion.SensorRec.Num =
					SA_ERR_HPI_INVALID_DATA;
	} else {
	      rdr.RdrTypeUnion.SensorRec.Num = num;
	}

	add_sensor_event_rdr(handler, sensor, &rdr,
		rpt->ResourceEntity, rpt->ResourceId);

	adjust_sensor_to_atcahpi_spec(handler, rpt, &rdr,
				sensor_info, sensor);

	rv = oh_add_rdr(handler->rptcache, rpt->ResourceId,
				&rdr, sensor_info, 1);
	if (rv != SA_OK) {
		free(sensor_info);
		err("Failed to add sensor rdr");
	}
}

void ohoi_sensor_event(enum ipmi_update_e op,
                       ipmi_entity_t      *ent,
                       ipmi_sensor_t      *sensor,
                       void               *cb_data)
{
      	char			name[33];
	int			rv;


	ipmi_sensor_id_t sid = ipmi_sensor_convert_to_id(sensor);
	struct oh_handler_state *handler = cb_data;
	struct ohoi_handler *ipmi_handler = handler->data;
	struct ohoi_resource_info *res_info;
	ipmi_entity_id_t entity_id;
	SaHpiRptEntryT *rpt_entry;

	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
	ipmi_sensor_get_id(sensor, name, 32);

        entity_id = ipmi_entity_convert_to_id(ent);

        rpt_entry = ohoi_get_resource_by_entityid(handler->rptcache,
						  &entity_id);
        if (!rpt_entry) {
                dump_entity_id("Sensor without RPT Entry?!", entity_id);
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
                return;
        }

	res_info =  oh_get_resource_data(handler->rptcache,
						rpt_entry->ResourceId);

	switch (op) {
		case IPMI_ADDED:
			trace_ipmi_sensors("ADD", sid);
			rpt_entry->ResourceCapabilities |=
				 SAHPI_CAPABILITY_RDR | SAHPI_CAPABILITY_SENSOR;

	               	/* fill in the sensor data, add it to ipmi_event_list
			 * and finally to the rpt-cache
			 */
			add_sensor_event(ent, sensor, handler, rpt_entry);

			trace_ipmi("Sensor Added");

			if (ipmi_sensor_get_event_reading_type(sensor) ==
					IPMI_EVENT_READING_TYPE_THRESHOLD) {
			      	rv = ipmi_sensor_add_threshold_event_handler(
					sensor, sensor_threshold_event,
					handler);
			} else {
				if (IS_ATCA(ipmi_handler->d_type) &&
					(ipmi_sensor_get_sensor_type(sensor)
								== 0xF0)) {
					break;
				}
			      	rv = ipmi_sensor_add_discrete_event_handler(
					sensor, sensor_discrete_event, handler);
			}
			if (rv)
			      	err("Unable to reg sensor event handler: %#x\n",
					rv);
			break;
		case IPMI_CHANGED:
			trace_ipmi_sensors("CHANGED", sid);
			add_sensor_event(ent, sensor, handler, rpt_entry);
			dbg("Sensor Changed");
			break;
		case IPMI_DELETED:
			trace_ipmi_sensors("DELELE", sid);
			if (ohoi_delete_orig_sensor_rdr(handler,
							rpt_entry, &sid)) {
				// no more sensors for rpt
				rpt_entry->ResourceCapabilities &=
				 ~SAHPI_CAPABILITY_SENSOR;
			}
			if ((oh_get_rdr_next(handler->rptcache,
					 rpt_entry->ResourceId,
					 SAHPI_FIRST_ENTRY) == NULL) &&
					 (res_info->fru == NULL)) {
				// no more rdrs for rpt
				rpt_entry->ResourceCapabilities &=
				 ~SAHPI_CAPABILITY_RDR;
			}
			/* We don't do anything, if the entity is gone
			   infrastructre deletes the RDRs automatically
			*/
			break;
	}
	trace_ipmi("Set updated for resource %d . Sensor", rpt_entry->ResourceId);
	entity_rpt_set_updated(res_info, ipmi_handler);
	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
}

/*
 * get_sensor_by_sensor_id_handler
 *
 * This is just callback to get ipmi_sensor_t from ipmi_sensor_id_t
 * and auxiliary structure to do it
 */



static void get_sensor_by_sensor_id_handler(ipmi_sensor_t *sensor,
					    void *cb_data)
{
	ipmi_entity_id_t    *entity_id = cb_data;
	if (sensor == NULL) {
		ipmi_entity_id_set_invalid(entity_id);
	}
	*entity_id = ipmi_entity_convert_to_id(
			ipmi_sensor_get_entity(sensor));
}


int ohoi_sensor_ipmi_event_to_hpi_event(
				struct ohoi_handler *ipmi_handler,
				ipmi_sensor_id_t sid,
				ipmi_event_t     *event,
				struct oh_event **e,
				ipmi_entity_id_t    *entity_id
				)
{
//	sens_info_t		info;
	enum ipmi_event_dir_e       dir;
	struct oh_event * ev = NULL;
	unsigned char	data[IPMI_EVENT_DATA_MAX_LEN];
	unsigned int	dt_len;
	int rv;

	dt_len = ipmi_event_get_data(event, data, 0, IPMI_EVENT_DATA_MAX_LEN);
	if (dt_len != 13) {
		err("Wrong size of ipmi event data = %i", dt_len);
		return 0;
	}

	rv = ipmi_sensor_pointer_cb(sid, get_sensor_by_sensor_id_handler,
					entity_id);
	if (rv) {
		err("no sensor for sensor_id rv = 0x%x", rv);
	}

	dir = data[9] >> 7;

	if ((data[9] & 0x7f) == IPMI_EVENT_READING_TYPE_THRESHOLD) {
		enum ipmi_thresh_e          threshold;
		enum ipmi_event_value_dir_e high_low;
		enum ipmi_value_present_e   value_present;
		unsigned int                raw_value;
		double                      value;

		threshold = (data[10] >> 1) & 0x07;
		high_low = data[10] & 1;
		raw_value = data[11];
		value = 0.0;
		value_present = IPMI_NO_VALUES_PRESENT;
		ev = sensor_threshold_map_event(dir, threshold, high_low,
		                   value_present, raw_value,  value, event);
	} else {
		int                   offset;
		int                   severity = 0;
		int                   prev_severity = 0;

		offset = data[10] & 0x0f;
		if ((data[10] >> 6) == 2) {
			severity = data[11] >> 4;
			prev_severity = data[11] & 0xf;
			 if (severity == 0xf) {
				severity = -1;
			}
			if (prev_severity == 0xf) {
				prev_severity = -1;
			}
		}
		ev = sensor_discrete_map_event(ipmi_handler,dir, offset,
					severity, prev_severity, event);
	}
	if (ev == NULL) {
		return 1;
	}

	if (ev->event.EventDataUnion.SensorEvent.SensorNum == 0) {
		ev->event.EventDataUnion.SensorEvent.SensorNum =
				data[8];
	}
	*e = ev;
	return 0;
}

