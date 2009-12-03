/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004-2008
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Renier Morales <renier@openhpi.org>
 *
 */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <oh_alarm.h>
#include <oh_config.h>
#include <oh_error.h>
#include <oh_utils.h>

static void __update_dat(struct oh_domain *d)
{
        struct timeval tv;

        if (!d) return;

        gettimeofday(&tv, NULL);

        d->dat.update_count++;
        d->dat.update_timestamp = (SaHpiTimeT) tv.tv_sec * 1000000000 + tv.tv_usec * 1000;
}

static GSList *__get_alarm_node(struct oh_domain *d,
                               SaHpiAlarmIdT *aid,
                               SaHpiSeverityT *severity,
                               SaHpiStatusCondTypeT *type,
                               SaHpiResourceIdT *rid,
                               SaHpiManufacturerIdT *mid,
                               SaHpiSensorNumT *num,
                               SaHpiEventStateT *state,
                               SaHpiBoolT unacknowledged,
                               int get_next)
{
        GSList *alarms = NULL;

        if (!d) return NULL;

        if (aid) {
                if (*aid == SAHPI_FIRST_ENTRY)
                        get_next = 1;
                else if (*aid == SAHPI_LAST_ENTRY) {
                        /* Just return the last node,
                           if not getting next alarm. */
                        if (get_next)
                                return NULL;
                        else
                                return g_slist_last(d->dat.list);
                }
        }


        for (alarms = d->dat.list; alarms; alarms = alarms->next) {
                SaHpiAlarmT *alarm = alarms->data;
                if (alarm &&
                    (aid ? (get_next ? alarm->AlarmId > *aid : alarm->AlarmId == *aid) : 1) &&
                    (severity ? (*severity != SAHPI_ALL_SEVERITIES ? alarm->Severity == *severity : 1) : 1) &&
                    (type ? alarm->AlarmCond.Type == *type : 1) &&
                    (rid ? alarm->AlarmCond.ResourceId == *rid: 1) &&
                    (mid ? alarm->AlarmCond.Mid == *mid : 1) &&
                    (num ? alarm->AlarmCond.SensorNum == *num : 1) &&
                    (state ? alarm->AlarmCond.EventState == *state : 1) &&
                    (unacknowledged ? !alarm->Acknowledged : 1)) {
                        return alarms;
                }
        }

        return NULL;
}

static SaHpiUint32T __count_alarms(struct oh_domain *d,
                                   SaHpiStatusCondTypeT *type,
                                   SaHpiSeverityT sev)
{
        GSList *alarms = NULL;
        SaHpiUint32T count = 0;

        if (!d) return 0;

        if (!type && sev == SAHPI_ALL_SEVERITIES)
                return g_slist_length(d->dat.list);
        else {
                for (alarms = d->dat.list; alarms; alarms = alarms->next) {
                        SaHpiAlarmT *alarm = alarms->data;
                        if (alarm &&
                            (type ? alarm->AlarmCond.Type == *type : 1) &&
                            (sev == SAHPI_ALL_SEVERITIES ? 1 : alarm->Severity == sev)) {
                                count++;
                        }
                }
        }

        return count;
}

/**
 * oh_add_alarm
 * @d: pointer to domain
 * @alarm: alarm to be added
 * @fromfile: if True will preserve alarm's id, timestamp, and
 * acknowledge flag. Also, it will not save immediatedly to disk,
 * if OPENHPI_DAT_SAVE is set.
 *
 * Return value: reference to newly added alarm or NULL if there was
 * an error
 **/
