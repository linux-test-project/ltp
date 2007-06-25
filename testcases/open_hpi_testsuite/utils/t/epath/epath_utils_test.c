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
 * Author(s):
 *     Steve Sherman <stevees@us.ibm.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SaHpi.h>
#include <oh_utils.h>

int main (int argc, char **argv) {
	gchar *test_string, *expected_string;
	oh_big_textbuffer bigbuf;
	SaErrorT   err, expected_err;
	SaHpiEntityPathT  ep;

	/************************************************ 
         * oh_encode_entitypath - Null parameter testcase
         ************************************************/
	expected_err = SA_ERR_HPI_INVALID_PARAMS;

	err = oh_encode_entitypath(0, &ep);
	if (err != expected_err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	/******************************************** 
         * oh_encode_entitypath - All blanks testcase
         ********************************************/
	test_string = "       ";
	expected_err = SA_ERR_HPI_INVALID_DATA;

	err = oh_encode_entitypath(test_string, &ep);
	if (err != expected_err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	/******************************************** 
         * oh_encode_entitypath - Begin junk testcase
         ********************************************/
	test_string = "junk{SYSTEM_CHASSIS,11}{SUBBOARD_CARRIER_BLADE,9}";
	expected_err = SA_ERR_HPI_INVALID_DATA;

	err = oh_encode_entitypath(test_string, &ep);
	if (err != expected_err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	/********************************************* 
         * oh_encode_entitypath - Middle junk testcase
         *********************************************/
	test_string = "{SYSTEM_CHASSIS,11}junk{SUBBOARD_CARRIER_BLADE,9}";
	expected_err = SA_ERR_HPI_INVALID_DATA;

	err = oh_encode_entitypath(test_string, &ep);
	if (err != expected_err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	/****************************************** 
         * oh_encode_entitypath - End junk testcase
         ******************************************/
	test_string = "{SYSTEM_CHASSIS,11}{SUBBOARD_CARRIER_BLADE,9}junk";
	expected_err = SA_ERR_HPI_INVALID_DATA;

	err = oh_encode_entitypath(test_string, &ep);
	if (err != expected_err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	/****************************************** 
         * oh_encode_entitypath - No comma testcase
         ******************************************/
	test_string = "{SYSTEM_CHASSIS.11}{SUBBOARD_CARRIER_BLADE,9}";
	expected_err = SA_ERR_HPI_INVALID_DATA;

	err = oh_encode_entitypath(test_string, &ep);
	if (err != expected_err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	/********************************************** 
         * oh_encode_entitypath - Bad HPI type testcase
         **********************************************/
	test_string = "{SYSTEM_CHASSIS,11}{WRONG_HPI_TYPE,9}";
	expected_err = SA_ERR_HPI_INVALID_DATA;

	err = oh_encode_entitypath(test_string, &ep);
	if (err != expected_err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	/************************************************** 
         * oh_encode_entitypath - Bad HPI instance testcase
         **************************************************/
	test_string = "{SYSTEM_CHASSIS,1abc1}{SYSTEM_SUB_CHASSIS,9}";
	expected_err = SA_ERR_HPI_INVALID_DATA;

	err = oh_encode_entitypath(test_string, &ep);
	if (err != expected_err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	/************************************************** 
         * oh_encode_entitypath - Extra parameters testcase
         **************************************************/
	test_string = "{SYSTEM_CHASSIS,2}{SYSTEM_SUB_CHASSIS,9,2}";
	expected_err = SA_ERR_HPI_INVALID_DATA;

	err = oh_encode_entitypath(test_string, &ep);
	if (err != expected_err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	/***************************************** 
         * oh_encode_entitypath - Nominal testcase
         *****************************************/
	test_string = "{SYSTEM_CHASSIS,1}{SUB_CHASSIS,2}";

	err = oh_encode_entitypath(test_string, &ep);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	oh_init_bigtext(&bigbuf);
	err = oh_decode_entitypath(&ep, &bigbuf);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	if (strcmp((char *)bigbuf.Data, test_string)) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received Entity Path=%s.\n", bigbuf.Data);
		return -1;
	}

	/**************************************** 
         * oh_encode_entitypath - Blanks testcase
         ****************************************/
	test_string = "  {SYSTEM_CHASSIS,  1111}  { CHASSIS_BACK_PANEL_BOARD  ,32 }  ";
	expected_string = "{SYSTEM_CHASSIS,1111}{CHASSIS_BACK_PANEL_BOARD,32}";

	err = oh_encode_entitypath(test_string, &ep);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	oh_init_bigtext(&bigbuf);
	err = oh_decode_entitypath(&ep, &bigbuf);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%s\n", oh_lookup_error(err));
		return -1;
	}

	if (strcmp((char *)bigbuf.Data, expected_string)) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received Entity Path=%s.\n", bigbuf.Data);
		return -1;
	}

	/********************************
         * oh_decode_entitypath testcases
         ********************************/
	{
                SaHpiEntityPathT  test_ep;

                oh_init_ep(&test_ep);
                test_ep.Entry[0].EntityType = SAHPI_ENT_SUB_CHASSIS;
                test_ep.Entry[0].EntityLocation = 109;
                test_ep.Entry[1].EntityType = SAHPI_ENT_SYSTEM_CHASSIS;
                test_ep.Entry[1].EntityLocation = 112;

		/*************************************** 
		 * oh_decode_entitypath  - Null testcase
		 ***************************************/
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_decode_entitypath(&test_ep, 0);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/*********************************************** 
		 * oh_decode_entitypath  - Bad instance testcase
		 ***********************************************/
		test_ep.Entry[0].EntityLocation = 1234567;
		
		expected_err = SA_ERR_HPI_INVALID_DATA;

		err = oh_decode_entitypath(&test_ep, &bigbuf);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}
		
		test_ep.Entry[0].EntityLocation = 109;
		
		/****************************************** 
		 * oh_decode_entitypath  - Nominal testcase
		 ******************************************/
		expected_string = "{SYSTEM_CHASSIS,112}{SUB_CHASSIS,109}";
	
		oh_init_bigtext(&bigbuf);
		oh_append_bigtext(&bigbuf, test_string);
	
		err = oh_decode_entitypath(&test_ep, &bigbuf);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}
		
		if (strcmp((char *)bigbuf.Data, expected_string)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received Entity Path=%s.\n", bigbuf.Data);
			return -1;
		}
	}

	return 0;
}
