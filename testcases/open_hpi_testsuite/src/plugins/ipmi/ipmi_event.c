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
 *     Tariq Shureih <tariq.shureih@intel.com>
 */

#include "ipmi.h"
#include <epath_utils.h>
#include <uid_utils.h>
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


#define dump_entity_id(s, x) \
        do { \
                dbg("%s domain id: %p, entity id: %x, entity instance: %x, channel: %x, address: %x, seq: %lx", \
                     s,                         \
                     (x).domain_id.domain,      \
                     (x).entity_id,             \
                     (x).entity_instance,       \
                     (x).channel,               \
                     (x).address,               \
                     (x).seq);                  \
        } while(0)

static inline int domain_id_is_equal(const ipmi_domain_id_t id1,
                                     const ipmi_domain_id_t id2)
{
        return (id1.domain == id2.domain);
}

static inline int entity_id_is_equal(const ipmi_entity_id_t id1, 
                                     const ipmi_entity_id_t id2)
{
	return (domain_id_is_equal(id1.domain_id, id2.domain_id)
                && (id1.entity_id == id2.entity_id)
                && (id1.entity_instance == id2.entity_instance)
                && (id1.channel == id2.channel)
                && (id1.address == id2.address)
                && (id1.seq == id2.seq));
}

static inline int mc_id_is_equal(const ipmi_mcid_t id1,
                                 const ipmi_mcid_t id2)
{
        return (domain_id_is_equal(id1.domain_id, id2.domain_id)
                && (id1.mc_num == id2.mc_num)
                && (id1.channel== id2.channel)
                && (id1.seq == id2.seq));
}

static inline int ohoi_resource_id_is_equal(
                const struct ohoi_resource_id id1,
                const struct ohoi_resource_id id2)
{
        if (id1.type != id2.type)
                return 0;
        switch (id1.type) {
                case OHOI_RESOURCE_ENTITY:
                        return (entity_id_is_equal(id1.u.entity_id, 
                                                   id2.u.entity_id));
                case OHOI_RESOURCE_MC:
                        return (mc_id_is_equal(id1.u.mc_id,
                                               id2.u.mc_id));
                default:
                        dbg("UNKNOWN OHOI RESOURCE TYPE!");
                        return 0;
        }
}

static void set_discrete_sensor_misc_event(ipmi_event_t		*event,
					   SaHpiSensorEventT	*e)
{
	enum ohoi_event_type  type;
	
	type = event->data[10] >> 6;
	if (type == EVENT_DATA_2) 
		e->Oem = (SaHpiUint32T)event->data[11]; 
	else if (type == EVENT_DATA_3)
		e->SensorSpecific = (SaHpiUint32T)event->data[11]; 

	type = (event->data[10] & 0x30) >> 4;
	if (type == EVENT_DATA_2)
		e->Oem = (SaHpiUint32T)event->data[12];
	else if (type == EVENT_DATA_3)
		e->SensorSpecific = (SaHpiUint32T)event->data[12];
}

static void 
set_discrete_sensor_event_state(ipmi_event_t		*event,
				  SaHpiEventStateT	*state)
{
	enum ohoi_discrete_e e = event->data[10] & 0x7;

	switch (e) {
		case IPMI_TRANS_IDLE:
			*state = SAHPI_ES_IDLE;
			break;
		
		case IPMI_TRANS_ACTIVE:
			*state = IPMI_TRANS_ACTIVE;
			break;

		case IPMI_TRANS_BUSY:
			*state = SAHPI_ES_BUSY;
			break;
	}
}

/*XXX algorithm here is so ulgy! */
static SaHpiRptEntryT *get_resource_by_entityid(RPTable                *table,
                                                const ipmi_entity_id_t *entity_id)
{
        struct ohoi_resource_id res_id1;
        SaHpiRptEntryT *rpt_entry;
        
        res_id1.type            = OHOI_RESOURCE_ENTITY;
        res_id1.u.entity_id     = *entity_id;
        
        rpt_entry = oh_get_resource_next(table, RPT_ENTRY_BEGIN);
        while (rpt_entry) {
                struct ohoi_resource_id *ohoi_res_id;
                ohoi_res_id = oh_get_resource_data(table, rpt_entry->ResourceId);
                if (ohoi_resource_id_is_equal(res_id1, *ohoi_res_id)) {
                        return rpt_entry;
		}
                rpt_entry = oh_get_resource_next(table, 
                                                 rpt_entry->ResourceId);            
        }

	dbg("Not found resource by entity_id");
        return NULL;
}

