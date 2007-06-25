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


int main(int argc, char *argv[]) {
    oSaHpiSensorReading *sr1, *sr2;

    // create the first text buffer
    sr1 = new oSaHpiSensorReading;
    if (sr1 == NULL) {
        printf("Error: Unable to create a oSaHpiSensorReading.\n");
        return -1;
    }

    // copy the first sensor reading to the second sensor reading
    sr2 = new oSaHpiSensorReading(*sr1);
    if (sr2 == NULL) {
        printf("Error: Unable to copy a oSaHpiSensorReading.\n");
        return -1;
    }

    // print the contents of the first sensor reading
    fprintf(stdout, "\nSaHpiSensorReading\n");
    if (sr1->fprint(stdout, 3)) {
        printf("Error: Unable to print the buffer.\n");
        return -1;

    }
    fprintf(stdout, "\n");

    // change the second sensor reading
    sr2->IsSupported = true;
    sr2->Type = SAHPI_SENSOR_READING_TYPE_BUFFER;
    strcpy((char *)sr2->Value.SensorBuffer, "My string");

    // print the contents of the second text buffer
    fprintf(stdout, "\nSaHpiSensorReading\n");
    if (sr2->fprint(stdout, 3)) {
        printf("Error: Unable to print the new buffer.\n");
        return -1;

    }
    fprintf(stdout, "\n");

    printf("Success!\n");
    return 0;
}

