/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
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
 *     Louis Zhuang <louis.zhuang@linux.intel.com>
 *     David Judkovics <djudkovi@us.ibm.com> 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SaHpi.h>
#include <openhpi.h>

#if 0
static inline int __dsel_add(GSList **sel_list, 
                const SaHpiSelEntryT *entry, int counter)
{
        struct oh_dsel *dsel;
        
        data_access_lock();

        dsel = malloc(sizeof(*dsel));
        if (!dsel) {
                dbg("Out of memory");
                data_access_unlock();
                return -1;
        }
        memset(dsel, 0, sizeof(*dsel));

        memcpy(&dsel->entry, entry, sizeof(*entry));
        dsel->entry.EntryId = counter;
        *sel_list = g_slist_append(*sel_list, dsel);

        data_access_unlock();

        return 0;
}

static inline void __dsel_clr(GSList **sel_list) 
{
        GSList *i, *i2;

        data_access_lock();

        g_slist_for_each_safe(i, i2, *sel_list) {
                struct oh_sel *sel;
                sel = i->data;
                *sel_list = g_slist_remove_link(*sel_list, i);
                free(sel);                      
        }

        data_access_unlock();

}

static inline void __rsel_clr(struct oh_resource *res, GSList **sel_list)
{
        GSList *i, *i2;

        data_access_lock();

        g_slist_for_each_safe(i, i2, *sel_list) {
                struct oh_rsel *rsel;
                rsel = i->data;
                *sel_list = g_slist_remove_link(*sel_list, i);
                res->handler->abi->del_sel_entry(res->handler->hnd, rsel->oid);
                free(rsel);                     
        }
        
        data_access_unlock();

}

int dsel_get_info(SaHpiDomainIdT domain_id, SaHpiSelInfoT *info)
{
        struct oh_domain *d;
        
        data_access_lock();
        
        d = get_domain_by_id(domain_id);
        if (!d) {
                dbg("Invalid domain");
                data_access_unlock();
                return -1;
        }
        
        info->Entries                   = g_slist_length(d->sel_list);
        info->Size                      = 0xFFFFFFFF;
        info->UpdateTimestamp           = 0;
        gettimeofday1(&info->CurrentTime);
        info->Enabled   = (d->sel_state==OH_SEL_ENABLED) ? 1 : 0;
        info->OverflowFlag              = 0;
        info->OverflowAction            = SAHPI_SEL_OVERFLOW_DROP;
        info->DeleteEntrySupported      = 1;

        data_access_unlock();

        return 0;
}

int dsel_get_state(SaHpiDomainIdT domain_id)
{
        struct oh_domain *d;
        
        d = get_domain_by_id(domain_id);
        if (!d) {
                dbg("Invalid domain");
                return -1;
        }
        
        return (d->sel_state==OH_SEL_ENABLED) ? 1 : 0;
}

int dsel_set_state(SaHpiDomainIdT domain_id, int enable)
{
        struct oh_domain *d;
        
        d = get_domain_by_id(domain_id);
        if (!d) {
                dbg("Invalid domain");
                return -1;
        }
        
        d->sel_state = enable ? OH_SEL_ENABLED : OH_SEL_DISABLED;
        return 0;
}

SaHpiTimeT dsel_get_time(SaHpiDomainIdT domain_id)
{
        SaHpiTimeT cur;
        gettimeofday1(&cur);
        return cur;
}

void dsel_set_time(SaHpiDomainIdT domain_id, SaHpiTimeT time)
{
        /* just escape */
        return; 
}

int dsel_add(SaHpiDomainIdT domain_id, SaHpiSelEntryT *entry)
{
        struct oh_domain *d;
        
        dbg("DSEL from application!");
        d = get_domain_by_id(domain_id);
        if (!d) {
                dbg("Invalid domain");
                return -1;
        }

        d->sel_counter++;
        return __dsel_add(&d->sel_list, entry, d->sel_counter);
}

