/*
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
 *                     All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the Hewlett-Packard Corporation, nor the names
 * of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author(s)
 *     Shuah Khan <shuah.khan@hp.com>
 *     Richard White <richard.white@hp.com>
 */

/***************
 * This source file contains Resource Presence Table (RPT) HPI ABI routines
 * iLO2 RIBCL plug-in implements. Other source files provide support 
 * functionality for these ABIs.
***************/

#include <ilo2_ribcl.h>
#include <ilo2_ribcl_ssl.h>
#include <ilo2_ribcl_xml.h>

/*****************************
	iLO2 RIBCL plug-in Resource Presence Table (RPT) ABI Interface
	functions.
*****************************/

/**
 * ilo2_ribcl_set_resource_severity:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @tag: Resource's severity.
 *
 * Sets severity of events when resource unexpectedly becomes unavailable.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - @sev is invalid.
 * SA_ERR_HPI_OUT_OF_MEMORY - No memory to allocate event.
 **/
SaErrorT ilo2_ribcl_set_resource_severity(void *hnd, SaHpiResourceIdT rid,
	SaHpiSeverityT sev)
{
        SaHpiRptEntryT *rpt;
        struct oh_handler_state *handle;
	ilo2_ribcl_handler_t *ilo2_ribcl_handler = NULL;
	ilo2_ribcl_resource_info_t *res_info = NULL;
        struct oh_event *e;

	if (oh_lookup_severity(sev) == NULL) {
		err("ilo2_ribcl_set_resource_severity(): Invalid parameter");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	handle = (struct oh_handler_state *)hnd;
	ilo2_ribcl_handler = (ilo2_ribcl_handler_t *)handle->data;
	
	if (!ilo2_ribcl_handler) {
		err("ilo2_ribcl_set_resource_severity(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
                err("ilo2_ribcl_set_resource_severity(): No RID.");
                return(SA_ERR_HPI_INVALID_RESOURCE);
        }

        rpt->ResourceSeverity = sev;

	res_info =  (ilo2_ribcl_resource_info_t *)oh_get_resource_data(
		handle->rptcache, rpt->ResourceId);
	if (!res_info) {
		err("ilo2_ribcl_set_resource_severity(): no resource info.");
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}

        /* Add changed resource to event queue */
        e = oh_new_event();
	if (e == NULL) {
		err("ilo2_ribcl_set_resource_severity(): Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
			
        e->resource = *rpt;
	
	/* Construct .event of struct oh_event      */	
	e->event.Severity = e->resource.ResourceSeverity;
	e->event.Source =   e->resource.ResourceId;
	if (oh_gettimeofday(&e->event.Timestamp) != SA_OK) {
		e->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
	}
	if (e->resource.ResourceCapabilities & SAHPI_CAPABILITY_FRU) {
		e->event.EventType = SAHPI_ET_HOTSWAP;
		e->event.EventDataUnion.HotSwapEvent.HotSwapState = e->event.EventDataUnion.HotSwapEvent.HotSwapState = res_info->fru_cur_state;
	} else {
		e->event.EventType = SAHPI_ET_RESOURCE;
		e->event.EventDataUnion.ResourceEvent.ResourceEventType =
			SAHPI_RESE_RESOURCE_ADDED;
	}

	/* Prime event to evenq                     */
        e->hid = handle->hid;
        oh_evt_queue_push(handle->eventq, e);

        return(SA_OK);
}

/**
 * ilo2_ribcl_set_resource_tag:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @tag: Pointer to SaHpiTextBufferT.
 *
 * Sets resource's tag.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_INVALID_PARAMS - @tag is NULL or invalid.
 * SA_ERR_HPI_OUT_OF_MEMORY - No memory to allocate event.
 **/
SaErrorT ilo2_ribcl_set_resource_tag(void *hnd, SaHpiResourceIdT rid, SaHpiTextBufferT *tag)
{
		
	SaErrorT err;
        SaHpiRptEntryT *rpt;
        struct oh_event *e;
	ilo2_ribcl_handler_t *ilo2_ribcl_handler = NULL;
	ilo2_ribcl_resource_info_t *res_info = NULL;
        struct oh_handler_state *handle;

	if (!oh_valid_textbuffer(tag) || !hnd) {
		err("ilo2_ribcl_set_resource_tag((): Invalid parameter");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	handle = (struct oh_handler_state *)hnd;
	ilo2_ribcl_handler = (ilo2_ribcl_handler_t *)handle->data;
	
	if (!ilo2_ribcl_handler) {
		err("ilo2_ribcl_set_resource_tag(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		err("ilo2_ribcl_set_resource_tag(): No RID.");
                return(SA_ERR_HPI_INVALID_RESOURCE);
        }

	err = oh_copy_textbuffer(&(rpt->ResourceTag), tag);
	if (err) {
		err("ilo2_ribcl_set_resource_tag(): Cannot copy textbuffer");
		return(err);
	}

	res_info =  (ilo2_ribcl_resource_info_t *)oh_get_resource_data(
		handle->rptcache, rpt->ResourceId);
	if (!res_info) {
		err("ilo2_ribcl_set_resource_severity(): no resource info.");
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}

        /* Add changed resource to event queue */
        e = oh_new_event();
	if (e == NULL) {
		err("ilo2_ribcl_set_resource_tag(): Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
			
        e->resource = *rpt;
	
	/* Construct .event of struct oh_event      */	
	e->event.Severity = e->resource.ResourceSeverity;
	e->event.Source =   e->resource.ResourceId;
	if (oh_gettimeofday(&e->event.Timestamp) != SA_OK) {
		e->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
	}
	if (e->resource.ResourceCapabilities & SAHPI_CAPABILITY_FRU) {
		e->event.EventType = SAHPI_ET_HOTSWAP;
		e->event.EventDataUnion.HotSwapEvent.HotSwapState =
		e->event.EventDataUnion.HotSwapEvent.HotSwapState =
				res_info->fru_cur_state;
	} else {
		e->event.EventType = SAHPI_ET_RESOURCE;
		e->event.EventDataUnion.ResourceEvent.ResourceEventType =
			SAHPI_RESE_RESOURCE_ADDED;
	}

	/* Prime event to evenq                     */
        e->hid = handle->hid;
        oh_evt_queue_push(handle->eventq, e);

        return(SA_OK);
}

/*****************************
	OpenHPI plug-in to iLO2 RIBCL plug-in ABI function mapping
*****************************/
void *oh_set_resource_tag (void *, SaHpiResourceIdT, SaHpiTextBufferT *) 
                __attribute__ ((weak, alias("ilo2_ribcl_set_resource_tag")));
		
void *oh_set_resource_severity (void *, SaHpiResourceIdT, SaHpiSeverityT) 
                __attribute__ ((weak, alias("ilo2_ribcl_set_resource_severity")));
