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

void set_data(SaHpiSensorReadingT *reading, SaHpiSensorReadingT *reading_old)
{
	SaHpiSensorInterpretedTypeT     type;
	
	type = reading_old->Interpreted.Type;
	reading->ValuesPresent = SAHPI_SRF_INTERPRETED;
	reading->Interpreted.Type = type;

	switch(type) {
	case SAHPI_SENSOR_INTERPRETED_TYPE_UINT8:
		reading->Interpreted.Value.SensorUint8 = THRESHOLDS_TEST_DATA;
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_UINT16:
		reading->Interpreted.Value.SensorUint16 = THRESHOLDS_TEST_DATA;
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_UINT32:
		reading->Interpreted.Value.SensorUint32 = THRESHOLDS_TEST_DATA;
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_INT8:
		reading->Interpreted.Value.SensorInt8 = THRESHOLDS_TEST_DATA;
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_INT16:
		reading->Interpreted.Value.SensorInt16 = THRESHOLDS_TEST_DATA;
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_INT32:
		reading->Interpreted.Value.SensorInt32 = THRESHOLDS_TEST_DATA;
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32:
		reading->Interpreted.Value.SensorFloat32 = THRESHOLDS_TEST_DATA;
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_BUFFER:
		memset(reading->Interpreted.Value.SensorBuffer, 0, 
			sizeof(reading->Interpreted
				       .Value.SensorBuffer));
		reading->Interpreted.Value.SensorBuffer[0] =
			THRESHOLDS_TEST_DATA;
		break;
	}
}

void value_init(SaHpiSensorThresholdsT *thresholds, SaHpiSensorThresholdsT 
		*thresholds_old, SaHpiSensorThdDefnT defn)
{
	SaHpiSensorThdMaskT 		read_thold = defn.ReadThold;
	SaHpiSensorThdMaskT 		write_thold = defn.WriteThold;

	if (read_thold & SAHPI_STM_LOW_CRIT && 
			write_thold & SAHPI_STM_LOW_CRIT) 
		set_data(&thresholds->LowCritical,
				&thresholds_old->LowCritical);

	if (read_thold & SAHPI_STM_LOW_MAJOR &&
			write_thold & SAHPI_STM_LOW_MAJOR) 
		set_data(&thresholds->LowMajor, &thresholds_old->LowMajor);

	if (read_thold & SAHPI_STM_LOW_MINOR && 
			write_thold & SAHPI_STM_LOW_MINOR) 
		set_data(&thresholds->LowMinor, &thresholds_old->LowMinor);

	if (read_thold & SAHPI_STM_UP_CRIT &&
			write_thold & SAHPI_STM_UP_CRIT) 
		set_data(&thresholds->UpCritical, &thresholds_old->UpCritical);
	
	if (read_thold & SAHPI_STM_UP_MAJOR &&
			write_thold & SAHPI_STM_UP_MAJOR) 
		set_data(&thresholds->UpMajor, &thresholds_old->UpMajor);
	
	if (read_thold & SAHPI_STM_UP_MINOR &&
			write_thold & SAHPI_STM_UP_MINOR) 
		set_data(&thresholds->UpMinor, &thresholds_old->UpMinor);
	
	if (read_thold & SAHPI_STM_UP_HYSTERESIS &&
			write_thold & SAHPI_STM_UP_HYSTERESIS) 
		set_data(&thresholds->PosThdHysteresis, 
				&thresholds_old->PosThdHysteresis);

	if (read_thold & SAHPI_STM_LOW_HYSTERESIS &&
			write_thold & SAHPI_STM_LOW_HYSTERESIS) 
		set_data(&thresholds->NegThdHysteresis,
				&thresholds_old->NegThdHysteresis);
}

