/* -*- linux-c -*-
 * 
 * (C) Copyright IBM Corp. 2004, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
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
#include <oh_utils.h>

/**
 * Tests string derive function - success case
 * - multiple substitutions
 * - expanded returned oid string
 * 
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv) 
{
	/*****************************
	 * oh_derive_string testcases
         *****************************/
	gchar *oid, *in_oid, *expected_oid;
	SaHpiEntityPathT ep = {
		.Entry[0] = 
		{
			.EntityType = SAHPI_ENT_SYS_MGMNT_MODULE,
			.EntityLocation = 100
		},
		{
			.EntityType = SAHPI_ENT_SUB_CHASSIS,
			.EntityLocation = 99
		}
	};

	/* Multiple character/digit expansion testcase */
	in_oid = "1.x.3.x";
	expected_oid = "1.99.3.100";

	oid = oh_derive_string(&ep, 0, 10, in_oid);
	if (strcmp(expected_oid, oid)) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received OID=%s; Expected OID=%s\n", oid, expected_oid);
		g_free(oid);
                return -1;
        }
        g_free(oid);

	/* location offset testcase */
	in_oid = "1.x.3.x";
	expected_oid = "1.109.3.110";

	oid = oh_derive_string(&ep, 10, 10, in_oid);
	if (strcmp(expected_oid, oid)) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received OID=%s; Expected OID=%s\n", oid, expected_oid);
		g_free(oid);
                return -1;
        }
        g_free(oid);

	/* base and offset testcase */
	in_oid = "1.x.3.x";
	expected_oid = "1.6D.3.6E";

	oid = oh_derive_string(&ep, 10, 16, in_oid);
	if (strcmp(expected_oid, oid)) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received OID=%s; Expected OID=%s\n", oid, expected_oid);
		g_free(oid);
                return -1;
        }
        g_free(oid);

	/* No expansion testcase */
	in_oid = "1.99.3.100";
	expected_oid = "1.99.3.100";

	oid = oh_derive_string(&ep, 0, 10, in_oid);
	if (strcmp(expected_oid, oid)) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received OID=%s; Expected OID=%s\n", oid, expected_oid);
		g_free(oid);
                return -1;
        }
        g_free(oid);

	/* Event testcase */
	SaHpiEntityPathT ep2 = {
		.Entry[0] = 
		{
			.EntityType = SAHPI_ENT_SYS_MGMNT_MODULE,
			.EntityLocation = 11
		},
		{
			.EntityType = SAHPI_ENT_SUB_CHASSIS,
			.EntityLocation = 14
		}
	};

	in_oid = "1.x.3.x";
	expected_oid = "1.E.3.B";

	oid = oh_derive_string(&ep2, 0, 16, in_oid);
	if (strcmp(expected_oid, oid)) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received OID=%s; Expected OID=%s\n", oid, expected_oid);
		g_free(oid);
                return -1;
        }
	g_free(oid);
	
        return 0;
}
