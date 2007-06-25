/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 */

#ifndef __SNMP_BC_EVENT_H
#define __SNMP_BC_EVENT_H

typedef enum {
	EVENT_NOT_MAPPED,
	EVENT_NOT_ALERTABLE,
} OEMReasonCodeT;

typedef struct {
	SaHpiResourceIdT      rid;
	BCRptEntryT           rpt;
	struct snmp_bc_sensor *sensor_array_ptr;
	SaHpiEntityPathT      ep;
} LogSource2ResourceT;

typedef struct {
	SaHpiEventT      hpievent;
	SaHpiEntityPathT ep;			  /* ep that matches hpievent.ResourceID */
        SaHpiEventStateT sensor_recovery_state;
        SaHpiHsStateT    hs_event_auto_state;     /* Hot swap state in hpievent */
        SaHpiHsStateT    hs_recovery_state;       /* Kill this when BC removes "Recovery" hot swap events */
        SaHpiHsStateT    hs_recovery_auto_state;  /* Kill this when BC removes "Recovery" hot swap events */
        SaHpiBoolT       event_res_failure;
        SaHpiBoolT       event_res_failure_unexpected;
} EventMapInfoT;

SaErrorT event2hpi_hash_init(struct oh_handler_state *handle);

SaErrorT event2hpi_hash_free(struct oh_handler_state *handle);

SaErrorT snmp_bc_discover_res_events(struct oh_handler_state *handle,
				     SaHpiEntityPathT *ep,
				     const struct ResourceInfo *resinfo);

SaErrorT snmp_bc_discover_sensor_events(struct oh_handler_state *handle,
					SaHpiEntityPathT *ep,
					SaHpiSensorNumT sid,
					const struct snmp_bc_sensor *sinfo);

SaErrorT snmp_bc_log2event(struct oh_handler_state *handle,
			   gchar *logstr,
			   SaHpiEventT *event,
			   int isdst,
			   LogSource2ResourceT *ret_logsrc2res); 

SaErrorT snmp_bc_add_to_eventq(struct oh_handler_state *handle,
			       SaHpiEventT *thisEvent, 
			       SaHpiBoolT prepend);
#endif
