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


#ifndef Included_oSaHpiSensorReading
#define Included_oSaHpiSensorReading

#include <stdio.h>
extern "C"
{
#include <SaHpi.h>
}


#define ISSUPPORTED_DEFAULT             false
#define SAHPISENSORREADINGTYPET_DEFAULT SAHPI_SENSOR_READING_TYPE_INT64


class oSaHpiSensorReading : public SaHpiSensorReadingT {
    public:
        // constructors
        oSaHpiSensorReading();
        // copy constructor
        oSaHpiSensorReading(const oSaHpiSensorReading& sr);
        // destructor
        ~oSaHpiSensorReading() {
        }
        // other methods
        static bool assignField(SaHpiSensorReadingT * ptr,
                                const char *field,
                                const char *value);
        inline bool assignField(const char *field,
                                const char *value) {
            return assignField(this, field, value);
        }
        inline SaHpiSensorReadingT *getStruct(void) {
            return this;
        }
        static bool fprint(FILE *stream,
                           const int indent,
                           const SaHpiSensorReadingT *ent);
        inline bool fprint(FILE *stream,
                           const int indent) {
            return fprint(stream, indent, this);
        }
        void initSensorReading(SaHpiSensorReadingT *reading);
};

#endif

