/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 * (C) Copyright IBM Corp. 2003-2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Louis Zhuang <louis.zhuang@linux.intel.com>
 *     David Judkovics <djudkovi@us.ibm.com>
 *     Sean Dague <http://dague.net/sean>
 *     Renier Morales <renier@openhpi.org>
 *     Racing Guo <racing.guo@intel.com>
 */

#include <string.h>

#include <oh_event.h>
#include <oh_config.h>
#include <oh_handler.h>
#include <oh_plugin.h>
#include <oh_domain.h>
#include <oh_session.h>
#include <oh_alarm.h>
#include <oh_utils.h>
#include <oh_error.h>

struct _oh_evt_queue {
        GAsyncQueue *q;
};
oh_evt_queue oh_process_q = { .q = NULL };

extern GMutex *oh_event_thread_mutex;

/*
 *  The following is required to set up the thread state for
 *  the use of event async queues.  This is true even if we aren't
 *  using live threads.
 */
int oh_event_init()
{
        dbg("Setting up event processing queue");
        if (!oh_process_q.q) oh_process_q.q = g_async_queue_new();
        if (oh_process_q.q) {
                dbg("Set up processing queue");
                return 1;
        } else {
                err("Failed to allocate processing queue");
                return 0;
        }
}

void oh_evt_queue_push(oh_evt_queue *equeue, gpointer data)
{
        g_async_queue_push(equeue->q, data);
        return;
}

void oh_event_free(struct oh_event *e, int only_rdrs)
{
	if (e) {
		if (e->rdrs) {
			GSList *node = NULL;
			for (node = e->rdrs; node; node = node->next) {
				g_free(node->data);
			}
			g_slist_free(e->rdrs);
		}
		if (!only_rdrs) g_free(e);
	}
}

struct oh_event *oh_dup_event(struct oh_event *old_event)
{
	GSList *node = NULL;
	struct oh_event *e = NULL;

	if (!old_event) return NULL;

	e = (struct oh_event *)g_malloc0(sizeof(struct oh_event));
	*e = *old_event;
	e->rdrs = NULL;
	for (node = old_event->rdrs; node; node = node->next) {
		e->rdrs = g_slist_append(e->rdrs, g_memdup(node->data,
							   sizeof(SaHpiRdrT)));
	}

	return e;
}

/*
 *  Event processing is split up into 2 stages
 *  1. Harvesting of the events
 *  2. Processing of the events into: Domain Event Log, Alarm Table,
 *  Session queues, Resource Precense Table.
 *
 */

static SaErrorT harvest_events_for_handler(struct oh_handler *h)
{
        SaErrorT error = SA_OK;

	if (!h->hnd || !h->abi->get_event) return SA_OK;

        do {
                error = h->abi->get_event(h->hnd);
                if (error < 1) {
                        dbg("Handler is out of Events");
                }
        } while (error > 0);

        return SA_OK;
}

SaErrorT oh_harvest_events()
{
        SaErrorT error = SA_ERR_HPI_ERROR;
        unsigned int hid = 0, next_hid;
        struct oh_handler *h = NULL;

        oh_getnext_handler_id(hid, &next_hid);
        while (next_hid) {
                dbg("harvesting for %d", next_hid);
                hid = next_hid;

                h = oh_get_handler(hid);
                if (!h) {
                        err("No such handler %d", hid);
                        break;
                }
                /*
                 * Here we want to record an error unless there is
                 * at least one harvest_events_for_handler that
                 * finished with SA_OK. (RM 1/6/2005)
                 */
                if (harvest_events_for_handler(h) == SA_OK && error)
                        error = SA_OK;

                oh_release_handler(h);

                oh_getnext_handler_id(hid, &next_hid);
        }

        return error;
}

static int oh_add_event_to_del(struct oh_domain *d, struct oh_event *e)
{
        struct oh_global_param param = { .type = OPENHPI_LOG_ON_SEV };
        char del_filepath[SAHPI_MAX_TEXT_BUFFER_LENGTH*2];
        int error = 0;

        if (!d || !e) return -1;

        oh_get_global_param(&param);

        /* Events get logged in DEL if they are of high enough severity */
        if (e->event.EventType == SAHPI_ET_USER ||
            e->event.Severity <= param.u.log_on_sev) {
                param.type = OPENHPI_DEL_SAVE;
                oh_get_global_param(&param);
		SaHpiEventLogInfoT elinfo;

                SaHpiRdrT *rdr = (e->rdrs) ? (SaHpiRdrT *)e->rdrs->data : NULL;
                SaHpiRptEntryT *rpte =
                        (e->resource.ResourceCapabilities) ?
                                &e->resource : NULL;
		error = oh_el_info(d->del, &elinfo);
		if (error == SA_OK && elinfo.Enabled) {
                	error = oh_el_append(d->del, &e->event, rdr, rpte);
		}

                if (param.u.del_save) {
                        param.type = OPENHPI_VARPATH;
                        oh_get_global_param(&param);
                        snprintf(del_filepath,
                                 SAHPI_MAX_TEXT_BUFFER_LENGTH*2,
                                 "%s/del.%u", param.u.varpath, d->id);
                        oh_el_map_to_file(d->del, del_filepath);
                }
        }

        return error;
}

