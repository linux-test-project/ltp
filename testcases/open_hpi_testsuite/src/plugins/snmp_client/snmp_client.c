/*      -*- linux-c -*-
 *
 *
 * (C) Copyright IBM Corp. 2003
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *      David Judkovics <djudkovi@us.ibm.com>
 *
 */
 
/* used for defining and externing the arrays that hold oids */
#define _SNMP_CLIENT_C_ 
 
#include <SaHpi.h>
#include <openhpi.h>

#include <epath_utils.h>
#include <rpt_utils.h>
#include <snmp_util.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h> 

#include <snmp_client.h>
#include <snmp_client_res.h>

/**
 * snmp_client_open: open snmp blade center plugin
 * @handler_config: hash table passed by infrastructure
 **/

static void *snmp_client_open(GHashTable *handler_config)
{
        struct oh_handler_state *handle;

        struct snmp_client_hnd *custom_handle;
        
	char *root_tuple;

        root_tuple = (char *)g_hash_table_lookup(handler_config, "entity_root");
        if(!root_tuple) {
                dbg("ERROR: Cannot open snmp_client plugin. No entity root found in configuration.");
                return NULL;
        }
        
        handle = 
		(struct oh_handler_state *)g_malloc0(sizeof(*handle));
        custom_handle =
                (struct snmp_client_hnd *)g_malloc0(sizeof(*custom_handle));

        if(!handle || !custom_handle) {
                dbg("Could not allocate memory for handle or custom_handle.");
                return NULL;
        }
        handle->data = custom_handle;
        
        handle->config = handler_config;

        /* Initialize RPT cache */
        handle->rptcache = (RPTable *)g_malloc0(sizeof(RPTable));
        
        /* Initialize snmp library */
        init_snmp("oh_snmp_client");
        
        snmp_sess_init(&(custom_handle->session)); /* Setting up all defaults for now. */
        custom_handle->session.peername = (char *)g_hash_table_lookup(handle->config, "host");

        /* set the SNMP version number */
        custom_handle->session.version = SNMP_VERSION_2c;

        /* set the SNMPv1 community name used for authentication */
        custom_handle->session.community = (char *)g_hash_table_lookup(handle->config, "community");
        custom_handle->session.community_len = strlen(custom_handle->session.community);

        /* windows32 specific net-snmp initialization (is a noop on unix) */
        SOCK_STARTUP;

        custom_handle->ss = snmp_open(&(custom_handle->session));

        if(!custom_handle->ss) {
                snmp_perror("ack");
                snmp_log(LOG_ERR, "something horrible happened!!!\n");
                dbg("Unable to open snmp session.");
                return NULL;
        }

        return handle;

}

/**
 * snmp_client_close: shut down plugin connection
 * @hnd: a pointer to the snmp_client_hnd struct that contains
 * a pointer to the snmp session and another to the configuration
 * data.
 **/

static void snmp_client_close(void *hnd)
{
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct snmp_client_hnd *custom_handle =
                (struct snmp_client_hnd *)handle->data;

        snmp_close(custom_handle->ss);
        /* Should we free handle->config? */

        /* windows32 specific net-snmp cleanup (is a noop on unix) */
        SOCK_CLEANUP;
}

/**
 * snmp_client_get_event:
 * @hnd: 
 * @event: 
 * @timeout: 
 *
 * Return value: 
 **/
static int snmp_client_get_event(void *hnd, struct oh_event *event, struct timeval *timeout)
{
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        
        if( g_slist_length(handle->eventq) > 0 ) {
                memcpy(event, handle->eventq->data, sizeof(*event));
                free(handle->eventq->data);
                handle->eventq = g_slist_remove_link(handle->eventq, handle->eventq);
                return 1;
        } else {
                return 0;
	}

	return(-1);
}


