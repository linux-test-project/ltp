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
 *     Louis Zhuang <louis.zhuang@linux.intel.com>
 *     Kevin Gao <kevin.gao@linux.intel.com>
 *     Rusty Lynch <rusty.lynch@linux.intel.com>
 *     Tariq Shureih <tariq.shureih@intel.com>
 */

#include "ipmi.h"
#include <netdb.h>

static ipmi_con_t *con;

/**
 * This is data structure reference by rsel_id.ptr
 */
struct ohoi_sel_entry {
        ipmi_mc_t       *mc;
        unsigned int    recid;
};

/* ABI Interface functions */


/**
 * *ipmi_open: open (initiate) instance of the ipmi plug-in
 * @handler_config: pointer to openhpi config file
 *
 * This function initiates an instance of the ipmi plug-in
 * and opens a new connection to OpenIPMI.
 * Depending on what the config file defines, the connection
 * could be SMI (local) or LAN.
 *
 **/
static void *ipmi_open(GHashTable *handler_config)
{
	struct oh_handler_state *handler;
	struct ohoi_handler *ipmi_handler;

	const char *name;
	const char *addr;
	int rv = 0;

	dbg("ipmi_open");
	if (!handler_config) {
		dbg("No config file provided.....ooops!");
		return(NULL);
	}

	name = g_hash_table_lookup(handler_config, "name");
	addr = g_hash_table_lookup(handler_config, "addr");
	entity_root = g_hash_table_lookup(handler_config, "entity_root");

	if ( !name || !addr || !entity_root) {
		dbg("Problem getting correct required parameters! \
				check config file");
		return NULL;
	} else
		dbg("name: %s, addr: %s, entity_root: %s", name, addr, entity_root);

	handler = (struct oh_handler_state *)g_malloc0(sizeof(struct oh_handler_state));
	ipmi_handler = (struct ohoi_handler *)g_malloc0(sizeof(struct ohoi_handler));
	if (!handler || !ipmi_handler) {
		dbg("Cannot allocate handler or private ipmi");
		return NULL;
	}	
	handler->data = ipmi_handler;

	handler->rptcache = (RPTable *)g_malloc0(sizeof(RPTable));

	handler->config = handler_config;

	ipmi_handler->SDRs_read_done = 0;
        ipmi_handler->SELs_read_done = 0;
	
	sel_alloc_selector(&ui_sel);
	ipmi_init(&ipmi_ui_cb_handlers);
	
	if (strcmp(name, "smi") == 0) {
		int tmp = strtol(addr, (char **)NULL, 10);
		
		rv = ipmi_smi_setup_con(tmp,&ipmi_ui_cb_handlers,ui_sel,&con);
		if (rv) {
			dbg("Cannot setup connection");
			return NULL;
		}		
	} else if (strcmp(name, "lan") == 0) {
		static struct in_addr lan_addr;
		static int lan_port;
		static int auth;
		static int priv;

		char *tok;

		char user[32], passwd[32];
		
		/* Address */
		tok = g_hash_table_lookup(handler_config, "addr");
		dbg("IPMI LAN Address: %s", tok);
		struct hostent *ent = gethostbyname(tok);
		if (!ent) {
			dbg("Unable to resolve IPMI LAN address");
			return NULL;
		}
		memcpy(&lan_addr, ent->h_addr_list[0], ent->h_length);
		/* Port */
		tok = g_hash_table_lookup(handler_config, "port");
		lan_port = atoi(tok);
		dbg("IPMI LAN Port: %i", lan_port);

		/* Authentication type */
		tok = g_hash_table_lookup(handler_config, "auth_type");
		if (strcmp(tok, "none") == 0) {
			auth = IPMI_AUTHTYPE_NONE;
		} else if (strcmp(tok, "straight") == 0) {
			auth = IPMI_AUTHTYPE_STRAIGHT;
		} else if (strcmp(tok, "md2") == 0) {
			auth = IPMI_AUTHTYPE_MD2;
		} else if (strcmp(tok, "md5") == 0) {
			auth = IPMI_AUTHTYPE_MD5;
		} else {
			dbg("Invalid IPMI LAN authenication method: %s", tok);
			return NULL;
		}
		dbg("IPMI LAN Authority: %s(%i)", tok, auth);

		/* Priviledge */
		tok = g_hash_table_lookup(handler_config, "auth_level");
		if (strcmp(tok, "callback") == 0) {
			priv = IPMI_PRIVILEGE_CALLBACK;
		} else if (strcmp(tok, "user") == 0) {
			priv = IPMI_PRIVILEGE_USER;
		} else if (strcmp(tok, "operator") == 0) {
			priv = IPMI_PRIVILEGE_OPERATOR;
		} else if (strcmp(tok, "admin") == 0) {
			priv = IPMI_PRIVILEGE_ADMIN;
		} else {
			dbg("Invalid IPMI LAN authenication method: %s", tok);
			return NULL;
		}
		dbg("IPMI LAN Priviledge: %s(%i)", tok, priv);

		/* User Name */
		tok = g_hash_table_lookup(handler_config, "username"); 
		strncpy(user, tok, 32);
		dbg("IPMI LAN User: %s", user);

		/* Password */
		tok = g_hash_table_lookup(handler_config, "password");  
		strncpy(passwd, tok, 32);
		dbg("IPMI LAN Password: %s", passwd);

		free(tok);

		rv = ipmi_lan_setup_con(&lan_addr, &lan_port, 1,
					auth, priv,
					user, strlen(user),
					passwd, strlen(passwd),
					&ipmi_ui_cb_handlers, ui_sel,
					&con);
	} else {
		dbg("Unsupported IPMI connection method: %s",name);
		return NULL;
	}

	rv = ipmi_init_domain(&con, 1, ohoi_setup_done, handler, NULL, &ipmi_handler->domain_id);

	if (rv) {
		fprintf(stderr, "ipmi_init_domain: %s\n", strerror(rv));
		return NULL;
	}

	return handler;
}

