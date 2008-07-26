/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2008
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 *  Authors:
 *  Suntrupth S Yadav <suntrupth@in.ibm.com>
 */
 
#include <sim_init.h>
#include <rpt_utils.h>

static SaErrorT new_dimi(struct oh_handler_state *state,
                            struct oh_event *e,
                            struct sim_dimi *mydimi) {
        SaHpiRdrT *rdr = NULL;
        struct sim_dimi_info *info;
        SaErrorT error = SA_OK;
        
        rdr = (SaHpiRdrT *)g_malloc0(sizeof(SaHpiRdrT));
        // Copy information from rdr array to res_rdr
        rdr->RdrType = SAHPI_DIMI_RDR;
        memcpy(&rdr->RdrTypeUnion.DimiRec, &mydimi->dimirec,
               sizeof(SaHpiDimiRecT));
        oh_init_textbuffer(&rdr->IdString);
        oh_append_textbuffer(&rdr->IdString, mydimi->comment);
        rdr->RecordId =
                oh_get_rdr_uid(SAHPI_DIMI_RDR, rdr->RdrTypeUnion.DimiRec.DimiNum); 
        
        // get the entity path
        rdr->Entity = e->resource.ResourceEntity;
        
        //set up our private data
        info = (struct sim_dimi_info *)g_malloc(sizeof(struct sim_dimi_info));
        
        memcpy(&info->info, &mydimi->info, sizeof(SaHpiDimiInfoT));
        memcpy(&info->test, &mydimi->test, sizeof(SaHpiDimiTestT)); 
        
         /* everything ready so inject the rdr */
        error = sim_inject_rdr(state, e, rdr, info);
        if (error) {
                g_free(rdr);
                g_free(info);
        }

        return error;
}

SaErrorT sim_discover_chassis_dimis(struct oh_handler_state *state,
                                       struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_chassis_dimis[i].dimirec.DimiNum != 0) {
                rc = new_dimi(state, e, &sim_chassis_dimis[i]);
                if (rc) {
                        err("Error %d returned when adding chassis dimi", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d chassis dimis injected", j, i);

        return 0;
}
