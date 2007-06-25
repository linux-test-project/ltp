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


#ifndef Included_oSaHpiResourceInfo
#define Included_oSaHpiResourceInfo

#include <stdio.h>
extern "C"
{
#include <SaHpi.h>
}


class oSaHpiResourceInfo : public SaHpiResourceInfoT {
    public:
        // constructors
        oSaHpiResourceInfo();
        // copy constructor
        oSaHpiResourceInfo(const oSaHpiResourceInfo& buf);
        // destructor
        ~oSaHpiResourceInfo() {
        }
        // other methods
        static bool assignField(SaHpiResourceInfoT * ptr,
                                const char *field,
                                const char *value);
        inline bool assignField(const char *field,
                                const char *value) {
            return assignField(this, field, value);
        }
        inline SaHpiResourceInfoT *getStruct(void) {
            return this;
        }
        static bool fprint(FILE *stream,
                           const int indent,
                           const SaHpiResourceInfoT *buffer);
        inline bool fprint(FILE *stream,
                           const int indent) {
            return fprint(stream, indent, this);
        }
};

#endif

