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
 *      David Judkovics <djudkovi@us.ibm.com>
 *
 */

#ifndef _SNMP_CLIENT_H
#define _SNMP_CLIENT_H

/* used for storing information of Remote Resource */
struct resource_data {
	SaHpiDomainIdT 		remote_domain;
	SaHpiResourceIdT 	remote_resource_id;
	SaHpiEntryIdT		remote_entry_id;
};



struct rdr_index {
	SaHpiDomainIdT 		remote_domain;
	SaHpiResourceIdT 	remote_resource_id;
	SaHpiEntryIdT        	remote_record_id;
	/* get this value for the rdr record proper */
	/* saHpiRdrType */
};


struct rdr_data {
	struct rdr_index 	index;
	void 			*control_rdr;
	void 			*sensor_rdr;
	void 			*inventory_rdr;
	void 			*watchdog_rdr;
};



/**
 * This handle will be unique per instance of
 * this plugin. SNMP session data is stored
 * in the handle along with config file data.
 **/
struct snmp_client_hnd {
        struct snmp_session session;
        struct snmp_session *ss; /* SNMP Session pointer */
};


/* functions in snmp_client_gen_evt.c */
struct oh_event *eventdup(const struct oh_event *event);
int eventq_event_add(struct oh_handler_state *oh_hnd);
void process_diff_table(struct oh_handler_state *handle, RPTable *diff_table);

#endif
