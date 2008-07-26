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
#include <snmp_utils.h>
#include <sim_resources.h>

int snmp_get(void *sessp, const char *objid, struct snmp_value *value) 
{
	SnmpMibInfoT *hash_data;
	
	hash_data = (SnmpMibInfoT *)g_hash_table_lookup(sim_hash, objid);
	if (hash_data) {
		value->type = hash_data->type;
		switch (value->type) {
		case ASN_INTEGER:
			value->integer = hash_data->value.integer;
			if (value->integer == SNMP_FORCE_ERROR) { return -1; }
			if (value->integer == SNMP_FORCE_TIMEOUT) {return SA_ERR_HPI_TIMEOUT;}
			if (value->integer == SA_ERR_SNMP_NOSUCHOBJECT) { return SA_ERR_HPI_NOT_PRESENT; }
			break;
		case ASN_OCTET_STR:
			strcpy(value->string, hash_data->value.string);
			value->str_len = strlen(hash_data->value.string);
			break;
		default:
			err("Unknown SNMP type=%d for oid=%s\n", value->type, objid);
			return -1;
		}
	}
	else {
		err("No value in sim_hash for OID = %s\n", objid);
		return SA_ERR_HPI_NOT_PRESENT;
	}

	return 0;
}

int snmp_set(void *sessp, char *objid, struct snmp_value value) 
{
	SnmpMibInfoT *hash_data;
	
	hash_data = (SnmpMibInfoT *)g_hash_table_lookup(sim_hash, objid);
	if (hash_data) {
		switch (hash_data->type) {
		case ASN_INTEGER:
		        err("Setting oid=%s with value=%d\n", objid, (int)hash_data->value.integer);
			if (hash_data->value.integer == SNMP_FORCE_TIMEOUT) {return SA_ERR_HPI_TIMEOUT;}
			hash_data->value.integer = value.integer;
			break;
			
		case ASN_OCTET_STR:
			err("Setting oid=%s with value=%s\n", objid, hash_data->value.string);
			strcpy(hash_data->value.string, value.string);
			break;
		default:
			err("Unknown SNMP type=%d for oid=%s\n", hash_data->type, objid);
			return -1;
		}
	}
	else {
		err("No value in sim_hash for OID = %s\n", objid);
		return SA_ERR_HPI_NOT_PRESENT;
	}

	return 0;
}

int snmp_getn_bulk( void *sessp,
                    oid *bulk_objid,
                    size_t bulk_objid_len,
                    struct snmp_pdu *bulk_pdu,
                    struct snmp_pdu **bulk_response,
                    int num_repetitions )
{
	return 0;

}

