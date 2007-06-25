/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004,2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *      David Ashley <dashley@us.ibm.com>
 *      Renier Morales <renier@openhpi.org>
 */

#ifndef __EL_UTILS_H
#define __EL_UTILS_H

#ifndef __OH_UTILS_H
#warning *** Include oh_utils.h instead of individual utility header files ***
#endif

#include <SaHpi.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OH_EL_MAX_SIZE 0

/* this struct encapsulates all the data for a system event log */
/* the log records themselves are stored in the el GList */
typedef struct {
        SaHpiTimeT basetime; // Time clock reference for this event log
	SaHpiTimeT sysbasetime; // The system time when the basetime was set
        SaHpiEventLogEntryIdT nextid; // id for next log entry
	SaHpiBoolT gentimestamp; // generate timestamp for entries.
	SaHpiEventLogInfoT info; /* Contains enabled state, overflow flag,
				      timestamp of last update,
				      and the max size for this log.
				    */
        GList *list; // list of event log entries for prev and next lookups
} oh_el;

/* this structure encapsulates the actual log entry and its context */
typedef struct {
        SaHpiEventLogEntryT event;
        SaHpiRdrT        rdr; // All 0's means no associated rdr
        SaHpiRptEntryT   res; // All 0's means no associated rpt
} oh_el_entry;

/* General EL utility calls */
oh_el *oh_el_create(SaHpiUint32T size);
SaErrorT oh_el_close(oh_el *el);
SaErrorT oh_el_append(oh_el *el,
		       const SaHpiEventT *event,
		       const SaHpiRdrT *rdr,
		       const SaHpiRptEntryT *res);
SaErrorT oh_el_prepend(oh_el *el,
		        const SaHpiEventT *event,
			const SaHpiRdrT *rdr,
			const SaHpiRptEntryT *res);
SaErrorT oh_el_clear(oh_el *el);
SaErrorT oh_el_get(oh_el *el,
		    SaHpiEventLogEntryIdT entryid,
		    SaHpiEventLogEntryIdT *prev,
                    SaHpiEventLogEntryIdT *next,
		    oh_el_entry **entry);
SaErrorT oh_el_info(oh_el *el, SaHpiEventLogInfoT *info);
SaErrorT oh_el_overflowreset(oh_el *el);
SaErrorT oh_el_overflowset(oh_el *el, SaHpiBoolT flag);
SaErrorT oh_el_map_to_file(oh_el *el, char *filename);
SaErrorT oh_el_map_from_file(oh_el *el, char *filename);
SaErrorT oh_el_timeset(oh_el *el, SaHpiTimeT timestamp);
SaErrorT oh_el_setgentimestampflag(oh_el *el, SaHpiBoolT flag);
SaErrorT oh_el_enableset(oh_el *el, SaHpiBoolT flag);

#ifdef __cplusplus
}
#endif

#endif
