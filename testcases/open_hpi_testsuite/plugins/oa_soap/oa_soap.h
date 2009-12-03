/*
 * Copyright (C) 2007-2009, Hewlett-Packard Development Company, LLP
 *                     All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the Hewlett-Packard Corporation, nor the names
 * of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author(s)
 *      Raghavendra P.G. <raghavendra.pg@hp.com>
 *      Raghavendra M.S. <raghavendra.ms@hp.com>
 *      Raja Kumar Thatte <raja-kumar.thatte@hp.com>
 *      Vivek Kumar <vivek.kumar2@hp.com>
 *      Shuah Khan <shuah.khan@hp.com>
 *      Mohan Devarajulu <mohan@fc.hp.com>
 */

#ifndef _OA_SOAP_H
#define _OA_SOAP_H

/* Include files */
#include <unistd.h>

#include <SaHpi.h>
#include <oh_handler.h>
#include <oh_error.h>

#include "oa_soap_calls.h"

/* The resource numbers in OA SOAP. The rpt and rdr arrays are indexed based on
 * the entity numbers defined below.
 * On supporting a new resource in OA SOAP plugin, please update the rpt and rdr
 * arrays in oa_soap_resources.c
 */ 
/* Enclosure */
#define OA_SOAP_ENT_ENC			0
/* Server Blade */
#define OA_SOAP_ENT_SERV		1
/* IO Blade */
#define OA_SOAP_ENT_IO			2
/* Storage Blade */
#define OA_SOAP_ENT_STORAGE		3
/* Switch Blade */
#define OA_SOAP_ENT_SWITCH		4
/* Onboard Administrator */
#define OA_SOAP_ENT_OA			5
/* Power Subsystem */
#define OA_SOAP_ENT_PS_SUBSYS		6
/* Power supply */
#define OA_SOAP_ENT_PS			7
/* Thermal subsystem */
#define OA_SOAP_ENT_THERM_SUBSYS 	8
/* Fan Zone */
#define OA_SOAP_ENT_FZ			9
/* Fan */
#define OA_SOAP_ENT_FAN			10
/* LCD */
#define OA_SOAP_ENT_LCD			11

/* The different enclosure types supported by OA SOAP 
 *
 * If a new enclosure is added, please update the OA_SOAP_MAX_FAN. Add
 * entries for the new enclosure in oa_soap_fz_map_arr in
 * oa_soap_resources.c file.
 */
#define OA_SOAP_ENC_C7000       0
#define OA_SOAP_ENC_C3000       1

/* Max Blade in HP BladeSystem c7000 c-Class enclosure*/
#define OA_SOAP_C7000_MAX_BLADE 16
/* Max Blade in HP BladeSystem c3000 c-Class enclosure*/
#define OA_SOAP_C3000_MAX_BLADE 8

/* Maximum Fan Zones present in different HP BladeSystem c-Class enclosures */
#define OA_SOAP_C7000_MAX_FZ 4
#define OA_SOAP_C3000_MAX_FZ 1

/* Maximum fans supported in an enclosure 
 *
 * If the max fan is changed, please update entries to
 * oa_soap_fz_map_arr in oa_soap_resources.c file
 */
#define OA_SOAP_MAX_FAN         10

/* Definitions for the different RDR instrument ids */
/* TODO: Move below definitons to SaHpiOaSoap.h file */
#define OA_SOAP_RES_INV_NUM                (SaHpiIdrIdT)     0x000
#define OA_SOAP_RES_CNTRL_NUM              (SaHpiCtrlNumT)   0x001

/* SOAP XML calls timeout values for event thread and hpi calls */
#define HPI_CALL_TIMEOUT 10
#define EVENT_CALL_TIMEOUT 40

/* Error code for SOAP XML calls */
#define SOAP_OK 0

/* SSH port */
#define PORT ":443"

/* Max URL and buffer size */
#define MAX_URL_LEN 255
#define MAX_BUF_SIZE 255

/* Max OA bays in HP BladeSystem c-Class */
#define MAX_OA_BAYS 2

/* OA Error numbers */
#define ERR_INVALID_PRIVILEGE_LEVEL 8
#define ERR_STANDBY_MODE 139

/* OA firmware versions */
#define OA_2_20 2.20
#define OA_2_21 2.21

