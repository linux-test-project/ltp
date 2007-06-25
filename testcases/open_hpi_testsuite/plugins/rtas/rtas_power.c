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

#include <rtas_power.h>
#include <librtas.h>

SaErrorT rtas_get_power_state(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiPowerStateT *state)
{
        *state = SAHPI_POWER_ON;

        return SA_OK;
}

SaErrorT rtas_set_power_state(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiPowerStateT state)
{
        int setlevel, rc;

        if (state == SAHPI_POWER_OFF) {
                rc = rtas_set_power_level(0, 0, &setlevel);
                if (rc > -1 && setlevel == 0) return SA_OK;
                else if (rc > -1 && setlevel != 0) return SA_ERR_HPI_BUSY;
                else return SA_ERR_HPI_INTERNAL_ERROR;
        } else {
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
}

void * oh_get_power_state (void *, SaHpiResourceIdT, SaHpiPowerStateT *)
        __attribute__ ((weak, alias("rtas_get_power_state")));
void * oh_set_power_state (void *, SaHpiResourceIdT, SaHpiPowerStateT)
        __attribute__ ((weak, alias("rtas_set_power_state")));
