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

#define THRESHOLDS_TEST_DATA 	2

int enables_cmp(SaHpiSensorEvtEnablesT enables, 
		SaHpiSensorEvtEnablesT enables_new)
{
	if (enables.SensorStatus == enables_new.SensorStatus && 
			enables.AssertEvents == enables_new.AssertEvents &&
			enables.DeassertEvents == enables_new.DeassertEvents)
		return 0;
	return -1;	
}

int do_sensor(SaHpiSessionIdT session_id, SaHpiResourceIdT resource_id, SaHpiRdrT rdr)
{
	SaHpiSensorEvtEnablesT	enables, enables_old, enables_new;
	SaErrorT        	val;
	SaHpiSensorNumT         num;
	int             	ret = HPI_TEST_UNKNOW;

	if (rdr.RdrType == SAHPI_SENSOR_RDR) {
		ret = HPI_TEST_PASS;
		num = rdr.RdrTypeUnion.SensorRec.Num;

		val = saHpiSensorEventEnablesGet(session_id, resource_id, num,
				&enables_old);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Cannot get the old event status message!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}

		memset(&enables, 0, sizeof(enables));
		enables.SensorStatus = SAHPI_SENSTAT_EVENTS_ENABLED;
		enables.AssertEvents = 1;
		enables.DeassertEvents = 1;
		
		val = saHpiSensorEventEnablesSet(session_id, resource_id, num, 
				&enables);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Cannot set the specified event status messages!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}
		
		val = saHpiSensorEventEnablesGet(session_id, resource_id, num,
				&enables_new);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Cannot get the new event status messages!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out1;
		}

		if (memcmp(&enables, &enables_new, sizeof(enables))) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  The new event status message is invalid!\n");
			ret = HPI_TEST_FAIL;
		}
		
out1:
		val = saHpiSensorEventEnablesSet(session_id, resource_id, num,
				&enables_old);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Cannot set the old event status message!\n");
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
		
	ret = process_domain(SAHPI_DEFAULT_DOMAIN_ID, &do_resource, &do_sensor,
			NULL);
	if (ret == HPI_TEST_UNKNOW) {
		printf("  No Sensor in SAHPI_DEFAULT_DOMAIN_ID!\n");
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
