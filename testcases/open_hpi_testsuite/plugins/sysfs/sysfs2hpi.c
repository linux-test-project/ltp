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
 *     Rusty Lynch <rusty.lynch@linux.intel.com>
 *     Julie Fleischer <julie.n.fleischer@intel.com>
 *     Tariq Shureih <tariq.shureih@intel.com>
 *     Racing Guo <racing.guo@intel.com>
 */

#include <stdlib.h>
#include <string.h>
#include <libsysfs.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_handler.h>
#include <oh_domain.h>
#include <oh_error.h>

#define SYSFS2HPI_ERROR -700
#define SCRATCHSIZE 60

int g_num_resources=0; /* total # resources */
SaHpiEntityPathT g_epbase; /* root entity path (from config) */

struct sysfsitems {
	GSList *resources;
	struct sysfs_bus *bus;
	int initialized;
};

struct sensor {
	int num;
	char name[SYSFS_NAME_LEN];
	struct sysfs_attribute *max;
	struct sysfs_attribute *min;
	struct sysfs_attribute *value;
	struct sysfs_attribute *div;
	SaHpiBoolT evt_enable;
};

struct resource {
	SaHpiEntityPathT path;
	char name[SYSFS_NAME_LEN];
	GSList *sensors;
};


static inline void reading_int64_set(SaHpiSensorReadingT *reading, int value)
{
        reading->IsSupported = SAHPI_TRUE;
        reading->Type = SAHPI_SENSOR_READING_TYPE_INT64;
        reading->Value.SensorInt64 = value;
}


/**
 * *sysfs2hpi_open:
 * @handler_config: pointer to config file
 *
 * This function creates an instance for the sysfs plugin
 * and returns a handler to the instance.
 * The two input parameters name and addr for the
 * mechanism's name and address, respectively, are N/A
 * for a sysfs plugin.
 **/
static void *sysfs2hpi_open(GHashTable *handler_config,
                            unsigned int hid,
                            oh_evt_queue *eventq)
{
        struct oh_handler_state *hnd;
	struct sysfsitems *sys;
        char *er;

	if (!handler_config) {
		err("empty handler_config");
		return NULL;
	}

	/* set up entity root in g_epbase */
        er = (char *)g_hash_table_lookup(handler_config,"entity_root");
        if (!er) {
                err("no entity root present");
		return NULL;
        }
        oh_encode_entitypath(er, &g_epbase);

        hnd = malloc(sizeof(*hnd));
        if (!hnd) {
		err("unable to allocate main handler");
		return NULL;
	}

	memset(hnd, '\0', sizeof(*hnd));

	/* assign config to handler_config and initialize rptcache */
        hnd->config = handler_config;
        hnd->hid = hid;
        hnd->eventq = eventq;

        hnd->rptcache = (RPTable *)g_malloc0(sizeof(RPTable));

	oh_init_rpt(hnd->rptcache);

	sys = malloc(sizeof(*sys));
	if (!sys) {
		err("unable to allocate sysfsitems structure");
		return NULL;
	}
	memset(sys, '\0', sizeof(*sys));

	hnd->data = (void *) sys;

        return hnd;

}

/**
 * sysfs2hpi_close:
 * @hnd: pointer to instance
 *  
 * Close the instance for the sysfs plugin. 
 * Note:  There is currently no way to test this code
 * as it is not called by the framework.
 **/

static void sysfs2hpi_close(void *hnd)
{
	GSList *tmp;
	struct resource *r;
	struct sysfsitems *sys;

	struct oh_handler_state *inst = (struct oh_handler_state *)hnd;

	if (!inst) {
		err("no instance to delete");
		return;
	}

	sys = inst->data;
	sysfs_close_bus(sys->bus);

	/* Free resources and their sensors */
	if (g_slist_length(sys->resources) != 0) {
		g_slist_for_each(tmp, sys->resources) {
			r = (struct resource *)tmp->data;
			g_slist_free(r->sensors);
			g_slist_free(sys->resources);
		}
	}
	/* Free main instance */
	free(inst);
}

/**
 * sysfs2hpi_get_event:
 * @hnd: pointer to handler instance
 *
 * This function gets a sysfs event from the sysfs event table
 * in instance.events.
 *
 * Return value: 0 if times out, > 0 is event is returned.
 **/
static int sysfs2hpi_get_event(void *hnd)
{
	return 0;
}

/**
 * sysfs2hpi_setup_rdr:
 * @type: Sensor type
 * @str: string holding sensor name
 * @num_sensors: sensor number
 * @d: pointer to struct sysfs_device for this sensor
 * @r: pointer to struct resource for this sensor
 * @inst: pointer to instance
 *
 * Helper function to sysfs2hpi_assign_rdrs().
 * Set up the RDR for either current, fan, voltage,
 * or temperature.
 *
 * Return value: 0 for success | Error code.
 **/
