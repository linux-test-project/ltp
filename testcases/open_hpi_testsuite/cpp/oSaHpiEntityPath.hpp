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


#ifndef Included_oSaHpiEntityPath
#define Included_oSaHpiEntityPath

#include <stdio.h>
extern "C"
{
#include <SaHpi.h>
}

class oSaHpiEntityPath : public SaHpiEntityPathT {
    public:
        // constructors
        oSaHpiEntityPath();
        // copy constructor
        oSaHpiEntityPath(const oSaHpiEntityPath& entpath);
        // destructor
        ~oSaHpiEntityPath() {

        }
        // other methods
        inline SaHpiEntityPathT *getStruct(void) {
            return this;
        }
        static bool fprint(FILE *stream,
                           const int indent,
                           const SaHpiEntityPathT *entpath);
        inline bool fprint(FILE *stream,
                           const int indent) {
            return fprint(stream, indent, this);
        }
        bool append(SaHpiEntityPathT *destpath,
                    const SaHpiEntityPathT *appendpath);
        inline bool append(SaHpiEntityPathT *appendpath) {
            return append((SaHpiEntityPathT *)this, appendpath);
        }
        bool compare(const SaHpiEntityPathT *ep1,
                     const SaHpiEntityPathT *ep2);
        inline bool compare(const SaHpiEntityPathT *ep2) {
            return compare((SaHpiEntityPathT *)this, ep2);
        }
};

#endif

