/*      -*- linux-c -*- 
 *
 * Copyright (c) 2003 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Racing Guo <racing.guo@intel.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <uuid/uuid.h>
#include <glib.h>


#include <SaHpi.h>
#include <openhpi.h>
#include <epath_utils.h>
#include <uid_utils.h>

#define info(f, ...) printf(__FILE__": " f "\n", ## __VA_ARGS__)
#define error(f, ...) perror("ERROR: " f, ## __VA_ARGS__)
#define trace(f, ...) printf(__FILE__":%s(" f ")\n", __FUNCTION__, ## __VA_ARGS__)
#define ELEMENT_NUM(x) (sizeof(x)/sizeof(x[0]))



static void *sim_open(GHashTable *handler_config)
{
        struct oh_handler_state *i;
	char *tok1, *tok2;

	if (!handler_config) {
		dbg("GHashTable *handler_config is NULL!");
		return(NULL);
	}

        trace("%s, %s, %s, %s",
	      (char *)g_hash_table_lookup(handler_config, "plugin"),
	      (char *)g_hash_table_lookup(handler_config, "name"),
	      tok1 = g_hash_table_lookup(handler_config, "entity_root"),
              tok2 = g_hash_table_lookup(handler_config, "root_path"));       
        if (!tok1) {
                dbg("entity_root is needed and not present");
                return (NULL);
        }      
        if (!tok2) {
                dbg("root_path is needed and not present");
                return (NULL);
        }

        i = malloc(sizeof(*i));
	if (!i) {
		dbg("out of memory");
		return (NULL);
	}
	memset(i, 0, sizeof(*i));

	/* save the handler config has table it holds 	*/
	/* the openhpi.conf file config info 		*/
	i->config = handler_config;

	/* initialize hashtable pointer */
	i->rptcache = (RPTable *)g_malloc0(sizeof(RPTable)); 
	return( (void *)i );
}

static void sim_close(void *hnd)
{
	struct oh_handler_state *inst = hnd;
	
	free(inst->rptcache);		            
	free(inst);
	return;
}


static int sim_get_event(void *hnd, struct oh_event *event, struct timeval *timeout)
{
  
        return -1;
}

static int sim_discover_resources(void *hnd)
{
	return -1;
}

static int sim_get_self_id(void *hnd, SaHpiResourceIdT id)
{
	return -1;
}


static int sim_get_sel_info(void *hnd, SaHpiResourceIdT id, SaHpiSelInfoT *info)
{
	return -1;
}

static int sim_set_sel_time(void *hnd, SaHpiResourceIdT id, SaHpiTimeT time)
{
	return -1;
}

static int sim_add_sel_entry(void *hnd, SaHpiResourceIdT id, const SaHpiSelEntryT *Event)
{
	return -1;
}

static int sim_del_sel_entry(void *hnd, SaHpiResourceIdT id, SaHpiSelEntryIdT sid)
{
	return -1;
}

static int sim_get_sel_entry(void *hnd, 
			       SaHpiResourceIdT id, 
			       SaHpiSelEntryIdT current,
			       SaHpiSelEntryIdT *prev, 
			       SaHpiSelEntryIdT *next, 
			       SaHpiSelEntryT *entry)
{       
	return -1;
}

/************************************************************************/
/* Begin: Sensor functions 						*/
/************************************************************************/
static int sim_get_sensor_data(void *hnd, SaHpiResourceIdT id, 
                           SaHpiSensorNumT num,
                           SaHpiSensorReadingT *data)
{
	return -1;
}

static int sim_get_sensor_thresholds(void *hnd, SaHpiResourceIdT id,
				       SaHpiSensorNumT num,
				       SaHpiSensorThresholdsT *thres)
{	
     return -1;
}

static int sim_set_sensor_thresholds(void *hnd, SaHpiResourceIdT id,
				       SaHpiSensorNumT num,
				       const SaHpiSensorThresholdsT *thres)
{	
	return -1;
}

static int sim_get_sensor_event_enables(void *hnd, SaHpiResourceIdT id,
					  SaHpiSensorNumT num,
					  SaHpiSensorEvtEnablesT *enables)
{

	return -1;
}

static int sim_set_sensor_event_enables(void *hnd, SaHpiResourceIdT id,
					  SaHpiSensorNumT num,
					  const SaHpiSensorEvtEnablesT *enables)
{			   

	return -1;
}
/************************************************************************/
/* End: Sensor functions 						*/
/************************************************************************/

static int sim_get_control_state(void *hnd, SaHpiResourceIdT id,
				   SaHpiCtrlNumT num,
				   SaHpiCtrlStateT *state)
{
	return -1;
}

static int sim_set_control_state(void *hnd, SaHpiResourceIdT id,
				   SaHpiCtrlNumT num,
				   SaHpiCtrlStateT *state)
{
			
	return -1;
}

/************************************************************************/
/* Begin: Inventory functions 						*/
/************************************************************************/
static int sim_get_inventory_size(void *hnd, SaHpiResourceIdT id,
				    SaHpiEirIdT num,
				    SaHpiUint32T *size)
{
	return -1;
}

