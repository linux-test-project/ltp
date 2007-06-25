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
#include "oSaHpiDomainInfo.hpp"


/**
 * Default constructor.
 */
oSaHpiDomainInfo::oSaHpiDomainInfo() {
    int i;

    DomainId = 1;
    DomainCapabilities = (SaHpiDomainCapabilitiesT)0;
    IsPeer = false;
    DomainTag.DataType = SAHPI_TL_TYPE_TEXT;
    DomainTag.Language = SAHPI_LANG_ENGLISH;
    DomainTag.DataLength = 0;
    DomainTag.Data[0] = '\0';
    DrtUpdateCount = 0;
    DrtUpdateTimestamp = 0;
    RptUpdateCount = 0;
    RptUpdateTimestamp = 0;
    DatUpdateCount = 0;
    DatUpdateTimestamp = 0;
    ActiveAlarms = 0;
    CriticalAlarms = 0;
    MajorAlarms = 0;
    MinorAlarms = 0;
    DatUserAlarmLimit = 0;
    DatOverflow = false;
    for (i = 0; i < 16; i++) {
        Guid[i] = 0;
    }
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiDomainInfo::oSaHpiDomainInfo(const oSaHpiDomainInfo& buf) {
    memcpy(this, &buf, sizeof(SaHpiDomainInfoT));
}



/**
 * Assign a field in the SaHpiDomainInfoT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiDomainInfo::assignField(SaHpiDomainInfoT *ptr,
                                   const char *field,
                                   const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "DomainId") == 0) {
        ptr->DomainId = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "DomainCapabilities") == 0) {
        if (strcmp(value, "SAHPI_DOMAIN_CAP_AUTOINSERT_READ_ONLY") == 0) {
            ptr->DomainCapabilities = SAHPI_DOMAIN_CAP_AUTOINSERT_READ_ONLY;
        }
        else {
            ptr->DomainCapabilities = (SaHpiDomainCapabilitiesT)0;
        }
        return false;
    }
    else if (strcmp(field, "IsPeer") == 0) {
        ptr->IsPeer = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    // DomainTag
    else if (strcmp(field, "DrtUpdateCount") == 0) {
        ptr->DrtUpdateCount = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "DrtUpdateTimestamp") == 0) {
        ptr->DrtUpdateTimestamp = strtoull(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "RptUpdateCount") == 0) {
        ptr->RptUpdateCount = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "RptUpdateTimestamp") == 0) {
        ptr->RptUpdateTimestamp = strtoull(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "DatUpdateCount") == 0) {
        ptr->DatUpdateCount = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "DatUpdateTimestamp") == 0) {
        ptr->DatUpdateTimestamp = strtoull(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "ActiveAlarms") == 0) {
        ptr->ActiveAlarms = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "CriticalAlarms") == 0) {
        ptr->CriticalAlarms = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "MajorAlarms") == 0) {
        ptr->MajorAlarms = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "MinorAlarms") == 0) {
        ptr->MinorAlarms = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "DatUserAlarmLimit") == 0) {
        ptr->DatUserAlarmLimit = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "DatOverflow") == 0) {
        ptr->DatOverflow = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    // Guid
    return true;
};


/**
 * Print the contents of the buffer.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiDomainInfoT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiDomainInfo::fprint(FILE *stream,
                              const int indent,
                              const SaHpiDomainInfoT *buffer) {
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
    err = fprintf(stream, "DomainId = %u\n", buffer->DomainId);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "DomainCapabilities = %X\n", buffer->DomainCapabilities);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "IsPeer = %s\n", oSaHpiTypesEnums::torf2str(buffer->IsPeer));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "DomainTag\n");
    if (err < 0) {
        return true;
    }
    const SaHpiTextBufferT *tb = (const SaHpiTextBufferT *)&buffer->DomainTag;
    err = oSaHpiTextBuffer::fprint(stream, indent + 3, tb);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "DrtUpdateCount = %u\n", buffer->DrtUpdateCount);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "DrtUpdateTimestamp = %lld\n", buffer->DrtUpdateTimestamp);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "RptUpdateCount = %u\n", buffer->RptUpdateCount);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "RptUpdateTimestamp = %lld\n", buffer->RptUpdateTimestamp);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "DatUpdateCount = %u\n", buffer->DatUpdateCount);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "DatUpdateTimestamp = %lld\n", buffer->DatUpdateTimestamp);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "ActiveAlarms = %u\n", buffer->ActiveAlarms);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "CriticalAlarms = %u\n", buffer->CriticalAlarms);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "MajorAlarms = %u\n", buffer->MajorAlarms);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "MinorAlarms = %u\n", buffer->MinorAlarms);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "DatUserAlarmLimit = %u\n", buffer->DatUserAlarmLimit);
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