static int snmp_client_discover_resources(void *hnd)
{
	int status = SA_OK;

	struct snmp_value get_value;

        struct oh_handler_state 
		*handle = (struct oh_handler_state *)hnd;
        struct snmp_client_hnd 
		*custom_handle = (struct snmp_client_hnd *)handle->data;
					
	RPTable *diff_rptable = g_malloc0(sizeof(*diff_rptable));

	/* Get the saHpiEntryCount */
	status = snmp_get(custom_handle->ss, SA_HPI_ENTRY_COUNT, &get_value);
	/* Get the saHpiEntries, these are the resources */
	status = get_sahpi_table_entries(diff_rptable, handle, SA_HPI_ENTRY, get_value.integer);
#if 0
	/* Get the saHpiRdrCount */
	status = snmp_get(custom_handle->ss, SA_HPI_RDR_COUNT, &get_value);
	/* Get the saHpiEntries, these are the resources */
	status = get_sahpi_rdr_table(diff_rptable, handle, SA_HPI_RDR_ENTRY, get_value.integer);
#endif
/* simple check */
//	handle->rptcache = ;

	process_diff_table(handle, diff_rptable);

	/* Build Events for Resources and their RDRs found in the rptcahce */
	if (status == SA_OK)
		status = eventq_event_add( handle );

        return(status);
}

/**
 * snmp_client_get_self_id:
 * @hnd: 
 * @id: 
 * 
 * Return value: 
 **/
static int snmp_client_get_self_id(void *hnd, SaHpiResourceIdT id)
{
        return -1;
}

static int snmp_client_get_sensor_data(void *hnd, SaHpiResourceIdT id,
                                   SaHpiSensorNumT num,
                                   SaHpiSensorReadingT *data)
{

dbg("TODO: snmp_client_get_sensor_data()");
#if 0
        gchar *oid;
	SaHpiSensorReadingT working;
        struct snmp_value get_value;
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct snmp_client_hnd *custom_handle = (struct snmp_client_hnd *)handle->data;
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, id, SAHPI_SENSOR_RDR, num);
        struct SensorMibInfo *s =
                (struct SensorMibInfo *)oh_get_rdr_data(handle->rptcache, id, rdr->RecordId);
        
	memset(&working, 0, sizeof(SaHpiSensorReadingT));

        /* Extract index from rdr id and get the snmp of the sensor */
        oid = snmp_derive_objid(rdr->Entity, s->oid);
	if(oid == NULL) {
		dbg("NULL SNMP OID returned for %s\n",s->oid);
		return -1;
	}

        /* Read the sensor value */
        if(net_snmp_get(custom_handle->ss, oid, &get_value) != 0){
                dbg("SNMP could not read sensor %s. Type = %d",oid,get_value.type);
		g_free(oid);
                return SA_ERR_HPI_NO_RESPONSE;
        }
        g_free(oid);

        /* Based on the sensor description, construct a reading to send up */
        /* format the value into the reading for each type of reading format */
        working.ValuesPresent = rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingFormats;
        if(working.ValuesPresent & SAHPI_SRF_RAW) {
                if(get_value.type != ASN_INTEGER) {
                        dbg("Sensor value type mismatches reading format.");
                        return -1;
                } else {
                        working.Raw = (SaHpiUint32T)get_value.integer;
                }
        }

        if(working.ValuesPresent & SAHPI_SRF_INTERPRETED) {
                if(get_value.type == ASN_INTEGER) {
                        working.Interpreted.Type = SAHPI_SENSOR_INTERPRETED_TYPE_INT32;
                        working.Interpreted.Value.SensorInt32 = get_value.integer;
                } else {
			SaHpiSensorInterpretedUnionT value;
			
		       	working.Interpreted.Type = rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Interpreted.Type;
			if(rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Interpreted.Type == SAHPI_SENSOR_INTERPRETED_TYPE_BUFFER) {
				strncpy(working.Interpreted.Value.SensorBuffer,
					get_value.string,
                                                SAHPI_SENSOR_BUFFER_LENGTH);
			} else {
				if(s->convert_snmpstr >= 0) {
					if(get_interpreted_value(get_value.string,s->convert_snmpstr,&value)) {
						dbg("Error: get_interpreted_value for %s, (%s)\n",s->oid,get_value.string);
						return -1;
					}
					working.Interpreted.Value = value;
				} else {
					dbg("Sensor %s SNMP string value needs to be converted\n", s->oid);
					return -1;
				}
			}
                }
        }

        /** FIXME: Need to map events */
        if(working.ValuesPresent & SAHPI_SRF_EVENT_STATE) { 
                if(get_value.type == ASN_OCTET_STR) {
                        dbg("Do not know how to format strings as events yet.");
                } else {
                        SaHpiUint32T shifting = 1;
                        shifting = shifting << get_value.integer;
                        if(rdr->RdrTypeUnion.SensorRec.Events & shifting) {
                                working.EventStatus.SensorStatus = SAHPI_SENSTAT_SCAN_ENABLED |
                                                                 SAHPI_SENSTAT_EVENTS_ENABLED;
                                working.EventStatus.EventStatus = shifting;
                        }
                }
        }

	memcpy(data,&working,sizeof(SaHpiSensorReadingT));
        
        return SA_OK;
