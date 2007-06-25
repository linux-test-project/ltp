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
#include "oSaHpiCtrlStateText.hpp"


/**
 * Default constructor.
 */
oSaHpiCtrlStateText::oSaHpiCtrlStateText() {
    Line = 0;
    Text.DataType = SAHPI_TL_TYPE_TEXT;
    Text.Language = SAHPI_LANG_ENGLISH;
    Text.DataLength = 0;
    Text.Data[0] = '\0';
};


/**
 * Constructor.
 *
 * @param str    The zero-terminated character string to be assigned to the
 *               text filed.
 */
oSaHpiCtrlStateText::oSaHpiCtrlStateText(const char *str) {
    Line = 0;
    Text.DataType = SAHPI_TL_TYPE_TEXT;
    Text.Language = SAHPI_LANG_ENGLISH;
    if (strlen(str) < SAHPI_CTRL_MAX_STREAM_LENGTH) {
        Text.DataLength = strlen(str);
        strcpy((char *)Text.Data, str);
    }
    else {
        Text.DataLength = SAHPI_CTRL_MAX_STREAM_LENGTH;
        memcpy(Text.Data, str, SAHPI_CTRL_MAX_STREAM_LENGTH);
    }
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiCtrlStateText::oSaHpiCtrlStateText(const oSaHpiCtrlStateText& buf) {
    memcpy(this, &buf, sizeof(SaHpiCtrlStateTextT));
}



/**
 * Assign a field in the SaHpiCtrlStateTextT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlStateText::assignField(SaHpiCtrlStateTextT *ptr,
                                      const char *field,
                                      const char *value) {
    // note that DataLength cannot be assigned a value using this method
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "Line") == 0) {
        ptr->Line = (SaHpiUint8T)atoi(value);
        return false;
    }
    // Text
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiCtrlStateTextT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlStateText::fprint(FILE *stream,
                                 const int indent,
                                 const SaHpiCtrlStateTextT *buffer) {
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
    err = fprintf(stream, "Line = %d\n", buffer->Line);

    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Text\n");
    const SaHpiTextBufferT *tb = (const SaHpiTextBufferT *)&buffer->Text;
    err = oSaHpiTextBuffer::fprint(stream, indent + 3, tb);
    if (err < 0) {
        return true;
    }

	return false;
}

