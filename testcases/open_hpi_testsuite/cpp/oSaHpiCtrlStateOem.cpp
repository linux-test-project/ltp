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
#include "oSaHpiCtrlStateOem.hpp"


/**
 * Default constructor.
 */
oSaHpiCtrlStateOem::oSaHpiCtrlStateOem() {
    MId = SAHPI_MANUFACTURER_ID_UNSPECIFIED;
    BodyLength = 0;
    Body[0] = '\0';
};


/**
 * Constructor.
 *
 * @param type   The manufacturer id.
 * @param str    The zero-terminated character string to be assigned to the
 *               stream.
 */
oSaHpiCtrlStateOem::oSaHpiCtrlStateOem(const SaHpiManufacturerIdT id,
                                       const char *str) {
    MId = id;
    if (strlen(str) < SAHPI_CTRL_MAX_OEM_BODY_LENGTH) {
        BodyLength = strlen(str);
        strcpy((char *)Body, str);
    }
    else {
        BodyLength = SAHPI_CTRL_MAX_OEM_BODY_LENGTH;
        memcpy(Body, str, SAHPI_CTRL_MAX_OEM_BODY_LENGTH);
    }
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiCtrlStateOem::oSaHpiCtrlStateOem(const oSaHpiCtrlStateOem& buf) {
    memcpy(this, &buf, sizeof(SaHpiCtrlStateOemT));
}



/**
 * Assign a field in the SaHpiCtrlStateOemT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlStateOem::assignField(SaHpiCtrlStateOemT *ptr,
                                     const char *field,
                                     const char *value) {
    // note that DataLength cannot be assigned a value using this method
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "MId") == 0) {
        ptr->MId = atoi(value);
        return false;
    }
    else if (strcmp(field, "Body") == 0) {
        if (strlen(value) < SAHPI_CTRL_MAX_OEM_BODY_LENGTH) {
            ptr->BodyLength = strlen(value);
            strcpy((char *)ptr->Body, value);
        }
        else {
            ptr->BodyLength = SAHPI_CTRL_MAX_OEM_BODY_LENGTH;
            memcpy(ptr->Body, value, SAHPI_CTRL_MAX_OEM_BODY_LENGTH);
        }
        return false;
    }
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiCtrlStateOemT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlStateOem::fprint(FILE *stream,
                                const int indent,
                                const SaHpiCtrlStateOemT *buffer) {
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
    err = fprintf(stream, "Body = ");
    for (i = 0; i < buffer->BodyLength; i++) {
        err = fprintf(stream, "%c\n", buffer->Body[i]);
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

