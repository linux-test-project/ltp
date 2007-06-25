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


#include <rtas_eventlog.h>

SaErrorT rtas_get_el_info(void *hnd,
                             SaHpiResourceIdT id,
                             SaHpiEventLogInfoT *info)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_set_el_time(void *hnd,
                             SaHpiResourceIdT id,
                             SaHpiTimeT time)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_add_el_entry(void *hnd,
                              SaHpiResourceIdT id,
                              const SaHpiEventT *Event)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_get_el_entry(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiEventLogEntryIdT current,
                              SaHpiEventLogEntryIdT *prev,
                              SaHpiEventLogEntryIdT *next,
                              SaHpiEventLogEntryT *entry,
                              SaHpiRdrT  *rdr,
                              SaHpiRptEntryT  *rptentry)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_clear_el(void *hnd, SaHpiResourceIdT id)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_set_el_state(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiBoolT e)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_reset_el_overflow(void *hnd, SaHpiResourceIdT id)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_get_sel_info (void *hnd, 
                               SaHpiResourceIdT id, 
			       SaHpiEventLogInfoT *evtlog)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}			       

SaErrorT rtas_set_sel_time (void *hnd, 
                             SaHpiResourceIdT id, 
			     const SaHpiEventT *evt)
{
	return SA_ERR_HPI_INTERNAL_ERROR;
}	

SaErrorT rtas_add_sel_entry (void *hnd, 
                                SaHpiResourceIdT id, 
				const SaHpiEventT *evt)
{
	return SA_ERR_HPI_INTERNAL_ERROR;
}
		     
SaErrorT rtas_get_sel_entry (void *hnd, 
                                SaHpiResourceIdT id, 
				SaHpiEventLogEntryIdT current,
                                SaHpiEventLogEntryIdT *prev, 
				SaHpiEventLogEntryIdT *next,
                                SaHpiEventLogEntryT *entry, 
				SaHpiRdrT *rdr, 
				SaHpiRptEntryT  *rdtentry) 
{
	return SA_ERR_HPI_INTERNAL_ERROR;
}

void * oh_get_el_info (void *, SaHpiResourceIdT, SaHpiEventLogInfoT *)
        __attribute__ ((weak, alias("rtas_get_sel_info")));
void * oh_set_el_time (void *, SaHpiResourceIdT, const SaHpiEventT *)
        __attribute__ ((weak, alias("rtas_set_sel_time")));
void * oh_add_el_entry (void *, SaHpiResourceIdT, const SaHpiEventT *)
        __attribute__ ((weak, alias("rtas_add_sel_entry")));
void * oh_get_el_entry (void *, SaHpiResourceIdT, SaHpiEventLogEntryIdT,
                        SaHpiEventLogEntryIdT *, SaHpiEventLogEntryIdT *,
                        SaHpiEventLogEntryT *, SaHpiRdrT *, SaHpiRptEntryT  *)
        __attribute__ ((weak, alias("rtas_get_sel_entry")));
void * oh_clear_el (void *, SaHpiResourceIdT)
        __attribute__ ((weak, alias("rtas_clear_el")));
void * oh_set_el_state (void *, SaHpiResourceIdT)
        __attribute__ ((weak, alias("rtas_set_el_state")));
void * oh_reset_el_overflow (void *, SaHpiResourceIdT)
        __attribute__ ((weak, alias("rtas_reset_el_overflow")));

