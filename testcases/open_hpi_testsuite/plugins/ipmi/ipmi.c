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
 *     Racing Guo  <racing.guo@intel.com>
 *     Andy Cress  <andrew.r.cress@intel.com> (watchdog)
 */

#include "ipmi.h"
#include <oh_domain.h>
#include <netdb.h>

/* Watchdog definitions */
#define WATCHDOG_RESET   0x22
#define WATCHDOG_SET     0x24
#define WATCHDOG_GET     0x25
#define NETFN_APP        0x06
#define IPMI_WD_RESOLUTION           100
#define IPMI_WD_PRETIMER_RESOLUTION 1000
#define uchar  unsigned char


extern int __ipmi_debug_malloc;
//extern int ipmi_close_mv(void);
extern int ipmicmd_mv(struct ohoi_handler *ipmi_handler,
               uchar cmd, uchar netfn, uchar lun, uchar *pdata,
               uchar sdata, uchar *presp, int sresp, int *rlen);

FILE *trace_msg_file = NULL;

/**
 * This is data structure reference by rsel_id.ptr
 */
struct ohoi_sel_entry {
        ipmi_mc_t       *mc;
        unsigned int    recid;
};

/* global reference count of instances */
static int ipmi_refcount = 0;



static void ipmi_domain_fully_up(ipmi_domain_t *domain,
                                void *user_data)
{
        struct oh_handler_state *handler = user_data;
        struct ohoi_handler *ipmi_handler = handler->data;

        if (getenv("OHOI_TRACE_DOMAINUP")) {
		printf("           ****    DOMAIN FULLY UP *****\n");
	}
	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
        ipmi_handler->fully_up = 1;
	ipmi_handler->d_type = ipmi_domain_get_type(domain);
	if (!IS_ATCA(ipmi_handler->d_type)) {
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		return;
	}
	ohoi_atca_create_shelf_virtual_rdrs(handler);
	ohoi_atca_create_fru_rdrs(handler);
        g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
}






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
static void *ipmi_open(GHashTable *handler_config,
                       unsigned int hid,
                       oh_evt_queue *eventq)
{
        struct oh_handler_state *handler = NULL;
        struct ohoi_handler *ipmi_handler = NULL;
        char    domain_name[24];
        char *domain_tag = NULL;

        const char *name;
        const char *addr;
        const char *timeout;
        const char *scan_time;
        const char *real_write_fru;
        int rv = 0;
	char *trace_file_name;
	/*
        SaHpiTextBufferT        buf = {
                                        .DataType = SAHPI_TL_TYPE_TEXT,
                                        .Language = SAHPI_LANG_ENGLISH,
                                };*/

        trace_ipmi("ipmi_open");
        if (!handler_config) {
                err("No config file provided.....ooops!");
                return(NULL);
        } else if (!hid) {
                err("Bad handler id passed.");
                return NULL;
        } else if (!eventq) {
                err("No event queue was passed.");
                return NULL;
        }

        name = g_hash_table_lookup(handler_config, "name");
        addr = g_hash_table_lookup(handler_config, "addr");
        timeout = g_hash_table_lookup(handler_config, "TimeOut");
        scan_time = g_hash_table_lookup(handler_config, "OpenIPMIscanTime");
        real_write_fru = g_hash_table_lookup(handler_config, "RealWriteFru");
        domain_tag = g_hash_table_lookup(handler_config, "DomainTag");

        handler = (struct oh_handler_state *)malloc(sizeof(
                        struct oh_handler_state));
	if (handler == NULL) {
		err("Out of memory");
		return NULL;
	}
	memset(handler, 0, sizeof(struct oh_handler_state));
        ipmi_handler = (struct ohoi_handler *)malloc(sizeof(
					struct ohoi_handler));
        if (ipmi_handler == NULL) {
                err("Out of memory");
		free(handler);
                return NULL;
        }
	memset(ipmi_handler, 0, sizeof(struct ohoi_handler));

        handler->rptcache = (RPTable *)malloc(sizeof(RPTable));
	if (handler->rptcache == NULL) {
		err("Out of memory");
		free(handler);
		free(ipmi_handler);
		return NULL;
	}
	memset(handler->rptcache, 0, sizeof(RPTable));

        handler->data = ipmi_handler;
        handler->config = handler_config;
        handler->hid = hid;
        handler->eventq = eventq;

        g_static_rec_mutex_init(&(ipmi_handler->ohoih_lock));

        snprintf(domain_name, 24, "%s %s", name, addr);
        /* Discovery routine depends on these flags */
        ipmi_handler->SDRs_read_done = 0;
        /* Domain (main) SDR flag, 1 when done */
        ipmi_handler->SELs_read_done = 0;
        /* SEL flag, 1 when done */
        ipmi_handler->mc_count = 0;
        /* MC level SDRs, 0 when done */

        ipmi_handler->FRU_done = 0;
        /* MC level SDRs, 0 when done */
        ipmi_handler->bus_scan_done = 0;

        ipmi_handler->fullup_timeout = 60;
        ipmi_handler->openipmi_scan_time = 0;
        ipmi_handler->real_write_fru = 0;

        /** This code has been commented due to the multi domain changes
	 ** in the infrastructure. (Renier Morales 11/21/06)
	multi_domains = g_hash_table_lookup(handler_config, "MultipleDomains");
        if (multi_domains != (char *)NULL) {
                if (domain_tag != NULL) {
                        oh_append_textbuffer(&buf, domain_tag);
                } else {
                        oh_append_textbuffer(&buf, "IPMI Domain");
                }

                ipmi_handler->did = oh_request_new_domain_aitimeout(hid, &buf,
                        SAHPI_DOMAIN_CAP_AUTOINSERT_READ_ONLY,
                        SAHPI_TIMEOUT_BLOCK, 0, 0);
        } else
                ipmi_handler->did = oh_get_default_domain_id();
	*/

        if (timeout != NULL) {
                ipmi_handler->fullup_timeout = (time_t)strtol(timeout,
                                        (char **)NULL, 10);
        }
        if (scan_time != NULL) {
                ipmi_handler->openipmi_scan_time = (time_t)strtol(scan_time,
                                        (char **)NULL, 10);
        }
        if (real_write_fru && !strcasecmp(real_write_fru, "yes")) {
                ipmi_handler->real_write_fru = 1;
        } else {
                ipmi_handler->real_write_fru = 0;
        }
        ipmi_handler->fully_up = 0;
        ipmi_handler->entity_root = g_hash_table_lookup(handler_config,
                                        "entity_root");

        if ( !name || !addr || !ipmi_handler->entity_root) {
                err("Problem getting correct required parameters! \
                                check config file");
                goto free_and_out;
        } else
                trace_ipmi("name: %s, addr: %s, entity_root: %s, timeout: %d",
                                name, addr,
                 ipmi_handler->entity_root, (int)ipmi_handler->fullup_timeout);

        /* OS handler allocated first. */
        ipmi_handler->os_hnd = ipmi_posix_get_os_handler();
        sel_alloc_selector(ipmi_handler->os_hnd, &ipmi_handler->ohoi_sel);
        ipmi_posix_os_handler_set_sel(ipmi_handler->os_hnd,
                        ipmi_handler->ohoi_sel);

	trace_file_name = getenv("OHOI_TRACE_FILE");
	if (trace_file_name != NULL) {
		trace_msg_file = fopen(trace_file_name, "w+");
		if (trace_msg_file) {
			if (getenv("OHOI_TRACE_MSG")) {
				DEBUG_MSG_ENABLE();
				DEBUG_MSG_ERR_ENABLE();
			}
			//if (getenv("OHOI_DBG_MEM")) {
			//	__ipmi_debug_malloc = 1;
			//	DEBUG_MALLOC_ENABLE();
			//}
		}
	}
        ipmi_init(ipmi_handler->os_hnd);
        if (strcmp(name, "general") == 0) {
#if 0
		char *tok;
		char *pptr;
		char *val;
		char **argv = NULL;
		int  vals_len = 0;
		int  argc = 0;
		int  curr_arg = 0;
		ipmi_args_t *iargs[2];
		int curr_iarg = 0;
		int i, j;

                tok = g_hash_table_lookup(handler_config, "addr");
                if (tok == NULL) {
                        err("no ""addr"" token in config file");
                        goto free_and_out;
                }
		val = strtok_r(tok, " \t\n", &pptr);
		while (val) {
			if (argc == vals_len) {
				vals_len += 10;
				char **nv = malloc(sizeof(char *) * vals_len);
				if (!nv) {
					err("Out of memory");
					if (argv)
						free(argv);
					goto free_and_out;
				}
				if (argv)
					free(argv);
				argv = nv;
			}
			argv[argc] = val;
			argc++;
			val = strtok_r(NULL, " \t\n", &pptr);
		}

		if (!argv) {
                        err("""addr"" token in config file was empty");
			goto free_and_out;
		}

		while (curr_arg < argc) {
			if (curr_iarg == 2)
				break;
			rv = ipmi_parse_args2(&curr_arg, argc, argv,
					      &iargs[curr_iarg]);
			if (rv) {
				err("Cannot parse connection arguments");
				free(argv);
				goto free_and_out;
			}
			curr_iarg++;
		}
		free(argv);

		for (i=0; i<curr_iarg; i++) {
			rv = ipmi_args_setup_con(iargs[i],
						 ipmi_handler->os_hnd,
						 ipmi_handler->ohoi_sel,
						 &ipmi_handler->cons[i]);
			if (rv) {
				err("Cannot setup connection");
				for (j=0; j<i; j++)
					ipmi_free_args(iargs[j]);
				for (j=0; j<curr_iarg; j++)
					ipmi_free_args(iargs[j]);
				goto free_and_out;
			}
		}
		for (i=0; i<curr_iarg; i++)
			ipmi_free_args(iargs[i]);

		ipmi_handler->num_cons = curr_iarg;

                ipmi_handler->islan = strcmp(ipmi_handler->cons[0]->con_type,
					     "smi") != 0;
#else
                err("Unsupported IPMI connection method: %s",name);
                goto free_and_out;
#endif
        } else if (strcmp(name, "smi") == 0) {
                int tmp = strtol(addr, (char **)NULL, 10);

                rv = ipmi_smi_setup_con(tmp,ipmi_handler->os_hnd,
                                ipmi_handler->ohoi_sel,
				&ipmi_handler->cons[0]);
                if (rv) {
                        err("Cannot setup connection");
                        goto free_and_out;
                }
		ipmi_handler->num_cons = 1;
                ipmi_handler->islan = 0;
        } else if (strcmp(name, "lan") == 0) {
                static struct in_addr lan_addr;
                static int lan_port;
                static int auth;
                static int priv;

                char *tok;
                char user[32], passwd[32];

                /* Address */
                tok = g_hash_table_lookup(handler_config, "addr");
                if (tok == NULL) {
                        err("no ""addr"" token in config file");
                        goto free_and_out;
                }
                trace_ipmi("IPMI LAN Address: %s", tok);
                struct hostent *ent = gethostbyname(tok);
                if (!ent) {
                        err("Unable to resolve IPMI LAN address");
                        goto free_and_out;
                }

                memcpy(&lan_addr, ent->h_addr_list[0], ent->h_length);

                /* Port */
                tok = g_hash_table_lookup(handler_config, "port");
                if (tok == NULL) {
                        err("no ""port"" token in config file. set 623");
                        lan_port = 623;
                } else {
                        lan_port = atoi(tok);
                }
                trace_ipmi("IPMI LAN Port: %i", lan_port);

                /* Authentication type */
                tok = g_hash_table_lookup(handler_config, "auth_type");
                if (tok == NULL) {
                        err("no ""auth_type"" token in config file. set ""none""");
                        auth = IPMI_AUTHTYPE_NONE;
                } else if (strcmp(tok, "none") == 0) {
                        auth = IPMI_AUTHTYPE_NONE;
                } else if (strcmp(tok, "straight") == 0) {
                        auth = IPMI_AUTHTYPE_STRAIGHT;
                } else if (strcmp(tok, "md2") == 0) {
                        auth = IPMI_AUTHTYPE_MD2;
                } else if (strcmp(tok, "md5") == 0) {
                        auth = IPMI_AUTHTYPE_MD5;
                } else {
                        err("Invalid IPMI LAN authenication method: %s", tok);
                        goto free_and_out;
                }

                trace_ipmi("IPMI LAN Authority: %s(%i)", tok, auth);

                /* Priviledge */
                tok = g_hash_table_lookup(handler_config, "auth_level");
                if (tok == NULL) {
                        err("no ""auth_level"" token in config file."
                                " set ""admin""");
                        priv = IPMI_PRIVILEGE_ADMIN;
                } else if (strcmp(tok, "callback") == 0) {
                        priv = IPMI_PRIVILEGE_CALLBACK;
                } else if (strcmp(tok, "user") == 0) {
                        priv = IPMI_PRIVILEGE_USER;
                } else if (strcmp(tok, "operator") == 0) {
                        priv = IPMI_PRIVILEGE_OPERATOR;
                } else if (strcmp(tok, "admin") == 0) {
                        priv = IPMI_PRIVILEGE_ADMIN;
                } else if (strcmp(tok, "oem") == 0) {
                        priv = IPMI_PRIVILEGE_OEM;
                } else {
                        err("Invalid IPMI LAN authenication method: %s", tok);
                        goto free_and_out;
                }

                trace_ipmi("IPMI LAN Priviledge: %s(%i)", tok, priv);

                /* User Name */
                tok = g_hash_table_lookup(handler_config, "username");
                if (tok == NULL) {
                        err("no ""username"" token in config file");
                        strncpy(user, "", 32);
                } else {
                        strncpy(user, tok, 32);
                }
                trace_ipmi("IPMI LAN User: %s", user);

                /* Password */
                tok = g_hash_table_lookup(handler_config, "password");
                if (tok == NULL) {
                        err("no ""password"" token in config file");
                        strncpy(passwd, "", 32);
                } else {
                        strncpy(passwd, tok, 32);
                        free(tok);
                }
                trace_ipmi("IPMI LAN Password: %s", passwd);


                rv = ipmi_lan_setup_con(&lan_addr, &lan_port, 1,
                                        auth, priv,
                                        user, strlen(user),
                                        passwd, strlen(passwd),
                                        ipmi_handler->os_hnd,
                                        ipmi_handler->ohoi_sel,
                                        &ipmi_handler->cons[0]);
                if (rv) {
			err("ipmi_lan_setup_con rv = %d", rv);
                	goto free_and_out;
		}
		ipmi_handler->num_cons = 1;
                ipmi_handler->islan = 1;
        } else {
                err("Unsupported IPMI connection method: %s",name);
                goto free_and_out;
        }

        ipmi_handler->connected = -1;

        rv = ipmi_open_domain(domain_name, ipmi_handler->cons,
			ipmi_handler->num_cons,
                        ipmi_connection_handler, handler,
                        ipmi_domain_fully_up, handler,
                        NULL, 0, &ipmi_handler->domain_id);
        if (rv) {
                fprintf(stderr, "ipmi_open_domain: %s\n", strerror(rv));
                goto free_and_out;
        }

        ipmi_refcount++;
        return handler;

free_and_out:
	g_free(handler->rptcache);
	g_free(handler);
	g_free(ipmi_handler);
	return NULL;
}


