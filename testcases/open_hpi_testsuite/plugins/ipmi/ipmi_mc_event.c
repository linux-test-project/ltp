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
 */

#include "ipmi.h"
#include <oh_utils.h>
#include <string.h>




static void trace_ipmi_mc(char *str, ipmi_mc_t *mc)
{
	if (!getenv("OHOI_TRACE_MC") && !IHOI_TRACE_ALL) {
		return;
	}
	fprintf(stderr, "*** MC (%d, %d):  %s. sel support = %d\n",
                 ipmi_mc_get_channel(mc), 
                 ipmi_mc_get_address(mc), str,
		 ipmi_mc_sel_device_support(mc));
}

static void mc_add(ipmi_mc_t                    *mc,
                   struct oh_handler_state      *handler)
{
	trace_ipmi_mc("ADDED and ACTIVE", mc);
}


static void mc_remove(ipmi_mc_t                    *mc,
                   struct oh_handler_state      *handler)
{
	struct ohoi_handler *ipmi_handler = handler->data;
	ipmi_mcid_t mcid;
	SaHpiRptEntryT *rpt;

	trace_ipmi_mc("REMOVED (not present)", mc);
	if (!IS_ATCA(ipmi_handler->d_type)) {
		return;
	}
	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
	mcid    = ipmi_mc_convert_to_id(mc);
	ohoi_atca_delete_fru_rdrs(handler, mcid);
	rpt = ohoi_get_resource_by_mcid(handler->rptcache, &mcid);
	if (rpt == NULL) {
		trace_ipmi_mc("COULDN'T FIND RPT", mc);
		err("couldn't find out resource");
	} else {
		rpt->ResourceCapabilities &=
			~SAHPI_CAPABILITY_EVENT_LOG;
	}
	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
}


static
void mc_active(ipmi_mc_t *mc, int active, void *cb_data)
{
	struct oh_handler_state *handler = cb_data;
//	struct ohoi_handler *ipmi_handler = handler->data;

	if (active) {
		mc_add(mc, handler);
	} else {
		mc_remove(mc, handler);
	}
}


static
void process_sel_support(ipmi_mc_t *mc, struct oh_handler_state *handler)
{
        struct ohoi_resource_info *res_info;
	struct ohoi_handler *ipmi_handler = handler->data;
	ipmi_mcid_t mcid;
	SaHpiRptEntryT *rpt;
        

        mcid    = ipmi_mc_convert_to_id(mc);
	rpt = ohoi_get_resource_by_mcid(handler->rptcache, &mcid);
	if (rpt == NULL) {
		trace_ipmi_mc("COULDN'T FIND RPT", mc);
		err("couldn't find out resource");
		return;
	}
	res_info =  oh_get_resource_data(handler->rptcache, rpt->ResourceId);
	if (ipmi_mc_sel_device_support(mc)) {
		rpt->ResourceCapabilities |=
					SAHPI_CAPABILITY_EVENT_LOG;
		entity_rpt_set_updated(res_info, ipmi_handler);
	}
}
	


static
void mc_processed(ipmi_mc_t *mc, void *cb_data)
{
	struct oh_handler_state *handler = cb_data;
	struct ohoi_handler *ipmi_handler = handler->data;

	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
	if (ipmi_mc_is_active(mc)) {
		process_sel_support(mc, handler);
	} else {
		trace_ipmi_mc("NOT ACTIVE IN PROCESSED", mc);
	}
	if (!ipmi_handler->fully_up) {
		// do nothing. we''ll process everything when
		// domain is fully up
	 	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		trace_ipmi_mc("PROCESSED, domain not fully up", mc);
		return;
	}
	trace_ipmi_mc("PROCESSED, handle this event", mc);
	if (IS_ATCA(ipmi_handler->d_type)) {
		ohoi_atca_create_fru_rdrs(handler);
	}
	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
}


void
ohoi_mc_event(enum ipmi_update_e op,
              ipmi_domain_t      *domain,
              ipmi_mc_t          *mc,
              void               *cb_data)
{

        struct oh_handler_state *handler = cb_data;
		struct ohoi_handler *ipmi_handler = handler->data;
		int rv;
		
	if ((ipmi_mc_get_channel(mc) == 0) && 
                 (ipmi_mc_get_address(mc) == 32) &&
		 ipmi_handler->d_type == IPMI_DOMAIN_TYPE_ATCA) {
		 	ipmi_handler->virt_mcid = ipmi_mc_convert_to_id(mc);
	}

	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);			
        switch (op) {
                case IPMI_ADDED:
			/* if we get an MC but inactive, register a call to add
			it once it goes active */
			rv = ipmi_mc_add_active_handler(mc, mc_active, handler);
			rv = ipmi_mc_set_sdrs_first_read_handler(mc,
				mc_processed, handler);
			if(!ipmi_mc_is_active(mc)) {
				trace_ipmi_mc("ADDED but inactive...we ignore", mc);
				break;
			} else {
				mc_add(mc, handler);
				break;
			}
		case IPMI_DELETED:
			trace_ipmi_mc("DELETED, but nothing done", mc);
			break;

		case IPMI_CHANGED:
			if(!ipmi_mc_is_active(mc)) {
				trace_ipmi("CHANGED and is inactive: (%d %x)\n",
						ipmi_mc_get_address(mc), 
						ipmi_mc_get_channel(mc));
			} else {
				mc_add(mc, handler);
			}
			break;
		}
	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);

}

