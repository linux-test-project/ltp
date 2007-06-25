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
#include "oSaHpiCtrlStateStream.hpp"


/**
 * Default constructor.
 */
oSaHpiCtrlStateStream::oSaHpiCtrlStateStream() {
    Repeat = false;
    StreamLength = 0;
    Stream[0] = '\0';
};


/**
 * Constructor.
 *
 * @param type   The repeat boolean.
 * @param str    The zero-terminated character string to be assigned to the
 *               stream.
 */
oSaHpiCtrlStateStream::oSaHpiCtrlStateStream(const SaHpiBoolT rep,
                                             const char *str) {
    Repeat = rep;
    if (strlen(str) < SAHPI_CTRL_MAX_STREAM_LENGTH) {
        StreamLength = strlen(str);
        strcpy((char *)Stream, str);
    }
    else {
        StreamLength = SAHPI_CTRL_MAX_STREAM_LENGTH;
        memcpy(Stream, str, SAHPI_CTRL_MAX_STREAM_LENGTH);
    }
};


/**
 * Constructor.
 *
 * @param type   The repeat boolean.
 * @param str    The data to be assigned to the stream.
 * @param len    The length of the data to be assigned to the stream.
 */
oSaHpiCtrlStateStream::oSaHpiCtrlStateStream(const SaHpiBoolT rep,
                                             const void *str,
                                             const SaHpiUint8T len) {
    Repeat = rep;
    if (len <= SAHPI_CTRL_MAX_STREAM_LENGTH) {
        StreamLength = len;
        memcpy(Stream, str, len);
    }
    else {
        StreamLength = SAHPI_CTRL_MAX_STREAM_LENGTH;
        memcpy(Stream, str, SAHPI_CTRL_MAX_STREAM_LENGTH);
    }
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiCtrlStateStream::oSaHpiCtrlStateStream(const oSaHpiCtrlStateStream& buf) {
    memcpy(this, &buf, sizeof(SaHpiCtrlStateStreamT));
}



/**
 * Assign a field in the SaHpiCtrlStateStreamT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlStateStream::assignField(SaHpiCtrlStateStreamT *ptr,
                                        const char *field,
                                        const char *value) {
    // note that DataLength cannot be assigned a value using this method
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "Repeat") == 0) {
        ptr->Repeat = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    else if (strcmp(field, "Stream") == 0) {
        if (strlen(value) < SAHPI_CTRL_MAX_STREAM_LENGTH) {
            ptr->StreamLength = strlen(value);
            strcpy((char *)ptr->Stream, value);
        }
        else {
            ptr->StreamLength = SAHPI_CTRL_MAX_STREAM_LENGTH;
            memcpy(ptr->Stream, value, SAHPI_CTRL_MAX_STREAM_LENGTH);
        }
        return false;
    }
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiCtrlStateStreamT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlStateStream::fprint(FILE *stream,
                                   const int indent,
                                   const SaHpiCtrlStateStreamT *buffer) {
	unsigned int i;
	int err;
    char indent_buf[indent + 1];

    if (stream == NULL || buffer == NULL) {
        return true;
    }
    for (i = 0; i < (unsigned int)indent; i++) {
        indent_buf[i] = ' ';
    }
    indent_buf[indent] = '\0';

    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Repeat = %s\n", oSaHpiTypesEnums::torf2str(buffer->Repeat));

    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Stream = ");
    for (i = 0; i < buffer->StreamLength; i++) {
        err = fprintf(stream, "%c\n", buffer->Stream[i]);
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