static int sysfs2hpi_setup_rdr(SaHpiSensorTypeT type,
                               const char* str, int num_sensors,
                               struct sysfs_device* d, struct resource* r,
                               struct oh_handler_state* inst,
                               struct oh_event *e)
{
	struct sensor *s;
	unsigned char strinput[SYSFS_NAME_LEN];
	int puid;
        SaHpiSensorDataFormatT *frmt;
        SaHpiRdrT *tmprdr;

	if ((type != SAHPI_TEMPERATURE) && (type != SAHPI_VOLTAGE) &&
       		(type != SAHPI_CURRENT) && (type != SAHPI_FAN))	{
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	s = (struct sensor*)malloc(sizeof(*s));
	if (!s) {
		err("unable to allocate sensor");
		return SA_ERR_HPI_OUT_OF_SPACE;
	}
	memset(s,'\0',sizeof(*s));			
	s->num = num_sensors;

	switch(type) {
		case SAHPI_TEMPERATURE:
			snprintf(s->name, SYSFS_NAME_LEN, "%i:Temp Sensor",s->num);
		
			snprintf((char*)strinput, SYSFS_NAME_LEN, "temp_input%s", str);
			s->value = sysfs_get_device_attr(d, (char*)strinput);
			snprintf((char*)strinput, SYSFS_NAME_LEN, "temp_max%s", str);
			s->max = sysfs_get_device_attr(d, (char*)strinput);
			snprintf((char*)strinput, SYSFS_NAME_LEN, "temp_min%s", str);
			s->min = sysfs_get_device_attr(d, (char*)strinput);
			s->div = NULL;
			break;
		case SAHPI_VOLTAGE:
			snprintf(s->name, SYSFS_NAME_LEN, "%i:Voltage Sensor",s->num);
		
			snprintf((char*)strinput, SYSFS_NAME_LEN, "in_input%s", str);
			s->value = sysfs_get_device_attr(d, (char*)strinput);
			snprintf((char*)strinput, SYSFS_NAME_LEN, "in_max%s", str);
			s->max = sysfs_get_device_attr(d, (char*)strinput);
			snprintf((char*)strinput, SYSFS_NAME_LEN, "in_min%s", str);
			s->min = sysfs_get_device_attr(d, (char*)strinput);
			s->div = NULL;
			break;
		case SAHPI_CURRENT:
			snprintf(s->name, SYSFS_NAME_LEN, "%i:Current Sensor",s->num);
		
			snprintf((char*)strinput, SYSFS_NAME_LEN, "curr_input%s", str);
			s->value = sysfs_get_device_attr(d, (char*)strinput);
			snprintf((char*)strinput, SYSFS_NAME_LEN, "curr_max%s", str);
			s->max = sysfs_get_device_attr(d, (char*)strinput);
			snprintf((char*)strinput, SYSFS_NAME_LEN, "curr_min%s", str);
			s->min = sysfs_get_device_attr(d, (char*)strinput);
			s->div = NULL;
			break;
		case SAHPI_FAN:
			snprintf(s->name, SYSFS_NAME_LEN, "%i:Fan Sensor",s->num);	
			snprintf((char*)strinput, SYSFS_NAME_LEN, "fan_input%s", str);
			s->value = sysfs_get_device_attr(d, (char*)strinput);
			snprintf((char*)strinput, SYSFS_NAME_LEN, "fan_max%s", str);
			s->max = sysfs_get_device_attr(d, (char*)strinput);
			snprintf((char*)strinput, SYSFS_NAME_LEN, "fan_min%s", str);
			s->min = sysfs_get_device_attr(d, (char*)strinput);
			snprintf((char*)strinput, SYSFS_NAME_LEN, "fan_div%s", str);
			s->div = sysfs_get_device_attr(d, (char*)strinput);
			break;
		default: /* should never be executed */
			return SA_ERR_HPI_INVALID_PARAMS;
	}

	if (!s->value && !s->max && !s->min) {
		/* RDR doesn't exist */
		free(s);
		return SYSFS2HPI_ERROR;
	}

	r->sensors = g_slist_append(r->sensors, s);

        tmprdr = (SaHpiRdrT *)malloc(sizeof(SaHpiRdrT));
        if (!tmprdr) return SA_ERR_HPI_OUT_OF_SPACE;

	tmprdr->RecordId = num_sensors;
	tmprdr->RdrType = SAHPI_SENSOR_RDR;
	tmprdr->Entity.Entry[0].EntityType = SAHPI_ENT_SYS_MGMNT_SOFTWARE;
	tmprdr->Entity.Entry[0].EntityLocation = g_num_resources;
	tmprdr->Entity.Entry[1].EntityType = SAHPI_ENT_OTHER_SYSTEM_BOARD;
	tmprdr->Entity.Entry[1].EntityLocation = 0; /* 0 b/c only 1 board */
        oh_concat_ep( &tmprdr->Entity, &g_epbase);
	tmprdr->RdrTypeUnion.SensorRec.Num = num_sensors;
	tmprdr->RdrTypeUnion.SensorRec.Type = type;

	/* Ignoring .Category, .EventCtrl, and .Events b/c sysfs has no events */
        frmt = &tmprdr->RdrTypeUnion.SensorRec.DataFormat;
        frmt->IsSupported = SAHPI_TRUE;
        frmt->ReadingType = SAHPI_SENSOR_READING_TYPE_INT64;

	switch(type) {
		case SAHPI_TEMPERATURE:
			/* Interpreted temperature is in degrees Celcius */
			frmt->BaseUnits = SAHPI_SU_DEGREES_C;
			break;
		case SAHPI_VOLTAGE:
			/* Interpreted voltage is in Volts */
			frmt->BaseUnits = SAHPI_SU_VOLTS;
			break;
		case SAHPI_CURRENT:
			/* Interpreted current is in Amps */
			frmt->BaseUnits = SAHPI_SU_AMPS;
			break;
		case SAHPI_FAN:
			/* Interpreted fan is in RPMs */
			frmt->BaseUnits = SAHPI_SU_RPM;
			break;
		default: /* should never be executed */
			return SA_ERR_HPI_INVALID_PARAMS;
        }
       
        frmt->ModifierUnits = 0;
        frmt->ModifierUse = 0;
        frmt->Percentage = 0;
/* Fix Me : I don't know range*/
        //frmt->Range = 0;
        frmt->AccuracyFactor = 0;

#if 0
	e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.Ignore = FALSE;
	e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.DataFormat.ReadingFormats = 
		SAHPI_SRF_RAW | SAHPI_SRF_INTERPRETED;
	e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.DataFormat.IsNumeric = TRUE;
	switch(type) {
		case SAHPI_TEMPERATURE:
			/* Interpreted temperature is in degrees Celcius */
			e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_DEGREES_C;
			break;
		case SAHPI_VOLTAGE:
			/* Interpreted voltage is in Volts */
			e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_VOLTS;
			break;
		case SAHPI_CURRENT:
			/* Interpreted current is in Amps */
			e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_AMPS;
			break;
		case SAHPI_FAN:
			/* Interpreted fan is in RPMs */
			e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_RPM;
			break;
		default: /* should never be executed */
			return SA_ERR_HPI_INVALID_PARAMS;
	}
	e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.ThresholdDefn.IsThreshold = SAHPI_TRUE;
	e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.ThresholdDefn.TholdCapabilities = 
								SAHPI_STC_RAW|SAHPI_STC_INTERPRETED;
	e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold = 
								SAHPI_STM_LOW_CRIT | SAHPI_STM_UP_CRIT;
	e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold = 
								SAHPI_STM_LOW_CRIT | SAHPI_STM_UP_CRIT;
	e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.ThresholdDefn.FixedThold = 0;
	e->u.rdr_event.rdr.IdString.DataType = SAHPI_TL_TYPE_ASCII6;
	e->u.rdr_event.rdr.IdString.Language = SAHPI_LANG_ENGLISH;
	e->u.rdr_event.rdr.IdString.DataLength = strlen(s->name);
	strcpy(e->u.rdr_event.rdr.IdString.Data, s->name);
#endif

	puid = oh_uid_lookup(&tmprdr->Entity);
	if (puid < 0) {
		err("could not find correct parent");
		return SA_ERR_HPI_ERROR;
	}

	if (oh_add_rdr(inst->rptcache, puid, tmprdr, (void *)s, 0)) {
		err("unable to add RDR to RPT");
		return SA_ERR_HPI_ERROR;
	}

        e->rdrs = g_slist_append(e->rdrs, tmprdr); /* Append RDR to event */

	return 0;
}

/**
 * sysfs2hpi_assign_rdrs:
 * @d: pointer to sysfs_device structure
 * @r: pointer to resource structure
 * @inst: pointer to instance
 *
 * Helper function for sysfs2hpi_assign_resource().
 * Assigns RDR values.
 * Currently, there are the following possible RDRs:
 * - current
 * - fan
 * - voltage
 * - temp
 * The fields:  alarms, beep, eeprom, pwm, vid, and vrm
 * are not used at this time.
 *
 * Return value: 
 **/
static int sysfs2hpi_assign_rdrs(struct sysfs_device* d,
                                 struct resource* r,
                                 struct oh_handler_state* inst,
                                 struct oh_event *e)
{
	int num_sensors = 0;
	char str[2];
	int i;

