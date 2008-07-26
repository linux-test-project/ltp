/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2007
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
 */

#ifndef __SIM_DIMI_FUNC_H
#define __SIM_DIMI_FUNC_H

SaErrorT sim_get_dimi_info(void *hnd,
	        	   SaHpiResourceIdT rid,
			   SaHpiDimiNumT num,
			   SaHpiDimiInfoT *info);

#endif
