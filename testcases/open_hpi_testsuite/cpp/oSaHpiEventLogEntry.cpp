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
#include "oSaHpiEvent.hpp"
#include "oSaHpiEventLogEntry.hpp"


/**
 * Default constructor.
 */
oSaHpiEventLogEntry::oSaHpiEventLogEntry() {
    EntryId = 1;
    Timestamp = 0;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiEventLogEntry::oSaHpiEventLogEntry(const oSaHpiEventLogEntry& buf) {
    memcpy(this, &buf, sizeof(SaHpiEventLogEntryT));
}



/**
 * Assign a field in the SaHpiEventLogEntryT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiEventLogEntry::assignField(SaHpiEventLogEntryT *ptr,
                                      const char *field,
                                      const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "EntryId") == 0) {
        ptr->EntryId = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "Timestamp") == 0) {
        ptr->Timestamp = strtoull(value, NULL, 10);
        return false;
    }
    // Event
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiEventLogEntryT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiEventLogEntry::fprint(FILE *stream,
                                 const int indent,
                                 const SaHpiEventLogEntryT *buffer) {
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
    err = fprintf(stream, "EntryId = %u\n", buffer->EntryId);
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
    err = fprintf(stream, "Event\n");
    if (err < 0) {
        return true;
    }
    const SaHpiEventT *e = (const SaHpiEventT *)&buffer->Event;
    err = oSaHpiEvent::fprint(stream, indent + 3, e);
    if (err < 0) {
        return true;
    }

	return false;
}

