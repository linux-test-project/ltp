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
#include "oSaHpiEventLogInfo.hpp"


/**
 * Default constructor.
 */
oSaHpiEventLogInfo::oSaHpiEventLogInfo() {
    Entries = 0;
    Size = 0;
    UserEventMaxSize = 0;
    UpdateTimestamp = 0;
    CurrentTime = 0;
    Enabled = false;
    OverflowFlag = false;
    OverflowResetable = false;
    OverflowAction = SAHPI_EL_OVERFLOW_DROP;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiEventLogInfo::oSaHpiEventLogInfo(const oSaHpiEventLogInfo& buf) {
    memcpy(this, &buf, sizeof(SaHpiEventLogInfoT));
}



/**
 * Assign a field in the SaHpiEventLogInfoT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiEventLogInfo::assignField(SaHpiEventLogInfoT *ptr,
                                     const char *field,
                                     const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "Entries") == 0) {
        ptr->Entries = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "Size") == 0) {
        ptr->Size = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "UserEventMaxSize") == 0) {
        ptr->UserEventMaxSize = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "UpdateTimestamp") == 0) {
        ptr->UpdateTimestamp = strtoull(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "CurrentTime") == 0) {
        ptr->CurrentTime = strtoull(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "Enabled") == 0) {
        ptr->Enabled = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    else if (strcmp(field, "OverflowFlag") == 0) {
        ptr->OverflowFlag = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    else if (strcmp(field, "OverflowResetable") == 0) {
        ptr->OverflowResetable = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    else if (strcmp(field, "OverflowAction") == 0) {
        ptr->OverflowAction = oSaHpiTypesEnums::str2eventlogoverflowaction(value);
        return false;
    }
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiEventLogInfoT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiEventLogInfo::fprint(FILE *stream,
                                const int indent,
                                const SaHpiEventLogInfoT *buffer) {
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
    err = fprintf(stream, "Entries = %u\n", buffer->Entries);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Size = %u\n", buffer->Size);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "UserEventMaxSize = %u\n", buffer->UserEventMaxSize);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "UpdateTimestamp = %lld\n", buffer->UpdateTimestamp);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "CurrentTime = %lld\n", buffer->CurrentTime);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Enabled = %s\n", oSaHpiTypesEnums::torf2str(buffer->Enabled));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "OverflowFlag = %s\n", oSaHpiTypesEnums::torf2str(buffer->OverflowFlag));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "OverflowResetable = %s\n", oSaHpiTypesEnums::torf2str(buffer->OverflowResetable));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "OverflowAction = %s\n", oSaHpiTypesEnums::eventlogoverflowaction2str(buffer->OverflowAction));
    if (err < 0) {
        return true;
    }

	return false;
}

