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
#include <netdb.h>

/*
 * internally used structure for SEL time data
 **/
struct get_sel_time_cb_data {
	SaHpiTimeT	time;
	int		flag;
};

/*
 * SEL num. of entries callback function
 **/
static void get_sel_count(ipmi_mc_t *mc, void *cb_data)
{
	int *count = cb_data;
	
	*count = ipmi_mc_sel_entries_used(mc);
}

/**
 * ohoi_get_sel_count: Get IPMI number of SEL entries
 * @mc_id: management controller id used for OpenIPMI callback
 * @count: number of entries returned
 *
 * This function is called by ipmi.c
 * This function registers a callback with OpenIPMI's
 * ipmi_mc_pointer_cb, get's the MC associated with the ID
 * and calls our function back with the MC and fills in 
 * the number of SEL entries returned to caller.
 * 
 **/
void ohoi_get_sel_count(ipmi_mcid_t mc_id, int *count)
{
	int rv;	

	*count = -1;
	rv = ipmi_mc_pointer_cb(mc_id, get_sel_count, count);
	if (rv<0)
		dbg("Unable to convert MC id to a pointer");
}

/**
 * get_sel_time_cb: callback registered by get_sel_time
 * @mc: management controller pointer
 * @err: int error
 * @time: time returned from OpenIPMI
 * @cb_data: user requested data
 *
 *
 **/
static void get_sel_time_cb(ipmi_mc_t	*mc,
		int           		err,
		unsigned long 		time,
		void          		*cb_data)
{
	struct get_sel_time_cb_data *data = cb_data;
	data->flag = 1;
	data->time = time;
}

/**
 * get_sel_time: get IPMI SEL time
 * @mc: management controller id used for OpenIPMI callback
 * @cb_data: user passed in data
 *
 *
 **/
static void get_sel_time(ipmi_mc_t *mc, void *cb_data)
{
	ipmi_mc_get_current_sel_time(mc, get_sel_time_cb, cb_data);
}

void ohoi_get_sel_time(ipmi_mcid_t mc_id, SaHpiTimeT *time)
{
	struct get_sel_time_cb_data data;
	int rv;	

        memset(&data, 0, sizeof(data));
	rv = ipmi_mc_pointer_cb(mc_id, get_sel_time, &data); 
	if (rv) {
		dbg("Unable to convert domain id to a pointer");
                return;
        }

        rv = ohoi_loop(&data.flag);
        if (rv)
                dbg("Unable to get sel time: Timeout!");
        
        *time = (SaHpiTimeT)data.time*1000000000;
}

static void get_sel_update_timestamp(ipmi_mc_t *mc, void *cb_data)
{
        SaHpiTimeT *time = cb_data;
        
        *time = (SaHpiTimeT)ipmi_mc_sel_get_last_addition_timestamp(mc)*1000000000;
}

void ohoi_get_sel_updatetime(ipmi_mcid_t mc_id, SaHpiTimeT *time)
{
        int rv;

        rv = ipmi_mc_pointer_cb(mc_id, get_sel_update_timestamp, time);
        if (rv)
                dbg("Unable to convert domain id to a pointer");
}

static void get_sel_overflow(ipmi_mc_t *mc, void *cb_data)
{
	char *overflow = cb_data;
	*overflow =  ipmi_mc_sel_get_overflow(mc);
}

void ohoi_get_sel_overflow(ipmi_mcid_t mc_id, char *overflow)
{
	int rv;	

	rv = ipmi_mc_pointer_cb(mc_id, get_sel_overflow, overflow); 
	if (rv<0)
		dbg("Unable to convert domain id to a pointer");
	
}

static void get_sel_support_del(ipmi_mc_t *mc, void *cb_data)
{
	char *support_del = cb_data;

	*support_del = ipmi_mc_sel_get_supports_delete_sel(mc);
}

void ohoi_get_sel_support_del(ipmi_mcid_t mc_id, char *support_del)
{
	int rv;	

	rv = ipmi_mc_pointer_cb(mc_id, get_sel_support_del, support_del); 
	if (rv<0)
		dbg("Unable to convert domain id to a pointer");
}

static void set_sel_time_done(ipmi_mc_t	*mc,
		              int   	err,
		              void      *cb_data)
{
        int *flag = cb_data;
        *flag = 1;
}

struct set_sel_time_cb_data {
        int flag;
        struct timeval time;
};

static void set_sel_time(ipmi_mc_t *mc, void *cb_data)
{
        struct set_sel_time_cb_data *data = cb_data;
        
	ipmi_mc_set_current_sel_time(mc, &data->time, set_sel_time_done, &data->flag);
}

