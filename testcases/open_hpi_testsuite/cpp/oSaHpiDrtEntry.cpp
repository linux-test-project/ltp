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
#include "oSaHpiDrtEntry.hpp"


/**
 * Default constructor.
 */
oSaHpiDrtEntry::oSaHpiDrtEntry() {
    EntryId = 1;
    DomainId = 1;
    IsPeer = false;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiDrtEntry::oSaHpiDrtEntry(const oSaHpiDrtEntry& buf) {
    memcpy(this, &buf, sizeof(SaHpiDrtEntryT));
}



/**
 * Assign a field in the SaHpiDrtEntryT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiDrtEntry::assignField(SaHpiDrtEntryT *ptr,
                                 const char *field,
                                 const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "EntryId") == 0) {
        ptr->EntryId = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "DomainId") == 0) {
        ptr->DomainId = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "IsPeer") == 0) {
        ptr->IsPeer = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiDrtEntryT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiDrtEntry::fprint(FILE *stream,
                            const int indent,
                            const SaHpiDrtEntryT *buffer) {
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
    err = fprintf(stream, "DomainId = %u\n", buffer->DomainId);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "IsPeer = %s\n", oSaHpiTypesEnums::torf2str(buffer->IsPeer));
    if (err < 0) {
        return true;
    }

	return false;
}

