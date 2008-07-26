/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005,2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *        Renier Morales <renier@openhpi.org>
 *        Daniel de Araujo <ddearauj@us.ibm.com>
 */


#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <rtas_discover.h>
#include <rtas_utils.h>
#include <rtas_inventory.h>

SaErrorT rtas_discover_resources(void *hnd)
{
        SaErrorT error = SA_OK;
        struct oh_handler_state *h = (struct oh_handler_state *)hnd;
        static int did_discovery = 0;

        char *entity_root = NULL;
        SaHpiEntityPathT root_ep;
        SaHpiRptEntryT lone_res;

        if (did_discovery) {
                return SA_OK;
        }

        if (!hnd) {
                err("Null handle!");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        entity_root = (char *)g_hash_table_lookup(h->config, "entity_root");
        if (entity_root == NULL) {
                err("Could not aquire entity_root parameter.");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        error = oh_encode_entitypath(entity_root, &root_ep);
        if (error) {
                err("Could not convert entity path to string. Error=%s.", oh_lookup_error(error));
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        // Discover lone resource
        lone_res.ResourceEntity = root_ep;
        lone_res.ResourceCapabilities = SAHPI_CAPABILITY_RESOURCE |
                                        SAHPI_CAPABILITY_RDR |
                                        SAHPI_CAPABILITY_SENSOR |
                                        SAHPI_CAPABILITY_INVENTORY_DATA |
                                        SAHPI_CAPABILITY_POWER;
        lone_res.ResourceSeverity = SAHPI_MAJOR;
        oh_init_textbuffer(&lone_res.ResourceTag);
        oh_append_textbuffer(&lone_res.ResourceTag, entity_root);
        lone_res.ResourceId = oh_uid_from_entity_path(&lone_res.ResourceEntity);
        error = oh_add_resource(h->rptcache, &lone_res, NULL, FREE_RPT_DATA);
        if (!error) {
                struct oh_event *e =
                        (struct oh_event *)g_malloc0(sizeof(struct oh_event));
                e->hid = h->hid;
                e->event.EventType = SAHPI_ET_RESOURCE;
                e->resource = lone_res;
                e->event.Source = lone_res.ResourceId;
                e->event.Severity = lone_res.ResourceSeverity;
                e->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
                e->event.EventDataUnion.ResourceEvent.ResourceEventType = SAHPI_RESE_RESOURCE_ADDED;
                // Discover sensors
                error = rtas_discover_sensors(h, e);
                // Discover Inventory
                error = rtas_discover_inventory(h, e);
                oh_evt_queue_push(h->eventq, e);
        } else {
                err("Error adding resource. %s", oh_lookup_error(error));
                return error;
        }

        if (!error) did_discovery = 1;
        return error;
}

/**
 * rtas_discover_sensors:
 * @h: Pointer to handler's data.
 * @e: Pointer to resource's event structure.
 *
 * Discovers resource's available sensors and its events.
 *
 * Return values:
 * Adds sensor RDRs to internal Infra-structure queues - normal case
 * SA_ERR_HPI_OUT_OF_SPACE - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - incoming parameters are NULL.
 **/
SaErrorT rtas_discover_sensors(struct oh_handler_state *h,
                               struct oh_event *e)

{
        FILE *file = NULL;
        struct sensor_pair {
                SaHpiUint32T token;
                SaHpiUint32T max_index;
        } chunk;
        int sensor_num = 0;
        //char err_buf[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        SaHpiUint32T token, max_index, index = 0;
        SaHpiInt32T val, state;
        struct SensorInfo *sensor_info;

        if (!h || !e)
        {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* open the binary file and read the indexes */
        file = fopen(RTAS_SENSORS_PATH, "r");
        if (!file) {
                err("Error reading RTAS sensor file %s.", RTAS_SENSORS_PATH);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        /* The rtas-sensors file is layed out in the following fashion :
         *
         *      32-bit-integer-token 32-bit-integer-index ...
         *
         * As a result, we read the first 32-bit chunk to establish
         * the type of sensor is in the system.  We then read the next
         * 32-bit chunk to determine how many sensors exist of type
         * token.  This process repeats until we've reached EOL.
         * Note, sensor indices are not necessarily contiguous.
         */
        while (fread(&chunk, sizeof(struct sensor_pair), 1, file) == 1) {
                token = chunk.token;
                max_index = chunk.max_index;

                /* Make sure we didn't get bogus tokens */
                if (    token != RTAS_RESERVED_SENSOR_2  &&
                        token != RTAS_RESERVED_SENSOR_4  &&
                        token != RTAS_RESERVED_SENSOR_5  &&
                        token != RTAS_RESERVED_SENSOR_6  &&
                        token != RTAS_RESERVED_SENSOR_7  &&
                        token != RTAS_RESERVED_SENSOR_8  &&
                        token != RTAS_RESERVED_SENSOR_10 &&
                        token != RTAS_RESERVED_SENSOR_11 &&
                        token != RTAS_RESERVED_SENSOR_9008 )
                {
                        /* Now that we have the token and the index, we can
                         * grab the specific data about the sensor  and build the RDR.
                         */
                        for (index = 0; index <= max_index; index++) {


                                /* We cannot assume that there are sensors in each index leading up
                                 * up the maxIndex.  We must determine whether the sensor actually
                                 * exists.  We do this by calling rtas_get_sensor.  Note: A return
                                 * value of "-3" means the sensor does not exist.
                                 */

                                state = rtas_get_sensor(token, index, &val);

                                if (state != -3) {

                                        SaHpiRdrT *rdr = (SaHpiRdrT *)g_malloc0(sizeof(SaHpiRdrT));
                                        sensor_info = (struct SensorInfo*)g_malloc0(sizeof(struct SensorInfo));

                                        if (!rdr || !sensor_info) {
                                                err("Out of memory.");
                                                return SA_ERR_HPI_OUT_OF_SPACE;
                                        }

                                        rdr->RdrType = SAHPI_SENSOR_RDR;
                                        rdr->Entity  = e->resource.ResourceEntity;

                                        /* Do entity path business */
                                        //rtas_modify_sensor_ep(); //needs more research

                                        /* For now, assume sensor number represents a count.  If we decide later to
                                         * create an RPT for each sensor type (and fill in the RDRs that consist of
                                         * the sensor type), then the num will need to be reset.
                                         */
                                        rdr->RdrTypeUnion.SensorRec.Num = sensor_num++;

                                        populate_rtas_sensor_rec_info(token, &(rdr->RdrTypeUnion.SensorRec));

                                        rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_THRESHOLD | SAHPI_EC_SEVERITY;
                                        rdr->RdrTypeUnion.SensorRec.EventCtrl = SAHPI_SEC_READ_ONLY;
                                        rdr->RdrTypeUnion.SensorRec.Events = SAHPI_ES_OK | SAHPI_ES_LOWER_MINOR | SAHPI_ES_LOWER_CRIT | SAHPI_ES_UPPER_MINOR |SAHPI_ES_UPPER_CRIT;

                                        rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_FALSE;

                                        sensor_info->token = token;
                                        sensor_info->index = index;

                                        sensor_info->enabled = SAHPI_TRUE;

                                        if ( (token == RTAS_SECURITY_SENSOR) ||
                                             (token == RTAS_SURVEILLANCE_SENSOR) ) {

                                                if (val == 0) { sensor_info->enabled = SAHPI_FALSE; }

                                        }

                                        sensor_info->current_val = val;

                                        switch ((rtasSensorState)state) {

                                                case SENSOR_OK:
                                                        sensor_info->current_state = SAHPI_ES_UNSPECIFIED;
                                                        break;

                                                case SENSOR_CRITICAL_LOW:
                                                        sensor_info->current_state = SAHPI_ES_LOWER_CRIT;
                                                        break;

                                                case SENSOR_WARNING_LOW:
                                                        sensor_info->current_state = SAHPI_ES_LOWER_MINOR;
                                                        break;

                                                case SENSOR_NORMAL:
                                                        sensor_info->current_state = SAHPI_ES_OK;
                                                        break;

                                                case SENSOR_WARNING_HIGH:
                                                        sensor_info->current_state = SAHPI_ES_UPPER_MINOR;
                                                        break;

                                                case SENSOR_CRITICAL_HIGH:
                                                        sensor_info->current_state = SAHPI_ES_UPPER_CRIT;
                                                        break;

                                                default:
                                                        sensor_info->current_state = SAHPI_ES_UNSPECIFIED;
                                                        break;
                                        }


                                        rtas_get_sensor_location_code(token, index, sensor_info->sensor_location);

                                        oh_add_rdr(h->rptcache, e->resource.ResourceId,
                                                   rdr, sensor_info, 0);
                                        e->rdrs = g_slist_append(e->rdrs, rdr);
                                }
                        }
                }
        }

        fclose(file);

        return SA_OK;
}

/**
 * populate_rtas_sensor_info
 *
 * @data - pointer to the location of the SaHpiSensorRecT.
 * @token - the sensor type.
 *
 * @return - writes the specific sensor info based on the type
 *           of sensor.
 */
void populate_rtas_sensor_rec_info(int token, SaHpiSensorRecT *sensor_info)
{

        if (!sensor_info)
                return;

        sensor_info->DataFormat.IsSupported = SAHPI_TRUE;

        switch ((rtasSensorToken)token) {

                case RTAS_SECURITY_SENSOR:


                        sensor_info->EnableCtrl = SAHPI_FALSE;

                        sensor_info->Type = SAHPI_PHYSICAL_SECURITY;

                        sensor_info->DataFormat.IsSupported    = SAHPI_TRUE;
                        sensor_info->DataFormat.ReadingType    = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.BaseUnits      = SAHPI_SU_CHARACTERS;
                        sensor_info->DataFormat.ModifierUnits  = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUse    = SAHPI_SMUU_NONE;
                        sensor_info->DataFormat.Percentage     = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Flags    = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

                        sensor_info->DataFormat.Range.Max.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Max.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Max.Value.SensorUint64 = 3;

                        sensor_info->DataFormat.Range.Min.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Min.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Min.Value.SensorUint64 = 0;



                        break;

                case RTAS_THERMAL_SENSOR:

                        sensor_info->EnableCtrl = SAHPI_FALSE;

                        sensor_info->Type = SAHPI_TEMPERATURE;

                        sensor_info->DataFormat.IsSupported    = SAHPI_TRUE;
                        sensor_info->DataFormat.ReadingType    = SAHPI_SENSOR_READING_TYPE_INT64;
                        sensor_info->DataFormat.BaseUnits      = SAHPI_SU_DEGREES_C;
                        sensor_info->DataFormat.ModifierUnits  = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUse    = SAHPI_SMUU_NONE;
                        sensor_info->DataFormat.Percentage     = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Flags    = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

                        sensor_info->DataFormat.Range.Max.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Max.Type               = SAHPI_SENSOR_READING_TYPE_INT64;

                        sensor_info->DataFormat.Range.Min.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Min.Type               = SAHPI_SENSOR_READING_TYPE_INT64;


                        break;

                case RTAS_POWER_STATE_SENSOR:

                        sensor_info->EnableCtrl = SAHPI_FALSE;

                        sensor_info->Type = SAHPI_SYSTEM_ACPI_POWER_STATE;

                        sensor_info->DataFormat.IsSupported    = SAHPI_TRUE;
                        sensor_info->DataFormat.ReadingType    = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.BaseUnits      = SAHPI_SU_CHARACTERS;
                        sensor_info->DataFormat.ModifierUnits  = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUse    = SAHPI_SMUU_NONE;
                        sensor_info->DataFormat.Percentage     = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Flags    = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

                        sensor_info->DataFormat.Range.Max.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Max.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Max.Value.SensorUint64 = 7;

                        sensor_info->DataFormat.Range.Min.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Min.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Min.Value.SensorUint64 = 0;

                        break;

                case RTAS_SURVEILLANCE_SENSOR:

                        sensor_info->EnableCtrl = SAHPI_FALSE;

                        sensor_info->Type = SAHPI_MANAGEMENT_SUBSYSTEM_HEALTH;

                        sensor_info->DataFormat.ReadingType    = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.BaseUnits      = SAHPI_SU_MINUTE;
                        sensor_info->DataFormat.ModifierUnits  = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUse    = SAHPI_SMUU_NONE;
                        sensor_info->DataFormat.Percentage     = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Flags    = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

                        sensor_info->DataFormat.Range.Max.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Max.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Max.Value.SensorUint64 = 255;

                        sensor_info->DataFormat.Range.Min.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Min.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Min.Value.SensorUint64 = 0;

                        break;

                case RTAS_FAN_SENSOR:

                        sensor_info->EnableCtrl = SAHPI_FALSE;

                        sensor_info->Type = SAHPI_FAN;

                        sensor_info->DataFormat.IsSupported    = SAHPI_TRUE;
                        sensor_info->DataFormat.ReadingType    = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.BaseUnits      = SAHPI_SU_RPM;
                        sensor_info->DataFormat.ModifierUnits  = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUse    = SAHPI_SMUU_NONE;
                        sensor_info->DataFormat.Percentage     = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Flags    = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

                        sensor_info->DataFormat.Range.Max.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Max.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;

                        sensor_info->DataFormat.Range.Min.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Min.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;

                        break;

                case RTAS_VOLTAGE_SENSOR:

                        sensor_info->EnableCtrl = SAHPI_FALSE;

                        sensor_info->Type = SAHPI_VOLTAGE;

                        sensor_info->DataFormat.IsSupported    = SAHPI_TRUE;
                        sensor_info->DataFormat.ReadingType    = SAHPI_SENSOR_READING_TYPE_INT64;
                        sensor_info->DataFormat.BaseUnits      = SAHPI_SU_VOLTS;
                        sensor_info->DataFormat.ModifierUnits  = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUse    = SAHPI_SMUU_NONE;
                        sensor_info->DataFormat.Percentage     = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Flags    = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

                        sensor_info->DataFormat.Range.Max.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Max.Type               = SAHPI_SENSOR_READING_TYPE_INT64;

                        sensor_info->DataFormat.Range.Min.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Min.Type               = SAHPI_SENSOR_READING_TYPE_INT64;


                        break;


                case RTAS_CONNECTOR_SENSOR:

                        sensor_info->EnableCtrl = SAHPI_FALSE;

                        sensor_info->Type = SAHPI_SLOT_CONNECTOR;

                        sensor_info->DataFormat.IsSupported    = SAHPI_TRUE;
                        sensor_info->DataFormat.ReadingType    = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.BaseUnits      = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUnits  = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUse    = SAHPI_SMUU_NONE;
                        sensor_info->DataFormat.Percentage     = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Flags    = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

                        sensor_info->DataFormat.Range.Max.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Max.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Max.Value.SensorUint64 = 4;

                        sensor_info->DataFormat.Range.Min.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Min.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Min.Value.SensorUint64 = 0;

                        break;

                case RTAS_POWER_SUPPLY_SENSOR:

                        sensor_info->EnableCtrl = SAHPI_FALSE;

                        sensor_info->Type = SAHPI_POWER_SUPPLY;

                        sensor_info->DataFormat.IsSupported    = SAHPI_TRUE;
                        sensor_info->DataFormat.ReadingType    = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.BaseUnits      = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUnits  = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUse    = SAHPI_SMUU_NONE;
                        sensor_info->DataFormat.Percentage     = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Flags    = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

                        sensor_info->DataFormat.Range.Max.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Max.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Max.Value.SensorUint64 = 3;

                        sensor_info->DataFormat.Range.Min.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Min.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Min.Value.SensorUint64 = 0;

                        break;

                case RTAS_GIQ_SENSOR:

                        sensor_info->EnableCtrl = SAHPI_TRUE;

                        sensor_info->Type = SAHPI_CRITICAL_INTERRUPT;

                        sensor_info->DataFormat.IsSupported    = SAHPI_TRUE;
                        sensor_info->DataFormat.ReadingType    = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.BaseUnits      = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUnits  = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUse    = SAHPI_SMUU_NONE;
                        sensor_info->DataFormat.Percentage     = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Flags    = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

                        sensor_info->DataFormat.Range.Max.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Max.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Max.Value.SensorUint64 = 1;

                        sensor_info->DataFormat.Range.Min.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Min.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Min.Value.SensorUint64 = 0;

                        break;

                case RTAS_SYSTEM_ATTENTION_SENSOR:

                        sensor_info->EnableCtrl = SAHPI_FALSE;

                        sensor_info->Type = SAHPI_PLATFORM_ALERT;

                        sensor_info->DataFormat.IsSupported    = SAHPI_TRUE;
                        sensor_info->DataFormat.ReadingType    = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.BaseUnits      = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUnits  = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUse    = SAHPI_SMUU_NONE;
                        sensor_info->DataFormat.Percentage     = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Flags    = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

                        sensor_info->DataFormat.Range.Max.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Max.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Max.Value.SensorUint64 = 1;

                        sensor_info->DataFormat.Range.Min.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Min.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Min.Value.SensorUint64 = 0;

                        break;

                case RTAS_IDENTIFY_INDICATOR_SENSOR:

                        sensor_info->EnableCtrl = SAHPI_FALSE;

                        sensor_info->Type = SAHPI_OTHER_FRU;

                        sensor_info->DataFormat.ReadingType    = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.BaseUnits      = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUnits  = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUse    = SAHPI_SMUU_NONE;
                        sensor_info->DataFormat.Percentage     = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Flags    = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

                        sensor_info->DataFormat.Range.Max.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Max.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Max.Value.SensorUint64 = 1;

                        sensor_info->DataFormat.Range.Min.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Min.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Min.Value.SensorUint64 = 0;

                        break;

                case RTAS_COMPONENT_RESET_STATE_SENSOR:

                        sensor_info->EnableCtrl = SAHPI_FALSE;

                        sensor_info->Type = SAHPI_OPERATIONAL;

                        sensor_info->DataFormat.ReadingType    = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.BaseUnits      = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUnits  = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUse    = SAHPI_SMUU_NONE;
                        sensor_info->DataFormat.Percentage     = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Flags    = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

                        sensor_info->DataFormat.Range.Max.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Max.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Max.Value.SensorUint64 = 2;

                        sensor_info->DataFormat.Range.Min.IsSupported        = SAHPI_TRUE;
                        sensor_info->DataFormat.Range.Min.Type               = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.Range.Min.Value.SensorUint64 = 0;

                        break;

                default:

                        sensor_info->EnableCtrl = SAHPI_FALSE;

                        sensor_info->Type = SAHPI_OEM_SENSOR;

                        sensor_info->DataFormat.ReadingType    = SAHPI_SENSOR_READING_TYPE_UINT64;
                        sensor_info->DataFormat.BaseUnits      = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUnits  = SAHPI_SU_UNSPECIFIED;
                        sensor_info->DataFormat.ModifierUse    = SAHPI_SMUU_NONE;
                        sensor_info->DataFormat.Percentage     = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Flags    = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

                        sensor_info->DataFormat.Range.Max.IsSupported        = SAHPI_FALSE;

                        sensor_info->DataFormat.Range.Min.IsSupported        = SAHPI_FALSE;

                        break;

        }
}

/**
 * rtas_discover_inventory:
 * @h: Pointer to handler's data.
 * @e: Pointer to resource's event structure.
 *
 * Discovers resource's available inventory fields.
 *
 * Return values:
 * Adds inventory RDRs to internal Infra-structure queues - normal case
 * SA_ERR_HPI_OUT_OF_SPACE - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - incoming parameters are NULL.
 **/
SaErrorT rtas_discover_inventory(struct oh_handler_state *h,
                                 struct oh_event *e)
{
        const int MAXLINE = 64;
        char line[MAXLINE];
        FILE *file = NULL;
        SaHpiInventoryRecT irec;
        struct oh_rtas_idr oh_idr;
        struct oh_rtas_idr_area *oh_area = NULL;

        if (!h || !e) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Prepare Inventory Data Record headers */
        irec.IdrId = oh_idr.hpi_idr.IdrId = SAHPI_DEFAULT_INVENTORY_ID;
        irec.Persistent = SAHPI_FALSE;
        irec.Oem = 0;
        oh_idr.hpi_idr.ReadOnly = SAHPI_TRUE;

        /* open the binary file and read the indexes */
        file = popen(LSVPD_CMD, "r");
        if (!file) {
                err("Error finding rtas inventory command %s", LSVPD_CMD);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Loop through output. Create area(s) with fields (more than one area based on blocks) */
        while (fgets(line, MAXLINE, file) != NULL) {
                /* Parse two columns of output. First one tells the field type, */
                /* second, one tells the data.*/
                /* Write switch statement on it to create new area and/or field */
                char type[3];

                if (strlen(line) < 5) { /* Check line. 5 chars long at least. */
                        err("Bad line from " LSVPD_CMD);
                        pclose(file);
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                strncpy(type, (line+1), sizeof(char)*2);
                type[2] = '\0';

                if (strncmp(type, "VC", sizeof(char)*2) == 0 ||
                    strncmp(type, "DS", sizeof(char)*2) == 0) {
                        /* Create new area */
                        oh_area = g_new0(struct oh_rtas_idr_area, 1);
                        oh_area->hpi_idr_area.AreaId = ++oh_idr.hpi_idr.NumAreas;
                        oh_area->hpi_idr_area.ReadOnly = SAHPI_TRUE;
                        oh_area->hpi_idr_area.Type = rtas_get_idr_area_type(type);
                        oh_idr.areas = g_slist_append(oh_idr.areas, oh_area);
                }

                if (oh_area) {
                        /* Create field */
                        SaHpiIdrFieldT *field = g_new0(SaHpiIdrFieldT, 1);
                        field->AreaId = oh_area->hpi_idr_area.AreaId;
                        field->FieldId = ++oh_area->hpi_idr_area.NumFields;
                        field->Type = rtas_get_idr_field_type(type);
                        field->ReadOnly = SAHPI_TRUE;
                        oh_init_textbuffer(&field->Field);
                        oh_append_textbuffer(&field->Field, line+4);
                        oh_area->fields = g_slist_append(oh_area->fields, field);
                } else {
                        err("Bad Error creating field. There is no area yet.");
                        pclose(file);
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }


        }
        pclose(file);

        /* Publish IDR headers: */
        /* Memdup idr_info, add as data to idr_rec (add idr_rec to rptcache). Create rdr event. */
        SaHpiRdrT *rdr = (SaHpiRdrT *)g_malloc0(sizeof(SaHpiRdrT));
        rdr->RdrType = SAHPI_INVENTORY_RDR;
        rdr->Entity = e->resource.ResourceEntity;
        oh_init_textbuffer(&rdr->IdString);
        oh_append_textbuffer(&rdr->IdString, LSVPD_CMD);
        rdr->RdrTypeUnion.InventoryRec = irec;
        oh_add_rdr(h->rptcache,
                   e->resource.ResourceId,
                   rdr,
                   g_memdup(&oh_idr, sizeof(struct oh_rtas_idr)),
                   0);
        e->rdrs = g_slist_append(e->rdrs, rdr);

        return SA_OK;
}

void * oh_discover_resources (void *)
        __attribute__ ((weak, alias("rtas_discover_resources")));

