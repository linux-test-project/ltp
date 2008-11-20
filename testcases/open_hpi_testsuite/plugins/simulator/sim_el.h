/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005,2006, 2007, 2008
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
 *      Suntrupth S Yadav <suntrupth@in.ibm.com> 
 */


SaErrorT sim_el_get_info(void *hnd, SaHpiResourceIdT id,
                         SaHpiEventLogInfoT *info);
SaErrorT sim_el_set_state(void *hnd, SaHpiResourceIdT id, SaHpiBoolT state);
SaErrorT sim_el_get_state(void *hnd, SaHpiResourceIdT id, SaHpiBoolT *state);
SaErrorT sim_el_set_time(void *hnd, SaHpiResourceIdT id, SaHpiTimeT time);
SaErrorT sim_el_add_entry(void *hnd, SaHpiResourceIdT id,
                          const SaHpiEventT *Event);
SaErrorT sim_el_get_entry(void *hnd, SaHpiResourceIdT id,
		          SaHpiEventLogEntryIdT current,
		          SaHpiEventLogEntryIdT *prev,
		          SaHpiEventLogEntryIdT *next,
		          SaHpiEventLogEntryT *entry, SaHpiRdrT  *rdr,
                          SaHpiRptEntryT *rptentry);
SaErrorT sim_el_clear(void *hnd, SaHpiResourceIdT id);
SaErrorT sim_el_overflow(void *hnd, SaHpiResourceIdT id);
SaErrorT sim_el_get_caps(void *hnd, SaHpiResourceIdT id,
                         SaHpiEventLogCapabilitiesT *caps);