/**
 * ipmi_close: close this instance of ipmi plug-in
 * @hnd: pointer to handler
 
 * This functions closes connection with OpenIPMI and frees
 * all allocated events and sensors
 *
 **/
static void ipmi_close(void *hnd)
{

        struct oh_handler_state *handler = (struct oh_handler_state *) hnd;
        struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;


        if (ipmi_handler->connected) {
                trace_ipmi("close connection");
                ohoi_close_connection(ipmi_handler->domain_id, handler);
        }

//        ipmi_close_mv();
        ipmi_refcount--;
        trace_ipmi("ipmi_refcount :%d", ipmi_refcount);

        if(ipmi_refcount == 0) {
                /* last connection and in case other instances didn't
                   close correctly we clean up all connections */
                trace_ipmi("Last connection :%d closing", ipmi_refcount);
                ipmi_shutdown();
        }

        oh_flush_rpt(handler->rptcache);
        free(handler->rptcache);

        free(ipmi_handler);
        free(handler);
}

/**
 * ipmi_get_event: get events populated earlier by OpenIPMI
 * @hnd: pointer to handler
 *
 * Return value: 1 or 0
 **/
static int ipmi_get_event(void *hnd)
{
        struct oh_handler_state *handler = hnd;
        struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;
        int sel_select_done = 0;

        for (;;) {

                if (sel_select_done) {
                        break;
                }

                while (1 == sel_select(ipmi_handler->ohoi_sel,
                        NULL, 0, NULL, NULL));
                sel_select_done = 1;
        };
        return 0;
}