SaHpiAlarmT *oh_add_alarm(struct oh_domain *d, SaHpiAlarmT *alarm, int fromfile)
{
        struct timeval tv1;
        SaHpiAlarmT *a = NULL;
        struct oh_global_param param = { .type = OPENHPI_DAT_SIZE_LIMIT };

        if (!d) {
                err("NULL domain pointer passed.");
                return NULL;
        }

        if (oh_get_global_param(&param))
                param.u.dat_size_limit = OH_MAX_DAT_SIZE_LIMIT;

        if (param.u.dat_size_limit != OH_MAX_DAT_SIZE_LIMIT &&
            g_slist_length(d->dat.list) >= param.u.dat_size_limit) {
                err("DAT for domain %d is overflowed", d->id);
                d->dat.overflow = SAHPI_TRUE;
                return NULL;
        } else if (alarm && alarm->AlarmCond.Type == SAHPI_STATUS_COND_TYPE_USER) {
                param.type = OPENHPI_DAT_USER_LIMIT;
                if (oh_get_global_param(&param))
                        param.u.dat_user_limit = OH_MAX_DAT_USER_LIMIT;

                if (param.u.dat_user_limit != OH_MAX_DAT_USER_LIMIT &&
                    __count_alarms(d,
                                   &alarm->AlarmCond.Type,
                                   SAHPI_ALL_SEVERITIES) >= param.u.dat_user_limit) {
                        err("DAT for domain %d has reached its user alarms limit", d->id);
                        return NULL;
                }
        }

        a = g_new0(SaHpiAlarmT, 1);
        if (alarm) { /* Copy contents of optional alarm reference */
                memcpy(a, alarm, sizeof(SaHpiAlarmT));
        }

        if (fromfile) {
                if (a->AlarmId > d->dat.next_id) {
                        d->dat.next_id = a->AlarmId;
                }
        } else {
                a->AlarmId = ++(d->dat.next_id);
                gettimeofday(&tv1, NULL);
                a->Timestamp =
                        (SaHpiTimeT) tv1.tv_sec * 1000000000 + tv1.tv_usec * 1000;
                a->Acknowledged = SAHPI_FALSE;
        }
        a->AlarmCond.DomainId = d->id;
        d->dat.list = g_slist_append(d->dat.list, a);

        /* Set alarm id and timestamp info in alarm reference */
        if (alarm) {
                alarm->AlarmId = a->AlarmId;
                alarm->Timestamp = a->Timestamp;
        }

        if (!fromfile) {
                __update_dat(d);
                param.type = OPENHPI_DAT_SAVE;
                oh_get_global_param(&param);
                if (param.u.dat_save) {
                        char dat_filepath[SAHPI_MAX_TEXT_BUFFER_LENGTH*2];
                        param.type = OPENHPI_VARPATH;
                        oh_get_global_param(&param);
                        snprintf(dat_filepath, SAHPI_MAX_TEXT_BUFFER_LENGTH*2,
                                        "%s/dat.%u", param.u.varpath, d->id);
                        oh_alarms_to_file(&d->dat, dat_filepath);
                }
        }

        return a;
}

/**
 * oh_get_alarm
 * @d: pointer to domain
 * @aid: Optional. alarm id for alarm to get
 * @severity: Optional. Severity of alarm to get
 * @type: Optional. Type of alarm to get
 * @rid: Optional. Resource Id of alarm to get
 * @mid: Optional. Manufacturer Id of alarm to get
 * @num: Optional. Sensor number of alarm to get
 * @state: Optional. Event State of alarm to get
 * @unacknowledged: If True, only gets unacknowledged.
 * @get_next: Instead of getting the exact @aid, get the next alarm
 * after that one
 *
 * Return value: pointer to SaHpiAlarmT that matched, or NULL of none found.
 **/
SaHpiAlarmT *oh_get_alarm(struct oh_domain *d,
                          SaHpiAlarmIdT *aid,
                          SaHpiSeverityT *severity,
                          SaHpiStatusCondTypeT *type,
                          SaHpiResourceIdT *rid,
                          SaHpiManufacturerIdT *mid,
                          SaHpiSensorNumT *num,
                          SaHpiEventStateT *state,
                          SaHpiBoolT unacknowledged,
                          int get_next)
{
        GSList *alarm_node = NULL;

        if (!d) return NULL;

        alarm_node = __get_alarm_node(d, aid, severity, type, rid, mid, num,
                                      state, unacknowledged, get_next);
        if (!alarm_node) return NULL;

        return alarm_node->data;
}

/**
 * oh_remove_alarm
 * @d: pointer to domain
 * @severity: Optional. Severity of alarm to remove
 * @type: Optional. Type of alarm to remove
 * @rid: Optional. Resource Id of alarm to remove
 * @mid: Optional. Manufacturer Id of alarm to remove
 * @num: Optional. Sensor Number of alarm to remove
 * @state: Optional. Event state of alarm to remove
 * @deassert_mask: Optional. Deassert Mask. Matches on a bit AND operation.
 * @multi: If True, does operation for all matching alarms, otherwise,
 * just the first matching one.
 *
 * Return value: SA_OK on success
 **/
