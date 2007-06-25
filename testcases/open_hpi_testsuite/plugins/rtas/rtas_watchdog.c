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

#include <rtas_watchdog.h>

SaErrorT rtas_get_watchdog_info(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiWatchdogNumT num,
                                   SaHpiWatchdogT *wdt)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_set_watchdog_info(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiWatchdogNumT num,
                                   SaHpiWatchdogT *wdt)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_reset_watchdog(void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiWatchdogNumT num)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

void * oh_get_watchdog_info (void *, SaHpiResourceIdT, SaHpiWatchdogNumT,
                             SaHpiWatchdogT *)
        __attribute__ ((weak, alias("rtas_get_watchdog_info")));
void * oh_set_watchdog_info (void *, SaHpiResourceIdT, SaHpiWatchdogNumT,
                             SaHpiWatchdogT *)
        __attribute__ ((weak, alias("rtas_set_watchdog_info")));
void * oh_reset_watchdog (void *, SaHpiResourceIdT, SaHpiWatchdogNumT)
        __attribute__ ((weak, alias("rtas_reset_watchdog")));
