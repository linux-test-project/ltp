/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 * (C) Copright IBM Corp 2003
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
 *     Sean Dague <http://dague.net/sean>
 */

#ifndef __OH_PLUGIN_H
#define __OH_PLUGIN_H

#include <uuid/uuid.h>
#include <glib.h>

#include <sys/time.h>
#include <SaHpi.h>
#include <rpt_utils.h>

#ifdef __cplusplus
extern "C" {
#endif 

/*
 * struct oh_sel_id is filled by plugin.
 * Open HPI use it to identy different resource SEL by the id.
 */
struct oh_sel_id {
	void *ptr;
};


/*
 * The event is used for plugin to report its resources
 * (Domain, SEL and RDR etc.).
 */
struct oh_resource_event {
	/* XXX: upper layer will fill some fields which does not 
	 * owned by plugins (such as domain Id)
	 */
	SaHpiRptEntryT		entry;
};

/*
 *
 */
struct oh_resource_del_event {
        SaHpiResourceIdT resource_id;        
};

/*
 * The event is used for plugin to report that a given resource
 * is a member of a specific domain.
 */
/*struct oh_domain_event {
	struct oh_resource_id	res_id;
	struct oh_domain_id	domain_id;
};*/

/* 
 * The event is used for plugin to report its RDRs in resource.
 */
struct oh_rdr_event {
	SaHpiRdrT		rdr;
};

/*
 * This event is used for requesting an rdr deletion.
 */
struct oh_rdr_del_event {
        SaHpiEntityPathT parent_entity; /* Identifies the resource parent */
        SaHpiEntryIdT record_id; /* Identifies the rdr to be deleted */
};

/*
 * The event is used for plugin to notify HPI events
 */
struct oh_hpi_event {
	/*This is resource id which the event belongs to */
        SaHpiResourceIdT parent;
        /* struct oh_resource_id	parent; */
	/*This is rdr id which the event relates*/
        SaHpiEntryIdT id;
	
	/* XXX: upper layer will fill some fields which does not
	 * owned by plugins (ResourceId etc.). */
	SaHpiEventT		event;
};

/* The structure is used to storage RSEL entry*/
struct oh_rsel {
	/* this is the entry's id */
	struct oh_sel_id	oid;
	
	SaHpiEntryIdT		entry_id;
};

/*
 * The event is used for plugin to notify SEL events
 */
/*struct oh_rsel_event {
	struct oh_rsel rsel;
};*/

/* 
 * This is the main event structure. It is used for plugin report
 * its discovery about new resource/rdr or what happend on resource
 */
struct oh_event {
	enum {
		OH_ET_RESOURCE,
		OH_ET_RESOURCE_DEL,
		OH_ET_RDR,
                OH_ET_RDR_DEL,
		OH_ET_HPI
	}type;
	union {
		struct oh_resource_event res_event;
                struct oh_resource_del_event res_del_event;
		struct oh_rdr_event	 rdr_event;
                struct oh_rdr_del_event  rdr_del_event;
		struct oh_hpi_event	 hpi_event;
	} u;		    
};

struct oh_handler_state {
        RPTable *rptcache;
        GSList *eventq;
        GHashTable *config;
        void *data;
};


/* Current abi is version 2. Version 1 is out-of-date and nobody
 * should use it
 */

#if 0
/* UUID_OH_ABI_V1 is out-of-date, keep here just for reference 
 * ee778a5f-32cf-453b-a650-518814dc956c 
 */
static const uuid_t UUID_OH_ABI_V1 = {
	0xee, 0x77, 0x8a, 0x5f, 0x32, 0xcf, 0x45, 0x3b,
	0xa6, 0x50, 0x51, 0x88, 0x14, 0xdc, 0x95, 0x6c
};
#endif

/* 13adfcc7-d788-4aaf-b966-5cd30bdcd808 */
static const uuid_t UUID_OH_ABI_V2 = {
        0x13, 0xad, 0xfc, 0xc7, 0xd7, 0x88, 0x4a, 0xaf,
        0xb9, 0x66, 0x5c, 0xd3, 0x0b, 0xdc, 0xd8, 0x08
};

struct oh_abi_v2 {
	/**
	 * The function create an instance 
	 * @return the handler of the instance, this can be recognised 
	 * as a domain in upper layer
	 * @param name the mechanism's name. 
	 * for example, "snmp" for SNMP, "smi" for IPMI SMI
	 * @param addr the interface name.
	 * for example, "ipaddr:port" for SNMP, "if_num" for IPMI SMI
	 */
	void *(*open)(GHashTable *handler_config);
	
	void (*close)(void *hnd);
	/**
	 * The function wait for event. 
	 * 
	 *
	 * @remark at the start-up, plugins must send out res/rdr event for all
	 * resources and rdrs so as to OpenHPI can build up RPT/RDR.
	 * @return >0 if an event is returned; 0 if timeout; otherwise an error
	 * occur.
	 * @param event if existing, plugin store the event. 
	 * @param timeout is an upper bound on the amount of time elapsed
	 * before returns. It may be zero, causing select to return 
	 * immediately.
	 */
	SaErrorT (*get_event)(void *hnd, struct oh_event *event, 
			 struct timeval *timeout);
	
	/**
	 * prompt plug-in to search for new resources
	 */
	SaErrorT (*discover_resources)(void *hnd);

        /**
         * set resource tag, this is passed down so the device has
         * a chance to set it in nv storage if it likes
         */
        SaErrorT (*set_resource_tag)(void *hnd, SaHpiResourceIdT id, SaHpiTextBufferT *tag);
        
        /**
         * set resource severity is pushed down so the device has
         * a chance to set it in nv storage
         */
        SaErrorT (*set_resource_severity)(void *hnd, SaHpiResourceIdT id, SaHpiSeverityT sev);

        /**
	 * get the id which the caller is running
	 */
	SaErrorT (*get_self_id)(void *hnd, SaHpiResourceIdT id);

        /******************************************************
         *
         *  System Event Log functions
         *
         *****************************************************/

	/**
	 * get info from RSEL
	 */
	SaErrorT (*get_sel_info)(void *hnd, SaHpiResourceIdT id, SaHpiSelInfoT *info);

	/**
	 * set time to RSEL
	 */
	SaErrorT (*set_sel_time)(void *hnd, SaHpiResourceIdT id, SaHpiTimeT time);

	/**
	 * add entry to RSEL
	 */
	SaErrorT (*add_sel_entry)(void *hnd, SaHpiResourceIdT id, const SaHpiSelEntryT *Event);

	/**
	 * del entry in RSEL
	 */
        SaErrorT (*del_sel_entry)(void *hnd, SaHpiResourceIdT id, SaHpiSelEntryIdT sid);

	/**
	 * get entry in RSEL
         * 
         * although it looks like we need Resource and RDR passed back up, we don't
         * because SelEntryT has that info stored in it.  We'll just unwind 
         * that in infrastructure.
	 */
        SaErrorT (*get_sel_entry)(void *hnd, SaHpiResourceIdT id, SaHpiSelEntryIdT current,
                             SaHpiSelEntryIdT *prev, SaHpiSelEntryIdT *next, SaHpiSelEntryT *entry);
        
        /**
         * clear SEL 
         */
        SaErrorT (*clear_sel)(void *hnd, SaHpiResourceIdT id);

        /* end of SEL functions */
	/**
	 * get sensor data
	 */
	SaErrorT (*get_sensor_data)(void *hnd, SaHpiResourceIdT id, 
                               SaHpiSensorNumT num,
                               SaHpiSensorReadingT *data);

	/**
	 * get sensor thresholds
	 */
	SaErrorT (*get_sensor_thresholds)(void *hnd, SaHpiResourceIdT id,
                                     SaHpiSensorNumT num,
                                     SaHpiSensorThresholdsT *thres);
	
	/**
	 * set sensor thresholds
	 */
	SaErrorT (*set_sensor_thresholds)(void *hnd, SaHpiResourceIdT id,
                                     SaHpiSensorNumT num,
                                     const SaHpiSensorThresholdsT *thres);

	/**
	 * get sensor event enables
	 */
	SaErrorT (*get_sensor_event_enables)(void *hnd, SaHpiResourceIdT id,
                                        SaHpiSensorNumT num,
                                        SaHpiSensorEvtEnablesT *enables);

	/**
	 * set sensor event enables
	 */
	SaErrorT (*set_sensor_event_enables)(void *hnd, SaHpiResourceIdT id,
                                    SaHpiSensorNumT num,
                                    const SaHpiSensorEvtEnablesT *enables);

	/**
	 * get control state
	 */
	SaErrorT (*get_control_state)(void *hnd, SaHpiResourceIdT id,
                                 SaHpiCtrlNumT num,
                                 SaHpiCtrlStateT *state);
	
	/**
	 * set control state
	 */
	SaErrorT (*set_control_state)(void *hnd, SaHpiResourceIdT id,
                                 SaHpiCtrlNumT num,
                                 SaHpiCtrlStateT *state);
	
	/**
	 * get inventory size
	 */
        SaErrorT (*get_inventory_size)(void *hnd, SaHpiResourceIdT id,
                                  SaHpiEirIdT num, /* yes, they don't call it a
                                                    * num, but it still is one
                                                    */
                                  SaHpiUint32T *size);

	/**
	 * get inventory state
	 */
        SaErrorT (*get_inventory_info)(void *hnd, SaHpiResourceIdT id,
                                  SaHpiEirIdT num,
                                  SaHpiInventoryDataT *data);

	/**
	 * set inventory state
	 */
        SaErrorT (*set_inventory_info)(void *hnd, SaHpiResourceIdT id,
                                  SaHpiEirIdT num,
                                  const SaHpiInventoryDataT *data);

	/**
	 * get watchdog timer info
	 */
	SaErrorT (*get_watchdog_info)(void *hnd, SaHpiResourceIdT id,
                                 SaHpiWatchdogNumT num,
                                 SaHpiWatchdogT *wdt);

	/** 
	 * set watchdog timer info
	 */
	SaErrorT (*set_watchdog_info)(void *hnd, SaHpiResourceIdT id,
                                 SaHpiWatchdogNumT num,
                                 SaHpiWatchdogT *wdt);

	/**
	 * reset watchdog timer info
	 */
        SaErrorT (*reset_watchdog)(void *hnd, SaHpiResourceIdT id,
                              SaHpiWatchdogNumT num);

	/**
	 * get hotswap state
	 */
	SaErrorT (*get_hotswap_state)(void *hnd, SaHpiResourceIdT id, 
				 SaHpiHsStateT *state);

	/**
	 * set hotswap state
	 */
	SaErrorT (*set_hotswap_state)(void *hnd, SaHpiResourceIdT id, 
				 SaHpiHsStateT state);

	/**
	 * request hotswap state
	 */
	SaErrorT (*request_hotswap_action)(void *hnd, SaHpiResourceIdT id, 
				      SaHpiHsActionT act);

	/**
	 * get power state
	 */
	SaErrorT (*get_power_state)(void *hnd, SaHpiResourceIdT id, 
			       SaHpiHsPowerStateT *state);

	/**
	 * set power state
	 */
	SaErrorT (*set_power_state)(void *hnd, SaHpiResourceIdT id, 
			       SaHpiHsPowerStateT state);
	
	/**
	 * get indicator state
	 */
	SaErrorT (*get_indicator_state)(void *hnd, SaHpiResourceIdT id, 
				   SaHpiHsIndicatorStateT *state);

	/**
	 * set indicator state
	 */
	SaErrorT (*set_indicator_state)(void *hnd, SaHpiResourceIdT id, 
				   SaHpiHsIndicatorStateT state);

	/**
	 * control parameter
	 */
	SaErrorT (*control_parm)(void *hnd, SaHpiResourceIdT id, SaHpiParmActionT act);

	/**
	 * get reset state
	 */
	SaErrorT (*get_reset_state)(void *hnd, SaHpiResourceIdT id, 
			       SaHpiResetActionT *act);

	/**
	 * set_reset state
	 */
	SaErrorT (*set_reset_state)(void *hnd, SaHpiResourceIdT id, 
			       SaHpiResetActionT act);

};


/*The function is used for plugin loader to get interface*/
int get_interface(void **pp, const uuid_t uuid) __attribute__ ((weak));

/* Structure for static plugins */
typedef int (*get_interface_t)( void **pp, const uuid_t uuid );

struct oh_static_plugin
{
        char           *name;
        get_interface_t get_interface;
};

#ifdef __cplusplus
}
#endif

#endif/*__OH_PLUGIN_H*/
