/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Kevin Gao <kevin.gao@intel.com>
 */

#include <stdio.h>
#include <string.h>
#include <hpitest.h>

static SaHpiWatchdogT wd = {
	.Log			= SAHPI_FALSE,
	.Running		= SAHPI_TRUE,
	.TimerUse		= SAHPI_WTU_SMS_OS,
	.TimerAction		= SAHPI_WA_RESET,
	.PretimerInterrupt	= SAHPI_WPI_NONE,
	.PreTimeoutInterval	= 0,
	.TimerUseExpFlags	= SAHPI_WATCHDOG_EXP_SMS_OS,
	.InitialCount		= 900000, //900 seconds
	.PresentCount		= 0
};

int do_watchdogtimer(SaHpiSessionIdT session_id, SaHpiResourceIdT resource_id, SaHpiRdrT rdr)
{
	SaHpiWatchdogT wd_old;
	SaHpiWatchdogNumT wd_num;
	SaErrorT val;
	int ret = HPI_TEST_UNKNOW;

	if (rdr.RdrType == SAHPI_WATCHDOG_RDR) {
		ret = HPI_TEST_PASS;
		wd_num = rdr.RdrTypeUnion.WatchdogRec.WatchdogNum;

		val = saHpiWatchdogTimerGet(session_id, resource_id, wd_num, 
				&wd_old);
		if (val != SA_OK) {
			printf("  saHpiWatchdogTimerGet failed!\n");
			printf("  Cannot get the old watchdog timer!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;				
		}
		
		val = saHpiWatchdogTimerSet(session_id, resource_id, wd_num, 
				&wd);
		if (val != SA_OK) {
			printf("  saHpiWatchdogTimerSet failed!\n");
			printf("  Cannot set the config info for the watchdog timer!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out1;				
		}

		val = saHpiWatchdogTimerReset(session_id, resource_id, wd_num);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Cannot reset the new watchdog timer!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
		}

out1:
		val = saHpiWatchdogTimerSet(session_id ,resource_id ,wd_num, 
				&wd_old);
		if (val != SA_OK) {
			printf("  saHpiWatchdogTimerSet failed!\n");
			printf("  Cannot set the old config info for the watchdog timer!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;				
		}
	}

out:
	return ret;
}

int main()
{
	SaHpiVersionT 	version;
	SaErrorT        val;
	int             ret = HPI_TEST_PASS;
	
	val = saHpiInitialize(&version);
	if (val != SA_OK) {
		printf("  Function \"saHpiInitialize\" works abnormally!\n");
		printf("  Cannot initialize HPI!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out;
	}
		
	ret = process_domain(SAHPI_DEFAULT_DOMAIN_ID, &do_resource, 
			&do_watchdogtimer, NULL);
	if (ret == HPI_TEST_UNKNOW) {
		printf("  No Watchdog timer in SAHPI_DEFAULT_DOMAIN_ID!\n");
		ret = HPI_TEST_FAIL;
	}
	
	val = saHpiFinalize();
	if (val != SA_OK) {
		printf("  Function \"saHpiFinalize\" works abnormally!\n");
		printf("  Cannot cleanup HPI");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
	}

out:	
	return ret;	
}
