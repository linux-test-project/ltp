/*      -*- linux-c -*-
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
 *      Renier Morales <renierm@users.sf.net>
 *
 */

#ifndef SNMP_BC_SESSION_H
#define SNMP_BC_SESSION_H

#include <SaHpi.h>
#include <openhpi.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

/**
 * This handle will be unique per instance of
 * this plugin. SNMP session data is stored
 * in the handle along with config file data.
 **/

void *snmp_bc_open(GHashTable *handler_config);
void snmp_bc_close(void *hnd);

#endif