/* OA switchover re-try wait period */
#define WAIT_ON_SWITCHOVER 10
/* OA switchover max re-try */
#define MAX_RETRY_ON_SWITCHOVER 10

/* Enum for storing the status of the plugin */
enum oa_soap_plugin_status {
        PRE_DISCOVERY = 0,
        PLUGIN_NOT_INITIALIZED = 1,
        DISCOVERY_FAIL = 2,
        DISCOVERY_COMPLETED = 3
};

/* Structure for storing the OA information */
struct oa_info
{
        enum oaRole oa_status;
        SaHpiInt32T event_pid;
        GThread *thread_handler;
        GMutex *mutex;
        char server[MAX_URL_LEN];
        SOAP_CON *hpi_con;
        SOAP_CON *event_con;
        SOAP_CON *event_con2;
        SaHpiFloat64T fm_version;
	struct oh_handler_state *oh_handler;
};

typedef enum resource_presence_status
{
        RES_ABSENT = 0,
        RES_PRESENT= 1
} resource_presence_status_t;

/* Resource presence matrix per resource type */
typedef struct resource_status
{
        SaHpiInt32T max_bays;
        enum resource_presence_status *presence;
        char **serial_number;
        SaHpiResourceIdT *resource_id;
} resource_status_t;

/* Resource presence matrix for all FRUs in HP BladeSystem c-Class */
struct oa_soap_resource_status
{
        SaHpiResourceIdT enclosure_rid;
        SaHpiResourceIdT power_subsystem_rid;
        SaHpiResourceIdT thermal_subsystem_rid;
        SaHpiResourceIdT lcd_rid;
        struct resource_status oa;
        struct resource_status server;
        struct resource_status interconnect;
        struct resource_status fan_zone;
        struct resource_status fan;
        struct resource_status ps_unit;
};

/* Structure for storing the OA SOAP plugin information */
struct oa_soap_handler
{
        enum oa_soap_plugin_status status;
        struct oa_soap_resource_status oa_soap_resources;
        SOAP_CON *active_con;
        struct oa_info *oa_1;
        struct oa_info *oa_2;
	/* Type of the enclsoure */
	SaHpiInt32T enc_type; 
	SaHpiBoolT shutdown_event_thread;
        SaHpiInt32T oa_switching; 
        GMutex *mutex;
};

/* Structure for storing the current hotswap state of the resource */
struct oa_soap_hotswap_state {
        SaHpiHsStateT currentHsState;
};

/* This define is the IANA-assigned private enterprise number for
 * Hewlett-Packard. A complete list of IANA numbers can be found at
 * http://www.iana.org/assignments/enterprise-numbers
 */
#define HP_MANUFACTURING_ID 11

/* This define is the IANA-assigned private enterprise number for Cisco Systems.
 * The HP BladeSystem c-Class can have interconnect blades from Cisco Systems
 */
#define CISCO_MANUFACTURING_ID 9

/* Checks for the shutdown request in event thread. On shutdown request, mutexes
 * locked by event thread are unlocked and exits the thread. It is necessary to
 * unlock the mutex, else g_free_mutex crahes on locked mutex
 */
#define OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, hnd_mutex, oa_mutex, timer) \
	{ \
		if (oa_handler->shutdown_event_thread == SAHPI_TRUE) { \
			dbg("Shutting down the OA SOAP event thread"); \
			if (oa_mutex != NULL) \
				g_mutex_unlock(oa_mutex); \
			if (hnd_mutex != NULL) \
				g_mutex_unlock(hnd_mutex); \
			if (timer != NULL) \
				g_timer_destroy(timer); \
			g_thread_exit(NULL); \
		} \
	}

/* Function prototypes */

SaErrorT build_oa_soap_custom_handler(struct oh_handler_state *oh_handler);

void *oa_soap_open(GHashTable *handler_config,
                   unsigned int hid,
                   oh_evt_queue *eventq);

void oa_soap_close(void *oh_handler);

SaErrorT oa_soap_set_resource_tag(void *oh_handler,
                                  SaHpiResourceIdT resource_id,
                                  SaHpiTextBufferT *tag);

SaErrorT oa_soap_set_resource_severity(void *oh_handler,
                                       SaHpiResourceIdT resource_id,
                                       SaHpiSeverityT severity);

SaErrorT oa_soap_control_parm(void *oh_handler,
                              SaHpiResourceIdT resource_id,
                              SaHpiParmActionT action);

#endif
