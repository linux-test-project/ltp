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
 * Authors:
 *     Renier Morales <renier@openhpi.org>
 */
 
#include <stdlib.h>
#include <string.h>
#include <SaHpi.h>
#include <oHpi.h>

/**
 * Set OPENHPI_CONF to valid file. Open session. Get parameter
 * that was set through config file comparing with known value.
 * Pass on success, otherwise a failure.
 **/
 
int main(int argc, char **argv)
{
        SaHpiSessionIdT sid = 0;
        oHpiGlobalParamT onep_param = { .Type = OHPI_ON_EP };
                
	if (saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &sid, NULL)) {
		printf("Could not open session\n");
                return -1;
	}
                
        if (oHpiGlobalParamGet(&onep_param)) {
		printf("Could not get parameter\n");
                return -1;
	}
                
        if (onep_param.u.OnEP.Entry[0].EntityType != SAHPI_ENT_SYSTEM_CHASSIS ||
            onep_param.u.OnEP.Entry[0].EntityLocation != 1)
                return -1;
                
        return 0;
}
