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
#include "oSaHpiCtrlRecOem.hpp"


/**
 * Default constructor.
 */
oSaHpiCtrlRecOem::oSaHpiCtrlRecOem() {
    MId = SAHPI_MANUFACTURER_ID_UNSPECIFIED;
    ConfigData[0] = '\0';
    Default.MId = SAHPI_MANUFACTURER_ID_UNSPECIFIED;
    Default.BodyLength = 0;
    Default.Body[0] = '\0';
};


/**
 * Constructor.
 */
oSaHpiCtrlRecOem::oSaHpiCtrlRecOem(SaHpiManufacturerIdT mid,
                                   const char *config,
                                   const char *str) {
    MId = mid;
    if (strlen(config) < SAHPI_CTRL_OEM_CONFIG_LENGTH) {
        strcpy((char *)ConfigData, config);
    }
    else {
        memcpy(ConfigData, config, SAHPI_CTRL_OEM_CONFIG_LENGTH);
    }
    Default.MId = mid;
    if (strlen(str) < SAHPI_CTRL_MAX_OEM_BODY_LENGTH) {
        Default.BodyLength = strlen(str);
        strcpy((char *)ConfigData, str);
    }
    else {
        Default.BodyLength = SAHPI_CTRL_MAX_OEM_BODY_LENGTH;
        memcpy(ConfigData, str, SAHPI_CTRL_MAX_OEM_BODY_LENGTH);
    }
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiCtrlRecOem::oSaHpiCtrlRecOem(const oSaHpiCtrlRecOem& ent) {
    memcpy(this, &ent, sizeof(SaHpiCtrlRecOemT));
}


/**
 * Assign a field in the SaHpiCtrlRecOemT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlRecOem::assignField(SaHpiCtrlRecOemT *ptr,
                                   const char *field,
                                   const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "MId") == 0) {
        ptr->MId = (SaHpiManufacturerIdT)atoi(value);
        return false;
    }
    else if (strcmp(field, "ConfigData") == 0) {
        if (strlen(value) < SAHPI_CTRL_OEM_CONFIG_LENGTH) {
            strcpy((char *)ptr->ConfigData, value);
        }
        else {
            memcpy(ptr->ConfigData, value, SAHPI_CTRL_OEM_CONFIG_LENGTH);
        }
        return false;
    }
    // Default
    return true;
};


/**
 * Print the contents of the entity.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiCtrlRecOemT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlRecOem::fprint(FILE *stream,
                              const int indent,
                              const SaHpiCtrlRecOemT *oem) {
	int i, err = 0;
    char indent_buf[indent + 1];

    if (stream == NULL || oem == NULL) {
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
    err = fprintf(stream, "MId = %d\n", oem->MId);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ConfigData = %s\n", oem->ConfigData);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Default\n");
    if (err < 0) {
        return true;
    }
    const SaHpiCtrlStateOemT *cs = (const SaHpiCtrlStateOemT *)&oem->Default;
    err = oSaHpiCtrlStateOem::fprint(stream, indent + 3, cs);
    if (err < 0) {
        return true;
    }

	return false;
}

