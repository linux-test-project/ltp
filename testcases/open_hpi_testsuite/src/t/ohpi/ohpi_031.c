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
 * Try to get handler info passing a bogus handler id.
 * Pass on success, otherwise failure.
 **/
 
int main(int argc, char **argv)
{
        SaHpiSessionIdT sid = 0;
        GHashTable *config = g_hash_table_new(g_str_hash, g_str_equal);
        oHpiHandlerIdT hid = 0;
        oHpiHandlerInfoT hinfo;
        
        setenv("OPENHPI_CONF","./noconfig", 1);
        
        if (saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &sid, NULL))
                return -1;

        /* Set configuration. */
        g_hash_table_insert(config, "plugin", "libsimulator");
        g_hash_table_insert(config, "entity_root", "{SYSTEM_CHASSIS,1}");
        g_hash_table_insert(config, "name", "test");
        g_hash_table_insert(config, "addr", "0");
        
        if (oHpiHandlerCreate(config, &hid))
                return -1;
                
        if (!oHpiHandlerInfo(555, &hinfo))
                return -1;
                        
        
        return 0;
}
