/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <SaHpi.h>
#include <oh_utils.h>

#define UNDEFINED_MANUFACTURER  -1
#define BAD_TYPE -1

int main(int argc, char **argv) 
{
	const char *expected_str;
	const char *str;
	SaErrorT   expected_err, err;
	SaHpiTextBufferT buffer, bad_buffer;

	/************************************ 
	 * oh_decode_manufacturerid testcases
         ************************************/
	{
		/* oh_decode_manufacturerid: SAHPI_MANUFACTURER_ID_UNSPECIFIED testcase */
		SaHpiManufacturerIdT mid;	

		expected_str = "Unspecified";
		mid = SAHPI_MANUFACTURER_ID_UNSPECIFIED;

		err = oh_decode_manufacturerid(mid, &buffer); 
		
                if (strcmp(expected_str, (char *)buffer.Data) || err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received string=%s; Expected string=%s; Error=%d\n", 
			       buffer.Data, expected_str, err);
                        return -1;
                }
		
		/* oh_decode_manufacturerid: IBM testcase */
		expected_str = "IBM";
		mid = 20944;

		err = oh_decode_manufacturerid(mid, &buffer); 
		
                if (strcmp(expected_str, (char *)buffer.Data) || err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received string=%s; Expected string=%s; Error=%d\n", 
			       buffer.Data, expected_str, err);
                         return -1;
                }
		
		/* oh_decode_manufacturerid: Undefined manufacturer testcase */
		expected_str = "Unknown";
		mid = UNDEFINED_MANUFACTURER;

		err = oh_decode_manufacturerid(mid, &buffer);
		
                if (strcmp(expected_str, (char *)buffer.Data) || err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received string=%s; Expected string=%s; Error=%d\n", 
			       buffer.Data, expected_str, err);
                         return -1;
                }

		/* oh_decode_manufacturerid: NULL buffer testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;
		mid = UNDEFINED_MANUFACTURER;

		err = oh_decode_manufacturerid(mid, 0);
		
                if (err != expected_err) {
 			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
                         return -1;
                }
	}

	/*********************************************************** 
	 * oh_decode_sensorreading/oh_encode_sensorreading testcases
         ***********************************************************/
	{
		SaHpiSensorDataFormatT format_default, format_test;
		SaHpiSensorReadingT reading_default, reading_test, encode_reading;
		memset(&format_default, 0, sizeof(SaHpiSensorDataFormatT));
		memset(&reading_default, 0, sizeof(SaHpiSensorReadingT));

		reading_default.IsSupported = SAHPI_TRUE;
		reading_default.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		reading_default.Value.SensorInt64 = 20;
		
		format_default.IsSupported = SAHPI_TRUE;
		format_default.ReadingType = SAHPI_SENSOR_READING_TYPE_INT64;
		format_default.BaseUnits = SAHPI_SU_VOLTS;
		format_default.ModifierUnits = SAHPI_SU_UNSPECIFIED;
		format_default.ModifierUse = SAHPI_SMUU_NONE;
		format_default.Percentage = SAHPI_FALSE;

		/* oh_decode_sensorreading: NULL buffer testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;
		
		err = oh_decode_sensorreading(reading_default, format_default, 0);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
 			return -1;
		}
		
		/* oh_decode_sensorreading: IsSupported == FALSE testcase */
		expected_err = SA_ERR_HPI_INVALID_CMD;
		reading_test = reading_default;
		format_test = format_default;
		format_test.IsSupported = SAHPI_FALSE;
	
		err = oh_decode_sensorreading(reading_test, format_test, &buffer);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
 			return -1;
		}

		/* oh_decode_sensorreading: Bad SaHpiSensorModifierUseT testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;
		reading_test = reading_default;
		format_test = format_default;
		format_test.ModifierUnits = SAHPI_SU_WEEK;
		format_test.ModifierUse = BAD_TYPE;
		
		err = oh_decode_sensorreading(reading_test, format_test, &buffer);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}

		/* oh_decode_sensorreading: Bad SaHpiSensorReadingT testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;
		reading_test = reading_default;
		reading_test.Type = BAD_TYPE;
		format_test = format_default;
		format_test.ReadingType = BAD_TYPE;
		
		err = oh_decode_sensorreading(reading_test, format_test, &buffer);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}
		
		/* oh_decode_sensorreading: Reading Types not equal testcase */
		expected_err = SA_ERR_HPI_INVALID_DATA;
		reading_test = reading_default;
		format_test = format_default;
		format_test.ReadingType = format_default.ReadingType + 1;
		
		err = oh_decode_sensorreading(reading_test, format_test, &buffer);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}

		/* oh_decode_sensorreading: SAHPI_SENSOR_READING_TYPE_INT64 testcase */
		expected_str = "20 Volts";
		reading_test = reading_default;
		format_test = format_default;
		
		err = oh_decode_sensorreading(reading_test, format_test, &buffer);
		if (err != SA_OK) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		
                if (strcmp(expected_str, (char *)buffer.Data)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received string=%s; Expected string=%s\n", 
			       buffer.Data, expected_str);
                        return -1;             
                }

		memset(&encode_reading, 0, sizeof(SaHpiSensorReadingT));
		err = oh_encode_sensorreading(&buffer, format_test.ReadingType, &encode_reading);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		
		if (memcmp((void *)&encode_reading, (void *)&reading_test, 
			   sizeof(SaHpiSensorReadingT))) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		
		/* oh_decode_sensorreading: SAHPI_SMUU_BASIC_OVER_MODIFIER testcase */
		expected_str = "20 Volts / Week";
		reading_test = reading_default;
		format_test = format_default;
		format_test.ModifierUnits = SAHPI_SU_WEEK;
		format_test.ModifierUse = SAHPI_SMUU_BASIC_OVER_MODIFIER;
		
		err = oh_decode_sensorreading(reading_test, format_test, &buffer);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		
                if (strcmp(expected_str, (char *)buffer.Data)) {
 			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received string=%s; Expected string=%s\n", 
			       buffer.Data, expected_str);
                       return -1;             
                }

		err = oh_encode_sensorreading(&buffer, format_test.ReadingType, &encode_reading);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		
		if (memcmp((void *)&encode_reading, (void *)&reading_test, 
			   sizeof(SaHpiSensorReadingTypeT))) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		/* oh_decode_sensorreading: SAHPI_SMUU_BASIC_TIMES_MODIFIER testcase */
		expected_str = "20 Volts * Week";
		reading_test = reading_default;
		format_test = format_default;
		format_test.ModifierUnits = SAHPI_SU_WEEK;
		format_test.ModifierUse = SAHPI_SMUU_BASIC_TIMES_MODIFIER;
		
		err = oh_decode_sensorreading(reading_test, format_test, &buffer);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		
                if (strcmp(expected_str, (char *)buffer.Data)) {
  			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received string=%s; Expected string=%s\n", 
			       buffer.Data, expected_str);
                       return -1;             
                }

		err = oh_encode_sensorreading(&buffer, format_test.ReadingType, &encode_reading);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		
		if (memcmp((void *)&encode_reading, (void *)&reading_test, 
			   sizeof(SaHpiSensorReadingTypeT))) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		/* oh_decode_sensorreading: Percentage testcase */
		expected_str = "20%";
		reading_test = reading_default;
		
		format_test = format_default;
		format_test.Percentage = SAHPI_TRUE;
		format_test.ModifierUnits = SAHPI_SU_WEEK;
		format_test.ModifierUse = SAHPI_SMUU_BASIC_TIMES_MODIFIER;
		
		err = oh_decode_sensorreading(reading_test, format_test, &buffer);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		
                if (strcmp(expected_str, (char *)buffer.Data)) {
   			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received string=%s; Expected string=%s\n", 
			       buffer.Data, expected_str);
                       return -1;             
                }

		/* oh_decode_sensorreading: SAHPI_SENSOR_READING_TYPE_UINT64 testcase */
		expected_str = "20 Volts";
		reading_test = reading_default;
		reading_test.Type = SAHPI_SENSOR_READING_TYPE_UINT64;
		reading_test.Value.SensorUint64 = 20;
		format_test = format_default;
		format_test.ReadingType = SAHPI_SENSOR_READING_TYPE_UINT64;
		
		err = oh_decode_sensorreading(reading_test, format_test, &buffer);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		
                if (strcmp(expected_str, (char *)buffer.Data)) {
    			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received string=%s; Expected string=%s\n", 
			       buffer.Data, expected_str);
                       return -1;             
                }

		err = oh_encode_sensorreading(&buffer, format_test.ReadingType, &encode_reading);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		
		if (memcmp((void *)&encode_reading, (void *)&reading_test, 
			   sizeof(SaHpiSensorReadingTypeT))) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		/* oh_decode_sensorreading: SAHPI_SENSOR_READING_TYPE_FLOAT64 testcase */
		expected_str = "20.200 Volts";
		reading_test = reading_default;
		reading_test.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		reading_test.Value.SensorFloat64 = 20.2;
		format_test = format_default;
		format_test.ReadingType = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		
		err = oh_decode_sensorreading(reading_test, format_test, &buffer);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		
                if (strcmp(expected_str, (char *)buffer.Data)) {
    			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received string=%s; Expected string=%s\n", 
			       buffer.Data, expected_str);
			return -1;             
                }

		/* oh_decode_sensorreading: SAHPI_SENSOR_READING_TYPE_BUFFER testcase */
		expected_str = "22222222222222222222222222222222 Volts";
		reading_test = reading_default;
		reading_test.Type = SAHPI_SENSOR_READING_TYPE_BUFFER;
		memset(reading_test.Value.SensorBuffer, 0x32, SAHPI_SENSOR_BUFFER_LENGTH);
		format_test = format_default;
		format_test.ReadingType = SAHPI_SENSOR_READING_TYPE_BUFFER;
		
		err = oh_decode_sensorreading(reading_test, format_test, &buffer);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		
                if (strcmp(expected_str, (char *)buffer.Data)) {
    			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received string=%s; Expected string=%s\n", 
			       buffer.Data, expected_str);
                        return -1;             
                }

		err = oh_encode_sensorreading(&buffer, format_test.ReadingType, &encode_reading);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		
		if (memcmp((void *)&encode_reading, (void *)&reading_test, 
			   sizeof(SaHpiSensorReadingTypeT))) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
	}

	/*********************************** 
         * oh_encode_sensorreading testcases
	 ***********************************/
	{
		const char *str;
		SaHpiTextBufferT buffer;
		SaHpiSensorReadingT reading;
		SaHpiInt64T   expected_int64;
		/* SaHpiUint64T  expected_uint64; */
		SaHpiFloat64T expected_float64;

		/* oh_encode_sensorreading: Bad type testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;
		err = oh_encode_sensorreading(&buffer, BAD_TYPE, &reading);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}
		
		/* oh_encode_sensorreading: Skip characters before '=' sign testcase */
		str = "+5Volt Sense 333=4";
		expected_int64 = 4;
		oh_init_textbuffer(&buffer);
		oh_append_textbuffer(&buffer, str);
		err = oh_encode_sensorreading(&buffer, SAHPI_SENSOR_READING_TYPE_INT64, &reading);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		if (reading.Value.SensorInt64 != expected_int64) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received value=%lld; Expected value=%lld\n", 
			       reading.Value.SensorInt64, expected_int64);
			return -1;
		}

		/* oh_encode_sensorreading: Extra spaces testcase */
		str = " + 20  Volts";
		expected_int64 = 20;
		oh_init_textbuffer(&buffer);
		oh_append_textbuffer(&buffer, str);
		err = oh_encode_sensorreading(&buffer, SAHPI_SENSOR_READING_TYPE_INT64, &reading);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		if (reading.Value.SensorInt64 != expected_int64) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received value=%lld; Expected value=%lld\n", 
			       reading.Value.SensorInt64, expected_int64);
			return -1;
		}

		/* oh_encode_sensorreading: Extra non-digits/commas testcase */
		str = "The, happy, %% result is ... +2,000Volts ,,... ";
		expected_int64 = 2000;
		oh_init_textbuffer(&buffer);
		oh_append_textbuffer(&buffer, str);
		err = oh_encode_sensorreading(&buffer, SAHPI_SENSOR_READING_TYPE_INT64, &reading);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		if (reading.Value.SensorInt64 != expected_int64) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received value=%lld; Expected value=%lld\n", 
			       reading.Value.SensorInt64, expected_int64);
			return -1;
		}

		/* oh_encode_sensorreading: No digits testcase */
		str = "There are no numbers in this string";
		oh_init_textbuffer(&buffer);
		oh_append_textbuffer(&buffer, str);
		err = oh_encode_sensorreading(&buffer, SAHPI_SENSOR_READING_TYPE_INT64, &reading);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		if (reading.Value.SensorInt64 != 0) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Expected Zero value; Received=%lld\n", reading.Value.SensorInt64);
			return -1;
		}

		/* oh_encode_sensorreading: Decimal point testcase */
		str = "-2.5volts";
		expected_float64 = -2.5;
		oh_init_textbuffer(&buffer);
		oh_append_textbuffer(&buffer, str);
		err = oh_encode_sensorreading(&buffer, SAHPI_SENSOR_READING_TYPE_FLOAT64, &reading);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		if (reading.Value.SensorFloat64 != expected_float64) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received value=%le; Expected value=%le\n", 
			       reading.Value.SensorFloat64, expected_float64);
			return -1;
		}

		/* oh_encode_sensorreading: Too many decimal points testcase */
		str = "1.000.000 volts";
		expected_err = SA_ERR_HPI_INVALID_DATA;
		oh_init_textbuffer(&buffer);
		oh_append_textbuffer(&buffer, str);
		err = oh_encode_sensorreading(&buffer, SAHPI_SENSOR_READING_TYPE_FLOAT64, &reading);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}

		/* oh_encode_sensorreading: Too many signs */
		str = "+-33e02";
		expected_err = SA_ERR_HPI_INVALID_DATA;
		oh_init_textbuffer(&buffer);
		oh_append_textbuffer(&buffer, str);
		err = oh_encode_sensorreading(&buffer, SAHPI_SENSOR_READING_TYPE_INT64, &reading);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}

		/* oh_encode_sensorreading: Percentage testcase */
		str = "33% RPM";
		expected_float64 = 33;
		oh_init_textbuffer(&buffer);
		oh_append_textbuffer(&buffer, str);
		err = oh_encode_sensorreading(&buffer, SAHPI_SENSOR_READING_TYPE_FLOAT64, &reading);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		if (reading.Value.SensorFloat64 != expected_float64) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received value=%le; Expected value=%le\n", 
			       reading.Value.SensorFloat64, expected_float64);
			return -1;
		}

		/* oh_encode_sensorreading: Too big int64 testcase */
		str = "99999999999999999999999999999999";
		expected_err = SA_ERR_HPI_INVALID_DATA;
		oh_init_textbuffer(&buffer);
		oh_append_textbuffer(&buffer, str);
		err = oh_encode_sensorreading(&buffer, SAHPI_SENSOR_READING_TYPE_INT64, &reading);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}

		/* oh_encode_sensorreading: Too big uint64 testcase */
		str = "99999999999999999999999999999999";
		expected_err = SA_ERR_HPI_INVALID_DATA;
		oh_init_textbuffer(&buffer);
		oh_append_textbuffer(&buffer, str);
		err = oh_encode_sensorreading(&buffer, SAHPI_SENSOR_READING_TYPE_UINT64, &reading);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}

