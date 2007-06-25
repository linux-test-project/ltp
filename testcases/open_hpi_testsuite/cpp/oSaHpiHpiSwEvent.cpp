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
#include "oSaHpiHpiSwEvent.hpp"


/**
 * Default constructor.
 */
oSaHpiHpiSwEvent::oSaHpiHpiSwEvent() {
    MId = SAHPI_MANUFACTURER_ID_UNSPECIFIED;
    Type = SAHPI_HPIE_AUDIT;
    EventData.DataType = SAHPI_TL_TYPE_TEXT;
    EventData.Language = SAHPI_LANG_ENGLISH;
    EventData.DataLength = 0;
    EventData.Data[0] = '\0';
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiHpiSwEvent::oSaHpiHpiSwEvent(const oSaHpiHpiSwEvent& buf) {
    memcpy(this, &buf, sizeof(SaHpiHpiSwEventT));
}



/**
 * Assign a field in the SaHpiHpiSwEventT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiHpiSwEvent::assignField(SaHpiHpiSwEventT *ptr,
                                   const char *field,
                                   const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "MId") == 0) {
        ptr->MId = atoi(value);
        return false;
    }
    else if (strcmp(field, "Type") == 0) {
        ptr->Type = oSaHpiTypesEnums::str2sweventtype(value);
        return false;
    }
    // EventData
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiHpiSwEventT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiHpiSwEvent::fprint(FILE *stream,
                              const int indent,
                              const SaHpiHpiSwEventT *buffer) {
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
    err = fprintf(stream, "Type = %s\n", oSaHpiTypesEnums::sweventtype2str(buffer->Type));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "EventData\n");
    if (err < 0) {
        return true;
    }
    const SaHpiTextBufferT *tb = (const SaHpiTextBufferT *)&buffer->EventData;
    err = oSaHpiTextBuffer::fprint(stream, indent + 3, tb);
    if (err < 0) {
        return true;
    }

	return false;
}

