/* -*- linux-c -*-
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
 * Author(s):
 *     Steve Sherman <stevees@us.ibm.com>
 */

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <SaHpi.h>
#include <snmp_bc_utils.h>

/**
 * Tests SNMP OID derive function - success case
 * - multiple substitutions
 * - expanded returned oid string
 * 
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv) 
{
	gchar *oid, *in_oid, *expected_oid;
	SaHpiEntityPathT ep = {
		.Entry[0] = 
		{
			.EntityType = SAHPI_ENT_SYS_MGMNT_MODULE,
			.EntityInstance = 100
		},
		{
			.EntityType = SAHPI_ENT_SUB_CHASSIS,
			.EntityInstance = 99
		}
	};

	/********************************************* 
	 * Multiple character/digit expansion TestCase
	 *********************************************/
	in_oid = "1.x.3.x";
	expected_oid = "1.99.3.100";

	oid = snmp_derive_objid(ep, in_oid);
	if (strcmp(expected_oid, oid)) {
		g_free(oid);
		printf("Error! Multiple character/digit expansion TestCase\n");
		printf("snmp_derive_objid returned oid=%s\n", oid);
                return -1;
        }
        g_free(oid);

	/*********************** 
	 * No expansion TestCase
	 ***********************/
	in_oid = "1.99.3.100";
	expected_oid = "1.99.3.100";

	oid = snmp_derive_objid(ep, in_oid);
	if (strcmp(expected_oid, oid)) {
		g_free(oid);
		printf("Error! No expansion TestCase\n");
		printf("snmp_derive_objid returned oid=%s\n", oid);
                return -1;
        }
        g_free(oid);


        return 0;
}

