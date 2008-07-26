/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2008
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Renier Morales <renier@openhpi.org>
 *
 */

#include <glib.h>
#include <SaHpi.h>

struct oh_domain_conf {
        SaHpiDomainIdT did;
        char host[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        unsigned int port;
};

int oh_load_client_config(const char *filename, GHashTable *domains);
