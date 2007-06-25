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
#include "oSaHpiResourceInfo.hpp"


/**
 * Default constructor.
 */
oSaHpiResourceInfo::oSaHpiResourceInfo() {
    int i;

    ResourceRev = 0;
    SpecificVer = 0;
    DeviceSupport = 0;
    ManufacturerId = SAHPI_MANUFACTURER_ID_UNSPECIFIED;
    ProductId = 0;
    FirmwareMajorRev = 0;
    FirmwareMinorRev = 0;
    AuxFirmwareRev = 0;
    for (i = 0; i < 16; i++) {
        Guid[i] = 0;
    }
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiResourceInfo::oSaHpiResourceInfo(const oSaHpiResourceInfo& buf) {
    memcpy(this, &buf, sizeof(SaHpiResourceInfoT));
}



/**
 * Assign a field in the SaHpiResourceInfoT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiResourceInfo::assignField(SaHpiResourceInfoT *ptr,
                                     const char *field,
                                     const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "ResourceRev") == 0) {
        ptr->ResourceRev = (SaHpiUint8T)strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "SpecificVer") == 0) {
        ptr->SpecificVer = (SaHpiUint8T)strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "DeviceSupport") == 0) {
        ptr->DeviceSupport = (SaHpiUint8T)strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "ManufacturerId") == 0) {
        ptr->ManufacturerId = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "ProductId") == 0) {
        ptr->ProductId = (SaHpiUint8T)strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "FirmwareMajorRev") == 0) {
        ptr->FirmwareMajorRev = (SaHpiUint8T)strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "FirmwareMinorRev") == 0) {
        ptr->FirmwareMinorRev = (SaHpiUint8T)strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "AuxFirmwareRev") == 0) {
        ptr->AuxFirmwareRev = (SaHpiUint8T)strtoul(value, NULL, 10);
        return false;
    }
    // Guid
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiResourceInfoT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiResourceInfo::fprint(FILE *stream,
                             const int indent,
                             const SaHpiResourceInfoT *buffer) {
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
    err = fprintf(stream, "ResourceRev = %u\n", buffer->ResourceRev);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "SpecificVer = %u\n", buffer->SpecificVer);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "DeviceSupport = %u\n", buffer->DeviceSupport);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ManufacturerId = %u\n", buffer->ManufacturerId);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ProductId = %u\n", buffer->ProductId);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "FirmwareMajorRev = %u\n", buffer->FirmwareMajorRev);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "FirmwareMinorRev = %u\n", buffer->FirmwareMinorRev);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "AuxFirmwareRev = %u\n", buffer->AuxFirmwareRev);
    if (err < 0) {
        return true;
    }
    for (i = 0; i < 16; i++) {
        err = fprintf(stream, "%s", indent_buf);
        if (err < 0) {
            return true;
        }
        err = fprintf(stream, "Guid[%d] = %u\n", i, buffer->Guid[i]);
        if (err < 0) {
            return true;
        }
    }

	return false;
}