#endif
return(-1);
}

#define get_raw_thresholds(thdmask, thdoid, thdname) \
do { \
        if(rdr->RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold & thdmask) { \
	        if(s->threshold_oids.RawThresholds.thdoid != NULL && s->threshold_oids.RawThresholds.thdoid[0] != '\0') { \
	                oid = snmp_derive_objid(rdr->Entity,s->threshold_oids.RawThresholds.thdoid); \
                        if(oid == NULL) { \
                                 dbg("NULL SNMP OID returned for %s\n",s->threshold_oids.RawThresholds.thdoid); \
                                 return -1; \
                        } \
	                if((net_snmp_get(custom_handle->ss, oid, &get_value) != 0) | \
	                   (get_value.type != ASN_INTEGER)) { \
		                dbg("SNMP could not read %s; Type=%d.\n",oid,get_value.type); \
		                g_free(oid); \
		                return SA_ERR_HPI_NO_RESPONSE; \
	                } \
	                g_free(oid); \
	                found_raw++; \
	                working.thdname.Raw = get_value.integer; \
	                working.thdname.ValuesPresent = working.thdname.ValuesPresent | SAHPI_SRF_RAW; \
	        } else { \
		        dbg("Raw threshold defined as readable but no OID defined\n"); \
	        } \
        } \
} while(0)

#define get_interpreted_thresholds(thdmask, thdoid, thdname) \
do { \
        if(rdr->RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold & thdmask) { \
	        if(s->threshold_oids.InterpretedThresholds.thdoid != NULL && s->threshold_oids.InterpretedThresholds.thdoid[0] != '\0') { \
		        oid = snmp_derive_objid(rdr->Entity,s->threshold_oids.InterpretedThresholds.thdoid); \
                        if(oid == NULL) { \
                                dbg("NULL SNMP OID returned for %s\n",s->threshold_oids.InterpretedThresholds.thdoid); \
                                return -1; \
                        } \
	         	if((net_snmp_get(custom_handle->ss, oid, &get_value) != 0) | \
	                   !((get_value.type == ASN_INTEGER) | (get_value.type == ASN_OCTET_STR))) { \
			        dbg("SNMP could not read %s; Type=%d.\n",oid,get_value.type); \
			        g_free(oid); \
			        return SA_ERR_HPI_NO_RESPONSE; \
		        } \
		        found_interpreted++; \
		        /* Means we always need to define this field in bc_resources.h */ \
		        working.thdname.Interpreted.Type = rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Interpreted.Type; \
		        working.thdname.ValuesPresent = working.thdname.ValuesPresent | SAHPI_SRF_INTERPRETED; \
		        if(get_value.type == ASN_INTEGER) { \
			         working.thdname.Interpreted.Value.SensorInt32 = get_value.integer; \
		        } else if(get_value.type == ASN_OCTET_STR && s->convert_snmpstr >= 0) { \
			        if(get_interpreted_value(get_value.string,s->convert_snmpstr,&value)) { \
				        dbg("Error: bad return from get_interpreted_value for %s\n",oid); \
                                        g_free(oid); \
				        return -1; \
			        } \
			        working.thdname.Interpreted.Value = value; \
		        } else { \
			        dbg("%s threshold is string but no conversion defined\n",oid); \
                                g_free(oid); \
			        return -1; \
		        } \
                        g_free(oid); \
	        } else { \
		        dbg("Interpreted threshold defined as readable but no OID defined\n"); \
	        } \
        } \
} while(0)

