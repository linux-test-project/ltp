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
 *      W. David Ashley <dashley@us.ibm.com>
 *	Renier Morales <renier@openhpi.org>
 *
 */


#ifndef __SIM_INJECTOR_H
#define __SIM_INJECTOR_H

#include <sim_resources.h>
#include <oh_event.h>

struct oh_handler_state *sim_get_handler_by_name(char *name);
SaErrorT sim_inject_resource(struct oh_handler_state *state,
                             struct sim_rpt *rpt_tmpl,
                             void *data,
                             struct oh_event **ohe);
SaErrorT sim_inject_rdr(struct oh_handler_state *state,
			struct oh_event *ohe,
                        SaHpiRdrT *rdr,
                        void *data);
SaErrorT sim_inject_event(struct oh_handler_state *state, struct oh_event *ohe);
SaErrorT sim_inject_ext_event(void *hnd,
			      SaHpiEventT *event,
			      SaHpiRptEntryT *rpte,
			      SaHpiRdrT *rdr);

#endif /*__SIM_INJECTOR_H*/