static void trace_ipmi_resources(SaHpiRptEntryT *rpt_entry,
                                 struct ohoi_resource_info *res_info)
{
	oh_big_textbuffer bigbuf;
	if (!getenv("OHOI_TRACE_DISCOVERY") && !IHOI_TRACE_ALL) {
		return;
	}
	unsigned char str[32];
	if (res_info->type & OHOI_RESOURCE_ENTITY) {
		ipmi_entity_id_t *e = &res_info->u.entity.entity_id;
		snprintf((char *)str, 32, "(%d,%d,%d,%d)", e->entity_id,
			e->entity_instance, e->channel, e->address);
	} else {
		str[0] = 0;
	}
	oh_decode_entitypath(&(rpt_entry->ResourceEntity), &bigbuf);
	fprintf(stderr, "%s %d %s presence: %d; updated:%d  %s\n",
			rpt_entry->ResourceTag.Data, rpt_entry->ResourceId,
			str, res_info->presence, res_info->updated,
			bigbuf.Data
			);
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
int ipmi_discover_resources(void *hnd)
{

        int rv = 1;
        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;
        struct oh_event *event;
        SaHpiRptEntryT *rpt_entry;
        SaHpiRdrT       *rdr_entry;
        time_t          tm0, tm;
        int was_connected = 0;
        struct ohoi_resource_info       *res_info;

        dbg("ipmi discover_resources");

	// Look for IPMI events until we get the ipmi_domain_fully_up callback
        time(&tm0);
        while (ipmi_handler->fully_up == 0) {
                if (!ipmi_handler->connected) {
                        fprintf(stderr, "IPMI connection is down\n");
                        return SA_ERR_HPI_NO_RESPONSE;
                }
                if ((ipmi_handler->connected == 1) && !was_connected) {
                        // set new time stamp. IPMI is alive
                        was_connected = 1;
                        time(&tm0);
                }

                rv = sel_select(ipmi_handler->ohoi_sel, NULL, 0 , NULL, NULL);
                if (rv < 0) {
                        // error while fetching sel
                        break;
                }

                time(&tm);
                if ((tm - tm0) > ipmi_handler->fullup_timeout) {
                        err("timeout on waiting for discovery. "
                                "SDR_read_done = %d;"
                                "scan_done = %d; mc_count = %d",
                                ipmi_handler->SDRs_read_done,
                                ipmi_handler->bus_scan_done,
                                ipmi_handler->mc_count);
                        return SA_ERR_HPI_NO_RESPONSE;
                }
        } // while (! ipmi_handler->fully_up)

	// BJS: Why are we doing this some more?
        while(rv == 1) {
                rv = sel_select(ipmi_handler->ohoi_sel, NULL, 0 , NULL, NULL);
        }
        if (rv != 0) {
                err("failed to scan SEL. error = %d", rv);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
	if (!ipmi_handler->updated) {
        	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		return 0;
	}
	ipmi_handler->updated = 0;
	// BJS: Loop over all rpt_entries (doing what?)
        rpt_entry = oh_get_resource_next(handler->rptcache, SAHPI_FIRST_ENTRY);
        while (rpt_entry) {
                res_info = oh_get_resource_data(handler->rptcache,
                        rpt_entry->ResourceId);
                trace_ipmi_resources(rpt_entry, res_info);
		// If this rpt has already been processed, skip it
                if (res_info->updated == 0) {
                        rpt_entry = oh_get_resource_next(handler->rptcache,
                                rpt_entry->ResourceId);
                        continue;
                }
		// If this rpt has been deleted, skip it
                if (res_info->deleted) {
			// We have already sent the event
                        rpt_entry = oh_get_resource_next(handler->rptcache,
                                rpt_entry->ResourceId);
                        continue;
                }
                event = malloc(sizeof(*event));
		if (event == NULL) {
			err("Out of memory");
        		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
			return SA_ERR_HPI_OUT_OF_MEMORY;
		}
                memset(event, 0, sizeof(*event));

                if (res_info->presence == 1) {
                        /* Add all RDRs of this RPTe */
                        rdr_entry = oh_get_rdr_next(handler->rptcache,
                                                        rpt_entry->ResourceId,
                                                        SAHPI_FIRST_ENTRY);
                        while (rdr_entry) {
                                event->rdrs = g_slist_append(event->rdrs,
                                	g_memdup(rdr_entry, sizeof(SaHpiRdrT)));
                                rdr_entry = oh_get_rdr_next(handler->rptcache,
                                        rpt_entry->ResourceId,
                                        rdr_entry->RecordId);
                        }
                }

		SaHpiEventUnionT *u = &event->event.EventDataUnion;
		if (rpt_entry->ResourceCapabilities & SAHPI_CAPABILITY_FRU) {
			event->event.EventType = SAHPI_ET_HOTSWAP;
			if (res_info->presence) {
				u->HotSwapEvent.HotSwapState =
					SAHPI_HS_STATE_ACTIVE;
				u->HotSwapEvent.PreviousHotSwapState =
					SAHPI_HS_STATE_ACTIVE;
			} else {
				u->HotSwapEvent.HotSwapState =
					SAHPI_HS_STATE_NOT_PRESENT;
				u->HotSwapEvent.PreviousHotSwapState =
					SAHPI_HS_STATE_ACTIVE;
			}
		} else {
			event->event.EventType = SAHPI_ET_RESOURCE;
			if (res_info->presence) {
				u->ResourceEvent.ResourceEventType =
					SAHPI_RESE_RESOURCE_ADDED;
			} else {
				u->ResourceEvent.ResourceEventType =
					SAHPI_RESE_RESOURCE_FAILURE;
			}
		}
		event->event.Source = rpt_entry->ResourceId;
		oh_gettimeofday(&event->event.Timestamp);
		event->event.Severity = rpt_entry->ResourceSeverity;
		event->resource = *rpt_entry;
                event->hid = handler->hid;
                oh_evt_queue_push(handler->eventq, event);

		// We're now done with this rpt
                res_info->updated = 0;
                rpt_entry = oh_get_resource_next(handler->rptcache,
                        rpt_entry->ResourceId);
        } // while (rpt_entry)
        g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
        return 0;
}

/**
 * ipmi_get_el_info: get ipmi SEL info
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
static SaErrorT ipmi_get_el_info(void               *hnd,
                             SaHpiResourceIdT   id,
                             SaHpiEventLogInfoT      *info)
{
        unsigned int count;
        unsigned int size;
        SaErrorT rv;
        char del_support;
        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;
        const struct ohoi_resource_info *ohoi_res_info;

        while (0 == ipmi_handler->fully_up) {
                rv = sel_select(ipmi_handler->ohoi_sel, NULL, 0 , NULL, NULL);
                if (rv<0) {
                        err("error on waiting for SEL");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }
        }

        ohoi_res_info = oh_get_resource_data(handler->rptcache, id);
        if (!(ohoi_res_info->type & OHOI_RESOURCE_MC)) {
                err("BUG: try to get sel in unsupported resource");
                return SA_ERR_HPI_INVALID_CMD;
        }

        ohoi_get_sel_count(ohoi_res_info->u.entity.mc_id, (int *)(&count));
        info->Entries = count;

        ohoi_get_sel_size(ohoi_res_info->u.entity.mc_id, (int *)(&size));
        info->Size = size / 16;
        ohoi_get_sel_updatetime(ohoi_res_info->u.entity.mc_id,
                                        &info->UpdateTimestamp);
        ohoi_get_sel_time(ohoi_res_info->u.entity.mc_id, &info->CurrentTime,
                                                                ipmi_handler);
        ohoi_get_sel_overflow(ohoi_res_info->u.entity.mc_id, (char *)(&info->OverflowFlag));
        info->OverflowAction = SAHPI_EL_OVERFLOW_DROP;
        ohoi_get_sel_support_del(ohoi_res_info->u.entity.mc_id, &del_support);
        rv = ohoi_get_sel_state(ipmi_handler, ohoi_res_info->u.entity.mc_id,
	/* compile error */
//                                                 (int *)&info->Enabled);
                                                 (int *)(void *)&info->Enabled);
        if (rv != SA_OK) {
                err("couldn't get sel state rv = %d", rv);
                return rv;
        }
        info->UserEventMaxSize = 0;
        return SA_OK;
}

/**
 * ipmi_set_el_time: set ipmi event log time
 * @hnd: pointer to handler
 * @id: resource id of resource holding sel
 * @time: pointer to time structure
 *
 * This functions set the clocl in the event log.
 *
 *
 * Return value: 0 for success, -1 for error
 **/
