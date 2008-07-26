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
#include <oh_utils.h>
#include <string.h>

static void SDRs_read_done(ipmi_domain_t *domain, int err, void *cb_data)
{
	int *flag = cb_data;
	*flag = 1;
	dbg("SDRs read done");
	return;
}

static void SELs_read_done(ipmi_domain_t *domain, int err, void *cb_data)
{
	int *flag = cb_data;
	*flag = 1;
	dbg("SELs read done");
	return;
}

static void bus_scan_done(ipmi_domain_t *domain, int err, void *cb_data)
{
		struct ohoi_handler *ipmi_handler = (struct ohoi_handler *) cb_data;
		int rv;
		int *flag = &ipmi_handler->bus_scan_done;
		*flag = 1;
		dbg("bus scan done");
		
		/* we have MCs now, get SEL */
		rv = ipmi_domain_reread_sels(domain, SELs_read_done, &ipmi_handler->SELs_read_done);

		if (rv)
			err("ipmi_domain_reread_sels returned error: %d\n", rv);
	
		return;
}


void ohoi_setup_done(ipmi_domain_t	*domain,
                     void 		*user_data)
{

        
	struct oh_handler_state *handler = user_data;
	struct ohoi_handler *ipmi_handler = handler->data;
	int rv;

	rv = ipmi_domain_enable_events(domain);
	if (rv) {
		err("ipmi_domain_enable_events return error %d", rv);
	}


	rv = ipmi_domain_add_entity_update_handler(domain, ohoi_entity_event, 
			 			  handler);

	if (rv)
		err("ipmi_bmc_iterate_entities return error");
	
	rv = ipmi_domain_set_main_SDRs_read_handler(domain, SDRs_read_done,
                                                    &ipmi_handler->SDRs_read_done);
	if (rv)
		err("ipmi_domain_set_main_SDRs_read_handler return error: %d\n", rv);
	
	rv = ipmi_domain_set_bus_scan_handler(domain, bus_scan_done, ipmi_handler);
	if (rv) 
		err("ipmi_domain_set_bus_scan_handler return error: %d\n", rv);


    rv = ipmi_domain_add_mc_updated_handler(domain, ohoi_mc_event, handler);
    if (rv)
			err("ipmi_domain_register_mc_update_handler return error: %d\n", rv);
	
}

