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

#ifndef __SIM_RESET_H
#define __SIM_RESET_H

SaErrorT sim_get_reset_state(void *hnd,
			     SaHpiResourceIdT rid,
			     SaHpiResetActionT *act);

SaErrorT sim_set_reset_state(void *hnd,
			     SaHpiResourceIdT rid,
			     SaHpiResetActionT act);
#endif