	/* Max # fan, voltage, temp, current entries is set by
	   sysfs.  Hard-coded here.  Voltage starts at 0.  All
	   others start at 1.  Current has no max. */

	const int fanmax = 3;
	const int voltagemax = 8;
	const int tempmax = 3;

	/* Set up current RDR */
	i=0;
	while(i != -1) {
		i++;
		snprintf(str, sizeof(str), "%d", i);
		if (sysfs2hpi_setup_rdr(SAHPI_CURRENT, str, 
                                        ++num_sensors, d, r, inst, e) != 0) {
			i=-1; /* keep going until we get an error returned */
			num_sensors--;
		}
	}
	
	/* Set up fan RDR */
	for (i=1;i<=fanmax;i++) {
		snprintf(str, sizeof(str), "%d", i);
		if (sysfs2hpi_setup_rdr(SAHPI_FAN, str, 
                                        ++num_sensors, d, r, inst, e) != 0) {
			num_sensors--;
		}
	}
	
	/* Set up voltage RDR */
	for (i=0;i<=voltagemax;i++) {
		snprintf(str, sizeof(str), "%d", i);
		if (sysfs2hpi_setup_rdr(SAHPI_VOLTAGE, str, 
                                        ++num_sensors, d, r, inst, e) != 0) {
			num_sensors--;
		}
	}
	
