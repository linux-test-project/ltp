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
#include "oSaHpiCondition.hpp"
#include "oSaHpiAlarm.hpp"


/**
 * Default constructor.
 */
oSaHpiAlarm::oSaHpiAlarm() {
    AlarmId = 1;
    Timestamp = 0;
    Severity = SAHPI_OK;
    Acknowledged = false;
    AlarmCond.Type = SAHPI_STATUS_COND_TYPE_SENSOR;
    AlarmCond.Entity.Entry[0].EntityType = SAHPI_ENT_ROOT;
    AlarmCond.Entity.Entry[0].EntityLocation = 0;
    AlarmCond.DomainId = SAHPI_UNSPECIFIED_DOMAIN_ID;
    AlarmCond.ResourceId = 1;
    AlarmCond.SensorNum = 1;
    AlarmCond.EventState = SAHPI_ES_UNSPECIFIED;
    AlarmCond.Name.Length = 0;
    AlarmCond.Name.Value[0] = '\0';
    AlarmCond.Mid = SAHPI_MANUFACTURER_ID_UNSPECIFIED;
    AlarmCond.Data.DataType = SAHPI_TL_TYPE_TEXT;
    AlarmCond.Data.Language = SAHPI_LANG_ENGLISH;
    AlarmCond.Data.DataLength = 0;
    AlarmCond.Data.Data[0] = '\0';
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiAlarm::oSaHpiAlarm(const oSaHpiAlarm& buf) {
    memcpy(this, &buf, sizeof(SaHpiAlarmT));
}



/**
 * Assign a field in the SaHpiAlarmT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiAlarm::assignField(SaHpiAlarmT *ptr,
                              const char *field,
                              const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "AlarmId") == 0) {
        ptr->AlarmId = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "Timestamp") == 0) {
        ptr->Timestamp = strtoull(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "Severity") == 0) {
        ptr->Severity = oSaHpiTypesEnums::str2severity(value);
        return false;
    }
    else if (strcmp(field, "Acknowledged") == 0) {
        ptr->Acknowledged = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    // AlarmCond
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiAlarmT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiAlarm::fprint(FILE *stream,
                         const int indent,
                         const SaHpiAlarmT *buffer) {
	int i, err;
    char indent_buf[indent + 1];

    if (stream == NULL || buffer == NULL) {
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
    err = fprintf(stream, "AlarmId = %u\n", buffer->AlarmId);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Timestamp = %lld\n", buffer->Timestamp);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Severity = %s\n", oSaHpiTypesEnums::severity2str(buffer->Severity));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Acknowledged = %s\n", oSaHpiTypesEnums::torf2str(buffer->Acknowledged));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "AlarmCond\n");
    if (err < 0) {
        return true;
    }
    const SaHpiConditionT *c = &buffer->AlarmCond;
    err = oSaHpiCondition::fprint(stream, indent + 3, c);
    if (err < 0) {
        return true;
    }

	return false;
}

