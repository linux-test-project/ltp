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
#include <string.h>
#include <stdio.h>
#include <SaHpi.h>
#include <snmp_bc_utils.h>

int main(int argc, char **argv) 
{
  	int err;
	SaHpiSensorInterpretedUnionT value, expected_value;
	SaHpiSensorInterpretedTypeT interpreted_type;
	gchar *snmp_string;

	/************************************
	 * Float32 - Positive Sign TestCase
	 ************************************/
	interpreted_type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32;
	snmp_string = " + 33.33 Volts";
	expected_value.SensorFloat32 = 33.33;

	err = get_interpreted_value(snmp_string, interpreted_type, &value);
	if (err) {
		printf("Error! Float32 - Positive Sign TestCase\n");
		printf("get_interpreted_value returned error=%d\n", err);
		return -1;
	}

	if (value.SensorFloat32 != expected_value.SensorFloat32) {
		printf("Error! Float32 - Positive Sign TestCase\n");
		printf ("Convert Value=%f; Expected Value=%f\n",
			value.SensorFloat32, expected_value.SensorFloat32);
		return -1;
	}

	/**********************************
	 * Float32 - Negative Sign TestCase
	 **********************************/
	interpreted_type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32;
	snmp_string = " - 3.3 Volts";
	expected_value.SensorFloat32 = -3.3;

	err = get_interpreted_value(snmp_string, interpreted_type, &value);
	if (err) {
		printf("Error! Float32 - Negative Sign TestCase\n");
		printf("get_interpreted_value returned error=%d\n", err);
		return -1;
	}

	if (value.SensorFloat32 != expected_value.SensorFloat32) {
		printf("Error! Float32 - Negative Sign TestCase\n");
		printf ("Convert Value=%f; Expected Value=%f\n",
			value.SensorFloat32, expected_value.SensorFloat32);
		return -1;
	}

	/**************************************
	 * Float32 - No Sign TestCase
	 **************************************/
	interpreted_type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32;
	snmp_string = "  3.33 Volts";
	expected_value.SensorFloat32 = 3.33;

	err = get_interpreted_value(snmp_string, interpreted_type, &value);
	if (err) {
		printf("Error! Float32 - No Sign TestCase\n");
		printf("get_interpreted_value returned error=%d\n", err);
		return -1;
	}

	if (value.SensorFloat32 != expected_value.SensorFloat32) {
		printf("Error! Float32 - No Sign TestCase\n");
		printf ("Convert Value=%f; Expected Value=%f\n",
			value.SensorFloat32, expected_value.SensorFloat32);
		return -1;
	}

	/*******************************
	 * Float32 - Percentage TestCase
	 *******************************/
	interpreted_type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32;
	snmp_string = "    67% of maximum";
	expected_value.SensorFloat32 = 0.67;

	err = get_interpreted_value(snmp_string, interpreted_type, &value);
	if (err) {
		printf("Error! Float32 - Percentage TestCase\n");
		printf("get_interpreted_value returned error=%d\n", err);
		return -1;
	}

	if (value.SensorFloat32 != expected_value.SensorFloat32) {
		printf("Error! Float32 - Percentage TestCase\n");
		printf ("Convert Value=%f; Expected Value=%f\n",
			value.SensorFloat32, expected_value.SensorFloat32);
		return -1;
	}

	return 0;
}