static void sensor_discrete_event(ipmi_sensor_t		*sensor,
				  enum ipmi_event_dir_e	dir,
				  int			offset,
				  int			severity,
				  int			prev_severity,
				  void			*cb_data,
				  ipmi_event_t		*event)
{
	struct oh_event         *e;
        struct oh_handler_state *handler;
	ipmi_entity_id_t        entity_id;
        SaHpiRptEntryT          *rpt_entry;

        handler    = cb_data;
        entity_id  = ipmi_entity_convert_to_id(ipmi_sensor_get_entity(sensor));
        rpt_entry  = get_resource_by_entityid(handler->rptcache, &entity_id);

        if (!rpt_entry) {
                dump_entity_id("Discrete Sensor without RPT?!", entity_id);
                return;
        }

        e = malloc(sizeof(*e));
	if (!e) {
	dbg("Out of space");
		return;
	}

	memset(e, 0, sizeof(*e));
	e->type = OH_ET_HPI;
	e->u.hpi_event.parent = rpt_entry->ResourceId;
	
	e->u.hpi_event.event.Source = 0;
	/* Do not find EventType in IPMI */
	e->u.hpi_event.event.EventType = SAHPI_ET_SENSOR;
	e->u.hpi_event.event.Timestamp = (SaHpiTimeT)ipmi_get_uint32(event->data);

	e->u.hpi_event.event.Severity = (SaHpiSeverityT)severity;

	e->u.hpi_event.event.EventDataUnion.SensorEvent.SensorNum = 0;
	e->u.hpi_event.event.EventDataUnion.SensorEvent.SensorType = 
		(SaHpiSensorTypeT)event->data[7];
	e->u.hpi_event.event.EventDataUnion.SensorEvent.EventCategory =
		(SaHpiEventCategoryT)event->data[9] & 0x7f;
	e->u.hpi_event.event.EventDataUnion.SensorEvent.Assertion = 
		(SaHpiBoolT)!(dir);
	
	set_discrete_sensor_event_state(event, &e->u.hpi_event.event.EventDataUnion.SensorEvent.EventState);
	e->u.hpi_event.event.EventDataUnion.SensorEvent.PreviousState 
                = SAHPI_ES_UNSPECIFIED;

	set_discrete_sensor_misc_event
		(event, &e->u.hpi_event.event.EventDataUnion.SensorEvent);

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
			dbg("Invalid threshold giving");
			event->EventState = SAHPI_ES_UNSPECIFIED;
	}
}

static void set_thresholds_sensor_misc_event(ipmi_event_t	*event,
					      SaHpiSensorEventT	*e)
{
	unsigned int type;
	
	type = event->data[10] >> 6;
	if (type == EVENT_DATA_1) {
		e->TriggerReading.ValuesPresent = SAHPI_SRF_RAW;
		e->TriggerReading.Raw = (SaHpiUint32T)event->data[11];
	}
	else if (type == EVENT_DATA_2) 
		e->Oem = (SaHpiUint32T)event->data[11]; 
	else if (type == EVENT_DATA_3)
		e->SensorSpecific = (SaHpiUint32T)event->data[11]; 

	type = (event->data[10] & 0x30) >> 4;
	if (type == EVENT_DATA_1) {
		e->TriggerThreshold.ValuesPresent = SAHPI_SRF_RAW;
		e->TriggerThreshold.Raw = (SaHpiUint32T)event->data[12];
	}
	else if (type == EVENT_DATA_2)
		e->Oem = (SaHpiUint32T)event->data[12];
	else if (type == EVENT_DATA_3)
		e->SensorSpecific = (SaHpiUint32T)event->data[12];
}

static void sensor_threshold_event(ipmi_sensor_t		*sensor,
				   enum ipmi_event_dir_e	dir,
				   enum ipmi_thresh_e		threshold,
				   enum ipmi_event_value_dir_e	high_low,
				   enum ipmi_value_present_e	value_present,
				   unsigned int			raw_value,
				   double			value,
				   void				*cb_data,
				   ipmi_event_t			*event)
{
	struct oh_event		*e;
        struct oh_handler_state *handler;
	ipmi_entity_id_t	entity_id;
	SaHpiSeverityT		severity;
        SaHpiRptEntryT          *rpt_entry;

      
        handler    = cb_data;
        entity_id  = ipmi_entity_convert_to_id(ipmi_sensor_get_entity(sensor));
        rpt_entry  = get_resource_by_entityid(handler->rptcache, &entity_id);
	
	e = malloc(sizeof(*e));
	if (!e) {
		dbg("Out of space");
		return;
	}

	memset(e, 0, sizeof(*e));
	e->type = OH_ET_HPI;
	e->u.hpi_event.parent = rpt_entry->ResourceId;
	
	e->u.hpi_event.event.Source = 0;
	/* Do not find EventType in IPMI */
	e->u.hpi_event.event.EventType = SAHPI_ET_SENSOR;
	e->u.hpi_event.event.Timestamp = (SaHpiTimeT)
		ipmi_get_uint32(event->data);

	e->u.hpi_event.event.EventDataUnion.SensorEvent.SensorNum = 0;
	e->u.hpi_event.event.EventDataUnion.SensorEvent.SensorType = 
		(SaHpiSensorTypeT)event->data[7];
	e->u.hpi_event.event.EventDataUnion.SensorEvent.EventCategory =
		(SaHpiEventCategoryT)event->data[9] & 0x7f;
	
	set_thresholed_sensor_event_state(threshold, dir, high_low,
			&e->u.hpi_event.event.EventDataUnion.SensorEvent,
			&severity);
	e->u.hpi_event.event.Severity = severity;
	e->u.hpi_event.event.EventDataUnion.SensorEvent.PreviousState 
                = SAHPI_ES_UNSPECIFIED;

	set_thresholds_sensor_misc_event
		(event, &e->u.hpi_event.event.EventDataUnion.SensorEvent);

}