void ohoi_set_sel_time(ipmi_mcid_t mc_id, const struct timeval *time)
{
        struct set_sel_time_cb_data data;
	int rv;
	
        data.flag = 0;
        data.time = *time;
	rv = ipmi_mc_pointer_cb(mc_id, set_sel_time, &data); 
        if (rv) {
                dbg("Unable to convert MC id to a pointer");
                return;
        }
                
        rv = ohoi_loop(&data.flag);
        if (rv) 
                dbg("Unable to get SEL time: Timeout!");
        
	return;
}

static void clear_sel(ipmi_mc_t *mc, void *cb_data)
{
        int rv;
        ipmi_event_t event[1];
      
        rv = ipmi_mc_first_event(mc, event);
        while (!rv) {
                ipmi_mc_del_event(mc, event, NULL, NULL);
                rv = ipmi_mc_next_event(mc, event);
        }
}

SaErrorT ohoi_clear_sel(ipmi_mcid_t mc_id)
{
        char support_del;
        int rv;
        
        ohoi_get_sel_support_del(mc_id, &support_del);
        if (!support_del) {
                dbg("MC SEL doesn't support del");
                return SA_ERR_HPI_INVALID_CMD;
        }

        rv = ipmi_mc_pointer_cb(mc_id, clear_sel, NULL);
        if (rv) {
                dbg("Unable to convert mcid to pointer: %d", rv);
                return SA_ERR_HPI_INVALID;
        }
        
        return SA_OK;
}


static void get_sel_first_entry(ipmi_mc_t *mc, void *cb_data)
{
	ipmi_event_t *event = cb_data;
	
	ipmi_mc_first_event(mc, event);
}

void ohoi_get_sel_first_entry(ipmi_mcid_t mc_id, ipmi_event_t *event)
{
	int rv;
	
	rv = ipmi_mc_pointer_cb(mc_id, get_sel_first_entry, event);

	if (rv)
		dbg("Unable to convert mcid to pointer");
}

static void get_sel_last_entry(ipmi_mc_t *mc, void *cb_data)
{
	ipmi_event_t *event = cb_data;
	
	ipmi_mc_last_event(mc, event);
}

void ohoi_get_sel_last_entry(ipmi_mcid_t mc_id, ipmi_event_t *event)
{
	int rv;
	
	rv = ipmi_mc_pointer_cb(mc_id, get_sel_last_entry, event);

	if (rv)
		dbg("Unable to convert mcid to pointer");
}

static void get_sel_next_entry(ipmi_mc_t *mc, void *cb_data)
{
	int rv;
	ipmi_event_t *event = cb_data;

	rv = ipmi_mc_next_event(mc, event);
	if (rv)
		event->record_id = SAHPI_NO_MORE_ENTRIES;
}

void ohoi_get_sel_next_recid(ipmi_mcid_t mc_id, 
                             const ipmi_event_t *event,
                             unsigned int *record_id)
{
        ipmi_event_t te;
	int rv;

        te = *event;
	rv = ipmi_mc_pointer_cb(mc_id, get_sel_next_entry, &te);
	if (rv)
		dbg("unable to convert mcid to pointer");
        *record_id  = te.record_id;
}

static void get_sel_prev_entry(ipmi_mc_t *mc, void *cb_data)
{
	int rv;
	ipmi_event_t *event = cb_data;

	rv = ipmi_mc_prev_event(mc, event);
	if (rv)
		event->record_id = SAHPI_NO_MORE_ENTRIES;
}

void ohoi_get_sel_prev_recid(ipmi_mcid_t mc_id, 
                             const ipmi_event_t *event, 
                             unsigned int *record_id)
{
        ipmi_event_t te;
	int rv;

        te = *event;
	rv = ipmi_mc_pointer_cb(mc_id, get_sel_prev_entry, &te);
	if (rv)
		dbg("unable to convert mcid to pointer");
        *record_id  = te.record_id;
}

static void get_sel_by_recid(ipmi_mc_t *mc, void *cb_data)
{
	ipmi_event_t *event = cb_data;
	
	ipmi_mc_event_by_recid(mc, event->record_id, event);
}

void ohoi_get_sel_by_recid(ipmi_mcid_t mc_id, SaHpiSelEntryIdT entry_id, ipmi_event_t *event)
{
	int rv;
	event->record_id = entry_id;

	rv = ipmi_mc_pointer_cb(mc_id, get_sel_by_recid, event);

	if(rv)
		dbg("failed to convert mc_id to pointer");
}
	
