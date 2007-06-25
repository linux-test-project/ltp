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
#include "oSaHpiAnnouncement.hpp"


/**
 * Default constructor.
 */
oSaHpiAnnouncement::oSaHpiAnnouncement() {
    EntryId = 1;
    Timestamp = SAHPI_TIME_UNSPECIFIED;
    AddedByUser = false;
    Severity = SAHPI_OK;
    Acknowledged = false;
    StatusCond.Type = SAHPI_STATUS_COND_TYPE_SENSOR;
    StatusCond.Entity.Entry[0].EntityType = SAHPI_ENT_ROOT;
    StatusCond.Entity.Entry[0].EntityLocation = 0;
    StatusCond.DomainId = SAHPI_UNSPECIFIED_DOMAIN_ID;
    StatusCond.ResourceId = 1;
    StatusCond.SensorNum = 1;
    StatusCond.EventState = SAHPI_ES_UNSPECIFIED;
    StatusCond.Name.Length = 0;
    StatusCond.Name.Value[0] = '\0';
    StatusCond.Mid = SAHPI_MANUFACTURER_ID_UNSPECIFIED;
    StatusCond.Data.DataType = SAHPI_TL_TYPE_TEXT;
    StatusCond.Data.Language = SAHPI_LANG_ENGLISH;
    StatusCond.Data.DataLength = 0;
    StatusCond.Data.Data[0] = '\0';
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiAnnouncement::oSaHpiAnnouncement(const oSaHpiAnnouncement& buf) {
    memcpy(this, &buf, sizeof(SaHpiAnnouncementT));
}



/**
 * Assign a field in the SaHpiAnnouncementT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiAnnouncement::assignField(SaHpiAnnouncementT *ptr,
                                     const char *field,
                                     const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "EntryId") == 0) {
        ptr->EntryId = atoi(value);
        return false;
    }
    else if (strcmp(field, "Timestamp") == 0) {
        ptr->Timestamp = strtoll(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "AddedByUser") == 0) {
        ptr->AddedByUser = oSaHpiTypesEnums::str2torf(value);
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
    // StatusCond
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiAnnouncementT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiAnnouncement::fprint(FILE *stream,
                                const int indent,
                                const SaHpiAnnouncementT *buffer) {
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
    err = fprintf(stream, "EntryId = %d\n", buffer->EntryId);
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
    err = fprintf(stream, "AddedByUsed = %s\n", oSaHpiTypesEnums::torf2str(buffer->AddedByUser));
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
    err = fprintf(stream, "StatusCond\n");
    if (err < 0) {
        return true;
    }
    const SaHpiConditionT * c = (const SaHpiConditionT *)&buffer->StatusCond;
    err = oSaHpiCondition::fprint(stream, indent + 3, c);
    if (err < 0) {
        return true;
    }

	return false;
}