#if 0
		/* oh_encode_sensorreading: Too big float64 testcase */
		str = "99999999999999999999999999999999";
		expected_err = SA_ERR_HPI_INVALID_DATA;
		oh_init_textbuffer(&buffer);
		oh_append_textbuffer(&buffer, str);
		err = oh_encode_sensorreading(&buffer, SAHPI_SENSOR_READING_TYPE_FLOAT64, &reading);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}
#endif
	}

	/***************************** 
	 * oh_xxx_textbuffer testcases
         *****************************/
	{
		char str[4] = "1234";

		/* oh_init_textbuffer: NULL buffer testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;
		
		err = oh_init_textbuffer(0);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}
		
		/* oh_append_textbuffer: NULL buffer testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_append_textbuffer(0, str);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}

		/* oh_append_textbuffer: NULL str testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_append_textbuffer(&buffer, 0);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}

		/* oh_append_textbuffer: Out of space testcase */
		{ 
			char bigstr[SAHPI_MAX_TEXT_BUFFER_LENGTH +1];
			
			expected_err = SA_ERR_HPI_OUT_OF_SPACE;
			memset(bigstr, 0x32, SAHPI_MAX_TEXT_BUFFER_LENGTH +1);
			
			err = oh_append_textbuffer(&buffer, bigstr);
			if (err != expected_err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d; Expected error=%d\n", err, expected_err);
				return -1;
			}
		}

		/* oh_copy_textbuffer: NULL buffer testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_copy_textbuffer(0, 0);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}
	}

	/******************************************** 
	 * oh_fprint_text/oh_fprint_bigtext testcases
         ********************************************/
	{
		str = "OK - Printing Line 1\nOK - Printing Line 2";
		oh_big_textbuffer big_buffer, big_bad_buffer;

		/* Don't need this if expose the oh_xxx_bigtext routines */
		big_buffer.DataType = SAHPI_TL_TYPE_TEXT;
		big_buffer.Language = SAHPI_LANG_ENGLISH;
		memset(big_buffer.Data, 0x32, SAHPI_MAX_TEXT_BUFFER_LENGTH + 2);
		big_buffer.Data[SAHPI_MAX_TEXT_BUFFER_LENGTH + 2] = 0x00;
		big_buffer.Data[SAHPI_MAX_TEXT_BUFFER_LENGTH + 1] = 0x33;
		big_bad_buffer = big_buffer;

		err = oh_init_textbuffer(&buffer);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
		err = oh_append_textbuffer(&buffer, str);	
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		/* oh_fprint_text: oh_print_text MACRO testcase */
		err = oh_print_text(&buffer);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		err = oh_print_bigtext(&big_buffer);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

                /* Bad data type testcase */
		expected_err = SA_ERR_HPI_INVALID_DATA;
		err = oh_copy_textbuffer(&bad_buffer, &buffer);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		bad_buffer.DataType = BAD_TYPE;
		err = oh_print_text(&bad_buffer);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}

		big_bad_buffer.DataType = BAD_TYPE;
		err = oh_print_bigtext(&big_bad_buffer);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}

