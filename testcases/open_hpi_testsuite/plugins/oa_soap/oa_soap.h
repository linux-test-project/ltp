/*
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
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
 */

#ifndef _OA_SOAP_H
#define _OA_SOAP_H

/* Include files */
#include <unistd.h>

#include <SaHpi.h>
#include <oh_handler.h>
#include <oh_error.h>

#include "oa_soap_calls.h"

/* Definitions for the different RDR instrument ids */
#define OA_SOAP_RES_INV_NUM                (SaHpiIdrIdT)     0x000
#define OA_SOAP_RES_CNTRL_NUM              (SaHpiCtrlNumT)   0x001
#define OA_SOAP_RES_SEN_TEMP_NUM           (SaHpiSensorNumT) 0x002
#define OA_SOAP_RES_SEN_EXH_TEMP_NUM       (SaHpiSensorNumT) 0x003
#define OA_SOAP_RES_SEN_FAN_NUM            (SaHpiSensorNumT) 0x004
#define OA_SOAP_RES_SEN_POWER_NUM          (SaHpiSensorNumT) 0x005
#define OA_SOAP_RES_SEN_PRES_NUM           (SaHpiSensorNumT) 0x006
#define OA_SOAP_RES_SEN_OPR_NUM            (SaHpiSensorNumT) 0x007
#define OA_SOAP_RES_SEN_IN_POWER_NUM       (SaHpiSensorNumT) 0x008
#define OA_SOAP_RES_SEN_OUT_POWER_NUM      (SaHpiSensorNumT) 0x009
#define OA_SOAP_RES_SEN_POWER_CAPACITY_NUM (SaHpiSensorNumT) 0x00a

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

/* OA firmware version 2.20 */
#define OA_2_20 2.20

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
        SaHpiFloat64T fm_version;
};

enum resource_presence_status
{
        RES_ABSENT = 0,
        RES_PRESENT= 1
};

/* Resource presence matrix per resource type */
struct resource_status
{
        SaHpiInt32T max_bays;
        enum resource_presence_status *presence;
        char **serial_number;
};

/* Resource presence matrix for all FRUs in HP BladeSystem c-Class */
struct oa_soap_resource_status
{
        struct resource_status oa;
        struct resource_status server;
        struct resource_status interconnect;
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
        GMutex *mutex;
};

/* Structure for passing the parameters for Event threads */
struct event_handler
{
        struct oh_handler_state *oh_handler;
        struct oa_info *oa;
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
