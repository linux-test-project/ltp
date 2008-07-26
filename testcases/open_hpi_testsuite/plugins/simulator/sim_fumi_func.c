/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2008
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 *  Authors:
 *  Suntrupth S Yadav <suntrupth@in.ibm.com>
 */

#include <sim_init.h>

SaErrorT sim_set_fumi_source(void *hnd,
	        	   SaHpiResourceIdT rid,
			   SaHpiFumiNumT fnum,
			   SaHpiBankNumT bnum,
               SaHpiTextBufferT *sourceuri)
{
	return SA_ERR_HPI_INVALID_CMD;
}
void* oh_set_fumi_source (void *, SaHpiResourceIdT,SaHpiFumiNumT,
                         SaHpiBankNumT, SaHpiTextBufferT *)
                __attribute__ ((weak, alias("sim_set_fumi_source")));