#if 0
		/* FIXME :: ??? Is there a way to force a bad FILE ID, without blowing up??? */
		/* Bad file handler testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_fprint_text(0, &buffer);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}

		err = oh_fprint_bigtext(0, &big_buffer);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}
#endif
		/* Normal write to file testcase */

		{
			FILE *fp, *big_fp;
			const char *name = "tmp";
			const char *big_name = "tmpbig";
			const char *mode = "a";

			fp = fopen(name, mode);
			if (fp == NULL) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				return -1;
			}
			err = oh_fprint_text(fp, &buffer);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}

			big_fp = fopen(big_name, mode);
			if (big_fp == NULL) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				return -1;
			}
			err = oh_fprint_bigtext(big_fp, &big_buffer);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}

			fclose(fp);
			fclose(big_fp);

			unlink(name);
			unlink(big_name);
		}
	}

	/****************************** 
	 * oh_print_sensorrec testcases
         ******************************/	
	{
		SaHpiSensorRecT sensor, default_sensor;
		memset(&sensor, 0, sizeof(SaHpiSensorRecT));
		memset(&default_sensor, 0, sizeof(SaHpiSensorRecT));
		
		sensor.Num = 1;
		sensor.Type = SAHPI_VOLTAGE;
		sensor.Category = SAHPI_EC_THRESHOLD;
		sensor.Events = SAHPI_ES_LOWER_MAJOR | SAHPI_ES_LOWER_MINOR;
		sensor.EventCtrl = SAHPI_SEC_READ_ONLY;
		sensor.DataFormat.IsSupported = SAHPI_TRUE;
		sensor.DataFormat.ReadingType = SAHPI_SENSOR_READING_TYPE_INT64;
		sensor.DataFormat.BaseUnits = SAHPI_SU_VOLTS;
		sensor.DataFormat.ModifierUnits = SAHPI_SU_SECOND;
		sensor.DataFormat.ModifierUse = SAHPI_SMUU_BASIC_TIMES_MODIFIER;
		sensor.DataFormat.Percentage = SAHPI_FALSE;
		sensor.DataFormat.Range.Flags = SAHPI_SRF_MIN | SAHPI_SRF_MAX | SAHPI_SRF_NOMINAL |
			                        SAHPI_SRF_NORMAL_MIN | SAHPI_SRF_NORMAL_MAX;
		sensor.DataFormat.Range.Min.IsSupported = SAHPI_TRUE;
		sensor.DataFormat.Range.Min.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		sensor.DataFormat.Range.Min.Value.SensorInt64 = 0;
		sensor.DataFormat.Range.Max.IsSupported = SAHPI_TRUE;
		sensor.DataFormat.Range.Max.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		sensor.DataFormat.Range.Max.Value.SensorInt64 = 100;
		sensor.DataFormat.Range.Nominal.IsSupported = SAHPI_TRUE;
		sensor.DataFormat.Range.Nominal.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		sensor.DataFormat.Range.Nominal.Value.SensorInt64 = 50;
		sensor.DataFormat.Range.NormalMax.IsSupported = SAHPI_TRUE;
		sensor.DataFormat.Range.NormalMax.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		sensor.DataFormat.Range.NormalMax.Value.SensorInt64 = 75;
		sensor.DataFormat.Range.NormalMin.IsSupported = SAHPI_TRUE;
		sensor.DataFormat.Range.NormalMin.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		sensor.DataFormat.Range.NormalMin.Value.SensorInt64 = 25;
		sensor.DataFormat.AccuracyFactor = 0.05;
		sensor.Oem = 0xFF;
		sensor.ThresholdDefn.IsAccessible = SAHPI_TRUE;
		sensor.ThresholdDefn.ReadThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_LOW_MAJOR | SAHPI_STM_LOW_CRIT | SAHPI_STM_LOW_HYSTERESIS;
		sensor.ThresholdDefn.WriteThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_MAJOR | SAHPI_STM_UP_CRIT | SAHPI_STM_UP_HYSTERESIS; 
		sensor.ThresholdDefn.Nonlinear = SAHPI_TRUE;
			
		/* oh_print_sensorrec: Bad parameter testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_print_sensorrec(0, 0);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}

		/* oh_print_sensorrec: Default sensor testcase */
		memset(&default_sensor, 0, sizeof(SaHpiSensorRecT));
		err = oh_print_sensorrec(&default_sensor, 0);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		/* oh_print_sensorrec: Normal sensor testcase */
		err = oh_print_sensorrec(&sensor, 0);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
	}

	/******************************* 
	 * oh_print_textbuffer testcases
         *******************************/	
	{
		SaHpiTextBufferT textbuffer, default_textbuffer;
		
		textbuffer.DataType = SAHPI_TL_TYPE_TEXT;
		textbuffer.Language = SAHPI_LANG_ZULU;
		strcpy((char *)textbuffer.Data, "Test Data");
		textbuffer.DataLength = strlen((char *)textbuffer.Data);

		/* oh_print_textbuffer: Bad parameter testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_print_textbuffer(0, 0);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}
		
		/* oh_print_textbuffer: Default textbuffer testcase */
		printf("Default TextBuffer\n");
		err = oh_print_textbuffer(&default_textbuffer, 1);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		/* oh_print_textbuffer: Normal textbuffer testcase */
		printf("Normal TextBuffer\n");
		err = oh_print_textbuffer(&textbuffer, 1);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}
	}

	/************************** 
	 * oh_print_event testcases
         **************************/	
	{
		SaHpiEventT sensor_event, default_event;
		memset(&sensor_event, 0, sizeof(SaHpiEventT));
		memset(&default_event, 0, sizeof(SaHpiEventT));

		sensor_event.Source = 1;
		sensor_event.EventType = SAHPI_ET_SENSOR;
		sensor_event.Severity = SAHPI_CRITICAL;
		sensor_event.EventDataUnion.SensorEvent.SensorNum = 2;
		sensor_event.EventDataUnion.SensorEvent.SensorType = SAHPI_VOLTAGE;
		sensor_event.EventDataUnion.SensorEvent.EventCategory = SAHPI_EC_THRESHOLD;
		sensor_event.EventDataUnion.SensorEvent.Assertion = SAHPI_TRUE;
		sensor_event.EventDataUnion.SensorEvent.EventState = SAHPI_ES_LOWER_MINOR;
		sensor_event.EventDataUnion.SensorEvent.OptionalDataPresent = 
			sensor_event.EventDataUnion.SensorEvent.OptionalDataPresent |
			SAHPI_SOD_TRIGGER_READING |
			SAHPI_SOD_TRIGGER_THRESHOLD |
			SAHPI_SOD_PREVIOUS_STATE |
			SAHPI_SOD_CURRENT_STATE |
			SAHPI_SOD_OEM |
			SAHPI_SOD_SENSOR_SPECIFIC;
		sensor_event.EventDataUnion.SensorEvent.TriggerReading.IsSupported = SAHPI_TRUE;
		sensor_event.EventDataUnion.SensorEvent.TriggerReading.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		sensor_event.EventDataUnion.SensorEvent.TriggerReading.Value.SensorInt64 = 100;
		sensor_event.EventDataUnion.SensorEvent.TriggerThreshold.IsSupported = SAHPI_TRUE;
		sensor_event.EventDataUnion.SensorEvent.TriggerThreshold.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		sensor_event.EventDataUnion.SensorEvent.TriggerThreshold.Value.SensorInt64 = 101;
		sensor_event.EventDataUnion.SensorEvent.PreviousState = SAHPI_ES_LOWER_MINOR | SAHPI_ES_LOWER_MAJOR;
		sensor_event.EventDataUnion.SensorEvent.CurrentState = SAHPI_ES_LOWER_MINOR;
		sensor_event.EventDataUnion.SensorEvent.Oem = 32;
		sensor_event.EventDataUnion.SensorEvent.SensorSpecific = 33;

		/* oh_print_event: Bad parameter testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_print_event(0, NULL, 0);
		if (err != expected_err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\n", err, expected_err);
			return -1;
		}
		
		/* oh_print_event: Default event testcase */
		printf("Default Event\n");
		err = oh_print_event(&default_event, NULL, 1);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		/* oh_print_event: Normal sensor event testcase */
		printf("Normal Sensor Event\n");
		err = oh_print_event(&sensor_event, NULL, 1);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		/* oh_print_event: Normal sensor event - no optional sensor data testcase */
		printf("Normal Sensor Event - no optional sensor data\n");
		sensor_event.EventDataUnion.SensorEvent.OptionalDataPresent = 0;
		err = oh_print_event(&sensor_event, NULL, 1);
		if (err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

		/*****************
		 * Resource events
		 *****************/
		{
			SaHpiEventT default_resource_event;
			memset(&default_resource_event, 0, sizeof(SaHpiEventT));

			default_resource_event.Source = 1;
			default_resource_event.EventType = SAHPI_ET_RESOURCE;
			default_resource_event.Severity = SAHPI_CRITICAL;

			/*  oh_print_event: Zero resource event testcase */
			printf("Default Resource Event - no data\n");
			err = oh_print_event(&default_resource_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
			
			/* oh_print_event: Normal resource event testcase */
			default_resource_event.EventDataUnion.ResourceEvent.ResourceEventType = SAHPI_RESE_RESOURCE_RESTORED;
			printf("Normal Resource Event\n");
			err = oh_print_event(&default_resource_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
		}

		/***************
		 * Domain events
		 ***************/
		{
			SaHpiEventT default_domain_event;
			memset(&default_domain_event, 0, sizeof(SaHpiEventT));

			default_domain_event.Source = 1;
			default_domain_event.EventType = SAHPI_ET_DOMAIN;
			default_domain_event.Severity = SAHPI_CRITICAL;

			/*  oh_print_event: Zero domain event testcase */
			printf("Default Domain Event - no data\n");
			err = oh_print_event(&default_domain_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
			
			/* oh_print_event: Normal domain event testcase */
			default_domain_event.EventDataUnion.DomainEvent.Type = SAHPI_DOMAIN_REF_ADDED;
			default_domain_event.EventDataUnion.DomainEvent.DomainId = 1;
			printf("Normal Domain Event\n");
			err = oh_print_event(&default_domain_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
		}

		/*****************************
		 * Sensor Enable Change events
		 *****************************/
		{
			SaHpiEventT default_sensor_enable_event;
			memset(&default_sensor_enable_event, 0, sizeof(SaHpiEventT));

			default_sensor_enable_event.Source = 1;
			default_sensor_enable_event.EventType = SAHPI_ET_SENSOR_ENABLE_CHANGE;
			default_sensor_enable_event.Severity = SAHPI_CRITICAL;

			/*  oh_print_event: Zero sensor enable event testcase */
			printf("Default Sensor Enable Event - no data\n");
			err = oh_print_event(&default_sensor_enable_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
			
			/* oh_print_event: Normal sensor enable event testcase */
			default_sensor_enable_event.EventDataUnion.SensorEnableChangeEvent.SensorNum = 1;
			default_sensor_enable_event.EventDataUnion.SensorEnableChangeEvent.SensorType = SAHPI_FAN;
			default_sensor_enable_event.EventDataUnion.SensorEnableChangeEvent.EventCategory = SAHPI_EC_THRESHOLD;
			default_sensor_enable_event.EventDataUnion.SensorEnableChangeEvent.SensorEnable = SAHPI_TRUE;
			default_sensor_enable_event.EventDataUnion.SensorEnableChangeEvent.SensorEventEnable = SAHPI_TRUE;
			default_sensor_enable_event.EventDataUnion.SensorEnableChangeEvent.AssertEventMask =
				SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR;
			default_sensor_enable_event.EventDataUnion.SensorEnableChangeEvent.DeassertEventMask =
				SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR;
			default_sensor_enable_event.EventDataUnion.SensorEnableChangeEvent.OptionalDataPresent = SAHPI_SEOD_CURRENT_STATE;
			default_sensor_enable_event.EventDataUnion.SensorEnableChangeEvent.CurrentState = SAHPI_ES_LOWER_MINOR;

			printf("Normal sensor enable Event\n");
			err = oh_print_event(&default_sensor_enable_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
		}

		/****************
		 * Hotswap events
		 ****************/
		{
			SaHpiEventT default_hotswap_event;
			memset(&default_hotswap_event, 0, sizeof(SaHpiEventT));

			default_hotswap_event.Source = 1;
			default_hotswap_event.EventType = SAHPI_ET_HOTSWAP;
			default_hotswap_event.Severity = SAHPI_CRITICAL;

			/*  oh_print_event: Zero hotswap event testcase */
			printf("Default Hotswap Event - no data\n");
			err = oh_print_event(&default_hotswap_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
			
			/* oh_print_event: Normal hotswap event testcase */
			default_hotswap_event.EventDataUnion.HotSwapEvent.HotSwapState = SAHPI_HS_STATE_ACTIVE;
			default_hotswap_event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = SAHPI_HS_STATE_INSERTION_PENDING;

			printf("Normal Hotswap Event\n");
			err = oh_print_event(&default_hotswap_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
		}

		/*****************
		 * Watchdog events
		 *****************/
		{
			SaHpiEventT default_watchdog_event;
			memset(&default_watchdog_event, 0, sizeof(SaHpiEventT));

			default_watchdog_event.Source = 1;
			default_watchdog_event.EventType = SAHPI_ET_WATCHDOG;
			default_watchdog_event.Severity = SAHPI_CRITICAL;

			/*  oh_print_event: Zero watchdog event testcase */
			printf("Default Watchdog Event - no data\n");
			err = oh_print_event(&default_watchdog_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
			
			/* oh_print_event: Normal watchdog event testcase */
			default_watchdog_event.EventDataUnion.WatchdogEvent.WatchdogNum = 1;
			default_watchdog_event.EventDataUnion.WatchdogEvent.WatchdogAction = SAHPI_WAE_POWER_DOWN;
			default_watchdog_event.EventDataUnion.WatchdogEvent.WatchdogPreTimerAction = SAHPI_WPI_NMI;
			default_watchdog_event.EventDataUnion.WatchdogEvent.WatchdogUse = SAHPI_WTU_OEM;

			printf("Normal Watchdog Event\n");
			err = oh_print_event(&default_watchdog_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
		}

		/***************
		 * HPI SW events
		 ***************/
		{
			SaHpiEventT default_hpisw_event;
			memset(&default_hpisw_event, 0, sizeof(SaHpiEventT));

			default_hpisw_event.Source = 1;
			default_hpisw_event.EventType = SAHPI_ET_HPI_SW;
			default_hpisw_event.Severity = SAHPI_CRITICAL;

			/*  oh_print_event: Zero HPI software event testcase */
			printf("Default HPI Software Event - no data\n");
			err = oh_print_event(&default_hpisw_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
			
			/* oh_print_event: Normal HPI software event testcase */
			default_hpisw_event.EventDataUnion.HpiSwEvent.MId = 1;
			default_hpisw_event.EventDataUnion.HpiSwEvent.Type = SAHPI_HPIE_AUDIT;
			default_hpisw_event.EventDataUnion.HpiSwEvent.EventData.DataType = SAHPI_TL_TYPE_TEXT;
			default_hpisw_event.EventDataUnion.HpiSwEvent.EventData.Language = SAHPI_LANG_URDU;
			default_hpisw_event.EventDataUnion.HpiSwEvent.EventData.DataLength = sizeof("HPI software event");
			strncpy((char *)(default_hpisw_event.EventDataUnion.HpiSwEvent.EventData.Data),
				"HPI software event",
				strlen("HPI software event"));

			printf("Normal HPI Sotware Event\n");
			err = oh_print_event(&default_hpisw_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
		}

		/****************
		 * HPI OEM events
		 ****************/
		{
			SaHpiEventT default_oem_event;
			memset(&default_oem_event, 0, sizeof(SaHpiEventT));

			default_oem_event.Source = 1;
			default_oem_event.EventType = SAHPI_ET_OEM;
			default_oem_event.Severity = SAHPI_CRITICAL;

			/*  oh_print_event: Zero OEM event testcase */
			printf("Default OEM Event - no data\n");
			err = oh_print_event(&default_oem_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
			
			/* oh_print_event: Normal OEM event testcase */
			default_oem_event.EventDataUnion.OemEvent.MId = 1;
			default_oem_event.EventDataUnion.OemEvent.OemEventData.DataType = SAHPI_TL_TYPE_TEXT;
			default_oem_event.EventDataUnion.OemEvent.OemEventData.Language = SAHPI_LANG_URDU;
			default_oem_event.EventDataUnion.OemEvent.OemEventData.DataLength = sizeof("OEM Event");
			strncpy((char *)(default_oem_event.EventDataUnion.OemEvent.OemEventData.Data),
				"OEM Event",
				strlen("OEM Event"));

			printf("Normal OEM Event\n");
			err = oh_print_event(&default_oem_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
		}

		/*****************
		 * HPI User events
		 *****************/
		{
			SaHpiEventT default_user_event;
			memset(&default_user_event, 0, sizeof(SaHpiEventT));

			default_user_event.Source = 1;
			default_user_event.EventType = SAHPI_ET_USER;
			default_user_event.Severity = SAHPI_CRITICAL;

			/*  oh_print_event: Zero User event testcase */
			printf("Default User Event - no data\n");
			err = oh_print_event(&default_user_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
			
			/* oh_print_event: Normal User event testcase */
			default_user_event.EventDataUnion.UserEvent.UserEventData.DataType = SAHPI_TL_TYPE_TEXT;
			default_user_event.EventDataUnion.UserEvent.UserEventData.Language = SAHPI_LANG_URDU;
			default_user_event.EventDataUnion.UserEvent.UserEventData.DataLength = sizeof("User Event");
			strncpy((char *)(default_user_event.EventDataUnion.UserEvent.UserEventData.Data),
				"User Event",
				strlen("User Event"));

			printf("Normal User Event\n");
			err = oh_print_event(&default_user_event, NULL, 1);
			if (err) {
				printf("  Error! Testcase failed. Line=%d\n", __LINE__);
				printf("  Received error=%d\n", err);
				return -1;
			}
		}
	}

	/****************************
	 * oh_print_ctrlrec testcases
         ****************************/
	SaHpiCtrlRecT control;
	memset(&control, 0, sizeof(SaHpiCtrlRecT));

	/* oh_print_ctrlrec: Default testcase */
	printf("Print control - default case\n");
	err = oh_print_ctrlrec(&control, 1);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		return -1;
	}
	
	control.Num = 1;
	control.OutputType = SAHPI_CTRL_LED;
	control.DefaultMode.Mode = SAHPI_CTRL_MODE_AUTO;
	control.DefaultMode.ReadOnly = SAHPI_TRUE;
	control.WriteOnly = SAHPI_TRUE;
	control.Oem = 0;
	     
	/* oh_print_ctrlrec: Normal digital testcase */
	control.Type = SAHPI_CTRL_TYPE_DIGITAL;
	control.TypeUnion.Digital.Default = SAHPI_CTRL_STATE_PULSE_ON;

	printf("Print control - normal digital case\n");
	err = oh_print_ctrlrec(&control, 1);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		return -1;
	}

	/* oh_print_ctrlrec: Normal discrete testcase */
	control.Type = SAHPI_CTRL_TYPE_DISCRETE;
	control.TypeUnion.Discrete.Default = 2;

	printf("Print control - normal discrete case\n");
	err = oh_print_ctrlrec(&control, 1);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		return -1;
	}

	/* oh_print_ctrlrec: Normal analog testcase */
	control.Type = SAHPI_CTRL_TYPE_ANALOG;
	control.TypeUnion.Analog.Min = 1;
	control.TypeUnion.Analog.Max = 10;
	control.TypeUnion.Analog.Default = 5;
	
	printf("Print control - normal analog case\n");
	err = oh_print_ctrlrec(&control, 1);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		return -1;
	}

	/* oh_print_ctrlrec: Normal stream testcase */
	control.Type = SAHPI_CTRL_TYPE_STREAM;
	control.TypeUnion.Stream.Default.Repeat = SAHPI_TRUE;
	control.TypeUnion.Stream.Default.StreamLength = MIN(SAHPI_CTRL_MAX_STREAM_LENGTH, strlen("Stream Data"));
        strncpy((char *)control.TypeUnion.Stream.Default.Stream, "Stream Data", SAHPI_CTRL_MAX_STREAM_LENGTH);
	
	printf("Print control - normal stream case\n");
	err = oh_print_ctrlrec(&control, 1);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		return -1;
	}

	/* oh_print_ctrlrec: Normal text testcase */
	control.Type = SAHPI_CTRL_TYPE_TEXT;
	control.TypeUnion.Text.MaxChars = 10;
	control.TypeUnion.Text.MaxLines = 100;
	control.TypeUnion.Text.Language = SAHPI_LANG_ENGLISH;
	control.TypeUnion.Text.DataType = SAHPI_TL_TYPE_TEXT;
	control.TypeUnion.Text.Default.Line = 1;
	control.TypeUnion.Text.Default.Text.DataType = SAHPI_TL_TYPE_TEXT;
	control.TypeUnion.Text.Default.Text.Language = SAHPI_LANG_ENGLISH;
	control.TypeUnion.Text.Default.Text.DataLength = MIN(SAHPI_MAX_TEXT_BUFFER_LENGTH, strlen("Text Data"));
        strncpy((char *)(control.TypeUnion.Text.Default.Text.Data), "Text Data", SAHPI_MAX_TEXT_BUFFER_LENGTH);
	
	printf("Print control - normal text case\n");
	err = oh_print_ctrlrec(&control, 1);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		return -1;
	}

	/* oh_print_ctrlrec: Normal oem testcase */
	control.Type = SAHPI_CTRL_TYPE_OEM;
	control.TypeUnion.Oem.MId = 1;
	strncpy((char *)control.TypeUnion.Oem.ConfigData, "Config Data", SAHPI_CTRL_OEM_CONFIG_LENGTH);
	control.TypeUnion.Oem.Default.MId = 1;
	control.TypeUnion.Oem.Default.BodyLength = MIN(SAHPI_CTRL_MAX_OEM_BODY_LENGTH, strlen("Config Default"));
	strncpy((char *)control.TypeUnion.Oem.Default.Body, "Config Default", SAHPI_CTRL_MAX_OEM_BODY_LENGTH); 
	
	printf("Print control - normal OEM case\n");
	err = oh_print_ctrlrec(&control, 1);
	if (err) {
		printf("  Error! Testcase failed. Line=%d\n", __LINE__);
		printf("  Received error=%d\n", err);
		return -1;
	}

	/*******************************
	 * oh_valid_textbuffer testcases
         *******************************/
	{
		SaHpiTextBufferT buffer;
		SaHpiBoolT result, expected_result;

		/* oh_valid_textbuffer: NULL buffer testcase */
		expected_result = SAHPI_FALSE;
		result = oh_valid_textbuffer(0);
		if (result != expected_result) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			return -1;
		}

		/* oh_valid_textbuffer: Bad text type testcase */
		expected_result = SAHPI_FALSE;
		oh_init_textbuffer(&buffer);
		buffer.DataType = BAD_TYPE;
		
		result = oh_valid_textbuffer(&buffer);
		if (result != expected_result) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			return -1;
		}

		/* oh_valid_textbuffer: Bad language type testcase */
		expected_result = SAHPI_FALSE;
		oh_init_textbuffer(&buffer);
		buffer.Language = BAD_TYPE;
		
		result = oh_valid_textbuffer(&buffer);
		if (result != expected_result) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			return -1;
		}
		
		/* oh_valid_textbuffer: Bad Unicode length testcase */
		expected_result = SAHPI_FALSE;
		oh_init_textbuffer(&buffer);
		buffer.DataType = SAHPI_TL_TYPE_UNICODE;
		strcpy((char *)buffer.Data, "123");
		buffer.DataLength = strlen("123");

		result = oh_valid_textbuffer(&buffer);
		if (result != expected_result) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			return -1;
		}

                /* oh_valid_textbuffer: Bad Data text cases */
                expected_result = SAHPI_FALSE;
		oh_init_textbuffer(&buffer);
		buffer.DataType = SAHPI_TL_TYPE_BCDPLUS;
		strcpy((char *)buffer.Data, "123");
                buffer.Data[1] = ';';
		buffer.DataLength = strlen("123");

		result = oh_valid_textbuffer(&buffer);
		if (result != expected_result) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			return -1;
		}
           
                buffer.DataType = SAHPI_TL_TYPE_ASCII6;
                buffer.Data[1] = 0xff;
                result = oh_valid_textbuffer(&buffer);
		if (result != expected_result) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			return -1;
		}
		
		/* oh_valid_textbuffer: Good buffer testcase */
		expected_result = SAHPI_TRUE;
		oh_init_textbuffer(&buffer);

		result = oh_valid_textbuffer(&buffer);
		if (result != expected_result) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			return -1;
		}
	}

	/*******************************
	 * oh_valid_thresholds testcases
         *******************************/
	{
		SaHpiRdrT default_rdr, test_rdr;
		SaHpiSensorThresholdsT default_thresholds_int64, test_thresholds_int64;
		SaHpiSensorThresholdsT default_thresholds_float64, test_thresholds_float64;
		SaHpiSensorThresholdsT default_thresholds_uint64, test_thresholds_uint64;

		SaHpiSensorDataFormatT default_format_int64;
		SaHpiSensorDataFormatT default_format_float64;
		SaHpiSensorDataFormatT default_format_uint64;

		default_rdr.RecordId = 1;
		default_rdr.RdrType = SAHPI_SENSOR_RDR;
                default_rdr.Entity.Entry[0].EntityType = SAHPI_ENT_ROOT;
		default_rdr.Entity.Entry[0].EntityLocation = 0;
		default_rdr.IsFru = SAHPI_TRUE;
		default_rdr.RdrTypeUnion.SensorRec.Num = 1;
		default_rdr.RdrTypeUnion.SensorRec.Type = SAHPI_TEMPERATURE;
		default_rdr.RdrTypeUnion.SensorRec.Category = SAHPI_EC_THRESHOLD;
		default_rdr.RdrTypeUnion.SensorRec.EnableCtrl = SAHPI_FALSE;
		default_rdr.RdrTypeUnion.SensorRec.EventCtrl= SAHPI_SEC_READ_ONLY;
		default_rdr.RdrTypeUnion.SensorRec.Events = 
			SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT;
		default_rdr.RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
		default_rdr.RdrTypeUnion.SensorRec.DataFormat.ReadingType = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		default_rdr.RdrTypeUnion.SensorRec.DataFormat.BaseUnits = SAHPI_SU_DEGREES_C;
		default_rdr.RdrTypeUnion.SensorRec.DataFormat.ModifierUnits = SAHPI_SU_UNSPECIFIED;
		default_rdr.RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
		default_rdr.RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
		default_rdr.RdrTypeUnion.SensorRec.DataFormat.Range.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN;
		default_rdr.RdrTypeUnion.SensorRec.DataFormat.Range.Max.IsSupported = SAHPI_TRUE;
		default_rdr.RdrTypeUnion.SensorRec.DataFormat.Range.Max.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		default_rdr.RdrTypeUnion.SensorRec.DataFormat.Range.Max.Value.SensorFloat64 = 125;
		default_rdr.RdrTypeUnion.SensorRec.DataFormat.Range.Min.IsSupported = SAHPI_TRUE;
		default_rdr.RdrTypeUnion.SensorRec.DataFormat.Range.Min.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		default_rdr.RdrTypeUnion.SensorRec.DataFormat.Range.Min.Value.SensorFloat64 = 0;
		default_rdr.RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_TRUE;
		default_rdr.RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold = 0;
		default_rdr.RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold = 
			SAHPI_STM_LOW_MINOR | SAHPI_STM_LOW_MAJOR | SAHPI_STM_LOW_CRIT |
			SAHPI_STM_UP_MINOR | SAHPI_STM_UP_MAJOR | SAHPI_STM_UP_CRIT |
			SAHPI_STM_UP_HYSTERESIS | SAHPI_STM_LOW_HYSTERESIS;
		
		default_thresholds_int64.UpCritical.IsSupported = SAHPI_TRUE;
		default_thresholds_int64.UpCritical.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		default_thresholds_int64.UpCritical.Value.SensorInt64 = 50;
		default_thresholds_int64.UpMajor.IsSupported = SAHPI_TRUE;
		default_thresholds_int64.UpMajor.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		default_thresholds_int64.UpMajor.Value.SensorInt64 = 40;
		default_thresholds_int64.UpMinor.IsSupported = SAHPI_TRUE;
		default_thresholds_int64.UpMinor.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		default_thresholds_int64.UpMinor.Value.SensorInt64 = 30;
		default_thresholds_int64.LowMinor.IsSupported = SAHPI_TRUE;
		default_thresholds_int64.LowMinor.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		default_thresholds_int64.LowMinor.Value.SensorInt64 = 20;
		default_thresholds_int64.LowMajor.IsSupported = SAHPI_TRUE;
		default_thresholds_int64.LowMajor.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		default_thresholds_int64.LowMajor.Value.SensorInt64 = 10;
		default_thresholds_int64.LowCritical.IsSupported = SAHPI_TRUE;
		default_thresholds_int64.LowCritical.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		default_thresholds_int64.LowCritical.Value.SensorInt64 = 0;
		default_thresholds_int64.PosThdHysteresis.IsSupported = SAHPI_TRUE;
		default_thresholds_int64.PosThdHysteresis.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		default_thresholds_int64.PosThdHysteresis.Value.SensorInt64 = 2;
		default_thresholds_int64.NegThdHysteresis.IsSupported = SAHPI_TRUE;
		default_thresholds_int64.NegThdHysteresis.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		default_thresholds_int64.NegThdHysteresis.Value.SensorInt64 = 2;

		default_format_int64.IsSupported = SAHPI_TRUE;
		default_format_int64.ReadingType = SAHPI_SENSOR_READING_TYPE_INT64;
		default_format_int64.Range.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN;
		default_format_int64.Range.Max.Value.SensorInt64 = 60;
		default_format_int64.Range.Min.Value.SensorInt64 = 0;

		/* oh_valid_thresholds: Bad parameters testcase */
		expected_err = SA_ERR_HPI_INVALID_PARAMS;
		err = oh_valid_thresholds(0, &default_rdr);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}
		
		/* oh_valid_thresholds: Bad RDR testcase */
		test_rdr = default_rdr;
		test_thresholds_int64 = default_thresholds_int64;	
		test_rdr.RdrType = SAHPI_WATCHDOG_RDR;
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_valid_thresholds(&test_thresholds_int64, &test_rdr);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_thresholds: Bad threshold type testcase */
		test_rdr = default_rdr;
		test_rdr.RdrTypeUnion.SensorRec.DataFormat = default_format_int64;
		test_thresholds_int64 = default_thresholds_int64;
		
		expected_err = SA_ERR_HPI_INVALID_DATA;
		test_thresholds_int64.LowCritical.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		
		err = oh_valid_thresholds(&test_thresholds_int64, &test_rdr);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_thresholds: Bad text buffer type threshold testcase */
		test_rdr = default_rdr;
		test_rdr.RdrTypeUnion.SensorRec.DataFormat = default_format_int64;		
		test_thresholds_int64 = default_thresholds_int64;
		
		expected_err = SA_ERR_HPI_INVALID_DATA;
		test_thresholds_int64.LowCritical.Type = SAHPI_SENSOR_READING_TYPE_BUFFER;
		
		err = oh_valid_thresholds(&test_thresholds_int64, &test_rdr);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_thresholds: Bad threshold hysteresis testcase */
		test_rdr = default_rdr;
		test_rdr.RdrTypeUnion.SensorRec.DataFormat = default_format_int64;		
		test_thresholds_int64 = default_thresholds_int64;
		
		expected_err = SA_ERR_HPI_INVALID_DATA;
		test_thresholds_int64.PosThdHysteresis.Value.SensorInt64 = -1;
		test_rdr.RdrTypeUnion.SensorRec.DataFormat.Range.Flags = 
			test_rdr.RdrTypeUnion.SensorRec.DataFormat.Range.Flags &
			~SAHPI_SRF_MAX;
		test_rdr.RdrTypeUnion.SensorRec.DataFormat.Range.Flags = 
			test_rdr.RdrTypeUnion.SensorRec.DataFormat.Range.Flags &
			~SAHPI_SRF_MIN;
		
		err = oh_valid_thresholds(&test_thresholds_int64, &test_rdr);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_thresholds: Bad range threshold testcase */
		test_rdr = default_rdr;
		test_rdr.RdrTypeUnion.SensorRec.DataFormat = default_format_int64;		
		test_thresholds_int64 = default_thresholds_int64;

		expected_err = SA_ERR_HPI_INVALID_CMD;
		test_rdr.RdrTypeUnion.SensorRec.DataFormat.Range.Max.Value.SensorInt64 = 0;

		err = oh_valid_thresholds(&test_thresholds_int64, &test_rdr);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_thresholds: Bad order threshold testcase */
		test_rdr = default_rdr;
		test_rdr.RdrTypeUnion.SensorRec.DataFormat = default_format_int64;		
		test_thresholds_int64 = default_thresholds_int64;

		expected_err = SA_ERR_HPI_INVALID_DATA;
		test_thresholds_int64.LowCritical.Value.SensorInt64 = 20;
		
		err = oh_valid_thresholds(&test_thresholds_int64, &test_rdr);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_thresholds: Bad writable threshold testcase */
		test_rdr = default_rdr;
		test_rdr.RdrTypeUnion.SensorRec.DataFormat = default_format_int64;		
		test_thresholds_int64 = default_thresholds_int64;

		expected_err = SA_ERR_HPI_INVALID_CMD;
		test_rdr.RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold = 
			SAHPI_STM_LOW_MINOR | SAHPI_STM_LOW_MAJOR | SAHPI_STM_LOW_CRIT;
		
		err = oh_valid_thresholds(&test_thresholds_int64, &test_rdr);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_thresholds: Normal threshold testcase - int64 */
		test_rdr = default_rdr;
		test_rdr.RdrTypeUnion.SensorRec.DataFormat = default_format_int64;		
		test_thresholds_int64 = default_thresholds_int64;

		expected_err = SA_OK;
		
		err = oh_valid_thresholds(&test_thresholds_int64, &test_rdr);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_thresholds: Normal subset testcase */
		test_rdr = default_rdr;
		test_rdr.RdrTypeUnion.SensorRec.DataFormat = default_format_int64;		
		test_thresholds_int64 = default_thresholds_int64;


		expected_err = SA_OK;
		test_thresholds_int64.UpCritical.IsSupported = SAHPI_FALSE;
		test_thresholds_int64.UpCritical.Type = BAD_TYPE; /* This should be ignored */
		test_thresholds_int64.LowCritical.IsSupported = SAHPI_FALSE;
		test_thresholds_int64.LowCritical.Type = BAD_TYPE; /* This should be ignored */

		err = oh_valid_thresholds(&test_thresholds_int64, &test_rdr);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

 		default_thresholds_float64.UpCritical.IsSupported = SAHPI_TRUE;
		default_thresholds_float64.UpCritical.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		default_thresholds_float64.UpCritical.Value.SensorFloat64 = 50.3;
		default_thresholds_float64.UpMajor.IsSupported = SAHPI_TRUE;
		default_thresholds_float64.UpMajor.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		default_thresholds_float64.UpMajor.Value.SensorFloat64 = 40.2;
		default_thresholds_float64.UpMinor.IsSupported = SAHPI_TRUE;
		default_thresholds_float64.UpMinor.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		default_thresholds_float64.UpMinor.Value.SensorFloat64 = 30.1;
		default_thresholds_float64.LowMinor.IsSupported = SAHPI_TRUE;
		default_thresholds_float64.LowMinor.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		default_thresholds_float64.LowMinor.Value.SensorFloat64 = 20.3;
		default_thresholds_float64.LowMajor.IsSupported = SAHPI_TRUE;
		default_thresholds_float64.LowMajor.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		default_thresholds_float64.LowMajor.Value.SensorFloat64 = 10.2;
		default_thresholds_float64.LowCritical.IsSupported = SAHPI_TRUE;
		default_thresholds_float64.LowCritical.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		default_thresholds_float64.LowCritical.Value.SensorFloat64 = 0.5;
		default_thresholds_float64.PosThdHysteresis.IsSupported = SAHPI_TRUE;
		default_thresholds_float64.PosThdHysteresis.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		default_thresholds_float64.PosThdHysteresis.Value.SensorFloat64 = 2;
		default_thresholds_float64.NegThdHysteresis.IsSupported = SAHPI_TRUE;
		default_thresholds_float64.NegThdHysteresis.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		default_thresholds_float64.NegThdHysteresis.Value.SensorFloat64 = 2;

		default_format_float64.IsSupported = SAHPI_TRUE;
		default_format_float64.ReadingType = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		default_format_float64.Range.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN;
		default_format_float64.Range.Max.Value.SensorFloat64 = 60;
		default_format_float64.Range.Min.Value.SensorFloat64 = 0;

                /* oh_valid_thresholds: Normal threshold testcase - float64 */
		test_rdr = default_rdr;
		test_rdr.RdrTypeUnion.SensorRec.DataFormat = default_format_float64;
		test_thresholds_float64 = default_thresholds_float64;

		expected_err = SA_OK;
		
		err = oh_valid_thresholds(&test_thresholds_float64, &test_rdr);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

 		default_thresholds_uint64.UpCritical.IsSupported = SAHPI_TRUE;
		default_thresholds_uint64.UpCritical.Type = SAHPI_SENSOR_READING_TYPE_UINT64;
		default_thresholds_uint64.UpCritical.Value.SensorUint64 = 50.3;
		default_thresholds_uint64.UpMajor.IsSupported = SAHPI_TRUE;
		default_thresholds_uint64.UpMajor.Type = SAHPI_SENSOR_READING_TYPE_UINT64;
		default_thresholds_uint64.UpMajor.Value.SensorUint64 = 40.2;
		default_thresholds_uint64.UpMinor.IsSupported = SAHPI_TRUE;
		default_thresholds_uint64.UpMinor.Type = SAHPI_SENSOR_READING_TYPE_UINT64;
		default_thresholds_uint64.UpMinor.Value.SensorUint64 = 30.1;
		default_thresholds_uint64.LowMinor.IsSupported = SAHPI_TRUE;
		default_thresholds_uint64.LowMinor.Type = SAHPI_SENSOR_READING_TYPE_UINT64;
		default_thresholds_uint64.LowMinor.Value.SensorUint64 = 20.3;
		default_thresholds_uint64.LowMajor.IsSupported = SAHPI_TRUE;
		default_thresholds_uint64.LowMajor.Type = SAHPI_SENSOR_READING_TYPE_UINT64;
		default_thresholds_uint64.LowMajor.Value.SensorUint64 = 10.2;
		default_thresholds_uint64.LowCritical.IsSupported = SAHPI_TRUE;
		default_thresholds_uint64.LowCritical.Type = SAHPI_SENSOR_READING_TYPE_UINT64;
		default_thresholds_uint64.LowCritical.Value.SensorUint64 = 0.5;
		default_thresholds_uint64.PosThdHysteresis.IsSupported = SAHPI_TRUE;
		default_thresholds_uint64.PosThdHysteresis.Type = SAHPI_SENSOR_READING_TYPE_UINT64;
		default_thresholds_uint64.PosThdHysteresis.Value.SensorUint64 = 2;
		default_thresholds_uint64.NegThdHysteresis.IsSupported = SAHPI_TRUE;
		default_thresholds_uint64.NegThdHysteresis.Type = SAHPI_SENSOR_READING_TYPE_UINT64;
		default_thresholds_uint64.NegThdHysteresis.Value.SensorUint64 = 2;

		default_format_uint64.IsSupported = SAHPI_TRUE;
		default_format_uint64.ReadingType = SAHPI_SENSOR_READING_TYPE_UINT64;
		default_format_uint64.Range.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN;
		default_format_uint64.Range.Max.Value.SensorUint64 = 60;
		default_format_uint64.Range.Min.Value.SensorUint64 = 0;



		/* oh_valid_thresholds: Normal threshold testcase - uint64 */
		test_rdr = default_rdr;
		test_rdr.RdrTypeUnion.SensorRec.DataFormat = default_format_uint64;
		test_thresholds_uint64 = default_thresholds_uint64;

		expected_err = SA_OK;
		
		err = oh_valid_thresholds(&test_thresholds_uint64, &test_rdr);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}
	}

	/************************************
	 * oh_compare_sensorreading testcases
         ************************************/
	{
		SaHpiSensorReadingT reading1, reading2;
		int rtn, expected_rtn;

		reading1.IsSupported = reading2.IsSupported = SAHPI_TRUE;
		reading1.Type = reading2.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
		
		/* oh_compare_sensorreading - reading1 < reading2 float64 */
		reading1.Value.SensorFloat64 = 5;
		reading2.Value.SensorFloat64 = 10;
		expected_rtn = -1;
		rtn = oh_compare_sensorreading(reading1.Type, &reading1, &reading2);
		if (rtn != expected_rtn) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received return code=%d\n", rtn);
			return -1;
		}

		/* oh_compare_sensorreading - reading1 = reading2 float64 */
		reading1.Value.SensorFloat64 = 5;
		reading2.Value.SensorFloat64 = 5;
		expected_rtn = 0;
		rtn = oh_compare_sensorreading(reading1.Type, &reading1, &reading2);
		if (rtn != expected_rtn) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received return code=%d\n", rtn);
			return -1;
		}

		/* oh_compare_sensorreading - reading1 = reading2 float64 */
		reading1.Value.SensorFloat64 = 10;
		reading2.Value.SensorFloat64 = 5;
		expected_rtn = 1;
		rtn = oh_compare_sensorreading(reading1.Type, &reading1, &reading2);
		if (rtn != expected_rtn) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received return code=%d\n", rtn);
			return -1;
		}

		reading1.Type = reading2.Type = SAHPI_SENSOR_READING_TYPE_INT64;
		/* oh_compare_sensorreading - reading1 < reading2 int64 */
		reading1.Value.SensorInt64 = 5;
		reading2.Value.SensorInt64 = 10;
		expected_rtn = -1;
		rtn = oh_compare_sensorreading(reading1.Type, &reading1, &reading2);
		if (rtn != expected_rtn) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received return code=%d\n", rtn);
			return -1;
		}

		/* oh_compare_sensorreading - reading1 = reading2 int64 */
		reading1.Value.SensorInt64 = 5;
		reading2.Value.SensorInt64 = 5;
		expected_rtn = 0;
		rtn = oh_compare_sensorreading(reading1.Type, &reading1, &reading2);
		if (rtn != expected_rtn) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received return code=%d\n", rtn);
			return -1;
		}

		/* oh_compare_sensorreading - reading1 = reading2 int64 */
		reading1.Value.SensorInt64 = 10;
		reading2.Value.SensorInt64 = 5;
		expected_rtn = 1;
		rtn = oh_compare_sensorreading(reading1.Type, &reading1, &reading2);
		if (rtn != expected_rtn) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received return code=%d\n", rtn);
			return -1;
		}

		reading1.Type = reading2.Type = SAHPI_SENSOR_READING_TYPE_UINT64;
		/* oh_compare_sensorreading - reading1 < reading2 uint64 */
		reading1.Value.SensorUint64 = 5;
		reading2.Value.SensorUint64 = 10;
		expected_rtn = -1;
		rtn = oh_compare_sensorreading(reading1.Type, &reading1, &reading2);
		if (rtn != expected_rtn) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received return code=%d\n", rtn);
			return -1;
		}

		/* oh_compare_sensorreading - reading1 = reading2 uint64 */
		reading1.Value.SensorUint64 = 5;
		reading2.Value.SensorUint64 = 5;
		expected_rtn = 0;
		rtn = oh_compare_sensorreading(reading1.Type, &reading1, &reading2);
		if (rtn != expected_rtn) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received return code=%d\n", rtn);
			return -1;
		}

		/* oh_compare_sensorreading - reading1 = reading2 uint64 */
		reading1.Value.SensorUint64 = 10;
		reading2.Value.SensorUint64 = 5;
		expected_rtn = 1;
		rtn = oh_compare_sensorreading(reading1.Type, &reading1, &reading2);
		if (rtn != expected_rtn) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received return code=%d\n", rtn);
			return -1;
		}

		reading1.Type = reading2.Type = SAHPI_SENSOR_READING_TYPE_BUFFER;
		/* oh_compare_sensorreading - reading1 < reading2 uint64 */
		strncpy((char *)reading1.Value.SensorBuffer, "AAA", SAHPI_SENSOR_BUFFER_LENGTH);
		strncpy((char *)reading2.Value.SensorBuffer, "BBB", SAHPI_SENSOR_BUFFER_LENGTH);
		expected_rtn = -1;
		rtn = oh_compare_sensorreading(reading1.Type, &reading1, &reading2);
		if (rtn != expected_rtn) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received return code=%d\n", rtn);
			return -1;
		}
	}

	/************************************
	 * oh_valid_ctrl_state_mode testcases
         ************************************/
	{
		SaHpiCtrlModeT default_mode, mode;
		SaHpiCtrlStateT default_state, state;
		SaHpiCtrlRecT default_rdr, rdr;

		memset(&default_mode, 0, sizeof(SaHpiCtrlModeT));
		memset(&default_state, 0, sizeof(SaHpiCtrlStateT));
		memset(&default_rdr, 0, sizeof(SaHpiCtrlRecT));
		
		default_mode = SAHPI_CTRL_MODE_AUTO;
		default_state.Type = SAHPI_CTRL_TYPE_DIGITAL;
		default_state.StateUnion.Digital = SAHPI_CTRL_STATE_OFF;
		default_rdr.Num = 1;
		default_rdr.OutputType = SAHPI_CTRL_LED;
		default_rdr.Type = SAHPI_CTRL_TYPE_DIGITAL;
		default_rdr.TypeUnion.Digital.Default = SAHPI_CTRL_STATE_OFF;
		default_rdr.DefaultMode.Mode = SAHPI_CTRL_MODE_AUTO;
		default_rdr.DefaultMode.ReadOnly = SAHPI_TRUE;
		default_rdr.WriteOnly = SAHPI_TRUE;
		default_rdr.Oem = 0;

		/* oh_valid_ctrl_state_mode: Normal testcase */
		mode = default_mode;
		state = default_state;
		rdr = default_rdr;
		expected_err = SA_OK;

		err = oh_valid_ctrl_state_mode(&rdr, mode, &state);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}
		
		/* oh_valid_ctrl_state_mode: Bad mode testcase */
		mode = BAD_TYPE;
		state = default_state;
		rdr = default_rdr;
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_valid_ctrl_state_mode(&rdr, mode, &state);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_ctrl_state_mode: Write read-only control testcase */
		mode = SAHPI_CTRL_MODE_MANUAL;
		state = default_state;
		rdr = default_rdr;
		expected_err = SA_ERR_HPI_READ_ONLY;

		err = oh_valid_ctrl_state_mode(&rdr, mode, &state);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_ctrl_state_mode: Bad analog testcase */
		mode = SAHPI_CTRL_MODE_MANUAL;
		state = default_state;
		rdr = default_rdr;
		state.Type = SAHPI_CTRL_TYPE_ANALOG;
		state.StateUnion.Analog = 50;
		rdr.DefaultMode.ReadOnly = SAHPI_FALSE;
		rdr.Type = SAHPI_CTRL_TYPE_ANALOG;
		rdr.TypeUnion.Analog.Max = 49;
		expected_err = SA_ERR_HPI_INVALID_DATA;

		err = oh_valid_ctrl_state_mode(&rdr, mode, &state);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_ctrl_state_mode: Bad stream testcase */
		mode = SAHPI_CTRL_MODE_MANUAL;
		state = default_state;
		rdr = default_rdr;
		state.Type = SAHPI_CTRL_TYPE_STREAM;
		state.StateUnion.Stream.StreamLength = SAHPI_CTRL_MAX_STREAM_LENGTH + 1;
		rdr.DefaultMode.ReadOnly = SAHPI_FALSE;
		rdr.Type = SAHPI_CTRL_TYPE_STREAM;
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_valid_ctrl_state_mode(&rdr, mode, &state);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_ctrl_state_mode: Bad overflow testcase */
		mode = SAHPI_CTRL_MODE_MANUAL;
		state = default_state;
		rdr = default_rdr;
		state.Type = SAHPI_CTRL_TYPE_TEXT;
		state.StateUnion.Text.Line = 10;
		state.StateUnion.Text.Text.DataType = SAHPI_TL_TYPE_TEXT;
		state.StateUnion.Text.Text.Language = SAHPI_LANG_ENGLISH;
		state.StateUnion.Text.Text.DataLength = 11;
		rdr.DefaultMode.ReadOnly = SAHPI_FALSE;
		rdr.Type = SAHPI_CTRL_TYPE_TEXT;
		rdr.TypeUnion.Text.MaxChars = 10;
		rdr.TypeUnion.Text.MaxLines = 10;
		rdr.TypeUnion.Text.Language = SAHPI_LANG_ENGLISH;
		rdr.TypeUnion.Text.DataType = SAHPI_TL_TYPE_TEXT;
		expected_err = SA_ERR_HPI_INVALID_DATA;

		err = oh_valid_ctrl_state_mode(&rdr, mode, &state);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_ctrl_state_mode: Bad overflow all lines testcase */
		mode = SAHPI_CTRL_MODE_MANUAL;
		state = default_state;
		rdr = default_rdr;
		state.Type = SAHPI_CTRL_TYPE_TEXT;
		state.StateUnion.Text.Line = SAHPI_TLN_ALL_LINES;
		state.StateUnion.Text.Text.DataType = SAHPI_TL_TYPE_TEXT;
		state.StateUnion.Text.Text.Language = SAHPI_LANG_ENGLISH;
		state.StateUnion.Text.Text.DataLength = 101;
		rdr.DefaultMode.ReadOnly = SAHPI_FALSE;
		rdr.Type = SAHPI_CTRL_TYPE_TEXT;
		rdr.TypeUnion.Text.MaxChars = 10;
		rdr.TypeUnion.Text.MaxLines = 10;
		rdr.TypeUnion.Text.Language = SAHPI_LANG_ENGLISH;
		rdr.TypeUnion.Text.DataType = SAHPI_TL_TYPE_TEXT;
		expected_err = SA_ERR_HPI_INVALID_DATA;

		err = oh_valid_ctrl_state_mode(&rdr, mode, &state);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}
	}

	return(0);
}
