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


static SaErrorT new_annunciator(struct oh_handler_state *state,
                                struct oh_event *e,
                                struct sim_annunciator * myannun) {
        SaHpiRdrT *rdr;
        struct simAnnunciatorInfo *info = NULL;
        int i;
	SaErrorT error = SA_OK;

        rdr = (SaHpiRdrT *)g_malloc0(sizeof(SaHpiRdrT));
        // set up res_rdr
        rdr->RdrType = SAHPI_ANNUNCIATOR_RDR;
        memcpy(&rdr->RdrTypeUnion.AnnunciatorRec,
               &myannun->annun, sizeof(SaHpiAnnunciatorRecT));
        oh_init_textbuffer(&rdr->IdString);
        oh_append_textbuffer(&rdr->IdString, myannun->comment);

        // get the entity path
        rdr->Entity = e->resource.ResourceEntity;

        // save the announcements for the annunciator
        for (i = 0; myannun->announs[i].EntryId != 0; i++) {
                if (info == NULL) {
                        info = (struct simAnnunciatorInfo *)g_malloc0(sizeof(struct simAnnunciatorInfo));
                        // set the default mode value
                        info->mode = SAHPI_ANNUNCIATOR_MODE_SHARED;
                        // set up the announcement list
                        info->announs = oh_announcement_create();
                        if (info->announs == NULL) {
                                return SA_ERR_HPI_OUT_OF_SPACE;
                        }
                }
                /* fix the resource id for the announcement */
                myannun->announs[i].StatusCond.ResourceId = e->resource.ResourceId;

                oh_announcement_append(info->announs, &myannun->announs[i]);
        }

        /* everything ready so inject the rdr */
	error = sim_inject_rdr(state, e, rdr, info);
        if (error) {
                g_free(rdr);
                g_free(info);
        }

        return error;
}


SaErrorT sim_discover_chassis_annunciators(struct oh_handler_state *state,
                                           struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_chassis_annunciators[i].index != 0) {
                rc = new_annunciator(state, e, &sim_chassis_annunciators[i]);
                if (rc) {
                        err("Error %d returned when adding chassis annunciator", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d chassis annunciators injected", j, i);

        return 0;
}


SaErrorT sim_discover_cpu_annunciators(struct oh_handler_state *state,
                                       struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_cpu_annunciators[i].index != 0) {
                rc = new_annunciator(state, e, &sim_cpu_annunciators[i]);
                if (rc) {
                        err("Error %d returned when adding cpu annunciator", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d cpu annunciators injected", j, i);

        return 0;
}


SaErrorT sim_discover_dasd_annunciators(struct oh_handler_state *state,
                                        struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_dasd_annunciators[i].index != 0) {
                rc = new_annunciator(state, e, &sim_dasd_annunciators[i]);
                if (rc) {
                        err("Error %d returned when adding dasd annunciator", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d dasd annunciators injected", j, i);

        return 0;
}


SaErrorT sim_discover_hs_dasd_annunciators(struct oh_handler_state *state,
                                           struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_hs_dasd_annunciators[i].index != 0) {
                rc = new_annunciator(state, e, &sim_hs_dasd_annunciators[i]);
                if (rc) {
                        err("Error %d returned when adding hs dasd annunciator", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d hs dasd annunciators injected", j, i);

        return 0;
}


SaErrorT sim_discover_fan_annunciators(struct oh_handler_state *state,
                                       struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_fan_annunciators[i].index != 0) {
                rc = new_annunciator(state, e, &sim_fan_annunciators[i]);
                if (rc) {
                        err("Error %d returned when adding fan annunciator", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d fan annunciators injected", j, i);

        return 0;
}