static void entity_presence(ipmi_entity_t	*entity,
			    int			present,
			    void		*cb_data,
			    ipmi_event_t	*event)
{
	struct oh_event	*e;
	SaHpiResourceIdT *rid = cb_data;

	e = malloc(sizeof(*e));
	if (!e) {
		dbg("Out of space");
		return;
	}

	memset(e, 0, sizeof(*e));
	e->type = OH_ET_HPI;
	e->u.hpi_event.parent = *rid;
	e->u.hpi_event.id = 0;

	e->u.hpi_event.event.Source = 0;
	/* Do not find EventType in IPMI */
	e->u.hpi_event.event.EventType = SAHPI_ET_HOTSWAP;

	/* FIXIT! */
	if (event)
		e->u.hpi_event.event.Timestamp = ipmi_get_uint32(event->data);

	/* Do not find the severity of hotswap event */
	e->u.hpi_event.event.Severity = SAHPI_MAJOR;

	if (present)
		e->u.hpi_event.event.EventDataUnion.HotSwapEvent.HotSwapState
			= SAHPI_HS_STATE_ACTIVE_HEALTHY;
	else
		e->u.hpi_event.event.EventDataUnion.HotSwapEvent.HotSwapState
			= SAHPI_HS_STATE_NOT_PRESENT;
	
}

