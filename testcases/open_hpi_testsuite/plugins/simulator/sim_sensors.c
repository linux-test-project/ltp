/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *        Christina Hernandez <hernanc@us.ibm.com>
 *        W. David Ashley <dashley@us.ibm.com>
 *	  Renier Morales <renier@openhpi.org>
 */

#include <sim_init.h>
#include <rpt_utils.h>

static SaErrorT new_sensor(struct oh_handler_state *state,
                           struct oh_event *e,
                           struct sim_sensor *mysensor) {
        SaHpiRdrT *rdr = NULL;
        struct SensorInfo *info = NULL;
	SaErrorT error = SA_OK;

        rdr = (SaHpiRdrT *)g_malloc0(sizeof(SaHpiRdrT));
        info = (struct SensorInfo *)g_malloc0(sizeof(struct SensorInfo));

        // set up rdr
        rdr->RdrType = SAHPI_SENSOR_RDR;
        rdr->RdrTypeUnion.SensorRec = mysensor->sensor;
        rdr->IsFru = 1;
        rdr->RecordId = oh_get_rdr_uid(rdr->RdrType,
                                    rdr->RdrTypeUnion.SensorRec.Num);
        oh_init_textbuffer(&rdr->IdString);
        oh_append_textbuffer(&rdr->IdString, mysensor->comment);

        rdr->Entity = e->resource.ResourceEntity;

        // now set up our extra info for the sensor
        *info = mysensor->sensor_info;

        // everything ready so add the rdr and extra info to the rptcache
        error = sim_inject_rdr(state, e, rdr, info);
	if (error) {
		g_free(rdr);
		g_free(info);
	}

        return error;
}

SaErrorT sim_discover_chassis_sensors(struct oh_handler_state *state,
                                      struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_chassis_sensors[i].index != 0) {
                rc = new_sensor(state, e, &sim_chassis_sensors[i]);
                if (rc) {
                        err("Error %s returned when adding chassis sensor",
			    oh_lookup_error(rc));
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d chassis sensors injected", j, i);

        return 0;

}

SaErrorT sim_discover_cpu_sensors(struct oh_handler_state *state,
                                  struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_cpu_sensors[i].index != 0) {
                rc = new_sensor(state, e, &sim_cpu_sensors[i]);
                if (rc) {
                        err("Error %d returned when adding cpu sensor", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d cpu sensors injected", j, i);

        return 0;

}

SaErrorT sim_discover_dasd_sensors(struct oh_handler_state *state,
                                   struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_dasd_sensors[i].index != 0) {
                rc = new_sensor(state, e, &sim_dasd_sensors[i]);
                if (rc) {
                        err("Error %d returned when adding dasd sensor", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d dasd sensors injected", j, i);

        return 0;

}

SaErrorT sim_discover_hs_dasd_sensors(struct oh_handler_state *state,
                                      struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_hs_dasd_sensors[i].index != 0) {
                rc = new_sensor(state, e, &sim_hs_dasd_sensors[i]);
                if (rc) {
                        err("Error %d returned when adding hs dasd sensor", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d hs dasd sensors injected", j, i);

        return 0;

}

SaErrorT sim_discover_fan_sensors(struct oh_handler_state *state,
                                  struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_fan_sensors[i].index != 0) {
                rc = new_sensor(state, e, &sim_fan_sensors[i]);
                if (rc) {
                        err("Error %d returned when adding fan sensor", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d fan sensors injected", j, i);

        return 0;

}
