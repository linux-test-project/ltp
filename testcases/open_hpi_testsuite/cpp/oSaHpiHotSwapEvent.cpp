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
#include "oSaHpiHotSwapEvent.hpp"


/**
 * Default constructor.
 */
oSaHpiHotSwapEvent::oSaHpiHotSwapEvent() {
    HotSwapState = SAHPI_HS_STATE_ACTIVE;
    PreviousHotSwapState = SAHPI_HS_STATE_NOT_PRESENT;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiHotSwapEvent::oSaHpiHotSwapEvent(const oSaHpiHotSwapEvent& range) {
    memcpy(this, &range, sizeof(SaHpiHotSwapEventT));
}


/**
 * Assign a field in the SaHpiHotSwapEventT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiHotSwapEvent::assignField(SaHpiHotSwapEventT *ptr,
                                     const char *field,
                                     const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "HotSwapState") == 0) {
        ptr->HotSwapState = oSaHpiTypesEnums::str2hsstate(value);
        return false;
    }
    else if (strcmp(field, "PreviousHotSwapState") == 0) {
        ptr->PreviousHotSwapState = oSaHpiTypesEnums::str2hsstate(value);
        return false;
    }
    return true;
};


/**
 * Print the contents of the entity.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiHotSwapEventT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiHotSwapEvent::fprint(FILE *stream,
                                const int indent,
                                const SaHpiHotSwapEventT *hse) {
	int i, err = 0;
    char indent_buf[indent + 1];

    if (stream == NULL || hse == NULL) {
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
    err = fprintf(stream, "HotSwapState = %s\n", oSaHpiTypesEnums::hsstate2str(hse->HotSwapState));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "PreviousHotSwapState = %s\n", oSaHpiTypesEnums::hsstate2str(hse->PreviousHotSwapState));
    if (err < 0) {
        return true;
    }

	return false;
}

