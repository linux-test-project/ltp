/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005,2006
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

#include <rtas_reset.h>

SaErrorT rtas_get_reset_state(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiResetActionT *act)
{
        return SA_ERR_HPI_CAPABILITY;
}

SaErrorT rtas_set_reset_state(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiResetActionT act)
{
        return SA_ERR_HPI_CAPABILITY;
}

void * oh_get_reset_state (void *, SaHpiResourceIdT, SaHpiResetActionT *)
        __attribute__ ((weak, alias("rtas_get_reset_state")));
void * oh_set_reset_state (void *, SaHpiResourceIdT, SaHpiResetActionT)
        __attribute__ ((weak, alias("rtas_set_reset_state")));
