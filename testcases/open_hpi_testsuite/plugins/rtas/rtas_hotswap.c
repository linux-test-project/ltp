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

#include <rtas_hotswap.h>
 
SaErrorT rtas_get_hotswap_state(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiHsStateT *state)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_set_hotswap_state(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiHsStateT state)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_request_hotswap_action(void *hnd,
                                        SaHpiResourceIdT id,
                                        SaHpiHsActionT act)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

void * oh_get_hotswap_state (void *, SaHpiResourceIdT, SaHpiHsStateT *)
        __attribute__ ((weak, alias("rtas_get_hotswap_state")));
void * oh_set_hotswap_state (void *, SaHpiResourceIdT, SaHpiHsStateT)
        __attribute__ ((weak, alias("rtas_set_hotswap_state")));
void * oh_request_hotswap_action (void *, SaHpiResourceIdT, SaHpiHsActionT)
        __attribute__ ((weak, alias("rtas_request_hotswap_action")));
