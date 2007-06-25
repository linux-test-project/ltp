/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004-2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Sean Dague <sdague@users.sf.net>
 *	Renier Morales <renier@openhpi.org>
 *
 */

#ifndef __OH_EVENT_H
#define __OH_EVENT_H

#include <SaHpi.h>
#include <glib.h>

#define OH_MAX_EVT_QUEUE_LIMIT 0

#ifdef __cplusplus
extern "C" {
#endif

/***
 * Instructions for using oh_event
 *********************************
 * oh_event is primarily used by the plugins to report HPI events.
 *
 * Required Fields:
 * .hid, .event
 *
 * Optional Fields:
 * .resource, .rdrs
 *
 * FRU Resource oh_events:
 * If reporting a resource, the plugin sets the appropiate event in .event.
 * For example, if reporting a new FRU resource, then the event should be
 * a hotswap type showing the correct hotswap state (any except NOT_PRESENT)
 * indicating the library that it should add it to the RPT.
 * If its just updating the FRU RPT entry, then it should come with a hotswap
 * event with appropiate transition (current and previous hotswap states equal
 * or as appropiate). The library will update the RPT entry accordingly.
 * If the plugin needs to report an extracted FRU, then the hotswap
 * event has to show the NOT_PRESENT current state, indicating to the library
 * that it should remove it from the RPT.
 * Hotswap events must have their accompaining resource set its capability bit
 * for FRU to 1 or it will be dropped by the infrastructure.
 * The .resource.ResourceId field can be zero. If so, the RPT will not be
 * updated, but the SaHpiEventT will be passed on to the session queues and
 * domain event log normally.
 *
 * Non-FRU Resource oh_events:
 * For adding or updating Non-FRU resources, the .event should be a resource
 * type HPI event and the ResourceEventType should be RESOURCE_ADDED or
 * RESOURCE_RESTORED. The resource itself should have its capability bit for
 * FRU set to zero or the event will be dropped by the infrastructure.
 * Removing Non-FRU resource from the RPT is not supported anymore as this is
 * not spec compliant. The Non-FRU resource are always there, but they are
 * either working or failed. If a resource is failed, then the oh_event should
 * have a resource event type with the resource state as RESOURCE_FAILED.
 * The .resource field should have the resource in question. This is used by
 * the infrastructure to update the RPT and mark the resource as failed
 * (ResourceFailed == True). The .resource.ResourceId field can be zero. If so,
 * the RPT will not be updated, but the SaHpiEventT will be passed on to the
 * session queues and domain event log normally.
 *
 * RDRs:
 * If the event is for a resource, be it FRU or Non-FRU, and the resource did
 * not previously exist in the RPT for the domain, then the .rdrs field is
 * scanned for valid SaHpiRdrTs (RdrType != SAHPI_NO_RECORD) objects and each
 * one is added as an rdr for the resource to the RPT. If the resource is
 * already in the RPT, then the rdrs field will be ignored.
 * This is to avoid changes to the RDR repository of a resource once the
 * resource has already been added as this is not spec compliant.
 *
 * Other event types:
 * If the event is of type SENSOR, SENSOR_ENABLE_CHANGE, WATCHDOG, or OEM, then
 * The .resource field is scanned for a valid resource to use as reference for
 * the domain event log. Also, the .rdrs field is scanned for exactly one
 * SaHpiRdrT to be used as reference for the domain event log and session event
 * queue. If multiple rdrs are passed for these event types, only the first one
 * will be used.
 **/

struct oh_event {
        unsigned int hid; /* handler id for the event */
        SaHpiEventT event;
        /* If no resource, ResourceCapabilities must be 0 */
        SaHpiRptEntryT resource;
        GSList *rdrs;
};

typedef struct _oh_evt_queue oh_evt_queue;
extern oh_evt_queue oh_process_q;

/* Event utility macros */
#define oh_new_event() g_new0(struct oh_event, 1)
#define oh_copy_event(dest, src) memcpy(dest, src, sizeof(struct oh_event))
#define sahpi_new_event() g_new0(SaHpiEventT, 1)
#define sahpi_dup_event(old) g_memdup(old, sizeof(SaHpiEventT))
#define sahpi_copy_event(dest, src) memcpy(dest, src, sizeof(SaHpiEventT))

/* function definitions */
int oh_event_init(void);
void oh_evt_queue_push(oh_evt_queue *equeue, gpointer data);
SaErrorT oh_harvest_events(void);
SaErrorT oh_process_events(void);
void oh_event_free(struct oh_event *e, int only_rdrs);
struct oh_event *oh_dup_event(struct oh_event *old_event);

#ifdef __cplusplus
}
#endif

#endif /* __OH_EVENT_H */