static void add_sensor_event_thresholds(ipmi_sensor_t	*sensor,
					SaHpiSensorRecT	*rec)
{
	int 			val;
	SaHpiSensorThdMaskT	temp;
	unsigned int		access;

	if (rec->Category != SAHPI_EC_THRESHOLD) {
		rec->ThresholdDefn.IsThreshold = SAHPI_FALSE;
		return;
	}
	
	access = ipmi_sensor_get_threshold_access(sensor);
	if (access == IPMI_THRESHOLD_ACCESS_SUPPORT_NONE) {
		rec->ThresholdDefn.IsThreshold = SAHPI_FALSE;
		return;
	}

	if (access >= IPMI_THRESHOLD_ACCESS_SUPPORT_READABLE) {
		rec->ThresholdDefn.IsThreshold = SAHPI_TRUE;
		rec->ThresholdDefn.TholdCapabilities = SAHPI_SRF_RAW |
						       SAHPI_SRF_INTERPRETED;
		
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
	
	temp = 0;
	val = ipmi_sensor_get_hysteresis_support(sensor);
	if (val == IPMI_HYSTERESIS_SUPPORT_FIXED)
		temp |= SAHPI_STM_UP_HYSTERESIS | SAHPI_STM_LOW_HYSTERESIS;
	rec->ThresholdDefn.FixedThold = temp;
}

//static void add_control_event_data_format(ipmi_control_t	*control,
					   //SaHpiCtrlRecT	*rec)
//{


//}


static void add_sensor_event_data_format(ipmi_sensor_t		*sensor,
					 SaHpiSensorRecT	*rec)
{
	SaHpiSensorRangeFlagsT temp = 0;
	
	/* Depends on IPMI */
	if (rec->Category == SAHPI_EC_THRESHOLD)
		rec->DataFormat.ReadingFormats = SAHPI_SRF_RAW |
						 SAHPI_SRF_INTERPRETED;
	else
		rec->DataFormat.ReadingFormats = SAHPI_SRF_EVENT_STATE;
	
	/*No info about IsNumeric in IPMI */
	rec->DataFormat.IsNumeric = SAHPI_TRUE;
	rec->DataFormat.SignFormat = (SaHpiSensorSignFormatT)
		ipmi_sensor_get_analog_data_format(sensor);
	rec->DataFormat.BaseUnits = (SaHpiSensorUnitsT)
		ipmi_sensor_get_base_unit(sensor);
	rec->DataFormat.ModifierUnits = (SaHpiSensorUnitsT)
		ipmi_sensor_get_modifier_unit(sensor);
	rec->DataFormat.ModifierUse = (SaHpiSensorModUnitUseT)
		ipmi_sensor_get_modifier_unit_use(sensor);
	
	rec->DataFormat.FactorsStatic = SAHPI_TRUE;
	/* We use first...*/
	rec->DataFormat.Factors.M_Factor = (SaHpiInt16T)
		ipmi_sensor_get_raw_m(sensor, 0);
	rec->DataFormat.Factors.B_Factor = (SaHpiInt16T)
		ipmi_sensor_get_raw_b(sensor, 0);
	rec->DataFormat.Factors.AccuracyFactor = (SaHpiUint16T)
		ipmi_sensor_get_raw_accuracy(sensor, 0);
	rec->DataFormat.Factors.ToleranceFactor = (SaHpiUint8T)
		ipmi_sensor_get_raw_tolerance(sensor, 0);
	rec->DataFormat.Factors.ExpA = (SaHpiUint8T)
		ipmi_sensor_get_raw_accuracy_exp(sensor, 0);
	rec->DataFormat.Factors.ExpR = (SaHpiUint8T)
		ipmi_sensor_get_raw_r_exp(sensor, 0);
	rec->DataFormat.Factors.ExpB = (SaHpiUint8T)
		ipmi_sensor_get_raw_b_exp(sensor, 0);
	rec->DataFormat.Factors.Linearization = (SaHpiSensorLinearizationT)
		ipmi_sensor_get_linearization(sensor);
	
	rec->DataFormat.Percentage = (SaHpiBoolT)
		ipmi_sensor_get_percentage(sensor);

	temp |= SAHPI_SRF_MAX | SAHPI_SRF_MIN;	
	rec->DataFormat.Range.Max.ValuesPresent = SAHPI_SRF_RAW;
	rec->DataFormat.Range.Max.Raw = (SaHpiUint32T)
		ipmi_sensor_get_raw_sensor_max(sensor);
	
	rec->DataFormat.Range.Min.ValuesPresent = SAHPI_SRF_RAW;
	rec->DataFormat.Range.Max.Raw = (SaHpiUint32T)
		ipmi_sensor_get_raw_sensor_min(sensor);
		
	if (ipmi_sensor_get_nominal_reading_specified(sensor)) {
		rec->DataFormat.Range.Nominal.ValuesPresent = SAHPI_SRF_RAW;
		rec->DataFormat.Range.Nominal.Raw = (SaHpiUint32T)
			ipmi_sensor_get_raw_nominal_reading(sensor);
		temp |= SAHPI_SRF_NOMINAL;
	}

	if (ipmi_sensor_get_normal_max_specified(sensor)) {
		rec->DataFormat.Range.NormalMax.ValuesPresent = SAHPI_SRF_RAW;
		rec->DataFormat.Range.NormalMax.Raw = (SaHpiUint32T)
			ipmi_sensor_get_raw_normal_max(sensor);
		temp |= SAHPI_SRF_NORMAL_MAX;
	}
	
	if (ipmi_sensor_get_normal_min_specified(sensor)) {
		rec->DataFormat.Range.NormalMin.ValuesPresent = SAHPI_SRF_RAW;
		rec->DataFormat.Range.NormalMin.Raw = (SaHpiUint32T)
			ipmi_sensor_get_raw_normal_min(sensor);
		temp |= SAHPI_SRF_NORMAL_MIN;
	}	
	rec->DataFormat.Range.Flags = temp;
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

static void add_control_event_control_rec(ipmi_control_t	*control,
					SaHpiCtrlRecT	*rec)
{
        static int control_num = 0;
	int	control_type;
	rec->Num = ++control_num;
	control_type = ipmi_control_get_type(control);
	switch(control_type) {

		/* This is a special case we're handle later */
		//case IPMI_CONTROL_RESET:
		//case IPMI_CONTROL_POWER:
		//case IPMI_CONTROL_ONE_SHOT_RESET:
			//break;
			
		/* **FIXME: need to verify this mapping */
		/* assuming binary control */
		case IPMI_CONTROL_ALARM:
		case IPMI_CONTROL_DISPLAY:
		case IPMI_CONTROL_LIGHT:
		case IPMI_CONTROL_RELAY:
		case IPMI_CONTROL_FAN_SPEED:
		case IPMI_CONTROL_IDENTIFIER:
			rec->Type = SAHPI_CTRL_TYPE_DIGITAL;
			break;
		default:
			rec->Type = SAHPI_CTRL_TYPE_OEM;
	}
		
	
	rec->Ignore = (SaHpiBoolT)ipmi_control_get_ignore_if_no_entity(control);

	//add_control_event_data_format(control, rec);

	/* We do not care about oem. */
	rec->Oem = 0;
}

static void add_sensor_event_sensor_rec(ipmi_sensor_t	*sensor,
					SaHpiSensorRecT	*rec)
{
        static int sensor_num = 0;
	rec->Num = ++sensor_num;
	rec->Type = (SaHpiSensorTypeT)ipmi_sensor_get_sensor_type(sensor);
	rec->Category = (SaHpiEventCategoryT)
		ohoi_sensor_get_event_reading_type(sensor);
	rec->EventCtrl = (SaHpiSensorEventCtrlT)
		ipmi_sensor_get_event_support(sensor);
	/* Cannot find Events in IPMI. */
	rec->Events = 0xffff;
	rec->Ignore = (SaHpiBoolT)ipmi_sensor_get_ignore_if_no_entity(sensor);

	add_sensor_event_data_format(sensor, rec);

	add_sensor_event_thresholds(sensor, rec);
	
	/* We do not care about oem. */
	rec->Oem = 0;
}

static void add_control_event_rdr(ipmi_control_t		*control, 
				 SaHpiRdrT		*rdr,
				 SaHpiEntityPathT	parent_ep,
				 SaHpiResourceIdT	res_id)
{
	char	name[32];
	//SaHpiEntityPathT rdr_ep;

	memset(name,'\0',32);
	rdr->RecordId = 0;
	rdr->RdrType = SAHPI_CTRL_RDR;
	//rdr->Entity.Entry[0].EntityType = (SaHpiEntityTypeT)id;
	//rdr->Entity.Entry[0].EntityInstance = (SaHpiEntityInstanceT)instance;
	//rdr->Entity.Entry[1].EntityType = 0;
	//rdr->Entity.Entry[1].EntityInstance = 0;
	rdr->Entity = parent_ep;

	/* append ep */
	//string2entitypath(entity_root, &rdr_ep);
	//append_root(&rdr_ep);

	//ep_concat (&rdr->Entity, &rdr_ep);

	add_control_event_control_rec(control, &rdr->RdrTypeUnion.CtrlRec);

	ipmi_control_get_id(control, name, 32);
	rdr->IdString.DataType = SAHPI_TL_TYPE_ASCII6;
	rdr->IdString.Language = SAHPI_LANG_ENGLISH;
	rdr->IdString.DataLength = 32;

	memcpy(rdr->IdString.Data,name, strlen(name) + 1);
}

static void add_sensor_event_rdr(ipmi_sensor_t		*sensor, 
				 SaHpiRdrT		*rdr,
				 SaHpiEntityPathT	parent_ep,
				 SaHpiResourceIdT	res_id)
{
	char	name[32];
	//SaHpiEntityPathT rdr_ep;

	memset(name,'\0',32);
	rdr->RecordId = 0;
	rdr->RdrType = SAHPI_SENSOR_RDR;
	//rdr->Entity.Entry[0].EntityType = (SaHpiEntityTypeT)id;
	//rdr->Entity.Entry[0].EntityInstance = (SaHpiEntityInstanceT)instance;
	//rdr->Entity.Entry[1].EntityType = 0;
	//rdr->Entity.Entry[1].EntityInstance = 0;
	rdr->Entity = parent_ep;

	/* append ep */
	//string2entitypath(entity_root, &rdr_ep);
	//append_root(&rdr_ep);

	//ep_concat (&rdr->Entity, &rdr_ep);

	add_sensor_event_sensor_rec(sensor, &rdr->RdrTypeUnion.SensorRec);

	ipmi_sensor_get_id(sensor, name, 32);
	rdr->IdString.DataType = SAHPI_TL_TYPE_ASCII6;
	rdr->IdString.Language = SAHPI_LANG_ENGLISH;
	rdr->IdString.DataLength = 32;

	memcpy(rdr->IdString.Data,name, strlen(name) + 1);
}

static void add_control_event(ipmi_entity_t	*ent,
			     ipmi_control_t	*control,
			     struct oh_handler_state *handler,
			     SaHpiEntityPathT	parent_ep,
			     SaHpiResourceIdT	rid)
{
        ipmi_control_id_t        *control_id; 
	struct oh_event         *e;

        control_id = malloc(sizeof(*control_id));
        if (!control_id) {
                dbg("Out of memory");
                return;
        }
        *control_id = ipmi_control_convert_to_id(control);
        
	e = malloc(sizeof(*e));
	if (!e) {
                free(control_id);
		dbg("Out of space");   
		return;
	}
	memset(e, 0, sizeof(*e));

	e->type = OH_ET_RDR;

	add_control_event_rdr(control, &e->u.rdr_event.rdr, parent_ep, rid);	

	rid = oh_uid_lookup(&e->u.rdr_event.rdr.Entity);

	oh_add_rdr(handler->rptcache, rid, &e->u.rdr_event.rdr, control_id, 0);
}

static void control_change(enum ipmi_update_e op,
			   ipmi_entity_t    *ent,
			   ipmi_control_t   *control,
			   void		    *cb_data)
{
	struct oh_handler_state *handler = cb_data;
	
	char			name[33];
	//int			rv;
	ipmi_entity_id_t	entity_id;
	
	SaHpiRptEntryT		*rpt_entry;

	ipmi_control_get_id(control, name, 32);
	
	entity_id = ipmi_entity_convert_to_id(ent);
	
	rpt_entry = get_resource_by_entityid(
			handler->rptcache,
			&entity_id);

	if (!rpt_entry) {
		dump_entity_id("Control with RPT Entry?!", entity_id);
		return;
	}

	if (op == IPMI_ADDED) {
		struct ohoi_resource_id *ohoi_res_id;
		ohoi_res_id = oh_get_resource_data(handler->rptcache, rpt_entry->ResourceId);
		
		/* skip Chassis with for now since all we have in hardware
		   is power and reset */
		dbg("resource: %s", rpt_entry->ResourceTag.Data);
		if((strcmp(rpt_entry->ResourceTag.Data,"system_chassis")) == 0) {
			dbg("controls on chassis are probably power/reset...skipping");
			return;
		} else {
			rpt_entry->ResourceCapabilities |= SAHPI_CAPABILITY_CONTROL; 
				 
			oh_add_resource(handler->rptcache, rpt_entry, ohoi_res_id, 0);
			add_control_event(ent, control, handler, 
        	                         rpt_entry->ResourceEntity, 
                	                 rpt_entry->ResourceId);
		}
		

	}
		

	

}
	
static void add_sensor_event(ipmi_entity_t	*ent,
			     ipmi_sensor_t	*sensor,
			     struct oh_handler_state *handler,
			     SaHpiEntityPathT	parent_ep,
			     SaHpiResourceIdT	rid)
{
        ipmi_sensor_id_t        *sensor_id; 
	struct oh_event         *e;

        sensor_id = malloc(sizeof(*sensor_id));
        if (!sensor_id) {
                dbg("Out of memory");
                return;
        }
        *sensor_id = ipmi_sensor_convert_to_id(sensor);
        
	e = malloc(sizeof(*e));
	if (!e) {
                free(sensor_id);
		dbg("Out of space");   
		return;
	}
	memset(e, 0, sizeof(*e));

	e->type = OH_ET_RDR;
	add_sensor_event_rdr(sensor, &e->u.rdr_event.rdr, parent_ep, rid);	

	rid = oh_uid_lookup(&e->u.rdr_event.rdr.Entity);

	oh_add_rdr(handler->rptcache, rid, &e->u.rdr_event.rdr, sensor_id, 1);
}

static void sensor_change(enum ipmi_update_e op,
			  ipmi_entity_t      *ent,
			  ipmi_sensor_t      *sensor,
			  void               *cb_data)
{
	char			name[33];    
	int			rv;
	struct oh_handler_state *handler = cb_data;
        ipmi_entity_id_t entity_id;
        SaHpiRptEntryT *rpt_entry;
     
	ipmi_sensor_get_id(sensor, name, 32);

        entity_id = ipmi_entity_convert_to_id(ent);

        rpt_entry = get_resource_by_entityid(
                        handler->rptcache,
                        &entity_id);
        if (!rpt_entry) {
                dump_entity_id("Sensor without RPT Entry?!", entity_id);
                return;
        }

	if ( op == IPMI_ADDED ) {
                /* XXX Ugly! Need to change rtp_util */
                struct ohoi_resource_id *ohoi_res_id;     
                ohoi_res_id = oh_get_resource_data(handler->rptcache, rpt_entry->ResourceId);
		
                rpt_entry->ResourceCapabilities |=  SAHPI_CAPABILITY_RDR 
                                                    | SAHPI_CAPABILITY_SENSOR;
                /* we update the resource in the cache with new capability */
                oh_add_resource(handler->rptcache, rpt_entry, ohoi_res_id, 1);

                /* fill in the sensor data, add it to ipmi_event_list
		 * and finally to the rpt-cache
		 */		 
		add_sensor_event(ent, sensor, handler, 
                                 rpt_entry->ResourceEntity, 
                                 rpt_entry->ResourceId);

		if (ipmi_sensor_get_event_reading_type(sensor) == 
				IPMI_EVENT_READING_TYPE_THRESHOLD) 
			rv = ipmi_sensor_threshold_set_event_handler(
					sensor, sensor_threshold_event, handler);
		else
			rv = ipmi_sensor_discrete_set_event_handler(
					sensor, sensor_discrete_event, handler);

		if (rv)
			dbg("Unable to reg sensor event handler: %#x\n", rv);
	}
}

static void get_mc_entity_event(ipmi_mc_t	*mc,
			        SaHpiRptEntryT	*entry)
{
	uint8_t	vals[4];
	SaHpiEntityPathT mc_ep;
	char mc_name[128];
        
	memset(&mc_ep, 0, sizeof(SaHpiEntityPathT));
	dbg("entity_root: %s", entity_root);
	string2entitypath(entity_root, &mc_ep);
	append_root(&mc_ep);

        snprintf(mc_name, sizeof(mc_name),
                 "Management Controller(%x, %x)",
                 ipmi_mc_get_channel(mc), 
                 ipmi_mc_get_address(mc));
        
	entry->EntryId = 0;
	entry->ResourceInfo.ResourceRev = 0;
	entry->ResourceInfo.SpecificVer = 0;
	entry->ResourceInfo.DeviceSupport = 0;
	entry->ResourceInfo.ManufacturerId =
		(SaHpiManufacturerIdT)ipmi_mc_manufacturer_id(mc);
	entry->ResourceInfo.ProductId = (SaHpiUint16T)ipmi_mc_product_id(mc);
	entry->ResourceInfo.FirmwareMajorRev =
		(SaHpiUint8T)ipmi_mc_major_fw_revision(mc);
	entry->ResourceInfo.FirmwareMinorRev =
		(SaHpiUint8T)ipmi_mc_minor_fw_revision(mc);
	
	ipmi_mc_aux_fw_revision(mc, vals);
	/* There are 4, and we only use first. */
	entry->ResourceInfo.AuxFirmwareRev = (SaHpiUint8T)vals[0];
	entry->ResourceEntity.Entry[0].EntityType = SAHPI_ENT_SYS_MGMNT_MODULE ;
	entry->ResourceEntity.Entry[0].EntityInstance = 0;
	entry->ResourceCapabilities = SAHPI_CAPABILITY_SEL;
	entry->ResourceSeverity = SAHPI_OK;
	entry->DomainId = 0;
	entry->ResourceTag.DataType = SAHPI_TL_TYPE_ASCII6;
	entry->ResourceTag.Language = SAHPI_LANG_ENGLISH;
	entry->ResourceTag.DataLength = strlen(mc_name); 
	memcpy(entry->ResourceTag.Data, mc_name, strlen(mc_name)+1);

	ep_concat(&entry->ResourceEntity, &mc_ep);
        
	entry->ResourceId = oh_uid_from_entity_path(&mc_ep);
	dbg("MC ResourceId: %d", (int)entry->ResourceId);
}

static void get_entity_event(ipmi_entity_t	*entity,
			     SaHpiRptEntryT	*entry)
{
	char	*str;
	SaHpiEntityPathT entity_ep;

	entry->ResourceInfo.ResourceRev = 0;
	entry->ResourceInfo.SpecificVer = 0;
	entry->ResourceInfo.DeviceSupport = 0;
	entry->ResourceInfo.ManufacturerId = 0;
	entry->ResourceInfo.ProductId = 0;
	entry->ResourceInfo.FirmwareMajorRev = 0;
	entry->ResourceInfo.FirmwareMinorRev = 0;
	
	entry->ResourceInfo.AuxFirmwareRev = 0;
	entry->ResourceEntity.Entry[0].EntityType 
                = ipmi_entity_get_entity_id(entity);
	entry->ResourceEntity.Entry[0].EntityInstance 
                = ipmi_entity_get_entity_instance(entity);
	entry->ResourceEntity.Entry[1].EntityType = 0;
	entry->ResourceEntity.Entry[1].EntityInstance = 0;
	
	/* let's append entity_root from config */

	string2entitypath(entity_root, &entity_ep);
	append_root(&entity_ep);

	ep_concat(&entry->ResourceEntity, &entity_ep);

	entry->EntryId = 0;
	entry->ResourceId = oh_uid_from_entity_path(&entry->ResourceEntity);

	entry->ResourceCapabilities = SAHPI_CAPABILITY_RESOURCE;
	entry->ResourceSeverity = SAHPI_OK;
	entry->DomainId = 0;
	entry->ResourceTag.DataType = SAHPI_TL_TYPE_ASCII6;
	
	str = ipmi_entity_get_entity_id_string(entity);
	
	entry->ResourceTag.Language = SAHPI_LANG_ENGLISH;
	entry->ResourceTag.DataLength = strlen(str); 

	memcpy(entry->ResourceTag.Data, str, strlen(str) + 1);
}

static void add_entity_event(ipmi_entity_t	        *entity,
			     struct oh_handler_state    *handler)
{
        struct ohoi_resource_id *ohoi_res_id;
	struct oh_event	*e;
	int 		rv;
        
        ohoi_res_id = g_malloc0(sizeof(*ohoi_res_id));
        if (!ohoi_res_id) {
                dbg("Out of memory");
                return;
        }	
	e = g_malloc0(sizeof(*e));
	if (!e) {
                free(ohoi_res_id);
		dbg("Out of space");   
		return;
	}

        ohoi_res_id->type       = OHOI_RESOURCE_ENTITY; 
        ohoi_res_id->u.entity_id= ipmi_entity_convert_to_id(entity);
        
	memset(e, 0, sizeof(*e));
	e->type = OH_ET_RESOURCE;			
	get_entity_event(entity, &(e->u.res_event.entry));	

	handler->eventq = g_slist_append(handler->eventq, e);

	/* sensors */
	oh_add_resource(handler->rptcache, &(e->u.res_event.entry), ohoi_res_id, 1);
	rv= ipmi_entity_set_sensor_update_handler(entity, sensor_change,
						  handler);
	if (rv) {
		dbg("ipmi_entity_set_sensor_update_handler: %#x", rv);
		return;
	}
	
	/* controls */
	rv = ipmi_entity_set_control_update_handler(entity, control_change,
							handler);
	if (rv) {
		dbg("ipmi_entity_set_control_update_handler: %#x", rv);
		return;
	}

	/* entity presence overall */
	rv = ipmi_entity_set_presence_handler(entity, entity_presence, &e->u.res_event.entry.ResourceId);
	if (rv) {
		dbg("ipmi_entity_set_presence_handler: %#x", rv);
		return;
	}

}

static void entity_change(enum ipmi_update_e	op,
			  ipmi_domain_t		*domain,
			  ipmi_entity_t 	*entity,
			  void 			*cb_data)
{
	struct oh_handler_state *handler = cb_data;

	if (op == IPMI_ADDED) {
		add_entity_event(entity, handler);
        } else if (op == IPMI_DELETED) {
		dbg("Entity deleted: %d.%d", 
                    ipmi_entity_get_entity_id(entity), 
                    ipmi_entity_get_entity_instance(entity));
        } else if (op == IPMI_CHANGED) {
		dbg("Entity changed: %d.%d",
                    ipmi_entity_get_entity_id(entity), 
                    ipmi_entity_get_entity_instance(entity));
        } else {
                dbg("Entity: Unknow change?!");
        }

}

static void SDRs_read_done(ipmi_domain_t *domain, int err, void *cb_data)
{
	int *flag = cb_data;
	*flag = 1;
	dbg("SDRs read done");
	return;
}

static void SELs_read_done(ipmi_domain_t *domain, int err, void *cb_data)
{
	int *flag = cb_data;
	*flag = 1;
	dbg("SELs read done");
	return;
}

static void bus_scan_done(ipmi_domain_t *domain, int err, void *cb_data)
{
	ipmi_domain_reread_sels(domain, SELs_read_done, cb_data);
	dbg("bus scan done");
	return;
}

static void mc_add(ipmi_mc_t                    *mc,
                   struct oh_handler_state      *handler)
{
        struct ohoi_resource_id *ohoi_res_id;
        struct oh_event *e;
        
        ohoi_res_id = g_malloc0(sizeof(*ohoi_res_id));
        if (!ohoi_res_id) {
                dbg("Out of space");
                return;
        }
        ohoi_res_id->type       = OHOI_RESOURCE_MC;
        ohoi_res_id->u.mc_id    = ipmi_mc_convert_to_id(mc);
                
	e = malloc(sizeof(*e));
	if (!e) {
		dbg("Out of space");   
		return;
	}
	memset(e, 0, sizeof(*e));

	e->type = OH_ET_RESOURCE;

	get_mc_entity_event(mc, &(e->u.res_event.entry));

	/* add to rptcache */
	oh_add_resource(handler->rptcache, &(e->u.res_event.entry), ohoi_res_id, 1);

}

static void
mc_change(enum ipmi_update_e op,
          ipmi_domain_t      *domain,
          ipmi_mc_t          *mc,
          void               *cb_data)
{
        struct oh_handler_state *handler = cb_data;
        int  addr = ipmi_mc_get_address(mc);
        int  channel = ipmi_mc_get_channel(mc);

        switch (op) {
                case IPMI_ADDED:
                        mc_add(mc, handler);
                        dbg("MC added: (%d %x)\n", channel, addr);
                        break;
                case IPMI_DELETED:
                        dbg("MC deleted: (%d %x)\n", channel, addr);
                        break;
                case IPMI_CHANGED:
                        dbg("MC changed: (%d %x)\n", channel, addr);
                        break;
        }
}

void ohoi_setup_done(ipmi_domain_t	*domain,
			int 		err, 
			unsigned int	conn_num,
			unsigned int	port_num,
			int		still_connected,
			void 		*user_data)
{
        /* Why need this, Corey? */
        static ipmi_domain_mc_upd_t *mc_update_handler_id;
        
	struct oh_handler_state *handler = user_data;
	struct ohoi_handler *ipmi_handler = handler->data;
        int rv;

	rv = ipmi_domain_enable_events(domain);
    	if (rv)
    	    dbg("ipmi_domain_enable_events return error %d", rv);


	rv = ipmi_domain_set_entity_update_handler(domain, entity_change, 
			 			  handler);
	if (rv)
		dbg("ipmi_bmc_iterate_entities return error");
	
	rv = ipmi_domain_set_main_SDRs_read_handler(domain, SDRs_read_done,
                                                    &ipmi_handler->SDRs_read_done);
	if (rv)
		dbg("ipmi_domain_set_main_SDRs_read_handler return error: %d\n", rv);
	
        rv = ipmi_domain_set_bus_scan_handler(domain, bus_scan_done,
					      &ipmi_handler->SELs_read_done);
	if (rv) 
		dbg("ipmi_domain_set_bus_scan_handler return error: %d\n", rv);

        rv = ipmi_domain_register_mc_update_handler(domain, mc_change, handler,
                                                    &mc_update_handler_id);

        if (rv)
                dbg("ipmi_domain_register_mc_update_handler return error: %d\n", rv);
            
}
