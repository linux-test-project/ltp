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
#include <openhpi.h>
#include <epath_utils.h>

int main (int argc, char **argv) {
	SaHpiEntityPathT  ep;
	gchar *test_string, *expected_string;
	const int MAX_STRING_SIZE = 512;
	gchar returned_string[MAX_STRING_SIZE];
	int   err;
	
	/*********************************** 
         * string2entitypath - Null TestCase
         ***********************************/
	test_string = NULL;

	err = string2entitypath(test_string, &ep);
	if (err == 0) {
		printf("Error! string2entitypath - Null TestCase\n");
		printf("Error! string2entitypath returned err=%d\n", err);
		return -1;
	}

	/***************************************** 
         * string2entitypath - All blanks TestCase
         *****************************************/
	test_string = "       ";

	err = string2entitypath(test_string, &ep);
	if (err == 0) {
		printf("Error! string2entitypath - All blanks TestCase\n");
		printf("Error! string2entitypath returned err=%d\n", err);
		return -1;
	}

	/***************************************** 
         * string2entitypath - Begin junk TestCase
         *****************************************/
	test_string = "junk{SYSTEM_CHASSIS,11}{SUBBOARD_CARRIER_BLADE,9}";

	err = string2entitypath(test_string, &ep);
	if (err == 0) {
		printf("Error! string2entitypath - Begin junk TestCase\n");
		printf("Error! string2entitypath returned err=%d\n", err);
		return -1;
	}

	/****************************************** 
         * string2entitypath - Middle junk TestCase
         ******************************************/
	test_string = "{SYSTEM_CHASSIS,11}junk{SUBBOARD_CARRIER_BLADE,9}";

	err = string2entitypath(test_string, &ep);
	if (err == 0) {
		printf("Error! string2entitypath - Middle junk TestCase\n");
		printf("Error! string2entitypath returned err=%d\n", err);
		return -1;
	}

	/*************************************** 
         * string2entitypath - End junk TestCase
         ***************************************/
	test_string = "{SYSTEM_CHASSIS,11}{SUBBOARD_CARRIER_BLADE,9}junk";

	err = string2entitypath(test_string, &ep);
	if (err == 0) {
		printf("Error! string2entitypath - End junk TestCase\n");
		printf("Error! string2entitypath returned err=%d\n", err);
		return -1;
	}

	/*************************************** 
         * string2entitypath - No comma TestCase
         ***************************************/
	test_string = "{SYSTEM_CHASSIS.11}{SUBBOARD_CARRIER_BLADE,9}";

	err = string2entitypath(test_string, &ep);
	if (err == 0) {
		printf("Error! string2entitypath - No comma TestCase\n");
		printf("Error! string2entitypath returned err=%d\n", err);
		return -1;
	}

	/******************************************* 
         * string2entitypath - Bad HPI type TestCase
         *******************************************/
	test_string = "{SYSTEM_CHASSIS,11}{WRONG_HPI_TYPE,9}";

	err = string2entitypath(test_string, &ep);
	if (err == 0) {
		printf("Error! string2entitypath - Bad HPI type TestCase\n");
		printf("Error! string2entitypath returned err=%d\n", err);
		return -1;
	}

	/*********************************************** 
         * string2entitypath - Bad HPI instance TestCase
         ***********************************************/
	test_string = "{SYSTEM_CHASSIS,1abc1}{SYSTEM_SUB_CHASSIS,9}";

	err = string2entitypath(test_string, &ep);
	if (err == 0) {
		printf("Error! string2entitypath - Bad HPI instance TestCase\n");
		printf("Error! string2entitypath returned err=%d\n", err);
		return -1;
	}

	/*********************************************** 
         * string2entitypath - Extra parameters TestCase
         ***********************************************/
	test_string = "{SYSTEM_CHASSIS,2}{SYSTEM_SUB_CHASSIS,9,2}";

	err = string2entitypath(test_string, &ep);
	if (err == 0) {
		printf("Error! string2entitypath - Extra parameters TestCase\n");
		printf("Error! string2entitypath returned err=%d\n", err);
		return -1;
	}

	/*************************************** 
         * string2entitypath - Too long TestCase
         ***************************************/
	test_string = "{SYSTEM_CHASSIS,11}{CHASSIS_BACK_PANEL_BOARD,32}{OTHER_CHASSIS_BOARD,15}{SBC_BLADE,2}{COMPACTPCI_CHASSIS,15}{ADVANCEDTCA_CHASSIS, 30}{SYSTEM_SLOT, 14}{IO_BLADE,3}{DISK_BLADE, 0}{DISK_DRIVE, 22}{FAN,44}{POWER_DISTRIBUTION_UNIT,45}{SPEC_PROC_BLADE,1}{IO_SUBBOARD,13}{SBC_SUBBOARD, 10}{ALARM_MANAGER, 3}{ALARM_MANAGER_BLADE, 15}";

	err = string2entitypath(test_string, &ep);
	if (err == 0) {
		printf("Error! string2entitypath - Too long TestCase\n");
		printf("Error! string2entitypath returned err=%d\n", err);
		return -1;
	}

	/************************************** 
         * string2entitypath - Nominal TestCase
         **************************************/
	test_string = "{SYSTEM_CHASSIS,1}{SUB_CHASSIS,2}";

	err = string2entitypath(test_string, &ep);
	if (err) {
		printf("Error! string2entitypath - Nominal TestCase\n");
		printf("Error! string2entitypath returned err=%d\n", err);
		return -1;
	}

	err = entitypath2string(&ep, returned_string, MAX_STRING_SIZE);
	if (err < 0) {
		printf("Error! entitypath2string - Nominal TestCase\n");
		printf("Error! entitypath2string returned err=%d\n", err);
		return -1;
	}

	if (strcmp(returned_string, test_string)) {
		printf("Error! Nominal TestCase\n");
		printf("Error! Unexpected value=%s\n", returned_string);
		return -1;
	}

	/*************************************** 
         * string2entitypath - Max size TestCase
         ***************************************/
	test_string = "{SYSTEM_CHASSIS,11}{CHASSIS_BACK_PANEL_BOARD,32}{OTHER_CHASSIS_BOARD,15}{SBC_BLADE,2}{COMPACTPCI_CHASSIS,15}{ADVANCEDTCA_CHASSIS,30}{SYSTEM_SLOT,14}{IO_BLADE,3}{DISK_BLADE,0}{DISK_DRIVE,22}{FAN,44}{POWER_DISTRIBUTION_UNIT,45}{SPEC_PROC_BLADE,1}{IO_SUBBOARD,13}{SBC_SUBBOARD,10}{ALARM_MANAGER,3}";

	err = string2entitypath(test_string, &ep);
	if (err) {
		printf("Error! string2entitypath - Max size TestCase\n");
		printf("Error! string2entitypath returned err=%d\n", err);
		return -1;
	}

	err = entitypath2string(&ep, returned_string, MAX_STRING_SIZE);
	if (err < 0) {
		printf("Error! entitypath2string - Max size TestCase\n");
		printf("Error! entitypath2string returned err=%d\n", err);
		return -1;
	}

	if (strcmp(returned_string, test_string)) {
		printf("Error! Max size TestCase\n");
		printf("Error! Unexpected value=%s\n", returned_string);
		return -1;
	}

	/************************************* 
         * string2entitypath - Blanks TestCase
         *************************************/
	test_string = "  {SYSTEM_CHASSIS,  1111}  { CHASSIS_BACK_PANEL_BOARD  ,32 }  ";
	expected_string = "{SYSTEM_CHASSIS,1111}{CHASSIS_BACK_PANEL_BOARD,32}";

	err = string2entitypath(test_string, &ep);
	if (err) {
		printf("Error! string2entitypath - Blanks TestCase\n");
		printf("Error! string2entitypath returned err=%d\n", err);
		return -1;
	}

	err = entitypath2string(&ep, returned_string, MAX_STRING_SIZE);
	if (err < 0) {
		printf("Error! entitypath2string - Blanks TestCase\n");
		printf("Error! entitypath2string returned err=%d\n", err);
		return -1;
	}

	if (strcmp(returned_string, expected_string)) {
		printf("Error! Blanks TestCase\n");
		printf("Error! Unexpected value=%s\n", returned_string);
		return -1;
	}

	/*****************************
         * entitypath2string TestCases
         *****************************/
	{

		SaHpiEntityPathT  test_ep = {
			.Entry[0] = {
				.EntityType = SAHPI_ENT_SUB_CHASSIS,
				.EntityInstance = 109
			},
			{
				.EntityType = SAHPI_ENT_SYSTEM_CHASSIS,
				.EntityInstance = 112
			}
		};

		gchar *returned_string;

		if (NULL == (returned_string = (g_malloc0(MAX_STRING_SIZE)))) { 
			printf("ERROR! Test Case program cannot allocate memory\n");
			return -1;
		}

		/************************************ 
		 * entitypath2string  - Null TestCase
		 ************************************/
		err = entitypath2string(&test_ep, NULL, MAX_STRING_SIZE);
		if (err >= 0) {
			printf("Error! entitypath2string - Null TestCase\n");
			printf("Error! entitypath2string returned err=%d\n", err);
			return -1;
		}

		/*********************************************** 
		 * entitypath2string  - Bad string size TestCase
		 ***********************************************/
		err = entitypath2string(&test_ep, returned_string, 10);
		if (err >= 0) {
			printf("Error! entitypath2string - Bad string size TestCase\n");
			printf("Error! entitypath2string returned err=%d\n", err);
			return -1;
		}

		/******************************************** 
		 * entitypath2string  - Bad instance TestCase
		 ********************************************/
		test_ep.Entry[0].EntityInstance = 1234567;
		
		err = entitypath2string(&test_ep, returned_string, MAX_STRING_SIZE);
		if (err >= 0) {
			printf("Error! entitypath2string - Bad instance TestCase\n");
			printf("Error! entitypath2string returned err=%d\n", err);
			return -1;
		}
		
		test_ep.Entry[0].EntityInstance = 109;
		
		/*************************************** 
		 * entitypath2string  - NULL EP TestCase
		 ***************************************/
		expected_string = "";
		strcpy(returned_string, "123");
		
		err = entitypath2string(NULL, returned_string, MAX_STRING_SIZE);
		if (err != 0) {
			printf("Error! entitypath2string - NULL EP TestCase\n");
			printf("Error! entitypath2string returned err=%d\n", err);
			return -1;
		}
		
		if (strcmp(returned_string, expected_string)) {
			printf("Error! NULL EP TestCase\n");
			printf("Error! Unexpected value=%s\n", returned_string);
			return -1;
		}

		/*************************************** 
		 * entitypath2string  - Nominal TestCase
		 ***************************************/
		expected_string = "{SYSTEM_CHASSIS,112}{SUB_CHASSIS,109}";
		
		err = entitypath2string(&test_ep, returned_string, MAX_STRING_SIZE);
		if (err < 0) {
			printf("Error! entitypath2string - Nominal TestCase\n");
			printf("Error! entitypath2string returned err=%d\n", err);
			return -1;
		}
		
		if (strcmp(returned_string, expected_string)) {
			printf("Error! Nominal TestCase\n");
			printf("Error! Unexpected value=%s\n", returned_string);
			return -1;
		}
	}

	return 0;
}