SaErrorT oh_remove_alarm(struct oh_domain *d,
                         SaHpiSeverityT *severity,
                         SaHpiStatusCondTypeT *type,
                         SaHpiResourceIdT *rid,
                         SaHpiManufacturerIdT *mid,
                         SaHpiSensorNumT *num,
                         SaHpiEventStateT *state,
                         SaHpiEventStateT *deassert_mask,
                         int multi)
{
        GSList *alarm_node = NULL;
        SaHpiAlarmT *alarm = NULL;
        SaHpiAlarmIdT aid = SAHPI_FIRST_ENTRY; /* Set to zero */
        struct oh_global_param param = { .type = OPENHPI_DAT_SIZE_LIMIT };

        if (!d) return SA_ERR_HPI_INVALID_PARAMS;

        do {
                alarm_node = __get_alarm_node(d, &aid, severity, type, rid, mid,
                                              num, state, 0, 1);
                if (alarm_node) alarm = alarm_node->data;
                else break;

                aid = alarm->AlarmId;
                if (deassert_mask ? *deassert_mask & alarm->AlarmCond.EventState : 1) {
                        d->dat.list = g_slist_delete_link(d->dat.list, alarm_node);
                        g_free(alarm);
                }
                alarm_node = NULL;
                alarm = NULL;
        } while (multi);

        __update_dat(d);
        if (!oh_get_global_param(&param)) { /* Reset overflow flag if not overflowed */
                if (param.u.dat_size_limit != OH_MAX_DAT_SIZE_LIMIT &&
                    g_slist_length(d->dat.list) < param.u.dat_size_limit)
                        d->dat.overflow = SAHPI_FALSE;
        }

        return SA_OK;
}

/**
 * oh_close_alarmtable
 * @d: pointer to domain
 *
 * Frees all memory held by alarm table.
 *
 * Return value: SA_OK on success
 **/
SaErrorT oh_close_alarmtable(struct oh_domain *d)
{
        SaErrorT error = SA_OK;

        if (!d) return SA_ERR_HPI_INVALID_PARAMS;

        error = oh_remove_alarm(d, NULL, NULL, NULL, NULL,
                                NULL, NULL, NULL, 1);
        d->dat.next_id = 0;
        d->dat.update_count = 0;
        d->dat.update_timestamp = SAHPI_TIME_UNSPECIFIED;

        return error;
}

/**
 * oh_count_alarms
 * @d: pointer to domain
 * @sev: Severity of alarms to count
 *
 * Counts alarms in the domain table. You can count alarms of a specific
 * severity, or for all severities (SAHPI_ALL_SEVERITIES).
 *
 * Return value: Number of alarms counted.
 **/
SaHpiUint32T oh_count_alarms(struct oh_domain *d, SaHpiSeverityT sev)
{
        SaHpiUint32T count = 0;

        count = __count_alarms(d, NULL, sev);

        return count;
}

static void oh_detect_oem_event_alarm(struct oh_domain *d, SaHpiEventT *event)
{
        SaHpiAlarmT a;
        SaHpiStatusCondTypeT type = SAHPI_STATUS_COND_TYPE_OEM;

        if (!d || !event) return;

        /* Search for possible oem alarm, if severity is "non-alarming" */
        if (event->Severity > SAHPI_MINOR) {
                oh_remove_alarm(d, NULL, &type, &event->Source,
                                &event->EventDataUnion.OemEvent.MId,
                                NULL, NULL, NULL, 1);
                return;
        }

        /* Severity is "alarming". Add/Create OEM alarm */
        memset( &a, 0, sizeof( a ) );	/* Make sure alarm has valid fields */
        a.Severity = event->Severity;
        a.AlarmCond.Type = type;
        oh_entity_path_lookup(event->Source, &a.AlarmCond.Entity);
        a.AlarmCond.ResourceId = event->Source;
        a.AlarmCond.Mid = event->EventDataUnion.OemEvent.MId;
        memcpy(&a.AlarmCond.Data,
               &event->EventDataUnion.OemEvent.OemEventData,
               sizeof(SaHpiTextBufferT));
        
	oh_add_alarm(d, &a, 0);
        
	return;
}

static void oh_detect_resource_event_alarm(struct oh_domain *d, SaHpiEventT *event)
{
        SaHpiStatusCondTypeT type = SAHPI_STATUS_COND_TYPE_RESOURCE;
        SaHpiAlarmT a;

        if (!d || !event) return;

        if (event->EventType != SAHPI_ET_RESOURCE) return;

        /* Search for possible clearance of resource alarm,
           if event is not a resource failure */
        if (event->EventDataUnion.ResourceEvent.ResourceEventType !=
            SAHPI_RESE_RESOURCE_FAILURE) {
                oh_remove_alarm(d, NULL, &type, &event->Source, NULL,
                                NULL, NULL, NULL, 1);
                return;
        }

        /* Failed resource.
           Add/Create resource alarm if severity is "alarming" */
        if (event->Severity <= SAHPI_MINOR) {
                memset( &a, 0, sizeof( a ) ); /* Make sure alarm has valid
					       * fields
					       */
                a.Severity = event->Severity;
                a.AlarmCond.Type = type;
                oh_entity_path_lookup(event->Source, &a.AlarmCond.Entity);
                a.AlarmCond.ResourceId = event->Source;
                oh_add_alarm(d, &a, 0);
        }
        
	return;
}

