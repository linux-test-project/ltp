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
 *      W. David Ashley <dashley@us.ibm.com>
 *	Renier Morales <renier@openhpi.org>
 *
 */

#include <math.h>

#include <sim_injector.h>
#include <oh_utils.h>
#include <oh_error.h>
#include <sim_init.h>


static SaErrorT sim_create_resourcetag(SaHpiTextBufferT *buffer, const char *str, SaHpiEntityLocationT loc)
{
        char *locstr;
        SaErrorT err = SA_OK;
        SaHpiTextBufferT working;

        if (!buffer || loc < SIM_HPI_LOCATION_BASE ||
            loc > (pow(10, OH_MAX_LOCATION_DIGITS) - 1)) {
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        err = oh_init_textbuffer(&working);
        if (err) { return(err); }

        locstr = (gchar *)g_malloc0(OH_MAX_LOCATION_DIGITS + 1);
        snprintf(locstr, OH_MAX_LOCATION_DIGITS + 1, " %d", loc);

        if (str) { oh_append_textbuffer(&working, str); }
        err = oh_append_textbuffer(&working, locstr);
        if (!err) {
                err = oh_copy_textbuffer(buffer, &working);
        }
        g_free(locstr);
        return(err);
}

/* return a handler state pointer by looking for its handler_name */
struct oh_handler_state *sim_get_handler_by_name(char *name)
{
        struct oh_handler_state *state = NULL;
        int i = 0;
        char *handler_name;
        state = (struct oh_handler_state *)g_slist_nth_data(sim_handler_states, i);
        while (state != NULL) {
                handler_name = (char *)g_hash_table_lookup(state->config,
                                                           "name");
                if (strcmp(handler_name, name) == 0) {
                        return state;
                }
                i++;
                state = (struct oh_handler_state *)g_slist_nth_data(sim_handler_states, i);
        }

        return NULL;
}

/* Sets entitypath based on entity_root, and assigns ResourceId */
static void setup_rpte(struct oh_handler_state *state,
		       SaHpiRptEntryT *rpte)
{
	SaHpiEntityPathT root_ep;
        char *entity_root = NULL;

        if (!state || !rpte) return;

	entity_root =
		(char *)g_hash_table_lookup(state->config,"entity_root");
	oh_encode_entitypath (entity_root, &root_ep);
	/* set up the rpt entry */
        oh_concat_ep(&rpte->ResourceEntity, &root_ep);
        rpte->ResourceId =
        	oh_uid_from_entity_path(&rpte->ResourceEntity);
}

/* inject a resource */
// assumptions about the input SaHpiRptEntryT *data entry
// - all fields are assumed to have valid values except
//    o EntryId (filled in by oh_add_resource function)
//    o ResourceId
//    o ResourceEntity (assumed to have only partial data)
SaErrorT sim_inject_resource(struct oh_handler_state *state,
                             struct sim_rpt *rpt_tmpl,
                             void *data,
                             struct oh_event **ohe) {
        struct oh_event *e = NULL;
        struct simResourceInfo *privinfo = NULL;
        SaErrorT rc = SA_OK;

        /* check arguments */
        if (state == NULL || rpt_tmpl == NULL || ohe == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* set up the rpt entry */
        e = g_malloc0(sizeof(struct oh_event));
        e->resource = rpt_tmpl->rpt;
        setup_rpte(state, &e->resource);
        sim_create_resourcetag(&e->resource.ResourceTag, rpt_tmpl->comment,
                               e->resource.ResourceEntity.Entry[0].EntityLocation);

        /* set up our private data store for resource state info */
        if (!data) {
                privinfo = (struct simResourceInfo *)g_malloc0(sizeof(struct simResourceInfo));
                if (e->resource.ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) {
                        privinfo->cur_hsstate = SAHPI_HS_STATE_ACTIVE;
                        privinfo->cur_indicator_hsstate = SAHPI_HS_INDICATOR_ON;
                }
                if (e->resource.ResourceCapabilities & SAHPI_CAPABILITY_POWER) {
                	privinfo->cur_powerstate = SAHPI_POWER_ON;
                }
                if (e->resource.ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) {
                	privinfo->ae_timeout = SAHPI_TIMEOUT_IMMEDIATE;
                }
                data = (void *)privinfo;
        }

        /* perform the injection */
        dbg("Injecting ResourceId %d", e->resource.ResourceId);
        rc = oh_add_resource(state->rptcache, &e->resource, data, FREE_RPT_DATA);
        if (rc) {
                err("Error %s injecting ResourceId %d",
		    oh_lookup_error(rc),
		    e->resource.ResourceId);
		g_free(e);
                return rc;
        }

        /* now add an event for the resource add */
        e->event.Source = e->resource.ResourceId;
        oh_gettimeofday(&e->event.Timestamp);
        e->event.Severity = e->resource.ResourceSeverity;
        if (e->resource.ResourceCapabilities & SAHPI_CAPABILITY_FRU) {
        	e->event.EventType = SAHPI_ET_HOTSWAP;
        	e->event.EventDataUnion.HotSwapEvent.HotSwapState =
        		SAHPI_HS_STATE_ACTIVE;
        	e->event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
        		SAHPI_HS_STATE_ACTIVE;
        } else {
		e->event.EventType = SAHPI_ET_RESOURCE;
		e->event.EventDataUnion.ResourceEvent.ResourceEventType =
			SAHPI_RESE_RESOURCE_ADDED;
        }

        *ohe = e;
        return SA_OK;
}

/* inject an rdr */
// assumptions about the input SaHpiRdrT *data entry
// - all fields are assumed to have valid values
// - no checking of the data is performed
// assuptions about the input *privdata entry
// - no checking of the data is performed
SaErrorT sim_inject_rdr(struct oh_handler_state *state,
			struct oh_event *ohe,
                        SaHpiRdrT *rdr,
                        void *data) {
        SaErrorT rc;
        SaHpiResourceIdT rid;

        /* check arguments */
        if (state == NULL || ohe == NULL || rdr == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        rid = ohe->resource.ResourceId;

        /* perform the injection */
        dbg("Injecting rdr for ResourceId %d", rid);
        rc = oh_add_rdr(state->rptcache, rid, rdr, data, 0);
        if (rc) {
                err("Error %s injecting rdr for ResourceId %d",
		    oh_lookup_error(rc), rid);
                return rc;
        }

        /* now add rdr to event */
	ohe->rdrs = g_slist_append(ohe->rdrs, (void *)rdr);

        return SA_OK;
}

/* inject an event */
// assumptions about the input oh_event *data entry
// - all fields are assumed to have valid values
// - no checking of the data is performed
SaErrorT sim_inject_event(struct oh_handler_state *state, struct oh_event *ohe) {

        /* check arguments */
        if (state == NULL || ohe == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* perform the injection */
        dbg("Injecting event");
        ohe->hid = state->hid;
        oh_evt_queue_push(state->eventq, ohe);

        return SA_OK;
}

SaErrorT sim_inject_ext_event(void *hnd,
                              SaHpiEventT *event,
                              SaHpiRptEntryT *rpte,
                              SaHpiRdrT *rdre)
{
	struct oh_handler_state *state = hnd;
	GSList *node = NULL;
	GSList *rdrs = NULL;
	struct oh_event e;
	/*
	 * Nums assigned through this call start at 1000.
	 * This assumes that no amount of RDRs within a
	 * specific type defined elsewhere in this plugin will
	 * be greater than 999. This, of course, is not an ideal
	 * solution here, but it suffices for now.
	 */
	static unsigned int ctrl_num 	 = 1000;
	static unsigned int sensor_num 	 = 1000;
	static unsigned int inv_num	 = 1000;
	static unsigned int watchdog_num = 1000;
	static unsigned int ann_num 	 = 1000;

	if (!hnd || !event || !rpte || !rdre) return SA_ERR_HPI_INVALID_PARAMS;

	dbg("Injecting external event");
	memset(&e, 0, sizeof(struct oh_event));

	if (rpte) {
		setup_rpte(state, rpte);
		event->Source = rpte->ResourceId;
	} else {
		event->Source = SAHPI_UNSPECIFIED_RESOURCE_ID;
	}

	rdrs = g_slist_append(rdrs, rdre);

	for (node = rdrs; node; node = node->next) {
		SaHpiRdrT *rdr = (SaHpiRdrT *)node->data;
		switch (rdr->RdrType) {
		case SAHPI_CTRL_RDR:
			rdr->RdrTypeUnion.CtrlRec.Num = ctrl_num++;
			rdr->RecordId =
			 oh_get_rdr_uid(rdr->RdrType,
				rdr->RdrTypeUnion.CtrlRec.Num);
			break;
		case SAHPI_SENSOR_RDR:
			rdr->RdrTypeUnion.SensorRec.Num = sensor_num++;
			rdr->RecordId =
			 oh_get_rdr_uid(rdr->RdrType,
                               	rdr->RdrTypeUnion.SensorRec.Num);
			break;
		case SAHPI_INVENTORY_RDR:
			rdr->RdrTypeUnion.InventoryRec.IdrId =
				inv_num++;
			rdr->RecordId =
			 oh_get_rdr_uid(rdr->RdrType,
                               	rdr->RdrTypeUnion.InventoryRec.IdrId);
			break;
		case SAHPI_WATCHDOG_RDR:
			rdr->RdrTypeUnion.WatchdogRec.WatchdogNum =
				watchdog_num++;
			rdr->RecordId =
			 oh_get_rdr_uid(rdr->RdrType,
                           rdr->RdrTypeUnion.WatchdogRec.WatchdogNum);
			break;
		case SAHPI_ANNUNCIATOR_RDR:
			rdr->RdrTypeUnion.AnnunciatorRec.AnnunciatorNum
				= ann_num++;
			rdr->RecordId =
			 oh_get_rdr_uid(rdr->RdrType,
                           rdr->RdrTypeUnion.AnnunciatorRec.AnnunciatorNum);
			break;
		default:
			err("Invalid record type");
			return SA_ERR_HPI_INVALID_PARAMS;
		}

		if (rpte) {
			rdr->Entity = rpte->ResourceEntity;
		}
	}

	e.event = *event;
	if (rpte) e.resource = *rpte;
	e.rdrs = rdrs;
        e.hid = state->hid;
	oh_evt_queue_push(state->eventq, oh_dup_event(&e));

	return SA_OK;
}

void * oh_inject_event (void *, SaHpiEventT *, SaHpiRptEntryT *, SaHpiRdrT *)
                __attribute__ ((weak, alias("sim_inject_ext_event")));


