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


#ifndef Included_oSaHpiRptEntry
#define Included_oSaHpiRptEntry

#include <stdio.h>
extern "C"
{
#include <SaHpi.h>
}


class oSaHpiRptEntry : public SaHpiRptEntryT {
    public:
        // constructors
        oSaHpiRptEntry();
        // copy constructor
        oSaHpiRptEntry(const oSaHpiRptEntry& buf);
        // destructor
        ~oSaHpiRptEntry() {
        }
        // other methods
        static bool assignField(SaHpiRptEntryT * ptr,
                                const char *field,
                                const char *value);
        inline bool assignField(const char *field,
                                const char *value) {
            return assignField(this, field, value);
        }
        inline SaHpiRptEntryT *getStruct(void) {
            return this;
        }
        static bool fprint(FILE *stream,
                           const int indent,
                           const SaHpiRptEntryT *buffer);
        inline bool fprint(FILE *stream,
                           const int indent) {
            return fprint(stream, indent, this);
        }
};

#endif

