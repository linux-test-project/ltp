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
 * Author: Louis Zhuang <louis.zhuang@linux.intel.com>
 *         David Judkovics <djudkovi@us.ibm.com> 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SaHpi.h>
#include <openhpi.h>

GSList *global_rpt = NULL;
unsigned int global_rpt_counter = 0; /*XXX: I use the couter for two purposes. 
                                   1) RptInfo counter 2) ResourceId allocation */
struct timeval global_rpt_timestamp = {0, 0};

struct oh_resource *get_resource(SaHpiResourceIdT rid)
{
        GSList *i;

        data_access_lock();
        
        g_slist_for_each(i, global_rpt) {
                struct oh_resource *res = i->data;
                if (res->entry.ResourceId == rid) {
                        data_access_unlock();
                        return res;
                }
        }

        data_access_unlock();
        
        return NULL;
}

int resource_is_in_domain(struct oh_resource *res, SaHpiDomainIdT did)
{
        GSList *i;
        
        data_access_lock();
        
        g_slist_for_each(i, res->domain_list) {
                SaHpiDomainIdT id = GPOINTER_TO_UINT(i->data);
                if (id == did) {
                        data_access_unlock();
                        return 1;
                }
        }

        data_access_unlock();
        
        return 0;
}