/**
 * ipmi_close: close this instance of ipmi plug-in
 * @hnd: pointer to handler
 *
 * This functions closes connection with OpenIPMI and frees
 * all allocated events and sensors
 *
 **/
static void ipmi_close(void *hnd)
{
	struct oh_handler_state *handler = (struct oh_handler_state *) hnd;
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;

	ipmi_shutdown();
	
	g_free(ipmi_handler);
	g_free(handler);
}

/**
 * ipmi_get_event: get events populated earlier by OpenIPMI
 * @hnd: pointer to handler
 * @event: pointer to an openhpi event structure
 * @timeout: time to block
 *
 *
 *
 * Return value: 1 or 0
 **/
static int ipmi_get_event(void *hnd, struct oh_event *event, 
			  struct timeval *timeout)
{
	struct oh_handler_state *handler = hnd;

	if(g_slist_length(handler->eventq)>0) {
		
		memcpy(event, handler->eventq->data, sizeof(*event));
		free(handler->eventq->data);
		handler->eventq = g_slist_remove_link(handler->eventq, handler->eventq);
		return 1;
	} else
		return 0;
			
}

/**
 * ipmi_discover_resources: discover resources in system
 * @hnd: pointer to handler
 *
 * This function is both sepcific to OpenIPMI and conforms to openhpi
 * plug-in interface in waiting until OpenIPMI discovery is done,
 * then retieving entries from the rptcach of the oh_handler_state and
 * populating the eventq for the infrastructure to fetch.
 *
 * Return value: -1 for failure or 0 for success
 **/
static int ipmi_discover_resources(void *hnd)
{
	int rv;
	struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;
	struct oh_event *event;
	SaHpiRptEntryT *rpt_entry;
	SaHpiRdrT	*rdr_entry;

	dbg("ipmi discover_resources");
	
	while (0 == ipmi_handler->SDRs_read_done || 0 == ipmi_handler->SELs_read_done) {
		rv = sel_select(ui_sel, NULL, 0 , NULL, NULL);
		if (rv<0) {
			dbg("error on waiting for discovery");
			return -1;
		}
	}

        rpt_entry = oh_get_resource_next(handler->rptcache, RPT_ENTRY_BEGIN);
        while (rpt_entry) {
                event = g_malloc0(sizeof(*event));
                memset(event, 0, sizeof(*event));
                event->type = OH_ET_RESOURCE;
                memcpy(&event->u.res_event.entry, rpt_entry, sizeof(SaHpiRptEntryT));
                handler->eventq = g_slist_append(handler->eventq, event);

                dbg("Now adding rdr for resource: %d", event->u.res_event.entry.ResourceId);
                rdr_entry = oh_get_rdr_next(handler->rptcache,rpt_entry->ResourceId, RDR_BEGIN);
                
                while (rdr_entry) {
                        event = g_malloc0(sizeof(*event));
                        memset(event, 0, sizeof(*event));
                        event->type = OH_ET_RDR;
                        memcpy(&event->u.rdr_event.rdr, rdr_entry, sizeof(SaHpiRdrT));
                        handler->eventq = g_slist_append(handler->eventq, event);
                        
                        rdr_entry = oh_get_rdr_next(handler->rptcache, rpt_entry->ResourceId, rdr_entry->RecordId);
                }
                
                rpt_entry = oh_get_resource_next(handler->rptcache, rpt_entry->ResourceId);
        }

	return 0;
}

