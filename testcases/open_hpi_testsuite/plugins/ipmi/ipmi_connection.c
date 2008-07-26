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

static int init_domain_handlers(ipmi_domain_t	*domain,
                     void 		*user_data)
{

        
	struct oh_handler_state *handler = user_data;
	char	dmn_name[IPMI_DOMAIN_NAME_LEN];
	int rv;
	int ret = 0;

	rv = ipmi_domain_enable_events(domain);
	if (rv) {
		fprintf(stderr, "ipmi_domain_enable_events return error %d\n", rv);
		if (ret == 0) {
			ret = rv;
		}
	}


	rv = ipmi_domain_add_entity_update_handler(domain, ohoi_entity_event, 
			 			  handler);
	if (rv) {
		fprintf(stderr, "ipmi_domain_add_entity_update_handler error %d\n", rv);
		if (ret == 0) {
			ret = rv;
		}
	}

	rv = ipmi_domain_add_mc_updated_handler(domain, ohoi_mc_event, handler);
	if (rv)  {
		fprintf(stderr,
			"ipmi_domain_add_mc_updated_handler return error: %d\n", rv);
		if (ret == 0) {
			ret = rv;
		}
	}
	if (ret) {
		ipmi_domain_get_name(domain, dmn_name, IPMI_DOMAIN_NAME_LEN);
		fprintf(stderr, "Could not initialize ipmi domain %s\n", dmn_name);
	}
	return ret;
	
}


void ipmi_connection_handler (ipmi_domain_t	*domain,
			      int		err,
			      unsigned int	conn_num,
			      unsigned int	port_num,
			      int		still_connected,
			      void		*cb_data)
{
	struct oh_handler_state *handler = cb_data;
	struct ohoi_handler	*ipmi_handler = handler->data;
	int rv;

	trace_ipmi("connection handler called. Error code: 0x%x", err);

	ipmi_handler->d_type = ipmi_domain_get_type(domain);

	if (err) {
	  	err("Failed to connect to IPMI domain. err = 0x%x", err);
		ipmi_handler->connected = 0;
	} else {
	  	err("IPMI domain Connection success");
		ipmi_handler->connected = 1;
	}
	if (!still_connected) {
		err("All IPMI connections down\n");
		ipmi_handler->connected = 0;
	}
	if (ipmi_handler->connected == 0) {
		return;
	}
	rv = init_domain_handlers(domain, cb_data);
	if (rv) {
		/* we can do something better */
		err("Couldn't init_domain_handlers. rv = 0x%x", rv);
		ipmi_handler->connected = 0;
	}
	if (ipmi_handler->connected && ipmi_handler->openipmi_scan_time) {
		ipmi_domain_set_sel_rescan_time(domain,
					ipmi_handler->openipmi_scan_time);
	}
}




