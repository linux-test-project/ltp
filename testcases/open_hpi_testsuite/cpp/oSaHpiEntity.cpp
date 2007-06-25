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
#include "oSaHpiEntity.hpp"


/**
 * Default constructor.
 */
oSaHpiEntity::oSaHpiEntity() {
    EntityType = SAHPI_ENT_ROOT;
    EntityLocation = 0;
};


/**
 * Constructor.
 *
 * @param type   The SaHpiEntityTypeT.
 * @param loc    The SaHpiEntityLocationT.
 */
oSaHpiEntity::oSaHpiEntity(const SaHpiEntityTypeT type,
                           const SaHpiEntityLocationT loc) {
    if (oSaHpiTypesEnums::entitytype2str(type)) {
        EntityType = type;
    }
    else {
        EntityType = SAHPI_ENT_ROOT;
    }
    EntityLocation = loc;
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiEntity::oSaHpiEntity(const oSaHpiEntity& ent) {
    EntityType = ent.EntityType;
    EntityLocation = ent.EntityLocation;
}


/**
 * Assign a field in the SaHpiEntityT struct a value.
 *
 * @param field  The pointer to the struct (class).
 * @param field  The field name as a text string (case sensitive).
 * @param value  The character string value to be assigned to the field. This
 *               value will be converted as necessary.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiEntity::assignField(SaHpiEntityT *ptr,
                               const char *field,
                               const char *value) {
    if (ptr == NULL || field == NULL || value == NULL) {
        return true;
    }
    if (strcmp(field, "EntityType") == 0) {
        ptr->EntityType = oSaHpiTypesEnums::str2entitytype(value);
        return false;
    }
    else if (strcmp(field, "EntityLocation") == 0) {
        ptr->EntityLocation = strtoul(value, NULL, 10);
        return false;
    }
    return true;
};


/**
 * Print the contents of the entity.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiEntityT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiEntity::fprint(FILE *stream,
                          const int indent,
                          const SaHpiEntityT *ent) {
	int i, err = 0;
    char indent_buf[indent + 1];

    if (stream == NULL || ent == NULL) {
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
    err = fprintf(stream, "EntityType = %s\n", oSaHpiTypesEnums::entitytype2str(ent->EntityType));
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "%s", indent_buf);
    if (err < 0) {
        return true;
    }
    err = fprintf(stream, "EntityLocation = %u\n", ent->EntityLocation);
 	if (err < 0) {
   		return true;
   	}

	return false;
}

