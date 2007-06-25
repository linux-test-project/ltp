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


static SaErrorT new_control(struct oh_handler_state *state,
                            struct oh_event *e,
                            struct sim_control *mycontrol) {
        SaHpiRdrT *rdr = NULL;
        struct sim_control_info *info;
	SaErrorT error = SA_OK;

	if (!state || !e || !mycontrol) return SA_ERR_HPI_INVALID_PARAMS;

	rdr = (SaHpiRdrT *)g_malloc0(sizeof(SaHpiRdrT));
        // Copy information from rdr array to res_rdr
        rdr->RdrType = SAHPI_CTRL_RDR;
        memcpy(&rdr->RdrTypeUnion.CtrlRec, &mycontrol->control,
               sizeof(SaHpiCtrlRecT));

        oh_init_textbuffer(&rdr->IdString);
        oh_append_textbuffer(&rdr->IdString, mycontrol->comment);
        rdr->RecordId =
                oh_get_rdr_uid(SAHPI_CTRL_RDR, rdr->RdrTypeUnion.CtrlRec.Num);

	rdr->Entity = e->resource.ResourceEntity;

        //set up our private data
        info = (struct sim_control_info *)g_malloc(sizeof(struct sim_control_info));
        info->mode = mycontrol->mode;

        // everything ready so add the rdr and extra info to the rptcache
	error = sim_inject_rdr(state, e, rdr, info);
        if (error) {
                g_free(rdr);
                g_free(info);
        }

        return error;
}


SaErrorT sim_discover_chassis_controls(struct oh_handler_state *state,
                                       struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_chassis_controls[i].index != 0) {
                rc = new_control(state, e, &sim_chassis_controls[i]);
                if (rc) {
                        dbg("Error %d returned when adding chassis control", rc);
                } else {
                        j++;
                }
                i++;
        }
        trace("%d of %d chassis controls injected", j, i);

        return 0;
}


SaErrorT sim_discover_cpu_controls(struct oh_handler_state *state,
                                   struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_cpu_controls[i].index != 0) {
                rc = new_control(state, e, &sim_cpu_controls[i]);
                if (rc) {
                        dbg("Error %d returned when adding cpu control", rc);
                } else {
                        j++;
                }
                i++;
        }
        trace("%d of %d cpu controls injected", j, i);

        return 0;
}


SaErrorT sim_discover_dasd_controls(struct oh_handler_state *state,
                                    struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_dasd_controls[i].index != 0) {
                rc = new_control(state, e, &sim_dasd_controls[i]);
                if (rc) {
                        dbg("Error %d returned when adding dasd control", rc);
                } else {
                        j++;
                }
                i++;
        }
        trace("%d of %d dasd controls injected", j, i);

        return 0;
}


SaErrorT sim_discover_hs_dasd_controls(struct oh_handler_state *state,
                                       struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_hs_dasd_controls[i].index != 0) {
                rc = new_control(state, e, &sim_hs_dasd_controls[i]);
                if (rc) {
                        dbg("Error %d returned when adding hs dasd control", rc);
                } else {
                        j++;
                }
                i++;
        }
        trace("%d of %d hs dasd controls injected", j, i);

        return 0;
}


SaErrorT sim_discover_fan_controls(struct oh_handler_state *state,
                                   struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_fan_controls[i].index != 0) {
                rc = new_control(state, e, &sim_fan_controls[i]);
                if (rc) {
                        dbg("Error %d returned when adding fan control", rc);
                } else {
                        j++;
                }
                i++;
        }
        trace("%d of %d fan controls injected", j, i);

        return 0;
}

