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
#include "oSaHpiCtrlStateText.hpp"
#include "oSaHpiCtrlRecText.hpp"


/**
 * Default constructor.
 */
oSaHpiCtrlRecText::oSaHpiCtrlRecText() {
    MaxChars = SAHPI_MAX_TEXT_BUFFER_LENGTH;
    MaxLines = 1;
    Language = SAHPI_LANG_ENGLISH;
    DataType = SAHPI_TL_TYPE_TEXT;
    Default.Line = 1;
    Default.Text.Language = SAHPI_LANG_ENGLISH;
    Default.Text.DataType = SAHPI_TL_TYPE_TEXT;
    Default.Text.DataLength = 0;
    Default.Text.Data[0] = '\0';
};


/**
 * Constructor.
 */
oSaHpiCtrlRecText::oSaHpiCtrlRecText(SaHpiUint8T maxch,
                                     SaHpiUint8T maxlin,
                                     SaHpiLanguageT lang,
                                     SaHpiTextTypeT  type,
                                     const char *str) {
    MaxChars = maxch;
    MaxLines = maxlin;
    Language = lang;
    DataType = type;
    Default.Line = 1;
    Default.Text.Language = lang;
    Default.Text.DataType = type;
    if (strlen(str) < MaxChars) {
        Default.Text.DataLength = strlen(str);
        strcpy((char *)Default.Text.Data, str);
    }
    else {
        Default.Text.DataLength = MaxChars;
        memcpy(Default.Text.Data, str, MaxChars);
    }
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiCtrlRecText::oSaHpiCtrlRecText(const oSaHpiCtrlRecText& ent) {
    memcpy(this, &ent, sizeof(SaHpiCtrlRecTextT));
}


/**
 * Assign a field in the SaHpiCtrlRecTextT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlRecText::assignField(SaHpiCtrlRecTextT *ptr,
                                    const char *field,
                                    const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "MaxChars") == 0) {
        ptr->MaxChars = (SaHpiUint8T)atoi(value);
        return false;
    }
    else if (strcmp(field, "MaxLines") == 0) {
        ptr->MaxLines = (SaHpiUint8T)atoi(value);
        return false;
    }
    else if (strcmp(field, "Language") == 0) {
        ptr->Language = oSaHpiTypesEnums::str2language(value);
        return false;
    }
    else if (strcmp(field, "DataType") == 0) {
        ptr->DataType = oSaHpiTypesEnums::str2texttype(value);
        return false;
    }
    // Default
    return true;
};


/**
 * Print the contents of the entity.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiCtrlRecTextT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlRecText::fprint(FILE *stream,
                               const int indent,
                               const SaHpiCtrlRecTextT *txt) {
	int i, err = 0;
    char indent_buf[indent + 1];

    if (stream == NULL || txt == NULL) {
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
    err = fprintf(stream, "MaxChars = %u\n", txt->MaxChars);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "MaxLines = %u\n", txt->MaxLines);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Language = %s\n", oSaHpiTypesEnums::language2str(txt->Language));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "DataType = %s\n", oSaHpiTypesEnums::texttype2str(txt->DataType));
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
    const SaHpiCtrlStateTextT *cs = (const SaHpiCtrlStateTextT *)&txt->Default;
    err = oSaHpiCtrlStateText::fprint(stream, indent + 3, cs);
    if (err < 0) {
        return true;
    }

	return false;
}