static void oh_detect_sensor_event_alarm(struct oh_domain *d, SaHpiEventT *event)
{
        SaHpiStatusCondTypeT type = SAHPI_STATUS_COND_TYPE_SENSOR;
        SaHpiAlarmT a;

        if (!d || !event) return;

        if ( ( event->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_CURRENT_STATE ) != 0 ) {
                SaHpiEventStateT deasserted = ~ ( event->EventDataUnion.SensorEvent.CurrentState );
                oh_remove_alarm(d, NULL, &type, &event->Source, NULL,
                                &event->EventDataUnion.SensorEvent.SensorNum,
                                NULL,
                                &deasserted, 1);
        }
        if (!event->EventDataUnion.SensorEvent.Assertion) {
                /* Check for possible sensor alarm removals,
                   since sensor is not asserted. */
                oh_remove_alarm(d, NULL, &type, &event->Source, NULL,
                                &event->EventDataUnion.SensorEvent.SensorNum,
                                &event->EventDataUnion.SensorEvent.EventState,
                                NULL, 1);
        } else if (event->Severity <= SAHPI_MINOR &&
                   event->EventDataUnion.SensorEvent.Assertion) {
                /* Add sensor alarm to dat, since event is severe
                   enough and is asserted. */
                memset( &a, 0, sizeof( a ) ); /* Make sure alarm has valid
					       * fields
					       */
                a.Severity = event->Severity;
                a.AlarmCond.Type = type;
                oh_entity_path_lookup(event->Source, &a.AlarmCond.Entity);
                a.AlarmCond.ResourceId = event->Source;
                a.AlarmCond.SensorNum = event->EventDataUnion.SensorEvent.SensorNum;
                a.AlarmCond.EventState = event->EventDataUnion.SensorEvent.EventState;
                oh_add_alarm(d, &a, 0);
        }
        
	return;
}

static void oh_detect_sensor_enable_change_alarm(struct oh_domain *d,
                                                 SaHpiEventT *event)
{
        SaHpiStatusCondTypeT type = SAHPI_STATUS_COND_TYPE_SENSOR;

        if (!d || !event) return;

        if (!event->EventDataUnion.SensorEnableChangeEvent.SensorEnable ||
            !event->EventDataUnion.SensorEnableChangeEvent.SensorEventEnable) {
                oh_remove_alarm(d, NULL, &type, &event->Source, NULL,
                                &event->EventDataUnion.SensorEnableChangeEvent.SensorNum,
                                NULL, NULL, 1);
        }

}

static void oh_remove_resource_alarms(struct oh_domain *d, SaHpiResourceIdT rid, int all)
{
        SaHpiStatusCondTypeT type = SAHPI_STATUS_COND_TYPE_RESOURCE;

        if (!d || !rid) return;

        oh_remove_alarm(d, NULL, &type, &rid, NULL,
                        NULL, NULL, NULL, 1);
        if (all) {
                type = SAHPI_STATUS_COND_TYPE_OEM;
                oh_remove_alarm(d, NULL, &type, &rid, NULL,
                                NULL, NULL, NULL, 1);
                type = SAHPI_STATUS_COND_TYPE_SENSOR;
                oh_remove_alarm(d, NULL, &type, &rid, NULL,
                                NULL, NULL, NULL, 1);
        }

        return;
}

static void oh_detect_hpi_alarm(struct oh_domain *d, SaHpiEventT *event)
{
        if (!d || !event) return;

        switch (event->EventType) {
                case SAHPI_ET_OEM:
                        oh_detect_oem_event_alarm(d, event);
                        break;
                case SAHPI_ET_RESOURCE:
                        oh_detect_resource_event_alarm(d, event);
                        break;
                case SAHPI_ET_SENSOR:
                        oh_detect_sensor_event_alarm(d, event);
                        break;
                case SAHPI_ET_SENSOR_ENABLE_CHANGE:
                        oh_detect_sensor_enable_change_alarm(d, event);
                        break;
                default:;
        }

        return;
}