static int snmp_client_get_sensor_thresholds(void *hnd, SaHpiResourceIdT id,
                                         SaHpiSensorNumT num,
                                         SaHpiSensorThresholdsT *thres)
{

dbg("TODO: snmp_client_get_sensor_thresholds()");
#if 0
        gchar *oid = NULL;
	int  found_raw, found_interpreted;
	SaHpiSensorThresholdsT working;
	SaHpiSensorInterpretedUnionT value;
        struct snmp_value get_value;
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct snmp_client_hnd *custom_handle = (struct snmp_client_hnd *)handle->data;
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, id, SAHPI_SENSOR_RDR, num);
        struct SensorMibInfo *s =
                (struct SensorMibInfo *)oh_get_rdr_data(handle->rptcache, id, rdr->RecordId);

        memset(&working, 0, sizeof(SaHpiSensorThresholdsT));

	if(rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsThreshold == SAHPI_TRUE) {
		found_raw = found_interpreted = 0;
		if(rdr->RdrTypeUnion.SensorRec.ThresholdDefn.TholdCapabilities & SAHPI_STC_RAW) {

			get_raw_thresholds(SAHPI_STM_LOW_MINOR, OidLowMinor, LowMinor);
			get_raw_thresholds(SAHPI_STM_LOW_MAJOR, OidLowMajor, LowMajor);
			get_raw_thresholds(SAHPI_STM_LOW_CRIT, OidLowCrit, LowCritical);
			get_raw_thresholds(SAHPI_STM_UP_MINOR, OidUpMinor, UpMinor);
			get_raw_thresholds(SAHPI_STM_UP_MAJOR, OidUpMajor, UpMajor);
			get_raw_thresholds(SAHPI_STM_UP_CRIT, OidUpCrit, UpCritical);

			/* FIXME:: Add PosThdHysteresis and NegThdHysteresis */			 	
		}

		if(rdr->RdrTypeUnion.SensorRec.ThresholdDefn.TholdCapabilities & SAHPI_STC_INTERPRETED) {

			get_interpreted_thresholds(SAHPI_STM_LOW_MINOR, OidLowMinor, LowMinor);
			get_interpreted_thresholds(SAHPI_STM_LOW_MAJOR, OidLowMajor, LowMajor);
			get_interpreted_thresholds(SAHPI_STM_LOW_CRIT, OidLowCrit, LowCritical);
			get_interpreted_thresholds(SAHPI_STM_UP_MINOR, OidUpMinor, UpMinor);
			get_interpreted_thresholds(SAHPI_STM_UP_MAJOR, OidUpMajor, UpMajor);
			get_interpreted_thresholds(SAHPI_STM_UP_CRIT, OidUpCrit, UpCritical);

			/* FIXME:: Add PosThdHysteresis and NegThdHysteresis */			 	
		}

		/* FIXME:: Do we need to add events as well? */

		if (found_raw || found_interpreted) {
			memcpy(thres,&working,sizeof(SaHpiSensorThresholdsT));
			return SA_OK;
		} else {
			dbg("No threshold values found\n");
			return -1;
		}
        } else {
                dbg("Thresholds requested, but sensor does not support them.\n");
                return SA_ERR_HPI_INVALID_CMD;
        } 
#endif 
return(-1);
}

static int snmp_client_set_sensor_thresholds(void *hnd, SaHpiResourceIdT id,
                                         SaHpiSensorNumT num,
                                         const SaHpiSensorThresholdsT *thres)
{
	/* Writable thresholds not supported */
        return SA_ERR_HPI_INVALID_CMD;
}

static int snmp_client_get_sensor_event_enables(void *hnd, SaHpiResourceIdT id,
                                            SaHpiSensorNumT num,
                                            SaHpiSensorEvtEnablesT *enables)
{
        return -1;
}

static int snmp_client_set_sensor_event_enables(void *hnd, SaHpiResourceIdT id,
                                            SaHpiSensorNumT num,
                                            const SaHpiSensorEvtEnablesT *enables)
{
        return -1;
}