int dsel_add2(struct oh_domain *d, struct oh_hpi_event *e)
{
        struct oh_dsel *dsel;
        
        data_access_lock();

//      dbg("DSEL from plugin!");
        dsel = malloc(sizeof(*dsel));
        if (!dsel) {
                dbg("Out of memory");
                data_access_unlock();
                return -1;
        }
        memset(dsel, 0, sizeof(*dsel));

        dsel->res_id            = e->parent;
        dsel->rdr_id            = e->id;
        dsel->entry.EntryId     = d->sel_counter++;
        gettimeofday1(&dsel->entry.Timestamp);
        
        memcpy(&dsel->entry.Event, &e->event, sizeof(dsel->entry.Event));
        
        d->sel_list = g_slist_append(d->sel_list, dsel);

        data_access_unlock();

        return 0;
}

int dsel_del(SaHpiDomainIdT domain_id, SaHpiSelEntryIdT id)
{
        struct oh_domain *d;
        GSList *i;

        data_access_lock();

        d = get_domain_by_id(domain_id);
        if (!d) {
                dbg("Invalid domain");
                data_access_unlock();
                return -1;
        }

        g_slist_for_each(i, d->sel_list) {
                struct oh_dsel *dsel;
                dsel = i->data;
                if (dsel->entry.EntryId == id) {
                        d->sel_list = g_slist_remove_link(d->sel_list, i);
                        free(dsel);
                        break;
                }
        }
        
        if (!i) {
                dbg("No such entry");
                data_access_unlock();
                return -1;
        }

        data_access_unlock();
        
        return 0;
}

int dsel_clr(SaHpiDomainIdT domain_id) 
{
        struct oh_domain *d;
        
        d = get_domain_by_id(domain_id);
        if (!d) {
                dbg("Invalid domain");
                return -1;
        }

        __dsel_clr(&d->sel_list);
        return 0;
}

/* here is a little tricky. rsel_add doesn't add the event into
 * sel_list in resource. Instead, it is plug-in's repository to
 * construct a RSEL event and send the event into infrastruture
 * so that infrastructure can get the entry into resource's 
 * sel_list
 */
int rsel_add(SaHpiResourceIdT res_id, SaHpiSelEntryT *entry)
{
        struct oh_resource *res;
        
        res = get_resource(res_id);
        if (!res) {
                dbg("Invalid resource id");
                return -1;
        }
        
        if (!res->handler->abi->add_sel_entry) {
                dbg("No such function");
                return -1;
        }
        if (res->handler->abi->add_sel_entry(res->handler->hnd, res->oid, entry)<0) {
                dbg("Error when calling");
                return -1;
        }
        
        return 0;
}

int rsel_add2(struct oh_resource *res, struct oh_rsel_event *e)
{
        struct oh_rsel *rsel;

        data_access_lock();
        
        rsel = malloc(sizeof(*rsel));
        if (!rsel) {
                dbg("Out of memory");
                data_access_unlock();
                return -1;
        }
        memset(rsel, 0, sizeof(*rsel));

        *rsel = e->rsel;
        rsel->entry_id = res->sel_counter++;    
        res->sel_list = g_slist_append(res->sel_list, rsel);

        data_access_unlock();

        return 0;
}

int rsel_del(SaHpiResourceIdT res_id, SaHpiSelEntryIdT id)
{
        struct oh_resource *res;
        GSList *i;
        
        data_access_lock();

        res = get_resource(res_id);
        if (!res) {
                dbg("Invalid resource id");
                data_access_unlock();
                return -1;
        }

        g_slist_for_each(i, res->sel_list) {
                struct oh_rsel *rsel;
                SaHpiSelEntryT entry;
                
                rsel = i->data;
                res->handler->abi->get_sel_entry(res->handler->hnd, rsel->oid, &entry);
                if (entry.EntryId == id) {
                        res->sel_list = g_slist_remove_link(res->sel_list, i);
                        free(rsel);
                        break;
                }
        }
        
        if (!i) {
                dbg("No such entry");
                data_access_unlock();
                return -1;
        }
        
        data_access_unlock();

        return 0;
}

int rsel_clr(SaHpiResourceIdT res_id) 
{
        struct oh_resource *res;
        
        res = get_resource(res_id);
        if (!res) {
                dbg("Invalid resource id");
                return -1;
        }

        __rsel_clr(res, &res->sel_list);
        return 0;
}
#endif