/**
 * ipmi_get_sel_info: get ipmi SEL info
 * @hnd: pointer to handler
 * @id: resource id
 * @info: output -- pointer to info structure passed from infra.
 *
 * This function retrieves the SEL information from
 * the BMC or an SEL capable MC in an IPMI domain
 * 
 *
 * Return value: 0
 **/
static SaErrorT ipmi_get_sel_info(void               *hnd,
                             SaHpiResourceIdT   id,
                             SaHpiSelInfoT      *info)
{
        unsigned int count;

        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        const struct ohoi_resource_id *ohoi_res_id;

        ohoi_res_id = oh_get_resource_data(handler->rptcache, id);
        if (ohoi_res_id->type != OHOI_RESOURCE_MC) {
                dbg("BUG: try to get sel in unsupported resource");
                return SA_ERR_HPI_INVALID_CMD;
        }
	
        ohoi_get_sel_count(ohoi_res_id->u.mc_id, &count);
	
        info->Entries           = count;
        info->Size              = -1; /* FIXME: how to get total size in OpenIPMI? */
        ohoi_get_sel_updatetime(ohoi_res_id->u.mc_id, &info->UpdateTimestamp);
        ohoi_get_sel_time(ohoi_res_id->u.mc_id, &info->CurrentTime);
        info->Enabled           = 1; /* FIXME: how to disable SEL in OpenIPMI */
        ohoi_get_sel_overflow(ohoi_res_id->u.mc_id, &info->OverflowFlag);
        info->OverflowAction    = SAHPI_SEL_OVERFLOW_DROP;
        ohoi_get_sel_support_del(ohoi_res_id->u.mc_id, &info->DeleteEntrySupported);
        
        return 0;
}

/**
 * ipmi_set_sel_time: set ipmi event log time
 * @hnd: pointer to handler
 * @id: resource id of resource holding sel
 * @time: pointer to time structure
 *
 * This functions set the clocl in the event log.
 * 
 *
 * Return value: 0 for success, -1 for error
 **/
static int ipmi_set_sel_time(void               *hnd,
                             SaHpiResourceIdT   id,
                             SaHpiTimeT    time)
{
        struct ohoi_resource_id *ohoi_res_id;
	struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        struct timeval tv;
        
        ohoi_res_id = oh_get_resource_data(handler->rptcache, id);
        if (ohoi_res_id->type != OHOI_RESOURCE_MC) {
                dbg("BUG: try to get sel in unsupported resource");
                return SA_ERR_HPI_INVALID_CMD;
        }
        
        tv.tv_sec = time/1000000000;
        tv.tv_usec= (time%1000000000)/1000;
        ohoi_set_sel_time(ohoi_res_id->u.mc_id, &tv);
        return 0;
}

#if 0
/**
 * ipmi_set_sel_state: set ipmi sel state (enabled)
 * @hnd: pointer to handler
 * @id: resource id of resource with SEL capability
 * @enable: int value for enable
 *
 *
 *
 * Return value: 0 for success, -1 for error
 **/
