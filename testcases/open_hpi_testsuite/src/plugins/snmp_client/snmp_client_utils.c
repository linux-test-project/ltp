/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *	David Judkovics <djudkovi@us.ibm.com>
 *
 */
 

#include <stdio.h>
#include <time.h>

#include <SaHpi.h>
#include <openhpi.h>
#include <epath_utils.h>
#include <rpt_utils.h>
#include <uid_utils.h>
#include <snmp_util.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <snmp_client.h>
#include <snmp_client_res.h>
#include <snmp_client_utils.h>

#include <netinet/in.h>

int snmp_getn_bulk( struct snmp_session *ss, 
		    oid *bulk_objid, 
		    size_t bulk_objid_len,
		    struct snmp_pdu *bulk_pdu, 
		    struct snmp_pdu **bulk_response,
		    int num_repetitions )
{
//	size_t anOID_len = MAX_OID_LEN;
//	oid anOID[MAX_OID_LEN];
	int status;


	/* Create the PDU for theenrty_count data for our request. */
//	read_objid(bulk_objid, anOID, &anOID_len);

	bulk_pdu = snmp_pdu_create(SNMP_MSG_GETBULK);
 	
	bulk_pdu->non_repeaters = 0; 
	
	bulk_pdu->max_repetitions = num_repetitions;

printf("OID LENGTH %d\n", bulk_objid_len);
printf("Num Repetitions %d\n", num_repetitions);
printf("OID %d\n", (int)bulk_objid[0]);
printf("OID %d\n", (int)bulk_objid[1]);
printf("OID %d\n", (int)bulk_objid[2]);
printf("OID %d\n", (int)bulk_objid[3]);
printf("OID %d\n", (int)bulk_objid[4]);
printf("OID %d\n", (int)bulk_objid[5]);

	
	snmp_add_null_var(bulk_pdu, bulk_objid, bulk_objid_len);
	
	/* Send the Request out.*/
	status = snmp_synch_response(ss, bulk_pdu, bulk_response);

	return(status);

}

