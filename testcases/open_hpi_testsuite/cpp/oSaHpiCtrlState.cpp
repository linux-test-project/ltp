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
#include "oSaHpiCtrlStateStream.hpp"
#include "oSaHpiCtrlStateText.hpp"
#include "oSaHpiCtrlStateOem.hpp"
#include "oSaHpiCtrlState.hpp"


/**
 * Default constructor.
 */
oSaHpiCtrlState::oSaHpiCtrlState() {
    Type = SAHPI_CTRL_TYPE_DIGITAL;
    StateUnion.Digital = SAHPI_CTRL_STATE_OFF;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiCtrlState::oSaHpiCtrlState(const oSaHpiCtrlState& ent) {
    memcpy(this, &ent, sizeof(SaHpiCtrlStateT));
}


/**
 * Assign a field in the SaHpiCtrlStateT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlState::assignField(SaHpiCtrlStateT *ptr,
                                  const char *field,
                                  const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "Type") == 0) {
        ptr->Type = oSaHpiTypesEnums::str2ctrltype(value);
        return false;
    }
    // StateUnion
    return true;
};


/**
 * Print the contents of the entity.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiCtrlStateT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlState::fprint(FILE *stream,
                             const int indent,
                             const SaHpiCtrlStateT *cs) {
	int i, err = 0;
    char indent_buf[indent + 1];

    if (stream == NULL || cs == NULL) {
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
    err = fprintf(stream, "Type = %s\n", oSaHpiTypesEnums::ctrltype2str(cs->Type));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    switch (cs->Type) {
    case SAHPI_CTRL_TYPE_DIGITAL: {
        err = fprintf(stream, "StateUnion.Digital = %s\n", oSaHpiTypesEnums::ctrlstatedigital2str(cs->StateUnion.Digital));
        if (err < 0) {
            return true;
        }
        break;
    }
    case SAHPI_CTRL_TYPE_DISCRETE: {
        err = fprintf(stream, "StateUnion.Discrete = %u\n", cs->StateUnion.Discrete);
        if (err < 0) {
            return true;
        }
        break;
    }
    case SAHPI_CTRL_TYPE_ANALOG: {
        err = fprintf(stream, "StateUnion.Analog = %d\n", cs->StateUnion.Analog);
        if (err < 0) {
            return true;
        }
        break;
    }
    case SAHPI_CTRL_TYPE_STREAM: {
        err = fprintf(stream, "StateUnion.Stream\n");
        const SaHpiCtrlStateStreamT *css = (const SaHpiCtrlStateStreamT *)&cs->StateUnion.Stream;
        err = oSaHpiCtrlStateStream::fprint(stream, indent + 3, css);
        if (err < 0) {
            return true;
        }
        break;
    }
    case SAHPI_CTRL_TYPE_TEXT: {
        err = fprintf(stream, "StateUnion.Text\n");
        const SaHpiCtrlStateTextT *cst = (const SaHpiCtrlStateTextT *)&cs->StateUnion.Text;
        err = oSaHpiCtrlStateText::fprint(stream, indent + 3, cst);
        if (err < 0) {
            return true;
        }
        break;
    }
    case SAHPI_CTRL_TYPE_OEM: {
        err = fprintf(stream, "StateUnion.Oem\n");
        const SaHpiCtrlStateOemT *cso = (const SaHpiCtrlStateOemT *)&cs->StateUnion.Oem;
        err = oSaHpiCtrlStateOem::fprint(stream, indent + 3, cso);
        if (err < 0) {
            return true;
        }
        break;
    }
    default:
        err = fprintf(stream, "StateUnion = Unknown\n");
        if (err < 0) {
            return true;
        }
        break;
    }

	return false;
}

