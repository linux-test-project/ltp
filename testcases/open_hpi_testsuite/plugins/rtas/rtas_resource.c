/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *        Renier Morales <renier@openhpi.org>
 *        Daniel de Araujo <ddearauj@us.ibm.com>
 */

#include <rtas_resource.h>
#include <oh_handler.h>
#include <oh_event.h>
#include <oh_domain.h>
#include <oh_utils.h>

SaErrorT rtas_set_resource_tag(void *hnd,
                               SaHpiResourceIdT id,
                               SaHpiTextBufferT *tag)
{
        struct oh_handler_state *h = hnd;
        SaHpiRptEntryT *rptentry = NULL;
        SaErrorT error = SA_OK;
        struct oh_event *e = NULL;

        if (!hnd) return SA_ERR_HPI_INTERNAL_ERROR;

        rptentry = oh_get_resource_by_id(h->rptcache, id);
        if (!rptentry) return SA_ERR_HPI_NOT_PRESENT;

        error = oh_append_textbuffer(&rptentry->ResourceTag, (char *)tag->Data);
        if (error) return error;

        e = (struct oh_event *)g_malloc0(sizeof(struct oh_event));
        e->hid = h->hid;
        e->event.EventType = SAHPI_ET_RESOURCE;
        e->event.Source = rptentry->ResourceId;
        e->event.Severity = rptentry->ResourceSeverity;
        e->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
        e->event.EventDataUnion.ResourceEvent.ResourceEventType = SAHPI_RESE_RESOURCE_ADDED;
        e->resource = *rptentry;
        oh_evt_queue_push(h->eventq, e);

        return SA_OK;
}

SaErrorT rtas_set_resource_severity(void *hnd,
                                    SaHpiResourceIdT id,
                                    SaHpiSeverityT sev)
{
        struct oh_handler_state *h = hnd;
        SaHpiRptEntryT *rptentry = NULL;
        struct oh_event *e = NULL;

        if (!hnd) return SA_ERR_HPI_INTERNAL_ERROR;

        rptentry = oh_get_resource_by_id(h->rptcache, id);
        if (!rptentry) return SA_ERR_HPI_NOT_PRESENT;

        rptentry->ResourceSeverity = sev;

        e = (struct oh_event *)g_malloc0(sizeof(struct oh_event));
        e->event.EventType = SAHPI_ET_RESOURCE;
        e->event.Source = rptentry->ResourceId;
        e->event.Severity = rptentry->ResourceSeverity;
        e->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
        e->event.EventDataUnion.ResourceEvent.ResourceEventType = SAHPI_RESE_RESOURCE_ADDED;
        e->resource = *rptentry;
        oh_evt_queue_push(h->eventq, e);

        return SA_OK;
}


void * oh_set_resource_tag (void *, SaHpiResourceIdT, SaHpiTextBufferT *)
        __attribute__ ((weak, alias("rtas_set_resource_tag")));
void * oh_set_resource_severity (void *, SaHpiResourceIdT, SaHpiSeverityT)
        __attribute__ ((weak, alias("rtas_set_resource_severity")));