	/* Set up temp RDR */
	for (i=1;i<=tempmax;i++) {
		snprintf(str, sizeof(str), "%d", i);
		if (sysfs2hpi_setup_rdr(SAHPI_TEMPERATURE, str, 
                                        ++num_sensors, d, r, inst, e) != 0) {
			num_sensors--;
		}
	}

	return 0;
}

/**
 * sysfs2hpi_assign_resource:
 * @d: pointer to sysfs_device structure
 * @inst: pointer to instance
 *
 * Helper function for sysfs2hpi_discover_resources().
 * This function fills in the resource structure and
 * places the event for the resource on the internal
 * event queue.
 * Currently, all resources for sysfs are critical
 * as there is no way to differentiate priority
 * between devices.  [No data available to tell
 * location of device -- i2c bus # doesn't; sysfs
 * data is all for one device and also doesn't tell
 * location.]
 *
 * Return value: 0 for success | Error code
 **/
static int sysfs2hpi_assign_resource(struct sysfs_device* d, 
		struct oh_handler_state* inst)
{
	struct oh_event *e;
	struct resource *r;
	struct sysfsitems *sys;


	r = (struct resource *)malloc(sizeof(*r));
	if (!r) {
		err("unable to allocate resource");
		return SA_ERR_HPI_OUT_OF_SPACE;
	}
	memset(r,'\0',sizeof(*r));
	r->path.Entry[0].EntityType = SAHPI_ENT_SYS_MGMNT_SOFTWARE;
	r->path.Entry[0].EntityLocation = g_num_resources;
	r->path.Entry[1].EntityType = SAHPI_ENT_OTHER_SYSTEM_BOARD;
	r->path.Entry[1].EntityLocation = 0; /* 0 b/c only 1 board */
	//snprintf(r->name, "%s_%s", d->name, d->bus_id);
	strncpy(r->name,d->name,SYSFS_NAME_LEN);	
	sys = inst->data;
	sys->resources = g_slist_append(sys->resources, r);

	e = (struct oh_event *)malloc(sizeof(*e));
	if (!e) {
		err("unable to allocate event");
		return SA_ERR_HPI_OUT_OF_SPACE;
	}
        memset(e, '\0', sizeof(struct oh_event));
        e->hid = inst->hid;
        oh_concat_ep( &(r->path), &g_epbase);
	e->resource.ResourceId = oh_uid_from_entity_path(&(r->path));
	e->resource.EntryId = e->resource.ResourceId; /* EntryId = ResourceId */
	/* Note:  .res_event.entry.ResourceInfo currently unassigned */
	memcpy(&(e->resource.ResourceEntity),&(r->path),sizeof(SaHpiEntityPathT));
	e->resource.ResourceCapabilities = SAHPI_CAPABILITY_RESOURCE|SAHPI_CAPABILITY_RDR|SAHPI_CAPABILITY_SENSOR;
	e->resource.ResourceSeverity = SAHPI_CRITICAL; /* sysfs data always critical */
	e->resource.ResourceTag.DataType = SAHPI_TL_TYPE_ASCII6;
	e->resource.ResourceTag.Language = SAHPI_LANG_ENGLISH;
	e->resource.ResourceTag.DataLength = strlen(r->name);
	strcpy((char*)e->resource.ResourceTag.Data, r->name);

        e->event.Source = e->resource.ResourceId;
        e->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
        e->event.Severity = e->resource.ResourceSeverity;
        e->event.EventType = SAHPI_ET_RESOURCE;
        e->event.EventDataUnion.ResourceEvent.ResourceEventType = SAHPI_RESE_RESOURCE_ADDED;
	
	/* add resource */
	if (0 != oh_add_resource(inst->rptcache, &(e->resource), NULL, 0)) {
		err("unable to add resource to RPT");
		return SA_ERR_HPI_ERROR;
	}

        /* Assign RDRs to this resource */
        sysfs2hpi_assign_rdrs(d, r, inst, e);
        
	/* add event */
	oh_evt_queue_push(inst->eventq, e);

