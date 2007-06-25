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
#include "oSaHpiCtrlDefaultMode.hpp"


/**
 * Default constructor.
 */
oSaHpiCtrlDefaultMode::oSaHpiCtrlDefaultMode() {
    Mode = SAHPI_CTRL_MODE_AUTO;
    ReadOnly = false;
};


/**
 * Constructor.
 */
oSaHpiCtrlDefaultMode::oSaHpiCtrlDefaultMode(SaHpiCtrlModeT mode,
                                             SaHpiBoolT ro) {
    Mode = mode;
    ReadOnly = ro;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiCtrlDefaultMode::oSaHpiCtrlDefaultMode(const oSaHpiCtrlDefaultMode& ent) {
    memcpy(this, &ent, sizeof(SaHpiCtrlDefaultModeT));
}


/**
 * Assign a field in the SaHpiCtrlDefaultModeT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlDefaultMode::assignField(SaHpiCtrlDefaultModeT *ptr,
                                        const char *field,
                                        const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "Mode") == 0) {
        ptr->Mode = oSaHpiTypesEnums::str2ctrlmode(value);
        return false;
    }
    else if (strcmp(field, "ReadOnly") == 0) {
        ptr->ReadOnly = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    return true;
};


/**
 * Print the contents of the entity.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiCtrlDefaultModeT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlDefaultMode::fprint(FILE *stream,
                                   const int indent,
                                   const SaHpiCtrlDefaultModeT *cdm) {
	int i, err = 0;
    char indent_buf[indent + 1];

    if (stream == NULL || cdm == NULL) {
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
    err = fprintf(stream, "Mode = %s\n", oSaHpiTypesEnums::ctrlmode2str(cdm->Mode));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ReadOnly = %s\n", oSaHpiTypesEnums::torf2str(cdm->ReadOnly));
    if (err < 0) {
        return true;
    }

	return false;
}

