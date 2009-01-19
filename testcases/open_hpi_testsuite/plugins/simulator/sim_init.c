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


/* This list maintains a list of all handler state structs. It is used by
   to determine the name of a handler so that the pointer to the handler state
   can be returned to an injector API.
 */
GSList *sim_handler_states = NULL;


void *sim_open(GHashTable *handler_config,
               unsigned int hid,
               oh_evt_queue *eventq)
{
        struct oh_handler_state *state = NULL;
        char *tok = NULL;

        if (!handler_config) {
                err("GHashTable *handler_config is NULL!");
                return NULL;
        } else if (!hid) {
                err("Bad handler id passed.");
                return NULL;
        } else if (!eventq) {
                err("No event queue was passed.");
                return NULL;
        }
        /* check for required hash table entries */
        tok = g_hash_table_lookup(handler_config, "entity_root");
        if (!tok) {
                err("entity_root is needed and not present in conf");
                return NULL;
        }

        state = g_malloc0(sizeof(struct oh_handler_state));
        if (!state) {
                err("out of memory");
                return NULL;
        }

        /* initialize rpt hashtable pointer */
        state->rptcache = (RPTable *)g_malloc0(sizeof(RPTable));
        oh_init_rpt(state->rptcache);

        /* initialize the event log */
        state->elcache = oh_el_create(256);
        if (!state->elcache) {
                err("Event log creation failed");
                g_free(state->rptcache);
                g_free(state);
                return NULL;
        }

        /* save the handler config hash table, it holds  */
        /* the openhpi.conf file config info             */
        state->config = handler_config;

        /* Store reference to event queue */
        state->eventq = eventq;

        /* Store id of this handler */
        state->hid = hid;

        /* save the handler state to our list */
        sim_handler_states = g_slist_append(sim_handler_states, state);

        return (void *)state;
}


SaErrorT sim_discover(void *hnd)
{
        /* NOTE!!!!!!!!!!!!!!!!!!!!!!!!!!!!
           Since the simulator uses the full managed hot swap model and we
           do not have any latency issues, discovery only needs to be performed
           one time for each handler instance. Subsequent calls should just
           return SA_OK for that instance.
         */
        struct oh_handler_state *inst = (struct oh_handler_state *)hnd;
        int i;
        struct oh_event *e = NULL;
        SaErrorT error = SA_OK;

        /* We use the inst->data variable to store the initial discovery state
           for an instance of the handler.
         */
        if (inst->data) {
                return SA_OK;
        }

        /* ---------------------------------------------------------------
           The following assumes that the resource array is in a specific
           order. Changing this order means changing some of this code.
           ------------------------------------------------------------ */

        /* discover chassis resources and RDRs */
        i = SIM_RPT_ENTRY_CHASSIS - 1;
        error = sim_inject_resource(inst, &sim_rpt_array[i], NULL, &e);
        if (!error) {
		sim_discover_chassis_sensors(inst, e);
		sim_discover_chassis_controls(inst, e);
		sim_discover_chassis_annunciators(inst, e);
		sim_discover_chassis_watchdogs(inst, e);
		sim_discover_chassis_inventory(inst, e);
		sim_discover_chassis_dimis(inst,e); 
		sim_discover_chassis_fumis(inst,e);
		sim_inject_event(inst, e);
		e = NULL;
        } else err("Error discovering chassis");

        /* discover cpu resources and RDRs */
        i = SIM_RPT_ENTRY_CPU - 1;
        error = sim_inject_resource(inst, &sim_rpt_array[i], NULL, &e);
        if (!error) {
        	sim_discover_cpu_sensors(inst, e);
        	sim_discover_cpu_controls(inst, e);
        	sim_discover_cpu_annunciators(inst, e);
        	sim_discover_cpu_watchdogs(inst, e);
        	sim_discover_cpu_inventory(inst, e);
		sim_inject_event(inst, e);
        	e = NULL;
	} else err("Error discovering CPU");

        /* discover dasd resources and RDRs */
        i = SIM_RPT_ENTRY_DASD - 1;
	error = sim_inject_resource(inst, &sim_rpt_array[i], NULL, &e);
	if (!error) {
		sim_discover_dasd_sensors(inst, e);
		sim_discover_dasd_controls(inst, e);
		sim_discover_dasd_annunciators(inst, e);
		sim_discover_dasd_watchdogs(inst, e);
		sim_discover_dasd_inventory(inst, e);
		sim_inject_event(inst, e);
		e = NULL;
        } else err("Error discovering DASD");

        /* discover hot swap dasd resources and RDRs */
        i = SIM_RPT_ENTRY_HS_DASD - 1;
	error = sim_inject_resource(inst, &sim_rpt_array[i], NULL, &e);
	if (!error) {
		sim_discover_hs_dasd_sensors(inst, e);
		sim_discover_hs_dasd_controls(inst, e);
		sim_discover_hs_dasd_annunciators(inst, e);
		sim_discover_hs_dasd_watchdogs(inst, e);
		sim_discover_hs_dasd_inventory(inst, e);
		sim_inject_event(inst, e);
		e = NULL;
        } else err("Error discovering HS DASD");

        /* discover fan resources and RDRs */
        i = SIM_RPT_ENTRY_FAN - 1;
	error = sim_inject_resource(inst, &sim_rpt_array[i], NULL, &e);
	if (!error) {
		sim_discover_fan_sensors(inst, e);
		sim_discover_fan_controls(inst, e);
		sim_discover_fan_annunciators(inst, e);
		sim_discover_fan_watchdogs(inst, e);
		sim_discover_fan_inventory(inst, e);
		sim_inject_event(inst, e);
		e = NULL;
        } else err("Error discovering FAN");

        /* Let subsequent discovery invocations know that discovery has already
           been performed.
         */
        inst->data = (void *)1;
        return SA_OK;
}


