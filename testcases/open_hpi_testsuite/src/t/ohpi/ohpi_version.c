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
#include <stdio.h>
#include <string.h>
#include <SaHpi.h>
#include <oHpi.h>
#include <config.h>

/**
 * Load the simulator plugin.
 * Pass on success, otherwise a failure.
 **/
 
int main(int argc, char **argv)
{
    SaHpiUint64T v = 0;

	char * buf = g_malloc(100);
        char * version = VERSION ".0";	
        
        setenv("OPENHPI_CONF","./noconfig", 1);

        v = oHpiVersionGet();

        snprintf(buf,100,"%llu.%llu.%llu.%llu", 
               (v >> 48), 
               ((v >> 32) & 0x0000ffff), // my gcc barfed unless it did this 
               ((v & 0x00000000ffffffff) >> 16),
               (v & 0x000000000000ffff));
	if(strcmp(buf,version) == 0) {
        	return 0;
	}
	return -1;
}