static int process_hpi_event(struct oh_domain *d, struct oh_event *e)
{
        int i;
        GArray *sessions = NULL;
        SaHpiSessionIdT sid;
        SaHpiEventT *event = NULL;
        SaHpiRptEntryT *resource = NULL;
        SaHpiRdrT *rdr = NULL;

        if (!d || !e) return -1;

        event = &e->event;
        resource = &e->resource;
        rdr = (e->rdrs) ? (SaHpiRdrT *)e->rdrs->data : NULL;

        if (event->EventType == SAHPI_ET_USER) {
                resource->ResourceCapabilities = 0;
                if (rdr) rdr->RdrType = SAHPI_NO_RECORD;
        }

        oh_add_event_to_del(d, e);
        dbg("Added event to EL");

        /*
         * Here is the SESSION MULTIPLEXING code
         */
        sessions = oh_list_sessions(d->id);
        if (!sessions) {
                err("Error: Got an empty session list on domain id %u", d->id);
                return -2;
        }
        dbg("Got session list for domain %u", d->id);

        /* Drop events if there are no sessions open to receive them.
         */
        if (sessions->len < 1) {
                g_array_free(sessions, TRUE);
                dbg("No sessions open for event's domain %u. "
                    "Dropping hpi_event", d->id);
                return 0;
        }

        /* multiplex event to the appropriate sessions */
        for (i = 0; i < sessions->len; i++) {
                SaHpiBoolT is_subscribed = SAHPI_FALSE;
#if defined(__sparc) || defined(__sparc__)
                sid = ((SaHpiSessionIdT *)((void *)(sessions->data)))[i];
#else
                sid = g_array_index(sessions, SaHpiSessionIdT, i);
#endif
                oh_get_session_subscription(sid, &is_subscribed);
                if (is_subscribed) {
                        oh_queue_session_event(sid, e);
                }
        }
        g_array_free(sessions, TRUE);
        dbg("done multiplexing event into sessions");

        return 0;
}

static int process_resource_event(struct oh_domain *d, struct oh_event *e)
{
        RPTable *rpt = NULL;
        SaHpiRptEntryT *exists = NULL;
        unsigned int *hidp = NULL;
        SaErrorT error = SA_OK;
        SaHpiResourceEventTypeT *retype = NULL;
        SaHpiHsStateT state = SAHPI_HS_STATE_NOT_PRESENT;
        SaHpiBoolT process_hpi = TRUE;

        if (!d || !e) return -1;

	rpt = &(d->rpt);
	exists = oh_get_resource_by_id(rpt, e->resource.ResourceId);

	if (e->event.EventType == SAHPI_ET_RESOURCE) {
		retype = &e->event.EventDataUnion.ResourceEvent.ResourceEventType;
		if (*retype != SAHPI_RESE_RESOURCE_FAILURE) {
			/* If previously failed, set EventT to RESTORED */
			if (exists && exists->ResourceFailed) {
				*retype = SAHPI_RESE_RESOURCE_RESTORED;
			} else if (exists &&
				   !exists->ResourceFailed) {
				process_hpi = FALSE;
			}
			e->resource.ResourceFailed = SAHPI_FALSE;
		} else {
			e->resource.ResourceFailed = SAHPI_TRUE;
		}
	} else if (e->event.EventType == SAHPI_ET_HOTSWAP) {
		state = e->event.EventDataUnion.HotSwapEvent.HotSwapState;
		if (state == SAHPI_HS_STATE_NOT_PRESENT) {
			oh_remove_resource(rpt, e->resource.ResourceId);
		}
	} else {
		err("Expected a resource or hotswap event.");
		return -1;
	}

	if (e->event.EventType == SAHPI_ET_RESOURCE ||
	    (e->event.EventType == SAHPI_ET_HOTSWAP &&
	     state != SAHPI_HS_STATE_NOT_PRESENT)) {
        	hidp = g_malloc0(sizeof(unsigned int));
		*hidp = e->hid;
		error = oh_add_resource(rpt, &e->resource,
					hidp, FREE_RPT_DATA);
		if (error == SA_OK && !exists) {
			GSList *node = NULL;
			for (node = e->rdrs; node; node = node->next) {
                                SaHpiRdrT *rdr = (SaHpiRdrT *)node->data;
				oh_add_rdr(rpt, e->resource.ResourceId,
					   rdr, NULL, 0);
			}
		}
	}

	if (process_hpi) process_hpi_event(d, e);

        return 0;
}