static void oh_detect_resource_alarm(struct oh_domain *d, SaHpiRptEntryT *res)
{
        SaHpiAlarmT a;
        SaHpiStatusCondTypeT type = SAHPI_STATUS_COND_TYPE_SENSOR;

        if (!d || !res) return;

        /* Check possible alarms for removal, if resource is not failed. */
        if (!res->ResourceFailed || res->ResourceSeverity > SAHPI_MINOR) {
                oh_remove_alarm(d, NULL, &type, &res->ResourceId, NULL,
                                NULL, NULL, NULL, 1);
        } else if (res->ResourceSeverity <= SAHPI_MINOR && res->ResourceFailed) {
                /* Otherwise, if sev is "alarming" and resource failed, create alarm. */
                memset( &a, 0, sizeof( a ) ); /* Make sure alarm has valid
					       * fields
					       */
                a.Severity = res->ResourceSeverity;
                a.AlarmCond.Type = SAHPI_STATUS_COND_TYPE_RESOURCE;
                oh_entity_path_lookup(res->ResourceId, &a.AlarmCond.Entity);
                a.AlarmCond.ResourceId = res->ResourceId;
                a.AlarmCond.Mid = res->ResourceInfo.ManufacturerId;
                memcpy(&a.AlarmCond.Data, &res->ResourceTag, sizeof(SaHpiTextBufferT));
                oh_add_alarm(d, &a, 0);
        }
        
	return;
}

/**
 * oh_detect_event_alarm
 * @d: pointer to domain
 * @e: pointer to event
 *
 * Study event and determine if alarms need to be removed.
 *
 * Return value: SA_OK on success
 **/
SaErrorT oh_detect_event_alarm(struct oh_domain *d,
                               struct oh_event *e)
{
        SaHpiEventTypeT etype;

        if (!d || !e) return SA_ERR_HPI_INVALID_PARAMS;

        etype = e->event.EventType;
        if (etype == SAHPI_ET_RESOURCE) {
                if (e->resource.ResourceId) {
                        oh_detect_resource_alarm(d, &e->resource);
                } else {
                        oh_detect_resource_event_alarm(d, &e->event);
                }
        } else if (etype == SAHPI_ET_HOTSWAP) {
                if (e->event.EventDataUnion.HotSwapEvent.HotSwapState ==
                    SAHPI_HS_STATE_NOT_PRESENT) {
                            SaHpiResourceIdT rid = e->resource.ResourceId;
                            if (!rid) rid = e->event.Source;
                            oh_remove_resource_alarms(d, rid, 1);
                }
        } else {
                oh_detect_hpi_alarm(d, &e->event);
        }

        return SA_OK;
}

/**
 * oh_detect_res_sev_alarm
 * @did: domain id
 * @res: resource id
 * @new_sev: severity being set in resource
 *
 * Detect if severity on resource change makes any alarms invalid.
 * If so, remove such alarms.
 *
 * Return value: SA_OK on success
 **/
