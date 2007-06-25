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


#ifndef Included_oSaHpiSensorThresholds
#define Included_oSaHpiSensorThresholds

#include <stdio.h>
extern "C"
{
#include <SaHpi.h>
}
#include "oSaHpiSensorReading.hpp"


class oSaHpiSensorThresholds : public SaHpiSensorThresholdsT {
    public:
        // constructors
        oSaHpiSensorThresholds();
        // copy constructor
        oSaHpiSensorThresholds(const oSaHpiSensorThresholds& sr);
        // destructor
        ~oSaHpiSensorThresholds() {

        }
        // other methods
        inline SaHpiSensorThresholdsT *getStruct(void) {
            return this;
        }
        static bool fprint(FILE *stream,
                           const int indent,
                           const SaHpiSensorThresholdsT *ent);
        inline bool fprint(FILE *stream,
                           const int indent) {
            return fprint(stream, indent, this);
        }
};

#endif