static int snmp_client_get_control_state(void *hnd, SaHpiResourceIdT id,
                                     SaHpiCtrlNumT num,
                                     SaHpiCtrlStateT *state)
{

dbg("TODO: snmp_client_get_control_state()");
#if 0
        gchar *oid;
	int i, found;
	SaHpiCtrlStateT working;
        struct snmp_value get_value;
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct snmp_client_hnd *custom_handle = (struct snmp_client_hnd *)handle->data;
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, id, SAHPI_CTRL_RDR, num);
        struct ControlMibInfo *s =
                (struct ControlMibInfo *)oh_get_rdr_data(handle->rptcache, id, rdr->RecordId);

	memset(&working, 0, sizeof(SaHpiCtrlStateT));
	working.Type = rdr->RdrTypeUnion.CtrlRec.Type;
	
	switch (working.Type) {
	case SAHPI_CTRL_TYPE_DIGITAL:
		oid = snmp_derive_objid(rdr->Entity, s->oid);
		if(oid == NULL) {
			dbg("NULL SNMP OID returned for %s\n",s->oid);
			return -1;
		}
		if((net_snmp_get(custom_handle->ss, oid, &get_value) != 0) | (get_value.type != ASN_INTEGER)) {
			dbg("SNMP could not read %s; Type=%d.\n", oid, get_value.type);
			g_free(oid);
			return SA_ERR_HPI_NO_RESPONSE;
		}
		g_free(oid);
		
		found = 0;
		/* Icky dependency on SaHpiStateDigitalT enum */
		for(i=0; i<ELEMENTS_IN_SaHpiStateDigitalT; i++) {
			if(s->digitalmap[i] == get_value.type) { 
				found++;
				break; 
			}
		}

		if(found) {
			switch (i) {
			case 0:
				working.StateUnion.Digital = SAHPI_CTRL_STATE_OFF;
			case 1:
				working.StateUnion.Digital = SAHPI_CTRL_STATE_ON;
			case 2:
				working.StateUnion.Digital = SAHPI_CTRL_STATE_PULSE_OFF;
			case 3:
				working.StateUnion.Digital = SAHPI_CTRL_STATE_PULSE_ON;
			case 4:
				working.StateUnion.Digital = SAHPI_CTRL_STATE_AUTO;
			default:
				dbg("Spec Change: MAX_SaHpiStateDigitalT incorrect?\n");
				return -1;
			}
		} else {
			dbg("Control's value not defined\n");
			return -1;
		}

	case SAHPI_CTRL_TYPE_DISCRETE:
		dbg("Discrete controls not supported\n");
		return -1;
	case SAHPI_CTRL_TYPE_ANALOG:
		dbg("Analog controls not supported\n");
		return -1;
	case SAHPI_CTRL_TYPE_STREAM:
		dbg("Stream controls not supported\n");
		return -1;
	case SAHPI_CTRL_TYPE_TEXT:
		dbg("Text controls not supported\n");
		return -1;
	case SAHPI_CTRL_TYPE_OEM:	
		dbg("Oem controls not supported\n");
		return -1;
        default:
		dbg("%s has invalid control state=%d\n", s->oid,working.Type);
                return -1;
        }

	memcpy(state,&working,sizeof(SaHpiCtrlStateT));
	return SA_OK;
#endif
return(-1);
}

