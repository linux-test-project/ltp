/*      -*- linux-c -*-
 *
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
#include <SaHpi.h>
#include <openhpi.h>


#include <SaHpi.h>
#include <openhpi.h>
#include <epath_utils.h>
#include <rpt_utils.h>
#include <uid_utils.h>
#include <snmp_util.h>


#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h> 

#include <snmp_client.h>
#include <snmp_client_res.h>


static SaErrorT remote_rdr_data_free(SaHpiRdrT *rdr, gpointer data);
static SaErrorT remote_res_data_free(SaHpiRptEntryT *rdr, gpointer data);

struct oh_event *eventdup(const struct oh_event *event)
{
	struct oh_event *e;
	e = g_malloc0(sizeof(*e));
	if (!e) {
		dbg("Out of memory!");
		return NULL;
	}
	memcpy(e, event, sizeof(*e));
	return e;
}

int eventq_event_add(struct oh_handler_state *oh_hnd)
{
	struct oh_event event;
	SaHpiRptEntryT *rpt_entry;
	SaHpiRdrT      *rdr_entry;

	/* get the first rpt entry */
	
	rpt_entry = oh_get_resource_next(oh_hnd->rptcache, RPT_ENTRY_BEGIN);
	
	while (rpt_entry) {
		memset(&event, 0, sizeof(event));
		event.type = OH_ET_RESOURCE;
		memcpy(&event.u.res_event.entry, rpt_entry, sizeof(SaHpiRptEntryT));
		oh_hnd->eventq = g_slist_append(oh_hnd->eventq, eventdup(&event) );
	
	
		/* get every resource rdr's */
		rdr_entry = oh_get_rdr_next(oh_hnd->rptcache, 
					    rpt_entry->ResourceId, 
					    RDR_BEGIN);
		while (rdr_entry) {
			memset(&event, 0, sizeof(event));
			event.type = OH_ET_RDR;
			memcpy(&event.u.rdr_event.rdr, rdr_entry, sizeof(SaHpiRdrT));
			oh_hnd->eventq = g_slist_append(oh_hnd->eventq, eventdup(&event));
			rdr_entry = oh_get_rdr_next(oh_hnd->rptcache, 
						    rpt_entry->ResourceId, 
						    rdr_entry->RecordId);
		}
		/* get any resource rdr's end */
	
	
		rpt_entry = oh_get_resource_next(oh_hnd->rptcache, rpt_entry->ResourceId);
	}

	
	return SA_OK;
}


void process_diff_table(struct oh_handler_state *handle, RPTable *diff_table) 
{

        /* Rediscovery: Get difference between current rptcache and tmpcache. */
	/* Delete obsolete items from rptcache and add new items in.   	      */
	GSList *res_new = NULL;
	GSList *rdr_new = NULL;
        GSList *res_gone = NULL; 
	GSList *rdr_gone = NULL;

	GSList *node = NULL;
        
        rpt_diff(handle->rptcache, diff_table, &res_new, &rdr_new, &res_gone, &rdr_gone);

        for (node = rdr_gone; node != NULL; node = node->next) {

                SaHpiRdrT *rdr = (SaHpiRdrT *)node->data;				
                SaHpiRptEntryT *res = oh_get_resource_by_ep(handle->rptcache, &(rdr->Entity));

		/* Create remove rdr event and add to event queue */
		struct oh_event *e = (struct oh_event *)g_malloc0(sizeof(struct oh_event));
		e->type = OH_ET_RDR_DEL;
		e->u.rdr_del_event.record_id = rdr->RecordId;
        	e->u.rdr_del_event.parent_entity = rdr->Entity;	    
		handle->eventq = g_slist_append(handle->eventq, e);

        	/* free rdr remote data */
                gpointer data = oh_get_rdr_data(diff_table, res->ResourceId, rdr->RecordId);
		remote_rdr_data_free(rdr, data);
		
		/* Remove rdr from plugin's rpt cache */
                oh_remove_rdr(handle->rptcache, res->ResourceId, rdr->RecordId);
        }
        g_slist_free(rdr_gone);

        for (node = res_gone; node != NULL; node = node->next) {

                SaHpiRptEntryT *res = (SaHpiRptEntryT *)node->data;

		/* Create remove resource event and add to event queue */
		struct oh_event *e = (struct oh_event *)g_malloc0(sizeof(struct oh_event));
		e->type = OH_ET_RESOURCE_DEL;
        	e->u.res_del_event.resource_id = res->ResourceId;
		handle->eventq = g_slist_append(handle->eventq, e);
 
		/* free the remote resource data */
                gpointer data = oh_get_resource_data(diff_table, res->ResourceId);
		remote_res_data_free(res, data);

		/* Remove resource from plugin's rpt cache */
                oh_remove_resource(handle->rptcache, res->ResourceId);
        }
        g_slist_free(res_gone);

        for (node = res_new; node != NULL; node = node->next) {

                SaHpiRptEntryT *res = (SaHpiRptEntryT *)node->data;

                gpointer data = oh_get_resource_data(diff_table, res->ResourceId);
                oh_add_resource(handle->rptcache, res, data, 1);

		/* Add new/changed resources to the event queue */
		struct oh_event *e = (struct oh_event *)g_malloc0(sizeof(struct oh_event));
		e->type = OH_ET_RESOURCE;
		e->u.res_event.entry.ResourceId = res->ResourceId;
		memcpy(&e->u.res_event.entry, res, sizeof(*res));
		handle->eventq = g_slist_append(handle->eventq, e);
			     
        }
        g_slist_free(res_new);
        
        for (node = rdr_new; node != NULL; node = node->next) {
                
		SaHpiRdrT *rdr = (SaHpiRdrT *)node->data;
                
		SaHpiRptEntryT *res = oh_get_resource_by_ep(handle->rptcache, &(rdr->Entity));
                gpointer data = oh_get_rdr_data(diff_table, res->ResourceId, rdr->RecordId);
                  
		oh_add_rdr(handle->rptcache, res->ResourceId, rdr, data, 1);

		/* Add new/changed rdrs to the event queue */
		struct oh_event *e = (struct oh_event *)g_malloc0(sizeof(struct oh_event));
		e->type = OH_ET_RDR;
		e->u.rdr_event.rdr.RecordId = rdr->RecordId;
		memcpy(&e->u.rdr_event.rdr, rdr, sizeof(*rdr)); 
		handle->eventq = g_slist_append(handle->eventq, e);

        }        
        g_slist_free(rdr_new);
        
	/* Clean up tmpqueue and tmpcache */
        
}

static SaErrorT remote_rdr_data_free(SaHpiRdrT *rdr, gpointer data)
{

	printf("TODO: big bad memory leak");

	return(SA_OK);
}

static SaErrorT remote_res_data_free(SaHpiRptEntryT *rdr, gpointer data)
{

	printf("TODO: big bad memory leak");

	return(SA_OK);
}