static int sim_get_inventory_info(void *hnd, SaHpiResourceIdT id,
				    SaHpiEirIdT num,
				    SaHpiInventoryDataT *data)
{
	return -1;
}

static int sim_set_inventory_info(void *hnd, SaHpiResourceIdT id,
				    SaHpiEirIdT num,
				    const SaHpiInventoryDataT *data)
{

	return -1;
}
/************************************************************************/
/* End: Inventory functions 						*/
/************************************************************************/


static int sim_get_watchdog_info(void *hnd, SaHpiResourceIdT id,
				   SaHpiWatchdogNumT num,
				   SaHpiWatchdogT *wdt)
{
	return -1;
}

static int sim_set_watchdog_info(void *hnd, SaHpiResourceIdT id,
				   SaHpiWatchdogNumT num,
				   SaHpiWatchdogT *wdt)
{
	return -1;
}

static int sim_reset_watchdog(void *hnd, SaHpiResourceIdT id,
				SaHpiWatchdogNumT num)
{
	return -1;
}

/************************************************************************/
/* Begin: Hotswap functions 						*/
/************************************************************************/
static int sim_get_hotswap_state(void *hnd, SaHpiResourceIdT id, 
				   SaHpiHsStateT *state)
{
	return -1;
}

static int sim_set_hotswap_state(void *hnd, SaHpiResourceIdT id, 
				   SaHpiHsStateT state)
{
	return -1;

}

static int sim_request_hotswap_action(void *hnd, SaHpiResourceIdT id, 
					SaHpiHsActionT act)
{
	return -1;

}

static int sim_get_power_state(void *hnd, SaHpiResourceIdT id, SaHpiHsPowerStateT *state)
{
	return -1;
}

static int sim_set_power_state(void *hnd, SaHpiResourceIdT id, 
				 SaHpiHsPowerStateT state)
{
	return -1;
}
	
static int sim_get_indicator_state(void *hnd, SaHpiResourceIdT id, 
				     SaHpiHsIndicatorStateT *state)
{
	return -1;

}

static int sim_set_indicator_state(void *hnd, SaHpiResourceIdT id, 
				     SaHpiHsIndicatorStateT state)
{
	return -1;
}
/************************************************************************/
/* End: Hotswap functions 						*/
/************************************************************************/


static int sim_control_parm(void *hnd, SaHpiResourceIdT id, 
			      SaHpiParmActionT act)
{
	return -1;
}

static int sim_get_reset_state(void *hnd, SaHpiResourceIdT id, 
				 SaHpiResetActionT *act)
{
	return -1;
}

static int sim_set_reset_state(void *hnd, SaHpiResourceIdT id, 
				 SaHpiResetActionT act)
{
	return -1;
}

static struct oh_abi_v2 oh_sim_plugin = {
	.open				= sim_open,
	.close				= sim_close,
	.get_event			= sim_get_event,
	.discover_resources     	= sim_discover_resources,
	.get_self_id			= sim_get_self_id,
	.get_sel_info			= sim_get_sel_info,
	.set_sel_time			= sim_set_sel_time,
	.add_sel_entry			= sim_add_sel_entry,
	.del_sel_entry			= sim_del_sel_entry,
	.get_sel_entry			= sim_get_sel_entry,
	.get_sensor_data		= sim_get_sensor_data,
	.get_sensor_thresholds		= sim_get_sensor_thresholds,
	.set_sensor_thresholds		= sim_set_sensor_thresholds,
	.get_sensor_event_enables	= sim_get_sensor_event_enables,
	.set_sensor_event_enables	= sim_set_sensor_event_enables,
	.get_control_state		= sim_get_control_state,
	.set_control_state		= sim_set_control_state,
	.get_inventory_size	= sim_get_inventory_size,
	.get_inventory_info	= sim_get_inventory_info,
	.set_inventory_info	= sim_set_inventory_info,
	.get_watchdog_info	= sim_get_watchdog_info,
	.set_watchdog_info	= sim_set_watchdog_info,
	.reset_watchdog		= sim_reset_watchdog,
	.get_hotswap_state	= sim_get_hotswap_state,
	.set_hotswap_state	= sim_set_hotswap_state,
	.request_hotswap_action	= sim_request_hotswap_action,
	.get_power_state	= sim_get_power_state,
	.set_power_state	= sim_set_power_state,
	.get_indicator_state	= sim_get_indicator_state,
	.set_indicator_state	= sim_set_indicator_state,
	.control_parm		= sim_control_parm,
	.get_reset_state	= sim_get_reset_state,
	.set_reset_state	= sim_set_reset_state
};

int get_interface(void **pp, const uuid_t uuid)
{
	if (uuid_compare(uuid, UUID_OH_ABI_V2)==0) {
		*(struct oh_abi_v2 **)pp = &oh_sim_plugin;
	return 0;
	}

	*pp = NULL;
	return -1;
}
