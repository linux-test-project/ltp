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
#include "oSaHpiName.hpp"


/**
 * Default constructor.
 */
oSaHpiName::oSaHpiName() {
    Length = 0;
    Value[0] = '\0';
};


/**
 * Constructor.
 *
 * @param str    The zero-terminated character string to be assigned to the
 *               stream.
 */
oSaHpiName::oSaHpiName(const char *str) {
    if (strlen(str) < SA_HPI_MAX_NAME_LENGTH) {
        Length = strlen(str);
        strcpy((char *)Value, str);
    }
    else {
        Length = SA_HPI_MAX_NAME_LENGTH;
        memcpy(Value, str, SA_HPI_MAX_NAME_LENGTH);
    }
};


/**
 * Constructor.
 *
 * @param str    The data to be assigned to the stream.
 * @param len    The length of the data to be assigned to the stream.
 */
oSaHpiName::oSaHpiName(const void *str,
                       const SaHpiUint8T len) {
    Length = len;
    memcpy(Value, str, len);
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiName::oSaHpiName(const oSaHpiName& buf) {
    memcpy(this, &buf, sizeof(SaHpiNameT));
}



/**
 * Assign a field in the SaHpiNameT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiName::assignField(SaHpiNameT *ptr,
                             const char *field,
                             const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "Value") == 0) {
        if (strlen(value) < SA_HPI_MAX_NAME_LENGTH) {
            ptr->Length = strlen(value);
            strcpy((char *)ptr->Value, value);
        }
        else {
            ptr->Length = SA_HPI_MAX_NAME_LENGTH;
            memcpy(ptr->Value, value, SA_HPI_MAX_NAME_LENGTH);
        }
        return false;
    }
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiNameT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiName::fprint(FILE *stream,
                        const int indent,
                        const SaHpiNameT *buffer) {
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
    err = fprintf(stream, "Value = ");
    for (i = 0; i < buffer->Length; i++) {
        err = fprintf(stream, "%c\n", buffer->Value[i]);
        if (err < 0) {
            return true;
        }
    }
    err = fprintf(stream, "\n");
    if (err < 0) {
        return true;
    }

	return false;
}

