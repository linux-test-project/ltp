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
#include "oSaHpiCtrlRecDigital.hpp"
#include "oSaHpiCtrlRecDiscrete.hpp"
#include "oSaHpiCtrlRecAnalog.hpp"
#include "oSaHpiCtrlRecStream.hpp"
#include "oSaHpiCtrlRecText.hpp"
#include "oSaHpiCtrlRecOem.hpp"
#include "oSaHpiCtrlRec.hpp"


/**
 * Default constructor.
 */
oSaHpiCtrlRec::oSaHpiCtrlRec() {
    Num = 1;
    OutputType = SAHPI_CTRL_GENERIC;
    Type = SAHPI_CTRL_TYPE_DIGITAL;
    TypeUnion.Digital.Default = SAHPI_CTRL_STATE_OFF;
    DefaultMode.Mode = SAHPI_CTRL_MODE_AUTO;
    DefaultMode.ReadOnly = false;
    WriteOnly = false;
    Oem = 0;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiCtrlRec::oSaHpiCtrlRec(const oSaHpiCtrlRec& cr) {
    memcpy(this, &cr, sizeof(SaHpiCtrlRecT));
}


/**
 * Assign a field in the SaHpiCtrlRecT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlRec::assignField(SaHpiCtrlRecT *ptr,
                                const char *field,
                                const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "Num") == 0) {
        ptr->Num = strtoul(value, NULL, 10);
        return false;
    }
    else if (strcmp(field, "OutputType") == 0) {
        ptr->OutputType = oSaHpiTypesEnums::str2ctrloutputtype(value);
        return false;
    }
    else if (strcmp(field, "Type") == 0) {
        ptr->Type = oSaHpiTypesEnums::str2ctrltype(value);
        return false;
    }
    // Digital
    // Discrete
    // Analog
    // Stream
    // Text
    //Oem
    else if (strcmp(field, "WriteOnly") == 0) {
        ptr->WriteOnly = oSaHpiTypesEnums::str2torf(value);
        return false;
    }
    else if (strcmp(field, "Oem") == 0) {
        ptr->Oem = strtoul(value, NULL, 10);
        return false;
    }
    return true;
};


/**
 * Print the contents of the entity.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiCtrlRecT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlRec::fprint(FILE *stream,
                           const int indent,
                           const SaHpiCtrlRecT *cr) {
	int i, err = 0;
    char indent_buf[indent + 1];

    if (stream == NULL) {
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
    err = fprintf(stream, "Num = %d\n", cr->Num);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "OutputType = %s\n", oSaHpiTypesEnums::ctrloutputtype2str(cr->OutputType));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Type = %s\n", oSaHpiTypesEnums::ctrltype2str(cr->Type));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    switch (cr->Type) {
    case SAHPI_CTRL_TYPE_DIGITAL: {
        err = fprintf(stream, "TypeUnion.Digital\n");
        if (err < 0) {
            return true;
        }
        const SaHpiCtrlRecDigitalT *crd = (const SaHpiCtrlRecDigitalT *)&cr->TypeUnion.Digital;
        err = oSaHpiCtrlRecDigital::fprint(stream, indent + 3, crd);
        if (err < 0) {
            return true;
        }
        break;
    }
    case SAHPI_CTRL_TYPE_DISCRETE: {
        err = fprintf(stream, "TypeUnion.Discrete\n");
        if (err < 0) {
            return true;
        }
        const SaHpiCtrlRecDiscreteT *crds = (const SaHpiCtrlRecDiscreteT *)&cr->TypeUnion.Discrete;
        err = oSaHpiCtrlRecDiscrete::fprint(stream, indent + 3, crds);
        if (err < 0) {
            return true;
        }
        break;
    }
    case SAHPI_CTRL_TYPE_ANALOG: {
        err = fprintf(stream, "TypeUnion.Analog\n");
        if (err < 0) {
            return true;
        }
        const SaHpiCtrlRecAnalogT *cra = (const SaHpiCtrlRecAnalogT *)&cr->TypeUnion.Analog;
        err = oSaHpiCtrlRecAnalog::fprint(stream, indent + 3, cra);
        if (err < 0) {
            return true;
        }
        break;
    }
    case SAHPI_CTRL_TYPE_STREAM: {
        err = fprintf(stream, "TypeUnion.Stream\n");
        if (err < 0) {
            return true;
        }
        const SaHpiCtrlRecStreamT *crs = (const SaHpiCtrlRecStreamT *)&cr->TypeUnion.Stream;
        err = oSaHpiCtrlRecStream::fprint(stream, indent + 3, crs);
        if (err < 0) {
            return true;
        }
        break;
    }
    case SAHPI_CTRL_TYPE_TEXT: {
        err = fprintf(stream, "TypeUnion.Text\n");
        if (err < 0) {
            return true;
        }
        const SaHpiCtrlRecTextT *crt = (const SaHpiCtrlRecTextT *)&cr->TypeUnion.Text;
        err = oSaHpiCtrlRecText::fprint(stream, indent + 3, crt);
        if (err < 0) {
            return true;
        }
        break;
    }
    case SAHPI_CTRL_TYPE_OEM: {
        err = fprintf(stream, "TypeUnion.Oem\n");
        if (err < 0) {
            return true;
        }
        const SaHpiCtrlRecOemT *cro = (const SaHpiCtrlRecOemT *)&cr->TypeUnion.Oem;
        err = oSaHpiCtrlRecOem::fprint(stream, indent + 3, cro);
        if (err < 0) {
            return true;
        }
        break;
    }
    default:
        break;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "DefaultMode\n");
    if (err < 0) {
        return true;
    }
    const SaHpiCtrlDefaultModeT *dm = (const SaHpiCtrlDefaultModeT *)&cr->DefaultMode;
    err = oSaHpiCtrlDefaultMode::fprint(stream, indent + 3, dm);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "WriteOnly = %s\n", oSaHpiTypesEnums::torf2str(cr->WriteOnly));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "Oem = %d\n", cr->Oem);
    if (err < 0) {
        return true;
    }

	return false;
}

