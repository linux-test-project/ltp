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
#include "oSaHpiSensorReading.hpp"
#include "oSaHpiSensorRange.hpp"
#include "oSaHpiSensorDataFormat.hpp"


/**
 * Default constructor.
 */
oSaHpiSensorDataFormat::oSaHpiSensorDataFormat() {
    oSaHpiSensorReading *sr;

    IsSupported = 0;
    ReadingType = SAHPI_SENSOR_READING_TYPE_INT64;
    BaseUnits = SAHPI_SU_UNSPECIFIED;
    ModifierUnits = SAHPI_SU_UNSPECIFIED;
    ModifierUse = SAHPI_SMUU_NONE;
    Percentage = false;
    Range.Flags = 0;
    sr = (oSaHpiSensorReading *)&Range.Max;
    sr->initSensorReading(sr);
    sr = (oSaHpiSensorReading *)&Range.Min;
    sr->initSensorReading(sr);
    sr = (oSaHpiSensorReading *)&Range.Nominal;
    sr->initSensorReading(sr);
    sr = (oSaHpiSensorReading *)&Range.NormalMax;
    sr->initSensorReading(sr);
    sr = (oSaHpiSensorReading *)&Range.NormalMin;
    sr->initSensorReading(sr);
    AccuracyFactor = 0;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiSensorDataFormat::oSaHpiSensorDataFormat(const oSaHpiSensorDataFormat& df) {
    memcpy(this, &df, sizeof(SaHpiSensorDataFormatT));
}


/**
 * Assign a field in the SaHpiSensorDataFormatT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiSensorDataFormat::assignField(SaHpiSensorDataFormatT *ptr,
                                         const char *field,
                                         const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "IsSupported") == 0) {
        ptr->IsSupported = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    else if (strcmp(field, "ReadingType") == 0) {
        ptr->ReadingType = oSaHpiTypesEnums::str2sensorreadingtype(value);
        return false;
    }
    else if (strcmp(field, "BaseUnits") == 0) {
        ptr->BaseUnits = oSaHpiTypesEnums::str2sensorunits(value);
        return false;
    }
    else if (strcmp(field, "ModifierUnits") == 0) {
        ptr->ModifierUnits = oSaHpiTypesEnums::str2sensorunits(value);
        return false;
    }
    else if (strcmp(field, "ModifierUse") == 0) {
        ptr->ModifierUse = oSaHpiTypesEnums::str2sensoruse(value);
        return false;
    }
    else if (strcmp(field, "Percentage") == 0) {
        ptr->Percentage = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    else if (strcmp(field, "AccuracyFactor") == 0) {
        ptr->AccuracyFactor = (SaHpiFloat64T)atof(value);
        return false;
    }
    return true;
};


/**
 * Print the contents of the entity.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiSensorReadingT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiSensorDataFormat::fprint(FILE *stream,
                                    const int indent,
                                    const SaHpiSensorDataFormatT *df) {
	int i, err = 0;
    char indent_buf[indent + 1];

    if (stream == NULL || df == NULL) {
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
    err = fprintf(stream, "IsSupported = %s\n", oSaHpiTypesEnums::torf2str(df->IsSupported));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ReadingType = %s\n", oSaHpiTypesEnums::sensorreadingtype2str(df->ReadingType));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "BaseUnits = %s\n", oSaHpiTypesEnums::sensorunits2str(df->BaseUnits));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ModifierUnits = %s\n", oSaHpiTypesEnums::sensorunits2str(df->ModifierUnits));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ModifierUse = %s\n", oSaHpiTypesEnums::sensoruse2str(df->ModifierUse));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Percentage = %s\n", oSaHpiTypesEnums::torf2str(df->Percentage));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Range\n");
    if (err < 0) {
        return true;
    }
    const SaHpiSensorRangeT *sr = (const SaHpiSensorRangeT *)&df->Range;
    err = oSaHpiSensorRange::fprint(stream, indent + 3, sr);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "AccuracyFactor = %f\n", df->AccuracyFactor);
    if (err < 0) {
        return true;
    }

	return false;
}

