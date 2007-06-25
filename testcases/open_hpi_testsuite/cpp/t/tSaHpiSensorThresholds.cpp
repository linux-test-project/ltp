/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *    W. David Ashley <dashley@us.ibm.com>
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
extern "C"
{
#include <SaHpi.h>
}
#include "oSaHpiSensorReading.hpp"
#include "oSaHpiSensorThresholds.hpp"


int main(int argc, char *argv[]) {
    oSaHpiSensorThresholds *th;

    // create the sensor thresholds
    th = new oSaHpiSensorThresholds;
    if (th == NULL) {
        printf("Error: Unable to create a oSaHpiSensorThresholds.\n");
        return -1;
    }

    // print the contents of the sensor thresholds
    fprintf(stdout, "\nSaHpiSensorThresholds\n");
    if (th->fprint(stdout, 3)) {
        printf("Error: Unable to print the sensor thresholds.\n");
        return -1;

    }
    fprintf(stdout, "\n");

    // set the LowCritical sensor threshold
    th->LowCritical.IsSupported = true;
    th->LowCritical.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
    th->LowCritical.Value.SensorFloat64 = -21;

    // print the contents of the sensor thresholds
    fprintf(stdout, "\nSaHpiSensorThresholds\n");
    if (th->fprint(stdout, 3)) {
        printf("Error: Unable to print the sensor thresholds.\n");
        return -1;

    }
    fprintf(stdout, "\n");

    printf("Success!\n");
    return 0;
}

