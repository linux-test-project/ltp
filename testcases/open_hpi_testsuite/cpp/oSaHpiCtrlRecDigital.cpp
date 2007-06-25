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
#include "oSaHpiCtrlRecDigital.hpp"


/**
 * Default constructor.
 */
oSaHpiCtrlRecDigital::oSaHpiCtrlRecDigital() {
    Default = SAHPI_CTRL_STATE_OFF;
};


/**
 * Constructor.
 */
oSaHpiCtrlRecDigital::oSaHpiCtrlRecDigital(SaHpiCtrlStateDigitalT cs) {
    Default = cs;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiCtrlRecDigital::oSaHpiCtrlRecDigital(const oSaHpiCtrlRecDigital& ent) {
    memcpy(this, &ent, sizeof(SaHpiCtrlRecDigitalT));
}


/**
 * Assign a field in the SaHpiCtrlRecDigitalT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlRecDigital::assignField(SaHpiCtrlRecDigitalT *ptr,
                                       const char *field,
                                       const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "Default") == 0) {
        ptr->Default = oSaHpiTypesEnums::str2ctrlstatedigital(value);
        return false;
    }
    // StateUnion
    return true;
};


/**
 * Print the contents of the entity.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiCtrlRecDigitalT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlRecDigital::fprint(FILE *stream,
                                  const int indent,
                                  const SaHpiCtrlRecDigitalT *crd) {
	int i, err = 0;
    char indent_buf[indent + 1];

    if (stream == NULL || crd == NULL) {
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
    err = fprintf(stream, "Default = %s\n", oSaHpiTypesEnums::ctrlstatedigital2str(crd->Default));
    if (err < 0) {
        return true;
    }

	return false;
}