static int ipmi_set_sel_state(void      *hnd, 
                //SaHpiResourceIdT   id, 
                //int                     enable)
{
        ///* need OpenIPMI support */
        //return -1;
}

/**
 * ipmi_add_sel_entry: add an entry to system sel from user
 * @hnd: pointer to handler
 * @id: resource id with SEL capability
 * @Event: SaHpiSelEntryT pointer to event to be added
 *
 *
 *
 * Return value: -1 for error, success is OpenIPMI command succeed
 **/
static int ipmi_add_sel_entry(void      *hnd, 
                SaHpiResourceIdT   id, 
                const SaHpiSelEntryT    *Event)
{
        //ipmi_mc_t *mc = id.ptr; 
        ipmi_msg_t msg;
        unsigned char *buf;
        
        /* Send a software event into Event Receiver IPMI_1_5 9.4 */
        buf = malloc(sizeof(*Event)+1);
        if (!buf) {
                dbg("Out of memory");
                return -1;
        }
        
        msg.netfn       = 0x05;
        msg.cmd         = 0x02;
        msg.data[0]     = 0x41;
        memcpy(msg.data+1, Event, sizeof(*Event));
        msg.data_len    = sizeof(*Event)+1;
        
        //return ipmi_mc_send_command(mc, 0, &msg, NULL, NULL);
	return(-1);
}

static int ipmi_del_sel_entry(void      *hnd,
        		        SaHpiResourceIdT id,
				SaHpiSelEntryIdT sid)
{
        //struct ohoi_sel_entry *entry = id.ptr;
        //ipmi_event_t event;
        //event.record_id = entry->recid;
        //_ipmi_mc_del_event(entry->mc, &event, NULL, NULL);
        return -1;
}
#endif

/**
 * ipmi_get_sel_entry: get IPMI SEL entry
 * @hnd: pointer to handler instance
 * @id: resourde id with SEL capability
 * @current: SaHpiEntryIdT of entry to retrieve
 * @prev: previous entry in log relative to current
 * @next: next entry in log
 * @entry: [out]SaHpiSelEntryT entry requested
 *
 * This function will get event(s) from SEL capable IPMI device
 * one at a time by the record id starting with HPI's
 * SAHPI_OLDEST_ENTRY or SAHPI_NEWEST_ENTRY.
 *
 * Return value: 0 for success -1 for failure
 **/
static int ipmi_get_sel_entry(void *hnd, SaHpiResourceIdT id,
				SaHpiSelEntryIdT current,
				SaHpiSelEntryIdT *prev,
				SaHpiSelEntryIdT *next,
	       			SaHpiSelEntryT *entry)
{
        struct ohoi_resource_id *ohoi_res_id;
	struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        ipmi_event_t event;

        ohoi_res_id = oh_get_resource_data(handler->rptcache, id);
        if (ohoi_res_id->type != OHOI_RESOURCE_MC) {
                dbg("BUG: try to get sel in unsupported resource");
                return SA_ERR_HPI_INVALID_CMD;
        }
         

	switch (current) {
        case SAHPI_OLDEST_ENTRY:
		ohoi_get_sel_first_entry(ohoi_res_id->u.mc_id, &event);
                
		ohoi_get_sel_next_recid(ohoi_res_id->u.mc_id, &event, next);
		
                *prev = SAHPI_NO_MORE_ENTRIES;
                break;
		
        case SAHPI_NEWEST_ENTRY:
                ohoi_get_sel_last_entry(ohoi_res_id->u.mc_id, &event);

                *next = SAHPI_NO_MORE_ENTRIES;

                ohoi_get_sel_prev_recid(ohoi_res_id->u.mc_id, &event, prev);
                break;
                
        default:                		
		/* get the entry requested by id */
		ohoi_get_sel_by_recid(ohoi_res_id->u.mc_id, *next, &event);

		ohoi_get_sel_next_recid(ohoi_res_id->u.mc_id, &event, next);

                ohoi_get_sel_prev_recid(ohoi_res_id->u.mc_id, &event, prev);
                break; 
	}
        memcpy(&entry->Event.EventDataUnion.UserEvent.UserEventData[3],
               event.data, 
               sizeof(event.data));	

	return 0;		
}

static SaErrorT ipmi_clear_sel(void *hnd, SaHpiResourceIdT id)
{
        struct ohoi_resource_id *ohoi_res_id;
	struct oh_handler_state *handler = (struct oh_handler_state *)hnd;

        ohoi_res_id = oh_get_resource_data(handler->rptcache, id);
        if (ohoi_res_id->type != OHOI_RESOURCE_MC) {
                dbg("BUG: try to get sel in unsupported resource");
                return SA_ERR_HPI_INVALID_CMD;
        }

        return ohoi_clear_sel(ohoi_res_id->u.mc_id);
}

static SaErrorT get_rdr_data(const struct oh_handler_state *handler,
                             SaHpiResourceIdT              id,
                             SaHpiRdrTypeT                 type,
                             SaHpiUint8T                   num,
                             void                          **pdata)
{
        SaHpiRdrT * rdr;
        rdr = oh_get_rdr_by_type(handler->rptcache,
                                 id,
                                 type,
                                 num);
        if (!rdr) {
                /*XXX No err code for invalid rdr?*/
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        *pdata = oh_get_rdr_data(handler->rptcache,
                                 id,
                                 rdr->RecordId);
        return SA_OK;
}

/**
 * ipmi_get_sensor_data: get sensor reading, type, category and other info.
 * @hnd: pointer to handler instance
 * @id: ResourceId -- parent of this sensor
 * @num: sensor number
 * @data: struct returned with data about the sensor.
 *
 *
 *
 * Return value: 0 for success or negative for error
 **/
static int ipmi_get_sensor_data(void 			*hnd, 
				SaHpiResourceIdT	id,
				SaHpiSensorNumT		num,
				SaHpiSensorReadingT	*data)
{
        SaErrorT         rv;
	ipmi_sensor_id_t *sensor;
        
        rv = get_rdr_data(hnd, id, SAHPI_SENSOR_RDR, num, (void *)&sensor);
        if (rv!=SA_OK)
                return rv;

	memset(data, 0, sizeof(*data));
	return ohoi_get_sensor_data(*sensor, data);
}

/**
 * ipmi_get_sensor_thresholds: for hysteresis sensors, get thresholds.
 * @hnd: handler instance
 * @id: ResourceId parent of this sensor
 * @num: sensor number
 * @thres: struct returned with data about sensor thresholds.
 *
 *
 *
 * Return value: 0 for success or negative for error
 **/
static int ipmi_get_sensor_thresholds(void			*hnd, 
				      SaHpiResourceIdT		id,
				      SaHpiSensorNumT		num,
				      SaHpiSensorThresholdsT	*thres)
{
        SaErrorT         rv;
	ipmi_sensor_id_t *sensor;
        
        rv = get_rdr_data(hnd, id, SAHPI_SENSOR_RDR, num, (void *)&sensor);
        if (rv!=SA_OK)
                return rv;

	memset(thres, 0, sizeof(*thres));
	return ohoi_get_sensor_thresholds(*sensor, thres);
}

static int ipmi_set_sensor_thresholds(void				*hnd,
				      SaHpiResourceIdT			id,
				      SaHpiSensorNumT			num,
				      const SaHpiSensorThresholdsT	*thres)
{
        SaErrorT         rv;
	ipmi_sensor_id_t *sensor;
        
        rv = get_rdr_data(hnd, id, SAHPI_SENSOR_RDR, num, (void *)&sensor);
        if (rv!=SA_OK)
                return rv;

	return ohoi_set_sensor_thresholds(*sensor, thres);	
}

static int ipmi_get_sensor_event_enables(void			*hnd, 
					 SaHpiResourceIdT	id,
					 SaHpiSensorNumT	num,
					 SaHpiSensorEvtEnablesT	*enables)
{
        SaErrorT         rv;
	ipmi_sensor_id_t *sensor;
        
        rv = get_rdr_data(hnd, id, SAHPI_SENSOR_RDR, num, (void *)&sensor);
        if (rv!=SA_OK)
                return rv;

	return ohoi_get_sensor_event_enables(*sensor, enables);
}

static int ipmi_set_sensor_event_enables(void 			  *hnd,
			      		SaHpiResourceIdT  id,
					SaHpiSensorNumT	num,
				     const SaHpiSensorEvtEnablesT *enables)
{
	
	//ipmi_sensor_t	*sensor = id.ptr;
	//int		rv;
	//
	//rv = ohoi_set_sensor_event_enable(sensor, enables);
	//return rv < 0 ? rv : 0;	
	return(-1);
}

static struct oh_abi_v2 oh_ipmi_plugin = {
	.open	 			= ipmi_open,
	.close				= ipmi_close,
	.get_event			= ipmi_get_event,
	.discover_resources		= ipmi_discover_resources,
	.get_sel_info                   = ipmi_get_sel_info,
        .set_sel_time                   = ipmi_set_sel_time,
        //.add_sel_entry                  = ipmi_add_sel_entry,
        //.del_sel_entry                  = ipmi_del_sel_entry,
        .get_sel_entry                  = ipmi_get_sel_entry,
        .clear_sel                      = ipmi_clear_sel, 
        //.add_sel_entry                  = ipmi_add_sel_entry,
	.get_sensor_data		= ipmi_get_sensor_data,
	.get_sensor_thresholds		= ipmi_get_sensor_thresholds,
	.set_sensor_thresholds		= ipmi_set_sensor_thresholds,
	.get_sensor_event_enables       = ipmi_get_sensor_event_enables,
	.set_sensor_event_enables       = ipmi_set_sensor_event_enables
};

int get_interface(void **pp, const uuid_t uuid)
{
	if (uuid_compare(uuid, UUID_OH_ABI_V2)==0) {
		*pp = &oh_ipmi_plugin;
		return 0;
	}

	*pp = NULL;
	return -1;
}
