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
 * Set a paramter, get it, and compare values.
 * Tests without opening a session to see if the global
 * parameter table is initialized correctly anyway.
 * Pass on success, otherwise a failure.
 **/
 
int main(int argc, char **argv)
{
        oHpiGlobalParamT path_param = {
                .Type = OHPI_PATH,
                .u.Path = "/mylibdir"
        };
        
        /* Unset config file env variable */
        setenv("OPENHPI_CONF","./noconfig", 1);
                
        if (oHpiGlobalParamSet(&path_param))
                return -1;
                
        memset(path_param.u.Path, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
        if (oHpiGlobalParamGet(&path_param))
                return -1;
                
        if (strcmp("/mylibdir", path_param.u.Path))
                return -1;
                
        
        return 0;
}
