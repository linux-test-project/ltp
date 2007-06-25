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
#include "oSaHpiTypesEnums.hpp"
#include "oSaHpiSensorEnableChangeEvent.hpp"


/**
 * Default constructor.
 */
oSaHpiSensorEnableChangeEvent::oSaHpiSensorEnableChangeEvent() {
    SensorNum = 1;
    SensorType = SAHPI_TEMPERATURE;
    EventCategory = SAHPI_EC_UNSPECIFIED;
    SensorEnable = false;
    SensorEventEnable = false;
    AssertEventMask = SAHPI_ES_UNSPECIFIED;
    DeassertEventMask = SAHPI_ES_UNSPECIFIED;
    OptionalDataPresent = (SaHpiSensorOptionalDataT)0;
    CurrentState = SAHPI_ES_UNSPECIFIED;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiSensorEnableChangeEvent::oSaHpiSensorEnableChangeEvent(const oSaHpiSensorEnableChangeEvent& range) {
    memcpy(this, &range, sizeof(SaHpiSensorEnableChangeEventT));
}


/**
 * Assign a field in the SaHpiSensorEnableChangeEventT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiSensorEnableChangeEvent::assignField(SaHpiSensorEnableChangeEventT *ptr,
                                                const char *field,
                                                const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "SensorNum") == 0) {
        ptr->SensorNum = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "SensorType") == 0) {
        ptr->SensorType = oSaHpiTypesEnums::str2sensortype(value);
        return false;
    }
    else if (strcmp(field, "EventCategory") == 0) {
        ptr->EventCategory |= oSaHpiTypesEnums::str2eventcategory(value);
        return false;
    }
    else if (strcmp(field, "SensorEnable") == 0) {
        ptr->SensorEnable = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    else if (strcmp(field, "SensorEventEnable") == 0) {
        ptr->SensorEventEnable = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    else if (strcmp(field, "AssertEventMask") == 0) {
        ptr->AssertEventMask |= oSaHpiTypesEnums::str2eventstate(value);
        return false;
    }
    else if (strcmp(field, "DeassertEventMask") == 0) {
        ptr->DeassertEventMask |= oSaHpiTypesEnums::str2eventstate(value);
        return false;
    }
    else if (strcmp(field, "OptionalDataPresent") == 0) {
        ptr->OptionalDataPresent |= oSaHpiTypesEnums::str2sensoroptionaldata(value);
        return false;
    }
    else if (strcmp(field, "CurrentState") == 0) {
        ptr->CurrentState |= oSaHpiTypesEnums::str2eventstate(value);
        return false;
    }
    return true;
};


/**
 * Print the contents of the entity.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiSensorEnableChangeEventT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiSensorEnableChangeEvent::fprint(FILE *stream,
                                           const int indent,
                                           const SaHpiSensorEnableChangeEventT *se) {
	int i, err = 0;
    char indent_buf[indent + 1];

    if (stream == NULL || se == NULL) {
        return true;
    }
    for (i = 0; i < indent; i++) {
        indent_buf[i] = ' ';
    }
    indent_buf[indent] = '\0';

    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "SensorNum = %u\n", se->SensorNum);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "SensorType = %s\n", oSaHpiTypesEnums::sensortype2str(se->SensorType));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "EventCategory = %X\n", se->EventCategory);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "SensorEnable = %s\n", oSaHpiTypesEnums::torf2str(se->SensorEnable));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "SensorEventEnable = %s\n", oSaHpiTypesEnums::torf2str(se->SensorEventEnable));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "AssertEventMask = %X\n", se->AssertEventMask);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "DeassertEventMask = %X\n", se->DeassertEventMask);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "OptionalDataPresent = %X\n", se->OptionalDataPresent);
    if (err < 0) {
        return true;
    }
    if (se->OptionalDataPresent && SAHPI_SOD_CURRENT_STATE) {
        err = fprintf(stream, "%s", indent_buf);
        if (err < 0) {
            return true;
        }
        err = fprintf(stream, "CurrentState = %X\n", se->CurrentState);
        if (err < 0) {
            return true;
        }
    }

	return false;
}

