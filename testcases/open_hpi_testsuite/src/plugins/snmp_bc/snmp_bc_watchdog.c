/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 */

#include <SaHpi.h>

#include <snmp_bc_watchdog.h>

SaErrorT snmp_bc_get_watchdog_info(void *hnd,
				   SaHpiResourceIdT id,
				   SaHpiWatchdogNumT num,
				   SaHpiWatchdogT *wdt)
{
	/* Watchdog not supported */
        return SA_ERR_HPI_NOT_PRESENT;
}

SaErrorT snmp_bc_set_watchdog_info(void *hnd,
				   SaHpiResourceIdT id,
				   SaHpiWatchdogNumT num,
				   SaHpiWatchdogT *wdt)
{
	/* Watchdog not supported */
        return SA_ERR_HPI_NOT_PRESENT;
}

SaErrorT snmp_bc_reset_watchdog(void *hnd,
				SaHpiResourceIdT id,
				SaHpiWatchdogNumT num)
{
 	/* Watchdog not supported */
        return SA_ERR_HPI_NOT_PRESENT;
}
