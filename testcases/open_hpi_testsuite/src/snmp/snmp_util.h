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

#ifndef SNMP_BC_UTIL_H
#define SNMP_BC_UTIL_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#define MAX_ASN_STR_LEN 300

/* Place-holder for values set and returned by snmp */
struct snmp_value {
        u_char type;
        char string[MAX_ASN_STR_LEN];
        unsigned int str_len;
        long integer;
};

int snmp_get(
        struct snmp_session *ss,
        const char *objid,
        struct snmp_value *value);

int snmp_set(
        struct snmp_session *ss,
        char *objid,
        struct snmp_value value);

#endif
