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
 *      Steve Sherman <stevees@us.ibm.com>
 */

#include <SaHpi.h>
#include <openhpi.h>
#include <snmp_util.h>
#include <sim_resources.h>

int snmp_get(struct snmp_session *ss, const char *objid, struct snmp_value *value) 
{
	SnmpMibInfoT *hash_data;
	
	hash_data = (SnmpMibInfoT *)g_hash_table_lookup(sim_hash, objid);
	if (hash_data) {
		value->type = hash_data->type;
		switch (value->type) {
		case ASN_INTEGER:
			value->integer = hash_data->value.integer;
			break;
		case ASN_OCTET_STR:
			strcpy(value->string, hash_data->value.string); 
			break;
		default:
			dbg("Unknown SNMP type=%d for oid=%s\n", value->type, objid);
			return -1;
		}
	}
	else {
		dbg("No value in sim_hash for OID = %s\n", objid);
		return -1;
	}

	return 0;
}

int snmp_set(struct snmp_session *ss, char *objid, struct snmp_value value) 
{
	SnmpMibInfoT *hash_data;
	
	hash_data = (SnmpMibInfoT *)g_hash_table_lookup(sim_hash, objid);
	if (hash_data) {
		switch (hash_data->type) {
		case ASN_INTEGER:
		        dbg("Setting oid=%s with value=%d\n", objid, (int)hash_data->value.integer);
			break;
			
		case ASN_OCTET_STR:
			dbg("Setting oid=%s with value=%s\n", objid, hash_data->value.string);
			break;
		default:
			dbg("Unknown SNMP type=%d for oid=%s\n", hash_data->type, objid);
			return -1;
		}
	}
	else {
		dbg("No value in sim_hash for OID = %s\n", objid);
		return -1;
	}

	return 0;
}