static int snmp_client_set_control_state(void *hnd, SaHpiResourceIdT id,
                                     SaHpiCtrlNumT num,
                                     SaHpiCtrlStateT *state)
{

dbg("TODO: snmp_client_set_control_state()");
#if 0
        gchar *oid;
	int value;
        struct snmp_value set_value;
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct snmp_client_hnd *custom_handle = (struct snmp_client_hnd *)handle->data;
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, id, SAHPI_CTRL_RDR, num);
        struct ControlMibInfo *s =
                (struct ControlMibInfo *)oh_get_rdr_data(handle->rptcache, id, rdr->RecordId);

	if(state->Type != rdr->RdrTypeUnion.CtrlRec.Type) {
		dbg("Control %s type %d cannot be changed\n",s->oid,state->Type);
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	switch (state->Type) {
	case SAHPI_CTRL_TYPE_DIGITAL:

		/* More icky dependencies on SaHpiStateDigitalT enum */
		switch (state->StateUnion.Digital) {
		case SAHPI_CTRL_STATE_OFF:
			value = s->digitalmap[SAHPI_CTRL_STATE_OFF];
		case SAHPI_CTRL_STATE_ON:
			value = s->digitalmap[SAHPI_CTRL_STATE_ON];
		case SAHPI_CTRL_STATE_PULSE_OFF:
			value = s->digitalmap[SAHPI_CTRL_STATE_PULSE_OFF];
		case SAHPI_CTRL_STATE_PULSE_ON:
			value = s->digitalmap[SAHPI_CTRL_STATE_PULSE_ON];
		case SAHPI_CTRL_STATE_AUTO:
			value = s->digitalmap[ELEMENTS_IN_SaHpiStateDigitalT - 1];
		default:
			dbg("Spec Change: MAX_SaHpiStateDigitalT incorrect?\n");
			return -1;
		}

		if(value < 0) {
			dbg("Control state %d not allowed to be set\n",state->StateUnion.Digital);
			return SA_ERR_HPI_INVALID_CMD;
		}

		oid = snmp_derive_objid(rdr->Entity, s->oid);
		if(oid == NULL) {
			dbg("NULL SNMP OID returned for %s\n",s->oid);
			return -1;
		}

		set_value.type = ASN_INTEGER;
		set_value.integer = value;

		if((snmp_client_set(custom_handle->ss, oid, set_value) != 0)) {
			dbg("SNMP could not set %s; Type=%d.\n",s->oid,set_value.type);
			g_free(oid);
			return SA_ERR_HPI_NO_RESPONSE;
		}
		g_free(oid);

	case SAHPI_CTRL_TYPE_DISCRETE:
		dbg("Discrete controls not supported\n");
		return SA_ERR_HPI_INVALID_CMD;
	case SAHPI_CTRL_TYPE_ANALOG:
		dbg("Analog controls not supported\n");
		return SA_ERR_HPI_INVALID_CMD;
	case SAHPI_CTRL_TYPE_STREAM:
		dbg("Stream controls not supported\n");
		return SA_ERR_HPI_INVALID_CMD;
	case SAHPI_CTRL_TYPE_TEXT:
		dbg("Text controls not supported\n");
		return SA_ERR_HPI_INVALID_CMD;
	case SAHPI_CTRL_TYPE_OEM:	
		dbg("Oem controls not supported\n");
		return SA_ERR_HPI_INVALID_CMD;
        default:
		dbg("Request has invalid control state=%d\n", state->Type);
                return SA_ERR_HPI_INVALID_PARAMS;
        }
	
        return SA_OK;
#endif
return(-1);
}

static int snmp_client_get_inventory_size(void *hnd, SaHpiResourceIdT id,
                                      SaHpiEirIdT num, /* yes, they don't call it a
                                                    * num, but it still is one
                                                    */
                                      SaHpiUint32T *size)
{
        return -1;
}

static int snmp_client_get_inventory_info(void *hnd, SaHpiResourceIdT id,
                                      SaHpiEirIdT num,
                                      SaHpiInventoryDataT *data)
{
        return -1;
}

static int snmp_client_set_inventory_info(void *hnd, SaHpiResourceIdT id,
                                      SaHpiEirIdT num,
                                      const SaHpiInventoryDataT *data)
{
        return -1;
}

static int snmp_client_get_watchdog_info(void *hnd, SaHpiResourceIdT id,
                                     SaHpiWatchdogNumT num,
                                     SaHpiWatchdogT *wdt)
{
	/* Watchdog not supported */
        return SA_ERR_HPI_NOT_PRESENT;
}

static int snmp_client_set_watchdog_info(void *hnd, SaHpiResourceIdT id,
                                     SaHpiWatchdogNumT num,
                                     SaHpiWatchdogT *wdt)
{
	/* Watchdog not supported */
        return SA_ERR_HPI_NOT_PRESENT;
}

static int snmp_client_reset_watchdog(void *hnd, SaHpiResourceIdT id,
                                  SaHpiWatchdogNumT num)
{
 	/* Watchdog not supported */
        return SA_ERR_HPI_NOT_PRESENT;
}

