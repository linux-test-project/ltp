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
 */

#include <stdlib.h>
#include <string.h>
#include <libsysfs.h>

#include <SaHpi.h>
#include <openhpi.h>
#include <uid_utils.h>
#include <epath_utils.h>

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
	SaHpiSensorEvtEnablesT enables;
};

struct resource {
	SaHpiEntityPathT path;
	char name[SYSFS_NAME_LEN];
	GSList *sensors;
};

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
static void *sysfs2hpi_open(GHashTable *handler_config)
{
        struct oh_handler_state *hnd;
	struct sysfsitems *sys;
        char *er;

	if (!handler_config) {
		dbg("empty handler_config");
		return NULL;
	}

	/* set up entity root in g_epbase */
        er = (char *)g_hash_table_lookup(handler_config,"entity_root");
        if (!er) {
                dbg("no entity root present");
		return NULL;
        }
        string2entitypath(er, &g_epbase);
        append_root(&g_epbase);

        hnd = malloc(sizeof(*hnd));
        if (!hnd) {
		dbg("unable to allocate main handler");
		return NULL;
	}

	memset(hnd, '\0', sizeof(*hnd));

	/* assign config to handler_config and initialize rptcache */
        hnd->config = handler_config;

        hnd->rptcache = (RPTable *)g_malloc0(sizeof(RPTable));

	sys = malloc(sizeof(*sys));
	if (!sys) {
		dbg("unable to allocate sysfsitems structure");
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
		dbg("no instance to delete");
		return;
	}

	sys = inst->data;
	sysfs_close_bus(sys->bus);

	/* Free unused events */
	g_slist_free(inst->eventq);

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
 * @event: pointer to oh_event
 * @timeout: struct timeval
 *
 * This function gets a sysfs event from the sysfs event table
 * in instance.events.  It copies the event to memory and then
 * deletes it from the instance.events table.
 *
 * Return value: 0 if times out, > 0 is event is returned.
 **/
static int sysfs2hpi_get_event(void *hnd, struct oh_event *event, 
			       struct timeval *timeout)
{
	struct oh_handler_state *inst = (struct oh_handler_state *)hnd;
	GSList	*tmp;
	
	if (g_slist_length(inst->eventq) != 0) {
		g_slist_for_each(tmp, inst->eventq) {
			struct oh_event *e;
			e = tmp->data;
			memcpy(event, e, sizeof(*event));
			inst->eventq = g_slist_remove_link(inst->eventq, tmp);
			g_slist_free(tmp);
			free(e);
			return 1;
		}
	}

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
		struct oh_handler_state* inst)
{
	struct sensor *s;
	struct oh_event *e;
	unsigned char strinput[SYSFS_NAME_LEN];
	int puid;

	if ((type != SAHPI_TEMPERATURE) && (type != SAHPI_VOLTAGE) &&
       		(type != SAHPI_CURRENT) && (type != SAHPI_FAN))	{
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	s = (struct sensor*)malloc(sizeof(*s));
	if (!s) {
		dbg("unable to allocate sensor");
		return SA_ERR_HPI_OUT_OF_SPACE;
	}
	memset(s,'\0',sizeof(*s));			
	s->num = num_sensors;

	switch(type) {
		case SAHPI_TEMPERATURE:
			sprintf(s->name,"%i:Temp Sensor",s->num);
		
			sprintf(strinput, "temp_input%s", str);
			s->value = sysfs_get_device_attr(d, strinput);
			sprintf(strinput, "temp_max%s", str);
			s->max = sysfs_get_device_attr(d, strinput);
			sprintf(strinput, "temp_min%s", str);
			s->min = sysfs_get_device_attr(d, strinput);
			s->div = NULL;
			break;
		case SAHPI_VOLTAGE:
			sprintf(s->name,"%i:Voltage Sensor",s->num);
		
			sprintf(strinput, "in_input%s", str);
			s->value = sysfs_get_device_attr(d, strinput);
			sprintf(strinput, "in_max%s", str);
			s->max = sysfs_get_device_attr(d, strinput);
			sprintf(strinput, "in_min%s", str);
			s->min = sysfs_get_device_attr(d, strinput);
			s->div = NULL;
			break;
		case SAHPI_CURRENT:
			sprintf(s->name,"%i:Current Sensor",s->num);
		
			sprintf(strinput, "curr_input%s", str);
			s->value = sysfs_get_device_attr(d, strinput);
			sprintf(strinput, "curr_max%s", str);
			s->max = sysfs_get_device_attr(d, strinput);
			sprintf(strinput, "curr_min%s", str);
			s->min = sysfs_get_device_attr(d, strinput);
			s->div = NULL;
			break;
		case SAHPI_FAN:
			sprintf(s->name,"%i:Fan Sensor",s->num);
		
			sprintf(strinput, "fan_input%s", str);
			s->value = sysfs_get_device_attr(d, strinput);
			sprintf(strinput, "fan_max%s", str);
			s->max = sysfs_get_device_attr(d, strinput);
			sprintf(strinput, "fan_min%s", str);
			s->min = sysfs_get_device_attr(d, strinput);
			sprintf(strinput, "fan_div%s", str);
			s->div = sysfs_get_device_attr(d, strinput);
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

	e = (struct oh_event *)malloc(sizeof(*e));
	if (!e) {
		dbg("unable to allocate event");
		return SA_ERR_HPI_OUT_OF_SPACE;
	}
	memset(e, '\0', sizeof(*e));
	e->type = OH_ET_RDR;

	e->u.rdr_event.rdr.RecordId = num_sensors;
	e->u.rdr_event.rdr.RdrType = SAHPI_SENSOR_RDR;
	e->u.rdr_event.rdr.Entity.Entry[0].EntityType = SAHPI_ENT_SYS_MGMNT_SOFTWARE;
	e->u.rdr_event.rdr.Entity.Entry[0].EntityInstance = g_num_resources;
	e->u.rdr_event.rdr.Entity.Entry[1].EntityType = SAHPI_ENT_OTHER_SYSTEM_BOARD;
	e->u.rdr_event.rdr.Entity.Entry[1].EntityInstance = 0; /* 0 b/c only 1 board */
        ep_concat( &e->u.rdr_event.rdr.Entity, &g_epbase);
	e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.Num = num_sensors;
	e->u.rdr_event.rdr.RdrTypeUnion.SensorRec.Type = type;
	/* Ignoring .Category, .EventCtrl, and .Events b/c sysfs has no events */
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

	inst->eventq = g_slist_append(inst->eventq, e);

	puid = oh_uid_lookup(&e->u.rdr_event.rdr.Entity);
	if (puid < 0) {
		dbg("could not find correct parent");
		return SA_ERR_HPI_ERROR;
	}

	if (oh_add_rdr(inst->rptcache, puid, &e->u.rdr_event.rdr, (void *)s, 0)) {
		dbg("unable to add RDR to RPT");
		return SA_ERR_HPI_ERROR;
	}

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
		struct resource* r, struct oh_handler_state* inst)
{
	int num_sensors = 0;
	char str[1];
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
		sprintf(str, "%d", i);
		if (sysfs2hpi_setup_rdr(SAHPI_CURRENT, str, 
				++num_sensors, d, r, inst) != 0) {
			i=-1; /* keep going until we get an error returned */
			num_sensors--;
		}
	}
	
	/* Set up fan RDR */
	for (i=1;i<=fanmax;i++) {
		sprintf(str, "%d", i);
		if (sysfs2hpi_setup_rdr(SAHPI_FAN, str, 
				++num_sensors, d, r, inst) != 0) {
			num_sensors--;
		}
	}
	
	/* Set up voltage RDR */
	for (i=0;i<=voltagemax;i++) {
		sprintf(str, "%d", i);
		if (sysfs2hpi_setup_rdr(SAHPI_VOLTAGE, str, 
				++num_sensors, d, r, inst) != 0) {
			num_sensors--;
		}
	}
	
	/* Set up temp RDR */
	for (i=1;i<=tempmax;i++) {
		sprintf(str, "%d", i);
		if (sysfs2hpi_setup_rdr(SAHPI_TEMPERATURE, str, 
				++num_sensors, d, r, inst) != 0) {
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
		dbg("unable to allocate resource");
		return SA_ERR_HPI_OUT_OF_SPACE;
	}
	memset(r,'\0',sizeof(*r));
	r->path.Entry[0].EntityType = SAHPI_ENT_SYS_MGMNT_SOFTWARE;
	r->path.Entry[0].EntityInstance = g_num_resources;
	r->path.Entry[1].EntityType = SAHPI_ENT_OTHER_SYSTEM_BOARD;
	r->path.Entry[1].EntityInstance = 0; /* 0 b/c only 1 board */
	//sprintf(r->name, "%s_%s", d->name, d->bus_id);
	strncpy(r->name,d->name,SYSFS_NAME_LEN);	
	sys = inst->data;
	sys->resources = g_slist_append(sys->resources, r);

	e = (struct oh_event *)malloc(sizeof(*e));
	if (!e) {
		dbg("unable to allocate event");
		return SA_ERR_HPI_OUT_OF_SPACE;
	}
	memset(e, '\0', sizeof(struct oh_event));
	e->type = OH_ET_RESOURCE;
        ep_concat( &(r->path), &g_epbase);
	e->u.res_event.entry.ResourceId = oh_uid_from_entity_path(&(r->path));
	e->u.res_event.entry.EntryId = e->u.res_event.entry.ResourceId; /* EntryId = ResourceId */
	/* Note:  .res_event.entry.ResourceInfo currently unassigned */
	memcpy(&(e->u.res_event.entry.ResourceEntity),&(r->path),sizeof(SaHpiEntityPathT));
	e->u.res_event.entry.ResourceCapabilities = SAHPI_CAPABILITY_RESOURCE|SAHPI_CAPABILITY_RDR|SAHPI_CAPABILITY_SENSOR;
	e->u.res_event.entry.ResourceSeverity = SAHPI_CRITICAL; /* sysfs data always critical */
	e->u.res_event.entry.ResourceTag.DataType = SAHPI_TL_TYPE_ASCII6;
	e->u.res_event.entry.ResourceTag.Language = SAHPI_LANG_ENGLISH;
	e->u.res_event.entry.ResourceTag.DataLength = strlen(r->name);
	strcpy(e->u.res_event.entry.ResourceTag.Data, r->name);
	
	/* add resource */
	if (0 != oh_add_resource(inst->rptcache, &(e->u.res_event.entry), NULL, 0)) {
		dbg("unable to add resource to RPT");
		return SA_ERR_HPI_ERROR;
	}
	/* add event */
	inst->eventq = g_slist_append(inst->eventq, e);

	/* Assign RDRs to this resource */
	sysfs2hpi_assign_rdrs(d, r, inst);

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
		dbg("null handle");
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
		dbg("unable to open i2c bus");
		return SA_ERR_HPI_NOT_PRESENT; /* better error checking would
						  ensure a better return val */
	}

	if (!(sys->bus->devices)) {
		dbg("no i2c devices found");
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
 * sysfs2hpi_get_sensor_data:
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
static int sysfs2hpi_get_sensor_data(void *hnd, SaHpiResourceIdT id,
					SaHpiSensorNumT num,
				     	SaHpiSensorReadingT *data)
{
	struct sensor *s;
	char tmp[SCRATCHSIZE];
	struct oh_handler_state *inst = (struct oh_handler_state *)hnd;
        SaHpiRdrT *tmprdr;

	if (!hnd) {
		dbg("null handle");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

        /* sequential search of rdr list for current RDR */
        tmprdr = oh_get_rdr_next(inst->rptcache, id, 0);

        while ((tmprdr->RdrTypeUnion.SensorRec.Num != num) && (tmprdr)) {
               	tmprdr = oh_get_rdr_next(inst->rptcache, id, tmprdr->RecordId);
        }

        if (tmprdr->RdrTypeUnion.SensorRec.Num != num) { 
		/* didn't find sensor */
		dbg("could not find sensor");
		return SA_ERR_HPI_INVALID_DATA;
        }

	/* get sensor data */
	s = (struct sensor *) oh_get_rdr_data(inst->rptcache, id, tmprdr->RecordId);

	if (!s) {
		dbg("could not get sensor data");
		return SA_ERR_HPI_INVALID_DATA;
	}

	data->ValuesPresent = SAHPI_SRF_RAW | SAHPI_SRF_INTERPRETED;
	data->EventStatus.SensorStatus = s->enables.SensorStatus;
	data->EventStatus.EventStatus = s->enables.AssertEvents;

	if (!s->value) {
		dbg("input data for sensor not available");
		return SA_ERR_HPI_INVALID_DATA;
	}
	if (sysfs_read_attribute_value(s->value->path,tmp,SCRATCHSIZE)) {
		dbg("error attempting to read value of %s",s->name);
		return SA_ERR_HPI_INVALID_DATA;
	}
	data->Raw = atoi(tmp);
	
	data->Interpreted.Type = SAHPI_SENSOR_INTERPRETED_TYPE_UINT32;
	if (!s->div) { /* assume not a fan sensor */
		data->Interpreted.Value.SensorUint32 = (SaHpiUint32T) data->Raw/1000;
	} else { /* fan sensor */
		if (sysfs_read_attribute_value(s->div->path,tmp,SCRATCHSIZE)) {
			dbg("error attempting to read value of %s",s->name);
			return SA_ERR_HPI_INVALID_DATA;
		}
		data->Interpreted.Value.SensorUint32 = (SaHpiUint32T) data->Raw/atoi(tmp);
	}
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
	char tmp[SCRATCHSIZE];
	struct oh_handler_state *inst = (struct oh_handler_state *)hnd;
	SaHpiRdrT *tmprdr;

	if (!hnd) {
		dbg("null handle");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

        /* sequential search of rdr list for current RDR */
        tmprdr = oh_get_rdr_next(inst->rptcache, id, 0);

        while ((tmprdr->RdrTypeUnion.SensorRec.Num != num) && (tmprdr)) {
               	tmprdr = oh_get_rdr_next(inst->rptcache, id, tmprdr->RecordId);
        }

        if (tmprdr->RdrTypeUnion.SensorRec.Num != num) { 
		/* didn't find sensor */
		dbg("could not find sensor");
		return SA_ERR_HPI_INVALID_DATA;
        }

	/* get sensor data */
	s = (struct sensor *) oh_get_rdr_data(inst->rptcache, id, tmprdr->RecordId);

	if (!s) {
		dbg("could not get sensor thresholds");
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
	if (sysfs_read_attribute_value(s->min->path,tmp,SCRATCHSIZE)) {
		dbg("error attempting to read value of %s",s->name);
		return SA_ERR_HPI_INVALID_DATA;
	}
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
			dbg("error attempting to read value of %s",s->name);
			return SA_ERR_HPI_INVALID_DATA;
		}
		thres->LowCritical.Interpreted.Value.SensorUint32 = 
			(SaHpiUint32T) thres->LowCritical.Raw/atoi(tmp);
	}

	if (sysfs_read_attribute_value(s->max->path,tmp,SCRATCHSIZE)) {
		dbg("error attempting to read value of %s",s->name);
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
			dbg("error attempting to read value of %s",s->name);
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
		struct sysfs_attribute *div,
		SaHpiSensorReadingT reading)
{
	char tmp[SCRATCHSIZE], tmprd[SCRATCHSIZE];

	/* try to set raw values first, per SAF HPI spec */
	if ((reading.ValuesPresent & SAHPI_SRF_RAW) &&
		(rdr->RdrTypeUnion.SensorRec.ThresholdDefn.TholdCapabilities
		& SAHPI_STC_RAW))	{
		sprintf(tmp, "%d", reading.Raw);
		if (sysfs_write_attribute(attr,tmp,SCRATCHSIZE)) {
			dbg("error attempting to write value");
			return SA_ERR_HPI_INVALID_DATA;
		}
		return 0;
	}

	if ((reading.ValuesPresent & SAHPI_SRF_INTERPRETED) &&
		(rdr->RdrTypeUnion.SensorRec.ThresholdDefn.TholdCapabilities 
		& SAHPI_STC_INTERPRETED) &&
		(reading.Interpreted.Type & SAHPI_SENSOR_INTERPRETED_TYPE_UINT32)) {
		if (!div) { /* assume not a fan sensor */
			sprintf(tmp, "%d", 
				reading.Interpreted.Value.SensorUint32*1000);
		} else { /* fan sensor */
			if (sysfs_read_attribute_value(div->path,tmprd,SCRATCHSIZE)) {
				dbg("error attempting to read value");
				return SA_ERR_HPI_INVALID_DATA;
			}
			sprintf(tmp, "%d", 
				reading.Interpreted.Value.SensorUint32*
				atoi(tmprd));
		}
		if (sysfs_write_attribute(attr,tmp,SCRATCHSIZE)) {
			dbg("error attempting to write value");
			return SA_ERR_HPI_INVALID_DATA;
		}
		return 0;
	}

	dbg("No values were set");
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
		dbg("null handle");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

        /* sequential search of rdr list for current RDR */
        tmprdr = oh_get_rdr_next(inst->rptcache, id, 0);

        while ((tmprdr->RdrTypeUnion.SensorRec.Num != num) && (tmprdr)) {
               	tmprdr = oh_get_rdr_next(inst->rptcache, id, tmprdr->RecordId);
        }

        if (tmprdr->RdrTypeUnion.SensorRec.Num != num) { 
		/* didn't find sensor */
		dbg("could not find sensor");
		return SA_ERR_HPI_INVALID_DATA;
        }

	/* get sensor data */
	s = (struct sensor *) oh_get_rdr_data(inst->rptcache, id, tmprdr->RecordId);

	if (!s) {
		dbg("could not get sensor data for thresholds");
		return SA_ERR_HPI_INVALID_DATA;
	}

	/*
	 * sysfs only defines a min and a max, which have been mapped
	 * to LowCritical and UpCritical, respectively, so all other input
	 * will be ignored
	 */
	if ((0 == thres->LowCritical.ValuesPresent) && 
			(0 == thres->UpCritical.ValuesPresent)) {
		/* if no LowCritical or UpCritical values are sent, return error */
		dbg("No LowCritical or UpCritical values were sent");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	/* set min value */
	if (thres->LowCritical.ValuesPresent) {
		ret = sysfs2hpi_set_sensor_reading(tmprdr, s->min, s->div, thres->LowCritical);
	}

	/* set max values */
	if (thres->UpCritical.ValuesPresent) {
		ret = sysfs2hpi_set_sensor_reading(tmprdr, s->max, s->div, thres->UpCritical);
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
					      	SaHpiSensorEvtEnablesT *enables)
{
	struct sensor *s;
	struct oh_handler_state *inst = (struct oh_handler_state *)hnd;
	SaHpiRdrT *tmprdr;

	if (!hnd) {
		dbg("null handle");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

        /* sequential search of rdr list for current RDR */
        tmprdr = oh_get_rdr_next(inst->rptcache, id, 0);

        while ((tmprdr->RdrTypeUnion.SensorRec.Num != num) && (tmprdr)) {
               	tmprdr = oh_get_rdr_next(inst->rptcache, id, tmprdr->RecordId);
        }

        if (tmprdr->RdrTypeUnion.SensorRec.Num != num) { 
		/* didn't find sensor */
		dbg("could not find sensor");
		return SA_ERR_HPI_INVALID_DATA;
        }

	/* get sensor data */
	s = (struct sensor *) oh_get_rdr_data(inst->rptcache, id, tmprdr->RecordId);

	if (!s) {
		dbg("could not get sensor data for event enables");
		return SA_ERR_HPI_INVALID_DATA;
	}

	memcpy(enables,&s->enables,sizeof(*enables));
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
					      	const SaHpiSensorEvtEnablesT *enables)
{
	struct sensor *s;
	struct oh_handler_state *inst = (struct oh_handler_state *)hnd;
	SaHpiRdrT *tmprdr;

	if (!hnd) {
		dbg("null handle");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

        /* sequential search of rdr list for current RDR */
        tmprdr = oh_get_rdr_next(inst->rptcache, id, 0);

        while ((tmprdr->RdrTypeUnion.SensorRec.Num != num) && (tmprdr)) {
               	tmprdr = oh_get_rdr_next(inst->rptcache, id, tmprdr->RecordId);
        }

        if (tmprdr->RdrTypeUnion.SensorRec.Num != num) { 
		/* didn't find sensor */
		dbg("could not find sensor");
		return SA_ERR_HPI_INVALID_DATA;
        }

	/* get sensor data */
	s = (struct sensor *) oh_get_rdr_data(inst->rptcache, id, tmprdr->RecordId);

	if (!s) {
		dbg("could not get sensor data for event enables");
		return SA_ERR_HPI_INVALID_DATA;
	}

	memcpy(&s->enables,enables,sizeof(*enables));
	return 0;
}

/* Interface definition */
static struct oh_abi_v2 oh_sysfs2hpi_plugin = {
	.open	 			= sysfs2hpi_open,
	.close				= sysfs2hpi_close,
	.get_event			= sysfs2hpi_get_event,
	.discover_resources		= sysfs2hpi_discover_resources,
	.get_sensor_data		= sysfs2hpi_get_sensor_data,
	.get_sensor_thresholds		= sysfs2hpi_get_sensor_thresholds,
	.set_sensor_thresholds		= sysfs2hpi_set_sensor_thresholds,
	.get_sensor_event_enables       = sysfs2hpi_get_sensor_event_enables,
	.set_sensor_event_enables       = sysfs2hpi_set_sensor_event_enables
};

/**
 * get_interface:
 * @pp: 
 * @uuid: 
 *
 * Called by infrastructure to get this
 * plugin's interface/functions
 *
 * Return value: 0 for success | -1 for error
 **/
int get_interface(void **pp, const uuid_t uuid)
{
	if (uuid_compare(uuid, UUID_OH_ABI_V2)==0) {
		*pp = &oh_sysfs2hpi_plugin;
		return 0;
	}

	*pp = NULL;
	return -1;
}