static int process_event(SaHpiDomainIdT did,
                         struct oh_event *e)
{
        struct oh_domain *d = NULL;

        if (!e) {
		err("Got NULL event");
		return -1;
	}

        d = oh_get_domain(did);
        if (!d) return -2;

        dbg("Processing event for domain %u", d->id);

        switch (e->event.EventType) {
        case SAHPI_ET_RESOURCE:
                if (!e->hid) {
                        err("Resource event with invalid handler id! Dropping.");
                        break;
                } else if (!(e->resource.ResourceCapabilities &
                             SAHPI_CAPABILITY_RESOURCE)) {
                        err("Resource event with invalid capabilities. Dropping.");
                        break;
                } else if ((e->resource.ResourceCapabilities & SAHPI_CAPABILITY_FRU) &&
			   (e->event.EventDataUnion.ResourceEvent.ResourceEventType == SAHPI_RESE_RESOURCE_ADDED)) {
                        err("Invalid event. Resource in resource added event "
                            "has FRU capability. Dropping.");
                } else {
                        process_resource_event(d, e);
                }
                break;
        case SAHPI_ET_HOTSWAP:
                if (!e->hid) {
                        err("Hotswap event with invalid handler id! Dropping.");
                        break;
                } else if (!(e->resource.ResourceCapabilities &
                             SAHPI_CAPABILITY_RESOURCE)) {
                        err("Hotswap event with invalid capabilities. Dropping.");
                        break;
                } else if (!(e->resource.ResourceCapabilities
                             & SAHPI_CAPABILITY_FRU)) {
                        err("Invalid event. Resource in hotswap event "
                                "has no FRU capability. Dropping.");
                } else {
                        process_resource_event(d, e);
                }
                break;
        case SAHPI_ET_SENSOR:
        case SAHPI_ET_SENSOR_ENABLE_CHANGE:
        case SAHPI_ET_WATCHDOG:
        case SAHPI_ET_OEM:
	case SAHPI_ET_DOMAIN:
	case SAHPI_ET_USER:
        case SAHPI_ET_DIMI:
        case SAHPI_ET_DIMI_UPDATE:
        case SAHPI_ET_FUMI:
                process_hpi_event(d, e);
                break;
        default:
		err("Don't know what to do for event type  %d", e->event.EventType);
        }
        oh_detect_event_alarm(d, e);
        oh_release_domain(d);

        return 0;
}

SaErrorT oh_process_events()
{
        struct oh_event *e;
        // GArray *domain_results = NULL;
        SaHpiDomainIdT tmp_did;
        char *et;

        // domain_results = oh_query_domains();

        while ((e = g_async_queue_pop(oh_process_q.q)) != NULL) {
                et = oh_lookup_eventtype(e->event.EventType);
                dbg("Event Type = %s", (et) ? et : "<Unknown>");
                
                /* 1. Take care of special cases: user and domain type events */
                if (e->event.EventType == SAHPI_ET_DOMAIN) {
                        tmp_did = e->event.Source;
                        e->event.Source = SAHPI_UNSPECIFIED_RESOURCE_ID;
                        process_event(tmp_did, e);
                        goto free_event;
                } else if (e->event.EventType == SAHPI_ET_USER) {
                        tmp_did = e->resource.ResourceId;
                        e->resource.ResourceId = SAHPI_UNSPECIFIED_RESOURCE_ID;
                        process_event(tmp_did, e);
                        goto free_event;
                } /* TODO: Include HPI_SW event type in special case */
                
                /* All events get processed in the default domain regardless. */
                process_event(OH_DEFAULT_DOMAIN_ID, e);
                
free_event:
                oh_event_free(e, FALSE);
	}
        /* Should never get here */
        // g_array_free(domain_results, TRUE);
	return SA_ERR_HPI_INTERNAL_ERROR;
}

