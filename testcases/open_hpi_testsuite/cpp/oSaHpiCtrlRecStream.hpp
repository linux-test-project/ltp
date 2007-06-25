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


#ifndef Included_oSaHpiCtrlRecStream
#define Included_oSaHpiCtrlRecStream

#include <stdio.h>
extern "C"
{
#include <SaHpi.h>
}


class oSaHpiCtrlRecStream : public SaHpiCtrlRecStreamT {
    public:
        // constructors
        oSaHpiCtrlRecStream();
        // copy constructor
        oSaHpiCtrlRecStream(const oSaHpiCtrlRecStream& crd);
        // destructor
        ~oSaHpiCtrlRecStream() {
        }
        // other methods
        inline SaHpiCtrlRecStreamT *getStruct(void) {
            return this;
        }
        static bool fprint(FILE *stream,
                           const int indent,
                           const SaHpiCtrlRecStreamT *ent);
        inline bool fprint(FILE *stream,
                           const int indent) {
            return fprint(stream, indent, this);
        }
};

#endif

