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

#include <sim_init.h>


SaErrorT sim_get_dimi_info(void *hnd,
			   SaHpiResourceIdT rid,
			   SaHpiDimiNumT num,
			   SaHpiDimiInfoT *info)
{
	return SA_ERR_HPI_INVALID_CMD;
}

void * oh_get_dimi_info (void *, SaHpiResourceIdT, SaHpiDimiNumT,
                         SaHpiDimiInfoT *)
                __attribute__ ((weak, alias("sim_get_dimi_info")));

