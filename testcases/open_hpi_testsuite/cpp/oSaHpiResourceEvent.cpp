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
#include "oSaHpiResourceEvent.hpp"


/**
 * Default constructor.
 */
oSaHpiResourceEvent::oSaHpiResourceEvent() {
    ResourceEventType = SAHPI_RESE_RESOURCE_FAILURE;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiResourceEvent::oSaHpiResourceEvent(const oSaHpiResourceEvent& buf) {
    memcpy(this, &buf, sizeof(SaHpiResourceEventT));
}



/**
 * Assign a field in the SaHpiResourceEventT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiResourceEvent::assignField(SaHpiResourceEventT *ptr,
                                      const char *field,
                                      const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "ResourceEventType") == 0) {
        ptr->ResourceEventType = oSaHpiTypesEnums::str2resourceeventtype(value);
        return false;
    }
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiResourceEventT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiResourceEvent::fprint(FILE *stream,
                                 const int indent,
                                 const SaHpiResourceEventT *buffer) {
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
    err = fprintf(stream, "ResourceEventType = %s\n", oSaHpiTypesEnums::resourceeventtype2str(buffer->ResourceEventType));
    if (err < 0) {
        return true;
    }

	return false;
}

