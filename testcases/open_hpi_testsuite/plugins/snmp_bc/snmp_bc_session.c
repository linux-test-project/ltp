/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Renier Morales <renier@openhpi.org>
 *      Steve Sherman <stevees@us.ibm.com>
 */

#include <snmp_bc_plugin.h>
#include <sim_init.h>

#define check_snmp_parm(input_parm) \
do { \
	if (input_parm != NULL) { \
		if ( (strlen(input_parm) == 0) || \
			( g_ascii_strncasecmp(input_parm, "NONE", 4) == 0 )) { \
			input_parm = NULL; \
		} \
	} \
} while(0)


/**
 * snmp_bc_open:
 * @handler_config: Pointer to hash table (passed by infrastructure)
 *
 * Open an SNMP BladeCenter/RSA plugin handler instance.
 *
 * Returns:
 * Plugin handle - normal operation.
 * NULL - on error.
 **/
void *snmp_bc_open(GHashTable *handler_config,
                   unsigned int hid,
                   oh_evt_queue *eventq)
{
        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
        char *hostname, *version, *sec_level, 
		*authtype, *user, *pass, *community, 
		*context_name, *count_per_getbulk, 
		*privacy_passwd, *privacy_protocol;
        char *root_tuple;
	SaErrorT rv;

	if (!handler_config) {
                err("INVALID PARM - NULL handler_config pointer.");
                return NULL;	
	} else if (!hid) {
                err("Bad handler id passed.");
                return NULL;
        } else if (!eventq) {
                err("No event queue was passed.");
                return NULL;
        }

        root_tuple = (char *)g_hash_table_lookup(handler_config, "entity_root");
        if (!root_tuple) {
                err("Cannot find \"entity_root\" configuration parameter.");
                return NULL;
        }

        hostname = (char *)g_hash_table_lookup(handler_config, "host");
        if (!hostname) {
                err("Cannot find \"host\" configuration parameter.");
                return NULL;
        }

        handle = (struct oh_handler_state *)g_malloc0(sizeof(struct oh_handler_state));
        custom_handle = (struct snmp_bc_hnd *)g_malloc0(sizeof(struct snmp_bc_hnd));
        if (!handle || !custom_handle) {
                err("Out of memory.");
                return NULL;
        }

        handle->data = custom_handle;
        handle->config = handler_config;
        handle->hid = hid;
        handle->eventq = eventq;

        /* Initialize the lock */
        /* g_static_rec_mutex_init(&handle->handler_lock); */
	g_static_rec_mutex_init(&custom_handle->snmp_bc_hlock.lock);
	custom_handle->snmp_bc_hlock.count = 0;
	
        /* Initialize resource masks */
	/* Set all masks and counts to zero's    */
	custom_handle->max_pb_supported = 0;		/* pb - processor blade   */
	custom_handle->max_blower_supported = 0;	/* blower - blower        */
	custom_handle->max_pm_supported = 0;		/* pm - power module      */
	custom_handle->max_sm_supported = 0;		/* sm - switch module     */
	custom_handle->max_mm_supported = 0;		/* mm - management module */
	custom_handle->max_mt_supported = 0;		/* mt - media tray        */
	custom_handle->max_filter_supported = 0;	/* filter - front bezel   */
	custom_handle->max_tap_supported = 0;		/* tap-telco alarm panel  */
	custom_handle->max_nc_supported = 0;		/* nc - network clock card*/
	custom_handle->max_mx_supported = 0;		/* mx - multiplex card    */
	custom_handle->max_mmi_supported = 0;		/* mmi- mm interposer     */	
	custom_handle->max_smi_supported = 0;		/* smi- switch interposer */
	
	memset(&custom_handle->installed_pb_mask, '\0', SNMP_BC_MAX_RESOURCES_MASK);
	memset(&custom_handle->installed_blower_mask, '\0', SNMP_BC_MAX_RESOURCES_MASK);
	memset(&custom_handle->installed_pm_mask, '\0', SNMP_BC_MAX_RESOURCES_MASK);
	memset(&custom_handle->installed_sm_mask, '\0', SNMP_BC_MAX_RESOURCES_MASK);
	memset(&custom_handle->installed_mm_mask, '\0', SNMP_BC_MAX_RESOURCES_MASK);
	memset(&custom_handle->installed_tap_mask, '\0', SNMP_BC_MAX_RESOURCES_MASK);
	memset(&custom_handle->installed_nc_mask, '\0', SNMP_BC_MAX_RESOURCES_MASK);
	memset(&custom_handle->installed_mx_mask, '\0', SNMP_BC_MAX_RESOURCES_MASK);	
	memset(&custom_handle->installed_mmi_mask, '\0', SNMP_BC_MAX_RESOURCES_MASK);	
	memset(&custom_handle->installed_smi_mask, '\0', SNMP_BC_MAX_RESOURCES_MASK);
	custom_handle->installed_mt_mask = 0;
	custom_handle->installed_filter_mask = 0;

	custom_handle->host             = NULL;
	custom_handle->host_alternate   = NULL;

        /* Indicate this is the 1st discovery (T0 discovery) */
        /* Use to see if we need to create events for log entries.  */
	/* Do not report any event from event log entries, for      */
	/* entries that are created before this instance of OpenHPI */	
	custom_handle->isFirstDiscovery = SAHPI_TRUE;
	
        /* Initialize RPT cache */
        handle->rptcache = (RPTable *)g_malloc0(sizeof(RPTable));
        oh_init_rpt(handle->rptcache);
	 
        /* Initialize event log cache */
        handle->elcache = oh_el_create(BC_EL_MAX_SIZE);
	handle->elcache->gentimestamp = FALSE;

	/* Initialize simulator tables */
	if (is_simulator()) {
		custom_handle->ss = NULL;
		sim_init();
	}
	else {
		/* Initialize SNMP library */
		init_snmp("oh_snmp_bc");
		snmp_sess_init(&(custom_handle->session));
		custom_handle->session.peername = hostname;
		custom_handle->host             = hostname;
		custom_handle->host_alternate    = 
				((char *)g_hash_table_lookup(handle->config, "host_alternate"));
			 
		
		/* Set retries/timeouts - based on testing with BC/BCT MM SNMP V3 agent */
		custom_handle->session.retries = 3;
		custom_handle->session.timeout = 15000000; /* 15000000 in microseconds */  
		version = (char *)g_hash_table_lookup(handle->config, "version");
		if (!version) {
			err("Cannot find \"version\" configuration parameter.");
			return NULL;
		}
		sec_level = (char *)g_hash_table_lookup(handle->config, "security_level");
		authtype = (char *)g_hash_table_lookup(handle->config, "auth_type");
		user = (char *)g_hash_table_lookup(handle->config, "security_name");
		pass = (char *)g_hash_table_lookup(handle->config, "passphrase");
		community = (char *)g_hash_table_lookup(handle->config, "community");
		context_name = (char *)g_hash_table_lookup(handle->config, "context_name");
		count_per_getbulk = (char *)g_hash_table_lookup(handle->config, "count_per_getbulk");
		privacy_passwd = (char *)g_hash_table_lookup(handle->config, "privacy_passwd");
		privacy_protocol = (char *)g_hash_table_lookup(handle->config, "privacy_protocol");
		
		
		/* Treating three (3) cases of non-declared parm the same  */
		/* That is   Not-declared-parm, parm = "", parm ="none"    */
		/* are the same to us                                      */   
		check_snmp_parm(sec_level);
		check_snmp_parm(authtype);
		check_snmp_parm(user);
		check_snmp_parm(pass);
		check_snmp_parm(community);
		check_snmp_parm(context_name);
		check_snmp_parm(count_per_getbulk);
		check_snmp_parm(privacy_passwd);
		check_snmp_parm(privacy_protocol);
		
		/* Configure SNMP V3 session */
		if (!g_ascii_strncasecmp(version, "3", sizeof("3"))) {
			if (!user) {
				err("Cannot find \"security_name\" configuration parameter.");
				return NULL;
			}
			custom_handle->session.version = SNMP_VERSION_3;
			custom_handle->session.securityName = user;
			custom_handle->session.securityNameLen = strlen(user);
			custom_handle->session.securityLevel = SNMP_SEC_LEVEL_NOAUTH;
			
			if (!g_ascii_strncasecmp(sec_level, "auth", 4)) { /* If using password */
				if (!pass) {
					err("Cannot find \"passphrase\" configuration parameter.");
					return NULL;
				}
				
				custom_handle->session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
				if (!authtype || !g_ascii_strncasecmp(authtype, "MD5", sizeof("MD5"))) {
					custom_handle->session.securityAuthProto = usmHMACMD5AuthProtocol;
					custom_handle->session.securityAuthProtoLen = USM_AUTH_PROTO_MD5_LEN;
				} else if (!g_ascii_strncasecmp(authtype, "SHA", sizeof("SHA"))) {
					custom_handle->session.securityAuthProto = usmHMACSHA1AuthProtocol;
					custom_handle->session.securityAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;
				} else {
					err("Unrecognized authenication type=%s.", authtype); 
					return NULL;
				}
				
				custom_handle->session.securityAuthKeyLen = USM_AUTH_KU_LEN;
				if (generate_Ku(custom_handle->session.securityAuthProto,
						custom_handle->session.securityAuthProtoLen,
						(u_char *) pass, strlen(pass),
						custom_handle->session.securityAuthKey,
						&(custom_handle->session.securityAuthKeyLen)) != SNMPERR_SUCCESS) {
					snmp_perror("snmp_bc");
					snmp_log(LOG_ERR,
						 "Error generating Ku from authentication passphrase.\n");
					err("Unable to establish SNMP authnopriv session.");
					return NULL;
				}
				
				if (!g_ascii_strncasecmp(sec_level, "authPriv", sizeof("authPriv"))) { /* if using encryption */
				
					if (!privacy_passwd) {
						err("Cannot find \"privacy_passwd\" configuration parameter.");
						return NULL;
					}
				
					custom_handle->session.securityLevel = SNMP_SEC_LEVEL_AUTHPRIV;
					custom_handle->session.securityPrivProto = usmDESPrivProtocol;
					custom_handle->session.securityPrivProtoLen = USM_PRIV_PROTO_DES_LEN;
					custom_handle->session.securityPrivKeyLen = USM_PRIV_KU_LEN;
					if (generate_Ku(custom_handle->session.securityAuthProto,
							custom_handle->session.securityAuthProtoLen,
							(u_char *) privacy_passwd, strlen(privacy_passwd),
							custom_handle->session.securityPrivKey,
							&(custom_handle->session.securityPrivKeyLen)) != SNMPERR_SUCCESS) {
						snmp_perror("snmp_bc");
						snmp_log(LOG_ERR,
							 "Error generating Ku from private passphrase.\n");
						err("Unable to establish SNMP authpriv session.");
						return NULL;
					}
					
				}
				

				if (context_name != NULL) 
				{
					custom_handle->session.contextName = (char *)context_name;
					custom_handle->session.contextNameLen = strlen(context_name);
				}
			}
			
			if (count_per_getbulk != NULL) 
			{
				custom_handle->count_per_getbulk = atoi((char *)count_per_getbulk);
				if (custom_handle->count_per_getbulk != 0) { 
						
					if (custom_handle->count_per_getbulk <= SNMP_BC_BULK_MIN)
					 	custom_handle->count_per_getbulk = SNMP_BC_BULK_MIN;
					else if (custom_handle->count_per_getbulk > SNMP_BC_MM_BULK_MAX)
						custom_handle->count_per_getbulk = SNMP_BC_MM_BULK_MAX;
				}
							
			} else { 
					custom_handle->count_per_getbulk = SNMP_BC_BULK_DEFAULT;
			}
			                                
                /* Configure SNMP V1 session */
		} else if (!g_ascii_strncasecmp(version, "1", sizeof("1"))) { 
			if (!community) {
				err("Cannot find \"community\" configuration parameter.");
				return NULL;
			}
			custom_handle->session.version = SNMP_VERSION_1;
			custom_handle->session.community = (u_char *)community;
			custom_handle->session.community_len = strlen(community);
		} else {
			err("Unrecognized SNMP version=%s.", version);
			return NULL;
		}

		rv = snmp_bc_manage_snmp_open(custom_handle, SAHPI_TRUE);                
		if (rv != SA_OK) return NULL;
	}

	/* Determine BladeCenter chassis type */
	{
		const char *oid;
		struct snmp_value get_value;
		SaErrorT err;
		do {
			err = snmp_bc_snmp_get(custom_handle, SNMP_BC_CHASSIS_TYPE_OID, &get_value, SAHPI_FALSE);
			if (err == SA_OK) {
			        int chassis_type, chassis_subtype;
				
				chassis_type = get_value.integer;
				err = snmp_bc_snmp_get(custom_handle, SNMP_BC_CHASSIS_SUBTYPE_OID, &get_value, SAHPI_FALSE);
				if (err) {
					err("Cannot read model subtype");
					chassis_subtype = SNMP_BC_CHASSIS_SUBTYPE_ORIG;
				} else {
					chassis_subtype = get_value.integer;
				}
				

				if (chassis_type == SNMP_BC_CHASSIS_TYPE_BC &&
				    chassis_subtype == SNMP_BC_CHASSIS_SUBTYPE_ORIG) {
					dbg("Found BC");
					custom_handle->platform = SNMP_BC_PLATFORM_BC;
					break;
				}

				if (chassis_type == SNMP_BC_CHASSIS_TYPE_BC &&
				    chassis_subtype == SNMP_BC_CHASSIS_SUBTYPE_H) {
					dbg("Found BCH");
					custom_handle->platform = SNMP_BC_PLATFORM_BCH;
					break;
				}
				
				if (chassis_type == SNMP_BC_CHASSIS_TYPE_BCT &&
				    chassis_subtype == SNMP_BC_CHASSIS_SUBTYPE_ORIG) {
					dbg("Found BCT");
					custom_handle->platform = SNMP_BC_PLATFORM_BCT;
					break;
				}

								
				if (chassis_type == SNMP_BC_CHASSIS_TYPE_BCT &&
				    chassis_subtype == SNMP_BC_CHASSIS_SUBTYPE_H) {
					dbg("Found BCHT");
					custom_handle->platform = SNMP_BC_PLATFORM_BCHT;
					break;
				}
				
				err("Unknown BladeCenter model");
				return NULL;
			}
			else {  /* Older MM software doesn't support chassis type and subtype OIDs */
				err = snmp_bc_snmp_get(custom_handle, SNMP_BC_PLATFORM_OID_BCT, &get_value, SAHPI_FALSE);
				if (err == SA_OK) {
					dbg("Found BCT");
					custom_handle->platform = SNMP_BC_PLATFORM_BCT;
					break;
				} 
				
				err = snmp_bc_snmp_get(custom_handle, SNMP_BC_PLATFORM_OID_BC, &get_value, SAHPI_FALSE);
				if (err == SA_OK) {
					dbg("Found BC");
					custom_handle->platform = SNMP_BC_PLATFORM_BC;
					break;
				}
				
				err = snmp_bc_snmp_get(custom_handle, SNMP_BC_PLATFORM_OID_RSA, &get_value, SAHPI_FALSE);
				if (err == SA_OK) {
					dbg("Found RSA");
					custom_handle->platform = SNMP_BC_PLATFORM_RSA;
					break;
				} 
				
				err("Unknown BladeCenter model");
				return NULL;
			}
		} while(0);
		
		/* Determine if daylight savings time (DST) is enabled */
		if (custom_handle->platform == SNMP_BC_PLATFORM_RSA) { oid = SNMP_BC_DST_RSA; }
		else { oid = SNMP_BC_DST; }

		err = snmp_bc_snmp_get(custom_handle, oid, &get_value, SAHPI_TRUE);
		if (err == SA_OK) {
			strncpy(custom_handle->handler_timezone, get_value.string,9);
		}
		else {
			err("Cannot read DST=%s; Error=%d.", oid, get_value.type);
			return NULL;
		}
	}

	/* Initialize "String to Event" mapping hash table */
	if (errlog2event_hash_use_count == 0) {
		if (errlog2event_hash_init(custom_handle)) {
			err("Out of memory.");
			return NULL;
		}
	}
	errlog2event_hash_use_count++;
	
	/* Initialize "Event Number to HPI Event" mapping hash table */
	if (event2hpi_hash_init(handle)) {
		err("Out of memory.");
		return NULL;
	}
  
	if (is_simulator()) {
		sim_banner(custom_handle);
	}
	
        return handle;
}