static int snmp_client_get_hotswap_state(void *hnd, SaHpiResourceIdT id,
				     SaHpiHsStateT *state)
{
        /* TODO: when is a resource ACTIVE_UNHEALTHY */
        *state = SAHPI_HS_STATE_ACTIVE_HEALTHY;

        return SA_OK;
}

static int snmp_client_set_hotswap_state(void *hnd, SaHpiResourceIdT id,
				     SaHpiHsStateT state)
{
        return -1;
}

static int snmp_client_request_hotswap_action(void *hnd, SaHpiResourceIdT id,
				          SaHpiHsActionT act)
{
        return -1;
}

static int snmp_client_get_power_state(void *hnd, SaHpiResourceIdT id,
			           SaHpiHsPowerStateT *state)
{
        return -1;
}

static int snmp_client_set_power_state(void *hnd, SaHpiResourceIdT id,
			           SaHpiHsPowerStateT state)
{
        return -1;
}

static int snmp_client_get_indicator_state(void *hnd, SaHpiResourceIdT id,
				       SaHpiHsIndicatorStateT *state)
{
        return -1;
}

static int snmp_client_set_indicator_state(void *hnd, SaHpiResourceIdT id,
				       SaHpiHsIndicatorStateT state)
{
        return -1;
}

static int snmp_client_control_parm(void *hnd, SaHpiResourceIdT id, SaHpiParmActionT act)
{
        return -1;
}

static int snmp_client_get_reset_state(void *hnd, SaHpiResourceIdT id,
			           SaHpiResetActionT *act)
{
        return -1;
}

static int snmp_client_set_reset_state(void *hnd, SaHpiResourceIdT id,
			           SaHpiResetActionT act)
{
        return -1;

}

struct oh_abi_v2 oh_snmp_client_plugin = {
        .open				= snmp_client_open,
        .close				= snmp_client_close,
        .get_event			= snmp_client_get_event,
        .discover_resources     	= snmp_client_discover_resources,
        .get_self_id			= snmp_client_get_self_id,
//        .get_sel_info			= snmp_client_get_sel_info,
        .get_sel_info			= NULL,
//        .set_sel_time			= snmp_client_set_sel_time,
        .set_sel_time			= NULL,
//        .add_sel_entry			= snmp_client_add_sel_entry,
        .add_sel_entry			= NULL,
//        .del_sel_entry			= snmp_client_del_sel_entry,
        .del_sel_entry			= NULL,
//	.get_sel_entry			= snmp_client_get_sel_entry,        
	.get_sel_entry			= NULL,
	.get_sensor_data		= snmp_client_get_sensor_data,
        .get_sensor_thresholds		= snmp_client_get_sensor_thresholds,
        .set_sensor_thresholds		= snmp_client_set_sensor_thresholds,
        .get_sensor_event_enables	= snmp_client_get_sensor_event_enables,
        .set_sensor_event_enables	= snmp_client_set_sensor_event_enables,
        .get_control_state		= snmp_client_get_control_state,
        .set_control_state		= snmp_client_set_control_state,
        .get_inventory_size		= snmp_client_get_inventory_size,
        .get_inventory_info		= snmp_client_get_inventory_info,
        .set_inventory_info		= snmp_client_set_inventory_info,
        .get_watchdog_info		= snmp_client_get_watchdog_info,
        .set_watchdog_info		= snmp_client_set_watchdog_info,
        .reset_watchdog			= snmp_client_reset_watchdog,
        .get_hotswap_state		= snmp_client_get_hotswap_state,
        .set_hotswap_state		= snmp_client_set_hotswap_state,
        .request_hotswap_action		= snmp_client_request_hotswap_action,
        .get_power_state		= snmp_client_get_power_state,
        .set_power_state		= snmp_client_set_power_state,
        .get_indicator_state		= snmp_client_get_indicator_state,
        .set_indicator_state		= snmp_client_set_indicator_state,
        .control_parm			= snmp_client_control_parm,
        .get_reset_state		= snmp_client_get_reset_state,
        .set_reset_state		= snmp_client_set_reset_state
};

int get_interface(void **pp, const uuid_t uuid)
{
        if(uuid_compare(uuid, UUID_OH_ABI_V2)==0) {
                *pp = &oh_snmp_client_plugin;
                return 0;
        }

        *pp = NULL;
        return -1;
}