static int ipmi_set_el_time(void               *hnd,
                             SaHpiResourceIdT   id,
                             SaHpiTimeT    time)
{
        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;

        struct ohoi_resource_info *ohoi_res_info;
        struct timeval tv;

        dbg("sel_set_time called");

        ohoi_res_info = oh_get_resource_data(handler->rptcache, id);
        if (!(ohoi_res_info->type & OHOI_RESOURCE_MC)) {
                err("BUG: try to get sel in unsupported resource");
                return SA_ERR_HPI_INVALID_CMD;
        }

        tv.tv_sec = time/1000000000;
        tv.tv_usec= (time%1000000000)/1000;
        ohoi_set_sel_time(ohoi_res_info->u.entity.mc_id, &tv, ipmi_handler);
        return 0;
}


/**
 * ipmi_set_sel_state: set ipmi sel state (enabled)
 * @hnd: pointer to handler
 * @id: resource id of resource with SEL capability
 * @enable: int value for enable
 *
 *
 *
 * Return value: SA_OK for success, SA_ERR_HPI_....  for error
 **/
static SaErrorT ipmi_set_sel_state(void      *hnd,
                SaHpiResourceIdT   id,
                SaHpiBoolT                    enable)
{
        struct ohoi_resource_info *ohoi_res_info;
        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;

        ohoi_res_info = oh_get_resource_data(handler->rptcache, id);
        if (!(ohoi_res_info->type & OHOI_RESOURCE_MC)) {
                err("BUG: try to set sel state in unsupported resource");
                return SA_ERR_HPI_CAPABILITY;
        }
        return ohoi_set_sel_state(ipmi_handler, ohoi_res_info->u.entity.mc_id, enable);
}




typedef struct {
        ipmi_sensor_id_t        *sid;
        ipmi_event_t            *event;
} get_event_sid_t;


static void _get_event_sid(ipmi_mc_t *mc, void *cb_data)
{
         get_event_sid_t *info = cb_data;
        *info->sid = ipmi_event_get_generating_sensor_id(
                ipmi_mc_get_domain(mc), mc, info->event);
}


/**
 * ipmi_get_el_entry: get IPMI SEL entry
 * @hnd: pointer to handler instance
 * @id: resourde id with SEL capability
 * @current: SaHpiEntryIdT of entry to retrieve
 * @prev: previous entry in log relative to current
 * @next: next entry in log
 * @entry: [out]SaHpiEventLogEntryT entry requested
 *
 * This function will get event(s) from SEL capable IPMI device
 * one at a time by the record id starting with HPI's
 * SAHPI_OLDEST_ENTRY or SAHPI_NEWEST_ENTRY.
 *
 * Return value: SA_OK for success -1 for failure
 **/
static int ipmi_get_el_entry(void *hnd, SaHpiResourceIdT id,
                                SaHpiEventLogEntryIdT current,
                                SaHpiEventLogEntryIdT *prev,
                                SaHpiEventLogEntryIdT *next,
                                SaHpiEventLogEntryT *entry,
                                SaHpiRdrT  *rdr,
                                SaHpiRptEntryT  *rptentry)
{
        struct ohoi_resource_info *ohoi_res_info;
        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        ipmi_event_t            *event;
        SaHpiRptEntryT          *myrpt;
        SaHpiRdrT               *myrdr;
        SaHpiEventTypeT         event_type;
        struct oh_event         *e;
        char                    Data[IPMI_EVENT_DATA_MAX_LEN];
        int                     data_len;
        ipmi_sensor_id_t        sid;
        ipmi_entity_id_t        et;
        get_event_sid_t         info;

        ohoi_res_info = oh_get_resource_data(handler->rptcache, id);
        if (!(ohoi_res_info->type & OHOI_RESOURCE_MC)) {
                err("BUG: try to get sel in unsupported resource");
                return SA_ERR_HPI_INVALID_CMD;
        }

        if (rdr)
                rdr->RdrType = SAHPI_NO_RECORD;
        if (rptentry) {
                rptentry->ResourceCapabilities = 0;
                rptentry->ResourceId = SAHPI_UNSPECIFIED_RESOURCE_ID;
        }

        switch (current) {
                case SAHPI_OLDEST_ENTRY:
                        ohoi_get_sel_first_entry(
                                ohoi_res_info->u.entity.mc_id, &event);
                        if (!event)
                                return SA_ERR_HPI_NOT_PRESENT;

                        ohoi_get_sel_next_recid(ohoi_res_info->u.entity.mc_id,
                                                event, next);

                        *prev = SAHPI_NO_MORE_ENTRIES;

                        break;

                case SAHPI_NEWEST_ENTRY:

                        ohoi_get_sel_last_entry(ohoi_res_info->u.entity.mc_id,
                                        &event);
                        if (!event)
                                return SA_ERR_HPI_NOT_PRESENT;

                        *next = SAHPI_NO_MORE_ENTRIES;

                        ohoi_get_sel_prev_recid(ohoi_res_info->u.entity.mc_id,
                                                event, prev);

                        break;

                case SAHPI_NO_MORE_ENTRIES:
                        err("SEL is empty!");
                        if (!event)
                                return SA_ERR_HPI_INVALID_PARAMS;

                default:
                        /* get the entry requested by id */
                        ohoi_get_sel_by_recid(ohoi_res_info->u.entity.mc_id,
                                                current, &event);
                        if (!event)
                                return SA_ERR_HPI_NOT_PRESENT;
                        ohoi_get_sel_next_recid(ohoi_res_info->u.entity.mc_id,
                                                event, next);

                        ohoi_get_sel_prev_recid(ohoi_res_info->u.entity.mc_id,
                                                event, prev);

                        break;

        }

        entry->EntryId = ipmi_event_get_record_id(event);
        event_type = ipmi_event_get_type(event);
        data_len = ipmi_event_get_data(event, (unsigned char *)Data,
                                        0, IPMI_EVENT_DATA_MAX_LEN);



        if (event_type == 0x02) {   // sensor event
		if (rptentry) {
			rptentry->ResourceCapabilities = 0;
		}
		if (rdr) {
			rdr->RdrType = SAHPI_NO_RECORD;
		}
                info.sid = &sid;
                info.event = event;
                ipmi_mc_pointer_cb(ohoi_res_info->u.entity.mc_id,
                                _get_event_sid, &info);

                trace_ipmi_sensors("LOOK FOR", sid);
                if (ohoi_sensor_ipmi_event_to_hpi_event(handler->data,
							sid, event, &e, &et)) {
			//we will handle sensor event as user event
			goto no_sensor_event;
		}
                myrpt = ohoi_get_resource_by_entityid(handler->rptcache, &et);
		if (myrpt == NULL) {
			goto no_rpt;
		}
                myrdr = ohoi_get_rdr_by_data(handler->rptcache,
                                 myrpt->ResourceId, SAHPI_SENSOR_RDR, &sid);
                e->event.Source = myrpt->ResourceId;
                if (rptentry) {
                        memcpy(rptentry, myrpt, sizeof (*myrpt));
                }
                if (myrdr) {
                        e->event.EventDataUnion.SensorEvent.SensorNum =
                        	myrdr->RdrTypeUnion.SensorRec.Num;
			if (rdr) {
                        	memcpy(rdr, myrdr, sizeof (*myrdr));
			}
                }
no_rpt:
                memcpy(&entry->Event, &e->event, sizeof (SaHpiEventT));
                oh_event_free(e, FALSE);
                entry->Event.EventType = SAHPI_ET_SENSOR;
                entry->Timestamp = ipmi_event_get_timestamp(event);
                return SA_OK;
        }

no_sensor_event:

        entry->Event.Source = SAHPI_UNSPECIFIED_RESOURCE_ID;

        if (data_len != 13) {
                err("Strange data len in ipmi event: %d instead of 13\n",
                                  data_len);
                return SA_ERR_HPI_ERROR;
        }


        if ((event_type >= 0xC0) && (event_type <= 0xDF)) {
                // OEM timestamp event type
                entry->Timestamp = ipmi_event_get_timestamp(event);
                entry->Event.EventType = SAHPI_ET_OEM;
                entry->Event.Timestamp = entry->Timestamp;
                entry->Event.EventDataUnion.OemEvent.MId = Data[4] |
                                                     (Data[5] << 8) |
                                                     (Data[6] << 16);
                entry->Event.Severity = SAHPI_DEBUG; // FIX ME
                entry->Event.EventDataUnion.OemEvent.
                       OemEventData.DataLength = 6;
                memcpy(entry->Event.EventDataUnion.OemEvent.
                     OemEventData.Data, Data + 7, data_len);
                entry->Event.EventDataUnion.OemEvent.OemEventData.
                      DataType = SAHPI_TL_TYPE_BINARY;
                entry->Event.EventDataUnion.OemEvent.OemEventData.
                      Language = SAHPI_LANG_UNDEF;
                return SA_OK;
        };

        entry->Event.Source = SAHPI_UNSPECIFIED_RESOURCE_ID;
        entry->Event.EventType = SAHPI_ET_USER;
        oh_gettimeofday(&entry->Event.Timestamp);
//      entry->Event.Timestamp = SAHPI_TIME_UNSPECIFIED;
        entry->Event.Severity = SAHPI_DEBUG; // FIX ME
        entry->Event.EventDataUnion.UserEvent.UserEventData.DataType =
                                SAHPI_TL_TYPE_BINARY;
        entry->Event.EventDataUnion.UserEvent.UserEventData.Language =
                                SAHPI_LANG_UNDEF;
        entry->Event.EventDataUnion.UserEvent.UserEventData.DataLength =
                               ipmi_event_get_data_len(event);
        memcpy(entry->Event.EventDataUnion.UserEvent.UserEventData.Data,
               Data, data_len);
        return SA_OK;
}


