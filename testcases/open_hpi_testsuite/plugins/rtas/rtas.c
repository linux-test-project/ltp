/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *        Renier Morales <renier@openhpi.org>
 */

#include <rtas.h>

void *rtas_open(GHashTable *handler_config, unsigned int hid, oh_evt_queue *eventq)
{
        struct oh_handler_state *h = NULL;
	char *entity_root = NULL;

        if (!handler_config) {
                err("No configuration passed.");
                return NULL;
        } else if (!hid) {
                err("Bad handler id passed.");
                return NULL;
        } else if (!eventq) {
                err("No event queue was passed.");
                return NULL;
        }

        entity_root = (char *)g_hash_table_lookup(handler_config, "entity_root");
	if (!entity_root) {
	        err("Cannot find \"entity_root\" configuration parameter.");
		return NULL;
	}

	h = (struct oh_handler_state *)g_malloc0(sizeof(struct oh_handler_state));
	h->config = handler_config;

        h->rptcache = (RPTable *)g_malloc0(sizeof(RPTable));
        oh_init_rpt(h->rptcache);

        h->elcache = oh_el_create(0);
        h->elcache->gentimestamp = FALSE;

        h->hid = hid;
        h->eventq = eventq;

        return h;
}

void rtas_close(void *hnd)
{
        return;
}

				  			
/* Function ABI aliases */
void * oh_open (GHashTable *, unsigned int, oh_evt_queue *) __attribute__ ((weak, alias("rtas_open")));
void * oh_close (void *) __attribute__ ((weak, alias("rtas_close")));
