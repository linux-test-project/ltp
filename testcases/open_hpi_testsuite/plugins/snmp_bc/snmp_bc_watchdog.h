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
 *      Steve Sherman <sesherman@us.ibm.com>
 */

#ifndef __SNMP_BC_WATCHDOG_H
#define __SNMP_BC_WATCHDOG_H

SaErrorT snmp_bc_get_watchdog_info(void *hnd,
				   SaHpiResourceIdT id,
				   SaHpiWatchdogNumT num,
				   SaHpiWatchdogT *wdt);

SaErrorT snmp_bc_set_watchdog_info(void *hnd,
				   SaHpiResourceIdT id,
				   SaHpiWatchdogNumT num,
				   SaHpiWatchdogT *wdt);

SaErrorT snmp_bc_reset_watchdog(void *hnd,
				SaHpiResourceIdT id,
				SaHpiWatchdogNumT num);

#endif
