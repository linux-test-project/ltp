/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 */

#ifndef __SIM_RESOURCES_H
#define __SIM_RESOURCES_H

typedef union {
	char string[MAX_ASN_STR_LEN];
        long integer;
} SnmpValueT;
 
typedef struct {
	int type;
	SnmpValueT value;  
} SnmpMibInfoT;

struct snmp_bc_data {
        const char *oid;
	SnmpMibInfoT mib;
};

extern GHashTable * sim_hash;
extern struct snmp_bc_data sim_resource_array[];

#define SNMP_FORCE_TIMEOUT -7777
#define SNMP_FORCE_ERROR -9999

#endif
