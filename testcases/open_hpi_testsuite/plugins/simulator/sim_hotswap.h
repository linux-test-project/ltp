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
 *      W. David Ashley <dashley@us.ibm.com>
 */

#ifndef __SIM_HOTSWAP_H
#define __SIM_HOTSWAP_H

SaErrorT sim_get_hotswap_state(void *hnd,
			       SaHpiResourceIdT rid,
			       SaHpiHsStateT *state);

SaErrorT sim_set_hotswap_state(void *hnd,
			       SaHpiResourceIdT rid,
			       SaHpiHsStateT state);

SaErrorT sim_request_hotswap_action(void *hnd,
				    SaHpiResourceIdT rid,
				    SaHpiHsActionT act);

SaErrorT sim_get_indicator_state(void *hnd,
	         		 SaHpiResourceIdT rid,
				 SaHpiHsIndicatorStateT *state);

SaErrorT sim_set_indicator_state(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiHsIndicatorStateT state);

SaErrorT sim_get_autoextract_timeout(void *hnd,
				     SaHpiResourceIdT rid,
				     SaHpiTimeoutT *timeout);

SaErrorT sim_set_autoextract_timeout(void *hnd,
				     SaHpiResourceIdT rid,
				     SaHpiTimeoutT timeout);

#endif
