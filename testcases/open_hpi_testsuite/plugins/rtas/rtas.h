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

#ifndef RTAS_H
#define RTAS_H

#include <glib.h>
#include <SaHpi.h>
#include <oh_handler.h>
#include <oh_utils.h>
#include <oh_error.h>

void *rtas_open(GHashTable *handler_config, unsigned int hid, oh_evt_queue *eventq);

void rtas_close(void *hnd);
		
#endif /* _OH_RTAS_H */