SaErrorT oh_detect_res_sev_alarm(SaHpiDomainIdT did,
                                 SaHpiResourceIdT rid,
                                 SaHpiSeverityT new_sev)
{
        struct oh_domain *d = NULL;
        SaHpiRptEntryT *res = NULL;

        if (!rid) return SA_ERR_HPI_INVALID_PARAMS;

        d = oh_get_domain(did);
        if (!d) return SA_ERR_HPI_INVALID_DOMAIN;

        res = oh_get_resource_by_id(&d->rpt, rid);
        if (!res) {
                oh_release_domain(d);
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        if (res->ResourceSeverity <= SAHPI_MINOR && new_sev > SAHPI_MINOR)
                oh_remove_resource_alarms(d, res->ResourceId, 0);

        oh_release_domain(d);
        return SA_OK;
}


/**
 * oh_detect_sensor_enable_alarm
 * @did: domain id
 * @rid: resource id
 * @num: sensor number
 * @enable: sensor enable flag
 *
 * This will detect if a sensor-enable related alarm needs to be removed,
 * and if so, will remove it accordingly.
 *
 * Return value: SA_OK on success
 **/
SaErrorT oh_detect_sensor_enable_alarm(SaHpiDomainIdT did,
                                       SaHpiResourceIdT rid,
                                       SaHpiSensorNumT num,
                                       SaHpiBoolT enable)
{
        struct oh_domain *d = NULL;
        SaHpiStatusCondTypeT type = SAHPI_STATUS_COND_TYPE_SENSOR;
        SaErrorT error = SA_OK;

        if (!rid) return SA_ERR_HPI_INVALID_PARAMS;

        /* Only need to scan alarm table if enable is false */
        if (enable) return SA_OK;

        d = oh_get_domain(did);
        if (!d) return SA_ERR_HPI_INVALID_DOMAIN;

        /* Enable is false, so scan alarm table and remove any matching sensor alarms */
        error = oh_remove_alarm(d, NULL, &type, &rid, NULL,
                                &num, NULL, NULL, 1);

        oh_release_domain(d);
        return error;
}

/**
 * oh_detect_sensor_mask_alarm
 * @did: domain id
 * @rid: resource id
 * @num: sensor number
 * @action: event mask action
 * @deassert_mask: deassert mask
 *
 * This will detect if a sensor related alarm needs to be removed,
 * and if so, will remove it accordingly.
 *
 * Return value: SA_OK on success
 **/
SaErrorT oh_detect_sensor_mask_alarm(SaHpiDomainIdT did,
                                     SaHpiResourceIdT rid,
                                     SaHpiSensorNumT num,
                                     SaHpiSensorEventMaskActionT action,
                                     SaHpiEventStateT deassert_mask)
{
        struct oh_domain *d = NULL;
        SaHpiStatusCondTypeT type = SAHPI_STATUS_COND_TYPE_SENSOR;
        SaErrorT error = SA_OK;

        if (!rid) return SA_ERR_HPI_INVALID_PARAMS;

        if (action == SAHPI_SENS_ADD_EVENTS_TO_MASKS)
                return SA_OK;

        if (action != SAHPI_SENS_REMOVE_EVENTS_FROM_MASKS)
                return SA_ERR_HPI_INVALID_PARAMS;

        d = oh_get_domain(did);
        if (!d) return SA_ERR_HPI_INVALID_DOMAIN;

        /* Find matching sensor alarms and compare alarm's state with
           the deassert mask. If deassert for that state is being disabled
           on the sensor, then remove the alarm.
        */
        error = oh_remove_alarm(d, NULL, &type, &rid, NULL,
                                &num, NULL, &deassert_mask, 1);

        oh_release_domain(d);
        return error;
}

/**
 * oh_alarms_to_file
 *
 * @at: pointer to alarm table. alarms in this table will be saved to a file
 * @filename: file to which alarms will be saved.
 *
 * Return value: SA_OK on success.
 **/
SaErrorT oh_alarms_to_file(struct oh_dat *at, char *filename)
{
        GSList *alarms = NULL;
        int file;

        if (!at || !filename) {
                err("Invalid Parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        file = open(filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
        if (file < 0) {
                err("File '%s' could not be opened", filename);
                return SA_ERR_HPI_ERROR;
        }

        for (alarms = at->list; alarms; alarms = alarms->next) {
                int bytes_written = 0;
                bytes_written = write(file, (void *)alarms->data, sizeof(SaHpiAlarmT));
                if (bytes_written != sizeof(SaHpiAlarmT)) {
                        err("Couldn't write to file '%s'.", filename);
                        close(file);
                        return SA_ERR_HPI_ERROR;
                }
        }

        if (close(file) != 0) {
                err("Couldn't close file '%s'.", filename);
                return SA_ERR_HPI_ERROR;
        }

        return SA_OK;
}

/**
 * oh_alarms_from_file
 *
 * @d: pointer to domain. alarm table in this domain will receive
 * the alarms stored in @filename.
 * @filename: filename where alarms will be read from
 *
 * Return value: SA_OK on success
 **/
SaErrorT oh_alarms_from_file(struct oh_domain *d, char *filename)
{
        int file;
        SaHpiAlarmT alarm;

        if (!d || !filename) {
                err("Invalid Parameters");
                return SA_ERR_HPI_ERROR;
        }

        file = open(filename, O_RDONLY);
        if (file < 0) {
                err("File '%s' could not be opened", filename);
                return SA_ERR_HPI_ERROR;
        }

        while (read(file, &alarm, sizeof(SaHpiAlarmT)) == sizeof(SaHpiAlarmT)) {
                SaHpiAlarmT *a = oh_add_alarm(d, &alarm, 1);
                if (!a) {
                        close(file);
                        err("Error adding alarm read from file.");
                        return SA_ERR_HPI_ERROR;
                }
        }

        if (close(file) != 0) {
                err("Couldn't close file '%s'.", filename);
                return SA_ERR_HPI_ERROR;
        }

        return SA_OK;
}

