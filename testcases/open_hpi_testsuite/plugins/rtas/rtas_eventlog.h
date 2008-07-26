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
 
#ifndef RTAS_EVENTLOG_H
#define RTAS_EVENTLOG_H

#include <glib.h>
#include <SaHpi.h>
 
SaErrorT rtas_get_el_info(void *hnd,
                             SaHpiResourceIdT id,
                             SaHpiEventLogInfoT *info);
			     
SaErrorT rtas_set_el_time(void *hnd,
                             SaHpiResourceIdT id,
                             SaHpiTimeT time);
			     
SaErrorT rtas_add_el_entry(void *hnd,
                              SaHpiResourceIdT id,
                              const SaHpiEventT *Event);
			      
SaErrorT rtas_get_el_entry(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiEventLogEntryIdT current,
                              SaHpiEventLogEntryIdT *prev,
                              SaHpiEventLogEntryIdT *next,
                              SaHpiEventLogEntryT *entry,
                              SaHpiRdrT  *rdr,
                              SaHpiRptEntryT  *rptentry);
			      
SaErrorT rtas_clear_el(void *hnd, SaHpiResourceIdT id);

SaErrorT rtas_set_el_state(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiBoolT e);
			      
SaErrorT rtas_reset_el_overflow(void *hnd, SaHpiResourceIdT id); 

SaErrorT rtas_get_sel_info (void *hnd, 
                               SaHpiResourceIdT id, 
			       SaHpiEventLogInfoT *evtlog);
			       
SaErrorT rtas_set_sel_time (void *hnd, 
                             SaHpiResourceIdT id, 
			     const SaHpiEventT *evt);
			     
SaErrorT rtas_add_sel_entry (void *hnd, 
                                SaHpiResourceIdT id, 
				const SaHpiEventT *evt);
				
SaErrorT rtas_get_sel_entry (void *hnd, 
                                SaHpiResourceIdT id, 
				SaHpiEventLogEntryIdT current,
                                SaHpiEventLogEntryIdT *prev, 
				SaHpiEventLogEntryIdT *next,
                                SaHpiEventLogEntryT *entry, 
				SaHpiRdrT *rdr, 
				SaHpiRptEntryT  *rdtentry);

#endif
