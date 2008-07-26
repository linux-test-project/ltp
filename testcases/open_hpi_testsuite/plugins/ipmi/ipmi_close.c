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
 *     Tariq Shureih <tariq.shureih@intel.com>
 */

#include "ipmi.h"

static void close_done(void *cb_data)
{
	struct ohoi_handler *ipmi_handler = cb_data;
	trace_ipmi("close_done");
	ipmi_handler->fully_up = 0;
}

static void close_connection(ipmi_domain_t *domain, void *user_data)
{
	int rv, *flag = user_data;

	trace_ipmi("close flag:%d", *flag);

	
	rv = ipmi_domain_close(domain, close_done, user_data);
	if (rv) {
		err("ipmi_close_connection failed!");
		*flag = 1;
	}
}

void ohoi_close_connection(ipmi_domain_id_t domain_id, void *user_data)
{
	struct oh_handler_state *handler = (struct oh_handler_state *)user_data;
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;
	int rv;

	trace_ipmi("ohoi_close_connection");

	rv = ipmi_domain_pointer_cb(domain_id, close_connection, ipmi_handler);
	
	if (rv) {
		err("ipmi_domain_pointer_cb failed!");
		return;
	}

	while (ipmi_handler->fully_up != 0) {
		sel_select(ipmi_handler->ohoi_sel, NULL, 0, NULL, NULL);
	}
}
