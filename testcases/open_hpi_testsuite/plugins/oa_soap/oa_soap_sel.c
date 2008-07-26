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
 *      Raghavendra M.S. <raghavendra.ms@hp.com>
 **/

#include "oa_soap_sel.h"

/**
 * oa_soap_get_sel_info:
 *      @oh_handler:  Handler data pointer.
 *      @resource_id: Resource ID.
 *      @info:        Sensor rdr number.
 *
 * Purpose:
 *      Gets current number of entries in the event log.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa soap implementation
 *                                      does not support this API.
 **/
SaErrorT oa_soap_get_sel_info(void *oh_handler,
                            SaHpiResourceIdT resource_id,
                            SaHpiEventLogInfoT *info)
{
        err("Get Event Log info is not supported");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_set_sel_time:
 *      @oh_handler:  Handler data pointer.
 *      @resource_id: Resource ID.
 *      @time:        Time to be set for event log.
 *
 * Purpose:
 *      Sets the event log's clock.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa soap implementation
 *                                      does not support this API.
 **/
SaErrorT oa_soap_set_sel_time(void *oh_handler,
                            SaHpiResourceIdT resource_id,
                            SaHpiTimeT time)
{
        err("Set Event log time is not supported");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_add_sel_entry.
 *      @oh_handler:  Handler data pointer.
 *      @resource_id: Resource ID.
 *      @Event:       Event entry.
 *
 * Purpose:
 *      Adds the event entries to event log.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa soap implementation
 *                                      does not support this API.
 **/
SaErrorT oa_soap_add_sel_entry(void *oh_handler,
                             SaHpiResourceIdT resource_id,
                             const SaHpiEventT *Event)
{
        err("Adding entries to Event log is not supported");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_clear_sel.
 *      @oh_handler:  Handler data pointer.
 *      @resource_id: Resource ID.
 *
 * Purpose:
 *      Clears all entries from Event log.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa soap implementation
 *                                      does not support this API.
 **/
SaErrorT oa_soap_clear_sel(void *oh_handler,
                         SaHpiResourceIdT resource_id)
{
        err("Clearing entries from Event log is not supported");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_reset_sel_overflow.
 *      @oh_handler:  Handler data pointer.
 *      @resource_id: Resource ID.
 *
 * Purpose:
 *      Resets the overflow flag in the event log.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa soap implementation
 *                                      does not support this API.
 **/
SaErrorT oa_soap_reset_sel_overflow(void *oh_handler,
                                  SaHpiResourceIdT resource_id)
{
        err("Reset overflow of Event log is not supported");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_get_sel_entry:
 *      @oh_handler:  Handler data pointer.
 *      @resource_id: Resource ID.
 *      @current:     Current event's ID.
 *      @prev:        Location to store previous event's ID.
 *      @next:        Location to store next event's ID.
 *      @entry:       Location to store retrieved event.
 *      @rdr:         Rdr structure.
 *      @rpt:         Rpt entry.
 *
 * Purpose:
 *      Gets the event log entry.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa soap implementation
 *                                      does not support this API.
 **/
SaErrorT oa_soap_get_sel_entry(void *oh_handler,
                               SaHpiResourceIdT resource_id,
                               SaHpiEventLogEntryIdT current,
                               SaHpiEventLogEntryIdT *prev,
                               SaHpiEventLogEntryIdT *next,
                               SaHpiEventLogEntryT *entry,
                               SaHpiRdrT *rdr,
                               SaHpiRptEntryT  *rpt)
{
        err("Get Event log entry is not supported");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_sel_set_state:
 *      @oh_handler: Handler data pointer.
 *      @id:         Resource ID.
 *      @enable:     SEL state.
 *
 * Purpose:
 *      Sets the event log state to enabled or disabled.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa soap implementation
 *                                      does not support this API.
 **/
SaErrorT oa_soap_sel_state_set(void *oh_handler,
                              SaHpiResourceIdT resource_id,
                              SaHpiBoolT enable)
{
        err("Set Event log state is not supported");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

void * oh_get_el_info (void *,
                       SaHpiResourceIdT,
                       SaHpiEventLogInfoT *)
                __attribute__ ((weak, alias("oa_soap_get_sel_info")));

void * oh_set_el_time (void *,
                       SaHpiResourceIdT,
                       const SaHpiEventT *)
                __attribute__ ((weak, alias("oa_soap_set_sel_time")));

void * oh_add_el_entry (void *,
                        SaHpiResourceIdT,
                        const SaHpiEventT *)
                __attribute__ ((weak, alias("oa_soap_add_sel_entry")));

void * oh_get_el_entry (void *,
                        SaHpiResourceIdT,
                        SaHpiEventLogEntryIdT,
                        SaHpiEventLogEntryIdT *,
                        SaHpiEventLogEntryIdT *,
                        SaHpiEventLogEntryT *,
                        SaHpiRdrT *,
                        SaHpiRptEntryT  *)
                __attribute__ ((weak, alias("oa_soap_get_sel_entry")));

void * oh_clear_el (void *,
                    SaHpiResourceIdT)
                __attribute__ ((weak, alias("oa_soap_clear_sel")));

void * oh_reset_el_overflow (void *,
                             SaHpiResourceIdT)
                __attribute__ ((weak, alias("oa_soap_reset_sel_overflow")));

void * oh_set_el_state(void *, SaHpiResourceIdT, SaHpiBoolT)
                __attribute__ ((weak, alias("oa_soap_sel_state_set")));

