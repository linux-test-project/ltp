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
#include "oSaHpiEntityPath.hpp"
#include "oSaHpiTextBuffer.hpp"
#include "oSaHpiRptEntry.hpp"


/**
 * Default constructor.
 */
oSaHpiRptEntry::oSaHpiRptEntry() {
    int i;

    EntryId = 1;
    ResourceId = 1;
    ResourceInfo.ResourceRev = 0;
    ResourceInfo.SpecificVer = 0;
    ResourceInfo.DeviceSupport = 0;
    ResourceInfo.ManufacturerId = SAHPI_MANUFACTURER_ID_UNSPECIFIED;
    ResourceInfo.ProductId = 0;
    ResourceInfo.FirmwareMajorRev = 0;
    ResourceInfo.FirmwareMinorRev = 0;
    ResourceInfo.AuxFirmwareRev = 0;
    for (i = 0; i < 16; i++) {
        ResourceInfo.Guid[i] = 0;
    }
    ResourceEntity.Entry[0].EntityType = SAHPI_ENT_ROOT;
    ResourceEntity.Entry[0].EntityLocation = 0;
    ResourceCapabilities = (SaHpiCapabilitiesT)0;
    HotSwapCapabilities = (SaHpiHsCapabilitiesT)0;
    ResourceSeverity = SAHPI_OK;
    ResourceFailed = false;
    ResourceTag.DataType = SAHPI_TL_TYPE_TEXT;
    ResourceTag.Language = SAHPI_LANG_ENGLISH;
    ResourceTag.DataLength = 0;
    ResourceTag.Data[0] = '\0';
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiRptEntry::oSaHpiRptEntry(const oSaHpiRptEntry& buf) {
    memcpy(this, &buf, sizeof(SaHpiRptEntryT));
}



/**
 * Assign a field in the SaHpiRptEntryT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiRptEntry::assignField(SaHpiRptEntryT *ptr,
                                 const char *field,
                                 const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "EntryId") == 0) {
        ptr->EntryId = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "ResourceId") == 0) {
        ptr->ResourceId = strtoul(value, NULL, 10);
        return false;
    }
    // ResourceInfo
    // ResourceEntity
    else if (strcmp(field, "ResourceCapabilities") == 0) {
        ptr->ResourceCapabilities |= oSaHpiTypesEnums::str2capabilities(value);
        return false;
    }
    else if (strcmp(field, "HotSwapCapabilities") == 0) {
        ptr->HotSwapCapabilities |= oSaHpiTypesEnums::str2hscapabilities(value);
        return false;
    }
    else if (strcmp(field, "ResourceSeverity") == 0) {
        ptr->ResourceSeverity = oSaHpiTypesEnums::str2severity(value);
        return false;
    }
    else if (strcmp(field, "ResourceFailed") == 0) {
        ptr->ResourceFailed = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    // ResourceTag
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiRptEntryT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiRptEntry::fprint(FILE *stream,
                            const int indent,
                            const SaHpiRptEntryT *buffer) {
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
    err = fprintf(stream, "EntryId = %d\n", buffer->EntryId);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ResourceId = %d\n", buffer->ResourceId);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ResourceInfo\n");
    if (err < 0) {
        return true;
    }
    const SaHpiResourceInfoT *ri = (const SaHpiResourceInfoT *)&buffer->ResourceInfo;
    err = oSaHpiResourceInfo::fprint(stream, indent + 3, ri);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ResourceEntity\n");
    if (err < 0) {
        return true;
    }
    const SaHpiEntityPathT *ep = (const SaHpiEntityPathT *)&buffer->ResourceEntity;
    err = oSaHpiEntityPath::fprint(stream, indent + 3, ep);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ResourceCapabilities = %X\n", buffer->ResourceCapabilities);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "HotSwapCapabilities = %X\n", buffer->HotSwapCapabilities);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ResourceSeverity = %s\n", oSaHpiTypesEnums::severity2str(buffer->ResourceSeverity));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ResourceTag\n");
    if (err < 0) {
        return true;
    }
    const SaHpiTextBufferT *tb = (const SaHpiTextBufferT *)&buffer->ResourceTag;
    err = oSaHpiTextBuffer::fprint(stream, indent + 3, tb);
    if (err < 0) {
        return true;
    }

	return false;
}

