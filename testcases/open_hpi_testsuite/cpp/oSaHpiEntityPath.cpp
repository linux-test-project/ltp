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
#include "oSaHpiEntity.hpp"
#include "oSaHpiEntityPath.hpp"


/**
 * Default constructor.
 */
oSaHpiEntityPath::oSaHpiEntityPath() {
    int i;

    for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i ++) {
        Entry[i].EntityType = SAHPI_ENT_ROOT;
        Entry[i].EntityLocation = 0;
    }
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiEntityPath::oSaHpiEntityPath(const oSaHpiEntityPath& entpath) {
    int i;

    for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i ++) {
        Entry[i].EntityType = entpath.Entry[i].EntityType;
        Entry[i].EntityLocation = entpath.Entry[i].EntityLocation;
    }
}


/**
 * Print the contents of the entity path.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiEntityPathT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiEntityPath::fprint(FILE *stream,
                              const int indent,
                              const SaHpiEntityPathT *entpath) {
	int i, err = 0;
    char indent_buf[indent + 1];
    const SaHpiEntityT *ent;

    if (stream == NULL || entpath == NULL) {
        return true;
    }
    for (i = 0; i < indent; i++) {
        indent_buf[i] = ' ';
    }
    indent_buf[indent] = '\0';

    for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i++) {
        err = fprintf(stream, "%s", indent_buf);
        if (err < 0) {
            return true;
        }
        err = fprintf(stream, "Entry[%d]\n", i);
        if (err < 0) {
            return true;
        }
        ent = (const SaHpiEntityT *)&(entpath->Entry[i]);
        err = oSaHpiEntity::fprint(stream, indent + 3, ent);
        if (err < 0) {
            return true;
        }
        if (entpath->Entry[i].EntityType == SAHPI_ENT_ROOT) {
            // no need to print past the root
            break;
        }
    }

	return false;
}


/**
 * Append one entity path to another.
 *
 * @param destpath   The destination entity path.
 * @param appendpath The entity path to be appended to the destpath. If the result
 *                   entity path is too large it will be truncated.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiEntityPath::append(SaHpiEntityPathT *destpath,
                              const SaHpiEntityPathT *appendpath) {
    int i, j;

    if (!destpath) {
        return true;
    }
    if (!appendpath) {
        return false;
    }

    for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i++) {
        if (destpath->Entry[i].EntityType == SAHPI_ENT_ROOT)
            break;
    }

    for (j = 0; i < SAHPI_MAX_ENTITY_PATH; i++) {
        destpath->Entry[i].EntityLocation = appendpath->Entry[j].EntityLocation;
        destpath->Entry[i].EntityType = appendpath->Entry[j].EntityType;
        if (appendpath->Entry[j].EntityType == SAHPI_ENT_ROOT)
            break;
        j++;
    }

    return false;
}


/**
 * Compare one entity path to another.
 *
 * @param destpath   The destination entity path.
 * @param appendpath The entity path to be appended to the destpath. If the result
 *                   entity path is too large it will be truncated.
 *
 * @return True if the two entity paths are equal, otherwise false.
 */
bool oSaHpiEntityPath::compare(const SaHpiEntityPathT *ep1,
                               const SaHpiEntityPathT *ep2) {
    unsigned int i, j;

    if (!ep1 || !ep2) {
        return false;
    }

    for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i++) {
        if (ep1->Entry[i].EntityType == SAHPI_ENT_ROOT) {
            i++;
            break;
        }
    }
    for (j = 0; j < SAHPI_MAX_ENTITY_PATH; j++) {
        if (ep2->Entry[j].EntityType == SAHPI_ENT_ROOT) {
            j++;
            break;
        }
    }
    if (i != j)
        return false;

    for (i = 0; i < j; i++) {
        if (ep1->Entry[i].EntityType != ep2->Entry[i].EntityType ||
         ep1->Entry[i].EntityLocation != ep2->Entry[i].EntityLocation) {
            return false;
        }
    }

    return true;
}

