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
#include <SaHpi.h>

#include <oh_plugin.h>
#include <rpt_utils.h>
#include <snmp_util.h>
#include <snmp_bc.h>
#include <bc_resources.h>
#include <snmp_bc_utils.h>
#include <snmp_bc_sensor.h>

#include <tstubs_rdr.h>
#include <tstubs_snmp.h>
#include <tsensor.h>

static int sensors_equal(SaHpiSensorReadingT sensor1, SaHpiSensorReadingT sensor2);

int main(int argc, char **argv) 
{

	struct snmp_bc_hnd snmp_handle;
	struct oh_handler_state hnd = {
		.rptcache = (RPTable *)&test_rdr,
		.eventq = NULL,
		.config = NULL,
		.data = (void *)&snmp_handle,
	};
	
	/* Fill in test RDR */
	test_rdr.Rdr.RdrTypeUnion.SensorRec = test_sensor.sensor;
	test_rdr.Rdr.IdString.DataLength = strlen(test_sensor.comment);
	strcpy(test_rdr.Rdr.IdString.Data, test_sensor.comment);
	test_rdr.Test = test_sensor;

	SaHpiResourceIdT    id = 1;
	SaHpiSensorNumT     num = 1;
	SaHpiSensorReadingT sensor_data;
	SaErrorT err;


	/******************************
	 * Read Sensor Float32 TestCase
	 ******************************/
	SaHpiSensorReadingT expected_value = {
		.ValuesPresent = SAHPI_SRF_INTERPRETED,
		.Raw = 0,
		.Interpreted = {
			.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.Value = {
				.SensorFloat32 = 3.2,
			},
		},
		.EventStatus = {
/*		.SensorStatus = SAHPI_SENSTAT_SCAN_ENABLED, */
/*		.EventStatus = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR, */
		},
	};

	err = snmp_bc_get_sensor_data((void *)&hnd, id, num, &sensor_data); 
	if (err) { 
		printf("Error! Read Sensor Float32 TestCase\n");
		printf("snmp_bc_get_sensor_data returned err=%d\n", err);
		return -1;
	}

	if (sensors_equal(sensor_data, expected_value)) { return 0; }
	else {
		printf("Error! Read Sensor Float32 TestCase\n");
		return -1;
	}
}





static int sensors_equal(SaHpiSensorReadingT sensor_reading, SaHpiSensorReadingT expected_reading)
{
	if (sensor_reading.ValuesPresent != expected_reading.ValuesPresent) {
		printf ("ValuesPresent not equal! Sensor_Reading=%x; Expected_Reading=%x\n",
			sensor_reading.ValuesPresent, expected_reading.ValuesPresent);
		return 0;
	}
	if (sensor_reading.Raw != expected_reading.Raw) {
		printf ("Raw Values not equal! Sensor_Reading=%x; Expected_Reading=%x\n",
			sensor_reading.Raw, expected_reading.Raw);
		return 0;
	}
	if (sensor_reading.Interpreted.Type != expected_reading.Interpreted.Type) {
		printf ("Interpreted Types not equal! Sensor_Reading=%x; Expected_Reading=%x\n",
			sensor_reading.Interpreted.Type, expected_reading.Interpreted.Type);
		return 0;
	}

        switch (sensor_reading.Interpreted.Type) {
	case SAHPI_SENSOR_INTERPRETED_TYPE_UINT8:
		if (sensor_reading.Interpreted.Value.SensorUint8 != expected_reading.Interpreted.Value.SensorUint8) {
			printf ("Interpreted Values not equal! Sensor_Reading=%x; Expected_Reading=%x\n",
				sensor_reading.Interpreted.Value.SensorUint8, expected_reading.Interpreted.Value.SensorUint8);
			return 0;
		}
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_UINT16:	
		if (sensor_reading.Interpreted.Value.SensorUint16 != expected_reading.Interpreted.Value.SensorUint16) {
			printf ("Interpreted Values not equal! Sensor_Reading=%x; Expected_Reading=%x\n",
				sensor_reading.Interpreted.Value.SensorUint16, expected_reading.Interpreted.Value.SensorUint16);
			return 0;
		}
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_UINT32:
		if (sensor_reading.Interpreted.Value.SensorUint32 != expected_reading.Interpreted.Value.SensorUint32) {
			printf ("Interpreted Values not equal! Sensor_Reading=%x; Expected_Reading=%x\n",
				sensor_reading.Interpreted.Value.SensorUint32, expected_reading.Interpreted.Value.SensorUint32);
			return 0;
		}
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_INT8:
		if (sensor_reading.Interpreted.Value.SensorInt8 != expected_reading.Interpreted.Value.SensorInt8) {
			printf ("Interpreted Values not equal! Sensor_Reading=%x; Expected_Reading=%x\n",
				sensor_reading.Interpreted.Value.SensorInt8, expected_reading.Interpreted.Value.SensorInt8);
			return 0;
		}
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_INT16:
		if (sensor_reading.Interpreted.Value.SensorInt16 != expected_reading.Interpreted.Value.SensorInt16) {
			printf ("Interpreted Values not equal! Sensor_Reading=%x; Expected_Reading=%x\n",
				sensor_reading.Interpreted.Value.SensorInt16, expected_reading.Interpreted.Value.SensorInt16);
			return 0;
		}
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_INT32:
		if (sensor_reading.Interpreted.Value.SensorInt32 != expected_reading.Interpreted.Value.SensorInt32) {
			printf ("Interpreted Values not equal! Sensor_Reading=%x; Expected_Reading=%x\n",
				sensor_reading.Interpreted.Value.SensorInt32, expected_reading.Interpreted.Value.SensorInt32);
			return 0;
		}
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32:
		if (sensor_reading.Interpreted.Value.SensorFloat32 != expected_reading.Interpreted.Value.SensorFloat32) {
			printf ("Interpreted Values not equal! Sensor_Reading=%f; Expected_Reading=%f\n",
				sensor_reading.Interpreted.Value.SensorFloat32, expected_reading.Interpreted.Value.SensorFloat32);
			return 0;
		}
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_BUFFER:	
		if (!strncmp(sensor_reading.Interpreted.Value.SensorBuffer, expected_reading.Interpreted.Value.SensorBuffer, SAHPI_SENSOR_BUFFER_LENGTH)) {
		    printf ("Interpreted Values not equal! Sensor_Reading=%s; Expected_Reading=%s\n",
			sensor_reading.Interpreted.Value.SensorBuffer, expected_reading.Interpreted.Value.SensorBuffer);
		    return 0;
		}
		break;
        default:
		printf("Invalid sensor interpreted type=%d\n", sensor_reading.Interpreted.Type);
                return 0;
        }
	if (sensor_reading.EventStatus.SensorStatus != expected_reading.EventStatus.SensorStatus) {
		printf ("Sensor Status not equal! Sensor_Reading=%x; Expected_Reading=%x\n",
			sensor_reading.EventStatus.SensorStatus, expected_reading.EventStatus.SensorStatus);
		return 0;
	}
	if (sensor_reading.EventStatus.EventStatus != expected_reading.EventStatus.EventStatus) {
		printf ("Sensor Event Status not equal! Sensor_Reading=%x; Expected_Reading=%x\n",
			sensor_reading.EventStatus.EventStatus, expected_reading.EventStatus.EventStatus);
		return 0;
	}

	return 1; /* Sensors are equal */
}

/****************
 * Stub Functions
 ****************/

#include <tstubs_rdr.c>
#include <tstubs_snmp.c>