static SaErrorT ipmi_clear_el(void *hnd, SaHpiResourceIdT id)
{
        struct ohoi_resource_info *ohoi_res_info;
        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;

                int rv;
                int i;

        ohoi_res_info = oh_get_resource_data(handler->rptcache, id);
        if (!(ohoi_res_info->type & OHOI_RESOURCE_MC)) {
                err("BUG: try to get sel in unsupported resource");
                return SA_ERR_HPI_INVALID_CMD;
        }

        ipmi_handler->sel_clear_done = 0;

        rv = ohoi_clear_sel(ohoi_res_info->u.entity.mc_id, ipmi_handler);

        if (rv != SA_OK) {
                err("Error in attempting to clear sel");
                return rv;
        }


        for (i =0; i < 6; i++) {
                /* long wait - 1 min */
                rv = ohoi_loop(&ipmi_handler->sel_clear_done, ipmi_handler);
                if (rv == SA_OK) {
                        break;
                }
        }
        return rv;
}



#define SENSOR_CHECK(handler, sensor_info, id, num)                           \
do {                                                                          \
        SaErrorT         rv;                                                  \
        SaHpiRdrT *rdr;                                                       \
                                                                              \
        rdr = oh_get_rdr_by_type(handler->rptcache, id,                       \
                                 SAHPI_SENSOR_RDR, num);                      \
        if (!rdr) {                                                           \
                err("no rdr");                                                \
                return SA_ERR_HPI_NOT_PRESENT;                                \
        }                                                                     \
                                                                              \
        rv = ohoi_get_rdr_data(handler, id, SAHPI_SENSOR_RDR, num,            \
                                                (void *)&sensor_info);        \
        if (rv != SA_OK)                                                      \
                return rv;                                                    \
                                                                              \
        if (!sensor_info)                                                     \
                return SA_ERR_HPI_NOT_PRESENT;                                \
                                                                              \
} while (0)

#define CHECK_SENSOR_SEN_ENABLE(sensor_info)                                      \
do {                                                                          \
        if (sensor_info->sen_enabled == SAHPI_FALSE)                          \
                return SA_ERR_HPI_INVALID_REQUEST;                            \
} while (0)


static int ipmi_get_sensor_event_enable(void *hnd,
					SaHpiResourceIdT id,
					SaHpiSensorNumT num,
					SaHpiBoolT *enable)

{
        SaErrorT		rv;
        struct oh_handler_state	*handler = (struct oh_handler_state *)hnd;
        struct ohoi_sensor_info	*sensor_info;
        SaHpiBoolT		t_enable;
        SaHpiEventStateT	t_assert;
        SaHpiEventStateT	t_deassert;

        SENSOR_CHECK(handler, sensor_info, id, num);

        if (!enable)
                return SA_ERR_HPI_INVALID_PARAMS;

        rv = ohoi_get_sensor_event_enable(hnd, sensor_info, &t_enable,
					  &t_assert, &t_deassert);
        if (rv)
                return rv;

        if (sensor_info->sen_enabled) {
		sensor_info->enable = t_enable;
        	sensor_info->assert = t_assert;
        	sensor_info->deassert = t_deassert;
	}

        *enable = t_enable;
        return SA_OK;
}

