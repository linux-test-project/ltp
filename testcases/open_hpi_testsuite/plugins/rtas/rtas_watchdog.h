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

#ifndef RTAS_WATCHDOG_H
#define RTAS_WATCHDOG_H

#include <glib.h>
#include <SaHpi.h>
 
SaErrorT rtas_get_watchdog_info(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiWatchdogNumT num,
                                   SaHpiWatchdogT *wdt);
				   
SaErrorT rtas_set_watchdog_info(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiWatchdogNumT num,
                                   SaHpiWatchdogT *wdt);
				   
SaErrorT rtas_reset_watchdog(void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiWatchdogNumT num); 

#endif