/*
 * Return values:
 * 1 - events to be processed.
 * SA_OK - No events to be processed.
 * SA_ERR_HPI_INVALID_PARAMS - @hnd is NULL.
 */
SaErrorT sim_get_event(void *hnd)
{

        if (!hnd) return SA_ERR_HPI_INVALID_PARAMS;
        
	return SA_OK;
}


SaErrorT sim_close(void *hnd)
{
        struct oh_handler_state *state = hnd;

        /* TODO: we may need to do more here than just this! */
//      g_free(state->rptcache);
        g_free(state);
        return 0;
}

SaErrorT sim_set_resource_tag(void *hnd, SaHpiResourceIdT id, SaHpiTextBufferT *tag)
{
        struct oh_handler_state *inst = hnd;
        SaHpiRptEntryT *resource = NULL;

        if (!tag)
                return SA_ERR_HPI_INVALID_PARAMS;

        resource = oh_get_resource_by_id(inst->rptcache, id);
        if (!resource) {
                return SA_ERR_HPI_NOT_PRESENT;
        }

        memcpy(&resource->ResourceTag, tag, sizeof(SaHpiTextBufferT));

        return SA_OK;
}

SaErrorT sim_set_resource_severity(void *hnd, SaHpiResourceIdT rid, SaHpiSeverityT sev)
{
	struct oh_handler_state *h = hnd;
	SaHpiRptEntryT *resource = NULL;
	
	resource = oh_get_resource_by_id(h->rptcache, rid);
	if (!resource) {
		return SA_ERR_HPI_NOT_PRESENT;
	}
	
	resource->ResourceSeverity = sev;
	
	return SA_OK;
}

SaErrorT sim_resource_failed_remove(void *hnd, SaHpiResourceIdT rid)
{
	struct oh_handler_state *h;
	SaHpiRptEntryT *resource = NULL;
	struct oh_event e;
	SaHpiHsStateT hsstate = SAHPI_HS_STATE_ACTIVE;
	SaErrorT rv;

	if (hnd == NULL) {
		err("Invalid parameter");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	h = (struct oh_handler_state *) hnd;
	resource = oh_get_resource_by_id(h->rptcache, rid);
	if (resource == NULL) {
		err("Failed to get the RPT entry");
		return SA_ERR_HPI_NOT_PRESENT;
	}

	if (resource->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) {
		rv = sim_get_hotswap_state(hnd, rid, &hsstate);
		if (rv != SA_OK) {
			err("Failed to get the hotswap state");
			return rv;
		}
	}

	/* Raise the resource removal hotswap event */
	memset(&e, 0, sizeof(struct oh_event));
	e.hid = h->hid;
	e.resource = *resource;
	e.rdrs = NULL;
	e.event.Source = rid;
	e.event.Severity = resource->ResourceSeverity;
	oh_gettimeofday(&e.event.Timestamp);
	e.event.EventType = SAHPI_ET_HOTSWAP;
	e.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = hsstate;
	e.event.EventDataUnion.HotSwapEvent.HotSwapState =
		SAHPI_HS_STATE_NOT_PRESENT;
	e.event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
		SAHPI_HS_CAUSE_USER_UPDATE;

	oh_evt_queue_push(h->eventq, oh_dup_event(&e));

	/* Remove the failed resource from plugin rptcache */
	rv = oh_remove_resource(h->rptcache, rid);
	if (rv != SA_OK) {
		err("Resource removal from RPTable failed");
		return rv;
	}

	return SA_OK;
}

/*
 * Simulator plugin interface
 *
 */

void * oh_open (GHashTable *, unsigned int, oh_evt_queue *) __attribute__ ((weak, alias("sim_open")));

void * oh_close (void *) __attribute__ ((weak, alias("sim_close")));

void * oh_get_event (void *)
                __attribute__ ((weak, alias("sim_get_event")));

void * oh_discover_resources (void *)
                __attribute__ ((weak, alias("sim_discover")));

void * oh_set_resource_tag (void *, SaHpiResourceIdT, SaHpiTextBufferT *)
                __attribute__ ((weak, alias("sim_set_resource_tag")));
                
void * oh_set_resource_severity (void *, SaHpiResourceIdT, SaHpiSeverityT)
		__attribute__ ((weak, alias("sim_set_resource_severity")));

void * oh_resource_failed_remove (void *, SaHpiResourceIdT)
		__attribute__ ((weak, alias("sim_resource_failed_remove")));