	return 0;
}

/**
 * sysfs2hpi_discover_resources:
 * @hnd: void pointer to handler
 *
 * Discover the resources in sysfs.
 * Currently, we are only discovering devices on
 * the i2c bus.  Each device is a resource.  All
 * data underneath is part of the RDR.
 *
 * Return value: 0 for success | Error code
 **/
static int sysfs2hpi_discover_resources(void *hnd)
{
	struct sysfs_device *d = NULL;
	struct oh_handler_state *inst = (struct oh_handler_state *)hnd;
	struct sysfsitems *sys;

	if (!hnd) {
		err("null handle");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	/*
	 * This plug-in is written for hardwired i2c sensors,
	 * and therefore doesn't really need to do any discovery
	 * after the initial discovery
	 */
	sys = inst->data;
	if (sys->initialized)
		return 0;

	sys->bus = sysfs_open_bus("i2c");
	if (!sys->bus) {
		err("unable to open i2c bus");
		return SA_ERR_HPI_NOT_PRESENT; /* better error checking would
						  ensure a better return val */
	}

	if (!(sys->bus->devices)) {
		err("no i2c devices found");
		sysfs_close_bus(sys->bus);
		sys->initialized++;
		return 0;
	}

	/*
	 * One resource per device under /sys/bus/i2c/devices
	 */	

	oh_uid_initialize();
	dlist_for_each_data(sys->bus->devices, d, struct sysfs_device) {
		int ret = 0;
		ret = sysfs2hpi_assign_resource(d, inst);
		g_num_resources++;
		if (0 != ret) {
			return ret;
		}
	} /* end dlist_for_each_data */

	sys->initialized++;
	return 0;
}

/**
 * sysfs2hpi_get_sensor_reading:
 * @hnd: void pointer to handler
 * @id: ResourceId for resource with data
 * @num: Sensor number for sensor with data
 * @data: pointer to SaHpiSensorReading data type
 *
 * Get the data for the RDR sensor passed to this function.
 * This function rereads the data from the machine in case it
 * has changed.
 * Note:  libsysfs documentation states that current, voltage,
 * and temperature raw readings need to be divided by 1000 to
 * get interpreted values.  fan readings need to be divided
 * by fan_div.  We are assuming that if there is not a div
 * parameter, this is not a fan sensor (This assumption is
 * not fully robust.).
 *
 * Return value: 0 for success | Error code
 **/

/**
 * Change:
 * all readings get interpreted values
 **/

static int sysfs2hpi_get_sensor_reading(void *hnd, SaHpiResourceIdT id,
					SaHpiSensorNumT num,
				     	SaHpiSensorReadingT *reading,
					SaHpiEventStateT *state)
{
	struct sensor *s;
	struct sysfs_attribute *sysattr = NULL;
	struct oh_handler_state *inst = (struct oh_handler_state *)hnd;
        SaHpiRdrT *tmprdr;

	if (!hnd) {
		err("null handle");
		return SA_ERR_HPI_INVALID_PARAMS;
	}


        /* sequential search of rdr list for current RDR */
        tmprdr = oh_get_rdr_next(inst->rptcache, id, 0);

        while ((tmprdr->RdrTypeUnion.SensorRec.Num != num) && (tmprdr)) {
               	tmprdr = oh_get_rdr_next(inst->rptcache, id, tmprdr->RecordId);
        }

        if (tmprdr->RdrTypeUnion.SensorRec.Num != num) { 
		/* didn't find sensor */
		err("could not find sensor");
		return SA_ERR_HPI_INVALID_DATA;
        }

	/* get sensor data */
	s = (struct sensor *) oh_get_rdr_data(inst->rptcache, id, tmprdr->RecordId);

	if (!s) {
		err("could not get sensor data");
		return SA_ERR_HPI_INVALID_DATA;
	}
	if (!s->value) {
		err("input data for sensor not available");
		return SA_ERR_HPI_INVALID_DATA;
	}

	*state = 0x0000;

	sysattr = sysfs_open_attribute(s->value->path);
	if (!sysattr) {
		err("failed opening attribute at %s", s->value->path);
		return SA_ERR_HPI_INVALID_DATA;
	}
	
	if (sysfs_read_attribute(sysattr)) {
		err("error attempting to read value of %s",s->name);
		sysfs_close_attribute(sysattr);
		return SA_ERR_HPI_INVALID_DATA;
	}

        reading_int64_set(reading, atoi(sysattr->value));
	sysfs_close_attribute(sysattr);
	
	return 0;
}

/**
 * sysfs2hpi_get_sensor_thresholds:
 * @hnd: void pointer to handler
 * @id: ResourceId for resource with data
 * @num: Sensor number for sensor with data
 * @thres: pointer to SaHpiSensorThresholdsT data type
 *
 * Get the thresholds for the RDR sensor passed to this function.
 * This function rereads the data from the machine in case it
 * has changed.
 * Note:  libsysfs documentation states that current, voltage,
 * and temperature raw readings need to be divided by 1000 to
 * get interpreted values.  fan readings need to be divided
 * by fan_div.  We are assuming that if there is not a div
 * parameter, this is not a fan sensor (This assumption is
 * not fully robust.).
 *
 * Return value: 0 for success | Error code
 **/
static int sysfs2hpi_get_sensor_thresholds(void *hnd, 
						SaHpiResourceIdT id,
						SaHpiSensorNumT num,
					   	SaHpiSensorThresholdsT *thres)
{
	struct sensor *s;
	struct sysfs_attribute *sysattr = NULL;
	struct oh_handler_state *inst = (struct oh_handler_state *)hnd;
	SaHpiRdrT *tmprdr;

	if (!hnd) {
		err("null handle");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

        /* sequential search of rdr list for current RDR */
        tmprdr = oh_get_rdr_next(inst->rptcache, id, 0);

        while ((tmprdr->RdrTypeUnion.SensorRec.Num != num) && (tmprdr)) {
               	tmprdr = oh_get_rdr_next(inst->rptcache, id, tmprdr->RecordId);
        }

        if (tmprdr->RdrTypeUnion.SensorRec.Num != num) { 
		/* didn't find sensor */
		err("could not find sensor");
		return SA_ERR_HPI_INVALID_DATA;
        }

	/* get sensor data */
	s = (struct sensor *) oh_get_rdr_data(inst->rptcache, id, tmprdr->RecordId);

	if (!s) {
		err("could not get sensor thresholds");
		return SA_ERR_HPI_INVALID_DATA;
	}

	/*
	 * sysfs only defines a min and a max
	 * setting these to LowCritical and UpCritical, respectively
	 * The min value for temperature can technically be a hysteresis value,
	 * but this currently is not part of the hysteresis fields.
	 * Setting ValuesPresent for all other items to 0.
	 */
	/* get min values */
	sysattr = sysfs_open_attribute(s->min->path);
        if (!sysattr) {
                err("failed opening attribute at %s", s->min->path);
                return SA_ERR_HPI_INVALID_DATA;
        }

        if (sysfs_read_attribute(sysattr)) {
                err("error attempting to read value of %s",s->name);
		sysfs_close_attribute(sysattr);
                return SA_ERR_HPI_INVALID_DATA;
        }
        reading_int64_set(&thres->LowCritical, atoi(sysattr->value));
	sysfs_close_attribute(sysattr);
	sysattr = NULL;

	sysattr = sysfs_open_attribute(s->max->path);
        if (!sysattr) {
                err("failed opening attribute at %s", s->max->path);
                return SA_ERR_HPI_INVALID_DATA;
        }

        if (sysfs_read_attribute(sysattr)) {
                err("error attempting to read value of %s",s->name);
		sysfs_close_attribute(sysattr);
                return SA_ERR_HPI_INVALID_DATA;
        }
        reading_int64_set(&thres->UpCritical, atoi(sysattr->value));
	sysfs_close_attribute(sysattr);

        thres->LowMajor.IsSupported = SAHPI_FALSE;
        thres->LowMinor.IsSupported = SAHPI_FALSE;
        thres->UpMajor.IsSupported = SAHPI_FALSE;
        thres->UpMinor.IsSupported = SAHPI_FALSE;
        thres->PosThdHysteresis.IsSupported = SAHPI_FALSE;
        thres->NegThdHysteresis.IsSupported = SAHPI_FALSE;

#if 0
	thres->LowCritical.Raw = atoi(tmp);
	thres->LowCritical.ValuesPresent = SAHPI_SRF_RAW;
	thres->LowCritical.EventStatus.SensorStatus = s->enables.SensorStatus;
	thres->LowCritical.EventStatus.EventStatus = s->enables.AssertEvents;

	thres->LowCritical.Interpreted.Type = SAHPI_SENSOR_INTERPRETED_TYPE_UINT32;
	if (!s->div) { /* assume not a fan sensor */
		thres->LowCritical.Interpreted.Value.SensorUint32 = 
			(SaHpiUint32T) thres->LowCritical.Raw/1000;
	} else { /* fan sensor */
		if (sysfs_read_attribute_value(s->div->path,tmp,SCRATCHSIZE)) {
			err("error attempting to read value of %s",s->name);
			return SA_ERR_HPI_INVALID_DATA;
		}
		thres->LowCritical.Interpreted.Value.SensorUint32 = 
			(SaHpiUint32T) thres->LowCritical.Raw/atoi(tmp);
	}

	if (sysfs_read_attribute_value(s->max->path,tmp,SCRATCHSIZE)) {
		err("error attempting to read value of %s",s->name);
		return SA_ERR_HPI_INVALID_DATA;
	}

	/* get max values */
	thres->UpCritical.Raw = atoi(tmp);
	thres->UpCritical.ValuesPresent = SAHPI_SRF_RAW;
	thres->UpCritical.EventStatus.SensorStatus = s->enables.SensorStatus;
	thres->UpCritical.EventStatus.EventStatus = s->enables.AssertEvents;

	thres->UpCritical.Interpreted.Type = SAHPI_SENSOR_INTERPRETED_TYPE_UINT32;
	if (!s->div) { /* assume not a fan sensor */
		thres->UpCritical.Interpreted.Value.SensorUint32 = 
			(SaHpiUint32T) thres->UpCritical.Raw/1000;
	} else { /* fan sensor */
		if (sysfs_read_attribute_value(s->div->path,tmp,SCRATCHSIZE)) {
			err("error attempting to read value of %s",s->name);
			return SA_ERR_HPI_INVALID_DATA;
		}
		thres->UpCritical.Interpreted.Value.SensorUint32 = 
			(SaHpiUint32T) thres->UpCritical.Raw/atoi(tmp);
	}

	thres->LowMajor.ValuesPresent = 0;
	thres->LowMinor.ValuesPresent = 0;
	thres->UpMajor.ValuesPresent = 0;
	thres->UpMinor.ValuesPresent = 0;
	thres->PosThdHysteresis.ValuesPresent = 0;
	thres->NegThdHysteresis.ValuesPresent = 0;
#endif
	return 0;
}

/**
 * sysfs2hpi_set_reading:
 * @rdr - current RDR
 * @attr - sysfs attribute to be set
 * @div - sysfs attribute for div, if needed
 * @reading - sensor reading to set
 *
 * Helper function to sysfs2hpi_set_sensor_thresholds.
 * Set an individual reading of a sensor.
 *
 * Return value: 0 for success | Error code
 **/
static int sysfs2hpi_set_sensor_reading(SaHpiRdrT *rdr,
		struct sysfs_attribute *attr,
		SaHpiSensorReadingT reading)
{
	
        char tmp[SCRATCHSIZE];

        if (reading.Type == SAHPI_SENSOR_READING_TYPE_INT64) {
		snprintf(tmp, SCRATCHSIZE, "%lld", reading.Value.SensorInt64);
		if (sysfs_write_attribute(attr,tmp,SCRATCHSIZE)) {
			err("error attempting to write value");
			return SA_ERR_HPI_INVALID_DATA;
		}
		return 0;
	}
	err("No values were set");
	return SA_ERR_HPI_INVALID_REQUEST;
}

/**
 * sysfs2hpi_set_sensor_thresholds:
 * @hnd: void pointer to handler
 * @id: ResourceId for resource with data
 * @num: Sensor number for sensor with data
 * @thres: pointer to SaHpiSensorThresholdsT data type
 *
 * Functions to set a given sensor's thresholds values
 *
 * Return value: 0 for success | Error code
 **/
static int sysfs2hpi_set_sensor_thresholds(void *hnd, 
						SaHpiResourceIdT id,
						SaHpiSensorNumT num,
					   	const SaHpiSensorThresholdsT *thres)
{
	struct sensor *s;
	struct oh_handler_state *inst = (struct oh_handler_state *)hnd;
	SaHpiRdrT *tmprdr;
	int ret = 0;

	if (!hnd) {
		err("null handle");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

        /* sequential search of rdr list for current RDR */
        tmprdr = oh_get_rdr_next(inst->rptcache, id, 0);

        while ((tmprdr->RdrTypeUnion.SensorRec.Num != num) && (tmprdr)) {
               	tmprdr = oh_get_rdr_next(inst->rptcache, id, tmprdr->RecordId);
        }

        if (tmprdr->RdrTypeUnion.SensorRec.Num != num) { 
		/* didn't find sensor */
		err("could not find sensor");
		return SA_ERR_HPI_INVALID_DATA;
        }

	/* get sensor data */
	s = (struct sensor *) oh_get_rdr_data(inst->rptcache, id, tmprdr->RecordId);

	if (!s) {
		err("could not get sensor data for thresholds");
		return SA_ERR_HPI_INVALID_DATA;
	}

	/*
	 * sysfs only defines a min and a max, which have been mapped
	 * to LowCritical and UpCritical, respectively, so all other input
	 * will be ignored
	 */

	if ((SAHPI_TRUE != thres->LowCritical.IsSupported) && 
			(SAHPI_TRUE != thres->UpCritical.IsSupported)) {
		/* if no LowCritical or UpCritical values are sent, return error */
		err("No LowCritical or UpCritical values were sent");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	/* set min value */
	if (SAHPI_TRUE == thres->LowCritical.IsSupported) {
		ret = sysfs2hpi_set_sensor_reading(tmprdr, s->min, thres->LowCritical);
	}

	/* set max values */
	if (SAHPI_TRUE == thres->UpCritical.IsSupported) {
		ret = sysfs2hpi_set_sensor_reading(tmprdr, s->max, thres->UpCritical);
	}

	return ret;
}

/**
 * sysfs2hpi_get_sensor_event_enables:
 * @hnd: void pointer to handler
 * @id: ResourceId for resource with data
 * @num: Sensor number for sensor with data
 * @enables: pointer to SaHpiSensorEvtEnablesT data type
 *
 * Get the enables field from the RDR passed to this function.
 *
 * Return value: 0 for success | Error code
 **/
static int sysfs2hpi_get_sensor_event_enables(void *hnd, 
						SaHpiResourceIdT id,
						SaHpiSensorNumT num,
					      	SaHpiBoolT *enable)
{
	struct sensor *s;
	struct oh_handler_state *inst = (struct oh_handler_state *)hnd;
	SaHpiRdrT *tmprdr;

	if (!hnd) {
		err("null handle");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

        /* sequential search of rdr list for current RDR */
        tmprdr = oh_get_rdr_next(inst->rptcache, id, 0);

        while ((tmprdr->RdrTypeUnion.SensorRec.Num != num) && (tmprdr)) {
               	tmprdr = oh_get_rdr_next(inst->rptcache, id, tmprdr->RecordId);
        }

        if (tmprdr->RdrTypeUnion.SensorRec.Num != num) { 
		/* didn't find sensor */
		err("could not find sensor");
		return SA_ERR_HPI_INVALID_DATA;
        }

	/* get sensor data */
	s = (struct sensor *) oh_get_rdr_data(inst->rptcache, id, tmprdr->RecordId);

	if (!s) {
		err("could not get sensor data for event enables");
		return SA_ERR_HPI_INVALID_DATA;
	}
	
	*enable = s->evt_enable;
	
	return 0;
}

/**
 * sysfs2hpi_set_sensor_event_enables:
 * @hnd: void pointer to handler
 * @id: ResourceId for resource with data
 * @num: Sensor number for sensor with data
 * @enables: pointer to SaHpiSensorEvtEnablesT data type
 *
 * Set the enables field on the RDR passed to this function.
 *
 * Return value: 0 for success | Error code
 **/
static int sysfs2hpi_set_sensor_event_enables(void *hnd, 
						SaHpiResourceIdT id,
						SaHpiSensorNumT num,
					      	SaHpiBoolT enable)
{
	struct sensor *s;
	struct oh_handler_state *inst = (struct oh_handler_state *)hnd;
	SaHpiRdrT *tmprdr;

	if (!hnd) {
		err("null handle");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

        /* sequential search of rdr list for current RDR */
        tmprdr = oh_get_rdr_next(inst->rptcache, id, 0);

        while ((tmprdr->RdrTypeUnion.SensorRec.Num != num) && (tmprdr)) {
               	tmprdr = oh_get_rdr_next(inst->rptcache, id, tmprdr->RecordId);
        }

        if (tmprdr->RdrTypeUnion.SensorRec.Num != num) { 
		/* didn't find sensor */
		err("could not find sensor");
		return SA_ERR_HPI_INVALID_DATA;
        }

	/* get sensor data */
	s = (struct sensor *) oh_get_rdr_data(inst->rptcache, id, tmprdr->RecordId);

	if (!s) {
		err("could not get sensor data for event enables");
		return SA_ERR_HPI_INVALID_DATA;
	}

	s->evt_enable = enable;
	return 0;
}

void * oh_open (GHashTable *, unsigned int, oh_evt_queue *) __attribute__ ((weak, alias("sysfs2hpi_open")));

void * oh_close (void *) __attribute__ ((weak, alias("sysfs2hpi_close")));

void * oh_get_event (void *) 
            __attribute__ ((weak, alias("sysfs2hpi_get_event")));
		
void * oh_discover_resources (void *) 
            __attribute__ ((weak, alias("sysfs2hpi_discover_resources")));

void * oh_get_sensor_reading (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiSensorReadingT *, 
			     SaHpiEventStateT    *) 
            __attribute__ ((weak, alias("sysfs2hpi_get_sensor_reading")));
		  	     
void * oh_get_sensor_thresholds (void *, SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 SaHpiSensorThresholdsT *) 
            __attribute__ ((weak, alias("sysfs2hpi_get_sensor_thresholds")));
		
void * oh_set_sensor_thresholds (void *, SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 const SaHpiSensorThresholdsT *) 
            __attribute__ ((weak, alias("sysfs2hpi_set_sensor_thresholds")));
		
		
void * oh_get_sensor_event_enables (void *, SaHpiResourceIdT,
                                    SaHpiSensorNumT,
                                    SaHpiBoolT *) 
            __attribute__ ((weak, alias("sysfs2hpi_get_sensor_event_enables")));

void * oh_set_sensor_event_enables (void *, SaHpiResourceIdT id, SaHpiSensorNumT,
                                    SaHpiBoolT *)
            __attribute__ ((weak, alias("sysfs2hpi_set_sensor_event_enables")));
