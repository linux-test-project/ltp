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
 * Load 'libsimulator' and 'libwatchdog', create two handlers on each.
 * Pass on success, otherwise failure.
 **/
 
#define PLUGIN_NAME_SIZE 32
 
int main(int argc, char **argv)
{
        SaHpiSessionIdT sid = 0;
        oHpiHandlerIdT hid0, hid1, hid2, hid3;
        GHashTable *h0 = g_hash_table_new(g_str_hash, g_str_equal),
                   *h1 = g_hash_table_new(g_str_hash, g_str_equal),
                   *h2 = g_hash_table_new(g_str_hash, g_str_equal),
                   *h3 = g_hash_table_new(g_str_hash, g_str_equal);
        
        setenv("OPENHPI_CONF","./noconfig", 1);
        
        if (saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &sid, NULL))
                return -1;
                    
        /* Set configuration for handlers and create them. */
        g_hash_table_insert(h0, "plugin", "libsimulator");
        g_hash_table_insert(h0, "entity_root", "{SYSTEM_CHASSIS,1}");
        g_hash_table_insert(h0, "name", "test0");
        g_hash_table_insert(h0, "addr", "0");
        
        g_hash_table_insert(h1, "plugin", "libsimulator");
        g_hash_table_insert(h1, "entity_root", "{SYSTEM_CHASSIS,2}");
        g_hash_table_insert(h1, "name", "test1");
        g_hash_table_insert(h1, "addr", "1");
        
        /* Set configuration for two handlers and create them. */
        g_hash_table_insert(h2, "plugin", "libwatchdog");
        g_hash_table_insert(h2, "entity_root", "{SYSTEM_CHASSIS,3}");
        g_hash_table_insert(h2, "addr", "0");
        
        g_hash_table_insert(h3, "plugin", "libwatchdog");
        g_hash_table_insert(h3, "entity_root", "{SYSTEM_CHASSIS,4}");
        g_hash_table_insert(h3, "addr", "1");
        
        if (oHpiHandlerCreate(h0,&hid0) || oHpiHandlerCreate(h1,&hid1))
                return -1;
                
        if (oHpiHandlerCreate(h2,&hid2) || oHpiHandlerCreate(h3,&hid3))
                return -1;
                
        
        return 0;
}