/**
 * snmp_bc_close: 
 * @hnd: Pointer to handler structure.
 * 
 * Close an SNMP BladeCenter/RSA plugin handler instance.
 *
 * Returns:
 * Void
 **/
void snmp_bc_close(void *hnd)
{

	struct oh_handler_state *handle;
	
	if (!hnd) {
                err("INVALID PARM - NULL handle pointer.");
                return;	
	}
	
	handle = (struct oh_handler_state *)hnd;
        oh_el_close(handle->elcache);

	if (is_simulator()) {
		sim_close();
	}
	else {
		struct snmp_bc_hnd *custom_handle = (struct snmp_bc_hnd *)handle->data;

		snmp_sess_close(custom_handle->sessp);
		/* Windows32 specific net-snmp cleanup (noop on unix) */
		SOCK_CLEANUP;
	}

	/* Cleanup event2hpi hash table */
	event2hpi_hash_free(handle);

	/* Cleanup errlog2event_hash table */
	errlog2event_hash_use_count--;
	if (errlog2event_hash_use_count == 0) {
		errlog2event_hash_free();
	}
	
        oh_flush_rpt(handle->rptcache);  
        g_free(handle->rptcache);
	
}

/**
 * snmp_bc_manage_snmp_open: 
 * @hnd: Pointer to handler structure.
 * 
 * .
 *
 * Returns:
 * SA_OK  - able to open a snmp session
 **/
