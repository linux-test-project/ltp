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
#include "oSaHpiCtrlRecAnalog.hpp"


/**
 * Default constructor.
 */
oSaHpiCtrlRecAnalog::oSaHpiCtrlRecAnalog() {
    Min = 0;
    Max = 0;
    Default = 0;
};


/**
 * Constructor.
 */
oSaHpiCtrlRecAnalog::oSaHpiCtrlRecAnalog(SaHpiCtrlStateAnalogT mn,
                                         SaHpiCtrlStateAnalogT mx,
                                         SaHpiCtrlStateAnalogT def) {
    Min = mn;
    Max = mx;
    Default = def;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiCtrlRecAnalog::oSaHpiCtrlRecAnalog(const oSaHpiCtrlRecAnalog& ent) {
    memcpy(this, &ent, sizeof(SaHpiCtrlRecAnalogT));
}


/**
 * Assign a field in the SaHpiCtrlRecAnalogT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlRecAnalog::assignField(SaHpiCtrlRecAnalogT *ptr,
                                      const char *field,
                                      const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "Min") == 0) {
        ptr->Min = atoi(value);
        return false;
    }
    else if (strcmp(field, "Max") == 0) {
        ptr->Max = atoi(value);
        return false;
    }
    else if (strcmp(field, "Default") == 0) {
        ptr->Default = atoi(value);
        return false;
    }
    return true;
};


/**
 * Print the contents of the entity.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiCtrlRecAnalogT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlRecAnalog::fprint(FILE *stream,
                                 const int indent,
                                 const SaHpiCtrlRecAnalogT *cra) {
	int i, err = 0;
    char indent_buf[indent + 1];

    if (stream == NULL || cra == NULL) {
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
    err = fprintf(stream, "Min = %d\n", cra->Min);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Max = %d\n", cra->Max);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Default = %d\n", cra->Default);
    if (err < 0) {
        return true;
    }

	return false;
}

