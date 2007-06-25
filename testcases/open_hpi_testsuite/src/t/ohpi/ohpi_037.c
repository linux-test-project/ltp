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
 * Pass null arguments to oHpiGlobalParamGet
 * Pass on error, otherwise test failed.
 **/
 
int main(int argc, char **argv)
{
        SaHpiSessionIdT sid = 0;
        
        /* Set config file env variable */
        setenv("OPENHPI_CONF","./openhpi.conf", 1);
        
        if (saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &sid, NULL))
                return -1;
                
        if (!oHpiGlobalParamGet(NULL))
                return -1;
                
        
        return 0;
}