int thrd_cmp(SaHpiSensorThresholdsT thresholds, SaHpiSensorThresholdsT thresholds_new, SaHpiSensorThdDefnT defn)
{
	SaHpiSensorThdMaskT read_thold = defn.ReadThold;
	SaHpiSensorThdMaskT write_thold = defn.WriteThold;

	if (read_thold & SAHPI_STM_LOW_CRIT && 
			write_thold & SAHPI_STM_LOW_CRIT) {
		if (memcmp(&thresholds.LowCritical, &thresholds_new.LowCritical,
					sizeof(thresholds.LowCritical)))
			return -1;
		else 
			return 0;
	}

	if (read_thold & SAHPI_STM_LOW_MAJOR &&
			write_thold & SAHPI_STM_LOW_MAJOR) {
		if (memcmp(&thresholds.LowMajor, &thresholds_new.LowMajor,
					sizeof(thresholds.LowMajor)))
			return -1;
		else 
			return 0;
	}

	if (read_thold & SAHPI_STM_LOW_MINOR && 
			write_thold & SAHPI_STM_LOW_MINOR) {
		if (memcmp(&thresholds.LowMinor, &thresholds_new.LowMinor,
					sizeof(thresholds.LowMinor)))
			return -1;
		else 
			return 0;
	}

	if (read_thold & SAHPI_STM_UP_CRIT &&
			write_thold & SAHPI_STM_UP_CRIT) {
		if (memcmp(&thresholds.UpCritical, &thresholds_new.UpCritical,
					sizeof(thresholds.UpCritical)))
			return -1;
		else 
			return 0;
	}
	
	if (read_thold & SAHPI_STM_UP_MAJOR &&
			write_thold & SAHPI_STM_UP_MAJOR) {
		if (memcmp(&thresholds.UpMajor, &thresholds_new.UpMajor,
					sizeof(thresholds.UpMajor)))
			return -1;
		else 
			return 0;
	}
	
	if (read_thold & SAHPI_STM_UP_MINOR &&
			write_thold & SAHPI_STM_UP_MINOR) {
		if (memcmp(&thresholds.UpMinor, &thresholds_new.UpMinor,
					sizeof(thresholds.UpMinor)))
			return -1;
		else 
			return 0;
	}
	
	if (read_thold & SAHPI_STM_UP_HYSTERESIS &&
			write_thold & SAHPI_STM_UP_HYSTERESIS) {
		if (memcmp(&thresholds.PosThdHysteresis, 
					&thresholds_new.PosThdHysteresis,
					sizeof(thresholds.PosThdHysteresis)))
			return -1;
		else 
			return 0;
	}

	if (read_thold & SAHPI_STM_LOW_HYSTERESIS &&
			write_thold & SAHPI_STM_LOW_HYSTERESIS) {
		if (memcmp(&thresholds.NegThdHysteresis, 
					&thresholds_new.NegThdHysteresis,
					sizeof(thresholds.NegThdHysteresis)))
			return -1;
		else 
			return 0;
	}
	return 1;
}

int do_sensor(SaHpiSessionIdT session_id, SaHpiResourceIdT resource_id, SaHpiRdrT rdr)
{
	SaHpiSensorThresholdsT	thresholds, thresholds_old, thresholds_new;
	SaHpiSensorThdDefnT	defn;
	SaHpiSensorNumT		num;
	SaErrorT        	val;
	int             	ret = HPI_TEST_UNKNOW;

	if (rdr.RdrType == SAHPI_SENSOR_RDR) {
		num = rdr.RdrTypeUnion.SensorRec.Num;
		defn = rdr.RdrTypeUnion.SensorRec.ThresholdDefn;

		if (defn.IsThreshold == SAHPI_FALSE)
			goto out;
		if (!defn.ReadThold || !defn.WriteThold)
			goto out;
		
		ret = HPI_TEST_PASS;
		val = saHpiSensorThresholdsGet(session_id, resource_id,
				num, &thresholds_old);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Cannot get the old thresholds!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}

		memset(&thresholds, 0, sizeof(thresholds));
		value_init(&thresholds, &thresholds_old, defn);
		val = saHpiSensorThresholdsSet(session_id, resource_id,
				num, &thresholds);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Cannot set the specified thresholds!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}
			
		val = saHpiSensorThresholdsGet(session_id, resource_id,
				num, &thresholds_new);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Cannot get the new thresholds!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out1;
		}

		if (thrd_cmp(thresholds, thresholds_new, defn)) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  The new thresholds is invalid!\n");
			ret = HPI_TEST_FAIL;
		}
		
out1:
		val = saHpiSensorThresholdsSet(session_id, resource_id,
				num, &thresholds_old);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Cannot set the old thresholds!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
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