SaErrorT snmp_bc_manage_snmp_open(struct snmp_bc_hnd *custom_handle, SaHpiBoolT recovery_requested)
{

	SaErrorT rv;
	
	rv = SA_OK;
	
	/* Windows32 specific net-snmp initialization (noop on unix) */
	SOCK_STARTUP;

	custom_handle->sessp = snmp_sess_open(&(custom_handle->session));
	
	if (custom_handle->sessp == NULL) {
		// snmp_perror("ack");
		// snmp_log(LOG_ERR, "Something horrible happened!!!\n");
		if (recovery_requested) {
			rv = snmp_bc_recover_snmp_session(custom_handle);
		} else {
			rv = SA_ERR_HPI_NO_RESPONSE;
		}
	}
		
	if (rv == SA_OK) 
		custom_handle->ss    = snmp_sess_session(custom_handle->sessp);
	
	return(rv);
}


/**
 * snmp_bc_recover_snmp_session: 
 * @hnd: Pointer to handler structure.
 * 
 * .
 *
 * Returns:
 * SA_OK  - able to open a snmp session
 **/
SaErrorT snmp_bc_recover_snmp_session(struct snmp_bc_hnd *custom_handle)
{

	SaErrorT rv;	
	rv = SA_OK;
	
	if (custom_handle->host_alternate != NULL) {
		/* This openhpi.conf stanza has 2 different hosts defined */
		if (!custom_handle->sessp) {
			snmp_sess_close(custom_handle->sessp);
			/* Windows32 specific net-snmp cleanup (noop on unix) */
			SOCK_CLEANUP;		
		}
		
		if ( strcmp(custom_handle->host, custom_handle->session.peername) == 0 ) {
			dbg("Attemp recovery with custom_handle->host_alternate %s\n", custom_handle->host_alternate);
			custom_handle->session.peername = custom_handle->host_alternate;
		} else {
			dbg("Attemp recovery with custom_handle->host %s\n", custom_handle->host);		 
			custom_handle->session.peername = custom_handle->host;
		}
		rv = snmp_bc_manage_snmp_open(custom_handle, SAHPI_FALSE);
		
	} else {
		dbg("No host_alternate defined in openhpi.conf. No recovery on host_alternate.\n");
		rv = SA_ERR_HPI_NO_RESPONSE;
	}

	return(rv);
}
/**
 **/
void * oh_open (GHashTable *, unsigned int, oh_evt_queue *) __attribute__ ((weak, alias("snmp_bc_open")));
void * oh_close (void *) __attribute__ ((weak, alias("snmp_bc_close")));