static int ipmi_set_sensor_event_enable(void *hnd,
                                        SaHpiResourceIdT id,
                                        SaHpiSensorNumT num,
                                        const SaHpiBoolT enable)
{

        SaErrorT		rv;
        struct oh_handler_state	*handler = (struct oh_handler_state *)hnd;
        struct ohoi_sensor_info	*sensor_info;
        struct oh_event		*e;
        SaHpiRdrT		*rdr = NULL;
        SaHpiRptEntryT		*rpte = NULL;
        SaHpiSensorEnableChangeEventT	*sen_evt;

        SENSOR_CHECK(handler, sensor_info, id, num);

        rv = ohoi_set_sensor_event_enable(hnd, sensor_info,
					  enable, sensor_info->assert,
					  sensor_info->deassert,
					  sensor_info->support_assert,
					  sensor_info->support_deassert);
        if (rv)
                return rv;

        if (sensor_info->enable == enable)
                return(SA_OK);
        sensor_info->enable = enable;
 //       sensor_info->saved_enable = enable;
        e = malloc(sizeof(*e));
        if (!e) {
                err("Out of space");
                return IPMI_EVENT_NOT_HANDLED;
        }
        memset(e, 0, sizeof(*e));

        rpte = oh_get_resource_by_id(handler->rptcache, id);
        if (rpte) {
        	e->resource = *rpte;
        }

        rdr = oh_get_rdr_by_type(handler->rptcache, id, SAHPI_SENSOR_RDR, num);
        if (!rdr) {
                err("no rdr");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        e->event.Source = id;
        e->event.EventType = SAHPI_ET_SENSOR_ENABLE_CHANGE;
        e->event.Severity = SAHPI_INFORMATIONAL;
        oh_gettimeofday(&e->event.Timestamp);
        e->rdrs = g_slist_append(e->rdrs, g_memdup(rdr, sizeof(SaHpiRdrT)));
//      e->u.hpi_event.event.Timestamp = SAHPI_TIME_UNSPECIFIED;
        sen_evt = &(e->event.EventDataUnion.SensorEnableChangeEvent);
        sen_evt->SensorNum = num;
        sen_evt->SensorType = rdr->RdrTypeUnion.SensorRec.Type;
        sen_evt->EventCategory = rdr->RdrTypeUnion.SensorRec.Category;
        sen_evt->SensorEnable = sensor_info->enable;
        sen_evt->SensorEventEnable = sensor_info->enable;
        sen_evt->AssertEventMask = sensor_info->assert;
        sen_evt->DeassertEventMask = sensor_info->deassert;
        e->hid = handler->hid;
        oh_evt_queue_push(handler->eventq, e);
        return SA_OK;

}



/**
 * ipmi_get_sensor_reading: get sensor reading, type, category and other info.
 * @hnd: pointer to handler instance
 * @id: ResourceId -- parent of this sensor
 * @num: sensor number
 * @data: struct returned with data about the sensor.
 *
 *
 *
 * Return value: 0 for success or negative for error
 **/
static int ipmi_get_sensor_reading(void   *hnd,
                                   SaHpiResourceIdT  id,
                                   SaHpiSensorNumT  num,
                                   SaHpiSensorReadingT *reading,
                                   SaHpiEventStateT  *ev_state)
{
        SaErrorT		rv;
        struct oh_handler_state	*handler = (struct oh_handler_state *)hnd;
        struct ohoi_sensor_info	*sensor_info;
        SaHpiSensorReadingT	tmp_reading;
        SaHpiEventStateT	tmp_state;

        SENSOR_CHECK(handler, sensor_info, id, num);
        CHECK_SENSOR_SEN_ENABLE(sensor_info);

        rv = ohoi_get_sensor_reading(hnd, sensor_info,
				     &tmp_reading, &tmp_state);
        if (rv)
                return rv;

        if (reading)
                *reading = tmp_reading;
        if (ev_state)
                *ev_state = tmp_state;

        return SA_OK;
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
static int ipmi_get_sensor_thresholds(void *hnd,
                                      SaHpiResourceIdT id,
                                      SaHpiSensorNumT num,
                                      SaHpiSensorThresholdsT *thres)
{
        struct oh_handler_state	*handler = (struct oh_handler_state *)hnd;
        struct ohoi_sensor_info	*sensor_info;

        SENSOR_CHECK(handler, sensor_info, id, num);
//      CHECK_SENSOR_ENABLE(sensor_info);

        if (!thres)
                return SA_ERR_HPI_INVALID_PARAMS;

        memset(thres, 0, sizeof(*thres));
        return ohoi_get_sensor_thresholds(hnd, sensor_info, thres);
}

static int ipmi_set_sensor_thresholds(void                              *hnd,
                                      SaHpiResourceIdT                  id,
                                      SaHpiSensorNumT                   num,
                                      const SaHpiSensorThresholdsT      *thres)
{
        struct oh_handler_state	*handler = (struct oh_handler_state *)hnd;
        struct ohoi_sensor_info	*sensor_info;

        SENSOR_CHECK(handler, sensor_info, id, num);
//      CHECK_SENSOR_ENABLE(sensor_info);

        if (!thres)
                return SA_ERR_HPI_INVALID_PARAMS;

        return ohoi_set_sensor_thresholds(hnd, sensor_info, thres);
}

static int ipmi_get_sensor_enable(void *hnd, SaHpiResourceIdT id,
                                  SaHpiSensorNumT num,
                                  SaHpiBoolT *enable)
{
        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        struct ohoi_sensor_info *sensor_info;

        SENSOR_CHECK(handler, sensor_info, id, num);

        if (!enable)
                return SA_ERR_HPI_INVALID_PARAMS;

        *enable = sensor_info->sen_enabled;
        return SA_OK;
}


static int ipmi_set_sensor_enable(void *hnd, SaHpiResourceIdT id,
                                  SaHpiSensorNumT num,
                                  SaHpiBoolT enable)
{

        struct oh_handler_state	*handler = (struct oh_handler_state *)hnd;
        struct ohoi_sensor_info	*sensor_info;
        SaErrorT		rv;

        SENSOR_CHECK(handler, sensor_info, id, num);

        if (sensor_info->sen_enabled == enable) {
                return SA_OK;
        }

        if (enable == SAHPI_FALSE) {
//                sensor_info->saved_enable = sensor_info->enable;
                if (sensor_info->enable == SAHPI_FALSE) {
                        sensor_info->sen_enabled = SAHPI_FALSE;
                        return SA_OK;
                }
       		 // Disable events
		rv = ohoi_set_sensor_event_enable(hnd, sensor_info,
					  SAHPI_FALSE, sensor_info->assert,
					  sensor_info->deassert,
					  sensor_info->support_assert,
					  sensor_info->support_deassert);
                if (rv == SA_OK) {
                        sensor_info->sen_enabled = SAHPI_FALSE;
                };
                return rv;
        }
        // enable == SAHPI_TRUE
        if (sensor_info->enable == SAHPI_FALSE) {
                sensor_info->sen_enabled = SAHPI_TRUE;
                return SA_OK;
        }
        // Restore events
	rv = ohoi_set_sensor_event_enable(hnd, sensor_info,
					  SAHPI_TRUE, sensor_info->assert,
					  sensor_info->deassert,
					  sensor_info->support_assert,
					  sensor_info->support_deassert);
        if (rv != SA_OK) {
		err("ipmi_set_sensor_event_enable = %d", rv);
		sensor_info->enable = SAHPI_FALSE;
        }
	sensor_info->sen_enabled = SAHPI_TRUE;
        return rv;
}


static int ipmi_get_sensor_event_masks(void *hnd,
				       SaHpiResourceIdT id,
				       SaHpiSensorNumT  num,
				       SaHpiEventStateT *assert,
				       SaHpiEventStateT *deassert)
{

        SaErrorT		rv;
        struct oh_handler_state	*handler = (struct oh_handler_state *)hnd;
        struct ohoi_sensor_info	*sensor_info;
        SaHpiBoolT		t_enable;
        SaHpiEventStateT	t_assert;
        SaHpiEventStateT	t_deassert;

        SENSOR_CHECK(handler, sensor_info, id, num);

        if (!assert || !deassert)
                return SA_ERR_HPI_INVALID_PARAMS;

        rv = ohoi_get_sensor_event_enable(hnd, sensor_info,
					  &t_enable, &t_assert, &t_deassert);
        if (rv)
                return rv;


	if (sensor_info->sen_enabled) {
		sensor_info->enable = t_enable;
        	sensor_info->assert = t_assert;
        	sensor_info->deassert = t_deassert;
	}

        *assert = t_assert;
        *deassert = t_deassert;

        return SA_OK;
}

static int ipmi_set_sensor_event_masks(void *hnd, SaHpiResourceIdT id,
                                       SaHpiSensorNumT num,
                                       SaHpiSensorEventMaskActionT act,
                                       SaHpiEventStateT  assert,
                                       SaHpiEventStateT  deassert)
{
        SaErrorT		rv;
        struct oh_handler_state	*handler = (struct oh_handler_state *)hnd;
        struct ohoi_sensor_info	*sensor_info;
        SaHpiEventStateT	t_assert;
        SaHpiEventStateT	t_deassert;
        struct oh_event 	*e;
        SaHpiRptEntryT		*rpte = NULL;
        SaHpiRdrT		*rdr = NULL;
        SaHpiSensorEnableChangeEventT	*sen_evt;

        SENSOR_CHECK(handler, sensor_info, id, num);

        if (act == SAHPI_SENS_ADD_EVENTS_TO_MASKS) {
                t_assert = assert | sensor_info->assert;
                t_deassert = deassert | sensor_info->deassert;
        } else if (act == SAHPI_SENS_REMOVE_EVENTS_FROM_MASKS) {
                t_assert = (assert ^ 0xffff) & sensor_info->assert;
                t_deassert = (deassert ^ 0xffff) & sensor_info->deassert;
        } else
                return SA_ERR_HPI_INVALID_PARAMS;

        rv = ohoi_set_sensor_event_enable(hnd, sensor_info,
					  sensor_info->enable,
					  t_assert, t_deassert,
					  sensor_info->support_assert,
					  sensor_info->support_deassert);
        if (rv)
                return rv;

        if ((sensor_info->assert == t_assert) &&
                (sensor_info->deassert == t_deassert))
                return (SA_OK);
        sensor_info->assert = t_assert;
        sensor_info->deassert = t_deassert;
        e = malloc(sizeof(*e));
        if (!e) {
                err("Out of space");
                return IPMI_EVENT_NOT_HANDLED;
        }
        memset(e, 0, sizeof(*e));

	rpte = oh_get_resource_by_id(handler->rptcache, id);
	if (rpte) {
		e->resource = *rpte;
	}

        rdr = oh_get_rdr_by_type(handler->rptcache, id, SAHPI_SENSOR_RDR, num);
        if (!rdr) {
                err("no rdr");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        e->event.Source = id;
        e->event.EventType = SAHPI_ET_SENSOR_ENABLE_CHANGE;
        e->event.Severity = SAHPI_INFORMATIONAL;
        oh_gettimeofday(&e->event.Timestamp);
        e->rdrs = g_slist_append(e->rdrs, g_memdup(rdr, sizeof(SaHpiRdrT)));
//      e->u.hpi_event.event.Timestamp = SAHPI_TIME_UNSPECIFIED;
        sen_evt = &(e->event.EventDataUnion.SensorEnableChangeEvent);
        sen_evt->SensorNum = num;
        sen_evt->SensorType = rdr->RdrTypeUnion.SensorRec.Type;
        sen_evt->EventCategory = rdr->RdrTypeUnion.SensorRec.Category;
        sen_evt->SensorEnable = sensor_info->enable;
        sen_evt->SensorEventEnable = sensor_info->enable;
        sen_evt->AssertEventMask = sensor_info->assert;
        sen_evt->DeassertEventMask = sensor_info->deassert;
        e->hid = handler->hid;
        oh_evt_queue_push(handler->eventq, e);
        return SA_OK;
}






	/*
	 *          WATCHDOG FUNCTIONS
	 */






static int ipmi_get_watchdog_info(void *hnd,
         SaHpiResourceIdT  id,
         SaHpiWatchdogNumT num,
         SaHpiWatchdogT    *watchdog)
{
        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        struct ohoi_handler *ipmi_handler =(struct ohoi_handler *)handler->data;
        unsigned char reqdata[16];
        unsigned char response[16];
        int rlen;
        int rv;

        /*
         * OpenIPMI library doesn't have watchdog calls, so talk
         * directly to the driver (via ipmi_drv.c).
         * Currently support only default watchdog num, and only local.
         */
        if (ipmi_handler->islan) return(SA_ERR_HPI_UNSUPPORTED_API);
        if (num != SAHPI_DEFAULT_WATCHDOG_NUM) {
		err("num = %d", num);
                return SA_ERR_HPI_INVALID_PARAMS;
	}
        rlen = sizeof(response);
	memset(reqdata, 0, 16);
	memset(response, 0, 16);
        rv = ipmicmd_mv(ipmi_handler, WATCHDOG_GET, NETFN_APP, 0,
				reqdata, 0, response, rlen, &rlen);
        if (rv != 0) return(rv);

        dbg("wdog_get: %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
		response[0],
                response[1], response[2], response[3], response[4],
                response[5], response[6], response[7], response[8]);

        rv = response[0];  /*completion code*/
        if (rv != 0) {
		rv |= IPMI_IPMI_ERR_TOP;
		OHOI_MAP_ERROR(rv, rv);
		return(rv);
	}

        /* Translate IPMI response to HPI format */
        memset(watchdog, 0, sizeof(SaHpiWatchdogT));
        if (response[1] & 0x80)
                watchdog->Log = SAHPI_FALSE;
        else
                watchdog->Log = SAHPI_TRUE;

        if (response[1] & 0x40)
                watchdog->Running = SAHPI_TRUE;
        else
                watchdog->Running = SAHPI_FALSE;

        switch(response[1] & 0x07) {
                case 0x01:
                        watchdog->TimerUse = SAHPI_WTU_BIOS_FRB2;
                        break;
                case 0x02:
                        watchdog->TimerUse = SAHPI_WTU_BIOS_POST;
                        break;
                case 0x03:
                         watchdog->TimerUse = SAHPI_WTU_OS_LOAD;
                         break;
                case 0x04:
                         watchdog->TimerUse = SAHPI_WTU_SMS_OS;
                         break;
                case 0x05:
                         watchdog->TimerUse = SAHPI_WTU_OEM;
                         break;
                default:
                         watchdog->TimerUse = SAHPI_WTU_UNSPECIFIED;
                         break;
        }

        switch (response[2] & 0x70)
        {
        case 0x00:
            watchdog->PretimerInterrupt = SAHPI_WPI_NONE;
            break;
        case 0x10:
            watchdog->PretimerInterrupt = SAHPI_WPI_SMI;
            break;
        case 0x20:
            watchdog->PretimerInterrupt = SAHPI_WPI_NMI;
            break;
        case 0x30:
            watchdog->PretimerInterrupt = SAHPI_WPI_MESSAGE_INTERRUPT;
            break;
        default :
            watchdog->PretimerInterrupt = SAHPI_WPI_NONE;
            // watchdog->PretimerInterrupt = SAHPI_WPI_OEM;
            break;
        }

        switch (response[2] & 0x07)
        {
        case 0x00:
            watchdog->TimerAction = SAHPI_WA_NO_ACTION;
            break;
        case 0x01:
            watchdog->TimerAction = SAHPI_WA_RESET;
            break;
        case 0x02:
            watchdog->TimerAction = SAHPI_WA_POWER_DOWN;
            break;
        case 0x03:
            watchdog->TimerAction = SAHPI_WA_POWER_CYCLE;
            break;
        default:
            watchdog->TimerAction = SAHPI_WA_NO_ACTION;
            break;
        }

        watchdog->PreTimeoutInterval =
                response[3] * IPMI_WD_PRETIMER_RESOLUTION;

        watchdog->TimerUseExpFlags = 0;
        if (response[4] & 0x02)
           watchdog->TimerUseExpFlags |= SAHPI_WATCHDOG_EXP_BIOS_FRB2;
        if (response[4] & 0x04)
           watchdog->TimerUseExpFlags |= SAHPI_WATCHDOG_EXP_BIOS_POST;
        if (response[4] & 0x08)
           watchdog->TimerUseExpFlags |= SAHPI_WATCHDOG_EXP_OS_LOAD;
        if (response[4] & 0x10)
           watchdog->TimerUseExpFlags |= SAHPI_WATCHDOG_EXP_SMS_OS;
        if (response[4] & 0x20)
           watchdog->TimerUseExpFlags |= SAHPI_WATCHDOG_EXP_OEM;

        watchdog->InitialCount = (response[5] + (response[6] * 256)) * IPMI_WD_RESOLUTION;
        watchdog->PresentCount = (response[7] + (response[8] * 256)) * IPMI_WD_RESOLUTION;

        return SA_OK;
}

static int ipmi_set_watchdog_info(void *hnd,
         SaHpiResourceIdT  id,
         SaHpiWatchdogNumT num,
         SaHpiWatchdogT    *watchdog)
{
        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        struct ohoi_handler *ipmi_handler =(struct ohoi_handler *)handler->data;
        unsigned char reqdata[16];
        unsigned char response[16];
        int rlen;
        int tv;
        int rv;

        if (ipmi_handler->islan) return(SA_ERR_HPI_UNSUPPORTED_API);

        if (num != SAHPI_DEFAULT_WATCHDOG_NUM) {
		err("watchdog num = %d", num);
                return SA_ERR_HPI_INVALID_PARAMS;
	}
        /* translate HPI values to IPMI */
        switch (watchdog->TimerUse)
        {
        case SAHPI_WTU_BIOS_FRB2:
            reqdata[0] = 0x01;
            break;
        case SAHPI_WTU_BIOS_POST:
            reqdata[0] = 0x02;
            break;
        case SAHPI_WTU_OS_LOAD:
            reqdata[0] = 0x03;
            break;
        case SAHPI_WTU_SMS_OS:
            reqdata[0] = 0x04;
            break;
        case SAHPI_WTU_OEM:
            reqdata[0] = 0x05;
            break;
        case SAHPI_WTU_NONE:
        default:
            reqdata[0] = 0x00;
        }
        if (watchdog->Log == SAHPI_FALSE)    reqdata[0] |= 0x80;;
        if (watchdog->Running == SAHPI_TRUE) reqdata[0] |= 0x40;;

        switch (watchdog->TimerAction)
        {
        case SAHPI_WA_RESET:
            reqdata[1] = 0x01;
            break;
        case SAHPI_WA_POWER_DOWN:
            reqdata[1] = 0x02;
            break;
        case SAHPI_WA_POWER_CYCLE:
            reqdata[1] = 0x03;
            break;
        case SAHPI_WA_NO_ACTION:
        default:
            reqdata[1] = 0x00;
        }

        switch (watchdog->PretimerInterrupt)
        {
        case SAHPI_WPI_SMI:
            reqdata[1] |= 0x10;
            break;
        case SAHPI_WPI_NMI:
            reqdata[1] |= 0x20;
            break;
        case SAHPI_WPI_MESSAGE_INTERRUPT:
            reqdata[1] |= 0x30;
            break;
        case SAHPI_WPI_OEM:
            /* Undefined, so treat it like SAHPI_WPI_NONE */
        case SAHPI_WPI_NONE:
        default:
            break;
        }

        tv = watchdog->PreTimeoutInterval / IPMI_WD_PRETIMER_RESOLUTION;
        reqdata[2] = tv & 0x00ff;

        reqdata[3] = 0;
        if (watchdog->TimerUseExpFlags & SAHPI_WATCHDOG_EXP_BIOS_FRB2)
                reqdata[3] |= 0x02;
        if (watchdog->TimerUseExpFlags & SAHPI_WATCHDOG_EXP_BIOS_POST)
                reqdata[3] |= 0x04;
        if (watchdog->TimerUseExpFlags & SAHPI_WATCHDOG_EXP_OS_LOAD)
                reqdata[3] |= 0x08;
        if (watchdog->TimerUseExpFlags & SAHPI_WATCHDOG_EXP_SMS_OS)
                reqdata[3] |= 0x10;
        if (watchdog->TimerUseExpFlags & SAHPI_WATCHDOG_EXP_OEM)
                reqdata[3] |= 0x20;

        if ((watchdog->InitialCount < IPMI_WD_RESOLUTION) &&
            (watchdog->InitialCount != 0))
             tv = IPMI_WD_RESOLUTION;
        else tv = watchdog->InitialCount / IPMI_WD_RESOLUTION;
        reqdata[4] = tv & 0x00ff;  // tv % 256;
        reqdata[5] = tv / 256;   // (tv & 0xff00) >> 8;

        dbg("wdog_set: %02x %02x %02x %02x %02x %02x\n",
                reqdata[0], reqdata[1], reqdata[2],
                reqdata[3], reqdata[4], reqdata[5]);

        rlen = sizeof(response);
        rv = ipmicmd_mv(ipmi_handler, WATCHDOG_SET, NETFN_APP, 0,
				reqdata, 6, response, rlen, &rlen);
        if (rv != 0) return(rv);

        rv = response[0];  /*completion code*/
	if (rv != 0) {
		err("wdog_set response: %02x", rv);
		OHOI_MAP_ERROR(rv, rv);
	} else {
		rv = SA_OK;
	}
        return rv;
}

static int ipmi_reset_watchdog(void *hnd,
         SaHpiResourceIdT  id,
         SaHpiWatchdogNumT num)
{
        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        struct ohoi_handler *ipmi_handler =(struct ohoi_handler *)handler->data;
        unsigned char response[16];
        int rlen;
        int rv;

        if (ipmi_handler->islan) return(SA_ERR_HPI_UNSUPPORTED_API);

        if (num != SAHPI_DEFAULT_WATCHDOG_NUM) {
		err("watchdog num = %d", num);
                return SA_ERR_HPI_INVALID_PARAMS;
	}

        rlen = sizeof(response);
        rv = ipmicmd_mv(ipmi_handler, WATCHDOG_RESET, NETFN_APP, 0,
		NULL, 0, response, rlen, &rlen);
        if (rv != 0) return(rv);

        rv = response[0];  /*completion code*/
		if (rv != 0) {
		err("wdog_set response: %02x", rv);
		OHOI_MAP_ERROR(rv, rv);
	} else {
		rv = SA_OK;
	}

        return rv;
}

static
void ohoi_set_resource_tag(ipmi_entity_t *entity, void *cb_data)
{
//              SaHpiTextBufferT *tag = cb_data;
//              ipmi_entity_set_entity_id_string(entity, (char *)tag);
                err("New resource Tag set");
}

static SaErrorT ipmi_set_res_tag (void                  *hnd,
                                  SaHpiResourceIdT      id,
                                  SaHpiTextBufferT      *tag)
{
        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        SaHpiRptEntryT *rpt_entry;
        struct ohoi_resource_info *res_info;
        int rv;

        res_info = oh_get_resource_data(handler->rptcache, id);
        if (!res_info)
                        err("No private resource info for resource %d", id);

        rpt_entry = oh_get_resource_by_id(handler->rptcache, id);
        if (!rpt_entry) {
                err("No rpt for resource %d?", id);
                return  SA_ERR_HPI_NOT_PRESENT;
        }

        /* do it in openIPMI's memory first for subsequest updates */

        /* can only be an Entity in the ohoi_resource_info struct */
        if (res_info->type & OHOI_RESOURCE_ENTITY) {
                dbg("Setting new Tag: %s for resource: %d", (char *) tag->Data, id);
                rv = ipmi_entity_pointer_cb(res_info->u.entity.entity_id, ohoi_set_resource_tag,
                                    tag->Data);
                if (rv)
                        err("Error retrieving entity pointer for resource %d",
                        rpt_entry->ResourceId);
        }

        rpt_entry->ResourceTag.DataType = tag->DataType;
        rpt_entry->ResourceTag.Language = tag->Language;
        rpt_entry->ResourceTag.DataLength = tag->DataLength;

        /* change it in our memory as well */
        memcpy(&rpt_entry->ResourceTag.Data, tag->Data, sizeof(tag->Data));
        oh_add_resource(handler->rptcache, rpt_entry, res_info, 1);
        entity_rpt_set_updated(res_info, handler->data);
        return SA_OK;
}

static SaErrorT ipmi_set_res_sev(void                   *hnd,
                                 SaHpiResourceIdT       res_id,
                                 SaHpiSeverityT         severity)
{
        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
        struct ohoi_handler *ipmi_handler = handler->data;
        struct ohoi_resource_info *res_info;

        SaHpiRptEntryT  *rpt_entry;

        res_info = oh_get_resource_data(handler->rptcache, res_id);
        if (res_info == NULL) {
                err("Failed to retrieve RPT private data");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        rpt_entry = oh_get_resource_by_id(handler->rptcache, res_id);

        if (!rpt_entry) {
                err("Can't find RPT for resource id: %d", res_id);
                return  SA_ERR_HPI_NOT_PRESENT;
        }

        dbg("Current Severity: %d\n", rpt_entry->ResourceSeverity);
        dbg("To be set New Severity: %d\n", severity);

        memcpy(&rpt_entry->ResourceSeverity, &severity, sizeof(severity));

        oh_add_resource(handler->rptcache, rpt_entry, res_info, 1);
        dbg("New Severity: %d\n", rpt_entry->ResourceSeverity);
        entity_rpt_set_updated(res_info, ipmi_handler);
        return SA_OK;
}


void * oh_open (GHashTable *, unsigned int, oh_evt_queue *) __attribute__ ((weak, alias("ipmi_open")));

void * oh_close (void *) __attribute__ ((weak, alias("ipmi_close")));

void * oh_get_event (void *)
                __attribute__ ((weak, alias("ipmi_get_event")));

void * oh_discover_resources (void *)
                __attribute__ ((weak, alias("ipmi_discover_resources")));

void * oh_set_resource_tag (void *, SaHpiResourceIdT, SaHpiTextBufferT *)
                __attribute__ ((weak, alias("ipmi_set_res_tag")));

void * oh_set_resource_severity (void *, SaHpiResourceIdT, SaHpiSeverityT)
                __attribute__ ((weak, alias("ipmi_set_res_sev")));

void * oh_get_el_info (void *, SaHpiResourceIdT, SaHpiEventLogInfoT *)
                __attribute__ ((weak, alias("ipmi_get_el_info")));

void * oh_set_el_time (void *, SaHpiResourceIdT, const SaHpiEventT *)
                __attribute__ ((weak, alias("ipmi_set_el_time")));

void * oh_get_el_entry (void *, SaHpiResourceIdT, const SaHpiEventT *)
                __attribute__ ((weak, alias("ipmi_get_el_entry")));

void * oh_set_el_state (void *, SaHpiResourceIdT , SaHpiBoolT )
                __attribute__ ((weak, alias("ipmi_set_sel_state")));

void * oh_clear_el (void *, SaHpiResourceIdT)
                __attribute__ ((weak, alias("ipmi_clear_el")));


void * oh_get_sensor_reading (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiSensorReadingT *,
                             SaHpiEventStateT    *)
                __attribute__ ((weak, alias("ipmi_get_sensor_reading")));

void * oh_get_sensor_thresholds (void *, SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 SaHpiSensorThresholdsT *)
                __attribute__ ((weak, alias("ipmi_get_sensor_thresholds")));

void * oh_set_sensor_thresholds (void *, SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 const SaHpiSensorThresholdsT *)
                __attribute__ ((weak, alias("ipmi_set_sensor_thresholds")));

void * oh_get_sensor_enable (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiBoolT *)
                __attribute__ ((weak, alias("ipmi_get_sensor_enable")));

void * oh_set_sensor_enable (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiBoolT)
                __attribute__ ((weak, alias("ipmi_set_sensor_enable")));

void * oh_get_sensor_event_enables (void *, SaHpiResourceIdT,
                                    SaHpiSensorNumT,
                                    SaHpiBoolT *)
                __attribute__ ((weak, alias("ipmi_get_sensor_event_enable")));

void * oh_set_sensor_event_enables (void *, SaHpiResourceIdT id, SaHpiSensorNumT,
                                    SaHpiBoolT *)
                __attribute__ ((weak, alias("ipmi_set_sensor_event_enable")));

void * oh_get_sensor_event_masks (void *, SaHpiResourceIdT, SaHpiSensorNumT,
                                  SaHpiEventStateT *, SaHpiEventStateT *)
                __attribute__ ((weak, alias("ipmi_get_sensor_event_masks")));

void * oh_set_sensor_event_masks (void *, SaHpiResourceIdT, SaHpiSensorNumT,
                                  SaHpiSensorEventMaskActionT,
                                  SaHpiEventStateT,
                                  SaHpiEventStateT)
                __attribute__ ((weak, alias("ipmi_set_sensor_event_masks")));

void * oh_get_watchdog_info (void *, SaHpiResourceIdT, SaHpiWatchdogNumT,
                             SaHpiWatchdogT *)
                __attribute__ ((weak, alias("ipmi_get_watchdog_info")));

void * oh_set_watchdog_info (void *, SaHpiResourceIdT, SaHpiWatchdogNumT,
                             SaHpiWatchdogT *)
                __attribute__ ((weak, alias("ipmi_set_watchdog_info")));

void * oh_reset_watchdog (void *, SaHpiResourceIdT , SaHpiWatchdogNumT )
                __attribute__ ((weak, alias("ipmi_reset_watchdog")));

        SaErrorT (*reset_watchdog)(void *hnd, SaHpiResourceIdT id,
                              SaHpiWatchdogNumT num);

