/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *      Steve Sherman  <stevees@us.ibm.com>
 */

#include <glib.h>

#include <openhpi.h>
#include <snmp_util.h>
#include <snmp_bc_session.h>
#include <snmp_bc.h>
#include <rpt_utils.h>
#include <sim_resources.h>

static int sim_init(void);
static int sim_close(void);
static void free_hash_data(gpointer key, gpointer value, gpointer user_data);

/**
 * snmp_bc_open: open snmp blade center plugin
 * @handler_config: hash table passed by infrastructure
 **/

void *snmp_bc_open(GHashTable *handler_config)
{
        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
        char *root_tuple;

        root_tuple = (char *)g_hash_table_lookup(handler_config, "entity_root");
        if(!root_tuple) {
                dbg("ERROR: Cannot open snmp_bc plugin. No entity root found in configuration.");
                return NULL;
        }
        
        handle = (struct oh_handler_state *)g_malloc0(sizeof(struct oh_handler_state));
        custom_handle =
                (struct snmp_bc_hnd *)g_malloc0(sizeof(struct snmp_bc_hnd));
        if(!handle || !custom_handle) {
                dbg("Could not allocate memory for handle or custom_handle.");
                return NULL;
        }
        handle->data = custom_handle;
        
        handle->config = handler_config;

        /* Initialize RPT cache */
        handle->rptcache = (RPTable *)g_malloc0(sizeof(RPTable));
        
        custom_handle->ss = NULL;
        
        /* this initializes the simulator tables */
        sim_init();

        return handle;
}

/**
 * snmp_bc_close: shut down plugin connection
 * @hnd: a pointer to the snmp_bc_hnd struct that contains
 * a pointer to the snmp session and another to the configuration
 * data.
 **/

void snmp_bc_close(void *hnd)
{
        /*        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct snmp_bc_hnd *custom_handle =
                (struct snmp_bc_hnd *)handle->data;
        */
        /* clean up the simulator */
        sim_close();
}

static int sim_init() {

	int i;

	dbg("************************************\n");
	dbg("****** Blade Center Simulator ******\n");
	dbg("************************************\n");

	sim_hash = g_hash_table_new(g_str_hash, g_str_equal);

	if (sim_hash == NULL) {
		dbg("Cannot allocate simulation hash table\n");
		return -1;
	}

	for (i=0; sim_resource_array[i].oid != NULL; i++) {
		
		char *key;
		char *key_exists;

		SnmpMibInfoT *mibinfo;
    
		key = g_strdup(sim_resource_array[i].oid);
		if (!key) {
			dbg("Cannot allocate memory for key for oid=%s\n",
			    sim_resource_array[i].oid);
			sim_close();
			return -1;
		}
		mibinfo = g_malloc0(sizeof(SnmpMibInfoT));
		if (!mibinfo) {
			dbg("Cannot allocate memory for hash value for oid=%s", 
			    sim_resource_array[i].oid);
			sim_close();
			return -1;
		}

		key_exists = g_hash_table_lookup(sim_hash, key); 
		if (!key_exists) {
			mibinfo->type = sim_resource_array[i].mib.type;

			switch (mibinfo->type) {
			case ASN_INTEGER:
				mibinfo->value.integer = sim_resource_array[i].mib.value.integer;
				break;
			case ASN_OCTET_STR:
				strcpy(mibinfo->value.string, sim_resource_array[i].mib.value.string);
				break;
			default:
				dbg("Unknown SNMP type=%d for oid=%s\n", mibinfo->type, key);
				return -1;
			}
			g_hash_table_insert(sim_hash, key, mibinfo);
		}
		else {
			dbg("WARNING: Oid %s is defined twice\n", sim_resource_array[i].oid);
		}
	}

	return 0;
}

static int sim_close()
{

        g_hash_table_foreach(sim_hash, free_hash_data, NULL);
	g_hash_table_destroy(sim_hash);

	return 0;
}

static void free_hash_data(gpointer key, gpointer value, gpointer user_data)
{
        g_free(key);
        g_free(value);
}

