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
#include "oSaHpiIdrInfo.hpp"


/**
 * Default constructor.
 */
oSaHpiIdrInfo::oSaHpiIdrInfo() {
    IdrId = 1;
    UpdateCount = 0;
    ReadOnly = false;
    NumAreas = 0;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiIdrInfo::oSaHpiIdrInfo(const oSaHpiIdrInfo& buf) {
    memcpy(this, &buf, sizeof(SaHpiIdrInfoT));
}



/**
 * Assign a field in the SaHpiIdrInfoT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiIdrInfo::assignField(SaHpiIdrInfoT *ptr,
                                const char *field,
                                const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "IdrId") == 0) {
        ptr->IdrId = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "UpdateCount") == 0) {
        ptr->UpdateCount = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "ReadOnly") == 0) {
        ptr->ReadOnly = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    else if (strcmp(field, "NumAreas") == 0) {
        ptr->NumAreas = strtoul(value, NULL, 10);
        return false;
    }
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiIdrInfoT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiIdrInfo::fprint(FILE *stream,
                           const int indent,
                           const SaHpiIdrInfoT *buffer) {
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
    err = fprintf(stream, "IdrId = %d\n", buffer->IdrId);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "UpdateCount = %d\n", buffer->UpdateCount);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ReadOnly = %s\n", oSaHpiTypesEnums::torf2str(buffer->ReadOnly));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "NumAreas = %d\n", buffer->NumAreas);
    if (err < 0) {
        return true;
    }

	return false;
}

