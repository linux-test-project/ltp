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
#include "oSaHpiCtrlRecStream.hpp"


/**
 * Default constructor.
 */
oSaHpiCtrlRecStream::oSaHpiCtrlRecStream() {
    Default.Repeat = false;
    Default.StreamLength = 0;
    Default.Stream[0] = '\0';
};


/**
 * Constructor.
 *
 * @param buf    The reference to the class to be copied.
 */
oSaHpiCtrlRecStream::oSaHpiCtrlRecStream(const oSaHpiCtrlRecStream& ent) {
    memcpy(this, &ent, sizeof(SaHpiCtrlRecStreamT));
}


/**
 * Print the contents of the entity.
 *
 * @param stream Target stream.
 * @param buffer Address of the SaHpiCtrlRecStreamT struct.
 *
 * @return True if there was an error, otherwise false.
 */
bool oSaHpiCtrlRecStream::fprint(FILE *stream,
                                 const int indent,
                                 const SaHpiCtrlRecStreamT *strm) {
	int i, err = 0;
    char indent_buf[indent + 1];

    if (stream == NULL || strm == NULL) {
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
    err = fprintf(stream, "Default\n");
    if (err < 0) {
        return true;
    }
    const SaHpiCtrlStateStreamT *css = (const SaHpiCtrlStateStreamT *)&strm->Default;
    err = oSaHpiCtrlStateStream::fprint(stream, indent + 3, css);
    if (err < 0) {
        return true;
    }

	return false;
}

