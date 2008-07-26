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

static SaErrorT new_fumi(struct oh_handler_state *state,
                            struct oh_event *e,
                            struct sim_fumi *myfumi) {
        SaHpiRdrT *rdr = NULL;
        struct sim_fumi_info *info;
        SaErrorT error = SA_OK;
        
        rdr = (SaHpiRdrT *)g_malloc0(sizeof(SaHpiRdrT));
        // Copy information from rdr array to res_rdr
        rdr->RdrType = SAHPI_FUMI_RDR;
        memcpy(&rdr->RdrTypeUnion.FumiRec, &myfumi->fumirec,
               sizeof(SaHpiFumiRecT));
        oh_init_textbuffer(&rdr->IdString);
        oh_append_textbuffer(&rdr->IdString, myfumi->comment);
        rdr->RecordId =
                oh_get_rdr_uid(SAHPI_FUMI_RDR, rdr->RdrTypeUnion.FumiRec.Num); 
        // get the entity path
        rdr->Entity = e->resource.ResourceEntity;
        
        //set up our private data
        info = (struct sim_fumi_info *)g_malloc(sizeof(struct sim_fumi_info));
        
        memcpy(&info->srcinfo, &myfumi->srcinfo, sizeof(SaHpiFumiSourceInfoT));
        memcpy(&info->info, &myfumi->info, sizeof(SaHpiFumiBankInfoT)); 
        
         /* everything ready so inject the rdr */
        error = sim_inject_rdr(state, e, rdr, info);
        if (error) {
                g_free(rdr);
                g_free(info);
        }

        return error;
}

SaErrorT sim_discover_chassis_fumis(struct oh_handler_state *state,
                                       struct oh_event *e) {
        SaErrorT rc;
        int i = 0;
        int j = 0;

        while (sim_chassis_fumis[i].fumirec.Num != 0) {
                rc = new_fumi(state, e, &sim_chassis_fumis[i]);
                if (rc) {
                        err("Error %d returned when adding chassis fumi", rc);
                } else {
                        j++;
                }
                i++;
        }
        dbg("%d of %d chassis fumis injected", j, i);

        return 0;
}
