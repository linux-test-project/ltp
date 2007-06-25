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
#include "oSaHpiTextBuffer.hpp"
#include "oSaHpiOemEvent.hpp"


/**
 * Default constructor.
 */
oSaHpiOemEvent::oSaHpiOemEvent() {
    MId = SAHPI_MANUFACTURER_ID_UNSPECIFIED;
    OemEventData.DataType = SAHPI_TL_TYPE_TEXT;
    OemEventData.Language = SAHPI_LANG_ENGLISH;
    OemEventData.DataLength = 0;
    OemEventData.Data[0] = '\0';
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiOemEvent::oSaHpiOemEvent(const oSaHpiOemEvent& buf) {
    memcpy(this, &buf, sizeof(SaHpiOemEventT));
}



/**
 * Assign a field in the SaHpiOemEventT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiOemEvent::assignField(SaHpiOemEventT *ptr,
                                 const char *field,
                                 const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "MId") == 0) {
        ptr->MId = strtoul(value, NULL, 10);
        return false;
    }
    // OemEventData
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiOemEventT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiOemEvent::fprint(FILE *stream,
                            const int indent,
                            const SaHpiOemEventT *buffer) {
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
    err = fprintf(stream, "MId = %d\n", buffer->MId);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "OemEventData\n");
    if (err < 0) {
        return true;
    }
    const SaHpiTextBufferT *tb = (const SaHpiTextBufferT *)&buffer->OemEventData;
    err = oSaHpiTextBuffer::fprint(stream, indent + 3, tb);
    if (err < 0) {
        return true;
    }

	return false;
}

